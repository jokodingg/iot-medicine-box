#include "telegram_handler.h"
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <esp_task_wdt.h>
#include "app.h"
#include "backoff.h"
#include "commands.h"
#include "config.h"
#include "secrets.h"

static WiFiClientSecure client;
static UniversalTelegramBot bot(BOT_TOKEN, client);

struct OutMsg { String chat; String text; int8_t slot; };
static const uint8_t OUTBOX_CAP = 10;
static OutMsg outbox[OUTBOX_CAP];
static uint8_t outHead = 0, outCount = 0;

static uint32_t lastPoll = 0;
static Backoff sendBackoff, pollBackoff;
static bool pollTurn = false;
static const uint8_t MAX_SEND_FAILURES = 4;  // then drop: e.g. 403 chat never started the bot

static void enqueue(const String& chat, const String& text, int8_t slot) {
  if (outCount == OUTBOX_CAP) {  // drop oldest
    outHead = (outHead + 1) % OUTBOX_CAP;
    outCount--;
  }
  outbox[(outHead + outCount) % OUTBOX_CAP] = {chat, text, slot};
  outCount++;
}

void tgBroadcast(const String& text, int8_t slot) {
  String list = ALLOWED_CHATS;
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

void tgBegin() {
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  client.setTimeout(10);  // seconds
  bot.longPoll = 0;
}

static void sendOne(uint32_t now) {
  OutMsg& m = outbox[outHead];
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
    outHead = (outHead + 1) % OUTBOX_CAP;
    outCount--;
    sendBackoff.ok();
  }
}

static void pollOnce(uint32_t now) {
  lastPoll = now;
  int n = bot.getUpdates(bot.last_message_received + 1);
  esp_task_wdt_reset();
  if (n < 0) { pollBackoff.fail(now); return; }
  pollBackoff.ok();
  for (int i = 0; i < n; i++) {
    telegramMessage& msg = bot.messages[i];
    if (!isAllowed(msg.chat_id.c_str(), ALLOWED_CHATS)) continue;  // ignore strangers silently
    if (msg.type == "callback_query") {
      handleCallback(msg.chat_id, msg.text);
      bot.answerCallbackQuery(msg.query_id);
      esp_task_wdt_reset();
    } else {
      handleText(msg.chat_id, msg.text);
    }
  }
}

// At most one blocking HTTPS round per loop; sends and polls alternate so a stuck
// message can never starve incoming commands.
void tgLoop(uint32_t now, bool online) {
  if (!online) return;
  bool canSend = outCount && sendBackoff.ready(now);
  bool canPoll = now - lastPoll >= TG_POLL_MS && pollBackoff.ready(now);
  if (canSend && (!canPoll || !pollTurn)) {
    pollTurn = true;
    sendOne(now);
  } else if (canPoll) {
    pollTurn = false;
    pollOnce(now);
  }
}
