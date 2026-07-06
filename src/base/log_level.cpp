#include "agora/base/log_level.h"

namespace agora {
    
const std::string& logLevelToString(LogLevel level) {
    static const std::string levelStrings[] = {
        "INFO",
        "WARN",
        "ERROR",
        "FATAL"
    };

    return levelStrings[static_cast<int>(level)];
}

} // namespace agora