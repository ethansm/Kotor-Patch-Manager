// Party.h -- shared party-table / controlled-creature lookup for KOTOR2 patch DLLs (Steam Aspyr build).
// Walks app -> *(app+4) -> *(a+4) -> *(b+0x270) with saferead probes, then asks the engine for the controlled creature.
// Replaces the per-mod copies of this chain; failure reasons are static strings so callers can dedupe by pointer.
// Tests substitute g_appGlobal / g_getControlled. 32-bit engine: all chain links are read as uint32_t.
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

} // namespace party
