// Combat Action Hotbar -- pure logic (C++17, no Win32/engine types, so it is unit-tested on the host and compiles unchanged in the DLL build).
// The DLL snapshots the engine's per-target-type action groups into GroupView/EntryView; this header only decides.
#pragma once
#include <stdint.h>
#include <string.h>

namespace ch {

struct EntryView { uint32_t key; bool usable; uint32_t owner; char icon[17]; };
struct GroupView { int count; EntryView e[16]; };

inline int clampCount(int c) { return c < 0 ? 0 : (c > 16 ? 16 : c); }

// Index of the entry to invoke for a target of type 0..3: the LAST usable entry with a non-zero owner, or -1.
// Any other targetType (0xFF = no target included) -> -1.
inline int pickInvokeEntry(const GroupView& g, int targetType) {
    if (targetType < 0 || targetType > 3) return -1;
    for (int i = clampCount(g.count) - 1; i >= 0; --i)
        if (g.e[i].usable && g.e[i].owner != 0) return i;
    return -1;
}

// Edge detector: update() returns true only on the up -> down transition.
struct KeyEdge {
    bool prev = false;
    bool update(bool down) { bool fire = down && !prev; prev = down; return fire; }
};

struct Slot { int group; uint32_t key; bool usable; char icon[17]; };

// Flatten the three groups (0,1,2 in order) into at most maxSlots slots, the first perGroup entries of each group. Returns the number written.
inline int fillSlots(const GroupView g[3], Slot* out, int maxSlots, int perGroup) {
    if (!g || !out || maxSlots <= 0 || perGroup <= 0) return 0;
    int n = 0;
    for (int gi = 0; gi < 3; ++gi) {
        int cnt = clampCount(g[gi].count);
        if (cnt > perGroup) cnt = perGroup;
        for (int i = 0; i < cnt; ++i) {
            if (n >= maxSlots) return n;
            Slot& s = out[n++];
            s.group = gi;
            s.key = g[gi].e[i].key;
            s.usable = g[gi].e[i].usable;
            memcpy(s.icon, g[gi].e[i].icon, 16);
            s.icon[16] = '\0';   // guaranteed NUL even if the source was not terminated
        }
    }
    return n;
}

struct Rect { int x, y, w, h; };

// n square buttons (btn px, gap px apart), the row centred on the banner's centre and directly below it (y = banner.y + banner.h + gap).
// The row is clamped horizontally into the 4:3 safe area when the screen is wider than 4:3, else into [0, screenW]. A row wider than the area pins to its left edge.
inline void layoutRow(int screenW, int screenH, Rect banner, int n, int btn, int gap, Rect* out) {
    if (!out || n <= 0) return;
    int lo = 0, hi = screenW;
    const int safeW = screenH * 4 / 3;
    if (screenW > safeW) { lo = (screenW - safeW) / 2; hi = lo + safeW; }
    const int rowW = n * btn + (n - 1) * gap;
    int x0 = banner.x + banner.w / 2 - rowW / 2;
    if (x0 + rowW > hi) x0 = hi - rowW;
    if (x0 < lo) x0 = lo;
    const int y = banner.y + banner.h + gap;
    for (int i = 0; i < n; ++i) { out[i].x = x0 + i * (btn + gap); out[i].y = y; out[i].w = btn; out[i].h = btn; }
}

// Index of the rect containing (x,y) (inclusive left/top, exclusive right/bottom), -1 on miss.
inline int hitSlot(const Rect* r, int n, int x, int y) {
    if (!r) return -1;
    for (int i = 0; i < n; ++i)
        if (x >= r[i].x && x < r[i].x + r[i].w && y >= r[i].y && y < r[i].y + r[i].h) return i;
    return -1;
}

}  // namespace ch
