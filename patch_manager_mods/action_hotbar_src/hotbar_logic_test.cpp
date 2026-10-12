// Pure-logic tests for the hotbar model (U2: picking an action already on the bar MOVES it).
#include "HotbarConfig.h"
#include "HotbarLogic.h"
#include "TwoDA.h"
#include "check.h"
#include <climits>
#include <memory>
#include <cstring>
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
  // ======== units extracted from ActionHotbar.cpp (behaviour-neutral; doc 20 sec 4.1) ========
  { // 1. decodeKey / tabOfKey
    ActionRef i = decodeKey(0x40000007, "Medpac (3)");
    KT_CHECK(i.kind == K_Item); KT_CHECK_EQ(i.name.c_str(), "Medpac"); KT_CHECK_EQ(i.id, 0);
    ActionRef e = decodeKey(0x40000007, "");                              // item with no usable name -> K_None
    KT_CHECK(e.kind == K_None);
    ActionRef sp = decodeKey(0x2000002A, "ignored");
    KT_CHECK(sp.kind == K_Spell); KT_CHECK_EQ(sp.id, 0x2A); KT_CHECK(sp.name.empty());
    ActionRef both = decodeKey((int)0x60000005, "Both");                 // item bit wins over the spell bit
    KT_CHECK(both.kind == K_Item);
    ActionRef o = decodeKey(0x1234, "Mode");
    KT_CHECK(o.kind == K_Other); KT_CHECK_EQ(o.id, 0x1234);
    ActionRef neg = decodeKey(-1, "x");                                  // all bits set: item
    KT_CHECK(neg.kind == K_Item);
    KT_CHECK_EQ(tabOfKey(0x40000001), 1); KT_CHECK_EQ(tabOfKey(0x20000001), 0); KT_CHECK_EQ(tabOfKey(5), 2);
    KT_CHECK_EQ(tabOfKey(0x60000000), 1);
  }
  { // 2. characterKeyFor: obj%08x / invalid / party==1 / tags / slots
    bool bad = true;
    KT_CHECK_EQ(characterKeyFor(0x7F000001u, 3, &bad).c_str(), "obj7f000001"); KT_CHECK(!bad);
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFDu, 1, &bad).c_str(), "objfffffffd"); KT_CHECK(!bad);
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFFu, 1, &bad).c_str(), "objfffffffd"); KT_CHECK(bad);   // one-member party = the Exile
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFFu, 2, &bad, true).c_str(), "objfffffffd"); KT_CHECK(bad);   // member 0 in multi-member party = the Exile
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFFu, 2, &bad, false).c_str(), ""); KT_CHECK(bad);
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFFu, 2, &bad).c_str(), ""); KT_CHECK(bad);
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFFu, 0, &bad).c_str(), ""); KT_CHECK(bad);
    KT_CHECK_EQ(characterKeyFor(0u, 1, &bad).c_str(), ""); KT_CHECK(bad);                       // id 0 never gets the party==1 rescue
    KT_CHECK_EQ(characterKeyFor(0u, 3, &bad).c_str(), ""); KT_CHECK(bad);
    KT_CHECK_EQ(characterKeyFor(1u, 3).c_str(), "obj00000001");                                // bad == nullptr allowed
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFEu, 1).c_str(), "objfffffffd");                        // AH2: stabilized to Exile canonical key
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFEu, 2, &bad, true).c_str(), "objfffffffd"); KT_CHECK(!bad);
    KT_CHECK_EQ(characterKeyFor(0x80000042u, 3, &bad, false, "atton", 1).c_str(), "atton"); KT_CHECK(!bad);
    KT_CHECK_EQ(characterKeyFor(0x80000043u, 3, &bad, false, "  Kreia  ", 2).c_str(), "kreia"); KT_CHECK(!bad);
    KT_CHECK_EQ(characterKeyFor(0x80000042u, 3, &bad, false, "", 1).c_str(), "slot1"); KT_CHECK(!bad);
    KT_CHECK_EQ(characterKeyFor(0x80000043u, 3, &bad, false, nullptr, 2).c_str(), "slot2"); KT_CHECK(!bad);
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFDu, 3, &bad, true, "player", 0).c_str(), "objfffffffd"); KT_CHECK(!bad);
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFEu, 3, &bad, true, "", 0).c_str(), "objfffffffd"); KT_CHECK(!bad);
    // AH3: full Odyssey negative player sentinel range 0xFFFFFFF0..0xFFFFFFFE maps to canonical Exile key
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFF0u, 1, &bad).c_str(), "objfffffffd"); KT_CHECK(!bad);
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFF0u, 2, &bad, true).c_str(), "objfffffffd"); KT_CHECK(!bad);
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFF9u, 2, &bad, false).c_str(), "objfffffffd"); KT_CHECK(!bad);   // fast-travel sentinel
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFAu, 1, &bad).c_str(), "objfffffffd"); KT_CHECK(!bad);
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFEu, 1, &bad).c_str(), "objfffffffd"); KT_CHECK(!bad);
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFFu, 2, &bad, false).c_str(), ""); KT_CHECK(bad);   // 0xFFFFFFFF stays invalid (outside sentinel range)
  }
  { // 2b. pickPartySlot (BG04): never match by 0 / 0xFFFFFFFF alone
    const uintptr_t A = 0x1000, B = 0x2000, C = 0x3000, Z = 0x9000;
    { uintptr_t p[3] = {A, B, C}; unsigned o[3] = {7, 7, 9};
      KT_CHECK_EQ((int)pickPartySlot(B, 7, p, o, 3), 1); }                                   // pointer beats earlier OID hit
    { uintptr_t p[2] = {A, B}; unsigned o[2] = {0xFFFFFFFFu, 0xFFFFFFFFu};
      KT_CHECK_EQ((int)pickPartySlot(Z, 0xFFFFFFFFu, p, o, 2), -1);                          // shared sentinel, unknown pointer: ambiguous
      KT_CHECK_EQ((int)pickPartySlot(B, 0xFFFFFFFFu, p, o, 2), 1); }                         // pointer match still wins
    { uintptr_t p[2] = {A, B}; unsigned o[2] = {0xFFFFFFFFu, 0x80000042u};
      KT_CHECK_EQ((int)pickPartySlot(Z, 0xFFFFFFFFu, p, o, 2), 0); }                         // Ebon Hawk rescue: one holder
    { uintptr_t p[1] = {A}; unsigned o[1] = {0};
      KT_CHECK_EQ((int)pickPartySlot(Z, 0, p, o, 1), -1); }                                  // id 0 never matches by OID
    { uintptr_t p[2] = {A, B}; unsigned o[2] = {0xFFFFFFFDu, 0x80000042u};
      KT_CHECK_EQ((int)pickPartySlot(Z, 0x80000042u, p, o, 2), 1); }                         // valid id, wrapped pointer
    { uintptr_t p[2] = {0, B}; unsigned o[2] = {0xFFFFFFFFu, 5};
      KT_CHECK_EQ((int)pickPartySlot(Z, 0xFFFFFFFFu, p, o, 2), -1); }                        // empty slot skipped
    { uintptr_t p[3] = {A, B, C}; unsigned o[3] = {1, 2, 3};
      KT_CHECK_EQ((int)pickPartySlot(Z, 3, p, o, 3), 2); }
    // end to end with characterKeyFor
    { bool bad = false; int pick = -1;
      KT_CHECK_EQ(characterKeyFor(0xFFFFFFFFu, 2, &bad, pick == 0, nullptr, pick).c_str(), ""); KT_CHECK(bad);
      pick = 0;
      KT_CHECK_EQ(characterKeyFor(0xFFFFFFFFu, 2, &bad, pick == 0, nullptr, pick).c_str(), "objfffffffd"); }
  }
  { // 3. AutoFill merge + FNV signature
    ActionRef cfg[Slots], eff[Slots]; bool isAuto[Slots];
    std::vector<ActionRef> autos = { mk(K_Spell, 1), mk(K_Spell, 2), mk(K_Item, 0, "Medpac"), mk(K_Spell, 4) };
    cfg[1] = mk(K_Spell, 9);                    // explicit
    cfg[3].blank = true;                        // explicit "none": stays empty, takes no auto
    cfg[4] = mk(K_Spell, 9);
    int n = assignAutoFill(cfg, autos, eff, isAuto);
    KT_CHECK_EQ(n, 4 + 0);                      // empty slots 0,2,5,6,... fill in order: 4 autos -> slots 0,2,5,6
    KT_CHECK(eff[0].kind == K_Spell && eff[0].id == 1 && isAuto[0]);
    KT_CHECK(eff[1].id == 9 && !isAuto[1]);
    KT_CHECK(eff[2].id == 2 && isAuto[2]);
    KT_CHECK(eff[3].empty() && eff[3].blank && !isAuto[3]);
    KT_CHECK(eff[4].id == 9 && !isAuto[4]);
    KT_CHECK(eff[5].kind == K_Item && isAuto[5]);
    KT_CHECK(eff[6].kind == K_Spell && eff[6].id == 4 && isAuto[6]);
    for (int s = 7; s < Slots; ++s) KT_CHECK(eff[s].empty() && !isAuto[s]);
    KT_CHECK_EQ(assignAutoFill(cfg, std::vector<ActionRef>(), eff, isAuto), 0);   // no autos: eff == cfg
    KT_CHECK(eff[0].empty() && eff[1].id == 9);
    // signature: golden values from the original DLL loop; changes with id / auto flag / name
    ActionRef z[Slots]; bool za[Slots] = {};
    unsigned h0 = effSignature(z, za);
    KT_CHECK_EQ(h0, effSignature(z, za));                                          // deterministic
    KT_CHECK_EQ(h0, 1078699117u);                                                  // HB21: golden FNV-1a values (no reimplementation of the hash here);
    ActionRef a[Slots]; bool aa[Slots] = {}; a[2] = mk(K_Spell, 8); a[5] = mk(K_Item, 0, "Medpac");
    unsigned h1 = effSignature(a, aa);                                             // a change in the hash scheme must be a deliberate edit of these constants
    KT_CHECK_EQ(h1, 1090647492u);
    KT_CHECK(h1 != h0);
    bool ab[Slots] = {}; ab[2] = true;
    KT_CHECK_EQ(effSignature(a, ab), 2617374148u);                                 // auto flag changes it
    a[2].id = 9; KT_CHECK(effSignature(a, aa) != h1);
    // helpers: known-power key filter, dedup, selection pick
    KT_CHECK(isKnownPowerKey(0x20000005)); KT_CHECK(!isKnownPowerKey(0x40000005)); KT_CHECK(!isKnownPowerKey(0x60000005)); KT_CHECK(!isKnownPowerKey(0x5));
    std::vector<ActionRef> v = { mk(K_Spell, 3), mk(K_Item, 0, "medpac") };
    KT_CHECK(hasSame(v, mk(K_Spell, 3))); KT_CHECK(hasSame(v, mk(K_Item, 0, "MEDPAC"))); KT_CHECK(!hasSame(v, mk(K_Feat, 3)));
    int keys[3] = { 0x40000001, 0x40000002, 0x40000003 };
    KT_CHECK_EQ(pickIndexByKey(keys, 3, 0x40000002u), 1);
    KT_CHECK_EQ(pickIndexByKey(keys, 3, 0xFFFFFFFFu), 0);                          // no match: first
    KT_CHECK_EQ(pickIndexByKey(keys, 0, 1u), -1);
  }
  { // 4a. wrapText
    KT_CHECK_EQ(wrapText("alpha beta gamma", 10, 3).c_str(), "alpha beta\ngamma");
    KT_CHECK_EQ(wrapText("supercalifragilistic ok", 5, 3).c_str(), "supercalifragilistic\nok");   // long word is never split
    KT_CHECK_EQ(wrapText("supercalifragilistic", 5, 3).c_str(), "supercalifragilistic");
    KT_CHECK_EQ(wrapText("aa bb cc", 4, 1).c_str(), "aa...");                      // maxLines <= 1: cut at the first wrap
    KT_CHECK_EQ(wrapText("aa bb cc", 4, 0).c_str(), "aa...");
    KT_CHECK_EQ(wrapText("aa bb cc", 4, -3).c_str(), "aa...");
    KT_CHECK_EQ(wrapText("aa bb", 20, 1).c_str(), "aa bb");                         // fits: no cut even with maxLines 1
    KT_CHECK_EQ(wrapText("", 10, 3).c_str(), "");
    KT_CHECK_EQ(wrapText("a  b", 10, 3).c_str(), "a  b");                          // double space survives inside a line
    KT_CHECK_EQ(wrapText(" a", 10, 3).c_str(), "a");                               // leading space dropped
    KT_CHECK_EQ(wrapText("abc ", 10, 3).c_str(), "abc");                           // trailing space dropped
    KT_CHECK_EQ(wrapText("aa bb cc dd ee", 2, 2).c_str(), "aa\nbb...");            // overflow: "..." on the last allowed line
    KT_CHECK_EQ(wrapText("abc de", 6, 2).c_str(), "abc de");                       // exact fit
    KT_CHECK_EQ(wrapText("abc def", 6, 2).c_str(), "abc\ndef");
    KT_CHECK_EQ(wrapText("a b c d e f", 3, 3).c_str(), "a b\nc d\ne f");
    KT_CHECK_EQ(wrapText("a b c d e f g", 3, 3).c_str(), "a b\nc d\ne f...");
  }
  { // 4b. shortDesc
    KT_CHECK_EQ(shortDesc("Universal Power Restricted By Armor This power heals. More text.").c_str(), "This power heals.");
    KT_CHECK_EQ(shortDesc("Light Side Power Prerequisites: Character Level 6 This power does X. Y.").c_str(), "This power does X.");
    KT_CHECK_EQ(shortDesc("One. Two").c_str(), "One.");                            // no header
    KT_CHECK_EQ(shortDesc("No full stop").c_str(), "No full stop");
    KT_CHECK_EQ(shortDesc("").c_str(), "");
    KT_CHECK_EQ(shortDesc(" This x").c_str(), "This x");                           // marker at 0
    std::string late = std::string(130, 'a') + " This power. Rest";                // marker at 130 (>= 120): header kept
    KT_CHECK_EQ(shortDesc(late).c_str(), (std::string(130, 'a') + " This power.").c_str());
    std::string edge = std::string(119, 'a') + " This b. c";                       // marker at 119 (< 120): trimmed
    KT_CHECK_EQ(shortDesc(edge).c_str(), "This b.");
    KT_CHECK_EQ(shortDesc("Ends with dot.").c_str(), "Ends with dot.");            // ". " needs a following space
  }
  { // 5. tooltip size + screen clamp
    TipRect t = tipLayout("ab\ncde", 100, 500, -1, 3840);
    KT_CHECK_EQ(t.lines, 2); KT_CHECK_EQ(t.maxLen, 3); KT_CHECK_EQ(t.w, 3 * 21 + 44); KT_CHECK_EQ(t.h, 2 * 32 + 44);
    KT_CHECK_EQ(t.x, 100); KT_CHECK_EQ(t.y, 500 - 108 - 12);
    TipRect a = tipLayout("ab\ncde", 100, 500, 300, 3840);                          // picker tooltips sit above the picker
    KT_CHECK_EQ(a.y, 300 - 108 - 10); KT_CHECK_EQ(a.x, 100);
    TipRect r = tipLayout("ab\ncde", 100, 500, -1, 150);                            // right edge: x + w <= right - 8
    KT_CHECK_EQ(r.x, 150 - 8 - 107);
    TipRect l = tipLayout("ab\ncde", -50, 500, -1, 3840); KT_CHECK_EQ(l.x, 8);      // left clamp
    TipRect tp = tipLayout("ab\ncde", 100, 50, -1, 3840); KT_CHECK_EQ(tp.y, 8);     // top clamp
    TipRect w = tipLayout("ab\ncde", 100, 500, -1, 100); KT_CHECK_EQ(w.x, 8);       // wider than the screen: left clamp wins
    TipRect e = tipLayout("", 10, 500, -1, 3840); KT_CHECK_EQ(e.lines, 1); KT_CHECK_EQ(e.maxLen, 0); KT_CHECK_EQ(e.w, 44); KT_CHECK_EQ(e.h, 76);
    TipRect nl = tipLayout("a\n", 10, 500, -1, 3840); KT_CHECK_EQ(nl.lines, 2); KT_CHECK_EQ(nl.maxLen, 1);   // trailing newline = extra line
    TipRect lg = tipLayout("abcd\nx\nabcdefg", 10, 500, -1, 3840); KT_CHECK_EQ(lg.lines, 3); KT_CHECK_EQ(lg.maxLen, 7);
  }
  { // 6. settling predicate
    KT_CHECK(!isSettling(0, 5000, 1500));                                          // no character change seen yet
    KT_CHECK(isSettling(1000, 2000, 1500));
    KT_CHECK(isSettling(1000, 2499, 1500));
    KT_CHECK(!isSettling(1000, 2500, 1500));                                       // strictly less than
    KT_CHECK(isSettling(1000, 1000, 1500));
    KT_CHECK(isSettling(0xFFFFFF00u, 0x100u, 1500));                               // GetTickCount wrap: 512 ms elapsed
    KT_CHECK(!isSettling(0xFFFFFF00u, 0x1000u, 1500));
  }
  { // 7. per-slot display state
    const float E = EmptyFrameAlpha, D = DimAlpha;
    SlotView u = slotView(true, false, false, 0, 0, "", false);                    // unbound slot (edit mode)
    KT_CHECK_EQ(u.state, "unbound"); KT_CHECK(!u.hideSettling && !u.outOfStock && !u.showQty);
    KT_CHECK_EQ(u.qtyHideWhy, "unbound"); KT_CHECK(u.bgFactor == E && u.icFactor == 1.0f);
    SlotView p = slotView(false, false, true, 1, 0x20000005, "Heal", false);       // usable power
    KT_CHECK_EQ(p.state, "usable"); KT_CHECK(!p.showQty && !p.outOfStock);
    KT_CHECK_EQ(p.qtyHideWhy, "single item / not an item"); KT_CHECK(p.bgFactor == 1.0f && p.icFactor == 1.0f);
    SlotView d = slotView(false, false, true, 0, 0x20000005, "Heal", false);       // unusable: icon dimmed
    KT_CHECK_EQ(d.state, "unusable (dimmed)"); KT_CHECK(d.icFactor == D && d.bgFactor == 1.0f);
    SlotView m = slotView(false, true, true, 1, 0x40000009, "Medpac (3)", false);  // stack of 3
    KT_CHECK(m.showQty && !m.grayQty && m.qty == 3); KT_CHECK_EQ(m.state, "usable");
    SlotView s1 = slotView(false, true, true, 1, 0x40000009, "Medpac", false);     // single item: no count
    KT_CHECK(!s1.showQty && s1.qty == 0);
    SlotView s2 = slotView(false, true, true, 1, 0x40000009, "Medpac (1)", false); // "(1)" is not > 1
    KT_CHECK(!s2.showQty);
    SlotView o = slotView(false, true, false, 0, 0, "", false);                    // out of stock: gray "0", dim icon
    KT_CHECK_EQ(o.state, "out of stock"); KT_CHECK(o.outOfStock && o.showQty && o.grayQty && o.qty == 0);
    KT_CHECK(o.icFactor == D && o.bgFactor == E); KT_CHECK(!o.hideSettling);
    SlotView n = slotView(false, false, false, 0, 0, "", false);                   // power not available
    KT_CHECK_EQ(n.state, "entry not available"); KT_CHECK(!n.outOfStock && !n.showQty && n.icFactor == 1.0f && n.bgFactor == E);
    KT_CHECK_EQ(n.qtyHideWhy, "entry not available");
    KT_CHECK(slotView(false, false, false, 0, 0, "", true).hideSettling);          // not resolved while settling in Normal mode: hide
    KT_CHECK(slotView(false, true, false, 0, 0, "", true).hideSettling);
    KT_CHECK(!slotView(false, true, true, 1, 0x40000001, "X", true).hideSettling); // resolved: never hidden
  }
  { // 8. picker maxOff + clamp + wheel steps
    KT_CHECK_EQ(pickerMaxOff(0, 8), 0); KT_CHECK_EQ(pickerMaxOff(5, 8), 0); KT_CHECK_EQ(pickerMaxOff(8, 8), 0);
    KT_CHECK_EQ(pickerMaxOff(9, 8), 1); KT_CHECK_EQ(pickerMaxOff(20, 8), 12);
    KT_CHECK_EQ(clampOffset(5, 3), 3); KT_CHECK_EQ(clampOffset(2, 3), 2); KT_CHECK_EQ(clampOffset(3, 3), 3); KT_CHECK_EQ(clampOffset(4, 0), 0);
    KT_CHECK_EQ(wheelSteps(120), 1); KT_CHECK_EQ(wheelSteps(240), 2); KT_CHECK_EQ(wheelSteps(-120), -1); KT_CHECK_EQ(wheelSteps(-360), -3);
    KT_CHECK_EQ(wheelSteps(60), 1); KT_CHECK_EQ(wheelSteps(-60), -1); KT_CHECK_EQ(wheelSteps(1), 1); KT_CHECK_EQ(wheelSteps(-1), -1);
    KT_CHECK_EQ(wheelSteps(119), 1); KT_CHECK_EQ(wheelSteps(239), 1); KT_CHECK_EQ(wheelSteps(0), 0);
    KT_CHECK_EQ(scrollClamp(-4, 12), 0); KT_CHECK_EQ(scrollClamp(5, 12), 5); KT_CHECK_EQ(scrollClamp(99, 12), 12); KT_CHECK_EQ(scrollClamp(1, 0), 0);
    KT_CHECK_EQ(scrollClamp(5 - wheelSteps(120), 12), 4);                          // wheel up = back toward the start
    KT_CHECK_EQ(scrollClamp(0 - wheelSteps(120), 12), 0);
    KT_CHECK_EQ(scrollClamp(12 - wheelSteps(-120), 12), 12);
  }
  { // 9. arrow repeat timing: 350 ms first, then every 90 ms
    KT_CHECK(!arrowRepeatDue(1349, 1000, 1000)); KT_CHECK(arrowRepeatDue(1350, 1000, 1000));
    KT_CHECK(!arrowRepeatDue(1400, 1000, 1350)); KT_CHECK(!arrowRepeatDue(1439, 1000, 1350)); KT_CHECK(arrowRepeatDue(1440, 1000, 1350));
    KT_CHECK(!arrowRepeatDue(5000, 4800, 4990));                                   // repeat fine, first delay not reached (200 ms)
    KT_CHECK(arrowRepeatDue(0x100u, 0xFFFFFF00u, 0xFFFFFF00u) == true);            // wrap: 512 ms since start
    KT_CHECK(!arrowRepeatDue(0x20u, 0xFFFFFF00u, 0xFFFFFF00u));                    // wrap: 288 ms
  }
  { // 10. aspect correction + layout units -> px (the H3 baseline: 3840x1600, ref 800x600)
    double c = aspectCorrection(3840, 1600, 800, 600);
    KT_CHECK(c > 0.555555 && c < 0.555556);
    Scale k = makeScale(3840, 1600, 800, 600);
    KT_CHECK(k.sx == 4.8 && k.sy == 1600.0 / 600 && k.c == c);
    struct R { const char* name; int x, y, w, h; int e[4]; };
    const R rs[] = {   // layout units from Layout[] -> rect=[..] logged by build 26 (baseline_h3_build26.txt)
        { "LBL_HOTBAREDIT",   4, 478, 20, 35,    { 11, 1274, 53, 93 } },
        { "LBL_HOTBARBG1",   29, 478, 35, 35,    { 77, 1274, 93, 93 } },
        { "LBL_HOTBARBG0",  389, 478, 35, 35,    { 1037, 1274, 93, 93 } },
        { "LBL_HOTBAR1",     31, 480, 31, 31,    { 82, 1280, 82, 82 } },
        { "LBL_HOTBARPBG",    4, 360, 420, 112,  { 11, 960, 1120, 298 } },
        { "LBL_HOTBARQTY1",  33, 495, 27, 17,    { 88, 1320, 72, 45 } },
        { "LBL_HOTBARCELLBG0", 56, 426, 35, 35,  { 149, 1136, 93, 93 } },
        { "LBL_HOTBARCELLQTY7", 340, 443, 27, 17, { 907, 1181, 72, 45 } },
        { "LBL_HOTBARTIPBG",  12, 380, 300, 90,  { 32, 1013, 800, 240 } },
    };
    for (const R& q : rs) { int r[4]; unitRect(q.x, q.y, q.w, q.h, k, r); char b[96]; std::snprintf(b, sizeof b, "%s", q.name);
        KT_CHECK(r[0] == q.e[0] && r[1] == q.e[1] && r[2] == q.e[2] && r[3] == q.e[3]); if (r[0] != q.e[0] || r[1] != q.e[1] || r[2] != q.e[2] || r[3] != q.e[3]) std::printf("  ^ %s got [%d,%d,%d,%d]\n", b, r[0], r[1], r[2], r[3]); }
    Scale i = makeScale(800, 600, 800, 600); KT_CHECK(i.sx == 1.0 && i.sy == 1.0 && i.c == 1.0);   // 4:3: identity
    int r4[4]; unitRect(4, 478, 20, 35, i, r4); KT_CHECK(r4[0] == 4 && r4[1] == 478 && r4[2] == 20 && r4[3] == 35);
    Scale i2 = makeScale(1600, 1200, 800, 600); KT_CHECK(i2.sx == 1.0 && i2.c == 1.0);                // 4:3 at 2x: still the identity (c == 1)
    KT_CHECK_EQ(aspectCorrection(0, 1200, 800, 600) == 1.0, 1); KT_CHECK_EQ(aspectCorrection(1600, 0, 800, 600) == 1.0, 1);
    KT_CHECK_EQ(aspectCorrection(1600, 1200, 0, 600) == 1.0, 1); KT_CHECK_EQ(aspectCorrection(1600, 1200, 800, 0) == 1.0, 1);
    Scale iv = makeScale(-5, 1200, 800, 600); KT_CHECK(iv.sx == 1.0 && iv.c == 1.0);
    Scale w = makeScale(2560, 1080, 800, 600); KT_CHECK(w.c < 1.0 && w.sx == 3.2 && w.sy == 1.8);     // 21:9
  }
  { // 12. DIK -> slot decision table (decision strings are logged: they must not change)
    auto K = [](unsigned dik, bool down) { KeyEvent k; k.dik = dik; k.down = down; return k; };
    KeyEvent k;
    // digit down, plain, everything fine, bound -> swallow + enqueue slot (dik - 2)
    k = K(0x05, true); k.hudActive = true; k.slotBound = true;
    KeyAction a = decideKey(k);
    KT_CHECK_EQ(a.decision, "SWALLOW"); KT_CHECK(a.swallow && a.markDown && a.enqueue && !a.clearDown && !a.escHit); KT_CHECK_EQ(a.slot, 3);
    k.ringFull = true; a = decideKey(k);
    KT_CHECK_EQ(a.decision, "SWALLOW(ring full, request dropped)"); KT_CHECK(a.swallow && a.markDown && !a.enqueue);
    k.ringFull = false;
    k.shift = true; a = decideKey(k); KT_CHECK_EQ(a.decision, "pass(shift)"); KT_CHECK(!a.swallow && !a.enqueue && !a.markDown); k.shift = false;
    k.ctrl = true; a = decideKey(k); KT_CHECK_EQ(a.decision, "pass(ctrl/alt)"); KT_CHECK(!a.swallow); k.ctrl = false;
    k.alt = true; a = decideKey(k); KT_CHECK_EQ(a.decision, "pass(ctrl/alt)"); k.alt = false;
    k.ctrl = k.alt = true; k.hudActive = false; a = decideKey(k); KT_CHECK_EQ(a.decision, "pass(ctrl/alt)"); k.ctrl = k.alt = false;   // ctrl/alt tested before inactive
    k.hudActive = false; a = decideKey(k); KT_CHECK_EQ(a.decision, "pass(hud inactive)"); KT_CHECK(!a.swallow); k.hudActive = true;
    k.editOpen = true; a = decideKey(k); KT_CHECK_EQ(a.decision, "pass(edit mode: never invoke)"); KT_CHECK(!a.swallow); k.editOpen = false;
    k.slotBound = false; a = decideKey(k); KT_CHECK_EQ(a.decision, "pass(unbound)"); KT_CHECK(!a.swallow && !a.enqueue); k.slotBound = true;
    k.hudActive = false; k.editOpen = true; k.slotBound = false; a = decideKey(k); KT_CHECK_EQ(a.decision, "pass(hud inactive)");     // order: inactive before edit before unbound
    // every digit -> its own slot
    for (unsigned dik = 0x02; dik <= 0x0B; ++dik) { KeyEvent q = K(dik, true); q.hudActive = true; q.slotBound = true; KeyAction r = decideKey(q); KT_CHECK(r.enqueue && r.slot == (int)dik - 2); }
    // digit up: swallow only if the down was swallowed
    k = K(0x03, false); k.downSwallowed = true; a = decideKey(k);
    KT_CHECK_EQ(a.decision, "swallow-up"); KT_CHECK(a.swallow && a.clearDown && !a.markDown && !a.enqueue);
    k.downSwallowed = false; a = decideKey(k); KT_CHECK_EQ(a.decision, "pass"); KT_CHECK(!a.swallow && !a.clearDown);
    // Esc
    k = K(0x01, true); k.editOpen = true; k.hudActive = true; a = decideKey(k);
    KT_CHECK_EQ(a.decision, "SWALLOW (edit Esc)"); KT_CHECK(a.swallow && a.markDown && a.escHit && !a.enqueue);
    k.editOpen = false; a = decideKey(k); KT_CHECK_EQ(a.decision, "pass(edit closed)"); KT_CHECK(!a.swallow && !a.escHit);
    k.editOpen = true; k.hudActive = false; a = decideKey(k); KT_CHECK_EQ(a.decision, "pass(edit closed)");
    k = K(0x01, false); k.downSwallowed = true; a = decideKey(k);
    KT_CHECK_EQ(a.decision, "swallow-up (edit)"); KT_CHECK(a.swallow && a.clearDown && !a.escHit);
    k.downSwallowed = false; a = decideKey(k); KT_CHECK_EQ(a.decision, "pass");
    // other keys: pass, never touched
    for (unsigned dik : { 0x00u, 0x0Cu, 0x1Du, 0x2Au, 0x38u, 0x9Du, 0xFFu }) { KeyEvent q = K(dik, true); q.hudActive = true; q.slotBound = true; q.downSwallowed = true; KeyAction r = decideKey(q); KT_CHECK_EQ(r.decision, "pass"); KT_CHECK(!r.swallow && !r.enqueue && !r.markDown && !r.clearDown && !r.escHit); }
    KT_CHECK(isHotbarDigit(0x02) && isHotbarDigit(0x0B) && !isHotbarDigit(0x01) && !isHotbarDigit(0x0C));
  }
  { // 13. TGA -> RGBA
    auto hdr = [](std::vector<unsigned char>& d, int idlen, int w, int h, int bpp = 32, int type = 2, int cmap = 0) {
      d.assign(18 + idlen, 0); d[0] = (unsigned char)idlen; d[1] = (unsigned char)cmap; d[2] = (unsigned char)type;
      d[12] = (unsigned char)(w & 255); d[13] = (unsigned char)(w >> 8); d[14] = (unsigned char)(h & 255); d[15] = (unsigned char)(h >> 8); d[16] = (unsigned char)bpp; };
    { // valid 2x1 with a 3-byte id field and a footer
      std::vector<unsigned char> d; hdr(d, 3, 2, 1);
      const unsigned char px[8] = { 1, 2, 3, 4, 5, 6, 7, 8 }; d.insert(d.end(), px, px + 8); d.insert(d.end(), 26, 0xEE);
      TgaInfo t; KT_CHECK_EQ(tgaInspect(d.data(), d.size(), &t), TGA_OK);
      KT_CHECK_EQ(t.w, 2); KT_CHECK_EQ(t.h, 1); KT_CHECK_EQ((long long)t.off, 21); KT_CHECK_EQ((long long)t.bytes, 8);
      unsigned char out[8] = {}; tgaBgraToRgba(d.data(), t, out);
      const unsigned char want[8] = { 3, 2, 1, 4, 7, 6, 5, 8 };                    // BGRA -> RGBA
      KT_CHECK(memcmp(out, want, 8) == 0);
      std::vector<unsigned char> s(d.begin(), d.begin() + 21 + 7);                 // one byte short
      KT_CHECK_EQ(tgaInspect(s.data(), s.size(), &t), TGA_TRUNCATED); KT_CHECK_EQ(t.w, 2); KT_CHECK_EQ(t.h, 1);
      std::vector<unsigned char> e(d.begin(), d.begin() + 21 + 8);                 // exact, no footer
      KT_CHECK_EQ(tgaInspect(e.data(), e.size(), &t), TGA_OK);
    }
    { // format rejections
      std::vector<unsigned char> d; TgaInfo t; hdr(d, 0, 1, 1); d.insert(d.end(), 4, 0);
      KT_CHECK_EQ(tgaInspect(d.data(), d.size(), &t), TGA_OK);
      KT_CHECK_EQ(tgaInspect(d.data(), 17, &t), TGA_NOT_UNCOMPRESSED32);           // header cut
      KT_CHECK_EQ(tgaInspect(d.data(), 0, &t), TGA_NOT_UNCOMPRESSED32);
      hdr(d, 0, 1, 1, 24); d.insert(d.end(), 4, 0); KT_CHECK_EQ(tgaInspect(d.data(), d.size(), &t), TGA_NOT_UNCOMPRESSED32);
      hdr(d, 0, 1, 1, 32, 10); d.insert(d.end(), 4, 0); KT_CHECK_EQ(tgaInspect(d.data(), d.size(), &t), TGA_NOT_UNCOMPRESSED32);   // RLE
      hdr(d, 0, 1, 1, 32, 2, 1); d.insert(d.end(), 4, 0); KT_CHECK_EQ(tgaInspect(d.data(), d.size(), &t), TGA_NOT_UNCOMPRESSED32); // colour map
      hdr(d, 0, 0, 1); d.insert(d.end(), 4, 0); KT_CHECK_EQ(tgaInspect(d.data(), d.size(), &t), TGA_TRUNCATED);                    // w == 0
      hdr(d, 0, 1, 0); d.insert(d.end(), 4, 0); KT_CHECK_EQ(tgaInspect(d.data(), d.size(), &t), TGA_TRUNCATED);                    // h == 0
      hdr(d, 255, 1, 1); d.insert(d.end(), 3, 0); KT_CHECK_EQ(tgaInspect(d.data(), d.size(), &t), TGA_TRUNCATED);                  // id field eats the pixel
    }
    { // crafted headers: 32-bit w*h*4 overflow must never pass the bounds check (RED before the fix; ASan faults on the read past the array)
      struct C { int w, h; const char* why; };
      const C cs[] = { { 32767, 32769, "w*h*4 == 2^32-4: off+n wraps to 14 and passed the old check" },
                       { 32768, 32768, "w*h*4 == 0 (mod 2^32)" },
                       { 65535, 65535, "w*h*4 wraps to a huge value" },
                       { 65535, 1, "wide" }, { 1, 65535, "tall" }, { 4097, 1, "one past the dimension limit" } };
      for (const C& c : cs) {
        std::vector<unsigned char> d; hdr(d, 0, c.w, c.h); d.insert(d.end(), 64, 0x5A);
        TgaInfo t; TgaStatus s = tgaInspect(d.data(), d.size(), &t);
        KT_CHECK(s != TGA_OK); if (s == TGA_OK) std::printf("  ^ accepted %dx%d (%s)\n", c.w, c.h, c.why);
        if (s == TGA_OK) { std::unique_ptr<unsigned char[]> dst(new unsigned char[(size_t)t.bytes]); tgaBgraToRgba(d.data(), t, dst.get()); }   // what the DLL does next
        else KT_CHECK_EQ(s, TGA_TOO_LARGE);
      }
    }
    { // limits: exactly MaxTgaDim x MaxTgaDim is fine when the data is there; one more is not
      std::vector<unsigned char> d; hdr(d, 0, MaxTgaDim, MaxTgaDim); d.resize(d.size() + (size_t)MaxTgaDim * MaxTgaDim * 4);
      TgaInfo t; KT_CHECK_EQ(tgaInspect(d.data(), d.size(), &t), TGA_OK); KT_CHECK_EQ((long long)t.bytes, (long long)MaxTgaDim * MaxTgaDim * 4);
      hdr(d, 0, MaxTgaDim + 1, 1); d.resize(d.size() + 16); KT_CHECK_EQ(tgaInspect(d.data(), d.size(), &t), TGA_TOO_LARGE);
    }
  }
  // ===== mutation-triage additions (doc 20e): boundaries and defaults the pure units rely on =====
  { // Candidate defaults: unreadable, not down, eligible (the DLL only fills the readable ones)
    Candidate c; KT_CHECK_EQ(c.cur, 0); KT_CHECK_EQ(c.max, 0); KT_CHECK(!c.ok); KT_CHECK(!c.down); KT_CHECK(c.eligible);
  }
  { // chooseTarget: threshold clamping, max <= 0 rows, 1% resolution, selfIdx bound
    Candidate m[4]; for (Candidate& c : m) c.ok = true;
    m[0].cur = 0; m[0].max = 100;
    KT_CHECK_EQ(chooseTarget(m, 1, 0, -5, false), -1);        // negative threshold clamps to 0: nobody is below 0%
    KT_CHECK_EQ(chooseTarget(m, 1, 0, 1, false), 0);          // 0 of 100 is below 1%
    m[0].cur = 100; m[0].max = 100;
    KT_CHECK_EQ(chooseTarget(m, 1, 0, 100, false), -1);       // full health is never below the threshold
    KT_CHECK_EQ(chooseTarget(m, 1, 0, 101, false), -1);       // thresholds above 100 clamp to 100 (not 101%)
    KT_CHECK_EQ(chooseTarget(m, 1, 0, 500, false), -1);
    m[0].cur = -5; m[0].max = 0;
    KT_CHECK_EQ(chooseTarget(m, 1, 0, 50, false), -1);        // max <= 0 is skipped even with a negative cur
    m[0].cur = 0; m[0].max = 1;
    KT_CHECK_EQ(chooseTarget(m, 1, 0, 50, false), 0);         // max == 1 is a valid row
    m[0].cur = 499; m[0].max = 1000;
    KT_CHECK_EQ(chooseTarget(m, 1, 0, 50, false), 0);         // 49.9% is strictly below 50% (no rounding slack)
    m[0].cur = 500; m[0].max = 1000;
    KT_CHECK_EQ(chooseTarget(m, 1, 0, 50, false), -1);        // exactly 50% is not
    for (int i = 0; i < 3; ++i) { m[i].cur = 100; m[i].max = 100; }
    m[3].cur = 0; m[3].max = 100;
    KT_CHECK_EQ(chooseTarget(m, 3, 3, 50, true), -1);         // selfIdx == n is out of range: never read m[n]
    KT_CHECK_EQ(chooseTarget(m, 3, 2, 50, true), 2);          // selfIdx n-1 is fine
  }
  { // pickIndexByKey with a single entry
    const int keys[1] = { 5 };
    KT_CHECK_EQ(pickIndexByKey(keys, 1, 5), 0); KT_CHECK_EQ(pickIndexByKey(keys, 1, 9), 0);
  }
  { // shortDesc: the " This " header cut applies only below index 120
    std::string a(120, 'a'), b(119, 'a');
    KT_CHECK_EQ(shortDesc(a + " This is it. More.").c_str(), (a + " This is it.").c_str());   // p == 120: kept whole
    KT_CHECK_EQ(shortDesc(b + " This is it. More.").c_str(), "This is it.");                  // p == 119: header dropped
  }
  { // tipLayout edges
    KT_CHECK_EQ(tipLayout("ab", 100, 1000, 0, 3840).y, 8);          // topAnchor 0 is an anchor (>= 0): sits above it, clamped to the top margin
    KT_CHECK_EQ(tipLayout("ab", 100, 1000, -1, 3840).y, 1000 - 76 - 12);
    // w = 2*21+44 = 86: the right edge keeps 8 px; one px inside / exactly on / one px over the limit
    KT_CHECK_EQ(tipLayout("ab", 906, 500, -1, 1000).x, 906);
    KT_CHECK_EQ(tipLayout("ab", 905, 500, -1, 1000).x, 905);
    KT_CHECK_EQ(tipLayout("ab", 907, 500, -1, 1000).x, 906);        // 7 px from the edge: pushed to 8
    KT_CHECK_EQ(tipLayout("ab", 950, 500, -1, 1000).x, 906);
    KT_CHECK_EQ(tipLayout("ab", 7, 500, -1, 3840).x, 8);            // left margin
    KT_CHECK_EQ(tipLayout("ab", 8, 500, -1, 3840).x, 8); KT_CHECK_EQ(tipLayout("ab", 9, 500, -1, 3840).x, 9);
    KT_CHECK_EQ(tipLayout("ab", 100, 95, -1, 3840).y, 8);           // y would be 7: top margin
    KT_CHECK_EQ(tipLayout("ab", 100, 96, -1, 3840).y, 8); KT_CHECK_EQ(tipLayout("ab", 100, 97, -1, 3840).y, 9);
    TipRect d; KT_CHECK_EQ(d.lines, 1); KT_CHECK_EQ(d.maxLen, 0); KT_CHECK_EQ(d.x, 0); KT_CHECK_EQ(d.y, 0); KT_CHECK_EQ(d.w, 0); KT_CHECK_EQ(d.h, 0);
  }
  { // slotView: defaults stay quiet on the settling early-out, qty 2 shows a count
    SlotView h = slotView(false, true, false, 0, 0, "", true);
    KT_CHECK(h.hideSettling); KT_CHECK(!h.outOfStock); KT_CHECK(!h.showQty); KT_CHECK(!h.grayQty); KT_CHECK_EQ(h.qty, 0);
    KT_CHECK(!slotView(false, false, true, 1, 0x20000005, "Heal", false).hideSettling);
    SlotView t = slotView(false, true, true, 1, 0x40000009, "Medpac (2)", false);
    KT_CHECK_EQ(t.qty, 2); KT_CHECK(t.showQty); KT_CHECK(!t.grayQty);
  }
  { // aspectCorrection: every dimension of 1 is a real size (only <= 0 means unknown)
    auto near = [](double a, double b) { return a - b < 1e-9 && b - a < 1e-9; };
    KT_CHECK(near(aspectCorrection(1, 10, 10, 10), 10.0));
    KT_CHECK(near(aspectCorrection(10, 1, 10, 10), 0.1));
    KT_CHECK(near(aspectCorrection(10, 10, 1, 10), 0.1));
    KT_CHECK(near(aspectCorrection(10, 10, 10, 1), 10.0));
    KT_CHECK(near(aspectCorrection(0, 10, 10, 10), 1.0)); KT_CHECK(near(aspectCorrection(10, 0, 10, 10), 1.0));
  }
  { // decideKey / KeyEvent / KeyAction defaults: a default event is an idle, unbound, not-down key 0
    KeyAction a = decideKey(KeyEvent());
    KT_CHECK_EQ(a.decision, "pass"); KT_CHECK(!a.swallow && !a.markDown && !a.clearDown && !a.enqueue && !a.escHit); KT_CHECK_EQ(a.slot, -1);
    KeyEvent d; d.down = true; a = decideKey(d); KT_CHECK_EQ(a.decision, "pass");                    // dik defaults to 0 (not Esc)
    KeyEvent g; g.dik = 0x02;
    a = decideKey(g); KT_CHECK_EQ(a.decision, "pass"); KT_CHECK(!a.swallow && !a.clearDown);        // down / downSwallowed default false
    g.down = true; a = decideKey(g); KT_CHECK_EQ(a.decision, "pass(hud inactive)");                  // hudActive defaults false
    g.hudActive = true; a = decideKey(g); KT_CHECK_EQ(a.decision, "pass(unbound)"); KT_CHECK(!a.swallow);   // slotBound defaults false
    g.slotBound = true; a = decideKey(g); KT_CHECK_EQ(a.decision, "SWALLOW"); KT_CHECK_EQ(a.slot, 0);
  }
  { // tgaInspect: a header-sized (18 byte) buffer is long enough to be read (-> truncated, not "not uncompressed"); early-outs leave *t alone
    unsigned char h[18] = {0}; h[2] = 2; h[16] = 32; h[12] = 1; h[14] = 1;
    TgaInfo t; KT_CHECK_EQ(tgaInspect(h, 18, &t), TGA_TRUNCATED);
    KT_CHECK_EQ(t.w, 1); KT_CHECK_EQ(t.h, 1); KT_CHECK_EQ((long long)t.off, 18); KT_CHECK_EQ((long long)t.bytes, 0);
    TgaInfo u; KT_CHECK_EQ(tgaInspect(h, 17, &u), TGA_NOT_UNCOMPRESSED32);
    KT_CHECK_EQ(u.w, 0); KT_CHECK_EQ(u.h, 0); KT_CHECK_EQ((long long)u.off, 0); KT_CHECK_EQ((long long)u.bytes, 0);
    h[12] = 0; TgaInfo z; z.bytes = 7; KT_CHECK_EQ(tgaInspect(h, 18, &z), TGA_TRUNCATED); KT_CHECK_EQ((long long)z.bytes, 0);   // w == 0
    TgaInfo e; KT_CHECK_EQ(e.w, 0); KT_CHECK_EQ(e.h, 0); KT_CHECK_EQ((long long)e.off, 0); KT_CHECK_EQ((long long)e.bytes, 0);
  }
  { // ModeWatch / modeChanged / maskForm: fire once per stance/form edge; vanilla resyncs on character switch itself
    ModeWatch w;
    KT_CHECK(!modeChanged(w, 0x100, 9, 0x5, true));                 // first sight: no fire
    KT_CHECK(w.valid && w.creature == 0x100 && w.stance == 9 && w.form == 0x5);
    KT_CHECK(!modeChanged(w, 0x200, 10, 0x6, true));                // character switch: no fire
    KT_CHECK(w.creature == 0x200 && w.stance == 10 && w.form == 0x6);
    KT_CHECK(modeChanged(w, 0x200, 0xB, 0x6, true));                // stance change on new creature fires
    KT_CHECK(!modeChanged(w, 0x200, 0xB, 0x6, true));               // ...exactly once
    KT_CHECK(modeChanged(w, 0x200, 0xB, 0x7, true));                // form change fires
    KT_CHECK(!modeChanged(w, 0x200, 0xB, 0x7 | 0x20000000u, true)); // bit 29 only: vanilla masks it, no fire
    KT_CHECK_EQ(maskForm(0x20000007u), 7u); KT_CHECK_EQ(maskForm(0xFFFFFFFFu), 0xDFFFFFFFu);
    ModeWatch before = w;
    KT_CHECK(!modeChanged(w, 0x999, 16, 0x1, false));               // read failure: never fires, state untouched
    KT_CHECK(w.creature == before.creature && w.stance == before.stance && w.form == before.form && w.valid);
    KT_CHECK(!modeChanged(w, 0x200, 0xB, 0x7, true));               // still in sync after the failed read
    ModeWatch f; KT_CHECK(!modeChanged(f, 1, 2, 3, false)); KT_CHECK(!f.valid && f.creature == 0);   // failed first read leaves it invalid
  }
  { // A24 request ring: RingSize slots, one always empty (full = RingSize - 1 queued), indices wrap
    KT_CHECK_EQ(RingSize, 16);
    KT_CHECK_EQ(ringNext(0), 1); KT_CHECK_EQ(ringNext(RingSize - 2), RingSize - 1); KT_CHECK_EQ(ringNext(RingSize - 1), 0);
    KT_CHECK_EQ(ringCount(0, 0), 0); KT_CHECK_EQ(ringCount(5, 2), 3); KT_CHECK_EQ(ringCount(2, 5), RingSize - 3); KT_CHECK_EQ(ringCount(0, 1), RingSize - 1);
    KT_CHECK(!ringFull(0, 0)); KT_CHECK(ringFull(RingSize - 1, 0)); KT_CHECK(ringFull(4, 5)); KT_CHECK(!ringFull(3, 5));
    int head = 0, tail = 0, queued = 0;   // producer fills until full, consumer drains: every index stays in range
    while (!ringFull(head, tail)) { head = ringNext(head); ++queued; }
    KT_CHECK_EQ(queued, RingSize - 1); KT_CHECK_EQ(ringCount(head, tail), RingSize - 1);
    for (int i = 0; i < 5; ++i) tail = ringNext(tail);
    KT_CHECK_EQ(ringCount(head, tail), RingSize - 6); KT_CHECK(!ringFull(head, tail));
    for (int i = 0; i < 40; ++i) { head = ringNext(head); tail = ringNext(tail); KT_CHECK(head >= 0 && head < RingSize && tail >= 0 && tail < RingSize); }
  }
  { // A25/A26 key + entry flag constants: bit layout the HUD uses, shared by decodeKey / tabOfKey / isKnownPowerKey / tipText
    KT_CHECK_EQ(KeyItemFlag, 0x40000000); KT_CHECK_EQ(KeySpellFlag, 0x20000000); KT_CHECK_EQ(KeyRowMask, 0x1FFFFFFF); KT_CHECK_EQ(EntryUsable, 1);
    KT_CHECK_EQ(KeyRowMask & (KeyItemFlag | KeySpellFlag), 0);
    KT_CHECK_EQ(tabOfKey(KeySpellFlag | 7), 0); KT_CHECK_EQ(tabOfKey(KeyItemFlag | 7), 1); KT_CHECK_EQ(tabOfKey(KeyItemFlag | KeySpellFlag), 1); KT_CHECK_EQ(tabOfKey(0x410), 2);
    KT_CHECK(Tab_Power == 0 && Tab_Item == 1 && Tab_Mode == 2);
    KT_CHECK(isKnownPowerKey(KeySpellFlag | 7)); KT_CHECK(!isKnownPowerKey(KeyItemFlag | KeySpellFlag)); KT_CHECK(!isKnownPowerKey(0x410));
    KT_CHECK_EQ(decodeKey(KeySpellFlag | 0x1234, "x").id, 0x1234);
    TipSpell none;
    KT_CHECK_EQ(tipText('1', "Stance", 0x410, EntryUsable, none).c_str(), "Stance\nKey: 1   Mode");
    KT_CHECK_EQ(tipText(0, "Medpac (3)", KeyItemFlag | 9, 0, none).c_str(), "Medpac\nItem\nQuantity: 3\nNot usable right now");
    TipSpell sp; sp.fp = "10"; sp.range = "T";
    KT_CHECK_EQ(tipText('2', "Heal", KeySpellFlag | 3, EntryUsable, sp).c_str(), "Heal\nKey: 2   Force Power\nForce Points: 10\nRange: Target");
  }
  { // target-mode cycle / badge / button state / tooltips
    ActionRef it; it.kind = K_Item; it.id = 1; it.name = "medpac";
    ActionRef sp; sp.kind = K_Spell; sp.id = 3; sp.target.mode = TM_LowHp; sp.target.thresholdPct = 30;
    ActionRef em;
    auto M = [](TargetMode m, int th = -1, int fb = -1) { TargetSpec t; t.mode = m; t.thresholdPct = th; t.fallback = fb; return t; };
    auto step = [&](ActionRef a, TargetMode m, bool heal, bool selfOk) { a.target = M(m); return nextTargetMode(a, heal, selfOk); };
    KT_CHECK_EQ(step(it, TM_Default, true, true).mode, TM_LowHp);
    KT_CHECK_EQ(step(it, TM_LowHp, true, true).mode, TM_Self);
    KT_CHECK_EQ(step(it, TM_Self, true, true).mode, TM_Default);
    KT_CHECK_EQ(step(it, TM_Default, true, false).mode, TM_LowHp);
    KT_CHECK_EQ(step(it, TM_LowHp, true, false).mode, TM_Default);
    KT_CHECK_EQ(step(it, TM_Default, false, true).mode, TM_Self);
    KT_CHECK_EQ(step(it, TM_Self, false, true).mode, TM_Default);
    KT_CHECK_EQ(step(it, TM_Default, false, false).mode, TM_Default);
    KT_CHECK_EQ(step(it, TM_LowHp, false, true).mode, TM_Self);      // odd non-healing LowHp -> Self
    KT_CHECK_EQ(step(it, TM_LowHp, false, false).mode, TM_Default);
    KT_CHECK_EQ(step(it, TM_Self, true, false).mode, TM_Default);    // stale Self with self disallowed
    { ActionRef c = it; c.target = M(TM_LowHp, 25, 1); TargetSpec n = nextTargetMode(c, true, true);
      KT_CHECK_EQ(n.mode, TM_Self); KT_CHECK_EQ(n.thresholdPct, -1); KT_CHECK_EQ(n.fallback, -1);
      n = nextTargetMode(c, true, false); KT_CHECK_EQ(n.mode, TM_Default); KT_CHECK_EQ(n.thresholdPct, -1); KT_CHECK_EQ(n.fallback, -1);
      KT_CHECK(dropsCustomSuffix(c.target)); KT_CHECK(dropsCustomSuffix(M(TM_LowHp, 25))); KT_CHECK(dropsCustomSuffix(M(TM_LowHp, -1, 0)));
      KT_CHECK(!dropsCustomSuffix(M(TM_LowHp))); KT_CHECK(!dropsCustomSuffix(M(TM_Self))); }
    { TargetSpec n = nextTargetMode(sp, true, true); KT_CHECK_EQ(n.mode, TM_LowHp); KT_CHECK_EQ(n.thresholdPct, 30); }   // non-item unchanged
    { TargetSpec n = nextTargetMode(em, true, true); KT_CHECK_EQ(n.mode, TM_Default); }
    KT_CHECK(std::strcmp(badgeFor(M(TM_LowHp)), "fph_tgt_lowhp") == 0);
    KT_CHECK(std::strcmp(badgeFor(M(TM_Self)), "fph_tgt_self") == 0);
    KT_CHECK(badgeFor(M(TM_Default)) == nullptr);
    KT_CHECK_EQ(modeButtonState(em, true, true), MB_Hidden);
    KT_CHECK_EQ(modeButtonState(sp, true, true), MB_DisabledItemsOnly);
    KT_CHECK_EQ(modeButtonState(it, true, true), MB_Enabled);
    KT_CHECK_EQ(modeButtonState(it, true, false), MB_Enabled);
    KT_CHECK_EQ(modeButtonState(it, false, true), MB_Enabled);
    KT_CHECK_EQ(modeButtonState(it, false, false), MB_DisabledCannotTarget);
    KT_CHECK_EQ(modeTooltip(it, true, true).c_str(), "Target: Default (click: Lowest HP)");
    { ActionRef a = it; a.target = M(TM_LowHp); KT_CHECK_EQ(modeTooltip(a, true, true).c_str(), "Target: Lowest HP (click: Self)");
      KT_CHECK_EQ(modeTooltip(a, true, false).c_str(), "Target: Lowest HP (click: Default)");
      a.target = M(TM_Self); KT_CHECK_EQ(modeTooltip(a, true, true).c_str(), "Target: Self (click: Default)"); }
    KT_CHECK_EQ(modeTooltip(it, false, true).c_str(), "Target: Default (click: Self)");
    KT_CHECK_EQ(modeTooltip(sp, true, true).c_str(), "Target modes: items only");
    KT_CHECK_EQ(modeTooltip(it, false, false).c_str(), "Target: cannot target this item");
    KT_CHECK_EQ(modeTooltip(em, true, true).c_str(), "");
    KT_CHECK_EQ(slotTargetLine(M(TM_LowHp), 50).c_str(), "Target: Lowest HP (below 50%)");
    KT_CHECK_EQ(slotTargetLine(M(TM_LowHp, 30), 30).c_str(), "Target: Lowest HP (below 30%)");
    KT_CHECK_EQ(slotTargetLine(M(TM_Self), 50).c_str(), "Target: Self");
    KT_CHECK_EQ(slotTargetLine(M(TM_Default), 50).c_str(), "");
  }
  { // per-slot target badge rect: flush top-right inside the frame, clear of the digit and the qty row
    KT_CHECK(rectsOverlap({0,0,10,10},{5,5,10,10})); KT_CHECK(!rectsOverlap({0,0,10,10},{10,0,5,5})); KT_CHECK(!rectsOverlap({0,0,10,10},{0,10,5,5}));
    for (int i = 0; i < 10; ++i) {
        Rect4 b = badgeRect(i), digit = {33 + 40 * i, 479, 18, 18}, qty = {33 + 40 * i, 495, 27, 17}, frame = {29 + 40 * i, 478, 35, 35};
        KT_CHECK(!rectsOverlap(b, digit)); KT_CHECK(!rectsOverlap(b, qty));
        KT_CHECK(b.x >= frame.x && b.y >= frame.y && b.x + b.w <= frame.x + frame.w && b.y + b.h <= frame.y + frame.h);
        KT_CHECK_EQ(b.x, 29 + 40 * i + 22); KT_CHECK_EQ(b.y, 479); KT_CHECK_EQ(b.w, 13); KT_CHECK_EQ(b.h, 13);
    }
  }
  { // B2: item edge-case decision table (spells.2da columns, item property 0)
    KT_CHECK_EQ(parseHexOrInt("0x1408"), 0x1408);
    KT_CHECK_EQ(parseHexOrInt("0X1048"), 0x1048);
    KT_CHECK_EQ(parseHexOrInt("12"), 12);
    KT_CHECK_EQ(parseHexOrInt("****"), -1);
    KT_CHECK_EQ(parseHexOrInt(""), -1);
    KT_CHECK(isHealingRow(4, "T", 0x1408) && isHealingRow(4, "T", 0x1404) && isHealingRow(4, "T", 0x1048));
    KT_CHECK(!isHealingRow(3, "T", 0x1408) && !isHealingRow(4, "L", 0x1408) && !isHealingRow(4, "T", 0x1000) && !isHealingRow(4, "T", -1));
    KT_CHECK(selfAllowedRow("T", 0) && selfAllowedRow("t", -1) && !selfAllowedRow("T", 1));
    KT_CHECK(!selfAllowedRow("L", 0) && !selfAllowedRow("M", 0) && !selfAllowedRow("S", 0) && !selfAllowedRow("", 0));
    ItemProp p; p.count = 1; p.type = 10; p.row = 55; p.usable = 1;
    KT_CHECK_EQ(usableSpellRow(p), 55); KT_CHECK_EQ(itemPropWhy(p), "ok");
    ItemProp q = p; q.count = 0; KT_CHECK_EQ(usableSpellRow(q), -1); KT_CHECK_EQ(itemPropWhy(q), "no properties");
    q = p; q.type = 9; KT_CHECK_EQ(usableSpellRow(q), -1); KT_CHECK_EQ(itemPropWhy(q), "property 0 not CastSpell");
    q = p; q.usable = 0; KT_CHECK_EQ(usableSpellRow(q), -1); KT_CHECK_EQ(itemPropWhy(q), "no uses left");
    q = p; q.row = -1; KT_CHECK_EQ(usableSpellRow(q), -1); KT_CHECK_EQ(itemPropWhy(q), "bad row");
    KT_CHECK_EQ(usableSpellRow(ItemProp()), -1);
    { Candidate m[1] = { C(10, 100, true, false, false) };        // party of 1, self ineligible
      KT_CHECK_EQ(chooseTarget(m, 1, 0, 50, true), -1); }
    { Candidate m[2] = { C(10, 100, true, false, false), C(100, 100) };   // repair kit: only member 1 eligible, healthy; never self
      KT_CHECK_EQ(chooseTarget(m, 2, 0, 50, true), -1); }
    { Candidate m[3] = { C(100, 100, true, false, false), C(20, 100), C(60, 100) };   // medpac, self is a droid
      KT_CHECK_EQ(chooseTarget(m, 3, 0, 50, true), 1);
      Candidate n[3] = { C(100, 100, true, false, false), C(90, 100), C(20, 100) };
      KT_CHECK_EQ(chooseTarget(n, 3, 0, 50, true), 2); }
    { Candidate m[3] = { C(0, 100, true, true), C(0, 100, true, true), C(0, 100, true, true) };
      KT_CHECK_EQ(chooseTarget(m, 3, 0, 50, true), -1); }
    { Candidate m[1] = { C(10, 100) }; KT_CHECK_EQ(chooseTarget(m, 1, 0, 50, false), 0); }
  }

  { // tipTitleQty: qty only for a digits-only trailing " (N)"
    TipTitle a = tipTitleQty("Medpac (3)"); KT_CHECK_EQ(a.title.c_str(), "Medpac"); KT_CHECK_EQ(a.qty.c_str(), "3");
    TipTitle b = tipTitleQty("Medpac (Type)"); KT_CHECK_EQ(b.title.c_str(), "Medpac"); KT_CHECK_EQ(b.qty.c_str(), "");
    TipTitle c = tipTitleQty("X (Type) (3)"); KT_CHECK_EQ(c.title.c_str(), "X (Type)"); KT_CHECK_EQ(c.qty.c_str(), "3");
    TipTitle d = tipTitleQty("(3)"); KT_CHECK_EQ(d.title.c_str(), "(3)"); KT_CHECK_EQ(d.qty.c_str(), "");
    TipTitle e = tipTitleQty(""); KT_CHECK_EQ(e.title.c_str(), ""); KT_CHECK_EQ(e.qty.c_str(), "");
  }
  { // mergeSnapshot
    General g; g.healThreshold = 33; g.extra["Foo"] = "1";
    Config n = mergeSnapshot(g, nullptr);
    KT_CHECK_EQ(n.general.healThreshold, 33); KT_CHECK_EQ(n.profiles.size(), 1); KT_CHECK(n.find("Default") != nullptr);
    Config s; s.general.healThreshold = 99; s.profile("Default").slots[0][1] = mk(K_Spell, 8); s.profile("Bob");
    Config m = mergeSnapshot(g, &s);
    KT_CHECK_EQ(m.general.healThreshold, 33); KT_CHECK_EQ(m.profiles.size(), 2);
    KT_CHECK(m.find("Default") && m.find("Default")->slots[0][1].id == 8); KT_CHECK(m.find("Bob") != nullptr);
    Config s2; s2.profile("default").slots[0][0] = mk(K_Spell, 5);
    Config m2 = mergeSnapshot(g, &s2);
    KT_CHECK_EQ(m2.profiles.size(), 1); KT_CHECK(m2.find("Default") && m2.find("Default")->slots[0][0].id == 5);
    Config s3; s3.warnings.push_back("w"); Config m3 = mergeSnapshot(g, &s3);
    KT_CHECK(m3.warnings.empty()); KT_CHECK(m3.find("Default") != nullptr);
  }
  { // pickSnapshot
    struct Ex { static bool newOnly(const std::string& p) { return p == "d/action_hotbar.ini"; }
                static bool legOnly(const std::string& p) { return p == "d/force_power_hotbar.ini"; }
                static bool both(const std::string&) { return true; }
                static bool none(const std::string&) { return false; } };
    SnapPick a = pickSnapshot("d", Ex::newOnly); KT_CHECK(a.src == SS_New); KT_CHECK_EQ(a.path.c_str(), "d/action_hotbar.ini");
    SnapPick b = pickSnapshot("d", Ex::legOnly); KT_CHECK(b.src == SS_Legacy); KT_CHECK_EQ(b.path.c_str(), "d/force_power_hotbar.ini");
    SnapPick c = pickSnapshot("d", Ex::both); KT_CHECK(c.src == SS_New);
    SnapPick d = pickSnapshot("d", Ex::none); KT_CHECK(d.src == SS_Absent); KT_CHECK_EQ(d.path.c_str(), "d/action_hotbar.ini");
    KT_CHECK_EQ(snapStatus(SS_New, true), "found"); KT_CHECK_EQ(snapStatus(SS_Legacy, true), "found (legacy name)");
    KT_CHECK_EQ(snapStatus(SS_Absent, false), "absent (live hotbar kept)");
    KT_CHECK_EQ(snapStatus(SS_New, false), "UNREADABLE (live hotbar kept)"); KT_CHECK_EQ(snapStatus(SS_Legacy, false), "UNREADABLE (live hotbar kept)");
  }
  { // targetProbe
    General g; bool bad = true;
    KT_CHECK_EQ(targetProbe(g, &bad), 2); KT_CHECK(!bad); KT_CHECK_EQ(targetProbe(g), 2);
    g.extra["TargetProbe"] = "0"; KT_CHECK_EQ(targetProbe(g, &bad), 0); KT_CHECK(!bad);
    g.extra["TargetProbe"] = "1"; KT_CHECK_EQ(targetProbe(g), 1);
    g.extra["TargetProbe"] = "2"; KT_CHECK_EQ(targetProbe(g), 2);
    g.extra["TargetProbe"] = "abc"; bad = false; KT_CHECK_EQ(targetProbe(g, &bad), 2); KT_CHECK(bad);
    General h; h.extra["targetprobe"] = "1"; bad = true; KT_CHECK_EQ(targetProbe(h, &bad), 1); KT_CHECK(!bad);
  }
  { // HB04 freezeEffective: only empty, non-"none" slots take the effective action; returns the count
    ActionRef row[Slots], eff[Slots];
    row[0] = mk(K_Spell, 1);                       // explicit: kept
    row[1].blank = true;                           // explicit none: kept empty
    // row[2] empty + eff[2] set -> frozen; row[3] empty + eff[3] empty -> untouched
    eff[0] = mk(K_Spell, 99); eff[1] = mk(K_Spell, 98); eff[2] = mk(K_Item, 0, "Medpac"); eff[5] = mk(K_Spell, 7); eff[5].blank = true;   // blank flag on eff must be cleared in the copy
    KT_CHECK_EQ(freezeEffective(row, eff), 2);
    KT_CHECK(row[0].kind == K_Spell && row[0].id == 1);
    KT_CHECK(row[1].empty() && row[1].blank);
    KT_CHECK(row[2].kind == K_Item && row[2].name == "Medpac" && !row[2].blank);
    KT_CHECK(row[3].empty() && !row[3].blank);
    KT_CHECK(row[5].kind == K_Spell && row[5].id == 7 && !row[5].blank);
    KT_CHECK_EQ(freezeEffective(row, eff), 0);     // idempotent
    ActionRef none[Slots], noneEff[Slots]; KT_CHECK_EQ(freezeEffective(none, noneEff), 0);
  }
  { // HB05 useRoute: item @self needs probe >= 2, item @lowhp needs probe > 0 (asymmetry pinned); everything else invoke / log-only
    ActionRef self = mk(K_Item, 0, "Medpac"); self.target.mode = TM_Self;
    ActionRef low = self; low.target.mode = TM_LowHp;
    ActionRef def = self; def.target.mode = TM_Default;
    ActionRef spellLow = mk(K_Spell, 8); spellLow.target.mode = TM_LowHp;
    ActionRef spellSelf = mk(K_Spell, 8); spellSelf.target.mode = TM_Self;
    struct Row { const ActionRef* a; int probe; bool invoke; UseRoute want; };
    const Row t[] = {
      { &self, 0, true, UR_Invoke }, { &self, 1, true, UR_Invoke }, { &self, 2, true, UR_SelfDirect }, { &self, 3, false, UR_SelfDirect },
      { &self, 1, false, UR_LogOnly }, { &self, 0, false, UR_LogOnly },
      { &low, 0, true, UR_Invoke }, { &low, 1, true, UR_LowHpDirect }, { &low, 2, true, UR_LowHpDirect }, { &low, 1, false, UR_LowHpDirect },
      { &low, 0, false, UR_LogOnly },
      { &def, 2, true, UR_Invoke }, { &def, 2, false, UR_LogOnly },
      { &spellLow, 2, true, UR_Invoke }, { &spellSelf, 2, false, UR_LogOnly },
    };
    for (const Row& r : t) KT_CHECK_EQ((int)useRoute(*r.a, r.probe, r.invoke), (int)r.want);
    KT_CHECK(useRoute(self, 1, true) != useRoute(low, 1, true));   // probe 1: lowhp acts directly, self does not
  }
  { // HB07 collectAutos
    const ActionRef sp8 = mk(K_Spell, 8), sp10 = mk(K_Spell, 10), sp22 = mk(K_Spell, 22);
    const int kPow = 0x20000000, kItem = 0x40000000;
    std::vector<std::pair<int, ActionRef>> powers = { { kPow | 8, sp8 }, { kItem | 5, mk(K_Item, 0, "Medpac") }, { 3, mk(K_Other, 3) }, { kPow | 10, sp10 }, { kPow | 22, sp22 } };
    AutoGroup g[3];
    unsigned dm = 0xFFu;
    // autoFill off: nothing, mask cleared
    KT_CHECK(collectAutos({}, powers, g, false, &dm).empty()); KT_CHECK_EQ(dm, 0u);
    // known powers only (no item/mode keys), in order; explicit ones skipped
    std::vector<ActionRef> a = collectAutos({ sp10 }, powers, g, true, &dm);
    KT_CHECK_EQ(a.size(), 2); KT_CHECK(a.size() == 2 && a[0].id == 8 && a[1].id == 22); KT_CHECK_EQ(dm, 0u);
    // item groups: selKey match picks that entry; no match falls back to the first; empty group skipped
    ActionRef med = mk(K_Item, 0, "Medpac"), stim = mk(K_Item, 0, "Stim"), shield = mk(K_Item, 0, "Shield"), mine = mk(K_Item, 0, "Mine");
    g[0].keys = { 101, 102 }; g[0].refs = { med, stim }; g[0].selKey = 102;
    g[1].keys = { 201 }; g[1].refs = { shield }; g[1].selKey = 999;
    a = collectAutos({}, {}, g, true, &dm);
    KT_CHECK_EQ(a.size(), 2); KT_CHECK(a.size() == 2 && a[0].name == "Stim" && a[1].name == "Shield"); KT_CHECK_EQ(dm, 0u);
    // dup against explicit sets the group's bit and skips; dup against an earlier auto (power list) too
    g[2].keys = { 301 }; g[2].refs = { mine }; g[2].selKey = 301;
    a = collectAutos({ stim, mine }, {}, g, true, &dm);
    KT_CHECK_EQ(a.size(), 1); KT_CHECK(a.size() == 1 && a[0].name == "Shield"); KT_CHECK_EQ(dm, 0x5u);   // groups 0 and 2
    g[0].keys = { 7 }; g[0].refs = { sp8 }; g[0].selKey = 7;                                              // group pick equals a collected power
    a = collectAutos({}, powers, g, true, &dm);
    KT_CHECK_EQ(a.size(), 5); KT_CHECK_EQ(dm, 1u);   // 3 powers + shield + mine; group 0 dup of power 8
    KT_CHECK(collectAutos({ med }, {}, g, true, nullptr).size() == 3);                                    // null mask accepted
    // two groups picking the same item: the second is the dup
    AutoGroup h[3]; h[0].keys = { 1 }; h[0].refs = { med }; h[1].keys = { 2 }; h[1].refs = { med };
    a = collectAutos({}, {}, h, true, &dm); KT_CHECK_EQ(a.size(), 1); KT_CHECK_EQ(dm, 0x2u);
    // an empty-ref pick is not collected and sets no dup bit; a group with fewer refs than keys uses the shorter length
    AutoGroup e[3]; e[0].keys = { 1 }; e[0].refs = { ActionRef() }; e[1].keys = { 5, 6 }; e[1].refs = { shield }; e[1].selKey = 6;
    a = collectAutos({}, {}, e, true, &dm);
    KT_CHECK_EQ(a.size(), 1); KT_CHECK(a.size() == 1 && a[0].name == "Shield"); KT_CHECK_EQ(dm, 0u);
    AutoGroup m[3]; m[0].keys = { 1 }; KT_CHECK(collectAutos({}, {}, m, true, &dm).empty());              // keys without refs = empty group
  }
  { // HB08 classifyRow: direct strings
    ItemClass c = classifyRow(-1, "no uses left", 43, "T", "4", "0", "0x1408");
    KT_CHECK_EQ(c.row, -1); KT_CHECK(!c.healing && !c.selfOk); KT_CHECK_EQ(c.why, "no uses left");
    c = classifyRow(64, "ok", 0, "T", "4", "0", "0x1408");                                   // table not loaded
    KT_CHECK_EQ(c.row, 64); KT_CHECK(!c.healing && !c.selfOk); KT_CHECK_EQ(c.why, "spells.2da row unknown");
    c = classifyRow(64, "ok", 43, "T", "4", "0", "0x1408");                                  // row past the end
    KT_CHECK_EQ(c.row, 64); KT_CHECK(!c.healing && !c.selfOk); KT_CHECK_EQ(c.why, "spells.2da row unknown");
    c = classifyRow(5, "ok", 43, "T", "4", "0", "0x1408");                                   // medpac
    KT_CHECK(c.healing && c.selfOk); KT_CHECK_EQ(c.why, "ok"); KT_CHECK_EQ(c.row, 5);
    c = classifyRow(5, "ok", 43, "t", "4", "1", "0x1404");                                   // repair, hostile-flagged: no self use; range case-insensitive for self only
    KT_CHECK(!c.healing && !c.selfOk);                                                       // healing needs a capital "T"
    c = classifyRow(5, "ok", 43, "T", "4", "1", "0x1048");                                   // cure, hostile 1
    KT_CHECK(c.healing && !c.selfOk);
    c = classifyRow(5, "ok", 43, "T", "3", "****", "0x1408");                                // wrong usertype; "****" hostile = -1 -> self allowed
    KT_CHECK(!c.healing && c.selfOk);
    c = classifyRow(5, "ok", 43, "P", "4", "0", "0x1408");                                   // personal range: neither
    KT_CHECK(!c.healing && !c.selfOk);
    c = classifyRow(5, "ok", 43, "T", "4", "", "5128");                                      // decimal category 5128 = 0x1408
    KT_CHECK(c.healing && c.selfOk);
    c = classifyRow(5, "ok", 43, "T", "4", "0", "0x2441");                                   // category not on the list
    KT_CHECK(!c.healing && c.selfOk);
    c = classifyRow(0, "ok", 1, "", "", "", "");                                             // all cells empty: row 0 is valid
    KT_CHECK_EQ(c.row, 0); KT_CHECK(!c.healing && !c.selfOk); KT_CHECK_EQ(c.why, "ok");
    // spells_fixture.2da (HB09): carries the usertype / hostilesetting columns the DLL reads; rows 11/12 are synthetic healing rows
    TwoDA t; KT_CHECK(t.Load("testdata/spells_fixture.2da"));
    int n = (int)t.rows.size(); KT_CHECK_EQ(n, 43);
    KT_CHECK(t.Col("usertype") >= 0 && t.Col("hostilesetting") >= 0);
    auto cls = [&](int row) { return classifyRow(row, "ok", n, t.Get(row, "range"), t.Get(row, "usertype"), t.Get(row, "hostilesetting"), t.Get(row, "category")); };
    c = cls(11); KT_CHECK(c.healing && c.selfOk); KT_CHECK_EQ(c.row, 11);                    // usertype 4, range T, category 0x1408 (medpac), hostile 0
    c = cls(12); KT_CHECK(c.healing && !c.selfOk);                                           // category 0x1404 (repair), hostilesetting 1: healing, but no self use
    c = cls(10);                                                                             // Cure: usertype 1, range T, category 0x2441 -> not healing, self allowed
    KT_CHECK(!c.healing && c.selfOk);
    c = cls(8); KT_CHECK(!c.healing && !c.selfOk);                                           // Speed Burst: range P
    c = cls(22); KT_CHECK(!c.healing && !c.selfOk);
    c = cls(7); KT_CHECK(!c.healing && !c.selfOk);                                           // filler row: "****" range
    c = cls(42); KT_CHECK_EQ(c.row, 42); KT_CHECK_EQ(c.why, "ok");
    c = cls(43); KT_CHECK_EQ(c.why, "spells.2da row unknown");                               // one past the last row
  }
  { // HB16: parseHexOrInt: decimal, or hex only with a 0x/0X prefix (never octal); anything else, a sign, a space or an overflow is -1
    KT_CHECK_EQ(parseHexOrInt("0x1408"), 0x1408); KT_CHECK_EQ(parseHexOrInt("0X1408"), 0x1408); KT_CHECK_EQ(parseHexOrInt("0x1a0f"), 0x1a0f); KT_CHECK_EQ(parseHexOrInt("0xFF"), 255);
    KT_CHECK_EQ(parseHexOrInt("5128"), 5128); KT_CHECK_EQ(parseHexOrInt("4"), 4); KT_CHECK_EQ(parseHexOrInt("0"), 0); KT_CHECK_EQ(parseHexOrInt("0x0"), 0); KT_CHECK_EQ(parseHexOrInt("00"), 0);
    KT_CHECK_EQ(parseHexOrInt("010"), 10);                                         // was 8 (strtol base 0 = octal)
    KT_CHECK_EQ(parseHexOrInt("012010"), 12010); KT_CHECK_EQ(parseHexOrInt("08"), 8); KT_CHECK_EQ(parseHexOrInt("09"), 9);   // "08"/"09" were -1 as octal
    KT_CHECK_EQ(parseHexOrInt("2147483647"), INT_MAX); KT_CHECK_EQ(parseHexOrInt("0x7FFFFFFF"), INT_MAX);
    for (const char* bad : { "", "****", "abc", "0x", "0xG", "0x1G", "0x 5", "1408h", "12 ", " 12", "+5", "-5", "-0", "1.5", "0b101", "1e3", "x1408", "0x-1", "0x+1", "\t5", "5\n" })
      if (!KT_CHECK_EQ(parseHexOrInt(bad), -1)) std::printf("  (input '%s')\n", bad);
    KT_CHECK_EQ(parseHexOrInt("2147483648"), -1); KT_CHECK_EQ(parseHexOrInt("4294967295"), -1); KT_CHECK_EQ(parseHexOrInt("99999999999999999999999"), -1);   // overflow: -1, not a wrapped value
    KT_CHECK_EQ(parseHexOrInt("0x80000000"), -1); KT_CHECK_EQ(parseHexOrInt("0xFFFFFFFF"), -1); KT_CHECK_EQ(parseHexOrInt("0xFFFFFFFFFFFFFFFFFFFF"), -1);
    // through classifyRow: an octal-looking category is decimal now (012010 = 0x1408 only as octal), usertype "04" is 4
    ItemClass c = classifyRow(5, "ok", 43, "T", "4", "0", "012010"); KT_CHECK(!c.healing);
    c = classifyRow(5, "ok", 43, "T", "04", "0", "0x1408"); KT_CHECK(c.healing);
    c = classifyRow(5, "ok", 43, "T", "4", "01", "0x1408"); KT_CHECK(c.healing && !c.selfOk);
    c = classifyRow(5, "ok", 43, "T", "4", "-1", "0x1408"); KT_CHECK(c.healing && c.selfOk);   // a signed cell is unknown (-1), not "hostile 1"
  }
  { // HB17: the range test differs on purpose or by accident: isHealingRow wants a capital "T", selfAllowedRow accepts "t" as well. Pinned, shared HealItems.h untouched.
    KT_CHECK(!isHealingRow(4, "t", 0x1408)); KT_CHECK(selfAllowedRow("t", 0));
    KT_CHECK(isHealingRow(4, "T", 0x1408)); KT_CHECK(selfAllowedRow("T", 0));
    KT_CHECK(!isHealingRow(4, "T ", 0x1408)); KT_CHECK(!selfAllowedRow("T ", 0)); KT_CHECK(!selfAllowedRow("P", 0)); KT_CHECK(!selfAllowedRow("", 0));
    KT_CHECK(!selfAllowedRow("T", 1)); KT_CHECK(selfAllowedRow("T", -1)); KT_CHECK(selfAllowedRow("t", 2));   // only hostilesetting == 1 blocks self use
  }
  { // HB18: an unbound slot (wantEmpty) shows nothing even if a stale resolved entry is passed in: no qty, no out-of-stock, no dimming
    const float E = EmptyFrameAlpha;
    SlotView v = slotView(true, true, true, 1, 0x40000009, "Medpac (3)", false);
    KT_CHECK_EQ(v.state, "unbound"); KT_CHECK(!v.showQty && !v.grayQty && !v.outOfStock && !v.hideSettling); KT_CHECK_EQ(v.qty, 0);
    KT_CHECK_EQ(v.qtyHideWhy, "unbound"); KT_CHECK(v.bgFactor == E && v.icFactor == 1.0f);
    v = slotView(true, false, true, 0, 0x20000005, "Heal", false);                 // unusable flag on a stale entry: still not dimmed
    KT_CHECK_EQ(v.state, "unbound"); KT_CHECK(v.icFactor == 1.0f && v.bgFactor == E);
    v = slotView(true, true, false, 0, 0, "", false);                             // unbound + "wants an item" is not out of stock
    KT_CHECK_EQ(v.state, "unbound"); KT_CHECK(!v.outOfStock && !v.showQty && !v.grayQty);
    KT_CHECK(!slotView(true, false, false, 0, 0, "", true).hideSettling);          // unbound is never hidden by the settling rule
    SlotView o = slotView(false, true, false, 0, 0, "", false);                    // unchanged: a bound item that left the HUD is out of stock
    KT_CHECK(o.outOfStock && o.showQty && o.grayQty && o.qty == 0);
    // moveDuplicates: the identity is kind + id/name, the target suffix does not matter: all holders move, whatever their suffix
    Profile p; ActionRef lo = mk(K_Item, 0, "Medpac"); lo.target.mode = TM_LowHp; lo.target.thresholdPct = 40; lo.target.fallback = 0;
    ActionRef se = mk(K_Item, 0, "medpac"); se.target.mode = TM_Self; ActionRef de = mk(K_Item, 0, "MEDPAC");
    p.slots[0][1] = lo; p.slots[0][2] = se; p.slots[0][7] = de; p.slots[0][4] = mk(K_Item, 0, "Stim");
    ActionRef pick = mk(K_Item, 0, "Medpac"); pick.target.mode = TM_Self;
    std::vector<int> mv = moveDuplicates(p, 0, 4, pick);
    KT_CHECK_EQ(mv.size(), 3); KT_CHECK(mv.size() == 3 && mv[0] == 1 && mv[1] == 2 && mv[2] == 7);
    for (int s : { 1, 2, 7 }) { KT_CHECK(p.slots[0][s].empty() && p.slots[0][s].blank); KT_CHECK_EQ(p.notes[0][s].c_str(), "moved"); KT_CHECK_EQ(p.slots[0][s].target.mode, TM_Default); }
    KT_CHECK_EQ(p.slots[0][4].name.c_str(), "Stim");                              // the target slot is the caller's: untouched
    pick.target = TargetSpec(); p.slots[0][2] = se; mv = moveDuplicates(p, 0, 2, pick);   // picking onto a holder with a different suffix: that slot is the target, never moved
    KT_CHECK_EQ(mv.size(), 0); KT_CHECK_EQ(p.slots[0][2].target.mode, TM_Self);
  }
  { // HB23 (was hotbar_bugs_test): slotDigit covers 0..9 and returns '?' for anything else
    KT_CHECK_EQ(slotDigit(0), '1'); KT_CHECK_EQ(slotDigit(8), '9'); KT_CHECK_EQ(slotDigit(9), '0');
    KT_CHECK_EQ(slotDigit(-1), '?'); KT_CHECK_EQ(slotDigit(10), '?'); KT_CHECK_EQ(slotDigit(INT_MIN), '?'); KT_CHECK_EQ(slotDigit(INT_MAX), '?');
  }
  { // A11 tipText
    TipSpell none;
    KT_CHECK_EQ(tipText('1', "Medpac (3)", 0x40000000 | 5, 1, none).c_str(), "Medpac\nKey: 1   Item\nQuantity: 3");
    KT_CHECK_EQ(tipText(0, "Medpac (3)", 0x40000000 | 5, 1, none).c_str(), "Medpac\nItem\nQuantity: 3");
    KT_CHECK_EQ(tipText('2', "Grenade (x)", 0x40000000 | 5, 1, none).c_str(), "Grenade\nKey: 2   Item");        // non-numeric suffix: stripped from the title, no Quantity line
    KT_CHECK_EQ(tipText('3', "Stim", 0x40000000 | 5, 0, none).c_str(), "Stim\nKey: 3   Item\nNot usable right now");
    KT_CHECK_EQ(tipText('4', "Stance", 12, 1, none).c_str(), "Stance\nKey: 4   Mode");
    KT_CHECK_EQ(tipText(0, "Stance", 12, 0, none).c_str(), "Stance\nMode\nNot usable right now");
    TipSpell sp; sp.fp = "30"; sp.range = "T"; sp.desc = "Heals. Second sentence.";
    KT_CHECK_EQ(tipText('5', "Cure", 0x20000000 | 10, 1, sp).c_str(), "Cure\nKey: 5   Force Power\nForce Points: 30\nRange: Target\nHeals.");
    sp.fp = "0"; sp.range = "P"; sp.desc.clear();
    KT_CHECK_EQ(tipText('6', "Burst", 0x20000000 | 8, 0, sp).c_str(), "Burst\nKey: 6   Force Power\nRange: Personal\nNot usable right now");
    sp.fp.clear(); sp.range = "X";
    KT_CHECK_EQ(tipText('7', "Odd", 0x20000000 | 9, 1, sp).c_str(), "Odd\nKey: 7   Force Power");
    sp.fp = "5"; sp.range = "P"; sp.desc = "x";
    KT_CHECK_EQ(tipText('8', "Item flag wins", 0x60000000 | 9, 1, sp).c_str(), "Item flag wins\nKey: 8   Item");   // both bits: item, no spell lines, qty absent
  }
  { // A11 pickerSignature: fixed FNV, sensitive to key/flags/name/tab/order
    struct E { int key, flags; std::string name; };
    std::vector<E> a = { { 0x20000008, 1, "Speed" }, { 0x40000005, 0, "Medpac (3)" } };
    unsigned h0 = pickerSignature(a, 0);
    KT_CHECK_EQ(pickerSignature(std::vector<E>(), 0), 84696351u);
    KT_CHECK_EQ(pickerSignature(a, 0), h0);
    KT_CHECK(pickerSignature(a, 1) != h0);
    { auto b = a; b[1].name = "Medpac (2)"; KT_CHECK(pickerSignature(b, 0) != h0); }
    { auto b = a; b[0].flags = 0; KT_CHECK(pickerSignature(b, 0) != h0); }
    { auto b = a; b[0].key ^= 1; KT_CHECK(pickerSignature(b, 0) != h0); }
    { auto b = a; std::swap(b[0], b[1]); KT_CHECK(pickerSignature(b, 0) != h0); }
    KT_CHECK_EQ(h0, 860471925u);                                                 // HB21: golden values of the original edPicker loop
    KT_CHECK_EQ(pickerSignature(a, 1), 843694306u);
  }
  { // A11 modeBtnWhy: click-ignored log wording per button state
    KT_CHECK_EQ(modeBtnWhy(MB_Hidden), "empty slot"); KT_CHECK_EQ(modeBtnWhy(MB_DisabledItemsOnly), "items only");
    KT_CHECK_EQ(modeBtnWhy(MB_DisabledCannotTarget), "cannot target this item"); KT_CHECK_EQ(modeBtnWhy(MB_Enabled), "");
    ActionRef it = mk(K_Item, 0, "Medpac");
    KT_CHECK_EQ(modeBtnWhy(modeButtonState(ActionRef(), false, false)), "empty slot");
    KT_CHECK_EQ(modeBtnWhy(modeButtonState(mk(K_Spell, 8), false, false)), "items only");
    KT_CHECK_EQ(modeBtnWhy(modeButtonState(it, false, false)), "cannot target this item");
    KT_CHECK_EQ(modeBtnWhy(modeButtonState(it, true, false)), "");
  }
  return kt::Report("hotbar_logic_test");
}
