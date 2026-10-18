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
struct Slot { uint32_t oid; uint32_t hash; uint32_t nextMs; uint32_t busyUntilMs; uint32_t plays; uint16_t app; bool military; bool used; bool probed; bool playable; };

constexpr int kMaxSlots = 96;

struct Roster {
    Slot slots[kMaxSlots];
    int count = 0;
    int dropped = 0;                         // creatures that did not fit (logged once by the glue)

    void clear() { count = 0; dropped = 0; for (auto& s : slots) s.used = false; }
    Slot* find(uint32_t oid) { for (int i = 0; i < count; ++i) if (slots[i].oid == oid) return &slots[i]; return nullptr; }
    // Add on first sight. nowMs = the clock when it was added. Returns the slot, or null when full or oid == 0.
    // playable = the row's animation set was checked (EligibleRow); a non-playable slot is only probed (logged), never played.
    Slot* add(uint32_t oid, uint16_t appearance, uint32_t visitSalt, bool military, uint32_t nowMs, bool playable = true) {
        if (oid == 0) return nullptr;
        if (Slot* s = find(oid)) return s;
        if (count >= kMaxSlots) { ++dropped; return nullptr; }
        Slot& s = slots[count++];
        s.oid = oid; s.hash = HashCreatureId(oid, appearance, visitSalt); s.military = military; s.plays = 0; s.app = appearance; s.used = true; s.probed = false; s.playable = playable;
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

// True when the slot may be looked at now (not inside our own flourish, past its next time). The glue resolves the engine chain only then.
inline bool Ready(const Slot& s, uint32_t nowMs) {
    return static_cast<int32_t>(nowMs - s.busyUntilMs) >= 0 && static_cast<int32_t>(nowMs - s.nextMs) >= 0;
}

// armed = the creature holds a weapon right now (user 10-18: only armed NPCs flourish). Unarmed or unverified rows are looked at again
// only every kUnarmedRetryMs, so a civilian costs one chain resolve per 30 s.
constexpr uint32_t kUnarmedRetryMs = 30000;

inline const Flourish* Due(Slot& s, uint32_t nowMs, bool idle, bool armed = true) {
    if (!Ready(s, nowMs)) return nullptr;
    if (!armed || !s.playable) { s.nextMs = nowMs + kUnarmedRetryMs; return nullptr; }
    if (!idle) { s.nextMs = nowMs + kBusyRetryMs; return nullptr; }
    const Flourish& f = Pick(Mix(s.hash, s.plays + 1u), s.military);
    s.busyUntilMs = nowMs + f.lenMs;
    s.nextMs = nowMs + f.lenMs + GapMs(s.hash, s.plays);
    ++s.plays;
    return &f;
}

// ---- eligibility and idle test (pure; the engine glue only reads the facts) --------------------------------------------------------------
// Only ARMED creatures flourish (user 10-18: unarmed civilians do not need it; Telos security with guns do). Two gates:
//   1. EligibleRow = the row's animation set was checked: 479 = N_RepSold and 485 = N_RepOff (TSF bodies, all 7 ambient flourishes + salute in
//      anim_pool.tsv). Armor-body rows build PMB/PFB meshes, so the descriptor subtype is NOT filtered. Other armed rows (Czerka guards,
//      mercenaries) stay out until their animation sets are checked.
//   2. HasWeapon = the server creature's right-hand inventory slot holds an item (nb2_weapon_recon.md: [[server+0x1150]+0x14] = object id).
// The old 11-row civilian list ({12,18,24,39,47,185,190,569,601} + these two) is gone; the civilian rows are kept in doc 40 section 15.4.
constexpr uint16_t kArmedRows[] = {479, 485};
inline bool EligibleRow(uint16_t appearance, bool* military = nullptr) {
    if (military) *military = false;
    for (uint16_t r : kArmedRows) if (r == appearance) { if (military) *military = true; return true; }
    return false;
}

// Server creature facts (recon nb2_recon.md): +0x08 object type (5 creature), +0x11AC party flag, +0x10E8 is-PC, +0x84 current action word
// (0xFFFF none), +0x100 action list head (LOGGED ONLY: Run A2 10-18 showed it non-null on idle creatures, so it is not an idle test), +0x520 combat state, +0x54 dialog partner oid (0x7F000000 or 0 = none, unverified).
struct ServerFacts { uint8_t type; int32_t party; int32_t isPc; uint16_t action; uint32_t queueHead; uint32_t combat; uint32_t dialog; uint32_t rightHand; };
constexpr uint32_t kObjInvalid = 0x7F000000u;
// rightHand = object id in the creature's right-hand slot (0 or kObjInvalid = empty). Unverified live that empty is 0 vs 0x7F000000: both count as unarmed.
inline bool HasWeapon(const ServerFacts& f) { return f.rightHand != 0 && f.rightHand != kObjInvalid; }
inline bool IsIdle(const ServerFacts& f) {
    return f.type == 5 && f.party == 0 && f.isPc == 0 && f.action == 0xFFFFu && f.combat == 0 &&
           (f.dialog == 0 || f.dialog == kObjInvalid);
}

}  // namespace Anim
}  // namespace NpcDiversity
