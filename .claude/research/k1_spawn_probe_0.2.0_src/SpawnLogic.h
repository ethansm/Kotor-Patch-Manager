// SpawnLogic.h -- Pure C++ spawn gating for the K1 spawn probe hook. Header-only, std only.
// No Windows headers, no exceptions, no heap. Everything is deterministic except SpawnState,
// a small value type the hook owns for one area load.
#pragma once

#include <cmath>
#include <cstddef>

namespace k1spawn {

// Consecutive zero-add ticks an area must hold before a spawn is allowed.
constexpr int kStableTicks = 30;

// Fixed-size roster row. A field may be full length (16 or 32 chars) with no NUL terminator.
struct RosterRow {
    char resref[17];
    char tag[33];
    char area[17];
    float x, y, z, facing;
};

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

// Per-area-load spawn gate. Arm() on each load; Tick() once per frame with the creature adds seen
// since the previous tick; ShouldSpawn() once the area has been quiet long enough.
class SpawnState {
public:
    // Per-load reset: clears the stable run and the spawned flag, stores up to 16 chars of the area.
    // A null areaResref arms the empty area.
    void Arm(const char* areaResref) {
        stableTicks_ = 0;
        spawned_ = false;
        std::size_t n = 0;
        if (areaResref) {
            while (n < 16 && areaResref[n] != '\0') {
                armedArea_[n] = areaResref[n];
                ++n;
            }
        }
        armedArea_[n] = '\0';
    }

    // Any add resets the stable run. Quiet ticks count up and saturate at kStableTicks.
    void Tick(int addsSinceLastTick) {
        if (addsSinceLastTick != 0) {
            stableTicks_ = 0;
        } else if (stableTicks_ < kStableTicks) {
            ++stableTicks_;
        }
    }

    // True when the area has been quiet for kStableTicks ticks, targetArea is the armed area,
    // and no spawn has been marked since the last Arm().
    bool ShouldSpawn(const char* targetArea) const {
        return stableTicks_ >= kStableTicks && !spawned_ && ResrefEquals(armedArea_, targetArea);
    }

    void MarkSpawned() { spawned_ = true; }

    int StableTicks() const { return stableTicks_; }
    bool Spawned() const { return spawned_; }

private:
    int stableTicks_ = 0;
    bool spawned_ = false;
    char armedArea_[17] = {};
};

struct Vec3 {
    float x, y, z;
};

// Unit facing vector in the XY plane: (cos f, sin f, 0). A non-finite facing gives (1, 0, 0),
// so the result is always unit length.
inline Vec3 OrientationFromFacing(float facingRadians) {
    if (!std::isfinite(facingRadians)) return Vec3{1.0f, 0.0f, 0.0f};
    return Vec3{std::cos(facingRadians), std::sin(facingRadians), 0.0f};
}

// True when getTagAt(i) for i in [0, count) equals wantTag (case-insensitive, up to 32 chars).
// getTagAt returns const char*; null entries are skipped. A null wantTag or count <= 0 returns
// false without calling getTagAt.
template <class Cb>
bool ExistsByTag(int count, Cb getTagAt, const char* wantTag) {
    if (!wantTag || count <= 0) return false;
    for (int i = 0; i < count; ++i) {
        const char* tag = getTagAt(i);
        if (tag && CiEqualsN(tag, wantTag, 32)) return true;
    }
    return false;
}

}  // namespace k1spawn
