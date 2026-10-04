// Pure-logic tests for the hotbar model (U2: picking an action already on the bar MOVES it).
#include "HotbarConfig.h"
#include "HotbarLogic.h"
#include "check.h"
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
  // ======== units extracted from ForcePowerHotbar.cpp (behaviour-neutral; doc 20 sec 4.1) ========
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
  { // 2. characterKeyFor: obj%08x / invalid / party==1
    bool bad = true;
    KT_CHECK_EQ(characterKeyFor(0x7F000001u, 3, &bad).c_str(), "obj7f000001"); KT_CHECK(!bad);
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFDu, 1, &bad).c_str(), "objfffffffd"); KT_CHECK(!bad);
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFFu, 1, &bad).c_str(), "objfffffffd"); KT_CHECK(bad);   // one-member party = the Exile
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFFu, 2, &bad).c_str(), ""); KT_CHECK(bad);
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFFu, 0, &bad).c_str(), ""); KT_CHECK(bad);
    KT_CHECK_EQ(characterKeyFor(0u, 1, &bad).c_str(), ""); KT_CHECK(bad);                       // id 0 never gets the party==1 rescue
    KT_CHECK_EQ(characterKeyFor(0u, 3, &bad).c_str(), ""); KT_CHECK(bad);
    KT_CHECK_EQ(characterKeyFor(1u, 3).c_str(), "obj00000001");                                // bad == nullptr allowed
    KT_CHECK_EQ(characterKeyFor(0xFFFFFFFEu, 1).c_str(), "objfffffffe");
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
    // signature: equals the original loop (reproduced below); changes with id / auto flag / name
    ActionRef z[Slots]; bool za[Slots] = {};
    unsigned h0 = effSignature(z, za);
    KT_CHECK_EQ(h0, effSignature(z, za));                                          // deterministic
    unsigned h = 2166136261u;                                                      // reference implementation, verbatim from the DLL
    for (int s = 0; s < Slots; ++s) { h = (h ^ (unsigned)Format(z[s]).size() ^ ((unsigned)z[s].id << 8) ^ (za[s] ? 0x5000000u : 0)) * 16777619u; for (char c : Format(z[s])) h = (h ^ (unsigned char)c) * 16777619u; }
    KT_CHECK_EQ(h0, h);
    ActionRef a[Slots]; bool aa[Slots] = {}; a[2] = mk(K_Spell, 8); a[5] = mk(K_Item, 0, "Medpac");
    unsigned h1 = effSignature(a, aa);
    h = 2166136261u;
    for (int s = 0; s < Slots; ++s) { h = (h ^ (unsigned)Format(a[s]).size() ^ ((unsigned)a[s].id << 8) ^ (aa[s] ? 0x5000000u : 0)) * 16777619u; for (char c : Format(a[s])) h = (h ^ (unsigned char)c) * 16777619u; }
    KT_CHECK_EQ(h1, h);
    KT_CHECK(h1 != h0);
    bool ab[Slots] = {}; ab[2] = true;
    KT_CHECK(effSignature(a, ab) != h1);                                           // auto flag changes it
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
    KT_CHECK_EQ(TipWrap, 38); KT_CHECK_EQ(TipMaxDescLines, 3);
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
    KT_CHECK(QtyOutOfStockRgb[0] == 0.55f && QtyOutOfStockRgb[1] == 0.55f && QtyOutOfStockRgb[2] == 0.55f);
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
    KT_CHECK_EQ(ArrowFirstMs, 350); KT_CHECK_EQ(ArrowRepeatMs, 90);
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
  { // 11. save-folder parsing
    KT_CHECK_EQ(normalizeSaveFolder("SAVES:000012 - Game11").c_str(), "000012 - Game11");
    KT_CHECK_EQ(normalizeSaveFolder("saves:000012 - Game11").c_str(), "000012 - Game11");          // alias is case-insensitive
    KT_CHECK_EQ(normalizeSaveFolder("000012 - Game11").c_str(), "000012 - Game11");
    KT_CHECK_EQ(normalizeSaveFolder("SAVES:/000012 - Game11").c_str(), "000012 - Game11");
    KT_CHECK_EQ(normalizeSaveFolder("SAVES:\\\\000012 - Game11").c_str(), "000012 - Game11");
    KT_CHECK_EQ(normalizeSaveFolder("/000001 - AUTOSAVE").c_str(), "000001 - AUTOSAVE");
    KT_CHECK_EQ(normalizeSaveFolder("SAVES:").c_str(), "");
    KT_CHECK_EQ(normalizeSaveFolder("").c_str(), "");
    KT_CHECK_EQ(normalizeSaveFolder("12 - Game").c_str(), "");                                      // too short
    KT_CHECK_EQ(normalizeSaveFolder("000012 - ").c_str(), "");                                      // no name
    KT_CHECK_EQ(normalizeSaveFolder("000012 - G").c_str(), "000012 - G");
    KT_CHECK_EQ(normalizeSaveFolder("00001x - Game11").c_str(), "");                                // non-digit
    KT_CHECK_EQ(normalizeSaveFolder("000012-Game11").c_str(), "");                                  // separator
    KT_CHECK_EQ(normalizeSaveFolder("000012  -Game11").c_str(), "");
    KT_CHECK_EQ(normalizeSaveFolder("SAVES:SAVES:000012 - Game11").c_str(), "");
    KT_CHECK_EQ(joinSaveDir("saves/", "000012 - G").c_str(), "saves/000012 - G");
    KT_CHECK_EQ(joinSaveDir("cloudsaves/123/", "000012 - G").c_str(), "cloudsaves/123/000012 - G");
    KT_CHECK_EQ(joinSaveDir("cloudsaves/123", "000012 - G").c_str(), "cloudsaves/123/000012 - G");   // missing slash added
    KT_CHECK_EQ(joinSaveDir("cloudsaves\\123\\", "000012 - G").c_str(), "cloudsaves\\123\\000012 - G");
    KT_CHECK_EQ(joinSaveDir("", "000012 - G").c_str(), "saves/000012 - G");                          // empty cloud dir -> saves/
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
      KT_CHECK_EQ(MaxTgaDim, 4096);
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
  { // slotView: defaults stay quiet on the settling early-out, qty 2 shows a count, out-of-stock colour has 3 channels
    SlotView h = slotView(false, true, false, 0, 0, "", true);
    KT_CHECK(h.hideSettling); KT_CHECK(!h.outOfStock); KT_CHECK(!h.showQty); KT_CHECK(!h.grayQty); KT_CHECK_EQ(h.qty, 0);
    KT_CHECK(!slotView(false, false, true, 1, 0x20000005, "Heal", false).hideSettling);
    SlotView t = slotView(false, true, true, 1, 0x40000009, "Medpac (2)", false);
    KT_CHECK_EQ(t.qty, 2); KT_CHECK(t.showQty); KT_CHECK(!t.grayQty);
    KT_CHECK_EQ(sizeof(QtyOutOfStockRgb) / sizeof(QtyOutOfStockRgb[0]), 3);
  }
  { // aspectCorrection: every dimension of 1 is a real size (only <= 0 means unknown)
    auto near = [](double a, double b) { return a - b < 1e-9 && b - a < 1e-9; };
    KT_CHECK(near(aspectCorrection(1, 10, 10, 10), 10.0));
    KT_CHECK(near(aspectCorrection(10, 1, 10, 10), 0.1));
    KT_CHECK(near(aspectCorrection(10, 10, 1, 10), 0.1));
    KT_CHECK(near(aspectCorrection(10, 10, 10, 1), 10.0));
    KT_CHECK(near(aspectCorrection(0, 10, 10, 10), 1.0)); KT_CHECK(near(aspectCorrection(10, 0, 10, 10), 1.0));
  }
  { // normalizeSaveFolder: the first digit is checked and '9' is a digit
    KT_CHECK_EQ(normalizeSaveFolder("A00012 - Game11").c_str(), "");
    KT_CHECK_EQ(normalizeSaveFolder("0000a2 - Game11").c_str(), "");
    KT_CHECK_EQ(normalizeSaveFolder("999999 - Game11").c_str(), "999999 - Game11");
    KT_CHECK_EQ(normalizeSaveFolder("SAVES:090909 - G").c_str(), "090909 - G");
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
  { // ModeWatch / modeChanged / maskForm / stanceKey: fire once per stance/form edge; vanilla resyncs on character switch itself
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
    KT_CHECK_EQ(stanceKey(9), 0x410); KT_CHECK_EQ(stanceKey(10), 0x411); KT_CHECK_EQ(stanceKey(0xB), 0x412);
    KT_CHECK_EQ(stanceKey(0xC), 0x413); KT_CHECK_EQ(stanceKey(0xD), 0x414); KT_CHECK_EQ(stanceKey(16), 0x415);
    KT_CHECK_EQ(stanceKey(0), 0); KT_CHECK_EQ(stanceKey(14), 0); KT_CHECK_EQ(stanceKey(-1), 0);
  }
  return kt::Report("hotbar_logic_test");
}
