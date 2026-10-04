// Regression tests for BuffHudLogic.h (bug F7: unclamped `len += snprintf(buf + len, cap - len, ...)` idiom) plus T1 tests of the
// logic extracted from BuffDurationHUD.cpp (doc 20): engine arrow counter bump, icon column geometry, menu portrait gates.
// Build/run: python3 scripts/run_tests.py -k buff_hud
// Expected values in the geometry tests are hand-computed (see the comment on each case), not read back from the function.
#include "BuffHudLogic.h"
#include "K2Addr.h"
#include "check.h"
#include <climits>
#include <cstdlib>
#include <string>

namespace {

// ICONCACHE entry format, copied verbatim from BuffDurationHUD.cpp.
const char* kIconFmt = " [%d id=%d f14=%d f1c=%d]";

// Backing store bigger than the logical buffer; bytes past `cap` are canaries that must never change.
struct Guarded {
    static constexpr int Pad = 128;
    unsigned char raw[1024 + Pad];
    int cap;
    explicit Guarded(int c) : cap(c) { memset(raw, 0xAA, sizeof raw); }
    char* buf() { return (char*)raw; }
    bool canaryIntact() const {
        for (int i = cap; i < cap + Pad; ++i) if (raw[i] != 0xAA) return false;
        return true;
    }
};

}  // namespace

// ================================================================ engine arrow counters
static void testArrowGates() {
    // slot+0x590 gate: only bit 1 (value 2) opens it.
    KT_CHECK(!bh::arrowGateOpen(0));
    KT_CHECK(!bh::arrowGateOpen(1));
    KT_CHECK(bh::arrowGateOpen(2));
    KT_CHECK(bh::arrowGateOpen(3));
    KT_CHECK(!bh::arrowGateOpen(4));
    KT_CHECK(!bh::arrowGateOpen(~2));        // every bit but bit 1
    KT_CHECK(bh::arrowGateOpen(-1));
    KT_CHECK(bh::arrowGateOpen(INT_MIN | 2));
    KT_CHECK(!bh::arrowGateOpen(INT_MIN));

    // Overlay words: ANY of the three having bit 1 hides the arrows (dead/disabled).
    KT_CHECK(!bh::arrowsHidden(0, 0, 0));
    KT_CHECK(bh::arrowsHidden(2, 0, 0));
    KT_CHECK(bh::arrowsHidden(0, 2, 0));
    KT_CHECK(bh::arrowsHidden(0, 0, 2));
    KT_CHECK(bh::arrowsHidden(2, 2, 2));
    KT_CHECK(bh::arrowsHidden(0x10, 0x20, 0x3));          // bit 1 set in the third word only
    KT_CHECK(!bh::arrowsHidden(1, 0, 4));                 // other bits are irrelevant
    KT_CHECK(!bh::arrowsHidden(0x1, 0x4, 0x8));
    KT_CHECK(!bh::arrowsHidden(~2, ~2, ~2));
    KT_CHECK(bh::arrowsHidden(-1, 0, 0));
    KT_CHECK(!bh::arrowsHidden(INT_MIN, INT_MIN, INT_MIN));
}

static void testBumpArrowCounters() {
    using bh::bumpArrowCounters;
    // The bug (bc34464): a companion whose creature+0x148 list holds buffs but whose engine icon cache is empty has raw 0,
    // so the engine never runs the row draw. Tracked > 0 and raw == 0 -> raised to 1 so native code draws one arrow.
    {
        bh::ArrowCounters c = bumpArrowCounters(0, 0, 3, 0, false);
        KT_CHECK_EQ(c.buff, 1); KT_CHECK_EQ(c.debuff, 0);
        c = bumpArrowCounters(0, 0, 0, 2, false);
        KT_CHECK_EQ(c.buff, 0); KT_CHECK_EQ(c.debuff, 1);
        c = bumpArrowCounters(0, 0, 16, 16, false);
        KT_CHECK_EQ(c.buff, 1); KT_CHECK_EQ(c.debuff, 1);       // 1, not the tracked count: arrows are identical placeholders
    }
    // Nothing tracked: untouched.
    { bh::ArrowCounters c = bumpArrowCounters(0, 0, 0, 0, false); KT_CHECK_EQ(c.buff, 0); KT_CHECK_EQ(c.debuff, 0); }
    // Engine already non-zero: never lowered, never raised (even when tracked is higher or lower).
    {
        bh::ArrowCounters c = bumpArrowCounters(2, 5, 9, 9, false);
        KT_CHECK_EQ(c.buff, 2); KT_CHECK_EQ(c.debuff, 5);
        c = bumpArrowCounters(7, 1, 1, 0, false);
        KT_CHECK_EQ(c.buff, 7); KT_CHECK_EQ(c.debuff, 1);
        c = bumpArrowCounters(255, 255, 0, 0, false);
        KT_CHECK_EQ(c.buff, 255); KT_CHECK_EQ(c.debuff, 255);
    }
    // Each side independently.
    {
        bh::ArrowCounters c = bumpArrowCounters(4, 0, 1, 1, false);   // buff has engine arrows, debuff needs the bump
        KT_CHECK_EQ(c.buff, 4); KT_CHECK_EQ(c.debuff, 1);
        c = bumpArrowCounters(0, 4, 1, 1, false);
        KT_CHECK_EQ(c.buff, 1); KT_CHECK_EQ(c.debuff, 4);
        c = bumpArrowCounters(0, 4, 1, 0, false);                      // debuff raw non-zero, no debuff tracked
        KT_CHECK_EQ(c.buff, 1); KT_CHECK_EQ(c.debuff, 4);
        c = bumpArrowCounters(3, 0, 0, 0, false);                      // raw buff only, nothing tracked
        KT_CHECK_EQ(c.buff, 3); KT_CHECK_EQ(c.debuff, 0);
    }
    // Hidden overlay (dead/disabled): the engine's choice stands, even when effects are tracked. The caller passes tracked = 0 when
    // hidden, but the function must hold the line on its own too.
    {
        bh::ArrowCounters c = bumpArrowCounters(0, 0, 3, 3, true);
        KT_CHECK_EQ(c.buff, 0); KT_CHECK_EQ(c.debuff, 0);
        c = bumpArrowCounters(0, 0, 0, 0, true);
        KT_CHECK_EQ(c.buff, 0); KT_CHECK_EQ(c.debuff, 0);
        c = bumpArrowCounters(6, 0, 3, 3, true);
        KT_CHECK_EQ(c.buff, 6); KT_CHECK_EQ(c.debuff, 0);
    }
    // Exhaustive over a grid: result == "1 iff (!hidden && tracked > 0 && raw == 0) else raw", and never below raw.
    static const int raws[] = { 0, 1, 2, 3, 127, 128, 254, 255 };
    static const int ns[] = { 0, 1, 2, 15, 16 };
    for (int rb : raws) for (int rd : raws) for (int nb : ns) for (int nd : ns) for (int hid = 0; hid < 2; ++hid) {
        bh::ArrowCounters c = bumpArrowCounters(rb, rd, nb, nd, hid != 0);
        int wantB = (!hid && nb > 0 && rb == 0) ? 1 : rb;
        int wantD = (!hid && nd > 0 && rd == 0) ? 1 : rd;
        if (c.buff != wantB || c.debuff != wantD || c.buff < rb || c.debuff < rd) {
            char w[96]; snprintf(w, sizeof w, "grid raw=(%d,%d) n=(%d,%d) hid=%d", rb, rd, nb, nd, hid);
            kt::Check(w, false, "mismatch");
        } else kt::Check("grid", true);
    }
}

static void testArrowRawTable() {
    // Slot -> last-seen tuple table (8 entries in the DLL; use 3 here).
    bh::ArrowRaw t[3] = {};
    bh::ArrowRaw* a = bh::arrowRawFor(t, 3, 0x1000);
    KT_CHECK(a == &t[0]); KT_CHECK_EQ(a->slot, 0x1000);
    bh::ArrowRaw* b = bh::arrowRawFor(t, 3, 0x2000);
    KT_CHECK(b == &t[1]);
    KT_CHECK(bh::arrowRawFor(t, 3, 0x1000) == a);           // existing entry found, not re-claimed
    KT_CHECK(bh::arrowRawFor(t, 3, 0x2000) == b);
    bh::arrowRawFor(t, 3, 0x3000);
    KT_CHECK(bh::arrowRawFor(t, 3, 0x4000) == nullptr);     // full: no entry (the caller then skips the change log)
    KT_CHECK(bh::arrowRawFor(t, 3, 0x3000) == &t[2]);       // known slots still resolve when full
    // A claimed entry is zeroed even if it held stale data in a free (slot == 0) cell.
    bh::ArrowRaw u[2] = {}; u[0].slot = 0; u[0].buff = 9; u[0].nDebuff = 9;
    bh::ArrowRaw* c = bh::arrowRawFor(u, 2, 0x55);
    KT_CHECK(c == &u[0]); KT_CHECK_EQ(c->buff, 0); KT_CHECK_EQ(c->nDebuff, 0);
    KT_CHECK(bh::arrowRawFor(u, 0, 0x55) == nullptr);        // empty table

    // CHARACTERIZATION: slot 0 is the "free" marker, so asking for slot 0 returns the first free cell (and never claims it).
    bh::ArrowRaw v[2] = {};
    KT_CHECK(bh::arrowRawFor(v, 2, 0) == &v[0]);
    KT_CHECK_EQ(v[0].slot, 0);

    // arrowRawUpdate: change detection + stored values.
    bh::ArrowRaw r = {}; r.slot = 7;
    KT_CHECK(bh::arrowRawUpdate(&r, 1, 0, 2, 0));            // differs from the zeroed state
    KT_CHECK_EQ(r.buff, 1); KT_CHECK_EQ(r.debuff, 0); KT_CHECK_EQ(r.nBuff, 2); KT_CHECK_EQ(r.nDebuff, 0);
    KT_CHECK(!bh::arrowRawUpdate(&r, 1, 0, 2, 0));           // same tuple: not "changed"
    KT_CHECK(bh::arrowRawUpdate(&r, 1, 0, 2, 1));            // any one field
    KT_CHECK(bh::arrowRawUpdate(&r, 1, 1, 2, 1));
    KT_CHECK(bh::arrowRawUpdate(&r, 1, 1, 3, 1));
    KT_CHECK(bh::arrowRawUpdate(&r, 0, 1, 3, 1));
    KT_CHECK_EQ(r.slot, 7);                                  // slot untouched
    // CHARACTERIZATION: counts are truncated to a byte, so 256 looks like 0 (tracked counts are <= 16 in practice).
    bh::ArrowRaw z = {}; z.slot = 1;
    KT_CHECK(!bh::arrowRawUpdate(&z, 0, 0, 256, 0));         // first sighting of an all-zero tuple is "unchanged": never logged
    KT_CHECK_EQ(z.nBuff, 0);
    KT_CHECK(bh::arrowRawUpdate(&z, 0, 0, 257, 0));
    KT_CHECK_EQ(z.nBuff, 1);
    KT_CHECK(!bh::arrowRawUpdate(&z, 0, 0, 1, 0));
}

static void testArrowSuppressed() {
    // First `tracked` arrows of a row are hidden; later (untracked engine) arrows are kept.
    KT_CHECK(bh::arrowSuppressed(0, 1));
    KT_CHECK(!bh::arrowSuppressed(1, 1));                    // index == tracked: kept (boundary)
    KT_CHECK(bh::arrowSuppressed(2, 3));
    KT_CHECK(!bh::arrowSuppressed(3, 3));
    KT_CHECK(!bh::arrowSuppressed(0, 0));                    // nothing tracked: nothing hidden
    KT_CHECK(!bh::arrowSuppressed(5, 0));
}

// ================================================================ column geometry
static void testColumnLayout() {
    using bh::ColumnLayout; using bh::columnLayout;
    // Case A (hand-computed): scale 2.0 -> gap int(2.5)=2, min 10, max 48. Small card 100 x 200: preferred int(100*.19)=19.
    // n=3: perCol 3, fitSize = 198/3-2 = 64 >= 19 -> keeps 19, 1 column. perCol = 198/(19+2) = 9, fit 9, drawN 3.
    {
        ColumnLayout l = columnLayout(3, 1, 0, 100, 200, 2.0f);
        KT_CHECK_EQ(l.gap, 2); KT_CHECK_EQ(l.size, 19); KT_CHECK_EQ(l.cols, 1);
        KT_CHECK_EQ(l.perCol, 9); KT_CHECK_EQ(l.fit, 9); KT_CHECK_EQ(l.drawN, 3);
    }
    // Case B: large card 300 wide: preferred int(66.0)=66 -> clamped to max 48; n=2 in 100 high: fitSize = 98/2-2 = 47 -> size 47.
    {
        ColumnLayout l = columnLayout(2, 0, 1, 300, 100, 2.0f);
        KT_CHECK_EQ(l.size, 47); KT_CHECK_EQ(l.cols, 1); KT_CHECK_EQ(l.perCol, 98 / 49); KT_CHECK_EQ(l.perCol, 2);
        KT_CHECK_EQ(l.fit, 2); KT_CHECK_EQ(l.drawN, 2);
    }
    // Large vs small fraction on the same width: 200 -> large 44 (0.22), small 38 (0.19).
    {
        ColumnLayout lg = columnLayout(1, 0, 1, 200, 400, 2.0f), sm = columnLayout(1, 0, 0, 200, 400, 2.0f);
        KT_CHECK_EQ(lg.size, 44); KT_CHECK_EQ(sm.size, 38);
        KT_CHECK(lg.size > sm.size);
    }
    // isLarge is an int flag: any non-zero value counts as large.
    KT_CHECK_EQ(columnLayout(1, 0, 7, 200, 400, 2.0f).size, 44);
    // Max clamp: huge card never exceeds int(24*scale); min clamp: tiny card never goes below int(5*scale).
    KT_CHECK_EQ(columnLayout(1, 0, 1, 5000, 5000, 2.0f).size, 48);
    KT_CHECK_EQ(columnLayout(1, 0, 1, 5000, 5000, 1.0f).size, 24);
    KT_CHECK_EQ(columnLayout(1, 0, 0, 20, 5000, 2.0f).size, 10);    // int(3.8)=3 -> min 10
    KT_CHECK_EQ(columnLayout(1, 0, 0, 20, 5000, 1.0f).size, 5);
    // gap = int(scale+0.5), at least 1: scale 0.4 -> int(0.9)=0 -> 1; 1.49 -> 1; 1.5 -> 2; 2.5 -> 3.
    KT_CHECK_EQ(columnLayout(1, 0, 0, 100, 200, 0.4f).gap, 1);
    KT_CHECK_EQ(columnLayout(1, 0, 0, 100, 200, 1.49f).gap, 1);
    KT_CHECK_EQ(columnLayout(1, 0, 0, 100, 200, 1.5f).gap, 2);
    KT_CHECK_EQ(columnLayout(1, 0, 0, 100, 200, 2.5f).gap, 3);

    // Case C (extra column): scale 2, small 100 x 60, n=8. cols 1: perCol 8, fitSize = 58/8-2 = 5 < min 10 -> next column;
    // cols 2: size reset to 19, perCol 4, fitSize = 58/4-2 = 12 -> size 12 >= 10 -> stop. perCol = 58/14 = 4, fit 8, all drawn.
    {
        ColumnLayout l = columnLayout(8, 0, 0, 100, 60, 2.0f);
        KT_CHECK_EQ(l.cols, 2); KT_CHECK_EQ(l.size, 12); KT_CHECK_EQ(l.perCol, 4); KT_CHECK_EQ(l.fit, 8); KT_CHECK_EQ(l.drawN, 8);
    }
    // The opposite column's count drives the shared size too: n=1 but nOther=8 gives the same size as n=8.
    {
        ColumnLayout a = columnLayout(1, 8, 0, 100, 60, 2.0f), b = columnLayout(8, 0, 0, 100, 60, 2.0f);
        KT_CHECK_EQ(a.size, b.size); KT_CHECK_EQ(a.cols, b.cols); KT_CHECK_EQ(a.perCol, b.perCol); KT_CHECK_EQ(a.fit, b.fit);
        KT_CHECK_EQ(a.drawN, 1);                              // ...but only draws its own n
    }
    // Case D (overflow past MaxColumns): scale 2, 100 x 60, n=100: cols 1 -> size 5; cols 2: perCol 50, fit -> 58/50-2 < 0 -> 1;
    // cols 3: perCol 34 -> 58/34-2 = -1 -> 1; stop at MaxColumns. perCol = 58/3 = 19, fit 57 < 100, drawN 57.
    {
        ColumnLayout l = columnLayout(100, 0, 0, 100, 60, 2.0f);
        KT_CHECK_EQ(l.cols, bh::MaxColumns); KT_CHECK_EQ(l.size, 1); KT_CHECK_EQ(l.perCol, 19); KT_CHECK_EQ(l.fit, 57); KT_CHECK_EQ(l.drawN, 57);
    }
    // Overflow edges around fit: n = fit-1, fit, fit+1 (Case A geometry: fit 9 for n <= 9 at size 19).
    {
        KT_CHECK_EQ(columnLayout(8, 0, 0, 100, 200, 2.0f).drawN, 8);
        KT_CHECK_EQ(columnLayout(9, 0, 0, 100, 200, 2.0f).drawN, 9);
        ColumnLayout l = columnLayout(10, 0, 0, 100, 200, 2.0f);   // perCol 10: fitSize = 198/10-2 = 17 -> size 17, perCol = 198/19 = 10
        KT_CHECK_EQ(l.size, 17); KT_CHECK_EQ(l.perCol, 10); KT_CHECK_EQ(l.drawN, 10);
    }
    // Empty column: perCol clamps to 1 in the fit loop, drawN 0.
    {
        ColumnLayout l = columnLayout(0, 0, 0, 100, 200, 2.0f);
        KT_CHECK_EQ(l.drawN, 0); KT_CHECK_EQ(l.size, 19); KT_CHECK_EQ(l.cols, 1);
    }
    // Degenerate sizes (the engine could hand us zeros or garbage): never divide by zero, never a size < 1, never perCol < 1.
    {
        ColumnLayout z = columnLayout(3, 3, 0, 0, 0, 2.0f);
        KT_CHECK(z.size >= 1); KT_CHECK(z.perCol >= 1); KT_CHECK(z.cols >= 1 && z.cols <= bh::MaxColumns);
        KT_CHECK_EQ(z.size, 1);        // cardH 0: fitSize = (0-2)/perCol - 2 < 0 -> clamped to 1
        KT_CHECK_EQ(z.perCol, 1);      // (0-2)/(1+2) = 0 -> clamped to 1
        KT_CHECK_EQ(z.drawN, 3);       // fit = perCol*cols = 3 columns
        ColumnLayout n = columnLayout(2, 2, 1, -50, -80, 2.0f);
        KT_CHECK(n.size >= 1); KT_CHECK(n.perCol >= 1);
        ColumnLayout tiny = columnLayout(0, 0, 0, 0, 0, 0.1f);      // smallest scale hudScale() lets through: min size int(0.5) = 0
        KT_CHECK(tiny.perCol >= 1);
    }
    // CHARACTERIZATION: the multi-column loop re-derives the preferred size WITHOUT the MinIconSize clamp, so a degenerate card
    // (cardW 0 at scale 2: preferred int(0)=0) can end at size 0 (zero-sized icons), and at HUD scale < 0.2 (min size int(5*s) = 0)
    // a negative cardW gives size -1 and `(cardH-gap)/(size+gap)` divides by zero. Neither is reachable with real cards (cardW >= ~60,
    // scale 1..3); the division case is deliberately NOT exercised here.
    KT_CHECK_EQ(columnLayout(0, 4, 0, 0, 30, 2.0f).size, 0);
    KT_CHECK_EQ(columnLayout(1, 0, 0, 0, 5000, 2.0f).size, 10);   // but with room to spare the min clamp of the FIRST pass stands

    // Property sweep over a spread of inputs (no negative cardW, see above): invariants only.
    for (int n = 0; n <= 16; ++n) for (int no = 0; no <= 16; no += 4) for (int w : { 0, 1, 30, 100, 400, 2000 })
    for (int h : { -10, 0, 1, 30, 100, 400, 2000 }) for (float sc : { 0.1f, 0.5f, 1.0f, 2.0f, 3.7f, 9.9f }) for (int lg = 0; lg < 2; ++lg) {
        ColumnLayout l = columnLayout(n, no, lg, w, h, sc);
        bool ok = l.gap >= 1 && l.size >= 0 && (w < 30 || l.size >= 1) && l.cols >= 1 && l.cols <= bh::MaxColumns && l.perCol >= 1
               && l.fit == l.perCol * l.cols && l.drawN >= 0 && l.drawN <= n && l.drawN <= l.fit && (l.drawN == n || l.drawN == l.fit);
        kt::Check("layout invariants", ok, ok ? "" : "invariant violated");
    }
}

static void testIconPos() {
    // Case A geometry (gap 2, size 19, perCol 9, card 100 wide). Buffs: left edge, columns step right; debuffs: right edge, step left.
    // An explicit layout so the numbers do not depend on columnLayout.
    bh::ColumnLayout l = { 2, 19, 2, 9, 18, 12 };
    bh::IconPos p = bh::iconPos(l, 0, false, 100);
    KT_CHECK_EQ(p.x, 2); KT_CHECK_EQ(p.y, 2);
    p = bh::iconPos(l, 1, false, 100);   KT_CHECK_EQ(p.x, 2);  KT_CHECK_EQ(p.y, 23);     // next row: + size + gap
    p = bh::iconPos(l, 8, false, 100);   KT_CHECK_EQ(p.x, 2);  KT_CHECK_EQ(p.y, 2 + 8 * 21);
    p = bh::iconPos(l, 9, false, 100);   KT_CHECK_EQ(p.x, 23); KT_CHECK_EQ(p.y, 2);      // second column starts at the top
    p = bh::iconPos(l, 11, false, 100);  KT_CHECK_EQ(p.x, 23); KT_CHECK_EQ(p.y, 2 + 2 * 21);
    p = bh::iconPos(l, 0, true, 100);    KT_CHECK_EQ(p.x, 100 - 2 - 19);                 // 79: right edge minus gap minus size
    KT_CHECK_EQ(p.y, 2);
    p = bh::iconPos(l, 9, true, 100);    KT_CHECK_EQ(p.x, 79 - 21); KT_CHECK_EQ(p.y, 2);  // extra debuff column steps LEFT
    p = bh::iconPos(l, 10, true, 100);   KT_CHECK_EQ(p.x, 58);     KT_CHECK_EQ(p.y, 23);
    KT_CHECK_EQ(bh::RightInset, 0);
    // Whole drawn set stays inside the card for a sweep of real layouts (every icon rect within [0,cardW] x [0,cardH]).
    for (int n = 1; n <= 16; ++n) for (int w : { 60, 100, 250, 400 }) for (int h : { 70, 120, 300 }) for (float sc : { 1.0f, 2.0f }) for (int lg = 0; lg < 2; ++lg) {
        bh::ColumnLayout q = bh::columnLayout(n, 0, lg, w, h, sc);
        bool in = true, same = true;
        for (int i = 0; i < q.drawN; ++i) for (int d = 0; d < 2; ++d) {
            bh::IconPos a = bh::iconPos(q, i, d != 0, w);
            if (a.x < 0 || a.y < 0 || a.x + q.size > w) in = false;      // the icons only need to fit horizontally (y can overflow at size 1)
            if (a.y + q.size > h && q.size > 1) in = false;
            if (i > 0) { bh::IconPos prev = bh::iconPos(q, i - 1, d != 0, w); if (a.x == prev.x && a.y == prev.y) same = false; }
        }
        (void)w;
        kt::Check("icons inside card", in || q.cols == bh::MaxColumns, in ? "" : "icon outside card");
        kt::Check("icons distinct", same);
    }
}

static void testBrightPx() {
    KT_CHECK_EQ(bh::brightPx(20, 1.0f), 20);
    KT_CHECK_EQ(bh::brightPx(20, 0.0f), 0);
    KT_CHECK_EQ(bh::brightPx(20, 0.5f), 10);        // int(10.5)
    KT_CHECK_EQ(bh::brightPx(20, 0.49f), 10);       // int(10.3)
    KT_CHECK_EQ(bh::brightPx(20, 0.4f), 8);         // int(8.5)
    KT_CHECK_EQ(bh::brightPx(20, 0.024f), 0);       // 0.48 + 0.5 < 1: rounds to 0
    KT_CHECK_EQ(bh::brightPx(20, 0.03f), 1);        // 0.6 + 0.5 -> 1
    KT_CHECK_EQ(bh::brightPx(20, 0.99f), 20);       // int(20.3)
    KT_CHECK_EQ(bh::brightPx(20, 1.5f), 20);        // clamped to size
    KT_CHECK_EQ(bh::brightPx(20, 1000.0f), 20);
    KT_CHECK(bh::brightPx(20, -0.5f) <= 0);         // CHARACTERIZATION: no lower clamp; int(-9.5) = -9, the caller skips when not > 0
    KT_CHECK_EQ(bh::brightPx(20, -0.5f), -9);
    KT_CHECK_EQ(bh::brightPx(1, 0.5f), 1);          // int(1.0)
    KT_CHECK_EQ(bh::brightPx(1, 0.49f), 0);
    KT_CHECK_EQ(bh::brightPx(0, 1.0f), 0);
    // Monotone in ratio and bounded by size across a sweep.
    for (int size = 1; size <= 48; ++size) {
        int prev = -1;
        for (int k = 0; k <= 200; ++k) {
            int b = bh::brightPx(size, k / 200.0f);
            bool ok = b >= 0 && b <= size && b >= prev;
            kt::Check("bright monotone", ok);
            prev = b;
        }
    }
}

static void testDiscLine() {
    // 20 px icon at (ix=5, iy=0): r = 9.5, centre y = 10. Line row y0 = 10: dy = 0.5, chord2 = 90.25-0.25 = 90 -> lw = int(2*9.4868+.5) = 19.
    bh::DiscLine d = bh::discLine(5, 0, 20, 10);
    KT_CHECK_EQ(d.w, 19); KT_CHECK_EQ(d.x, 5 + (20 - 19) / 2);
    // Top and bottom rows of the square are outside the circle (|dy| = 9.5 = r -> chord2 = 0 -> not > 0 -> width 0).
    d = bh::discLine(5, 0, 20, 0);  KT_CHECK_EQ(d.w, 0); KT_CHECK_EQ(d.x, 5 + 10);
    d = bh::discLine(5, 0, 20, 19); KT_CHECK_EQ(d.w, 0); KT_CHECK_EQ(d.x, 5 + 10);
    // Row 1 (dy = -8.5): chord2 = 90.25 - 72.25 = 18 -> lw = int(2*4.2426+.5) = 8 ; x offset (20-8)/2 = 6.
    d = bh::discLine(0, 0, 20, 1);  KT_CHECK_EQ(d.w, 8); KT_CHECK_EQ(d.x, 6);
    // iy shifts everything: same row relative to the icon gives the same width, x unchanged by iy.
    d = bh::discLine(5, 100, 20, 110); KT_CHECK_EQ(d.w, 19); KT_CHECK_EQ(d.x, 5);
    // size 1: r = 0 -> chord2 = -dy^2 <= 0 -> 0.
    d = bh::discLine(3, 4, 1, 4); KT_CHECK_EQ(d.w, 0);
    // size 2: r = 0.5, centre 1: y0 = iy -> dy = -0.5 -> chord2 = 0 -> 0.
    d = bh::discLine(0, 0, 2, 0); KT_CHECK_EQ(d.w, 0);
    // Sweep: width in [0,size], x inside the square, and rows mirror about the centre.
    for (int size = 1; size <= 48; ++size) for (int iy : { 0, 7, 100 }) for (int r = 0; r < size; ++r) {
        bh::DiscLine a = bh::discLine(10, iy, size, iy + r), m = bh::discLine(10, iy, size, iy + (size - 1 - r));
        bool ok = a.w >= 0 && a.w <= size && a.x >= 10 && a.x + a.w <= 10 + size && a.w == m.w && a.x == m.x;
        kt::Check("disc chord", ok, ok ? "" : "bad chord");
    }
    // The chord peaks at the middle rows and never narrows toward the centre.
    for (int size = 2; size <= 48; ++size) {
        int prev = -1;
        for (int r = 0; r <= (size - 1) / 2; ++r) {
            int w = bh::discLine(0, 0, size, r).w;
            kt::Check("chord widens to centre", w >= prev); prev = w;
        }
    }
}

static void testColumnSig() {
    KT_CHECK_EQ(bh::columnSig(0, 0, 0, 0, 0, 0, 0), 0);
    KT_CHECK_EQ(bh::columnSig(1, 0, 0, 0, 0, 0, 0), 1 << 16);
    KT_CHECK_EQ(bh::columnSig(0, 1, 0, 0, 0, 0, 0), 1 << 4);
    KT_CHECK_EQ(bh::columnSig(0, 0, 1, 0, 0, 0, 0), 1 << 12);
    KT_CHECK_EQ(bh::columnSig(0, 0, 0, 1, 0, 0, 0), 1 << 8);
    KT_CHECK_EQ(bh::columnSig(0, 0, 0, 0, 1, 0, 0), 1 << 20);
    KT_CHECK_EQ(bh::columnSig(0, 0, 0, 0, 0, 7, 0), 7);
    KT_CHECK_EQ(bh::columnSig(0, 0, 0, 0, 0, 0, 1), 1);
    KT_CHECK_EQ(bh::columnSig(3, 1, 2, 19, 100, 200, 0), (3 << 16) ^ (1 << 4) ^ (2 << 12) ^ (19 << 8) ^ (100 << 20) ^ 200);
    // Any single input changing changes the signature (when fields do not overlap).
    int base = bh::columnSig(3, 1, 2, 19, 100, 200, 0);
    KT_CHECK(bh::columnSig(4, 1, 2, 19, 100, 200, 0) != base);
    KT_CHECK(bh::columnSig(3, 0, 2, 19, 100, 200, 0) != base);
    KT_CHECK(bh::columnSig(3, 1, 3, 19, 100, 200, 0) != base);
    KT_CHECK(bh::columnSig(3, 1, 2, 20, 100, 200, 0) != base);
    KT_CHECK(bh::columnSig(3, 1, 2, 19, 101, 200, 0) != base);
    KT_CHECK(bh::columnSig(3, 1, 2, 19, 100, 201, 0) != base);
    KT_CHECK(bh::columnSig(3, 1, 2, 19, 100, 200, 1) != base);
    // CHARACTERIZATION (log-only weakness): the fields overlap (cardH is up to 11 bits and shares bits 0-10 with isLarge, debuff<<4
    // and size<<8), so distinct inputs can collide and a real change would then go unlogged: debuff=1/cardH=0 == debuff=0/cardH=16.
    KT_CHECK_EQ(bh::columnSig(0, 1, 0, 0, 0, 0, 0), bh::columnSig(0, 0, 0, 0, 0, 16, 0));
}

// ================================================================ menu companion portraits
static void testMenuGates() {
    KT_CHECK(!bh::menuPortraitVisible(0));
    KT_CHECK(!bh::menuPortraitVisible(1));
    KT_CHECK(bh::menuPortraitVisible(2));
    KT_CHECK(bh::menuPortraitVisible(3));
    KT_CHECK(!bh::menuPortraitVisible(4));
    KT_CHECK(!bh::menuPortraitVisible(~2));
    KT_CHECK(bh::menuPortraitVisible(-1));

    KT_CHECK(bh::menuRectOk(1, 1));
    KT_CHECK(bh::menuRectOk(2000, 2000));
    KT_CHECK(bh::menuRectOk(100, 150));
    KT_CHECK(!bh::menuRectOk(0, 100));
    KT_CHECK(!bh::menuRectOk(100, 0));
    KT_CHECK(!bh::menuRectOk(-1, 100));
    KT_CHECK(!bh::menuRectOk(100, -1));
    KT_CHECK(!bh::menuRectOk(2001, 100));
    KT_CHECK(!bh::menuRectOk(100, 2001));
    KT_CHECK(!bh::menuRectOk(0, 0));
    KT_CHECK(!bh::menuRectOk(INT_MIN, INT_MAX));
    KT_CHECK(!bh::menuRectOk(INT_MAX, 100));
    // Every size menuRectOk accepts yields a sane layout (the menu draws with isLarge=0 and the control rect as the card).
    for (int w = 1; w <= 2000; w += 37) for (int h = 1; h <= 2000; h += 41) {
        bh::ColumnLayout l = bh::columnLayout(4, 2, 0, w, h, 2.0f);
        KT_CHECK(l.size >= 0 && (w < 30 || l.size >= 1) && l.perCol >= 1);   // w of a few px can give size 0 (see testColumnLayout)
    }
}


// Address-table gates (doc 20 B2): every feature runs only when the K2AddrTable has the fields it needs. Null table / a zero field = off.
void testTableGates() {
    // Steam: everything available
    KT_CHECK(bh::arrowFieldsOk(&kSteam)); KT_CHECK(bh::arrowHideFieldsOk(&kSteam)); KT_CHECK(bh::columnFieldsOk(&kSteam));
    KT_CHECK(bh::menuFieldsOk(&kSteam)); KT_CHECK(bh::iconCacheFieldsOk(&kSteam));
    // GOG column is filled from doc 21b (static-verified): every feature available
    KT_CHECK(bh::arrowFieldsOk(&kGog)); KT_CHECK(bh::arrowHideFieldsOk(&kGog)); KT_CHECK(bh::columnFieldsOk(&kGog));
    KT_CHECK(bh::menuFieldsOk(&kGog)); KT_CHECK(bh::iconCacheFieldsOk(&kGog));
    // a table that knows nothing: every feature off
    static const K2AddrTable kEmpty{};
    KT_CHECK(!bh::arrowFieldsOk(&kEmpty)); KT_CHECK(!bh::arrowHideFieldsOk(&kEmpty)); KT_CHECK(!bh::columnFieldsOk(&kEmpty));
    KT_CHECK(!bh::menuFieldsOk(&kEmpty)); KT_CHECK(!bh::iconCacheFieldsOk(&kEmpty));
    // null table (unknown exe)
    const K2AddrTable* none = nullptr;
    KT_CHECK(!bh::arrowFieldsOk(none)); KT_CHECK(!bh::arrowHideFieldsOk(none)); KT_CHECK(!bh::columnFieldsOk(none));
    KT_CHECK(!bh::menuFieldsOk(none)); KT_CHECK(!bh::iconCacheFieldsOk(none));
    // one zero field at a time, per gate
    K2AddrTable t;
#define ZERO_BREAKS(gate, field) t = kSteam; t.field = 0; KT_CHECK(!gate(&t))
    ZERO_BREAKS(bh::arrowFieldsOk, Slot_Flags); ZERO_BREAKS(bh::arrowFieldsOk, Slot_ArrowBuff); ZERO_BREAKS(bh::arrowFieldsOk, Slot_ArrowDebuff);
    ZERO_BREAKS(bh::arrowFieldsOk, Slot_Overlay0); ZERO_BREAKS(bh::arrowFieldsOk, Slot_Overlay1); ZERO_BREAKS(bh::arrowFieldsOk, Slot_Overlay2);
    ZERO_BREAKS(bh::arrowHideFieldsOk, Label_Border); ZERO_BREAKS(bh::arrowHideFieldsOk, BorderVtable);
    ZERO_BREAKS(bh::arrowHideFieldsOk, Border_Width); ZERO_BREAKS(bh::arrowHideFieldsOk, Border_Height);
    ZERO_BREAKS(bh::columnFieldsOk, Label_Border); ZERO_BREAKS(bh::columnFieldsOk, Border_Rot); ZERO_BREAKS(bh::columnFieldsOk, Border_Alpha);
    ZERO_BREAKS(bh::columnFieldsOk, Border_Tint); ZERO_BREAKS(bh::columnFieldsOk, ImageVt_Draw);
    ZERO_BREAKS(bh::menuFieldsOk, Ctl_Flags); ZERO_BREAKS(bh::menuFieldsOk, Ctl_X); ZERO_BREAKS(bh::menuFieldsOk, Ctl_Y);
    ZERO_BREAKS(bh::menuFieldsOk, Ctl_W); ZERO_BREAKS(bh::menuFieldsOk, Ctl_H);
    ZERO_BREAKS(bh::iconCacheFieldsOk, Creature_IconCacheData); ZERO_BREAKS(bh::iconCacheFieldsOk, Creature_IconCacheCount);
    ZERO_BREAKS(bh::iconCacheFieldsOk, IconEnt_F14); ZERO_BREAKS(bh::iconCacheFieldsOk, IconEnt_F1c);
#undef ZERO_BREAKS
    // the gates follow the forced table (what the DLL reads through k2addr::get())
    k2addr::setForTest(nullptr); KT_CHECK(!bh::arrowFieldsOk(k2addr::get()));
    k2addr::setForTest(&kSteam); KT_CHECK(bh::arrowFieldsOk(k2addr::get()));
    k2addr::clearForTest();
}

int main() {
    // 1. Fits: exact text, exact length, NUL-terminated.
    {
        Guarded g(64); char* b = g.buf(); b[0] = 0; int len = 0;
        len = bh::appendf(b, g.cap, len, " [%d id=%d]", 3, 42);
        len = bh::appendf(b, g.cap, len, " %02x", 0xab);
        KT_CHECK_EQ(b, " [3 id=42] ab");
        KT_CHECK_EQ(len, 13);
        KT_CHECK_EQ((int)strlen(b), len);
        KT_CHECK(g.canaryIntact());
    }

    // 2. Truncation: strlen == cap-1, return == cap-1, canary untouched.
    {
        Guarded g(16); char* b = g.buf(); b[0] = 0; int len = 0;
        len = bh::appendf(b, g.cap, len, "%s", "0123456789abcdefghij");
        KT_CHECK_EQ(len, 15);
        KT_CHECK_EQ((int)strlen(b), 15);
        KT_CHECK_EQ(b, "0123456789abcde");
        KT_CHECK(g.canaryIntact());
    }

    // 3. Full buffer: repeated calls are no-ops, len stable, still terminated, no overrun.
    {
        Guarded g(16); char* b = g.buf(); b[0] = 0; int len = 0;
        len = bh::appendf(b, g.cap, len, "%s", "0123456789abcdefghij");
        for (int i = 0; i < 5; ++i) {
            int l2 = bh::appendf(b, g.cap, len, " more %d", i);
            KT_CHECK_EQ(l2, len);
            len = l2;
        }
        KT_CHECK_EQ(b, "0123456789abcde");
        KT_CHECK(g.canaryIntact());
    }

    // 3b. A caller that passes a len already past the end (the old buggy state) is clamped.
    {
        Guarded g(16); char* b = g.buf(); memset(b, 'x', 16);
        int len = bh::appendf(b, g.cap, 276, " boom");
        KT_CHECK_EQ(len, 15);
        KT_CHECK_EQ((int)b[15], 0);
        KT_CHECK(g.canaryIntact());
    }

    // 4. cap = 1: buffer stays "" and the return is 0; cap <= 0 is a pure no-op.
    {
        Guarded g(1); char* b = g.buf(); b[0] = 0;
        int len = bh::appendf(b, g.cap, 0, "%s", "abc");
        KT_CHECK_EQ(len, 0);
        KT_CHECK_EQ(b, "");
        KT_CHECK(g.canaryIntact());
        char c = 'q';
        KT_CHECK_EQ(bh::appendf(&c, 0, 0, "zzz"), 0);
        KT_CHECK_EQ(bh::appendf(&c, -5, 3, "zzz"), 3);
        KT_CHECK_EQ((int)c, (int)'q');
    }

    // 5. Format error / empty output keeps the NUL and the length.
    {
        Guarded g(16); char* b = g.buf(); b[0] = 0;
        int len = bh::appendf(b, g.cap, 0, "%s", "ab");
        len = bh::appendf(b, g.cap, len, "%s", "");
        KT_CHECK_EQ(len, 2);
        KT_CHECK_EQ(b, "ab");
    }

    // 6. The ICONCACHE scenario: char ent[256], six maximum-width entries (k=5, id=-32768, f14=f1c=INT_MIN).
    {
        const int cap = 256;
        std::string full;
        for (int k = 0; k < 6; ++k) {
            char t[96]; snprintf(t, sizeof t, kIconFmt, 5, (short)-32768, INT_MIN, INT_MIN);
            full += t;
        }
        KT_CHECK(full.size() > (size_t)cap);   // the scenario really exceeds the buffer (~276 chars)

        Guarded g(cap); char* b = g.buf(); b[0] = 0; int len = 0;
        for (int k = 0; k < 6; ++k) {
            short id = -32768; int f14 = INT_MIN, f1c = INT_MIN;
            len = bh::appendf(b, cap, len, kIconFmt, 5, id, f14, f1c);
        }
        KT_CHECK_EQ(len, cap - 1);
        KT_CHECK_EQ((int)strlen(b), cap - 1);
        KT_CHECK_EQ(b, full.substr(0, cap - 1).c_str());   // truncated prefix is byte-identical to the untruncated text
        // Extra writes after truncation (what a longer loop would do) must still be harmless.
        for (int k = 0; k < 4; ++k) len = bh::appendf(b, cap, len, kIconFmt, 5, 1, 2, 3);
        KT_CHECK_EQ(len, cap - 1);
        KT_CHECK_EQ((int)strlen(b), cap - 1);
        KT_CHECK(g.canaryIntact());
    }

    // 7. Entries that fit are reproduced byte-for-byte (same as the old idiom when nothing truncates).
    {
        Guarded g(256); char* b = g.buf(); b[0] = 0; int len = 0;
        std::string want;
        for (int k = 0; k < 3; ++k) {
            len = bh::appendf(b, 256, len, kIconFmt, k, 100 + k, 7, -1);
            char t[96]; snprintf(t, sizeof t, kIconFmt, k, 100 + k, 7, -1); want += t;
        }
        KT_CHECK_EQ(b, want.c_str());
        KT_CHECK_EQ(len, (int)want.size());
    }

    testArrowGates();
    testBumpArrowCounters();
    testArrowRawTable();
    testArrowSuppressed();
    testColumnLayout();
    testIconPos();
    testBrightPx();
    testDiscLine();
    testColumnSig();
    testMenuGates();
    testTableGates();

    return kt::Report("buff_hud_logic");
}
