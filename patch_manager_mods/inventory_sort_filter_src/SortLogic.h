// Inventory Sort & Filter -- pure logic (no Win32, no engine access) so it can be unit-tested on the host.
// Doc: patch_manager_mods/11_inventory_sort_filter.md
#pragma once
#include <algorithm>
#include <stdio.h>
#include <string.h>

namespace isf {

// ---- backpack row ordering ---------------------------------------------------------------------------
struct Entry {
    int item;       // server item pointer (opaque here)
    int id;         // server object id (item+0x14)
    int orig;       // index in the vanilla order (stable tie-break)
    bool isNew;     // item+0x2C8 bit 7
    bool usable;    // usableForCycle(): equippable by the shown character, or activatable
    int baseItem;   // item+0xC
    bool tint;      // tintRow(): equippable type the shown character cannot equip
    char name[64] = {};   // localised item name (CExoLocString at item+0x2C0), NUL-terminated; "" = unknown. Default member init keeps
                          // the 7-value aggregate inits warning-free (-Wmissing-field-initializers) and a default-built Entry empty.
};

// ASCII-only fold on unsigned char: locale-independent, cp1252 bytes >= 0x80 compare as raw bytes.
inline int foldCmp(const char* a, const char* b) {
    for (;; ++a, ++b) {
        unsigned char x = (unsigned char)*a, y = (unsigned char)*b;
        if (x >= 'A' && x <= 'Z') x += 32;
        if (y >= 'A' && y <= 'Z') y += 32;
        if (x != y) return x < y ? -1 : 1;
        if (!x) return 0;
    }
}

// New first, then usable, then by base item id (groups identical kinds), then localised name, then vanilla order.
// Names compare ASCII case-insensitively (foldCmp), so "" sorts before any name: equal names (all-empty groups included) fall to orig,
// and a failed lookup in a mixed group clusters first. Skipping the name key when either side is empty would make the order cyclic (UB in sort).
inline bool before(const Entry& a, const Entry& b) {
    if (a.isNew != b.isNew) return a.isNew;
    if (a.usable != b.usable) return a.usable;
    if (a.baseItem != b.baseItem) return a.baseItem < b.baseItem;
    int c = foldCmp(a.name, b.name);
    if (c != 0) return c < 0;
    return a.orig < b.orig;
}

inline void sortEntries(Entry* e, int n) { std::stable_sort(e, e + n, before); }

// Lazy tie names (WI-146). before() reads the name only inside a tie on (isNew, usable, baseItem), so only rows that share that key
// need one. Calls readName(row) for every row whose key is shared with another row (name cleared first) and clears name on the
// rest. Returns the call count. Index array is a fixed stack buffer; above kMaxTieRows every name is read (same order, just slower).
constexpr int kMaxTieRows = 1024;
inline bool tieKeyLess(const Entry& a, const Entry& b) {
    if (a.isNew != b.isNew) return b.isNew;
    if (a.usable != b.usable) return b.usable;
    return a.baseItem < b.baseItem;
}
inline bool tieKeySame(const Entry& a, const Entry& b) {
    return a.isNew == b.isNew && a.usable == b.usable && a.baseItem == b.baseItem;
}
template <typename F> inline int fillTiedNames(Entry* e, int n, F readName) {
    if (n <= 0) return 0;
    if (n > kMaxTieRows) {
        for (int i = 0; i < n; ++i) { e[i].name[0] = 0; readName(e[i]); }
        return n;
    }
    int idx[kMaxTieRows];
    for (int i = 0; i < n; ++i) idx[i] = i;
    std::sort(idx, idx + n, [e](int x, int y) { return tieKeyLess(e[x], e[y]); });
    int calls = 0;
    for (int i = 0; i < n;) {
        int j = i + 1;
        while (j < n && tieKeySame(e[idx[i]], e[idx[j]])) ++j;
        const bool tied = j - i >= 2;
        for (int k = i; k < j; ++k) {
            Entry& r = e[idx[k]];
            if (tied) { r.name[0] = 0; readName(r); ++calls; }
            else r.name[0] = 0;
        }
        i = j;
    }
    return calls;
}

// ---- filter cycle (re-clicking the active All tab) ---------------------------------------------------
// New is the engine's own mode 1. Usable and Upgradeable are our filters on top of engine mode 0 (All), applied
// to backpack rows in the populate hook; `Extra` says which one is active. (0.1.0 used engine mode 5 for Usable,
// whose predicate disagreed with the tint; 0.2.0 uses the same equip predicate for both.)
enum State { S_All = 0, S_New = 1, S_Usable = 2, S_Upgrade = 3, S_Count = 4 };
enum Extra { X_None = 0, X_Usable = 1, X_Upgrade = 2 };
constexpr int kEngineModeNew = 1;

// Maps the engine filter mode byte (GuiInGame+0xFC1) plus our extra filter to a cycle state.
// Modes that are not part of the cycle (weapon/armor/... tabs, engine mode 5) return false.
inline bool stateOf(int engineMode, Extra extra, State* s) {
    if (engineMode == 0) { *s = extra == X_Usable ? S_Usable : extra == X_Upgrade ? S_Upgrade : S_All; return true; }
    if (engineMode == kEngineModeNew) { *s = S_New; return true; }
    return false;
}

inline int engineModeOf(State s) { return s == S_New ? kEngineModeNew : 0; }
inline Extra extraOf(State s) { return s == S_Usable ? X_Usable : s == S_Upgrade ? X_Upgrade : X_None; }

inline State nextState(State s, unsigned enabledMask) {   // bit (1<<state); All is always allowed
    for (int i = 1; i <= S_Count; ++i) {
        State t = State((int(s) + i) % S_Count);
        if (t == S_All || (enabledMask & (1u << t))) return t;
    }
    return S_All;
}

inline const char* captionOf(State s) {
    switch (s) {
    case S_New: return "Filter: New";
    case S_Usable: return "Filter: Usable";
    case S_Upgrade: return "Filter: Upgradeable";
    default: return "Filter";
    }
}

// Caption with the visible-row count for filtered states (WI-151): All stays "Filter"; New/Usable/Upgradeable get " (rows)".
// kCaptionBytes is the caption buffer size: the longest text ("Filter: Upgradeable (1024)") is 25 chars + NUL. A small cap truncates, still NUL-terminated.
constexpr int kCaptionBytes = 32;
inline void captionWithCount(State s, int rows, char* out, int cap) {
    if (!out || cap <= 0) return;
    if (s == S_All) snprintf(out, cap, "%s", captionOf(s));
    else snprintf(out, cap, "%s (%d)", captionOf(s), rows);
}

// Click on a tab whose engine mode is `clicked`. Only a click on the All tab (mode 0) while the panel is in a
// cycle state advances it. Returns true when the click is consumed and fills *next.
inline bool onTabClick(int clicked, int engineMode, Extra extra, unsigned enabledMask, State* next) {
    State cur;
    if (clicked != 0 || !stateOf(engineMode, extra, &cur)) return false;
    State n = nextState(cur, enabledMask);
    if (n == cur) return false;
    *next = n;
    return true;
}

// ---- equip predicate (doc 11 "0.2.0 tint predicate") -------------------------------------------------
// The engine's equip test 0x5BD6A0 takes ONE slot bit (it dispatches on 0x10/0x20, 0x40000/0x80000, other).
// An item fits the shown character if any bit of its baseitem slot mask passes. Stops at the first pass.
template <typename F> inline bool anySlotBit(unsigned mask, F pass) {
    for (unsigned b = 1; b; b <<= 1)
        if ((mask & b) && pass(b)) return true;
    return false;
}
// baseitem+0xB4: 0 none, 1 / 2 = only for the gender whose stats+0xE0 value equals g1 / g2 (0x8AC330's test).
inline bool genderAllows(int req, int gender, int g1, int g2) {
    return req == 1 ? gender == g1 : req == 2 ? gender == g2 : true;
}
inline bool tintRow(bool equippable, bool canEquip) { return equippable && !canEquip; }
inline bool usableForCycle(bool canEquip, bool activatable) { return canEquip || activatable; }

inline bool ieq(const char* a, const char* b) {
    for (; *a && *b; ++a, ++b) {
        char x = (*a >= 'A' && *a <= 'Z') ? char(*a + 32) : *a, y = (*b >= 'A' && *b <= 'Z') ? char(*b + 32) : *b;
        if (x != y) return false;
    }
    return *a == *b;
}

// "new,usable,upgrade" -> mask (unknown names ignored, spaces around names ignored)
inline unsigned parseCycleStates(const char* s) {
    unsigned m = 0;
    char buf[128]; strncpy(buf, s ? s : "", sizeof buf - 1); buf[sizeof buf - 1] = 0;
    char* p = buf;
    for (;;) {
        char* e = p; while (*e && *e != ',') ++e;
        bool last = (*e == 0); *e = 0;
        char* a = p; while (*a == ' ' || *a == '\t') ++a;
        char* z = a + strlen(a); while (z > a && (z[-1] == ' ' || z[-1] == '\t')) *--z = 0;
        if (ieq(a, "new")) m |= 1u << S_New;
        else if (ieq(a, "usable")) m |= 1u << S_Usable;
        else if (ieq(a, "upgrade") || ieq(a, "upgradeable")) m |= 1u << S_Upgrade;
        if (last) break;
        p = e + 1;
    }
    return m;
}

// ---- open upgrade slot predicate (doc 11 R9) ---------------------------------------------------------
// type = GetUpgradeType(item): 0 none, 1 melee weapon, 2 ranged, 3 armor, 4 gloves/other.
// slots[i] = *(int*)(item+0x2F4+4*i); -1 means empty. Type 1 never fills slot index 1 (the upgrade screen skips it).
inline int upgradeSlotCount(int type) {
    switch (type) { case 1: return 6; case 2: return 3; case 3: return 3; case 4: return 2; default: return 0; }
}
inline bool hasOpenSlot(int type, const int* slots) {
    int n = upgradeSlotCount(type);
    for (int i = 0; i < n; ++i) {
        if (type == 1 && i == 1) continue;
        if (slots[i] == -1) return true;
    }
    return false;
}

} // namespace isf
