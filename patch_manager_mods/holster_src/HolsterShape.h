#pragma once
// HolsterShape.h: per-model shape lookup for the holstered-weapons mod (HW30 / C2b; doc 42).
// Pure header: no heap, no std::string, no Win32 types. Table = HolsterShape.inc (generated, read-only here).
// Build/run: python3 scripts/run_tests.py holster_shape
// ShapeEntry (model space, metres): axis = long axis index (0 x, 1 y, 2 z), len = extent on it, c = bbox centre.
// exitSign = direction along +axis (1) or -axis (-1) the weapon leaves its mount on a draw; 0 = unknown, no extract
// offset (HW31 must then skip the offset). exitLen = travel needed to clear the mount. Values come from the authored
// family table in scripts/gen_holster_shape.py; the -1 rows there are marked GUESS.

namespace hs {

#include "HolsterShape.inc"   // struct ShapeEntry, kShapes[], kShapeCount (strictly sorted, byte order)

// Named shape* on purpose: HolsterLogic.h defines hs::lower and hs::stemIs, and Holster.cpp includes both headers.
inline char shapeLower(char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c; }

// Byte compare of two lowercase strings: <0, 0, >0 (like strcmp, unsigned).
inline int shapeCmp(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return static_cast<int>(static_cast<unsigned char>(*a)) - static_cast<int>(static_cast<unsigned char>(*b));
}

// True when s begins with the whole of p (both lowercase).
inline bool shapeHasPrefix(const char* s, const char* p) {
    while (*p) {
        if (*s != *p) return false;
        ++s; ++p;
    }
    return true;
}

// Families with no family fallback. Rule (r2 Q6.2, .claude/research/holster_hw29/r2_stems.md:81): a family whose
// row lengths spread more than 5% ((max - min) / max) gets no fallback row, so a _NNN miss stays nullptr.
// holster_shape_test.cpp re-derives this list from kShapes and fails when the generated table drifts from it.
// Each key ends in '_' so it compares against the family prefix directly.
static const char* const kNoFallback[] = {
    "w_blstrcrbn_", "w_blstrpstl_", "w_blstrrfl_", "w_dsrptrfl_", "w_hvrptbltr_",
    "w_ionrfl_", "w_qtrstaff_", "w_rptnblstr_", "w_sonicrfl_", "w_stunbaton_"};
static const int kNoFallbackCount = static_cast<int>(sizeof(kNoFallback) / sizeof(kNoFallback[0]));

// nullptr for null, empty, or 32+ characters. Exact match first; on a miss, a stem ending in _NNN falls back to its
// family's first row. The key buffer caps lookups at 31 characters (a buffer bound, not a game limit: tuneCopyName
// copies 16, so production callers never reach it). A miss never changes the category; callers keep classify().
inline const ShapeEntry* shapeOf(const char* stem) {
    if (!stem || !*stem) return nullptr;
    char key[32];
    int n = 0;
    while (stem[n]) {
        if (n == 31) return nullptr;
        key[n] = shapeLower(stem[n]);
        ++n;
    }
    key[n] = 0;
    const int count = static_cast<int>(kShapeCount);
    int lo = 0, hi = count - 1;
    while (lo <= hi) {
        int m = (lo + hi) / 2;
        int c = shapeCmp(kShapes[m].stem, key);
        if (c == 0) return &kShapes[m];
        if (c < 0) lo = m + 1; else hi = m - 1;
    }
    // family: trailing digits must follow '_', and the family name must be non-empty
    int d = n;
    while (d > 0 && key[d - 1] >= '0' && key[d - 1] <= '9') --d;
    if (d == n || d < 2 || key[d - 1] != '_') return nullptr;
    key[d] = 0;                                  // key is now "family_"
    for (int i = 0; i < kNoFallbackCount; ++i)
        if (shapeCmp(kNoFallback[i], key) == 0) return nullptr;
    lo = 0; hi = count;                          // lower bound of the prefix; family rows are contiguous
    while (lo < hi) {
        int m = (lo + hi) / 2;
        if (shapeCmp(kShapes[m].stem, key) < 0) lo = m + 1; else hi = m;
    }
    if (lo < count && shapeHasPrefix(kShapes[lo].stem, key)) return &kShapes[lo];
    return nullptr;
}

} // namespace hs
