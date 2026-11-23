#pragma once
#include <stdint.h>
#include "scheduler.h"

void netBegin();
void netLoop(uint32_t now);
bool netOnline();
Clock netClock(uint32_t now);
uint32_t netEpoch();  // 0 if time not synced
void netStartPortal();
void netResetWifi();  // erase saved WiFi and restart into the portal
bool netPortalActive();
