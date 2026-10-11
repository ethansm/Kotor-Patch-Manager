// DiversityLogic.h -- Pure C++ deterministic selection logic for Master NPC Diversity Dispatcher
#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>

#include "DiversityRows.h"
#include "DiversityExclusion.h"
#include "DiversityTables.h"

namespace NpcDiversity {

// FNV-1a 32-bit hash constants
constexpr uint32_t FNV_OFFSET_BASIS = 2166136261u;
constexpr uint32_t FNV_PRIME = 16777619u;

// Selection percentages (per generic NPC class). Each is applied to BandRoll(hash).
// Kept here, not in DiversityRows.h (generated).
constexpr uint32_t kTwilekMaleRemapPct = 80;  // male Twi'lek: remapped to 805..808, rest unchanged
constexpr uint32_t kFatPct = 15;              // male and female commoners: fat body
constexpr uint32_t kCzerkaPct = 5;            // female commoner: Czerka officer, after the fat band
constexpr uint32_t kDancerPct = 20;           // female Twi'lek: dancer variants

// Commoner clothing texture slots N_CommM%02d / N_CommF%02d. Any content works;
// ComputeSlot indexes this array, so removing an entry only shrinks the pool.
constexpr uint8_t kCommonerSlots[] = {
    1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 32, 33, 34, 35};
constexpr size_t kCommonerSlotCount = sizeof(kCommonerSlots) / sizeof(kCommonerSlots[0]);

// Murmur3 32-bit finaliser (fmix32). Bijective, full avalanche: every input bit
// flips about half the output bits. Used to turn a low-entropy FNV state into a
// well-mixed value before taking a modulus.
inline uint32_t Fmix32(uint32_t h) {
    h ^= h >> 16;
    h *= 0x85ebca6bu;
    h ^= h >> 13;
    h *= 0xc2b2ae35u;
    h ^= h >> 16;
    return h;
}

// Salted draw: Mix(hash, salt) = Fmix32(hash ^ (salt * 0x9E3779B1)).
// Each independent decision uses its own salt so draws do not share bits:
//   salt 1 = band roll (BandRoll), 2 = Twi'lek male variant, 3 = female dancer variant,
//   salt 4 = commoner texture slot (ComputeSlot).
inline uint32_t Mix(uint32_t hash, uint32_t salt) {
    return Fmix32(hash ^ (salt * 0x9E3779B1u));
}

// Percentage roll in [0, 100) for the class-level remap decisions (salt 1).
inline uint32_t BandRoll(uint32_t hash) {
    return Mix(hash, 1) % 100u;
}

// FNV-1a over the 8 bytes of (ptr, ret). Fallback seed when a per-creature stash is absent.
inline uint32_t HashSeedFallback(uint32_t ptr, uint32_t ret) {
    uint32_t hash = FNV_OFFSET_BASIS;
    for (int i = 0; i < 4; ++i) {
        hash = (hash ^ static_cast<uint8_t>((ptr >> (8 * i)) & 0xFFu)) * FNV_PRIME;
    }
    for (int i = 0; i < 4; ++i) {
        hash = (hash ^ static_cast<uint8_t>((ret >> (8 * i)) & 0xFFu)) * FNV_PRIME;
    }
    return hash;
}

// Fallback deterministic descriptor hash when coordinates or tags are not present
inline uint32_t HashDescriptor(uint16_t appearance, uint8_t gender, uint8_t textureVar, uint32_t seed = 0) {
    uint32_t hash = FNV_OFFSET_BASIS;
    hash = (hash ^ static_cast<uint8_t>(appearance & 0xFF)) * FNV_PRIME;
    hash = (hash ^ static_cast<uint8_t>((appearance >> 8) & 0xFF)) * FNV_PRIME;
    hash = (hash ^ static_cast<uint8_t>(gender)) * FNV_PRIME;
    hash = (hash ^ static_cast<uint8_t>(textureVar)) * FNV_PRIME;
    if (seed != 0) {
        hash = (hash ^ (seed & 0xFF)) * FNV_PRIME;
        hash = (hash ^ ((seed >> 8) & 0xFF)) * FNV_PRIME;
        hash = (hash ^ ((seed >> 16) & 0xFF)) * FNV_PRIME;
        hash = (hash ^ ((seed >> 24) & 0xFF)) * FNV_PRIME;
    }
    return hash;
}

// Map a hash to a commoner texture slot (salt 4), chosen from kCommonerSlots
inline int ComputeSlot(uint32_t hash) {
    return static_cast<int>(kCommonerSlots[Mix(hash, 4) % kCommonerSlotCount]);
}

// Check if a texture ResRef belongs to generic commoner clothing
inline bool IsCommonerTexture(const char* resRef, bool& isFemale) {
    if (!resRef) return false;
    if (StartsWithCaseInsensitive(resRef, "n_commf")) {
        isFemale = true;
        return true;
    }
    if (StartsWithCaseInsensitive(resRef, "n_commm")) {
        isFemale = false;
        return true;
    }
    return false;
}

// Determines if texture should be diversified and writes the new ResRef name
inline bool ShouldDiversifyTexture(const char* currentTex, uint32_t hash, char* outTex, size_t outSize) {
    if (!currentTex || !outTex || outSize < 16) return false;

    bool isFemale = false;
    if (!IsCommonerTexture(currentTex, isFemale)) {
        return false;
    }

    int slot = ComputeSlot(hash);
    if (isFemale) {
        std::snprintf(outTex, outSize, "N_CommF%02d", slot);
    } else {
        std::snprintf(outTex, outSize, "N_CommM%02d", slot);
    }
    return true;
}

// Quantise a coordinate to 0.1 m units (v * 10, truncated).
// NaN maps to 0. Values are clamped to +/-1.0e6 first, so the float->int cast is always in range
// (|v * 10| <= 1e7) and +/-inf and huge values give a fixed, deterministic bucket.
inline int32_t QuantizeCoord(float v) {
    if (!(v == v)) return 0;  // NaN
    constexpr float kLimit = 1.0e6f;
    if (v > kLimit) v = kLimit;
    if (v < -kLimit) v = -kLimit;
    return static_cast<int32_t>(v * 10.0f);
}

// Hash creature position (quantized x*10, y*10) with objectId and appearance using FNV-1a.
// This is the per-creature hash used by the dispatcher.
inline uint32_t HashCreaturePosition(float x, float y, uint32_t objectId, uint16_t appearance) {
    uint32_t hash = FNV_OFFSET_BASIS;

    int32_t qx = QuantizeCoord(x);
    int32_t qy = QuantizeCoord(y);

    hash = (hash ^ static_cast<uint32_t>(qx & 0xFFFF)) * FNV_PRIME;
    hash = (hash ^ static_cast<uint32_t>((qx >> 16) & 0xFFFF)) * FNV_PRIME;
    hash = (hash ^ static_cast<uint32_t>(qy & 0xFFFF)) * FNV_PRIME;
    hash = (hash ^ static_cast<uint32_t>((qy >> 16) & 0xFFFF)) * FNV_PRIME;

    hash = (hash ^ static_cast<uint8_t>(objectId & 0xFF)) * FNV_PRIME;
    hash = (hash ^ static_cast<uint8_t>((objectId >> 8) & 0xFF)) * FNV_PRIME;
    hash = (hash ^ static_cast<uint8_t>((objectId >> 16) & 0xFF)) * FNV_PRIME;
    hash = (hash ^ static_cast<uint8_t>((objectId >> 24) & 0xFF)) * FNV_PRIME;

    hash = (hash ^ static_cast<uint8_t>(appearance & 0xFF)) * FNV_PRIME;
    hash = (hash ^ static_cast<uint8_t>((appearance >> 8) & 0xFF)) * FNV_PRIME;

    return hash;
}

// Remap appearance based on hash for diversity distribution.
// gender: 0 = male, 1 = female. A row whose gender does not match is returned unchanged.
// Every decision uses its own salted draw (see Mix); no bits are reused across decisions.
inline uint16_t RemapAppearance(uint16_t currentApp, uint8_t gender, uint32_t hash) {
    if (IsGenericMaleCommoner(currentApp)) {
        if (gender != 0) return currentApp;
        // kFatPct% fat, rest unchanged
        if (BandRoll(hash) < kFatPct) return APP_COMMONER_FAT_MALE;
        return currentApp;
    }
    if (IsGenericFemaleCommoner(currentApp)) {
        if (gender != 1) return currentApp;
        uint32_t roll = BandRoll(hash);
        if (roll < kFatPct) return APP_COMMONER_FAT_FEMALE;
        if (roll < kFatPct + kCzerkaPct) return APP_CZERKA_OFFICER_FEMALE;
        return currentApp;
    }
    if (IsGenericTwilekMale(currentApp)) {
        if (gender != 0) return currentApp;
        if (BandRoll(hash) >= kTwilekMaleRemapPct) return currentApp;
        switch (Mix(hash, 2) % 4u) {
            case 0: return APP_TWILEK_COMMONER_M_02_A;
            case 1: return APP_TWILEK_COMMONER_M_02_B;
            case 2: return APP_TWILEK_COMMONER_M_02_C;
            default: return APP_TWILEK_COMMONER_M_02_D;
        }
    }
    if (IsGenericTwilekFemale(currentApp)) {
        if (gender != 1) return currentApp;
        if (BandRoll(hash) >= kDancerPct) return currentApp;
        switch (Mix(hash, 3) % 3u) {
            case 0: return APP_TWILEK_FEMALE_DANCER_BOOT;
            case 1: return APP_TWILEK_FEMALE_DANCER;
            default: return APP_TWILEK_FEMALE_UNDERWEAR;
        }
    }
    // Any other appearance: unchanged
    return currentApp;
}

} // namespace NpcDiversity
