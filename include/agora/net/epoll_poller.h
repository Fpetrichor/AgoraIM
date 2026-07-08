#pragma once

#include "agora/net/poller.h"
#include "agora/net/channel.h"

namespace agora::net {

class EPollPoller : public Poller {
public:
    explicit EPollPoller(EventLoop* loop);
    ~EPollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

    void update(int operation, Channel* channel);
    
    int epollfd_;
    std::vector<struct epoll_event> events_;
};

} // namespace agora::net