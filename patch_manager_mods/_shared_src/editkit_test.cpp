// T1 host tests for EditKit.h (generic holster-editor UI helpers, pure; doc 20).
// Build/run: python3 scripts/run_tests.py editkit
#include "EditKit.h"
#include "check.h"
#include <cmath>
#include <string.h>

using namespace editkit;

static bool approx(float a, float b, float eps = 1e-4f) { return std::fabs(a - b) <= eps; }

static void testSlider() {
    Box tr{200, 10, 100, 12};
    KT_CHECK(approx(sliderToValue(200, tr, -1.f, 1.f), -1.f));
    KT_CHECK(approx(sliderToValue(300, tr, -1.f, 1.f), 1.f));
    KT_CHECK(approx(sliderToValue(250, tr, -1.f, 1.f), 0.f));
    KT_CHECK(sliderToValue(50, tr, -1.f, 1.f) == -1.f);          // clamped; ends are exact
    KT_CHECK(sliderToValue(999, tr, -1.f, 1.f) == 1.f);
    KT_CHECK(valueToSlider(-1.f, tr, -1.f, 1.f) == 200);
    KT_CHECK(valueToSlider(1.f, tr, -1.f, 1.f) == 300);
    KT_CHECK(valueToSlider(0.f, tr, -1.f, 1.f) == 250);
    KT_CHECK(valueToSlider(-50.f, tr, -1.f, 1.f) == 200);        // clamped
    KT_CHECK(valueToSlider(50.f, tr, -1.f, 1.f) == 300);
    int badPx = 0;                                               // pixel -> value -> pixel is exact
    for (int px = 200; px <= 300; ++px)
        if (valueToSlider(sliderToValue(px, tr, -1.f, 1.f), tr, -1.f, 1.f) != px) ++badPx;
    KT_CHECK_EQ(badPx, 0);
    const float vals[] = {-0.97f, -0.5f, -0.123f, 0.f, 0.333f, 0.999f};   // value -> pixel -> value within one px step
    int badV = 0;
    for (float v : vals) {
        float v2 = sliderToValue(valueToSlider(v, tr, -1.f, 1.f), tr, -1.f, 1.f);
        if (std::fabs(v2 - v) > 0.02f + 1e-5f) ++badV;
    }
    KT_CHECK_EQ(badV, 0);
    Box zoomTr{0, 0, 50, 8};                                     // non-symmetric range, one px = 0.038
    float v2 = sliderToValue(valueToSlider(1.f, zoomTr, 0.6f, 2.5f), zoomTr, 0.6f, 2.5f);
    KT_CHECK(std::fabs(v2 - 1.f) <= 1.9f / 50.f + 1e-5f);
    Box zero{5, 0, 0, 4};                                        // degenerate track
    KT_CHECK(sliderToValue(5, zero, 2.f, 3.f) == 2.f);
    KT_CHECK(valueToSlider(2.5f, zero, 2.f, 3.f) == 5);
}

static void testRepeat() {
    Repeat r = {0, 0, 0};
    KT_CHECK_EQ(repeatTick(r, true, 1000), 1);     // press: one step
    KT_CHECK_EQ(repeatTick(r, true, 1399), 0);     // 399 ms: nothing
    KT_CHECK_EQ(repeatTick(r, true, 1400), 1);     // 400 ms: first repeat
    KT_CHECK_EQ(repeatTick(r, true, 1420), 0);
    KT_CHECK_EQ(repeatTick(r, true, 1460), 1);     // 460 ms: next repeat, 60 ms later
    KT_CHECK_EQ(repeatTick(r, false, 1470), 0);    // release
    KT_CHECK(r.state == 0);
    KT_CHECK_EQ(repeatTick(r, true, 2000), 1);     // a fresh press fires again
    KT_CHECK_EQ(repeatTick(r, true, 2399), 0);     // 399 ms after the fresh press
    KT_CHECK_EQ(repeatTick(r, true, 2400), 1);     // 400 ms after it
    // a long stall replays the missed steps, up to kRepeatMaxPerTick, and then restarts the schedule at now
    Repeat s = {0, 0, 0};
    KT_CHECK_EQ(repeatTick(s, true, 0), 1);
    KT_CHECK_EQ(repeatTick(s, true, 2000), kRepeatMaxPerTick);
    KT_CHECK_EQ(repeatTick(s, true, 2059), 0);
    KT_CHECK_EQ(repeatTick(s, true, 2060), 1);
    // steps due inside one tick (no polling in between): 400, 460, 520 all due at 520
    Repeat u = {0, 0, 0};
    KT_CHECK_EQ(repeatTick(u, true, 0), 1);
    KT_CHECK_EQ(repeatTick(u, true, 520), 3);
    KT_CHECK_EQ(repeatTick(u, true, 579), 0);
    KT_CHECK_EQ(repeatTick(u, true, 580), 1);
}

static void testDragMode() {
    DragSel s = dragMode(true, false, false, false, false, 0);
    KT_CHECK(s.mode == DmRotate && !s.fine && !s.snap && s.lockAxis == -1);
    s = dragMode(false, true, false, false, false, 0);
    KT_CHECK(s.mode == DmMove && s.lockAxis == -1);
    s = dragMode(false, false, true, false, false, 0);
    KT_CHECK(s.mode == DmSpin && s.lockAxis == -1);
    s = dragMode(true, true, true, false, false, 0);             // priority L > R > M
    KT_CHECK(s.mode == DmRotate);
    s = dragMode(false, true, true, false, false, 0);
    KT_CHECK(s.mode == DmMove);
    s = dragMode(false, false, false, false, false, 0);
    KT_CHECK(s.mode == DmNone);
    s = dragMode(true, false, false, true, false, 0);            // Shift = fine
    KT_CHECK(s.fine && !s.snap);
    s = dragMode(true, false, false, false, true, 0);            // Ctrl = snap
    KT_CHECK(!s.fine && s.snap);
    s = dragMode(false, true, false, true, true, 0);
    KT_CHECK(s.mode == DmMove && s.fine && s.snap);
    KT_CHECK(dragMode(true, false, false, false, false, 1).lockAxis == 0);   // X
    KT_CHECK(dragMode(true, false, false, false, false, 2).lockAxis == 1);   // Y
    KT_CHECK(dragMode(true, false, false, false, false, 4).lockAxis == 2);   // Z
    KT_CHECK(dragMode(true, false, false, false, false, 6).lockAxis == 1);   // lowest set bit wins
    KT_CHECK(dragMode(true, false, false, false, false, 5).lockAxis == 0);
    KT_CHECK(dragMode(true, false, false, false, false, 7).lockAxis == 0);
    KT_CHECK(dragMode(true, false, false, false, false, 0).lockAxis == -1);
}

static void testMidClick() {
    KT_CHECK(midClick(10, 10, 10, 10));           // no movement
    KT_CHECK(midClick(0, 0, 3, 0));               // 3 px: click
    KT_CHECK(!midClick(0, 0, 4, 0));              // 4 px: drag
    KT_CHECK(midClick(0, 0, 0, -3));
    KT_CHECK(!midClick(0, 0, 0, -4));
    KT_CHECK(!midClick(0, 0, -4, 0));
    KT_CHECK(midClick(0, 0, 2, 2));               // 2.83 px Euclidean
    KT_CHECK(!midClick(0, 0, 3, 3));              // 4.24 px Euclidean
    KT_CHECK(midClick(5, 5, 2, 7));               // (-3, +2): 3.6 px
}

static void testFmtValue() {
    char b[32];
    fmtValue(b, sizeof b, 12.345f, true);
    KT_CHECK(strcmp(b, "+12.3\xB0") == 0);
    KT_CHECK((unsigned char)b[5] == 0xB0 && b[6] == 0);          // degree sign is one cp1252 byte
    fmtValue(b, sizeof b, -0.5f, true);
    KT_CHECK(strcmp(b, "-0.5\xB0") == 0);
    fmtValue(b, sizeof b, 0.f, true);
    KT_CHECK(strcmp(b, "+0.0\xB0") == 0);
    fmtValue(b, sizeof b, 0.1234f, false);
    KT_CHECK(strcmp(b, "+0.123 m") == 0);
    fmtValue(b, sizeof b, -1.5f, false);
    KT_CHECK(strcmp(b, "-1.500 m") == 0);
    fmtValue(b, 5, 12.3f, true);                                 // truncated, still NUL-terminated
    KT_CHECK(strlen(b) == 4u && b[4] == 0);
    char z[2] = {'x', 'y'};
    fmtValue(z, 0, 1.f, true);                                   // cap 0 writes nothing
    KT_CHECK(z[0] == 'x' && z[1] == 'y');
}

int main() {
    testSlider();
    testRepeat();
    testDragMode();
    testMidClick();
    testFmtValue();
    return kt::Report("editkit");
}
