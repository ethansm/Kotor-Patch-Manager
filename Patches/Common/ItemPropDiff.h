// ItemPropDiff.h -- pure (engine-free, heap-free) item property diff for the equip-compare feature.
// Plan: .claude/research/equip_compare/PLAN.md sec 7; design: .claude/research/equip_compare/phase2/Q3_propdiff.md.
// The adapter (engine side) fills PropRow arrays with display text already resolved; this header never calls the engine.
// W4 (ISF-EQBOX-3/w4b, variant B): numeric awareness. Leading signed numbers are parsed per row inside propDiff;
// phase 1 pairs exact text by closest raw costValue/costTable/param1, phase 2 pairs same-key rows by numeric
// closeness (fixes sort-order mis-pairing); pdFormatChange() renders "<old> -> <new> (up|down n)" for known
// categories and "(changed)" otherwise. PropRow / PropDiff / PropEntry layouts are unchanged (workbench-upgrade API).
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

namespace itemcmp {

constexpr unsigned PROP_DIFF_MAX = 16;   // rows per side the core accepts; the adapter drops the rest and reports it in dropA/dropB
constexpr uint8_t PD_NONE = 0xFF;

struct PropRow {
    uint16_t type, subtype, costValue;   // KEY = (type, subtype); costValue is NOT part of the key
    uint8_t costTable, param1;           // raw magnitude (param1 = Param1Value), compared only for equal-text pairs
    uint8_t known;                       // 0 = label miss ("Unknown"): suppress, never report as Lost/Gained
    char text[64];                       // NUL-terminated display text ("Yes" already applied by the adapter when empty)
};

struct PropEntry { uint8_t a, b, magnitude; };   // indices into side A / side B (PD_NONE = absent); magnitude=1: text equal, raw differs

struct PropDiff {
    PropEntry changed[PROP_DIFF_MAX]; uint8_t nChanged;
    PropEntry gained[PROP_DIFF_MAX];  uint8_t nGained;   // a == PD_NONE
    PropEntry lost[PROP_DIFF_MAX];    uint8_t nLost;     // b == PD_NONE
    uint8_t suppressed;                                   // rows removed as Unknown (label miss, or sharing a key with one)
    uint8_t notCompared[2];                               // rows the adapter dropped beyond PROP_DIFF_MAX (A, B)
};

// ---- numeric awareness -------------------------------------------------------------------------------------------

// |value| above this is treated as non-numeric, so every difference stays far inside int64 (no overflow).
constexpr int64_t PD_NUM_LIMIT = 1000000000;

// Direction table: a category word found (whole word, case-sensitive) in the row text. +1 = a bigger number is better,
// so an increase reads "up" and a decrease reads "down". The table is kept small on purpose: a category with unverified
// polarity (for example "Penalty", where a smaller magnitude is the improvement) is left out and renders "(changed)".
// Order = priority: the first table entry found in the text wins.
struct PdCategory { const char* word; int pol; };
constexpr PdCategory kPdCategories[] = {
    {"Penalty", 0}, {"Decreased", 0}, {"Decrease", 0}, {"Vulnerability", 0},
    {"Attack", +1}, {"Armor", +1}, {"Defense", +1}, {"Saves", +1}, {"Save", +1},
    {"AC", +1}, {"DC", +1}, {"Strength", +1}, {"Dexterity", +1}, {"Constitution", +1},
    {"Intelligence", +1}, {"Wisdom", +1}, {"Charisma", +1},
    {"Regeneration", +1}, {"Regenerate", +1}, {"Mighty", +1}, {"Skill", +1},
};

// Parsed per-row numeric state (carried through propDiff's pairing; never stored in PropRow).
struct PdNum { int64_t v; uint8_t has; int8_t pol; };   // has = leading integer parsed; pol = category polarity (0 = none)

// Bounded helpers: never read past n bytes, and stop at NUL.
inline bool pdIsDigit(char c) { return c >= '0' && c <= '9'; }
inline bool pdIsAlnum(char c) { return pdIsDigit(c) || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

// Leading or trailing signed integer:
// First try parsing at the start of s (optional spaces, optional sign, digits).
// If s does not start with an integer, find the last token in s (after ':' or ' ', trimming trailing spaces):
// if that last token is an optional '+' / '-' followed ONLY by digits (no '%', no '.', no '-' range,
// ending at NUL/trailing spaces, magnitude <= PD_NUM_LIMIT), parse it into *out and return true.
inline bool pdLeadNum(const char* s, size_t n, int64_t* out) {
    if (s == nullptr || out == nullptr) return false;
    size_t len = 0;
    while (len < n && s[len] != '\0') ++len;

    // 1. Try leading signed integer
    size_t p = 0;
    while (p < len && s[p] == ' ') ++p;
    bool leadNeg = false;
    size_t numStart = p;
    if (p < len && (s[p] == '+' || s[p] == '-')) { leadNeg = (s[p] == '-'); ++numStart; }
    if (numStart < len && pdIsDigit(s[numStart])) {
        p = numStart;
        int64_t v = 0;
        while (p < len && pdIsDigit(s[p])) {
            v = v * 10 + (s[p] - '0');             // v <= PD_NUM_LIMIT before the step, so this stays < 1e10
            if (v > PD_NUM_LIMIT) return false;
            ++p;
        }
        if (p + 1 < len && s[p] == '.' && pdIsDigit(s[p + 1])) return false;   // decimal
        if (p + 1 < len && s[p] == '-' && pdIsDigit(s[p + 1])) return false;   // range "1-6"
        if (p < len && s[p] == '%') return false;                              // "5%" / "10% chance": same rule as the trailing path (no '%')
        if (p + 1 < len && (s[p] == 'd' || s[p] == 'D') && pdIsDigit(s[p + 1])) return false;   // dice "1d4": the leading 1 is a count, not a magnitude
        *out = leadNeg ? -v : v;
        return true;
    }

    // 2. Try trailing signed integer after ':' or ' '
    size_t end = len;
    while (end > 0 && s[end - 1] == ' ') --end;
    if (end == 0) return false;

    size_t tok = end;
    while (tok > 0 && s[tok - 1] != ' ' && s[tok - 1] != ':') {
        --tok;
    }

    size_t tp = tok;
    bool trailNeg = false;
    if (tp < end && (s[tp] == '+' || s[tp] == '-')) {
        trailNeg = (s[tp] == '-');
        ++tp;
    }
    if (tp >= end || !pdIsDigit(s[tp])) return false;

    int64_t tv = 0;
    while (tp < end) {
        if (!pdIsDigit(s[tp])) return false;
        tv = tv * 10 + (s[tp] - '0');
        if (tv > PD_NUM_LIMIT) return false;
        ++tp;
    }

    *out = trailNeg ? -tv : tv;
    return true;
}

// Whole-word match of word at position i (word boundaries are non-alphanumeric or text edges).
inline bool pdWordAt(const char* s, size_t n, size_t i, const char* word) {
    const size_t len = strlen(word);
    if (i + len > n) return false;
    if (memcmp(s + i, word, len) != 0) return false;
    if (i > 0 && pdIsAlnum(s[i - 1])) return false;
    if (i + len < n && pdIsAlnum(s[i + len])) return false;
    return true;
}

// Category polarity from the table; 0 when no table word appears.
inline int pdPolarity(const char* s, size_t n) {
    for (const PdCategory& c : kPdCategories) {
        for (size_t i = 0; i < n && s[i] != '\0'; ++i) {
            if (pdWordAt(s, n, i, c.word)) return c.pol;
        }
    }
    return 0;
}

inline PdNum pdNumOf(const PropRow& r) {
    PdNum x;
    x.v = 0;
    x.has = pdLeadNum(r.text, sizeof r.text, &x.v) ? 1 : 0;
    if (!x.has) x.v = 0;
    x.pol = static_cast<int8_t>(pdPolarity(r.text, sizeof r.text));
    return x;
}

inline int64_t pdAbsDiff(int64_t x, int64_t y) {   // saturates instead of overflowing for extreme operands
    const int64_t hi = x > y ? x : y, lo = x > y ? y : x;
    if (lo < 0 && hi > INT64_MAX + lo) return INT64_MAX;
    return hi - lo;
}

// Writes the value part of a Changed row into out (NUL-terminated, never past cap).
// "<old> -> <new> (up|down n)" when both texts lead with an in-range number, both carry the same known category
// polarity, and the number differs; otherwise "(changed)". Returns true only for the numeric form.
inline bool pdFormatChange(const PropRow& a, const PropRow& b, char* out, size_t cap) {
    if (out == nullptr || cap == 0) return false;
    const PdNum na = pdNumOf(a), nb = pdNumOf(b);
    if (na.has && nb.has && na.pol != 0 && na.pol == nb.pol && na.v != nb.v) {
        const int64_t d = nb.v - na.v;                       // |v| <= PD_NUM_LIMIT, so no overflow
        const bool up = (d > 0) == (na.pol > 0);             // up = improved, down = worsened
        const int64_t mag = d < 0 ? -d : d;
        const int n = snprintf(out, cap, "%lld -> %lld (%s%lld)", static_cast<long long>(na.v), static_cast<long long>(nb.v),
                               up ? "+" : "-", static_cast<long long>(mag));
        return n >= 0 && static_cast<size_t>(n) < cap;      // false when the numeric text was truncated (needs cap >= 41)
    }
    snprintf(out, cap, "%s", "(changed)");
    return false;
}

// ---- ordering and equality ---------------------------------------------------------------------------------------

// Comparator for (type, subtype, text, costValue, costTable, param1): gives a total order for sorting.
inline int pdCmpRow(const PropRow& x, const PropRow& y) {
    if (x.type != y.type) return x.type < y.type ? -1 : 1;
    if (x.subtype != y.subtype) return x.subtype < y.subtype ? -1 : 1;
    const int c = strncmp(x.text, y.text, sizeof x.text);
    if (c != 0) return c < 0 ? -1 : 1;
    if (x.costValue != y.costValue) return x.costValue < y.costValue ? -1 : 1;
    if (x.costTable != y.costTable) return x.costTable < y.costTable ? -1 : 1;
    if (x.param1 != y.param1) return x.param1 < y.param1 ? -1 : 1;
    return 0;
}
inline bool pdKeyEq(const PropRow& x, const PropRow& y) { return x.type == y.type && x.subtype == y.subtype; }
inline bool pdTextEq(const PropRow& x, const PropRow& y) { return strncmp(x.text, y.text, sizeof x.text) == 0; }

// Insertion sort of row indices by pdCmpRow (stable; n <= PROP_DIFF_MAX).
inline void pdSortIdx(const PropRow* R, unsigned n, uint8_t* ord) {
    for (unsigned i = 0; i < n; ++i) ord[i] = static_cast<uint8_t>(i);
    for (unsigned i = 1; i < n; ++i) {
        const uint8_t v = ord[i];
        unsigned j = i;
        while (j > 0 && pdCmpRow(R[v], R[ord[j - 1]]) < 0) {
            ord[j] = ord[j - 1];
            --j;
        }
        ord[j] = v;
    }
}

// unkKey(key(r)): some row on either side with the same key has known == 0 (label miss).
inline bool pdUnkKey(const PropRow* A, unsigned na, const PropRow* B, unsigned nb, const PropRow& r) {
    for (unsigned i = 0; i < na; ++i) if (!A[i].known && pdKeyEq(A[i], r)) return true;
    for (unsigned i = 0; i < nb; ++i) if (!B[i].known && pdKeyEq(B[i], r)) return true;
    return false;
}

// One candidate pair during greedy pairing. Lower tuple wins; oi/oj (sorted positions) break every remaining tie.
struct PdPick { int64_t a, b, c; unsigned oi, oj; };
inline bool pdPickLess(const PdPick& x, const PdPick& y) {
    if (x.a != y.a) return x.a < y.a;
    if (x.b != y.b) return x.b < y.b;
    if (x.c != y.c) return x.c < y.c;
    if (x.oi != y.oi) return x.oi < y.oi;
    return x.oj < y.oj;
}

// Phase 1 (exact text only): rank by closest costValue, then by costTable + param1 distance.
// Phase 2 (same key, any text): both numeric first ranked by |value gap|; otherwise sorted order.
// The raw-distance term is the final tie-break in both phases.
inline bool pdCandidate(unsigned phase, const PropRow& x, const PdNum& nx, const PropRow& y, const PdNum& ny, PdPick* c) {
    const int64_t dOther = pdAbsDiff(x.costTable, y.costTable) + pdAbsDiff(x.param1, y.param1);
    if (phase == 1) {
        if (!pdTextEq(x, y)) return false;
        c->a = pdAbsDiff(x.costValue, y.costValue);
        c->b = dOther;
        c->c = 0;
        return true;
    }
    const bool both = nx.has && ny.has;
    c->a = both ? 0 : 1;
    c->b = both ? pdAbsDiff(nx.v, ny.v) : 0;
    c->c = dOther;
    return true;
}

// Q3_propdiff.md sec 2. Phase 1 cancels exact (key, text) pairs; phase 2 pairs same-key rows as Changed.
inline void propDiff(const PropRow* A, unsigned na, const PropRow* B, unsigned nb, unsigned dropA, unsigned dropB, PropDiff* out) {
    memset(out, 0, sizeof *out);
    uint64_t da = dropA, db = dropB;                                               // 64-bit: dropX + overflow must not wrap before the 255 clamp
    if (na > PROP_DIFF_MAX) { da += na - PROP_DIFF_MAX; na = PROP_DIFF_MAX; }   // defensive: never index past the arrays
    if (nb > PROP_DIFF_MAX) { db += nb - PROP_DIFF_MAX; nb = PROP_DIFF_MAX; }
    out->notCompared[0] = static_cast<uint8_t>(da > 255u ? 255u : da);
    out->notCompared[1] = static_cast<uint8_t>(db > 255u ? 255u : db);

    uint8_t ordA[PROP_DIFF_MAX] = {}, ordB[PROP_DIFF_MAX] = {};
    pdSortIdx(A, na, ordA);
    pdSortIdx(B, nb, ordB);

    PdNum nA[PROP_DIFF_MAX] = {}, nB[PROP_DIFF_MAX] = {};
    for (unsigned i = 0; i < na; ++i) nA[i] = pdNumOf(A[i]);
    for (unsigned j = 0; j < nb; ++j) nB[j] = pdNumOf(B[j]);

    bool liveA[PROP_DIFF_MAX] = {}, liveB[PROP_DIFF_MAX] = {};
    for (unsigned i = 0; i < na; ++i) {
        liveA[i] = A[i].known != 0 && !pdUnkKey(A, na, B, nb, A[i]);
        if (!liveA[i]) ++out->suppressed;
    }
    for (unsigned j = 0; j < nb; ++j) {
        liveB[j] = B[j].known != 0 && !pdUnkKey(A, na, B, nb, B[j]);
        if (!liveB[j]) ++out->suppressed;
    }

    uint8_t pairA[PROP_DIFF_MAX];
    for (unsigned i = 0; i < PROP_DIFF_MAX; ++i) pairA[i] = PD_NONE;
    uint16_t usedB = 0;

    for (unsigned phase = 1; phase <= 2; ++phase) {
        for (;;) {                                         // greedy: take the best remaining candidate, repeat
            bool found = false;
            PdPick best = {0, 0, 0, 0, 0};
            unsigned bi = 0, bj = 0;
            for (unsigned oi = 0; oi < na; ++oi) {
                const unsigned i = ordA[oi];
                if (!liveA[i] || pairA[i] != PD_NONE) continue;
                for (unsigned oj = 0; oj < nb; ++oj) {
                    const unsigned j = ordB[oj];
                    if (!liveB[j] || ((usedB >> j) & 1u) || !pdKeyEq(A[i], B[j])) continue;
                    PdPick c = {0, 0, 0, oi, oj};
                    if (!pdCandidate(phase, A[i], nA[i], B[j], nB[j], &c)) continue;
                    if (!found || pdPickLess(c, best)) { best = c; bi = i; bj = j; found = true; }
                }
            }
            if (!found) break;
            pairA[bi] = static_cast<uint8_t>(bj);
            usedB = static_cast<uint16_t>(usedB | (1u << bj));
        }
    }

    for (unsigned oi = 0; oi < na; ++oi) {                 // Changed / Lost, ascending A
        const unsigned i = ordA[oi];
        if (!liveA[i]) continue;
        if (pairA[i] == PD_NONE) {
            PropEntry& e = out->lost[out->nLost++];
            e.a = static_cast<uint8_t>(i);
            e.b = PD_NONE;
            e.magnitude = 0;
            continue;
        }
        const unsigned j = pairA[i];
        const bool sameText = pdTextEq(A[i], B[j]);
        if (sameText && A[i].costValue == B[j].costValue && A[i].costTable == B[j].costTable && A[i].param1 == B[j].param1) continue;
        PropEntry& e = out->changed[out->nChanged++];
        e.a = static_cast<uint8_t>(i);
        e.b = static_cast<uint8_t>(j);
        e.magnitude = static_cast<uint8_t>(sameText ? 1 : 0);
    }

    for (unsigned oj = 0; oj < nb; ++oj) {                 // Gained, ascending B
        const unsigned j = ordB[oj];
        if (!liveB[j] || ((usedB >> j) & 1u)) continue;
        PropEntry& e = out->gained[out->nGained++];
        e.a = PD_NONE;
        e.b = static_cast<uint8_t>(j);
        e.magnitude = 0;
    }
}

}  // namespace itemcmp
