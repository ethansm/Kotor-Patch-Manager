// common_contract_test.cpp -- Patches/Common CONTRACT test for Combat Action Hotbar (Mod 31).
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod.
// Pins semantics relied on by CombatHotbar.cpp in SafeRead.h, KpLog.h and HudHost.h (k2hud::Client inert without hud-host).
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SafeRead.h"
#include "KpLog.h"
#include "HudHost.h"
#include "K2Addr.h"
#include "GuiKit.h"
#include "CombatLogic.h"
#include "check.h"
#include "FakeKPatchApi.h"

#define HCHECK(why, expr) ::kt::Check("combat-hotbar relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("combat-hotbar relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("combat-hotbar");
KP_CHANNEL(chTarget, "target");

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#if defined(MAP_32BIT)
    flags |= MAP_32BIT;
#endif
    uint8_t* arena = static_cast<uint8_t*>(mmap(nullptr, 0x10000, PROT_READ | PROT_WRITE, flags, -1, 0));
    HCHECK("arena allocated", arena != nullptr && arena != MAP_FAILED);

    uintptr_t menu = reinterpret_cast<uintptr_t>(arena);
    g_stubVirtualQueryAll = true;   // host stub: all memory reads as one committed read-write region (off = every probe fails)
    saferead::beginScope();

    // 1. Verify readAt for int and int8_t
    int8_t charIdx = 1;
    memcpy(arena + 0x1BAA, &charIdx, sizeof(charIdx));
    int8_t outChar = -1;
    HCHECK("readAt charIdx succeeds", saferead::readAt(1, menu, 0x1BAA, &outChar));
    HEQ("readAt charIdx correct", outChar, 1);

    int count0 = 5;
    memcpy(arena + 0x04, &count0, sizeof(count0));
    int outCount = -1;
    HCHECK("readAt count succeeds", saferead::readAt(2, menu, 0x04, &outCount));
    HEQ("readAt count correct", outCount, 5);

    // 2. Verify readExoLoose
    const char* testStr = "Master Flurry";
    memcpy(arena + 0x500, testStr, strlen(testStr) + 1);
    uint32_t strPtr = static_cast<uint32_t>(menu + 0x500);
    int32_t strLen = static_cast<int32_t>(strlen(testStr));
    char nameBuf[32] = {0};
    int nCopied = saferead::readExoLoose(3, strPtr, strLen, nameBuf, sizeof(nameBuf));
    HCHECK("readExoLoose copied bytes", nCopied > 0);
    HEQ("readExoLoose string matches", (const char*)nameBuf, testStr);

    // 2b. Verify readExo (CExoString {p, len} struct read)
    uint32_t exoStruct[2] = { static_cast<uint32_t>(menu + 0x500), static_cast<uint32_t>(strlen(testStr)) };
    memcpy(arena + 0x700, exoStruct, sizeof(exoStruct));
    char exoBuf[32] = {0};
    int exoLen = saferead::readExo(5, menu + 0x700, exoBuf, sizeof(exoBuf));
    HEQ("readExo returns length", exoLen, static_cast<int>(strlen(testStr)));
    HEQ("readExo string matches", (const char*)exoBuf, testStr);
    char tinyBuf[4] = {0};
    HEQ("readExo rejects len >= cap", saferead::readExo(5, menu + 0x700, tinyBuf, sizeof(tinyBuf)), 0);

    // 3. Verify formatResRef16 / readResRef16
    char rawRef[16] = { 'i', 'p', '_', 'f', 'l', 'u', 'r', 'r', 'y', 0, 0, 0, 0, 0, 0, 0 };
    memcpy(arena + 0x600, rawRef, 16);
    char iconBuf[17] = {0};
    HCHECK("readResRef16 succeeds", saferead::readResRef16(4, menu + 0x600, iconBuf, 0));
    HEQ("readResRef16 matches", (const char*)iconBuf, "ip_flurry");

    // 4. HudHost.h: with hud-host missing (not installed / listed later) KPatchInit's connect() is a safe no-op
    fakekp::reset();
    k2hud::Client hud; hud.init(fakekp::api(), "combat-hotbar");
    HCHECK("add() accepts the TARGET_MENU_PRE callback", hud.add(K2HUD_TARGET_MENU_PRE, 100, [](const K2HudContext*, void*) {}));
    HCHECK("connect() without hud-host is false (inert)", !hud.connect());
    HCHECK("client stays unconnected", !hud.connected);
    HEQ("hud() without hud-host = 0", hud.hud(), 0);
    HEQ("K2HudContext v1 size", int(sizeof(K2HudContext)), 40);

    // 5. CH2/CH3b: probeWrite on a writable slot, winForeground host-callable, K2HUD_TARGET_MENU_POST registration, KeyEdge/pickInvokeEntry
    HCHECK("probeWrite accepts a writable selection slot", saferead::probeWrite(0x6F60, arena + 0x24, 4));
    HCHECK("probeWrite accepts an arrow flag dword", saferead::probeWrite(0x6F70, arena + 0x43C, 4));
    uint32_t arrowFlags = 0xFF;
    *reinterpret_cast<uint32_t*>(arena + 0x43C) = arrowFlags;
    *reinterpret_cast<uint32_t*>(arena + 0x43C) &= ~2u;
    HEQ("arrow hide clears only bit1", *reinterpret_cast<uint32_t*>(arena + 0x43C), 0xFDu);
    HCHECK("beginScope re-arms the cache", (saferead::beginScope(), saferead::probeRead(0x6F50, arena, 4)));
    // DLL build uses guikit::detail::winForeground() (GuiKit.h Win32 default, absent under GUIKIT_HOST); the host pin is the platform hook it installs.
    HCHECK("GuiKit gameForeground hook is null under GUIKIT_HOST (Win32 default not installed)", guikit::g_gk.gameForeground == nullptr);
    HCHECK("add() accepts the TARGET_MENU_POST callback", hud.add(K2HUD_TARGET_MENU_POST, 100, [](const K2HudContext*, void*) {}));
    ch::KeyEdge ke;
    HCHECK("KeyEdge fires on up->down", ke.update(true));
    HCHECK("KeyEdge does not refire while held", !ke.update(true));
    ch::GroupView gv; memset(&gv, 0, sizeof(gv)); gv.count = 1; gv.e[0].usable = true; gv.e[0].owner = 1;
    HEQ("pickInvokeEntry picks a usable owned entry", ch::pickInvokeEntry(gv, 2), 0);
    HEQ("pickInvokeEntry rejects 0xFF target", ch::pickInvokeEntry(gv, 0xFF), -1);

    // 6. CH4b: K2HUD_BUILD phase + ctx.panel (HudHost.h), GuiKit flag/extent semantics on a fake control, grid layout contract
    HEQ("K2HUD_BUILD phase id", K2HUD_BUILD, 3);
    HCHECK("K2HUD_PHASE_COUNT covers BUILD", K2HUD_PHASE_COUNT > K2HUD_BUILD);
    HCHECK("K2HudContext carries panel inside struct_size", offsetof(K2HudContext, panel) + sizeof(int32_t) <= sizeof(K2HudContext));
    HEQ("K2HudContext.panel is the last v1 field", int(offsetof(K2HudContext, panel)), 36);
    HCHECK("add() accepts the BUILD callback", hud.add(K2HUD_BUILD, 300, [](const K2HudContext*, void*) {}));
    {
        uintptr_t ctl = menu + 0x900;   // fake control: flags + extent ints at the Steam table offsets
        const int oFlags = guikit::detail::off(&K2AddrTable::Ctl_Flags);
        const int oX = guikit::detail::off(&K2AddrTable::Ctl_X), oY = guikit::detail::off(&K2AddrTable::Ctl_Y);
        const int oW = guikit::detail::off(&K2AddrTable::Ctl_W), oH = guikit::detail::off(&K2AddrTable::Ctl_H);
        HCHECK("Ctl offsets known for this exe", oFlags && oX && oY && oW && oH);
        *reinterpret_cast<int*>(ctl + oFlags) = 0;
        HCHECK("setVisible(true) succeeds", guikit::setVisible(ctl, true));
        HCHECK("shown control has bit1 set and bit5 clear", (*reinterpret_cast<int*>(ctl + oFlags) & guikit::FlagVisible) && !(*reinterpret_cast<int*>(ctl + oFlags) & guikit::FlagClickThrough));
        HCHECK("setClickable(false) sets bit5 only", guikit::setClickable(ctl, false) && (*reinterpret_cast<int*>(ctl + oFlags) & guikit::FlagClickThrough) && (*reinterpret_cast<int*>(ctl + oFlags) & guikit::FlagVisible));
        HCHECK("setVisible(false) hides and goes click-through", guikit::setVisible(ctl, false) && !(*reinterpret_cast<int*>(ctl + oFlags) & guikit::FlagVisible) && (*reinterpret_cast<int*>(ctl + oFlags) & guikit::FlagClickThrough));
        *reinterpret_cast<int*>(ctl + oX) = 10; *reinterpret_cast<int*>(ctl + oY) = 20; *reinterpret_cast<int*>(ctl + oW) = 30; *reinterpret_cast<int*>(ctl + oH) = 40;
        int ex = 0, ey = 0, ew = 0, eh = 0;
        HCHECK("getExtent reads menu-local x,y,w,h", guikit::getExtent(ctl, &ex, &ey, &ew, &eh) && ex == 10 && ey == 20 && ew == 30 && eh == 40);
        guikit::ClickEdge ce;
        HCHECK("clickEdge is false without the platform hooks", !guikit::clickEdge(ce));
    }
    {
        ch::Rect box[3] = { {100, 50, 62, 20}, {200, 50, 62, 20}, {300, 50, 62, 20} };
        int cnt[3] = { 3, 0, 9 }, outN[3] = { -1, -1, -1 };
        ch::Rect out[3 * 8];
        ch::layoutGroupGrid(box, cnt, 2, 4, 2, 800, 600, out, outN);
        HCHECK("layoutGroupGrid: counts clamp to cols*maxRows", outN[0] == 3 && outN[1] == 0 && outN[2] == 8);
        HCHECK("layoutGroupGrid: row-major 2-col grid under the box", out[0].x == 100 && out[0].y == 72 && out[0].w == 30 && out[1].x == 132 && out[2].x == 100 && out[2].y == 104);
        int g = -2;
        HEQ("slotEntry maps flat slot back to entry", ch::slotEntry(8 + 5, 2, 4, &g), 5);
        HEQ("slotEntry group", g, 1);
        HEQ("hitSlot hits the first button", ch::hitSlot(out, 24, 101, 73), 0);
    }

    munmap(arena, 0x10000);
    return ::kt::Report("common_contract_test (combat-hotbar)");
}
