// src/main.cpp
#include "agora/net/event_loop.h"
#include "agora/net/tcp_server.h"
#include "agora/net/connector.h"
#include "agora/base/logger.h"
#include <iostream>

int main() {
    LOG_INFO("AgoraIM server starting...");
    
    agora::net::EventLoop loop;
    agora::net::InetAddress addr(8888);
    agora::net::TcpServer server(&loop, addr, "AgoraIM");
    
    server.setThreadNum(4);
    server.start();
    
    LOG_INFO("Server listening on 8888");
    loop.loop();
    
    return 0;
}