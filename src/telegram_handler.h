#pragma once
#include <Arduino.h>

// Starts the Telegram network task. Call once in setup(), before any tgBroadcast().
void tgBegin();
// Handles received commands on the caller's task; pauses the network task while
// the setup portal is open.
void tgLoop(uint32_t now, bool portalActive);
// Queue a message for every whitelisted chat. slot >= 0 adds inline buttons for that slot.
void tgBroadcast(const String& text, int8_t slot = -1);
