#pragma once

#include "agora/net/buffer.h"
#include <string>
#include <unordered_map>

namespace agora::net::http {

class HttpResponse {
public:
    enum class HttpStatusCode {
        kUnknown = 0,
        k200Ok = 200,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404,
        k500InternalServerError = 500
    };

    using HeaderMap = std::unordered_map<std::string, std::string>;

    explicit HttpResponse(bool close)
        : statusCode_(HttpStatusCode::kUnknown),
          closeConnection_(close) {}

    void setStatusCode(HttpStatusCode code) { statusCode_ = code; }
    void setStatusMessage(const std::string& message) { statusMessage_ = message; }

    void setCloseConnection(bool on) { closeConnection_ = on; }
    bool closeConnection() const { return closeConnection_; }

    void setContentType(const std::string& contentType) {
        addHeader("Content-Type", contentType);
    }

    void addHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }

    void setBody(const std::string& body) { body_ = body; }

    /**
     * 将 HTTP 响应写入 Buffer
     */
    void appendToBuffer(Buffer* output) const;

private:
    HttpStatusCode statusCode_;
    std::string statusMessage_;
    bool closeConnection_;
    HeaderMap headers_;
    std::string body_;
};

} // namespace agora::net::http