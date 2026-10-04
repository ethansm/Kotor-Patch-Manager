// common_contract_test.cpp -- Patches/Common CONTRACT test for Level-Up Override (doc 20 section 4.1; rule of 2026-10-03).
//
// Pins the semantics THIS mod relies on in GuiTip.h (new, LevelUpUi.h call sites) and the GuiKit.h helpers its character-screen UI uses; the
// headers' own behaviour is _shared_src/{guitip,guikit,saferead,k2addr}_test.cpp. Every message starts with "level-up-override relies on: " so a
// failure names the consumer and the reason. Built against ../_shared_src (see test.toml): any edit to GuiTip.h / GuiKit.h / K2Addr.h re-runs it.
// Expected values are independent literals (LevelUpUi.h comments, RE spike UX_native_tooltip.md, GuiKit engine facts), never read back from the headers.
// Host: the stub's VirtualQuery reports every address readable; engine 32-bit pointers live in a MAP_32BIT arena and are written as uint32_t.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "GuiTip.h"
#include "check.h"
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

#define LCHECK(why, expr) ::kt::Check("level-up-override relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define LEQ(why, a, b) ::kt::CheckEq("level-up-override relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

static uint8_t* g_arena = nullptr; static size_t g_used = 0; constexpr size_t ArenaSize = 1u << 20;
static uintptr_t arenaAlloc(size_t n) { n = (n + 15) & ~size_t(15); if (g_used + n > ArenaSize) { printf("arena full\n"); exit(4); } uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used); memset(g_arena + g_used, 0, n); g_used += n; return a; }
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

static char g_ctorText[64]; static void* g_ctorDst = nullptr, *g_dtorDst = nullptr; static int g_nCtor = 0, g_nDtor = 0;
static void* fExoCtor(void* e, const char* s) { ++g_nCtor; g_ctorDst = e; snprintf(g_ctorText, sizeof g_ctorText, "%s", s); return e; }
static void fExoDtor(void* e) { ++g_nDtor; g_dtorDst = e; }
static void fListAdd(void* list, void* v) { uintptr_t l = reinterpret_cast<uintptr_t>(list); uint32_t arr = at<uint32_t>(l, 0); int cnt = at<int>(l, 4); at<uint32_t>(arr, cnt * 4) = (uint32_t)reinterpret_cast<uintptr_t>(v); at<int>(l, 4) = cnt + 1; }
static bool fFg = true, fLmb = false;
static bool fgFn() { return fFg; }
static bool lmbFn() { return fLmb; }

int main() {
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (m == MAP_FAILED) { perror("mmap MAP_32BIT"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    guikit::Engine& e = guikit::g_gk;
    e.exoCtor = fExoCtor; e.exoDtor = fExoDtor; e.listAdd = fListAdd; e.gameForeground = fgFn; e.lbuttonDown = lmbFn;

    // ---- GuiTip: native tooltip fields (uiAppend -> setNative; the engine's DisplayToolTip reads strref +0x28 then the CExoString +0x2C) ----
    LEQ("tip strref offset 0x28", guitip::Ctl_TipStrRef, 0x28);
    LEQ("tip string offset 0x2C", guitip::Ctl_TipText, 0x2C);
    LEQ("tooltip-shown control flag 0x40", guitip::CtlFlag_TipShown, 0x40u);
    LCHECK("Steam table selected on the host (nativeSupported)", guitip::nativeSupported());
    {
        uintptr_t c = arenaAlloc(0x200);
        at<int>(c, 0x28) = 99;
        LCHECK("setNative succeeds on a readable control", guitip::setNative(c, "a\nb"));
        LEQ("setNative zeroes the TLK strref (the string is only used when it is 0)", at<int>(c, 0x28), 0);
        LCHECK("setNative frees then constructs the CExoString at ctl+0x2C", g_nDtor == 1 && g_nCtor == 1 && g_dtorDst == (void*)(c + 0x2C) && g_ctorDst == (void*)(c + 0x2C) && !strcmp(g_ctorText, "a\nb"));
        LCHECK("setNative on a null control / null text is a no-op", !guitip::setNative(0, "x") && !guitip::setNative(c, nullptr) && g_nCtor == 1);
        k2addr::setForTest(&kGog);
        at<int>(c, 0x28) = 7;
        LCHECK("no native tip write on the GOG table (offsets are Steam-decoded only)", !guitip::setNative(c, "x") && at<int>(c, 0x28) == 7 && g_nCtor == 1);
        k2addr::clearForTest();
    }
    // ---- GuiTip: manager snapshot (uiTipTick: owner == hovered control means the engine box is up) ----
    LEQ("manager mouse-over field 0x08", guitip::Mgr_Over, 0x08);
    LEQ("manager owner field 0x40", guitip::Mgr_Owner, 0x40);
    LEQ("manager appear-time field 0x48", guitip::Mgr_Appear, 0x48);
    LEQ("manager flags field 0x1C, bit3 = tooltip drawn", guitip::Mgr_Flags, 0x1C); LEQ("tooltip drawn bit", guitip::MgrFlag_TipDrawn, 8u);
    LCHECK("the manager cell is the Steam GuiSoundMgr global 0xA1B49C (read through g_gk, not a raw address)", kSteam.GuiSoundMgr == 0x00A1B49C);
    {
        uintptr_t cell = arenaAlloc(16), mgr = arenaAlloc(0x100);
        e.guiSoundMgrAddr = cell;
        guitip::Mgr g;
        LCHECK("readMgr is false (no crash) while the manager pointer is null", !guitip::readMgr(&g) && !g.ok);
        at<uint32_t>(cell, 0) = (uint32_t)mgr; at<uint32_t>(mgr, 0x40) = 0x4242; at<float>(mgr, 0x48) = 0.75f;
        LCHECK("readMgr reports owner and appear time", guitip::readMgr(&g) && g.ok && g.owner == 0x4242 && g.appear == 0.75f);
        LEQ("engine appear time 0.75 s -> 750 ms", guitip::delayMs(g), 750u);
        g.ok = false; LEQ("unreadable manager -> 400 ms default delay", guitip::delayMs(g), 400u);
    }
    // ---- GuiTip: fallback state machine (custom box = delay + 250 ms grace; engine-owned -> never) ----
    {
        guitip::State s;
        guitip::tick(s, 4, false, 1000, 500);
        LCHECK("custom box appears no earlier than delay + 250 ms", !guitip::tick(s, 4, false, 1749, 500) && guitip::tick(s, 4, false, 1750, 500));
        LCHECK("moving to another control hides the box and restarts the clock", !guitip::tick(s, 5, false, 1751, 500) && !s.boxShown);
        guitip::tick(s, 5, true, 1800, 500);
        LCHECK("once the engine owned a hovered control the custom box never shows again", !guitip::tick(s, 5, false, 90000, 500) && !guitip::tick(s, 6, false, 91000, 500) && !guitip::tick(s, 6, false, 95000, 500));
        guitip::reset(s);
        LCHECK("reset (click / page close) keeps nativeSeen", s.nativeSeen && !s.boxShown);
    }
    // ---- GuiTip: box layout inside the stats column (nothing may sit right of x 300 in the page area) ----
    {
        int lines = 0, maxLen = 0;
        guitip::measure("When (or whether) this companion\nbecomes a Jedi.", &lines, &maxLen);
        LCHECK("measure: line count and longest line (the engine box does not wrap, so '\\n' is explicit)", lines == 2 && maxLen == 32);
        const guitip::Metrics mt = {6, 12, 6, 5};
        guitip::Rect r = guitip::layout(3, 36, mt, {256, 386, 36, 24}, {54, 134, 300, 470}, 4);
        LCHECK("a 36-char tip next to the '+' button is clamped to x <= 300", r.x + r.w <= 300 && r.x >= 54 && r.w == 36 * 6 + 12);
        r = guitip::layout(3, 36, mt, {62, 442, 72, 26}, {54, 134, 300, 470}, 4);
        LCHECK("a tip for the bottom row (Apply) flips above the control", r.y + r.h <= 442 && r.y >= 134);
        r = guitip::layout(3, 31, mt, {362, 372, 94, 26}, {54, 134, 300, 372}, 4);
        LCHECK("the Configure button's tip sits left of / above the button, inside x 54..300", r.x >= 54 && r.x + r.w <= 300 && r.y + r.h <= 372);
    }
    // ---- GuiTip: probe change key ----
    {
        guitip::ProbeSnap last; guitip::Mgr g; g.ok = true;
        LCHECK("probeChanged logs the first sample, then only changes of over/owner/drawn/ctl40", guitip::probeChanged(last, g, 0x10, 0) && !guitip::probeChanged(last, g, 0x10, 0));
        g.owner = 0x10; LCHECK("owner becoming ours is a change", guitip::probeChanged(last, g, 0x10, 0));
        LCHECK("ctl flag 0x40 is a change", guitip::probeChanged(last, g, 0x10, 0x40));
    }

    // ---- SafeRead: every hook starts with beginScope(); readCreatureTag / pending reads / R10 frame reads (ushort feat ids) go through readAt ----
    {
        uintptr_t b = arenaAlloc(32); at<uint32_t>(b, 4) = 0xCAFEF00D; at<uint16_t>(b, 8) = 0xFFFE;
        saferead::beginScope();
        uint32_t v = 0; unsigned short us = 0;
        LCHECK("readAt delivers a uint32 and a ushort (featId) from readable memory", saferead::readAt(1u, b, 4, &v) && v == 0xCAFEF00Du && saferead::readAt<unsigned short>(1u, b, 8, &us) && us == 0xFFFE);
        v = 7; LCHECK("readAt on base 0 / below 0x10000 is a miss that leaves the pre-seeded value", !saferead::readAt(1u, 0, 0x68, &v) && !saferead::readAt(1u, 0x20, 0, &v) && v == 7);
        LCHECK("probeRead rejects null (readCreatureTag treats it as empty tag = PC)", !saferead::probeRead(1u, nullptr, 4) && saferead::probeRead(1u, reinterpret_cast<const void*>(b), 16));
    }

    // ---- GuiKit helpers the UI uses ----
    LEQ("panel control list pointer / count offsets 0x24 / 0x28", guikit::Panel_CtlArray * 100 + guikit::Panel_CtlCount, 0x24 * 100 + 0x28);
    LEQ("control flags offset 0x48 (bit1 visible, bit5 click-through)", guikit::Ctl_Flags, 0x48);
    {
        uintptr_t k = arenaAlloc(0x200);
        at<int>(k, 0x48) = 0x20;
        LCHECK("setVisible(true) sets bit1 and clears bit5; false does the reverse", guikit::setVisible(k, true) && at<int>(k, 0x48) == 0x2 && guikit::isVisible(k) && guikit::setVisible(k, false) && at<int>(k, 0x48) == 0x20 && !guikit::isVisible(k));
        guikit::setVisible(k, true);
        LCHECK("setClickable(false) sets only bit5 (tooltip box never swallows clicks), leaving the visible bit", guikit::setClickable(k, false) && at<int>(k, 0x48) == 0x22 && guikit::isVisible(k));
        at<int>(k, 4) = 10; at<int>(k, 8) = 20; at<int>(k, 0xC) = 30; at<int>(k, 0x10) = 40;
        int x, y, w, h;
        LCHECK("getExtent reads x,y,w,h at +4/+8/+0xC/+0x10", guikit::getExtent(k, &x, &y, &w, &h) && x == 10 && y == 20 && w == 30 && h == 40);
        LCHECK("hit: inside, exclusive far edges, and only while visible", guikit::hit(k, 10, 20) && guikit::hit(k, 39, 59) && !guikit::hit(k, 40, 30) && !guikit::hit(k, 25, 60) && (guikit::setVisible(k, false), !guikit::hit(k, 20, 30)));
        // scale: reference label px rect (100,50,800,600) for units (10,20,400,300) -> 2x, origin (100,50); rect edges are rounded then differenced
        at<int>(k, 4) = 100; at<int>(k, 8) = 50; at<int>(k, 0xC) = 800; at<int>(k, 0x10) = 600;
        const guikit::Scale s = guikit::scaleFromRef(k, 10, 20, 400, 300);
        int rx, ry, rw, rh;
        guikit::unitRect(s, 20, 30, 5, 3, &rx, &ry, &rw, &rh);
        LCHECK("unitRect maps layout units to engine px with the reference label", s.ok && rx == 120 && ry == 70 && rw == 10 && rh == 6);
    }
    {
        uintptr_t panel = arenaAlloc(0x100), arr = arenaAlloc(64 * 4), ctl = arenaAlloc(0x200);
        at<uint32_t>(panel, 0x24) = (uint32_t)arr; at<int>(panel, 0x28) = 3;
        LCHECK("appendToPanel appends at the end of the list and stamps id = index (draw order = append order)", guikit::appendToPanel(panel, ctl) && at<int>(panel, 0x28) == 4 && at<uint32_t>(arr, 3 * 4) == (uint32_t)ctl && at<int>(ctl, 0x54) == 3);
    }
    {   // clickEdge: one click per press, and only while the game window is foreground
        guikit::ClickEdge ce;
        fLmb = false; guikit::clickEdge(ce);
        fLmb = true; LCHECK("clickEdge fires once on the press edge", guikit::clickEdge(ce) && !guikit::clickEdge(ce));
        fLmb = false; guikit::clickEdge(ce); fFg = false; fLmb = true;
        LCHECK("clickEdge never fires while the game is not the foreground window", !guikit::clickEdge(ce));
        fFg = true;
    }
    return kt::Report("common_contract_test (level_up_override)");
}
