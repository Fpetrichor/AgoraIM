#pragma once

#include "agora/base/noncopyable.h"
#include "agora/base/timestamp.h"
#include <vector>
#include <unordered_map>

namespace agora::net {

class Channel;
class EventLoop;

class Poller : public NonCopyable {
public:
    using ChannelList = std::vector<Channel*>;

    explicit Poller(EventLoop* loop);

    virtual ~Poller();

    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

    virtual void updateChannel(Channel* channel) = 0;

    virtual void removeChannel(Channel* channel) = 0;

    bool hasChannel(Channel* channel) const;

    EventLoop* ownerLoop() const { return ownerLoop_; }

protected:
    using ChannelMap = std::unordered_map<int, Channel*>;

    EventLoop* ownerLoop_;
    ChannelMap channels_;
};

} // namespace agora::net