#include "agora/net/sockets_ops.h"
#include "agora/base/logger.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <string.h>
#include <arpa/inet.h>

namespace agora::net::sockets {

int createNonblockingOrDie() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_FATAL(std::string("Failed to create non-blocking socket: ") + strerror(errno));
    }
    return sockfd;
}

void bindOrDie(int sockfd, const InetAddress& addr) {
    if (::bind(sockfd, addr.getSockAddr(), static_cast<socklen_t>(sizeof(addr.getSockAddrIn()))) < 0) {
        LOG_FATAL(std::string("Failed to bind socket: ") + strerror(errno));
    }
}

void listenOrDie(int sockfd) {
    if (::listen(sockfd, SOMAXCONN) < 0) {
        LOG_FATAL(std::string("Failed to listen on socket: ") + strerror(errno));
    }
}

int accept(int sockfd, InetAddress* addr) {
    struct sockaddr_in peeraddr;
    socklen_t addrlen = sizeof(peeraddr);
    int connfd = ::accept4(sockfd, reinterpret_cast<struct sockaddr*>(&peeraddr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd < 0) {
        int savedErrno = errno;
        switch (savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EMFILE:
                // Expected errors, handle gracefully
                break;
            default:
                LOG_FATAL(std::string("Unexpected error in accept: ") + strerror(savedErrno));
                break;
        }
    } else if (addr != nullptr) {
        addr->setSockAddr(peeraddr);
    }

    return connfd;
}

void close(int sockfd) {
    if (::close(sockfd) < 0) {
        LOG_ERROR(std::string("Failed to close socket: ") + strerror(errno));
    }
}

ssize_t read(int sockfd, void* buf, size_t count) {
    return ::read(sockfd, buf, count);
}

ssize_t readv(int sockfd, const struct iovec* iov, int iovcnt) {
    return ::readv(sockfd, iov, iovcnt);
}

ssize_t write(int sockfd, const void* buf, size_t count) {
    return ::write(sockfd, buf, count);
}

void shutdownWrite(int sockfd) {
    if (::shutdown(sockfd, SHUT_WR) < 0) {
        LOG_ERROR(std::string("Failed to shutdown write on socket: ") + strerror(errno));
    }
}

void setReuseAddr(int sockfd, bool on) {
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        LOG_ERROR(std::string("Failed to set SO_REUSEADDR: ") + strerror(errno));
    }
}

void setReusePort(int sockfd, bool on) {
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0) {
        LOG_ERROR(std::string("Failed to set SO_REUSEPORT: ") + strerror(errno));
    }
}

void setKeepAlive(int sockfd, bool on) {
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
        LOG_ERROR(std::string("Failed to set SO_KEEPALIVE: ") + strerror(errno));
    }
}

int getSocketError(int sockfd) {
    int optval;
    socklen_t optlen = sizeof(optval);
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}

InetAddress getLocalAddr(int sockfd) {
    struct sockaddr_in localaddr;
    socklen_t addrlen = sizeof(localaddr);
    if (::getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&localaddr), &addrlen) < 0) {
        LOG_ERROR("sockets::getLocalAddr failed");
    }
    return InetAddress(localaddr);
}

} // namespace agora::net::sockets