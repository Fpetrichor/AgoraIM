#include "agora/net/http/http_context.h"
#include "agora/net/buffer.h"
#include "agora/base/timestamp.h"
#include "agora/base/logger.h"

#include <algorithm>   // 新增：std::find, std::search

namespace agora::net {

HttpContext::HttpContext()
    : state_(ParseState::kExpectRequestLine) {}

bool HttpContext::parseRequest(Buffer* buf, Timestamp receiveTime) {
    bool ok = true;
    bool hasMore = true;
    
    while (hasMore) {
        if (state_ == ParseState::kExpectRequestLine) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                ok = processRequestLine(buf->peek(), crlf);
                if (ok) {
                    state_ = ParseState::kExpectHeaders;
                    buf->retrieveUntil(crlf + 2);
                } else {
                    hasMore = false;
                }
            } else {
                hasMore = false;
            }
        } else if (state_ == ParseState::kExpectHeaders) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                const char* colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf) {
                    request_.addHeader(
                        std::string(buf->peek(), colon),
                        std::string(colon + 1, crlf));
                } else {
                    state_ = ParseState::kGotAll;
                }
                buf->retrieveUntil(crlf + 2);
            } else {
                hasMore = false;
            }
        } else if (state_ == ParseState::kExpectBody) {
            hasMore = false;
        } else if (state_ == ParseState::kGotAll) {
            hasMore = false;
        }
    }
    
    return ok;
}

bool HttpContext::processRequestLine(const char* begin, const char* end) {
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');
    
    if (space != end) {
        std::string method(start, space);
        if (method == "GET") {
            request_.setMethod(HttpRequest::Method::kGet);
        } else if (method == "POST") {
            request_.setMethod(HttpRequest::Method::kPost);
        } else if (method == "HEAD") {
            request_.setMethod(HttpRequest::Method::kHead);
        } else if (method == "PUT") {
            request_.setMethod(HttpRequest::Method::kPut);
        } else if (method == "DELETE") {
            request_.setMethod(HttpRequest::Method::kDelete);
        } else {
            return false;
        }
        
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end) {
            std::string pathAndQuery(start, space);
            const char* question = std::find(start, space, '?');
            if (question != space) {
                request_.setPath(std::string(start, question));
                request_.setQuery(std::string(question + 1, space));
            } else {
                request_.setPath(pathAndQuery);
            }
            
            start = space + 1;
            if (end - start == 8 && std::equal(start, end, "HTTP/1.1")) {
                request_.setVersion(HttpRequest::Version::kHttp11);
                succeed = true;
            } else if (end - start == 8 && std::equal(start, end, "HTTP/1.0")) {
                request_.setVersion(HttpRequest::Version::kHttp10);
                succeed = true;
            }
        }
    }
    
    return succeed;
}

void HttpContext::reset() {
    state_ = ParseState::kExpectRequestLine;
    HttpRequest dummy;
    request_.swap(dummy);
}

} // namespace agora::net