// R10Page.h -- pure model of the PC's R10 level-up filter page on the character screen (doc 25 batch 3a). PURE: no windows.h, no STL.
// Host-tested by r10_page_test.cpp. LevelUpUi.h renders the rows into the shared options-page controls (C_ROW0..5) and saves through Playthrough.h
// (the reserved `pc=r10|r10=1|pcfilter=..|ffilter=..|pfilter=..` line).
//
// Rows: Filter on/off | Role (5) | Style (none + 6) | Alignment (none + 3) | Feats mode | Powers mode. Modes offered: all, build_dim, combat, force
// (build_hide stays hidden until batch 2c; a stored build_hide loads as build_dim). Profile rows stay editable while the filter is off, so the user
// can set the build up before turning it on. Tokens are validated through R10Filter.h (parseProfile / parseMode).
//
// Defaults: a PC with NO pc line starts at defaultState() (off, melee/none/none, build_dim, build_dim). fromTokens maps an EMPTY mode token to `all`,
// because Playthrough.h stores `all` as an absent field; an invalid profile falls back to the default profile.
#pragma once
#include "R10Filter.h"
#include <stdio.h>
#include <string.h>

namespace r10p {

enum Row { RowOn = 0, RowRole, RowStyle, RowAlign, RowFeats, RowPowers, RowCount };

constexpr int RoleCount = 5, StyleCount = 7, AlignCount = 4, ModeCount = 4;   // style/align index 0 = none
constexpr int TokenLen = 24;                                                  // pcfilter buffer (== pt::FilterLen)
inline r10f::Mode modeAt(int i) {
    static const r10f::Mode m[ModeCount] = {r10f::All, r10f::BuildDim, r10f::Combat, r10f::Force};
    return m[i >= 0 && i < ModeCount ? i : 0];
}
inline int modeIndex(r10f::Mode m) {
    m = r10f::dimMode(m);   // build_hide is not offered yet: shown as build_dim
    for (int i = 0; i < ModeCount; ++i) if (modeAt(i) == m) return i;
    return 0;
}

struct State {
    bool on = false;
    int role = 0, style = 0, align = 0;   // role 0..4 = melee..tank; style 0 = none, 1..6 = single..rifle; align 0 = none, 1..3 = light..neutral
    int feats = 1, powers = 1;            // modeAt() index; 1 = build_dim
};
inline State defaultState() { return State(); }

inline const char* rowName(int row) {
    static const char* const n[RowCount] = {"on", "role", "style", "align", "feats", "powers"};
    return row >= 0 && row < RowCount ? n[row] : "-";
}
inline int rowCount(int row) {
    switch (row) {
        case RowOn: return 2;
        case RowRole: return RoleCount;
        case RowStyle: return StyleCount;
        case RowAlign: return AlignCount;
        case RowFeats: case RowPowers: return ModeCount;
        default: return 0;
    }
}
inline int* rowField(State& st, int row) {
    switch (row) {
        case RowRole: return &st.role;
        case RowStyle: return &st.style;
        case RowAlign: return &st.align;
        case RowFeats: return &st.feats;
        case RowPowers: return &st.powers;
        default: return nullptr;
    }
}
// Steps one row by dir (+1 / -1), wrapping. Out-of-range rows are ignored.
inline void cycle(State& st, int row, int dir) {
    const int n = rowCount(row);
    if (n <= 0) return;
    if (row == RowOn) { st.on = !st.on; return; }
    int* f = rowField(st, row);
    *f = ((*f + (dir < 0 ? -1 : 1)) % n + n) % n;
}

inline const char* roleWord(int i) { return r10f::tagWords()[i >= 0 && i < RoleCount ? i : 0]; }
inline const char* styleWord(int i) { return i > 0 && i < StyleCount ? r10f::tagWords()[RoleCount + i - 1] : "none"; }
inline const char* alignWord(int i) { return i > 0 && i < AlignCount ? r10f::tagWords()[RoleCount + StyleCount - 1 + i - 1] : "none"; }

// "Filter: on", "Role: melee", "Style: none", "Alignment: light", "Feats: build_dim", "Powers: all". "" for a bad row.
inline void rowText(const State& st, int row, char* buf, int cap) {
    if (!buf || cap <= 0) return;
    switch (row) {
        case RowOn: snprintf(buf, static_cast<size_t>(cap), "Filter: %s", st.on ? "on" : "off"); break;
        case RowRole: snprintf(buf, static_cast<size_t>(cap), "Role: %s", roleWord(st.role)); break;
        case RowStyle: snprintf(buf, static_cast<size_t>(cap), "Style: %s", styleWord(st.style)); break;
        case RowAlign: snprintf(buf, static_cast<size_t>(cap), "Alignment: %s", alignWord(st.align)); break;
        case RowFeats: snprintf(buf, static_cast<size_t>(cap), "Feats: %s", r10f::modeName(modeAt(st.feats))); break;
        case RowPowers: snprintf(buf, static_cast<size_t>(cap), "Powers: %s", r10f::modeName(modeAt(st.powers))); break;
        default: buf[0] = 0; break;
    }
}

inline int bitIndex(uint32_t bit, uint32_t first) { int i = 0; while (first < bit) { first <<= 1; ++i; } return i; }
// From stored tokens (any may be null / ""). Invalid profile -> default melee/none/none; empty/unknown mode -> all.
inline State fromTokens(bool on, const char* pcfilter, const char* ffilter, const char* pfilter) {
    State st;
    st.on = on;
    r10f::Profile p;
    if (r10f::parseProfile(pcfilter, &p)) {
        st.role = bitIndex(p.role, r10f::T_Melee);
        st.style = p.style ? 1 + bitIndex(p.style, r10f::T_Single) : 0;
        st.align = p.align ? 1 + bitIndex(p.align, r10f::T_Light) : 0;
    }
    st.feats = modeIndex(r10f::parseMode(ffilter && *ffilter ? ffilter : "all"));
    st.powers = modeIndex(r10f::parseMode(pfilter && *pfilter ? pfilter : "all"));
    return st;
}
// To tokens: pcfilter "role/style/align" into buf (cap >= TokenLen), mode tokens as static strings.
inline void toTokens(const State& st, char* pcfilter, int cap, const char** ffilter, const char** pfilter) {
    if (pcfilter && cap > 0) snprintf(pcfilter, static_cast<size_t>(cap), "%s/%s/%s", roleWord(st.role), styleWord(st.style), alignWord(st.align));
    if (ffilter) *ffilter = r10f::modeName(modeAt(st.feats));
    if (pfilter) *pfilter = r10f::modeName(modeAt(st.powers));
}

} // namespace r10p
