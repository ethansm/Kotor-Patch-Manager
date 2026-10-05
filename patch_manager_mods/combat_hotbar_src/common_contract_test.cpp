// common_contract_test.cpp -- Patches/Common CONTRACT test for Combat Action Hotbar (Mod 31).
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod.
// Pins semantics relied on by CombatHotbar.cpp in SafeRead.h, KpLog.h, HudHost.h and GuiKit.h (sound/image hooks, CH4c grid + freeze helpers) (k2hud::Client inert without hud-host).
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

// CH4c fakes: GUI sound manager call recorder, image loader.
static int g_soundCalls = 0, g_soundIdx = -1; static void* g_soundMgr = nullptr;
static void fPlaySound(void* mgr, char idx) { ++g_soundCalls; g_soundIdx = idx; g_soundMgr = mgr; }
static int g_imgLoads = 0;
static void* fLoadImg(const char* name) { ++g_imgLoads; return strcmp(name, "lbl_miscroll_f") == 0 ? reinterpret_cast<void*>(0x1111) : (strcmp(name, "lbl_miscroll_hi") == 0 ? reinterpret_cast<void*>(0x2222) : nullptr); }

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

    // 7. CH4c: own buttons. GuiKit sound/image engine hooks (playGuiSound mirrors playHoverSound with an idx param), imageFor pool, banner grid + freeze helpers
    {
        guikit::Engine saved = guikit::g_gk;
        const uint32_t mgrVal = static_cast<uint32_t>(menu + 0xA00);
        memcpy(arena + 0xB00, &mgrVal, 4);   // *guiSoundMgrAddr = manager pointer
        guikit::g_gk.playSound = fPlaySound;
        guikit::g_gk.guiSoundMgrAddr = menu + 0xB00;
        uint32_t mgrRead = 0;
        HCHECK("detail::rd32 reads the GUI sound manager global", guikit::detail::rd32(guikit::g_gk.guiSoundMgrAddr, 0, &mgrRead) && mgrRead == mgrVal);
        HCHECK("g_gk.playSound is substitutable and called with (mgr, idx)", (guikit::g_gk.playSound(reinterpret_cast<void*>(static_cast<uintptr_t>(mgrRead)), 0), g_soundCalls == 1 && g_soundIdx == 0 && g_soundMgr == reinterpret_cast<void*>(static_cast<uintptr_t>(mgrVal))));
        HCHECK("playHoverSound plays idx 1 (hover)", guikit::playHoverSound() && g_soundCalls == 2 && g_soundIdx == 1);
        memset(arena + 0xB00, 0, 4);
        HCHECK("playHoverSound is false with a null manager (no call)", !guikit::playHoverSound() && g_soundCalls == 2);
        guikit::g_gk.loadImage = fLoadImg;
        HEQ("imageFor resolves the frame art", reinterpret_cast<uintptr_t>(guikit::imageFor("lbl_miscroll_f")), uintptr_t(0x1111));
        HEQ("imageFor is case-insensitive and pooled (one load)", reinterpret_cast<uintptr_t>(guikit::imageFor("LBL_MISCROLL_F")), uintptr_t(0x1111));
        HEQ("imageFor resolves the hover art", reinterpret_cast<uintptr_t>(guikit::imageFor("lbl_miscroll_hi")), uintptr_t(0x2222));
        HEQ("imageFor loads each resref once", g_imgLoads, 2);
        guikit::g_gk = saved;

        HCHECK("probeWrite accepts the menu origin dword and a group control flags dword", saferead::probeWrite(0x6FA0, arena + 0x1648, 4) && saferead::probeWrite(0x6FA1, arena + 0x54 + 0x3A0 + 0x48, 4));
        // layoutBannerGrid: 800x600, banner 200..400 x 40..70, boxes y=75, 3 columns of 2x4
        ch::Rect banner = { 200, 40, 200, 30 }, boxes = { 200, 75, 200, 18 };
        int cnt[3] = { 2, 0, 5 }, outN[3] = { -1, -1, -1 };
        ch::Rect out[3 * 8];
        ch::layoutBannerGrid(banner, boxes, cnt, 2, 4, 0, 4, 8, 800, 600, out, outN);
        HCHECK("layoutBannerGrid: outN clamps counts", outN[0] == 2 && outN[1] == 0 && outN[2] == 5);
        HCHECK("layoutBannerGrid: grid starts at boxesUnion.y, square buttons", out[0].y == 75 && out[0].w == out[0].h && out[0].w > 0);
        HCHECK("layoutBannerGrid: group g rects at g*cols*maxRows (slotEntry stride)", out[2 * 8].w > 0 && out[1 * 8].w == 0);
        ch::Rect gb = ch::gridBounds(out, outN, 8);
        HCHECK("gridBounds covers the used rects", gb.w > 0 && gb.x <= out[0].x && gb.x + gb.w >= out[2 * 8].x + out[2 * 8].w);
        // layoutBannerGroups: same inputs plus capPx=10, sidePx=6; one uniform-height pill per group, buttons inside
        ch::Rect gout[3 * 8], gcont[3]; int goutN[3] = { -1, -1, -1 };
        ch::layoutBannerGroups(banner, boxes, cnt, 2, 4, 0, 4, 8, 10, 6, 800, 600, gout, goutN, gcont);
        HCHECK("layoutBannerGroups: outN clamps counts like layoutBannerGrid", goutN[0] == 2 && goutN[1] == 0 && goutN[2] == 5);
        HCHECK("layoutBannerGroups: three pills of equal height, empty group included, starting at boxesUnion.y", gcont[0].w > 0 && gcont[1].w > 0 && gcont[2].w > 0 && gcont[0].y == 75 && gcont[0].h == gcont[1].h && gcont[1].h == gcont[2].h);
        HCHECK("layoutBannerGroups: group g buttons at g*cols*maxRows, unused zeroed", gout[2 * 8].w > 0 && gout[1 * 8].w == 0);
        HCHECK("layoutBannerGroups: buttons sit inside their pill (capPx top, sidePx sides)", gout[0].x >= gcont[0].x + 6 && gout[0].y >= gcont[0].y + 10 && gout[2 * 8].x + gout[2 * 8].w <= gcont[2].x + gcont[2].w - 6);
        // GuiKit tint/alpha copy (CH4d): the borderOk gate refuses a control with no label border vtable, the setters/getters never fault on it
        {
            float rgb[3] = { 0.f, 0.f, 0.f }, al = 0.f;
            HCHECK("guikit::getBorderTint/getAlpha/setBorderTint refuse a null control", !guikit::getBorderTint(0, rgb) && !guikit::getAlpha(0, &al) && !guikit::setBorderTint(0, rgb));
        }
        ch::FreezeState fs = { false, 0, 0 }; int fx = -1, fy = -1;
        HCHECK("freezeOrigin: cursor outside follows the live origin", !ch::freezeOrigin(fs, false, 10, 20, &fx, &fy) && fx == 10 && fy == 20);
        HCHECK("freezeOrigin: cursor in grid latches the entry origin", ch::freezeOrigin(fs, true, 10, 20, &fx, &fy) && fx == 10 && fy == 20);
        HCHECK("freezeOrigin: stays latched while the live origin moves", ch::freezeOrigin(fs, true, 50, 60, &fx, &fy) && fx == 10 && fy == 20);
        HCHECK("freezeOrigin: leaving unfreezes", !ch::freezeOrigin(fs, false, 50, 60, &fx, &fy) && fx == 50 && fy == 60);
    }

    munmap(arena, 0x10000);
    return ::kt::Report("common_contract_test (combat-hotbar)");
}
