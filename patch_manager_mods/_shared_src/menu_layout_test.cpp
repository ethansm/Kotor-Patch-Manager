// Host test of menulayout (MenuLayout.h): columnReflow for an inserted menu row (vanilla and ultrawide columns, ragged and
// boundary gaps, too-small and invalid inputs, no writes on reject), slotBelow, and statusStrip clamping.
#include "MenuLayout.h"
#include "check.h"

using namespace menulayout;

static Rect rect(int x, int y, int w, int h) {
    Rect r;
    r.x = x; r.y = y; r.w = w; r.h = h;
    return r;
}

// Vanilla column: 8 rows, x 78, w 318, h 32, y 134 + 36*i.
static void vanillaColumn(Rect* col) {
    for (int i = 0; i < 8; ++i) col[i] = rect(78, 134 + 36 * i, 318, 32);
}

// Insert after row 2 (GAMEPLAY in the vanilla column): new row at y 242, moved rows at 278,314,350,386,422.
static void test_vanilla_insert() {
    Rect col[8];
    vanillaColumn(col);
    Rect nw = rect(-1, -1, -1, -1);
    Rect moved[5];
    KT_CHECK(columnReflow(col, 8, 2, &nw, moved));
    KT_CHECK(nw.x == 78 && nw.y == 242 && nw.w == 318 && nw.h == 32);
    KT_CHECK_EQ(nw.y, 242);
    static const int want[5] = {278, 314, 350, 386, 422};
    for (int j = 0; j < 5; ++j) {
        KT_CHECK_EQ(moved[j].y, want[j]);
        KT_CHECK(moved[j].x == 78 && moved[j].w == 318 && moved[j].h == 32);
    }
    KT_CHECK_EQ(moved[4].y, 422);   // last row (QUIT) moved by one pitch
}

// Ultrawide column: same ys, x 161 w 236. x and w must survive for new, moved and the last row.
static void test_ultrawide_keeps_xw() {
    Rect col[8];
    for (int i = 0; i < 8; ++i) col[i] = rect(161, 134 + 36 * i, 236, 32);
    Rect nw = rect(-1, -1, -1, -1);
    Rect moved[5];
    KT_CHECK(columnReflow(col, 8, 2, &nw, moved));
    KT_CHECK(nw.x == 161 && nw.w == 236 && nw.y == 242);
    for (int j = 0; j < 5; ++j) KT_CHECK(moved[j].x == 161 && moved[j].w == 236);
    KT_CHECK_EQ(moved[4].y, 422);
}

// Column with a gap of `gap` px at index 4 (between row 4 and row 5); every other gap is the 36 px pitch.
static bool reflowWithGap(int gap) {
    Rect col[8];
    int ys[8] = {134, 170, 206, 242, 278, 0, 0, 0};
    ys[5] = ys[4] + gap;
    ys[6] = ys[5] + 36;
    ys[7] = ys[6] + 36;
    for (int i = 0; i < 8; ++i) col[i] = rect(78, ys[i], 318, 32);
    Rect nw, moved[5];
    return columnReflow(col, 8, 2, &nw, moved);
}

// Ragged: one gap of 40 with a 36 pitch (4 px off, outside +-2) must be rejected, and nothing written.
static void test_ragged_rejected_no_writes() {
    Rect col[8];
    int ys[8] = {134, 170, 206, 242, 278, 318, 358, 398};   // gaps 36,36,36,36,40,40,40
    for (int i = 0; i < 8; ++i) col[i] = rect(78, ys[i], 318, 32);
    Rect nw = rect(-1, -1, -1, -1);
    Rect moved[5];
    for (int j = 0; j < 5; ++j) moved[j] = rect(-1, -1, -1, -1);
    KT_CHECK(!columnReflow(col, 8, 2, &nw, moved));
    KT_CHECK(nw.y == -1 && nw.x == -1);
    for (int j = 0; j < 5; ++j) KT_CHECK(moved[j].y == -1 && moved[j].x == -1);
}

// Gap tolerance is exactly +-2 px around the pitch: 34 and 38 accepted, 33 and 39 rejected.
static void test_gap_tolerance_boundary() {
    KT_CHECK(reflowWithGap(36));
    KT_CHECK(reflowWithGap(38));
    KT_CHECK(reflowWithGap(34));
    KT_CHECK(!reflowWithGap(39));
    KT_CHECK(!reflowWithGap(33));
}

// Pitch must be > 0 and y strictly ascending. A zero pitch and a descending column are both rejected.
static void test_pitch_and_order_rejected() {
    Rect nw, moved[5];
    Rect flat[8];
    int ys[8] = {134, 170, 206, 206, 242, 278, 314, 350};   // pitch at insertAfter=2 is 0
    for (int i = 0; i < 8; ++i) flat[i] = rect(78, ys[i], 318, 32);
    KT_CHECK(!columnReflow(flat, 8, 2, &nw, moved));

    Rect down[8];
    for (int i = 0; i < 8; ++i) down[i] = rect(78, 400 - 36 * i, 318, 32);   // pitch -36
    KT_CHECK(!columnReflow(down, 8, 2, &nw, moved));

    // Pitch 1, one gap of 0: within +-2 of the pitch, but not strictly ascending, so rejected.
    Rect tight[4] = {rect(0, 10, 1, 1), rect(0, 11, 1, 1), rect(0, 11, 1, 1), rect(0, 12, 1, 1)};
    KT_CHECK(!columnReflow(tight, 4, 0, &nw, moved));
}

// Minimum sizes: n = insertAfter + 2 is valid (one moved row, the last one); too small or negative insertAfter is not.
static void test_minimum_and_invalid_args() {
    Rect col[8];
    vanillaColumn(col);
    Rect nw = rect(-1, -1, -1, -1);
    Rect moved[5];
    for (int j = 0; j < 5; ++j) moved[j] = rect(-1, -1, -1, -1);

    KT_CHECK(!columnReflow(col, 3, 2, &nw, moved));      // n < insertAfter + 2
    KT_CHECK(!columnReflow(col, 8, -1, &nw, moved));     // negative insertAfter
    KT_CHECK(!columnReflow(0, 8, 2, &nw, moved));        // null column
    KT_CHECK(!columnReflow(col, 8, 2, 0, moved));        // null outNew
    KT_CHECK(!columnReflow(col, 8, 2, &nw, 0));          // null outMoved
    KT_CHECK(nw.y == -1 && moved[0].y == -1);            // rejected calls wrote nothing

    KT_CHECK(columnReflow(col, 2, 0, &nw, moved));       // n = insertAfter + 2
    KT_CHECK_EQ(nw.y, 170);                              // new row takes row 1's rect
    KT_CHECK_EQ(moved[0].y, 206);                        // the one moved row is row 1, one pitch down
    KT_CHECK(moved[0].x == 78 && moved[0].w == 318 && moved[0].h == 32);
}

static void test_slot_below() {
    const Rect quit = rect(78, 386, 318, 32);
    KT_CHECK_EQ(slotBelow(quit, 36, 1), 422);
    KT_CHECK_EQ(slotBelow(quit, 36, 0), 386);
    KT_CHECK_EQ(slotBelow(quit, 36, 3), 494);
}

static void test_status_strip() {
    const Rect view = rect(100, 134, 200, 336);   // bottom at 470, strip y = 474
    Rect s = statusStrip(view, 505);
    KT_CHECK(s.x == 100 && s.w == 200 && s.y == 474 && s.h == 28);
    s = statusStrip(view, 490);
    KT_CHECK(s.y == 474 && s.h == 14);
    s = statusStrip(view, 470);                   // floorY - y - 2 is negative: no room
    KT_CHECK(s.h == 0);
    s = statusStrip(view, 504);                   // exactly 28
    KT_CHECK(s.h == 28);
    s = statusStrip(view, 477);                   // exactly 1
    KT_CHECK(s.h == 1);
    s = statusStrip(view, 476);                   // exactly 0
    KT_CHECK(s.h == 0);
    s = statusStrip(view, 475);                   // -1, clamped to 0
    KT_CHECK(s.h == 0);
}

int main() {
    test_vanilla_insert();
    test_ultrawide_keeps_xw();
    test_ragged_rejected_no_writes();
    test_gap_tolerance_boundary();
    test_pitch_and_order_rejected();
    test_minimum_and_invalid_args();
    test_slot_below();
    test_status_strip();
    return kt::Report("menu_layout_test");
}
