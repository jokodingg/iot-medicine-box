#include <Arduino.h>
#include <esp_task_wdt.h>
#include <string.h>
#include "app.h"
#include "buzzer.h"
#include "config.h"
#include "debounce.h"
#include "display.h"
#include "net.h"
#include "pins.h"
#include "scheduler.h"
#include "storage.h"
#include "telegram_handler.h"

static Slot slots[3];
static History history;
static Debouncer reeds[3] = {Debouncer(DEBOUNCE_MS), Debouncer(DEBOUNCE_MS), Debouncer(DEBOUNCE_MS)};
static SchedConfig schedCfg;
static uint32_t lastScreen = 0, holdUntil = 0;

static void showMsg(const char* a, const char* b) {
  displayMsg(a, b);
  holdUntil = millis() + 4000;  // keep message visible before idle screen returns
}

static String hhmm(uint8_t h, uint8_t m) {
  char b[6];
  snprintf(b, sizeof b, "%02u:%02u", h, m);
  return b;
}

static bool anyAlerting() {
  for (auto& s : slots)
    if (s.state == SlotState::ALERT) return true;
  return false;
}

static void logResult(uint8_t i, bool taken) {
  uint32_t e = netEpoch();
  history.prune(e);
  history.add({e, i, (uint8_t)taken});
  storageSaveHistory(history);
}

static void onTaken(uint8_t i) {
  if (!anyAlerting()) buzzerStop();
  logResult(i, true);
  Clock c = netClock(millis());
  tgBroadcast("✅ " + String(slots[i].name) + " (" + SLOT_LABEL[i] + ") sudah diminum pukul " + hhmm(c.h, c.m));
  showMsg("Sudah diminum", slots[i].name);
}

static void onEvent(uint8_t i, SchedEvent ev) {
  Slot& s = slots[i];
  String sched = String(SLOT_LABEL[i]) + " " + hhmm(s.hour, s.minute);
  switch (ev) {
    case SchedEvent::ALERT:
      storageSaveSlot(i, s);  // persists lastHandledYmd
      buzzerStart(PAT_ALERT);
      displayAlert(sched.c_str(), s.name);
      tgBroadcast("⏰ Waktunya minum obat: " + String(s.name) + " (" + sched + ")", i);
      break;
    case SchedEvent::REMIND:
      buzzerStart(PAT_ALERT);
      displayAlert(sched.c_str(), s.name);
      tgBroadcast("🔔 Pengingat ke-" + String(s.reminders) + ": " + String(s.name) + " (" + sched + ") belum diminum.", i);
      break;
    case SchedEvent::TAKEN:
      onTaken(i);
      break;
    case SchedEvent::MISSED:
      if (!anyAlerting()) buzzerStop();
      logResult(i, false);
      tgBroadcast("⚠️ " + String(s.name) + " terlewat (jadwal " + sched + ").");
      showMsg("Obat terlewat", s.name);
      break;
    case SchedEvent::NONE:
      break;
  }
}

static String nextText(const Clock& c) {
  if (!c.valid) return "Menunggu sinkron waktu";
  int now = c.h * 60 + c.m, best = -1, bestDelta = 24 * 60 + 1;
  for (int i = 0; i < 3; i++) {
    if (!slots[i].enabled) continue;
    int delta = (slots[i].hour * 60 + slots[i].minute - now + 24 * 60) % (24 * 60);
    if (delta == 0) delta = 24 * 60;
    if (delta < bestDelta) { bestDelta = delta; best = i; }
  }
  if (best < 0) return "Belum ada jadwal";
  String t = String("Next ") + SLOT_LABEL[best] + " " + hhmm(slots[best].hour, slots[best].minute) + " " + slots[best].name;
  return t.substring(0, 21);
}

static void setupWatchdog() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  esp_task_wdt_config_t cfg = {WDT_TIMEOUT_S * 1000, 0, true};
  if (esp_task_wdt_init(&cfg) != ESP_OK) esp_task_wdt_reconfigure(&cfg);
#else
  esp_task_wdt_init(WDT_TIMEOUT_S, true);
#endif
  esp_task_wdt_add(NULL);
}

// ---- app.h ----
Slot* appSlots() { return slots; }
const History& appHistory() { return history; }
bool appDoorOpen(uint8_t i) { return reeds[i].stable(); }

void appSetSlot(uint8_t i, const char* name, uint8_t h, uint8_t m) {
  Slot& s = slots[i];
  strncpy(s.name, name, sizeof s.name - 1);
  s.name[sizeof s.name - 1] = 0;
  s.hour = h;
  s.minute = m;
  s.enabled = true;
  if (s.state != SlotState::ALERT) s.lastHandledYmd = 0;  // new time may still fire today
  storageSaveSlot(i, s);
}

void appClearSlot(uint8_t i) {
  Slot& s = slots[i];
  s.enabled = false;
  strcpy(s.name, "-");
  s.state = SlotState::IDLE;
  if (!anyAlerting()) buzzerStop();
  storageSaveSlot(i, s);
}

bool appMarkTaken(uint8_t i) {
  if (!schedulerMarkTaken(slots[i])) return false;
  onTaken(i);
  return true;
}

bool appSnoozeSlot(uint8_t i, uint16_t minutes) {
  if (slots[i].state != SlotState::ALERT) return false;
  schedulerSnooze(slots[i], millis(), minutes, schedCfg);
  buzzerStop();  // next reminder restarts it
  return true;
}

uint8_t appSnooze(uint16_t minutes) {
  uint8_t n = 0;
  for (uint8_t i = 0; i < 3; i++) n += appSnoozeSlot(i, minutes);
  return n;
}

void appMute() { buzzerMute(millis() + 3600000UL); }

void appTest() {
  buzzerStart(PAT_TEST);
  showMsg("Tes OLED", "Buzzer berbunyi");
}

// ---- Arduino ----
void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 3; i++) pinMode(PIN_REED[i], INPUT_PULLUP);
  pinMode(PIN_REED_SPARE, INPUT_PULLUP);
  buzzerBegin();
  displayBegin();
  storageBegin();
  storageLoadSlots(slots);
  storageLoadHistory(history);
  netBegin();
  tgBegin();
  setupWatchdog();
  tgBroadcast("💊 Medicine Box menyala. Ketik /bantuan.");
}

void loop() {
  esp_task_wdt_reset();
  uint32_t now = millis();
  netLoop(now);
  Clock c = netClock(now);

  for (uint8_t i = 0; i < 3; i++) {
    // Reed NO contact closes with magnet near (door shut) -> LOW. HIGH = door open.
    reeds[i].update(digitalRead(PIN_REED[i]) == HIGH, now);
    bool opened = reeds[i].rose();
    if (opened || reeds[i].fell()) displayWake();
    onEvent(i, schedulerTick(slots[i], c, opened, schedCfg));
  }

  tgLoop(now, netOnline());
  buzzerLoop(now);

  if (now - lastScreen >= 1000) {
    lastScreen = now;
    if (anyAlerting()) displayWake();
    else if ((int32_t)(now - holdUntil) < 0) {}  // keep alert screen visible
    else displayIdle(c, netOnline(), nextText(c).c_str());
  }
  displayLoop(now);
}
