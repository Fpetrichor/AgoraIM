#include "agora/net/event_loop.h"
#include "agora/net/epoll_poller.h"
#include "agora/base/logger.h"
#include "agora/base/current_thread.h"
#include <cassert>

namespace agora::net {

EventLoop::EventLoop()
    : looping_(false), quit_(false), threadId_(CurrentThread::tid())
    , poller_(std::make_unique<EPollPoller>(this)) {}

EventLoop::~EventLoop() {
    assert(!looping_);
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
    }

    LOG_INFO("EventLoop stop looping");
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    // TODO: 后续加入 wakeupFd，唤醒 epoll_wait
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

} // namespace agora::net