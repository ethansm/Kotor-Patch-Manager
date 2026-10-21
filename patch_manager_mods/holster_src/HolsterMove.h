// HolsterMove.h -- pure logic of the move hold and the party combat hold of the holstered-weapons mod (doc 42; no Win32, no heap, host-testable;
// tests in holster_move_test.cpp). 0.6.0-dev.
//   isLocoAnim         the walk / run animation ids (low 16 bits 0x2712, 0x2713, 0x2714) the engine resolves for a moving creature
//   MoveTable          16 (owner, last-seen tick) entries; note() records a locomotion sighting, movedWithin() asks "moved in the last holdMs ms?"
//                      (wrap-safe 32-bit tick compare; a full table overwrites the oldest entry; owner 0 is ignored)
//   anyCombat          party-wide combat hold: true while any of the first n client combat flags is non-zero
//   moveHoldBlocksCut  the delay cut (server +0x524 = 0) is skipped while the creature is moving and the hold is enabled
// Failure mode rule: nothing here reads the engine; an unknown owner is "not moving" (the old behaviour), a negative age (tick from the future) is "not moving".
#pragma once
#include <stdint.h>

namespace hs {

inline bool isLocoAnim(uint32_t id) {
    const uint32_t low = id & 0xFFFFu;
    return low == 0x2712u || low == 0x2713u || low == 0x2714u;
}

struct MoveTable {
    static constexpr int kCap = 16;
    struct Entry { uintptr_t owner; uint32_t tick; bool used; };
    Entry e[kCap] = {};

    void note(uintptr_t owner, uint32_t now) {
        if (owner == 0) return;
        for (int i = 0; i < kCap; ++i)
            if (e[i].used && e[i].owner == owner) { e[i].tick = now; return; }
        int slot = -1;
        for (int i = 0; i < kCap; ++i)
            if (!e[i].used) { slot = i; break; }
        if (slot < 0) {   // full: overwrite the entry with the oldest wrap-safe age
            int32_t oldestAge = static_cast<int32_t>(now - e[0].tick);
            slot = 0;
            for (int i = 1; i < kCap; ++i) {
                const int32_t age = static_cast<int32_t>(now - e[i].tick);
                if (age > oldestAge) { oldestAge = age; slot = i; }
            }
        }
        e[slot].owner = owner;
        e[slot].tick = now;
        e[slot].used = true;
    }

    bool movedWithin(uintptr_t owner, uint32_t now, uint32_t holdMs) const {
        if (owner == 0 || holdMs == 0) return false;
        for (int i = 0; i < kCap; ++i) {
            if (!e[i].used || e[i].owner != owner) continue;
            const int32_t age = static_cast<int32_t>(now - e[i].tick);
            return age >= 0 && static_cast<uint32_t>(age) <= holdMs;
        }
        return false;
    }
};

inline bool anyCombat(const uint32_t* flags, int n) {
    if (!flags || n <= 0) return false;
    if (n > 8) n = 8;
    for (int i = 0; i < n; ++i)
        if (flags[i] != 0) return true;
    return false;
}

inline bool moveHoldBlocksCut(bool enabled, bool moved) { return enabled && moved; }

}  // namespace hs
