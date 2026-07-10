#include "agora/net/http/mime_type.h"

namespace agora::net::http {

std::string MimeType::getType(const std::string& path) {
    size_t pos = path.rfind('.');
    if (pos == std::string::npos) {
        return "application/octet-stream";
    }
    
    std::string ext = path.substr(pos + 1);
    
    if (ext == "html" || ext == "htm") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "json") return "application/json";
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "gif") return "image/gif";
    if (ext == "ico") return "image/x-icon";
    if (ext == "svg") return "image/svg+xml";
    if (ext == "txt") return "text/plain";
    if (ext == "pdf") return "application/pdf";
    if (ext == "zip") return "application/zip";
    if (ext == "mp4") return "video/mp4";
    
    return "application/octet-stream";
}

} // namespace agora::net::http