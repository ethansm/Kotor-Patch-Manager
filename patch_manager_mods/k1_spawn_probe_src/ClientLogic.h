// ClientLogic.h -- Pure C++ logic of the k1-spawn-probe client of npc-world (k2.npcworld@1). Header-only, std only.
// No Windows headers, no exceptions, no heap. The POP-STABLE gate, the seen-tag list and the native spawn sequence live in the
// npc-world library now; this header keeps only the roster and the string matching the client needs.
#pragma once

#include <cmath>
#include <cstddef>

namespace k1spawn {

// Fixed-size roster row. A field may be full length (16 or 32 chars) with no NUL terminator.
struct RosterRow {
    char resref[17];
    char tag[33];
    char area[17];
    float x, y, z, facing;
};

// Stage 2 roster (constant; Stage 3 moves it to a file). z = 0.0: every 204tel floor point in the GIT is z 0.0.
inline const RosterRow kRoster[] = {
    {"c_drdmouse01", "MouseDroid", "204tel", -39.0f, -63.1f, 0.0f, 0.0f},        // pet: vanilla mouse droid at WP_com_ith_01
    {"k1p_juhani", "k1p_juhani", "204tel", -37.5f, -63.1f, 0.0f, 0.0f},          // K1 Juhani (appearance 818) 1.5 m from the pet's known-good point
    {"k1p_carth", "k1p_carth", "204tel", -36.0f, -63.1f, 0.0f, 0.0f},            // K1 Carth (appearance 819), 1.5 m beside Juhani; resources come from the juhani-char VFS bundle
    {"k1p_mission", "k1p_mission", "204tel", -40.5f, -63.1f, 0.0f, 0.0f},        // K1 Mission (appearance 823, importer pack): west of the pet; -33.0 was outside the walkmesh (user play 10-18), 204tele.wok checked 10-20 (0.8 m clearance)
    {"k1p_zaalbaru", "k1p_zaalbar", "204tel", -42.5f, -63.1f, 0.0f, 0.0f},       // K1 Zaalbar (appearance 824, importer pack; UTC k1p_zaalbaru, tag k1p_zaalbar): 2 m west of Mission, inside 204tele.wok with 0.8 m clearance
    {"k1p_bastilla", "k1p_bastilla", "204tel", -44.5f, -63.1f, 0.0f, 0.0f},      // K1 Bastilla (appearance 825, importer pack): 2 m west of Zaalbar, inside 204tele.wok with 0.8 m clearance
};
constexpr int kRosterCount = static_cast<int>(sizeof(kRoster) / sizeof(kRoster[0]));
constexpr int kMaxRowsPerArea = 8;

// ASCII case fold. Bytes outside 'A'..'Z' pass through unchanged.
inline char ToLowerAscii(char c) {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

// Case-insensitive compare of at most n chars. Stops at the first NUL in either string and never
// reads index n or later, so an exactly n-char buffer with no terminator is safe. Pointers must be non-null.
inline bool CiEqualsN(const char* a, const char* b, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        const char ca = a[i];
        const char cb = b[i];
        if (ca == '\0' || cb == '\0') return ca == cb;
        if (ToLowerAscii(ca) != ToLowerAscii(cb)) return false;
    }
    return true;
}

// Resref compare: case-insensitive, at most 16 chars. A null pointer on either side gives false.
inline bool ResrefEquals(const char* a, const char* b) {
    if (!a || !b) return false;
    return CiEqualsN(a, b, 16);
}

// Tag compare: case-insensitive, at most 32 chars. A null pointer on either side gives false.
inline bool TagEquals(const char* a, const char* b) {
    if (!a || !b) return false;
    return CiEqualsN(a, b, 32);
}

// Indices (into kRoster) of the rows for `area`, in roster order. Returns the count written (<= cap). A null area or cap <= 0 gives 0.
inline int RowsForArea(const char* area, int* out, int cap) {
    if (!area || !out || cap <= 0) return 0;
    int n = 0;
    for (int i = 0; i < kRosterCount && n < cap; ++i) {
        if (ResrefEquals(kRoster[i].area, area)) out[n++] = i;
    }
    return n;
}

// Tags whose creature is dumped once per process (the NPC dump event): the Stage 1 probe NPC, the Stage 2 native NPC, the pet and the K1 Juhani, Carth, Juhanv, Mission, Zaalbar and Bastilla test rows.
inline constexpr const char* kHarvestTags[] = {"k1p_probe_npc", "k1p_native_npc", "MouseDroid", "k1p_juhani", "k1p_carth", "k1p_juhanv", "k1p_mission", "k1p_zaalbar", "k1p_bastilla"};
constexpr int kHarvestTagCount = static_cast<int>(sizeof(kHarvestTags) / sizeof(kHarvestTags[0]));

inline bool IsHarvestTag(const char* tag) {
    if (!tag || !tag[0]) return false;
    for (int i = 0; i < kHarvestTagCount; ++i) {
        if (TagEquals(kHarvestTags[i], tag)) return true;
    }
    return false;
}

}  // namespace k1spawn
