// Connects to WiFi, syncs NTP, then sends one Telegram message to every
// chat in ALLOWED_CHATS. Requires include/secrets.h.
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <time.h>
#include "secrets.h"

static WiFiClientSecure client;
static UniversalTelegramBot bot(BOT_TOKEN, client);

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.printf("\n[WIFI] connecting to %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[WIFI] FAILED: check SSID/password, use 2.4 GHz network");
    return;
  }
  Serial.printf("\n[WIFI] ok IP=%s RSSI=%d dBm\n",
                WiFi.localIP().toString().c_str(), WiFi.RSSI());

  configTime(7 * 3600, 0, "pool.ntp.org", "time.google.com");
  struct tm tm;
  if (getLocalTime(&tm, 10000))
    Serial.printf("[NTP] %04d-%02d-%02d %02d:%02d:%02d WIB\n", tm.tm_year + 1900,
                  tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  else
    Serial.println("[NTP] FAILED");

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  String chats = ALLOWED_CHATS;
  int start = 0;
  while (start < (int)chats.length()) {
    int comma = chats.indexOf(',', start);
    if (comma < 0) comma = chats.length();
    String id = chats.substring(start, comma);
    id.trim();
    if (id.length()) {
      bool ok = bot.sendMessage(id, "Hardware test: koneksi Telegram OK", "");
      Serial.printf("[TELEGRAM] chat %s: %s\n", id.c_str(), ok ? "sent" : "FAILED");
    }
    start = comma + 1;
  }
}

void loop() {}
