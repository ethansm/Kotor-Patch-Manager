// T1 host test for VanillaTips.h (doc 51 ST6: hover table + tooltip builder for the vanilla character page rows).
// Build/run: python3 scripts/run_tests.py vanilla_tips_test
#include "VanillaTips.h"
#include "check.h"
#include <cstring>

using namespace vtips;

// True when one tooltip line of e equals text.
static bool hasLine(const cfx::Entry& e, const char* text) {
  for (int i = 0; i < e.nLines; ++i) {
    if (std::strcmp(e.lines[i], text) == 0) return true;
  }
  return false;
}

int main() {
  // a. Hover table: first match wins; gaps and the x < 304 strip beside Alignment miss.
  KT_CHECK(hit(100, 330) == V_DEF);
  KT_CHECK(hit(300, 180) == -1);
  KT_CHECK(hit(310, 300) == V_ALIGN);
  KT_CHECK(hit(10, 10) == -1);
  KT_CHECK(hit(167, 440) == V_XP);
  KT_CHECK(hit(200, 440) == V_NEXT);

  // b. Defense with live values: base, dex, remainder as "other", stance line, no description.
  {
    srows::StatsIn in{};
    in.defense = 18; in.dexMod = 4; in.stance = 0;
    cfx::Entry e{};
    KT_CHECK(build(V_DEF, in, true, &e));
    KT_CHECK(std::strcmp(e.name, "Defense") == 0 && std::strcmp(e.val, "18") == 0);
    KT_CHECK(hasLine(e, "Base 10") && hasLine(e, "Dexterity +4"));
    KT_CHECK(hasLine(e, "Armor, effects, other +4") && hasLine(e, "Total Defense stance"));
    KT_CHECK(e.desc == nullptr);   // ST6e: the lines say it
  }

  // c. Remainder 0 and stance not Total: only the base and dex lines.
  {
    srows::StatsIn in{};
    in.defense = 14; in.dexMod = 4; in.stance = 0xFF;
    cfx::Entry e{};
    KT_CHECK(build(V_DEF, in, true, &e));
    KT_CHECK(e.nLines == 2 && hasLine(e, "Base 10") && hasLine(e, "Dexterity +4"));
  }

  // d. Alignment: value and both power cost multipliers, no description.
  {
    srows::StatsIn in{};
    in.goodEvil = 72;
    cfx::Entry e{};
    KT_CHECK(build(V_ALIGN, in, true, &e));
    KT_CHECK(std::strcmp(e.val, "72 / 100") == 0);
    KT_CHECK(hasLine(e, "Light power cost x0.85") && hasLine(e, "Dark power cost x1.15"));
    KT_CHECK(e.desc == nullptr && e.nLines == 2);   // ST6d: multipliers only
    KT_CHECK(build(V_ALIGN, in, false, &e) && e.desc != nullptr);   // no live values: the description stays
  }

  // e. Without live values: description only, no numbers; a row with no description then has no tip.
  {
    srows::StatsIn in{};
    in.defense = 18; in.dexMod = 4;
    cfx::Entry e{};
    KT_CHECK(build(V_STR, in, false, &e));
    KT_CHECK(e.val[0] == '\0' && e.nLines == 0 && e.desc != nullptr);
    KT_CHECK(!build(V_DEF, in, false, &e));
  }

  // f. Every row builds with a name; rows whose label says it all (Vitality, Next level, Experience without classes) have no tip.
  {
    srows::StatsIn in{};
    for (int i = 0; i < V_COUNT; ++i) {
      cfx::Entry e{};
      bool none = i == V_VIT || i == V_NEXT || i == V_XP;
      KT_CHECK(build(i, in, true, &e) == !none);
      KT_CHECK(e.name[0] != '\0');
    }
    cfx::Entry e{};
    KT_CHECK(!build(V_COUNT, in, true, &e));
    KT_CHECK(!build(-1, in, true, &e));
  }

  // g. Hover rects are non-empty and inside the 800x600 GUI extent.
  for (int i = 0; i < V_COUNT; ++i) {
    const Rect& r = kRects[i];
    KT_CHECK(r.w > 0 && r.h > 0 && r.x >= 0 && r.y >= 0 && r.x + r.w <= 800 && r.y + r.h <= 600);
  }

  // h. Hover rects for the two new rows: Force Mastery and the Effects/Stats header.
  KT_CHECK(hit(400, 110) == V_MASTERY);
  KT_CHECK(hit(600, 115) == V_HEADER);

  // i. Mastery: the last class slot picks the line (Jedi Master here, not the first slot); active effect marked.
  {
    srows::StatsIn in{};
    in.nClass = 2; in.classId[0] = 3; in.classId[1] = 12; in.classLvl[0] = 10; in.classLvl[1] = 2;
    in.mastery = 1;
    cfx::Entry e{};
    KT_CHECK(build(V_MASTERY, in, true, &e));
    KT_CHECK(e.nLines == 3 && hasLine(e, "Class line: Consular") &&
             hasLine(e, "Light Mastery: +3 Wisdom (active)") && hasLine(e, "Dark Mastery: +50 Force points"));
  }

  // j. Mastery without a class line: the three static reference lines.
  {
    srows::StatsIn in{};
    cfx::Entry e{};
    KT_CHECK(build(V_MASTERY, in, true, &e));
    KT_CHECK(e.nLines == 3 && std::strncmp(e.lines[0], "Guardian line:", 14) == 0);
  }

  // k. Experience: one line per class slot.
  {
    srows::StatsIn in{};
    in.nClass = 2; in.classId[0] = 3; in.classId[1] = 11; in.classLvl[0] = 8; in.classLvl[1] = 4;
    cfx::Entry e{};
    KT_CHECK(build(V_XP, in, true, &e));
    KT_CHECK(e.nLines == 2 && hasLine(e, "Jedi Guardian 8") && hasLine(e, "Jedi Weapon Master 4"));
  }

  // l. Effects/Stats header: description only, no lines.
  {
    srows::StatsIn in{};
    cfx::Entry e{};
    KT_CHECK(build(V_HEADER, in, true, &e));
    KT_CHECK(e.desc != nullptr && e.nLines == 0);
  }

  // s1. Fortitude: class term = noEff - misc - ability - feat; effects = total - noEff; parts add up to the value.
  {
    srows::StatsIn in{};
    in.haveSaves = true; in.condTier = 2;
    in.save[0] = {11, 9, 1, 2};
    cfx::Entry e{};
    KT_CHECK(build(V_FORT, in, true, &e));
    KT_CHECK(std::strcmp(e.name, "Fortitude") == 0 && std::strcmp(e.val, "11") == 0);
    KT_CHECK(hasLine(e, "Class levels +4") && hasLine(e, "Constitution +2"));
    KT_CHECK(hasLine(e, "Improved Conditioning +2") && hasLine(e, "Other bonuses +1"));
    KT_CHECK(hasLine(e, "Active effects +2") && e.nLines == 5);
  }

  // s2. Will with a negative ability, no feat, no misc, an effect penalty; Reflex picks Dexterity.
  {
    srows::StatsIn in{};
    in.haveSaves = true;
    in.save[2] = {1, 3, 0, -1};
    in.save[1] = {5, 5, 0, 3};
    cfx::Entry e{};
    KT_CHECK(build(V_WILL, in, true, &e));
    KT_CHECK(std::strcmp(e.val, "1") == 0 && hasLine(e, "Class levels +4") && hasLine(e, "Wisdom -1"));
    KT_CHECK(hasLine(e, "Active effects -2") && e.nLines == 3);
    KT_CHECK(build(V_REF, in, true, &e));
    KT_CHECK(hasLine(e, "Dexterity +3") && hasLine(e, "Class levels +2") && e.nLines == 2);
  }

  // s3. No save data (read failed): description only, no value.
  {
    srows::StatsIn in{};
    cfx::Entry e{};
    KT_CHECK(build(V_FORT, in, true, &e));
    KT_CHECK(e.nLines == 0 && e.val[0] == 0 && e.desc != nullptr);
  }

  return kt::Report("vanilla_tips_test");
}
