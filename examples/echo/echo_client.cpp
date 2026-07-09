#include "agora/net/tcp_client.h"
#include "agora/net/event_loop.h"
#include "agora/net/tcp_connection.h"
#include "agora/net/buffer.h"
#include "agora/base/logger.h"
#include <iostream>

using namespace agora::net;

void onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        LOG_INFO("Connected to " + conn->peerAddress().toIpPort());
        conn->send("Hello from EchoClient!\n");
    } else {
        LOG_INFO("Disconnected from server");
    }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf) {
    std::string msg = buf->retrieveAllAsString();
    LOG_INFO("Received: " + msg);
    
    // 收到回显后，再发一条，然后断开
    static int count = 0;
    if (++count < 3) {
        conn->send("Message " + std::to_string(count) + "\n");
    } else {
        LOG_INFO("All messages echoed, disconnecting...");
        conn->shutdown();
    }
}

int main() {
    LOG_INFO("=== EchoClient starting ===");

    EventLoop loop;
    InetAddress serverAddr("127.0.0.1", 8888);
    
    TcpClient client(&loop, serverAddr, "EchoClient");
    
    client.setConnectionCallback(onConnection);
    client.setMessageCallback(onMessage);
    // client.enableRetry();
    
    client.connect();
    
    loop.loop();
    
    return 0;
}