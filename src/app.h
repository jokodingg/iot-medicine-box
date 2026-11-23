#pragma once
// Application state shared between main.cpp and telegram_handler.cpp.
#include <stdint.h>
#include "types.h"
#include "history.h"

Slot* appSlots();                 // array of 3
const History& appHistory();
bool appDoorOpen(uint8_t slot);
void appSetSlot(uint8_t slot, const char* name, uint8_t h, uint8_t m);
void appClearSlot(uint8_t slot);
bool appMarkTaken(uint8_t slot);  // false if slot was not alerting
uint8_t appSnooze(uint16_t minutes);  // returns number of slots snoozed
bool appSnoozeSlot(uint8_t slot, uint16_t minutes);
void appMute();
void appTest();
void appStartSetup();
void appResetWifi();  // open the WiFi/bot setup portal
