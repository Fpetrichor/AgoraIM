#include "agora/net/http/http_server.h"
#include "agora/net/http/http_context.h"
#include "agora/net/tcp_connection.h"
#include "agora/base/logger.h"

namespace agora::net::http {

HttpServer::HttpServer(EventLoop* loop,
                       const InetAddress& listenAddr,
                       const std::string& name)
    : server_(loop, listenAddr, name) {
    
    server_.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(
        std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
}

void HttpServer::start() {
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        conn->setContext(std::make_shared<HttpContext>());
    }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf) {
    auto context = std::static_pointer_cast<HttpContext>(conn->getContext());
    
    if (!context->parseRequest(buf, Timestamp::now())) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }
    
    if (context->gotAll()) {
        HttpRequest& request = context->request();
        HttpResponse response(true);
        
        if (httpCallback_) {
            httpCallback_(request, &response);
        } else {
            response.setStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
            response.setStatusMessage("Not Found");
        }
        
        Buffer buf;
        response.appendToBuffer(&buf);
        conn->send(&buf);
        
        if (response.closeConnection()) {
            conn->shutdown();
        } else {
            context->reset();
        }
    }
}

} // namespace agora::net::http