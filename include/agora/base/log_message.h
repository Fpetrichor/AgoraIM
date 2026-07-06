#pragma once

#include "agora/base/log_level.h"
#include "agora/base/timestamp.h"
#include <string>
#include <sstream>

namespace agora {

class LogMessage {
public:
    LogMessage(LogLevel level, const char* file, int line, std::string& content)
        : level_(level), file_(file), line_(line), content_(content), timestamp_(Timestamp::now()) {}

    void format(std::ostringstream& oss) const;

    std::string toString() const;

private:
    LogLevel level_;
    const char* file_;
    int line_;
    std::string content_;
    Timestamp timestamp_;
};

} // namespace agora