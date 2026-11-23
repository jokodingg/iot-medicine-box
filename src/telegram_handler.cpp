#include "telegram_handler.h"
#include <UniversalTelegramBot.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "app.h"
#include "backoff.h"
#include "commands.h"
#include "config.h"
#include "settings.h"

static WiFiClientSecure client;
static UniversalTelegramBot bot("", client);
static uint32_t settingsRev = 0;   // main side
static uint32_t resetWifiAt = 0;   // main side

// Threading: the network task owns `bot` and `client` and only does HTTPS.
// Commands are parsed and executed on the main loop, so app state is never
// touched from two tasks. Shared data: outbox + pending token (under `lock`),
// inbox (FreeRTOS queue), `paused` flag.
static SemaphoreHandle_t lock;
static QueueHandle_t inbox;
static volatile bool paused = false;
static volatile bool botReady = false;  // last poll succeeded
static char pendingToken[64] = {};
static bool tokenDirty = false;

struct InMsg {
  char chat[24];
  char text[160];
  bool callback;
};

struct OutMsg { String chat; String text; int8_t slot; };
static const uint8_t OUTBOX_CAP = 10;
static OutMsg outbox[OUTBOX_CAP];
static uint8_t outHead = 0, outCount = 0;

static uint32_t lastPoll = 0;
static Backoff sendBackoff, pollBackoff;
static bool pollTurn = false;
static const uint8_t MAX_SEND_FAILURES = 4;  // then drop: e.g. 403 chat never started the bot

static void enqueue(const String& chat, const String& text, int8_t slot) {
  xSemaphoreTake(lock, portMAX_DELAY);
  if (outCount == OUTBOX_CAP) {  // drop oldest
    outHead = (outHead + 1) % OUTBOX_CAP;
    outCount--;
  }
  outbox[(outHead + outCount) % OUTBOX_CAP] = {chat, text, slot};
  outCount++;
  xSemaphoreGive(lock);
}

void tgBroadcast(const String& text, int8_t slot) {
  String list = settingsChats();
  int start = 0;
  while (start < (int)list.length()) {
    int comma = list.indexOf(',', start);
    if (comma < 0) comma = list.length();
    String id = list.substring(start, comma);
    id.trim();
    if (id.length()) enqueue(id, text, slot);
    start = comma + 1;
  }
}

static void reply(const String& chat, const String& text) { enqueue(chat, text, -1); }

static String hhmm(uint8_t h, uint8_t m) {
  char b[6];
  snprintf(b, sizeof b, "%02u:%02u", h, m);
  return b;
}

static String fmtEpoch(uint32_t e) {
  time_t t = e;
  struct tm tm;
  localtime_r(&t, &tm);
  char b[16];
  snprintf(b, sizeof b, "%02d/%02d %02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_hour, tm.tm_min);
  return b;
}

static String helpText() {
  return F("💊 Smart Medicine Box\n\n"
           "/status - kondisi 3 kompartemen\n"
           "/jadwal - lihat semua jadwal\n"
           "/set <1-3> <HH:MM> <nama obat> - atur jadwal\n"
           "/hapus <1-3> - nonaktifkan jadwal\n"
           "/riwayat - riwayat 7 hari\n"
           "/tunda <menit> - tunda pengingat (default 10)\n"
           "/diam - matikan buzzer 1 jam\n"
           "/tes - tes buzzer dan OLED\n"
           "/setup - buka mode setup WiFi/bot 3 menit\n"
           "/resetwifi - hapus WiFi tersimpan lalu restart ke mode setup\n"
           "/bantuan - pesan ini\n\n"
           "Slot: 1=Pagi, 2=Siang, 3=Malam");
}

static String jadwalText() {
  String s = "📅 Jadwal:\n";
  Slot* sl = appSlots();
  for (uint8_t i = 0; i < 3; i++) {
    s += String(i + 1) + ". " + SLOT_LABEL[i] + " ";
    if (sl[i].enabled) s += hhmm(sl[i].hour, sl[i].minute) + " - " + sl[i].name + "\n";
    else s += "(nonaktif)\n";
  }
  return s;
}

static String statusText() {
  String s = "📦 Status:\n";
  Slot* sl = appSlots();
  const History& h = appHistory();
  for (uint8_t i = 0; i < 3; i++) {
    s += "\n" + String(i + 1) + ". " + SLOT_LABEL[i] + "\n";
    s += "  Pintu: " + String(appDoorOpen(i) ? "terbuka" : "tertutup") + "\n";
    s += "  Jadwal: " + (sl[i].enabled ? hhmm(sl[i].hour, sl[i].minute) + " " + sl[i].name : String("nonaktif")) + "\n";
    if (sl[i].state == SlotState::ALERT) s += "  ⏰ Menunggu diminum\n";
    String last = "-";
    for (size_t k = 0; k < h.size(); k++)
      if (h.at(k).slot == i && h.at(k).taken) { last = fmtEpoch(h.at(k).epoch); break; }
    s += "  Terakhir diminum: " + last + "\n";
  }
  return s;
}

static String riwayatText() {
  const History& h = appHistory();
  if (h.size() == 0) return "📜 Riwayat kosong.";
  String s = "📜 Riwayat 7 hari:\n";
  for (size_t k = 0; k < h.size() && k < 40; k++) {
    const Record& r = h.at(k);
    s += fmtEpoch(r.epoch) + " " + SLOT_LABEL[r.slot % 3] + " " + (r.taken ? "✅ diminum" : "❌ terlewat") + "\n";
  }
  return s;
}

static void handleText(const String& chat, const String& text) {
  Parsed p = parseCommand(text.c_str());
  switch (p.cmd) {
    case Cmd::START:
    case Cmd::HELP: reply(chat, helpText()); break;
    case Cmd::STATUS: reply(chat, statusText()); break;
    case Cmd::JADWAL: reply(chat, jadwalText()); break;
    case Cmd::RIWAYAT: reply(chat, riwayatText()); break;
    case Cmd::SET:
      appSetSlot(p.slot, p.name, p.h, p.m);
      reply(chat, "✅ Slot " + String(p.slot + 1) + " (" + SLOT_LABEL[p.slot] + ") diatur: " + hhmm(p.h, p.m) + " - " + p.name);
      break;
    case Cmd::HAPUS:
      appClearSlot(p.slot);
      reply(chat, "🗑 Slot " + String(p.slot + 1) + " (" + SLOT_LABEL[p.slot] + ") dinonaktifkan.");
      break;
    case Cmd::TUNDA: {
      uint8_t n = appSnooze(p.minutes);
      reply(chat, n ? "😴 Pengingat ditunda " + String(p.minutes) + " menit." : String("Tidak ada pengingat aktif."));
      break;
    }
    case Cmd::DIAM: appMute(); reply(chat, "🔇 Buzzer dimatikan selama 1 jam."); break;
    case Cmd::TES: appTest(); reply(chat, "🔔 Tes buzzer dan OLED dijalankan."); break;
    case Cmd::SETUP:
      appStartSetup();
      reply(chat, String("📶 Mode setup aktif ") + (PORTAL_TIMEOUT_S / 60) + " menit.\n"
                  "1. Sambungkan HP ke WiFi \"" + SETUP_AP_NAME + "\" (password " + SETUP_AP_PASS + ").\n"
                  "2. Buka 192.168.4.1 jika halaman tidak muncul otomatis.\n"
                  "3. Pilih \"Configure WiFi\", isi WiFi, token, dan chat ID, lalu Save.");
      break;
    case Cmd::RESETWIFI:
      resetWifiAt = millis() + 3000;  // let the reply go out first
      reply(chat, String("📶 WiFi tersimpan akan dihapus dan board restart.\n"
                         "Lalu sambungkan HP ke WiFi \"") + SETUP_AP_NAME + "\" (password " + SETUP_AP_PASS +
                         ") dan buka 192.168.4.1.");
      break;
    case Cmd::INVALID: reply(chat, "⚠️ Format salah. Ketik /bantuan untuk contoh."); break;
    case Cmd::UNKNOWN: reply(chat, "Perintah tidak dikenal. Ketik /bantuan."); break;
  }
}

static void handleCallback(const String& chat, const String& data) {
  int colon = data.indexOf(':');
  if (colon < 0) return;
  String act = data.substring(0, colon);
  int slot = data.substring(colon + 1).toInt();
  if (slot < 0 || slot > 2) return;
  if (act == "taken") {
    if (!appMarkTaken(slot)) reply(chat, "Pengingat slot " + String(slot + 1) + " sudah tidak aktif.");
  } else if (act == "snooze") {
    reply(chat, appSnoozeSlot(slot, 10) ? String("😴 Ditunda 10 menit.") : "Pengingat slot " + String(slot + 1) + " sudah tidak aktif.");
  }
}

// ---- main loop side ----

static void applySettings() {
  settingsRev = settingsRevision();
  xSemaphoreTake(lock, portMAX_DELAY);
  strncpy(pendingToken, settingsToken(), sizeof pendingToken - 1);
  tokenDirty = true;
  xSemaphoreGive(lock);
}

static uint8_t outboxSize() {
  xSemaphoreTake(lock, portMAX_DELAY);
  uint8_t n = outCount;
  xSemaphoreGive(lock);
  return n;
}

void tgLoop(uint32_t now, bool portalActive) {
  paused = portalActive;  // leave the radio to the setup portal
  if (settingsRev != settingsRevision()) applySettings();

  InMsg m;
  while (xQueueReceive(inbox, &m, 0) == pdTRUE) {
    if (!isAllowed(m.chat, settingsChats())) continue;  // ignore strangers silently
    if (m.callback) handleCallback(m.chat, m.text);
    else handleText(m.chat, m.text);
  }

  if (resetWifiAt) {
    int32_t since = (int32_t)(now - resetWifiAt);
    if ((since >= 0 && outboxSize() == 0) || since >= 15000) appResetWifi();  // 15 s: reply stuck, reset anyway
  }
}

// ---- network task side ----

static void sendOne(uint32_t now) {
  xSemaphoreTake(lock, portMAX_DELAY);
  if (!outCount) { xSemaphoreGive(lock); return; }
  OutMsg m = outbox[outHead];  // copy, then send without holding the lock
  xSemaphoreGive(lock);

  bool ok;
  if (m.slot >= 0) {
    String kb = "[[{\"text\":\"✅ Sudah minum\",\"callback_data\":\"taken:" + String(m.slot) +
                "\"},{\"text\":\"⏰ Tunda 10 menit\",\"callback_data\":\"snooze:" + String(m.slot) + "\"}]]";
    ok = bot.sendMessageWithInlineKeyboard(m.chat, m.text, "", kb);
  } else {
    ok = bot.sendMessage(m.chat, m.text, "");
  }
  if (!ok) sendBackoff.fail(now);
  if (ok || sendBackoff.failures() >= MAX_SEND_FAILURES) {
    if (!ok) Serial.printf("[tg] drop message to %s after %u failures\n", m.chat.c_str(), sendBackoff.failures());
    xSemaphoreTake(lock, portMAX_DELAY);
    if (outCount) {  // enqueue may have dropped the oldest meanwhile; pop whatever is at the head
      outHead = (outHead + 1) % OUTBOX_CAP;
      outCount--;
    }
    xSemaphoreGive(lock);
    sendBackoff.ok();
  }
}

static void pollOnce(uint32_t now) {
  lastPoll = now;
  int n = bot.getUpdates(bot.last_message_received + 1);
  if (n < 0) { pollBackoff.fail(now); botReady = false; return; }
  pollBackoff.ok();
  botReady = true;
  for (int i = 0; i < n; i++) {
    telegramMessage& msg = bot.messages[i];
    InMsg m = {};
    strncpy(m.chat, msg.chat_id.c_str(), sizeof m.chat - 1);
    strncpy(m.text, msg.text.c_str(), sizeof m.text - 1);
    m.callback = msg.type == "callback_query";
    if (m.callback) bot.answerCallbackQuery(msg.query_id);
    if (xQueueSend(inbox, &m, 0) != pdTRUE) Serial.println("[tg] inbox full, message dropped");
  }
}

// At most one blocking HTTPS round per iteration; sends and polls alternate so a
// stuck message can never starve incoming commands.
static void netStep(uint32_t now) {
  xSemaphoreTake(lock, portMAX_DELAY);
  bool dirty = tokenDirty;
  String token = pendingToken;
  tokenDirty = false;
  bool haveOut = outCount > 0;
  xSemaphoreGive(lock);
  if (dirty) bot.updateToken(token);

  if (paused || WiFi.status() != WL_CONNECTED) { botReady = false; return; }
  bool canSend = haveOut && sendBackoff.ready(now);
  bool canPoll = now - lastPoll >= TG_POLL_MS && pollBackoff.ready(now);
  if (canSend && (!canPoll || !pollTurn)) {
    pollTurn = true;
    sendOne(now);
  } else if (canPoll) {
    pollTurn = false;
    pollOnce(now);
  }
}

static void netTask(void*) {
  for (;;) {
    netStep(millis());
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

bool tgReady() { return botReady; }

void tgBegin() {
  lock = xSemaphoreCreateMutex();
  inbox = xQueueCreate(8, sizeof(InMsg));
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  client.setTimeout(10);  // seconds
  bot.longPoll = 0;
  applySettings();
  // TLS needs a large stack. Same priority as loopTask; it blocks on the network,
  // so the main loop keeps running while a request is in flight.
  xTaskCreate(netTask, "tg", 12288, nullptr, 1, nullptr);
}
