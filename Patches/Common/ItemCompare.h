// ItemCompare.h -- pure (engine-free, heap-free, STL-free) equip-compare formatter: stat deltas, band A (2 x 30 chars) and the equip block.
// Single shared header in Common.
// Row templates: .claude/waves/ISF-EQBOX-2/u09.md. Spec background: .claude/research/equip_compare/phase2/SPEC.md.
// Sides: PropDiff A = EQUIPPED item, B = VIEWED item. Numeric deltas read "(+N)"/"(-N)" (since 783d3698); whether the engine recolors the +/- in game is unverified.
// Contract: every band and block row is <= 30 chars. Names are read bounded. PropRow::text must be NUL-terminated (ItemPropDiff.h strcmp).
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ItemPropDiff.h"

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"   // every snprintf here truncates on purpose into a bounded buffer
#endif

namespace itemcmp {

constexpr unsigned kBlockMaxLines = 6;   // block lines including the header
constexpr unsigned kRowW = 30;           // band rows (CompareOut.band is char[2][32]), in chars (u09 budget)
constexpr unsigned kBlockW = 48;         // block rows (char[64]); the pixel fit in cmpDraw is the final cut
constexpr int kUnk = -1000;              // numeric sentinel: a failed read (a readable zero is a real value)

// Value bounds. A value outside its bound is treated as unknown (or as "no row"), never printed, so every row fits kRowW.
constexpr int kDmgMaxDice = 9;           // damage "lo-hi": lo = numDice <= 9
constexpr int kDmgMaxHi = 99;            // hi = numDice * die <= 99
constexpr int kAcMax = 99;               // base defense
constexpr int kDexMax = 99;              // max dex bonus (-1 = no cap)
constexpr int kCritMultMax = 9;          // crit multiplier x1..x9

enum class SlotState { Empty, Item, Unreadable };

// Why compare() returned the way it did. Log/diagnostics only: no output depends on it.
// Unscoped with an unsigned base so CompareOut::why stays a plain unsigned.
enum CompareWhy : unsigned { kWhyOk = 0, kWhySlotUnreadable = 1, kWhyNoRows = 2, kWhyNotWeaponOrArmor = 3, kWhyOther = 4, kWhyViewNotOk = 5 };
inline const char* whyName(unsigned w) {
    switch (w) {
        case kWhyOk: return "ok";
        case kWhySlotUnreadable: return "slotUnreadable";
        case kWhyNoRows: return "noRows";
        case kWhyNotWeaponOrArmor: return "notWeaponOrArmor";
        case kWhyViewNotOk: return "viewNotOk";
        default: return "other";
    }
}

struct ItemFacts {
    bool ok = false;
    uint32_t id = 0;                     // server id (0 = unknown); only used to tell the same item in both hands apart
    char name[64] = {};
    unsigned slotMask = 0;
    int numDice = kUnk, die = kUnk, critThreat = kUnk, critMult = kUnk, baseAC = kUnk, maxDex = kUnk;   // maxDex signed (-1 = none)
    bool keen = false;
    int critBonus = 0;                   // Improved Critical count m
    PropRow props[PROP_DIFF_MAX] = {};
    unsigned nProps = 0, dropped = 0;    // nProps above PROP_DIFF_MAX is clamped; dropped = rows the adapter could not send
};

struct SlotRead { SlotState state = SlotState::Unreadable; ItemFacts item; };

struct CompareInput {
    ItemFacts viewed;
    SlotRead right, left, armor, head, gauntlet, implant, belt;
    bool canEquip = true;
};

struct CompareOut {
    char band[2][32] = {};
    unsigned nBand = 0;
    char block[kBlockMaxLines][64] = {};
    unsigned nBlock = 0;
    char viewed[64] = {};       // the viewed item's name (the title row of the box is built from it)
    unsigned why = kWhyOther;   // CompareWhy; defaults to other so a path that forgets to set it never reads as ok
};

namespace cmpd {

constexpr unsigned kRowMax = 64;

struct Row { char b[64]; char k[128]; unsigned w; };    // band text, block text, hidden-row weight (for "N more")

inline void cut(char* dst, unsigned dstSize, const char* src, unsigned maxChars) {
    unsigned n = (unsigned)strlen(src);
    if (n > maxChars) n = maxChars;
    if (n > dstSize - 1) n = dstSize - 1;
    memcpy(dst, src, n);
    dst[n] = 0;
}

inline bool asciiAlnum(char c) { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

// Length of s, reading at most max bytes (safe on a field with no terminator).
inline unsigned blen(const char* s, unsigned max) { unsigned n = 0; while (n < max && s[n]) ++n; return n; }

// Item-property ids that are equip prerequisites, not stat changes (itempropdef.2da rows, read from the game install).
inline bool isPrereqType(uint16_t t) { return t == 43 || t == 44 || t == 45 || t == 52 || t == 57 || t == 60 || t == 61 || t == 62 || t == 69; }
// True when the two prerequisite sets differ (count, or any (type, subtype, text) of A missing from B).
inline bool prereqDiffer(const PropRow* a, unsigned na, const PropRow* b, unsigned nb) {
    if (na != nb) return true;
    for (unsigned i = 0; i < na; i++) {
        bool hit = false;
        for (unsigned j = 0; j < nb && !hit; j++) hit = a[i].type == b[j].type && a[i].subtype == b[j].subtype && strcmp(a[i].text, b[j].text) == 0;
        if (!hit) return true;
    }
    return false;
}

// "Attribute Restriction: Constitution 12" -> "Constitution 12": the label before ": " is dropped when it names a restriction/requirement/limitation (the row already says "Requires:").
inline const char* reqBody(const char* t) {
    const char* c = strstr(t, ": ");
    if (!c) return t;
    const unsigned n = (unsigned)(c - t);
    static const char* const kw[] = { "Restriction", "Requirement", "Limitation" };
    for (const char* k : kw) { const unsigned kl = (unsigned)strlen(k); if (n >= kl && !strncmp(t + n - kl, k, kl)) return c + 2; }
    return t;
}

// Word-boundary fit (S1 rule): the whole text if <= budget; else cut at the last space after the label (the label's own
// space is never a break), strip trailing non-alnum, append "...". Result <= budget chars. dst must hold budget+1.
inline void fit(char* dst, unsigned sz, const char* text, unsigned budget) {
    unsigned len = (unsigned)strlen(text);
    if (len <= budget || budget < 3) { cut(dst, sz, text, budget); return; }   // budget < 3 cannot hold "..."
    unsigned cap = budget - 3;
    unsigned sep = 0; while (text[sep] && text[sep] != ' ') sep++;   // label separator: first space
    unsigned p = cap; while (p > 0 && text[p] != ' ') p--;
    unsigned n = p > sep ? p : cap;
    while (n > 0 && !asciiAlnum(text[n - 1])) n--;
    if (n == 0) n = cap;
    snprintf(dst, sz, "%.*s...", (int)n, text);
}

inline void setRow(Row& r, const char* bandText, const char* blockText) {
    cut(r.b, sizeof r.b, bandText, 63);
    cut(r.k, sizeof r.k, blockText, 127);
    r.w = 1;
}

// "<label> = <v>" when both sides known and equal. "<label> <e> -> <v> (up|down <d>)" when both known and d given.
// Otherwise "<label> <e> -> <v> (changed)". An unknown side prints "--".
inline void fmtDelta(char* dst, unsigned sz, const char* label, bool haveE, const char* e, bool haveV, const char* v,
                     bool equal, bool up, const char* d) {
    if (haveE && haveV && equal) { snprintf(dst, sz, "%s = %s", label, v); return; }
    const char* es = haveE ? e : "--";
    const char* vs = haveV ? v : "--";
    if (haveE && haveV && d && d[0]) snprintf(dst, sz, "%s %s -> %s (%s%s)", label, es, vs, up ? "+" : "-", d);
    else snprintf(dst, sz, "%s %s -> %s (changed)", label, es, vs);
}

// Damage side: lo = numDice, hi = numDice * die.
// kind: 1 known range, 0 unknown, -1 readable zero (0 damage: empty hand, shield), -2 malformed (negative or out of bounds).
struct DmgSide { int kind; int lo; int hi; };
inline DmgSide dmgOf(int dice, int die) {
    DmgSide r = { 0, 0, 0 };
    if (dice == kUnk || die == kUnk) return r;
    if (dice < 0 || die < 0) { r.kind = -2; return r; }       // a negative count is a bad read, not a zero
    if (dice == 0 || die == 0) { r.kind = -1; return r; }
    if (dice > kDmgMaxDice) { r.kind = -2; return r; }
    const long long hi = (long long)dice * (long long)die;
    if (hi > kDmgMaxHi) { r.kind = -2; return r; }
    r.kind = 1; r.lo = dice; r.hi = (int)hi;
    return r;
}

// Weapon damage, range only: "Damage 2-20 -> 4-16 (-)". Direction = midpoint (lo+hi) compare. Same range "= 2-20".
// Same midpoint, other range "(changed)". Widest possible row: "Damage 9-99 -> 9-99 (changed)" is 29 chars.
inline void addDamage(Row* rows, unsigned& n, const ItemFacts& v, const SlotRead& s) {
    const DmgSide vd = dmgOf(v.numDice, v.die);
    if (vd.kind < 0) return;                          // viewed readable zero or malformed: no row
    DmgSide ed = { -1, 0, 0 };                        // empty hand = 0
    if (s.state != SlotState::Empty) {
        ed = dmgOf(s.item.numDice, s.item.die);
        if (ed.kind == -2) ed = { 0, 0, 0 };          // malformed equipped value: unknown, never a delta
    }
    const bool vk = vd.kind != 0, ek = ed.kind != 0;
    if (!vk && !ek) return;
    char vs[8] = "--", es[8] = "--";
    if (vk) { if (vd.kind == 1) snprintf(vs, sizeof vs, "%d-%d", vd.lo, vd.hi); else snprintf(vs, sizeof vs, "0"); }
    if (ek) { if (ed.kind == 1) snprintf(es, sizeof es, "%d-%d", ed.lo, ed.hi); else snprintf(es, sizeof es, "0"); }
    const int vMid = vd.lo + vd.hi, eMid = ed.lo + ed.hi;
    char t[64];
    if (vk && ek && vd.lo == ed.lo && vd.hi == ed.hi) snprintf(t, sizeof t, "Damage = %s", vs);
    else if (vk && ek && vMid != eMid) snprintf(t, sizeof t, "Damage %s -> %s (%s)", es, vs, vMid > eMid ? "+" : "-");
    else snprintf(t, sizeof t, "Damage %s -> %s (changed)", es, vs);
    setRow(rows[n++], t, t);
}

// returns 1 known (minRoll set), 0 unknown, -1 omit
inline int critOf(const ItemFacts& f, int* minRoll) {
    if (f.critThreat == kUnk) return 0;
    const int w = f.critThreat;
    if (w <= 0) return -1;
    if (f.critBonus == kUnk) return -1;   // keen / Improved Critical source unknown: a wrong range is worse than no row
    const int m = f.critBonus < 0 ? 0 : f.critBonus;
    long long threat = (long long)w + (f.keen ? (long long)w : 0) + (long long)m * w;
    long long mr = 21 - threat;
    if (mr < 1) mr = 1;
    if (mr > 20) mr = 20;
    *minRoll = (int)mr;
    return 1;
}

inline void critStr(char* dst, unsigned sz, int minRoll, int mult) {
    char core[16];
    if (minRoll == 20) snprintf(core, sizeof core, "20");
    else snprintf(core, sizeof core, "%d-20", minRoll);
    if (mult > 0) snprintf(dst, sz, "%s x%d", core, mult);
    else snprintf(dst, sz, "%s", core);
}

inline int multOf(int m) { return (m >= 1 && m <= kCritMultMax) ? m : 0; }   // 0 = absent or unknown: never printed

// Crit row: "Threat 20 -> 19-20 (up 1)". Multipliers ride along only when the row still fits kRowW.
inline void addCrit(Row* rows, unsigned& n, const ItemFacts& v, const SlotRead& s) {
    if (s.state == SlotState::Unreadable) return;
    auto weapon = [](const ItemFacts& f) { return f.numDice > 0 || f.die > 0; };   // shields / non-weapons read 0d0
    if (!weapon(v)) return;
    if (s.state == SlotState::Item && !weapon(s.item)) return;   // shield in hand carries no crit comparison (R3)
    int vm = 0, em = 0;
    if (critOf(v, &vm) <= 0) return;                  // viewed threat unknown or omitted: no row (D9)
    const bool itemSide = s.state == SlotState::Item;
    const int vMult = multOf(v.critMult);
    const int eMult = itemSide ? multOf(s.item.critMult) : 0;
    int ef = 0;                                       // empty side = 0 faces, shown "0"
    if (itemSide) {
        const int ek = critOf(s.item, &em);
        if (ek < 0) return;
        if (ek == 0) {                                // equipped threat unknown: viewed side only
            char vs[24], t[40];
            critStr(vs, sizeof vs, vm, 0);
            snprintf(t, sizeof t, "Crit Range -- -> %s", vs);
            setRow(rows[n++], t, t);
            return;
        }
        ef = 21 - em;
    }
    const int vf = 21 - vm;
    const bool multDiff = itemSide && vMult > 0 && eMult > 0 && vMult != eMult;
    const bool facesDiff = vf != ef;
    if (!facesDiff && !multDiff) return;              // equal threat and multiplier: omitted
    if (!facesDiff) {                                 // multiplier-only row keeps the "Crit mult" label (D2)
        char t[40];
        snprintf(t, sizeof t, "Crit mult x%d -> x%d (%s)", eMult, vMult, vMult > eMult ? "+" : "-");
        setRow(rows[n++], t, t);
        return;
    }
    const bool up = vf > ef;
    const unsigned d = up ? (unsigned)(vf - ef) : (unsigned)(ef - vf);
    char es[24], vs[24], es2[24], vs2[24], full[96], brief[96];
    if (itemSide) { critStr(es, sizeof es, em, multDiff ? eMult : 0); critStr(es2, sizeof es2, em, 0); }
    else { snprintf(es, sizeof es, "0"); snprintf(es2, sizeof es2, "0"); }
    critStr(vs, sizeof vs, vm, multDiff ? vMult : 0);
    critStr(vs2, sizeof vs2, vm, 0);
    snprintf(full, sizeof full, "Crit Range %s -> %s (%s%u)", es, vs, up ? "+" : "-", d);
    snprintf(brief, sizeof brief, "Crit Range %s -> %s (%s%u)", es2, vs2, up ? "+" : "-", d);
    const char* pick = strlen(full) <= kRowW ? full : brief;   // brief (no multipliers) is <= 29 chars
    setRow(rows[n++], pick, pick);
}

inline int acOf(int x) { return (x == kUnk || (x >= 0 && x <= kAcMax)) ? x : kUnk; }        // out of range = unknown
inline int dexOf(int x) { return (x == kUnk || x == -1 || (x >= 0 && x <= kDexMax)) ? x : kUnk; }   // -1 = no cap

inline void tokDex(char* d, unsigned sz, int x) {
    if (x == kUnk) snprintf(d, sz, "--");
    else if (x < 0) snprintf(d, sz, "none");
    else snprintf(d, sz, "%d", x);
}

inline void addArmor(Row* rows, unsigned& n, const ItemFacts& v, const SlotRead& s) {
    const int vac = acOf(v.baseAC), vdx = dexOf(v.maxDex);
    int eac = 0, edx = -1;                            // empty slot: no defense, no cap
    if (s.state != SlotState::Empty) { eac = acOf(s.item.baseAC); edx = dexOf(s.item.maxDex); }
    char t[96], es[24], vs[24], ds[24];
    if (!(vac == kUnk && eac == kUnk)) {
        const bool vk = vac != kUnk, ek = eac != kUnk, both = vk && ek, eq = both && vac == eac;
        const int d = (both && !eq) ? (vac > eac ? vac - eac : eac - vac) : 0;
        snprintf(es, sizeof es, "%d", eac); snprintf(vs, sizeof vs, "%d", vac); snprintf(ds, sizeof ds, "%d", d);
        fmtDelta(t, sizeof t, "Base Defense", ek, es, vk, vs, eq, vac >= eac, (both && !eq) ? ds : "");
        setRow(rows[n++], t, t);
    }
    if ((vdx != kUnk || edx != kUnk) && s.state != SlotState::Empty) {   // an empty slot has no cap to compare ("none" would read as a loss)
        if (!(vdx == -1 && edx == -1)) {              // no cap on both sides: nothing to show
            const bool vk = vdx != kUnk, ek = edx != kUnk;
            tokDex(es, sizeof es, edx); tokDex(vs, sizeof vs, vdx);
            const int vr = vdx < 0 ? 100 : vdx, er = edx < 0 ? 100 : edx;   // no cap ranks as the best value
            const bool up = vr > er;
            if (vk && ek && vdx == edx) snprintf(t, sizeof t, "Max Dex = %s", vs);
            else if (vk && ek && vdx >= 0 && edx >= 0) snprintf(t, sizeof t, "Max Dex %s -> %s (%s%d)", es, vs, up ? "+" : "-", up ? vdx - edx : edx - vdx);
            else if (vk && ek) snprintf(t, sizeof t, "Max Dex %s -> %s (%s)", es, vs, up ? "+" : "-");
            else snprintf(t, sizeof t, "Max Dex %s -> %s (changed)", es, vs);
            setRow(rows[n++], t, t);
        }
    }
}

struct Target { unsigned bit; const char* name; const SlotRead* sr; };

// Slot label for a single equip-mask bit ("?" for anything else). Kept for adapters and logs; the box rows use Target::name.
inline const char* slotName(unsigned bit) {
    switch (bit) {
    case 0x1: return "Head";
    case 0x2: return "Armor";
    case 0x8: return "Gloves";
    case 0x10: return "Right Hand";
    case 0x20: return "Left Hand";
    case 0x200: return "Implant";
    case 0x400: return "Belt";
    default: return "?";
    }
}

// "vs <Slot>: <name>", "vs <Slot>: empty", or "vs <Slot>: Unarmed" for an empty/zero-damage weapon hand. The name is word-cut with "..." so the row is exactly kRowW at most.
inline void headerOf(char* dst, unsigned sz, const char* slot, const SlotRead& s, unsigned W = kRowW) {
    char pre[32];
    snprintf(pre, sizeof pre, "vs %s: ", slot);
    if (s.state != SlotState::Item) { snprintf(dst, sz, "%sempty", pre); return; }
    const char* nm = s.item.name;
    const unsigned room = W - (unsigned)strlen(pre);   // >= 15: the longest slot word is "Right Hand"
    const unsigned nl = blen(nm, sizeof s.item.name - 1);
    if (nl <= room) { snprintf(dst, sz, "%s%.*s", pre, (int)nl, nm); return; }
    const unsigned cap = room - 3;
    unsigned p = cap; while (p > 0 && nm[p] != ' ') p--;
    unsigned n = p > 0 ? p : cap;
    while (n > 0 && !asciiAlnum(nm[n - 1])) n--;
    if (n == 0) n = cap;
    snprintf(dst, sz, "%s%.*s...", pre, (int)n, nm);
}

// ISF-EQBOX-4 u04 (POC): Changed prop row text. Same key, both texts lead with a number of the same known category
// polarity, and the number differs: "<label> <old> -> <new> (+n/-n)" (pdFormatChange). Label = b's text without its
// number. If label + value > kRowW the label is cut to make room ("Very Long Wea... 1 -> 2 (+1)"). If there is
// no direction, or no room for even a short label: "<a> -> <b> (changed)". Output is always <= kRowW chars.
inline void fmtPropChange(char* dst, unsigned sz, const PropRow& a, const PropRow& b, unsigned W = kRowW) {
    char val[64], t[160];
    bool numeric = false;
    if (pdFormatChange(a, b, val, sizeof val)) {
        const unsigned vl = blen(val, sizeof val);
        char label[64];
        unsigned p = 0;
        const char* s = b.text;
        while (s[p] == ' ') ++p;
        const bool hasLeadSign = (s[p] == '+' || s[p] == '-');
        const bool hasLeadNum = (s[p] >= '0' && s[p] <= '9') || (hasLeadSign && s[p + 1] >= '0' && s[p + 1] <= '9');
        if (hasLeadNum) {
            if (hasLeadSign) ++p;
            while (s[p] >= '0' && s[p] <= '9') ++p;
            while (s[p] == ' ') ++p;
            cut(label, sizeof label, s + p, 63);
        } else {
            unsigned start = 0;
            while (s[start] == ' ') ++start;
            unsigned slen = blen(s, sizeof b.text);
            while (slen > 0 && s[slen - 1] == ' ') --slen;
            unsigned end = slen;
            while (end > start && s[end - 1] >= '0' && s[end - 1] <= '9') --end;
            if (end > start && (s[end - 1] == '+' || s[end - 1] == '-')) --end;
            while (end > start && (s[end - 1] == ' ' || s[end - 1] == ':')) --end;
            unsigned len = (end > start) ? (end - start) : 0;
            cut(label, sizeof label, s + start, len < 63 ? len : 63);
        }
        unsigned ll = blen(label, sizeof label);
        while (ll > 0 && label[ll - 1] == ' ') label[--ll] = '\0';
        if (vl <= W) {
            if (ll == 0) { snprintf(t, sizeof t, "%s", val); numeric = true; }
            else if (ll + 1 + vl <= W) { snprintf(t, sizeof t, "%s %s", label, val); numeric = true; }
            else if (vl + 5 <= W) {                       // room for >= 1 label char + "..."
                char cutLab[64];
                cut(cutLab, sizeof cutLab, label, W - vl - 4);
                unsigned cl = blen(cutLab, sizeof cutLab);
                while (cl > 0 && cutLab[cl - 1] == ' ') cutLab[--cl] = '\0';
                snprintf(t, sizeof t, "%s... %s", cutLab, val);
                numeric = true;
            }
        }
    }
    if (!numeric) snprintf(t, sizeof t, "%.63s -> %.63s (changed)", a.text, b.text);
    fit(dst, sz, t, W);
}

// Display-only shortening of engine prop labels so a row fits one line (propDiff keys on the long text, so this runs after it):
// "Damage Bonus: Energy 2" -> "Dmg Energy 2", "Attribute Bonus: Strength +2" -> "Strength +2".
inline void shortProp(char* dst, unsigned sz, const char* src) {
    static const struct { const char* from; const char* to; } kMap[] = {
        {"Damage Bonus: ", "Dmg "}, {"Attribute Bonus: ", ""}, {"Skill Bonus: ", ""}, {"Improved Saving Throws: ", "Save "},
        {"Attack Bonus: ", "Attack "}, {"Damage Immunity: ", "Immune "}, {"Damage Resistance: ", "Resist "}, {"Attribute Penalty: ", "-"},
    };
    for (const auto& m : kMap) {
        const size_t fl = strlen(m.from);
        if (!strncmp(src, m.from, fl)) { snprintf(dst, sz, "%s%s", m.to, src + fl); return; }
    }
    snprintf(dst, sz, "%s", src);
}

}  // namespace cmpd

// compare() keeps ~20 KB of rows on the stack and CompareInput is ~10 KB: call it on the GUI thread with CompareInput/CompareOut held as statics.
inline void compare(const CompareInput& in, CompareOut* out) {
    using namespace cmpd;
    *out = CompareOut();
    const ItemFacts& v = in.viewed;
    if (!v.ok) { out->why = kWhyViewNotOk; return; }
    snprintf(out->viewed, sizeof out->viewed, "%s", v.name);

    // targets, ascending slot bit; hands collapse to one target. Left-only items target the left hand, so the right hand is never read.
    Target tg[8]; unsigned nt = 0;
    const unsigned m = v.slotMask;
    const bool hand = (m & 0x30) != 0;
    const bool leftOnly = hand && (m & 0x10) == 0;
    if (m & 0x1) tg[nt++] = { 0x1, "Headgear", &in.head };
    if (m & 0x2) tg[nt++] = { 0x2, "Armor", &in.armor };
    if (m & 0x8) tg[nt++] = { 0x8, "Gloves", &in.gauntlet };
    if (hand) tg[nt++] = leftOnly ? Target{ 0x20, "Left Hand", &in.left } : Target{ 0x10, "Right Hand", &in.right };
    if (m & 0x200) tg[nt++] = { 0x200, "Implant", &in.implant };
    if (m & 0x400) tg[nt++] = { 0x400, "Belt", &in.belt };
    if (!nt) { out->why = kWhyNotWeaponOrArmor; return; }

    // tri-state: a needed slot unreadable hides everything. The right hand is not a target for a left-only item (:315 rule),
    // so an unreadable right hand no longer hides a left-only item. The left hand is always checked for hand items.
    for (unsigned i = 0; i < nt; i++) {
        if (tg[i].sr->state == SlotState::Unreadable) { out->why = kWhySlotUnreadable; return; }
    }
    if (hand && in.left.state == SlotState::Unreadable) { out->why = kWhySlotUnreadable; return; }

    // hand source: right else left; both empty -> Right Hand / empty (Left Hand / empty for left-only items)
    SlotRead handSrc; const char* handSlot = leftOnly ? "Left Hand" : "Right Hand"; bool leftSrc = false;
    if (hand) {
        // a readable-zero-damage item (shield) is not a weapon source: with it in the only occupied hand the fists still hit (wave B u07)
        const auto zeroDmg = [](const SlotRead& s) { return s.item.numDice == 0 && s.item.die == 0; };
        if (!leftOnly && in.right.state == SlotState::Item && !zeroDmg(in.right)) handSrc = in.right;
        else if (in.left.state == SlotState::Item && !zeroDmg(in.left)) { handSrc = in.left; handSlot = "Left Hand"; leftSrc = true; }
        else {   // both hands empty = fists: 1d3, crit 20 x2 (K1/K2 unarmed; no baseitems row carries it)
            handSrc.state = SlotState::Item; handSrc.item.ok = true;
            snprintf(handSrc.item.name, sizeof handSrc.item.name, "Unarmed");
            handSrc.item.numDice = 1; handSrc.item.die = 3; handSrc.item.critThreat = 1; handSrc.item.critMult = 2; handSrc.item.critBonus = 0;
        }
    }

    Row rows[kRowMax]; unsigned nr = 0;
    char frees[64] = {};
    const bool single = (nt == 1);
    const SlotRead* src = tg[0].sr;
    bool weaponKind = false;
    if (single && hand) {
        src = &handSrc; weaponKind = true;
        if (m == 0x10 && in.left.state == SlotState::Item && !leftSrc && in.left.item.id != handSrc.item.id)
            snprintf(frees, sizeof frees, "Also unequips Left Hand: %.28s", in.left.item.name);   // a loss, named (the lost item's stats are not compared)
    }
    const bool armorKind = single && !hand && tg[0].bit == 0x2;

    PropDiff pd; memset(&pd, 0, sizeof pd);
    if (single) {
        if (weaponKind) { addDamage(rows, nr, v, *src); addCrit(rows, nr, v, *src); }
        else if (armorKind) addArmor(rows, nr, v, *src);
    }
    unsigned nStat = nr;
    // Equip prerequisites (itempropdef rows 43 alignment, 44 class, 45 racial, 52 special walk, 57 feat, 60 gender, 61 subrace, 62 PC,
    // 69 attribute) are not stat changes: they never become Gained/Lost/Changed rows. One "Requirements differ" row says they differ.
    PropRow eK[PROP_DIFF_MAX] = {}, vK[PROP_DIFF_MAX] = {};
    unsigned eKn = 0, vKn = 0;
    bool reqDiffer = false;
    char reqText[160] = "Requirements differ";   // band "Requirements differ"; block "Requires: <equipped> vs <selected>"
    if (single) {
        const PropRow* eP = (src->state == SlotState::Item) ? src->item.props : nullptr;
        unsigned eN = eP ? src->item.nProps : 0, eD = eP ? src->item.dropped : 0;
        if (eN > PROP_DIFF_MAX) eN = PROP_DIFF_MAX;
        const unsigned vN = v.nProps > PROP_DIFF_MAX ? PROP_DIFF_MAX : v.nProps;
        PropRow eR[PROP_DIFF_MAX] = {}, vR[PROP_DIFF_MAX] = {};
        unsigned eRn = 0, vRn = 0;
        for (unsigned i = 0; i < eN; i++) { if (eP[i].type == 28) continue; if (isPrereqType(eP[i].type)) eR[eRn++] = eP[i]; else eK[eKn++] = eP[i]; }   // Keen (28) lives in the Crit Range row; as a prop row it only showed when it sat inside the 16-row window
        for (unsigned i = 0; i < vN; i++) { if (v.props[i].type == 28) continue; if (isPrereqType(v.props[i].type)) vR[vRn++] = v.props[i]; else vK[vKn++] = v.props[i]; }
        reqDiffer = prereqDiffer(eR, eRn, vR, vRn);
        if (reqDiffer) {   // block: "Requires: <equipped> vs <selected>" (equipped first, "none" for a side with no prerequisite)
            char vs[96] = "", es[96] = "";
            unsigned vl = 0, el = 0;
            for (unsigned i = 0; i < vRn && vl < sizeof vs; i++) vl += (unsigned)snprintf(vs + vl, sizeof vs - vl, "%s%.31s", vl ? ", " : "", reqBody(vR[i].text));
            for (unsigned j = 0; j < eRn && el < sizeof es; j++) el += (unsigned)snprintf(es + el, sizeof es - el, "%s%.31s", el ? ", " : "", reqBody(eR[j].text));
            if (!vRn) snprintf(vs, sizeof vs, "none");
            if (!eRn) snprintf(es, sizeof es, "none");
            const unsigned avail = kBlockW - 14;   // "Requires: " + " vs "
            unsigned vm = (unsigned)strlen(vs), em = (unsigned)strlen(es);
            if (vm + em > avail) { const unsigned half = avail / 2; if (vm <= half) em = avail - vm; else if (em <= half) vm = avail - em; else { vm = half; em = avail - half; } }
            if (vm < strlen(vs)) { vs[vm > 3 ? vm - 3 : 0] = 0; strncat(vs, "...", 3); }
            if (em < strlen(es)) { es[em > 3 ? em - 3 : 0] = 0; strncat(es, "...", 3); }
            snprintf(reqText, sizeof reqText, "Requires: %s vs %s", es, vs);   // equipped first, like the title and the stat arrows
        }
        PropRow none[1] = {};
        propDiff(eKn ? eK : none, eKn, vK, vKn, eD, v.dropped, &pd);
    }

    // prop rows for band A (flat, Changed -> Gained -> Lost) in rows[nStat..]. Unknown direction: "(changed)" (u09).
    const unsigned propStart = nr;
    for (unsigned i = 0; i < pd.nChanged && nr < kRowMax; i++) {
        PropRow a = eK[pd.changed[i].a], b = vK[pd.changed[i].b];
        shortProp(a.text, sizeof a.text, eK[pd.changed[i].a].text); shortProp(b.text, sizeof b.text, vK[pd.changed[i].b].text);
        char t[160], tb[160];
        if (pd.changed[i].magnitude) {   // equipped first: a differing text prints "<equipped> -> <selected> (changed)"
            if (strcmp(a.text, b.text) != 0) snprintf(t, sizeof t, "%.28s -> %.28s (changed)", a.text, b.text); else snprintf(t, sizeof t, "%.63s (changed)", b.text);
            snprintf(tb, sizeof tb, "%s", t);
        }
        else { fmtPropChange(t, sizeof t, a, b); fmtPropChange(tb, sizeof tb, a, b, kBlockW); }   // band keeps the 30-char row, block gets the wide one
        setRow(rows[nr++], t, tb);
    }
    for (unsigned i = 0; i < pd.nGained && nr < kRowMax; i++) {
        char t[100], sh[80]; shortProp(sh, sizeof sh, vK[pd.gained[i].b].text); snprintf(t, sizeof t, "+ %.63s", sh); setRow(rows[nr++], t, t);
    }
    for (unsigned i = 0; i < pd.nLost && nr < kRowMax; i++) {
        char t[100], sh[80]; shortProp(sh, sizeof sh, eK[pd.lost[i].a].text); snprintf(t, sizeof t, "- %.63s", sh); setRow(rows[nr++], t, t);
    }
    bool reqRow = false;
    if (reqDiffer && nr < kRowMax) {   // band: fixed text; block: the "Requires: <equipped> vs <selected>" line (fit() cuts it to the width)
        setRow(rows[nr++], "Requirements differ", reqText); reqRow = true;
    }
    const unsigned notCmp = (unsigned)pd.notCompared[0] + pd.notCompared[1];

    // weapon / armor with nothing to show: no output at all
    if ((weaponKind || armorKind) && nr == 0) { out->why = kWhyNoRows; return; }
    // any other single slot with nothing to show: the fallback is a stat-style row, so band and block both carry it
    if (single && nr == 0) { setRow(rows[nr++], "No property changes", "No property changes"); nStat = nr; }

    // band A: first two rows; multi-slot stays empty
    if (single && nr) {
        const unsigned nb = nr < 2 ? nr : 2;
        fit(out->band[0], sizeof out->band[0], rows[0].b, kRowW);
        if (nb == 2) {
            if (nr > 2 && nStat <= 1) {            // hidden rows marked; a stat row on line 2 carries no marker (D4)
                char suffix[16], tmp[32];
                snprintf(suffix, sizeof suffix, " (%u more)", nr - 2);
                fit(tmp, sizeof tmp, rows[1].b, kRowW - (unsigned)strlen(suffix));
                snprintf(out->band[1], sizeof out->band[1], "%s%s", tmp, suffix);
            } else fit(out->band[1], sizeof out->band[1], rows[1].b, kRowW);
        }
        out->nBand = nb;
    }

    // block body lines: stat rows (k 0), then every prop row (k 1 Changed, 2 Gained, 3 Lost). The overflow tail is cut
    // below against the line budget L (u09 rows 21-22), so no band is pre-compressed here.
    // w = rows the line stands for, so a kind-named tail can sum hidden rows per kind.
    struct Line { char t[64]; unsigned w; unsigned k; };
        Line body[kRowMax + 4]; unsigned nb = 0;
    for (unsigned i = 0; i < nStat; i++) { fit(body[nb].t, 64, rows[i].k, kBlockW); body[nb].w = 1; body[nb++].k = 0; }
    const unsigned bandN[3] = { pd.nChanged, pd.nGained, pd.nLost };
    unsigned at = propStart;
    for (unsigned b = 0; b < 3; b++) {
        const unsigned kind = b + 1;
        const unsigned cnt = at + bandN[b] <= nr ? bandN[b] : nr - at;
        for (unsigned i = 0; i < cnt; i++) { fit(body[nb].t, 64, rows[at + i].k, kBlockW); body[nb].w = 1; body[nb++].k = kind; }
        at += cnt;
    }
    if (reqRow) { fit(body[nb].t, 64, rows[nr - 1].k, kBlockW); body[nb].w = 1; body[nb++].k = 0; }   // after the three kinds: plain, not counted in a tail

    // assemble block: every line goes through push(), which caps the count and the width
    unsigned nl = 0;
    auto push = [&](const char* s) { if (nl < kBlockMaxLines) fit(out->block[nl++], 64, s, kBlockW); };
    for (unsigned i = 0; i < nt; i++) {
        char h[96];
        if (single && hand) headerOf(h, sizeof h, handSlot, handSrc, kBlockW);
        else headerOf(h, sizeof h, tg[i].name, *tg[i].sr, kBlockW);
        push(h);
    }
    // D6 priority: header, Can't equip, stat rows (never hidden), prop lines, then Frees Left Hand and "N props not compared".
    if (single) {
        const unsigned nbP = nb - (reqRow ? 1u : 0u);   // the Requires row is a fixed line (review ISF-REV: it used to sit in the prop tail and vanish when props overflowed)
        const unsigned fixed = 1 + (in.canEquip ? 0u : 1u) + nStat + (reqRow ? 1u : 0u) + (notCmp ? 1u : 0u);
        if (!in.canEquip) push("Can't equip this");
        for (unsigned i = 0; i < nStat; i++) push(body[i].t);
        const unsigned L = fixed < kBlockMaxLines ? kBlockMaxLines - fixed : 0;   // prop lines that fit
        const unsigned nP = nbP - nStat;
        if (nP <= L) {
            for (unsigned i = nStat; i < nbP; i++) push(body[i].t);
            if (reqRow) push(body[nbP].t);
            if (frees[0] && L - nP >= 1) push(frees);
        } else {
            const unsigned keep = L > 0 ? L - 1 : 0;   // the last prop line is the kind-named tail
            for (unsigned i = nStat; i < nStat + keep; i++) push(body[i].t);
            unsigned kw[4] = { 0, 0, 0, 0 };
            for (unsigned i = nStat + keep; i < nbP; i++) kw[body[i].k] += body[i].w;
            const unsigned kinds = (kw[1] > 0) + (kw[2] > 0) + (kw[3] > 0);
            char t[96];
            if (kinds == 1) {
                const unsigned k = kw[1] ? 1 : (kw[2] ? 2 : 3);
                if (k == 1) snprintf(t, sizeof t, "%u more changed", kw[k]); else snprintf(t, sizeof t, "%c%u more", k == 2 ? '+' : '-', kw[k]);   // +n / -n: the colour is not the only cue
            } else {
                const unsigned total = kw[1] + kw[2] + kw[3];
                unsigned n = (unsigned)snprintf(t, sizeof t, "%u more:", total);
                const char* sep = " ";
                for (unsigned k = 1; k < 4; k++) {
                    if (kw[k] && n < sizeof t) { n += (unsigned)snprintf(t + n, sizeof t - n, k == 1 ? "%s%u changed" : (k == 2 ? "%s+%u" : "%s-%u"), sep, kw[k]); sep = ", "; }
                }
            }
            push(t);
            if (reqRow) push(body[nbP].t);
        }
        if (notCmp) {
            char t[40];
            if (notCmp == 1) snprintf(t, sizeof t, "1 prop not compared");
            else snprintf(t, sizeof t, "%u props not compared", notCmp);
            push(t);
        }
    }
    out->nBlock = nl;
    out->why = !single ? kWhyOther : (out->nBand ? kWhyOk : kWhyNoRows);   // multi-slot leaves the band empty by design (other)
}

}  // namespace itemcmp

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
