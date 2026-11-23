// Scans the I2C bus. OLED SSD1306 should appear at 0x3C (sometimes 0x3D).
#include <Arduino.h>
#include <Wire.h>
#include "pins.h"

void setup() {
  Serial.begin(115200);
  delay(1500);
  Wire.begin(PIN_SDA, PIN_SCL);
  Serial.println("\n[I2C SCAN] SDA=GPIO8 SCL=GPIO9");
}

void loop() {
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  device at 0x%02X%s\n", addr,
                    (addr == 0x3C || addr == 0x3D) ? "  <- OLED" : "");
      found++;
    }
  }
  if (found == 0) Serial.println("  no device found: check SDA/SCL, VCC, GND");
  Serial.println("---");
  delay(3000);
}
