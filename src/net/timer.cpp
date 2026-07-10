#include "agora/net/timer.h"

namespace agora::net {

std::atomic<int64_t> Timer::s_numCreated{0};

Timer::Timer(TimerCallback cb,
             Timestamp when,
             double interval)
    : callback_(std::move(cb)),
      expiration_(when),
      interval_(interval),
      repeat_(interval > 0.0),
      sequence_(++s_numCreated) {}

void Timer::run() const {
    callback_();
}

void Timer::restart(Timestamp now) {
    if (repeat_) {
        expiration_ = Timestamp::addTime(now, interval_);
    } else {
        expiration_ = Timestamp::invalid();
    }
}

} // namespace agora::net