#include "agora/net/acceptor.h"
#include "agora/net/event_loop.h"
#include "agora/net/sockets_ops.h"
#include "agora/base/logger.h"
#include <cassert>
#include <string.h>

namespace agora::net {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_(loop),
      acceptSocket_(sockets::createNonblockingOrDie()),
      acceptChannel_(loop, acceptSocket_.fd()),
      listening_(false) {
    
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    
    acceptChannel_.setReadCallback(
        std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() = default;

void Acceptor::listen() {
    loop_->assertInLoopThread();

    assert(!listening_);
    
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
    
    LOG_INFO("Acceptor listening");
}

void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    
    InetAddress peerAddr;
    
    while (true) {
        int connfd = acceptSocket_.accept(&peerAddr);
        
        if (connfd >= 0) {
            // 成功，交给上层
            if (newConnectionCallback_) {
                newConnectionCallback_(connfd, peerAddr);
            } else {
                sockets::close(connfd);
            }
        } else {
            break;
        }
    }
}

} // namespace agora::net