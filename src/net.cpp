#include "net.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <time.h>
#include "config.h"
#include "settings.h"

#if __has_include("secrets.h")
#include "secrets.h"
#endif

static WiFiManager wm;
static WiFiManagerParameter pToken("token", "Bot token (dari @BotFather)", "", 64);
static WiFiManagerParameter pChats("chats", "Chat ID (pisahkan dengan koma)", "", 160);

static uint32_t lastTry = 0, lastSync = 0, offlineSince = 0;
static bool synced = false, portalWasActive = false;

static void startNtp(uint32_t now) {
  configTzTime(TZ_INFO, NTP_1, NTP_2);
  lastSync = now;
  synced = true;
}

static void onSaveParams() {
  settingsSave(pToken.getValue(), pChats.getValue());
  Serial.println("[net] bot settings saved");
}

void netStartPortal() {
  if (wm.getConfigPortalActive()) return;
  pToken.setValue(settingsToken(), 64);
  pChats.setValue(settingsChats(), 160);
  Serial.printf("[net] setup portal open: AP %s\n", SETUP_AP_NAME);
  wm.startConfigPortal(SETUP_AP_NAME, SETUP_AP_PASS);  // non-blocking, returns at once
  portalWasActive = true;
}

void netResetWifi() {
  settingsSetWifiSeeded();  // do not fall back to secrets.h after a reset
  wm.resetSettings();       // erases the saved SSID and password
  delay(500);
  ESP.restart();            // boots with no WiFi saved, so the portal opens
}

bool netPortalActive() { return wm.getConfigPortalActive(); }

void netBegin() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  wm.setConfigPortalBlocking(false);  // schedule, buzzer and reeds keep running
  wm.setConfigPortalTimeout(PORTAL_TIMEOUT_S);
  wm.setConnectTimeout(15);
  wm.setTitle("Medicine Box");
  static std::vector<const char*> menu = {"wifi", "exit"};
  wm.setMenu(menu);
  wm.addParameter(&pToken);
  wm.addParameter(&pChats);
  wm.setSaveParamsCallback(onSaveParams);

  bool saved = wm.getWiFiIsSaved();
#ifdef WIFI_SSID
  if (!saved && !settingsWifiSeeded() && strlen(WIFI_SSID) > 0) {  // first boot default from secrets.h
    WiFi.begin(WIFI_SSID, WIFI_PASS);  // persisted by the core
    settingsSetWifiSeeded();
    saved = true;
  } else
#endif
  if (saved) WiFi.begin();
  // Many C3 Super Mini boards have a poor antenna match; lower TX power improves link stability.
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  lastTry = offlineSince = millis();
  setenv("TZ", TZ_INFO, 1);
  tzset();
  if (!saved) netStartPortal();
}

void netLoop(uint32_t now) {
  if (wm.getConfigPortalActive()) {
    wm.process();
    return;  // portal owns the radio; STA retries would break the access point
  }
  if (portalWasActive) {  // portal closed: saved, timed out, or exited
    portalWasActive = false;
    WiFi.mode(WIFI_STA);
    if (WiFi.status() != WL_CONNECTED) WiFi.begin();
    lastTry = offlineSince = now;
  }

  if (WiFi.status() != WL_CONNECTED) {
    if (now - offlineSince >= PORTAL_AFTER_OFFLINE_MS) {
      netStartPortal();
      return;
    }
    if (now - lastTry >= WIFI_RETRY_MS) {
      lastTry = now;
      WiFi.disconnect();
      WiFi.begin();
    }
    return;
  }
  offlineSince = now;
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
