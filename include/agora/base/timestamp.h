#pragma once

#include <cstdint>
#include <string>

namespace agora {

class Timestamp {
public:
    Timestamp();

    explicit Timestamp(int64_t microSeconds);

    static Timestamp now();

    std::string toString() const;

    std::string toFormattedString(bool showMicroseconds = false) const;

    int64_t microSecondsSinceEpoch() const;

private:
    int64_t microSecondsSinceEpoch_;
};

} // namespace agora