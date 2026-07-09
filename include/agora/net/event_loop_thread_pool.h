#pragma once

#include "agora/base/noncopyable.h"
#include <vector>
#include <memory>
#include <string>
#include <functional>

namespace agora::net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : public NonCopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& name);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }

    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    EventLoop* getNextLoop();

    bool started() const { return started_; }

    const std::vector<EventLoop*>& allLoops() const { return loops_; }

private:
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    size_t next_; 

    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

} // namespace agora::net