#pragma once

#include "agora/base/noncopyable.h"
#include "agora/base/timestamp.h"
#include "agora/net/timer_id.h"
#include <functional>
#include <atomic>

namespace agora::net {

class Timer : public NonCopyable {
public:
    Timer(TimerCallback cb,
          Timestamp when,
          double interval);

    void run() const;

    const Timestamp& expiration() const { return expiration_; }

    bool repeat() const { return repeat_; }

    int64_t sequence() const { return sequence_; }

    void restart(Timestamp now);

private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static std::atomic<int64_t> s_numCreated;
};

} // namespace agora::net