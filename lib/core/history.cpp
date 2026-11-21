#include "history.h"

void History::add(const Record& r) {
  buf[head] = r;
  head = (head + 1) % CAP;
  if (count < CAP) count++;
}

const Record& History::at(size_t i) const {
  return buf[(head + CAP - 1 - i) % CAP];
}

void History::prune(uint32_t now) {
  // records are chronological; oldest are at the tail end of newest-first order
  while (count > 0 && now - at(count - 1).epoch > KEEP_S) count--;
}
