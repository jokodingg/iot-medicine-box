#include <unity.h>
#include "backoff.h"

void setUp() {}
void tearDown() {}

void test_fresh_ready_after_long_uptime() {
  Backoff b;
  TEST_ASSERT_TRUE(b.ready(0x90000000UL));  // >24.8 days uptime
}

void test_fail_doubles_and_ok_resets() {
  Backoff b(2000, 300000);
  b.fail(1000);
  TEST_ASSERT_FALSE(b.ready(2999));
  TEST_ASSERT_TRUE(b.ready(3000));
  b.fail(3000);
  TEST_ASSERT_FALSE(b.ready(6999));
  TEST_ASSERT_TRUE(b.ready(7000));
  TEST_ASSERT_EQUAL(2, b.failures());
  b.ok();
  TEST_ASSERT_TRUE(b.ready(7001));
  TEST_ASSERT_EQUAL(0, b.failures());
}

void test_wraparound_wait() {
  Backoff b(2000, 300000);
  b.fail(0xFFFFFC00UL);
  TEST_ASSERT_FALSE(b.ready(0x00000100UL));
  TEST_ASSERT_TRUE(b.ready(0x00000400UL));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_fresh_ready_after_long_uptime);
  RUN_TEST(test_fail_doubles_and_ok_resets);
  RUN_TEST(test_wraparound_wait);
  return UNITY_END();
}
