// CombatTraceRead.h -- game-side reader for CombatTrace.h: HUD combat flag. Pause is NOT read here; callers use actq::playerPaused (ActQueue.h).
// Needs saferead::beginScope() per hook entry (SafeRead.h). 0xA904 is a struct offset on the HUD object, not an address.
#pragma once
#include <stdint.h>
#include "SafeRead.h"
#include "CombatTrace.h"

namespace ctrace {

// HUD flags dword at hud+0xA904, bit 1 = in combat (QueueHud.cpp). Returns 0|1, or -1 when hud==0 or unreadable.
inline int8_t readHudCombat(unsigned site, uintptr_t hud) {
    uint32_t flags = 0;
    if (hud == 0 || !saferead::readAt(site, hud, 0xA904, &flags)) return -1;
    return (int8_t)((flags >> 1) & 1);
}

}  // namespace ctrace
