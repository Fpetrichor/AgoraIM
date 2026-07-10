#include "agora/net/event_loop.h"
#include "agora/net/epoll_poller.h"
#include "agora/net/channel.h"
#include "agora/net/timer_queue.h"
#include "agora/base/logger.h"
#include "agora/base/current_thread.h"
#include <cassert>
#include <unistd.h>
#include <sys/eventfd.h>

namespace agora::net {

namespace {
    int createEventfd() {
        int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (evtfd < 0) {
            LOG_FATAL("Failed in eventfd");
        }
        return evtfd;
    }
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      threadId_(CurrentThread::tid()),
      poller_(std::make_unique<EPollPoller>(this)),
      callingPendingFunctors_(false),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      timerQueue_(new TimerQueue(this)) {
    
    wakeupChannel_->setReadCallback(
        std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    assert(!looping_);
    
    wakeupChannel_->disableAll();
    wakeupChannel_->removeChannel();
    
    ::close(wakeupFd_);
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();

    looping_ = true;
    quit_ = false;

    LOG_INFO("EventLoop start looping");

    while (!quit_) {
        activeChannels_.clear();
        
        poller_->poll(10000, &activeChannels_);
        
        for (Channel* channel : activeChannels_) {
            channel->handleEvent();
        }
        
        doPendingFunctors();
    }

    LOG_INFO("EventLoop stop looping");
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::updateChannel(Channel* channel) {
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    assertInLoopThread();
    poller_->removeChannel(channel);
}

bool EventLoop::isInLoopThread() const {
    return threadId_ == CurrentThread::tid();
}

void EventLoop::assertInLoopThread() const {
    if (!isInLoopThread()) {
        LOG_FATAL("EventLoop::assertInLoopThread - thread mismatch!");
    }
}

void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }

    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("EventLoop::wakeup() writes " + std::to_string(n) + " bytes instead of 8");
    }
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb) {
    return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb) {
    Timestamp time(Timestamp::addTime(Timestamp::now(), delay));
    return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb) {
    Timestamp time(Timestamp::addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(
        TimerId timerId)
{
    timerQueue_->cancel(timerId);
}

void EventLoop::handleRead() {
    uint64_t one;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("EventLoop::handleRead() reads " + std::to_string(n) + " bytes instead of 8");
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor& functor : functors) {
        functor();
    }

    callingPendingFunctors_ = false;
}

} // namespace agora::net