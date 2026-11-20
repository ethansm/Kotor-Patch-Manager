// DiversityExclusion.h -- tag-based exclusion of companions and story NPCs
//
// Header-only, C++11, no std::string, no heap, no exceptions. Also compiled
// into the Windows MinGW DLL.
//
// ClassifyExclusion() rules, applied in order:
//   1. NULL or empty tag -> None.
//   2. Strip ONE leading area prefix, exactly three digits then '_'
//      ("203_Lorso" -> "Lorso").
//   3. Strip ONE creature prefix: "npc_", "n_" or "c_" (case-insensitive).
//   4. Token rule: the remainder must EQUAL a listed name, or start with it
//      followed by end-of-string, '_' or a digit. So "mira", "mira_2" and
//      "mira01" match; "mirage_guard" and "miranda" do not.
//   5. Legacy exact-prefix names, any suffix allowed: "miramess",
//      "miraqstpris", "g0t0droid". They were excluded by the old prefix rule
//      and are kept on purpose (miramess1, miraqstpris1/2, g0t0droid*).
//
// Only the first 32 characters of a tag are examined. Every rule above
// decides within the first ~20 characters, so the cap never changes a result.
// ClassifyExclusionN() is for unterminated buffers: it reads at most
// min(maxLen, 31) characters.

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace NpcDiversity {

// Case-insensitive prefix helper
inline bool StartsWithCaseInsensitive(const char* str, const char* prefix) {
    if (!str || !prefix) return false;
    while (*prefix) {
        char c1 = *str++;
        char c2 = *prefix++;
        if (c1 >= 'A' && c1 <= 'Z') c1 = static_cast<char>(c1 + ('a' - 'A'));
        if (c2 >= 'A' && c2 <= 'Z') c2 = static_cast<char>(c2 + ('a' - 'A'));
        if (c1 != c2) return false;
    }
    return true;
}

enum class ExcludeKind : uint8_t { None = 0, Companion = 1, Story = 2 };

namespace detail {

// Lowercase names. Token rule (see header comment, rule 4).
const char* const kCompanionNames[] = {
    "kreia", "atton", "bao_dur", "baodur", "mira", "hanharr", "mandalore",
    "visas", "disciple", "handmaiden", "t3m4", "t3_m4", "hk47", "hk_47",
    "g0t0", "g0_t0", "player",
};

const char* const kStoryNames[] = {
    "chodo", "luxa", "grenn", "opo_chano", "opochano", "chano", "lorso",
    "b4d4", "ralon", "moza", "vrook", "zreztik", "geeda", "falt", "batono",
};

// Legacy exact-prefix names (rule 5). Any suffix matches.
const char* const kCompanionLegacyPrefixes[] = {
    "miramess", "miraqstpris", "g0t0droid",
};

// Largest number of tag characters ever examined.
const std::size_t kMaxTagChars = 32;

inline char AsciiLower(char c) {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c;
}

inline bool IsAsciiDigit(char c) {
    return c >= '0' && c <= '9';
}

// True when s[0..len) begins with the NUL-terminated lowercase literal `lit`,
// compared case-insensitively. Never reads at or past s + len.
inline bool BoundedPrefix(const char* s, std::size_t len, const char* lit) {
    std::size_t i = 0;
    for (; lit[i] != '\0'; ++i) {
        if (i >= len || AsciiLower(s[i]) != lit[i]) return false;
    }
    return true;
}

// Token rule: s[0..len) equals `name`, or starts with `name` followed by
// end-of-string, '_' or a digit.
inline bool TokenMatch(const char* s, std::size_t len, const char* name) {
    if (!BoundedPrefix(s, len, name)) return false;
    std::size_t n = 0;
    while (name[n] != '\0') ++n;
    if (n == len) return true;
    const char next = s[n];
    return next == '_' || IsAsciiDigit(next);
}

template <std::size_t N>
inline bool AnyTokenMatch(const char* s, std::size_t len, const char* const (&names)[N]) {
    for (std::size_t i = 0; i < N; ++i) {
        if (TokenMatch(s, len, names[i])) return true;
    }
    return false;
}

template <std::size_t N>
inline bool AnyPrefixMatch(const char* s, std::size_t len, const char* const (&names)[N]) {
    for (std::size_t i = 0; i < N; ++i) {
        if (BoundedPrefix(s, len, names[i])) return true;
    }
    return false;
}

// Classifies the first `len` characters of s. s need not be NUL-terminated.
inline ExcludeKind ClassifyBounded(const char* s, std::size_t len) {
    // Rule 2: one area prefix, three digits then '_'.
    if (len >= 4 && IsAsciiDigit(s[0]) && IsAsciiDigit(s[1]) &&
        IsAsciiDigit(s[2]) && s[3] == '_') {
        s += 4;
        len -= 4;
    }

    // Rule 3: one creature prefix. "npc_" is checked before "n_".
    if (BoundedPrefix(s, len, "npc_")) {
        s += 4;
        len -= 4;
    } else if (BoundedPrefix(s, len, "n_") || BoundedPrefix(s, len, "c_")) {
        s += 2;
        len -= 2;
    }

    if (len == 0) return ExcludeKind::None;

    // Rules 4 and 5.
    if (AnyTokenMatch(s, len, kCompanionNames) ||
        AnyPrefixMatch(s, len, kCompanionLegacyPrefixes)) {
        return ExcludeKind::Companion;
    }
    if (AnyTokenMatch(s, len, kStoryNames)) {
        return ExcludeKind::Story;
    }
    return ExcludeKind::None;
}

} // namespace detail

// Classifies a NUL-terminated tag. Examines at most 32 characters.
inline ExcludeKind ClassifyExclusion(const char* tag) {
    if (!tag) return ExcludeKind::None;
    std::size_t len = 0;
    while (len < detail::kMaxTagChars && tag[len] != '\0') ++len;
    return detail::ClassifyBounded(tag, len);
}

// Classifies an unterminated buffer. Copies at most min(maxLen, 31) characters
// into a local NUL-terminated buffer and classifies that copy. Never reads
// tag[maxLen] or beyond.
inline ExcludeKind ClassifyExclusionN(const char* tag, std::size_t maxLen) {
    if (!tag) return ExcludeKind::None;
    const std::size_t cap = (maxLen < 31) ? maxLen : 31;
    char buf[32];
    std::size_t n = 0;
    while (n < cap && tag[n] != '\0') {
        buf[n] = tag[n];
        ++n;
    }
    buf[n] = '\0';
    return ClassifyExclusion(buf);
}

// Compatibility wrapper: true for any companion or story tag.
inline bool IsExcludedFromDiversification(const char* tag) {
    return ClassifyExclusion(tag) != ExcludeKind::None;
}

} // namespace NpcDiversity
