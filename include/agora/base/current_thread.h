#pragma once

#include <thread>

namespace agora::CurrentThread {
    std::thread::id tid();
} // namespace agora::CurrentThread