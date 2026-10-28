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

// Row name and description (static literals). The description is shown with or without live values.
struct Tip { const char* name; const char* desc; };
constexpr Tip kTips[V_COUNT] = {
  {"Strength", "Adds its modifier to melee attack and melee damage rolls."},
  {"Dexterity", "Adds its modifier to Defense, ranged attack rolls and Reflex saves."},
  {"Constitution", "Adds its modifier to the vitality gained each level and to Fortitude saves."},
  {"Intelligence", "Adds its modifier to the skill points gained each level."},
  {"Wisdom", "Adds its modifier to Will saves and to the save DC of your Force powers."},
  {"Charisma", "Adds its modifier to the save DC of your Force powers."},
  {"Vitality", "Damage lowers vitality; at 0 the character falls."},
  {"Force points", "Force points pay for Force powers. Your alignment changes what light and dark side powers cost."},
  {"Defense", srows::kDefenseDesc},
  {"Fortitude", "Saving throw against poison, disease and similar physical effects: d20 + this must meet the effect's DC."},
  {"Reflex", "Saving throw against grenades, mines and other area effects: d20 + this must meet the effect's DC."},
  {"Will", "Saving throw against mind-affecting Force powers such as Stun and Fear: d20 + this must meet the effect's DC."},
  {"Alignment", "Above 50 is the light side, below 50 the dark side. Alignment changes what Force powers cost."},
  {"Experience", "Experience points earned so far."},
  {"Next level", "Experience points needed for the next level."},
  {"Force Mastery", "Reached only at alignment exactly 100 (light side) or exactly 0 (dark side), and lost as soon as alignment moves."},
  {"Effects / Stats", "Click the header to switch between active effects and combat stats."},
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

// Fills one tooltip entry for a vanilla row. have = in holds live values (false: description only, no numbers).
// Force Mastery is the exception: its static reference lines show without live values too.
// Returns false for an out-of-range row. e is memset 0 first, spell = -1, remMs = -1.
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
  if (!have) return true;
  if (row == V_FORCE) {
    addCostLines(e, in.goodEvil);
  } else if (row == V_DEF) {
    snprintf(e->val, sizeof e->val, "%d", in.defense);
    srows::addLine(e, "Base 10");
    srows::addSigned(e, "Dexterity", in.dexMod);
    int other = in.defense - 10 - in.dexMod;   // armor, active effects and other sources
    if (other != 0) srows::addSigned(e, "Armor, effects, other", other);
    if (in.stance == 0) srows::addLine(e, "Total Defense stance");
  } else if (row == V_ALIGN) {
    snprintf(e->val, sizeof e->val, "%d / 100", in.goodEvil);
    addCostLines(e, in.goodEvil);
  } else if (row == V_XP) {
    int nc = srows::classSlots(in.nClass);
    char line[cfx::LineLen];
    for (int i = 0; i < nc; ++i) {
      srows::classText(line, sizeof line, in.classId[i], in.classLvl[i]);
      srows::addLine(e, line);
    }
  }
  return true;
}

}  // namespace vtips
