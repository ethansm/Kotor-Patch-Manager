// Host test of SliderRowLogic.h (doc 46 MCF5): slot geometry, pixel <-> value mapping (ends, rounding, step snapping, odd ranges, 64-bit span),
// wedge / thumb rects, hit test and the drag state machine.
#include "SliderRowLogic.h"
#include <limits.h>
#include "check.h"

using namespace mui;

static RowCfg cfg(int lo, int hi, int step, int value) { return RowCfg{RK_SLIDER, lo, hi, step, value, 0}; }

static void test_geom() {
    const Rect slot = {100, 80, 400, 32};
    const SliderGeom g = sliderGeom(slot);
    KT_CHECK_EQ(g.caption.x, 100);
    KT_CHECK_EQ(g.caption.w, 168);                                  // 42 % of 400
    KT_CHECK_EQ(g.track.x, 100 + 168 + kRowGap);
    KT_CHECK_EQ(g.caption.w + kRowGap + g.track.w, slot.w);          // widths sum to the slot
    KT_CHECK(g.track.y == slot.y && g.track.h == slot.h && g.caption.h == slot.h);
    KT_CHECK(sliderHit(g, g.track.x, slot.y));
    KT_CHECK(sliderHit(g, g.track.x + g.track.w - 1, slot.y + 31));
    KT_CHECK(!sliderHit(g, g.track.x + g.track.w, slot.y + 5));     // right edge exclusive
    KT_CHECK(!sliderHit(g, g.track.x - 1, slot.y + 5));             // the gap
    KT_CHECK(!sliderHit(g, g.caption.x + 5, slot.y + 5));            // the caption is not the track
    KT_CHECK(!sliderHit(g, g.track.x + 5, slot.y + 32));
    const SliderGeom tiny = sliderGeom(Rect{0, 0, 4, 10});           // narrower than the gap: no negative widths
    KT_CHECK(tiny.caption.w >= 0 && tiny.track.w >= 0);
    KT_CHECK_EQ(tiny.caption.w + tiny.track.w + (tiny.track.w > 0 ? kRowGap : 0) <= 4 + kRowGap, 1);
}

static void test_value_at_ends_and_rounding() {
    const Rect tr = {200, 0, 100, 32};
    const RowCfg c = cfg(0, 100, 1, 50);
    KT_CHECK_EQ(sliderValueAt(c, tr, 200), 0);
    KT_CHECK_EQ(sliderValueAt(c, tr, 100), 0);                       // left of the track
    KT_CHECK_EQ(sliderValueAt(c, tr, 300), 100);
    KT_CHECK_EQ(sliderValueAt(c, tr, 999), 100);                     // right of the track
    KT_CHECK_EQ(sliderValueAt(c, tr, 250), 50);
    const RowCfg half = cfg(0, 10, 1, 0);                            // 10 values over 100 px: 4.5 px per... rounds to nearest
    KT_CHECK_EQ(sliderValueAt(half, tr, 204), 0);
    KT_CHECK_EQ(sliderValueAt(half, tr, 205), 1);                    // exactly half a unit rounds up
    KT_CHECK_EQ(sliderValueAt(half, tr, 214), 1);
    KT_CHECK_EQ(sliderValueAt(half, tr, 215), 2);
}

static void test_value_at_step_and_odd_range() {
    const Rect tr = {0, 0, 200, 32};
    const RowCfg c = cfg(5, 95, 5, 5);                               // the Self-Heal style 5..95 step 5 (19 steps)
    KT_CHECK_EQ(sliderValueAt(c, tr, 0), 5);
    KT_CHECK_EQ(sliderValueAt(c, tr, 200), 95);
    for (int px = 0; px <= 200; ++px) {
        const int v = sliderValueAt(c, tr, px);
        KT_CHECK(v >= 5 && v <= 95 && (v - 5) % 5 == 0);             // always on the step grid
    }
    KT_CHECK_EQ(sliderValueAt(c, tr, 100), 50);
    const RowCfg odd = cfg(0, 10, 4, 0);                             // max is not on the grid: the right end still reaches max
    KT_CHECK_EQ(sliderValueAt(odd, tr, 200), 10);
    KT_CHECK_EQ(sliderValueAt(odd, tr, 199) <= 10, 1);
    const RowCfg rev = cfg(100, 0, 1, 0);                            // reversed range is normalised, never inverted
    KT_CHECK_EQ(sliderValueAt(rev, tr, 0), 0);
    KT_CHECK_EQ(sliderValueAt(rev, tr, 200), 100);
    const RowCfg bad = cfg(0, 100, 0, 0);                            // step < 1 acts as 1
    KT_CHECK_EQ(sliderValueAt(bad, tr, 100), 50);
}

static void test_value_at_degenerate() {
    const RowCfg same = cfg(7, 7, 1, 7);
    KT_CHECK_EQ(sliderValueAt(same, Rect{0, 0, 100, 10}, 50), 7);    // zero span
    KT_CHECK_EQ(sliderPosOf(same, 100, 7), 0);
    const RowCfg c = cfg(0, 100, 1, 0);
    KT_CHECK_EQ(sliderValueAt(c, Rect{0, 0, 0, 10}, 50), 0);         // zero-width track
    KT_CHECK_EQ(sliderPosOf(c, 0, 50), 0);
    const RowCfg wide = cfg(INT_MIN, INT_MAX, 1, 0);                 // span needs 33 bits: no overflow, monotonic, ends exact
    const Rect tr = {0, 0, 1000, 10};
    KT_CHECK_EQ(sliderValueAt(wide, tr, 0), INT_MIN);
    KT_CHECK_EQ(sliderValueAt(wide, tr, 1000), INT_MAX);
    int prev = INT_MIN;
    for (int px = 0; px <= 1000; px += 10) {
        const int v = sliderValueAt(wide, tr, px);
        KT_CHECK(v >= prev);
        prev = v;
    }
    KT_CHECK_EQ(sliderValueAt(wide, tr, INT_MAX), INT_MAX);          // a huge pointer x must not overflow d * span
    KT_CHECK_EQ(sliderValueAt(wide, tr, INT_MIN), INT_MIN);
    const RowCfg bigStep = cfg(INT_MIN, INT_MAX, 1 << 30, 0);        // the nearest step can land just past max: clamped, never wrapped to INT_MIN
    KT_CHECK_EQ(sliderValueAt(bigStep, tr, 999), INT_MAX);
    const int mid = sliderValueAt(wide, tr, 500);
    KT_CHECK(mid > -1000000 && mid < 1000000);                       // about the middle of the 32-bit range
}

static void test_pos_roundtrip() {
    const Rect tr = {0, 0, 300, 32};
    const RowCfg c = cfg(0, 60, 1, 0);
    KT_CHECK_EQ(sliderPosOf(c, tr.w, 0), 0);
    KT_CHECK_EQ(sliderPosOf(c, tr.w, 60), 300);
    KT_CHECK_EQ(sliderPosOf(c, tr.w, 30), 150);
    KT_CHECK_EQ(sliderPosOf(c, tr.w, -50), 0);                       // out-of-range values clamp
    KT_CHECK_EQ(sliderPosOf(c, tr.w, 999), 300);
    const RowCfg sev = cfg(0, 7, 1, 0);                              // 100 px / 7: value 3 is 42.86 px, rounds to 43 (not 42)
    KT_CHECK_EQ(sliderPosOf(sev, 100, 3), 43);
    KT_CHECK_EQ(sliderPosOf(sev, 100, 1), 14);                       // 14.29 rounds down
    for (int v = 0; v <= 60; ++v) KT_CHECK_EQ(sliderValueAt(c, tr, sliderPosOf(c, tr.w, v)), v);   // value -> px -> value is exact
}

static void test_wedge_and_thumb() {
    const Rect tr = {100, 20, 200, 32};
    const RowCfg c = cfg(0, 100, 1, 25);
    const Rect w = sliderWedgeRect(c, tr, 25);
    KT_CHECK(w.x == 100 && w.y == 20 && w.h == 32);
    KT_CHECK_EQ(w.w, 50);
    KT_CHECK_EQ(sliderWedgeRect(c, tr, 0).w, 0);
    KT_CHECK_EQ(sliderWedgeRect(c, tr, 100).w, 200);
    const Rect t = sliderThumbRect(c, tr, 25);
    KT_CHECK_EQ(t.w, kSliderThumbW);
    KT_CHECK_EQ(t.x, 100 + 50 - kSliderThumbW / 2);                  // centred on the wedge end
    const Rect lo = sliderThumbRect(c, tr, 0), hi = sliderThumbRect(c, tr, 100);
    KT_CHECK_EQ(lo.x, 100);                                          // kept inside the track at both ends
    KT_CHECK_EQ(hi.x + hi.w, 300);
    const Rect narrow = sliderThumbRect(c, Rect{10, 0, 6, 8}, 50);   // track narrower than the thumb
    KT_CHECK(narrow.w == 6 && narrow.x == 10);
}

static void test_inner_inside_frame() {
    const Rect fr = {100, 20, 640, 80};                              // typical 1080p-ish frame: stroke ~10 px each side
    const Rect in = sliderInner(fr);
    KT_CHECK(in.x > fr.x && in.y > fr.y);
    KT_CHECK(in.x + in.w < fr.x + fr.w && in.y + in.h < fr.y + fr.h);
    KT_CHECK_EQ(in.x - fr.x, (fr.x + fr.w) - (in.x + in.w));         // symmetric
    KT_CHECK_EQ(in.y - fr.y, (fr.y + fr.h) - (in.y + in.h));
    const RowCfg c = cfg(0, 100, 1, 0);
    const Rect lo = sliderThumbRect(c, in, 0), hi = sliderThumbRect(c, in, 100);
    KT_CHECK(lo.x >= in.x && hi.x + hi.w <= in.x + in.w);            // thumb never reaches the stroke at either end
    KT_CHECK(lo.y >= in.y && lo.y + lo.h <= in.y + in.h);
    KT_CHECK(sliderWedgeRect(c, in, 100).x + sliderWedgeRect(c, in, 100).w <= in.x + in.w);
    KT_CHECK_EQ(sliderValueAt(c, in, fr.x), 0);                      // a press on the left stroke is the minimum, on the right stroke the maximum
    KT_CHECK_EQ(sliderValueAt(c, in, fr.x + fr.w - 1), 100);
    const Rect tiny = {5, 5, 4, 3};                                  // too small to inset: unchanged
    KT_CHECK(sliderInner(tiny).x == 5 && sliderInner(tiny).w == 4 && sliderInner(tiny).h == 3);
    const Rect small = sliderInner(Rect{0, 0, 100, 16});
    KT_CHECK(small.x >= 2 && small.y >= 2 && small.w > 0 && small.h > 0);
}

static void test_wedge_ends_at_thumb_centre() {
    const Rect fr = {100, 20, 640, 80};
    const Rect in = sliderInner(fr), tr = sliderTravelOf(fr);
    const RowCfg c = cfg(0, 100, 1, 0);
    for (int v = 0; v <= 100; ++v) {
        const Rect w = sliderWedgeFor(c, fr, v), th = sliderThumbFor(c, fr, v);
        KT_CHECK(th.x >= in.x && th.x + th.w <= in.x + in.w);        // thumb box never leaves the inner run
        if (v == 0) { KT_CHECK_EQ(w.w, 0); continue; }                // hidden at the minimum
        KT_CHECK(w.x == in.x && w.x + w.w <= in.x + in.w);
        KT_CHECK_EQ(w.x + w.w, th.x + th.w / 2);                      // the wedge ends exactly at the thumb centre, at every value
    }
    KT_CHECK_EQ(sliderThumbFor(c, fr, 0).x, in.x);                   // box flush with the inner left at min, inner right at max
    KT_CHECK_EQ(sliderThumbFor(c, fr, 100).x + sliderThumbFor(c, fr, 100).w, in.x + in.w);
    KT_CHECK_EQ(sliderValueAt(c, tr, tr.x), 0);                      // pointer at the travel ends is min / max
    KT_CHECK_EQ(sliderValueAt(c, tr, tr.x + tr.w), 100);
    const RowCfg snap = cfg(5, 95, 5, 50);                            // the Snap row: max reachable, 95 is the last notch
    KT_CHECK_EQ(sliderValueAt(snap, tr, tr.x + tr.w), 95);
    const Rect small = {0, 0, 24, 16};                                // narrower than a thumb: falls back, no overflow
    const Rect w2 = sliderWedgeFor(c, small, 100), t2 = sliderThumbFor(c, small, 100);
    KT_CHECK(w2.w >= 0 && t2.w >= 0 && t2.w <= small.w);
}

static void test_drag() {
    const Rect tr = {0, 0, 100, 32};
    const RowCfg c = cfg(0, 100, 1, 0);
    SliderDrag d;
    KT_CHECK(!d.active);
    bool ch = true;
    sliderMove(d, c, tr, 50, &ch);
    KT_CHECK(!ch);                                                   // not grabbed: never reports a change
    KT_CHECK_EQ(sliderBegin(d, 3, c, tr, 40), 40);                   // press jumps to the pointer
    KT_CHECK(d.active && d.row == 3);
    KT_CHECK_EQ(sliderMove(d, c, tr, 40, &ch), 40);
    KT_CHECK(!ch);                                                   // same value: no handler call
    KT_CHECK_EQ(sliderMove(d, c, tr, 60, &ch), 60);
    KT_CHECK(ch);
    KT_CHECK_EQ(sliderMove(d, c, tr, 500, &ch), 100);                // dragging past the right end clamps to max
    KT_CHECK(ch);
    KT_CHECK_EQ(sliderMove(d, c, tr, -500, &ch), 0);                 // and past the left end to min
    KT_CHECK(ch);
    sliderSetStored(d, 20);                                          // the handler snapped the stored value to 20
    sliderMove(d, c, tr, 20, &ch);
    KT_CHECK(!ch);                                                   // compared against the stored value, not the request
    sliderEnd(d);
    KT_CHECK(!d.active && d.row == -1);
}

int main() {
    test_geom();
    test_value_at_ends_and_rounding();
    test_value_at_step_and_odd_range();
    test_value_at_degenerate();
    test_pos_roundtrip();
    test_wedge_and_thumb();
    test_inner_inside_frame();
    test_wedge_ends_at_thumb_centre();
    test_drag();
    return kt::Report("slider_row_logic_test");
}
