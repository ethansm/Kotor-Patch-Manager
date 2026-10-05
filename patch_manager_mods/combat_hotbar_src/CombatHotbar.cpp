// Combat Action Hotbar (Mod 31, 0.3.0-dev: probe + CH2 invoke keys + CH3b arrow hide; consumer of hud-host)
// Diagnostic probe for CSWGuiTargetActionMenu (hud + 0xCC).
// Safely reads and logs target action groups when an enemy is targeted.
// 0.2.0: no own hook any more. hud-host owns the PopulateMenus hook (0x00743540) and calls onTargetMenu through k2.hud@1
// (K2HUD_TARGET_MENU_PRE). Connects once in KPatchInit, so hud-host must be listed before combat-hotbar in the config:
// there is no hook of ours to retry g_hud.poll() from.
// 0.3.0-dev: CH2 invoke keys (INI [Invoke]: Ctrl/Shift-masked F9..F11 -> select the last usable entry of group g and call the engine invoke
// wrapper 0x00751710 (thiscall hud, g, 0)) inside TARGET_MENU_PRE; CH3b (INI [Arrows] Hide=1) clears bit1 of the carousel arrow controls
// (menu+0x43C / +0x60C, stride 0x750, 3 each) in TARGET_MENU_POST (priority 100).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <string>

#include "KpLog.h"
#include "SafeRead.h"
#include "HudHost.h"
#include "GuiKit.h"
#include "CombatLogic.h"

KP_LOG_PATCH("combat-hotbar");

namespace {

constexpr unsigned SiteBase = 0x6F00;
constexpr unsigned S_Menu   = SiteBase + 0x10;
constexpr unsigned S_Group  = SiteBase + 0x20;
constexpr unsigned S_Entry  = SiteBase + 0x30;
constexpr unsigned S_Menu2  = SiteBase + 0x40;
constexpr unsigned S_InvRd  = SiteBase + 0x50;   // CH2 reads (target type, group header, entries, selection slot)
constexpr unsigned S_InvWr  = SiteBase + 0x60;   // CH2 selection slot write probe
constexpr unsigned S_ArrWr  = SiteBase + 0x70;   // CH3b arrow flag write probe

constexpr uintptr_t HudTargetInvoke = 0x00751710;   // thiscall (hud, group, 0), RET 8; reads the selected key (fallback entry[0])

constexpr int EntryStride = 0x3C;
constexpr int MaxEntriesToLog = 8;

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chTarget, "target");

struct TargetState {
    uintptr_t targetObj = 0;
    int8_t targetType = -1;
    int counts[3] = { -1, -1, -1 };
    int32_t selKeys[3] = { -1, -1, -1 };
    DWORD lastLogTick = 0;
};

TargetState g_state;
k2hud::Client g_hud;

struct Config {
    bool invokeEnabled = true;
    int mod = 3;                 // bit0 Ctrl, bit1 Shift (must match exactly)
    int keys[3] = { 0x78, 0x79, 0x7A };   // VK_F9..VK_F11
    bool keepSelection = false;
    bool hideArrows = false;
} g_cfg;
ch::KeyEdge g_edge[3];
int g_arrowState = -1;   // -1 never applied, 0 failed, 1 applied (log on change only)

const char* const IniPath = ".\\combat_hotbar.ini";   // ".\\" prefix: GetPrivateProfile*A with a bare name looks in the Windows dir

void loadIni() {
    g_cfg.invokeEnabled = GetPrivateProfileIntA("Invoke", "Enabled", 1, IniPath) != 0;
    g_cfg.mod = GetPrivateProfileIntA("Invoke", "Mod", 3, IniPath) & 3;
    const int defKeys[3] = { 0x78, 0x79, 0x7A };
    const char* const names[3] = { "Key0", "Key1", "Key2" };
    for (int i = 0; i < 3; ++i) {
        int k = GetPrivateProfileIntA("Invoke", names[i], defKeys[i], IniPath);
        g_cfg.keys[i] = (k > 0 && k <= 0xFF) ? k : defKeys[i];
    }
    g_cfg.keepSelection = GetPrivateProfileIntA("Invoke", "KeepSelection", 0, IniPath) != 0;
    g_cfg.hideArrows = GetPrivateProfileIntA("Arrows", "Hide", 0, IniPath) != 0;
    KP_LOGV(chMain, KPLOG_INFO, "config: invoke=%d mod=%d keys=[%02x,%02x,%02x] keepSelection=%d arrowsHide=%d",
            (int)g_cfg.invokeEnabled, g_cfg.mod, g_cfg.keys[0], g_cfg.keys[1], g_cfg.keys[2], (int)g_cfg.keepSelection, (int)g_cfg.hideArrows);
}

bool modifiersMatch() {
    const bool ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    const bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    return ctrl == ((g_cfg.mod & 1) != 0) && shift == ((g_cfg.mod & 2) != 0);
}

void invokeGroup(uintptr_t hud, uintptr_t menu, int g) {
    int8_t type = -1;
    if (!saferead::readAt(S_InvRd, menu, 0x1BAA, &type) || type < 0 || type > 3) { KP_LOGV(chTarget, KPLOG_INFO, "invoke g%d: skip, no target (type=%d)", g, (int)type); return; }

    ch::GroupView gv; memset(&gv, 0, sizeof(gv));
    int count = 0; uintptr_t base = 0;
    saferead::readAt(S_InvRd, menu, g * 12 + 4, &count);
    saferead::readAt(S_InvRd, menu, g * 12, &base);
    gv.count = base ? ch::clampCount(count) : 0;
    for (int i = 0; i < gv.count; ++i) {
        uintptr_t e = base + (uintptr_t)i * EntryStride;
        uint32_t flags = 0;
        if (!saferead::readAt(S_InvRd, e, 0x08, &gv.e[i].key) || !saferead::readAt(S_InvRd, e, 0x1C, &gv.e[i].owner) || !saferead::readAt(S_InvRd, e, 0x30, &flags)) { gv.count = i; break; }
        gv.e[i].usable = (flags & 1) != 0;
        saferead::readResRef16(S_InvRd, e + 0x20, gv.e[i].icon, 0);
    }
    const int idx = ch::pickInvokeEntry(gv, type);
    if (idx < 0) { KP_LOGV(chTarget, KPLOG_INFO, "invoke g%d: skip, no entry (type=%d count=%d)", g, (int)type, gv.count); return; }

    const ch::EntryView& ev = gv.e[idx];
    int32_t* slot = reinterpret_cast<int32_t*>(menu + 0x24 + (uintptr_t)type * 12 + (uintptr_t)g * 4);
    int32_t oldSel = 0;
    if (!saferead::readAt(S_InvRd, reinterpret_cast<uintptr_t>(slot), 0, &oldSel)) { KP_LOGV(chTarget, KPLOG_WARN, "invoke g%d: skip, selection slot unreadable", g); return; }
    if (!saferead::probeWrite(S_InvWr, slot, 4)) { KP_LOGV(chTarget, KPLOG_WARN, "invoke g%d: skip, not writable (slot=%p)", g, (void*)slot); return; }
    KP_LOGV(chTarget, KPLOG_INFO, "invoke g%d: pre idx=%d key=%08x oldSel=%08x owner=%08x usable=%d type=%d", g, idx, (unsigned)ev.key, (unsigned)oldSel, (unsigned)ev.owner, (int)ev.usable, (int)type);
    *slot = static_cast<int32_t>(ev.key);
    using Fn = void(__thiscall*)(void*, int, int);
    reinterpret_cast<Fn>(HudTargetInvoke)(reinterpret_cast<void*>(hud), g, 0);
    int32_t selAfter = 0;
    saferead::readAt(S_InvRd, reinterpret_cast<uintptr_t>(slot), 0, &selAfter);
    bool restored = false;
    if (!g_cfg.keepSelection && selAfter == static_cast<int32_t>(ev.key) && saferead::probeWrite(S_InvWr, slot, 4)) { *slot = oldSel; restored = true; }
    KP_LOGV(chTarget, KPLOG_INFO, "invoke g%d: returned selAfter=%08x restored=%d", g, (unsigned)selAfter, (int)restored);
}

void pollInvokeKeys(const K2HudContext* ctx) {
    if (!g_cfg.invokeEnabled) return;
    const bool active = guikit::detail::winForeground() && modifiersMatch();
    for (int g = 0; g < 3; ++g) {
        const bool down = active && (GetAsyncKeyState(g_cfg.keys[g]) & 0x8000) != 0;
        if (g_edge[g].update(down) && ctx->hud) invokeGroup(static_cast<uintptr_t>(static_cast<uint32_t>(ctx->hud)), static_cast<uintptr_t>(static_cast<uint32_t>(ctx->menu)), g);
    }
}

// CH3b: clear bit1 (visible) of the 6 carousel arrow controls after the stock population; log only when the outcome changes.
void KPATCH_CALL onTargetMenuPost(const K2HudContext* ctx, void*) {
    if (!g_cfg.hideArrows || !ctx || !ctx->menu) return;
    saferead::beginScope();
    const uintptr_t menu = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->menu));
    bool ok = true;
    for (int i = 0; i < 3; ++i) {
        const uintptr_t addrs[2] = { menu + 0x43C + (uintptr_t)i * 0x750, menu + 0x60C + (uintptr_t)i * 0x750 };
        for (uintptr_t a : addrs) {
            uint32_t* p = reinterpret_cast<uint32_t*>(a);
            if (!saferead::probeWrite(S_ArrWr, p, 4)) { ok = false; continue; }
            *p &= ~2u;
        }
    }
    const int st = ok ? 1 : 0;
    if (st != g_arrowState) { g_arrowState = st; KP_LOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "arrows: hide %s (menu=%08x)", ok ? "applied" : "FAILED (not writable)", (unsigned)menu); }
}

void onConnect(bool ok, const char* why) {
    KP_LOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "k2.hud connect: %s (%s)", ok ? "ok" : "FAILED", why ? why : "?");
}

void logTargetGroup(uintptr_t menu, int g, int count) {
    if (count <= 0) return;
    uintptr_t base = 0;
    if (!saferead::readAt(S_Group, menu, g * 12, &base) || !base) return;

    int logCount = (count > MaxEntriesToLog) ? MaxEntriesToLog : count;
    for (int i = 0; i < logCount; ++i) {
        uintptr_t e = base + (uintptr_t)i * EntryStride;
        if (!saferead::probeRead(S_Entry, reinterpret_cast<const void*>(e), EntryStride)) break;

        uint32_t key = 0, flags = 0, strPtr = 0;
        int32_t strLen = 0;
        saferead::readAt(S_Entry, e, 0x08, &key);
        saferead::readAt(S_Entry, e, 0x30, &flags);
        saferead::readAt(S_Entry, e, 0x00, &strPtr);
        saferead::readAt(S_Entry, e, 0x04, &strLen);
        uint32_t fn = 0, owner = 0;
        saferead::readAt(S_Entry, e, 0x0C, &fn);
        saferead::readAt(S_Entry, e, 0x1C, &owner);

        char name[64] = {0};
        saferead::readExoLoose(S_Entry, strPtr, strLen, name, sizeof(name));

        char icon[17] = {0};
        saferead::readResRef16(S_Entry, e + 0x20, icon, 0);

        KP_LOGV(chTarget, KPLOG_DEBUG, "  g%d[%d]: key=%08x usable=%d fn=%08x owner=%08x icon=%.16s name=%s",
                g, i, (unsigned)key, (int)(flags & 1), (unsigned)fn, (unsigned)owner, icon, name);
    }
}

void KPATCH_CALL onTargetMenu(const K2HudContext* ctx, void*) {
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, targetType) + sizeof(ctx->targetType)) return;
    if (!ctx->menu) return;
    saferead::beginScope();   // cheap; hud-host also scopes each PopulateMenus call
    pollInvokeKeys(ctx);
    uintptr_t menu = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->menu));
    uintptr_t leader = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->leader));
    uintptr_t targetObj = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->target));
    const int forceUnusable = ctx->forceUnusable;

    // ctx->targetType (read from menu+0x1BAA by hud-host): target type 0..3, -1 = no target (engine 0xFF).
    const int8_t targetType = static_cast<int8_t>(ctx->targetType);
    int menuState = 0;
    saferead::readAt(S_Menu2, menu, 0x1644, &menuState);

    int counts[3] = {0, 0, 0};
    saferead::readAt(S_Menu, menu, 0x04, &counts[0]);
    saferead::readAt(S_Menu, menu, 0x10, &counts[1]);
    saferead::readAt(S_Menu, menu, 0x1C, &counts[2]);

    int32_t selKeys[3] = { -1, -1, -1 };
    if (targetType >= 0 && targetType <= 3) {
        uintptr_t selBase = menu + 0x24 + (uintptr_t)targetType * 12;
        saferead::readAt(S_Menu, selBase, 0, &selKeys[0]);
        saferead::readAt(S_Menu, selBase, 4, &selKeys[1]);
        saferead::readAt(S_Menu, selBase, 8, &selKeys[2]);
    }

    DWORD now = GetTickCount();
    bool changed = (targetObj != g_state.targetObj) ||
                   (targetType != g_state.targetType) ||
                   (counts[0] != g_state.counts[0]) ||
                   (counts[1] != g_state.counts[1]) ||
                   (counts[2] != g_state.counts[2]) ||
                   (selKeys[0] != g_state.selKeys[0]) ||
                   (selKeys[1] != g_state.selKeys[1]) ||
                   (selKeys[2] != g_state.selKeys[2]);

    if (changed || (targetObj != 0 && (now - g_state.lastLogTick >= 5000))) {
        g_state.targetObj = targetObj;
        g_state.targetType = targetType;
        g_state.counts[0] = counts[0];
        g_state.counts[1] = counts[1];
        g_state.counts[2] = counts[2];
        g_state.selKeys[0] = selKeys[0];
        g_state.selKeys[1] = selKeys[1];
        g_state.selKeys[2] = selKeys[2];
        g_state.lastLogTick = now;

        if (targetObj != 0 || counts[0] > 0 || counts[1] > 0 || counts[2] > 0) {
            char typeStr[16];
            if (targetType >= 0 && targetType <= 3) snprintf(typeStr, sizeof(typeStr), "%d", (int)targetType);
            else snprintf(typeStr, sizeof(typeStr), "none");
            KP_LOG(chTarget, "target_menu: obj=%p leader=%p forceUnusable=%d state1644=%d type=%s counts=[%d,%d,%d] sel=[%08x,%08x,%08x]",
                   (void*)targetObj, (void*)leader, forceUnusable, menuState, typeStr,
                   counts[0], counts[1], counts[2],
                   (unsigned)selKeys[0], (unsigned)selKeys[1], (unsigned)selKeys[2]);

            for (int g = 0; g < 3; ++g) {
                logTargetGroup(menu, g, counts[g]);
            }
        }
    }
}

} // namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    g_hud.init(api, "combat-hotbar");
    g_hud.onConnect = onConnect;
    loadIni();
    g_hud.add(K2HUD_TARGET_MENU_PRE, 100, onTargetMenu);
    g_hud.add(K2HUD_TARGET_MENU_POST, 100, onTargetMenuPost);
    g_hud.connect();
}

} // extern "C"
