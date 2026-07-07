#include "agora/net/poller.h"
#include "agora/net/channel.h"
#include "agora/base/logger.h"

namespace agora::net {

Poller::Poller(EventLoop* loop)
    : ownerLoop_(loop) {}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel* channel) const {
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}

} // namespace agora::net