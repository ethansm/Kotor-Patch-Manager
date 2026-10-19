// Combat Hotbar -- pure preview model (CH6, doc 46). Builds the button/pill rectangles of a DEMO target menu by calling the SAME ch:: layout and
// ordering functions the live HUD uses (CombatLogic.h), so the options-page preview cannot drift from the real layout. No Win32/engine types.
//
// Virtual screen: the live HUD reads the vanilla target-menu rects from the engine; here they are the 1920x1080 production values
// (banner {760,40,400,20}, three vanilla button boxes unioned {760,64,400,56}, auto button = box0.w/2 = 64) scaled by sh/1080 and re-centred on
// sw/2, which is how the vanilla menu sits on a wider screen. Demo state is memory-only: it never reaches the ini or the live HUD.
#pragma once
#include "CombatLogic.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

namespace chprev {

constexpr int kMaxBtn = 96;         // = CombatHotbar.cpp MaxBtn: 3 groups * Cols max (4) * MaxRows max (8)
constexpr int kNumScreens = 4;
struct ScreenSize { int w, h; };
constexpr ScreenSize kScreens[kNumScreens] = { {1280, 720}, {1920, 1080}, {2560, 1080}, {3440, 1440} };
constexpr float kUnusableAlpha = 0.35f;   // CombatHotbar.cpp widAlpha(b.icon, e.usable ? 1.0f : 0.35f)
constexpr int kBannerUnits[4] = {760, 40, 400, 20}, kBoxesUnits[4] = {760, 64, 400, 56}, kBoxW = 128;   // 1080p production values

enum Unusable { UNUSABLE_NONE = 0, UNUSABLE_SOME = 1, UNUSABLE_ALL = 2 };

// What the gear modal edits (memory-only). count[g] 0..16 = entries in group g (0 attacks, 1 force powers, 2 items).
struct Demo {
    int count[3] = {4, 3, 2};
    int unusable = UNUSABLE_NONE;
    int screen = 1;   // index into kScreens
};

// The page's live row values plus the mod's other settings (ini defaults = CombatHotbar.cpp g_cfg).
struct Settings {
    int cols = 2, maxRows = 4;
    int btnPx = 0;   // 0 = auto
    int pad = ch::kDefaultPad, groupGap = ch::kDefaultGroupGap, capPad = ch::kDefaultCapPad, sidePad = ch::kDefaultSidePad;
    int hideEmpty = 1, colsTotal = 6;
};

struct Model {
    int sw = 0, sh = 0;
    ch::Rect banner = {0, 0, 0, 0}, boxesUnion = {0, 0, 0, 0};
    int per = 0;                     // cols * maxRows
    int btnPx = 0;                   // the value passed to the layout (auto resolved)
    ch::Rect rects[kMaxBtn];         // group g's k-th button at rects[g*per + k]
    int outN[3] = {0, 0, 0};
    ch::Rect cont[3];                // pill / column container per group (zero for a hidden group)
    bool usable[kMaxBtn];            // same indexing as rects
    uint32_t key[kMaxBtn];           // fake entry key (g*100 + original index), for stable per-slot icon picks
    float alpha[kMaxBtn];            // 1.0 usable, kUnusableAlpha not
    Model() { memset(rects, 0, sizeof rects); memset(cont, 0, sizeof cont); memset(usable, 0, sizeof usable); memset(key, 0, sizeof key); for (float& a : alpha) a = 1.f; }
};

inline int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline int scaled(int u, int sh) { return (u * sh + 540) / 1080; }

// Whether entry i of a group of n is unusable for the demo mode: NONE never, ALL always, SOME every odd entry.
inline bool entryUnusable(int mode, int i) { return mode == UNUSABLE_ALL || (mode == UNUSABLE_SOME && (i & 1)); }

// The screen's banner and boxes-union rects (production 1080p values scaled by height, centred on the width).
inline void screenRects(int sw, int sh, ch::Rect* banner, ch::Rect* boxes) {
    const int cx = sw / 2;
    banner->w = scaled(kBannerUnits[2], sh); banner->h = scaled(kBannerUnits[3], sh); banner->y = scaled(kBannerUnits[1], sh); banner->x = cx - banner->w / 2;
    boxes->w = scaled(kBoxesUnits[2], sh); boxes->h = scaled(kBoxesUnits[3], sh); boxes->y = scaled(kBoxesUnits[1], sh); boxes->x = cx - boxes->w / 2;
}

inline Model build(const Demo& demo, const Settings& s) {
    Model m;
    const ScreenSize sc = kScreens[clampi(demo.screen, 0, kNumScreens - 1)];
    m.sw = sc.w; m.sh = sc.h;
    ch::Rect box = {0, 0, 0, 0};
    screenRects(m.sw, m.sh, &m.banner, &box);
    m.boxesUnion = box;
    const int cols = clampi(s.cols, 1, 4), maxRows = clampi(s.maxRows, 1, 8);
    m.per = cols * maxRows;
    m.btnPx = s.btnPx > 0 ? s.btnPx : scaled(kBoxW, m.sh) / 2;

    // Fake group views -> the live ordering (default profile: items by stack count, descending) -> displayed counts.
    ch::GroupView gv[3]; char names[3][16][64]; ch::Profile prof; ch::defaultProfile(prof);
    int ord[3][16]; int pre[3];
    for (int g = 0; g < 3; ++g) {
        memset(&gv[g], 0, sizeof gv[g]); memset(names[g], 0, sizeof names[g]);
        gv[g].count = ch::clampCount(demo.count[g]);
        for (int i = 0; i < gv[g].count; ++i) {
            ch::EntryView& e = gv[g].e[i];
            e.key = static_cast<uint32_t>(g * 100 + i); e.owner = 1; e.usable = !entryUnusable(demo.unusable, i);
            e.count = g == 2 ? static_cast<uint32_t>(1 + (i * 7) % 5) : 0;
            snprintf(names[g][i], 64, "Demo %d-%d", g, i);
        }
        pre[g] = ch::orderGroup(gv[g], names[g], prof, g, ord[g], m.per > 16 ? 16 : m.per);
        if (pre[g] < 0) pre[g] = 0;
    }
    if (s.hideEmpty) ch::layoutBannerPills(m.banner, m.boxesUnion, pre, m.per, s.colsTotal, m.btnPx, s.pad, s.groupGap, s.capPad, s.sidePad, m.sw, m.sh, m.rects, m.outN, m.cont);
    else ch::layoutBannerGroups(m.banner, m.boxesUnion, pre, cols, maxRows, m.btnPx, s.pad, s.groupGap, s.capPad, s.sidePad, m.sw, m.sh, m.rects, m.outN, m.cont);
    for (int g = 0; g < 3; ++g)
        for (int k = 0; k < m.outN[g] && g * m.per + k < kMaxBtn; ++k) {
            const ch::EntryView& e = gv[g].e[ord[g][k]];
            const int idx = g * m.per + k;
            m.usable[idx] = e.usable; m.key[idx] = e.key; m.alpha[idx] = e.usable ? 1.f : kUnusableAlpha;
        }
    return m;
}

// ---- draw list (what the options-page preview puts on its label pool) ---------------------------------------------------------------------
// Each Prim is one label of the preview canvas (PreviewCanvas.h) in virtual-screen units. The label index is fixed per thing (so the z-order is stable):
// screen backdrop 0, name bar 1, health bar 2, pill of group g at 3+g, button k of group g at 6 + g*16 + k. One label per button (the entry icon; the
// live HUD adds a frame label under it, which the page's 80-item cap cannot afford). Entries are capped at 16 per group by the live ordering anyway.
enum PrimKind { PK_SCREEN = 0, PK_NAME, PK_HEALTH, PK_PILL, PK_BUTTON };
constexpr int kLblScreen = 0, kLblName = 1, kLblHealth = 2, kLblPill = 3, kLblBtn = 6, kDrawPerGroup = 16;
constexpr int kPoolSize = kLblBtn + 3 * kDrawPerGroup;   // 54 labels
constexpr int kMaxPrims = kPoolSize;
struct Prim { int label; int kind; ch::Rect r; int group; int slot; float alpha; };

// Fills out[] (cap kMaxPrims) and returns the count. The name bar is the top 60 % of the banner and the health bar the rest (the banner is the union
// of LBL_NAME and PB_HEALTH). Pills are the model's containers (hidden groups have none); buttons are the model's rects, first 16 per group.
inline int buildPrims(const Model& m, Prim* out, int cap) {
    int n = 0;
    auto add = [&](int label, int kind, ch::Rect r, int g, int slot, float a) { if (n < cap && r.w > 0 && r.h > 0) out[n++] = Prim{label, kind, r, g, slot, a}; };
    add(kLblScreen, PK_SCREEN, ch::Rect{0, 0, m.sw, m.sh}, -1, -1, 1.f);
    const int nameH = (m.banner.h * 60 + 50) / 100;
    add(kLblName, PK_NAME, ch::Rect{m.banner.x, m.banner.y, m.banner.w, nameH}, -1, -1, 1.f);
    add(kLblHealth, PK_HEALTH, ch::Rect{m.banner.x, m.banner.y + nameH, m.banner.w, m.banner.h - nameH}, -1, -1, 1.f);
    for (int g = 0; g < 3; ++g) {
        if (m.cont[g].w > 0 && m.cont[g].h > 0) add(kLblPill + g, PK_PILL, m.cont[g], g, -1, 0.95f);
        for (int k = 0; k < m.outN[g] && k < kDrawPerGroup && g * m.per + k < kMaxBtn; ++k) {
            const int idx = g * m.per + k;
            add(kLblBtn + g * kDrawPerGroup + k, PK_BUTTON, m.rects[idx], g, k, m.alpha[idx]);
        }
    }
    return n;
}

}  // namespace chprev
