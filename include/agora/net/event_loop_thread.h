#pragma once

#include "agora/base/noncopyable.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <functional>

namespace agora::net {

class EventLoop;

class EventLoopThread : public NonCopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    explicit EventLoopThread(const std::string& name = "");
    ~EventLoopThread();

    EventLoop* startLoop(const ThreadInitCallback& cb = ThreadInitCallback());

private:
    void threadFunc(const ThreadInitCallback& cb);

    EventLoop* loop_;
    std::string name_;
    bool started_;
    bool exiting_;

    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

} // namespace agora::net