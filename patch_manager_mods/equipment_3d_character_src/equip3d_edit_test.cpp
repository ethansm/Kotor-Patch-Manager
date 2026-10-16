// T1 host tests for Equip3dEdit.h (holster editor UI logic, pure; doc 20).
// Build/run: python3 scripts/run_tests.py equip3d_edit
#include "Equip3dEdit.h"
#include "check.h"
#include <cmath>
#include <string.h>

using namespace eq3d;

static bool approx(float a, float b, float eps = 1e-4f) { return std::fabs(a - b) <= eps; }
static bool inside(Box in, Box out) {
    return in.x >= out.x && in.y >= out.y && in.x + in.w <= out.x + out.w && in.y + in.h <= out.y + out.h;
}
static bool overlaps(Box a, Box b) {
    return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h;
}
static bool same(Box a, Box b) { return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h; }

// ---- layout checker ---------------------------------------------------------------------------------------------
struct Item { Box b; int row; int kind; };   // kind: 0 plain, 1 thumb, 2 track (row = slider row, -1 for plain boxes)
static int collectModal(const ModalLayout& L, Item* o) {
    int n = 0;
    o[n++] = Item{L.header, -1, 0}; o[n++] = Item{L.handPrev, -1, 0}; o[n++] = Item{L.handNext, -1, 0};
    for (int i = 0; i < kModalScope; ++i) o[n++] = Item{L.scope[i], -1, 0};
    for (int i = 0; i < kModalLock; ++i) o[n++] = Item{L.lock[i], -1, 0};
    for (int r = 0; r < kModalRows; ++r) {
        const ModalRow& R = L.rows[r];
        o[n++] = Item{R.name, r, 0}; o[n++] = Item{R.minus, r, 0}; o[n++] = Item{R.track, r, 2};
        o[n++] = Item{R.thumb, r, 1}; o[n++] = Item{R.value, r, 0}; o[n++] = Item{R.plus, r, 0};
    }
    o[n++] = Item{L.bonePrev, -1, 0}; o[n++] = Item{L.boneName, -1, 0}; o[n++] = Item{L.boneNext, -1, 0};
    for (int i = 0; i < kModalButtons; ++i) o[n++] = Item{L.buttons[i], -1, 0};
    o[n++] = Item{L.warning, -1, 0};
    return n;
}
// Count of violations: empty or negative boxes, boxes outside the pane, overlaps. A thumb may sit inside its own track
// (and must); every other pair must be disjoint.
static int layoutViolations(Box pane, const ModalLayout& L) {
    Item it[64];
    int n = collectModal(L, it);
    int bad = 0;
    for (int i = 0; i < n; ++i) {
        if (it[i].b.w <= 0 || it[i].b.h <= 0) ++bad;
        if (!inside(it[i].b, pane)) ++bad;
        for (int j = i + 1; j < n; ++j) {
            bool thumbTrack = it[i].row >= 0 && it[i].row == it[j].row &&
                              ((it[i].kind == 1 && it[j].kind == 2) || (it[i].kind == 2 && it[j].kind == 1));
            if (thumbTrack) {
                const Item& th = it[i].kind == 1 ? it[i] : it[j];
                const Item& tr = it[i].kind == 1 ? it[j] : it[i];
                if (!inside(th.b, tr.b)) ++bad;
                continue;
            }
            if (overlaps(it[i].b, it[j].b)) ++bad;
        }
    }
    return bad;
}

static void testRectHelpers() {
    KT_CHECK(overlaps(Box{0, 0, 10, 10}, Box{5, 5, 10, 10}));      // sanity: the checker sees a real overlap
    KT_CHECK(!overlaps(Box{0, 0, 10, 10}, Box{10, 0, 10, 10}));    // touching edges are not an overlap
    KT_CHECK(!overlaps(Box{0, 0, 10, 10}, Box{0, 10, 10, 10}));
    KT_CHECK(inside(Box{2, 2, 5, 5}, Box{0, 0, 10, 10}));
    KT_CHECK(!inside(Box{6, 2, 5, 5}, Box{0, 0, 10, 10}));
}

static void testWeaponRow() {
    Box btn{100, 50, 90, 30};
    WeaponRowLayout r = weaponRow(btn, false, true, true);     // button off: plain weapon button
    KT_CHECK(r.weapons.x == 100 && r.weapons.y == 50 && r.weapons.w == 90 && r.weapons.h == 30);
    KT_CHECK(!r.showHolster && !r.showGear);
    r = weaponRow(btn, true, false, true);                      // on, but this item cannot holster
    KT_CHECK(r.weapons.x == 100 && r.weapons.w == 90 && !r.showHolster && !r.showGear);
    r = weaponRow(btn, true, true, false);                      // on, holster-capable, drawn
    KT_CHECK(r.showHolster && !r.showGear);
    KT_CHECK(r.holster.x == 100 && r.holster.y == 50 && r.holster.w == 30 && r.holster.h == 30);
    KT_CHECK(r.weapons.x == 132 && r.weapons.y == 50 && r.weapons.w == 58 && r.weapons.h == 30);
    KT_CHECK(r.weapons.x + r.weapons.w == btn.x + btn.w);       // weapon slot keeps the button's right edge
    KT_CHECK(r.holster.x + r.holster.w + 2 == r.weapons.x);     // 2 px gap between the slots
    r = weaponRow(btn, true, true, true);                       // holstered: gear square to the left
    KT_CHECK(r.showHolster && r.showGear);
    KT_CHECK(r.gear.x == 68 && r.gear.y == 50 && r.gear.w == 30 && r.gear.h == 30);
    KT_CHECK(r.gear.x + r.gear.w + 2 == btn.x);
    KT_CHECK(r.weapons.x == 132 && r.weapons.w == 58);          // holstered does not move the slots
    Box odd{0, 0, 91, 20};                                      // odd width: holster 30, gap 2, weapons 59
    r = weaponRow(odd, true, true, false);
    KT_CHECK(r.holster.w == 30 && r.weapons.x == 32 && r.weapons.w == 59 && r.weapons.x + r.weapons.w == 91);
    // containment: holster and weapons inside the button; gear inside the view, which extends left of the button
    Box view{60, 40, 200, 60};
    r = weaponRow(btn, true, true, true);
    KT_CHECK(inside(r.holster, btn) && inside(r.weapons, btn) && inside(btn, view) && inside(r.gear, view));
    KT_CHECK(!overlaps(r.holster, r.weapons) && !overlaps(r.gear, r.holster) && !overlaps(r.gear, r.weapons));
}

static void testModalLayout() {
    Box real{294, 134, 232, 336};
    ModalLayout L = modalLayout(real, nullptr);
    KT_CHECK_EQ(layoutViolations(real, L), 0);
    // pinned positions for the real pane (pad 4, gaps 2)
    KT_CHECK(L.header.x == 298 && L.header.y == 138 && L.header.w == 184 && L.header.h == 18);
    KT_CHECK(L.handPrev.x == 484 && L.handPrev.y == 138 && L.handNext.x == 504 && L.handNext.x + 18 == 522);
    KT_CHECK(L.warning.x == 298 && L.warning.y == 158 && L.warning.w == 224 && L.warning.h == 14);
    KT_CHECK(L.scope[0].y == 174 && L.scope[0].h == 20 && L.scope[1].x == 373 && L.scope[2].x == 448);
    KT_CHECK(L.lock[0].y == 196 && L.lock[0].h == 18 && L.lock[2].x == 448);
    KT_CHECK(same(L.rows[0].name, Box{298, 216, 40, 20}));
    KT_CHECK(same(L.rows[0].minus, Box{340, 216, 18, 20}));
    KT_CHECK(same(L.rows[0].track, Box{360, 216, 88, 20}));
    KT_CHECK(same(L.rows[0].value, Box{450, 216, 52, 20}));
    KT_CHECK(same(L.rows[0].plus, Box{504, 216, 18, 20}));
    KT_CHECK(L.rows[5].track.y == 326);                          // six rows, 22 px apart
    KT_CHECK(same(L.bonePrev, Box{298, 348, 18, 18}));
    KT_CHECK(same(L.boneName, Box{318, 348, 184, 18}));
    KT_CHECK(same(L.boneNext, Box{504, 348, 18, 18}));
    KT_CHECK(same(L.buttons[0], Box{298, 368, 54, 22}));
    KT_CHECK(same(L.buttons[3], Box{466, 368, 54, 22}));
    // thumbs: default 0.5; extremes and out-of-range values stay inside the track
    KT_CHECK(L.rows[0].thumb.x == 399 && L.rows[0].thumb.y == 216 && L.rows[0].thumb.w == 10 && L.rows[0].thumb.h == 20);
    KT_CHECK(L.rows[0].thumb.x + 5 == L.rows[0].track.x + 44);   // centred at track.x + 0.5 * track.w
    float fr[6] = {0.f, 0.5f, 1.f, 0.25f, -3.f, 7.f};
    ModalLayout T = modalLayout(real, fr);
    KT_CHECK(T.rows[0].thumb.x == 360);                          // 0 -> flush left
    KT_CHECK(T.rows[1].thumb.x == 399);                          // 0.5
    KT_CHECK(T.rows[2].thumb.x == 438);                          // 1 -> flush right (track ends at 448)
    KT_CHECK(T.rows[3].thumb.x == 377);                          // 0.25 -> track.x + 22 - 5
    KT_CHECK(T.rows[4].thumb.x == 360);                          // -3 clamps to 0
    KT_CHECK(T.rows[5].thumb.x == 438);                          // 7 clamps to 1
    KT_CHECK_EQ(layoutViolations(real, T), 0);
    float fn[6] = {std::nanf(""), 0.f, 0.f, 0.f, 0.f, 0.f};     // NaN -> 0
    ModalLayout N = modalLayout(real, fn);
    KT_CHECK(N.rows[0].thumb.x == 360);
    // fits at the minimum pane size and in a larger pane
    ModalLayout Small = modalLayout(Box{0, 0, 200, 330}, nullptr);
    KT_CHECK_EQ(layoutViolations(Box{0, 0, 200, 330}, Small), 0);
    ModalLayout Big = modalLayout(Box{50, 60, 320, 400}, nullptr);
    KT_CHECK_EQ(layoutViolations(Box{50, 60, 320, 400}, Big), 0);
    KT_CHECK(Big.rows[0].track.w == 320 - 8 - 136);              // track is the flexible part
}

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

static void testText() {
    KT_CHECK(strcmp(legendText(), "L-drag rotate | R-drag move | Shift+wheel depth | wheel zoom | M-drag spin | X/Y/Z lock (bone axis; may change all 3 Rot) | Shift fine | Ctrl snap 5deg/0.01m") == 0);
    KT_CHECK(strcmp(tipFor(EcHolster), "Holster view: show weapons stowed") == 0);
    KT_CHECK(strcmp(tipFor(EcGear), "Edit holster placement") == 0);
    KT_CHECK(strcmp(tipFor(EcCount), "") == 0);
}

int main() {
    testRectHelpers();
    testWeaponRow();
    testModalLayout();
    testSlider();
    testRepeat();
    testDragMode();
    testMidClick();
    testFmtValue();
    testText();
    return kt::Report("equip3d_edit");
}
