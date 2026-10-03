// portrait-slots (Steam Aspyr build 6A522E71...): neutral owner of the party-portrait hooks, provider of "k2.portraitslots" v1.
// It draws nothing and changes no engine state by itself; consumers (buff-duration-hud, shield-bar, ...) register phase callbacks
// through the registry (PortraitSlots.h). Design, phases, priorities and install rules: patch_manager_mods/21_portrait_slots_and_split.md.
// The hook bodies below are the shared part of BuffDurationHUD.cpp v1.1.0 (commit 17b508d), in the same order.
//
// Hooks (cdecl detours, original bytes still run):
//   1a/1b  0x0074F25A / 0x0074F8AB  UpdatePortraits: slot -> creature (EAX/EDX = slot, [ebp-0x38] = creature)
//   6      0x00745774  FUN_00745770 (per-slot icon-row gate) after its prologue: ECX = slot, [ebp+0xc] = isLarge, [ebp] = caller frame.
//                      Runs every frame for every slot BEFORE the engine reads the arrow counters slot+0xddc/+0xddd.
// Engine offsets (menu panel layout, the caller-frame creature local) come from k2addr::get() (K2Addr.h). A null table (unknown exe) or a
// zero field makes that feature do nothing: no table = no snapshot / dispatch at all, no menu fields = no menu overlay. One WARN per session.
// Logging: kpatch.log channel "portrait-slots/core" (WARN always; INFO when kplog.ini enables it).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>

#include "kpatch_api.h"
#include "SafeRead.h"
#include "K2Addr.h"
#include "Effects.h"
#include "Gui2D.h"
#include "Party.h"
#include "PortraitSlots.h"
#include "PsDispatch.h"

namespace {

// ---- slot -> creature table (from BuffDurationHUD.cpp 87-101) -----------------------------------------------------------------
struct SlotEntry { int slotPtr; int creaturePtr; DWORD tick; };   // tick = when SnapshotPortraitSlot last refreshed it
SlotEntry g_slots[K2PS_MAX_SLOTS];
SlotEntry g_menuSlot;   // the menu top bar's portrait slot (menu+0x2350) -> selected creature, refreshed every menu frame

// UpdatePortraits refreshes every live HUD slot each frame, so an entry older than this is stale (area load / HUD rebuilt): its
// creature pointer may be freed memory.
constexpr DWORD SlotFreshMs = 500;
inline bool slotFresh(const SlotEntry& e) { return GetTickCount() - e.tick <= SlotFreshMs; }

int creatureForSlot(int slotPtr) {
    if (g_menuSlot.slotPtr == slotPtr) return g_menuSlot.creaturePtr;
    for (auto& e : g_slots) if (e.slotPtr == slotPtr) return slotFresh(e) ? e.creaturePtr : 0;
    return 0;
}

psd::Dispatcher g_disp;
uint32_t g_frame = 0;

// ---- kpatch.log -----------------------------------------------------------------------------------------------------------------
const KPatchLogApi* g_log = nullptr;
int32_t g_logCh = -1;
void logf(int level, const char* fmt, ...) {
    if (!g_log || g_logCh < 0 || !g_log->Enabled(g_logCh, level)) return;
    char buf[256];
    va_list ap; va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n < 0) return;
    if (n >= int(sizeof(buf))) n = int(sizeof(buf)) - 1;
    g_log->Submit(g_logCh, level, buf, uint32_t(n));
}

// One WARN per DLL session for the first feature that finds the address table unusable (never from KPatchInit).
void noteTableProblem(const char* what) {
    static bool once = false;
    if (once) return;
    once = true;
    logf(KPLOG_WARN, "k2addr: %s unavailable (no address table for this exe, or a needed field is 0): feature disabled", what);
}

// ---- the interface --------------------------------------------------------------------------------------------------------------
int32_t KPATCH_CALL apiRegister(int32_t phase, int32_t prio, K2PsCallback fn, void* user, const char* owner) {
    int32_t t = g_disp.add(phase, prio, fn, user, owner);
    logf(t > 0 ? KPLOG_INFO : KPLOG_WARN, "register phase=%d prio=%d owner=%s -> %d", phase, prio, owner ? owner : "?", t);
    return t;
}
void KPATCH_CALL apiUnregister(int32_t token) {
    bool ok = g_disp.remove(token);
    logf(KPLOG_INFO, "unregister %d -> %d", token, ok ? 1 : 0);
}
int32_t KPATCH_CALL apiCreatureForSlot(int32_t slot) { return creatureForSlot(slot); }
int32_t KPATCH_CALL apiSlotAt(int32_t i, int32_t* slot, int32_t* creature) {
    if (i < 0 || i >= K2PS_MAX_SLOTS) return 0;
    const SlotEntry& e = g_slots[i];
    if (!e.slotPtr || !e.creaturePtr || !slotFresh(e)) return 0;
    if (slot) *slot = e.slotPtr;
    if (creature) *creature = e.creaturePtr;
    return 1;
}
uint32_t KPATCH_CALL apiFrame() { return g_frame; }

const K2PortraitSlotsApi g_api = { sizeof(K2PortraitSlotsApi), apiRegister, apiUnregister, apiCreatureForSlot, apiSlotAt, apiFrame };

// ---- menu frame context ---------------------------------------------------------------------------------------------------------
// CSWGuiInGameMenu (the top bar shared by every in-game menu tab) draws its panel (BeginGui2D, panel clip, controls, pop, EndGui2D)
// and THEN calls FUN_00745770 with ECX = its portrait slot (menu+0x2350): the 2D batch is closed there, so MENU_OVERLAY reopens it.
// Selected creature = the caller's local [callerEbp-0x38]. Companions = party positions 1 / 2 (the menu fills LBL_CHAR2/3 from them).
// Offsets: K2AddrTable Menu_PortraitSlot / Menu_Char2 / Menu_Char3 / MenuCaller_Creature (psd::menuFieldsOk gates them).

int rosterCreature(int idx) {
    uintptr_t c = party::serverCreature(party::member(party::table(), idx));
    return k2fx::plausibleCreature(c) ? int(c) : 0;
}

void runMenuOverlay(K2PsContext& ctx) {
    const K2AddrTable* T = k2addr::get();
    if (!psd::menuFieldsOk(T)) { noteTableProblem("menu overlay"); return; }
    ctx.phase = K2PS_MENU_OVERLAY;
    ctx.panel = ctx.slot - T->Menu_PortraitSlot;
    ctx.menuCreature = g_menuSlot.slotPtr == ctx.slot ? g_menuSlot.creaturePtr : 0;
    ctx.companions[0] = rosterCreature(1);
    ctx.companions[1] = rosterCreature(2);
    ctx.companionCtrl[0] = ctx.panel + T->Menu_Char2;
    ctx.companionCtrl[1] = ctx.panel + T->Menu_Char3;
    if (!gui2d::panelRect(uintptr_t(ctx.panel), ctx.panelRect)) {
        static int lastBad = 0; int sig = ctx.panelRect[2] * 31 + ctx.panelRect[3];
        if (sig != lastBad) { lastBad = sig; logf(KPLOG_WARN, "menu: bad panel rect %d,%d,%d,%d (panel %08x), overlay skipped",
                                                 ctx.panelRect[0], ctx.panelRect[1], ctx.panelRect[2], ctx.panelRect[3], ctx.panel); }
        return;
    }
    gui2d::CapGuard capGuard;   // our flushes / colour clips toggle GL caps: put them back after EndGui2D
    gui2d::Gui2DScope batch;
    gui2d::ClipScope clip(ctx.panelRect[0], ctx.panelRect[1], ctx.panelRect[2], ctx.panelRect[3]);
    if (clip) g_disp.run(ctx);
}

} // namespace

extern "C" {

void __cdecl KPatchInit(const KPatchApi* api) {
    if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->provide) return;
    api->provide(K2PS_IFACE, K2PS_VERSION, &g_api);
    if (api->require) {
        const KPatchLogApi* lg = static_cast<const KPatchLogApi*>(api->require(KPATCH_LOG_IFACE, KPATCH_LOG_VERSION));
        if (lg && lg->struct_size >= offsetof(KPatchLogApi, Submit) + sizeof(lg->Submit) && lg->Register && lg->Channel && lg->Enabled) {
            int32_t p = lg->Register("portrait-slots");
            g_logCh = p >= 0 ? lg->Channel(p, "core") : -1;
            g_log = lg;
        }
    }
    logf(KPLOG_INFO, "provided %s v%d", K2PS_IFACE, K2PS_VERSION);
}

void __cdecl PS_SnapshotPortraitSlot(int slotPtr, int creaturePtr) {
    saferead::beginScope();                     // engine code between hook calls can free memory: drop the cached readable regions
    if (!k2addr::get()) { noteTableProblem("slot snapshot"); return; }
    if (!g_disp.empty(K2PS_SNAPSHOT)) {
        K2PsContext ctx = {};
        ctx.struct_size = sizeof(ctx); ctx.phase = K2PS_SNAPSHOT; ctx.slot = slotPtr; ctx.creature = creaturePtr; ctx.frame = g_frame;
        g_disp.run(ctx);
    }
    const DWORD now = GetTickCount();
    for (auto& e : g_slots) if (e.slotPtr == slotPtr) { e.creaturePtr = creaturePtr; e.tick = now; return; }
    for (auto& e : g_slots) if (e.slotPtr == 0) { e = { slotPtr, creaturePtr, now }; return; }
    static int evict = 0; g_slots[evict++ & 3] = { slotPtr, creaturePtr, now };   // HUD rebuilt (area load): replace a stale entry
}

void __cdecl PS_PortraitSlotDraw(int slotPtr, int isLarge, int callerEbp) {
    saferead::beginScope();
    const K2AddrTable* T = k2addr::get();
    if (!T) { noteTableProblem("portrait slot dispatch"); return; }
    const bool inMenu = gui2d::inMenuFrame();
    // menu frame: map the menu portrait slot to its creature so the consumers' hooks (which run later in 0x745770) know it
    if (inMenu && isLarge && psd::menuFieldsOk(T)) {
        int c = 0; k2fx::rd(callerEbp, T->MenuCaller_Creature, &c);
        g_menuSlot = k2fx::plausibleCreature(c) ? SlotEntry{ slotPtr, c, GetTickCount() } : SlotEntry{ 0, 0, 0 };
    } else if (g_menuSlot.slotPtr == slotPtr) g_menuSlot = { 0, 0, 0 };

    K2PsContext ctx = {};
    ctx.struct_size = sizeof(ctx); ctx.slot = slotPtr; ctx.isLarge = isLarge; ctx.callerEbp = callerEbp; ctx.inMenu = inMenu ? 1 : 0;
    ctx.creature = creatureForSlot(slotPtr);
    // PRE_ENGINE_READ runs before FUN_00745770 reads +0xddc/+0xddd (every slot). Everything that draws for all cards runs from the
    // LEADER's call (the first slot call of the frame): at later slots an earlier slot's icon row has already leaked GUI clip/batch
    // state (doc 08 session 4c).
    g_frame = psd::runSlotDraw(g_disp, ctx, g_frame, runMenuOverlay);
}

} // extern "C"
