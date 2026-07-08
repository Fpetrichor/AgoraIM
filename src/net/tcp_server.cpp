#include "agora/net/tcp_server.h"
#include "agora/net/acceptor.h"
#include "agora/net/event_loop.h"
#include "agora/net/sockets_ops.h"
#include "agora/base/logger.h"

namespace agora::net {

TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listenAddr,
                     const std::string& name,
                     bool reuseport)
    : loop_(loop),
      ipPort_(listenAddr.toIpPort()),
      name_(name),
      acceptor_(new Acceptor(loop, listenAddr, reuseport)),
      started_(false),
      nextConnId_(1) {
    
    // 设置 Acceptor 的新连接回调
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this,
                  std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    loop_->assertInLoopThread();
    LOG_INFO("TcpServer::~TcpServer [" + name_ + "] destructing");
    
    for (auto& item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();  // 从 map 中移除引用
        conn->connectDestroyed();
    }
}

void TcpServer::start() {
    if (!started_) {
        started_ = true;
        acceptor_->listen();
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    loop_->assertInLoopThread();
    
    // 生成连接名称：servername#connId
    std::string connName = name_ + "#" + std::to_string(nextConnId_++);
    
    LOG_INFO("TcpServer::newConnection [" + name_ + 
             "] - new connection [" + connName + 
             "] from " + peerAddr.toIpPort());
    
    // 获取本地地址
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    
    // 创建 TcpConnection
    TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    
    // 设置回调
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    
    // 设置关闭回调（TcpServer 需要清理 map）
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    
    // 保存到 map
    connections_[connName] = conn;
    
    // 连接建立完成，注册可读事件
    conn->connectEstablished();
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    // 可能在其他线程被调用，需要投递到 EventLoop 线程
    removeConnectionInLoop(conn);
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    
    LOG_INFO("TcpServer::removeConnection [" + name_ + 
             "] - connection " + conn->name());
    
    connections_.erase(conn->name());
    conn->connectDestroyed();
}

} // namespace agora::net