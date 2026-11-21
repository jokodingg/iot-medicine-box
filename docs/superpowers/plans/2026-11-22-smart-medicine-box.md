# Smart IoT Medicine Box Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** ESP32-C3 three-slot pill box with buzzer/OLED/Telegram reminders, reed-switch intake detection, 7-day history, offline-safe.

**Architecture:** Hardware-free logic (scheduler state machine, debounce, history ring, command parser) lives in `lib/core/` and is unit-tested with PlatformIO `native` env (Unity). Thin hardware adapters in `src/` (display, buzzer, net, storage, telegram) call the core. `loop()` is non-blocking, millis-driven, task watchdog enabled.

**Tech Stack:** PlatformIO, Arduino-ESP32, U8g2, UniversalTelegramBot, ArduinoJson 7, Preferences (NVS), Unity.

**Spec:** `docs/superpowers/specs/2026-11-22-smart-medicine-box-design.md`

## Global Constraints
- Board `esp32-c3-devkitm-1`, `ARDUINO_USB_CDC_ON_BOOT=1`.
- Pins: SDA 8, SCL 9, REED 3/4/5, REED_SPARE 6, BUZZER 7. Reed INPUT_PULLUP, LOW = magnet near = door closed.
- OLED SSD1306 128x64 I2C 0x3C, fallback 0x3D.
- TZ `WIB-7`, NTP `pool.ntp.org`, `id.pool.ntp.org`; re-sync every 6 h.
- Window 30 min, remind every 5 min, max 3 reminders, then MISSED.
- History: last 7 days, max 64 records, NVS.
- No `delay()` in loop paths (except ≤10 ms in setup).
- All user-facing text in Indonesian. Secrets only in `include/secrets.h` (gitignored); `include/secrets.example.h` committed.
- Each milestone = one commit, message ends with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.

## Review Focus
1. Door already open when alert fires: must NOT count as taken until a closed→open transition — test in Task 2.
2. Reboot mid-alert / after schedule time passed: slot not re-alerted for same day once TAKEN/MISSED (persist `lastHandledYmd`) — test in Task 2.
3. Time not synced (year < 2024): scheduler must not fire — test in Task 2.
4. `/set` with bad input (`25:00`, slot 4, empty name, name >24 chars) returns usage error, no state change — test in Task 4.
5. Non-whitelisted chat ID: no reply, no state change — test in Task 4.

---

### Task 1: Scaffold + core types + debounce

**Files:** Create `platformio.ini` (envs `c3` and `native`), `include/pins.h`, `include/config.h`, `include/secrets.example.h`, `lib/core/types.h`, `lib/core/debounce.{h,cpp}`, `test/test_debounce/test_main.cpp`, `src/main.cpp` (stub).

**Interfaces — Produces:**
- `enum class SlotState { IDLE, ALERT, TAKEN, MISSED };`
- `struct Slot { char name[25]; uint8_t hour, minute; bool enabled; SlotState state; uint8_t reminders; uint32_t alertStart; uint32_t lastRemind; uint32_t lastHandledYmd; uint32_t snoozeUntil; };`
- `class Debouncer { Debouncer(uint16_t ms=50); bool update(bool raw, uint32_t now); bool stable() const; bool rose() const; bool fell() const; };` (`rose()` = became HIGH = door opened, true for one update).

- [ ] Test `test_debounce`: bounce under 50 ms no change; stable 50 ms changes once; `rose()` true exactly one call.
- [ ] `pio test -e native -f test_debounce` → FAIL, implement, → PASS.
- [ ] `pio run -e c3` builds stub.
- [ ] Commit `feat: scaffold project and debouncer`.

### Task 2: Scheduler state machine

**Files:** Create `lib/core/scheduler.{h,cpp}`, `test/test_scheduler/test_main.cpp`.

**Interfaces — Produces:**
- `enum class SchedEvent { NONE, ALERT, REMIND, TAKEN, MISSED };`
- `struct Clock { uint32_t ymd; uint8_t h, m; bool valid; uint32_t ms; };`
- `SchedEvent schedulerTick(Slot&, const Clock&, bool doorOpened);` `doorOpened` = debounced rising edge this tick.
- `void schedulerSnooze(Slot&, uint32_t now, uint16_t minutes);` `void schedulerMarkTaken(Slot&, uint32_t ymd);`

Rules: IDLE→ALERT when valid && enabled && h:m == schedule && lastHandledYmd != ymd. ALERT + doorOpened within 30 min → TAKEN. Every 5 min without open → REMIND (reminders++); after 3rd reminder's 5 min elapse → MISSED. TAKEN/MISSED set lastHandledYmd and return to IDLE next tick. Snooze suppresses REMIND until snoozeUntil and extends window by snooze.

- [ ] Tests: alert fires at time; open at +10 min → TAKEN; no open → REMIND at 5/10/15, MISSED at 20; door open edge before alert ignored; same day no re-alert after handled; invalid clock never fires; disabled never fires; snooze 10 delays remind.
- [ ] Run FAIL → implement → PASS.
- [ ] Commit `feat: scheduler state machine`.

### Task 3: History ring + storage adapter

**Files:** Create `lib/core/history.{h,cpp}`, `test/test_history/test_main.cpp`, `src/storage.{h,cpp}`.

**Interfaces — Produces:**
- `struct Record { uint32_t epoch; uint8_t slot; uint8_t taken; };` (`taken` 1=diminum, 0=terlewat)
- `class History { void add(Record); void prune(uint32_t nowEpoch); size_t size(); const Record& at(size_t newestFirst); Record buf[64]; uint8_t head, count; };` prune drops > 7*86400 s old.
- storage: `void storageBegin(); void storageLoadSlots(Slot[3]); void storageSaveSlot(uint8_t i, const Slot&); void storageLoadHistory(History&); void storageSaveHistory(const History&);` Preferences namespace `medbox`, blob keys `s0..s2`, `hist`. Defaults: Pagi 07:00, Siang 12:00, Malam 19:00, disabled, name "-".

- [ ] Tests: wraparound at 64; newest-first order; prune 8-day-old.
- [ ] FAIL → implement → PASS; `pio run -e c3` builds.
- [ ] Commit `feat: persistent history and slot storage`.

### Task 4: Command parser + whitelist

**Files:** Create `lib/core/commands.{h,cpp}`, `test/test_commands/test_main.cpp`.

**Interfaces — Produces:**
- `enum class Cmd { START, HELP, STATUS, JADWAL, SET, HAPUS, RIWAYAT, TUNDA, DIAM, TES, UNKNOWN, INVALID };`
- `struct Parsed { Cmd cmd; uint8_t slot; uint8_t h, m; uint16_t minutes; char name[25]; };`
- `Parsed parseCommand(const char* text);` (strips `@botname`; `/set 1 07:30 Amlodipin 5mg`; `/tunda` default 10, range 1–120).
- `bool isAllowed(const char* chatId, const char* csvWhitelist);`

- [ ] Tests: each command; `/set` bad cases from Review Focus 4 → INVALID; whitelist `"111,222"` allows 222, rejects 333 and `"22"`.
- [ ] FAIL → implement → PASS.
- [ ] Commit `feat: telegram command parser and whitelist`.

### Task 5: Hardware adapters — buzzer, display, net

**Files:** Create `src/buzzer.{h,cpp}`, `src/display.{h,cpp}`, `src/net.{h,cpp}`.

**Interfaces — Produces:**
- `void buzzerBegin(); void buzzerStart(uint8_t pattern); void buzzerStop(); void buzzerMute(uint32_t untilMs); void buzzerLoop(uint32_t now);` patterns: `PAT_ALERT` (3×200 ms on/150 off, 1 s pause, repeat up to 60 s), `PAT_TEST` (one 500 ms). `BUZZER_ACTIVE` true → digitalWrite; false → `tone(PIN, BUZZER_FREQ=2700)`.
- `void displayBegin(); void displayIdle(const Clock&, bool wifi, const char* nextText); void displayAlert(const char* name); void displayMsg(const char* l1, const char* l2); void displayWake(); void displayLoop(uint32_t now);` dim (contrast 10) after 30 s idle, off after 120 s; wake on alert/door/command.
- `void netBegin(); void netLoop(uint32_t now); bool netOnline(); Clock netClock();` WiFi reconnect every 10 s while down; `configTzTime("WIB-7", ...)`, re-sync 6 h.

- [ ] `pio run -e c3` SUCCESS, no warnings in `src/`.
- [ ] Commit `feat: buzzer, display and network adapters`.

### Task 6: Telegram handler + main loop wiring

**Files:** Create `src/telegram_handler.{h,cpp}`; modify `src/main.cpp`.

**Interfaces — Consumes:** all above. **Produces:**
- `void tgBegin(); void tgLoop(uint32_t now); void tgQueue(const String& text, bool withButtons, uint8_t slot);`
- Outbox: 10 entries RAM; poll every 3 s when online; on send failure backoff 2,4,8…max 300 s.
- Callback data `taken:<slot>`, `snooze:<slot>` → `schedulerMarkTaken` / `schedulerSnooze(…,10)`.
- main: `esp_task_wdt` 30 s; loop order: net → reeds (debouncers) → scheduler per slot → on event: buzzer/display/tgQueue/history save → tg → buzzer → display.
- Messages: ALERT "⏰ Waktunya minum obat: <nama> (<Pagi/Siang/Malam> <HH:MM>)", TAKEN "✅ <nama> sudah diminum <HH:MM>", MISSED "⚠️ <nama> terlewat (jadwal <HH:MM>)".

- [ ] `pio run -e c3` SUCCESS; `pio test -e native` all PASS.
- [ ] Commit `feat: telegram handler and main loop`.

### Task 7: Docs

**Files:** Create `WIRING.md` (pin table, ASCII diagram, NPN + 1k + optional flyback note), `README.md` (BotFather, @userinfobot chat ID, secrets, flash `pio run -e c3 -t upload`, commands), `TESTING.md` (checklist: each reed, buzzer, OLED, schedule, power loss, WiFi down, whitelist), casing placement section in WIRING.md (reed on box body edge, magnet on lid, ≤10 mm gap; ESP32+OLED in box 2 front).

- [ ] Commit `docs: wiring, readme, test checklist`.
