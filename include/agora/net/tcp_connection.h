#pragma once

#include "agora/base/noncopyable.h"
#include "agora/net/buffer.h"
#include "agora/net/inet_address.h"
#include <memory>
#include <string>
#include <functional>

namespace agora::net {

class EventLoop;
class Socket;
class Channel;

class TcpConnection : public NonCopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
    using ConnectionCallback = std::function<void(const std::shared_ptr<TcpConnection>&)>;
    using MessageCallback = std::function<void(const std::shared_ptr<TcpConnection>&, Buffer*)>;
    using CloseCallback = std::function<void(const std::shared_ptr<TcpConnection>&)>;
    using WriteCompleteCallback = std::function<void(const std::shared_ptr<TcpConnection>&)>;
    using HighWaterMarkCallback = std::function<void(const std::shared_ptr<TcpConnection>&, size_t)>;

    // ① enum class
    enum class State {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };

    TcpConnection(EventLoop* loop,
                  const std::string& name,
                  int sockfd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }
    
    bool connected() const { return state_ == State::kConnected; }
    bool disconnected() const { return state_ == State::kDisconnected; }

    State state() const { return state_; }
    void setState(State s) { state_ = s; }

    void connectEstablished();
    void connectDestroyed();

    void send(const std::string& message);
    void send(Buffer* buffer);

    void shutdown();
    void forceClose();

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark) {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }

private:
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const std::string& message);
    void sendInLoop(const void* data, size_t len);

    void shutdownInLoop();

    EventLoop* loop_;
    const std::string name_;
    State state_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;

    size_t highWaterMark_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

} // namespace agora::net