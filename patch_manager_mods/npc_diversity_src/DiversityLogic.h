// DiversityLogic.h -- Pure C++ deterministic selection logic for Master NPC Diversity Dispatcher
#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>

#include "DiversityRows.h"
#include "DiversitySlots.h"
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
constexpr uint32_t kDirtyPct = 10;            // male and female commoners: dirty clothes, after the earlier bands
constexpr uint32_t kProtocolPct = 5;          // female commoner: Czerka protocol officer, after the dirty band
constexpr uint32_t kTwilekToneRemapPct = 30;  // female Twi'lek: skin tone A/B/C, after the dancer band
constexpr uint32_t kVaryPct = 35;             // share of creatures that get a texture variant (ShouldVary)

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
//   salt 4 = texture slot (PickTextureVar, ShouldDiversifyTexture), 5 = vary gate (ShouldVary),
//   salt 6 = female Twi'lek skin tone.
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

// Per-creature hash for the texture variant: FNV-1a over the 4 bytes of oid, then the 2 bytes of
// appearance (each least significant byte first), finished with Fmix32.
inline uint32_t HashCreatureId(uint32_t oid, uint16_t appearance) {
    uint32_t hash = FNV_OFFSET_BASIS;
    for (int i = 0; i < 4; ++i) {
        hash = (hash ^ static_cast<uint8_t>((oid >> (8 * i)) & 0xFFu)) * FNV_PRIME;
    }
    for (int i = 0; i < 2; ++i) {
        hash = (hash ^ static_cast<uint8_t>((appearance >> (8 * i)) & 0xFFu)) * FNV_PRIME;
    }
    return Fmix32(hash);
}

// True for rows that take a commoner texture variant: the generic male and female commoner tables,
// plus the appended rows 802 APP_COMMONER_FAT_MALE, 803 APP_COMMONER_FAT_FEMALE,
// 805..808 APP_TWILEK_COMMONER_M_02_A..D (male Twi'lek commoner variants) and
// 811 APP_TWILEK_FEMALE_UNDERWEAR. Excluded: 804 APP_CZERKA_OFFICER_FEMALE,
// 809/810 APP_TWILEK_FEMALE_DANCER_BOOT/APP_TWILEK_FEMALE_DANCER (dancers) and the Track A rows 812..817
// (APP_COMMONER_DIRTY_MALE/FEMALE, APP_CZERKA_PROTOCOL_OFFICER_FEMALE, APP_TWILEK_FEMALE_TONE_A..C), which
// ship one texture file each. Every other row is excluded.
inline bool TexVarEligible(uint16_t finalApp) {
    if (IsGenericMaleCommoner(finalApp) || IsGenericFemaleCommoner(finalApp)) return true;
    switch (finalApp) {
        case APP_COMMONER_FAT_MALE:
        case APP_COMMONER_FAT_FEMALE:
        case APP_TWILEK_COMMONER_M_02_A:
        case APP_TWILEK_COMMONER_M_02_B:
        case APP_TWILEK_COMMONER_M_02_C:
        case APP_TWILEK_COMMONER_M_02_D:
        case APP_TWILEK_FEMALE_UNDERWEAR:
            return true;
        default:
            return false;
    }
}

// Body family of a row, which selects the N_Comm slot allowlist (DiversitySlots.h).
// The union of M, F, FatM and FatF is exactly the TexVarEligible rows; None means no variant.
enum class SlotFamily : uint8_t { None, M, F, FatM, FatF };

inline SlotFamily FamilyOf(uint16_t finalApp) {
    if (IsGenericMaleCommoner(finalApp) || (finalApp >= APP_TWILEK_COMMONER_M_02_A && finalApp <= APP_TWILEK_COMMONER_M_02_D)) {
        return SlotFamily::M;
    }
    if (IsGenericFemaleCommoner(finalApp) || finalApp == APP_TWILEK_FEMALE_UNDERWEAR) return SlotFamily::F;
    if (finalApp == APP_COMMONER_FAT_MALE) return SlotFamily::FatM;
    if (finalApp == APP_COMMONER_FAT_FEMALE) return SlotFamily::FatF;
    return SlotFamily::None;
}

// Allowlist for a family. Sets n to its entry count (0 for SlotFamily::None, with a null pointer).
inline const uint8_t* FamilySlots(SlotFamily family, size_t& n) {
    switch (family) {
        case SlotFamily::M:    n = kMaleSlotsCount;      return kMaleSlots;
        case SlotFamily::F:    n = kFemaleSlotsCount;    return kFemaleSlots;
        case SlotFamily::FatM: n = kFatMaleSlotsCount;   return kFatMaleSlots;
        case SlotFamily::FatF: n = kFatFemaleSlotsCount; return kFatFemaleSlots;
        default:               n = 0;                    return nullptr;
    }
}

// Vary gate: whether a creature takes a texture variant at all. oid 0 (no creature id) never varies.
// Appended rows (APP_FIRST_APPENDED..APP_COUNT-1) are mod-produced and always vary; any other row varies for kVaryPct% of
// creatures, on the salt-5 draw of the per-creature hash.
inline bool ShouldVary(uint32_t oid, uint16_t originalApp) {
    if (oid == 0) return false;
    if (originalApp >= APP_FIRST_APPENDED && originalApp < APP_COUNT) return true;
    return Mix(HashCreatureId(oid, originalApp), 5) % 100u < kVaryPct;
}

// Texture variant for a creature: a slot from the family allowlist of finalApp, or 0 when no variant
// applies (oid 0, a row outside every family, or an empty allowlist). The slot is the salt-4 draw
// modulo the allowlist size. When that draw equals currentTv and the family has more than one slot,
// the next allowlist entry is used, so the variant changes. FatM and FatF have one slot and are exempt.
inline uint8_t PickTextureVar(uint32_t oid, uint16_t finalApp, uint8_t currentTv = 0) {
    if (oid == 0) return 0;
    size_t n = 0;
    const uint8_t* allow = FamilySlots(FamilyOf(finalApp), n);
    if (n == 0) return 0;
    size_t idx = Mix(HashCreatureId(oid, finalApp), 4) % n;
    if (allow[idx] == currentTv && n > 1) idx = (idx + 1) % n;
    return allow[idx];
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

// Determines if texture should be diversified and writes the new ResRef name. The slot is drawn (salt 4 of
// hash) from the family allowlist of appearance; the N_CommM/N_CommF prefix follows currentTex. Returns false
// when appearance has no family (SlotFamily::None).
inline bool ShouldDiversifyTexture(const char* currentTex, uint16_t appearance, uint32_t hash, char* outTex, size_t outSize) {
    if (!currentTex || !outTex || outSize < 16) return false;

    bool isFemale = false;
    if (!IsCommonerTexture(currentTex, isFemale)) {
        return false;
    }

    size_t n = 0;
    const uint8_t* allow = FamilySlots(FamilyOf(appearance), n);
    if (n == 0) return false;
    const int slot = allow[Mix(hash, 4) % n];
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
        // kFatPct% fat, then kDirtyPct% dirty clothes, rest unchanged
        uint32_t roll = BandRoll(hash);
        if (roll < kFatPct) return APP_COMMONER_FAT_MALE;
        if (roll < kFatPct + kDirtyPct) return APP_COMMONER_DIRTY_MALE;
        return currentApp;
    }
    if (IsGenericFemaleCommoner(currentApp)) {
        if (gender != 1) return currentApp;
        uint32_t roll = BandRoll(hash);
        if (roll < kFatPct) return APP_COMMONER_FAT_FEMALE;
        if (roll < kFatPct + kCzerkaPct) return APP_CZERKA_OFFICER_FEMALE;
        if (roll < kFatPct + kCzerkaPct + kDirtyPct) return APP_COMMONER_DIRTY_FEMALE;
        if (roll < kFatPct + kCzerkaPct + kDirtyPct + kProtocolPct) return APP_CZERKA_PROTOCOL_OFFICER_FEMALE;
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
        uint32_t roll = BandRoll(hash);
        if (roll < kDancerPct) {
            switch (Mix(hash, 3) % 3u) {
                case 0: return APP_TWILEK_FEMALE_DANCER_BOOT;
                case 1: return APP_TWILEK_FEMALE_DANCER;
                default: return APP_TWILEK_FEMALE_UNDERWEAR;
            }
        }
        if (roll < kDancerPct + kTwilekToneRemapPct) {
            switch (Mix(hash, 6) % 3u) {
                case 0: return APP_TWILEK_FEMALE_TONE_A;
                case 1: return APP_TWILEK_FEMALE_TONE_B;
                default: return APP_TWILEK_FEMALE_TONE_C;
            }
        }
        return currentApp;
    }
    // Any other appearance: unchanged
    return currentApp;
}

} // namespace NpcDiversity
