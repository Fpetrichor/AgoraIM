#pragma once

#include "agora/base/noncopyable.h"
#include <functional>
#include <cstdint>
#include <sys/epoll.h>

namespace agora::net {

static constexpr int kNoneEvent = 0;
static constexpr int kReadEvent = EPOLLIN | EPOLLPRI;
static constexpr int kWriteEvent = EPOLLOUT;

class EventLoop;

class Channel : public NonCopyable {
public:
    using EventCallback = std::function<void()>;
    using Event = uint32_t;
    
    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent();

    void setReadCallback(const EventCallback& cb) { readCallback_ = cb; }
    void setWriteCallback(const EventCallback& cb) { writeCallback_ = cb; }
    void setCloseCallback(const EventCallback& cb) { closeCallback_ = cb; }
    void setErrorCallback(const EventCallback& cb) { errorCallback_ = cb; }

    int fd() const { return fd_; }
    int index() const { return index_; }
    void setIndex(int idx) { index_ = idx; }

    uint32_t events() const { return events_; }
    uint32_t revents() const { return revents_; }
    void setRevents(uint32_t revents) { revents_ = revents; }
    
    bool isNoneEvent() const { return events_ == 0; }

    void enableReading();
    void disableReading();
    void enableWriting();
    void disableWriting();
    void disableAll();

private:
    void update();
    
private:
    EventLoop* loop_;
    const int fd_;
    Event events_;
    Event revents_;
    int index_ = -1; // used by Poller

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

} // namespace agora::net   