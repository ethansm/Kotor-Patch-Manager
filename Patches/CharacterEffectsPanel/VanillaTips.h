// VanillaTips.h - pure hover table + tooltip builder for the vanilla character page rows (doc 51 ST6).
// Hover rects cover each row's label and value controls in character_p.gui. No engine access, no windows.h, no STL.
#pragma once
#include <stdio.h>
#include <string.h>
#include "StatsRows.h"

namespace vtips {

// Vanilla character page rows in hover-table order (hit() returns the first match, so overlaps resolve by this order).
enum Row { V_STR, V_DEX, V_CON, V_INT, V_WIS, V_CHA, V_VIT, V_FORCE, V_DEF, V_FORT, V_REF, V_WILL, V_ALIGN, V_XP, V_NEXT,
           V_MASTERY, V_HEADER, V_COUNT };
struct Rect { float x, y, w, h; };   // GUI units (character_p.gui EXTENT, same space as the panel's PanelX/PanelY)

// Hover rects = union of each row's label + value controls in the live Override character_p.gui (read 2026-10-27).
// V_MASTERY = LBL_FORCEMASTERY, V_HEADER = LBL_EFX_HEADER.
constexpr Rect kRects[V_COUNT] = {
  {62,136,210,21},{62,160,210,21},{62,183,210,21},{62,206,210,21},{62,230,210,21},{62,253,210,21},
  {62,277,214,21},{62,301,214,21},{62,325,202,21},{62,350,167,21},{62,375,167,21},{62,399,167,21},
  {304,170,52,255},{60,432,108,35},{166,432,108,35},
  {294,105,232,21},{536,104,216,24}};

// First row whose rect contains (u,v) (x <= u < x+w, y <= v < y+h), else -1.
inline int hit(float u, float v) {
  for (int i = 0; i < V_COUNT; ++i) {
    const Rect& r = kRects[i];
    if (u >= r.x && u < r.x + r.w && v >= r.y && v < r.y + r.h) return i;
  }
  return -1;
}

// Row name and description (static literals, nullptr = none: the label says it). Shown with or without live values.
struct Tip { const char* name; const char* desc; };
constexpr Tip kTips[V_COUNT] = {
  {"Strength", "Modifier adds to melee attack and damage."},
  {"Dexterity", "Modifier adds to Defense, ranged attack and Reflex."},
  {"Constitution", "Modifier adds to vitality per level and Fortitude."},
  {"Intelligence", "Modifier adds to skill points per level."},
  {"Wisdom", "Modifier adds to Will and your Force power DC."},
  {"Charisma", "Modifier adds to your Force power DC."},
  {"Vitality", nullptr},
  {"Force points", nullptr},
  {"Defense", nullptr},
  {"Fortitude", "Resists poison, disease and other physical effects."},
  {"Reflex", "Resists grenades, mines and other area effects."},
  {"Will", "Resists mind-affecting powers such as Stun and Fear."},
  {"Alignment", "Above 50 is light side, below 50 dark side."},
  {"Experience", nullptr},
  {"Next level", nullptr},
  {"Force Mastery", "Active only at alignment exactly 100 or 0."},
  {"Effects / Stats", "Click to switch between active effects and combat stats."},
};

// "Light power cost x0.85" and "Dark power cost x1.15" from the alignment (same multipliers as the Stats tab).
inline void addCostLines(cfx::Entry* e, int goodEvil) {
  cstats::CostMult m = cstats::fpCostMult(goodEvil);
  char mult[16], line[cfx::LineLen];
  srows::multText(mult, sizeof mult, m.light);
  snprintf(line, sizeof line, "Light power cost %s", mult);
  srows::addLine(e, line);
  srows::multText(mult, sizeof mult, m.dark);
  snprintf(line, sizeof line, "Dark power cost %s", mult);
  srows::addLine(e, line);
}

// Mastery table per class line: 0 Guardian, 1 Consular, 2 Sentinel (engine SetStats 0x0084e6f0, strrefs 114161..114166).
constexpr const char* kMasteryLineName[3] = {"Guardian", "Consular", "Sentinel"};
constexpr const char* kMasteryLight[3] = {"+3 Strength", "+3 Wisdom", "+3 Constitution"};
constexpr const char* kMasteryDark[3] = {"+1-8 damage", "+50 Force points", "+3 Dexterity"};

// Mastery line of a class id (Guardian 3/11/14, Consular 4/12/15, Sentinel 5/13/16), -1 for any other class.
inline int masteryLine(int classId) {
  switch (classId) {
    case 3: case 11: case 14: return 0;
    case 4: case 12: case 15: return 1;
    case 5: case 13: case 16: return 2;
    default: return -1;
  }
}

// Force Mastery lines: the class line of the last class slot with its active mastery marked, or the three
// static reference lines when there is no such class (or no live values).
inline void addMasteryLines(cfx::Entry* e, const srows::StatsIn& in, bool have) {
  int nc = srows::classSlots(in.nClass);
  int ln = (have && nc > 0) ? masteryLine(in.classId[nc - 1]) : -1;
  char line[cfx::LineLen];
  if (ln >= 0) {
    snprintf(line, sizeof line, "Class line: %s", kMasteryLineName[ln]);
    srows::addLine(e, line);
    snprintf(line, sizeof line, "Light Mastery: %s%s", kMasteryLight[ln], in.mastery == 1 ? " (active)" : "");
    srows::addLine(e, line);
    snprintf(line, sizeof line, "Dark Mastery: %s%s", kMasteryDark[ln], in.mastery == 2 ? " (active)" : "");
    srows::addLine(e, line);
  } else {
    for (int i = 0; i < 3; ++i) {
      snprintf(line, sizeof line, "%s line: %s / %s", kMasteryLineName[i], kMasteryLight[i], kMasteryDark[i]);
      srows::addLine(e, line);
    }
  }
}

// Save breakdown lines (doc 51 finding 7). which: 0 Fortitude, 1 Reflex, 2 Will. The class term is what is left of the
// no-effects total after misc, ability and the Conditioning feat, so the parts always add up to the shown value.
constexpr const char* kSaveAbility[3] = {"Constitution", "Dexterity", "Wisdom"};
inline void addSaveLines(cfx::Entry* e, const srows::StatsIn& in, int which) {
  const srows::SaveIn& sv = in.save[which];
  snprintf(e->val, sizeof e->val, "%d", sv.total);
  int tier = in.condTier < 0 ? 0 : (in.condTier > 3 ? 3 : in.condTier);
  srows::addSigned(e, "Class levels", sv.noEff - sv.misc - sv.ability - tier);
  srows::addSigned(e, kSaveAbility[which], sv.ability);
  if (tier) srows::addSigned(e, tier == 1 ? "Conditioning" : (tier == 2 ? "Improved Conditioning" : "Master Conditioning"), tier);
  if (sv.misc) srows::addSigned(e, "Other bonuses", sv.misc);
  if (sv.total != sv.noEff) srows::addSigned(e, "Active effects", sv.total - sv.noEff);
}

// Fills one tooltip entry for a vanilla row. have = in holds live values (false: description only, no numbers).
// Force Mastery is the exception: its static reference lines show without live values too.
// Returns false for an out-of-range row or when there is nothing to show (no lines, no description: no tip). e is memset 0 first, spell = -1, remMs = -1.
inline bool build(int row, const srows::StatsIn& in, bool have, cfx::Entry* e) {
  if (!e) return false;
  memset(e, 0, sizeof *e);
  e->spell = -1;
  e->remMs = -1;
  if (row < 0 || row >= V_COUNT) return false;
  snprintf(e->name, sizeof e->name, "%s", kTips[row].name);
  e->desc = kTips[row].desc;
  if (row == V_MASTERY) {
    addMasteryLines(e, in, have);
    return true;
  }
  if (!have) return e->desc != nullptr;
  if (row == V_FORCE) {
    // Power DC (spec B): the total, then one signed line per part, same labels as the Stats tab Power DC row.
    // UNVERIFIED in Ghidra: the base 5 and the Force Focus tiers 2/3/4 come from doc 51 line 47, not from the engine.
    // DC lines go first (max 6 lines). The cost pair is all-or-nothing: it is dropped when it no longer fits in MaxLines.
    if (in.haveFdc) {
      cstats::Breakdown dc = cstats::forceDC(in.fdc);
      char line[cfx::LineLen];
      snprintf(line, sizeof line, "Power DC %d", dc.total);
      srows::addLine(e, line);
      srows::addParts(e, dc);
    }
    if (cfx::MaxLines - e->nLines >= 2) addCostLines(e, in.goodEvil);
  } else if (row == V_DEF) {
    snprintf(e->val, sizeof e->val, "%d", in.defense);
    srows::addDefenseLines(e, in);
  } else if (row >= V_FORT && row <= V_WILL) {
    if (in.haveSaves) addSaveLines(e, in, row - V_FORT);
  } else if (row == V_ALIGN) {
    snprintf(e->val, sizeof e->val, "%d / 100", in.goodEvil);
    addCostLines(e, in.goodEvil);
    e->desc = nullptr;   // the multipliers say it; the text stays only for the no-values tip (ST6d)
  } else if (row == V_XP) {
    int nc = srows::classSlots(in.nClass);
    char line[cfx::LineLen];
    for (int i = 0; i < nc; ++i) {
      srows::classText(line, sizeof line, in.classId[i], in.classLvl[i]);
      srows::addLine(e, line);
    }
  }
  return e->nLines > 0 || e->desc;
}

}  // namespace vtips
