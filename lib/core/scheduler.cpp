#include "scheduler.h"

SchedEvent schedulerTick(Slot& s, const Clock& c, bool doorOpened, const SchedConfig& cfg) {
  switch (s.state) {
    case SlotState::TAKEN:
    case SlotState::MISSED:
      s.state = SlotState::IDLE;
      return SchedEvent::NONE;

    case SlotState::IDLE:
      if (c.valid && s.enabled && c.h == s.hour && c.m == s.minute && s.lastHandledYmd != c.ymd) {
        s.state = SlotState::ALERT;
        s.lastHandledYmd = c.ymd;  // set at alert start: no double alert, survives reboot once persisted
        s.alertStart = s.lastRemind = c.ms;
        s.reminders = 0;
        s.snoozeUntil = 0;
        return SchedEvent::ALERT;
      }
      return SchedEvent::NONE;

    case SlotState::ALERT: {
      uint32_t elapsed = c.ms - s.alertStart;
      if (doorOpened && elapsed <= cfg.windowMs) {
        s.state = SlotState::TAKEN;
        return SchedEvent::TAKEN;
      }
      if (s.snoozeUntil && (int32_t)(c.ms - s.snoozeUntil) < 0) return SchedEvent::NONE;
      bool due = c.ms - s.lastRemind >= cfg.remindEveryMs;
      if (s.reminders < cfg.maxReminders && due && elapsed < cfg.windowMs) {
        s.reminders++;
        s.lastRemind = c.ms;
        return SchedEvent::REMIND;
      }
      if (elapsed >= cfg.windowMs) {
        s.state = SlotState::MISSED;
        return SchedEvent::MISSED;
      }
      return SchedEvent::NONE;
    }
  }
  return SchedEvent::NONE;
}

void schedulerSnooze(Slot& s, uint32_t now, uint16_t minutes, const SchedConfig& cfg) {
  if (s.state != SlotState::ALERT) return;
  uint32_t ms = (uint32_t)minutes * 60000UL;
  s.snoozeUntil = now + ms;
  s.lastRemind = s.snoozeUntil - cfg.remindEveryMs;  // next reminder exactly when snooze ends
  if (s.reminders > 0) s.reminders--;                // snooze gives back one reminder
  // extend window so snoozed time does not eat the intake window
  uint32_t windowEnd = s.alertStart + cfg.windowMs;
  if ((int32_t)(s.snoozeUntil + cfg.remindEveryMs - windowEnd) > 0)
    s.alertStart = s.snoozeUntil + cfg.remindEveryMs - cfg.windowMs;
}

bool schedulerMarkTaken(Slot& s) {
  if (s.state != SlotState::ALERT) return false;
  s.state = SlotState::TAKEN;
  return true;
}
