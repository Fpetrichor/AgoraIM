#include "agora/net/socket.h"
#include "agora/net/inet_address.h"
#include <iostream>
#include <sys/socket.h>

int main() {
    // 创建 fd
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    std::cout << "Created fd: " << fd << std::endl;

    {
        // RAII 封装
        agora::net::Socket socket(fd);
        std::cout << "Socket fd: " << socket.fd() << std::endl;

        // 设置选项
        socket.setReuseAddr(true);
        socket.setReusePort(true);
        socket.setKeepAlive(true);

        // 绑定地址
        agora::net::InetAddress addr(8080);
        socket.bindAddress(addr);

        // 监听
        socket.listen();
        std::cout << "Listening on port 8080" << std::endl;

        // 这里不 accept，直接退出作用域测试析构
    }

    std::cout << "Socket destroyed, fd closed automatically" << std::endl;

    return 0;
}