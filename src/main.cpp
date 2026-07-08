#include "agora/net/event_loop_thread.h"
#include "agora/net/event_loop.h"
#include "agora/base/logger.h"
#include <iostream>
#include <unistd.h>

int main() {
    LOG_INFO("=== EventLoopThread 测试 ===");

    agora::net::EventLoopThread thread;
    
    // 启动线程，获取 EventLoop 指针
    agora::net::EventLoop* loop = thread.startLoop();
    
    LOG_INFO("EventLoop started in another thread");
    
    // 可以在这个 loop 上操作（后续会实现 runInLoop）
    // 现在简单测试：主线程 sleep，然后 quit
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    LOG_INFO("Quitting loop...");
    loop->quit();
    
    LOG_INFO("Test completed");

    return 0;
}