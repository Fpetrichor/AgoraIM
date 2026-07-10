#include "agora/net/http/http_server.h"
#include "agora/net/event_loop.h"
#include "agora/net/inet_address.h"
#include "agora/base/logger.h"

using namespace agora::net;
using namespace agora::net::http;

int main() {
    EventLoop loop;
    HttpServer server(&loop, InetAddress(8080), "HttpServer");
    
    server.setHttpCallback([](const HttpRequest& req, HttpResponse* resp) {
        if (req.path() == "/") {
            resp->setStatusCode(HttpResponse::HttpStatusCode::k200Ok);
            resp->setStatusMessage("OK");
            resp->setContentType("text/html");
            resp->setBody("<h1>Hello, AgoraIM!</h1>");
        } else if (req.path() == "/hello") {
            resp->setStatusCode(HttpResponse::HttpStatusCode::k200Ok);
            resp->setStatusMessage("OK");
            resp->setContentType("text/plain");
            resp->setBody("Hello, World!");
        } else {
            resp->setStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
            resp->setStatusMessage("Not Found");
            resp->setBody("404 Not Found");
        }
    });
    
    server.setThreadNum(4);
    server.start();
    
    LOG_INFO("HttpServer listening on 8080");
    loop.loop();
    
    return 0;
}