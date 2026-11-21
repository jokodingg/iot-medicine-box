# Smart IoT Medicine Box — Design

## Intent
Three-compartment pill box (Pagi/Siang/Malam) on ESP32-C3 Super Mini, controlled via Telegram. Reminds on schedule (buzzer + OLED + Telegram), detects door opening via reed switch, logs taken/missed for 7 days. Must keep working offline.

## Goals
- Per slot: medicine name, HH:MM (WIB, NTP), enabled flag.
- Alert at schedule; door opened within 30 min => "diminum"; else remind every 5 min, max 3x, then "terlewat" + Telegram warning.
- OLED idle screen: clock, date, WiFi, next schedule; auto-dim then off.
- 7-day history persisted in flash.
- Telegram commands (Indonesian): /start /bantuan /status /jadwal /set /hapus /riwayat /tunda /diam /tes; inline buttons "Sudah minum", "Tunda 10 menit".
- Chat ID whitelist; secrets in gitignored secrets.h.

## Non-goals
Web UI, OTA, more than 3 slots (4th reed wired as spare only), multi-user roles.

## Anti-goals
Blocking delay(); losing schedule or history on power loss; reminders stopping when WiFi is down.

## Hardware (confirmed)
- Buzzer: bare 2-pin, driven via NPN (S8050/2N2222), 1k base resistor. `BUZZER_ACTIVE` constant switches to tone() for passive.
- Reed: bare 2-pin NO, to GND, INPUT_PULLUP, no resistor.
- OLED: 4-pin GND/VCC/SCK(SCL)/SDA, SSD1306 0x3C (fallback 0x3D).

## Pins
SDA 8, SCL 9, Reed 3/4/5 (+spare 6), Buzzer 7. Unused: 2, 20, 21.

## Approaches
1. **UniversalTelegramBot + U8g2 + Preferences (chosen)** — mature, small, NVS is power-safe for ~21 history records.
2. Manual HTTPS + ArduinoJson — more control, more code; not needed.
3. LittleFS log files — more flexible, overkill for this size.

## Architecture
Modules: config.h, pins.h, net (WiFi/NTP), reed (debounce), buzzer (non-blocking patterns), scheduler (per-slot state machine IDLE→ALERT→REMIND→TAKEN/MISSED), storage (Preferences), display (U8g2), telegram_handler (commands + outbox with exponential backoff), main.cpp loop. Task watchdog enabled.

## Left to builder
Exact beep patterns, OLED layout details, backoff constants.
