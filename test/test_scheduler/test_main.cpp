#include <unity.h>
#include <string.h>
#include "scheduler.h"

static const uint32_t MIN = 60000;
static Slot s;

static Clock at(uint32_t ms, uint8_t h = 7, uint8_t m = 0, uint32_t ymd = 20261122, bool valid = true) {
  return Clock{ymd, h, m, valid, ms};
}

void setUp() {
  memset(&s, 0, sizeof s);
  strcpy(s.name, "Amlodipin");
  s.hour = 7; s.minute = 0; s.enabled = true;
}
void tearDown() {}

void test_alert_fires_at_time() {
  TEST_ASSERT_EQUAL(SchedEvent::NONE, schedulerTick(s, at(0, 6, 59), false));
  TEST_ASSERT_EQUAL(SchedEvent::ALERT, schedulerTick(s, at(1000), false));
  TEST_ASSERT_EQUAL(SlotState::ALERT, s.state);
}

void test_open_within_window_is_taken() {
  schedulerTick(s, at(0), false);
  TEST_ASSERT_EQUAL(SchedEvent::TAKEN, schedulerTick(s, at(10 * MIN, 7, 10), true));
  TEST_ASSERT_EQUAL(SchedEvent::NONE, schedulerTick(s, at(10 * MIN + 1, 7, 10), false));
  TEST_ASSERT_EQUAL(SlotState::IDLE, s.state);
}

void test_reminders_then_missed_at_window_end() {
  schedulerTick(s, at(0), false);
  TEST_ASSERT_EQUAL(SchedEvent::REMIND, schedulerTick(s, at(5 * MIN), false));
  TEST_ASSERT_EQUAL(SchedEvent::REMIND, schedulerTick(s, at(10 * MIN), false));
  TEST_ASSERT_EQUAL(SchedEvent::REMIND, schedulerTick(s, at(15 * MIN), false));
  TEST_ASSERT_EQUAL(SchedEvent::NONE, schedulerTick(s, at(20 * MIN), false));
  TEST_ASSERT_EQUAL(SchedEvent::TAKEN, schedulerTick(s, at(25 * MIN), true));
}

void test_missed_after_window() {
  schedulerTick(s, at(0), false);
  for (uint32_t t = 1; t < 30; t++) schedulerTick(s, at(t * MIN), false);
  TEST_ASSERT_EQUAL(SchedEvent::MISSED, schedulerTick(s, at(30 * MIN), false));
  TEST_ASSERT_EQUAL(3, s.reminders);
}

void test_open_before_alert_ignored() {
  schedulerTick(s, at(0, 6, 59), true);
  TEST_ASSERT_EQUAL(SchedEvent::ALERT, schedulerTick(s, at(1000), false));
  TEST_ASSERT_EQUAL(SlotState::ALERT, s.state);
}

void test_no_realert_same_day() {
  schedulerTick(s, at(0), false);
  schedulerTick(s, at(1000), true);       // taken
  schedulerTick(s, at(2000), false);      // back to idle, still 07:00
  TEST_ASSERT_EQUAL(SchedEvent::NONE, schedulerTick(s, at(3000), false));
  TEST_ASSERT_EQUAL(SchedEvent::ALERT, schedulerTick(s, at(4000, 7, 0, 20261123), false));
}

void test_invalid_clock_never_fires() {
  TEST_ASSERT_EQUAL(SchedEvent::NONE, schedulerTick(s, at(0, 7, 0, 19700101, false), false));
}

void test_disabled_never_fires() {
  s.enabled = false;
  TEST_ASSERT_EQUAL(SchedEvent::NONE, schedulerTick(s, at(0), false));
}

void test_snooze_delays_reminder() {
  schedulerTick(s, at(0), false);
  schedulerSnooze(s, 1 * MIN, 10);
  TEST_ASSERT_EQUAL(SchedEvent::NONE, schedulerTick(s, at(5 * MIN), false));
  TEST_ASSERT_EQUAL(SchedEvent::REMIND, schedulerTick(s, at(11 * MIN), false));
}

void test_mark_taken() {
  TEST_ASSERT_FALSE(schedulerMarkTaken(s));
  schedulerTick(s, at(0), false);
  TEST_ASSERT_TRUE(schedulerMarkTaken(s));
  TEST_ASSERT_EQUAL(SlotState::TAKEN, s.state);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_alert_fires_at_time);
  RUN_TEST(test_open_within_window_is_taken);
  RUN_TEST(test_reminders_then_missed_at_window_end);
  RUN_TEST(test_missed_after_window);
  RUN_TEST(test_open_before_alert_ignored);
  RUN_TEST(test_no_realert_same_day);
  RUN_TEST(test_invalid_clock_never_fires);
  RUN_TEST(test_disabled_never_fires);
  RUN_TEST(test_snooze_delays_reminder);
  RUN_TEST(test_mark_taken);
  return UNITY_END();
}
