#include "agora/net/tcp_connection.h"
#include "agora/net/event_loop.h"
#include "agora/net/socket.h"
#include "agora/net/channel.h"
#include "agora/net/sockets_ops.h"
#include "agora/base/logger.h"
#include <cassert>
#include <string.h>

namespace agora::net {

namespace {

const char* stateToString(TcpConnection::State state) {
    switch (state) {
        case TcpConnection::State::kDisconnected:  return "kDisconnected";
        case TcpConnection::State::kConnecting:    return "kConnecting";
        case TcpConnection::State::kConnected:     return "kConnected";
        case TcpConnection::State::kDisconnecting: return "kDisconnecting";
        default: return "unknown";
    }
}

} // namespace

TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& name,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
    : loop_(loop),
      name_(name),
      state_(State::kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      highWaterMark_(64 * 1024) {
    
    LOG_INFO("TcpConnection::TcpConnection [" + name_ + "] at fd=" + std::to_string(sockfd));
    
    // 设置 Channel 回调
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() {
    LOG_INFO("TcpConnection::~TcpConnection [" + name_ + "] at fd=" + 
             std::to_string(socket_->fd()) + " state=" + stateToString(state_));
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == State::kConnecting);
    
    setState(State::kConnected);
    channel_->enableReading();  // 注册可读事件
    
    if (connectionCallback_) {
        connectionCallback_(shared_from_this());
    }
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    
    if (state_ == State::kConnected) {
        setState(State::kDisconnected);
        channel_->disableAll();
        
        if (connectionCallback_) {
            connectionCallback_(shared_from_this());
        }
    }
    
    channel_->removeChannel();  // 从 Poller 移除
}

void TcpConnection::send(const std::string& message) {
    // TODO: 跨线程安全版本
    if (state_ == State::kConnected) {
        sendInLoop(message);
    }
}

void TcpConnection::send(Buffer* buffer) {
    if (state_ == State::kConnected) {
        sendInLoop(buffer->peek(), buffer->readableBytes());
        buffer->retrieveAll();
    }
}

void TcpConnection::sendInLoop(const std::string& message) {
    sendInLoop(message.data(), message.size());
}


void TcpConnection::sendInLoop(const void* data, size_t len) {
    loop_->assertInLoopThread();
    
    if (state_ == State::kDisconnected) {
        LOG_WARN("TcpConnection::sendInLoop - disconnected, give up writing");
        return;
    }
    
    // 如果 outputBuffer_ 为空，直接尝试写入 socket
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        ssize_t n = sockets::write(socket_->fd(), data, len);
        
        if (n >= 0) {
            size_t remaining = len - static_cast<size_t>(n);
            if (remaining == 0 && writeCompleteCallback_) {
                // 全部发送完成
                writeCompleteCallback_(shared_from_this());
            } else if (remaining > 0) {
                // 还有剩余，放入 outputBuffer_
                outputBuffer_.append(static_cast<const char*>(data) + n, remaining);
                channel_->enableWriting();  // 注册可写事件
            }
        } else {
            // 出错
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                LOG_ERROR("TcpConnection::sendInLoop error: " + std::string(strerror(errno)));
            }
            // EAGAIN，放入 buffer 等待下次可写
            outputBuffer_.append(static_cast<const char*>(data), len);
            channel_->enableWriting();
        }
    } else {
        // 正在发送中，追加到 buffer
        outputBuffer_.append(static_cast<const char*>(data), len);
    }
}

void TcpConnection::shutdown() {
    if (state_ == State::kConnected) {
        setState(State::kDisconnecting);
        shutdownInLoop();
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    
    // 如果不在写数据，直接关闭写端
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
    // 如果正在写，等 handleWrite 完成后再关闭
}

void TcpConnection::forceClose() {
    if (state_ == State::kConnected || state_ == State::kDisconnecting) {
        setState(State::kDisconnecting);
        // 直接关闭，不等待数据发送
        handleClose();
    }
}

void TcpConnection::handleRead() {
    loop_->assertInLoopThread();
    
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    
    if (n > 0) {
        // 有数据，调用业务回调
        if (messageCallback_) {
            messageCallback_(shared_from_this(), &inputBuffer_);
        }
    } else if (n == 0) {
        // 对端关闭连接
        handleClose();
    } else {
        // 出错
        if (savedErrno != EAGAIN && savedErrno != EWOULDBLOCK) {
            LOG_ERROR("TcpConnection::handleRead error: " + std::string(strerror(savedErrno)));
            handleError();
        }
    }
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    
    if (channel_->isWriting()) {
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), nullptr);
        
        if (n > 0) {
            outputBuffer_.retrieve(n);  // ← 消费已发送的数据
            
            if (outputBuffer_.readableBytes() == 0) {
                // 全部发送完成
                channel_->disableWriting();
                
                if (writeCompleteCallback_) {
                    writeCompleteCallback_(shared_from_this());
                }
                
                // 如果正在断开，发送完成后关闭
                if (state_ == State::kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            LOG_ERROR("TcpConnection::handleWrite error");
        }
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    
    LOG_INFO("TcpConnection::handleClose [" + name_ + "] state=" + stateToString(state_));
    
    assert(state_ == State::kConnected || state_ == State::kDisconnecting);
    
    // 关闭时可能还有数据未发送，但第一版简单处理
    setState(State::kDisconnected);
    channel_->disableAll();
    
    TcpConnectionPtr guardThis(shared_from_this());
    
    // 通知业务层连接断开
    if (connectionCallback_) {
        connectionCallback_(guardThis);
    }
    
    // 通知 TcpServer 移除连接
    if (closeCallback_) {
        closeCallback_(guardThis);
    }
}

void TcpConnection::handleError() {
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR("TcpConnection::handleError [" + name_ + 
              "] - SO_ERROR=" + std::to_string(err) + " " + strerror(err));
}

} // namespace agora::net