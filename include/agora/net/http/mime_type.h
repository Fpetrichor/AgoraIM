#pragma once

#include <string>

namespace agora::net::http {

class MimeType {
public:
    static std::string getType(const std::string& path);
};

} // namespace agora::net::http