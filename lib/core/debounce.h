#pragma once
#include <stdint.h>

class Debouncer {
 public:
  explicit Debouncer(uint16_t ms = 50) : ms_(ms) {}
  bool update(bool raw, uint32_t now);  // true when stable state changed
  bool stable() const { return stable_; }
  bool rose() const { return rose_; }
  bool fell() const { return fell_; }
 private:
  uint16_t ms_;
  bool stable_ = false, last_ = false, init_ = false, rose_ = false, fell_ = false;
  uint32_t since_ = 0;
};
