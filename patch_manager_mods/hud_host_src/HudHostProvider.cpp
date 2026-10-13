// hud-host (Steam Aspyr build 6A522E71...): neutral owner of the CSWGuiTargetActionMenu::PopulateMenus hook (0x00743540) and provider
// of "k2.hud" v1 (HudHost.h). It draws nothing and writes nothing to the engine; consumers (combat-hotbar, ...) register phase
// callbacks through the registry. Design: patch_manager_mods/31_combat_action_hotbar.md section 5.
//
// Hook (cdecl detour at the prologue, original bytes still run, so parameters are esp-relative):
//   0x00743540  PopulateMenus: ecx = menu (hud + 0xCC), [esp+4] = leader, [esp+8] = forceUnusable, [esp+0xC] = target.
//               menu+0x1BAA = int8 target type (0..3, 0xFF none), read through SafeRead (site 0x7001).
//   0x0040F5A0  BUILD (hud-host 0.3.0): layout-closing function of every panel ctor, ecx = panel; dispatched only when the panel
//               vtable (read through SafeRead, site 0x7C10) is the HUD's (gameaddr::HudVtable); the layout is still open there.
//   0x00743847  TARGET_MENU_POST (hud-host 0.4.0), PopulateMenus tail (before POP ESI/.../RET 0xC): [ebp-0x30] = menu,
//               [ebp+8] = leader, [ebp+0x10] = target (may be null). 0x743850 has positioned the menu (origin +0x1648/+0x164C
//               final), nothing drawn yet. Dispatched only after a matching PRE (hhd::runPost), frame number reused from the PRE.
// Logging: kpatch.log patch "hud-host", channel "core" (registrations and the first dispatch at INFO).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

#include "kpatch_api.h"
#include "KpLog.h"
#include "SafeRead.h"
#include "GameAddr.h"
#include "HudHost.h"
#include "HhDispatch.h"
#include "HhStrip.h"

KP_LOG_PATCH("hud-host");

namespace {

constexpr unsigned S_TargetType = 0x7001;
constexpr unsigned S_PanelVtable = 0x7C10;   // panel vtable read in HH_LayoutClosing
constexpr int TargetTypeOff = 0x1BAA;

KP_CHANNEL(chCore, "core");

// Only stores the path and the level (no I/O): safe from a static initialiser.
const bool g_fallbackSet = (KpLog_SetFallbackFile("hud-host_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

hhd::Dispatcher g_disp;
uint32_t g_frame = 0;
int32_t g_hud = 0;   // last seen HUD object (0 = none yet)
hhd::PreState g_pre;   // last PRE context, consumed by the POST of the same PopulateMenus call

int32_t KPATCH_CALL apiRegister(int32_t phase, int32_t prio, K2HudCallback fn, void* user, const char* owner) {
    int32_t t = g_disp.add(phase, prio, fn, user, owner);
    KP_LOGV(chCore, t > 0 ? KPLOG_INFO : KPLOG_WARN, "register phase=%d prio=%d owner=%s -> %d", phase, prio, owner ? owner : "?", t);
    return t;
}
void KPATCH_CALL apiUnregister(int32_t token) {
    bool ok = g_disp.remove(token);
    KP_LOG(chCore, "unregister %d -> %d", token, ok ? 1 : 0);
}
int32_t KPATCH_CALL apiHud() { return g_hud; }
uint32_t KPATCH_CALL apiFrame() { return g_frame; }

hh::Strip g_strip;

int32_t KPATCH_CALL apiStripSlot(const char* owner, int32_t order, const K2HudRect* vit, int32_t width, int32_t height, K2HudRect* out) {
    if (!vit || !out) return 0;
    hh::Box r{};
    bool changed = false;
    if (!g_strip.slot(order, hh::Box{ vit->x, vit->y, vit->w, vit->h }, width, height, &r, &changed)) {
        KP_LOGV(chCore, KPLOG_WARN, "strip slot refused owner=%s order=%d", owner ? owner : "?", order);
        return 0;
    }
    // DEBUG: queue-hud registers a per-companion row width, so order 1 changes every frame when two companions queue different lengths.
    if (changed) KP_LOGV(chCore, KPLOG_DEBUG, "strip order %d width=%d owner=%s", order, width < 0 ? 0 : width, owner ? owner : "?");
    *out = K2HudRect{ r.x, r.y, r.w, r.h };
    return 1;
}

// BUILD48 probe (diagnostic): the HUD panel's dwords at +0x40..+0x4F around each BUILD consumer, first 4 builds only.
constexpr unsigned S_BuildProbe = 0x7C11;
constexpr int BuildProbeOff = 0x40;
constexpr int BuildProbeMax = 4;
int g_buildProbe = 0;          // builds probed so far
bool g_frame0Logged = false;   // +0x48 logged at the first FRAME dispatch
uintptr_t g_probePanel = 0;    // panel of the build being probed (0 = not probing)
uint32_t g_probeLast = 0;      // +0x48 before the next consumer

bool probeRead48(uint32_t* v) { return g_probePanel && saferead::readAt(S_BuildProbe, g_probePanel, BuildProbeOff + 8, v); }
void probeAfterConsumer(const char* owner, void*) {
    uint32_t now = 0;
    if (!probeRead48(&now)) return;
    KP_LOG(chCore, "BUILD48 owner=%s +0x48 %08x -> %08x", owner ? owner : "?", (unsigned)g_probeLast, (unsigned)now);
    g_probeLast = now;
}

// Probe 3: raw dump of the panel's first 0x60 bytes (24 dwords, 8 per line); "????????" = unreadable dword.
void probeDumpPanel(uintptr_t panel) {
    for (int row = 0; row < 3; ++row) {
        char line[8 * 9 + 1] = {};
        for (int i = 0; i < 8; ++i) {
            uint32_t v = 0;
            if (saferead::readAt(S_BuildProbe, panel, 0x20 * row + 4 * i, &v)) snprintf(line + 9 * i, 10, "%08x ", (unsigned)v);
            else memcpy(line + 9 * i, "???????? ", 9);
        }
        line[72] = 0;
        KP_LOG(chCore, "BUILD48 dump panel=%08x +0x%02x %s", (unsigned)panel, 0x20 * row, line);
    }
}

// Probe 2: per-frame watch of HUD +0x40..+0x4F; logs only on change (and when g_hud changes). Capped per process with separate budgets
// (64 hud changes, 32 frame dumps) so a flickering frame cannot use up the budget of the hud changes.
void watchHud48(int32_t hud, unsigned frame) {
    static int32_t lastHud = 0;
    static uint32_t last[4] = {};
    static bool haveLast = false;
    static int hudLines = 0, frameLines = 0;
    constexpr int HudCap = 64, FrameCap = 32;
    if (hudLines > HudCap && frameLines > FrameCap) return;
    const bool hudChanged = hud != lastHud;
    uint32_t d[4] = {};
    bool ok = true;
    for (int i = 0; i < 4; ++i) ok = saferead::readAt(S_BuildProbe, uintptr_t(hud), BuildProbeOff + 4 * i, &d[i]) && ok;
    const bool changed = ok && (!haveLast || memcmp(d, last, sizeof d) != 0);
    if (!hudChanged && !changed) return;
    if (hudChanged) {
        if (hudLines == HudCap) KP_LOG(chCore, "HUD48 cap reached what=hud");
        if (hudLines < HudCap) KP_LOG(chCore, "HUD48 hud=%08x (was %08x)", (unsigned)hud, (unsigned)lastHud);
        if (hudLines <= HudCap) ++hudLines;
        lastHud = hud;
    }
    if (changed) {
        if (frameLines == FrameCap) KP_LOG(chCore, "HUD48 cap reached what=frame");
        if (frameLines < FrameCap) KP_LOG(chCore, "HUD48 frame=%u +0x40 %08x %08x %08x %08x", frame, (unsigned)d[0], (unsigned)d[1], (unsigned)d[2], (unsigned)d[3]);
        if (frameLines <= FrameCap) ++frameLines;
        memcpy(last, d, sizeof d);
        haveLast = true;
    }
}

const K2HudApi g_api = { sizeof(K2HudApi), apiRegister, apiUnregister, apiHud, apiFrame, apiStripSlot };

} // namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->provide) return;
    api->provide(K2HUD_IFACE, K2HUD_VERSION, &g_api);
    KpLog_Init(api);
    KP_LOG(chCore, "provided %s v%d", K2HUD_IFACE, K2HUD_VERSION);
}

__declspec(dllexport) void __cdecl HH_TargetMenuPopulate(void* menuPtr, void* leaderPtr, int forceUnusable, void* targetPtr) {
    saferead::beginScope();   // engine code between hook calls can free memory: drop the cached readable regions
    const uintptr_t menu = reinterpret_cast<uintptr_t>(menuPtr);
    int8_t tt = -1;
    int raw = -1;
    if (menu && saferead::readAt(S_TargetType, menu, TargetTypeOff, &tt)) raw = tt;   // 0xFF reads as -1 = none
    K2HudContext ctx = hhd::makeContext(int32_t(menu), int32_t(reinterpret_cast<uintptr_t>(leaderPtr)), forceUnusable,
                                        int32_t(reinterpret_cast<uintptr_t>(targetPtr)), raw, ++g_frame);
    if (ctx.hud) g_hud = ctx.hud;
    if (!g_frame0Logged && g_hud) {
        g_frame0Logged = true;
        uint32_t v = 0;
        if (saferead::readAt(S_BuildProbe, uintptr_t(g_hud), BuildProbeOff + 8, &v)) KP_LOG(chCore, "BUILD48 frame0 +0x48 %08x", (unsigned)v);
    }
    if (g_hud) watchHud48(g_hud, (unsigned)g_frame);
    static bool first = true;
    if (first) {
        first = false;
        KP_LOG(chCore, "first dispatch: menu=%08x hud=%08x leader=%08x target=%08x type=%d consumers frame=%d pre=%d",
               (unsigned)ctx.menu, (unsigned)ctx.hud, (unsigned)ctx.leader, (unsigned)ctx.target, ctx.targetType,
               g_disp.n[K2HUD_FRAME], g_disp.n[K2HUD_TARGET_MENU_PRE]);
    }
    hhd::recordPre(g_pre, ctx);   // record before dispatch: the POST only runs after PopulateMenus returns
    hhd::runPopulate(g_disp, ctx);
}

__declspec(dllexport) void __cdecl HH_TargetMenuTail(void* menuPtr, void* leaderPtr, void* targetPtr) {
    saferead::beginScope();
    const uintptr_t menu = reinterpret_cast<uintptr_t>(menuPtr);
    int8_t tt = -1;
    int raw = -1;
    if (menu && saferead::readAt(S_TargetType, menu, TargetTypeOff, &tt)) raw = tt;
    const hhd::PostResult r = hhd::runPost(g_disp, g_pre, int32_t(menu), int32_t(reinterpret_cast<uintptr_t>(leaderPtr)),
                                           int32_t(reinterpret_cast<uintptr_t>(targetPtr)), raw);
    static bool first = true, skipLogged = false;
    if (r == hhd::PostRan) {
        if (first) {
            first = false;
            KP_LOG(chCore, "first POST dispatch: menu=%08x frame=%u consumers post=%d", (unsigned)menu, (unsigned)g_pre.ctx.frame,
                   g_disp.n[K2HUD_TARGET_MENU_POST]);
        }
    } else if (!skipLogged) {
        skipLogged = true;
        KP_LOG(chCore, "POST skipped (result=%d: 1 no PRE, 2 menu mismatch, 3 second POST): menu=%08x preMenu=%08x frame=%u; logged once",
               (int)r, (unsigned)menu, (unsigned)g_pre.ctx.menu, (unsigned)g_pre.ctx.frame);
    }
}

// 0x0040F5A0 (layout-closing function, called by every panel ctor): ecx = panel. Only the HUD panel (vtable 0x0099DCC4) dispatches
// K2HUD_BUILD; the layout is still open here, so consumers may create labels (guikit::createLabelFromTemplate).
__declspec(dllexport) void __cdecl HH_LayoutClosing(void* panelPtr) {
    saferead::beginScope();
    const uintptr_t panel = reinterpret_cast<uintptr_t>(panelPtr);
    uint32_t vtbl = 0;
    if (!panel || !saferead::readAt(S_PanelVtable, panel, 0, &vtbl)) return;
    if (!hhd::isHudPanel(vtbl, uint32_t(gameaddr::HudVtable))) return;
    g_hud = int32_t(panel);
    K2HudContext ctx = hhd::makeBuildContext(int32_t(panel), g_frame);
    static bool first = true;
    if (first) {
        first = false;
        KP_LOG(chCore, "first BUILD dispatch: panel=%08x consumers build=%d", (unsigned)panel, g_disp.n[K2HUD_BUILD]);
    } else {
        KP_LOGV(chCore, KPLOG_DEBUG, "BUILD dispatch: panel=%08x consumers build=%d", (unsigned)panel, g_disp.n[K2HUD_BUILD]);
    }
    if (g_buildProbe < BuildProbeMax) {
        ++g_buildProbe;
        if (g_buildProbe <= 2) probeDumpPanel(panel);
        uint32_t d[4] = {};
        bool ok = true;
        for (int i = 0; i < 4; ++i) ok = saferead::readAt(S_BuildProbe, panel, BuildProbeOff + 4 * i, &d[i]) && ok;
        if (ok) {
            KP_LOG(chCore, "BUILD48 entry +0x40 %08x %08x %08x %08x", (unsigned)d[0], (unsigned)d[1], (unsigned)d[2], (unsigned)d[3]);
            g_probePanel = panel;
            g_probeLast = d[2];
            hhd::runBuild(g_disp, ctx, probeAfterConsumer, nullptr);
            g_probePanel = 0;
            return;
        }
    }
    hhd::runBuild(g_disp, ctx);
}

} // extern "C"
