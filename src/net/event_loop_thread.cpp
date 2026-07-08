#include "agora/net/event_loop_thread.h"
#include "agora/net/event_loop.h"
#include "agora/base/logger.h"
#include <cassert>

namespace agora::net {

EventLoopThread::EventLoopThread(const std::string& name)
    : loop_(nullptr),
      name_(name),
      started_(false),
      exiting_(false) {}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    
    if (thread_.joinable()) {
        if (loop_) {
            loop_->quit();
        }
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop() {
    assert(!started_);
    started_ = true;
    
    LOG_INFO("EventLoopThread [" + name_ + "] starting...");

    thread_ = std::thread(&EventLoopThread::threadFunc, this);

    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this]() { return loop_ != nullptr; });
        loop = loop_;
    }
    
    LOG_INFO("EventLoopThread [" + name_ + "] started");

    return loop;
}

void EventLoopThread::threadFunc() {
    LOG_INFO("EventLoopThread [" + name_ + "] begin loop");

    EventLoop loop;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
    }
    cond_.notify_one();

    loop.loop();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = nullptr;
    }
    cond_.notify_all();
    
    LOG_INFO("EventLoopThread [" + name_ + "] exit");
}

} // namespace agora::net
