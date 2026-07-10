#include "agora/base/timestamp.h"

#include <chrono>
#include <iomanip>
#include <sstream>

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

Timestamp Timestamp::invalid() {
    return Timestamp();
}

bool Timestamp::isValid() const {
    return microSecondsSinceEpoch_ > 0;
}

Timestamp Timestamp::addTime(Timestamp timestamp, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds * 1000 * 1000);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

bool Timestamp::operator<(const Timestamp& rhs) const {
    return microSecondsSinceEpoch_ < rhs.microSecondsSinceEpoch_;
}

bool Timestamp::operator==(const Timestamp& rhs) const {
    return microSecondsSinceEpoch_ == rhs.microSecondsSinceEpoch_;
}

std::string Timestamp::toString() const {
    return std::to_string(microSecondsSinceEpoch_);
}

std::string Timestamp::toFormattedString(bool showMicroseconds) const {
    std::time_t seconds = microSecondsSinceEpoch_ / 1000000;
    auto microseconds = microSecondsSinceEpoch_ % 1000000;

    std::time_t time = static_cast<std::time_t>(seconds);

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");

    if (showMicroseconds) {
        oss << "." << std::setw(6) << std::setfill('0') << microseconds;
    }
    return oss.str();
}

int64_t Timestamp::microSecondsSinceEpoch() const {
    return microSecondsSinceEpoch_;
}

} // namespace agora