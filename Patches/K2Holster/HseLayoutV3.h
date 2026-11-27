// Holster editor page layout v3 (HW-UX3 L0c; pure: no Win32 types, no STL). Host-tested by hse_layout_v3_test.cpp.
// One call returns every Box of the rebuilt page: the left column (member, weapon, bone, scope, save/revert, status),
// the view area (mode bar in 2 x 2, enlarged view, Pause alone, stances 3 + 2, a bottom layer with Draw / Holster (left third) and the Speed slider (right two thirds, HW44), legend; one row each on tiny screens; the left column likewise has a roomy form (2 bone columns, one
// scope per row) and a compact one below kV3RoomyRowMin rows) and the inspector in the right pane (two groups, six slider
// rows with thumbs, value labels outside the track, reset squares, note, hint lines).
// Integer math only (float rounding differed between x87 and SSE builds, F18); the only floats are the thumb fractions,
// clamped before they touch a pixel (NaN -> 0), the same clamping the old modal layout used. 4 px pad, 2 px gaps.
//
// Geometry contract (measured UI px):
//   column  = MUI_PANE_COLUMN (left). The content uses at most kColWantW of its width; the rest is handed to the view.
//   view    = MUI_PANE_VIEW. The returned `view` is enlarged: it starts 4 px right of the used column and ends where
//             4 px short of the preview pane (preview.w > 0; clamped there even when the given view rect reaches into it,
//             never narrower than kV3MinViewW unless the pane leaves no room), else at view.x + view.w. Panes are assumed
//             ordered column < view < preview (left to right).
//   preview = MUI_PANE_PREVIEW (the area above the shrunken description). The inspector is at most kInspWantW wide,
//             left aligned at preview.x + 4.
//   hint    = unused (kept for the signature).
//   desc    = MUI_PANE_DESC (the SHRUNK description rect of mod-config under the split; optional, a zero Box = absent).
//             hint[0] is the LEFT description box. With a valid desc pane its BOTTOM is the pane bottom (so both boxes end on the same y) and its top is
//             max(status bottom + gap, bottom - kV3DescMaxH); it is shown from kV3DescMinH high (the box may reach below the column pane, never beyond the
//             column's x range). Without a desc pane: the free part of the left column under the status line (kV3DescMinH..kV3DescMaxH high).
//             hint[1] is the text area inset by kV3DescPad inside it (empty with it).
//   hintR   = the RIGHT description box: hintR[0] = the desc pane rect itself (drawn over mod-config's LB_DESC list box so its scrollbar is covered),
//             hintR[1] = its text area inset by kV3DescPad. Both empty when desc is absent or too small (the editor then writes only SetDesc).
// Documented minimum panes (everything has positive size and nothing overlaps): column 140 x 180, view 160 x 150,
// preview 170 x 200 (with or without the hint rect). Below the minimum the layout degrades
// by shrinking row heights to their floors, then by clipping each box to its own pane: widths and heights never go
// negative, boxes never leave their pane, and no two boxes of different widgets overlap (clipped boxes may be 0 sized).
// Every box belongs to one zone (left column, view area, inspector, hint rect); the zones are disjoint.
#pragma once
#include <math.h>
#include "../Common/EditKit.h"
#include "HseSpeed.h"   // speedThumbX: the Speed slider's thumb follows its value (integer math)

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

constexpr int kV3ColWantW = 600;  // widest the left column content grows (the editor widens the column pane into the left margin by ini margin_px)
constexpr int kV3InspWantW = 1100; // widest the inspector grows (the editor widens the preview pane into the right margin by ini margin_px)
constexpr int kV3LeftRowMax = 52, kV3InspRowMax = 70, kV3RowMin = 14;   // left column and inspector row heights (pick the largest that fits)
constexpr int kV3RoomyRowMin = 30;  // the roomy left column (2 bone columns, one scope per row) is used while its rows stay at least this high; below, the compact one
constexpr int kV3BarMax = 44, kV3BarMin = 12;   // mode bar / stance row heights
constexpr int kV3MinViewW = 8;    // the view area is at least this wide when the preview pane leaves any room
constexpr int kV3ViewGap = 16;    // the view area (and its bars) end this far short of the preview pane, so the right description box is not flush against them
constexpr int kV3CoverMax = 24, kV3CoverDefault = 6;   // description cover margin (coverDescPane; ini desc_cover_px)
constexpr int kV3DescPad = 12;   // text inset inside the left description box
constexpr int kV3DescMinH = 90, kV3DescMaxH = 360, kV3DescWantH = 130;   // left description box (hint[0], under the status line): shown from this height, at most this high
constexpr int kV3MinViewH = 60;   // the bars shrink before the view gets lower than this
constexpr int kV3NameW = 170, kV3ValueW = 200;    // inspector name and value label widths (shrink together when narrow)
constexpr int kV3TrackWant = 150;  // a narrow inspector keeps the track this wide before the labels take their full width
constexpr int kV3TrackMin = 60;   // the inspector slider track keeps at least this width (the labels shrink first)
constexpr int kV3ResetMax = 60;   // reset icon squares are up to 60 x 60 (smaller only when the row is): the button border insets the arrow by 16 px a side
constexpr int kV3SpeedValueW = 184;   // the Speed value label ("Speed 8x" is 171 px at text_k 2.65 + 8 px label pad) is at most this wide; the track takes the rest of the two thirds
constexpr int kV3SpeedTrackMin = 120; // the Speed track keeps this width before the label gives up its room (the label then shrinks to "8x"; see HolsterEditor.cpp speedShowText)
constexpr int kV3SpeedWedge = 1;      // the Speed track's wedge: one extra item beside the boxes (the six inspector rows have one each)

// Page items that become mod-config items: left 19 (member 3, captions 3, weapons 2, bones 5, scopes 3, save/revert 2,
// status 1) + view area 13 (mode 4, view reset + flip 2, view 1, stances 5, legend 1) + inspector 37 (group labels 2,
// group resets 2, rows 6 x 5, note 1, hints 2) + the Reset Scope button 1 (left column, appended after the hints so no index shifts)
// + the right description box and its text 2 (drawn over mod-config's LB_DESC) + the Remove button 1 (inspector zone, under the note) + the Pause button 1 (view area: alone and centred in the top layer under the view)
// + the Draw / Holster toggle 1 (view area: the left third of the bottom layer) + the Speed slider 3 (HW44: track, thumb, value label; the right two thirds of that layer; appended last).
// kPageItemsV3 + kV3Rows + kV3SpeedWedge (wedges) must stay <= 96 (kMaxItems; 80 up to mod-config 0.3.7-dev).
constexpr int kPageItemsV3 = 78;
constexpr int kV3RemoveMaxW = 480;   // the Remove saved placement button is at most this wide (left aligned under the note)

struct InspRowV3 { editkit::Box name, track, thumb, value, reset; };

struct PageLayoutV3 {
    // left column
    editkit::Box memberPrev, memberName, memberNext;
    editkit::Box capWeapon, capBone, capScope;            // small caption labels: WEAPON, BONE, APPLIES TO
    editkit::Box weapon[kV3Weapons];
    editkit::Box bone[kV3Bones];
    editkit::Box scope[kV3Scopes];
    editkit::Box save, revert, status;
    editkit::Box reset;                                   // third Save-row button (the small "..." toggle that opens the remove menu); appended after the hints in pageBoxesV3
    // view area
    editkit::Box modeBtn[kV3Modes];
    editkit::Box viewReset, viewFlip;                     // small icon squares at the right end of the mode strip
    editkit::Box view;                                    // the enlarged view rect
    editkit::Box stance[kV3Stances];
    editkit::Box pause;                                   // Pause / Play toggle for the preview animation (left half of the bar above the stance rows; appended in pageBoxesV3)
    editkit::Box draw;                                    // Draw / Holster toggle (HW20): the weapons slide to the hand / back to the mount in the preview (left third of the bottom layer; appended)
    editkit::Box speedTrack, speedThumb, speedValue;      // Speed slider (HW44, global, not per weapon): right two thirds of the bottom layer, beside draw; appended last
    editkit::Box legend;
    // inspector
    editkit::Box groupLabel[kV3Groups], groupReset[kV3Groups];
    InspRowV3 rows[kV3Rows];
    editkit::Box note;                                    // "Left: mirrored from right ..."
    editkit::Box hint[kV3Hints];                          // LEFT description box + its text area
    editkit::Box hintR[kV3Hints];                         // RIGHT description box (over LB_DESC) + its text area
    editkit::Box remove;                                  // "Remove saved placement" (shown only while the "..." menu is open): inspector zone, under the note
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
constexpr int kPad = 6, kGap = 4, kSpacer = 6;
constexpr int kMemberMinW = 106;   // member Prev / Next button width floor: "Next" is 83.5 px at text_k 2.65 and the engine box is the button width - 2 * (7 + 3)

// n equal cells across w with `gap` px gaps; cell i starts at x + i * (cellW + gap)
inline int cellW(int w, int n, int gap = 2) { return imax(0, (w - (n - 1) * gap) / n); }

// Left column at row height r. Returns the y just below the content plus the bottom pad.
// roomy: bones in 2 columns (3 rows) and one scope per row (3 rows) so every caption gets a wide button; else the compact grid (3 + 2 bones, 3 scopes in one row).
// fullCaps: the three section captions are full row boxes (the engine label's text inset leaves less than one 32 px line in a shorter box, so a r * 2 / 3 caption never drew its title).
// Only the last-resort fallback (no row height fits; the documented minimum panes) passes false and keeps the old r * 2 / 3 captions so the content still ends inside a tiny column.
inline int buildLeft(editkit::Box Z, int r, bool roomy, PageLayoutV3& L, bool fullCaps = true) {
    using editkit::Box;
    const int cap = fullCaps ? r : r * 2 / 3;
    const int x0 = Z.x + kPad, W = imax(0, Z.w - 2 * kPad);
    int y = Z.y + kPad;
    auto mk = [&](int x, int yy, int w, int h) { return clipTo(Box{x, yy, w, h}, Z); };

    int side = imin(imax(2 * r + 12, kMemberMinW), imax(0, (W - 2 * kGap) / 3));   // wide enough for the words "Prev" / "Next" (kMemberMinW keeps them inside the engine text box at r = 39)
    L.memberPrev = mk(x0, y, side, r);
    L.memberName = mk(x0 + side + kGap, y, imax(0, W - 2 * side - 2 * kGap), r);
    L.memberNext = mk(x0 + W - side, y, side, r);
    y += r + kGap;

    L.capWeapon = mk(x0, y, W, cap); y += cap + kGap;
    for (int i = 0; i < kV3Weapons; ++i) { L.weapon[i] = mk(x0, y, W, r); y += r + kGap; }

    L.capBone = mk(x0, y, W, cap); y += cap + kGap;
    int c3 = cellW(W, 3, kGap), c2 = cellW(W, 2, kGap);
    if (roomy) {
        // Upper back | Back / Pelvis | R hip / L hip (the odd one out takes the left half)
        for (int i = 0; i < kV3Bones; ++i) L.bone[i] = mk(x0 + (i % 2) * (c2 + kGap), y + (i / 2) * (r + kGap), c2, r);
        y += 3 * (r + kGap);
    } else {
        for (int i = 0; i < 3; ++i) L.bone[i] = mk(x0 + i * (c3 + kGap), y, c3, r);
        y += r + kGap;
        for (int i = 0; i < 2; ++i) L.bone[3 + i] = mk(x0 + i * (c2 + kGap), y, c2, r);
        y += r + kGap;
    }

    L.capScope = mk(x0, y, W, cap); y += cap + kGap;
    if (roomy) {
        for (int i = 0; i < kV3Scopes; ++i) { L.scope[i] = mk(x0, y, W, r); y += r + kGap; }
        y += kSpacer;
    } else {
        for (int i = 0; i < kV3Scopes; ++i) L.scope[i] = mk(x0 + i * (c3 + kGap), y, c3, r);
        y += r + kGap + kSpacer;
    }

    // Save | Revert | Reset: thirds, but Save gives 4 px to Reset (the label "Reset" misses the narrowest text budget by half a pixel at a plain third).
    const int w3 = c3;
    const int saveW = imax(w3 - 4, w3 / 2), revertW = w3;
    const int resetW = imax(0, W - saveW - revertW - 2 * kGap);
    L.save = mk(x0, y, saveW, r);
    L.revert = mk(x0 + saveW + kGap, y, revertW, r);
    L.reset = mk(x0 + saveW + revertW + 2 * kGap, y, resetW, r);
    y += r + kGap;
    L.status = mk(x0, y, W, r);
    y += r;
    // The left description box (hint[0]) takes what is left of the column below the status line (none when less than kV3DescMinH remains).
    // Placed from the final call only (the fit loops re-run buildLeft at smaller rows, which rewrites it).
    const int dTop = y + kGap, dH = Z.y + Z.h - kPad - dTop;
    L.hint[0] = dH >= kV3DescMinH ? mk(x0, dTop, W, imin(dH, kV3DescMaxH)) : Box{0, 0, 0, 0};
    // hint[1] is the text area inside the box (a label has no inner padding, so the text is its own item, drawn over the box).
    L.hint[1] = L.hint[0].w > 0 ? mk(L.hint[0].x + kV3DescPad, L.hint[0].y + kV3DescPad, imax(0, L.hint[0].w - 2 * kV3DescPad), imax(0, L.hint[0].h - 2 * kV3DescPad))
                                : Box{0, 0, 0, 0};
    return y + kPad;
}

// Inspector in zone Z (the preview pane) at row height rr, left aligned at Z.x + pad, width inspW.
// hintZone: unused (the hint lines left the inspector; see buildLeft).
inline int buildInspector(editkit::Box Z, editkit::Box /*hintZone: unused, the hints left the inspector*/, int rr, const float* thumbFrac, PageLayoutV3& L) {
    using editkit::Box;
    const int cap = rr * 2 / 3;
    const int sq = imin(kV3ResetMax, rr);
    const int X = Z.x + kPad, W = imin(imax(0, Z.w - 2 * kPad), kV3InspWantW);
    int y = Z.y + kPad;
    auto mk = [&](int x, int yy, int w, int h) { return clipTo(Box{x, yy, w, h}, Z); };

    // widths: name | track | value | reset, 2 px gaps; the labels take up to kV3NameW + kV3ValueW but leave the track at least kV3TrackMin
    // (the labels shrink together, name : value kept); the track takes what is left
    int avail = imax(0, W - sq - 3 * kGap);
    // The track keeps max(kV3TrackMin, min(kV3TrackWant, avail / 2)): a narrow inspector shares its width, a wide one gives the labels their full width.
    const int trackKeep = imax(kV3TrackMin, imin(kV3TrackWant, avail / 2));
    const int lw = imin(imax(0, avail - trackKeep), kV3NameW + kV3ValueW);
    const int nm = lw * kV3NameW / (kV3NameW + kV3ValueW), vw = lw - nm, tw = avail - lw;

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
    // The Remove button: a row-high button under the note (the inspector fit reserves it, so it exists at every size), left aligned, at most kV3RemoveMaxW wide.
    L.remove = mk(X, y, imin(W, kV3RemoveMaxW), rr);
    y += rr + kGap;

    return y - kGap + kPad;
}

// The Speed slider (HW44): one horizontal row R of height R.h, all of it inside R (clipped to R). Left to right: the track, a gap, the value label
// ("Speed 4x", at most kV3SpeedValueW) at the right end. The track keeps kV3SpeedTrackMin while the label keeps its full width; a narrower row takes the
// room from the label first (never below a third of the row, so the short form "8x" still fits). speed = the value (1..8): the thumb sits at its step.
inline void buildSpeed(editkit::Box R, int speed, PageLayoutV3& L) {
    using editkit::Box;
    auto mk = [&](int x, int yy, int w, int h) { return clipTo(Box{x, yy, w, h}, R); };
    const int avail = imax(0, R.w - kGap);                                            // the track and the value label share this
    const int vw = imin(kV3SpeedValueW, imax(avail / 3, avail - kV3SpeedTrackMin));   // the label keeps its full width while the track keeps kV3SpeedTrackMin
    const int tw = imax(0, avail - vw);                                                // the track takes the rest on the left
    L.speedTrack = mk(R.x, R.y, tw, R.h);
    L.speedValue = mk(R.x + tw + kGap, R.y, vw, R.h);
    const int thw = imin(kV3ThumbW, tw);
    L.speedThumb = mk(speedThumbX(speed, R.x, tw, thw), R.y, thw, R.h);
}

}  // namespace v3

// coverDescPane: grows the description pane by `cover` px on all four sides so the RIGHT description box (our framed label) fully covers the frame of
// mod-config's LB_DESC list box underneath (its brighter, thicker border otherwise peeks out around the box). Pure; the result is what the editor passes
// to pageLayoutV3 as `desc`, so the left box (bottom aligned on the pane) and the right box keep equal bottoms.
//   cover   clamped to 0..kV3CoverMax; 0 = the pane unchanged (the 0.4.9 behaviour)
//   top     never above the preview pane bottom (preview.h > 0); else only the cover
//   left    never left of preview.x - kPad (where the enlarged view ends), and never left of the screen
//   right   never right of max(pane right, preview right) (the editor widens the preview pane into the right margin), and never right of the screen
//   bottom  never below the screen (screen.w/h <= 0 = unknown, no screen clamp)
// A pane that is absent (w or h <= 0) is returned unchanged. The grown pane always contains the original (the clamps never cut into it).
inline editkit::Box coverDescPane(editkit::Box desc, editkit::Box preview, editkit::Box screen, int cover) {
    using editkit::Box;
    if (desc.w <= 0 || desc.h <= 0) return desc;
    const int c = v3::iclamp(cover, 0, kV3CoverMax);
    if (c == 0) return desc;
    int l = desc.x - c, t = desc.y - c, r = desc.x + desc.w + c, b = desc.y + desc.h + c;
    if (preview.w > 0 && preview.h > 0) {
        t = v3::imax(t, preview.y + preview.h);
        l = v3::imax(l, v3::imin(desc.x, preview.x - v3::kPad));
        r = v3::imin(r, v3::imax(desc.x + desc.w, preview.x + preview.w));
    }
    if (screen.w > 0 && screen.h > 0) {
        l = v3::imax(l, screen.x);
        t = v3::imax(t, screen.y);
        r = v3::imin(r, screen.x + screen.w);
        b = v3::imin(b, screen.y + screen.h);
    }
    // never smaller than the original pane (a pane already at the limit keeps its own edge)
    l = v3::imin(l, desc.x);
    t = v3::imin(t, desc.y);
    r = v3::imax(r, desc.x + desc.w);
    b = v3::imax(b, desc.y + desc.h);
    return Box{l, t, r - l, b - t};
}

// pageLayoutV3: see the geometry contract at the top. thumbFrac: six fractions in [0,1] (clamped; NaN -> 0), null = 0.5.
// hint: pass a zero Box {0,0,0,0} for none. speed: the Speed slider value (HseSpeed.h), places its thumb.
inline PageLayoutV3 pageLayoutV3(editkit::Box column, editkit::Box view, editkit::Box preview, editkit::Box hint,
                                 const float* thumbFrac, editkit::Box desc = editkit::Box{0, 0, 0, 0}, int speed = kSpeedDefault) {
    using editkit::Box;
    PageLayoutV3 L = PageLayoutV3();

    // ---- left column: content width capped, never reaching into the view pane
    int colW = v3::iclamp(v3::imin(column.w, kV3ColWantW), 0, v3::imax(0, view.x - 4 - column.x));
    Box colZ{column.x, column.y, colW, column.h};
    {
        // Rows first leave kV3DescWantH + kGap under the status line for the left description box; a column too short for that keeps the old fit
        // (the box then gets what is left, or is absent below kV3DescMinH).
        bool fitted = false;
        for (int pass = 0; pass < 2 && !fitted; ++pass) {
            const int bottom = colZ.y + v3::imax(0, colZ.h) - (pass == 0 ? kV3DescWantH + v3::kGap : 0);
            for (int r = kV3LeftRowMax; r >= kV3RoomyRowMin && !fitted; --r) {
                L.leftRowH = r;
                fitted = v3::buildLeft(colZ, r, true, L) <= bottom;
            }
            for (int r = kV3RoomyRowMin - 1; r >= kV3RowMin && !fitted; --r) {
                L.leftRowH = r;
                fitted = v3::buildLeft(colZ, r, false, L) <= bottom;
            }
        }
        if (!fitted) { L.leftRowH = kV3RowMin; v3::buildLeft(colZ, kV3RowMin, false, L, false); }
    }

    // ---- description boxes: bottom aligned on the desc pane (mod-config's shrunk LB_DESC)
    if (desc.w > 0 && desc.h > 0) {
        const int bottom = desc.y + desc.h;
        const int top = v3::imax(L.status.y + L.status.h + v3::kGap, bottom - kV3DescMaxH);
        if (L.status.w > 0 && bottom - top >= kV3DescMinH) {
            L.hint[0] = Box{L.status.x, top, L.status.w, bottom - top};
            L.hint[1] = Box{L.hint[0].x + kV3DescPad, top + kV3DescPad, v3::imax(0, L.hint[0].w - 2 * kV3DescPad), v3::imax(0, L.hint[0].h - 2 * kV3DescPad)};
        } else {
            L.hint[0] = Box{0, 0, 0, 0};
            L.hint[1] = Box{0, 0, 0, 0};
        }
        if (desc.w > 2 * kV3DescPad && desc.h > 2 * kV3DescPad) {
            L.hintR[0] = desc;
            L.hintR[1] = Box{desc.x + kV3DescPad, desc.y + kV3DescPad, desc.w - 2 * kV3DescPad, desc.h - 2 * kV3DescPad};
        }
    }

    // ---- view area: from 4 px right of the used column to the preview pane (or the old right edge)
    const int vl = column.x + colW + 4;
    // With a preview pane the view ends kV3ViewGap short of it (never beyond, so it cannot overlap the inspector even when the
    // given view rect reaches into the preview); only when that leaves less than kV3MinViewW the gap is given up (never past preview.x).
    int vr = view.x + view.w;
    if (preview.w > 0) {
        vr = preview.x - kV3ViewGap;
        if (vr - vl < kV3MinViewW) vr = v3::imin(vl + kV3MinViewW, preview.x);
    }
    const int vw = v3::imax(0, vr - vl);
    const Box vz{vl, view.y, vw, view.h};
    // Bar rows (HW44). Two-row bars while the view keeps kV3MinViewH + one bar of height at some bar height b >= kV3BarMin: modes 2 x 2 (the reset /
    // flip icons at the right end of the first row); under the view Pause alone (centred), stances Idle Walk Run (3) and Alert Combat (2), then the bottom
    // layer: Draw / Holster in its left third and the Speed slider in the right two thirds. Otherwise the single rows (tiny screens): one mode row, Pause,
    // the five stances in one row, the bottom layer. Cell widths are shared per row so the columns line up.
    auto barsNeed = [](int nb, int bb) { return nb * bb + bb * 2 / 3 + (nb + 1) * 2; };   // bar rows + the legend's third + the 2 px gaps around them
    constexpr int kTwoBars = 6, kOneBars = 4;   // bar rows around the view: 2 mode rows, Pause, 2 stance rows, bottom layer / mode row, Pause, stances, bottom layer
    int b = kV3BarMax;
    bool two = false;
    for (int bb = kV3BarMax; bb >= kV3BarMin; --bb) {
        if (view.h - barsNeed(kTwoBars, bb) >= kV3MinViewH + bb) { b = bb; two = true; break; }
    }
    if (!two) {
        for (b = kV3BarMax; b > kV3BarMin; --b) {
            if (view.h - barsNeed(kOneBars, b) >= kV3MinViewH) break;
        }
    }
    L.barH = b;
    const int lg = b * 2 / 3;
    const int nb = two ? kTwoBars : kOneBars;
    const int viewH = v3::imax(0, view.h - (nb * b + lg + (nb + 1) * 2));
    auto mkv = [&](int x, int y, int w, int h) { return v3::clipTo(Box{x, y, w, h}, vz); };
    int y0 = view.y;
    L.viewFlip = mkv(vr - b, y0, b, b);
    L.viewReset = mkv(vr - b - 2 - b, y0, b, b);
    const int limit = L.viewReset.x - 6;          // 6 px between the mode buttons and the icons
    int yv, ys, yb;
    const int s3 = v3::cellW(vw, 3), s2 = v3::cellW(vw, 2);
    if (two) {
        int mw = v3::iclamp((limit - vl - 2) / 2, 0, 190 * 2);
        for (int i = 0; i < kV3Modes; ++i) L.modeBtn[i] = mkv(vl + (i % 2) * (mw + 2), y0 + (i / 2) * (b + 2), mw, b);
        yv = y0 + 2 * (b + 2);
        L.view = mkv(vl, yv, vw, viewH);
        ys = yv + viewH + 2;
        // layers under the view: Pause / Play alone and centred (one half-width cell), Idle Walk Run (3 cells), Alert Combat (2 cells), then the bottom layer
        L.pause = mkv(vl + (vw - s2) / 2, ys, s2, b);
        for (int i = 0; i < kV3Stances; ++i) {
            if (i < 3) L.stance[i] = mkv(vl + i * (s3 + 2), ys + (b + 2), s3, b);
            else       L.stance[i] = mkv(vl + (i - 3) * (s2 + 2), ys + 2 * (b + 2), s2, b);
        }
        yb = ys + 3 * (b + 2);
    } else {
        int mw = v3::iclamp((limit - vl - 3 * 2) / 4, 0, 190);
        for (int i = 0; i < kV3Modes; ++i) L.modeBtn[i] = mkv(vl + i * (mw + 2), y0, mw, b);
        yv = y0 + b + 2;
        L.view = mkv(vl, yv, vw, viewH);
        ys = yv + viewH + 2;
        L.pause = mkv(vl + (vw - s2) / 2, ys, s2, b);
        const int sw = v3::cellW(vw, kV3Stances);
        for (int i = 0; i < kV3Stances; ++i) L.stance[i] = mkv(vl + i * (sw + 2), ys + (b + 2), sw, b);
        yb = ys + 2 * (b + 2);
    }
    // the bottom layer: Draw / Holster in the left third (as wide as one Idle / Walk / Run cell), the Speed slider in the right two thirds, one gap apart
    const int dw = s3;
    L.draw = mkv(vl, yb, dw, b);
    v3::buildSpeed(v3::clipTo(editkit::Box{vl + dw + 2, yb, v3::imax(0, vw - dw - 2), b}, vz), speed, L);
    const int yl = yb + b + 2;
    L.legend = mkv(vl, yl, vw, lg);

    // ---- inspector
    for (int rr = kV3InspRowMax; rr >= kV3RowMin; --rr) {
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
    out[n++] = L.reset;
    for (int i = 0; i < kV3Hints; ++i) out[n++] = L.hintR[i];
    out[n++] = L.remove;
    out[n++] = L.pause;
    out[n++] = L.draw;
    out[n++] = L.speedTrack; out[n++] = L.speedThumb; out[n++] = L.speedValue;
    return n;
}

// ---- adapters and item table (HW-UX3 Wave B) ----
// The editor holds its rects as hse::HseRect, K2MuiRect (mod-config) and editkit::Box; all three are {x, y, w, h} ints. These convert in and out so the
// caller never copies field by field (any struct with int x, y, w, h members works).
template <class R> inline editkit::Box boxOf(const R& r) { return editkit::Box{r.x, r.y, r.w, r.h}; }
template <class R> inline R rectOf(const editkit::Box& b) { R r; r.x = b.x; r.y = b.y; r.w = b.w; r.h = b.h; return r; }

// Index of each widget in pageBoxesV3 order. The editor's item table is indexed the same way (hse_layout_v3_test pins every index against pageBoxesV3).
enum PageIdxV3 {
    PI_MEMBER_PREV = 0, PI_MEMBER_NAME = 1, PI_MEMBER_NEXT = 2,
    PI_CAP_WEAPON = 3, PI_CAP_BONE = 4, PI_CAP_SCOPE = 5,
    PI_WEAPON = 6,        // + hand (2)
    PI_BONE = 8,          // + bone (5)
    PI_SCOPE = 13,        // + scope (3)
    PI_SAVE = 16, PI_REVERT = 17, PI_STATUS = 18,
    PI_MODE = 19,         // + mode (4)
    PI_VIEW_RESET = 23, PI_VIEW_FLIP = 24, PI_VIEW = 25,
    PI_STANCE = 26,       // + stance (5)
    PI_LEGEND = 31,
    PI_GROUP = 32,        // + group * 2 = label, + 1 = reset
    PI_ROW = 36,          // + row * 5 = name, +1 track, +2 thumb, +3 value, +4 reset
    PI_NOTE = 66,
    PI_HINT = 67,         // + hint (2)
    PI_RESET_SCOPE = 69,  // the small "..." toggle next to Save / Revert (appended after the hints); opens the remove menu
    PI_HINTR = 70,        // + hint (2): the RIGHT description box (over mod-config's LB_DESC) and its text label
    PI_REMOVE = 72,       // the "Remove saved placement" button (inspector zone, under the note); shown only while the menu is open
    PI_PAUSE = 73,        // the Pause / Play toggle for the preview animation (view area: left half of the bar above the Idle row; the sixth cell on tiny screens)
    PI_DRAW = 74,         // the Draw / Holster toggle (HW20; view area: the left third of the bottom layer under the stances (HW44); appended after the hints)
    PI_SPEED_TRACK = 75,  // the Speed slider (HW44, global): track, thumb, value label (the right two thirds of the bottom layer; appended last)
    PI_SPEED_THUMB = 76,
    PI_SPEED_VALUE = 77
};
constexpr int kRowStride = 5;   // boxes per inspector row (name, track, thumb, value, reset)
static_assert(PI_HINT + kV3Hints == PI_RESET_SCOPE && PI_RESET_SCOPE + 1 == PI_HINTR && PI_HINTR + kV3Hints == PI_REMOVE && PI_REMOVE + 1 == PI_PAUSE && PI_PAUSE + 1 == PI_DRAW && PI_DRAW + 1 == PI_SPEED_TRACK && PI_SPEED_TRACK + kSpeedItems == kPageItemsV3 && PI_SPEED_VALUE + 1 == kPageItemsV3, "the PageIdxV3 table must cover exactly kPageItemsV3 boxes");
static_assert(PI_ROW + kV3Rows * kRowStride == PI_NOTE, "inspector rows end at the note");

// The slider wedge (the teal fill behind each track) is a sixth item per row beside the boxes: its rect is the track rect cut to the thumb fraction
// (SliderArt.h sliderWedgeWidth). The Speed track has one wedge too (kV3SpeedWedge). The page therefore holds kPageItemsV3 + kV3Rows + kV3SpeedWedge
// items (85); the consumer cap (HseView.h kPageItemCap, 96) must hold them.
constexpr int kPageItemsV3Total = kPageItemsV3 + kV3Rows + kV3SpeedWedge;

// SetDescSplit(page, kDescSplitPermille): the inspector gets the top 85% of the description pane (mod-config clamps the value to 600..850).
constexpr int kDescSplitPermille = 850;

}  // namespace hse
