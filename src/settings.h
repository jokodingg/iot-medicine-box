#pragma once
#include <stdint.h>

// Bot token and chat whitelist, stored in flash. Set from the setup portal.
// First boot falls back to include/secrets.h when that file exists.
void settingsBegin();
const char* settingsToken();
const char* settingsChats();  // comma-separated chat IDs
void settingsSave(const char* token, const char* chats);
uint32_t settingsRevision();  // increments on every save
// secrets.h WiFi is used only once; after that the flash copy (or the portal) wins.
bool settingsWifiSeeded();
void settingsSetWifiSeeded();
