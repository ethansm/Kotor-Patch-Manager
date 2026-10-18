// Holster editor page layout v3 (HW-UX3 L0c; pure: no Win32 types, no STL). Host-tested by hse_layout_v3_test.cpp.
// One call returns every Box of the rebuilt page: the left column (member, weapon, bone, scope, save/revert, status),
// the view area (mode bar, enlarged view, stance row, legend) and the inspector in the right pane (two groups, six slider
// rows with thumbs, value labels outside the track, reset squares, note, hint lines).
// Integer math only (float rounding differed between x87 and SSE builds, F18); the only floats are the thumb fractions,
// clamped before they touch a pixel (NaN -> 0), exactly like HseLayout.h modalLayout. 4 px pad, 2 px gaps.
//
// Geometry contract (measured UI px):
//   column  = MUI_PANE_COLUMN (left). The content uses at most kColWantW of its width; the rest is handed to the view.
//   view    = MUI_PANE_VIEW. The returned `view` is enlarged: it starts 4 px right of the used column and ends where
//             the preview pane begins (view.x + view.w, or preview.x when that is further right). Panes are assumed
//             disjoint and ordered column < view < preview (left to right).
//   preview = MUI_PANE_PREVIEW (the area above the shrunken description). The inspector is at most kInspWantW wide,
//             left aligned at preview.x + 4.
//   hint    = optional description rect (w <= 0 or h <= 0 means none): the two hint lines go inside it. Without it they
//             are stacked under the mirrored note inside `preview`.
// Documented minimum panes (everything has positive size and nothing overlaps): column 140 x 180, view 160 x 150,
// preview 170 x 200 (with or without the hint rect). Below the minimum the layout degrades
// by shrinking row heights to their floors, then by clipping each box to its own pane: widths and heights never go
// negative, boxes never leave their pane, and no two boxes of different widgets overlap (clipped boxes may be 0 sized).
// Every box belongs to one zone (left column, view area, inspector, hint rect); the zones are disjoint.
#pragma once
#include <math.h>
#include "EditKit.h"

namespace hse {

constexpr int kV3Bones = 5;       // Upper back, Back, Pelvis, R hip, L hip (3 + 2 grid)
constexpr int kV3Weapons = 2;     // Right, Left (stacked)
constexpr int kV3Scopes = 3;      // This weapon, Type, All
constexpr int kV3Modes = 4;       // Move, Depth, Tilt, Roll/Turn
constexpr int kV3Stances = 5;     // Idle, Walk, Run, Alert, Combat
constexpr int kV3Rows = 6;        // Slide, Depth, Height (Position) then Tilt, Roll, Turn (Rotation)
constexpr int kV3Groups = 2;      // Position, Rotation
constexpr int kV3Hints = 2;       // hint lines under the inspector
constexpr int kV3ThumbW = 10;     // slider thumb width

constexpr int kV3ColWantW = 260;  // widest the left column content grows
constexpr int kV3InspWantW = 460; // widest the inspector grows
constexpr int kV3RowMax = 22, kV3RowMin = 14;   // left column and inspector row heights (pick the largest that fits)
constexpr int kV3BarMax = 24, kV3BarMin = 12;   // mode bar / stance row heights
constexpr int kV3MinViewH = 60;   // the bars shrink before the view gets lower than this
constexpr int kV3NameW = 52, kV3ValueW = 64;    // inspector name and value label widths (shrink together when narrow)
constexpr int kV3ResetMax = 20;   // reset icon squares are 20 x 20 (smaller only when the row is)

// Page items that become mod-config items: left 19 (member 3, captions 3, weapons 2, bones 5, scopes 3, save/revert 2,
// status 1) + view area 13 (mode 4, view reset + flip 2, view 1, stances 5, legend 1) + inspector 37 (group labels 2,
// group resets 2, rows 6 x 5, note 1, hints 2). Must stay <= 80 (kMaxItems), ideally <= 70.
constexpr int kPageItemsV3 = 69;

struct InspRowV3 { editkit::Box name, track, thumb, value, reset; };

struct PageLayoutV3 {
    // left column
    editkit::Box memberPrev, memberName, memberNext;
    editkit::Box capWeapon, capBone, capScope;            // small caption labels: WEAPON, BONE, APPLIES TO
    editkit::Box weapon[kV3Weapons];
    editkit::Box bone[kV3Bones];
    editkit::Box scope[kV3Scopes];
    editkit::Box save, revert, status;
    // view area
    editkit::Box modeBtn[kV3Modes];
    editkit::Box viewReset, viewFlip;                     // small icon squares at the right end of the mode strip
    editkit::Box view;                                    // the enlarged view rect
    editkit::Box stance[kV3Stances];
    editkit::Box legend;
    // inspector
    editkit::Box groupLabel[kV3Groups], groupReset[kV3Groups];
    InspRowV3 rows[kV3Rows];
    editkit::Box note;                                    // "Left: mirrored from right ..."
    editkit::Box hint[kV3Hints];
    // chosen sizes (for the caller's fonts and tests)
    int leftRowH, barH, inspRowH;
};

namespace v3 {

inline int imin(int a, int b) { return a < b ? a : b; }
inline int imax(int a, int b) { return a > b ? a : b; }
inline int iclamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

// Intersection of b with zone z, never negative, always inside z (a box wholly outside collapses to 0 size on the edge).
inline editkit::Box clipTo(editkit::Box b, editkit::Box z) {
    int zr = z.x + imax(0, z.w), zb = z.y + imax(0, z.h);
    int x0 = imin(imax(b.x, z.x), zr), x1 = imax(x0, imin(b.x + b.w, zr));
    int y0 = imin(imax(b.y, z.y), zb), y1 = imax(y0, imin(b.y + b.h, zb));
    return editkit::Box{x0, y0, x1 - x0, y1 - y0};
}
// n equal cells across w with 2 px gaps; cell i starts at x + i * (cellW + gap)
inline int cellW(int w, int n) { return imax(0, (w - (n - 1) * 2) / n); }

constexpr int kPad = 4, kGap = 2, kSpacer = 4;

// Left column at row height r. Returns the y just below the content plus the bottom pad.
inline int buildLeft(editkit::Box Z, int r, PageLayoutV3& L) {
    using editkit::Box;
    const int cap = r * 2 / 3;
    const int x0 = Z.x + kPad, W = imax(0, Z.w - 2 * kPad);
    int y = Z.y + kPad;
    auto mk = [&](int x, int yy, int w, int h) { return clipTo(Box{x, yy, w, h}, Z); };

    int side = imin(r, imax(0, (W - 2 * kGap) / 3));
    L.memberPrev = mk(x0, y, side, r);
    L.memberName = mk(x0 + side + kGap, y, imax(0, W - 2 * side - 2 * kGap), r);
    L.memberNext = mk(x0 + W - side, y, side, r);
    y += r + kGap;

    L.capWeapon = mk(x0, y, W, cap); y += cap + kGap;
    for (int i = 0; i < kV3Weapons; ++i) { L.weapon[i] = mk(x0, y, W, r); y += r + kGap; }

    L.capBone = mk(x0, y, W, cap); y += cap + kGap;
    int c3 = cellW(W, 3), c2 = cellW(W, 2);
    for (int i = 0; i < 3; ++i) L.bone[i] = mk(x0 + i * (c3 + kGap), y, c3, r);
    y += r + kGap;
    for (int i = 0; i < 2; ++i) L.bone[3 + i] = mk(x0 + i * (c2 + kGap), y, c2, r);
    y += r + kGap;

    L.capScope = mk(x0, y, W, cap); y += cap + kGap;
    for (int i = 0; i < kV3Scopes; ++i) L.scope[i] = mk(x0 + i * (c3 + kGap), y, c3, r);
    y += r + kGap + kSpacer;

    L.save = mk(x0, y, c2, r);
    L.revert = mk(x0 + c2 + kGap, y, c2, r);
    y += r + kGap;
    L.status = mk(x0, y, W, r);
    y += r;
    return y + kPad;
}

// Inspector in zone Z (the preview pane) at row height rr, left aligned at Z.x + pad, width inspW.
// hintZone: the optional description rect (w/h > 0) holding the hint lines; otherwise they are stacked in Z.
inline int buildInspector(editkit::Box Z, editkit::Box hintZone, int rr, const float* thumbFrac, PageLayoutV3& L) {
    using editkit::Box;
    const int cap = rr * 2 / 3;
    const int sq = imin(kV3ResetMax, rr);
    const int X = Z.x + kPad, W = imin(imax(0, Z.w - 2 * kPad), kV3InspWantW);
    int y = Z.y + kPad;
    auto mk = [&](int x, int yy, int w, int h) { return clipTo(Box{x, yy, w, h}, Z); };

    // widths: name | track | value | reset, 2 px gaps; the track takes what is left and the labels shrink only when it is 0
    int avail = imax(0, W - sq - 3 * kGap);
    int nm, vw, tw;
    if (avail < kV3NameW + kV3ValueW) { nm = avail * kV3NameW / (kV3NameW + kV3ValueW); vw = avail - nm; tw = 0; }
    else { nm = kV3NameW; vw = kV3ValueW; tw = avail - nm - vw; }

    for (int g = 0; g < kV3Groups; ++g) {
        if (g > 0) y += kSpacer;
        L.groupLabel[g] = mk(X, y, imax(0, W - sq - kGap), rr);
        L.groupReset[g] = mk(X + W - sq, y + (rr - sq) / 2, sq, sq);
        y += rr + kGap;
        for (int k = 0; k < 3; ++k) {
            const int ri = g * 3 + k;
            InspRowV3& R = L.rows[ri];
            int x = X;
            R.name = mk(x, y, nm, rr);
            x += nm + kGap;
            const int tx0 = x;
            R.track = mk(x, y, tw, rr);
            x += tw + kGap;
            R.value = mk(x, y, vw, rr);
            x += vw + kGap;
            R.reset = mk(x, y + (rr - sq) / 2, sq, sq);

            float f = thumbFrac ? thumbFrac[ri] : 0.5f;
            f = fminf(fmaxf(f, 0.f), 1.f);   // NaN -> 0
            const int thw = imin(kV3ThumbW, tw);
            int tx = tx0 + (int)(f * (float)tw + 0.5f) - kV3ThumbW / 2;
            tx = iclamp(tx, tx0, tx0 + tw - thw);
            R.thumb = mk(tx, y, thw, rr);
            y += rr + kGap;
        }
    }
    L.note = mk(X, y, W, cap);
    y += cap + kGap;

    if (hintZone.w > 0 && hintZone.h > 0) {
        int lh = iclamp((hintZone.h - 2 * kPad - kGap) / 2, 0, 16);
        for (int i = 0; i < kV3Hints; ++i)
            L.hint[i] = clipTo(Box{hintZone.x + kPad, hintZone.y + kPad + i * (lh + kGap), hintZone.w - 2 * kPad, lh}, hintZone);
    } else {
        for (int i = 0; i < kV3Hints; ++i) { L.hint[i] = mk(X, y, W, cap); y += cap + kGap; }
    }
    return y - kGap + kPad;
}

}  // namespace v3

// pageLayoutV3: see the geometry contract at the top. thumbFrac: six fractions in [0,1] (clamped; NaN -> 0), null = 0.5.
// hint: pass a zero Box {0,0,0,0} for none.
inline PageLayoutV3 pageLayoutV3(editkit::Box column, editkit::Box view, editkit::Box preview, editkit::Box hint,
                                 const float* thumbFrac) {
    using editkit::Box;
    PageLayoutV3 L = PageLayoutV3();

    // ---- left column: content width capped, never reaching into the view pane
    int colW = v3::iclamp(v3::imin(column.w, kV3ColWantW), 0, v3::imax(0, view.x - 4 - column.x));
    Box colZ{column.x, column.y, colW, column.h};
    for (int r = kV3RowMax; r >= kV3RowMin; --r) {
        L.leftRowH = r;
        if (v3::buildLeft(colZ, r, L) <= colZ.y + v3::imax(0, colZ.h)) break;
    }

    // ---- view area: from 4 px right of the used column to the preview pane (or the old right edge)
    const int vl = column.x + colW + 4;
    const int vr = v3::imax(view.x + view.w, preview.x);
    const int vw = v3::imax(0, vr - vl);
    const Box vz{vl, view.y, vw, view.h};
    int b = kV3BarMax;
    for (; b > kV3BarMin; --b) {
        if (view.h - (2 * b + b * 2 / 3 + 3 * 2) >= kV3MinViewH) break;
    }
    L.barH = b;
    const int lg = b * 2 / 3;
    const int viewH = v3::imax(0, view.h - (2 * b + lg + 3 * 2));
    auto mkv = [&](int x, int y, int w, int h) { return v3::clipTo(Box{x, y, w, h}, vz); };
    int y0 = view.y, y1 = y0 + b + 2, y2 = y1 + viewH + 2, y3 = y2 + b + 2;
    L.viewFlip = mkv(vr - b, y0, b, b);
    L.viewReset = mkv(vr - b - 2 - b, y0, b, b);
    {
        int limit = L.viewReset.x - 6;          // 6 px between the mode buttons and the icons
        int mw = v3::iclamp((limit - vl - 3 * 2) / 4, 0, 100);
        for (int i = 0; i < kV3Modes; ++i) L.modeBtn[i] = mkv(vl + i * (mw + 2), y0, mw, b);
    }
    L.view = mkv(vl, y1, vw, viewH);
    {
        int sw = v3::cellW(vw, kV3Stances);
        for (int i = 0; i < kV3Stances; ++i) L.stance[i] = mkv(vl + i * (sw + 2), y2, sw, b);
    }
    L.legend = mkv(vl, y3, vw, lg);

    // ---- inspector
    for (int rr = kV3RowMax; rr >= kV3RowMin; --rr) {
        L.inspRowH = rr;
        if (v3::buildInspector(preview, hint, rr, thumbFrac, L) <= preview.y + v3::imax(0, preview.h)) break;
    }
    return L;
}

// Every box of the page in a fixed order (left column, view area, inspector). Returns the count (== kPageItemsV3).
// `out` must hold kPageItemsV3 boxes.
inline int pageBoxesV3(const PageLayoutV3& L, editkit::Box* out) {
    int n = 0;
    out[n++] = L.memberPrev; out[n++] = L.memberName; out[n++] = L.memberNext;
    out[n++] = L.capWeapon; out[n++] = L.capBone; out[n++] = L.capScope;
    for (int i = 0; i < kV3Weapons; ++i) out[n++] = L.weapon[i];
    for (int i = 0; i < kV3Bones; ++i) out[n++] = L.bone[i];
    for (int i = 0; i < kV3Scopes; ++i) out[n++] = L.scope[i];
    out[n++] = L.save; out[n++] = L.revert; out[n++] = L.status;
    for (int i = 0; i < kV3Modes; ++i) out[n++] = L.modeBtn[i];
    out[n++] = L.viewReset; out[n++] = L.viewFlip; out[n++] = L.view;
    for (int i = 0; i < kV3Stances; ++i) out[n++] = L.stance[i];
    out[n++] = L.legend;
    for (int g = 0; g < kV3Groups; ++g) { out[n++] = L.groupLabel[g]; out[n++] = L.groupReset[g]; }
    for (int r = 0; r < kV3Rows; ++r) {
        out[n++] = L.rows[r].name; out[n++] = L.rows[r].track; out[n++] = L.rows[r].thumb;
        out[n++] = L.rows[r].value; out[n++] = L.rows[r].reset;
    }
    out[n++] = L.note;
    for (int i = 0; i < kV3Hints; ++i) out[n++] = L.hint[i];
    return n;
}

}  // namespace hse
