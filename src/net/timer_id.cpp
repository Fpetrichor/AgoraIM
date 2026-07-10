#include "agora/net/timer_id.h"

namespace agora::net {

TimerId::TimerId()
    : timer_(nullptr),
      sequence_(0) {
}

TimerId::TimerId(Timer* timer,
                 int64_t sequence)
    : timer_(timer),
      sequence_(sequence) {
}

} // namespace agora::net