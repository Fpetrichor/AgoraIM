#pragma once

#include "agora/base/noncopyable.h"
#include <memory>
#include <thread>
#include <vector>

namespace agora::net {

class Channel;
class Poller;

class EventLoop : public NonCopyable {
public:
    EventLoop();
    ~EventLoop();

    void loop();

    void quit();

    void updateChannel(Channel* channel);

    void removeChannel(Channel* channel);
    
    bool isInLoopThread() const;

    void assertInLoopThread() const;

private:
    using ChannelList = std::vector<Channel*>;

    bool looping_;
    bool quit_;

    const std::thread::id threadId_;

    ChannelList activeChannels_;
    std::unique_ptr<Poller> poller_;
};

} // namespace agora::net