#pragma once

#include "agora/base/log_level.h"
#include <string>

namespace agora {

class Logger {
public:
    static Logger& instance();

    void log(LogLevel level, const char* file, int line, const std::string& msg);

    void setLogLevel(LogLevel level);

    LogLevel level() const;

private:
    Logger() : level_(LogLevel::INFO) {}
    LogLevel level_;
};

} // namespace agora


#define LOG_INFO(msg)  \
    agora::Logger::instance().log(agora::LogLevel::INFO, __FILE__, __LINE__, (msg))

#define LOG_WARN(msg)  \
    agora::Logger::instance().log(agora::LogLevel::WARN, __FILE__, __LINE__, (msg))

#define LOG_ERROR(msg) \
    agora::Logger::instance().log(agora::LogLevel::ERROR, __FILE__, __LINE__, (msg))

#define LOG_FATAL(msg) \
    agora::Logger::instance().log(agora::LogLevel::FATAL, __FILE__, __LINE__, (msg))
