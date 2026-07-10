#pragma once

#include "agora/base/noncopyable.h"
#include "agora/base/timestamp.h"
#include "agora/net/timer.h"
#include "agora/net/timer_id.h"
#include <memory>
#include <set>

namespace agora::net {

class EventLoop;
class Channel;

class TimerQueue : public NonCopyable {
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(TimerCallback cb,
                     Timestamp when,
                     double interval);

    void cancel(TimerId timerId);

private:
    using Entry = std::pair<Timestamp, Timer*>;

    using TimerList = std::set<Entry>;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId); 

    void handleRead();

    std::vector<Entry> getExpired(Timestamp now);

    void reset(const std::vector<Entry>& expired,
               Timestamp now);

    bool insert(Timer* timer);

private:
    using ActiveTimer = std::pair<int64_t, Timer*>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    EventLoop* loop_;

    const int timerfd_;

    std::unique_ptr<Channel> timerfdChannel_;

    TimerList timers_;
    ActiveTimerSet activeTimers_;

    bool callingExpiredTimers_;      // 标记是否正在执行回调
    ActiveTimerSet cancelingTimers_; // 在回调中 cancel 的定时器
};

}