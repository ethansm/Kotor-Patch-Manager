// BoxGeometry.h - pure sizing for the ISF compare box (ISF-CMPBOX-1, wave 3, variant A).
// Engine px only. No engine calls, no globals. Spec: .claude/waves/ISF-CMPBOX-1/common.md
// (BoxGeometry.h) and u14.md (formulas). Host-testable: boxgeo_test.cpp next to this file.
#pragma once

#include <climits>
#include <cstdint>

namespace boxgeo {

struct In {
    int lbX, lbY, lbW, lbH;  // live LB_DESCRIPTION extent, engine px
    int sh;                  // screen height px (scale = sh/600)
    int n;                   // text rows incl. header, valid 1..6
    bool bottomOverride;     // false: mode A (box bottom = list bottom); true: mode B (box bottom = bottomY)
    int bottomY;             // used only when bottomOverride
};

struct Out {
    int rowP, boxH, boxX, boxY, boxW, listH;
    bool ok;                 // false: every numeric field is 0 (hide the box)
};

namespace detail {
inline bool fitsInt(int64_t v) { return v >= INT_MIN && v <= INT_MAX; }
}

inline Out compute(const In& in) {
    Out o{0, 0, 0, 0, 0, 0, false};

    const int64_t sh = in.sh;
    const int64_t lbY = in.lbY;
    const int64_t lbH = in.lbH;

    const int64_t pad = 6 * sh / 600;
    const int64_t capH = lbH * 35 / 100;
    // One row pitch for every n, so rows do not jump between selects.
    int64_t rowP = 16 * sh / 600;
    const int64_t fit = (capH - 2 * pad) / 6;
    if (fit < rowP) rowP = fit;

    if (in.n < 1 || in.n > 6 || rowP < 1 || lbH < 1) return o;

    const int64_t boxH = 2 * pad + static_cast<int64_t>(in.n) * rowP;
    const int64_t gap = lbH * 2 / 100;

    int64_t boxY;
    int64_t listH;
    if (!in.bottomOverride) {
        boxY = lbY + lbH - boxH;
        listH = lbH - boxH - gap;
    } else {
        boxY = static_cast<int64_t>(in.bottomY) - boxH;
        listH = boxY - gap - lbY;
    }

    if (listH < lbH / 2) return o;
    if (!detail::fitsInt(rowP) || !detail::fitsInt(boxH) || !detail::fitsInt(boxY) ||
        !detail::fitsInt(listH)) {
        return o;
    }

    o.rowP = static_cast<int>(rowP);
    o.boxH = static_cast<int>(boxH);
    o.boxX = in.lbX;
    o.boxY = static_cast<int>(boxY);
    o.boxW = in.lbW;
    o.listH = static_cast<int>(listH);
    o.ok = true;
    return o;
}

// Scrollbar slack: the engine shows the bar when the last text row ends past the list height, so a description that exactly fills the
// shortened list gets 3/4 of the 2% gap handed back; a long text still scrolls.
inline int shrinkSlack(int lbH) { return (lbH * 2 / 100) * 3 / 4; }
// Shortened list height: compute()'s listH (already excludes the gap) plus slack, never above the original list (mode B, few rows).
inline int shrinkListH(int goListH, int lbH) { const int h = goListH + shrinkSlack(lbH); return h > lbH ? lbH : h; }

}  // namespace boxgeo
