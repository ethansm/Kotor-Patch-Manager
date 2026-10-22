// HW37 host tests for the residual / via clock that Holster.cpp keeps in residDrawWindow, residStowWindow, viaProgress and the guards of
// carryShownBase. The copies live in HolsterSlide.h (slideResidBranch, slideResidDrawWindow, slideResidStowWindow, slideViaProgress); the
// via bump is HolsterReach.h reachViaOffset. Build/run: python3 scripts/run_tests.py holster_viaclock
#include "HolsterSlide.h"
#include "HolsterReach.h"
#include "check.h"
#include <cmath>
#include <stdint.h>

using namespace hs;

static const uint32_t kRes = kReachResidMs;   // 240: the default g_reachResidMs
static const float kOut = kReachViaOutM;      // 0.12 m: the default reach_via_out_m

static bool closeF(float a, float b) { return std::fabs(a - b) <= 1e-5f; }
static bool inUnit(float v) { return std::isfinite(v) && v >= 0.f && v <= 1.f; }

// One frame of the clock, named as the call sites in Holster.cpp name their inputs. Defaults: a draw in the hand carry (dir 1, from 0,
// carryW 0.5), capture on at tick 0, slide length 1000, residual window kRes.
struct Frame {
    bool active = true;
    uint32_t res = kRes;
    float carryW = 0.5f, oriF = 0.f;
    bool exOn = false;
    bool dir = true;
    float from = 0.f, carryFrom = 0.f;
    uint32_t now = 0, t0 = 0, len = 1000;
    float grab = 0.f;
    bool capOn = true;
    uint32_t capStart = 0, capMs = kRes;
};
static float sProg(const Frame& f) {
    return slideViaProgress(f.active, f.res, f.carryW, f.oriF, f.exOn, f.dir, f.from, f.carryFrom, f.now, f.t0, f.len, f.grab, f.capOn, f.capStart, f.capMs);
}
static SlideResidBranch brOf(const Frame& f) { return slideResidBranch(f.dir, f.from, f.carryFrom, f.carryW, f.active, f.res); }
// A stow frame: dir 0, carryFrom 1 (a full stow), slide started at tick t0 = 2000, len 1000, grab 0.
static Frame stowAt(uint32_t now) { Frame f; f.dir = false; f.carryFrom = 1.f; f.t0 = 2000; f.now = now; return f; }

// (a) s == 0 outside the carry window: reach (carry weight still 0), release (plan off, or the stow has not started / has ended).
static void testZeroOutsideWindow() {
    Frame dReach; dReach.carryW = 0.f; dReach.now = 120;          // draw reach phase: capture on, inside the window, carry 0
    KT_CHECK(sProg(dReach) == 0.f);
    Frame dRel; dRel.active = false; dRel.now = 120;              // draw release: plan inactive
    KT_CHECK(sProg(dRel) == 0.f);
    Frame dStale; dStale.now = 1101;                              // draw release: capture older than len + 100
    KT_CHECK(sProg(dStale) == 0.f);
    Frame sReach = stowAt(2880); sReach.carryW = 0.f;             // stow reach phase
    KT_CHECK(sProg(sReach) == 0.f);
    KT_CHECK(sProg(stowAt(2000)) == 0.f);                         // stow before the window: t = carry length >= ms, still in the hand
    KT_CHECK(sProg(stowAt(3050)) == 0.f);                         // stow release: t < 0
    Frame sOff = stowAt(2880); sOff.active = false;               // stow release: plan inactive
    KT_CHECK(sProg(sOff) == 0.f);
}

// (b) draw peak: the via bump is outM at t = ms / 2, and the draw progress reads 0.5 there. The progress is linear over the window.
static void testDrawPeak() {
    Frame d; d.capStart = 5000; d.now = 5120;                     // t = 120 = ms / 2 (ms = capMs = 240)
    KT_CHECK(closeF(sProg(d), 0.5f));
    KT_CHECK(closeF(reachViaOffset(120.f, 240.f, kOut), kOut));
    KT_CHECK(closeF(reachViaOffset(sProg(d) * 240.f, 240.f, kOut), kOut));   // the bump read through the progress
    bool okLin = true;
    for (uint32_t k = 0; k <= 240; ++k) {
        Frame f; f.capStart = 5000; f.now = 5000 + k;
        if (!closeF(sProg(f), static_cast<float>(k) / 240.f)) okLin = false;
    }
    KT_CHECK(okLin);
    Frame first; first.capStart = 5000; first.now = 5000;         // first tick of the capture: t = 0
    KT_CHECK(sProg(first) == 0.f);
}

// (c) stow mirror: the stow runs 1 -> 0 as its time left falls. s = 0.5 at t = ms / 2 and the bump is outM there.
static void testStowPeak() {
    Frame s = stowAt(2880);                                       // carry 1000, ms = min(1000, 240) = 240, t = 1000 - 880 = 120
    KT_CHECK(closeF(sProg(s), 0.5f));
    KT_CHECK(closeF(reachViaOffset(120.f, 240.f, kOut), kOut));
    const float a = sProg(stowAt(2800)), b = sProg(stowAt(2880)), c = sProg(stowAt(2960));
    KT_CHECK(a > b && b > c);                                     // falling as the time left runs down
    KT_CHECK(closeF(a, 200.f / 240.f) && closeF(c, 40.f / 240.f));
}

// (d) residMs == 0: no residual branch in either direction; the fallback is the plain blend (fRot = carryW, or oriF with exOn).
static void testResidZeroFallsBack() {
    Frame d; d.res = 0; d.now = 60;                               // a draw would read 0.25 here; the blend reads carryW 0.5
    KT_CHECK(brOf(d) == ResidNone);
    KT_CHECK(closeF(sProg(d), 0.5f));
    Frame s = stowAt(2960); s.res = 0;                            // a stow would read 40 / 240 here
    KT_CHECK(brOf(s) == ResidNone);
    KT_CHECK(closeF(sProg(s), 0.5f));
    Frame e = d; e.exOn = true; e.oriF = 0.3f;                    // exOn: fRot is oriF while carryW > 0
    KT_CHECK(closeF(sProg(e), 0.3f));
}

// Literal re-typing of the carryShownBase guards in Holster.cpp (the oracle for (e)). 0 = blend, 1 = draw, 2 = stow.
static int oracleBranch(bool dir, float from, float carryFrom, float carryW, bool active, uint32_t resMs, bool qok) {
    if (resMs == 0 || !active || !(carryW > 0.f) || !qok) return 0;
    if (dir) {
        if (from > 0.001f) return 0;
        return 1;
    }
    if (!(carryFrom >= 0.999f)) return 0;
    return 2;
}

// (e) grid: slideResidBranch against the oracle. NaN in from and carryFrom is included: both read it the way the source does.
static void testBranchGridMatchesGuards() {
    const float froms[] = {0.f, 0.0005f, 0.001f, 0.0011f, 0.5f, 1.f, -0.3f, NAN};
    const float carryFroms[] = {0.f, 0.5f, 0.998f, 0.999f, 1.f, 2.f, NAN};
    const float carryWs[] = {-1.f, 0.f, 1e-6f, 0.5f, 1.f, NAN};
    const uint32_t ress[] = {0u, 1u, 240u};
    int n = 0, bad = 0;
    for (int dir = 0; dir < 2; ++dir)
    for (int act = 0; act < 2; ++act)
    for (int qok = 0; qok < 2; ++qok)
    for (float fr : froms)
    for (float cf : carryFroms)
    for (float cw : carryWs)
    for (uint32_t rs : ress) {
        ++n;
        const int want = oracleBranch(dir != 0, fr, cf, cw, act != 0, rs, qok != 0);
        const int got = static_cast<int>(slideResidBranch(dir != 0, fr, cf, cw, act != 0, rs, qok != 0));
        if (want != got) ++bad;
    }
    KT_CHECK(n == 8064);
    KT_CHECK(bad == 0);
}

// Literal copy of Holster.cpp viaProgress with residDrawWindow and residStowWindow inlined (the oracle for (e2)).
static float oracleVia(const Frame& f) {
    if (!f.active) return 0.f;
    if (f.res != 0 && f.carryW > 0.f) {
        if (f.dir) {
            if (f.from <= 0.001f) {
                if (!f.capOn || static_cast<int32_t>(f.now - f.capStart) > static_cast<int32_t>(f.len) + 100) return 0.f;
                const float t = static_cast<float>(static_cast<int32_t>(f.now - f.capStart));
                const float ms = static_cast<float>(f.capMs);
                return ms > 0.f ? slideClamp01(t / ms) : 0.f;
            }
        } else if (f.carryFrom >= 0.999f) {
            const int32_t d = static_cast<int32_t>(f.now - f.t0);
            const float gg = f.grab > 0.6f ? 0.6f : (f.grab > 0.f ? f.grab : 0.f);
            const float carryLen = (1.f - gg) * static_cast<float>(f.len);
            const float resMs = static_cast<float>(f.res);
            const float ms = carryLen < resMs ? carryLen : resMs;
            const float t = carryLen - static_cast<float>(d);
            return (ms > 1.f && t < ms) ? slideClamp01(t / ms) : 0.f;
        }
    }
    return slideClamp01(f.exOn ? (f.carryW > 0.f ? f.oriF : 0.f) : f.carryW);
}

// (e2) grid: slideViaProgress against the oracle over finite from values. viaProgress tests from <= 0.001f and carryShownBase tests
// !(from > 0.001f); they differ only for NaN from (NOTES). Covers draw, stow, blend, the stale cut and the window ends.
static void testViaGridMatchesOriginal() {
    const float froms[] = {0.f, 0.0005f, 0.5f, 1.f};
    const float carryFroms[] = {0.f, 0.998f, 0.999f, 1.f};
    const float carryWs[] = {0.f, 0.5f, 1.f};
    const float grabs[] = {0.f, 0.3f, 0.9f};
    const float oriFs[] = {0.f, 0.7f};
    const uint32_t lens[] = {0u, 1000u};
    const uint32_t ress[] = {0u, 240u};
    const uint32_t capMss[] = {0u, 60u, 240u};
    const int32_t offs[] = {-10, 0, 120, 500, 1200};             // now = 2000 + off; the capture and the stow t0 both start at 2000
    int n = 0, bad = 0, nonzero = 0;
    for (int act = 0; act < 2; ++act)
    for (int dir = 0; dir < 2; ++dir)
    for (int ex = 0; ex < 2; ++ex)
    for (int cap = 0; cap < 2; ++cap)
    for (float fr : froms)
    for (float cf : carryFroms)
    for (float cw : carryWs)
    for (float gr : grabs)
    for (float of : oriFs)
    for (uint32_t ln : lens)
    for (uint32_t rs : ress)
    for (uint32_t cm : capMss)
    for (int32_t o : offs) {
        Frame f;
        f.active = act != 0; f.dir = dir != 0; f.exOn = ex != 0; f.capOn = cap != 0;
        f.from = fr; f.carryFrom = cf; f.carryW = cw; f.grab = gr; f.oriF = of;
        f.len = ln; f.res = rs; f.capMs = cm; f.t0 = 2000; f.capStart = 2000; f.now = 2000 + static_cast<uint32_t>(o);
        ++n;
        const float got = sProg(f);
        if (got != oracleVia(f)) ++bad;
        if (got > 0.f) ++nonzero;
    }
    KT_CHECK(n == 276480);
    KT_CHECK(bad == 0);
    KT_CHECK(nonzero > 10000);                                    // the grid is not vacuous: many frames carry a non-zero progress
}

// (f) NaN and infinite inputs, and len == 0: no NaN out, s in [0, 1].
static void testNaNFreeAndZeroLen() {
    const float bad[] = {NAN, INFINITY, -INFINITY, -1.f, 2.f};
    bool ok = true;
    for (float v : bad) {
        Frame d1; d1.from = v; d1.now = 120;                      // NaN from reads as a draw (carryShownBase); still in [0, 1]
        ok = ok && inUnit(sProg(d1));
        Frame d2; d2.carryW = v; d2.now = 120;
        ok = ok && inUnit(sProg(d2));
        Frame d3; d3.dir = false; d3.carryFrom = 0.5f; d3.exOn = true; d3.oriF = v;   // blend fRot = oriF
        ok = ok && inUnit(sProg(d3));
        Frame s1 = stowAt(2880); s1.grab = v;
        ok = ok && inUnit(sProg(s1));
        Frame s2 = stowAt(2880); s2.carryFrom = v;
        ok = ok && inUnit(sProg(s2));
        Frame s3 = stowAt(2880); s3.carryW = v;
        ok = ok && inUnit(sProg(s3));
    }
    KT_CHECK(ok);
    Frame gn = stowAt(2880); gn.grab = NAN;                       // NaN grab reads as 0: the full carry, t = 120, s = 0.5
    KT_CHECK(closeF(sProg(gn), 0.5f));
    float t = 0.f, ms = 0.f;
    slideResidStowWindow(2000, 2000, 0, NAN, 240, t, ms);         // empty carry: ms 0, finite t
    KT_CHECK(std::isfinite(t) && ms == 0.f);
    Frame z = stowAt(2000); z.len = 0;                            // stow with len 0: ms 0, s 0
    KT_CHECK(sProg(z) == 0.f);
    Frame zd; zd.len = 0; zd.capMs = 0; zd.now = 50;              // draw with an empty residual window: s 0
    KT_CHECK(sProg(zd) == 0.f);
    Frame zs; zs.len = 0; zs.now = 100;                           // len 0: the stale cut is len + 100 = 100, not yet cut
    KT_CHECK(closeF(sProg(zs), 100.f / 240.f));
    Frame zt; zt.len = 0; zt.now = 101;                           // one tick past the cut
    KT_CHECK(sProg(zt) == 0.f);
}

// (g) edge cases the carry can reach.
static void testEdgeCases() {
    // Reversal mid-window: a stow reversed into a draw half way (dir 1, from 0.4) and a draw reversed into a stow half way (dir 0,
    // carryFrom 0.3 < 0.999). Both fall to the plain blend (fRot = carryW 0.3), not to the residual peak (0.5).
    Frame rv; rv.dir = true; rv.from = 0.4f; rv.carryFrom = 0.4f; rv.carryW = 0.3f; rv.now = 120;
    KT_CHECK(brOf(rv) == ResidNone);
    KT_CHECK(closeF(sProg(rv), 0.3f));
    Frame rw = stowAt(2880); rw.carryFrom = 0.3f; rw.carryW = 0.3f;
    KT_CHECK(brOf(rw) == ResidNone);
    KT_CHECK(closeF(sProg(rw), 0.3f));

    // resMs > carry length: the window is clamped to the carry. len 100, grab 0: ms = 100, t = 50 at now 2050, s = 0.5.
    float t = 0.f, ms = 0.f;
    slideResidStowWindow(2050, 2000, 100, 0.f, 240, t, ms);
    KT_CHECK(closeF(ms, 100.f) && closeF(t, 50.f));
    Frame sh; sh.dir = false; sh.carryFrom = 1.f; sh.t0 = 2000; sh.len = 100; sh.now = 2050;
    KT_CHECK(closeF(sProg(sh), 0.5f));
    // grab 0.5 halves the carry to 50: ms = 50, t = 25 at now 2025, s = 0.5.
    slideResidStowWindow(2025, 2000, 100, 0.5f, 240, t, ms);
    KT_CHECK(closeF(ms, 50.f) && closeF(t, 25.f));
    Frame sg = sh; sg.grab = 0.5f; sg.now = 2025;
    KT_CHECK(closeF(sProg(sg), 0.5f));
    // grab above 0.6 clamps to 0.6: grab 0.9 leaves a carry of 0.4 * 1000 = 400, so ms = 240 and t = 400 - 300 = 100 at now 2300.
    // (0.6f and 1 - 0.6f are inexact in float, so t carries about 2e-5 of error: checked at 1e-3.)
    slideResidStowWindow(2300, 2000, 1000, 0.9f, 240, t, ms);
    KT_CHECK(closeF(ms, 240.f) && std::fabs(t - 100.f) < 1e-3f);
    Frame gc = stowAt(2300); gc.grab = 0.9f;
    KT_CHECK(closeF(sProg(gc), 100.f / 240.f));
    // A long carry keeps the residual length: len 1000, ms = 240, t = 120 at now 2880.
    slideResidStowWindow(2880, 2000, 1000, 0.f, 240, t, ms);
    KT_CHECK(closeF(ms, 240.f) && closeF(t, 120.f));

    // Capture not yet on: carryShownBase still takes the draw branch (it captures this frame), but the progress is 0.
    Frame nc; nc.capOn = false; nc.now = 120;
    KT_CHECK(brOf(nc) == ResidDraw);
    KT_CHECK(sProg(nc) == 0.f);
    // The draw clock does not close at t = ms: it stays at 1 until the capture is older than len + 100, then reads 0.
    Frame sc; sc.capStart = 0; sc.len = 1000; sc.now = 1100;
    KT_CHECK(sProg(sc) == 1.f);
    sc.now = 1101;
    KT_CHECK(sProg(sc) == 0.f);

    // Stow t >= ms: the weapon is still in the hand, s = 0. The check is strict: t == ms gives 0, t = ms - 1 gives 239 / 240.
    KT_CHECK(sProg(stowAt(2000)) == 0.f);
    KT_CHECK(sProg(stowAt(2760)) == 0.f);
    KT_CHECK(closeF(sProg(stowAt(2761)), 239.f / 240.f));
    Frame sn = stowAt(2000); sn.len = 1;                          // carry of 1 ms: ms = 1 is not > 1
    KT_CHECK(sProg(sn) == 0.f);
}

// (HW44d) reach_via_out_m_<cat>: unset (-1) -> the global value; set -> the category value (0 stays 0, i.e. off for that category); parse clamp 0..0.30, NaN -> 0.
static void testViaOutPerCategory() {
    KT_CHECK(closeF(reachViaOutFor(-1.f, kOut), kOut));                    // unset: byte-identical to the global
    KT_CHECK(closeF(reachViaOutFor(0.20f, kOut), 0.20f));
    KT_CHECK(reachViaOutFor(0.f, kOut) == 0.f);                            // an explicit 0 turns this category's bump off
    KT_CHECK(closeF(reachViaOutClamp(0.20f), 0.20f) && reachViaOutClamp(0.5f) == 0.30f && reachViaOutClamp(-0.1f) == 0.f && reachViaOutClamp(NAN) == 0.f);
    KT_CHECK(closeF(reachViaOffset(120.f, 240.f, reachViaOutFor(-1.f, kOut)), kOut));
    KT_CHECK(closeF(reachViaOffset(120.f, 240.f, reachViaOutFor(0.2f, kOut)), 0.2f));
}

int main() {
    testZeroOutsideWindow();
    testDrawPeak();
    testStowPeak();
    testResidZeroFallsBack();
    testBranchGridMatchesGuards();
    testViaGridMatchesOriginal();
    testNaNFreeAndZeroLen();
    testEdgeCases();
    testViaOutPerCategory();
    return kt::Report("holster_viaclock");
}
