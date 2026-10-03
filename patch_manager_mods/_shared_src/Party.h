// Party.h -- shared party-table / controlled-creature lookup for KOTOR2 patch DLLs (Steam Aspyr build).
// Walks app -> *(app+4) -> *(a+4) -> *(b+0x270) with saferead probes, then asks the engine for the controlled creature.
// Replaces the per-mod copies of this chain; failure reasons are static strings so callers can dedupe by pointer.
// Also: per-member accessors (member / serverCreature) and HP vitals read through the server creature's vtable (+0x98 max, +0x9C cur, one stack arg each).
// Tests substitute g_appGlobal / g_getControlled / g_member / g_serverCreature / g_callVital. 32-bit engine: all chain links are read as uint32_t.
#pragma once
#include "SafeRead.h"
#include "GameAddr.h"

namespace party {

constexpr unsigned SiteBase = 0x6C00;   // saferead site id for every read here

enum Fail { F_None, F_App, F_App4, F_A4, F_Table, F_Count };

using GetControlledFn = uintptr_t(__thiscall*)(void*);

inline uintptr_t g_appGlobal = gameaddr::AppGlobal;
inline GetControlledFn g_getControlled = reinterpret_cast<GetControlledFn>(gameaddr::GetControlledCreature);

// Party table pointer, or 0 with *why set to the failing link.
inline uintptr_t table(Fail* why = nullptr) {
    auto fail = [&](Fail f) -> uintptr_t { if (why) *why = f; return 0; };
    uint32_t app = 0, a = 0, b = 0, t = 0;
    if (!saferead::readAt<uint32_t>(SiteBase, g_appGlobal, 0, &app) || !app) return fail(F_App);
    if (!saferead::readAt<uint32_t>(SiteBase, app, 4, &a) || !a) return fail(F_App4);
    if (!saferead::readAt<uint32_t>(SiteBase, a, 4, &b) || !b) return fail(F_A4);
    if (!saferead::readAt<uint32_t>(SiteBase, b, 0x270, &t) || !t) return fail(F_Table);
    if (why) *why = F_None;
    return static_cast<uintptr_t>(t);
}

inline int partyCount(uintptr_t table) {
    int n = 0;
    if (table == 0 || !saferead::readAt<int>(SiteBase, table, 0, &n)) return 0;
    return n;
}

// Controlled creature, or 0. *count is written only once the table was found (even when the count is <= 0).
inline uintptr_t controlledCreature(Fail* why = nullptr, int* count = nullptr) {
    Fail local = F_None;
    Fail* w = why ? why : &local;
    uintptr_t t = table(w);
    if (!t) return 0;
    int n = partyCount(t);
    if (count) *count = n;
    if (n <= 0) { *w = F_Count; return 0; }
    *w = F_None;
    return g_getControlled ? g_getControlled(reinterpret_cast<void*>(t)) : 0;
}

inline const char* failText(Fail f) {
    switch (f) {
        case F_App: return "app global null";
        case F_App4: return "app+4 null";
        case F_A4: return "a+4 null";
        case F_Table: return "party table null";
        case F_Count: return "party count 0";
        default: return "ok";
    }
}

// ---- party members + vitals (additive) ----
using MemberFn = uintptr_t(__thiscall*)(void* table, int idx);   // PartyGetAt: thiscall(table, idx) RET 4 -> client creature of position idx (0 = controlled)
inline MemberFn g_member = reinterpret_cast<MemberFn>(gameaddr::PartyGetAt);

// Client creature at party position i, or 0 (null table, i out of [0, count), or no engine fn).
inline uintptr_t member(uintptr_t table, int i) {
    if (table == 0 || i < 0 || i >= partyCount(table) || !g_member) return 0;
    return g_member(reinterpret_cast<void*>(table), i);
}

using ServerCreatureFn = uintptr_t(__thiscall*)(void* member);   // client member -> server creature (CSWSCreature*)
inline ServerCreatureFn g_serverCreature = reinterpret_cast<ServerCreatureFn>(gameaddr::MemberServerCreature);

inline uintptr_t serverCreature(uintptr_t member) {
    if (member == 0 || !g_serverCreature) return 0;
    return g_serverCreature(reinterpret_cast<void*>(member));
}

// KOTOR party members at 0 HP are unconscious/dead; the engine's dead check is never called.
struct Vitals { uintptr_t client = 0, server = 0; int cur = 0, max = 0; bool ok = false; bool down = false; };

// Vtable HP slots: each is thiscall returning short with EXACTLY ONE stack arg (RET 4); a 2-arg call crashed the game (lesson 355).
constexpr int SlotMaxHp = 0x98;   // arg 1
constexpr int SlotCurHp = 0x9C;   // arg 0
using VitalCallFn = int(*)(uintptr_t fn, uintptr_t creature, int arg);
inline int callVitalSlot(uintptr_t fn, uintptr_t c, int arg) {
    return reinterpret_cast<short(__thiscall*)(uintptr_t, int)>(fn)(c, arg);
}
inline VitalCallFn g_callVital = callVitalSlot;

// Fills server/cur/max/ok/down; returns ok. Vtable pointer and both slots are probed and non-zero before any call.
inline bool vitals(uintptr_t server, Vitals* out) {
    Vitals v; v.server = server;
    uint32_t vt = 0, fMax = 0, fCur = 0;
    if (server && g_callVital &&
        saferead::readAt<uint32_t>(SiteBase, server, 0, &vt) && vt &&
        saferead::readAt<uint32_t>(SiteBase, vt, SlotMaxHp, &fMax) && fMax &&
        saferead::readAt<uint32_t>(SiteBase, vt, SlotCurHp, &fCur) && fCur) {
        v.max = g_callVital(fMax, server, 1);
        v.cur = g_callVital(fCur, server, 0);
        v.ok = v.max > 0;
        v.down = v.ok && v.cur <= 0;
    }
    if (out) *out = v;
    return v.ok;
}

// Fills out[0..n) aligned with party position (unreadable members stay ok=false); returns n, 0 on chain failure.
inline int members(Vitals* out, int cap) {
    if (!out || cap <= 0) return 0;
    uintptr_t t = table();
    if (!t) return 0;
    int n = partyCount(t);
    if (n > cap) n = cap;
    if (n <= 0) return 0;
    for (int i = 0; i < n; ++i) {
        out[i] = Vitals();
        uintptr_t c = member(t, i);
        out[i].client = c;
        if (!c) continue;
        uintptr_t s = serverCreature(c);
        if (s) { vitals(s, &out[i]); out[i].client = c; }
    }
    return n;
}

} // namespace party
