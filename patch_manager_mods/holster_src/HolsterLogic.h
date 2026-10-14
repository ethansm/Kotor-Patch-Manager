// HolsterLogic.h -- pure logic of the holstered-weapons mod (doc 42): no engine calls, no Windows headers, header-only, no STL.
// Host-tested by holster_logic_test.cpp. Holster.cpp (the DLL) is the only engine glue.
//
//   classify   itemclass stem (baseitems.itemclass, "w_Lghtsbr_001" or "w_Lghtsbr") -> Category; unknown stem -> weaponwield fallback
//   mount      Category + hand + set -> one of four body mounts (hip L/R, back L/R); two sets share the four mounts
//   fsm        per-creature Holstered / Drawing / Drawn / Stowing machine driven by the client combat flag; never delays an attack:
//              the hand attach happens on the combat edge itself, the slide (P3) is purely cosmetic and capped
//
// Evidence: research/holstered_weapons/wave2_3.md (R6a table, R6b extents, M1 bones, H3/H6 timing). Placement offsets are NOT
// final: every kPlacement row is a probe default until the P0 logs answer gates G2/G7 (see doc 42).
#pragma once
#include <stdint.h>

namespace hs {

enum Category : int { CatUnknown = 0, CatLightsaber, CatMelee, CatQStaff, CatPistol, CatRifle, CatHeavy, CatCount };

inline const char* categoryName(Category c) {
    switch (c) {
        case CatLightsaber: return "lightsaber";
        case CatMelee: return "melee";
        case CatQStaff: return "qstaff";
        case CatPistol: return "pistol";
        case CatRifle: return "rifle";
        case CatHeavy: return "heavy";
        default: return "unknown";
    }
}

// ---- ASCII helpers (no <cctype>/<string.h> so the header stays host/DLL neutral) ----
inline char lower(char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c; }

// Case-insensitive: does `s` begin with `prefix`, and is the next character the end, '_' or a digit (so "w_Dblsbr" does not match
// "w_Dblsbrx")? Used for stems like "w_Lghtsbr_001".
inline bool stemIs(const char* s, const char* stem) {
    if (!s || !stem) return false;
    while (*stem) {
        if (lower(*s) != lower(*stem)) return false;
        ++s; ++stem;
    }
    return *s == 0 || *s == '_' || (*s >= '0' && *s <= '9');
}

// Locked requirement 2: ALL lightsabers (baseitems rows 8, 9, 10, live 106) -> CatLightsaber. Vibroblades are not.
struct StemRow { const char* stem; Category cat; };
static const StemRow kStems[] = {
    {"w_Lghtsbr", CatLightsaber}, {"w_ShortSbr", CatLightsaber}, {"w_Dblsbr", CatLightsaber},
    {"w_Qtrstaff", CatQStaff},
    {"w_Stunbaton", CatMelee}, {"w_Lngswrd", CatMelee}, {"w_Vbroswrd", CatMelee}, {"w_Shortswrd", CatMelee},
    {"w_Vbroshort", CatMelee}, {"w_Dblswrd", CatMelee}, {"w_VbrDblswd", CatMelee}, {"w_gaffi", CatMelee},
    {"w_warblade", CatMelee}, {"w_waraxe", CatMelee}, {"w_ForcePike", CatMelee},
    {"w_crslash", CatMelee}, {"w_crpierce", CatMelee}, {"w_crbite", CatMelee},
    {"w_BlstrPstl", CatPistol}, {"w_HvyBlstr", CatPistol}, {"w_HldOBlst", CatPistol}, {"w_IonBlstr", CatPistol},
    {"w_DsrptPstl", CatPistol}, {"w_SonicPstl", CatPistol},
    {"w_IonRfl", CatRifle}, {"w_BowCstr", CatRifle}, {"w_BlstrCrbn", CatRifle}, {"w_DsrptRfl", CatRifle},
    {"w_SonicRfl", CatRifle}, {"w_Blstrrfl", CatRifle},
    {"w_RptnBlstr", CatHeavy}, {"w_HvRptBltr", CatHeavy},
};
static const int kStemCount = static_cast<int>(sizeof(kStems) / sizeof(kStems[0]));

// baseitems.weaponwield -> category for a stem the table does not know (a weapon-override mod may add rows):
// 1/2 one-handed melee, 3 two-handed melee, 4 pistol, 5 rifle, 6 heavy. Anything else is unknown (never holstered).
inline Category fromWeaponWield(int ww) {
    switch (ww) {
        case 1: case 2: case 3: return CatMelee;
        case 4: return CatPistol;
        case 5: return CatRifle;
        case 6: return CatHeavy;
        default: return CatUnknown;
    }
}

inline Category classify(const char* stem, int weaponWield) {
    for (int i = 0; i < kStemCount; ++i)
        if (stemIs(stem, kStems[i].stem)) return kStems[i].cat;
    return fromWeaponWield(weaponWield);
}

// Two-handed = long weapon. weaponwield 3 (quarterstaff, double blades, pike, gaffi, warblade) or a lightsaber-class double
// (never: sabers are hip by requirement). Pure shape test; the 0.6 m mesh-bounds refinement lands in P2.
inline bool isLong(Category c, int weaponWield) {
    return c == CatQStaff || c == CatRifle || c == CatHeavy || (c == CatMelee && weaponWield == 3);
}

// ---- mounts ----
enum Mount : int { MountNone = -1, MountHipR = 0, MountHipL = 1, MountBackR = 2, MountBackL = 3, MountCount = 4 };
enum Zone : int { ZoneHip = 0, ZoneBack = 1 };
enum Side : int { SideRight = 0, SideLeft = 1 };

inline Mount mountOf(Zone z, Side s) { return static_cast<Mount>(static_cast<int>(z) * 2 + static_cast<int>(s)); }
inline Zone zoneOf(Mount m) { return m >= MountBackR ? ZoneBack : ZoneHip; }
inline Side sideOf(Mount m) { return (static_cast<int>(m) & 1) ? SideLeft : SideRight; }

// Preferred zone: lightsabers always hip (locked); long weapons back; the rest hip.
inline Zone preferredZone(Category c, bool longWeapon) {
    if (c == CatLightsaber) return ZoneHip;
    return longWeapon ? ZoneBack : ZoneHip;
}

// Bone name candidates per mount (M1 evidence: present on all 133 humanoid bodies; match case-insensitively).
inline const char* mountBone(Mount m) {
    switch (m) {
        case MountHipR: return "rthigh_g";
        case MountHipL: return "lthigh_g";
        case MountBackR: case MountBackL: return "torsoUpr_g";
        default: return "";
    }
}
// Waist-level alternative for a hip mount (pelvis is shared by both sides); P0 flag-file experiment tries it.
inline const char* mountBoneAlt(Mount m) {
    switch (m) {
        case MountHipR: case MountHipL: return "pelvis_g";
        case MountBackR: case MountBackL: return "torso_g";
        default: return "";
    }
}

// Choose a mount for one weapon: its preferred zone on its own side if free, else the other zone on that side, else the other
// side in the preferred zone, else the other side in the other zone, else MountNone. `taken` = bitmask (1 << Mount).
// Two weapon sets therefore fill hip R/L + back R/L; a lightsaber spills to the back only when both hip mounts are taken, and
// a lightsaber that already holds a hip mount is never evicted (callers assign sabers first).
inline Mount chooseMount(Zone preferred, Side side, unsigned taken) {
    const Zone other = preferred == ZoneHip ? ZoneBack : ZoneHip;
    const Side oside = side == SideRight ? SideLeft : SideRight;
    const Mount order[4] = {mountOf(preferred, side), mountOf(other, side), mountOf(preferred, oside), mountOf(other, oside)};
    for (int i = 0; i < 4; ++i)
        if (!(taken & (1u << order[i]))) return order[i];
    return MountNone;
}

// ---- placement (bone frame; PROBE DEFAULTS until G7/P2) ----
// Local translation in metres and Euler rotation in degrees applied to the weapon relative to the mount bone. Zero = the weapon
// origin (the grip, except the double saber) sits on the bone origin, unrotated.
struct Placement { float pos[3]; float rotDeg[3]; };
inline Placement placementFor(Category, Mount) {
    Placement p = {{0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}};
    return p;
}

// Hilt-only length (m) used to centre a saber on its mount; median of the R6b samples. The double's origin is 0.21 m off its grip
// centre (R6b), so its grip offset is returned separately.
inline float hiltLengthMeters(const char* stem) {
    if (stemIs(stem, "w_Dblsbr")) return 0.544f;
    if (stemIs(stem, "w_ShortSbr")) return 0.230f;
    if (stemIs(stem, "w_Lghtsbr")) return 0.348f;
    return 0.f;
}
inline float gripOffsetFromOrigin(const char* stem) { return stemIs(stem, "w_Dblsbr") ? 0.21f : 0.f; }

// ---- per-creature state machine ----
// Drawing and Stowing are timed cosmetic slides. The machine NEVER gates the weapon being in the hand: on the combat 0->nonzero
// edge `inHand()` is true at once (the first attack must not wait), Drawing only describes the remaining slide.
enum Phase : int { PhHolstered = 0, PhDrawing, PhDrawn, PhStowing };

static const uint32_t kDrawMs = 350;        // combat-entry slide (inside the vanilla draw_weapon window 0.03-0.57 s)
static const uint32_t kSwapMs = 1000;       // in-combat weapon swap (action 0x0E, fixed 1000 ms)
static const uint32_t kStowDelayMs = 1500;  // after the combat flag clears, wait before stowing (exit countdown margin)

struct Fsm {
    Phase phase = PhHolstered;
    uint32_t phaseStart = 0;       // ms tick the current phase began
    uint32_t phaseLen = 0;         // slide length for Drawing / Stowing
    uint32_t stowAt = 0;           // tick when the pending stow begins (0 = none)
    bool stowPending = false;
    int lastCombat = 0;

    bool inHand() const { return phase == PhDrawing || phase == PhDrawn || (phase == PhStowing); }
    // Fraction hand<->mount for the slide: 0 = at the mount, 1 = in the hand.
    float handFraction(uint32_t now) const {
        switch (phase) {
            case PhHolstered: return 0.f;
            case PhDrawn: return 1.f;
            case PhDrawing: { float t = phaseLen ? static_cast<float>(now - phaseStart) / static_cast<float>(phaseLen) : 1.f; return t > 1.f ? 1.f : t; }
            case PhStowing: { float t = phaseLen ? static_cast<float>(now - phaseStart) / static_cast<float>(phaseLen) : 1.f; return t > 1.f ? 0.f : 1.f - t; }
        }
        return 0.f;
    }
    // Combat flag edge (0 <-> nonzero). Entering combat cancels a pending stow and draws at once.
    void onCombat(int combat, uint32_t now) {
        if (combat != 0 && lastCombat == 0) {
            stowPending = false;
            phase = PhDrawing; phaseStart = now; phaseLen = kDrawMs;
        } else if (combat == 0 && lastCombat != 0) {
            stowPending = true; stowAt = now + kStowDelayMs;
        }
        lastCombat = combat;
    }
    // In-combat weapon-set swap: the incoming set is drawn over the swap window (the outgoing set is stowed by the caller).
    void onSwap(uint32_t now) { phase = PhDrawing; phaseStart = now; phaseLen = kSwapMs; stowPending = false; }
    // Advance timers. Call once per observation (any cadence; pure function of `now`).
    void tick(uint32_t now) {
        if (phase == PhDrawing && now - phaseStart >= phaseLen) phase = PhDrawn;
        if (stowPending && static_cast<int32_t>(now - stowAt) >= 0) {
            stowPending = false;
            if (phase != PhHolstered) { phase = PhStowing; phaseStart = now; phaseLen = kDrawMs; }
        }
        if (phase == PhStowing && now - phaseStart >= phaseLen) phase = PhHolstered;
    }
};

} // namespace hs
