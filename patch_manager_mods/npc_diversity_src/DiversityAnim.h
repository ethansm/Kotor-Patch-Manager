// DiversityAnim.h -- NB2 pure scheduling logic for ambient flourishes on idle world NPCs (doc 40 sections 9 and 15). No engine calls:
// the DLL glue feeds it the clock and the idle verdict and plays whatever it returns. Header-only so the host test builds it directly.
//
// Pool (anim_pool.tsv, 13 target models, all of them have all 7): ambient = pausesh scanning shrug rolleyes touchheart nodyes greeting;
// military = salute (TSF bodies only). Everything else (victory taunt bow point equip itemequip) stays out. Lengths are the measured
// MDL lengths; the DLL re-issues the idle that long after the play (flourish recipe of Mod 10: speed +1.0, flags 0).
// Mix salts: 11 = flourish pick, 12 = first delay, 13 = gap between plays. Salts 1-6 belong to the appearance remap, 7-10 are reserved
// for V3 walkers.
#pragma once

#include "DiversityLogic.h"
#include <stdint.h>
#include <string.h>

namespace NpcDiversity {
namespace Anim {

struct Flourish { const char* name; uint32_t lenMs; };

constexpr Flourish kAmbient[] = {
    {"pausesh", 2000}, {"scanning", 5100}, {"shrug", 1600}, {"rolleyes", 2300},
    {"touchheart", 2800}, {"nodyes", 1500}, {"greeting", 1500},
};
constexpr int kAmbientCount = static_cast<int>(sizeof(kAmbient) / sizeof(kAmbient[0]));
constexpr Flourish kMilitary = {"salute", 1700};

constexpr uint32_t kSaltPick = 11, kSaltFirst = 12, kSaltGap = 13;

constexpr uint32_t kFirstMinMs = 3000, kFirstSpanMs = 12000;     // first flourish 3..15 s after the creature is first seen idle
constexpr uint32_t kGapMinMs = 20000, kGapSpanMs = 25000;        // then every 20..45 s, never less than 20 s (doc 40 section 9.4)

inline const Flourish& Pick(uint32_t hash, bool military) {
    if (military) {                                              // military bodies: salute or one of the ambient set, 1 in 4 salutes
        if (Mix(hash, kSaltPick) % 4u == 0u) return kMilitary;
    }
    return kAmbient[Mix(hash, kSaltPick + 100u) % static_cast<uint32_t>(kAmbientCount)];
}

inline uint32_t FirstDelayMs(uint32_t hash) { return kFirstMinMs + Mix(hash, kSaltFirst) % kFirstSpanMs; }

// n = how many flourishes this creature has played; each gap is a fresh draw.
inline uint32_t GapMs(uint32_t hash, uint32_t n) { return kGapMinMs + Mix(hash + n * 0x85EBCA6Bu, kSaltGap) % kGapSpanMs; }

// One slot per creature that has been seen. Keyed on oid (valid for one visit); the DLL clears the roster on every AREA_LOAD.
struct Slot { uint32_t oid; uint32_t hash; uint32_t nextMs; uint32_t busyUntilMs; uint32_t plays; bool military; bool used; };

constexpr int kMaxSlots = 96;

struct Roster {
    Slot slots[kMaxSlots];
    int count = 0;
    int dropped = 0;                         // creatures that did not fit (logged once by the glue)

    void clear() { count = 0; dropped = 0; for (auto& s : slots) s.used = false; }
    Slot* find(uint32_t oid) { for (int i = 0; i < count; ++i) if (slots[i].oid == oid) return &slots[i]; return nullptr; }
    // Add on first sight. nowMs = the clock when it was added. Returns the slot, or null when full or oid == 0.
    Slot* add(uint32_t oid, uint16_t appearance, uint32_t visitSalt, bool military, uint32_t nowMs) {
        if (oid == 0) return nullptr;
        if (Slot* s = find(oid)) return s;
        if (count >= kMaxSlots) { ++dropped; return nullptr; }
        Slot& s = slots[count++];
        s.oid = oid; s.hash = HashCreatureId(oid, appearance, visitSalt); s.military = military; s.plays = 0; s.used = true;
        s.nextMs = nowMs + FirstDelayMs(s.hash); s.busyUntilMs = nowMs;   // not 0: a 32-bit clock past 2^31 would read 0 as "in the future"
        return &s;
    }
    void remove(uint32_t oid) {
        for (int i = 0; i < count; ++i) if (slots[i].oid == oid) { slots[i] = slots[--count]; slots[count].used = false; return; }
    }
};

// Decision for one slot at nowMs. idle = the glue's verdict (no action, no conversation, no combat, not walking). Not idle pushes the
// next try out so a creature that is busy is retried soon but not every tick. Returns the flourish to play or null. On a play the slot is
// re-armed with its own gap and marked busy for the flourish length (the glue re-issues the idle then).
constexpr uint32_t kBusyRetryMs = 2000;

inline const Flourish* Due(Slot& s, uint32_t nowMs, bool idle) {
    if (static_cast<int32_t>(nowMs - s.busyUntilMs) < 0) return nullptr;           // our own flourish is still running
    if (static_cast<int32_t>(nowMs - s.nextMs) < 0) return nullptr;
    if (!idle) { s.nextMs = nowMs + kBusyRetryMs; return nullptr; }
    const Flourish& f = Pick(Mix(s.hash, s.plays + 1u), s.military);
    s.busyUntilMs = nowMs + f.lenMs;
    s.nextMs = nowMs + f.lenMs + GapMs(s.hash, s.plays);
    ++s.plays;
    return &f;
}

}  // namespace Anim
}  // namespace NpcDiversity
