#pragma once

#include "agora/base/noncopyable.h"
#include "agora/base/timestamp.h"           // 新增：Timestamp 完整类型
#include "agora/net/http/http_request.h"    // 已有

namespace agora::net {

class Buffer;

// HttpRequest 在 agora::net::http 命名空间
using HttpRequest = agora::net::http::HttpRequest;   // 新增

class HttpContext : public NonCopyable {
public:
    enum class ParseState {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
    };

    HttpContext();

    bool parseRequest(Buffer* buf, Timestamp receiveTime);

    bool gotAll() const {
        return state_ == ParseState::kGotAll;
    }

    void reset();

    const HttpRequest& request() const {
        return request_;
    }

    HttpRequest& request() {
        return request_;
    }

private:
    bool processRequestLine(const char* begin, const char* end);

    ParseState state_;
    HttpRequest request_;
};

} // namespace agora::net