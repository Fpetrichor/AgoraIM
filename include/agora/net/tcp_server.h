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

/**
 * TcpServer 管理所有 TCP 连接
 * 
 * 职责：
 * 1. 创建 Acceptor 监听端口
 * 2. 收到新连接时创建 TcpConnection
 * 3. 保存所有活跃连接
 * 4. 连接关闭时清理资源
 */
class TcpServer : public NonCopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    TcpServer(EventLoop* loop,
              const InetAddress& listenAddr,
              const std::string& name,
              bool reuseport = false);
    ~TcpServer();

    /**
     * 设置回调（必须在 start() 前调用）
     */
    void setConnectionCallback(const TcpConnection::ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }
    void setMessageCallback(const TcpConnection::MessageCallback& cb) {
        messageCallback_ = cb;
    }
    void setWriteCompleteCallback(const TcpConnection::WriteCompleteCallback& cb) {
        writeCompleteCallback_ = cb;
    }

    /**
     * 开始监听，启动服务器
     */
    void start();

    const std::string& name() const { return name_; }
    const std::string& ipPort() const { return ipPort_; }

private:
    /**
     * Acceptor 的新连接回调
     * 创建 TcpConnection 并保存
     */
    void newConnection(int sockfd, const InetAddress& peerAddr);

    /**
     * TcpConnection 的关闭回调
     * 从 connections_ 中移除
     */
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    EventLoop* loop_;           // 主 EventLoop（accept loop）
    const std::string ipPort_;  // "0.0.0.0:8888"
    const std::string name_;    // 服务器名称
    
    std::unique_ptr<Acceptor> acceptor_;  // 监听器
    
    // 所有活跃连接
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
    ConnectionMap connections_;

    // 回调
    TcpConnection::ConnectionCallback connectionCallback_;
    TcpConnection::MessageCallback messageCallback_;
    TcpConnection::WriteCompleteCallback writeCompleteCallback_;

    bool started_;  // 是否已启动
    int nextConnId_;  // 连接编号，用于生成唯一 name
};

} // namespace agora::net