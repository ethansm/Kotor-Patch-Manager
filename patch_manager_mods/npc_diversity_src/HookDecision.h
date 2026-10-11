// HookDecision.h -- pure, Linux-testable decisions for the NPC Diversity hook.
// No engine calls and no engine memory access. The descriptor pointer overload only dereferences
// the caller's pointer.
#pragma once

#include <cstddef>
#include <cstdint>

#include "DiversityTables.h"
#include "NpcDivLayout.h"

namespace NpcDiversity {

enum class SkipReason : uint8_t { None = 0, NullDesc, DescMode, Gender, NotCommonerRow };

// Order of checks: DescMode (descMode14 != 0), then Gender (gender > 1), then NotCommonerRow
// (row in none of the generic-commoner tables), then Gender again when the row's class does not
// match the gender (male tables and Twi'lek male need 0, female tables and Twi'lek female need 1).
inline SkipReason ShouldSkipDescriptor(int32_t descMode14, uint8_t gender, uint16_t appearance) {
    if (descMode14 != 0) return SkipReason::DescMode;
    if (gender > 1) return SkipReason::Gender;
    const bool maleClass = IsGenericMaleCommoner(appearance) || IsGenericTwilekMale(appearance);
    const bool femaleClass = IsGenericFemaleCommoner(appearance) || IsGenericTwilekFemale(appearance);
    if (!maleClass && !femaleClass) return SkipReason::NotCommonerRow;
    const uint8_t wantGender = maleClass ? uint8_t{0} : uint8_t{1};
    return gender == wantGender ? SkipReason::None : SkipReason::Gender;
}

// Descriptor overload: a null pointer is its own skip reason.
inline SkipReason ShouldSkipDescriptor(const layout::AppearanceDesc* d) {
    if (!d) return SkipReason::NullDesc;
    return ShouldSkipDescriptor(d->descMode14, d->gender, d->appearance);
}

// Texture seed for the hook's fallback path. A stash (haveStash) always wins, even when it is 0.
// Otherwise: FNV-1a over the 8 bytes of (pDescPtr, retAddr), least significant byte first.
// The result depends only on the two arguments. Callers must pass a pDescPtr that is stable across
// calls; a stack-resident descriptor would change the seed from call to call.
inline uint32_t ResolveTextureSeed(bool haveStash, uint32_t stashHash, uint32_t pDescPtr, uint32_t retAddr) {
    if (haveStash) return stashHash;
    constexpr uint32_t kOffsetBasis = 2166136261u;
    constexpr uint32_t kPrime = 16777619u;
    uint32_t h = kOffsetBasis;
    const uint32_t words[2] = {pDescPtr, retAddr};
    for (uint32_t w : words) {
        for (int shift = 0; shift < 32; shift += 8) {
            h ^= (w >> shift) & 0xFFu;
            h *= kPrime;
        }
    }
    return h;
}

// True when the 16 bytes look like a real resref: a non-NUL first byte, printable ASCII (0x20..0x7E)
// up to the first NUL, and only NUL bytes after it. A full 16-char name with no NUL is accepted.
inline bool IsPlausibleResRef(const char* p16) {
    if (!p16 || p16[0] == '\0') return false;
    bool sawNul = false;
    for (size_t i = 0; i < 16; ++i) {
        const unsigned char c = static_cast<unsigned char>(p16[i]);
        if (c == 0) {
            sawNul = true;
            continue;
        }
        if (sawNul) return false;  // character after the terminator
        if (c < 0x20 || c > 0x7E) return false;
    }
    return true;
}

} // namespace NpcDiversity
