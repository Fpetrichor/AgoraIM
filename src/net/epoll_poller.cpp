#include "agora/net/epoll_poller.h"
#include "agora/net/channel.h"
#include "agora/base/logger.h"

#include <unistd.h>
#include <sys/epoll.h>
#include <cstring>

namespace agora::net {

namespace {

const char* operationToString(int op) {
    switch (op) {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_MOD:
            return "MOD";
        case EPOLL_CTL_DEL:
            return "DEL";
        default:
            return "UNKNOWN";
    }
}
} // namespace

EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        LOG_FATAL("EPollPoller: epoll_create1 failed");
    }
}

EPollPoller::~EPollPoller() {
    if (epollfd_ >= 0) {
        ::close(epollfd_);
    }
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    int numEvents = ::epoll_wait(epollfd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);

    if (numEvents < 0) {
        LOG_ERROR("EPollPoller: epoll_wait failed");
    } else if (numEvents == 0) {
        
    } else {
        fillActiveChannels(numEvents, activeChannels);
        if (static_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    }
    return Timestamp::now();
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
    for (int i = 0; i < numEvents; ++i) {
        auto* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->setRevents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EPollPoller::updateChannel(Channel* channel) {
    const int index = channel->index();
    const int fd = channel->fd();
    
    if (index == Channel::kNew || index == Channel::kDeleted) {
        if (index == Channel::kNew) {
            channels_[fd] = channel;
        }
        channel->setIndex(Channel::kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(Channel::kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel* channel) {
    const int fd = channel->fd();
    
    if (channel->index() == Channel::kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }

    channels_.erase(fd);
    channel->setIndex(Channel::kNew);
}

void EPollPoller::update(int operation, Channel* channel) {
    epoll_event event{};
    event.events = channel->events();
    event.data.ptr = channel;

    if (::epoll_ctl(epollfd_, operation, channel->fd(), &event) < 0) {
        LOG_ERROR(
            std::string("epoll_ctl ")
            + operationToString(operation)
            + " failed: "
            + strerror(errno));
    }
}

} // namespace agora::net