#include "agora/net/event_loop_thread_pool.h"
#include "agora/net/event_loop_thread.h"
#include "agora/net/event_loop.h"
#include "agora/base/logger.h"
#include <cassert>

namespace agora::net {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& name)
    : baseLoop_(baseLoop),
      name_(name),
      started_(false),
      numThreads_(0),
      next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
    assert(!started_);
    assert(numThreads_ >= 0);

    baseLoop_->assertInLoopThread();

    started_ = true;

    // 创建工作线程
    for (int i = 0; i < numThreads_; ++i) {
        std::string threadName = name_ + "-" + std::to_string(i);
        
        LOG_INFO("Create EventLoopThread " + threadName);
        auto thread = std::make_unique<EventLoopThread>(threadName);
        EventLoop* loop = thread->startLoop(cb);
        
        threads_.push_back(std::move(thread));
        loops_.push_back(loop);
    }

    LOG_INFO("EventLoopThreadPool [" + name_ + "] started with " + 
             std::to_string(numThreads_) + " threads");
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);

    // 如果没有工作线程，返回 baseLoop
    if (loops_.empty()) {
        return baseLoop_;
    }

    // Round Robin
    EventLoop* loop = loops_[next_];
    ++next_;
    if (next_ >= loops_.size()) {
        next_ = 0;
    }

    return loop;
}

} // namespace agora::net