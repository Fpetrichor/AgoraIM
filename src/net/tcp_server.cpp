#include "agora/net/tcp_server.h"
#include "agora/net/acceptor.h"
#include "agora/net/event_loop.h"
#include "agora/net/sockets_ops.h"
#include "agora/net/event_loop_thread_pool.h"
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
      threadPool_(new EventLoopThreadPool(loop, name)),
      started_(false),
      nextConnId_(1) {
    
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this,
                  std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    loop_->assertInLoopThread();
    LOG_INFO("TcpServer::~TcpServer [" + name_ + "] destructing");
    
    for (auto& item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset(); 
        
        conn->connectDestroyed();
    }
}

void TcpServer::setThreadNum(int numThreads) {
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
    if (!started_) {
        started_ = true;
        threadPool_->start(threadInitCallback_);
        acceptor_->listen();
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    loop_->assertInLoopThread();

    EventLoop* ioLoop = threadPool_->getNextLoop();
    
    std::string connName = name_ + "#" + std::to_string(nextConnId_++);
    
    LOG_INFO("TcpServer::newConnection [" + name_ + 
             "] - new connection [" + connName + 
             "] from " + peerAddr.toIpPort());
    
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    
    connections_[connName] = conn;
    
    conn->connectEstablished();
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
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