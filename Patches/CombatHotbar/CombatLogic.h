// Combat Action Hotbar -- pure logic (C++17, no Win32/engine types, so it is unit-tested on the host and compiles unchanged in the DLL build).
// The DLL snapshots the engine's per-target-type action groups into GroupView/EntryView; this header only decides.
#pragma once
#include <stdint.h>
#include <string.h>

namespace ch {

// Sizing defaults: increased padding and inter-button gaps
constexpr int kDefaultPad = 8;         // was 4/6 (inside pill margin between buttons)
constexpr int kDefaultGroupGap = 10;   // was 8 (gap between group pills)
constexpr int kDefaultSidePad = 16;    // was 6/10 (side pill padding)
constexpr int kDefaultCapPad = 16;     // was 10/6 (top/bottom cap padding)


struct EntryView { uint32_t key; bool usable; uint32_t owner; char icon[17]; uint32_t count; };   // count = stack size, 0 = not a stack
struct GroupView { int count; EntryView e[16]; };

inline int clampCount(int c) { return c < 0 ? 0 : (c > 16 ? 16 : c); }

// Alpha for a group pill or button: the vanilla alpha when the read succeeded and is in 0.05..1.0, else the 0.95 default (WI-81).
// Positive range test, so NaN also takes the default.
inline float pillAlpha(bool readOk, float value) { return (readOk && value >= 0.05f && value <= 1.0f) ? value : 0.95f; }

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

// Map a flat slot index (group*cols*maxRows + entry) across nGroups back to its group and entry index. Returns the entry index and sets *group; -1 and *group = -1 when out of range, nGroups < 1, or negative.
inline int slotEntryN(int slotIndex, int cols, int maxRows, int nGroups, int* group) {
    const int per = cols * maxRows;
    if (slotIndex < 0 || per < 1 || nGroups < 1 || slotIndex >= nGroups * per) { if (group) *group = -1; return -1; }
    if (group) *group = slotIndex / per;
    return slotIndex % per;
}

// 3-group specialization of slotEntryN.
inline int slotEntry(int slotIndex, int cols, int maxRows, int* group) {
    return slotEntryN(slotIndex, cols, maxRows, 3, group);
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

// Pill variant of layoutBannerGrid for nGroups: one rounded container per group (cont[g]), buttons inside it. span, spanX, colW = (span - (nGroups-1)*gapPx)/nGroups and colX = spanX + g*(colW+gapPx).
// Buttons are square, side = btnPx (<= 0 or > fit -> fit), fit = (colW - 2*sidePx - (cols-1)*padPx)/cols, min 1; rows centred in the column, padPx between buttons and rows.
// rows = max over groups of ceil(outN[g]/cols), at least 1; every container is cont = {colX, boxesUnion.y, colW, 2*capPx + rows*btn + (rows-1)*padPx} (uniform height, empty groups included).
// Row r of a container sits at y = cont.y + capPx + r*(btn+padPx). outN[g] = min(max(count,0), cols*maxRows); group g's rects at out[g*cols*maxRows + i], unused ones zeroed.
// The bounding box of the nGroups containers is shifted, never shrunk, back inside [0,screenW) x [0,screenH) (right/bottom edge first, then left/top wins); containers and buttons move together.
inline void layoutBannerGroupsN(Rect banner, Rect boxesUnion, const int* count, int nGroups, int cols, int maxRows, int btnPx, int padPx, int gapPx, int capPx, int sidePx, int screenW, int screenH, Rect* out, int* outN, Rect* cont) {
    if (!out || !outN || !cont || nGroups < 1) return;
    if (!count || cols < 1 || maxRows < 1) {
        if (cols >= 1 && maxRows >= 1) memset(out, 0, sizeof(Rect) * (size_t)nGroups * (size_t)cols * (size_t)maxRows);
        for (int g = 0; g < nGroups; ++g) { outN[g] = 0; cont[g].x = cont[g].y = cont[g].w = cont[g].h = 0; }
        return;
    }
    const int per = cols * maxRows;
    memset(out, 0, sizeof(Rect) * (size_t)nGroups * (size_t)per);
    const int span = boxesUnion.w > banner.w ? boxesUnion.w : banner.w;
    const int spanX = banner.x + banner.w / 2 - span / 2;
    const int colW = (span - (nGroups - 1) * gapPx) / nGroups;
    int fit = (colW - 2 * sidePx - (cols - 1) * padPx) / cols;
    if (fit < 1) fit = 1;
    int btn = (btnPx <= 0 || btnPx > fit) ? fit : btnPx;
    if (btn < 1) btn = 1;
    int rows = 1;
    for (int g = 0; g < nGroups; ++g) {
        int n = count[g] < 0 ? 0 : count[g];
        if (n > per) n = per;
        outN[g] = n;
        const int r = (n + cols - 1) / cols;
        if (r > rows) rows = r;
    }
    const int h = 2 * capPx + rows * btn + (rows - 1) * padPx;
    for (int g = 0; g < nGroups; ++g) {
        const int n = outN[g];
        const int colX = spanX + g * (colW + gapPx);
        cont[g].x = colX; cont[g].y = boxesUnion.y; cont[g].w = colW; cont[g].h = h;
        for (int k = 0; k < n; ++k) {
            const int row = k / cols;
            int inRow = n - row * cols;
            if (inRow > cols) inRow = cols;
            const int rowW = inRow * btn + (inRow - 1) * padPx;
            Rect& r = out[g * per + k];
            r.x = colX + (colW - rowW) / 2 + (k % cols) * (btn + padPx);
            r.y = cont[g].y + capPx + row * (btn + padPx);
            r.w = btn; r.h = btn;
        }
    }
    int x0 = cont[0].x, x1 = cont[0].x + cont[0].w;
    for (int g = 1; g < nGroups; ++g) {
        if (cont[g].x < x0) x0 = cont[g].x;
        if (cont[g].x + cont[g].w > x1) x1 = cont[g].x + cont[g].w;
    }
    const int y0 = cont[0].y, y1 = cont[0].y + h;
    int dx = 0, dy = 0;
    if (x1 > screenW) dx = screenW - x1;
    if (x0 + dx < 0) dx = -x0;
    if (y1 > screenH) dy = screenH - y1;
    if (y0 + dy < 0) dy = -y0;
    if (dx == 0 && dy == 0) return;
    for (int g = 0; g < nGroups; ++g) {
        cont[g].x += dx; cont[g].y += dy;
        for (int k = 0; k < outN[g]; ++k) { out[g * per + k].x += dx; out[g * per + k].y += dy; }
    }
}

// 3-group specialization of layoutBannerGroupsN.
inline void layoutBannerGroups(Rect banner, Rect boxesUnion, const int count[3], int cols, int maxRows, int btnPx, int padPx, int gapPx, int capPx, int sidePx, int screenW, int screenH, Rect* out, int outN[3], Rect cont[3]) {
    layoutBannerGroupsN(banner, boxesUnion, count, 3, cols, maxRows, btnPx, padPx, gapPx, capPx, sidePx, screenW, screenH, out, outN, cont);
}

// Columns per visible pill: colsTotal / nVis for 1..3 visible groups; when 4+ groups are visible and colsTotal >= 6 (default 6 was chosen for 3x2),
// rounded division (colsTotal + nVis/2) / nVis keeps 2 columns per pill at default ColsTotal=6 while still allowing 1 column when colsTotal <= 5.
inline int pillCols(int colsTotal, int nVis, int per) {
    if (nVis < 1 || per < 1) return 1;
    int cols = (nVis >= 4 && colsTotal >= 6) ? (colsTotal + nVis / 2) / nVis : (colsTotal / nVis);
    if (cols < 1) cols = 1;
    if (cols > per) cols = per;
    return cols;
}

// Content-sized pills across nGroups (one per non-empty group, empty groups hidden): outN[g] = min(max(count,0), per); nVis = groups with outN > 0; none -> everything zeroed.
// cols = pillCols(colsTotal, nVis, per); span = max(boxesUnion.w, banner.w); share = (span - (nVis-1)*gapPx)/nVis; fit = (share - 2*sidePx - (cols-1)*padPx)/cols, min 1; btn = (btnPx <= 0 || btnPx > fit) ? fit : btnPx, min 1.
// usedCols = min(cols, max visible outN); rows = max over visible groups of ceil(outN/cols); every visible pill has pillW = 2*sidePx + usedCols*btn + (usedCols-1)*padPx, pillH = 2*capPx + rows*btn + (rows-1)*padPx.
// The pills (gapPx apart, in group order) are centred on the banner centre at y = boxesUnion.y; cont[g] of a hidden group is zero. Buttons fill row-major at out[g*per + k], each row centred in its pill, row r at y = cont.y + capPx + r*(btn+padPx).
// The pills' bounding box is shifted, never resized, back inside [0,screenW) x [0,screenH) (right/bottom edge first, then left/top wins); visible containers and their buttons move together.
inline void layoutBannerPillsN(Rect banner, Rect boxesUnion, const int* count, int nGroups, int per, int colsTotal, int btnPx, int padPx, int gapPx, int capPx, int sidePx, int screenW, int screenH, Rect* out, int* outN, Rect* cont) {
    if (!out || !outN || !cont || nGroups < 1) return;
    if (!count || per < 1) {
        if (per >= 1) memset(out, 0, sizeof(Rect) * (size_t)nGroups * (size_t)per);
        for (int g = 0; g < nGroups; ++g) { outN[g] = 0; cont[g].x = cont[g].y = cont[g].w = cont[g].h = 0; }
        return;
    }
    memset(out, 0, sizeof(Rect) * (size_t)nGroups * (size_t)per);
    int nVis = 0, maxN = 0;
    for (int g = 0; g < nGroups; ++g) {
        int n = count[g] < 0 ? 0 : count[g];
        if (n > per) n = per;
        outN[g] = n;
        cont[g].x = cont[g].y = cont[g].w = cont[g].h = 0;
        if (n > 0) { ++nVis; if (n > maxN) maxN = n; }
    }
    if (nVis == 0) return;
    const int cols = pillCols(colsTotal, nVis, per);
    const int span = boxesUnion.w > banner.w ? boxesUnion.w : banner.w;
    const int share = (span - (nVis - 1) * gapPx) / nVis;
    int fit = (share - 2 * sidePx - (cols - 1) * padPx) / cols;
    if (fit < 1) fit = 1;
    int btn = (btnPx <= 0 || btnPx > fit) ? fit : btnPx;
    if (btn < 1) btn = 1;
    const int usedCols = cols < maxN ? cols : maxN;
    int rows = 1;
    for (int g = 0; g < nGroups; ++g) {
        const int r = (outN[g] + cols - 1) / cols;
        if (outN[g] > 0 && r > rows) rows = r;
    }
    const int pillW = 2 * sidePx + usedCols * btn + (usedCols - 1) * padPx;
    const int pillH = 2 * capPx + rows * btn + (rows - 1) * padPx;
    const int total = nVis * pillW + (nVis - 1) * gapPx;
    const int startX = banner.x + banner.w / 2 - total / 2;
    int vi = 0;
    for (int g = 0; g < nGroups; ++g) {
        const int n = outN[g];
        if (n <= 0) continue;
        cont[g].x = startX + vi * (pillW + gapPx); cont[g].y = boxesUnion.y; cont[g].w = pillW; cont[g].h = pillH;
        ++vi;
        for (int k = 0; k < n; ++k) {
            const int row = k / cols;
            int inRow = n - row * cols;
            if (inRow > cols) inRow = cols;
            const int rowW = inRow * btn + (inRow - 1) * padPx;
            Rect& r = out[g * per + k];
            r.x = cont[g].x + (pillW - rowW) / 2 + (k % cols) * (btn + padPx);
            r.y = cont[g].y + capPx + row * (btn + padPx);
            r.w = btn; r.h = btn;
        }
    }
    const int x0 = startX, x1 = startX + total, y0 = boxesUnion.y, y1 = boxesUnion.y + pillH;
    int dx = 0, dy = 0;
    if (x1 > screenW) dx = screenW - x1;
    if (x0 + dx < 0) dx = -x0;
    if (y1 > screenH) dy = screenH - y1;
    if (y0 + dy < 0) dy = -y0;
    if (dx == 0 && dy == 0) return;
    for (int g = 0; g < nGroups; ++g) {
        if (outN[g] <= 0) continue;
        cont[g].x += dx; cont[g].y += dy;
        for (int k = 0; k < outN[g]; ++k) { out[g * per + k].x += dx; out[g * per + k].y += dy; }
    }
}

// 3-group specialization of layoutBannerPillsN.
inline void layoutBannerPills(Rect banner, Rect boxesUnion, const int count[3], int per, int colsTotal, int btnPx, int padPx, int gapPx, int capPx, int sidePx, int screenW, int screenH, Rect* out, int outN[3], Rect cont[3]) {
    layoutBannerPillsN(banner, boxesUnion, count, 3, per, colsTotal, btnPx, padPx, gapPx, capPx, sidePx, screenW, screenH, out, outN, cont);
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

// ---- INI profile: per-group pin / hide lists and sort mode, plus group ordering.
constexpr int ProfMax = 8, ProfName = 48;
struct Profile {
    char pin[3][ProfMax][ProfName]; int nPin[3];
    char hide[3][ProfMax][ProfName]; int nHide[3];
    bool byCount[3];
};

inline void defaultProfile(Profile& p) {
    memset(&p, 0, sizeof p);
    p.byCount[2] = true;
}

namespace detail {
inline char lower(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c; }
inline bool isBlank(char c) { return c == ' ' || c == '\t'; }
// Case-insensitive compare of a[0..alen) with b[0..blen).
inline bool eqNoCase(const char* a, size_t alen, const char* b, size_t blen) {
    if (alen != blen) return false;
    for (size_t i = 0; i < alen; ++i) if (lower(a[i]) != lower(b[i])) return false;
    return true;
}
inline bool eqNoCase(const char* a, const char* b) { return eqNoCase(a, strlen(a), b, strlen(b)); }
inline int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
// Parse trimmed case-insensitive "count" -> *byCount=true or "engine" -> *byCount=false.
inline bool parseSortValue(const char* val, bool* byCount) {
    if (!val || !byCount) return false;
    const char* b = val; const char* e = val + strlen(val);
    while (b < e && isBlank(*b)) ++b;
    while (e > b && isBlank(e[-1])) --e;
    if (eqNoCase(b, (size_t)(e - b), "count", 5)) { *byCount = true; return true; }
    if (eqNoCase(b, (size_t)(e - b), "engine", 6)) { *byCount = false; return true; }
    return false;
}
}  // namespace detail

// Split csv on ',', trim spaces/tabs, skip empty tokens, truncate each to ProfName-1 chars, drop case-insensitive duplicates, cap at max. Returns the count.
inline int parseNameList(const char* csv, char out[][ProfName], int max) {
    if (!csv || !out || max <= 0) return 0;
    int n = 0;
    const char* p = csv;
    while (*p && n < max) {
        const char* end = p;
        while (*end && *end != ',') ++end;
        const char* b = p; const char* e = end;
        while (b < e && detail::isBlank(*b)) ++b;
        while (e > b && detail::isBlank(e[-1])) --e;
        size_t len = (size_t)(e - b);
        if (len > (size_t)(ProfName - 1)) len = ProfName - 1;
        if (len > 0) {
            bool dup = false;
            for (int i = 0; i < n && !dup; ++i) dup = detail::eqNoCase(out[i], strlen(out[i]), b, len);
            if (!dup) { memcpy(out[n], b, len); out[n][len] = '\0'; ++n; }
        }
        p = *end ? end + 1 : end;
    }
    return n;
}

// Does an entry (display name, engine key) match a profile token?
//  - null/empty token: never.
//  - "0x..."/"0X..." token: hex key, matches iff equal to key (name is ignored and may be null). Malformed hex never matches.
//  - otherwise (case-insensitive): name equals token, OR name ends with " " + token. So "Flurry" matches "Master Flurry", but "Sonic Grenade" does
//    NOT match "Minor Sonic Detonator". By the same suffix rule "Attack" DOES match "Power Attack" -- that is the defined behaviour.
inline bool nameMatches(const char* name, uint32_t key, const char* token) {
    if (!token || !token[0]) return false;
    if (token[0] == '0' && (token[1] == 'x' || token[1] == 'X')) {
        const char* h = token + 2;
        int digits = 0; uint32_t v = 0;
        for (; *h; ++h, ++digits) {
            int d = detail::hexVal(*h);
            if (d < 0 || digits >= 8) return false;
            v = (v << 4) | (uint32_t)d;
        }
        return digits > 0 && v == key;
    }
    if (!name || !name[0]) return false;
    const size_t nl = strlen(name), tl = strlen(token);
    if (nl == tl) return detail::eqNoCase(name, nl, token, tl);
    if (nl > tl + 1 && name[nl - tl - 1] == ' ') return detail::eqNoCase(name + nl - tl, tl, token, tl);
    return false;
}

// Order group gi's entries (token matching = nameMatches, but an exact-name entry in the group disables the suffix rule for that token): hidden entries dropped (hide wins over pin); each pin token in INI order places the first unplaced match;
// the rest follow in engine order, or by stack count descending (stable) when p.byCount[gi]. Writes entry indices to ord (cap maxOut), returns the number written.
inline int orderGroup(const GroupView& g, const char names[][64], const Profile& p, int gi, int* ord, int maxOut) {
    if (gi < 0 || gi > 2 || !names || !ord || maxOut <= 0) return 0;
    const int n = clampCount(g.count);
    // A token that equals some entry's name exactly matches only exact names, so Hide0=Attack hides "Attack" but not "Power Attack";
    // the " " + token suffix rule applies only when no entry has that exact name ("Flurry" -> "Master Flurry").
    auto hits = [&](int i, const char* tok) {
        if (!nameMatches(names[i], g.e[i].key, tok)) return false;
        if (tok[0] == '0' && (tok[1] == 'x' || tok[1] == 'X')) return true;
        if (detail::eqNoCase(names[i], tok)) return true;
        for (int j = 0; j < n; ++j) if (names[j][0] && detail::eqNoCase(names[j], tok)) return false;
        return true;
    };
    bool hidden[16] = {}, placed[16] = {};
    for (int i = 0; i < n; ++i)
        for (int t = 0; t < p.nHide[gi] && !hidden[i]; ++t)
            hidden[i] = hits(i, p.hide[gi][t]);
    int seq[16]; int ns = 0;
    for (int t = 0; t < p.nPin[gi]; ++t)
        for (int i = 0; i < n; ++i)
            if (!hidden[i] && !placed[i] && hits(i, p.pin[gi][t])) { placed[i] = true; seq[ns++] = i; break; }
    const int restStart = ns;
    for (int i = 0; i < n; ++i) if (!hidden[i] && !placed[i]) seq[ns++] = i;
    if (p.byCount[gi]) {   // stable insertion sort, count descending
        for (int a = restStart + 1; a < ns; ++a) {
            const int v = seq[a]; int b = a - 1;
            while (b >= restStart && g.e[seq[b]].count < g.e[v].count) { seq[b + 1] = seq[b]; --b; }
            seq[b + 1] = v;
        }
    }
    const int w = ns < maxOut ? ns : maxOut;
    for (int i = 0; i < w; ++i) ord[i] = seq[i];
    return w;
}

// FNV-1a 32-bit over n (4 bytes, little-endian) then each g.e[ord[i]].key (4 bytes, little-endian). null ord with n > 0 hashes n only.
inline uint32_t orderSig(const GroupView& g, const int* ord, int n) {
    uint32_t h = 2166136261u;
    auto mix = [&h](uint32_t v) { for (int s = 0; s < 32; s += 8) { h ^= (v >> s) & 0xFFu; h *= 16777619u; } };
    mix((uint32_t)n);
    if (!ord) return h;
    for (int i = 0; i < n; ++i) mix((ord[i] >= 0 && ord[i] < 16) ? g.e[ord[i]].key : 0u);
    return h;
}

// Serialize as lines "Pin<g>=a,b", "Hide<g>=...", "Sort<g>=count|engine" (g = 0..2) joined with '\n', no trailing newline, NUL-terminated.
// Returns the length, or -1 (buf[0] = 0 when size > 0) if it does not fit.
inline int formatProfile(const Profile& p, char* buf, int size) {
    if (!buf || size <= 0) return -1;
    int len = 0; bool ok = true;
    auto put = [&](const char* s, size_t l) {
        if (!ok) return;
        if ((size_t)len + l + 1 > (size_t)size) { ok = false; return; }
        memcpy(buf + len, s, l); len += (int)l;
    };
    auto putS = [&](const char* s) { put(s, strlen(s)); };
    for (int g = 0; g < 3 && ok; ++g) {
        const char d[2] = {(char)('0' + g), '\0'};
        for (int k = 0; k < 2; ++k) {
            if (g > 0 || k > 0) putS("\n");
            putS(k == 0 ? "Pin" : "Hide"); putS(d); putS("=");
            const int cnt = k == 0 ? p.nPin[g] : p.nHide[g];
            for (int i = 0; i < cnt && i < ProfMax; ++i) { if (i) putS(","); putS(k == 0 ? p.pin[g][i] : p.hide[g][i]); }
        }
        putS("\nSort"); putS(d); putS(p.byCount[g] ? "=count" : "=engine");
    }
    if (!ok) { buf[0] = '\0'; return -1; }
    buf[len] = '\0';
    return len;
}

// Apply one INI line. key (case-insensitive): Pin0..2 / Hide0..2 replace that list (null val = empty); Sort0..2 accepts "count"/"engine" (trimmed, case-insensitive).
// Returns false for an unknown key or bad Sort value, leaving p unchanged.
inline bool parseProfileLine(Profile& p, const char* key, const char* val) {
    if (!key) return false;
    const size_t kl = strlen(key);
    auto isKey = [&](const char* base, int* gi) {
        const size_t bl = strlen(base);
        if (kl != bl + 1 || !detail::eqNoCase(key, bl, base, bl) || key[bl] < '0' || key[bl] > '2') return false;
        *gi = key[bl] - '0'; return true;
    };
    int gi = 0;
    if (isKey("Pin", &gi))  { p.nPin[gi] = parseNameList(val, p.pin[gi], ProfMax); return true; }
    if (isKey("Hide", &gi)) { p.nHide[gi] = parseNameList(val, p.hide[gi], ProfMax); return true; }
    if (isKey("Sort", &gi)) return detail::parseSortValue(val, &p.byCount[gi]);
    return false;
}

// ---- Display category 3: Wrist-Launcher Darts & Rockets (split from engine group 2) ----
constexpr int kNumCats = 4;        // 0 = attacks/feats, 1 = force powers, 2 = grenades, 3 = wrist-launcher darts/rockets
constexpr int kMaxRawItems = 32;   // pre-split scan cap for engine group 2 (10 grenade baseitems + 11 rocket/dart variants <= 32)

// Engine group index (0..2) that owns display category `cat` (0..3): both Grenades (2) and Darts/Rockets (3) live in engine group 2.
inline int engineGroupForCategory(int cat) { return cat == 3 ? 2 : cat; }

// True when an engine group 2 entry is a Wrist Launcher dart or rocket (baseitems.2da row 92, cls=w_rocket -> icon "iw_rocket_NNN";
// fallback to "Rocket:" / "Dart:" display name prefix if icon is empty or custom).
inline bool isRocketEntry(const char* icon, const char* name = nullptr) {
    if (icon && icon[0]) {
        size_t il = strlen(icon);
        if (il >= 9 && detail::eqNoCase(icon, 9, "iw_rocket", 9)) return true;
    }
    if (name && name[0]) {
        size_t nl = strlen(name);
        if (nl >= 7 && detail::eqNoCase(name, 7, "rocket:", 7)) return true;
        if (nl >= 5 && detail::eqNoCase(name, 5, "dart:", 5)) return true;
    }
    return false;
}

// FNV-1a 32-bit over n (clamped to 0..kMaxRawItems) then each entries[i].key (0..n-1).
inline uint32_t orderSigEntries(const EntryView* entries, int n) {
    const int cnt = (!entries || n < 0) ? 0 : (n > kMaxRawItems ? kMaxRawItems : n);
    uint32_t h = 2166136261u;
    auto mix = [&h](uint32_t v) { for (int s = 0; s < 32; s += 8) { h ^= (v >> s) & 0xFFu; h *= 16777619u; } };
    mix((uint32_t)cnt);
    for (int i = 0; i < cnt; ++i) mix(entries[i].key);
    return h;
}

// Partition up to kMaxRawItems entries from engine group 2 (rawEntries, rawCount, rawNames) into Grenades (outGrenades, greNames)
// and Wrist Launcher Darts/Rockets (outRockets, rocNames), each individually capped at 16 entries.
inline void splitItemEntries(const EntryView* rawEntries, int rawCount, const char rawNames[][64],
                             GroupView* outGrenades, char greNames[][64],
                             GroupView* outRockets, char rocNames[][64]) {
    if (outGrenades) memset(outGrenades, 0, sizeof(*outGrenades));
    if (greNames) memset(greNames, 0, 16 * 64);
    if (outRockets) memset(outRockets, 0, sizeof(*outRockets));
    if (rocNames) memset(rocNames, 0, 16 * 64);
    if (!rawEntries || rawCount <= 0) return;
    const int n = rawCount > kMaxRawItems ? kMaxRawItems : rawCount;
    for (int i = 0; i < n; ++i) {
        const char* nm = rawNames ? rawNames[i] : nullptr;
        if (isRocketEntry(rawEntries[i].icon, nm)) {
            if (outRockets && outRockets->count < 16) {
                const int d = outRockets->count++;
                outRockets->e[d] = rawEntries[i];
                if (rocNames && nm) { strncpy(rocNames[d], nm, 63); rocNames[d][63] = '\0'; }
            }
        } else {
            if (outGrenades && outGrenades->count < 16) {
                const int d = outGrenades->count++;
                outGrenades->e[d] = rawEntries[i];
                if (greNames && nm) { strncpy(greNames[d], nm, 63); greNames[d][63] = '\0'; }
            }
        }
    }
}

// GroupView wrapper around splitItemEntries.
inline void splitItemGroup(const GroupView& rawG2, const char rawNames[][64],
                           GroupView* outGrenades, char greNames[][64],
                           GroupView* outRockets, char rocNames[][64]) {
    splitItemEntries(rawG2.e, clampCount(rawG2.count), rawNames, outGrenades, greNames, outRockets, rocNames);
}

// Optional [Profile] overrides for Category 3 (Pin3, Hide3, Sort3). Any unset field inherits Category 2 (Pin2, Hide2, Sort2).
struct Category3Profile {
    char pin[ProfMax][ProfName];
    int nPin;
    bool pinSet;
    char hide[ProfMax][ProfName];
    int nHide;
    bool hideSet;
    bool byCount;
    bool sortSet;
};

inline void defaultCategory3Profile(Category3Profile& p3) {
    memset(&p3, 0, sizeof p3);
    p3.byCount = true;
}

// Apply one INI line for category 3 (case-insensitive Pin3, Hide3, Sort3=count|engine). Leaves p3 unchanged on error.
inline bool parseCategory3ProfileLine(Category3Profile& p3, const char* key, const char* val) {
    if (!key) return false;
    if (detail::eqNoCase(key, "Pin3"))  { p3.nPin = parseNameList(val, p3.pin, ProfMax); p3.pinSet = true; return true; }
    if (detail::eqNoCase(key, "Hide3")) { p3.nHide = parseNameList(val, p3.hide, ProfMax); p3.hideSet = true; return true; }
    if (detail::eqNoCase(key, "Sort3")) {
        bool bc = p3.byCount;
        if (!detail::parseSortValue(val, &bc)) return false;
        p3.byCount = bc; p3.sortSet = true; return true;
    }
    return false;
}

// Serialize any explicitly set Category 3 overrides ("Pin3=...", "Hide3=...", "Sort3=count|engine") joined with '\n', no trailing newline, NUL-terminated.
// Returns the length written (0 when no overrides are set), or -1 (buf[0] = 0 when size > 0) if it does not fit.
inline int formatCategory3Profile(const Category3Profile& p3, char* buf, int size) {
    if (!buf || size <= 0) return -1;
    int len = 0; bool ok = true;
    auto put = [&](const char* s, size_t l) {
        if (!ok) return;
        if ((size_t)len + l + 1 > (size_t)size) { ok = false; return; }
        memcpy(buf + len, s, l); len += (int)l;
    };
    auto putS = [&](const char* s) { put(s, strlen(s)); };
    if (p3.pinSet) {
        putS("Pin3=");
        const int cnt = p3.nPin < 0 ? 0 : p3.nPin;
        for (int i = 0; i < cnt && i < ProfMax; ++i) { if (i) putS(","); putS(p3.pin[i]); }
    }
    if (p3.hideSet) {
        if (len > 0) putS("\n");
        putS("Hide3=");
        const int cnt = p3.nHide < 0 ? 0 : p3.nHide;
        for (int i = 0; i < cnt && i < ProfMax; ++i) { if (i) putS(","); putS(p3.hide[i]); }
    }
    if (p3.sortSet) {
        if (len > 0) putS("\n");
        putS("Sort3="); putS(p3.byCount ? "count" : "engine");
    }
    if (!ok) { buf[0] = '\0'; return -1; }
    buf[len] = '\0';
    return len;
}

// Order category 3 entries using base Profile group 2 overridden by any fields explicitly set in p3.
inline int orderCategory3(const GroupView& g, const char names[][64], const Profile& base, const Category3Profile& p3, int* ord, int maxOut) {
    Profile eff = base;
    if (p3.pinSet) {
        eff.nPin[2] = p3.nPin < 0 ? 0 : (p3.nPin > ProfMax ? ProfMax : p3.nPin);
        memcpy(eff.pin[2], p3.pin, sizeof(eff.pin[2]));
    }
    if (p3.hideSet) {
        eff.nHide[2] = p3.nHide < 0 ? 0 : (p3.nHide > ProfMax ? ProfMax : p3.nHide);
        memcpy(eff.hide[2], p3.hide, sizeof(eff.hide[2]));
    }
    if (p3.sortSet) eff.byCount[2] = p3.byCount;
    return orderGroup(g, names, eff, 2, ord, maxOut);
}

// ---- tooltip decision (WI-80 / BG33, HB-1c-03; pure, host-tested) ----
// One button's native tooltip decision per UI tick. Empty or null name: clear the native tip once and mark it unset (tipSet=false, never stored
// as a set tip) and cleared (tipClr=true), so repeat empty frames do not re-clear. Same key with a tip already set: keep (no native write).
// Otherwise: record the key and set. Callers reset tipSet and tipClr when the button is hidden.
// CH7a: the hotbar passes chtip::hash(tip text) as `key`, so a count/usable change on the same entry re-sets the tip.
enum class TipAction { Keep, Set, Clear };
template <typename A, typename B>
inline TipAction tooltipStep(bool& tipSet, bool& tipClr, A& tipKey, const B& key, const char* name) {
    if (!name || !name[0]) {
        tipSet = false;
        if (tipClr) return TipAction::Keep;
        tipClr = true;
        return TipAction::Clear;
    }
    tipClr = false;
    if (tipSet && tipKey == key) return TipAction::Keep;
    tipKey = key; tipSet = true;
    return TipAction::Set;
}

}  // namespace ch
