#include "agora/base/log_message.h"
#include "agora/base/current_thread.h"
#include "agora/base/timestamp.h"

namespace agora {
    
void LogMessage::format(std::ostringstream& oss) const {
    oss << timestamp_.toFormattedString() << " "
        << "[" << logLevelToString(level_) << "] "
        << "[" << CurrentThread::tid() << "] "
        << file_ << ":" << line_ << " - " << content_;
}

std::string LogMessage::toString() const {
    std::ostringstream oss;
    format(oss);
    return oss.str();
}

} // namespace agora