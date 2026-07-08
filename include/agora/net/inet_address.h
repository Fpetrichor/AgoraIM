#pragma once

#include <cstdint>
#include <string>
#include <netinet/in.h>

namespace agora::net {
class InetAddress {
public:

    InetAddress() = default; 

    explicit InetAddress(uint16_t port, bool loopbackOnly = false, bool ipv6 = false);

    InetAddress(const std::string& ip, uint16_t port, bool ipv6 = false);

    explicit InetAddress(const struct sockaddr_in& addr)
        : addr_(addr) {}

    std::string toIp() const;

    uint16_t Port() const { return ntohs(addr_.sin_port); }

    std::string toIpPort() const;

    const sockaddr_in& getSockAddrIn() const { return addr_; }

    const sockaddr* getSockAddr() const { return reinterpret_cast<const struct sockaddr*>(&addr_); }
    
    void setSockAddr(const sockaddr_in& addr) { addr_ = addr; }

private:
    struct sockaddr_in addr_{};
};

} // namespace agora::net