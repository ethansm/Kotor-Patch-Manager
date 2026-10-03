// PsDispatch.h -- the portrait-slots phase dispatcher: per-phase callback lists sorted by (priority, registration order).
// Pure C++ (no Win32), so the order rules are host-tested (ps_dispatch_test.cpp). Used by PortraitSlotsProvider.cpp.
#pragma once
#include "PortraitSlots.h"
#include <string.h>

namespace psd {

constexpr int MaxPerPhase = 16;

// The address-table fields the menu overlay needs (template over K2AddrTable so this header stays free of K2Addr.h): the menu's
// top-bar portrait slot offset, LBL_CHAR2 / LBL_CHAR3, and the caller-frame local holding the selected creature. Null table or any
// field 0 (unknown) = the menu overlay must do nothing.
template <class Table> inline bool menuFieldsOk(const Table* t) {
    return t && t->Menu_PortraitSlot && t->Menu_Char2 && t->Menu_Char3 && t->MenuCaller_Creature;
}

struct Entry { int32_t token; int32_t prio; K2PsCallback fn; void* user; const char* owner; };

struct Dispatcher {
    Entry e[K2PS_PHASE_COUNT][MaxPerPhase] = {};
    int n[K2PS_PHASE_COUNT] = {};
    int32_t nextToken = 1;

    // Inserted after every entry with prio <= p (stable: equal priorities keep registration order). Token > 0, or
    // -1 bad phase / null fn, -2 phase full.
    int32_t add(int32_t phase, int32_t prio, K2PsCallback fn, void* user, const char* owner) {
        if (phase < 0 || phase >= K2PS_PHASE_COUNT || !fn) return -1;
        int& cnt = n[phase];
        if (cnt >= MaxPerPhase) return -2;
        int at = cnt;
        while (at > 0 && e[phase][at - 1].prio > prio) { e[phase][at] = e[phase][at - 1]; --at; }
        e[phase][at] = { nextToken, prio, fn, user, owner ? owner : "" };
        ++cnt;
        return nextToken++;
    }
    bool remove(int32_t token) {
        for (int p = 0; p < K2PS_PHASE_COUNT; ++p)
            for (int i = 0; i < n[p]; ++i)
                if (e[p][i].token == token) {
                    for (int k = i + 1; k < n[p]; ++k) e[p][k - 1] = e[p][k];
                    --n[p];
                    return true;
                }
        return false;
    }
    bool empty(int32_t phase) const { return phase < 0 || phase >= K2PS_PHASE_COUNT || n[phase] == 0; }
    // Calls every callback of ctx.phase in order. Iterates a snapshot, so a callback that (un)registers does not skip or repeat others.
    void run(const K2PsContext& ctx) const {
        if (empty(ctx.phase)) return;
        Entry snap[MaxPerPhase];
        const int cnt = n[ctx.phase];
        memcpy(snap, e[ctx.phase], sizeof(Entry) * cnt);
        for (int i = 0; i < cnt; ++i) snap[i].fn(&ctx, snap[i].user);
    }
};

// Hook 6's phase sequence (the order BuffDurationHUD v1.1.0 ran its work in, doc 21):
//   PRE_ENGINE_READ (every slot) -> [leader: frame++, FRAME_BEGIN] -> SLOT (every slot) -> leader only: menu frame ? menu(ctx) : HUD_DRAW.
// menu(ctx) is the provider's MENU_OVERLAY wrapper (reopens the 2D batch); it is only called when that phase has callbacks.
// ctx: slot / creature / isLarge / callerEbp / inMenu filled by the caller. Returns the new frame counter.
template <typename MenuFn>
inline uint32_t runSlotDraw(const Dispatcher& d, K2PsContext& ctx, uint32_t frame, MenuFn menu) {
    ctx.frame = frame;
    ctx.phase = K2PS_PRE_ENGINE_READ; d.run(ctx);
    if (ctx.isLarge) {
        ctx.frame = ++frame;
        ctx.phase = K2PS_FRAME_BEGIN; d.run(ctx);
    }
    ctx.phase = K2PS_SLOT; d.run(ctx);
    if (!ctx.isLarge) return frame;
    if (ctx.inMenu) { if (!d.empty(K2PS_MENU_OVERLAY)) menu(ctx); return frame; }
    ctx.phase = K2PS_HUD_DRAW; d.run(ctx);
    return frame;
}

} // namespace psd
