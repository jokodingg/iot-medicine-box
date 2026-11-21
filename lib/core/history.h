#pragma once
#include <stddef.h>
#include <stdint.h>

struct Record {
  uint32_t epoch;
  uint8_t slot;
  uint8_t taken;  // 1 = diminum, 0 = terlewat
};

class History {
 public:
  static const uint8_t CAP = 64;
  static const uint32_t KEEP_S = 7UL * 86400UL;
  void add(const Record& r);
  void prune(uint32_t nowEpoch);
  size_t size() const { return count; }
  const Record& at(size_t newestFirst) const;
  Record buf[CAP] = {};
  uint8_t head = 0;   // next write index
  uint8_t count = 0;
};
