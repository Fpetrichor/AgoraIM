#pragma once

#include <string>
#include <unordered_map>

namespace agora::net::http {

class HttpRequest {
public:
    enum class Method {
        kInvalid,
        kGet,
        kPost,
        kHead,
        kPut,
        kDelete
    };

    enum class Version {
        kUnknown,
        kHttp10,
        kHttp11
    };

    using HeaderMap = std::unordered_map<std::string, std::string>;

    HttpRequest()
        : method_(Method::kInvalid),
          version_(Version::kUnknown) {}

    void setMethod(Method method) { method_ = method; }
    Method method() const { return method_; }

    void setVersion(Version version) { version_ = version; }
    Version version() const { return version_; }

    void setPath(const std::string& path) { path_ = path; }
    const std::string& path() const { return path_; }

    void setQuery(const std::string& query) { query_ = query; }
    const std::string& query() const { return query_; }

    void addHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }

    std::string getHeader(const std::string& key) const {
        auto it = headers_.find(key);
        if (it != headers_.end()) {
            return it->second;
        }
        return "";
    }

    const HeaderMap& headers() const { return headers_; }

    void swap(HttpRequest& that) {
        std::swap(method_, that.method_);
        std::swap(version_, that.version_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        headers_.swap(that.headers_);
    }

private:
    Method method_;
    Version version_;
    std::string path_;
    std::string query_;
    HeaderMap headers_;
};

} // namespace agora::net::http