// Pure hover/click hit-testing for the inventory sort strip. No engine calls.
#pragma once
#include "StripLayout.h"

namespace stripclk {

// Region ids: 0..3 = the four state slots (isf::State order), 4 = sort button, -1 = none.
constexpr int kRegionSort = 4;
constexpr int kRegionCount = 5;
constexpr int kRegionNone = -1;

struct Out { int hover; bool hoverEnter; int click; };

// First enabled region (0..4) containing the cursor, else kRegionNone.
inline int hitTest(int cx, int cy, const striplay::Rect slot[4], striplay::Rect sortR, const bool enabled[kRegionCount]) {
    for (int id = 0; id < kRegionCount; ++id) {
        if (!enabled[id]) continue;
        const striplay::Rect r = id < 4 ? slot[id] : sortR;
        if (striplay::contains(r, cx, cy)) return id;
    }
    return kRegionNone;
}

// One frame of hover/click state. prevHover is updated to the current hover (-1 when not foreground).
// A click needs a press edge while foreground on an enabled region.
inline Out step(int& prevHover, bool pressEdge, bool foreground, int cx, int cy,
                const striplay::Rect slot[4], striplay::Rect sortR, const bool enabled[kRegionCount]) {
    const int hitId = hitTest(cx, cy, slot, sortR, enabled);
    Out o{};
    o.hover = foreground ? hitId : kRegionNone;
    o.hoverEnter = o.hover != kRegionNone && o.hover != prevHover;
    prevHover = o.hover;
    o.click = (pressEdge && foreground && hitId != kRegionNone) ? hitId : kRegionNone;
    return o;
}

}  // namespace stripclk
