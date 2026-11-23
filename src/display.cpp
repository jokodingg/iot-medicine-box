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

static uint8_t addr = OLED_ADDR;

void displayBegin() {
  Wire.begin(PIN_SDA, PIN_SCL);
  bool a = probe(OLED_ADDR), b = probe(OLED_ADDR_ALT);
  addr = (!a && b) ? OLED_ADDR_ALT : OLED_ADDR;  // default 0x3C unless only 0x3D answers
  Serial.printf("[oled] ack 0x%02X=%d 0x%02X=%d, using 0x%02X\n", OLED_ADDR, a, OLED_ADDR_ALT, b, addr);
  u8g2.setI2CAddress(addr * 2);
  u8g2.begin();
  u8g2.setContrast(255);
  displayMsg("Medicine Box", "Memulai...");
  lastWake = millis();
}

// Re-sends the SSD1306 init sequence. A supply dip (e.g. WiFi TX burst) can reset
// the OLED controller to display-off while the ESP32 keeps running.
void displayReinit() {
  bool ok = probe(addr);
  Serial.printf("[oled] reinit 0x%02X ack=%d\n", addr, ok);
  u8g2.begin();
  u8g2.setContrast(255);
  level = BRIGHT;
  lastWake = millis();
}

void displayWake() {
  lastWake = millis();
  if (level == OFF) displayReinit();  // also recovers a controller that reset while off
  else if (level != BRIGHT) {
    u8g2.setContrast(255);
    level = BRIGHT;
  }
}

bool displayIsOff() { return level == OFF; }

void displayLoop(uint32_t now) {
  // `now` is sampled at the top of loop(); a wake later in the same iteration sets
  // lastWake > now. Unsigned subtraction would then wrap to ~49 days and switch the
  // screen off at once, so treat a negative difference as zero.
  int32_t diff = (int32_t)(now - lastWake);
  uint32_t idle = diff > 0 ? (uint32_t)diff : 0;
  if (level == BRIGHT && idle >= DIM_AFTER_MS) { u8g2.setContrast(10); level = DIM; }
  if (OFF_AFTER_MS && level == DIM && idle >= OFF_AFTER_MS) { u8g2.setPowerSave(1); level = OFF; }
}

void displayIdle(const Clock& c, const char* nextText) {
  if (level == OFF) return;
  char buf[24];
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  if (c.valid) {
    time_t t = time(nullptr);
    struct tm tm;
    localtime_r(&t, &tm);
    snprintf(buf, sizeof buf, "%s %02d/%02d/%04d", HARI[tm.tm_wday], tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    u8g2.drawStr((128 - u8g2.getStrWidth(buf)) / 2, 9, buf);  // date, centered
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
