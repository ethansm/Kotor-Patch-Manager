// hud-host (Steam Aspyr build 6A522E71...): neutral owner of the CSWGuiTargetActionMenu::PopulateMenus hook (0x00743540) and provider
// of "k2.hud" v1 (HudHost.h). It draws nothing and writes nothing to the engine; consumers (combat-hotbar, ...) register phase
// callbacks through the registry. Design: patch_manager_mods/31_combat_action_hotbar.md section 5.
//
// Hook (cdecl detour at the prologue, original bytes still run, so parameters are esp-relative):
//   0x00743540  PopulateMenus: ecx = menu (hud + 0xCC), [esp+4] = leader, [esp+8] = forceUnusable, [esp+0xC] = target.
//               menu+0x1BAA = int8 target type (0..3, 0xFF none), read through SafeRead (site 0x7001).
//   0x00743850  TARGET_MENU_POST, called once at the end of PopulateMenus: ecx = menu, [esp+4] = leader, [esp+8] = target (may be
//               null); callee RET 8. Dispatched only after a matching PRE (hhd::runPost), frame number reused from the PRE.
// Logging: kpatch.log patch "hud-host", channel "core" (registrations and the first dispatch at INFO).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "kpatch_api.h"
#include "KpLog.h"
#include "SafeRead.h"
#include "HudHost.h"
#include "HhDispatch.h"

KP_LOG_PATCH("hud-host");

namespace {

constexpr unsigned S_TargetType = 0x7001;
constexpr int TargetTypeOff = 0x1BAA;

KP_CHANNEL(chCore, "core");

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

const K2HudApi g_api = { sizeof(K2HudApi), apiRegister, apiUnregister, apiHud, apiFrame };

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

__declspec(dllexport) void __cdecl HH_TargetMenuPost(void* menuPtr, void* leaderPtr, void* targetPtr) {
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

} // extern "C"
