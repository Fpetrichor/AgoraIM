#include "agora/net/timer_queue.h"
#include "agora/net/event_loop.h"
#include "agora/net/channel.h"
#include "agora/base/logger.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <string.h>
#include <cassert>

namespace agora::net {

namespace {
    // 创建 timerfd
    int createTimerfd() {
        int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        if (timerfd < 0) {
            LOG_FATAL("Failed in timerfd_create");
        }
        return timerfd;
    }

    // 计算 timespec
    struct timespec howMuchTimeFromNow(Timestamp when) {
        int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
        if (microseconds < 100) {
            microseconds = 100;
        }
        
        struct timespec ts;
        ts.tv_sec = static_cast<time_t>(microseconds / (1000 * 1000));
        ts.tv_nsec = static_cast<long>((microseconds % (1000 * 1000)) * 1000);
        return ts;
    }

    // 读取 timerfd（消耗事件）
    void readTimerfd(int timerfd, Timestamp now) {
        uint64_t howmany;
        ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
        if (n != sizeof(howmany)) {
            LOG_ERROR("TimerQueue::handleRead() reads " + std::to_string(n) + " bytes instead of 8");
        }
    }

    // 重置 timerfd 到期时间
    void resetTimerfd(int timerfd, Timestamp expiration) {
        struct itimerspec newValue;
        struct itimerspec oldValue;
        memset(&newValue, 0, sizeof(newValue));
        memset(&oldValue, 0, sizeof(oldValue));
        
        newValue.it_value = howMuchTimeFromNow(expiration);
        int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
        if (ret) {
            LOG_ERROR("timerfd_settime()");
        }
    }
}

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
      timerfd_(createTimerfd()),
      timerfdChannel_(new Channel(loop, timerfd_)) {
    
    timerfdChannel_->setReadCallback(
        std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_->enableReading();
}

TimerQueue::~TimerQueue() {
    timerfdChannel_->disableAll();
    timerfdChannel_->removeChannel();
    ::close(timerfd_);
    
    // 清理所有 Timer
    for (const Entry& entry : timers_) {
        delete entry.second;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb,
                             Timestamp when,
                             double interval) {
    Timer* timer = new Timer(std::move(cb), when, interval);
    
    loop_->runInLoop(
        std::bind(&TimerQueue::addTimerInLoop, this, timer));
    
    return TimerId(timer, timer->sequence());
}

// 新增：cancel 实现
void TimerQueue::cancel(TimerId timerId) {
    loop_->runInLoop(
        std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::cancelInLoop(TimerId timerId) {
    loop_->assertInLoopThread();
    
    if (!timerId.timer()) {
        return;
    }
    
    Timer* timer = timerId.timer();
    
    for (auto it = timers_.begin(); it != timers_.end(); ++it) {
        if (it->second == timer) {
            timers_.erase(it);
            delete timer;
            
            // 重置 timerfd
            if (!timers_.empty()) {
                resetTimerfd(timerfd_, timers_.begin()->first);
            }
            return;
        }
    }
    
    // 未找到，可能已过期正在执行
    // 暂不处理（需要 activeTimers_ 支持）
}

void TimerQueue::addTimerInLoop(Timer* timer) {
    loop_->assertInLoopThread();
    
    // 插入定时器，如果改变了最早到期时间，重置 timerfd
    bool earliestChanged = insert(timer);
    
    if (earliestChanged) {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);
    
    // 获取所有过期定时器
    std::vector<Entry> expired = getExpired(now);
    
    // 执行回调（注意：回调中可能添加新的定时器）
    for (const Entry& entry : expired) {
        entry.second->run();
    }
    
    // 重置重复定时器
    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    
    // 找到第一个未过期的定时器
    TimerList::iterator end = timers_.lower_bound(sentry);
    
    // [begin, end) 都是过期的
    std::copy(timers_.begin(), end, std::back_inserter(expired));
    timers_.erase(timers_.begin(), end);
    
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now) {
    Timestamp nextExpire;
    
    for (const Entry& entry : expired) {
        Timer* timer = entry.second;
        
        if (timer->repeat()) {
            // 重复定时器：重新计算下次触发时间
            timer->restart(now);
            insert(timer);
        } else {
            // 一次性定时器：删除
            delete timer;
        }
    }
    
    // 重置 timerfd 为下一个最早到期时间
    if (!timers_.empty()) {
        nextExpire = timers_.begin()->first;
        resetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(Timer* timer) {
    loop_->assertInLoopThread();
    assert(timers_.size() == static_cast<size_t>(
        std::distance(timers_.begin(), timers_.end())));
    
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    
    // 如果 timers_ 为空，或者新定时器比最早的还早
    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }
    
    // 插入 (expiration, timer)
    timers_.insert(Entry(when, timer));
    
    return earliestChanged;
}

} // namespace agora::net