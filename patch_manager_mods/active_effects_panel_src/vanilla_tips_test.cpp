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

  // b. Defense with live values: base, dex, remainder as "other", stance line, description.
  {
    srows::StatsIn in{};
    in.defense = 18; in.dexMod = 4; in.stance = 0;
    cfx::Entry e{};
    KT_CHECK(build(V_DEF, in, true, &e));
    KT_CHECK(std::strcmp(e.name, "Defense") == 0 && std::strcmp(e.val, "18") == 0);
    KT_CHECK(hasLine(e, "Base 10") && hasLine(e, "Dexterity +4"));
    KT_CHECK(hasLine(e, "Armor, effects, other +4") && hasLine(e, "Total Defense stance"));
    KT_CHECK(e.desc == srows::kDefenseDesc);
  }

  // c. Remainder 0 and stance not Total: only the base and dex lines.
  {
    srows::StatsIn in{};
    in.defense = 14; in.dexMod = 4; in.stance = 0xFF;
    cfx::Entry e{};
    KT_CHECK(build(V_DEF, in, true, &e));
    KT_CHECK(e.nLines == 2 && hasLine(e, "Base 10") && hasLine(e, "Dexterity +4"));
  }

  // d. Alignment: value and both power cost multipliers.
  {
    srows::StatsIn in{};
    in.goodEvil = 72;
    cfx::Entry e{};
    KT_CHECK(build(V_ALIGN, in, true, &e));
    KT_CHECK(std::strcmp(e.val, "72 / 100") == 0);
    KT_CHECK(hasLine(e, "Light power cost x0.85") && hasLine(e, "Dark power cost x1.15"));
  }

  // e. Without live values: description only, no numbers.
  {
    srows::StatsIn in{};
    in.defense = 18; in.dexMod = 4;
    cfx::Entry e{};
    KT_CHECK(build(V_DEF, in, false, &e));
    KT_CHECK(e.val[0] == '\0' && e.nLines == 0 && e.desc != nullptr);
  }

  // f. Every row builds with a name and a description; out-of-range rows fail.
  {
    srows::StatsIn in{};
    for (int i = 0; i < V_COUNT; ++i) {
      cfx::Entry e{};
      KT_CHECK(build(i, in, true, &e));
      KT_CHECK(e.name[0] != '\0' && e.desc != nullptr);
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

  return kt::Report("vanilla_tips_test");
}
