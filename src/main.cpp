#include "agora/net/http/http_context.h"
#include "agora/net/buffer.h"
#include "agora/base/timestamp.h"   // 新增
#include "agora/base/logger.h"
#include <cassert>                   // 新增

using namespace agora::net;
using agora::Timestamp;

void test1() {
    LOG_INFO("=== Test 1: GET request line ===");
    
    Buffer buf;
    buf.append("GET /hello HTTP/1.1\r\n\r\n");
    
    HttpContext context;
    bool ok = context.parseRequest(&buf, Timestamp::now());
    
    assert(ok);
    assert(context.gotAll());
    assert(context.request().method() == HttpRequest::Method::kGet);
    assert(context.request().path() == "/hello");
    assert(context.request().version() == HttpRequest::Version::kHttp11);
    
    LOG_INFO("Test 1 passed");
}

void test2() {
    LOG_INFO("=== Test 2: GET with query ===");
    
    Buffer buf;
    buf.append("GET /hello?name=agora HTTP/1.1\r\n\r\n");
    
    HttpContext context;
    bool ok = context.parseRequest(&buf, Timestamp::now());
    
    assert(ok);
    assert(context.gotAll());
    assert(context.request().path() == "/hello");
    assert(context.request().query() == "name=agora");
    
    LOG_INFO("Test 2 passed");
}

void test3() {
    LOG_INFO("=== Test 3: Invalid method ===");
    
    Buffer buf;
    buf.append("ABC / HTTP/1.1\r\n\r\n");
    
    HttpContext context;
    bool ok = context.parseRequest(&buf, Timestamp::now());
    
    assert(!ok);
    
    LOG_INFO("Test 3 passed");
}

void test4() {
    LOG_INFO("=== Test 4: Invalid version ===");
    
    Buffer buf;
    buf.append("GET / HTTP/2.0\r\n\r\n");
    
    HttpContext context;
    bool ok = context.parseRequest(&buf, Timestamp::now());
    
    assert(!ok);
    
    LOG_INFO("Test 4 passed");
}

void test5() {
    LOG_INFO("=== Test 5: reset ===");
    
    Buffer buf;
    buf.append("GET /hello HTTP/1.1\r\n\r\n");
    
    HttpContext context;
    context.parseRequest(&buf, Timestamp::now());
    assert(context.gotAll());
    
    context.reset();
    assert(!context.gotAll());
    assert(context.request().path().empty());
    
    LOG_INFO("Test 5 passed");
}

int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    
    LOG_INFO("All tests passed!");
    return 0;
}