#pragma once

#include "agora/base/noncopyable.h"
#include "agora/net/tcp_connection.h"
#include "agora/net/inet_address.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

namespace agora::net {

class EventLoop;
class Acceptor;
class EventLoopThreadPool;

class TcpServer : public NonCopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    TcpServer(EventLoop* loop,
              const InetAddress& listenAddr,
              const std::string& name,
              bool reuseport = false);
    ~TcpServer();

    void setConnectionCallback(const TcpConnection::ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }
    void setMessageCallback(const TcpConnection::MessageCallback& cb) {
        messageCallback_ = cb;
    }
    void setWriteCompleteCallback(const TcpConnection::WriteCompleteCallback& cb) {
        writeCompleteCallback_ = cb;
    }

    void start();
    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback& cb) {
        threadInitCallback_ = cb;
    }

    const std::string& name() const { return name_; }
    const std::string& ipPort() const { return ipPort_; }

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);

    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    const std::string ipPort_;
    const std::string name_;
    
    std::unique_ptr<Acceptor> acceptor_; 
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
    ConnectionMap connections_;

    TcpConnection::ConnectionCallback connectionCallback_;
    TcpConnection::MessageCallback messageCallback_;
    TcpConnection::WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_; 

    bool started_;
    int nextConnId_;
};

} // namespace agora::net