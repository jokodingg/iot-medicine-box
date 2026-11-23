// Prints the state of each reed switch whenever it changes.
// Lid closed (magnet near) = LOW = TUTUP. Lid open = HIGH = BUKA.
#include <Arduino.h>
#include "pins.h"

static const char* NAMES[4] = {"Pagi", "Siang", "Malam", "Cadangan"};
static const int PINS[4] = {PIN_REED[0], PIN_REED[1], PIN_REED[2], PIN_REED_SPARE};
static int last[4];

static void printAll() {
  for (int i = 0; i < 4; i++)
    Serial.printf("  %-8s GPIO%d: %s\n", NAMES[i], PINS[i],
                  digitalRead(PINS[i]) == LOW ? "TUTUP" : "BUKA");
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  for (int i = 0; i < 4; i++) {
    pinMode(PINS[i], INPUT_PULLUP);
    last[i] = digitalRead(PINS[i]);
  }
  Serial.println("\n[REED] buka/tutup tiap box satu per satu");
  printAll();
}

void loop() {
  for (int i = 0; i < 4; i++) {
    int v = digitalRead(PINS[i]);
    if (v != last[i]) {
      last[i] = v;
      Serial.printf("[%lu ms] %s -> %s\n", millis(), NAMES[i], v == LOW ? "TUTUP" : "BUKA");
    }
  }
  delay(20);
}
