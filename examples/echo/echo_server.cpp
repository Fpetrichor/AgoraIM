#include "agora/net/tcp_server.h"
#include "agora/net/event_loop.h"
#include "agora/net/tcp_connection.h"
#include "agora/net/buffer.h"
#include "agora/base/logger.h"

using namespace agora::net;

void onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        LOG_INFO("UP " + conn->peerAddress().toIpPort());
    } else {
        LOG_INFO("DOWN " + conn->peerAddress().toIpPort());
    }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf) {
    std::string msg = buf->retrieveAllAsString();
    LOG_INFO("Echo " + std::to_string(msg.size()) + " bytes from " + conn->name());
    conn->send(msg);
}

int main() {
    LOG_INFO("=== EchoServer starting ===");

    EventLoop loop;
    InetAddress listenAddr(8888);
    
    TcpServer server(&loop, listenAddr, "EchoServer");
    
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.setThreadNum(4);
    
    server.start();
    
    LOG_INFO("EchoServer listening on " + listenAddr.toIpPort());
    
    loop.loop();
    
    return 0;
}