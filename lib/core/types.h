#pragma once
#include <stdint.h>

enum class SlotState : uint8_t { IDLE, ALERT, TAKEN, MISSED };

struct Slot {
  char name[25];
  uint8_t hour, minute;
  bool enabled;
  SlotState state;
  uint8_t reminders;
  uint32_t alertStart;      // millis
  uint32_t lastRemind;      // millis
  uint32_t lastHandledYmd;  // YYYYMMDD
  uint32_t snoozeUntil;     // millis, 0 = none
};
