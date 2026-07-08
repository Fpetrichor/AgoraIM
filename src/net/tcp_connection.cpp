#include "agora/net/tcp_connection.h"
#include "agora/net/event_loop.h"
#include "agora/net/socket.h"
#include "agora/net/channel.h"
#include "agora/net/sockets_ops.h"
#include "agora/base/logger.h"
#include <cassert>

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
    // TODO
}

void TcpConnection::shutdown() {
    // TODO: 跨线程安全版本
    if (state_ == State::kConnected) {
        setState(State::kDisconnecting);
        shutdownInLoop();
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    // TODO
}

void TcpConnection::forceClose() {
    // TODO
}

// ========== Channel 回调（TODO）==========

void TcpConnection::handleRead() {
    loop_->assertInLoopThread();
    // TODO: 从 socket 读到 inputBuffer_
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    // TODO: 从 outputBuffer_ 写到 socket
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    // TODO: 连接关闭
}

void TcpConnection::handleError() {
    // TODO: 错误处理
}

} // namespace agora::net