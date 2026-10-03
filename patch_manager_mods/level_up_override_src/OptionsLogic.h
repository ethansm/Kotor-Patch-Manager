// OptionsLogic.h - pure (engine-free) model of the single "options page" for the level-up path chooser.
// Python twin: research/companion_levelup/options_twin.py ; parity: tools/parity/options_parity.py
// Rows (fixed order): Convert, Role, Style, Alignment, Skills, then the pseudo-row Variant. A row is SHOWN only when the tag's
// presets have >= 2 distinct values for it. State = the current preset index. cycle(row, dir): candidate values = distinct values
// (first-appearance order) among presets matching cur on all SHOWN rows before `row`; step to the next/previous value (wrapping),
// then pick the preset matching rows-before + the new value that agrees with cur on the most later rows, compared lexicographically
// in row order (row+1 first); ties -> lowest index. Variant is shown only when >1 preset matches all five answers of cur.
#pragma once
#include "WizardLogic.h"

namespace opt {

using wiz::Item;
using wiz::ieq;
constexpr int NumRows = 5, VariantRow = 5, MaxItems = wiz::MaxItems, MaxValues = wiz::MaxItems;
// Row order -> wiz page id (Item::ans index).
constexpr int kRowPage[NumRows] = { wiz::Convert, wiz::Role, wiz::Style, wiz::Alignment, wiz::Skills };

struct State {
    const Item* items;
    int n;
    int cur;
};

inline const char* rowName(int row) {
    static const char* const n[] = {"Convert", "Role", "Style", "Alignment", "Skills", "Variant"};
    return (row >= 0 && row <= VariantRow) ? n[row] : "None";
}

inline void init(State& s, const Item* items, int n, int startItem) {
    s.items = items;
    s.n = n < 0 ? 0 : (n > MaxItems ? MaxItems : n);
    s.cur = (startItem >= 0 && startItem < s.n) ? startItem : 0;
}

inline const Item& current(const State& s) { return s.items[s.cur]; }

namespace detail {
inline const char* val(const State& s, int i, int row) { return s.items[i].ans[kRowPage[row]]; }

inline bool rowShown(const State& s, int row) {
    for (int i = 1; i < s.n; ++i) if (!ieq(val(s, i, row), val(s, 0, row))) return true;
    return false;
}
// presets matching `ref` on every shown row < upTo
inline bool matchBefore(const State& s, int i, int ref, int upTo) {
    for (int r = 0; r < upTo; ++r)
        if (rowShown(s, r) && !ieq(val(s, i, r), val(s, ref, r))) return false;
    return true;
}
inline bool sameAll(const State& s, int i, int ref) {
    for (int r = 0; r < NumRows; ++r) if (!ieq(val(s, i, r), val(s, ref, r))) return false;
    return true;
}
inline int variantGroup(const State& s, int* out) {
    int c = 0;
    for (int i = 0; i < s.n; ++i) if (sameAll(s, i, s.cur)) out[c++] = i;
    return c;
}
// distinct values on `row` (first-appearance order) among presets matching cur on shown rows before it; indices of first holders
inline int candidates(const State& s, int row, int* firstItem) {
    int nv = 0;
    for (int i = 0; i < s.n; ++i) {
        if (!matchBefore(s, i, s.cur, row)) continue;
        bool seen = false;
        for (int j = 0; j < nv; ++j) if (ieq(val(s, firstItem[j], row), val(s, i, row))) { seen = true; break; }
        if (!seen) firstItem[nv++] = i;
    }
    return nv;
}
}  // namespace detail

inline bool shown(const State& s, int row) {
    if (row < 0 || row > VariantRow) return false;
    if (row == VariantRow) { int g[MaxItems]; return detail::variantGroup(s, g) > 1; }
    return detail::rowShown(s, row);
}

// Candidate values of `row` (Variant: short names of the identical presets). Returns the count written (<= cap).
inline int options(const State& s, int row, const char** out, int cap) {
    if (!shown(s, row)) return 0;
    int first[MaxValues];
    int nv;
    if (row == VariantRow) nv = detail::variantGroup(s, first);
    else nv = detail::candidates(s, row, first);
    if (nv > cap) nv = cap;
    for (int j = 0; j < nv; ++j) out[j] = row == VariantRow ? s.items[first[j]].shortName : detail::val(s, first[j], row);
    return nv;
}

inline bool cycle(State& s, int row, int dir) {
    if (!shown(s, row)) return false;
    int first[MaxValues];
    if (row == VariantRow) {
        int nv = detail::variantGroup(s, first);
        if (nv < 2) return false;
        int at = 0;
        for (int j = 0; j < nv; ++j) if (first[j] == s.cur) at = j;
        s.cur = first[((at + (dir < 0 ? -1 : 1)) % nv + nv) % nv];
        return true;
    }
    int nv = detail::candidates(s, row, first);
    if (nv < 2) return false;
    int at = 0;
    for (int j = 0; j < nv; ++j) if (ieq(detail::val(s, first[j], row), detail::val(s, s.cur, row))) at = j;
    const char* want = detail::val(s, first[((at + (dir < 0 ? -1 : 1)) % nv + nv) % nv], row);
    int best = -1;
    bool bestScore[NumRows];
    for (int i = 0; i < s.n; ++i) {
        if (!detail::matchBefore(s, i, s.cur, row) || !ieq(detail::val(s, i, row), want)) continue;
        bool sc[NumRows];
        for (int r = 0; r < NumRows; ++r) sc[r] = r > row && ieq(detail::val(s, i, r), detail::val(s, s.cur, r));
        bool better = best < 0;
        for (int r = row + 1; r < NumRows && best >= 0; ++r) {
            if (sc[r] != bestScore[r]) { better = sc[r]; break; }
        }
        if (better) { best = i; for (int r = 0; r < NumRows; ++r) bestScore[r] = sc[r]; }
    }
    if (best < 0) return false;
    s.cur = best;
    return true;
}

}  // namespace opt
