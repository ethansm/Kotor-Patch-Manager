// QuickLootBezel.h -- Configurable Hardware Bezels & Container Classifier (doc 52 QL14).
// Pure, host-testable header: 7 container categories, 7 bezel frames, 7 color palettes,
// placeable classifier and style resolution.
#pragma once
#include <stdint.h>
#include <string.h>

namespace qlbezel {

enum Category : uint8_t {
    CatDroidTech = 0,
    CatWeaponArmory = 1,
    CatMedicalLab = 2,
    CatStrongboxCache = 3,
    CatRemainsCorpse = 4,
    CatFootlockerGen = 5,
    CatDefaultFallback = 6,
    CatCount = 7,
    // Aliases
    CatDroid = CatDroidTech,
    CatArmory = CatWeaponArmory,
    CatMedical = CatMedicalLab,
    CatStrongbox = CatStrongboxCache,
    CatRemains = CatRemainsCorpse,
    CatLocker = CatFootlockerGen,
    CatGeneric = CatDefaultFallback
};

enum BezelFrame : uint8_t {
    FrameNone = 0,      // Plain wireframe border
    FrameDatapad = 1,   // ql_bzl_dpad
    FrameComputer = 2,  // ql_bzl_comp
    FrameUpgrade = 3,   // ql_bzl_upgr
    FrameWorkbench = 4, // ql_bzl_work
    FrameStore = 5,     // ql_bzl_stor
    FrameGold = 6,      // ql_bzl_gold
    FrameCount = 7
};

enum Palette : uint8_t {
    PalTeal = 0,
    PalBlue = 1,
    PalCobalt = 2,
    PalEmerald = 3,
    PalBronze = 4,
    PalGold = 5,
    PalCustom = 6,
    PalCount = 7
};

struct CategoryStyle {
    uint8_t frame;          // BezelFrame (0..6)
    uint8_t palette;        // Palette (0..6)
    bool led;               // Circular LED indicator
    float frameRgb[3];      // Wireframe / divider RGB
    float btnRgb[3];        // Action button border RGB
    float textRgb[3];       // Unhovered text RGB
    float ledRgb[3];        // LED indicator RGB
};

inline const char* categoryName(Category c) {
    switch (c) {
        case CatDroidTech:        return "Droids & Wreckage";
        case CatWeaponArmory:     return "Weapon Racks & Armory";
        case CatMedicalLab:       return "Medical & Lab";
        case CatStrongboxCache:   return "Strongboxes & Vaults";
        case CatRemainsCorpse:    return "Remains / Corpses";
        case CatFootlockerGen:    return "Footlockers & Lockers";
        case CatDefaultFallback:  return "Standard Containers";
        default:                  return "Unknown";
    }
}

inline const char* frameName(BezelFrame f) {
    switch (f) {
        case FrameNone:      return "None (Plain Border)";
        case FrameDatapad:   return "Datapad";
        case FrameComputer:  return "Computer Console";
        case FrameUpgrade:   return "Upgrade Workbench";
        case FrameWorkbench: return "Lab Station";
        case FrameStore:     return "Merchant Store";
        case FrameGold:      return "Gold Pause Bezel";
        default:             return "Unknown";
    }
}

inline const char* frameResref(BezelFrame f) {
    switch (f) {
        case FrameDatapad:   return "ql_bzl_dpad";
        case FrameComputer:  return "ql_bzl_comp";
        case FrameUpgrade:   return "ql_bzl_upgr";
        case FrameWorkbench: return "ql_bzl_work";
        case FrameStore:     return "ql_bzl_stor";
        case FrameGold:      return "ql_bzl_gold";
        default:             return "";
    }
}

inline const char* paletteName(Palette p) {
    switch (p) {
        case PalTeal:    return "Datapad Teal";
        case PalBlue:    return "Terminal Blue";
        case PalCobalt:  return "Workbench Cobalt";
        case PalEmerald: return "Lab Emerald";
        case PalBronze:  return "Store Bronze";
        case PalGold:    return "Classic Gold";
        case PalCustom:  return "Custom RGB";
        default:         return "Unknown";
    }
}

inline void getPaletteColors(Palette p, float frameRgb[3], float btnRgb[3], float textRgb[3], float ledRgb[3]) {
    switch (p) {
        case PalBlue:
            frameRgb[0] = 0.15f; frameRgb[1] = 0.45f; frameRgb[2] = 0.85f;
            btnRgb[0]   = 0.15f; btnRgb[1]   = 0.45f; btnRgb[2]   = 0.85f;
            textRgb[0]  = 0.35f; textRgb[1]  = 0.70f; textRgb[2]  = 1.00f;
            ledRgb[0]   = 0.20f; ledRgb[1]   = 0.60f; ledRgb[2]   = 1.00f;
            break;
        case PalCobalt:
            frameRgb[0] = 0.40f; frameRgb[1] = 0.50f; frameRgb[2] = 0.65f;
            btnRgb[0]   = 0.40f; btnRgb[1]   = 0.50f; btnRgb[2]   = 0.65f;
            textRgb[0]  = 0.70f; textRgb[1]  = 0.80f; textRgb[2]  = 0.95f;
            ledRgb[0]   = 0.50f; ledRgb[1]   = 0.70f; ledRgb[2]   = 0.95f;
            break;
        case PalEmerald:
            frameRgb[0] = 0.10f; frameRgb[1] = 0.65f; frameRgb[2] = 0.35f;
            btnRgb[0]   = 0.10f; btnRgb[1]   = 0.65f; btnRgb[2]   = 0.35f;
            textRgb[0]  = 0.30f; textRgb[1]  = 0.85f; textRgb[2]  = 0.50f;
            ledRgb[0]   = 0.20f; ledRgb[1]   = 0.95f; ledRgb[2]   = 0.45f;
            break;
        case PalBronze:
            frameRgb[0] = 0.75f; frameRgb[1] = 0.55f; frameRgb[2] = 0.20f;
            btnRgb[0]   = 0.75f; btnRgb[1]   = 0.55f; btnRgb[2]   = 0.20f;
            textRgb[0]  = 0.95f; textRgb[1]  = 0.75f; textRgb[2]  = 0.35f;
            ledRgb[0]   = 1.00f; ledRgb[1]   = 0.75f; ledRgb[2]   = 0.20f;
            break;
        case PalGold:
            frameRgb[0] = 0.85f; frameRgb[1] = 0.72f; frameRgb[2] = 0.25f;
            btnRgb[0]   = 0.85f; btnRgb[1]   = 0.72f; btnRgb[2]   = 0.25f;
            textRgb[0]  = 1.00f; textRgb[1]  = 0.90f; textRgb[2]  = 0.45f;
            ledRgb[0]   = 1.00f; ledRgb[1]   = 0.85f; ledRgb[2]   = 0.20f;
            break;
        case PalTeal:
        case PalCustom:
        default:
            frameRgb[0] = 0.10f; frameRgb[1] = 0.58f; frameRgb[2] = 0.62f;
            btnRgb[0]   = 0.10f; btnRgb[1]   = 0.58f; btnRgb[2]   = 0.62f;
            textRgb[0]  = 0.20f; textRgb[1]  = 0.80f; textRgb[2]  = 0.85f;
            ledRgb[0]   = 0.10f; ledRgb[1]   = 0.85f; ledRgb[2]   = 0.90f;
            break;
    }
}

inline CategoryStyle defaultStyle(Category c) {
    CategoryStyle s = {};
    switch (c) {
        case CatDroidTech:
            s.frame = FrameComputer;
            s.palette = PalBlue;
            s.led = true;
            break;
        case CatWeaponArmory:
            s.frame = FrameUpgrade;
            s.palette = PalCobalt;
            s.led = true;
            break;
        case CatMedicalLab:
            s.frame = FrameWorkbench;
            s.palette = PalEmerald;
            s.led = true;
            break;
        case CatStrongboxCache:
            s.frame = FrameStore;
            s.palette = PalBronze;
            s.led = true;
            break;
        case CatRemainsCorpse:
            s.frame = FrameNone;
            s.palette = PalTeal;
            s.led = false;
            break;
        case CatFootlockerGen:
            s.frame = FrameDatapad;
            s.palette = PalTeal;
            s.led = true;
            break;
        case CatDefaultFallback:
        default:
            s.frame = FrameDatapad;
            s.palette = PalTeal;
            s.led = true;
            break;
    }
    getPaletteColors(static_cast<Palette>(s.palette), s.frameRgb, s.btnRgb, s.textRgb, s.ledRgb);
    return s;
}

// Pure case-insensitive substring search without external dependencies
inline bool matchSubstr(const char* haystack, const char* needle) {
    if (!haystack || !needle || !*needle) return false;
    for (const char* h = haystack; *h; ++h) {
        const char* h_it = h;
        const char* n_it = needle;
        while (*h_it && *n_it) {
            char ch = *h_it;
            char cn = *n_it;
            if (ch >= 'A' && ch <= 'Z') ch = static_cast<char>(ch + ('a' - 'A'));
            if (cn >= 'A' && cn <= 'Z') cn = static_cast<char>(cn + ('a' - 'A'));
            if (ch != cn) break;
            ++h_it;
            ++n_it;
        }
        if (!*n_it) return true;
    }
    return false;
}

inline bool matchAny(const char* s, const char* const* list, size_t count) {
    if (!s || !*s) return false;
    for (size_t i = 0; i < count; ++i) {
        if (matchSubstr(s, list[i])) return true;
    }
    return false;
}

// Deterministic 7-category classifier based on placeables.2da row, corpse/bodybag flags, and tag/name/resref
inline Category classifyPlaceable(uint32_t isBodyBag, uint32_t isCorpse, uint32_t app, const char* name, const char* tag = nullptr, const char* resref = nullptr) {
    // 1. Droid / Tech keywords & appearances
    static const char* const kDroidKeywords[] = {
        "droid", "t3m4", "hk47", "g0t0", "remote", "console", "comp", "term", "sec_pnl", "slicer"
    };
    if (app == 63 || (app >= 73 && app <= 77) || (app >= 152 && app <= 154) || (app >= 195 && app <= 199) || app == 208 ||
        matchAny(tag, kDroidKeywords, 10) || matchAny(name, kDroidKeywords, 10) || matchAny(resref, kDroidKeywords, 10)) {
        return CatDroidTech;
    }

    // 2. Corpse / Remains keywords, flags & appearances
    static const char* const kCorpseKeywords[] = {
        "corpse", "remains", "bones", "skeleton", "dead", "body"
    };
    if (isCorpse != 0 || isBodyBag != 0 || (app >= 8 && app <= 10) || app == 56 || app == 71 || app == 203 || app == 211 ||
        matchAny(tag, kCorpseKeywords, 6) || matchAny(name, kCorpseKeywords, 6) || matchAny(resref, kCorpseKeywords, 6)) {
        return CatRemainsCorpse;
    }

    // 3. Medical / Lab keywords & appearances
    static const char* const kMedKeywords[] = {
        "med", "kolto", "bacta", "lab", "chem", "bio", "serum", "heal", "surg"
    };
    if (app == 65 || app == 191 || app == 238 || (app >= 377 && app <= 379) ||
        matchAny(tag, kMedKeywords, 9) || matchAny(name, kMedKeywords, 9) || matchAny(resref, kMedKeywords, 9)) {
        return CatMedicalLab;
    }

    // 4. Weapon / Armory keywords & appearances
    static const char* const kWeapKeywords[] = {
        "weap", "rack", "armor", "armory", "ammo", "blaster", "saber", "turret", "ordnance", "workbench"
    };
    if (app == 59 || app == 60 || app == 72 ||
        matchAny(tag, kWeapKeywords, 10) || matchAny(name, kWeapKeywords, 10) || matchAny(resref, kWeapKeywords, 10)) {
        return CatWeaponArmory;
    }

    // 5. Strongbox / Cache keywords & appearances
    static const char* const kBoxKeywords[] = {
        "strongbox", "safe", "vault", "cache", "sec_box", "merch", "credits", "lockbox", "treasure"
    };
    if (app == 7 || app == 159 ||
        matchAny(tag, kBoxKeywords, 9) || matchAny(name, kBoxKeywords, 9) || matchAny(resref, kBoxKeywords, 9)) {
        return CatStrongboxCache;
    }

    // 6. Footlocker / Standard Tech keywords & appearances
    static const char* const kLockerKeywords[] = {
        "flocker", "footlocker", "pl_locker", "locker", "crate", "box", "barrel", "cant", "desk"
    };
    if ((app >= 1 && app <= 5) || app == 58 || app == 61 || app == 62 || app == 160 ||
        matchAny(tag, kLockerKeywords, 9) || matchAny(name, kLockerKeywords, 9) || matchAny(resref, kLockerKeywords, 9)) {
        return CatFootlockerGen;
    }

    // 7. Generic Fallback
    return CatDefaultFallback;
}

} // namespace qlbezel
