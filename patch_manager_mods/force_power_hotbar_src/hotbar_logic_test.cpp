// Pure-logic tests for the hotbar model (U2: picking an action already on the bar MOVES it).
#include "HotbarConfig.h"
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
  return kt::Report("hotbar_logic_test");
}
