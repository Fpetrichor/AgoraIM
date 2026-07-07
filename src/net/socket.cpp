#include "agora/net/socket.h"
#include "agora/net/sockets_ops.h"

namespace agora::net {

Socket::Socket(int sockfd) : sockfd_(sockfd) {}

Socket::~Socket() {
    if (sockfd_ >= 0) {
        sockets::close(sockfd_);
    }
}

void Socket::bindAddress(const InetAddress& addr) {
    sockets::bindOrDie(sockfd_, addr);
}

void Socket::listen() {
    sockets::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress* peeraddr) {
    return sockets::accept(sockfd_, peeraddr);
}

void Socket::shutdownWrite() {
    sockets::shutdownWrite(sockfd_);
}

void Socket::setReuseAddr(bool on) {
    sockets::setReuseAddr(sockfd_, on);
}

void Socket::setReusePort(bool on) {
    sockets::setReusePort(sockfd_, on);
}

void Socket::setKeepAlive(bool on) {
    sockets::setKeepAlive(sockfd_, on);
}

} // namespace agora::net   