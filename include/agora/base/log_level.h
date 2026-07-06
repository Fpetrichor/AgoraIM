#pragma once

#include <string>

namespace agora {

enum class LogLevel {
    INFO,
    WARN,
    ERROR,
    FATAL
};

const std::string& logLevelToString(LogLevel level);
} // namespace agora