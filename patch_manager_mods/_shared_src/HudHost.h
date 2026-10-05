/* HudHost.h -- the "k2.hud" v1 registry interface (provider: the hud-host patch) + a C++ consumer helper.
 *
 * Why: several HUD mods (combat hotbar, force-power hotbar, buff/shield HUD, ...) need the same engine hook points around the in-game
 * menu/HUD (PopulateMenus 0x00743540), but the KPM installer rejects two patches hooking one address. hud-host owns those hooks,
 * remembers the last seen HUD object, and calls registered consumers at fixed points ("phases"), in priority order.
 * Design: patch_manager_mods/31_combat_action_hotbar.md section 5 (also doc 22 section 4.3).
 *
 * Plain C ABI (kpatch_api.h rules): fixed-width ints and function pointers only, KPATCH_CALL (cdecl), structs only grow,
 * check struct_size before reading a field newer than your oldest supported provider. Consumers MUST check ctx->struct_size
 * (K2HudContext) and api->struct_size (K2HudApi) before touching any field added after v1's first release.
 *
 * Phases (all on the game thread, inside the hooked engine function):
 *   K2HUD_FRAME            dispatched BEFORE K2HUD_TARGET_MENU_PRE on every PopulateMenus call (0x00743540).
 *   K2HUD_TARGET_MENU_PRE  PopulateMenus, before the stock target-menu population.
 *   K2HUD_TARGET_MENU_POST end of PopulateMenus (hud-host 0.2.0+, hook at PopulateMenus exit); fires once per PRE, same frame.
 *   K2HUD_BUILD            hud-host 0.3.0+: source = the layout-closing function 0x0040F5A0, which runs for every panel ctor; fires
 *                          only for the HUD panel (vtable 0x0099DCC4), once per HUD layout load. The layout is still open while
 *                          the callbacks run, so guikit::createLabelFromTemplate + appendToPanel are valid there. ctx.panel ==
 *                          ctx.hud == the HUD panel; ctx.menu == 0, ctx.targetType == -1. ctx.panel is only valid in this phase.
 * ctx.target / ctx.targetType use -1 for "none" (the engine's 0xFF).
 *
 * Register: returns a token > 0, or <= 0 on failure (null fn, bad phase, table full). Callbacks of a phase run in priority order
 * (lower first); ties run in registration order. A callback must be cheap and must not throw.
 *
 * Control ids: BUILD consumers get their ids from appendToPanel; there is no ReserveIds API.
 */
#ifndef K2_HUDHOST_H
#define K2_HUDHOST_H

#include "kpatch_api.h"

#define K2HUD_IFACE   "k2.hud"
#define K2HUD_VERSION 1

#define K2HUD_FRAME            0
#define K2HUD_TARGET_MENU_PRE  1
#define K2HUD_TARGET_MENU_POST 2   /* hud-host 0.2.0+: PopulateMenus exit, after a PRE */
#define K2HUD_BUILD            3   /* hud-host 0.3.0+: 0x0040F5A0, HUD panel only, layout still open */
#define K2HUD_PHASE_COUNT      4

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

typedef struct K2HudContext {
    uint32_t struct_size;
    int32_t phase;
    uint32_t frame;          /* provider frame counter */
    int32_t hud;             /* the HUD object (0 = unknown) */
    int32_t menu;            /* menu = hud + 0xCC */
    int32_t leader;          /* party leader creature pointer ([esp+4] at 0x743540), 0 = none */
    int32_t forceUnusable;   /* engine "force powers unusable" state at this call */
    int32_t target;          /* current target object pointer ([esp+0xC]), 0 = none */
    int32_t targetType;      /* current target type, -1 = none (0xFF) */
    int32_t panel;           /* K2HUD_BUILD only (0 otherwise) */
} K2HudContext;

typedef void (KPATCH_CALL *K2HudCallback)(const K2HudContext* ctx, void* user);

typedef struct K2HudApi {
    uint32_t struct_size;
    /* Add fn to a phase. owner = your manifest id (logged). Returns a token > 0, or <= 0 (null fn, bad phase, table full). */
    int32_t (KPATCH_CALL *Register)(int32_t phase, int32_t priority, K2HudCallback fn, void* user, const char* owner);
    void    (KPATCH_CALL *Unregister)(int32_t token);
    /* Last seen HUD object; 0 = none seen yet. */
    int32_t (KPATCH_CALL *Hud)(void);
    uint32_t (KPATCH_CALL *Frame)(void);
} K2HudApi;

#pragma pack(pop)

#ifdef __cplusplus
} /* extern "C" */

// ---- C++ consumer helper ------------------------------------------------------------------------------------------------------
// In KPatchInit:   g_hud.init(api, "my-patch-id"); g_hud.add(K2HUD_FRAME, 100, onFrame); g_hud.connect();
// In your hooks:   g_hud.poll();   (retries connect() if the provider was not there at init: config order is not a contract)
//                  int h = g_hud.hud();
// Without a provider (not installed, or a patcher that predates KPatchInit) every call is a safe no-op and returns 0.
namespace k2hud {

struct Client {
    struct Reg { int32_t phase, prio; K2HudCallback fn; void* user; };
    const KPatchApi* api = nullptr;
    const K2HudApi* hu = nullptr;
    const char* owner = "";
    Reg regs[K2HUD_PHASE_COUNT * 2] = {};
    int nRegs = 0;
    bool connected = false;
    unsigned pollCount = 0;
    void (*onConnect)(bool ok, const char* why) = nullptr;   // optional: log the outcome of each connect attempt that changes state
    bool reportedMissing = false;

    void init(const KPatchApi* a, const char* id) { api = a; owner = id ? id : ""; }
    bool add(int32_t phase, int32_t prio, K2HudCallback fn, void* user = nullptr) {
        if (nRegs >= int(sizeof(regs) / sizeof(regs[0])) || !fn) return false;
        regs[nRegs++] = { phase, prio, fn, user };
        return true;
    }
    static bool usable(const K2HudApi* p) {
        return p && p->struct_size >= offsetof(K2HudApi, Frame) + sizeof(p->Frame) && p->Register && p->Hud;
    }
    // require + register everything once. Idempotent; true when connected.
    bool connect() {
        if (connected) return true;
        if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->require) {
            if (!reportedMissing && onConnect) onConnect(false, "no KPatchApi (patcher predates the registry?)");
            reportedMissing = true;
            return false;
        }
        const K2HudApi* p = static_cast<const K2HudApi*>(api->require(K2HUD_IFACE, K2HUD_VERSION));
        if (!usable(p)) {
            if (!reportedMissing && onConnect) onConnect(false, p ? "k2.hud v1 table too small" : "k2.hud v1 not provided (hud-host missing or loaded later)");
            reportedMissing = true;
            return false;
        }
        hu = p;
        int ok = 0;
        for (int i = 0; i < nRegs; ++i) if (hu->Register(regs[i].phase, regs[i].prio, regs[i].fn, regs[i].user, owner) > 0) ++ok;
        connected = true;
        if (onConnect) onConnect(ok == nRegs, ok == nRegs ? "registered" : "some registrations refused");
        return true;
    }
    // Cheap retry from a hook: one require every 256 calls until connected.
    void poll() { if (!connected && (pollCount++ & 255) == 0) connect(); }
    int32_t hud() const { return hu ? hu->Hud() : 0; }
    uint32_t frame() const { return hu && hu->Frame ? hu->Frame() : 0; }
};

} // namespace k2hud
#endif /* __cplusplus */

#endif /* K2_HUDHOST_H */
