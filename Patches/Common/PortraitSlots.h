/* PortraitSlots.h -- the "k2.portraitslots" v1 registry interface (provider: the portrait-slots patch) + a C++ consumer helper.
 *
 * Why: the party-portrait hooks (UpdatePortraits 0x74F25A / 0x74F8AB and the per-slot draw gate 0x745774) are needed by more than
 * one mod (Buff Duration HUD, Shield Bar, ...), but the KPM installer rejects two patches hooking one address. portrait-slots owns
 * those three hooks, keeps the slot -> creature table, and calls registered consumers at fixed points ("phases") of each hook, in
 * priority order. Design + install rules: patch_manager_mods/21_portrait_slots_and_split.md.
 *
 * Plain C ABI (kpatch_api.h rules): fixed-width ints and function pointers only, KPATCH_CALL (cdecl), structs only grow,
 * check struct_size before reading a field newer than your oldest supported provider.
 *
 * Phases (all on the game thread, inside the hooked engine function):
 *   K2PS_SNAPSHOT         hooks 1a/1b, before the table update.          ctx: slot, creature (the engine's, may be 0)
 *   K2PS_PRE_ENGINE_READ  hook 6 entry, EVERY slot, before FUN_00745770 reads slot+0xddc/+0xddd (arrow counters).
 *   K2PS_FRAME_BEGIN      hook 6, leader call only, once per frame, after the frame counter advanced (flag polling, hotkeys).
 *   K2PS_SLOT             hook 6, every slot, after FRAME_BEGIN.            (per-slot layout work, e.g. moving stock bars)
 *   K2PS_MENU_OVERLAY     hook 6, leader (top-bar) call in a MENU frame. The provider has reopened the 2D batch and pushed the
 *                         menu panel rect as clip/origin: draw in panel-local coordinates; do NOT call Begin/EndGui2D yourself.
 *                         ctx.panel / panelRect / menuCreature / companions / companionCtrl are filled.
 *   K2PS_HUD_DRAW         hook 6, leader call in the HUD frame (clip stack empty). Draw for every slot here (later slots'
 *                         icon rows leak clip state, doc 08 session 4); use SlotAt to walk the table.
 * Priorities: lower runs first; ties run in registration order. Canonical values: shield bar 100, buff HUD 200 (the order the
 * single-DLL v1.1.0 drew in). A callback must be cheap and must not throw.
 */
#ifndef K2_PORTRAITSLOTS_H
#define K2_PORTRAITSLOTS_H

#include "kpatch_api.h"

#define K2PS_IFACE   "k2.portraitslots"
#define K2PS_VERSION 1

#define K2PS_SNAPSHOT        0
#define K2PS_PRE_ENGINE_READ 1
#define K2PS_FRAME_BEGIN     2
#define K2PS_SLOT            3
#define K2PS_MENU_OVERLAY    4
#define K2PS_HUD_DRAW        5
#define K2PS_PHASE_COUNT     6

#define K2PS_PRIO_SHIELD 100
#define K2PS_PRIO_BUFF   200

#define K2PS_MAX_SLOTS 4

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

typedef struct K2PsContext {
    uint32_t struct_size;
    int32_t phase;
    int32_t slot;            /* portrait slot object (ECX of FUN_00745770 / the UpdatePortraits slot) */
    int32_t creature;        /* SNAPSHOT: the engine's creature; else CreatureForSlot(slot) (0 = none or stale) */
    int32_t isLarge;         /* leader card (HUD) / top-bar portrait (menu) */
    int32_t callerEbp;       /* FUN_00745770's caller frame; 0 in SNAPSHOT */
    uint32_t frame;          /* provider frame counter */
    int32_t inMenu;          /* clip stack index != 0 (menu frame) */
    /* K2PS_MENU_OVERLAY only (0 otherwise): */
    int32_t panel;           /* CSWGuiInGameMenu = slot - 0x2350 */
    int32_t panelRect[4];    /* engine 0x410020 */
    int32_t menuCreature;    /* the selected creature ([callerEbp-0x38]), validated */
    int32_t companions[2];   /* party positions 1 / 2 (server creatures), 0 = none */
    int32_t companionCtrl[2];/* LBL_CHAR2 / LBL_CHAR3 controls (panel + 0x3134 / 0x36DC) */
} K2PsContext;

typedef void (KPATCH_CALL *K2PsCallback)(const K2PsContext* ctx, void* user);

typedef struct K2PortraitSlotsApi {
    uint32_t struct_size;
    /* Add fn to a phase. owner = your manifest id (logged). Returns a token > 0, or <= 0 (bad phase, null fn, table full). */
    int32_t (KPATCH_CALL *Register)(int32_t phase, int32_t priority, K2PsCallback fn, void* user, const char* owner);
    void (KPATCH_CALL *Unregister)(int32_t token);
    /* Creature shown in a HUD slot (fresh: refreshed by UpdatePortraits in the last 500 ms) or the menu top-bar slot; 0 = unknown. */
    int32_t (KPATCH_CALL *CreatureForSlot)(int32_t slot);
    /* HUD slot table entry i (0..K2PS_MAX_SLOTS-1): returns 1 and fills slot/creature when the entry is live and fresh. */
    int32_t (KPATCH_CALL *SlotAt)(int32_t i, int32_t* slot, int32_t* creature);
    uint32_t (KPATCH_CALL *Frame)(void);
} K2PortraitSlotsApi;

#pragma pack(pop)

#ifdef __cplusplus
} /* extern "C" */

// ---- C++ consumer helper ------------------------------------------------------------------------------------------------------
// In KPatchInit:   g_ps.init(api, "my-patch-id"); g_ps.add(K2PS_HUD_DRAW, 100, onHudDraw); g_ps.connect();
// In your hooks:   g_ps.poll();   (retries connect() if the provider was not there at init: config order is not a contract)
//                  int c = g_ps.creatureForSlot(slot);
// Without a provider (not installed, or a patcher that predates KPatchInit) every call is a safe no-op and returns 0.
namespace k2ps {

struct Client {
    struct Reg { int32_t phase, prio; K2PsCallback fn; void* user; };
    const KPatchApi* api = nullptr;
    const K2PortraitSlotsApi* ps = nullptr;
    const char* owner = "";
    Reg regs[K2PS_PHASE_COUNT * 2] = {};
    int nRegs = 0;
    bool connected = false;
    unsigned pollCount = 0;
    void (*onConnect)(bool ok, const char* why) = nullptr;   // optional: log the outcome of each connect attempt that changes state
    bool reportedMissing = false;

    void init(const KPatchApi* a, const char* id) { api = a; owner = id ? id : ""; }
    bool add(int32_t phase, int32_t prio, K2PsCallback fn, void* user = nullptr) {
        if (nRegs >= int(sizeof(regs) / sizeof(regs[0])) || !fn) return false;
        regs[nRegs++] = { phase, prio, fn, user };
        return true;
    }
    static bool usable(const K2PortraitSlotsApi* p) {
        return p && p->struct_size >= offsetof(K2PortraitSlotsApi, Frame) + sizeof(p->Frame) && p->Register && p->CreatureForSlot && p->SlotAt;
    }
    // require + register everything once. Idempotent; true when connected.
    bool connect() {
        if (connected) return true;
        if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->require) {
            if (!reportedMissing && onConnect) onConnect(false, "no KPatchApi (patcher predates the registry?)");
            reportedMissing = true;
            return false;
        }
        const K2PortraitSlotsApi* p = static_cast<const K2PortraitSlotsApi*>(api->require(K2PS_IFACE, K2PS_VERSION));
        if (!usable(p)) {
            if (!reportedMissing && onConnect) onConnect(false, p ? "k2.portraitslots v1 table too small" : "k2.portraitslots v1 not provided (portrait-slots missing or loaded later)");
            reportedMissing = true;
            return false;
        }
        ps = p;
        int ok = 0;
        for (int i = 0; i < nRegs; ++i) if (ps->Register(regs[i].phase, regs[i].prio, regs[i].fn, regs[i].user, owner) > 0) ++ok;
        connected = true;
        if (onConnect) onConnect(ok == nRegs, ok == nRegs ? "registered" : "some registrations refused");
        return true;
    }
    // Cheap retry from a hook: one require every 256 calls until connected.
    void poll() { if (!connected && (pollCount++ & 255) == 0) connect(); }
    int32_t creatureForSlot(int32_t slot) const { return ps ? ps->CreatureForSlot(slot) : 0; }
    bool slotAt(int32_t i, int32_t* slot, int32_t* creature) const { return ps && ps->SlotAt(i, slot, creature) == 1; }
    uint32_t frame() const { return ps && ps->Frame ? ps->Frame() : 0; }
};

} // namespace k2ps
#endif /* __cplusplus */

#endif /* K2_PORTRAITSLOTS_H */
