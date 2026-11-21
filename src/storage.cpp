#include "storage.h"
#include <Preferences.h>
#include <string.h>

static Preferences prefs;
static const uint8_t DEF_H[3] = {7, 12, 19};

// Only persistent fields are stored; runtime alert state is reset on boot.
struct SlotBlob {
  char name[25];
  uint8_t hour, minute, enabled;
  uint32_t lastHandledYmd;
};

void storageBegin() { prefs.begin("medbox", false); }

void storageLoadSlots(Slot slots[3]) {
  for (uint8_t i = 0; i < 3; i++) {
    Slot& s = slots[i];
    memset(&s, 0, sizeof s);
    char key[3] = {'s', char('0' + i), 0};
    SlotBlob b;
    if (prefs.getBytes(key, &b, sizeof b) == sizeof b) {
      memcpy(s.name, b.name, sizeof s.name);
      s.name[sizeof s.name - 1] = 0;
      s.hour = b.hour; s.minute = b.minute; s.enabled = b.enabled;
      s.lastHandledYmd = b.lastHandledYmd;
    } else {
      strcpy(s.name, "-");
      s.hour = DEF_H[i];
    }
  }
}

void storageSaveSlot(uint8_t i, const Slot& s) {
  SlotBlob b = {};
  memcpy(b.name, s.name, sizeof b.name);
  b.hour = s.hour; b.minute = s.minute; b.enabled = s.enabled;
  b.lastHandledYmd = s.lastHandledYmd;
  char key[3] = {'s', char('0' + i), 0};
  prefs.putBytes(key, &b, sizeof b);
}

void storageLoadHistory(History& h) {
  if (prefs.getBytes("hist", &h, sizeof h) != sizeof h) h = History();
  if (h.count > History::CAP || h.head >= History::CAP) h = History();
}

void storageSaveHistory(const History& h) { prefs.putBytes("hist", &h, sizeof h); }
