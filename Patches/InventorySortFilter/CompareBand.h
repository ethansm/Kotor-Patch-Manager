// CompareBand.h - pure, engine-free decision helpers for the inventory compare band (S10-b).
#pragma once
#include <string.h>
#include <stdio.h>

namespace cband {

struct Key {
    unsigned panel = 0, rawId = 0, creature = 0;
    unsigned slot[7] = {0, 0, 0, 0, 0, 0, 0};   // right, left, armor, head, gauntlet, implant, belt item ids (no stack count)
    bool operator==(const Key& o) const {
        if (panel != o.panel || rawId != o.rawId || creature != o.creature) return false;
        for (int i = 0; i < 7; ++i) if (slot[i] != o.slot[i]) return false;
        return true;
    }
};

inline bool shouldShow(bool cfgOn, unsigned creature, unsigned rawId, bool factsOk, unsigned nBand) {
    return cfgOn && creature != 0 && rawId != 0 && rawId != 0x7F000000u && factsOk && nBand > 0;
}

inline bool changed(const Key& last, bool lastValid, const Key& now) { return !lastValid || !(last == now); }

// Copies band[idx] when idx < nBand, else writes "". dst is always null-terminated within dstSize.
inline void lineOrEmpty(char* dst, unsigned dstSize, const char (*band)[32], unsigned nBand, unsigned idx) {
    if (!dst || dstSize == 0) return;
    dst[0] = 0;
    if (!band || idx >= nBand) return;
    unsigned i = 0;
    for (; i + 1 < dstSize && i < 32 && band[idx][i]; ++i) dst[i] = band[idx][i];
    dst[i] = 0;
}

inline unsigned labelsToShow(bool show, unsigned nBand) { return !show ? 0u : (nBand < 2u ? nBand : 2u); }


// ---- compare box (replaces the two band labels): row text -> name | value split and colour kind (pure, host-tested) ----
enum RowKind { RK_Plain = 0, RK_Header = 1, RK_Up = 2, RK_Down = 3, RK_Dim = 4 };

// Index where the value column starts for a "<name> <from> -> <to> (...)" row (the space after the name is skipped), or 0 = no split
// (single label). The value starts at the first space-separated token that begins with a digit, "none" or "--" and lies before " -> ".
inline unsigned splitName(const char* t) {
    if (!t) return 0;
    const char* arrow = strstr(t, " -> ");
    if (!arrow) return 0;
    for (const char* p = t; p < arrow; ++p) {
        if (*p != ' ' || p == t) continue;
        const char c = p[1];
        if (memchr(t, ':', static_cast<size_t>(p - t))) return 0;   // "Changed: Regen 1 -> Regen 2": prop rows carry a kind prefix and are one label (review r03 F1)
        if ((c >= '0' && c <= '9') || !strncmp(p + 1, "none", 4) || !strncmp(p + 1, "--", 2)) return static_cast<unsigned>(p + 1 - t);
    }
    return 0;
}

inline RowKind rowKind(const char* t, bool isHeader) {
    if (!t || !t[0]) return RK_Plain;
    if (isHeader) return RK_Header;
    if (strstr(t, "(changed)")) return RK_Plain;   // direction unknown (penalty/magnitude rows): never red or gold
    if (strstr(t, "(up") || strstr(t, "(+")) return RK_Up;
    if (strstr(t, "(down") || strstr(t, "(-")) return RK_Down;
    if (t[0] == '+' && t[1] == ' ') return RK_Up;     // "+ Damage Immunity: Cold 30%" (gained prop)
    if (t[0] == '-' && t[1] == ' ') return RK_Down;
    if (!strncmp(t, "Also unequips", 13)) return RK_Down;   // a two-hander displaces the left-hand item
    if (!strncmp(t, "Cannot equip", 12) || !strncmp(t, "Can't equip", 11)) return RK_Down;   // production text: "Can't equip this" (ItemCompare.h)
    if (!strncmp(t, "Not compared", 12) || strstr(t, " not compared") || strstr(t, " more")) return RK_Dim;   // production: "N props not compared"
    if ((t[0] == '+' || t[0] == '-') && t[1] >= '0' && t[1] <= '9') return t[0] == '+' ? RK_Up : RK_Down;   // net stat rows: "+2 Attack", "-1 Deflection"   // "- Damage Bonus: Bludgeoning 2" (lost prop)
    return RK_Plain;
}

// ---- block display lines: a title row "<Equipped> vs <Viewed>" on top ----
// itemcmp block = ["vs Gloves: Name", row1, row2, ...] (single target) or ["vs Right Hand: A", "vs Left Hand: B", rows...] (multi).
// Single target: the header line is replaced by one title row "<equipped name> vs <viewed name>" (empty slot: "Empty vs <viewed>"), then the
// body rows follow, so the row count is unchanged. Each name is cut with "..." to share kTitleChars (a name that fits keeps its full length).
// Multi target: every "vs " is dropped and the slot kept ("Right Hand: A"), no title. hdr stays empty (the right-hand label is unused).
constexpr unsigned kTitleChars = 48;
inline bool startsVs(const char* t) { return t && t[0] == 'v' && t[1] == 's' && t[2] == ' '; }
inline void cutDots(char* dst, unsigned n, const char* src, unsigned maxChars) {
    const unsigned len = static_cast<unsigned>(strlen(src));
    if (len <= maxChars) { snprintf(dst, n, "%s", src); return; }
    unsigned c = maxChars > 3 ? maxChars - 3 : 0; while (c > 0 && src[c - 1] == ' ') --c;
    snprintf(dst, n, "%.*s...", static_cast<int>(c), src);
}
inline void titleOf(const char* viewed, const char* header, char* out, unsigned n) {
    char eq[64];
    const char* p = startsVs(header) ? header + 3 : header;
    const char* colon = strstr(p, ": ");
    const char* name = colon ? colon + 2 : p;
    snprintf(eq, sizeof eq, "%s", !strcmp(name, "empty") ? "Empty" : name);
    const unsigned avail = kTitleChars - 4;   // " vs "
    const unsigned vl = static_cast<unsigned>(strlen(viewed)), el = static_cast<unsigned>(strlen(eq));
    unsigned vmax = vl, emax = el;
    if (vl + el > avail) {   // the shorter name keeps up to half, the other gets the rest
        const unsigned half = avail / 2;
        if (vl <= half) emax = avail - vl; else if (el <= half) vmax = avail - el; else { vmax = half; emax = avail - half; }
    }
    char a[64], b[64];
    cutDots(a, sizeof a, viewed, vmax); cutDots(b, sizeof b, eq, emax);
    snprintf(out, n, "%s vs %s", b, a);   // equipped -> selected, the order of the stat arrows
}
inline unsigned displayLines(const char (*in)[64], unsigned n, char (*out)[64], unsigned maxOut, char* hdr, const char* viewed) {
    hdr[0] = 0;
    if (!n || !maxOut) return 0;
    unsigned no = 0;
    const bool single = n >= 2 && startsVs(in[0]) && !startsVs(in[1]);
    if (single && viewed && viewed[0]) {
        titleOf(viewed, in[0], out[no++], 64);
        for (unsigned i = 1; i < n && no < maxOut; ++i) snprintf(out[no++], 64, "%s", in[i]);
        return no;
    }
    for (unsigned i = 0; i < n && no < maxOut; ++i) snprintf(out[no++], 64, "%s", startsVs(in[i]) ? in[i] + 3 : in[i]);
    return no;
}

// Name column width: widest split name (px, already padded) capped at cap; none measured -> cap.
inline int nameColumnW(const int* w, unsigned n, int cap) {
    int m = 0;
    for (unsigned i = 0; i < n; ++i) if (w && w[i] > m) m = w[i];
    return (m == 0 || m > cap) ? cap : m;
}

}  // namespace cband
