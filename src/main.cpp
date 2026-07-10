#include "agora/net/http/http_response.h"
#include "agora/net/buffer.h"
#include "agora/base/logger.h"

using namespace agora::net;
using namespace agora::net::http;

int main() {
    LOG_INFO("=== HttpResponse Test ===");

    // 测试1：200 OK
    {
        HttpResponse response(true);  // close connection
        response.setStatusCode(HttpResponse::HttpStatusCode::k200Ok);
        response.setStatusMessage("OK");
        response.setContentType("text/plain");
        response.setBody("Hello Agora");

        Buffer buf;
        response.appendToBuffer(&buf);

        std::string output = buf.retrieveAllAsString();
        LOG_INFO("Response:\n" + output);
    }

    // 测试2：404 Not Found
    {
        HttpResponse response(true);
        response.setStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
        response.setStatusMessage("Not Found");
        response.setContentType("text/html");
        response.setBody("<h1>404 Not Found</h1>");

        Buffer buf;
        response.appendToBuffer(&buf);

        std::string output = buf.retrieveAllAsString();
        LOG_INFO("404 Response:\n" + output);
    }

    LOG_INFO("Test passed!");

    return 0;
}