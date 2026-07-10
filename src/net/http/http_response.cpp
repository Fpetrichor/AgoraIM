#include "agora/net/http/http_response.h"

namespace agora::net::http {

void HttpResponse::appendToBuffer(Buffer* output) const {
    // 状态行：HTTP/1.1 200 OK\r\n
    output->append("HTTP/1.1 ");
    output->append(std::to_string(static_cast<int>(statusCode_)));
    output->append(" ");
    output->append(statusMessage_);
    output->append("\r\n");

    // 头部
    if (!headers_.empty()) {
        for (const auto& header : headers_) {
            output->append(header.first);
            output->append(": ");
            output->append(header.second);
            output->append("\r\n");
        }
    }

    // Content-Length
    output->append("Content-Length: ");
    output->append(std::to_string(body_.size()));
    output->append("\r\n");

    // Connection
    if (closeConnection_) {
        output->append("Connection: close\r\n");
    } else {
        output->append("Connection: Keep-Alive\r\n");
    }

    // 空行
    output->append("\r\n");

    // Body
    output->append(body_);
}

} // namespace agora::net::http