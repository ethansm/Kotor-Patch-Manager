// HhDispatch.h -- the hud-host phase dispatcher: per-phase callback lists sorted by (priority, registration order).
// Pure C++ (no Win32, no engine reads), so the order rules are host-tested (hh_dispatch_test.cpp). Used by HudHostProvider.cpp.
#pragma once
#include "HudHost.h"
#include <string.h>

namespace hhd {

constexpr int MaxPerPhase = 16;

struct Entry { int32_t token; int32_t prio; K2HudCallback fn; void* user; const char* owner; };

struct Dispatcher {
    Entry e[K2HUD_PHASE_COUNT][MaxPerPhase] = {};
    int n[K2HUD_PHASE_COUNT] = {};
    int32_t nextToken = 1;

    // Inserted after every entry with prio <= p (stable: equal priorities keep registration order). Token > 0, or
    // -1 bad phase / null fn, -2 phase full.
    int32_t add(int32_t phase, int32_t prio, K2HudCallback fn, void* user, const char* owner) {
        if (phase < 0 || phase >= K2HUD_PHASE_COUNT || !fn) return -1;
        int& cnt = n[phase];
        if (cnt >= MaxPerPhase) return -2;
        int at = cnt;
        while (at > 0 && e[phase][at - 1].prio > prio) { e[phase][at] = e[phase][at - 1]; --at; }
        e[phase][at] = { nextToken, prio, fn, user, owner ? owner : "" };
        ++cnt;
        return nextToken++;
    }
    bool remove(int32_t token) {
        for (int p = 0; p < K2HUD_PHASE_COUNT; ++p)
            for (int i = 0; i < n[p]; ++i)
                if (e[p][i].token == token) {
                    for (int k = i + 1; k < n[p]; ++k) e[p][k - 1] = e[p][k];
                    --n[p];
                    return true;
                }
        return false;
    }
    bool empty(int32_t phase) const { return phase < 0 || phase >= K2HUD_PHASE_COUNT || n[phase] == 0; }
    // Calls every callback of ctx.phase in order. Iterates a snapshot, so a callback that (un)registers does not skip or repeat others.
    void run(const K2HudContext& ctx) const {
        if (empty(ctx.phase)) return;
        Entry snap[MaxPerPhase];
        const int cnt = n[ctx.phase];
        memcpy(snap, e[ctx.phase], sizeof(Entry) * cnt);
        for (int i = 0; i < cnt; ++i) snap[i].fn(&ctx, snap[i].user);
    }
};

constexpr int32_t MenuToHud = 0xCC;   // menu = hud + 0xCC (CSWGuiTargetActionMenu inside the HUD object)

// Builds the PopulateMenus context from the hook parameters. targetTypeRaw = the int8 read at menu+0x1BAA, or any value < 0 / > 0x7F
// when the read failed or the engine holds 0xFF: both mean "none" (-1).
inline K2HudContext makeContext(int32_t menu, int32_t leader, int32_t forceUnusable, int32_t target, int32_t targetTypeRaw, uint32_t frame) {
    K2HudContext c = {};
    c.struct_size = sizeof(c);
    c.phase = K2HUD_FRAME;
    c.frame = frame;
    c.menu = menu;
    c.hud = menu ? menu - MenuToHud : 0;
    c.leader = leader;
    c.forceUnusable = forceUnusable;
    c.target = target;
    c.targetType = (targetTypeRaw < 0 || targetTypeRaw > 0x7F) ? -1 : targetTypeRaw;
    c.panel = 0;
    return c;
}

// PopulateMenus dispatch order: FRAME, then TARGET_MENU_PRE. ctx comes from makeContext (frame already incremented by the caller).
inline void runPopulate(const Dispatcher& d, K2HudContext& ctx) {
    ctx.phase = K2HUD_FRAME;           d.run(ctx);
    ctx.phase = K2HUD_TARGET_MENU_PRE; d.run(ctx);
}

} // namespace hhd
