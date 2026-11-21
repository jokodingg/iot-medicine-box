#pragma once
#include "types.h"
#include "history.h"

void storageBegin();
void storageLoadSlots(Slot slots[3]);
void storageSaveSlot(uint8_t i, const Slot& s);
void storageLoadHistory(History& h);
void storageSaveHistory(const History& h);
