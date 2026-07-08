#include "agora/net/channel.h"
#include "agora/net/event_loop.h"

namespace agora::net {

Channel::Channel(EventLoop* loop, int fd) 
    : loop_(loop), fd_(fd), events_(kNoneEvent), revents_(0), index_(kNew) {}

Channel::~Channel() = default;

void Channel::handleEvent() {
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if (closeCallback_) closeCallback_();
    }
    if (revents_ & EPOLLERR) {
        if (errorCallback_) errorCallback_();
    }
    if (revents_ & kReadEvent) {
        if (readCallback_) readCallback_();
    }
    if (revents_ & kWriteEvent) {
        if (writeCallback_) writeCallback_();
    }
}

void Channel::removeChannel() {
    loop_->removeChannel(this);
}

void Channel::enableReading() {
    events_ |= kReadEvent;
    update();
}

void Channel::disableReading() {
    events_ &= ~kReadEvent;
    update();
}

void Channel::enableWriting() {
    events_ |= kWriteEvent;
    update();
}

void Channel::disableWriting() {
    events_ &= ~kWriteEvent;
    update();
}

void Channel::disableAll() {
    events_ = kNoneEvent;
    update();
}

void Channel::update() {
    // TODO: Implement the logic to update the channel's events in the EventLoop.
    if (loop_) {
        loop_->updateChannel(this);
    }
}

} // namespace agora::net