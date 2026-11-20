// EquipBoxGeo.h - pure placement of the equip-screen compare box (ISF-EQBOX-3, W2, variant B: defensive).
// Engine px only. No engine calls, no globals, no UB: every intermediate is int64_t.
// Geometry: .claude/waves/ISF-EQBOX-2/u01.md section 3 (A5, A6 above LBL_BAR1; B6 fallback).
// Formulas copied from patch_manager_mods/inventory_sort_filter_src/BoxGeometry.h (pad, row pitch, gap;
// its 35%-band row cap is dropped). BoxGeometry::compute() is not called because it only places boxes inside LB_DESC.
// Host test: EquipBoxGeo_test.cpp next to this file.
#pragma once

#include <climits>
#include <cstdint>

namespace eqgeo {

// Which geometry placed the box. kModeNone = no box (hide it).
enum : int {
    kModeNone = 0,
    kModeA5 = 1,  // above LBL_BAR1, standard pad (6 gui) and gap (lbH*2/100)
    kModeA6 = 2,  // above LBL_BAR1, tight pad (3 gui) and gap (2 gui)
    kModeB6 = 3,  // fallback: x >= 482 gui (right of slot name), bottom at LB_DESC top minus gap
};

struct Rect {
    int x, y, w, h;  // engine px, origin top-left
};

struct In {
    Rect lb;    // LB_DESC (equip description list)
    Rect bar1;  // LBL_BAR1 (top header bar, full width)
    int sh;     // screen height px (scale = sh/600)
    int n;      // text rows including header, valid 1..6
};

struct Out {
    int rowP;          // row pitch px
    int boxX, boxY, boxW, boxH;
    int mode;          // kModeA5 / kModeA6 / kModeB6, or kModeNone
    bool ok;           // false: every numeric field 0 and mode kModeNone
};

namespace detail {

inline bool fitsInt(int64_t v) { return v >= INT_MIN && v <= INT_MAX; }

// Origin >= 0, size >= 1, and right/bottom edges representable as int.
inline bool rectOk(const Rect& r) {
    if (r.x < 0 || r.y < 0 || r.w < 1 || r.h < 1) return false;
    return static_cast<int64_t>(r.x) + r.w <= INT_MAX &&
           static_cast<int64_t>(r.y) + r.h <= INT_MAX;
}

// Slot grid top = LB_DESC top + 6 gui units (dump ids 2-11: grid starts y140, LB_DESC starts y134).
inline int64_t gridTop(int64_t lbY, int64_t sh) { return lbY + 6 * sh / 600; }

}  // namespace detail

inline Out compute(const In& in) {
    Out o{0, 0, 0, 0, 0, kModeNone, false};
    if (in.sh < 1 || in.n < 1 || in.n > 6) return o;
    if (!detail::rectOk(in.lb) || !detail::rectOk(in.bar1)) return o;

    const int64_t sh = in.sh;
    const int64_t n = in.n;
    const int64_t lbX = in.lb.x;
    const int64_t lbY = in.lb.y;
    const int64_t lbW = in.lb.w;
    const int64_t lbH = in.lb.h;
    const int64_t b1Y = in.bar1.y;

    // BoxGeometry formulas: standard pad 6 gui, row pitch 16 gui. The 35%-band cap is not applied:
    // it sizes rows to the backpack's inner band and would squeeze text in a short LB_DESC.
    const int64_t padStd = 6 * sh / 600;
    const int64_t rowP = 16 * sh / 600;
    if (rowP < 1) return o;

    const int64_t gapStd = lbH * 2 / 100;
    const int64_t aTop = b1Y < lbY ? b1Y : lbY;  // A boxes sit above the header bar
    const int64_t gTop = detail::gridTop(lbY, sh);

    for (int mode = kModeA5; mode <= kModeB6; ++mode) {
        int64_t pad = padStd;
        int64_t gap = gapStd;
        int64_t bx = lbX;
        int64_t bw = lbW;
        int64_t bottom = aTop - gapStd;
        if (mode == kModeA6) {
            pad = 3 * sh / 600;
            gap = 2 * sh / 600;
            bottom = aTop - gap;
        } else if (mode == kModeB6) {
            bx = lbX + 72 * lbW / 313;  // 482 gui: clears slot name (right edge 478 gui)
            bw = lbX + lbW - bx;
            bottom = lbY - gapStd;      // LB_DESC top is 134 gui, so 128 gui with gap 6
        } else {
            bottom = aTop - gap;
        }

        const int64_t bh = 2 * pad + n * rowP;
        const int64_t by = bottom - bh;
        const int64_t bb = by + bh;  // box bottom edge
        if (bw < 1 || bh < 1 || by < 0) continue;
        // Never overlaps LB_DESC or the slot grid, and stays on screen.
        if (bb > lbY || bb > gTop || bb > sh) continue;
        if (!detail::fitsInt(rowP) || !detail::fitsInt(bx) || !detail::fitsInt(bw) ||
            !detail::fitsInt(by) || !detail::fitsInt(bh)) {
            continue;
        }

        o.rowP = static_cast<int>(rowP);
        o.boxX = static_cast<int>(bx);
        o.boxY = static_cast<int>(by);
        o.boxW = static_cast<int>(bw);
        o.boxH = static_cast<int>(bh);
        o.mode = mode;
        o.ok = true;
        return o;
    }
    return o;
}

}  // namespace eqgeo
