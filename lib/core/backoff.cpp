#include "backoff.h"

void Backoff::fail(uint32_t now) {
  wait_ = wait_ ? (wait_ * 2 > max_ ? max_ : wait_ * 2) : first_;
  since_ = now;
  armed_ = true;
  if (failures_ < 255) failures_++;
}
