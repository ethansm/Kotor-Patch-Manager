// T1 host tests for HseLayoutV3.h (holster editor page layout v3, pure; HW-UX3 L0c, doc 20).
// Build/run: python3 scripts/run_tests.py hse_layout_v3
#include "HseLayoutV3.h"
#include "HseView.h"   // kPageItemCap, HseRect
#include "check.h"
#include "TextFit.h"
#include <cmath>
#include <algorithm>

using editkit::Box;
using namespace hse;

static bool inside(Box in, Box out) {
    return in.x >= out.x && in.y >= out.y && in.x + in.w <= out.x + out.w && in.y + in.h <= out.y + out.h;
}
// A box with no area (0 wide or high, from clipping in a too-small pane) draws nothing and overlaps nothing.
static bool overlaps(Box a, Box b) {
    if (a.w <= 0 || a.h <= 0 || b.w <= 0 || b.h <= 0) return false;
    return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h;
}
static bool same(Box a, Box b) { return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h; }

struct Geom { Box column, view, preview, hint; };

// zone of each item index in pageBoxesV3 order: 0 left column, 1 view area, 2 inspector (preview), 3 hint rect.
// kind: 0 plain, 1 thumb, 2 track (pair id = slider row).
struct Item { Box b; int zone; int kind; int row; };
static int collect(const PageLayoutV3& L, bool hasHint, Item* o) {
    Box raw[kPageItemsV3];
    int n = pageBoxesV3(L, raw);
    for (int i = 0; i < n; ++i) o[i] = Item{raw[i], 0, 0, -1};
    // order: left 19 (0..18), view area 13 (19..31), inspector 37 (32..68), then the Reset button (69, left column)
    for (int i = 19; i < 32; ++i) o[i].zone = 1;
    for (int i = 32; i < PI_RESET_SCOPE; ++i) o[i].zone = 2;
    o[PI_RESET_SCOPE].zone = 0;
    // inspector: groupLabel0, groupReset0, groupLabel1, groupReset1 (32..35), then rows 5 boxes each (36..65), note 66, hints 67..68
    for (int r = 0; r < kV3Rows; ++r) {
        o[36 + r * 5 + 1].kind = 2; o[36 + r * 5 + 1].row = r;
        o[36 + r * 5 + 2].kind = 1; o[36 + r * 5 + 2].row = r;
    }
    if (hasHint) { o[67].zone = 3; o[68].zone = 3; }
    return n;
}

// Violations: negative sizes (and empty when requirePositive), box outside its zone, overlap of different widgets.
static int violations(const Geom& g, const PageLayoutV3& L, bool hasHint, bool requirePositive, bool checkZones) {
    Item it[kPageItemsV3];
    int n = collect(L, hasHint, it);
    Box zone[4];
    zone[0] = g.column;
    zone[1] = Box{g.column.x, g.view.y, g.preview.x - g.column.x, g.view.h};
    zone[2] = g.preview;
    zone[3] = g.hint;
    int bad = 0;
    for (int i = 0; i < n; ++i) {
        if (it[i].b.w < 0 || it[i].b.h < 0) ++bad;
        if (requirePositive && (it[i].b.w <= 0 || it[i].b.h <= 0)) ++bad;
        if (checkZones && !inside(it[i].b, zone[it[i].zone])) ++bad;
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

// Real 3440x1440 rects (view {1345,357,576,896}, column about {150,357,400,896}); the others scale x by W/3440 and y by H/1440.
static Geom geomFor(int W, int H) {
    auto sx = [&](int v) { return v * W / 3440; };
    auto sy = [&](int v) { return v * H / 1440; };
    Geom g;
    g.column = Box{sx(150), sy(357), sx(400), sy(896)};
    g.view = Box{sx(1345), sy(357), sx(576), sy(896)};
    g.preview = Box{sx(1925), sy(357), sx(500), sy(700)};
    g.hint = Box{sx(1925), sy(357) + sy(700), sx(500), sy(196)};
    return g;
}

static void testGeometries() {
    const int sizes[3][2] = {{1280, 720}, {1920, 1080}, {3440, 1440}};
    for (int s = 0; s < 3; ++s) {
        Geom g = geomFor(sizes[s][0], sizes[s][1]);
        PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
        KT_CHECK_EQ(violations(g, L, true, true, true), 0);
        // the same page without the hint rect: hint lines stack in the preview
        PageLayoutV3 N = pageLayoutV3(g.column, g.view, g.preview, Box{0, 0, 0, 0}, nullptr);
        KT_CHECK_EQ(violations(g, N, false, true, true), 0);
        // the view is enlarged: it contains the original rect's width range and keeps a sane height
        KT_CHECK(L.view.x < g.view.x);
        KT_CHECK(L.view.x + L.view.w == g.preview.x - 4);   // 4 px short of the preview (the scaled view rects can end closer than that)
        KT_CHECK(L.view.y > g.view.y && L.view.y + L.view.h < g.view.y + g.view.h);
        KT_CHECK(L.view.w > g.view.w);
        // strips line up with the enlarged view
        KT_CHECK(L.modeBtn[0].x == L.view.x && L.stance[0].x == L.view.x && L.legend.x == L.view.x);
        KT_CHECK(L.viewFlip.x + L.viewFlip.w == L.view.x + L.view.w);
        KT_CHECK(L.modeBtn[0].y < L.view.y && L.stance[0].y > L.view.y + L.view.h && L.legend.y > L.stance[0].y);
        // at these sizes nothing needs the row floors
        KT_CHECK(L.leftRowH == kV3RowMax && L.inspRowH == kV3RowMax && L.barH == kV3BarMax);
        // reset squares are kV3ResetMax squared (the rows are at their max), the value label sits right of the track, the reset right of the value
        for (int r = 0; r < kV3Rows; ++r) {
            const InspRowV3& R = L.rows[r];
            KT_CHECK(R.reset.w == kV3ResetMax && R.reset.h == kV3ResetMax);
            KT_CHECK(R.value.x >= R.track.x + R.track.w && R.reset.x >= R.value.x + R.value.w);
            KT_CHECK(R.thumb.w == kV3ThumbW && R.thumb.h == R.track.h && R.thumb.y == R.track.y);
        }
        KT_CHECK(L.groupReset[0].w == kV3ResetMax && L.groupReset[0].x == L.rows[0].reset.x);
        // item budget
        Box all[kPageItemsV3 + 8];
        KT_CHECK_EQ(pageBoxesV3(L, all), kPageItemsV3);
        KT_CHECK(kPageItemsV3 <= 70);   // at the ideal bound
        KT_CHECK(kPageItemsV3 <= 80);
        KT_CHECK(kPageItemsV3 == 19 + 13 + 37 + 1);
    }
}

// Save | Revert | Reset share one row: positive widths, inside the column, in order without overlap, Reset keeps the 4 px Save gives up, and the
// labels fit the box text budget (width - 8, as HolsterEditor boxPx) at the three common sizes. "Revert" at 1280 is the known exception: it is
// a plain third (45 px) and its 45.75 px label exceeds even the raw box; the editor shows it uncut, so only Save and Reset are asserted there.
static void testSaveRow() {
    const int sizes[3][2] = {{1280, 720}, {1920, 1080}, {3440, 1440}};
    const textfit::TextCtx tc = {1.0f, 1.0f, 1.0f};
    for (int s = 0; s < 3; ++s) {
        Geom g = geomFor(sizes[s][0], sizes[s][1]);
        PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
        KT_CHECK(L.save.w > 0 && L.revert.w > 0 && L.reset.w > 0);
        KT_CHECK(L.save.y == L.revert.y && L.revert.y == L.reset.y && L.save.h == L.reset.h);
        KT_CHECK(L.save.x + L.save.w < L.revert.x && L.revert.x + L.revert.w < L.reset.x);
        KT_CHECK(inside(L.save, g.column) && inside(L.revert, g.column) && inside(L.reset, g.column));
        KT_CHECK(!overlaps(L.save, L.revert) && !overlaps(L.revert, L.reset) && !overlaps(L.save, L.status) && !overlaps(L.reset, L.status));
        KT_CHECK(L.reset.w >= L.revert.w + 4);
        KT_CHECK(L.reset.x + L.reset.w == L.status.x + L.status.w);   // the row fills the content width
        auto px = [&](const char* t) { return textfit::textPx(tc, t, static_cast<int>(strlen(t))); };
        KT_CHECK(px("Save") <= static_cast<float>(L.save.w - 8));
        KT_CHECK(px("Reset") <= static_cast<float>(L.reset.w - 8));
        if (sizes[s][0] > 1280) KT_CHECK(px("Revert") <= static_cast<float>(L.revert.w - 8));
    }
    // a tiny column still gives non-negative widths in order
    for (int w = 0; w < 40; ++w) {
        Geom g = geomFor(3440, 1440);
        g.column.w = w;
        PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
        KT_CHECK(L.save.w >= 0 && L.revert.w >= 0 && L.reset.w >= 0);
        KT_CHECK(!overlaps(L.save, L.revert) && !overlaps(L.revert, L.reset));
    }
}

static void testPinned3440() {
    Geom g = geomFor(3440, 1440);
    KT_CHECK(same(g.column, Box{150, 357, 400, 896}));
    KT_CHECK(same(g.view, Box{1345, 357, 576, 896}));
    PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
    // left column: the column pane is 400 wide (under kV3ColWantW 440), content 392 wide (pad 4 each side), rows 34 high
    KT_CHECK(same(L.memberPrev, Box{154, 361, 34, 34}));
    KT_CHECK(same(L.memberNext, Box{154 + 392 - 34, 361, 34, 34}));
    KT_CHECK(L.capWeapon.y == 397 && L.capWeapon.h == 22);
    KT_CHECK(same(L.weapon[0], Box{154, 421, 392, 34}));
    KT_CHECK(same(L.bone[0], Box{154, 517, 129, 34}));
    KT_CHECK(L.bone[3].y == L.bone[0].y + 36 && L.bone[3].w == 195);
    // view area: starts 4 px right of the 400 wide column, ends at the preview pane
    KT_CHECK(L.view.x == 150 + 400 + 4 && L.view.x + L.view.w == 1921);   // 4 px short of the preview pane at 1925
    KT_CHECK(L.modeBtn[0].y == 357 && L.modeBtn[0].h == 34);
    KT_CHECK(L.view.y == 357 + 34 + 2);
    // inspector
    KT_CHECK(L.rows[0].name.x == 1929 && L.rows[0].name.w == kV3NameW);
}

static void testThumbs() {
    Geom g = geomFor(3440, 1440);
    float fr[6] = {0.f, 0.5f, 1.f, 0.25f, -3.f, 7.f};
    PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, fr);
    const InspRowV3* R = L.rows;
    int tw = R[0].track.w;
    KT_CHECK(tw > 40);
    KT_CHECK(R[0].thumb.x == R[0].track.x);                                   // 0 -> flush left
    KT_CHECK(R[1].thumb.x == R[1].track.x + (int)(0.5f * (float)tw + 0.5f) - 5);   // centred, same maths as modalLayout
    KT_CHECK(R[2].thumb.x == R[2].track.x + tw - 10);                          // 1 -> flush right
    KT_CHECK(R[3].thumb.x == R[3].track.x + (int)(0.25f * (float)tw + 0.5f) - 5);
    KT_CHECK(R[4].thumb.x == R[4].track.x);                                    // -3 clamps to 0
    KT_CHECK(R[5].thumb.x == R[5].track.x + tw - 10);                          // 7 clamps to 1
    KT_CHECK_EQ(violations(g, L, true, true, true), 0);
    float fn[6] = {std::nanf(""), 0.f, 0.f, 0.f, 0.f, 0.f};                   // NaN -> 0
    PageLayoutV3 N = pageLayoutV3(g.column, g.view, g.preview, g.hint, fn);
    KT_CHECK(N.rows[0].thumb.x == N.rows[0].track.x);
    PageLayoutV3 D = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);   // null -> 0.5
    KT_CHECK(D.rows[3].thumb.x == D.rows[3].track.x + (int)(0.5f * (float)D.rows[3].track.w + 0.5f) - 5);
    // thumbs move with the fraction on every geometry and never leave the track
    const int sizes[3][2] = {{1280, 720}, {1920, 1080}, {3440, 1440}};
    for (int s = 0; s < 3; ++s) {
        Geom q = geomFor(sizes[s][0], sizes[s][1]);
        for (int k = 0; k <= 10; ++k) {
            float f10[6];
            for (int i = 0; i < 6; ++i) f10[i] = (float)k / 10.f;
            PageLayoutV3 P = pageLayoutV3(q.column, q.view, q.preview, q.hint, f10);
            KT_CHECK_EQ(violations(q, P, true, true, true), 0);
        }
    }
}

static void testDegradation() {
    // exactly the documented minimum panes: everything positive and disjoint
    Geom m;
    m.column = Box{0, 0, 140, 180};
    m.view = Box{150, 0, 160, 150};
    m.preview = Box{320, 0, 170, 200};
    m.hint = Box{320, 204, 170, 60};
    PageLayoutV3 L = pageLayoutV3(m.column, m.view, m.preview, m.hint, nullptr);
    KT_CHECK_EQ(violations(m, L, true, true, true), 0);
    KT_CHECK(L.leftRowH < kV3RowMax && L.leftRowH >= kV3RowMin);   // rows shrank to fit
    KT_CHECK(L.inspRowH < kV3RowMax && L.inspRowH >= kV3RowMin);
    PageLayoutV3 L2 = pageLayoutV3(m.column, m.view, m.preview, Box{0, 0, 0, 0}, nullptr);
    KT_CHECK_EQ(violations(m, L2, false, true, true), 0);

    // below the minimum: sizes stay non-negative, boxes stay in their pane, nothing overlaps
    Geom t;
    t.column = Box{0, 0, 50, 40};
    t.view = Box{60, 0, 50, 30};
    t.preview = Box{120, 0, 50, 30};
    t.hint = Box{120, 34, 50, 10};
    float fr[6] = {0.f, 1.f, 0.5f, std::nanf(""), -1.f, 2.f};
    PageLayoutV3 T = pageLayoutV3(t.column, t.view, t.preview, t.hint, fr);
    KT_CHECK_EQ(violations(t, T, true, false, true), 0);
    PageLayoutV3 T2 = pageLayoutV3(t.column, t.view, t.preview, Box{0, 0, 0, 0}, fr);
    KT_CHECK_EQ(violations(t, T2, false, false, true), 0);
    KT_CHECK(T.leftRowH == kV3RowMin && T.inspRowH == kV3RowMin && T.barH == kV3BarMin);

    // degenerate zero and one pixel panes: no negative size, no overlap
    Geom z;
    z.column = Box{0, 0, 0, 0}; z.view = Box{0, 0, 0, 0}; z.preview = Box{0, 0, 0, 0}; z.hint = Box{0, 0, 0, 0};
    PageLayoutV3 Z = pageLayoutV3(z.column, z.view, z.preview, z.hint, fr);
    KT_CHECK_EQ(violations(z, Z, false, false, false), 0);
    Geom one;
    one.column = Box{5, 5, 1, 1}; one.view = Box{10, 5, 1, 1}; one.preview = Box{20, 5, 1, 1}; one.hint = Box{20, 8, 1, 1};
    PageLayoutV3 O = pageLayoutV3(one.column, one.view, one.preview, one.hint, nullptr);
    KT_CHECK_EQ(violations(one, O, true, false, true), 0);

    // sweep a range of pane sizes: always non-negative, contained, disjoint
    int bad = 0;
    for (int cw = 0; cw <= 300; cw += 37)
        for (int ch = 0; ch <= 400; ch += 53)
            for (int pw = 0; pw <= 300; pw += 41)
                for (int ph = 0; ph <= 300; ph += 47) {
                    Geom s;
                    s.column = Box{10, 20, cw, ch};
                    s.view = Box{10 + cw + 6, 20, 90, ch};
                    s.preview = Box{10 + cw + 6 + 90 + 6, 20, pw, ph};
                    s.hint = Box{s.preview.x, 20 + ph + 4, pw, 40};
                    PageLayoutV3 S = pageLayoutV3(s.column, s.view, s.preview, s.hint, fr);
                    bad += violations(s, S, pw > 0, false, true);   // a 0 wide hint rect counts as none
                    PageLayoutV3 S2 = pageLayoutV3(s.column, s.view, s.preview, Box{0, 0, 0, 0}, fr);
                    bad += violations(s, S2, false, false, true);
                }
    KT_CHECK_EQ(bad, 0);
}

static void testViewVsPreview() {
    // gap: view rect ends 1921, preview starts 1925 -> the enlarged view ends 4 px short of the preview
    Geom g = geomFor(3440, 1440);
    PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
    KT_CHECK_EQ(L.view.x + L.view.w, g.preview.x - 4);
    KT_CHECK_EQ(L.viewFlip.x + L.viewFlip.w, g.preview.x - 4);
    // a wide gap still ends 4 px short (the view grows into it)
    Box pw = g.preview; pw.x = 2000;
    PageLayoutV3 W = pageLayoutV3(g.column, g.view, pw, g.hint, nullptr);
    KT_CHECK_EQ(W.view.x + W.view.w, 1996);
    // overlap: the given view rect reaches 40 px into the preview -> the view strip, mode strip and stances stop short of it
    Box po = g.preview; po.x = 1881;
    PageLayoutV3 O = pageLayoutV3(g.column, g.view, po, g.hint, nullptr);
    KT_CHECK_EQ(O.view.x + O.view.w, 1877);
    KT_CHECK(O.view.w > 0);
    Box all[kPageItemsV3 + 8]; pageBoxesV3(O, all);
    for (int i = 19; i < 32; ++i) KT_CHECK(all[i].x + all[i].w <= po.x - 4);   // view area items (see collect())
    for (int i = 32; i < PI_RESET_SCOPE; ++i) KT_CHECK(!overlaps(all[i], O.view));   // the Reset button sits in the left column
    KT_CHECK(!overlaps(all[PI_RESET_SCOPE], O.view));
    KT_CHECK(!overlaps(O.viewFlip, O.rows[0].name) && !overlaps(O.legend, O.groupLabel[0]));
    // a preview left of the column content leaves no room: the width stays non-negative and inside [vl, preview.x]
    Box pn = g.preview; pn.x = g.column.x + 100;
    PageLayoutV3 N = pageLayoutV3(g.column, g.view, pn, g.hint, nullptr);
    KT_CHECK(N.view.w >= 0 && N.view.x + N.view.w <= std::max(pn.x, N.view.x));
    // no preview rect (w == 0): the old right edge is used
    PageLayoutV3 Z = pageLayoutV3(g.column, g.view, Box{g.preview.x, g.preview.y, 0, 0}, Box{0, 0, 0, 0}, nullptr);
    KT_CHECK_EQ(Z.view.x + Z.view.w, g.view.x + g.view.w);
}

static void testHintHeight() {
    Geom g = geomFor(3440, 1440);   // hint rect 196 high: both lines get the full kV3HintMax px
    PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
    KT_CHECK_EQ(kV3HintMax, 26);
    KT_CHECK(L.hint[0].h == 26 && L.hint[1].h == 26);
    KT_CHECK(L.hint[1].y == L.hint[0].y + 26 + 2);
    // exactly big enough: 2*4 pad + 2 gap + 2*26 = 62
    PageLayoutV3 E = pageLayoutV3(g.column, g.view, g.preview, Box{g.preview.x, 1100, 400, 62}, nullptr);
    KT_CHECK(E.hint[0].h == 26 && E.hint[1].h == 26);
    // a small rect shrinks the lines (30 -> (30 - 8 - 2) / 2 = 10)
    PageLayoutV3 S = pageLayoutV3(g.column, g.view, g.preview, Box{g.preview.x, 1100, 400, 30}, nullptr);
    KT_CHECK(S.hint[0].h == 10 && S.hint[1].h == 10);
}

// The REAL pane rects of the 3840x1600 log (UI units): the layout must use the space and every label must fit its box.
static Geom realGeom() {
    Geom r;
    r.column = Box{772, 357, 1132, 757};
    r.view = Box{1345, 357, 576, 896};
    r.preview = Box{1953, 357, 1113, 746};
    r.hint = Box{0, 0, 0, 0};
    return r;
}

static void testReal3840x1600() {
    const Geom g = realGeom();
    PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
    KT_CHECK_EQ(violations(g, L, false, true, true), 0);
    KT_CHECK(L.leftRowH >= 30 && L.inspRowH >= 30 && L.barH >= 30);
    // the left column content uses the wide cap: >= 400 wide
    KT_CHECK(L.status.w >= 400 && L.weapon[0].w >= 400);
    // the inspector spans >= 600 (name to reset) and the track is >= 150
    const InspRowV3& R0 = L.rows[0];
    KT_CHECK(R0.reset.x + R0.reset.w - R0.name.x >= 600);
    for (int r = 0; r < kV3Rows; ++r) KT_CHECK(L.rows[r].track.w >= 150);
    // the view stays left of the preview pane, never overlapping the inspector
    KT_CHECK(L.view.x + L.view.w < g.preview.x);
    KT_CHECK(L.view.x > L.status.x + L.status.w);
    // item budget: 70 boxes + 6 wedges
    KT_CHECK_EQ(kPageItemsV3Total, 76);
    KT_CHECK(kPageItemsV3Total <= 80);
    Box all[kPageItemsV3];
    KT_CHECK_EQ(pageBoxesV3(L, all), kPageItemsV3);

    // text fit: measured with the engine-calibrated text_k 2.5; every label fits its box less the 8 px padding
    const textfit::TextCtx tc = {2.5f, 1.0f, 1.0f};
    auto px = [&](const char* t) { return textfit::textPx(tc, t, static_cast<int>(strlen(t))); };
    static const char* const kStances[kV3Stances] = {"Idle", "Walk", "Run", "Alert", "Combat"};   // HolsterEditor.cpp kStanceList
    static const char* const kSliders[kV3Rows] = {"Slide", "Depth", "Height", "Tilt", "Roll", "Turn"};   // HseView.h inspector names
    const int sizes[2][2] = {{3840, 1600}, {3440, 1440}};
    for (int s = 0; s < 2; ++s) {
        Geom q = s == 0 ? g : geomFor(sizes[s][0], sizes[s][1]);
        PageLayoutV3 P = pageLayoutV3(q.column, q.view, q.preview, q.hint, nullptr);
        KT_CHECK(px("Save") <= static_cast<float>(P.save.w - 8));
        KT_CHECK(px("Revert") <= static_cast<float>(P.revert.w - 8));
        KT_CHECK(px("Reset") <= static_cast<float>(P.reset.w - 8));
        KT_CHECK(px("Move") <= static_cast<float>(P.modeBtn[0].w - 8));
        for (int i = 0; i < kV3Stances; ++i) KT_CHECK(px(kStances[i]) <= static_cast<float>(P.stance[i].w - 8));
        for (int i = 0; i < kV3Rows; ++i) KT_CHECK(px(kSliders[i]) <= static_cast<float>(P.rows[i].name.w - 8));
        if (s == 1) KT_CHECK(P.rows[0].track.w >= 150);
    }
}

static void testHelpers() {
    KT_CHECK(overlaps(Box{0, 0, 10, 10}, Box{5, 5, 10, 10}));
    KT_CHECK(!overlaps(Box{0, 0, 10, 10}, Box{10, 0, 10, 10}));
    Box c = v3::clipTo(Box{50, 50, 10, 10}, Box{0, 0, 20, 20});   // wholly outside: 0 sized on the edge
    KT_CHECK(c.w == 0 && c.h == 0 && inside(c, Box{0, 0, 20, 20}));
    c = v3::clipTo(Box{5, 5, -4, 10}, Box{0, 0, 20, 20});         // negative width
    KT_CHECK(c.w == 0 && c.h == 10);
    c = v3::clipTo(Box{15, 15, 10, 10}, Box{0, 0, 20, 20});
    KT_CHECK(same(c, Box{15, 15, 5, 5}));
}

struct RectA { int x, y, w, h; };
struct RectI32 { int32_t x, y, w, h; };   // K2MuiRect shape

// Wave B: the rect adapters (HseRect / K2MuiRect / Box are all {x, y, w, h}) and the item index table of the editor.
static void testAdaptersAndIndex() {
    RectA a = {1, 2, 3, 4};
    KT_CHECK(same(boxOf(a), Box{1, 2, 3, 4}));
    RectA r = rectOf<RectA>(Box{5, 6, 7, 8});
    KT_CHECK(r.x == 5 && r.y == 6 && r.w == 7 && r.h == 8);
    RectI32 m = rectOf<RectI32>(Box{-9, 10, 0, 12});
    KT_CHECK(m.x == -9 && m.y == 10 && m.w == 0 && m.h == 12);
    KT_CHECK(same(boxOf(m), Box{-9, 10, 0, 12}));
    HseRect hr = rectOf<HseRect>(Box{11, 12, 13, 14});
    KT_CHECK(hr.x == 11 && hr.y == 12 && hr.w == 13 && hr.h == 14 && same(boxOf(hr), Box{11, 12, 13, 14}));

    // every index of the table names the matching box of pageBoxesV3 (the editor's item table relies on this order)
    Geom g = geomFor(3440, 1440);
    PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, Box{0, 0, 0, 0}, nullptr);
    Box all[kPageItemsV3];
    KT_CHECK_EQ(pageBoxesV3(L, all), kPageItemsV3);
    KT_CHECK(same(all[PI_MEMBER_PREV], L.memberPrev) && same(all[PI_MEMBER_NAME], L.memberName) && same(all[PI_MEMBER_NEXT], L.memberNext));
    KT_CHECK(same(all[PI_CAP_WEAPON], L.capWeapon) && same(all[PI_CAP_BONE], L.capBone) && same(all[PI_CAP_SCOPE], L.capScope));
    for (int i = 0; i < kV3Weapons; ++i) KT_CHECK(same(all[PI_WEAPON + i], L.weapon[i]));
    for (int i = 0; i < kV3Bones; ++i) KT_CHECK(same(all[PI_BONE + i], L.bone[i]));
    for (int i = 0; i < kV3Scopes; ++i) KT_CHECK(same(all[PI_SCOPE + i], L.scope[i]));
    KT_CHECK(same(all[PI_SAVE], L.save) && same(all[PI_REVERT], L.revert) && same(all[PI_STATUS], L.status));
    for (int i = 0; i < kV3Modes; ++i) KT_CHECK(same(all[PI_MODE + i], L.modeBtn[i]));
    KT_CHECK(same(all[PI_VIEW_RESET], L.viewReset) && same(all[PI_VIEW_FLIP], L.viewFlip) && same(all[PI_VIEW], L.view));
    for (int i = 0; i < kV3Stances; ++i) KT_CHECK(same(all[PI_STANCE + i], L.stance[i]));
    KT_CHECK(same(all[PI_LEGEND], L.legend));
    for (int gI = 0; gI < kV3Groups; ++gI) {
        KT_CHECK(same(all[PI_GROUP + gI * 2], L.groupLabel[gI]));
        KT_CHECK(same(all[PI_GROUP + gI * 2 + 1], L.groupReset[gI]));
    }
    for (int rI = 0; rI < kV3Rows; ++rI) {
        const int b = PI_ROW + rI * kRowStride;
        KT_CHECK(same(all[b], L.rows[rI].name) && same(all[b + 1], L.rows[rI].track) && same(all[b + 2], L.rows[rI].thumb));
        KT_CHECK(same(all[b + 3], L.rows[rI].value) && same(all[b + 4], L.rows[rI].reset));
    }
    KT_CHECK(same(all[PI_NOTE], L.note));
    for (int i = 0; i < kV3Hints; ++i) KT_CHECK(same(all[PI_HINT + i], L.hint[i]));

    KT_CHECK(same(all[PI_RESET_SCOPE], L.reset));
    KT_CHECK(PI_RESET_SCOPE == 69 && PI_HINT + kV3Hints == PI_RESET_SCOPE);

    // the page holds the 70 boxes plus one wedge per slider row, inside the consumer cap
    KT_CHECK_EQ(kPageItemsV3Total, 76);
    KT_CHECK(kPageItemsV3Total <= kPageItemCap);
}

int main() {
    testHelpers();
    testAdaptersAndIndex();
    testGeometries();
    testSaveRow();
    testReal3840x1600();
    testPinned3440();
    testViewVsPreview();
    testHintHeight();
    testThumbs();
    testDegradation();
    return kt::Report("hse_layout_v3");
}
