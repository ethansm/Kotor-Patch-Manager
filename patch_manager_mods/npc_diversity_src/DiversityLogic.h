// DiversityLogic.h -- Pure C++ deterministic selection logic for Master NPC Diversity Dispatcher
#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>

#include "DiversityRows.h"

namespace NpcDiversity {

// FNV-1a 32-bit hash constants
constexpr uint32_t FNV_OFFSET_BASIS = 2166136261u;
constexpr uint32_t FNV_PRIME = 16777619u;

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

// Exclusion tag lookup for companions and major story NPCs
inline bool IsExcludedFromDiversification(const char* tag) {
    if (!tag) return false;

    // Skip common creature tag prefixes (e.g. "n_chodo" -> "chodo", "npc_atton" -> "atton")
    const char* p = tag;
    if (StartsWithCaseInsensitive(p, "npc_")) {
        p += 4;
    } else if (StartsWithCaseInsensitive(p, "n_") || StartsWithCaseInsensitive(p, "c_")) {
        p += 2;
    }

    // Companion exclusion tags (case-insensitive)
    if (StartsWithCaseInsensitive(p, "kreia") ||
        StartsWithCaseInsensitive(p, "atton") ||
        StartsWithCaseInsensitive(p, "bao_dur") ||
        StartsWithCaseInsensitive(p, "baodur") ||
        StartsWithCaseInsensitive(p, "mira") ||
        StartsWithCaseInsensitive(p, "hanharr") ||
        StartsWithCaseInsensitive(p, "mandalore") ||
        StartsWithCaseInsensitive(p, "visas") ||
        StartsWithCaseInsensitive(p, "disciple") ||
        StartsWithCaseInsensitive(p, "handmaiden") ||
        StartsWithCaseInsensitive(p, "t3m4") ||
        StartsWithCaseInsensitive(p, "t3_m4") ||
        StartsWithCaseInsensitive(p, "hk47") ||
        StartsWithCaseInsensitive(p, "hk_47") ||
        StartsWithCaseInsensitive(p, "g0t0") ||
        StartsWithCaseInsensitive(p, "g0_t0") ||
        StartsWithCaseInsensitive(p, "player")) {
        return true;
    }

    // Major story NPC exclusion tags (case-insensitive)
    if (StartsWithCaseInsensitive(p, "chodo") ||
        StartsWithCaseInsensitive(p, "luxa") ||
        StartsWithCaseInsensitive(p, "grenn") ||
        StartsWithCaseInsensitive(p, "opo_chano") ||
        StartsWithCaseInsensitive(p, "opochano") ||
        StartsWithCaseInsensitive(p, "lorso") ||
        StartsWithCaseInsensitive(p, "b4d4") ||
        StartsWithCaseInsensitive(p, "ralon") ||
        StartsWithCaseInsensitive(p, "moza") ||
        StartsWithCaseInsensitive(p, "vrook") ||
        StartsWithCaseInsensitive(p, "zreztik") ||
        StartsWithCaseInsensitive(p, "geeda")) {
        return true;
    }

    return false;
}

// Deterministic hash computed from creature spawn parameters
inline uint32_t HashCreatureSpawn(const char* areaResRef, float x, float y, const char* tag) {
    uint32_t hash = FNV_OFFSET_BASIS;

    auto mix_str = [&](const char* s) {
        if (!s) return;
        while (*s) {
            hash = (hash ^ static_cast<uint8_t>(*s++)) * FNV_PRIME;
        }
    };

    mix_str(areaResRef);
    mix_str(tag);

    // Quantize position to 0.1m precision
    int32_t qx = static_cast<int32_t>(x * 10.0f);
    int32_t qy = static_cast<int32_t>(y * 10.0f);

    hash = (hash ^ static_cast<uint32_t>(qx & 0xFFFF)) * FNV_PRIME;
    hash = (hash ^ static_cast<uint32_t>((qx >> 16) & 0xFFFF)) * FNV_PRIME;
    hash = (hash ^ static_cast<uint32_t>(qy & 0xFFFF)) * FNV_PRIME;
    hash = (hash ^ static_cast<uint32_t>((qy >> 16) & 0xFFFF)) * FNV_PRIME;

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

// Map hash value to an integer slot in [minSlot, maxSlot] inclusive
inline int ComputeSlot(uint32_t hash, int minSlot = 1, int maxSlot = 35) {
    if (minSlot >= maxSlot) return minSlot;
    int range = maxSlot - minSlot + 1;
    return minSlot + static_cast<int>(hash % static_cast<uint32_t>(range));
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

    int slot = ComputeSlot(hash, 1, 35);
    if (isFemale) {
        std::snprintf(outTex, outSize, "N_CommF%02d", slot);
    } else {
        std::snprintf(outTex, outSize, "N_CommM%02d", slot);
    }
    return true;
}

// Generic civilian appearance checks (matching generic civilian appearance rows)
inline bool IsGenericMaleCommoner(uint16_t app) {
    switch (app) {
        // Test ranges
        case 300: case 301: case 302: case 303: case 304:
        case 305: case 306: case 307: case 308: case 309: case 310:
        // Vanilla appearance.2da generic male commoners
        case 17:
        case 190: case 191: case 192: case 193: case 194:
        case 345: case 346: case 347: case 348: case 349:
        case 423: case 424: case 428: case 430:
        case 591: case 592: case 593: case 594: case 595: case 596:
        case 615: case 616: case 617:
            return true;
        default:
            return false;
    }
}

inline bool IsGenericFemaleCommoner(uint16_t app) {
    switch (app) {
        // Test ranges
        case 350: case 351: case 352: case 353: case 354:
        case 355: case 356: case 357: case 358: case 359: case 360:
        // Vanilla appearance.2da generic female commoners
        case 14:
        case 185: case 186: case 187: case 188: case 189:
        case 339: case 340: case 341: case 342: case 343: case 344:
        case 429: case 431:
        case 585: case 586: case 587: case 588: case 589: case 590:
        case 609: case 610: case 611: case 612: case 613: case 614:
            return true;
        default:
            return false;
    }
}

inline bool IsGenericTwilekMale(uint16_t app) {
    switch (app) {
        // Test ranges
        case 500: case 501: case 502: case 503: case 504:
        case 505: case 506: case 507: case 508: case 509: case 510:
        // Vanilla appearance.2da generic male Twi'leks
        case 416: case 417: case 445: case 449: case 450:
        case 606: case 618: case 619:
            return true;
        default:
            return false;
    }
}

inline bool IsGenericTwilekFemale(uint16_t app) {
    switch (app) {
        // Test ranges
        case 550: case 551: case 552: case 553: case 554:
        case 555: case 556: case 557: case 558: case 559: case 560:
        // Vanilla appearance.2da generic female Twi'leks
        case 280: case 446: case 447: case 448:
        case 620: case 621: case 622:
            return true;
        default:
            return false;
    }
}

// Hash creature position (quantized x*10, y*10) with objectId and appearance using FNV-1a
inline uint32_t HashCreaturePosition(float x, float y, uint32_t objectId, uint16_t appearance) {
    uint32_t hash = FNV_OFFSET_BASIS;

    int32_t qx = static_cast<int32_t>(x * 10.0f);
    int32_t qy = static_cast<int32_t>(y * 10.0f);

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

// Remap appearance based on hash for diversity distribution
inline uint16_t RemapAppearance(uint16_t currentApp, uint32_t hash) {
    if (IsGenericMaleCommoner(currentApp)) {
        uint32_t pct = hash % 100;
        if (pct < 15) return APP_COMMONER_FAT_MALE;
        if (pct < 20) {
            // Rotate between Kel Dor 01 and 02
            return (hash % 2 == 0) ? APP_ALIEN_KELDOR_01 : APP_ALIEN_KELDOR_02;
        }
        if (pct < 25) {
            // Rotate between Iridorian 01 and 02
            return (hash % 2 == 0) ? APP_ALIEN_IRIDORIAN_01 : APP_ALIEN_IRIDORIAN_02;
        }
        // 75% unchanged
        return currentApp;
    }
    if (IsGenericFemaleCommoner(currentApp)) {
        uint32_t pct = hash % 100;
        if (pct < 15) return APP_COMMONER_FAT_FEMALE;
        if (pct < 20) return APP_CZERKA_OFFICER_FEMALE;
        // 80% unchanged
        return currentApp;
    }
    if (IsGenericTwilekMale(currentApp)) {
        switch (hash % 4) {
            case 0: return APP_TWILEK_COMMONER_M_02_A;
            case 1: return APP_TWILEK_COMMONER_M_02_B;
            case 2: return APP_TWILEK_COMMONER_M_02_C;
            case 3: return APP_TWILEK_COMMONER_M_02_D;
        }
        return currentApp;
    }
    if (IsGenericTwilekFemale(currentApp)) {
        uint32_t pct = hash % 100;
        if (pct < 20) {
            // Dancer variants: rotate through the three dancer variants
            uint32_t dancerSlot = (hash / 100) % 3;
            if (dancerSlot == 0) return APP_TWILEK_FEMALE_DANCER_BOOT;
            if (dancerSlot == 1) return APP_TWILEK_FEMALE_DANCER;
            return APP_TWILEK_FEMALE_UNDERWEAR;
        }
        // 80% unchanged
        return currentApp;
    }
    // Any other appearance: unchanged
    return currentApp;
}

} // namespace NpcDiversity
