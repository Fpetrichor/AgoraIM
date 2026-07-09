#include "agora/net/event_loop.h"
#include "agora/net/event_loop_thread.h"
#include "agora/base/logger.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <cassert>

using namespace agora::net;

// 测试1：runInLoop 在同线程直接执行
void testRunInLoopSameThread() {
    LOG_INFO("=== Test 1: runInLoop in same thread ===");
    
    EventLoop loop;
    bool executed = false;
    
    // 在主线程调用 runInLoop，应该直接执行
    loop.runInLoop([&executed]() {
        executed = true;
        LOG_INFO("runInLoop callback executed directly");
    });
    
    assert(executed);  // 直接执行了
    LOG_INFO("Test 1 passed: runInLoop executed directly in same thread");
}

// 测试2：queueInLoop + wakeup 跨线程投递
void testQueueInLoopCrossThread() {
    LOG_INFO("=== Test 2: queueInLoop cross thread ===");
    
    EventLoopThread thread("worker");
    EventLoop* loop = thread.startLoop();
    
    std::atomic<bool> executed(false);
    
    // 在主线程投递任务到工作线程
    std::thread t([loop, &executed]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        loop->queueInLoop([&executed]() {
            executed = true;
            LOG_INFO("queueInLoop callback executed in worker thread");
        });
    });
    
    // 等待任务执行
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    assert(executed.load());
    LOG_INFO("Test 2 passed: queueInLoop executed in worker thread");
    
    t.join();
    loop->quit();
}

// 测试3：runInLoop 跨线程自动转 queueInLoop
void testRunInLoopCrossThread() {
    LOG_INFO("=== Test 3: runInLoop cross thread (auto queue) ===");
    
    EventLoopThread thread("worker2");
    EventLoop* loop = thread.startLoop();
    
    std::atomic<bool> executed(false);
    
    // 在主线程调用 runInLoop，应该自动转 queueInLoop
    std::thread t([loop, &executed]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        loop->runInLoop([&executed]() {
            executed = true;
            LOG_INFO("runInLoop callback executed in worker thread (queued)");
        });
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    assert(executed.load());
    LOG_INFO("Test 3 passed: runInLoop auto-queued in cross thread");
    
    t.join();
    loop->quit();
}

// 测试4：多次 queueInLoop，验证批量执行
void testMultipleQueueInLoop() {
    LOG_INFO("=== Test 4: Multiple queueInLoop ===");
    
    EventLoopThread thread("worker3");
    EventLoop* loop = thread.startLoop();
    
    std::atomic<int> count(0);
    
    // 投递10个任务
    for (int i = 0; i < 10; ++i) {
        loop->queueInLoop([&count, i]() {
            ++count;
            LOG_INFO("Task " + std::to_string(i) + " executed");
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    assert(count.load() == 10);
    LOG_INFO("Test 4 passed: 10 tasks all executed");
    
    loop->quit();
}

// 测试5：quit() 跨线程唤醒
void testQuitCrossThread() {
    LOG_INFO("=== Test 5: quit() cross thread ===");
    
    EventLoopThread thread("worker4");
    EventLoop* loop = thread.startLoop();
    
    // 在另一个线程调用 quit
    std::thread t([loop]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        LOG_INFO("Calling quit() from another thread");
        loop->quit();
    });
    
    // loop 应该被唤醒并退出
    t.join();
    
    // 给一点时间让 loop 线程退出
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    LOG_INFO("Test 5 passed: quit() woke up and stopped loop");
}

// 测试6：在 pending functor 中继续投递任务
void testQueueInCallback() {
    LOG_INFO("=== Test 6: queueInLoop inside callback ===");
    
    EventLoopThread thread("worker5");
    EventLoop* loop = thread.startLoop();
    
    std::atomic<bool> innerExecuted(false);
    
    loop->queueInLoop([loop, &innerExecuted]() {
        LOG_INFO("Outer callback executing");
        
        // 在回调中继续投递任务
        loop->queueInLoop([&innerExecuted]() {
            innerExecuted = true;
            LOG_INFO("Inner callback executed");
        });
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    assert(innerExecuted.load());
    LOG_INFO("Test 6 passed: Inner callback executed");
    
    loop->quit();
}

int main() {
    LOG_INFO("========================================");
    LOG_INFO("  EventLoop runInLoop/queueInLoop Test");
    LOG_INFO("========================================");
    
    testRunInLoopSameThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    testQueueInLoopCrossThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    testRunInLoopCrossThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    testMultipleQueueInLoop();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    testQuitCrossThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    testQueueInCallback();
    
    LOG_INFO("========================================");
    LOG_INFO("  All tests passed!");
    LOG_INFO("========================================");
    
    return 0;
}