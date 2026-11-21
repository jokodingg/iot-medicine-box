#include <unity.h>
#include "debounce.h"

void setUp() {}
void tearDown() {}

void test_bounce_ignored() {
  Debouncer d(50);
  d.update(false, 0);
  d.update(true, 10);
  d.update(false, 20);
  d.update(true, 30);
  TEST_ASSERT_FALSE(d.update(true, 60));  // only 30ms stable
  TEST_ASSERT_FALSE(d.stable());
}

void test_stable_changes_once_and_rose_one_shot() {
  Debouncer d(50);
  d.update(false, 0);
  d.update(true, 100);
  TEST_ASSERT_TRUE(d.update(true, 150));
  TEST_ASSERT_TRUE(d.stable());
  TEST_ASSERT_TRUE(d.rose());
  TEST_ASSERT_FALSE(d.update(true, 160));
  TEST_ASSERT_FALSE(d.rose());
  d.update(false, 200);
  d.update(false, 250);
  TEST_ASSERT_TRUE(d.fell());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_bounce_ignored);
  RUN_TEST(test_stable_changes_once_and_rose_one_shot);
  return UNITY_END();
}
