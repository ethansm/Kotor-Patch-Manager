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

// Per-group button grids under three banner boxes. btn = (box.w - gap*(cols-1)) / cols (min 1), square; group g's grid starts at (box.x, box.y+box.h+gap), filled row-major (column k%cols, row k/cols).
// outN[g] = min(max(count,0), cols*maxRows). out is 3*cols*maxRows rects, slot (g,k) at g*cols*maxRows+k, unused ones zeroed. Each grid's used bounding box is shifted (never shrunk) into [0,screenW) x [0,screenH).
inline void layoutGroupGrid(const Rect box[3], const int count[3], int cols, int maxRows, int gap, int screenW, int screenH, Rect* out, int outN[3]) {
    if (!out || !outN) return;
    if (!box || !count || cols < 1 || maxRows < 1) {
        if (cols >= 1 && maxRows >= 1) memset(out, 0, sizeof(Rect) * 3 * cols * maxRows);
        for (int g = 0; g < 3; ++g) outN[g] = 0;
        return;
    }
    const int per = cols * maxRows;
    memset(out, 0, sizeof(Rect) * 3 * per);
    for (int g = 0; g < 3; ++g) {
        int n = count[g] < 0 ? 0 : count[g];
        if (n > per) n = per;
        outN[g] = n;
        if (n == 0) continue;
        int btn = (box[g].w - gap * (cols - 1)) / cols;
        if (btn < 1) btn = 1;
        const int usedCols = n < cols ? n : cols, usedRows = (n + cols - 1) / cols;
        const int bw = usedCols * btn + (usedCols - 1) * gap, bh = usedRows * btn + (usedRows - 1) * gap;
        int x0 = box[g].x, y0 = box[g].y + box[g].h + gap;
        if (x0 + bw > screenW) x0 = screenW - bw;
        if (x0 < 0) x0 = 0;
        if (y0 + bh > screenH) y0 = screenH - bh;
        if (y0 < 0) y0 = 0;
        for (int k = 0; k < n; ++k) {
            Rect& r = out[g * per + k];
            r.x = x0 + (k % cols) * (btn + gap); r.y = y0 + (k / cols) * (btn + gap); r.w = btn; r.h = btn;
        }
    }
}

// Map a flat slot index (group*cols*maxRows + entry) back to its group and entry index. Returns the entry index and sets *group; -1 and *group = -1 when out of range or negative.
inline int slotEntry(int slotIndex, int cols, int maxRows, int* group) {
    const int per = cols * maxRows;
    if (slotIndex < 0 || per < 1 || slotIndex >= 3 * per) { if (group) *group = -1; return -1; }
    if (group) *group = slotIndex / per;
    return slotIndex % per;
}

// Union bounding box of the used rects: group g's outN[g] rects start at out[g*stride]. w = h = 0 when none are used.
inline Rect gridBounds(const Rect* out, const int outN[3], int stride) {
    Rect b = {0, 0, 0, 0};
    if (!out || !outN || stride < 1) return b;
    bool any = false;
    int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    for (int g = 0; g < 3; ++g) {
        const int n = outN[g] < 0 ? 0 : (outN[g] > stride ? stride : outN[g]);
        for (int i = 0; i < n; ++i) {
            const Rect& r = out[g * stride + i];
            if (!any) { x0 = r.x; y0 = r.y; x1 = r.x + r.w; y1 = r.y + r.h; any = true; continue; }
            if (r.x < x0) x0 = r.x;
            if (r.y < y0) y0 = r.y;
            if (r.x + r.w > x1) x1 = r.x + r.w;
            if (r.y + r.h > y1) y1 = r.y + r.h;
        }
    }
    if (any) { b.x = x0; b.y = y0; b.w = x1 - x0; b.h = y1 - y0; }
    return b;
}

// Three group columns spanning max(boxesUnion.w, banner.w), centred on the banner's centre, gapPx between columns (colW = (span - 2*gapPx)/3).
// Each column holds `cols` square buttons per row (side = min(btnPx, fit), fit = (colW - (cols-1)*padPx)/cols; btnPx <= 0 -> fit; min 1), rows centred in the column, padPx between buttons and rows.
// The grid starts at y = boxesUnion.y, filled row-major. outN[g] = min(max(count,0), cols*maxRows); group g's rects at out[g*cols*maxRows + i] (the slotEntry stride), unused ones zeroed.
// The whole grid (all groups together) is shifted, never shrunk, back inside [0,screenW) x [0,screenH) when its bounding box crosses an edge.
inline void layoutBannerGrid(Rect banner, Rect boxesUnion, const int count[3], int cols, int maxRows, int btnPx, int padPx, int gapPx, int screenW, int screenH, Rect* out, int outN[3]) {
    if (!out || !outN) return;
    if (!count || cols < 1 || maxRows < 1) {
        if (cols >= 1 && maxRows >= 1) memset(out, 0, sizeof(Rect) * 3 * cols * maxRows);
        for (int g = 0; g < 3; ++g) outN[g] = 0;
        return;
    }
    const int per = cols * maxRows;
    memset(out, 0, sizeof(Rect) * 3 * per);
    const int span = boxesUnion.w > banner.w ? boxesUnion.w : banner.w;
    const int spanX = banner.x + banner.w / 2 - span / 2;
    const int colW = (span - 2 * gapPx) / 3;
    int fit = (colW - (cols - 1) * padPx) / cols;
    if (fit < 1) fit = 1;
    int btn = (btnPx <= 0 || btnPx > fit) ? fit : btnPx;
    if (btn < 1) btn = 1;
    for (int g = 0; g < 3; ++g) {
        int n = count[g] < 0 ? 0 : count[g];
        if (n > per) n = per;
        outN[g] = n;
        const int colX = spanX + g * (colW + gapPx);
        for (int k = 0; k < n; ++k) {
            const int row = k / cols;
            int inRow = n - row * cols;
            if (inRow > cols) inRow = cols;
            const int rowW = inRow * btn + (inRow - 1) * padPx;
            Rect& r = out[g * per + k];
            r.x = colX + (colW - rowW) / 2 + (k % cols) * (btn + padPx);
            r.y = boxesUnion.y + row * (btn + padPx);
            r.w = btn; r.h = btn;
        }
    }
    const Rect b = gridBounds(out, outN, per);
    if (b.w == 0 && b.h == 0) return;
    int dx = 0, dy = 0;
    if (b.x + b.w > screenW) dx = screenW - (b.x + b.w);
    if (b.x + dx < 0) dx = -b.x;
    if (b.y + b.h > screenH) dy = screenH - (b.y + b.h);
    if (b.y + dy < 0) dy = -b.y;
    if (dx == 0 && dy == 0) return;
    for (int g = 0; g < 3; ++g)
        for (int k = 0; k < outN[g]; ++k) { out[g * per + k].x += dx; out[g * per + k].y += dy; }
}

// Hover freeze (CH4c): while the cursor stays inside the grid, the menu origin (menu+0x1648/+0x164C) captured on entry is held, so the banner and
// the buttons under the cursor stop following the target. curX/curY = this frame's engine origin, not the cursor.
struct FreezeState { bool active; int x, y; };

// Returns true (and sets *outX/*outY to the origin remembered on entry) while frozen; false (and *out = cur) otherwise.
inline bool freezeOrigin(FreezeState& s, bool cursorInGrid, int curX, int curY, int* outX, int* outY) {
    if (!cursorInGrid) {
        s.active = false;
        if (outX) *outX = curX;
        if (outY) *outY = curY;
        return false;
    }
    if (!s.active) { s.active = true; s.x = curX; s.y = curY; }
    if (outX) *outX = s.x;
    if (outY) *outY = s.y;
    return true;
}

}  // namespace ch
