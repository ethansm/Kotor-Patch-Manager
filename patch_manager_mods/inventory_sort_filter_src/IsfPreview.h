// Inventory Sort & Filter -- pure options-page preview model (no Win32, no engine types). A mock backpack (kDemo) runs through the same
// isf:: filter, sort and strip helpers the live page uses (SortLogic.h), and buildPrims() lays it out as labels of a fixed pool on a
// 1000 x 700 virtual canvas. The glue draws each Prim on the preview canvas; the PK_ICON colour comes from categoryColor(rows[row].category).
// Demo state is memory-only: it never reaches the ini or the live page.
#pragma once
#include "SortLogic.h"
#include <stdio.h>

namespace isfprev {

struct Settings {
    isf::SortOrder order = isf::SO_Category;
    bool sortOn = true, tint = true, cycle = true, strip = true;
    unsigned mask = (1u << isf::S_New) | (1u << isf::S_Usable) | (1u << isf::S_Upgrade);
    int tick = 0;   // the preview filter advances one state per 90 ticks (3 s)
};

// One mock backpack row. category = SortLogic.h categoryOf(): 0 melee, 1 ranged, 2 armour, 3 other equippable, 4 upgrade,
// 5 consumable (activatable, not equippable), 6 misc/quest. baseItem values are demo ids (only their order matters), except the
// crystal, which uses the real isf::kBaseLightsaberCrystals so it agrees with categoryOf().
struct DemoItem { const char* name; unsigned char category; int baseItem; bool isNew, usable, tint, upgradeable; };
constexpr int kDemoCount = 8;
constexpr DemoItem kDemo[kDemoCount] = {
    {"Vibroblade",              0, 2,                            true,  true,  false, false},
    {"Blaster Pistol",          1, 8,                            false, true,  false, false},
    {"Heavy Battle Armor",      2, 40,                           false, false, true,  false},
    {"Stealth Field Generator", 5, 60,                           false, true,  false, false},
    {"Lightsaber Crystal",      4, isf::kBaseLightsaberCrystals, false, false, false, true},
    {"Medpac",                  5, 30,                           false, true,  false, false},
    {"Adrenal Strength",        5, 31,                           false, true,  false, false},
    {"Datapad",                 6, 70,                           false, false, false, false},
};

// Entry i of the demo backpack: orig = i (vanilla order, and the key that maps a sorted row back to kDemo for upg[]).
inline void fillEntry(int i, isf::Entry* e) {
    const DemoItem& d = kDemo[i];
    *e = isf::Entry{};
    e->item = i + 1;
    e->id = i + 1;
    e->orig = i;
    e->isNew = d.isNew;
    e->usable = d.usable;
    e->baseItem = d.baseItem;
    e->tint = d.tint;
    e->category = d.category;
    snprintf(e->name, sizeof e->name, "%s", d.name);
}

// Active preview state: [S_All, then each enabled state in kCycleOrder order] indexed by (tick / 90) % count. cycle off -> All only.
inline isf::State activeAt(const Settings& s) {
    if (!s.cycle) return isf::S_All;
    isf::State list[isf::S_Count] = {isf::S_All, isf::S_All, isf::S_All, isf::S_All};
    int n = 1;
    for (isf::State c : isf::kCycleOrder) if (s.mask & (1u << c)) list[n++] = c;
    const int t = s.tick > 0 ? s.tick : 0;
    return list[(t / 90) % n];
}

inline bool shownBy(isf::State st, const DemoItem& d) {
    switch (st) {
    case isf::S_New: return d.isNew;
    case isf::S_Usable: return d.usable;
    case isf::S_Upgrade: return d.upgradeable;
    default: return true;
    }
}

constexpr int kMaxRows = 7;   // rows the pane draws (3 pool labels per row)

// shown/offered mirror the live strip counts; rows[0..shown) are the visible entries in display order, upg[r] is row r's Upgradeable flag.
struct Model {
    isf::State active = isf::S_All;
    int shown = 0, offered = 0;
    isf::Entry rows[kDemoCount];
    bool upg[kDemoCount] = {};
    char line1[isf::kStripBytes] = {};
    char line2[isf::kStripBytes] = {};
    Settings s;
};

// Filter, sort (unless sortOn is off or the order is vanilla), then the strip lines. The sort moves Entries, so upg[] is re-derived
// from orig after sorting.
inline void build(const Settings& s, Model* m) {
    *m = Model{};
    m->s = s;
    m->active = activeAt(s);
    m->offered = kDemoCount;
    for (int i = 0; i < kDemoCount; ++i) {
        if (!shownBy(m->active, kDemo[i])) continue;
        fillEntry(i, &m->rows[m->shown]);
        ++m->shown;
    }
    if (s.sortOn && s.order != isf::SO_Vanilla) isf::sortEntries(m->rows, m->shown, s.order);
    for (int r = 0; r < m->shown; ++r) m->upg[r] = kDemo[m->rows[r].orig].upgradeable;
    isf::stripLine1(m->active, s.mask, m->line1, (int)sizeof m->line1);
    isf::stripLine2(s.order, s.sortOn, m->shown, m->offered, m->line2, (int)sizeof m->line2);
}

// ---- draw list (what the options-page preview puts on its label pool) ---------------------------------------------------------------------
// Each Prim is one label of the preview canvas in virtual units (1000 x 700). Label indices are fixed per thing so z-order is stable:
// pane backdrop 0, strip frame 1, strip line 1 at 2, strip line 2 at 3, row r at 4+3r (frame), 5+3r (icon), 6+3r (name), caption last.
enum PrimKind { PK_BG, PK_STRIP, PK_STRIP_TEXT1, PK_STRIP_TEXT2, PK_ROW, PK_ICON, PK_NAME, PK_CAPTION };
struct Prim { int label; int kind; int x, y, w, h; int row; bool tinted; const char* text; };

constexpr int kLblBg = 0, kLblStrip = 1, kLblStripText1 = 2, kLblStripText2 = 3, kLblRow0 = 4;
constexpr int kLblCaption = kLblRow0 + 3 * kMaxRows;   // 25
constexpr int kPoolSize = kLblCaption + 1;             // 26 labels
constexpr int kScreenW = 1000, kScreenH = 700;
constexpr int kStripX = 40, kStripY = 40, kStripW = 920, kStripH = 56;
constexpr int kRowsTopNoStrip = 40, kRowsTopStrip = 112, kRowStride = 72, kRowH = 64;
constexpr int kCaptionY = 660;

// Icon colour per sort category (0..6; anything else reads as misc). Muted so the name text stays readable on the pane.
inline void categoryColor(int category, float rgb[3]) {
    static const float kCat[7][3] = {
        {0.55f, 0.70f, 0.85f},   // 0 melee: steel blue
        {0.85f, 0.65f, 0.35f},   // 1 ranged: copper
        {0.45f, 0.62f, 0.45f},   // 2 armour: green
        {0.62f, 0.52f, 0.78f},   // 3 other equippable: violet
        {0.95f, 0.85f, 0.30f},   // 4 upgrades: gold
        {0.40f, 0.80f, 0.80f},   // 5 consumables: teal
        {0.65f, 0.65f, 0.65f},   // 6 misc: grey
    };
    const int c = (category >= 0 && category < 7) ? category : 6;
    for (int i = 0; i < 3; ++i) rgb[i] = kCat[c][i];
}

// Fills out[] (cap >= kPoolSize) and returns the count. Strip prims only when s.strip; rows start below the strip (or at the top);
// at most kMaxRows rows; the PK_NAME text is tinted only when s.tint and the row is an equippable type the character cannot equip.
// Text pointers reference m's own buffers, so m must outlive the Prim list.
inline int buildPrims(const Model& m, Prim* out, int cap) {
    int n = 0;
    auto add = [&](int label, int kind, int x, int y, int w, int h, int row, bool tinted, const char* text) {
        if (n < cap) out[n++] = Prim{label, kind, x, y, w, h, row, tinted, text};
    };
    add(kLblBg, PK_BG, 0, 0, kScreenW, kScreenH, -1, false, nullptr);
    int top = kRowsTopNoStrip;
    if (m.s.strip) {
        add(kLblStrip, PK_STRIP, kStripX, kStripY, kStripW, kStripH, -1, false, nullptr);
        add(kLblStripText1, PK_STRIP_TEXT1, kStripX + 16, kStripY + 4, kStripW - 32, 24, -1, false, m.line1);
        add(kLblStripText2, PK_STRIP_TEXT2, kStripX + 16, kStripY + 28, kStripW - 32, 20, -1, false, m.line2);
        top = kRowsTopStrip;
    }
    const int rows = m.shown < kMaxRows ? m.shown : kMaxRows;
    for (int r = 0; r < rows; ++r) {
        const int y = top + r * kRowStride;
        const int lbl = kLblRow0 + 3 * r;
        add(lbl, PK_ROW, 40, y, 920, kRowH, r, false, nullptr);
        add(lbl + 1, PK_ICON, 56, y + 12, 40, 40, r, false, nullptr);
        add(lbl + 2, PK_NAME, 112, y + 12, 800, 40, r, m.s.tint && m.rows[r].tint, m.rows[r].name);
    }
    add(kLblCaption, PK_CAPTION, 40, kCaptionY, 920, 24, -1, false, "Preview: filter cycles every 3 s");
    return n;
}

}  // namespace isfprev
