#include "agora/base/logger.h"
#include "agora/base/log_message.h"
#include <iostream>

namespace agora {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::log(LogLevel level, const char* file, int line, const std::string& msg) {
    if (level >= level_) {
        // Here you can implement the actual logging logic, e.g., writing to a file or console.
        // For simplicity, we'll just print to the console.
        std::ostringstream oss;
        LogMessage logMessage(level, file, line, const_cast<std::string&>(msg));
        logMessage.format(oss);
        std::cout << oss.str() << std::endl;

        if (level == LogLevel::FATAL) {
            std::abort(); // Terminate the program on fatal log
        }
    }
}

void Logger::setLogLevel(LogLevel level) {
    level_ = level;
}

LogLevel Logger::level() const {
    return level_;
}

} // namespace agora