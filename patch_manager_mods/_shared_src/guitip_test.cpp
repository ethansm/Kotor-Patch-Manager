// Host test of GuiTip.h: native tooltip fields on a fake control, the manager snapshot, the hover/fallback state machine, the custom box layout
// and the probe change detector. Fake engine in a MAP_32BIT arena (see guikit_test.cpp). Expected values are independent literals.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "GuiTip.h"
#include "check.h"
#include <sys/mman.h>
#include <stdio.h>

using namespace guitip;
static uint8_t* g_arena = nullptr; static size_t g_used = 0; constexpr size_t ArenaSize = 1u << 20;
static uintptr_t arenaAlloc(size_t n) { n = (n + 15) & ~size_t(15); if (g_used + n > ArenaSize) { printf("arena full\n"); exit(4); } uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used); memset(g_arena + g_used, 0, n); g_used += n; return a; }
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

static int nCtor = 0, nDtor = 0; static void* lastCtorDst = nullptr, *lastDtorDst = nullptr; static char lastText[64];
static void* fExoCtor(void* e, const char* s) { ++nCtor; lastCtorDst = e; snprintf(lastText, sizeof lastText, "%s", s); return e; }
static void fExoDtor(void* e) { ++nDtor; lastDtorDst = e; }

int main() {
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (m == MAP_FAILED) { perror("mmap MAP_32BIT"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;

    // ---- constants (spike section 1: Steam decomp) ----
    KT_CHECK_EQ(Ctl_TipStrRef, 0x28); KT_CHECK_EQ(Ctl_TipText, 0x2C); KT_CHECK_EQ(CtlFlag_TipShown, 0x40u);
    KT_CHECK_EQ(Mgr_Over, 0x08); KT_CHECK_EQ(Mgr_Flags, 0x1C); KT_CHECK_EQ(Mgr_Owner, 0x40); KT_CHECK_EQ(Mgr_Timer, 0x44); KT_CHECK_EQ(Mgr_Appear, 0x48); KT_CHECK_EQ(Mgr_Disabled, 0x4C);
    KT_CHECK_EQ(MgrFlag_TipDrawn, 8u); KT_CHECK_EQ(DefaultDelayMs, 400u); KT_CHECK_EQ(FallbackGraceMs, 250u);
    KT_CHECK(nativeSupported());

    // ---- setNative / clearNative ----
    guikit::g_gk.exoCtor = fExoCtor; guikit::g_gk.exoDtor = fExoDtor;
    uintptr_t ctl = arenaAlloc(0x200);
    at<int>(ctl, 0x28) = 77;                      // a stale strref must be zeroed
    KT_CHECK(setNative(ctl, "line1\nline2"));
    KT_CHECK_EQ(at<int>(ctl, 0x28), 0);
    KT_CHECK_EQ(nDtor, 1); KT_CHECK_EQ(nCtor, 1);
    KT_CHECK(lastDtorDst == reinterpret_cast<void*>(ctl + 0x2C) && lastCtorDst == reinterpret_cast<void*>(ctl + 0x2C));
    KT_CHECK(!strcmp(lastText, "line1\nline2"));
    KT_CHECK(clearNative(ctl)); KT_CHECK(!strcmp(lastText, "")); KT_CHECK_EQ(nCtor, 2);
    KT_CHECK(!setNative(0, "x")); KT_CHECK(!setNative(ctl, nullptr));
    KT_CHECK(!setNative(0x10, "x"));              // unreadable (low page): no write, no engine call
    KT_CHECK_EQ(nCtor, 2);
    { auto* sv = guikit::g_gk.exoCtor; guikit::g_gk.exoCtor = nullptr; KT_CHECK(!setNative(ctl, "x")); guikit::g_gk.exoCtor = sv; }
    { k2addr::setForTest(&kGog); KT_CHECK(!nativeSupported()); at<int>(ctl, 0x28) = 5; KT_CHECK(!setNative(ctl, "x")); KT_CHECK_EQ(at<int>(ctl, 0x28), 5); KT_CHECK_EQ(nCtor, 2);
      k2addr::setForTest(nullptr); KT_CHECK(!setNative(ctl, "x")); k2addr::clearForTest(); KT_CHECK(setNative(ctl, "x")); }

    // ---- readMgr / delayMs ----
    uintptr_t cell = arenaAlloc(16), mgr = arenaAlloc(0x100);
    guikit::g_gk.guiSoundMgrAddr = cell;
    Mgr mg; mg.ok = true;
    KT_CHECK(!readMgr(&mg)); KT_CHECK(!mg.ok);    // cell holds a null manager
    at<uint32_t>(cell, 0) = (uint32_t)mgr;
    at<uint32_t>(mgr, 0x08) = 0x1111; at<uint32_t>(mgr, 0x40) = 0x2222; at<float>(mgr, 0x44) = 0.25f; at<float>(mgr, 0x48) = 0.5f; at<uint32_t>(mgr, 0x1C) = 8; at<uint32_t>(mgr, 0x4C) = 1;
    KT_CHECK(readMgr(&mg) && mg.ok);
    KT_CHECK(mg.over == 0x1111 && mg.owner == 0x2222 && mg.timer == 0.25f && mg.appear == 0.5f && mg.flags == 8 && mg.disabled == 1);
    KT_CHECK(!readMgr(nullptr));
    KT_CHECK_EQ(delayMs(mg), 500u);
    mg.appear = 0.f; KT_CHECK_EQ(delayMs(mg), 400u);
    mg.appear = 10.f; KT_CHECK_EQ(delayMs(mg), 400u);
    mg.appear = 9.99f; KT_CHECK_EQ(delayMs(mg), 9990u);
    mg.appear = -1.f; KT_CHECK_EQ(delayMs(mg), 400u);
    mg.appear = 0.01f; KT_CHECK_EQ(delayMs(mg), 10u);
    mg.appear = 0.5f; mg.ok = false; KT_CHECK_EQ(delayMs(mg), 400u);
    // manager pointer non-null but unreadable (0x20 is in the low page): false, and the out struct is reset to its defaults.
    // (A page that is readable up to Flags and unreadable at Disabled cannot be built here: the host VirtualQuery stub is all-or-nothing.)
    {
        at<uint32_t>(cell, 0) = 0x20;
        Mgr p; p.ok = true; p.over = 0x7777; p.owner = 0x8888; p.timer = 1.f; p.appear = 2.f; p.flags = 3; p.disabled = 4;
        KT_CHECK(!readMgr(&p));
        KT_CHECK(!p.ok && p.over == 0 && p.owner == 0 && p.timer == 0.f && p.appear == 0.f && p.flags == 0 && p.disabled == 0);
        at<uint32_t>(cell, 0) = (uint32_t)mgr;
    }
    guikit::g_gk.guiSoundMgrAddr = 0; KT_CHECK(!readMgr(&mg));

    // ---- state machine ----
    {
        State s;
        KT_CHECK(!tick(s, -1, false, 1000, 400) && s.key == -1);
        KT_CHECK(!tick(s, 3, false, 1000, 400) && s.key == 3 && s.since == 1000);      // first sight starts the clock
        KT_CHECK(!tick(s, 3, false, 1000 + 400 + 249, 400));                           // delay + grace - 1
        KT_CHECK(tick(s, 3, false, 1000 + 400 + 250, 400) && s.boxShown);              // exactly delay + grace
        KT_CHECK(tick(s, 3, false, 5000, 400));                                        // stays shown
        KT_CHECK(!tick(s, 4, false, 5001, 400) && !s.boxShown && s.since == 5001);     // key change hides + restarts
        KT_CHECK(!tick(s, 4, false, 5001 + 649, 400)); KT_CHECK(tick(s, 4, false, 5001 + 650, 400));
        KT_CHECK(!tick(s, -1, false, 9000, 400) && !s.boxShown && s.key == -1);        // no hover hides
        // grace follows the delay argument
        State d; tick(d, 1, false, 0, 1000); KT_CHECK(!tick(d, 1, false, 1249, 1000)); KT_CHECK(tick(d, 1, false, 1250, 1000));
        // native owns the hovered control: never the custom box, sticky
        State n; tick(n, 2, false, 0, 400);
        KT_CHECK(!tick(n, 2, true, 100, 400) && n.nativeSeen && !n.boxShown);
        KT_CHECK(!tick(n, 2, false, 99999, 400));                                      // owner gone again, long after the delay: still no box
        KT_CHECK(!tick(n, 7, false, 100000, 400)); KT_CHECK(!tick(n, 7, false, 200000, 400));   // other controls too (engine works)
        // custom box up, then the engine takes over: box hidden
        State u; tick(u, 2, false, 0, 400); KT_CHECK(tick(u, 2, false, 650, 400)); KT_CHECK(!tick(u, 2, true, 651, 400) && !u.boxShown && u.nativeSeen);
        // owner flag on the tick of a key change does not count (stale owner)
        State k; tick(k, 1, false, 0, 400); KT_CHECK(!tick(k, 2, true, 10, 400) && !k.nativeSeen);
        // reset hides, restarts the clock, keeps nativeSeen
        State r; tick(r, 5, false, 0, 400); KT_CHECK(tick(r, 5, false, 700, 400)); reset(r);
        KT_CHECK(!r.boxShown && r.key == -1); KT_CHECK(!tick(r, 5, false, 701, 400) && r.since == 701); KT_CHECK(!tick(r, 5, false, 701 + 649, 400)); KT_CHECK(tick(r, 5, false, 701 + 650, 400));
        State rs; rs.nativeSeen = true; reset(rs); KT_CHECK(rs.nativeSeen);
        // millisecond counter wrap
        State w; tick(w, 1, false, 0xFFFFFF00u, 400); KT_CHECK(!tick(w, 1, false, 0xFFFFFF00u + 649, 400)); KT_CHECK(tick(w, 1, false, 0xFFFFFF00u + 650, 400));
    }

    // ---- measure / layout ----
    {
        int l = -1, mx = -1;
        measure("abc\nde\nfghij", &l, &mx); KT_CHECK(l == 3 && mx == 5);
        measure("one line", &l, &mx); KT_CHECK(l == 1 && mx == 8);
        measure("", &l, &mx); KT_CHECK(l == 0 && mx == 0);
        measure(nullptr, &l, &mx); KT_CHECK(l == 0 && mx == 0);
        measure("a\n\nb", &l, &mx); KT_CHECK(l == 3 && mx == 1);
        // null out-params are optional (GuiTip.h:99-100): must not crash, must not write, and one null must not suppress the other
        l = -1; mx = -1;
        measure("a\nb", nullptr, nullptr); KT_CHECK(l == -1 && mx == -1);
        measure("a\nb", nullptr, &mx); KT_CHECK(l == -1 && mx == 1);
        mx = -1; measure("a\nb", &l, nullptr); KT_CHECK(l == 2 && mx == -1);
        measure("abc\nd", nullptr, &mx); KT_CHECK(mx == 3);
        const Metrics mt = {6, 12, 6, 5};
        const Region reg = {54, 134, 300, 470};
        // box below the control: w = 20*6+12 = 132, h = 2*12+10 = 34
        Rect r = layout(2, 20, mt, {62, 162, 230, 24}, reg, 4);
        KT_CHECK(r.w == 132 && r.h == 34 && r.x == 62 && r.y == 190);
        // right edge clamp: control near x1
        r = layout(2, 20, mt, {250, 162, 40, 24}, reg, 4); KT_CHECK(r.x == 300 - 132 && r.y == 190);
        // left clamp: control left of the region
        r = layout(2, 20, mt, {10, 162, 40, 24}, reg, 4); KT_CHECK(r.x == 54);
        // flip above: control at the bottom (y 442..468): below would be 472 + 34 > 470
        r = layout(2, 20, mt, {62, 442, 72, 26}, reg, 4); KT_CHECK(r.y == 442 - 34 - 4 && r.y + r.h <= 442);
        // exactly fits below (y + h == y1): no flip
        r = layout(2, 20, mt, {62, 400, 72, 32}, reg, 4); KT_CHECK(r.y == 436 && r.y + r.h == 470);
        // one unit too much: flips
        r = layout(2, 20, mt, {62, 401, 72, 32}, reg, 4); KT_CHECK(r.y == 401 - 34 - 4);
        // top clamp: flipped box would leave the region's top
        r = layout(3, 20, mt, {62, 140, 72, 20}, {54, 134, 300, 170}, 4); KT_CHECK(r.y == 134);
        // closed page (cfg button at x 362): region ends above y 372, box goes left + above
        r = layout(3, 31, mt, {362, 372, 94, 26}, {54, 134, 300, 372}, 4); KT_CHECK(r.x + r.w == 300 && r.y + r.h <= 372 && r.y == 372 - r.h - 4);
        // oversize box wider than the region: left edge pinned at x0
        r = layout(1, 60, mt, {100, 200, 20, 20}, reg, 4); KT_CHECK(r.x == 54 && r.w == 372);
    }

    // ---- probe ----
    {
        ProbeSnap last; Mgr g; g.ok = true; const uintptr_t c = 0x5000;
        KT_CHECK(probeChanged(last, g, c, 0));                     // first sample always logs
        KT_CHECK(!probeChanged(last, g, c, 0));
        g.over = c; KT_CHECK(probeChanged(last, g, c, 0)); KT_CHECK(!probeChanged(last, g, c, 0));
        g.owner = c; KT_CHECK(probeChanged(last, g, c, 0));
        g.flags = 8; KT_CHECK(probeChanged(last, g, c, 0));
        KT_CHECK(probeChanged(last, g, c, 0x40)); KT_CHECK(!probeChanged(last, g, c, 0x40 | 0x2));   // other control flag bits do not count
        g.timer = 3.f; g.appear = 1.f; g.disabled = 1; KT_CHECK(!probeChanged(last, g, c, 0x40));      // timers / disabled are not part of the change key
        g.over = 0x9999; KT_CHECK(probeChanged(last, g, c, 0x40));                                      // another control under the mouse
        g.ok = false; KT_CHECK(probeChanged(last, g, c, 0x40) && last.over == 0 && last.owner == 0 && last.shown == 0 && last.ctl40 == 1);
    }
    return kt::Report("guitip_test");
}
