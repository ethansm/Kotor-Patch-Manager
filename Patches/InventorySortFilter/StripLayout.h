// Pure geometry for the inventory sort strip (line 1: four state slots, line 2: sort button). No engine calls.
#pragma once

namespace striplay {

struct Rect { int x, y, w, h; };

// True when (px,py) is inside r: x inclusive, x+w exclusive, same for y. Zero or negative size is never contained.
inline bool contains(Rect r, int px, int py) {
    if (r.w <= 0 || r.h <= 0) return false;
    return px >= r.x && px < r.x + r.w && py >= r.y && py < r.y + r.h;
}

// Four equal slots left to right on line 1. Slot i is isf::State value i: All(0), New(1), Usable(2), Upgrade(3).
inline void slotRects(Rect line1, Rect out[4]) {
    const int sw = line1.w / 4;
    for (int i = 0; i < 4; ++i) {
        out[i].x = line1.x + i * sw;
        out[i].y = line1.y;
        out[i].w = sw;
        out[i].h = line1.h;
    }
}

// Sort button: left 45% of line 2, same x/y/h.
inline Rect sortRect(Rect line2) {
    return Rect{line2.x, line2.y, line2.w * 45 / 100, line2.h};
}

// Label drawn in a slot. Any other slot index returns "".
inline const char* slotLabel(int slot) {
    switch (slot) {
        case 0: return "All";
        case 1: return "New";
        case 2: return "Can Equip";
        case 3: return "Upgradeable";
        default: return "";
    }
}

}  // namespace striplay
