#pragma once
#include <stdint.h>

// Exponential backoff safe across millis() wraparound.
class Backoff {
 public:
  Backoff(uint32_t firstMs = 2000, uint32_t maxMs = 300000) : first_(firstMs), max_(maxMs) {}
  bool ready(uint32_t now) const { return !armed_ || now - since_ >= wait_; }
  void fail(uint32_t now);
  void ok() { armed_ = false; wait_ = 0; failures_ = 0; }
  uint8_t failures() const { return failures_; }
 private:
  uint32_t first_, max_, since_ = 0, wait_ = 0;
  bool armed_ = false;
  uint8_t failures_ = 0;
};
