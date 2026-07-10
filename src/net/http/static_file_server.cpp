#include "agora/net/http/static_file_server.h"
#include "agora/net/http/mime_type.h"
#include "agora/base/logger.h"

#include <fstream>
#include <sstream>
#include <filesystem>

namespace agora::net::http {

StaticFileServer::StaticFileServer(EventLoop* loop,
                                   const InetAddress& listenAddr,
                                   const std::string& name,
                                   const std::string& rootDir)
    : server_(loop, listenAddr, name),
      rootDir_(rootDir) {
    
    server_.setHttpCallback(
        std::bind(&StaticFileServer::onRequest, this,
                  std::placeholders::_1, std::placeholders::_2));
}

void StaticFileServer::start() {
    server_.start();
}

void StaticFileServer::onRequest(const HttpRequest& req, HttpResponse* resp) {
    std::string path = urlDecode(req.path());
    
    // 安全：防止目录遍历攻击
    if (path.find("..") != std::string::npos) {
        resp->setStatusCode(HttpResponse::HttpStatusCode::k400BadRequest);
        resp->setStatusMessage("Bad Request");
        resp->setBody("400 Bad Request");
        return;
    }
    
    // 默认 index.html
    if (path == "/") {
        path = "/index.html";
    }
    
    std::string filePath = rootDir_ + path;
    
    if (serveFile(filePath, resp)) {
        return;
    }
    
    // 404
    resp->setStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setContentType("text/html");
    resp->setBody("<h1>404 Not Found</h1>");
}

bool StaticFileServer::serveFile(const std::string& path, HttpResponse* resp) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    
    // 获取文件大小
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // 读取内容
    std::string content(size, '\0');
    file.read(content.data(), size);
    
    // 设置响应
    resp->setStatusCode(HttpResponse::HttpStatusCode::k200Ok);
    resp->setStatusMessage("OK");
    resp->setContentType(MimeType::getType(path));
    resp->setBody(content);
    
    LOG_INFO("StaticFileServer: served " + path + " (" + 
             std::to_string(size) + " bytes)");
    
    return true;
}

std::string StaticFileServer::urlDecode(const std::string& url) {
    std::string result;
    for (size_t i = 0; i < url.length(); ++i) {
        if (url[i] == '%' && i + 2 < url.length()) {
            int hex = std::stoi(url.substr(i + 1, 2), nullptr, 16);
            result += static_cast<char>(hex);
            i += 2;
        } else if (url[i] == '+') {
            result += ' ';
        } else {
            result += url[i];
        }
    }
    return result;
}

} // namespace agora::net::http