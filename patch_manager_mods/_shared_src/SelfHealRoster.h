// SelfHealRoster.h -- the character list of the Companion Self-Heal options page (doc 46 MCF4): who can be edited, in what order, under
// which display name. Pure (fixed arrays, no heap / STL, no engine access); host test selfheal_roster_test.cpp.
//
// buildRoster(...) order:
//   0   "Everyone (defaults)"  tag ""            (edits [defaults])
//   1.. the current party, in party order        (tag = the key CompanionSelfHeal uses: shui::leaderKey for the leader, the tag for followers)
//   ..  tags that have a row in the INI but are not in the party
//   ..  the built-in table of known companions   (so a character can be set up before it joins)
// Dedupe is case-insensitive and ignores '_', '-' and ' ' (so bao_dur == BaoDur == bao-dur); the FIRST spelling seen wins and is the key the
// page edits. A "slotN" key (legacy, written when no tag could be read) shows as "Slot N" when no tag is known for it. A party member
// keeps the display name the game gave it; otherwise the built-in table's name; otherwise the tag itself. Tags longer than 23 characters
// cannot round-trip a RosterEntry and are skipped.
#pragma once
#include <stddef.h>
#include <string.h>

namespace shroster {

enum { kTagLen = 24, kNameLen = 32, kMaxRoster = 40 };

struct RosterEntry {
    char tag[kTagLen];     // "" = the defaults entry
    char name[kNameLen];
    bool inParty;
};

struct Known {
    const char* tag;       // engine tag, lowercase
    const char* name;      // display name
};

namespace detail {

inline char lower(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c; }

// Same character after normalisation? Compares a and b skipping '_', '-', ' ' and ignoring case.
inline bool sameTag(const char* a, const char* b) {
    for (;;) {
        while (*a == '_' || *a == '-' || *a == ' ') ++a;
        while (*b == '_' || *b == '-' || *b == ' ') ++b;
        if (!*a || !*b) return !*a && !*b;
        if (lower(*a) != lower(*b)) return false;
        ++a;
        ++b;
    }
}

inline void copy(char* dst, size_t cap, const char* src) {
    size_t i = 0;
    for (; src && src[i] && i + 1 < cap; ++i) dst[i] = src[i];
    dst[i] = 0;
}

// "slot" + digits only -> true and *n.
inline bool slotIndex(const char* tag, int* n) {
    if (!tag || lower(tag[0]) != 's' || lower(tag[1]) != 'l' || lower(tag[2]) != 'o' || lower(tag[3]) != 't' || !tag[4]) return false;
    int v = 0;
    for (const char* p = tag + 4; *p; ++p) {
        if (*p < '0' || *p > '9' || v > 9999) return false;
        v = v * 10 + (*p - '0');
    }
    *n = v;
    return true;
}

inline void slotName(char* out, size_t cap, int n) {
    char t[12];
    int k = 0;
    unsigned u = (unsigned)n;
    do { t[k++] = (char)('0' + u % 10u); u /= 10u; } while (u);
    char tmp[kNameLen];
    size_t o = 0;
    for (const char* p = "Slot "; *p && o + 1 < sizeof tmp; ++p) tmp[o++] = *p;
    while (k && o + 1 < sizeof tmp) tmp[o++] = t[--k];
    tmp[o] = 0;
    copy(out, cap, tmp);
}

inline const char* knownName(const Known* k, int n, const char* tag) {
    for (int i = 0; k && i < n; ++i)
        if (k[i].tag && sameTag(k[i].tag, tag)) return k[i].name;
    return nullptr;
}

inline bool present(const RosterEntry* out, int n, const char* tag) {
    for (int i = 1; i < n; ++i)
        if (sameTag(out[i].tag, tag)) return true;
    return false;
}

// Appends one entry unless the tag is empty / too long / a duplicate or the list is full. Returns the new count.
inline int add(RosterEntry* out, int n, int cap, const char* tag, const char* name, bool inParty, const Known* known, int nKnown) {
    if (n >= cap || !tag || !tag[0] || strlen(tag) >= (size_t)kTagLen || present(out, n, tag)) return n;
    RosterEntry& e = out[n];
    copy(e.tag, sizeof e.tag, tag);
    e.inParty = inParty;
    int slot = 0;
    const char* kn = knownName(known, nKnown, tag);
    if (name && name[0]) copy(e.name, sizeof e.name, name);
    else if (kn) copy(e.name, sizeof e.name, kn);
    else if (slotIndex(tag, &slot)) slotName(e.name, sizeof e.name, slot);
    else copy(e.name, sizeof e.name, tag);
    return n + 1;
}

}  // namespace detail

// partyNames may be null (or hold nulls): the built-in names / the tag are used. Returns the number of entries written (>= 1 when cap >= 1).
inline int buildRoster(const char* const* partyTags, const char* const* partyNames, int nParty, const char* const* iniTags, int nIni, const Known* builtin,
                       int nBuiltin, RosterEntry* out, int cap) {
    if (!out || cap < 1) return 0;
    int n = 0;
    memset(&out[0], 0, sizeof out[0]);
    detail::copy(out[0].name, sizeof out[0].name, "Everyone (defaults)");
    n = 1;
    for (int i = 0; partyTags && i < nParty; ++i)
        n = detail::add(out, n, cap, partyTags[i], partyNames ? partyNames[i] : nullptr, true, builtin, nBuiltin);
    for (int i = 0; iniTags && i < nIni; ++i) n = detail::add(out, n, cap, iniTags[i], nullptr, false, builtin, nBuiltin);
    for (int i = 0; builtin && i < nBuiltin; ++i) n = detail::add(out, n, cap, builtin[i].tag, builtin[i].name, false, builtin, nBuiltin);
    return n;
}

// Index of the entry whose tag matches (same normalisation as the dedupe), else 0 (the defaults entry).
inline int indexOfTag(const RosterEntry* r, int n, const char* tag) {
    if (!tag || !tag[0]) return 0;
    for (int i = 1; r && i < n; ++i)
        if (detail::sameTag(r[i].tag, tag)) return i;
    return 0;
}

}  // namespace shroster
