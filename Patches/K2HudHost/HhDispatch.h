// HhDispatch.h -- the hud-host phase dispatcher: per-phase callback lists sorted by (priority, registration order).
// Pure C++ (no Win32, no engine reads), so the order rules are host-tested (hh_dispatch_test.cpp). Used by HudHostProvider.cpp.
#pragma once
#include "../Common/HudHost.h"
#include <string.h>

namespace hhd {

constexpr int MaxPerPhase = 16;

// Optional per-entry observer: called right after each consumer returns, with that consumer's owner name (diagnostics only).
typedef void (*EntryObserver)(const char* owner, void* user);

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
    // obs (default null = identical behaviour) is called once after each callback, in order, with that entry's owner.
    void run(const K2HudContext& ctx, EntryObserver obs = nullptr, void* obsUser = nullptr) const {
        if (empty(ctx.phase)) return;
        Entry snap[MaxPerPhase];
        const int cnt = n[ctx.phase];
        memcpy(snap, e[ctx.phase], sizeof(Entry) * cnt);
        for (int i = 0; i < cnt; ++i) {
            snap[i].fn(&ctx, snap[i].user);
            if (obs) obs(snap[i].owner, obsUser);
        }
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

// ---- BUILD (0x0040F5A0, the panel layout-closing function; the layout is still open while it runs) ----
// Fires only for the HUD panel: the hook runs for every panel ctor, so the vtable decides.
inline bool isHudPanel(uint32_t vtbl, uint32_t hudVtbl) { return vtbl != 0 && vtbl == hudVtbl; }

// BUILD context: panel == hud == the HUD object; no menu / leader / target.
inline K2HudContext makeBuildContext(int32_t panel, uint32_t frame) {
    K2HudContext c = {};
    c.struct_size = sizeof(c);
    c.phase = K2HUD_BUILD;
    c.frame = frame;
    c.panel = panel;
    c.hud = panel;
    c.menu = 0;
    c.targetType = -1;
    return c;
}

// PANEL_BUILD context (hud-host 0.6.0+): any panel's layout load. panel + its vtable; hud = the last HUD seen (0 = none yet).
inline K2HudContext makePanelBuildContext(int32_t panel, uint32_t vtbl, int32_t hud, uint32_t frame) {
    K2HudContext c = {};
    c.struct_size = sizeof(c);
    c.phase = K2HUD_PANEL_BUILD;
    c.frame = frame;
    c.panel = panel;
    c.panelVtbl = vtbl;
    c.hud = hud;
    c.menu = 0;
    c.targetType = -1;
    return c;
}

// Dispatches K2HUD_PANEL_BUILD only. Runs for every panel, HUD included (after its K2HUD_BUILD); consumers filter on ctx.panelVtbl.
inline void runPanelBuild(const Dispatcher& d, K2HudContext& ctx) {
    ctx.phase = K2HUD_PANEL_BUILD;
    d.run(ctx);
}

// Dispatches K2HUD_BUILD only (never FRAME / PRE / POST). ctx comes from makeBuildContext.
// obs (optional, diagnostics) is called after each BUILD consumer with its owner name.
inline void runBuild(const Dispatcher& d, K2HudContext& ctx, EntryObserver obs = nullptr, void* obsUser = nullptr) {
    ctx.phase = K2HUD_BUILD;
    d.run(ctx, obs, obsUser);
}

// ---- TARGET_MENU_POST (0x00743847 PopulateMenus tail since hud-host 0.4.0; was the 0x00743850 entry) ----
// POST only fires for a PopulateMenus call whose PRE was seen: PreState remembers the last PRE; runPost consumes it. Rules:
//   - no PRE seen yet, or the PRE was already consumed by an earlier POST (second POST in the same frame) -> skipped;
//   - POST menu != PRE menu -> skipped (a PRE stays pending, so the matching POST can still fire);
//   - otherwise ctx = PRE ctx with phase POST, frame reused (no increment), leader/target from the POST call.
enum PostResult { PostRan = 0, PostNoPre = 1, PostMenuMismatch = 2, PostAlreadyRan = 3 };

struct PreState {
    K2HudContext ctx = {};
    bool seen = false;       // a PRE was recorded
    bool pending = false;    // ... and no POST has consumed it yet
};

inline void recordPre(PreState& st, const K2HudContext& preCtx) {
    st.ctx = preCtx; st.seen = true; st.pending = true;
}

inline PostResult runPost(const Dispatcher& d, PreState& st, int32_t menu, int32_t leader, int32_t target, int32_t targetTypeRaw) {
    if (!st.seen) return PostNoPre;
    if (!st.pending) return PostAlreadyRan;
    if (menu != st.ctx.menu) return PostMenuMismatch;
    st.pending = false;
    K2HudContext c = st.ctx;
    c.phase = K2HUD_TARGET_MENU_POST;
    c.leader = leader;
    c.target = target;
    c.targetType = (targetTypeRaw < 0 || targetTypeRaw > 0x7F) ? -1 : targetTypeRaw;
    d.run(c);
    return PostRan;
}

} // namespace hhd
