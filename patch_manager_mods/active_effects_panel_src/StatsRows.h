// StatsRows.h - pure row builder for the Stats tab of the combat-stats page (doc 51 ST3 mockup, ST4 feed).
// Turns the CombatStats.h breakdowns and plain character values into cfx::Entry rows (section headers and
// value rows with a tooltip). No engine access, no windows.h, no STL: the DLL fills StatsIn from game state.
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "CombatStats.h"
#include "ChareffDecode.h"

namespace srows {

// Everything the Stats tab shows, as plain values.
struct StatsIn {
  cstats::DeflectIn defl; int ungated;          // stats+0x178/+0x179 sum (int8)
  int defense, dexMod; uint8_t stance;          // GetArmorClass, GetDEXMod(stats,1), creature+0x110C
  bool haveCombat; int atkMain, atkOff, critThreatMain, critMultMain, critThreatOff, critMultOff; bool hasOff;
  cstats::ForceDCIn fdc; int goodEvil, forceResist;
  char resist[48];                              // short damage-resist text, "" = none
  bool haveInfluence; int npcSlot, influence;   // party NPC slot 0..11 (nwscript NPC_*) + party table +0x11C+slot*4, -1 = unset
  int nClass; uint8_t classId[3]; int classLvl[3];   // class slots in slot order (stats+0xAB+i*0x28); the last one picks the Mastery line
  int mastery;                                       // 0 none, 1 Light Mastery effect present, 2 Dark Mastery effect present
};

// Party NPC slot -> tag, as the engine's fallback chain in HandleAlignmentInfluence 0x0056AE40 matches it (doc 51 ST5b).
constexpr const char* kNpcTags[12] = {"atton", "baodur", "mand", "g0t0", "handmaiden", "hk47",
                                      "kreia", "mira", "t3m4", "visasmarr", "hanharr", "disciple"};
constexpr int kNpcKreia = 6;
constexpr const char* kInfluenceDesc =
    "How much this companion is swayed by you. Above 50 their alignment drifts toward yours, below 50 away from it.";

// Class id -> name, classes.2da rows 0..16 (name strrefs, dialog.tlk).
constexpr const char* kClassNames[17] = {"Soldier","Scout","Scoundrel","Jedi Guardian","Jedi Consular","Jedi Sentinel",
                                         "Combat Droid","Expert Droid","Minion","Tech Specialist","Bounty Hunter",
                                         "Jedi Weapon Master","Jedi Master","Jedi Watchman","Sith Marauder","Sith Lord",
                                         "Sith Assassin"};

constexpr const char* kDeflectDesc =
    "Your d20 roll plus this total is checked against each blaster bolt; beat it by 6 to reflect.";
constexpr const char* kDefenseDesc = "Attackers roll d20 + attack bonus against this.";

// Hands out zeroed slots from out[0..cap). Every write goes through next(), so nothing lands past cap.
struct Writer {
  cfx::Entry* out; int cap; int n;
  cfx::Entry* next() {
    if (n >= cap) return nullptr;
    cfx::Entry* e = &out[n++];
    memset(e, 0, sizeof *e);
    e->spell = -1;
    e->remMs = -1;
    return e;
  }
};

// Section header: section = true, name = title.
inline void section(Writer& w, const char* title) {
  cfx::Entry* e = w.next();
  if (e) {
    e->section = true;
    snprintf(e->name, sizeof e->name, "%s", title);
  }
}

// Value row: name, value text (cut to val[24]), description. Returns nullptr once the cap is reached.
inline cfx::Entry* row(Writer& w, const char* name, const char* val, const char* desc) {
  cfx::Entry* e = w.next();
  if (e) {
    snprintf(e->name, sizeof e->name, "%s", name);
    snprintf(e->val, sizeof e->val, "%s", val);
    e->desc = desc;
  }
  return e;
}

// Tooltip line. Lines past MaxLines are dropped; a null entry (past cap) is ignored.
inline void addLine(cfx::Entry* e, const char* text) {
  if (!e || e->nLines >= cfx::MaxLines) return;
  snprintf(e->lines[e->nLines], cfx::LineLen, "%s", text);
  ++e->nLines;
}

// Tooltip line "<label> +N" or "<label> -N".
inline void addSigned(cfx::Entry* e, const char* label, int value) {
  char buf[cfx::LineLen];
  snprintf(buf, sizeof buf, "%s %+d", label, value);
  addLine(e, buf);
}

// One signed tooltip line per breakdown part.
inline void addParts(cfx::Entry* e, const cstats::Breakdown& b) {
  for (int i = 0; i < b.count; ++i) addSigned(e, b.parts[i].label, b.parts[i].value);
}

// Multiplier text: "x0.85", "x1.5", "x1.0" (2 places, trailing zeros trimmed to 1 decimal at least).
inline void multText(char* out, size_t n, float m) {
  int h = static_cast<int>(m * 100.0f + 0.5f);   // hundredths
  if (h % 10 == 0) {
    snprintf(out, n, "x%d.%d", h / 100, (h % 100) / 10);
  } else {
    snprintf(out, n, "x%d.%02d", h / 100, h % 100);
  }
}

// Critical row: "19-20 x2" for threat 2; "20 x2" when threat <= 1 (only a natural 20 crits).
inline void addCrit(Writer& w, const char* name, int threat, int mult) {
  char range[8], val[24], line[cfx::LineLen];
  if (threat <= 1) {
    snprintf(range, sizeof range, "%s", "20");
  } else {
    int minRoll = 21 - threat;
    if (minRoll < 1) minRoll = 1;
    snprintf(range, sizeof range, "%d-20", minRoll);
  }
  snprintf(val, sizeof val, "%s x%d", range, mult);
  cfx::Entry* e = row(w, name, val, nullptr);
  snprintf(line, sizeof line, "Threat range %s", range);
  addLine(e, line);
  snprintf(line, sizeof line, "Multiplier x%d", mult);
  addLine(e, line);
}

// Class slots in use, clamped to the 3 stored slots.
inline int classSlots(int n) { return n < 0 ? 0 : (n > 3 ? 3 : n); }

// Class line text: "Jedi Guardian 8"; ids outside the name table read "Class 23 8".
inline void classText(char* out, size_t n, int id, int lvl) {
  if (id >= 0 && id < 17) snprintf(out, n, "%s %d", kClassNames[id], lvl);
  else snprintf(out, n, "Class %d %d", id, lvl);
}

// Writes the Stats tab rows into out[0..cap) in tab order. Returns the count written, never more than cap.
inline int build(const StatsIn& in, cfx::Entry* out, int cap) {
  Writer w{out, cap, 0};
  char val[24];

  section(w, "Defense");
  {
    cstats::Breakdown d = cstats::deflection(in.defl);
    snprintf(val, sizeof val, "%d", d.total + in.ungated);
    cfx::Entry* e = row(w, "Deflection", val, kDeflectDesc);
    addParts(e, d);
    if (in.ungated != 0) addSigned(e, "Ungated", in.ungated);
    addLine(e, cstats::kDeflectNote);
  }
  {
    snprintf(val, sizeof val, "%d", in.defense);
    cfx::Entry* e = row(w, "Defense", val, kDefenseDesc);
    addSigned(e, "Dexterity", in.dexMod);
    if (in.stance == 0) addLine(e, "Total Defense stance");
  }
  {
    // Full text is the first tooltip line; val gets a copy cut to 23 chars by row().
    char text[sizeof in.resist + 1];
    snprintf(text, sizeof text, "%.*s", static_cast<int>(sizeof in.resist), in.resist);
    cfx::Entry* e = row(w, "Damage resist", text[0] ? text : "none", nullptr);
    if (text[0]) addLine(e, text);
  }

  if (in.haveCombat) {
    section(w, "Offense");
    snprintf(val, sizeof val, "%+d", in.atkMain);
    row(w, "Attack (main hand)", val, nullptr);
    if (in.hasOff) {
      snprintf(val, sizeof val, "%+d", in.atkOff);
      row(w, "Attack (off hand)", val, nullptr);
    }
    addCrit(w, "Critical (main hand)", in.critThreatMain, in.critMultMain);
    if (in.hasOff) addCrit(w, "Critical (off hand)", in.critThreatOff, in.critMultOff);
  }

  section(w, "Force");
  {
    cstats::Breakdown p = cstats::forceDC(in.fdc);
    snprintf(val, sizeof val, "%d", p.total);
    addParts(row(w, "Power DC", val, nullptr), p);
  }
  {
    cstats::CostMult m = cstats::fpCostMult(in.goodEvil);
    multText(val, sizeof val, m.light);
    row(w, "Light power cost", val, nullptr);
    multText(val, sizeof val, m.dark);
    row(w, "Dark power cost", val, nullptr);
  }
  snprintf(val, sizeof val, "%d", in.forceResist);
  row(w, "Force resist", val, nullptr);

  section(w, "Character");
  {
    int nc = classSlots(in.nClass);
    if (nc > 0) {
      int total = 0;
      for (int i = 0; i < nc; ++i) total += in.classLvl[i];
      snprintf(val, sizeof val, "%d", total);
      cfx::Entry* e = row(w, "Level", val, nullptr);
      char line[cfx::LineLen];
      for (int i = 0; i < nc; ++i) {
        classText(line, sizeof line, in.classId[i], in.classLvl[i]);
        addLine(e, line);
      }
    }
  }
  snprintf(val, sizeof val, "%d / 100", in.goodEvil);
  row(w, "Alignment", val, nullptr);
  if (in.haveInfluence && in.npcSlot >= 0 && in.npcSlot < 12) {
    if (in.influence < 0) snprintf(val, sizeof val, "--");
    else snprintf(val, sizeof val, "%d / 100", in.influence);
    cfx::Entry* e = row(w, "Influence", val, kInfluenceDesc);
    char line[cfx::LineLen];
    snprintf(line, sizeof line, "Party slot %d (%s)", in.npcSlot, kNpcTags[in.npcSlot]);
    addLine(e, line);
    addLine(e, "Neutral is 50");
    if (in.influence < 0) addLine(e, "Not set yet: the game starts it from the companion's base value");
    if (in.npcSlot == kNpcKreia) addLine(e, "Kreia's alignment does not follow influence");
  }
  return w.n;
}

}  // namespace srows
