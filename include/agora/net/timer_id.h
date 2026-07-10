#pragma once

#include <cstdint>
#include <functional>

namespace agora::net {

class Timer;

using TimerCallback = std::function<void()>;

class TimerId {
public:
    TimerId();
    TimerId(Timer* timer, int64_t sequence);

    Timer* timer() const { return timer_; }
    int64_t sequence() const { return sequence_; }

private:
    Timer* timer_;
    int64_t sequence_;

    friend class TimerQueue;
};

} // namespace agora::net