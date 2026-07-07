#include "agora/net/channel.h"
#include "agora/base/logger.h"
#include <iostream>

// 模拟 EventLoop（暂时用 nullptr）
class FakeLoop {};

int main() {
    LOG_INFO("=== Channel 测试 ===");

    // 创建一个 Channel，绑定 stdout fd (1)
    agora::net::Channel channel(nullptr, 1);

    // 设置回调
    channel.setReadCallback([]() {
        std::cout << "read callback triggered\n";
    });

    channel.setWriteCallback([]() {
        std::cout << "write callback triggered\n";
    });

    channel.setErrorCallback([]() {
        std::cout << "error callback triggered\n";
    });

    channel.setCloseCallback([]() {
        std::cout << "close callback triggered\n";
    });

    // 测试 enableReading
    channel.enableReading();
    std::cout << "events after enableReading: " << channel.events() << " (EPOLLIN=" << EPOLLIN << ")\n";

    // 测试 enableWriting
    channel.enableWriting();
    std::cout << "events after enableWriting: " << channel.events() << "\n";

    // 模拟 epoll 返回 EPOLLIN
    channel.setRevents(EPOLLIN);
    std::cout << "\n模拟 EPOLLIN:\n";
    channel.handleEvent();

    // 模拟 epoll 返回 EPOLLOUT
    channel.setRevents(EPOLLOUT);
    std::cout << "\n模拟 EPOLLOUT:\n";
    channel.handleEvent();

    // 模拟 epoll 返回 EPOLLIN | EPOLLERR
    channel.setRevents(EPOLLIN | EPOLLERR);
    std::cout << "\n模拟 EPOLLIN | EPOLLERR:\n";
    channel.handleEvent();

    // 测试 disableWriting
    channel.disableWriting();
    std::cout << "\nevents after disableWriting: " << channel.events() << "\n";

    // 测试 disableAll
    channel.disableAll();
    std::cout << "events after disableAll: " << channel.events() << "\n";

    return 0;
}