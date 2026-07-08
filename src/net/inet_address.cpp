#include "agora/net/inet_address.h"

#include <arpa/inet.h>
#include <cstring>

namespace agora::net {

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6) {
    addr_ = {};

    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    if (loopbackOnly) {
        addr_.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    } else {
        addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    }
}

InetAddress::InetAddress(const std::string& ip, uint16_t port, bool ipv6) {
    addr_ = {};

    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
        // Handle error: invalid IP address
        addr_.sin_addr.s_addr = htonl(INADDR_ANY); // Default to INADDR_ANY on error
    }
}

std::string InetAddress::toIp() const {
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return std::string(buf);
}

std::string InetAddress::toIpPort() const {
    return toIp() + ":" + std::to_string(Port());
}

} // namespace agora::net