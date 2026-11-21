#pragma once
#include <stdint.h>

#define BUZZER_ACTIVE true          // false = passive buzzer, driven with tone()
constexpr uint16_t BUZZER_FREQ = 2700;

constexpr uint8_t OLED_ADDR = 0x3C;
constexpr uint8_t OLED_ADDR_ALT = 0x3D;

constexpr const char* TZ_INFO = "WIB-7";
constexpr const char* NTP_1 = "pool.ntp.org";
constexpr const char* NTP_2 = "id.pool.ntp.org";
constexpr uint32_t NTP_RESYNC_MS = 6UL * 3600UL * 1000UL;

constexpr uint32_t WINDOW_MIN = 30;
constexpr uint32_t REMIND_EVERY_MIN = 5;
constexpr uint8_t  MAX_REMINDERS = 3;

constexpr uint16_t DEBOUNCE_MS = 50;
constexpr uint32_t DIM_AFTER_MS = 30000;
constexpr uint32_t OFF_AFTER_MS = 120000;
constexpr uint32_t WIFI_RETRY_MS = 10000;
constexpr uint32_t TG_POLL_MS = 3000;
constexpr uint32_t WDT_TIMEOUT_S = 30;

static const char* const SLOT_LABEL[3] = {"Pagi", "Siang", "Malam"};
