// Effects.h -- creature active-effect list walk + CGameEffect decode for KOTOR2 Patch Manager mods (Steam Aspyr, i686).
// Header-only, no STL, every engine read through saferead. Extracted from BuffDurationHUD.cpp (Mod 2 + Mod 4, commit 17b508d)
// for the buff / shield split (doc 21); survey doc 19b A4 "k2.effects". Include as "../_shared/Effects.h".
//
// Engine facts (doc 02 + doc 08 "Session 2 decode"):
//   creature (CSWSObject) +0x148 CGameEffect** array, +0x14c int count (sane: 0..512).
//   CGameEffect (non-virtual): +0x08 ushort type, +0x0a ushort subtype, +0x0c float duration (s), +0x10 int expiry day,
//   +0x14 int expiry ms-of-day, +0x18 creator, +0x1c spells.2da row, +0x30 int count of ints, +0x34 int* ints.
//   Shield (EffectForceShield): type 0x02 with subtype bit 8 = absorb pool, ints [flags, resist, REMAINING, vuln, 1]
//   (remaining is decremented in place by FUN_00541fd0); the sibling type 0x1e effect with the same spell id has
//   ints[0] = visualeffects.2da row (colour key).
//   World time: FUN_0051ad20 thiscall(timer, int* day, uint* ms) RET 8, timer = *(*(*(*(0xA1B4A4)+8)+4)+0x10048).
// Host tests: define K2FX_HOST to replace the engine time call through k2fx::g_fx (see effects_test.cpp).
#pragma once
#include "SafeRead.h"
#include <stdint.h>

namespace k2fx {

constexpr int Effect_Type = 0x08, Effect_Subtype = 0x0a;     // ushort
constexpr int Effect_Duration = 0x0c;                        // float, total seconds
constexpr int Effect_ExpiryDay = 0x10, Effect_ExpiryTime = 0x14;
constexpr int Effect_Creator = 0x18;
constexpr int Effect_SpellId = 0x1c;
constexpr int Effect_IntCount = 0x30, Effect_Ints = 0x34;
constexpr int Creature_EffectArray = 0x148, Creature_EffectCount = 0x14c;
constexpr int MaxEffects = 512;
constexpr int MsPerDay = 86400000;
constexpr int EffType_Pool = 0x02, EffSub_PoolBit = 8, EffType_Visual = 0x1e;
constexpr uintptr_t AddrAppGlobal = 0x00A1B4A4;              // = gameaddr::AppGlobal
constexpr uintptr_t AddrWorldTimerGetCurrentTime = 0x0051AD20;

struct Engine {
    // thiscall(timer, int* day, int* ms) RET 8; a host test substitutes a fake
    void (*getTime)(uintptr_t timer, int* day, int* ms) = [](uintptr_t timer, int* day, int* ms) {
        reinterpret_cast<void(__thiscall*)(void*, int*, int*)>(AddrWorldTimerGetCurrentTime)(reinterpret_cast<void*>(timer), day, ms);
    };
    uintptr_t appGlobal = AddrAppGlobal;
};
inline Engine g_fx;

template <typename T> inline bool rd(uintptr_t base, int off, T* out) {
    return saferead::readAt(1, base, off, out);
}

// The effect array + count of a creature, raw (no range check). False = unreadable.
inline bool readEffectList(uintptr_t creature, int* arr, int* count) {
    return rd(creature, Creature_EffectArray, arr) && rd(creature, Creature_EffectCount, count);
}
// Non-empty and sane (1..MaxEffects): what every per-effect scan wants.
inline bool effectListNonEmpty(uintptr_t creature, int* arr, int* count) {
    return readEffectList(creature, arr, count) && *arr != 0 && *count > 0 && *count <= MaxEffects;
}
// Cheap "looks like a live creature" check used before trusting a creature pointer read from a caller frame or the roster.
inline bool plausibleCreature(uintptr_t c) {
    int cnt = 0, arr = 0;
    return c && rd(c, Creature_EffectCount, &cnt) && rd(c, Creature_EffectArray, &arr) && cnt >= 0 && cnt <= MaxEffects;
}

// Current world time (day, ms of day); false when no world timer (main menu, load).
inline bool currentTime(int* day, int* ms) {
    int g, a, b;
    if (!rd(g_fx.appGlobal, 0, &g) || !rd(g, 8, &a) || !rd(a, 4, &b)) return false;
    int timer;                                  // FUN_0051c370: *(*(a+4)+0x10048)
    if (!rd(b, 0x10048, &timer) || timer == 0) return false;
    g_fx.getTime(static_cast<uintptr_t>(timer), day, ms);
    return true;
}

// Fraction of the effect's duration still left at (day, ms), clamped to [minRatio, 1].
inline float remainingRatio(int eDay, int eMs, float duration, int day, int ms, float minRatio = 0.0f) {
    double remaining = double(eDay - day) * MsPerDay + double(eMs - ms);
    float ratio = float(remaining / (double(duration) * 1000.0));
    if (ratio > 1.0f) ratio = 1.0f;
    if (ratio < minRatio) ratio = minRatio;
    return ratio;
}
// remainingRatio for one effect; 1.0 when it has no duration (permanent) or the time is unknown.
inline float effectRatio(uintptr_t eff, int day, int ms, bool haveTime, float minRatio = 0.0f) {
    float duration; int eDay, eMs;
    if (haveTime && rd(eff, Effect_Duration, &duration) && duration > 0.0f &&
        rd(eff, Effect_ExpiryDay, &eDay) && rd(eff, Effect_ExpiryTime, &eMs))
        return remainingRatio(eDay, eMs, duration, day, ms, minRatio);
    return 1.0f;
}

// spells.2da rows that are ITEM abilities (stims, shots, shields, cloak); force powers are not.
inline bool isItemAbility(int spell) {
    return (spell >= 69 && spell <= 77) || (spell >= 99 && spell <= 108) || (spell >= 110 && spell <= 115) || spell == 129 || spell == 132 || spell == 257;
}

// One shield absorb pool as stored on the creature (no max tracking, no colour: those are the shield bar's policy).
struct PoolRaw { int eff; int spell; int flags; int remaining; int vfx; };

// Every live shield pool (type 0x02, subtype bit 8, >= 3 ints, remaining > 0), in effect-list order. vfx = ints[0] of the
// type 0x1e effect with the same spell id (0 if none). Returns the count written.
inline int scanShieldPools(uintptr_t creature, PoolRaw* out, int maxOut) {
    int arr = 0, count = 0;
    if (!effectListNonEmpty(creature, &arr, &count)) return 0;
    int n = 0;
    for (int i = 0; i < count && n < maxOut; ++i) {
        int eff = 0; unsigned short type = 0, sub = 0;
        if (!rd(arr, i * 4, &eff) || !eff || !rd(eff, Effect_Type, &type) || !rd(eff, Effect_Subtype, &sub)) continue;
        if (type != EffType_Pool || !(sub & EffSub_PoolBit)) continue;
        int ip = 0, cnt = 0, spell = 0;
        if (!rd(eff, Effect_IntCount, &cnt) || cnt < 3 || !rd(eff, Effect_Ints, &ip) || !ip || !rd(eff, Effect_SpellId, &spell)) continue;
        int flags = 0, remaining = 0;
        if (!rd(ip, 0, &flags) || !rd(ip, 8, &remaining) || remaining <= 0) continue;
        int vfx = 0;
        for (int k = 0; k < count; ++k) {
            int e2 = 0; unsigned short t2 = 0; int sp2 = 0, ip2 = 0;
            if (!rd(arr, k * 4, &e2) || !e2 || !rd(e2, Effect_Type, &t2) || t2 != EffType_Visual) continue;
            if (!rd(e2, Effect_SpellId, &sp2) || sp2 != spell || !rd(e2, Effect_Ints, &ip2) || !ip2) continue;
            rd(ip2, 0, &vfx); break;
        }
        out[n++] = { eff, spell, flags, remaining, vfx };
    }
    return n;
}

} // namespace k2fx
