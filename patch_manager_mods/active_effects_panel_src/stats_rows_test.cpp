// T1 host test for StatsRows.h (doc 51 Stats tab: the pure row builder behind the combat-stats page).
// Build/run: python3 scripts/run_tests.py stats_rows_test
#include "StatsRows.h"
#include "check.h"
#include <cstring>

// Index of the first row named name in out[0..n), or -1.
static int findRow(const cfx::Entry* out, int n, const char* name) {
  for (int i = 0; i < n; ++i) {
    if (std::strcmp(out[i].name, name) == 0) return i;
  }
  return -1;
}

// True when one tooltip line of e equals text.
static bool hasLine(const cfx::Entry& e, const char* text) {
  for (int i = 0; i < e.nLines; ++i) {
    if (std::strcmp(e.lines[i], text) == 0) return true;
  }
  return false;
}

// Input that writes every row: combat with an off hand, a Force DC, a non-zero alignment.
static srows::StatsIn fullIn() {
  srows::StatsIn in{};
  in.defl.bab = 10;
  in.defl.hasAJD = true;
  in.defl.hasForceRedirection = true;
  in.ungated = 2;
  in.defense = 14;
  in.dexMod = 2;
  in.haveCombat = true;
  in.atkMain = 14;
  in.atkOff = 9;
  in.critThreatMain = 2;
  in.critMultMain = 2;
  in.critThreatOff = 1;
  in.critMultOff = 3;
  in.hasOff = true;
  in.fdc.level = 8;
  in.fdc.chaMod = 2;
  in.fdc.wisMod = 1;
  in.goodEvil = 72;
  in.forceResist = 6;
  return in;
}

int main() {
  cfx::Entry out[64] = {};

  // a. AJD + Force Redirection at BAB 10: Deflection 16, parts, then the fixed note as the last line.
  {
    srows::StatsIn in{};
    in.defl.bab = 10;
    in.defl.hasAJD = true;
    in.defl.hasForceRedirection = true;
    int n = srows::build(in, out, 64);
    int r = findRow(out, n, "Deflection");
    KT_CHECK(r >= 0);
    if (r >= 0) {
      const cfx::Entry& e = out[r];
      KT_CHECK(std::strcmp(e.val, "16") == 0);
      KT_CHECK(hasLine(e, "Base attack bonus +10"));
      KT_CHECK(hasLine(e, "Advanced Jedi Defense +3"));
      KT_CHECK(hasLine(e, "Force Redirection +3"));
      KT_CHECK(e.nLines == 4);
      KT_CHECK(std::strcmp(e.lines[e.nLines - 1], cstats::kDeflectNote) == 0);
      KT_CHECK(e.desc != nullptr);
    }
  }

  // b. Critical range: threat 2 -> "19-20 x2"; threat 1 -> "20 x2".
  {
    srows::StatsIn in{};
    in.haveCombat = true;
    in.critThreatMain = 2;
    in.critMultMain = 2;
    int n = srows::build(in, out, 64);
    int r = findRow(out, n, "Critical (main hand)");
    KT_CHECK(r >= 0 && std::strcmp(out[r].val, "19-20 x2") == 0);
    in.critThreatMain = 1;
    n = srows::build(in, out, 64);
    r = findRow(out, n, "Critical (main hand)");
    KT_CHECK(r >= 0 && std::strcmp(out[r].val, "20 x2") == 0);
  }

  // c. Alignment 72: Light x0.85, Dark x1.15, Alignment "72 / 100".
  {
    srows::StatsIn in{};
    in.goodEvil = 72;
    int n = srows::build(in, out, 64);
    int l = findRow(out, n, "Light power cost");
    int d = findRow(out, n, "Dark power cost");
    int a = findRow(out, n, "Alignment");
    KT_CHECK(l >= 0 && std::strcmp(out[l].val, "x0.85") == 0);
    KT_CHECK(d >= 0 && std::strcmp(out[d].val, "x1.15") == 0);
    KT_CHECK(a >= 0 && std::strcmp(out[a].val, "72 / 100") == 0);
  }

  // d. No combat: no Offense section and no attack rows.
  {
    srows::StatsIn in{};
    in.haveCombat = false;
    in.hasOff = true;
    int n = srows::build(in, out, 64);
    KT_CHECK(findRow(out, n, "Offense") < 0);
    KT_CHECK(findRow(out, n, "Attack (main hand)") < 0);
  }

  // e. Off-hand rows only when hasOff.
  {
    srows::StatsIn in{};
    in.haveCombat = true;
    in.hasOff = false;
    int n = srows::build(in, out, 64);
    KT_CHECK(findRow(out, n, "Offense") >= 0);
    KT_CHECK(findRow(out, n, "Attack (off hand)") < 0);
    KT_CHECK(findRow(out, n, "Critical (off hand)") < 0);
    in.hasOff = true;
    n = srows::build(in, out, 64);
    KT_CHECK(findRow(out, n, "Attack (off hand)") >= 0);
    KT_CHECK(findRow(out, n, "Critical (off hand)") >= 0);
  }

  // f. Cap: exactly cap rows are written and the slot past cap is untouched.
  {
    srows::StatsIn in = fullIn();
    KT_CHECK(srows::build(in, out, 64) > 3);
    cfx::Entry capped[4] = {};
    std::strcpy(capped[3].name, "SENTINEL");
    int n = srows::build(in, capped, 3);
    KT_CHECK_EQ(n, 3);
    KT_CHECK(std::strcmp(capped[3].name, "SENTINEL") == 0);
  }

  // g. No row is an attacks-per-round row.
  {
    srows::StatsIn in = fullIn();
    int n = srows::build(in, out, 64);
    bool any = false;
    for (int i = 0; i < n; ++i) {
      if (std::strstr(out[i].name, "Attacks") != nullptr) any = true;
    }
    KT_CHECK(!any);
  }

  // h. Section headers come first; Damage resist shows "none" when empty and the text otherwise.
  {
    srows::StatsIn in{};
    int n = srows::build(in, out, 64);
    KT_CHECK(n > 0 && out[0].section && std::strcmp(out[0].name, "Defense") == 0);
    int r = findRow(out, n, "Damage resist");
    KT_CHECK(r >= 0 && std::strcmp(out[r].val, "none") == 0);
    std::strcpy(in.resist, "Energy 5");
    n = srows::build(in, out, 64);
    r = findRow(out, n, "Damage resist");
    KT_CHECK(r >= 0 && std::strcmp(out[r].val, "Energy 5") == 0 && hasLine(out[r], "Energy 5"));
  }

  return kt::Report("stats_rows_test");
}
