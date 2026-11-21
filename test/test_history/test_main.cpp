#include <unity.h>
#include "history.h"

void setUp() {}
void tearDown() {}

void test_newest_first() {
  History h;
  h.add({100, 0, 1});
  h.add({200, 1, 0});
  TEST_ASSERT_EQUAL(2, h.size());
  TEST_ASSERT_EQUAL_UINT32(200, h.at(0).epoch);
  TEST_ASSERT_EQUAL_UINT32(100, h.at(1).epoch);
}

void test_wraparound() {
  History h;
  for (uint32_t i = 1; i <= 70; i++) h.add({i, 0, 1});
  TEST_ASSERT_EQUAL(64, h.size());
  TEST_ASSERT_EQUAL_UINT32(70, h.at(0).epoch);
  TEST_ASSERT_EQUAL_UINT32(7, h.at(63).epoch);
}

void test_prune_old() {
  History h;
  uint32_t now = 1800000000;
  h.add({now - 8 * 86400, 0, 1});
  h.add({now - 6 * 86400, 1, 1});
  h.add({now - 60, 2, 0});
  h.prune(now);
  TEST_ASSERT_EQUAL(2, h.size());
  TEST_ASSERT_EQUAL_UINT32(now - 6 * 86400, h.at(1).epoch);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_newest_first);
  RUN_TEST(test_wraparound);
  RUN_TEST(test_prune_old);
  return UNITY_END();
}
