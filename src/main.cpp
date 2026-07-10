#include "agora/net/event_loop.h"
#include "agora/base/logger.h"

using namespace agora;
using namespace agora::net;

int main() {
    LOG_INFO("===== Timer Test Start =====");

    EventLoop loop;

    // 2 秒后执行一次
    loop.runAfter(2.0, []() {
        LOG_INFO("runAfter(): after 2 seconds");
    });

    // 每秒执行一次
    TimerId timer = loop.runEvery(1.0, []() {
        LOG_INFO("runEvery(): tick");
    });

    // 5 秒后取消循环定时器
    loop.runAfter(5.0, [&]() {
        LOG_INFO("Cancel repeating timer");
        loop.cancel(timer);
    });

    // 7 秒退出 EventLoop
    loop.runAfter(7.0, [&]() {
        LOG_INFO("Quit EventLoop");
        loop.quit();
    });

    loop.loop();

    LOG_INFO("===== Timer Test End =====");

    return 0;
}