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
    unsigned char category = 6;   // categoryOf() (WI-154); 6 = misc. Last member, so the 7-value aggregate inits still compile.
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

// [Inventory] sort_order (WI-154). The values double as the WI-155 mod-config list index (0..3).
//   SO_Category: isNew, usable, category, baseItem, name, orig     SO_Base: before() (isNew, usable, baseItem, name, orig)
//   SO_Name:     isNew, usable, name, orig                         SO_Vanilla: no sort (the buffer is already in vanilla order)
enum SortOrder : unsigned char { SO_Category = 0, SO_Base = 1, SO_Name = 2, SO_Vanilla = 3 };
inline bool beforeBy(SortOrder o, const Entry& a, const Entry& b) {
    if (o == SO_Vanilla) return a.orig < b.orig;
    if (a.isNew != b.isNew) return a.isNew;
    if (a.usable != b.usable) return a.usable;
    if (o == SO_Category && a.category != b.category) return a.category < b.category;
    if (o != SO_Name && a.baseItem != b.baseItem) return a.baseItem < b.baseItem;
    int c = foldCmp(a.name, b.name);
    return c != 0 ? c < 0 : a.orig < b.orig;
}
inline void sortEntries(Entry* e, int n, SortOrder o) {
    if (o == SO_Vanilla) return;   // already in vanilla order (the buffer is filled in loop order)
    std::stable_sort(e, e + n, [o](const Entry& a, const Entry& b) { return beforeBy(o, a, b); });
}

// Lazy tie names (WI-146, per order WI-154). beforeBy() reads the name only inside a tie on every key before it, so only rows that
// share the order's tie key need one: base (isNew, usable, baseItem), category (isNew, usable, category, baseItem), name (isNew, usable);
// vanilla reads none. Calls readName(row) for every row whose key is shared with another row (name cleared first) and clears name on the
// rest. Returns the call count. Index array is a fixed stack buffer; above kMaxTieRows every name is read (same order, just slower), except
// vanilla, which never reads.
constexpr int kMaxTieRows = 1024;
inline bool tieKeyLess(const Entry& a, const Entry& b, SortOrder o = SO_Base) {
    if (a.isNew != b.isNew) return b.isNew;
    if (a.usable != b.usable) return b.usable;
    if (o == SO_Category && a.category != b.category) return a.category < b.category;
    if (o == SO_Name) return false;
    return a.baseItem < b.baseItem;
}
inline bool tieKeySame(const Entry& a, const Entry& b, SortOrder o = SO_Base) {
    return !tieKeyLess(a, b, o) && !tieKeyLess(b, a, o);
}
template <typename F> inline int fillTiedNames(Entry* e, int n, F readName, SortOrder o = SO_Base) {
    if (n <= 0) return 0;
    if (o == SO_Vanilla) { for (int i = 0; i < n; ++i) e[i].name[0] = 0; return 0; }
    if (n > kMaxTieRows) {
        for (int i = 0; i < n; ++i) { e[i].name[0] = 0; readName(e[i]); }
        return n;
    }
    int idx[kMaxTieRows];
    for (int i = 0; i < n; ++i) idx[i] = i;
    std::sort(idx, idx + n, [e, o](int x, int y) { return tieKeyLess(e[x], e[y], o); });
    int calls = 0;
    for (int i = 0; i < n;) {
        int j = i + 1;
        while (j < n && tieKeySame(e[idx[i]], e[idx[j]], o)) ++j;
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

// Caption for a filtered state (WI-151 count, WI-152 position). All stays "Filter". Otherwise "Filter: <name>", then " P/N"
// (1-based place among the ENABLED non-All states in cycle order New, Usable, Upgradeable; shown only when N >= 2), then
// " (rows)" when rows >= 0. rows < 0 omits the count (captionCountTo adds it once g_n is final). A state outside the mask
// gets no place; an empty mask writes plain "Filter". The longest text "Filter: Upgradeable 3/3 (1024)" is 30 chars;
// kCaptionBytes = 32 holds it with its NUL. A small cap truncates, still NUL-terminated.
constexpr int kCaptionBytes = 32;
constexpr State kCycleOrder[] = {S_New, S_Usable, S_Upgrade};
inline void captionWithCount(State s, unsigned enabledMask, int rows, char* out, int cap) {
    if (!out || cap <= 0) return;
    int total = 0, place = 0;
    for (State c : kCycleOrder) {
        if (!(enabledMask & (1u << c))) continue;
        if (c == s) place = total + 1;
        ++total;
    }
    if (s == S_All || total == 0) { snprintf(out, cap, "Filter"); return; }
    char pos[32] = "", cnt[24] = "";
    if (place > 0 && total > 1) snprintf(pos, sizeof pos, " %d/%d", place, total);
    if (rows >= 0) snprintf(cnt, sizeof cnt, " (%d)", rows);
    snprintf(out, cap, "%s%s%s", captionOf(s), pos, cnt);
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

// ---- controller bumper ring (WI-153) -----------------------------------------------------------------
// The engine steps the filter mode inline on the bumpers (0->6->7..->11->0, reverse for prev; New=1 is not in it). With the cycle
// on, our states join that ring after All: All > New > Usable > Upgradeable > 6 > .. > 11 > All (prev is the reverse). Slots 0..3
// are the State values (mask bit 1u << slot), slots 4..9 are engine modes 6..11 (always visited, no mask bit).
// Pass-through (result = the engine's computed mode, extra None, i.e. the vanilla ring exactly): cycle off, dir 0, none of
// New/Usable/Upgradeable enabled in the mask, or a current mode outside the ring (2..5, unknown 0xFF).
struct Bump { int mode; Extra extra; };
constexpr int kRingLen = 10;
inline Bump bumperNext(bool cycle, int curMode, Extra curExtra, int computedMode, unsigned enabledMask, int dir) {
    const Bump engine = {computedMode, X_None};
    constexpr unsigned kOurs = (1u << S_New) | (1u << S_Usable) | (1u << S_Upgrade);
    if (!cycle || dir == 0 || !(enabledMask & kOurs)) return engine;
    State s;
    int p;
    if (curMode >= 6 && curMode <= 11) p = curMode - 2;
    else if (stateOf(curMode, curExtra, &s)) p = int(s);
    else return engine;
    for (int i = 1; i < kRingLen; ++i) {
        const int q = ((p + dir * i) % kRingLen + kRingLen) % kRingLen;
        if (q == S_All) return Bump{0, X_None};
        if (q >= S_Count) return Bump{q + 2, X_None};   // slots 4..9 = engine modes 6..11
        if (enabledMask & (1u << q)) return Bump{engineModeOf(State(q)), extraOf(State(q))};
    }
    return Bump{0, X_None};   // unreachable: the walk reaches slot 0 (All) within kRingLen steps
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

// ---- sort category (WI-154, owner-approved order) ----------------------------------------------------
// 0 melee weapons, 1 ranged weapons, 2 armour/robes, 3 other equippable (head, hands, arms, belt, implant), 4 upgrades,
// 5 consumables (activatable, not equippable: medpacs, stims, grenades), 6 misc/quest.
// upgradeType = GetUpgradeType(item) (0 none, 1 melee, 2 ranged, 3 armor, 4 gloves/other); slotMask = baseitem+4 (0 = not equippable).
// Upgrade items are the baseitems rows 11 Lightsaber_Crystals and 101 Upgrade_Items (K2 baseitems.2da dump
// .claude/research/k1_items/baseitems_k2.tsv; also R8_item_model_survey_2026-09-29.txt). In items_k2.tsv every row with baseitem 101 is an
// upgrade (60 rows) and rows with baseitem 11 are crystals. Neither has equip slots and GetUpgradeType is 0 for them.
constexpr int kBaseLightsaberCrystals = 11, kBaseUpgradeItems = 101;
inline unsigned char categoryOf(unsigned upgradeType, unsigned slotMask, bool activatable, int baseItem) {
    if (upgradeType == 1) return 0;
    if (upgradeType == 2) return 1;
    if (upgradeType == 3) return 2;
    if (upgradeType == 4 || slotMask) return 3;
    if (baseItem == kBaseLightsaberCrystals || baseItem == kBaseUpgradeItems) return 4;
    return activatable ? 5 : 6;
}

inline bool ieq(const char* a, const char* b) {
    for (; *a && *b; ++a, ++b) {
        char x = (*a >= 'A' && *a <= 'Z') ? char(*a + 32) : *a, y = (*b >= 'A' && *b <= 'Z') ? char(*b + 32) : *b;
        if (x != y) return false;
    }
    return *a == *b;
}

// [Inventory] sort_order: category|base|name|vanilla (ASCII case-insensitive, spaces around ignored) or the digits 0..3 in that order
// (WI-155's mod-config list row stores the index). Unknown, empty or null -> category.
inline SortOrder parseSortOrder(const char* s) {
    char buf[32]; strncpy(buf, s ? s : "", sizeof buf - 1); buf[sizeof buf - 1] = 0;
    char* a = buf; while (*a == ' ' || *a == '\t') ++a;
    char* z = a + strlen(a); while (z > a && (z[-1] == ' ' || z[-1] == '\t')) *--z = 0;
    if (ieq(a, "1") || ieq(a, "base")) return SO_Base;
    if (ieq(a, "2") || ieq(a, "name")) return SO_Name;
    if (ieq(a, "3") || ieq(a, "vanilla")) return SO_Vanilla;
    return SO_Category;
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

// ---- strip above LB_ITEMS (doc 11 VU4) ----------------------------------------------------------------
// Line 1 lists the cycle states: All first, then the enabled ones in cycle order, the active one in brackets
// ("[All]  New  Can Equip  Upgradeable"). Line 2 is the sort order plus the count ("Sort: Category    13 of 22 shown",
// or "22 items" when nothing is hidden). Both truncate at cap, always NUL-terminated. kStripBytes holds the longest line.
constexpr int kStripBytes = 96;
inline const char* stripNameOf(State s) {
    switch (s) {
    case S_New: return "New";
    case S_Usable: return "Can Equip";
    case S_Upgrade: return "Upgradeable";
    default: return "All";
    }
}
inline void stripLine1(State active, unsigned enabledMask, char* out, int cap) {
    if (!out || cap <= 0) return;
    char buf[kStripBytes] = "";
    int n = 0;
    auto seg = [&](State s) {
        if (n > 0) n += snprintf(buf + n, sizeof buf - n, "  ");
        if (n < (int)sizeof buf) n += snprintf(buf + n, sizeof buf - n, s == active ? "[%s]" : "%s", stripNameOf(s));
        if (n > (int)sizeof buf - 1) n = (int)sizeof buf - 1;
    };
    seg(S_All);
    for (State c : kCycleOrder) if (enabledMask & (1u << c)) seg(c);
    snprintf(out, cap, "%s", buf);
}
inline const char* sortNameOf(SortOrder o) {
    switch (o) {
    case SO_Base: return "Base item";
    case SO_Name: return "Name";
    case SO_Vanilla: return "Vanilla";
    default: return "Category";
    }
}
inline void stripLine2(SortOrder order, bool sortOn, int shown, int offered, char* out, int cap) {
    if (!out || cap <= 0) return;
    const char* name = sortOn ? sortNameOf(order) : "Vanilla";
    if (shown == offered) snprintf(out, cap, "Sort: %s    %d items", name, offered);
    else snprintf(out, cap, "Sort: %s    %d of %d shown", name, shown, offered);
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
