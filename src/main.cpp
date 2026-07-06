#include "agora/base/logger.h"
#include <thread>
#include <iostream>

void test_basic() {
    std::cout << "=== 基础测试 ===" << std::endl;
    
    LOG_INFO("Server started");
    LOG_WARN("Client disconnected");
    LOG_ERROR("Connect failed");
    
    std::cout << std::endl;
}

void test_level_filter() {
    std::cout << "=== 等级过滤测试 ===" << std::endl;
    
    agora::Logger::instance().setLogLevel(agora::LogLevel::WARN);
    
    LOG_INFO("This should NOT appear");   // 被过滤
    LOG_WARN("This should appear");
    LOG_ERROR("This should also appear");
    
    // 恢复默认
    agora::Logger::instance().setLogLevel(agora::LogLevel::INFO);
    
    std::cout << std::endl;
}

void test_multithread() {
    std::cout << "=== 多线程测试 ===" << std::endl;
    
    std::thread t1([]() {
        LOG_INFO("Thread 1 message");
    });
    
    std::thread t2([]() {
        LOG_WARN("Thread 2 message");
    });
    
    t1.join();
    t2.join();
    
    std::cout << std::endl;
}

// FATAL 会终止程序，单独测试
void test_fatal() {
    std::cout << "=== FATAL 测试（会终止程序）===" << std::endl;
    LOG_FATAL("Database unavailable");
    // 不会执行到这里
}

int main() {
    test_basic();
    test_level_filter();
    test_multithread();
    
    // 取消注释测试 FATAL
    // test_fatal();
    
    return 0;
}