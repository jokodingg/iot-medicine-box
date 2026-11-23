// Isolates OLED vs WiFi power: draws a counter every 500 ms with WiFi OFF for
// 15 s, then turns WiFi ON (saved credentials from the setup portal).
// If the screen goes dark only after "WIFI ON", the 3V3 supply dips under WiFi load.
#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "pins.h"

static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
static bool wifiStarted = false;

void setup() {
  Serial.begin(115200);
  delay(1500);
  Wire.begin(PIN_SDA, PIN_SCL);
  u8g2.begin();
  u8g2.setContrast(255);
  Serial.println("\n[OLED+WIFI] WiFi starts at 15 s");
}

void loop() {
  uint32_t s = millis() / 1000;
  if (!wifiStarted && s >= 15) {
    wifiStarted = true;
    Serial.println("[OLED+WIFI] WIFI ON");
    WiFi.mode(WIFI_STA);
    WiFi.begin();  // credentials saved by the main firmware
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
  }
  Wire.beginTransmission(0x3C);
  bool ack = Wire.endTransmission() == 0;

  char l1[24], l2[24];
  snprintf(l1, sizeof l1, "t=%lus ack=%d", (unsigned long)s, ack);
  snprintf(l2, sizeof l2, "WiFi: %s", !wifiStarted ? "OFF" : WiFi.status() == WL_CONNECTED ? "CONNECTED" : "connecting");
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawFrame(0, 0, 128, 64);
  u8g2.drawStr(4, 20, l1);
  u8g2.drawStr(4, 40, l2);
  u8g2.sendBuffer();
  Serial.printf("%s | %s\n", l1, l2);
  delay(500);
}
