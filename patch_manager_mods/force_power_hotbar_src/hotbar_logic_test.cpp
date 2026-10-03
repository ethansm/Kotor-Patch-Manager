// Pure-logic tests for the hotbar model (U2: picking an action already on the bar MOVES it).
#include "HotbarConfig.h"
#include "HotbarLogic.h"
#include "check.h"
using namespace hb;

static ActionRef mk(Kind k, int id, const char* name = "") { ActionRef r; r.kind = k; r.id = id; r.name = name; return r; }

int main() {
  const ActionRef spell8 = mk(K_Spell, 8);

  { // a. a move clears the old slot (explicit none + "moved" note); the target slot is the caller's job
    Profile p; p.slots[0][2] = spell8;
    std::vector<int> r = moveDuplicates(p, 0, 5, spell8);
    KT_CHECK_EQ(r.size(), 1);
    KT_CHECK(r.size() == 1 && r[0] == 2);
    KT_CHECK(p.slots[0][2].empty() && p.slots[0][2].blank);
    KT_CHECK_EQ(p.notes[0][2].c_str(), "moved");
    KT_CHECK(p.slots[0][5].empty() && !p.slots[0][5].blank);   // untouched
  }
  { // b. an empty/blank ref (the Clear path) never matches anything
    Profile p; p.slots[0][1].blank = true; p.notes[0][1] = "keep";
    KT_CHECK(moveDuplicates(p, 0, 5, ActionRef()).empty());
    ActionRef bl; bl.blank = true;
    KT_CHECK(moveDuplicates(p, 0, 5, bl).empty());
    KT_CHECK(p.slots[0][1].blank && p.slots[0][0].empty() && !p.slots[0][0].blank);
    KT_CHECK_EQ(p.notes[0][1].c_str(), "keep");
    KT_CHECK_EQ(p.notes[0][0].c_str(), "");
  }
  { // c. item names compare case-insensitively
    Profile p; p.slots[0][1] = mk(K_Item, 0, "Medpac");
    std::vector<int> r = moveDuplicates(p, 0, 3, mk(K_Item, 0, "MEDPAC"));
    KT_CHECK(r.size() == 1 && r[0] == 1);
    KT_CHECK(p.slots[0][1].empty() && p.slots[0][1].blank);
  }
  { // d. spell vs feat with the same id are different actions
    Profile p; p.slots[0][1] = mk(K_Feat, 8);
    KT_CHECK(moveDuplicates(p, 0, 3, spell8).empty());
    KT_CHECK(p.slots[0][1].kind == K_Feat && !p.slots[0][1].blank);
    KT_CHECK_EQ(p.notes[0][1].c_str(), "");
  }
  { // e. picking into its own slot is a no-op
    Profile p; p.slots[0][4] = spell8; p.notes[0][4] = "mine";
    KT_CHECK(moveDuplicates(p, 0, 4, spell8).empty());
    KT_CHECK(p.slots[0][4].kind == K_Spell && p.slots[0][4].id == 8);
    KT_CHECK_EQ(p.notes[0][4].c_str(), "mine");
  }
  { // f. hand-edited INI with several duplicates: all cleared, ascending
    Profile p; p.slots[0][7] = spell8; p.slots[0][0] = spell8; p.slots[0][3] = spell8;
    std::vector<int> r = moveDuplicates(p, 0, 5, spell8);
    KT_CHECK_EQ(r.size(), 3);
    KT_CHECK(r.size() == 3 && r[0] == 0 && r[1] == 3 && r[2] == 7);
    for (int i : { 0, 3, 7 }) KT_CHECK(p.slots[0][i].empty() && p.slots[0][i].blank);
  }
  { // g. Serialize shows the moved slot as "none" with its note, and the text re-parses cleanly
    Config c; Profile& p = c.profile("obj00000001"); p.slots[0][2] = spell8;
    moveDuplicates(p, 0, 6, spell8);
    std::string s = c.Serialize();
    KT_CHECK(s.find("Bottom3=none   ; moved") != std::string::npos);
    Config d; d.Parse(s);
    KT_CHECK(d.warnings.empty());
    KT_CHECK(d.slotFor("obj00000001", 0, 2).blank);
  }
  { // h. blank (not empty) so the Default fallback cannot resurrect the moved action
    Config c; c.Parse("[Profile.Default]\nBottom3=spell:8\n");
    Profile& p = c.profile("obj1"); p.slots[0][2] = spell8;   // frozen from Default, as the editor does
    moveDuplicates(p, 0, 6, spell8);
    ActionRef r = c.slotFor("obj1", 0, 2);
    KT_CHECK(r.empty());
    KT_CHECK(r.blank);
  }
  { // out-of-range row is a no-op
    Profile p; p.slots[0][2] = spell8;
    KT_CHECK(moveDuplicates(p, -1, 5, spell8).empty());
    KT_CHECK(moveDuplicates(p, Rows, 5, spell8).empty());
    KT_CHECK(p.slots[0][2].kind == K_Spell);
  }
  // ---- lowest-HP item targeting (chooseTarget / slotEnabled / isTargetableItemType) ----
  auto C = [](int cur, int max, bool ok = true, bool down = false, bool elig = true) { Candidate c; c.cur = cur; c.max = max; c.ok = ok; c.down = down; c.eligible = elig; return c; };
  { // threshold edge: strictly below
    Candidate m[1] = { C(50, 100) };
    KT_CHECK_EQ(chooseTarget(m, 1, 0, 50, false), -1);          // exactly 50% does not qualify
    Candidate k[2] = { C(100, 100), C(49, 100) };
    KT_CHECK_EQ(chooseTarget(k, 2, 0, 50, false), 1);           // 49% does
    Candidate o[2] = { C(75, 75), C(37, 75) };                  // 37/75 = 49.33%
    KT_CHECK_EQ(chooseTarget(o, 2, 0, 50, false), 1);
    Candidate q[2] = { C(75, 75), C(38, 75) };                  // 50.67%
    KT_CHECK_EQ(chooseTarget(q, 2, 0, 50, false), -1);
  }
  { // lowest among several
    Candidate m[4] = { C(90, 100), C(40, 100), C(10, 100), C(30, 100) };
    KT_CHECK_EQ(chooseTarget(m, 4, 0, 50, false), 2);
  }
  { // ratio compare across different max: 30/100 (30%) vs 20/50 (40%)
    Candidate m[3] = { C(100, 100), C(20, 50), C(30, 100) };
    KT_CHECK_EQ(chooseTarget(m, 3, 0, 50, false), 2);
    Candidate n[3] = { C(100, 100), C(30, 100), C(20, 50) };
    KT_CHECK_EQ(chooseTarget(n, 3, 0, 50, false), 1);
  }
  { // ties: self preferred, else lowest index
    Candidate m[3] = { C(50, 100), C(25, 50), C(10, 100) };
    m[2] = C(25, 50);
    KT_CHECK_EQ(chooseTarget(m, 3, 2, 80, false), 2);           // 50%,50%,50% -> self (2)
    KT_CHECK_EQ(chooseTarget(m, 3, 0, 80, false), 0);
    Candidate t[3] = { C(100, 100), C(20, 100), C(40, 200) };   // 20% vs 20%, self 0 not tied
    KT_CHECK_EQ(chooseTarget(t, 3, 0, 80, false), 1);
    Candidate u[3] = { C(100, 100), C(30, 100), C(30, 100) };
    KT_CHECK_EQ(chooseTarget(u, 3, 0, 80, true), 1);            // lowest index, not self-fallback
  }
  { // all down: fallback self only if self is not down
    Candidate m[2] = { C(0, 100, true, true), C(0, 100, true, true) };
    KT_CHECK_EQ(chooseTarget(m, 2, 0, 50, true), -1);
    Candidate n[2] = { C(80, 100), C(0, 100, true, true) };
    KT_CHECK_EQ(chooseTarget(n, 2, 0, 50, true), 0);
    KT_CHECK_EQ(chooseTarget(n, 2, 0, 50, false), -1);
  }
  { // all ineligible (repair kit, no droids): fallback self only if self eligible
    Candidate m[2] = { C(10, 100, true, false, false), C(20, 100, true, false, false) };
    KT_CHECK_EQ(chooseTarget(m, 2, 0, 50, true), -1);
    Candidate n[2] = { C(90, 100, true, false, true), C(20, 100, true, false, false) };
    KT_CHECK_EQ(chooseTarget(n, 2, 0, 50, true), 0);
  }
  { // single member
    Candidate m[1] = { C(10, 100) };
    KT_CHECK_EQ(chooseTarget(m, 1, 0, 50, false), 0);
    Candidate n[1] = { C(60, 100) };
    KT_CHECK_EQ(chooseTarget(n, 1, 0, 50, true), 0);
    KT_CHECK_EQ(chooseTarget(n, 1, 0, 50, false), -1);
  }
  { // threshold 100: hurt qualifies, full does not; 0: nobody, fallback decides; clamped
    Candidate m[2] = { C(100, 100), C(99, 100) };
    KT_CHECK_EQ(chooseTarget(m, 2, 0, 100, false), 1);
    Candidate f[2] = { C(100, 100), C(50, 50) };
    KT_CHECK_EQ(chooseTarget(f, 2, 0, 100, false), -1);
    KT_CHECK_EQ(chooseTarget(f, 2, 1, 100, true), 1);
    Candidate h[2] = { C(1, 100), C(2, 100) };
    KT_CHECK_EQ(chooseTarget(h, 2, 1, 0, false), -1);
    KT_CHECK_EQ(chooseTarget(h, 2, 1, 0, true), 1);
    KT_CHECK_EQ(chooseTarget(h, 2, 1, -5, false), -1);          // clamped to 0
    KT_CHECK_EQ(chooseTarget(m, 2, 0, 500, false), 1);          // clamped to 100
  }
  { // unreadable members are skipped
    Candidate m[3] = { C(100, 100), C(1, 100, false), C(0, 0) };
    KT_CHECK_EQ(chooseTarget(m, 3, 0, 50, false), -1);
    KT_CHECK_EQ(chooseTarget(m, 3, 0, 50, true), 0);
    Candidate n[3] = { C(100, 100), C(1, 100, false), C(40, 100) };
    KT_CHECK_EQ(chooseTarget(n, 3, 0, 50, false), 2);
    Candidate z[1] = { C(5, 100, false) };   // unreadable self: no self fallback
    KT_CHECK_EQ(chooseTarget(z, 1, 0, 50, true), -1);
  }
  { // self fallback needs a valid selfIdx; defensive inputs
    Candidate m[2] = { C(100, 100), C(100, 100) };
    KT_CHECK_EQ(chooseTarget(m, 2, 5, 50, true), -1);
    KT_CHECK_EQ(chooseTarget(m, 2, -1, 50, true), -1);
    KT_CHECK_EQ(chooseTarget(nullptr, 2, 0, 50, true), -1);
    KT_CHECK_EQ(chooseTarget(m, 0, 0, 50, true), -1);
    KT_CHECK_EQ(chooseTarget(m, -3, 0, 50, true), -1);
    Candidate h[2] = { C(100, 100), C(10, 100) };
    KT_CHECK_EQ(chooseTarget(h, 2, 9, 50, true), 1);            // bad selfIdx still picks a qualifier
  }
  { // slotEnabled
    General g; g.healThreshold = 50; g.healFallbackSelf = 0;
    Candidate hurt[2] = { C(100, 100), C(10, 100) }, full[2] = { C(100, 100), C(100, 100) };
    TargetSpec def;
    KT_CHECK(slotEnabled(full, 2, 0, def, g));                  // TM_Default always on
    KT_CHECK(slotEnabled(nullptr, 0, 0, def, g));
    TargetSpec ls; ls.mode = TM_LowHp; ls.fallback = 1;
    KT_CHECK(slotEnabled(full, 2, 0, ls, g));                   // fallback self
    TargetSpec ln; ln.mode = TM_LowHp; ln.fallback = 0; ln.thresholdPct = 50;
    KT_CHECK(slotEnabled(hurt, 2, 0, ln, g));
    KT_CHECK(!slotEnabled(full, 2, 0, ln, g));
    TargetSpec unset; unset.mode = TM_LowHp;                    // -> [General]
    KT_CHECK(slotEnabled(hurt, 2, 0, unset, g));
    KT_CHECK(!slotEnabled(full, 2, 0, unset, g));
    g.healFallbackSelf = 1;
    KT_CHECK(slotEnabled(full, 2, 0, unset, g));
    g.healFallbackSelf = 0; g.healThreshold = 5;                // General threshold applies: 10% not below 5
    KT_CHECK(!slotEnabled(hurt, 2, 0, unset, g));
    unset.thresholdPct = 20;                                    // per-slot override wins
    KT_CHECK(slotEnabled(hurt, 2, 0, unset, g));
  }
  { // vanilla "usable on another member" rule
    for (int t : { 45, 26, 25, 0 }) KT_CHECK(isTargetableItemType(t));
    for (int t : { 20, 47, -1, -100 }) KT_CHECK(!isTargetableItemType(t));
  }
  return kt::Report("hotbar_logic_test");
}
