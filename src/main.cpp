#include "agora/net/event_loop_thread_pool.h"
#include "agora/net/event_loop.h"
#include "agora/base/logger.h"
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <cassert>
#include <map>
#include <mutex>

using namespace agora::net;

// 测试1：基本功能 - 创建线程池并获取 Loop
void testBasic() {
    LOG_INFO("=== Test 1: Basic ThreadPool ===");
    
    EventLoop mainLoop;
    EventLoopThreadPool pool(&mainLoop, "worker");
    
    pool.setThreadNum(4);
    pool.start();
    
    assert(pool.started());
    assert(pool.allLoops().size() == 4);
    
    LOG_INFO("ThreadPool started with 4 threads");
    
    // 验证每个 Loop 都不同
    std::vector<EventLoop*> loops;
    for (int i = 0; i < 4; ++i) {
        EventLoop* loop = pool.getNextLoop();
        assert(loop != nullptr);
        assert(loop != &mainLoop);
        loops.push_back(loop);
        LOG_INFO("Got loop " + std::to_string(i) + " at " + 
                 std::to_string(reinterpret_cast<uintptr_t>(loop)));
    }
    
    // 验证4个Loop互不相同
    for (int i = 0; i < 4; ++i) {
        for (int j = i + 1; j < 4; ++j) {
            assert(loops[i] != loops[j]);
        }
    }
    
    LOG_INFO("Test 1 passed: 4 unique worker loops");
}

// 测试2：Round Robin 轮询策略
void testRoundRobin() {
    LOG_INFO("=== Test 2: Round Robin ===");
    
    EventLoop mainLoop;
    EventLoopThreadPool pool(&mainLoop, "rr");
    
    pool.setThreadNum(3);
    pool.start();
    
    EventLoop* loop0 = pool.getNextLoop();
    EventLoop* loop1 = pool.getNextLoop();
    EventLoop* loop2 = pool.getNextLoop();
    
    EventLoop* loop3 = pool.getNextLoop();
    EventLoop* loop4 = pool.getNextLoop();
    EventLoop* loop5 = pool.getNextLoop();
    
    assert(loop0 == loop3);
    assert(loop1 == loop4);
    assert(loop2 == loop5);
    
    LOG_INFO("Test 2 passed: Round Robin works correctly");
}

// 测试3：ThreadInitCallback
void testThreadInitCallback() {
    LOG_INFO("=== Test 3: ThreadInitCallback ===");
    
    EventLoop mainLoop;
    EventLoopThreadPool pool(&mainLoop, "init");
    
    int initCount = 0;
    std::mutex countMutex;
    std::vector<std::thread::id> threadIds;
    std::mutex idsMutex;
    
    pool.setThreadNum(3);
    pool.start([&](EventLoop* loop) {
        std::lock_guard<std::mutex> lock(countMutex);
        ++initCount;
        
        std::lock_guard<std::mutex> idLock(idsMutex);
        threadIds.push_back(std::this_thread::get_id());
        
        LOG_INFO("ThreadInitCallback executed in thread " + 
                 std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())));
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    assert(initCount == 3);
    assert(threadIds.size() == 3);
    
    for (size_t i = 0; i < threadIds.size(); ++i) {
        for (size_t j = i + 1; j < threadIds.size(); ++j) {
            assert(threadIds[i] != threadIds[j]);
        }
    }
    
    LOG_INFO("Test 3 passed: ThreadInitCallback called 3 times in different threads");
}

// 测试4：0个线程 - 回退到 baseLoop
void testZeroThreads() {
    LOG_INFO("=== Test 4: Zero Threads (fallback to baseLoop) ===");
    
    EventLoop mainLoop;
    EventLoopThreadPool pool(&mainLoop, "single");
    
    pool.start();
    
    assert(pool.allLoops().empty());
    
    EventLoop* loop = pool.getNextLoop();
    assert(loop == &mainLoop);
    
    EventLoop* loop2 = pool.getNextLoop();
    assert(loop2 == &mainLoop);
    
    LOG_INFO("Test 4 passed: Fallback to baseLoop when 0 threads");
}

// 测试5：模拟 TcpServer 分配连接
void testConnectionDistribution() {
    LOG_INFO("=== Test 5: Simulate TcpServer Connection Distribution ===");
    
    EventLoop mainLoop;
    EventLoopThreadPool pool(&mainLoop, "server");
    
    pool.setThreadNum(4);
    pool.start();
    
    std::vector<EventLoop*> connLoops;
    for (int i = 0; i < 8; ++i) {
        EventLoop* ioLoop = pool.getNextLoop();
        connLoops.push_back(ioLoop);
        LOG_INFO("Connection " + std::to_string(i) + " assigned to loop at " +
                 std::to_string(reinterpret_cast<uintptr_t>(ioLoop)));
    }
    
    std::map<EventLoop*, int> distribution;
    for (auto* loop : connLoops) {
        ++distribution[loop];
    }
    
    assert(distribution.size() == 4);
    for (auto& [loop, count] : distribution) {
        assert(count == 2);
    }
    
    LOG_INFO("Test 5 passed: 8 connections evenly distributed to 4 loops");
}

// 测试6：生命周期 - 线程池析构
void testDestruction() {
    LOG_INFO("=== Test 6: Destruction ===");
    
    EventLoop mainLoop;
    {
        EventLoopThreadPool pool(&mainLoop, "destructor");
        pool.setThreadNum(2);
        pool.start();
        
        EventLoop* loop = pool.getNextLoop();
        (void)loop;
        
        LOG_INFO("Pool going out of scope...");
    }
    
    LOG_INFO("Test 6 passed: Pool destroyed cleanly");
}

int main() {
    LOG_INFO("========================================");
    LOG_INFO("  EventLoopThreadPool Test Suite");
    LOG_INFO("========================================");
    
    testBasic();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    testRoundRobin();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    testThreadInitCallback();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    testZeroThreads();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    testConnectionDistribution();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    testDestruction();
    
    LOG_INFO("========================================");
    LOG_INFO("  All tests passed!");
    LOG_INFO("========================================");
    
    return 0;
}