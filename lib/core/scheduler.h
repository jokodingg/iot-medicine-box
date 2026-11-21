#pragma once
#include <stdint.h>
#include "types.h"

enum class SchedEvent : uint8_t { NONE, ALERT, REMIND, TAKEN, MISSED };

struct Clock {
  uint32_t ymd;   // YYYYMMDD
  uint8_t h, m;
  bool valid;     // time synced
  uint32_t ms;    // millis()
};

struct SchedConfig {
  uint32_t windowMs = 30UL * 60000UL;
  uint32_t remindEveryMs = 5UL * 60000UL;
  uint8_t maxReminders = 3;
};

// doorOpened = debounced closed->open edge seen this tick.
SchedEvent schedulerTick(Slot& s, const Clock& c, bool doorOpened, const SchedConfig& cfg = SchedConfig());
void schedulerSnooze(Slot& s, uint32_t now, uint16_t minutes, const SchedConfig& cfg = SchedConfig());
bool schedulerMarkTaken(Slot& s);  // true if slot was alerting
