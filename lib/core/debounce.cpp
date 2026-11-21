#include "debounce.h"

bool Debouncer::update(bool raw, uint32_t now) {
  rose_ = fell_ = false;
  if (!init_) {
    init_ = true;
    stable_ = last_ = raw;
    since_ = now;
    return false;
  }
  if (raw != last_) {
    last_ = raw;
    since_ = now;
    return false;
  }
  if (raw != stable_ && (uint32_t)(now - since_) >= ms_) {
    stable_ = raw;
    rose_ = raw;
    fell_ = !raw;
    return true;
  }
  return false;
}
