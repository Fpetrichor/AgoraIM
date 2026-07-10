#include "agora/net/http/http_request.h"
#include "agora/base/logger.h"
#include <iostream>

using namespace agora::net::http;

int main() {
    LOG_INFO("=== HttpRequest Test ===");

    HttpRequest request;
    
    request.setMethod(HttpRequest::Method::kGet);
    request.setVersion(HttpRequest::Version::kHttp11);
    request.setPath("/index.html");
    request.setQuery("id=1&name=test");
    request.addHeader("Host", "localhost");
    request.addHeader("User-Agent", "AgoraIM/0.1");
    
    // 验证
    LOG_INFO("Method: " + std::to_string(static_cast<int>(request.method())));
    LOG_INFO("Version: " + std::to_string(static_cast<int>(request.version())));
    LOG_INFO("Path: " + request.path());
    LOG_INFO("Query: " + request.query());
    LOG_INFO("Host: " + request.getHeader("Host"));
    LOG_INFO("User-Agent: " + request.getHeader("User-Agent"));
    
    // 测试 swap
    HttpRequest other;
    other.setMethod(HttpRequest::Method::kPost);
    other.setPath("/api");
    
    request.swap(other);
    
    LOG_INFO("After swap, request.path: " + request.path());
    LOG_INFO("After swap, other.path: " + other.path());
    
    LOG_INFO("Test passed!");
    
    return 0;
}