#pragma once

#include "agora/base/noncopyable.h"
#include "inet_address.h"

namespace agora::net {

class Socket : public NonCopyable {
public:
    explicit Socket(int sockfd);

    ~Socket();

    int fd() const { return sockfd_; }

    void bindAddress(const InetAddress& addr);

    void listen();

    int accept(InetAddress* peeraddr);

    void shutdownWrite();

    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

private:
    int sockfd_;
};

} // namespace agora::net