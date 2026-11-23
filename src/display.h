#pragma once
#include <stdint.h>
#include "scheduler.h"

void displayBegin();
void displayIdle(const Clock& c, bool wifi, const char* nextText);
void displayAlert(const char* label, const char* name);
void displayMsg(const char* l1, const char* l2);
void displayWake();
void displayReinit();  // re-send the init sequence to recover a reset OLED
void displayLoop(uint32_t now);
bool displayIsOff();
