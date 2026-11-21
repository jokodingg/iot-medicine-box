#include "buzzer.h"
#include <Arduino.h>
#include "config.h"
#include "pins.h"

static bool running = false, on = false;
static uint8_t pat = PAT_ALERT, step = 0;
static uint32_t stepAt = 0, startAt = 0, muteUntil = 0;
static const uint32_t ALERT_MAX_MS = 60000;

static void out(bool v) {
  on = v;
#if BUZZER_ACTIVE
  digitalWrite(PIN_BUZZER, v ? HIGH : LOW);
#else
  if (v) tone(PIN_BUZZER, BUZZER_FREQ); else noTone(PIN_BUZZER);
#endif
}

void buzzerBegin() {
  pinMode(PIN_BUZZER, OUTPUT);
  out(false);
}

void buzzerStart(uint8_t p) {
  uint32_t now = millis();
  if (p == PAT_ALERT && buzzerMuted(now)) return;
  pat = p; step = 0; running = true;
  startAt = stepAt = now;
  out(true);
}

void buzzerStop() { running = false; out(false); }

void buzzerMute(uint32_t untilMs) { muteUntil = untilMs; if (pat == PAT_ALERT) buzzerStop(); }

bool buzzerMuted(uint32_t now) { return muteUntil && (int32_t)(now - muteUntil) < 0; }

void buzzerLoop(uint32_t now) {
  if (!running) return;
  uint32_t dt = now - stepAt;
  if (pat == PAT_TEST) {
    if (dt >= 500) buzzerStop();
    return;
  }
  if (now - startAt >= ALERT_MAX_MS) { buzzerStop(); return; }
  // steps 0..5: on/off x3 (200/150), step 6: pause 1000
  static const uint16_t dur[7] = {200, 150, 200, 150, 200, 150, 1000};
  if (dt < dur[step]) return;
  step = (step + 1) % 7;
  stepAt = now;
  out(step < 6 && step % 2 == 0);
}
