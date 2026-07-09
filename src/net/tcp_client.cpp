#include "agora/net/tcp_client.h"
#include "agora/net/connector.h"
#include "agora/net/event_loop.h"
#include "agora/net/sockets_ops.h"
#include "agora/base/logger.h"
#include <cassert>

namespace agora::net {

TcpClient::TcpClient(EventLoop* loop,
                     const InetAddress& serverAddr,
                     const std::string& name)
    : loop_(loop),
      connector_(new Connector(loop, serverAddr)),
      retry_(false),
      connect_(false),
      name_(name) {
    
    connector_->setNewConnectionCallback(
        std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
}

TcpClient::~TcpClient() {
    LOG_INFO("TcpClient::~TcpClient [" + name_ + "] destructing");

    connector_->stop();
    
    if (connection_) {
        assert(loop_->isInLoopThread());
        
        TcpConnectionPtr conn(connection_);
        connection_.reset();
        
        conn->setCloseCallback(TcpConnection::CloseCallback());
        
        loop_->queueInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpClient::connect() {
    LOG_INFO("TcpClient::connect [" + name_ + "] - connecting to " + 
             connector_->serverAddress().toIpPort());
    
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect() {
    connect_ = false;
    
    if (connection_) {
        connection_->shutdown();
    }
}

void TcpClient::stop() {
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd) {
    loop_->assertInLoopThread();
    
    LOG_INFO("TcpClient::newConnection [" + name_ + 
             "] - new connection established, sockfd=" + std::to_string(sockfd));
    
    // 直接使用 sockets_ops
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    
    connection_ = std::make_shared<TcpConnection>(
    loop_,
    name_,
    sockfd,
    localAddr,
    peerAddr);
    
    connection_->setConnectionCallback(connectionCallback_);
    connection_->setMessageCallback(messageCallback_);
    connection_->setWriteCompleteCallback(writeCompleteCallback_);
    
    connection_->setCloseCallback(
        std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
    
    connection_->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    
    assert(connection_ == conn);
    
    LOG_INFO("TcpClient::removeConnection [" + name_ + 
             "] - connection " + conn->name() + " removed");
    
    connection_.reset();
    
    loop_->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
    
    if (retry_ && connect_) {
        LOG_INFO("TcpClient::removeConnection [" + name_ + "] - retrying...");
        connector_->restart();
    }
}

} // namespace agora::net