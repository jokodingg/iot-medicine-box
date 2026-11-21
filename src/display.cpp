#include "display.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <time.h>
#include "config.h"
#include "pins.h"

static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
static uint32_t lastWake = 0;
static enum { BRIGHT, DIM, OFF } level = BRIGHT;
static const char* HARI[7] = {"Min", "Sen", "Sel", "Rab", "Kam", "Jum", "Sab"};

static bool probe(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

void displayBegin() {
  Wire.begin(PIN_SDA, PIN_SCL);
  uint8_t addr = probe(OLED_ADDR) ? OLED_ADDR : OLED_ADDR_ALT;
  u8g2.setI2CAddress(addr * 2);
  u8g2.begin();
  u8g2.setContrast(255);
  displayMsg("Medicine Box", "Memulai...");
  lastWake = millis();
}

void displayWake() {
  lastWake = millis();
  if (level != BRIGHT) {
    u8g2.setPowerSave(0);
    u8g2.setContrast(255);
    level = BRIGHT;
  }
}

bool displayIsOff() { return level == OFF; }

void displayLoop(uint32_t now) {
  uint32_t idle = now - lastWake;
  if (level == BRIGHT && idle >= DIM_AFTER_MS) { u8g2.setContrast(10); level = DIM; }
  if (level == DIM && idle >= OFF_AFTER_MS) { u8g2.setPowerSave(1); level = OFF; }
}

void displayIdle(const Clock& c, bool wifi, const char* nextText) {
  if (level == OFF) return;
  char buf[24];
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 9, wifi ? "WiFi: OK" : "WiFi: --");
  if (c.valid) {
    time_t t = time(nullptr);
    struct tm tm;
    localtime_r(&t, &tm);
    snprintf(buf, sizeof buf, "%s %02d/%02d/%04d", HARI[tm.tm_wday], tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    u8g2.drawStr(128 - u8g2.getStrWidth(buf), 9, buf);
    snprintf(buf, sizeof buf, "%02d:%02d", c.h, c.m);
  } else {
    strcpy(buf, "--:--");
  }
  u8g2.setFont(u8g2_font_logisoso24_tn);
  u8g2.drawStr((128 - u8g2.getStrWidth(buf)) / 2, 42, buf);
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 62, nextText);
  u8g2.sendBuffer();
}

void displayAlert(const char* label, const char* name) {
  displayWake();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 10, "Waktunya minum obat:");
  u8g2.drawStr(0, 24, label);
  u8g2.setFont(u8g2_font_helvB10_tf);
  // name max 24 chars: split into 2 lines of 12
  char l1[13] = {}, l2[13] = {};
  strncpy(l1, name, 12);
  if (strlen(name) > 12) strncpy(l2, name + 12, 12);
  u8g2.drawStr(0, 44, l1);
  u8g2.drawStr(0, 60, l2);
  u8g2.sendBuffer();
}

void displayMsg(const char* l1, const char* l2) {
  displayWake();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB10_tf);
  u8g2.drawStr(0, 24, l1);
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 46, l2);
  u8g2.sendBuffer();
}
