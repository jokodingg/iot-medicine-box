#include "settings.h"
#include <Arduino.h>
#include <Preferences.h>

#if __has_include("secrets.h")
#include "secrets.h"
#endif
#ifndef BOT_TOKEN
#define BOT_TOKEN ""
#endif
#ifndef ALLOWED_CHATS
#define ALLOWED_CHATS ""
#endif

static Preferences prefs;
static String token, chats;
static uint32_t revision = 1;

void settingsBegin() {
  prefs.begin("cfg", false);
  token = prefs.getString("token", BOT_TOKEN);
  chats = prefs.getString("chats", ALLOWED_CHATS);
}

const char* settingsToken() { return token.c_str(); }
const char* settingsChats() { return chats.c_str(); }
uint32_t settingsRevision() { return revision; }
bool settingsWifiSeeded() { return prefs.getBool("seeded", false); }
void settingsSetWifiSeeded() { prefs.putBool("seeded", true); }

void settingsSave(const char* t, const char* c) {
  token = t;
  chats = c;
  token.trim();
  chats.trim();
  chats.replace(" ", "");
  prefs.putString("token", token);
  prefs.putString("chats", chats);
  revision++;
}
