// Draws text, a border and a moving counter on the SSD1306.
// Pass: text is sharp, border touches all four edges, counter increments.
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "pins.h"

static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void setup() {
  Serial.begin(115200);
  delay(1500);
  Wire.begin(PIN_SDA, PIN_SCL);
  if (!u8g2.begin()) Serial.println("[OLED] begin failed");
  else Serial.println("[OLED] begin ok");
}

void loop() {
  static uint32_t n = 0;
  u8g2.clearBuffer();
  u8g2.drawFrame(0, 0, 128, 64);
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(6, 14, "OLED TEST");
  u8g2.drawStr(6, 30, "Medicine Box");
  char buf[24];
  snprintf(buf, sizeof(buf), "count: %lu", (unsigned long)n++);
  u8g2.drawStr(6, 46, buf);
  u8g2.drawBox(6 + (n % 20) * 5, 52, 8, 6);
  u8g2.sendBuffer();
  delay(500);
}
