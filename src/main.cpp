#include "agora/net/tcp_server.h"
#include "agora/net/event_loop.h"
#include "agora/net/inet_address.h"
#include "agora/base/logger.h"
#include <iostream>

int main() {
    LOG_INFO("=== TcpServer 测试 ===");

    agora::net::EventLoop loop;
    agora::net::InetAddress listenAddr(8888);
    
    agora::net::TcpServer server(&loop, listenAddr, "AgoraServer");
    
    server.setConnectionCallback([](const agora::net::TcpConnectionPtr& conn) {
        if (conn->connected()) {
            LOG_INFO("Connection UP: " + conn->name());
        } else {
            LOG_INFO("Connection DOWN: " + conn->name());
        }
    });
    
    server.setMessageCallback([](const agora::net::TcpConnectionPtr& conn,
                                  agora::net::Buffer* buffer) {
        std::string msg = buffer->retrieveAllAsString();
        LOG_INFO("Received from " + conn->name() + ": " + msg);
        conn->send(msg);  // echo
    });
    
    server.start();
    LOG_INFO("Server started on port 8888");
    
    loop.loop();

    return 0;
}