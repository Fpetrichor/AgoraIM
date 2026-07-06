#include "agora/base/current_thread.h"

namespace agora::CurrentThread {
    thread_local std::thread::id t_cached_tid;
    thread_local bool t_cached_tid_valid = false;


    std::thread::id tid() {
        if (!t_cached_tid_valid) {
            t_cached_tid = std::this_thread::get_id();
            t_cached_tid_valid = true;
        }
        return t_cached_tid;
    }

} // namespace agora::CurrentThread