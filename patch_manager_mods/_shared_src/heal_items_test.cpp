// Host test of HealItems.h: formula table (vs R3 items.tsv), pickHealItem worked examples + tie-breaks, shouldAutoUse boundaries.
// Expected values are independent literals (FINDINGS.md "Worked examples").
#include "HealItems.h"
#include "check.h"

using namespace heal;

static HealCand C(int key, int row, int h, int cnt, int cost = 100, bool u = true) { return HealCand{key, row, h, cnt, cost, u}; }
template <size_t N> static int pick(int miss, const HealCand (&c)[N], int minCount = 1) { return pickHealItem(miss, c, (int)N, minCount); }

int main() {
    // ---- formula table (items.tsv) ----
    struct T { int row; Family f; int base, mul; } rows[] = {
        {64, Family::Organic, 10, 1}, {65, Family::Organic, 20, 2}, {66, Family::Organic, 30, 3},
        {84, Family::Droid, 15, 1}, {127, Family::Droid, 25, 2}, {128, Family::Droid, 35, 3}};
    for (auto& r : rows) {
        const Formula* f = formulaFor(r.row);
        KT_CHECK(f != nullptr);
        if (f) { KT_CHECK(f->fam == r.f); KT_CHECK_EQ(f->base, r.base); KT_CHECK_EQ(f->skillMul, r.mul); KT_CHECK_EQ(f->row, r.row); }
    }
    KT_CHECK(formulaFor(67) == nullptr); KT_CHECK(formulaFor(129) == nullptr); KT_CHECK(formulaFor(0) == nullptr); KT_CHECK(formulaFor(-1) == nullptr);
    KT_CHECK_EQ(abilityMod(9), -1); KT_CHECK_EQ(abilityMod(10), 0); KT_CHECK_EQ(abilityMod(15), 2); KT_CHECK_EQ(abilityMod(8), -1); KT_CHECK_EQ(abilityMod(7), -2);
    KT_CHECK_EQ(abilityMod(11), 0); KT_CHECK_EQ(abilityMod(12), 1); KT_CHECK_EQ(abilityMod(6), -2); KT_CHECK_EQ(abilityMod(5), -3);

    // ---- healEstimate (TI=10, WIS 14 -> +2; Repair 8, INT 16 -> +3) ----
    KT_CHECK_EQ(healEstimate(64, 10, 99, 14, 99), 22); KT_CHECK_EQ(healEstimate(65, 10, 99, 14, 99), 42); KT_CHECK_EQ(healEstimate(66, 10, 99, 14, 99), 62);
    KT_CHECK_EQ(healEstimate(84, 99, 8, 99, 16), 26); KT_CHECK_EQ(healEstimate(127, 99, 8, 99, 16), 44); KT_CHECK_EQ(healEstimate(128, 99, 8, 99, 16), 62);
    KT_CHECK_EQ(healEstimate(64, 0, 0, 10, 10), 10); KT_CHECK_EQ(healEstimate(65, 0, 0, 10, 10), 20);
    KT_CHECK_EQ(healEstimate(64, 24, 0, 16, 0), 37); KT_CHECK_EQ(healEstimate(65, 24, 0, 16, 0), 71); KT_CHECK_EQ(healEstimate(66, 24, 0, 16, 0), 105);
    KT_CHECK_EQ(healEstimate(67, 10, 10, 10, 10), -1); KT_CHECK_EQ(healEstimate(129, 10, 10, 10, 10), -1);

    // ---- worked examples 1-11 ----
    { HealCand c[] = {C(1, 64, 22, 5), C(2, 65, 42, 2), C(3, 66, 62, 1)};
      KT_CHECK_EQ(pick(15, c), 0); KT_CHECK_EQ(pick(30, c), 1); KT_CHECK_EQ(pick(100, c), 2); KT_CHECK_EQ(pick(0, c), -1); KT_CHECK_EQ(pick(-5, c), -1); }
    { HealCand c[] = {C(1, 64, 22, 1), C(2, 65, 42, 1)}; KT_CHECK_EQ(pick(22, c), 0); KT_CHECK_EQ(pick(23, c), 1); }
    { HealCand c[] = {C(1, 84, 26, 3), C(2, 127, 44, 1), C(3, 128, 62, 0)}; KT_CHECK_EQ(pick(40, c), 1); }
    { HealCand c[] = {C(1, 67, 10, 1), C(2, 129, 10, 4)}; KT_CHECK_EQ(pick(50, c), -1); }
    { HealCand c[] = {C(1, 64, 22, 0), C(2, 65, 42, 2)}; KT_CHECK_EQ(pick(10, c), 1); }
    { HealCand c[] = {C(1, 64, 22, 1, 40), C(2, 64, 22, 4, 40)}; KT_CHECK_EQ(pick(20, c), 1); }
    { HealCand c[] = {C(1, 64, 10, 2), C(2, 65, 20, 1)}; KT_CHECK_EQ(pick(15, c), 1); }
    { HealCand c[] = {C(1, 64, 37, 1), C(2, 65, 71, 1), C(3, 66, 105, 1)}; KT_CHECK_EQ(pick(60, c), 1); }

    // ---- filters ----
    { HealCand c[] = {C(1, 64, 22, 5, 100, false)}; KT_CHECK_EQ(pick(10, c), -1); }          // unusable
    { HealCand c[] = {C(1, 64, 0, 5)}; KT_CHECK_EQ(pick(10, c), -1); }                         // heal 0
    { HealCand c[] = {C(1, 64, -3, 5)}; KT_CHECK_EQ(pick(10, c), -1); }                        // heal negative
    { HealCand c[] = {C(1, 99, 50, 5)}; KT_CHECK_EQ(pick(10, c), -1); }                        // no formula
    KT_CHECK_EQ(pickHealItem(10, nullptr, 0, 1), -1);
    { HealCand c[] = {C(1, 64, 22, 1)}; KT_CHECK_EQ(pickHealItem(10, c, 0, 1), -1); }
    // minCount / keep: auto keep=2 -> minCount 3
    { HealCand c[] = {C(1, 64, 22, 3), C(2, 65, 42, 2), C(3, 66, 62, 5)};
      KT_CHECK_EQ(pick(10, c, 3), 0); KT_CHECK_EQ(pick(30, c, 3), 2); KT_CHECK_EQ(pick(30, c, 1), 1);
      KT_CHECK_EQ(pick(10, c, 6), -1); KT_CHECK_EQ(pick(10, c, 5), 2); }
    { HealCand c[] = {C(1, 64, 22, 2)}; KT_CHECK_EQ(pick(10, c, 2), 0); KT_CHECK_EQ(pick(10, c, 3), -1); }

    // ---- covering vs non-covering preference (order independent) ----
    { HealCand c[] = {C(1, 66, 62, 1), C(2, 65, 42, 1), C(3, 64, 22, 1)}; KT_CHECK_EQ(pick(30, c), 1); KT_CHECK_EQ(pick(100, c), 0); KT_CHECK_EQ(pick(10, c), 2); }
    { HealCand c[] = {C(1, 64, 22, 1), C(2, 65, 42, 1)}; KT_CHECK_EQ(pick(100, c), 1); }      // non-covering: largest
    { HealCand c[] = {C(1, 65, 42, 1), C(2, 64, 22, 1)}; KT_CHECK_EQ(pick(100, c), 0); KT_CHECK_EQ(pick(25, c), 0); }
    { HealCand c[] = {C(1, 64, 22, 1), C(2, 65, 42, 1)}; KT_CHECK_EQ(pick(15, c), 0); }       // both cover, smaller wins (later candidate larger)
    { HealCand c[] = {C(1, 64, 22, 1), C(2, 65, 42, 1)}; KT_CHECK_EQ(pick(30, c), 1); }       // earlier non-covering replaced by later covering
    // ---- tie-breaks, each rule isolated ----
    { HealCand c[] = {C(1, 64, 22, 1, 50), C(2, 64, 22, 1, 40)}; KT_CHECK_EQ(pick(20, c), 1); }   // lower cost
    { HealCand c[] = {C(1, 64, 22, 1, 40), C(2, 64, 22, 1, 50)}; KT_CHECK_EQ(pick(20, c), 0); }
    { HealCand c[] = {C(1, 64, 22, 1, 40), C(2, 64, 22, 3, 40)}; KT_CHECK_EQ(pick(20, c), 1); }   // larger count
    { HealCand c[] = {C(1, 64, 22, 3, 40), C(2, 64, 22, 1, 40)}; KT_CHECK_EQ(pick(20, c), 0); }
    { HealCand c[] = {C(7, 64, 22, 2, 40), C(3, 64, 22, 2, 40)}; KT_CHECK_EQ(pick(20, c), 1); }   // lower key
    { HealCand c[] = {C(3, 64, 22, 2, 40), C(7, 64, 22, 2, 40)}; KT_CHECK_EQ(pick(20, c), 0); }
    { HealCand c[] = {C(1, 64, 22, 1, 40), C(2, 64, 22, 9, 90)}; KT_CHECK_EQ(pick(20, c), 0); }   // cost beats count
    { HealCand c[] = {C(1, 64, 22, 1, 40), C(2, 65, 42, 9, 10)}; KT_CHECK_EQ(pick(20, c), 0); }   // heal beats cost
    // non-covering ties use same rules
    { HealCand c[] = {C(1, 64, 22, 1, 50), C(2, 64, 22, 1, 40)}; KT_CHECK_EQ(pick(500, c), 1); }

    // ---- isHealingRow ----
    KT_CHECK(isHealingRow(4, "T", 0x1408)); KT_CHECK(isHealingRow(4, "T", 0x1404)); KT_CHECK(isHealingRow(4, "T", 0x1048));
    KT_CHECK(!isHealingRow(3, "T", 0x1408)); KT_CHECK(!isHealingRow(4, "t", 0x1408)); KT_CHECK(!isHealingRow(4, "T", 0x1409));

    // ---- shouldAutoUse ----
    AutoCfg on; on.on = true;   // hpPct 35, keep 2
    AutoState st;
    KT_CHECK(shouldAutoUse(on, 34, 100, true, true, true, st, 1000));
    KT_CHECK(!shouldAutoUse(on, 35, 100, true, true, true, st, 1000));      // exactly at threshold
    KT_CHECK(!shouldAutoUse(on, 36, 100, true, true, true, st, 1000));
    KT_CHECK(shouldAutoUse(on, 0, 100, true, true, true, st, 1000));
    KT_CHECK(shouldAutoUse(on, 6, 20, true, true, true, st, 1000));         // 600 < 700
    KT_CHECK(!shouldAutoUse(on, 7, 20, true, true, true, st, 1000));        // 700 !< 700
    KT_CHECK(!shouldAutoUse(on, 0, 0, true, true, true, st, 1000));         // max 0
    KT_CHECK(!shouldAutoUse(on, 0, -5, true, true, true, st, 1000));
    { AutoCfg off; KT_CHECK(!shouldAutoUse(off, 1, 100, true, true, true, st, 1000)); }
    KT_CHECK(!shouldAutoUse(on, 1, 100, false, true, true, st, 1000));
    KT_CHECK(!shouldAutoUse(on, 1, 100, true, false, true, st, 1000));
    KT_CHECK(!shouldAutoUse(on, 1, 100, true, true, false, st, 1000));
    { AutoCfg c = on; c.hpPct = 100; KT_CHECK(shouldAutoUse(c, 99, 100, true, true, true, st, 5)); KT_CHECK(!shouldAutoUse(c, 100, 100, true, true, true, st, 5)); }
    { AutoCfg c = on; c.hpPct = 0; KT_CHECK(!shouldAutoUse(c, 0, 100, true, true, true, st, 5)); }
    { AutoCfg c = on; c.hpPct = 100; KT_CHECK(shouldAutoUse(c, 2000000000, 2100000000, true, true, true, st, 5)); }   // int64: 2e11 < 2.1e11
    { AutoCfg c = on; c.hpPct = 50; KT_CHECK(!shouldAutoUse(c, 2000000000, 2100000000, true, true, true, st, 5)); }
    { AutoState p; p.pending = true; KT_CHECK(!shouldAutoUse(on, 1, 100, true, true, true, p, 1000)); }
    { AutoState a; a.lastUseMs = 1000; KT_CHECK(!shouldAutoUse(on, 1, 100, true, true, true, a, 6999)); KT_CHECK(shouldAutoUse(on, 1, 100, true, true, true, a, 7000));
      KT_CHECK(shouldAutoUse(on, 1, 100, true, true, true, a, 7001)); KT_CHECK(!shouldAutoUse(on, 1, 100, true, true, true, a, 1000)); }
    { AutoState a; a.lastUseMs = 0xFFFFFFFFu - 1000; // now wrapped to small: elapsed = 1001+now
      KT_CHECK(!shouldAutoUse(on, 1, 100, true, true, true, a, 4000));     // 5001
      KT_CHECK(!shouldAutoUse(on, 1, 100, true, true, true, a, 4998));     // 5999
      KT_CHECK(shouldAutoUse(on, 1, 100, true, true, true, a, 4999));      // 6000
      KT_CHECK(shouldAutoUse(on, 1, 100, true, true, true, a, 10000)); }
    KT_CHECK_EQ(AutoCooldownMs, 6000u); KT_CHECK_EQ(AutoEvalMs, 250u);
    { AutoCfg d; KT_CHECK(!d.on); KT_CHECK_EQ(d.hpPct, 35); KT_CHECK_EQ(d.keep, 2); }
    return kt::Report("heal_items_test");
}
