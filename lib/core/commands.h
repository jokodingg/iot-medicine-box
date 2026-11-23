#pragma once
#include <stdint.h>

enum class Cmd : uint8_t { START, HELP, STATUS, JADWAL, SET, HAPUS, RIWAYAT, TUNDA, DIAM, TES, SETUP, RESETWIFI, UNKNOWN, INVALID };

struct Parsed {
  Cmd cmd;
  uint8_t slot;      // 0-based (user types 1-3)
  uint8_t h, m;
  uint16_t minutes;
  char name[25];
};

Parsed parseCommand(const char* text);
bool isAllowed(const char* chatId, const char* csvWhitelist);
