// CombatStats.h - pure KOTOR II stat math for the combat-stats page (doc 51 ST2).
// No engine access: the DLL fills the input structs in ST4 from creature/game state.
// Formulas come from project design doc 51 "Verified stat catalog".
// Header-only, C++17, <stdint.h> only (no windows.h, no STL).
#pragma once

#include <stdint.h>

namespace cstats {

// Feat, spell, class and form ids (2da row numbers).
constexpr uint16_t kFeatAJD = 1;                 // feat.2da row 1 Advanced Jedi Defense
constexpr uint16_t kFeatMJD = 24;                // feat.2da row 24 Master Jedi Defense
constexpr uint16_t kFeatWMDeflect = 168;         // feat.2da row 168 Weapon Master Deflect
constexpr uint16_t kFeatFF1 = 88;                // feat.2da row 88 Force Focus
constexpr uint16_t kFeatFF2 = 89;                // feat.2da row 89 Improved Force Focus
constexpr uint16_t kFeatFF3 = 90;                // feat.2da row 90 Master Force Focus
constexpr uint32_t kSpellForceRedirection = 163; // spells.2da row 163 Force Redirection
constexpr uint8_t kClassWeaponMaster = 0x0b;     // classes.2da row 11 Weapon Master
constexpr int32_t kFormShiiCho = 258;            // spells.2da row 258 Shii-Cho
constexpr int32_t kFormMakashi = 259;            // spells.2da row 259 Makashi
constexpr int32_t kFormSoresu = 260;             // spells.2da row 260 Soresu
constexpr int32_t kFormAtaru = 261;              // spells.2da row 261 Ataru
constexpr int32_t kFormShien = 262;              // spells.2da row 262 Shien
constexpr int32_t kFormNiman = 263;              // spells.2da row 263 Niman
constexpr int32_t kFormJuyo = 264;               // spells.2da row 264 Juyo

// Labelled additive terms. A Breakdown holds the total plus up to 12 non-zero parts.
struct Part { const char* label; int value; };
struct Breakdown { int total; int count; Part parts[12]; };

// Adds value to total. Appends a labelled part when value != 0 and fewer than 12 parts exist.
inline void addPart(Breakdown& b, const char* label, int value) {
  b.total += value;
  if (value != 0 && b.count < 12) {
    b.parts[b.count].label = label;
    b.parts[b.count].value = value;
    ++b.count;
  }
}

// Attacker Precise Shot (-2..-10) is the attacker's term and is not part of deflection().
constexpr const char* kDeflectNote = "Attacker Precise Shot lowers this by 2-10.";

struct DeflectIn {
  int bab;                  // base attack bonus
  bool hasAJD, hasMJD;      // feats 1 / 24
  bool hasForceRedirection; // spell 163
  bool hasWMDeflect;        // feat 168
  bool isWeaponMaster;      // has class 0x0b
  int wmLevel;              // Weapon Master class level
  bool saber;               // wielding a lightsaber
  int32_t form;             // creature+0x1110, 0 = none
  int effectDelta;          // sum of EffectBlasterDeflection increase - decrease
};

// Deflection: BAB, JD tier, Force Redirection, WM Deflect, form term (saber only), effects.
// Form attack bonuses (Shien/Niman descriptions) do NOT feed deflection (doc 51 finding 1).
inline Breakdown deflection(const DeflectIn& in) {
  Breakdown b{};
  addPart(b, "Base attack bonus", in.bab);
  if (in.hasMJD) {
    addPart(b, "Master Jedi Defense", 6); // MJD replaces AJD, never +9
  } else if (in.hasAJD) {
    addPart(b, "Advanced Jedi Defense", 3);
  }
  if (in.hasForceRedirection) addPart(b, "Force Redirection", 3);
  if (in.hasWMDeflect && in.isWeaponMaster && in.saber) {
    addPart(b, "Weapon Master Deflect", (in.wmLevel + 1) / 2);
  }
  if (in.saber) {
    switch (in.form) {
      case kFormMakashi: addPart(b, "Makashi", -5); break;
      case kFormSoresu:  addPart(b, "Soresu", 4); break;
      case kFormAtaru:   addPart(b, "Ataru", -4); break;
      case kFormShien:   addPart(b, "Shien", 2); break;
      case kFormNiman:   addPart(b, "Niman", 1); break;
      default: break;
    }
  }
  addPart(b, "Effects", in.effectDelta);
  return b;
}

struct ForceDCIn { int level; int chaMod; int wisMod; bool ff1, ff2, ff3; };

// Force DC: level + base 5 + Charisma + Wisdom + the best Force Focus tier only.
inline Breakdown forceDC(const ForceDCIn& in) {
  Breakdown b{};
  addPart(b, "Level", in.level);
  addPart(b, "Base", 5);
  addPart(b, "Charisma", in.chaMod);
  addPart(b, "Wisdom", in.wisMod);
  if (in.ff3) {
    addPart(b, "Master Force Focus", 4);
  } else if (in.ff2) {
    addPart(b, "Improved Force Focus", 3);
  } else if (in.ff1) {
    addPart(b, "Force Focus", 2);
  }
  return b;
}

// Force Scream (spells 159-161) adds this to the DC; the caller adds it, forceDC() does not.
constexpr int kForceScreamDCBonus = 5;
inline bool isForceScreamLine(uint32_t spellId) { return spellId >= 159 && spellId <= 161; }

struct CostMult { float light; float dark; };

// forceadjust.2da indexed by goodEvil/10 clamped to 0..10. light = goodcost, dark = evilcost.
inline CostMult fpCostMult(int goodEvil) {
  static const CostMult kTable[11] = {
      {1.5f, 0.5f}, {1.3f, 0.7f}, {1.2f, 0.8f}, {1.15f, 0.85f}, {1.1f, 0.9f}, {1.0f, 1.0f},
      {0.9f, 1.1f}, {0.85f, 1.15f}, {0.8f, 1.2f}, {0.7f, 1.3f}, {0.5f, 1.5f}};
  int row = goodEvil / 10;
  if (row < 0) row = 0;
  if (row > 10) row = 10;
  return kTable[row];
}

struct CritRange { int threat; int minRoll; };

// w = baseitems crit threat, keen doubles it, m = special-attack tier (0..3).
inline CritRange critRange(int w, bool keen, int m) {
  CritRange r{};
  r.threat = w + (keen ? w : 0) + m * w;
  r.minRoll = 21 - r.threat;
  if (r.minRoll < 1) r.minRoll = 1;
  return r;
}

// Crit multiplier: base, +1 for Power/Sniper/Flurry-type specials, +1 for Shien saber.
inline int critMult(int base, bool special, bool shienSaber) {
  return base + (special ? 1 : 0) + (shienSaber ? 1 : 0);
}

struct FormDef { int vsOthers; int vsTarget; };

// Form defence modifiers from the doc 51 catalog (vsOthers / vsTarget).
inline FormDef formDefense(int32_t form) {
  switch (form) {
    case kFormShiiCho: return {3, 0};
    case kFormSoresu:  return {0, 2};
    case kFormAtaru:   return {-2, 3};
    case kFormShien:   return {0, -5};
    case kFormNiman:   return {1, 1};
    case kFormJuyo:    return {-4, -2};
    default:           return {0, 0};
  }
}

}  // namespace cstats
