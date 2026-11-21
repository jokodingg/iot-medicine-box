#include "net.h"
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "config.h"
#include "secrets.h"

static uint32_t lastTry = 0, lastSync = 0;
static bool synced = false;

static void startNtp(uint32_t now) {
  configTzTime(TZ_INFO, NTP_1, NTP_2);
  lastSync = now;
  synced = true;
}

void netBegin() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  // Many C3 Super Mini boards have a poor antenna match; lower TX power improves link stability.
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  lastTry = millis();
  setenv("TZ", TZ_INFO, 1);
  tzset();
}

void netLoop(uint32_t now) {
  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastTry >= WIFI_RETRY_MS) {
      lastTry = now;
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
    return;
  }
  if (!synced || now - lastSync >= NTP_RESYNC_MS) startNtp(now);
}

bool netOnline() { return WiFi.status() == WL_CONNECTED; }

uint32_t netEpoch() {
  time_t t = time(nullptr);
  return t >= 1704067200 ? (uint32_t)t : 0;  // 2024-01-01
}

Clock netClock(uint32_t now) {
  Clock c = {};
  c.ms = now;
  time_t t = time(nullptr);
  c.valid = t >= 1704067200;
  if (c.valid) {
    struct tm tm;
    localtime_r(&t, &tm);
    c.ymd = (tm.tm_year + 1900) * 10000 + (tm.tm_mon + 1) * 100 + tm.tm_mday;
    c.h = tm.tm_hour;
    c.m = tm.tm_min;
  }
  return c;
}
