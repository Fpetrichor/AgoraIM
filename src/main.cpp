#include "agora/net/inet_address.h"
#include <iostream>

int main() {
    // 测试1：只指定端口（服务器监听）
    agora::net::InetAddress addr1(8080);
    std::cout << "addr1: " << addr1.toIp() << ":" << addr1.Port() << std::endl;
    // 期望：0.0.0.0:8080

    // 测试2：指定 IP 和端口
    agora::net::InetAddress addr2("127.0.0.1", 9090);
    std::cout << "addr2: " << addr2.toIp() << ":" << addr2.Port() << std::endl;
    // 期望：127.0.0.1:9090

    // 测试3：getSockAddr 返回指针
    const sockaddr* sa = addr2.getSockAddr();
    std::cout << "sockaddr family: " << sa->sa_family << " (AF_INET=" << AF_INET << ")" << std::endl;

    return 0;
}