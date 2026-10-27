// T1 host test for CombatStats.h (doc 51 ST2: pure stat math for the combat-stats page).
// Build/run: python3 scripts/run_tests.py combat_stats_test
#include "CombatStats.h"
#include "check.h"
#include <cstring>

// Returns the value of the named part, or -999 when that part is absent.
static int partValue(const cstats::Breakdown& b, const char* label) {
  for (int i = 0; i < b.count; ++i) {
    if (std::strcmp(b.parts[i].label, label) == 0) return b.parts[i].value;
  }
  return -999;
}

static bool near(float a, float b) { return a > b - 0.001f && a < b + 0.001f; }

int main() {
  // a. AJD + Force Redirection at BAB 10.
  {
    cstats::DeflectIn in{};
    in.bab = 10;
    in.hasAJD = true;
    in.hasForceRedirection = true;
    cstats::Breakdown d = cstats::deflection(in);
    KT_CHECK_EQ(d.total, 16);
    KT_CHECK_EQ(partValue(d, "Advanced Jedi Defense"), 3);
    KT_CHECK_EQ(partValue(d, "Force Redirection"), 3);
    KT_CHECK_EQ(partValue(d, "Base attack bonus"), 10);
    KT_CHECK_EQ(d.count, 3);
  }

  // b. MJD replaces AJD (never +9).
  {
    cstats::DeflectIn in{};
    in.bab = 10;
    in.hasAJD = true;
    in.hasMJD = true;
    cstats::Breakdown d = cstats::deflection(in);
    KT_CHECK_EQ(partValue(d, "Master Jedi Defense"), 6);
    KT_CHECK_EQ(partValue(d, "Advanced Jedi Defense"), -999);
    KT_CHECK_EQ(d.total, 10 + 6);
  }

  // c. Form term applies only with a lightsaber in hand (doc 51 finding 1: no attack-modifier term).
  {
    cstats::DeflectIn shien{};
    shien.bab = 10;
    shien.saber = true;
    shien.form = cstats::kFormShien;
    cstats::Breakdown d = cstats::deflection(shien);
    KT_CHECK_EQ(d.total, 12);
    KT_CHECK_EQ(partValue(d, "Shien"), 2);

    cstats::DeflectIn makashi{};
    makashi.bab = 10;
    makashi.saber = true;
    makashi.form = cstats::kFormMakashi;
    KT_CHECK_EQ(cstats::deflection(makashi).total, 5);

    cstats::DeflectIn noSaber{};
    noSaber.bab = 10;
    noSaber.saber = false;
    noSaber.form = cstats::kFormShien;
    cstats::Breakdown n = cstats::deflection(noSaber);
    KT_CHECK_EQ(n.total, 10);
    KT_CHECK_EQ(partValue(n, "Shien"), -999);
  }

  // d. Weapon Master Deflect needs WM feat, WM class and a saber; effects delta is a labelled part.
  {
    cstats::DeflectIn wm{};
    wm.bab = 10;
    wm.hasWMDeflect = true;
    wm.isWeaponMaster = true;
    wm.saber = true;
    wm.wmLevel = 5;
    KT_CHECK_EQ(partValue(cstats::deflection(wm), "Weapon Master Deflect"), 3);

    cstats::DeflectIn wmNoSaber = wm;
    wmNoSaber.saber = false;
    KT_CHECK_EQ(partValue(cstats::deflection(wmNoSaber), "Weapon Master Deflect"), -999);

    cstats::DeflectIn fx{};
    fx.bab = 10;
    fx.effectDelta = -2;
    KT_CHECK_EQ(partValue(cstats::deflection(fx), "Effects"), -2);
  }

  // e. Force point cost multiplier table (forceadjust.2da), clamped to rows 0..10.
  {
    cstats::CostMult c = cstats::fpCostMult(0);
    KT_CHECK(near(c.light, 1.5f) && near(c.dark, 0.5f));
    c = cstats::fpCostMult(50);
    KT_CHECK(near(c.light, 1.0f) && near(c.dark, 1.0f));
    c = cstats::fpCostMult(100);
    KT_CHECK(near(c.light, 0.5f) && near(c.dark, 1.5f));
    c = cstats::fpCostMult(105);
    KT_CHECK(near(c.light, 0.5f) && near(c.dark, 1.5f));
    c = cstats::fpCostMult(-5);
    KT_CHECK(near(c.light, 1.5f) && near(c.dark, 0.5f));
    c = cstats::fpCostMult(49);
    KT_CHECK(near(c.light, 1.1f) && near(c.dark, 0.9f));
  }

  // f. Force DC: only the best Force Focus tier counts; Force Scream lines 159-161 get +5 from the caller.
  {
    cstats::ForceDCIn in{};
    in.level = 12;
    in.chaMod = 2;
    in.wisMod = 4;
    in.ff2 = true;
    cstats::Breakdown d = cstats::forceDC(in);
    KT_CHECK_EQ(d.total, 26);
    KT_CHECK_EQ(partValue(d, "Improved Force Focus"), 3);

    cstats::ForceDCIn all = in;
    all.ff1 = true;
    all.ff3 = true;
    cstats::Breakdown m = cstats::forceDC(all);
    KT_CHECK_EQ(partValue(m, "Master Force Focus"), 4);
    KT_CHECK_EQ(partValue(m, "Improved Force Focus"), -999);
    KT_CHECK_EQ(partValue(m, "Force Focus"), -999);
    KT_CHECK_EQ(m.total, 27);

    KT_CHECK(cstats::isForceScreamLine(159));
    KT_CHECK(cstats::isForceScreamLine(161));
    KT_CHECK(!cstats::isForceScreamLine(158));
    KT_CHECK(!cstats::isForceScreamLine(162));
    KT_CHECK_EQ(d.total + cstats::kForceScreamDCBonus, 31);
  }

  // g. Crit threat range, minimum roll clamp, and crit multiplier.
  {
    cstats::CritRange r = cstats::critRange(2, true, 0);
    KT_CHECK_EQ(r.threat, 4);
    KT_CHECK_EQ(r.minRoll, 17);

    r = cstats::critRange(1, false, 0);
    KT_CHECK_EQ(r.threat, 1);
    KT_CHECK_EQ(r.minRoll, 20);

    r = cstats::critRange(2, true, 3);
    KT_CHECK_EQ(r.threat, 10);
    KT_CHECK_EQ(r.minRoll, 11);

    r = cstats::critRange(10, true, 3);
    KT_CHECK_EQ(r.minRoll, 1);

    KT_CHECK_EQ(cstats::critMult(2, true, true), 4);
    KT_CHECK_EQ(cstats::critMult(2, false, false), 2);
  }

  // h. Form defence modifiers (vsOthers / vsTarget) from the doc 51 catalog.
  {
    struct Row {
      int32_t form;
      int vsOthers;
      int vsTarget;
    };
    static const Row rows[] = {{258, 3, 0},  {260, 0, 2},  {261, -2, 3}, {262, 0, -5},
                               {263, 1, 1},  {264, -4, -2}, {259, 0, 0},  {0, 0, 0}};
    for (const Row& row : rows) {
      cstats::FormDef f = cstats::formDefense(row.form);
      KT_CHECK_EQ(f.vsOthers, row.vsOthers);
      KT_CHECK_EQ(f.vsTarget, row.vsTarget);
    }
  }

  return kt::Report("combat_stats_test");
}
