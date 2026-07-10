#pragma once

#include "agora/net/http/http_server.h"

namespace agora::net::http {

class StaticFileServer : public NonCopyable {
public:
    StaticFileServer(EventLoop* loop,
                     const InetAddress& listenAddr,
                     const std::string& name,
                     const std::string& rootDir);

    void setThreadNum(int numThreads) {
        server_.setThreadNum(numThreads);
    }

    void start();

private:
    void onRequest(const HttpRequest& req, HttpResponse* resp);
    
    bool serveFile(const std::string& path, HttpResponse* resp);
    std::string urlDecode(const std::string& url);

    HttpServer server_;
    std::string rootDir_;
};

} // namespace agora::net::http