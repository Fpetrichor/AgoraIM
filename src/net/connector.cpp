#include "agora/net/connector.h"
#include "agora/net/event_loop.h"
#include "agora/net/channel.h"
#include "agora/net/sockets_ops.h"
#include "agora/base/logger.h"
#include <cassert>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace agora::net {

namespace {
    bool isInProgress(int err) {
        return err == EINPROGRESS || err == EINTR;
    }
}

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      state_(State::kDisconnected),
      retryDelayMs_(kInitRetryDelayMs) {}

Connector::~Connector() {
    assert(!channel_ || channel_->isNoneEvent());
}

void Connector::setState(State s) {
    state_ = s;
    // TODO: 以后加日志 LOG_TRACE("Connector state changed to " + ...)
}

void Connector::start() {
    connect_ = true;
    loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::restart() {
    loop_->assertInLoopThread();
    setState(State::kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::stop() {
    connect_ = false;
    loop_->queueInLoop(std::bind(&Connector::stopInLoop, this));   // ① 修复
}

void Connector::startInLoop() {
    LOG_INFO("Connector::startInLoop - now=" + Timestamp::now().toFormattedString() +
             ", state=" + std::to_string(static_cast<int>(state_)));

    loop_->assertInLoopThread();
    
    if (state_ != State::kDisconnected) {
        LOG_WARN("Connector::startInLoop - state is not Disconnected, current=" + 
                 std::to_string(static_cast<int>(state_)));
        return;
    }
    
    if (connect_) {
        connect();
    } else {
        LOG_INFO("Connector::startInLoop - do not connect");
    }
}

void Connector::stopInLoop() {                                    // ① 新增
    loop_->assertInLoopThread();
    
    if (state_ == State::kConnecting) {
        int sockfd = removeAndResetChannel();
        sockets::close(sockfd);
        setState(State::kDisconnected);
    }
}

void Connector::connect() {
    // 确保没有遗留的 channel_（retry 时可能还没 reset）
    if (channel_) {
        LOG_WARN("Connector::connect - channel_ still exists, reset it");
        channel_->disableAll();
        channel_->removeChannel();
        channel_.reset();
    }
    
    // ② 统一使用 sockets::createNonblockingOrDie()
    int sockfd = sockets::createNonblockingOrDie();
    
    // 发起 connect
    int ret = ::connect(sockfd, serverAddr_.getSockAddr(), 
                        static_cast<socklen_t>(sizeof(struct sockaddr_in)));
    
    int savedErrno = (ret == 0) ? 0 : errno;
    
    if (ret == 0) {
        LOG_INFO("Connector::connect - connect succeeded immediately");
        retryDelayMs_ = kInitRetryDelayMs;
        connecting(sockfd);
    } else if (isInProgress(savedErrno)) {
        LOG_INFO("Connector::connect - connect in progress");
        connecting(sockfd);
    } else {
        LOG_ERROR("Connector::connect - connect failed immediately, errno=" + 
                  std::to_string(savedErrno));
        sockets::close(sockfd);
        if (connect_) {
            retry();
        }
    }
}

void Connector::connecting(int sockfd) {
    setState(State::kConnecting);
    assert(!channel_);
    
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
    channel_->setErrorCallback(std::bind(&Connector::handleError, this));
    channel_->enableWriting();
}

void Connector::handleWrite() {
    LOG_INFO("Connector::handleWrite - state=" + 
             std::to_string(static_cast<int>(state_)));
    
    if (state_ == State::kConnecting) {
        int sockfd = removeAndResetChannel();
        
        int err = sockets::getSocketError(sockfd);
        
        if (err == 0) {
            setState(State::kConnected);
            retryDelayMs_ = kInitRetryDelayMs;
            if (connect_) {
                // ④ 防御性检查
                if (newConnectionCallback_) {
                    newConnectionCallback_(sockfd);
                } else {
                    LOG_WARN("Connector::handleWrite - no newConnectionCallback set");
                    sockets::close(sockfd);
                }
            } else {
                sockets::close(sockfd);
            }
        } else {
            LOG_ERROR("Connector::handleWrite - SO_ERROR=" + std::to_string(err));
            sockets::close(sockfd);
            if (connect_) {
                retry();
            }
        }
    } else {
    LOG_WARN("Connector::handleWrite unexpected state");
}
}

void Connector::handleError() {
    LOG_ERROR("Connector::handleError - state=" + 
              std::to_string(static_cast<int>(state_)));
    
    if (state_ == State::kConnecting) {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        LOG_ERROR("Connector::handleError - SO_ERROR=" + std::to_string(err));
        sockets::close(sockfd);
        if (connect_) {
            retry();
        }
    }
}

void Connector::retry() {
    setState(State::kDisconnected);
    
    if (connect_) {
        LOG_INFO("Connector::retry - now=" + Timestamp::now().toFormattedString() +
                 ", will retry in " + std::to_string(retryDelayMs_) + "ms");

        loop_->runAfter(retryDelayMs_ / 1000.0,
                        std::bind(&Connector::startInLoop, this));
        
        // 指数退避
        retryDelayMs_ *= 2;
        if (retryDelayMs_ > kMaxRetryDelayMs) {
            retryDelayMs_ = kMaxRetryDelayMs;
        }
    }
}

int Connector::removeAndResetChannel() {
    channel_->disableAll();
    channel_->removeChannel();
    int sockfd = channel_->fd();
    
    // ⑤ 经典写法：延迟 reset，避免在 handleEvent 中 delete 自己
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
    
    return sockfd;
}

void Connector::resetChannel() {
    channel_.reset();
}

} // namespace agora::net