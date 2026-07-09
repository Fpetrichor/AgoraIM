#pragma once

#include "agora/base/noncopyable.h"
#include "agora/net/tcp_connection.h"
#include "agora/net/inet_address.h"
#include <memory>
#include <string>

namespace agora::net {

class EventLoop;
class Connector;

/**
 * TcpClient
 * 
 * TCP 客户端，封装 Connector + TcpConnection
 * 
 * 使用方式：
 *   EventLoop loop;
 *   InetAddress serverAddr("127.0.0.1", 8888);
 *   TcpClient client(&loop, serverAddr, "EchoClient");
 *   
 *   client.setConnectionCallback(onConnection);
 *   client.setMessageCallback(onMessage);
 *   client.connect();  // 发起连接
 *   
 *   loop.loop();
 */
class TcpClient : public NonCopyable {
public:
    TcpClient(EventLoop* loop,
              const InetAddress& serverAddr,
              const std::string& name);
    ~TcpClient();

    /**
     * 发起连接
     */
    void connect();

    /**
     * 断开连接（优雅关闭）
     */
    void disconnect();

    /**
     * 停止客户端
     */
    void stop();

    TcpConnectionPtr connection() const {
        return connection_;
    }

    bool retry() const { return retry_; }
    void enableRetry() { retry_ = true; }
    void disableRetry() { retry_ = false; }

    const std::string& name() const { return name_; }

    // 回调设置
    void setConnectionCallback(const TcpConnection::ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const TcpConnection::MessageCallback& cb) {
        messageCallback_ = cb;
    }

    void setWriteCompleteCallback(const TcpConnection::WriteCompleteCallback& cb) {
        writeCompleteCallback_ = cb;
    }

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    std::shared_ptr<Connector> connector_;
    TcpConnectionPtr connection_;

    TcpConnection::ConnectionCallback connectionCallback_;
    TcpConnection::MessageCallback messageCallback_;
    TcpConnection::WriteCompleteCallback writeCompleteCallback_;

    std::string name_;
    bool retry_;        // 是否自动重连
    bool connect_;      // 是否保持连接
};

} // namespace agora::net