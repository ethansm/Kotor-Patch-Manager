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

struct Geom { Box column, view, preview, hint; };   // hint: still passed to pageLayoutV3, which ignores it (the description box lives in the left column now)
static Geom realGeom();

// Independent model of the left column height (rows only; derived from the layout formula, not by calling buildLeft): the distance from the column top to
// the bottom of the content plus the bottom pad, at row height r. The three section captions are full row boxes (a shorter box drew no text in game);
// kPad above and below; every row is followed by kGap.
//   roomy:   member 1 row, 3 captions, 2 weapons, 3 bone rows, 3 scope rows + spacer, save row, status (no gap) = 13 (r + kGap) + r + kSpacer + 2 kPad = 14 r + 70
//   compact: member 1 row, 3 captions, 2 weapons, 2 bone rows, 1 scope row + spacer, save row, status           = 10 (r + kGap) + r + kSpacer + 2 kPad = 11 r + 58
static int leftNeed(int r, bool roomy) {
    return (roomy ? 13 : 10) * (r + 4) + r + 6 + 2 * 6;
}
// The two-pass fit, restated: pass 0 keeps kV3DescWantH + kGap free under the status line (the bound is the column height minus that); only when no row height
// from kV3LeftRowMax down to kV3RowMin fits it, pass 1 uses the whole column height. Roomy form from kV3RoomyRowMin up, compact below; none fits -> kV3RowMin.
static int expectedLeftRowH(int colH, bool* pass1 = nullptr) {
    for (int pass = 0; pass < 2; ++pass) {
        const int bound = colH - (pass == 0 ? kV3DescWantH + 4 : 0);
        for (int r = kV3LeftRowMax; r >= kV3RowMin; --r)
            if (leftNeed(r, r >= kV3RoomyRowMin) <= bound) { if (pass1) *pass1 = pass == 1; return r; }
    }
    if (pass1) *pass1 = true;
    return kV3RowMin;
}

// zone of each item index in pageBoxesV3 order: 0 left column (incl. the left description box hint[0] and its text area hint[1]), 1 view area, 2 inspector (preview).
// kind: 0 plain, 1 thumb, 2 track (pair id = slider row), 3 left description box, 4 its text area, 5 right description box (over LB_DESC), 6 its text area.
struct Item { Box b; int zone; int kind; int row; };
static int collect(const PageLayoutV3& L, Item* o) {
    Box raw[kPageItemsV3];
    int n = pageBoxesV3(L, raw);
    for (int i = 0; i < n; ++i) o[i] = Item{raw[i], 0, 0, -1};
    // order: left 19 (0..18), view area 13 (19..31), inspector 37 (32..68), then the "..." toggle (69, left column), the right description box (70, 71), Remove (72, inspector)
    for (int i = 19; i < 32; ++i) o[i].zone = 1;
    for (int i = 32; i < PI_RESET_SCOPE; ++i) o[i].zone = 2;
    o[PI_RESET_SCOPE].zone = 0;
    o[PI_REMOVE].zone = 2;                            // the Remove button sits in the inspector zone, under the note
    // inspector: groupLabel0, groupReset0, groupLabel1, groupReset1 (32..35), then rows 5 boxes each (36..65), note 66
    for (int r = 0; r < kV3Rows; ++r) {
        o[36 + r * 5 + 1].kind = 2; o[36 + r * 5 + 1].row = r;
        o[36 + r * 5 + 2].kind = 1; o[36 + r * 5 + 2].row = r;
    }
    // the two hint boxes (67, 68) belong to the LEFT column now, not the inspector
    o[PI_HINT].zone = 0; o[PI_HINT].kind = 3;
    o[PI_HINT + 1].zone = 0; o[PI_HINT + 1].kind = 4;
    o[PI_HINTR].zone = 0; o[PI_HINTR].kind = 5;       // zone not checked (kinds 5 / 6 are exempt: the pane is not one of the three zones)
    o[PI_HINTR + 1].zone = 0; o[PI_HINTR + 1].kind = 6;
    return n;
}
static bool isZeroBox(Box b) { return b.x == 0 && b.y == 0 && b.w == 0 && b.h == 0; }

// Independent model of the inspector height (from the header constants, not from buildInspector): 2 groups of (label + 3 rows) at rr + kGap each,
// kSpacer between the groups, the note (rr * 2 / 3 high) + kGap, the Remove row (rr high) + kGap, kPad above and below, minus the last kGap.
static int inspNeed(int rr) { return 2 * v3::kPad + 8 * (rr + v3::kGap) + v3::kSpacer + rr * 2 / 3 + (rr + v3::kGap); }

// Violations: negative sizes (and empty when requirePositive), box outside its zone, overlap of different widgets, and the left description box contract
// (an all-zero hint box means "absent" and is exempt from the size and zone checks).
static int violations(const Geom& g, const PageLayoutV3& L, bool requirePositive, bool checkZones) {
    Item it[kPageItemsV3];
    int n = collect(L, it);
    Box zone[3];
    zone[0] = g.column;
    zone[1] = Box{g.column.x, g.view.y, g.preview.x - g.column.x, g.view.h};
    zone[2] = g.preview;
    int bad = 0;
    for (int i = 0; i < n; ++i) {
        const bool absent = (it[i].kind >= 3) && isZeroBox(it[i].b);
        if (it[i].b.w < 0 || it[i].b.h < 0) ++bad;
        if (!absent && requirePositive && (it[i].b.w <= 0 || it[i].b.h <= 0)) ++bad;
        if (!absent && checkZones && it[i].kind < 5 && !inside(it[i].b, zone[it[i].zone])) ++bad;
        for (int j = i + 1; j < n; ++j) {
            if ((it[i].kind == 3 && it[j].kind == 4) || (it[i].kind == 4 && it[j].kind == 3)) continue;   // the text area sits inside its box
            if ((it[i].kind == 5 && it[j].kind == 6) || (it[i].kind == 6 && it[j].kind == 5)) continue;
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
    // left description box contract
    const Box d0 = L.hint[0], d1 = L.hint[1];
    if (!isZeroBox(d0)) {
        if (d0.h < kV3DescMinH || d0.h > kV3DescMaxH) ++bad;                       // shown from kV3DescMinH, at most kV3DescMaxH
        if (d0.y < L.status.y + L.status.h + v3::kGap) ++bad;                      // below the status line
        if (d0.w > 0) {
            if (d0.x != L.status.x || d0.w != L.status.w) ++bad;                    // spans the content width
            if (d0.w > 2 * kV3DescPad) {
                if (!same(d1, Box{d0.x + kV3DescPad, d0.y + kV3DescPad, d0.w - 2 * kV3DescPad, d0.h - 2 * kV3DescPad})) ++bad;   // inset by kV3DescPad on every side
            } else if (d1.w > 0 && d1.h > 0) ++bad;
        } else if (!isZeroBox(d1)) ++bad;
    } else if (!isZeroBox(d1)) ++bad;                                               // no box, no text area
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
        KT_CHECK_EQ(violations(g, L, true, true), 0);
        // the hint argument is ignored: the same page without a hint rect is identical box for box
        PageLayoutV3 N = pageLayoutV3(g.column, g.view, g.preview, Box{0, 0, 0, 0}, nullptr);
        KT_CHECK_EQ(violations(g, N, true, true), 0);
        {
            Box a[kPageItemsV3], b[kPageItemsV3];
            pageBoxesV3(L, a); pageBoxesV3(N, b);
            for (int i = 0; i < kPageItemsV3; ++i) KT_CHECK(same(a[i], b[i]));
            KT_CHECK(L.leftRowH == N.leftRowH && L.inspRowH == N.inspRowH && L.barH == N.barH);
        }
        // the view is enlarged: it contains the original rect's width range and keeps a sane height
        KT_CHECK(L.view.x < g.view.x);
        KT_CHECK(L.view.x + L.view.w == g.preview.x - v3::kPad);   // kPad short of the preview (the scaled view rects can end closer than that)
        KT_CHECK(L.view.y > g.view.y && L.view.y + L.view.h < g.view.y + g.view.h);
        KT_CHECK(L.view.w > g.view.w);
        // strips line up with the enlarged view
        KT_CHECK(L.modeBtn[0].x == L.view.x && L.stance[0].x == L.view.x && L.legend.x == L.view.x);
        KT_CHECK(L.viewFlip.x + L.viewFlip.w == L.view.x + L.view.w);
        KT_CHECK(L.modeBtn[0].y < L.view.y && L.stance[0].y > L.view.y + L.view.h && L.legend.y > L.stance[0].y);
        // at these sizes nothing needs the row floors. The rows now also leave kV3DescWantH + kGap (134) under the status line for the left description box:
        // roomy column content at row r = 11 r + 3 (2r/3) + 70 (leftNeed), column bottom bound = pane height - 134:
        //   1280x720  pane 448: bound 314; the smallest roomy row (30) needs 460 > 314, so the compact grid (8 r + 3 (2r/3) + 58): r=26 needs 317 > 314, r=25 needs 306 -> 25
        //   1920x1080 pane 672: bound 538; r=36 needs 396 + 72 + 70 = 538 (exactly), r=37 needs 549                                                           -> 36
        //   3440x1440 pane 896: bound 762; r=52 needs 572 + 102 + 70 = 744                                                                                    -> 52 (max)
        if (sizes[s][0] == 1280) KT_CHECK(L.leftRowH >= kV3RowMin && L.leftRowH < kV3RoomyRowMin);
        else KT_CHECK(L.leftRowH >= kV3RoomyRowMin);
        KT_CHECK_EQ(g.column.h, sizes[s][1] * 896 / 1440);
        KT_CHECK_EQ(L.leftRowH, expectedLeftRowH(g.column.h));
        KT_CHECK_EQ(L.leftRowH, sizes[s][0] == 1280 ? 23 : (sizes[s][0] == 1920 ? 33 : 49));
        KT_CHECK(L.leftRowH <= kV3LeftRowMax && L.inspRowH >= 30 && L.inspRowH <= kV3InspRowMax && L.barH == kV3BarMax);
        if (sizes[s][0] == 3440) KT_CHECK(L.leftRowH == 49 && L.inspRowH == 66);   // the inspector pane (700 px) fits rows up to 66, below kV3InspRowMax
        // the rows are the largest that fit with the description reserve: the content ends kV3DescWantH + kGap above the column bottom (pass 0; each of these
        // columns has a row height that fits it), and one row more (roomy from kV3RoomyRowMin up) would not fit that bound
        {
            const Box colZ{g.column.x, g.column.y, std::min(g.column.w, kV3ColWantW), g.column.h};
            const int bound = colZ.y + colZ.h - (kV3DescWantH + v3::kGap);
            PageLayoutV3 T;
            KT_CHECK(v3::buildLeft(colZ, L.leftRowH, L.leftRowH >= kV3RoomyRowMin, T) <= bound);
            KT_CHECK_EQ(v3::buildLeft(colZ, L.leftRowH, L.leftRowH >= kV3RoomyRowMin, T), colZ.y + leftNeed(L.leftRowH, L.leftRowH >= kV3RoomyRowMin));
            if (L.leftRowH < kV3LeftRowMax)
                KT_CHECK(v3::buildLeft(colZ, L.leftRowH + 1, L.leftRowH + 1 >= kV3RoomyRowMin, T) > bound);
            // the inspector ends after the Remove row (no hint space): the largest row height whose total (inspNeed, from the header constants) fits the pane
            KT_CHECK(inspNeed(L.inspRowH) <= g.preview.h);
            if (L.inspRowH < kV3InspRowMax) KT_CHECK(inspNeed(L.inspRowH + 1) > g.preview.h);
            // and the model agrees with the implementation
            KT_CHECK_EQ(v3::buildInspector(g.preview, g.hint, L.inspRowH, nullptr, T), g.preview.y + inspNeed(L.inspRowH));
            // the left description box exists exactly when kV3DescMinH px are free under the status line (kGap below it, kPad above the column bottom), at most kV3DescMaxH high
            const int free = g.column.y + g.column.h - v3::kPad - (L.status.y + L.status.h + v3::kGap);
            // pass 0 reserved kV3DescWantH: free = pane - leftNeed - kGap (1280: 448 - 306 - 4 = 138, 1920: 672 - 538 - 4 = 130, 3440: 896 - 744 - 4 = 148)
            KT_CHECK_EQ(free, g.column.h - leftNeed(L.leftRowH, L.leftRowH >= kV3RoomyRowMin) - v3::kGap);
            KT_CHECK_EQ(free, sizes[s][0] == 1280 ? 133 : (sizes[s][0] == 1920 ? 136 : 136));
            KT_CHECK(free >= kV3DescWantH);
            if (free >= kV3DescMinH) {
                KT_CHECK(L.hint[0].y == L.status.y + L.status.h + v3::kGap && L.hint[0].h == std::min(free, kV3DescMaxH));
                KT_CHECK(L.hint[0].x == L.status.x && L.hint[0].w == L.status.w);
                KT_CHECK(same(L.hint[1], Box{L.hint[0].x + kV3DescPad, L.hint[0].y + kV3DescPad, L.hint[0].w - 2 * kV3DescPad, L.hint[0].h - 2 * kV3DescPad}));
            } else {
                KT_CHECK(isZeroBox(L.hint[0]) && isZeroBox(L.hint[1]));
            }
        }
        // reset squares are min(kV3ResetMax, row height) squared, the value label sits right of the track, the reset right of the value
        const int sq = L.inspRowH < kV3ResetMax ? L.inspRowH : kV3ResetMax;
        for (int r = 0; r < kV3Rows; ++r) {
            const InspRowV3& R = L.rows[r];
            KT_CHECK(R.reset.w == sq && R.reset.h == sq);
            KT_CHECK(R.value.x >= R.track.x + R.track.w && R.reset.x >= R.value.x + R.value.w);
            KT_CHECK(R.thumb.w == kV3ThumbW && R.thumb.h == R.track.h && R.thumb.y == R.track.y);
        }
        KT_CHECK(L.groupReset[0].w == sq && L.groupReset[0].x == L.rows[0].reset.x);
        // item budget
        Box all[kPageItemsV3 + 8];
        KT_CHECK_EQ(pageBoxesV3(L, all), kPageItemsV3);
        KT_CHECK_EQ(kPageItemsV3, 73);
        KT_CHECK(kPageItemsV3 + kV3Rows <= 80);
        KT_CHECK(kPageItemsV3 == 19 + 13 + 37 + 1 + 2 + 1);
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
        // 1280 is a second exception since kPad 6 / kGap 4: the column content is 136 px, a third is (136 - 2 * kGap) / 3 = 42, Save 38, so its 33 px
        // label misses the 30 px text budget by 3 px but still fits the raw box
        if (sizes[s][0] > 1280) KT_CHECK(px("Save") <= static_cast<float>(L.save.w - 8));
        else KT_CHECK(px("Save") <= static_cast<float>(L.save.w));
        KT_CHECK(px("...") <= static_cast<float>(L.reset.w - 8));   // the "..." toggle (removeToggleLabel)
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
    // left column: the column pane is 400 wide (under kV3ColWantW 600), content W = 400 - 2 * kPad = 388 wide from x0 = 150 + kPad, rows r = 49 high (the largest roomy row whose content, with the captions as full rows, ends kV3DescWantH + kGap above the column bottom: 14 r + 70 <= 896 - 134)
    const int P = v3::kPad, G = v3::kGap, r = 49;
    const int x0 = 150 + P, y0 = 357 + P, W = 400 - 2 * P;
    KT_CHECK_EQ(L.leftRowH, r);
    KT_CHECK(same(L.memberPrev, Box{x0, y0, r, r}));   // side = min(r, (W - 2 * kGap) / 3) = r
    KT_CHECK(same(L.memberNext, Box{x0 + W - r, y0, r, r}));
    const int cap = r;                                              // the captions are full rows (a shorter box drew no text)
    const int yCapW = y0 + r + G;                                   // below the member row
    KT_CHECK(same(L.capWeapon, Box{x0, yCapW, W, cap}));
    const int yW0 = yCapW + cap + G;
    KT_CHECK(same(L.weapon[0], Box{x0, yW0, W, r}));
    KT_CHECK(L.weapon[1].y == yW0 + r + G);
    const int yB0 = yW0 + 2 * (r + G) + cap + G;                    // two weapons, then the bone caption
    const int c2 = (W - G) / 2;                                     // cellW(W, 2, kGap)
    KT_CHECK(same(L.bone[0], Box{x0, yB0, c2, r}));                 // roomy column: 2 bone columns
    KT_CHECK(same(L.bone[1], Box{x0 + c2 + G, yB0, c2, r}));
    KT_CHECK(L.bone[3].y == L.bone[0].y + r + G && L.bone[3].w == c2);
    KT_CHECK(same(L.capBone, Box{x0, yB0 - cap - G, W, cap}));
    KT_CHECK(same(L.capScope, Box{x0, yB0 + 3 * (r + G), W, cap}));
    // under the bones: caption, 3 scope rows, spacer, Save row, status; then the description box fills down to the column bottom pad (357 + 896 - kPad)
    const int yS0 = yB0 + 3 * (r + G) + cap + G + 3 * (r + G) + v3::kSpacer;      // Save row
    KT_CHECK(L.save.y == yS0 && L.status.y == yS0 + r + G);
    const int dTop = yS0 + r + G + r + G, dH = 357 + 896 - P - dTop;               // below the status line
    KT_CHECK_EQ(dH, 136);
    KT_CHECK(same(L.hint[0], Box{x0, dTop, W, dH}));
    KT_CHECK(same(L.hint[1], Box{x0 + kV3DescPad, dTop + kV3DescPad, W - 2 * kV3DescPad, dH - 2 * kV3DescPad}));
    // view area: starts 4 px right of the 400 wide column (the literal gap in pageLayoutV3), ends kPad short of the preview pane
    KT_CHECK(L.view.x == 150 + 400 + 4 && L.view.x + L.view.w == 1925 - P);
    KT_CHECK(L.modeBtn[0].y == 357 && L.modeBtn[0].h == kV3BarMax);
    KT_CHECK(L.view.y == 357 + 2 * (kV3BarMax + 2));   // two mode rows
    // inspector: the 500 wide pane: W = 500 - 2 * kPad, avail = W - sq - 3 * kGap (sq = kV3ResetMax), the track keeps kV3TrackWant (150),
    // the rest lw shared between name and value (170 : 200)
    const int avail = 500 - 2 * P - kV3ResetMax - 3 * G;
    const int lw = avail - kV3TrackWant;
    KT_CHECK(L.rows[0].name.x == 1925 + P && L.rows[0].name.w == lw * kV3NameW / (kV3NameW + kV3ValueW));
    KT_CHECK(L.rows[0].value.w == lw - L.rows[0].name.w);
    KT_CHECK(L.rows[0].track.w == 150);
}

static void testThumbs() {
    Geom g = geomFor(3440, 1440);
    float fr[6] = {0.f, 0.5f, 1.f, 0.25f, -3.f, 7.f};
    PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, fr);
    const InspRowV3* R = L.rows;
    int tw = R[0].track.w;
    KT_CHECK(tw > 40);
    KT_CHECK(R[0].thumb.x == R[0].track.x);                                   // 0 -> flush left
    KT_CHECK(R[1].thumb.x == R[1].track.x + (int)(0.5f * (float)tw + 0.5f) - 5);   // centred, the thumb centre is the rounded fraction of the track width
    KT_CHECK(R[2].thumb.x == R[2].track.x + tw - 10);                          // 1 -> flush right
    KT_CHECK(R[3].thumb.x == R[3].track.x + (int)(0.25f * (float)tw + 0.5f) - 5);
    KT_CHECK(R[4].thumb.x == R[4].track.x);                                    // -3 clamps to 0
    KT_CHECK(R[5].thumb.x == R[5].track.x + tw - 10);                          // 7 clamps to 1
    KT_CHECK_EQ(violations(g, L, true, true), 0);
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
            KT_CHECK_EQ(violations(q, P, true, true), 0);
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
    KT_CHECK_EQ(violations(m, L, true, true), 0);
    KT_CHECK(L.leftRowH < kV3LeftRowMax && L.leftRowH >= kV3RowMin);   // rows shrank to fit
    KT_CHECK(L.inspRowH < kV3InspRowMax && L.inspRowH >= kV3RowMin);
    PageLayoutV3 L2 = pageLayoutV3(m.column, m.view, m.preview, Box{0, 0, 0, 0}, nullptr);
    KT_CHECK_EQ(violations(m, L2, true, true), 0);

    // below the minimum: sizes stay non-negative, boxes stay in their pane, nothing overlaps
    Geom t;
    t.column = Box{0, 0, 50, 40};
    t.view = Box{60, 0, 50, 30};
    t.preview = Box{120, 0, 50, 30};
    t.hint = Box{120, 34, 50, 10};
    float fr[6] = {0.f, 1.f, 0.5f, std::nanf(""), -1.f, 2.f};
    PageLayoutV3 T = pageLayoutV3(t.column, t.view, t.preview, t.hint, fr);
    KT_CHECK_EQ(violations(t, T, false, true), 0);
    PageLayoutV3 T2 = pageLayoutV3(t.column, t.view, t.preview, Box{0, 0, 0, 0}, fr);
    KT_CHECK_EQ(violations(t, T2, false, true), 0);
    KT_CHECK(T.leftRowH == kV3RowMin && T.inspRowH == kV3RowMin && T.barH == kV3BarMin);

    // degenerate zero and one pixel panes: no negative size, no overlap
    Geom z;
    z.column = Box{0, 0, 0, 0}; z.view = Box{0, 0, 0, 0}; z.preview = Box{0, 0, 0, 0}; z.hint = Box{0, 0, 0, 0};
    PageLayoutV3 Z = pageLayoutV3(z.column, z.view, z.preview, z.hint, fr);
    KT_CHECK_EQ(violations(z, Z, false, false), 0);
    Geom one;
    one.column = Box{5, 5, 1, 1}; one.view = Box{10, 5, 1, 1}; one.preview = Box{20, 5, 1, 1}; one.hint = Box{20, 8, 1, 1};
    PageLayoutV3 O = pageLayoutV3(one.column, one.view, one.preview, one.hint, nullptr);
    KT_CHECK_EQ(violations(one, O, false, true), 0);

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
                    bad += violations(s, S, false, true);   // a 0 wide hint rect counts as none
                    PageLayoutV3 S2 = pageLayoutV3(s.column, s.view, s.preview, Box{0, 0, 0, 0}, fr);
                    bad += violations(s, S2, false, true);
                }
    KT_CHECK_EQ(bad, 0);
}

static void testViewVsPreview() {
    // gap: view rect ends 1921, preview starts 1925 -> the enlarged view ends kPad short of the preview
    Geom g = geomFor(3440, 1440);
    PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
    KT_CHECK_EQ(L.view.x + L.view.w, g.preview.x - v3::kPad);
    KT_CHECK_EQ(L.viewFlip.x + L.viewFlip.w, g.preview.x - v3::kPad);
    // a wide gap still ends kPad short (the view grows into it)
    Box pw = g.preview; pw.x = 2000;
    PageLayoutV3 W = pageLayoutV3(g.column, g.view, pw, g.hint, nullptr);
    KT_CHECK_EQ(W.view.x + W.view.w, pw.x - v3::kPad);   // 1994
    // overlap: the given view rect reaches 40 px into the preview -> the view strip, mode strip and stances stop short of it
    Box po = g.preview; po.x = 1881;
    PageLayoutV3 O = pageLayoutV3(g.column, g.view, po, g.hint, nullptr);
    KT_CHECK_EQ(O.view.x + O.view.w, po.x - v3::kPad);   // 1875
    KT_CHECK(O.view.w > 0);
    Box all[kPageItemsV3 + 8]; pageBoxesV3(O, all);
    for (int i = 19; i < 32; ++i) KT_CHECK(all[i].x + all[i].w <= po.x - v3::kPad);   // view area items (see collect())
    for (int i = 32; i < PI_RESET_SCOPE; ++i) KT_CHECK(!overlaps(all[i], O.view));   // the "..." toggle sits in the left column
    KT_CHECK(!overlaps(all[PI_RESET_SCOPE], O.view));
    KT_CHECK(!overlaps(all[PI_REMOVE], O.view));
    KT_CHECK(!overlaps(O.viewFlip, O.rows[0].name) && !overlaps(O.legend, O.groupLabel[0]));
    // a preview left of the column content leaves no room: the width stays non-negative and inside [vl, preview.x]
    Box pn = g.preview; pn.x = g.column.x + 100;
    PageLayoutV3 N = pageLayoutV3(g.column, g.view, pn, g.hint, nullptr);
    KT_CHECK(N.view.w >= 0 && N.view.x + N.view.w <= std::max(pn.x, N.view.x));
    // no preview rect (w == 0): the old right edge is used
    PageLayoutV3 Z = pageLayoutV3(g.column, g.view, Box{g.preview.x, g.preview.y, 0, 0}, Box{0, 0, 0, 0}, nullptr);
    KT_CHECK_EQ(Z.view.x + Z.view.w, g.view.x + g.view.w);
}

// The left description box (hint[0]) and its text area (hint[1]): the free part of the left column below the status line.
static void testDescBox() {
    const int P = v3::kPad, G = v3::kGap;
    Geom g = geomFor(3440, 1440);   // column {150,357,400,896}: rows 49 high, 136 px free under the status line
    PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
    const int r = 49, cap = r;   // the 896 px pane fits 49 rows (14 r + 70 <= 896 - 134); the captions are full rows
    // roomy column content height above the status bottom, from the header constants: member row, 3 captions (full rows), 2 weapons, 3 bone rows, 3 scope rows, spacer, save row, status
    const int statusBottom = g.column.y + P + (r + G) + 3 * (cap + G) + 2 * (r + G) + 3 * (r + G) + 3 * (r + G) + v3::kSpacer + (r + G) + r;
    KT_CHECK_EQ(L.status.y + L.status.h, statusBottom);
    const int x0 = g.column.x + P, W = g.column.w - 2 * P;
    const int free = g.column.y + g.column.h - P - (statusBottom + G);
    KT_CHECK_EQ(L.leftRowH, r);
    KT_CHECK_EQ(free, 136);
    KT_CHECK(free >= kV3DescMinH && free <= kV3DescMaxH);
    KT_CHECK(same(L.hint[0], Box{x0, statusBottom + G, W, free}));
    KT_CHECK(same(L.hint[1], Box{x0 + kV3DescPad, statusBottom + G + kV3DescPad, W - 2 * kV3DescPad, free - 2 * kV3DescPad}));
    KT_CHECK(L.hint[0].y + L.hint[0].h == g.column.y + g.column.h - P);   // reaches the column bottom pad
    KT_CHECK_EQ(violations(g, L, true, true), 0);
    // zone: the left column, nothing of it in the view area or the inspector
    KT_CHECK(inside(L.hint[0], g.column) && !overlaps(L.hint[0], L.view) && !overlaps(L.hint[0], g.preview));
    // disjoint from every other left item (also the Reset-scope row and the status line)
    Box all[kPageItemsV3]; pageBoxesV3(L, all);
    for (int i = 0; i < PI_STATUS + 1; ++i) KT_CHECK(!overlaps(all[i], L.hint[0]) && !overlaps(all[i], L.hint[1]));
    KT_CHECK(!overlaps(L.reset, L.hint[0]));
    // the inspector no longer holds hints: the note is its last item
    KT_CHECK(L.note.y + L.note.h <= g.preview.y + g.preview.h);

    // boundary of the fit: the largest r whose content ends kV3DescWantH + kGap above the column bottom. leftNeed(49) = 756, so the column is exactly
    // 756 + 134 = 890 high -> still 49 rows and the box is exactly kV3DescWantH (130) high; one px less -> 48 rows (leftNeed(48) = 14 * 48 + 70 = 742 <= 755),
    // whose status line is 14 px higher, so the box is 889 - 742 - 4 = 143 high
    KT_CHECK_EQ(leftNeed(r, true), 756);
    KT_CHECK_EQ(leftNeed(r - 1, true), 742);
    Geom e = g; e.column.h = leftNeed(r, true) + kV3DescWantH + G;
    KT_CHECK_EQ(e.column.h, 890);
    PageLayoutV3 E = pageLayoutV3(e.column, e.view, e.preview, e.hint, nullptr);
    KT_CHECK_EQ(E.leftRowH, r);
    KT_CHECK_EQ(E.status.y + E.status.h, statusBottom);
    KT_CHECK_EQ(E.hint[0].h, kV3DescWantH);
    KT_CHECK_EQ(E.hint[1].h, kV3DescWantH - 2 * kV3DescPad);
    KT_CHECK_EQ(violations(e, E, true, true), 0);
    Geom u = e; u.column.h -= 1;
    PageLayoutV3 U = pageLayoutV3(u.column, u.view, u.preview, u.hint, nullptr);
    KT_CHECK_EQ(U.leftRowH, r - 1);
    KT_CHECK_EQ(U.status.y + U.status.h, g.column.y + leftNeed(r - 1, true) - P);
    KT_CHECK_EQ(U.hint[0].h, u.column.h - leftNeed(r - 1, true) - G);
    KT_CHECK_EQ(U.hint[0].h, 143);
    KT_CHECK_EQ(violations(u, U, true, true), 0);
    // pass 0 never leaves less than kV3DescWantH, so the kV3DescMinH threshold shows only in buildLeft itself (pass 1 sizes the rows to the whole column, which
    // leaves a few px at most): exactly kV3DescMinH free under the status line -> box; one px less -> both boxes all-zero
    {
        PageLayoutV3 B;
        const Box zb{g.column.x, g.column.y, g.column.w, leftNeed(r, true) + G + kV3DescMinH};   // content + bottom pad, kGap, kV3DescMinH
        KT_CHECK_EQ(zb.h, 756 + 4 + 90);
        v3::buildLeft(zb, r, true, B);
        KT_CHECK_EQ(B.hint[0].h, kV3DescMinH);
        KT_CHECK_EQ(B.hint[1].h, kV3DescMinH - 2 * kV3DescPad);
        PageLayoutV3 B2;
        v3::buildLeft(Box{zb.x, zb.y, zb.w, zb.h - 1}, r, true, B2);
        KT_CHECK(isZeroBox(B2.hint[0]) && isZeroBox(B2.hint[1]));
    }

    // cap: a very tall column leaves hundreds of px, the box stays kV3DescMaxH high and starts under the status line
    Geom t = g; t.column.h = 1400; t.view.h = 1400;
    PageLayoutV3 T = pageLayoutV3(t.column, t.view, t.preview, t.hint, nullptr);
    KT_CHECK(t.column.y + t.column.h - P - (T.status.y + T.status.h + G) > kV3DescMaxH);
    KT_CHECK_EQ(T.hint[0].h, kV3DescMaxH);
    KT_CHECK_EQ(T.hint[0].y, T.status.y + T.status.h + G);
    KT_CHECK_EQ(T.hint[1].h, kV3DescMaxH - 2 * kV3DescPad);
    KT_CHECK_EQ(violations(t, T, true, true), 0);

    // (b) a tiny column (height 180): no room, both hint boxes are all-zero, rest of the page stays valid
    // The smallest compact row (14) needs leftNeed = 11 * 14 + 58 = 212 > 180 - 134 and > 180 itself, so no row fits either pass: kV3RowMin (with the old r * 2 / 3 captions, 197 high), box absent.
    Geom s = g; s.column.h = 180;
    PageLayoutV3 S = pageLayoutV3(s.column, s.view, s.preview, s.hint, nullptr);
    KT_CHECK_EQ(leftNeed(kV3RowMin, false), 212);
    KT_CHECK_EQ(S.capWeapon.h, kV3RowMin * 2 / 3);   // the fallback keeps the short captions (the full ones would push the status line out of a 180 px column)
    KT_CHECK_EQ(S.leftRowH, kV3RowMin);
    KT_CHECK(isZeroBox(S.hint[0]) && isZeroBox(S.hint[1]));
    KT_CHECK_EQ(violations(s, S, false, true), 0);
    // pass 1 fallback (no row fits the bound minus 134 but some fit the whole column): 300 high column, pass 0 bound 166 < 212. Compact leftNeed(22) = 11 * 22 + 58 = 300
    // <= 300 < leftNeed(23) = 311, so the rows are 22 (not 14); 300 - 300 - 4 < kV3DescMinH leaves no box
    {
        Geom f = g; f.column.h = 300;
        bool p1 = false;
        KT_CHECK_EQ(expectedLeftRowH(300, &p1), 22);
        KT_CHECK(p1);
        PageLayoutV3 F = pageLayoutV3(f.column, f.view, f.preview, f.hint, nullptr);
        KT_CHECK_EQ(leftNeed(22, false), 300);
        KT_CHECK_EQ(leftNeed(23, false), 311);
        KT_CHECK_EQ(F.leftRowH, 22);
        KT_CHECK(isZeroBox(F.hint[0]) && isZeroBox(F.hint[1]));
        KT_CHECK_EQ(violations(f, F, false, true), 0);
    }
    Geom mn; mn.column = Box{0, 0, 140, 180}; mn.view = Box{150, 0, 160, 150}; mn.preview = Box{320, 0, 170, 200}; mn.hint = Box{320, 204, 170, 60};
    PageLayoutV3 M = pageLayoutV3(mn.column, mn.view, mn.preview, mn.hint, nullptr);
    KT_CHECK(isZeroBox(M.hint[0]) && isZeroBox(M.hint[1]));
    // the zero boxes still travel through pageBoxesV3 at the PI_HINT slots
    Box mb[kPageItemsV3]; pageBoxesV3(M, mb);
    KT_CHECK(isZeroBox(mb[PI_HINT]) && isZeroBox(mb[PI_HINT + 1]));
}

// (a) A 3840x1600-style pane taller than the real one (940 vs 757): the box exists, kV3DescMinH..kV3DescMaxH high (the real pane is checked in testFill3840x1600).
static void testDescBox3840() {
    const int P = v3::kPad, G = v3::kGap;
    Geom g = realGeom();
    g.column.h = 940;   // same x and width as the real pane, taller: rows stay at kV3LeftRowMax and what is left under the status line is well above kV3DescMinH
    PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
    KT_CHECK_EQ(L.leftRowH, kV3LeftRowMax);
    KT_CHECK(!isZeroBox(L.hint[0]) && !isZeroBox(L.hint[1]));
    KT_CHECK(L.hint[0].h >= kV3DescMinH && L.hint[0].h <= kV3DescMaxH);
    KT_CHECK(L.hint[1].h > 0 && L.hint[1].w > 0);
    // wide content: the wide cap (kV3ColWantW 600 - 2 * kPad) wide, same x / width as the status line
    KT_CHECK(L.hint[0].x == L.status.x && L.hint[0].w == L.status.w && L.hint[0].w >= 400);
    KT_CHECK(L.hint[0].y >= L.status.y + L.status.h + G);
    KT_CHECK(L.hint[0].y + L.hint[0].h <= g.column.y + g.column.h - P);
    KT_CHECK(same(L.hint[1], Box{L.hint[0].x + kV3DescPad, L.hint[0].y + kV3DescPad, L.hint[0].w - 2 * kV3DescPad, L.hint[0].h - 2 * kV3DescPad}));
    KT_CHECK_EQ(violations(g, L, true, true), 0);
    // the view area starts right of the box (the box is a left column item)
    KT_CHECK(L.hint[0].x + L.hint[0].w <= L.view.x);
}

// The shrunk description pane as mod-config makes it (mui::splitDesc at 850 permille): the bottom 15 % of the original rect (same x / width as the preview
// pane, the original rect has the view's y and height), at least ~a gap under the preview.
static Box descFor(const Geom& g) {
    const int bottom = g.view.y + g.view.h, h = (150 * g.view.h + 500) / 1000;
    return Box{g.preview.x, bottom - h, g.preview.w, h};
}

// Description boxes with a valid desc pane: both boxes end on the pane bottom, the right box is the pane, the left box is tall enough for text and clear of the rows.
static void checkDescPane(const Geom& g, const char* what) {
    (void)what;
    const Box d = descFor(g);
    PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr, d);
    PageLayoutV3 N = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
    KT_CHECK(!isZeroBox(L.hint[0]));
    KT_CHECK_EQ(L.hint[0].y + L.hint[0].h, d.y + d.h);                       // bottoms equal
    KT_CHECK_EQ(L.hintR[0].y + L.hintR[0].h, d.y + d.h);
    KT_CHECK(L.hint[0].y + L.hint[0].h == L.hintR[0].y + L.hintR[0].h);
    KT_CHECK(same(L.hintR[0], d));                                           // right box == pane
    KT_CHECK(same(L.hintR[1], Box{d.x + kV3DescPad, d.y + kV3DescPad, d.w - 2 * kV3DescPad, d.h - 2 * kV3DescPad}));
    KT_CHECK(L.hint[0].h >= kV3DescMinH && L.hint[0].h <= kV3DescMaxH);
    KT_CHECK(L.hint[0].h >= 3 * 36 + 2 * kV3DescPad);                        // ~3 lines of description text
    KT_CHECK(L.hint[0].y >= L.status.y + L.status.h + v3::kGap);             // below the status line
    KT_CHECK(L.hint[0].x == L.status.x && L.hint[0].w == L.status.w);
    KT_CHECK(same(L.hint[1], Box{L.hint[0].x + kV3DescPad, L.hint[0].y + kV3DescPad, L.hint[0].w - 2 * kV3DescPad, L.hint[0].h - 2 * kV3DescPad}));
    KT_CHECK(L.hint[0].x + L.hint[0].w <= L.view.x);                         // left of the view area
    KT_CHECK(!overlaps(L.hint[0], L.hintR[0]));
    Box all[kPageItemsV3]; pageBoxesV3(L, all);
    for (int i = 0; i < PI_HINT; ++i) KT_CHECK(!overlaps(all[i], L.hint[0]) && !overlaps(all[i], L.hint[1]));   // no row, status or view item under the left box
    KT_CHECK(!overlaps(L.reset, L.hint[0]));
    for (int i = 0; i < kPageItemsV3; ++i) {
        if (i == PI_HINTR || i == PI_HINTR + 1) continue;
        KT_CHECK(!overlaps(all[i], L.hintR[0]));                             // the right box covers nothing of ours
    }
    // the rows are the same as without a desc pane (only the description boxes change)
    KT_CHECK_EQ(L.leftRowH, N.leftRowH);
    KT_CHECK(same(L.status, N.status) && same(L.view, N.view) && same(L.rows[0].track, N.rows[0].track));
    KT_CHECK(isZeroBox(N.hintR[0]) && isZeroBox(N.hintR[1]));
}

static void testDescPane() {
    checkDescPane(realGeom(), "3840x1600 real");
    checkDescPane(geomFor(1920, 1080), "1920x1080");
    checkDescPane(geomFor(1440, 1080), "4:3");
    // 3840x1600 real: the left box is much taller than the old rule's (which ended at the column bottom, 139 px above the pane bottom)
    {
        const Geom g = realGeom();
        const Box d = descFor(g);
        PageLayoutV3 O = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
        PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr, d);
        KT_CHECK(O.hint[0].y + O.hint[0].h < d.y + d.h);
        KT_CHECK(L.hint[0].h > O.hint[0].h);
    }
    // a very tall pane: the left box is kV3DescMaxH high, starting at bottom - kV3DescMaxH
    {
        Geom g = realGeom();
        g.column.h = 1400; g.view.h = 1400;
        const Box d = Box{g.preview.x, 357 + 1400 - 210, g.preview.w, 210};
        PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr, d);
        KT_CHECK_EQ(L.hint[0].h, kV3DescMaxH);
        KT_CHECK_EQ(L.hint[0].y, d.y + d.h - kV3DescMaxH);
    }
    // an invalid pane (zero Box, no width, no height) falls back to the old rule and parks the right box
    {
        const Geom g = realGeom();
        PageLayoutV3 O = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
        const Box bad[3] = {Box{0, 0, 0, 0}, Box{1953, 1119, 0, 134}, Box{1953, 1119, 1113, 0}};
        for (int i = 0; i < 3; ++i) {
            PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr, bad[i]);
            KT_CHECK(same(L.hint[0], O.hint[0]) && same(L.hint[1], O.hint[1]));
            KT_CHECK(isZeroBox(L.hintR[0]) && isZeroBox(L.hintR[1]));
        }
        KT_CHECK_EQ(violations(g, O, true, true), 0);
    }
    // a pane too small for the padded text area: the right box is parked (zero), the left box still follows the pane bottom
    {
        const Geom g = realGeom();
        const Box tiny{1953, 1119, 1113, 2 * kV3DescPad};
        PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr, tiny);
        KT_CHECK(isZeroBox(L.hintR[0]) && isZeroBox(L.hintR[1]));
        KT_CHECK_EQ(L.hint[0].y + L.hint[0].h, tiny.y + tiny.h);
    }
    // a pane so high up that less than kV3DescMinH is left under the status line: no left box (never overlaps the rows), the right box stays
    {
        const Geom g = realGeom();
        PageLayoutV3 N = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
        const Box high{1953, N.status.y, 1113, 120};
        PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr, high);
        KT_CHECK(isZeroBox(L.hint[0]) && isZeroBox(L.hint[1]));
        KT_CHECK(same(L.hintR[0], high));
    }
    // the whole-screen sweep: with the pane the layout never violates (the left/right boxes are exempt from the zone test)
    for (int W = 800; W <= 3840; W += 320)
        for (int H = 600; H <= 1600; H += 200) {
            Geom g = geomFor(W, H);
            const Box d = descFor(g);
            PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr, d);
            KT_CHECK_EQ(violations(g, L, false, true), 0);
            if (!isZeroBox(L.hint[0])) {
                KT_CHECK_EQ(L.hint[0].y + L.hint[0].h, d.y + d.h);
                KT_CHECK(L.hint[0].y >= L.status.y + L.status.h + v3::kGap);
                KT_CHECK(L.hint[0].h >= kV3DescMinH && L.hint[0].h <= kV3DescMaxH);
            }
        }
}

// coverDescPane (the right description box covers LB_DESC's own frame): grows on all four sides, clamped, 0 = unchanged, both boxes keep equal bottoms.
static void testCoverDesc() {
    const Geom g = realGeom();
    const Box d = descFor(g);
    const Box none{0, 0, 0, 0};
    const Box screen{0, 0, 3840, 1600};
    // cover 0 = the pane unchanged; a negative cover too; an absent pane stays absent
    KT_CHECK(same(coverDescPane(d, g.preview, screen, 0), d));
    KT_CHECK(same(coverDescPane(d, g.preview, screen, -5), d));
    KT_CHECK(same(coverDescPane(none, g.preview, screen, 6), none));
    KT_CHECK(same(coverDescPane(Box{10, 10, 0, 50}, g.preview, screen, 6), (Box{10, 10, 0, 50})));
    // the preview pane the editor passes is widened into the right margin (and the screen leaves room below): all four sides grow by the cover
    Box wide = g.preview;
    wide.w += 120;
    wide.h = d.y - wide.y;   // the preview ends where the pane starts: the top clamp = the preview bottom
    {
        const Box C = coverDescPane(d, wide, screen, 6);
        KT_CHECK(C.x == d.x - 6 || C.x == wide.x - v3::kPad);                       // left: cover, or the view edge when that is tighter
        KT_CHECK_EQ(C.x + C.w, d.x + d.w + 6);                                  // right
        KT_CHECK_EQ(C.y, d.y);                                                  // top: clamped at the preview bottom (the pane already starts there)
        KT_CHECK_EQ(C.y + C.h, d.y + d.h + 6);                                  // bottom
        KT_CHECK(C.x <= d.x && C.y <= d.y && C.x + C.w >= d.x + d.w && C.y + C.h >= d.y + d.h);   // always contains the pane
    }
    // a pane starting below the preview bottom grows upward by the cover; one at the preview bottom does not go higher
    {
        const Box low{d.x, d.y + 40, d.w, d.h - 40};
        const Box C = coverDescPane(low, wide, screen, 6);
        KT_CHECK_EQ(C.y, low.y - 6);
        KT_CHECK(C.y >= wide.y + wide.h);
        const Box C2 = coverDescPane(Box{d.x, wide.y + wide.h + 2, d.w, 100}, wide, screen, 24);
        KT_CHECK_EQ(C2.y, wide.y + wide.h);   // clamped at the preview bottom, never above
    }
    // clamps: the cover is limited to kV3CoverMax; never left of the view edge (preview.x - kPad); never right of the preview right; never off the screen
    {
        const Box C = coverDescPane(d, wide, screen, 1000);
        KT_CHECK(C.x >= wide.x - v3::kPad && C.x >= 0);
        KT_CHECK(C.x + C.w <= wide.x + wide.w && C.x + C.w <= screen.w);
        KT_CHECK(C.y >= wide.y + wide.h);
        KT_CHECK(C.y + C.h <= d.y + d.h + kV3CoverMax);
        KT_CHECK(same(coverDescPane(d, wide, screen, 1000), coverDescPane(d, wide, screen, kV3CoverMax)));
    }
    {   // right edge at the preview right: no growth to the right (the margin ini margin_px 0 case), the other sides still grow
        const Box C = coverDescPane(d, g.preview, screen, 6);
        KT_CHECK_EQ(C.x + C.w, d.x + d.w);
        KT_CHECK(C.y + C.h == d.y + d.h + 6);
    }
    {   // a pane at the screen edges never leaves the screen
        const Box edge{3700, 1500, 140, 100};
        const Box C = coverDescPane(edge, none, screen, 12);
        KT_CHECK(C.x + C.w <= 3840 && C.y + C.h <= 1600 && C.x >= 0 && C.y >= 0);
        KT_CHECK(C.x <= edge.x && C.y <= edge.y && C.x + C.w >= edge.x + edge.w && C.y + C.h >= edge.y + edge.h);
        const Box E{0, 0, 100, 100};
        const Box C0 = coverDescPane(E, none, screen, 12);
        KT_CHECK(C0.x == 0 && C0.y == 0 && C0.w == 112 && C0.h == 112);
    }
    // no preview pane and no screen: the plain cover on all sides
    {
        const Box C = coverDescPane(d, none, none, 6);
        KT_CHECK(same(C, (Box{d.x - 6, d.y - 6, d.w + 12, d.h + 12})));
    }
    // the cover never reaches into the enlarged view: with the real layout the view ends kPad short of the preview
    {
        const Box C = coverDescPane(d, wide, screen, kV3CoverMax);
        const PageLayoutV3 L = pageLayoutV3(g.column, g.view, wide, g.hint, nullptr, C);
        KT_CHECK(L.view.x + L.view.w <= C.x);
        for (int i = 0; i < kV3Modes; ++i) KT_CHECK(L.modeBtn[i].x + L.modeBtn[i].w <= C.x || L.modeBtn[i].y + L.modeBtn[i].h <= C.y);
    }
    // layout with the inflated pane: both boxes keep equal bottoms (= the inflated pane bottom), the right box is the inflated pane, the rows do not change
    for (int W = 800; W <= 3840; W += 320)
        for (int H = 600; H <= 1600; H += 200) {
            Geom gg = geomFor(W, H);
            const Box dd = descFor(gg);
            Box pv = gg.preview;
            pv.w += 120;
            const Box sc{0, 0, W, H};
            const Box C = coverDescPane(dd, pv, sc, 6);
            KT_CHECK(C.y >= std::min(dd.y, pv.y + pv.h));   // never above the preview bottom unless the pane itself started higher
            const PageLayoutV3 L = pageLayoutV3(gg.column, gg.view, pv, gg.hint, nullptr, C);
            const PageLayoutV3 L0 = pageLayoutV3(gg.column, gg.view, pv, gg.hint, nullptr, dd);
            KT_CHECK_EQ(L.leftRowH, L0.leftRowH);
            KT_CHECK_EQ(L.inspRowH, L0.inspRowH);
            if (!isZeroBox(L.hintR[0])) KT_CHECK(same(L.hintR[0], C));
            if (!isZeroBox(L.hint[0]) && !isZeroBox(L.hintR[0])) KT_CHECK_EQ(L.hint[0].y + L.hint[0].h, L.hintR[0].y + L.hintR[0].h);
            if (!isZeroBox(L.hint[0])) KT_CHECK_EQ(L.hint[0].y + L.hint[0].h, C.y + C.h);
            // cover 0 gives the same layout as the raw pane
            const PageLayoutV3 L00 = pageLayoutV3(gg.column, gg.view, pv, gg.hint, nullptr, coverDescPane(dd, pv, sc, 0));
            KT_CHECK(same(L00.hintR[0], L0.hintR[0]) && same(L00.hint[0], L0.hint[0]));
        }
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
    KT_CHECK_EQ(violations(g, L, true, true), 0);
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
    // item budget: 73 boxes + 6 wedges
    KT_CHECK_EQ(kPageItemsV3Total, 79);
    KT_CHECK(kPageItemsV3Total <= 80);
    Box all[kPageItemsV3];
    KT_CHECK_EQ(pageBoxesV3(L, all), kPageItemsV3);

    // text fit: measured with the engine-calibrated text_k 2.65; button captions get the engine inset (7 px per side) plus 3 px slack per side,
    // labels the 4 px per side (HseView.h buttonTextPx / labelTextPx)
    const textfit::TextCtx tc = {2.65f, 1.0f, 1.0f};
    auto px = [&](const char* t) { return textfit::textPx(tc, t, static_cast<int>(strlen(t))); };
    static const char* const kStances[kV3Stances] = {"Idle", "Walk", "Run", "Alert", "Combat"};   // HolsterEditor.cpp kStanceList
    static const char* const kSliders[kV3Rows] = {"Slide", "Depth", "Height", "Tilt", "Roll", "Turn"};   // HseView.h inspector names
    const int sizes[2][2] = {{3840, 1600}, {3440, 1440}};
    for (int s = 0; s < 2; ++s) {
        Geom q = s == 0 ? g : geomFor(sizes[s][0], sizes[s][1]);
        PageLayoutV3 P = pageLayoutV3(q.column, q.view, q.preview, q.hint, nullptr);
        KT_CHECK(px("Save") <= static_cast<float>(buttonTextPx(P.save.w)));
        if (s == 0) KT_CHECK(px("Revert") <= static_cast<float>(buttonTextPx(P.revert.w)));   // the synthetic 3440 column is 400 wide: its 129 px Revert gets cut
        KT_CHECK(px(removeToggleLabel()) <= static_cast<float>(buttonTextPx(P.reset.w)));
        KT_CHECK(px(removeLabel()) <= static_cast<float>(buttonTextPx(P.remove.w)));   // "Remove saved placement" fits its button
        KT_CHECK(px("Move") <= static_cast<float>(buttonTextPx(P.modeBtn[0].w)));
        for (int i = 0; i < kV3Stances; ++i)
            KT_CHECK(px(kStances[i]) <= static_cast<float>(buttonTextPx(P.stance[i].w)));
        for (int i = 0; i < kV3Rows; ++i) KT_CHECK(px(kSliders[i]) <= static_cast<float>(labelTextPx(P.rows[i].name.w)));
        // the widest inspector value ("-180.0 deg", 189 px) fits its label box where the pane is wide (3840x1600), so the number never word-wraps away
        if (s == 0) for (int i = 0; i < kV3Rows; ++i) KT_CHECK(px("-180.0 deg") <= static_cast<float>(labelTextPx(P.rows[i].value.w)));
        if (s == 1) KT_CHECK(P.rows[0].track.w >= 150);
    }
}

// At the real 3840x1600 panes the left rows are taller than the old 34 px cap and the column ends inside its pane (HW-UX3 fill fix).
static void testFill3840x1600() {
    const Geom g = realGeom();
    PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
    KT_CHECK(L.leftRowH > 34 && L.inspRowH > 34 && L.barH > 34);
    // the rows leave kV3DescWantH + kGap under the status line: leftNeed(r) <= 757 - 134 = 623 -> roomy r=39 (14 * 39 + 70 = 616; the captions are full rows), r=40 needs 14 * 40 + 70 = 630
    KT_CHECK_EQ(leftNeed(39, true), 616);
    KT_CHECK_EQ(leftNeed(40, true), 630);
    KT_CHECK_EQ(L.leftRowH, 39);
    KT_CHECK_EQ(L.leftRowH, expectedLeftRowH(g.column.h));
    KT_CHECK(L.reset.y + L.reset.h <= g.column.y + g.column.h && L.status.y + L.status.h <= g.column.y + g.column.h);
    KT_CHECK(L.status.y + L.status.h > g.column.y + g.column.h / 2);   // and it uses more than half of the pane
    KT_CHECK(L.rows[0].value.w >= 190 && L.rows[0].name.w >= 160);
    // the inspector has no hint space any more: its rows are at the max where inspNeed(kV3InspRowMax) fits the pane
    KT_CHECK(inspNeed(kV3InspRowMax) <= g.preview.h);
    KT_CHECK_EQ(L.inspRowH, kV3InspRowMax);
    KT_CHECK_EQ(kV3InspRowMax, 70);   // the cap grew from 60 so the rows fill the pane below the Turn slider: 3840 rows 70, 3440 rows 66
    KT_CHECK_EQ(g.preview.h - inspNeed(70), 16);   // slack left under the Remove button (inspNeed counts the last gap; 71 would not fit with the cap lifted: the pane takes rows up to 70)
    // the three section captions (WEAPON, BONE, APPLIES TO) are full row boxes: a shorter box drew no text in game
    KT_CHECK(L.capWeapon.h == L.leftRowH && L.capBone.h == L.leftRowH && L.capScope.h == L.leftRowH);
    // the left description box: shown only when kV3DescMinH px are free under the status line in the column pane (decided from the pane and the status line)
    const int free = g.column.y + g.column.h - v3::kPad - (L.status.y + L.status.h + v3::kGap);
    // (a) the real pane HAS the box: free = 757 - 616 - 4 = 137, within kV3DescMinH..kV3DescMaxH
    KT_CHECK_EQ(free, g.column.h - leftNeed(39, true) - v3::kGap);
    KT_CHECK_EQ(free, 137);
    KT_CHECK(free >= kV3DescMinH && free <= kV3DescMaxH);
    KT_CHECK_EQ(L.hint[0].h, free);
    KT_CHECK(L.hint[0].h >= kV3DescMinH && L.hint[0].h <= kV3DescMaxH);
    KT_CHECK_EQ(L.hint[1].h, free - 2 * kV3DescPad);
    KT_CHECK(inside(L.hint[0], g.column) || isZeroBox(L.hint[0]));
}

// Zero-cut captions at the real 3840x1600 panes: the bones sit in 2 columns, the scopes one per row, the modes 2 x 2 (icons at the right end of the first
// row) and the stances 3 + 2, so every selector button is wide (bones, scopes, modes >= 220; the narrowest, a stance of the first row, is 199 px:
// the wider column (kV3ColWantW 600, capped at the view pane's x) leaves the view vw = preview.x - kPad - view.x = 602, so (vw - 2 * 2) / 3; the text need is 148 px).
static void testRoomyButtons3840() {
    const Geom g = realGeom();
    PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
    KT_CHECK(L.leftRowH >= 39 && L.inspRowH >= 40);
    KT_CHECK_EQ(violations(g, L, true, true), 0);
    for (int i = 0; i < kV3Bones; ++i) KT_CHECK(L.bone[i].w >= 220);
    for (int i = 0; i < kV3Scopes; ++i) KT_CHECK(L.scope[i].w >= 220);
    KT_CHECK_EQ(L.view.x, g.view.x);   // the column (pane 1132 wide) is capped at view.x - 4 - column.x, so the view starts where the view pane does
    const int s3 = (g.preview.x - v3::kPad - g.view.x - 2 * 2) / 3;
    KT_CHECK_EQ(s3, 199);
    for (int i = 0; i < kV3Stances; ++i) KT_CHECK(L.stance[i].w >= s3);
    for (int i = 0; i < kV3Modes; ++i) KT_CHECK(L.modeBtn[i].w >= 220);
    // grid shape: Upper back | Back / Pelvis | R hip / L hip; one scope per row; modes Move Depth / Tilt Roll/Turn; stances Idle Walk Run / Alert Combat
    KT_CHECK(L.bone[0].y == L.bone[1].y && L.bone[1].x > L.bone[0].x && L.bone[2].y > L.bone[0].y && L.bone[2].y == L.bone[3].y && L.bone[4].y > L.bone[2].y);
    KT_CHECK(L.scope[0].x == L.scope[1].x && L.scope[1].y > L.scope[0].y && L.scope[2].y > L.scope[1].y);
    KT_CHECK(L.modeBtn[0].y == L.modeBtn[1].y && L.modeBtn[2].y > L.modeBtn[0].y && L.modeBtn[2].y == L.modeBtn[3].y && L.modeBtn[2].x == L.modeBtn[0].x);
    KT_CHECK(L.viewReset.y == L.modeBtn[0].y && L.viewFlip.y == L.modeBtn[0].y);   // the icons stay on the first mode row
    KT_CHECK(L.stance[0].y == L.stance[2].y && L.stance[3].y > L.stance[0].y && L.stance[3].y == L.stance[4].y);
    // the text fits the buttons at text_k 2.65 (button budget w - 20): no "..." for any caption
    const textfit::TextCtx tc = {2.65f, 1.0f, 1.0f};
    auto fits = [&](const char* t, const Box& b) { return textfit::textPx(tc, t, static_cast<int>(strlen(t))) <= static_cast<float>(buttonTextPx(b.w)); };
    static const char* const kBones[kV3Bones] = {"Upper back", "Back", "Pelvis", "R hip", "L hip"};
    static const char* const kStances[kV3Stances] = {"Idle", "Walk", "Run", "Alert", "Combat"};
    static const char* const kModes[kV3Modes] = {"Move", "Depth", "Tilt", "Roll/Turn"};
    for (int i = 0; i < kV3Bones; ++i) KT_CHECK(fits(kBones[i], L.bone[i]));
    for (int i = 0; i < kV3Stances; ++i) KT_CHECK(fits(kStances[i], L.stance[i]));
    for (int i = 0; i < kV3Modes; ++i) KT_CHECK(fits(kModes[i], L.modeBtn[i]));
    {   // the scope captions (scopeSegText) fit at 3840x1600 with the worst producible plural ("All vibroswords" is never produced: vibroswords are melee)
        char t[48];
        scopeSegText(t, sizeof t, 0, "heavy"); KT_CHECK(!strcmp(t, "This weapon model")); KT_CHECK(fits(t, L.scope[0]));
        scopeSegText(t, sizeof t, 1, "heavy"); KT_CHECK(!strcmp(t, "All heavy weapons")); KT_CHECK(fits(t, L.scope[1]));
        scopeSegText(t, sizeof t, 2, "heavy"); KT_CHECK(!strcmp(t, "This body + armor")); KT_CHECK(fits(t, L.scope[2]));
        KT_CHECK(fits("This weapon model", L.scope[0]) && fits("All heavy weapons", L.scope[1]) && fits("This body + armor", L.scope[2]));
    }
    KT_CHECK(L.view.h >= 600);
}

// The Remove button (PI_REMOVE): in the inspector zone, left aligned under the note, row high, at most kV3RemoveMaxW wide, overlapping nothing (not the view,
// the right description box, the left column or any other box) at the two gated resolutions and at the smaller sizes.
static void testRemoveBox() {
    struct Case { const char* what; Geom g; };
    const Case cases[] = {{"3840x1600 real", realGeom()}, {"3440x1440", geomFor(3440, 1440)}, {"1920x1080", geomFor(1920, 1080)}, {"1280x720", geomFor(1280, 720)}};
    for (const Case& c : cases) {
        const Geom& g = c.g;
        PageLayoutV3 L = pageLayoutV3(g.column, g.view, g.preview, g.hint, nullptr);
        KT_CHECK_EQ(violations(g, L, true, true), 0);   // covers pairwise overlap of every box incl. PI_REMOVE, and its inspector zone
        const Box R = L.remove;
        KT_CHECK(R.w > 0 && R.h > 0);
        KT_CHECK(inside(R, g.preview));
        KT_CHECK(R.y >= L.note.y + L.note.h + v3::kGap);          // below the note
        KT_CHECK(R.x == L.note.x && R.h == L.inspRowH);            // left aligned under it, a slider row high
        KT_CHECK(R.w <= kV3RemoveMaxW && R.w <= L.note.w);
        KT_CHECK(!overlaps(R, L.view) && !overlaps(R, L.hintR[0]) && !overlaps(R, L.hintR[1]) && !overlaps(R, L.hint[0]) && !overlaps(R, L.reset));
        for (int r = 0; r < kV3Rows; ++r) {
            const InspRowV3& W = L.rows[r];
            KT_CHECK(!overlaps(R, W.name) && !overlaps(R, W.track) && !overlaps(R, W.thumb) && !overlaps(R, W.value) && !overlaps(R, W.reset));
        }
        Box all[kPageItemsV3];
        pageBoxesV3(L, all);
        for (int i = 0; i < kPageItemsV3 - 1; ++i) KT_CHECK(!overlaps(all[i], R));
        // the wedges sit in the track rects: nothing of them reaches the Remove box either
        for (int r = 0; r < kV3Rows; ++r) KT_CHECK(!overlaps(R, L.rows[r].track));
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
    for (int i = 0; i < kV3Hints; ++i) KT_CHECK(same(all[PI_HINTR + i], L.hintR[i]));
    KT_CHECK(PI_HINTR == 70 && PI_HINTR + kV3Hints == PI_REMOVE);
    KT_CHECK(same(all[PI_REMOVE], L.remove));
    KT_CHECK(PI_REMOVE == 72 && PI_REMOVE + 1 == kPageItemsV3 && kPageItemCap == 80);

    // the page holds the 73 boxes plus one wedge per slider row, inside the consumer cap
    KT_CHECK_EQ(kPageItemsV3Total, 79);
    KT_CHECK(kPageItemsV3Total <= kPageItemCap);
}

int main() {
    testHelpers();
    testAdaptersAndIndex();
    testGeometries();
    testSaveRow();
    testReal3840x1600();
    testRemoveBox();
    testFill3840x1600();
    testRoomyButtons3840();
    testPinned3440();
    testViewVsPreview();
    testDescBox();
    testDescBox3840();
    testDescPane();
    testCoverDesc();
    testThumbs();
    testDegradation();
    return kt::Report("hse_layout_v3");
}
