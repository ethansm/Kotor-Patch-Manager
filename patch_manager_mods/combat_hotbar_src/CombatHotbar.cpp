// Combat Action Hotbar (Mod 31, Phase 1 Probe)
// Diagnostic probe for CSWGuiTargetActionMenu (hud + 0xCC).
// Safely reads and logs target action groups when an enemy is targeted.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>

#include "KpLog.h"
#include "SafeRead.h"

KP_LOG_PATCH("combat-hotbar");

namespace {

constexpr unsigned SiteBase = 0x6F00;
constexpr unsigned S_Menu   = SiteBase + 0x10;
constexpr unsigned S_Group  = SiteBase + 0x20;
constexpr unsigned S_Entry  = SiteBase + 0x30;
constexpr unsigned S_Menu2  = SiteBase + 0x40;

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

} // namespace

extern "C" __declspec(dllexport) void __cdecl CombatTargetMenuUpdate(void* menuPtr, void* leaderPtr, int forceUnusable, void* targetObjPtr) {
    saferead::beginScope();
    if (!menuPtr) return;
    uintptr_t menu = reinterpret_cast<uintptr_t>(menuPtr);
    uintptr_t leader = reinterpret_cast<uintptr_t>(leaderPtr);
    uintptr_t targetObj = reinterpret_cast<uintptr_t>(targetObjPtr);

    // menu+0x1BAA: target type 0..3, 0xFF (-1) = no target.
    int8_t targetType = -1;
    saferead::readAt(S_Menu, menu, 0x1BAA, &targetType);
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
