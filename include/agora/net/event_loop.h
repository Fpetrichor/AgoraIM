#pragma once

#include "agora/base/noncopyable.h"
#include <memory>
#include <thread>
#include <vector>
#include <functional>
#include <mutex>

namespace agora::net {

class Channel;
class Poller;

class EventLoop : public NonCopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    bool isInLoopThread() const;
    void assertInLoopThread() const;

    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    void wakeup();

private:
    void handleRead();
    void doPendingFunctors();

    using ChannelList = std::vector<Channel*>;

    bool looping_;
    bool quit_;

    const std::thread::id threadId_;

    ChannelList activeChannels_;
    std::unique_ptr<Poller> poller_;

    std::vector<Functor> pendingFunctors_;
    std::mutex mutex_;
    bool callingPendingFunctors_;

    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
};

} // namespace agora::net