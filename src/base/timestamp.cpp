#include "agora/base/timestamp.h"

#include <chrono>

namespace agora {

Timestamp::Timestamp()
    : microSecondsSinceEpoch_(0) {}

Timestamp::Timestamp(int64_t microSeconds)
    : microSecondsSinceEpoch_(microSeconds) {}

Timestamp Timestamp::now() {
    auto now = std::chrono::system_clock::now();

    auto microSeconds = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch())
        .count();

    return Timestamp(microSeconds);
}

std::string Timestamp::toString() const {
    return std::to_string(microSecondsSinceEpoch_);
}

int64_t Timestamp::microSecondsSinceEpoch() const {
    return microSecondsSinceEpoch_;
}

} // namespace agora