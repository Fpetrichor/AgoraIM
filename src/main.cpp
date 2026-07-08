#include "agora/net/event_loop.h"
#include "agora/net/acceptor.h"
#include "agora/net/inet_address.h"
#include "agora/base/logger.h"
#include "agora/net/sockets_ops.h"

int main() {
    LOG_INFO("=== Acceptor 测试 ===");

    agora::net::EventLoop loop;
    agora::net::InetAddress listenAddr(8080);
    
    agora::net::Acceptor acceptor(&loop, listenAddr, false);
    
    acceptor.setNewConnectionCallback([](int sockfd, const agora::net::InetAddress& peerAddr) {
        LOG_INFO("New connection from " + peerAddr.toIpPort() + 
                 ", fd=" + std::to_string(sockfd));
        agora::net::sockets::close(sockfd);
    });
    
    acceptor.listen();
    LOG_INFO("Server running on port 8080");
    
    loop.loop();

    return 0;
}