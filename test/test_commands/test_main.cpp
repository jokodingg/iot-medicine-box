#include <unity.h>
#include "commands.h"

void setUp() {}
void tearDown() {}

void test_simple_commands() {
  TEST_ASSERT_EQUAL(Cmd::START, parseCommand("/start").cmd);
  TEST_ASSERT_EQUAL(Cmd::HELP, parseCommand("/bantuan").cmd);
  TEST_ASSERT_EQUAL(Cmd::STATUS, parseCommand("/status@MedBoxBot").cmd);
  TEST_ASSERT_EQUAL(Cmd::JADWAL, parseCommand("/jadwal").cmd);
  TEST_ASSERT_EQUAL(Cmd::RIWAYAT, parseCommand("/riwayat").cmd);
  TEST_ASSERT_EQUAL(Cmd::DIAM, parseCommand("/diam").cmd);
  TEST_ASSERT_EQUAL(Cmd::TES, parseCommand("/tes").cmd);
  TEST_ASSERT_EQUAL(Cmd::UNKNOWN, parseCommand("halo").cmd);
  TEST_ASSERT_EQUAL(Cmd::UNKNOWN, parseCommand("/foo").cmd);
}

void test_set_valid() {
  Parsed p = parseCommand("/set 1 07:30 Amlodipin 5mg");
  TEST_ASSERT_EQUAL(Cmd::SET, p.cmd);
  TEST_ASSERT_EQUAL(0, p.slot);
  TEST_ASSERT_EQUAL(7, p.h);
  TEST_ASSERT_EQUAL(30, p.m);
  TEST_ASSERT_EQUAL_STRING("Amlodipin 5mg", p.name);
}

void test_set_invalid() {
  TEST_ASSERT_EQUAL(Cmd::INVALID, parseCommand("/set 1 25:00 Obat").cmd);
  TEST_ASSERT_EQUAL(Cmd::INVALID, parseCommand("/set 1 07:60 Obat").cmd);
  TEST_ASSERT_EQUAL(Cmd::INVALID, parseCommand("/set 4 07:00 Obat").cmd);
  TEST_ASSERT_EQUAL(Cmd::INVALID, parseCommand("/set 1 07:00").cmd);
  TEST_ASSERT_EQUAL(Cmd::INVALID, parseCommand("/set 1 07:00   ").cmd);
  TEST_ASSERT_EQUAL(Cmd::INVALID, parseCommand("/set 1 7 Obat").cmd);
  TEST_ASSERT_EQUAL(Cmd::INVALID, parseCommand("/set 1 07:00 Nama obat yang sangat panjang sekali").cmd);
}

void test_hapus_and_tunda() {
  Parsed p = parseCommand("/hapus 3");
  TEST_ASSERT_EQUAL(Cmd::HAPUS, p.cmd);
  TEST_ASSERT_EQUAL(2, p.slot);
  TEST_ASSERT_EQUAL(Cmd::INVALID, parseCommand("/hapus 0").cmd);
  TEST_ASSERT_EQUAL(Cmd::INVALID, parseCommand("/hapus").cmd);
  TEST_ASSERT_EQUAL(10, parseCommand("/tunda").minutes);
  TEST_ASSERT_EQUAL(15, parseCommand("/tunda 15").minutes);
  TEST_ASSERT_EQUAL(Cmd::INVALID, parseCommand("/tunda 0").cmd);
  TEST_ASSERT_EQUAL(Cmd::INVALID, parseCommand("/tunda 500").cmd);
  TEST_ASSERT_EQUAL(Cmd::INVALID, parseCommand("/tunda abc").cmd);
}

void test_whitelist() {
  TEST_ASSERT_TRUE(isAllowed("222", "111,222"));
  TEST_ASSERT_TRUE(isAllowed("111", "111, 222"));
  TEST_ASSERT_FALSE(isAllowed("333", "111,222"));
  TEST_ASSERT_FALSE(isAllowed("22", "111,222"));
  TEST_ASSERT_FALSE(isAllowed("", "111,222"));
  TEST_ASSERT_TRUE(isAllowed("-1001", "-1001"));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_simple_commands);
  RUN_TEST(test_set_valid);
  RUN_TEST(test_set_invalid);
  RUN_TEST(test_hapus_and_tunda);
  RUN_TEST(test_whitelist);
  return UNITY_END();
}
