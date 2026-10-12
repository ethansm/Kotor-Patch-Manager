// DiversityKeep.h -- keep-default lookup for the generic commoner rows
//
// Header-only, C++17, no std::string, no heap, no exceptions. Also compiled
// into the Windows MinGW DLL.
//
// A creature is keep-default (never swapped for a diversity variant) when:
//   1. Its tag equals one of roles::kKeepTags[] (whole-string, ASCII case-insensitive), or
//   2. Its position lies within roles::kKeepPosTol10 (0.1 m units) on both axes
//      of one of roles::kKeepPositions[]. Only used when the caller says the
//      position is trustworthy (xyOk).
//
// Only the first 32 characters of a tag are examined. Every stored keep tag is
// at most 16 characters, so a longer tag can never match and the cap is exact.

#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "DiversityRoles.h"
#include "DiversityExclusion.h"

namespace NpcDiversity {

// Largest number of tag characters examined.
constexpr std::size_t kKeepTagMaxChars = 32;

// Positions with |x| or |y| above this (0.1 m units, i.e. 10 km) are rejected
// before any float-to-int conversion.
constexpr float kKeepPosMaxAbs = 1.0e5f;

// Whole-string, ASCII case-insensitive match against roles::kKeepTags[].
inline bool IsKeepDefaultTag(const char* tag) {
    if (!tag || tag[0] == '\0') return false;

    char lower[kKeepTagMaxChars + 1];
    std::size_t n = 0;
    for (; tag[n] != '\0' && n < kKeepTagMaxChars; ++n) {
        lower[n] = detail::AsciiLower(tag[n]);
    }
    // Still characters left after 32: longer than any keep tag.
    if (tag[n] != '\0') return false;
    lower[n] = '\0';

    for (std::size_t i = 0; i < roles::kKeepTagCount; ++i) {
        if (std::strcmp(lower, roles::kKeepTags[i]) == 0) return true;
    }
    return false;
}

// True when (x, y) in metres lies within roles::kKeepPosTol10 on both axes of
// a stored keep position. NaN, infinity and out-of-range values are rejected
// before the int cast.
inline bool IsKeepDefaultPos(float x, float y) {
    if (!std::isfinite(x) || !std::isfinite(y)) return false;
    if (std::fabs(x) > kKeepPosMaxAbs || std::fabs(y) > kKeepPosMaxAbs) return false;

    const int32_t qx = static_cast<int32_t>(std::floor(x * 10.0f + 0.5f));
    const int32_t qy = static_cast<int32_t>(std::floor(y * 10.0f + 0.5f));
    const int32_t tol = roles::kKeepPosTol10;

    for (std::size_t i = 0; i < roles::kKeepPositionCount; ++i) {
        const int32_t dx = qx - roles::kKeepPositions[i].x10;
        const int32_t dy = qy - roles::kKeepPositions[i].y10;
        if (dx >= -tol && dx <= tol && dy >= -tol && dy <= tol) return true;
    }
    return false;
}

// Tag match, or (when xyOk) position match.
inline bool IsKeepDefault(const char* tag, float x, float y, bool xyOk) {
    return IsKeepDefaultTag(tag) || (xyOk && IsKeepDefaultPos(x, y));
}

} // namespace NpcDiversity
