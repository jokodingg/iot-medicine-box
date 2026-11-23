#include "commands.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static const char* skipSpace(const char* p) {
  while (*p == ' ') p++;
  return p;
}

// Reads 1-3 decimal digits followed by space/end. Returns -1 on failure.
static long readInt(const char*& p) {
  p = skipSpace(p);
  if (!isdigit((unsigned char)*p)) return -1;
  char* end;
  long v = strtol(p, &end, 10);
  if (*end != ' ' && *end != 0) return -1;
  p = end;
  return v;
}

static bool readSlot(const char*& p, uint8_t& slot) {
  long v = readInt(p);
  if (v < 1 || v > 3) return false;
  slot = (uint8_t)(v - 1);
  return true;
}

Parsed parseCommand(const char* text) {
  Parsed r = {};
  r.cmd = Cmd::UNKNOWN;
  const char* p = skipSpace(text);
  if (*p != '/') return r;
  p++;
  char word[16] = {};
  size_t n = 0;
  while (*p && *p != ' ' && *p != '@' && n < sizeof word - 1) word[n++] = (char)tolower((unsigned char)*p++);
  while (*p && *p != ' ') p++;  // drop @botname

  struct { const char* w; Cmd c; } simple[] = {
      {"start", Cmd::START}, {"bantuan", Cmd::HELP}, {"help", Cmd::HELP}, {"status", Cmd::STATUS},
      {"jadwal", Cmd::JADWAL}, {"riwayat", Cmd::RIWAYAT}, {"diam", Cmd::DIAM}, {"tes", Cmd::TES},
      {"setup", Cmd::SETUP}, {"resetwifi", Cmd::RESETWIFI}};
  for (auto& s : simple)
    if (!strcmp(word, s.w)) { r.cmd = s.c; return r; }

  if (!strcmp(word, "set")) {
    r.cmd = Cmd::INVALID;
    if (!readSlot(p, r.slot)) return r;
    p = skipSpace(p);
    if (!(isdigit((unsigned char)p[0]) && isdigit((unsigned char)p[1]) && p[2] == ':' &&
          isdigit((unsigned char)p[3]) && isdigit((unsigned char)p[4]) && (p[5] == ' ' || p[5] == 0)))
      return r;
    int h = (p[0] - '0') * 10 + (p[1] - '0'), m = (p[3] - '0') * 10 + (p[4] - '0');
    if (h > 23 || m > 59) return r;
    p = skipSpace(p + 5);
    size_t len = strlen(p);
    while (len && p[len - 1] == ' ') len--;
    if (len == 0 || len > 24) return r;
    memcpy(r.name, p, len);
    r.name[len] = 0;
    r.h = (uint8_t)h; r.m = (uint8_t)m;
    r.cmd = Cmd::SET;
    return r;
  }
  if (!strcmp(word, "hapus")) {
    r.cmd = readSlot(p, r.slot) ? Cmd::HAPUS : Cmd::INVALID;
    return r;
  }
  if (!strcmp(word, "tunda")) {
    if (*skipSpace(p) == 0) { r.cmd = Cmd::TUNDA; r.minutes = 10; return r; }
    long v = readInt(p);
    if (v < 1 || v > 120) { r.cmd = Cmd::INVALID; return r; }
    r.cmd = Cmd::TUNDA;
    r.minutes = (uint16_t)v;
    return r;
  }
  return r;
}

bool isAllowed(const char* id, const char* csv) {
  size_t idLen = strlen(id);
  if (idLen == 0) return false;
  const char* p = csv;
  while (*p) {
    p = skipSpace(p);
    const char* end = p;
    while (*end && *end != ',' && *end != ' ') end++;
    if ((size_t)(end - p) == idLen && !strncmp(p, id, idLen)) return true;
    while (*end && *end != ',') end++;
    p = *end ? end + 1 : end;
  }
  return false;
}
