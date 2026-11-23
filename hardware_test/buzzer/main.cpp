// Tests the buzzer on GPIO7 (buzzer + to GPIO7, - to GND).
// Step 1: plain HIGH/LOW (active buzzer beeps, passive buzzer only clicks).
// Step 2: tone() sweep (passive buzzer plays pitches).
#include <Arduino.h>
#include "pins.h"

void setup() {
  Serial.begin(115200);
  delay(1500);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  Serial.println("\n[BUZZER] GPIO7");
}

void loop() {
  Serial.println("Step 1: digital on/off x3");
  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_BUZZER, HIGH); delay(200);
    digitalWrite(PIN_BUZZER, LOW);  delay(300);
  }
  delay(800);

  Serial.println("Step 2: tone sweep 1000-4000 Hz");
  for (int f = 1000; f <= 4000; f += 500) {
    Serial.printf("  %d Hz\n", f);
    tone(PIN_BUZZER, f); delay(300);
  }
  noTone(PIN_BUZZER);
  digitalWrite(PIN_BUZZER, LOW);
  Serial.println("---");
  delay(3000);
}
