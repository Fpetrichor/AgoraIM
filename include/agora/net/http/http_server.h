#pragma once

#include "agora/base/noncopyable.h"
#include "agora/net/tcp_server.h"
#include "agora/net/http/http_request.h"
#include "agora/net/http/http_response.h"

namespace agora::net::http {

class HttpServer : public NonCopyable {
public:
    using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;

    HttpServer(EventLoop* loop,
               const InetAddress& listenAddr,
               const std::string& name);

    void setHttpCallback(const HttpCallback& cb) {
        httpCallback_ = cb;
    }

    void setThreadNum(int numThreads) {
        server_.setThreadNum(numThreads);
    }

    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf);

    TcpServer server_;
    HttpCallback httpCallback_;
};

} // namespace agora::net::http