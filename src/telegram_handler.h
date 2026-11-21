#pragma once
#include <Arduino.h>

void tgBegin();
void tgLoop(uint32_t now, bool online);
// Queue a message for every whitelisted chat. slot >= 0 adds inline buttons for that slot.
void tgBroadcast(const String& text, int8_t slot = -1);
