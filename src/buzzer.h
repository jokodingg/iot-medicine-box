#pragma once
#include <stdint.h>

enum BuzzPattern : uint8_t { PAT_ALERT, PAT_TEST };

void buzzerBegin();
void buzzerStart(uint8_t pattern);
void buzzerStop();
void buzzerMute(uint32_t untilMs);  // absolute millis
bool buzzerMuted(uint32_t now);
void buzzerLoop(uint32_t now);
