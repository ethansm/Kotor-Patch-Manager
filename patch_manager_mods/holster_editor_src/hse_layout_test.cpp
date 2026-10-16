// T1 host tests for HseLayout.h (holster editor modal layout and legend, pure; doc 20).
// Build/run: python3 scripts/run_tests.py hse_layout
#include "HseLayout.h"
#include "check.h"
#include <cmath>
#include <string.h>

using editkit::Box;
using namespace hse;

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

static void testLegend() {
    KT_CHECK(strcmp(legendText(), "L-drag rotate | R-drag move | Shift+wheel depth | wheel zoom | M-drag spin | X/Y/Z lock (bone axis; may change all 3 Rot) | Shift fine | Ctrl snap 5deg/0.01m") == 0);
}

int main() {
    testRectHelpers();
    testModalLayout();
    testLegend();
    return kt::Report("hse_layout");
}
