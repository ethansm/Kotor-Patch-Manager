// Tests for EnemyStripLogic.h (doc 39): effect classification, awareness tiers, slot selection, row layout, cooldown overlay maths.
// Build/run: python3 scripts/run_tests.py enemy_strip_logic
// Expected values are hand-computed (see each case), not read back from the functions.
#include "EnemyStripLogic.h"
#include "check.h"
#include <math.h>

using namespace esl;

static EffIn mk(int type, int sub, int spell, int state, const char* dbg = nullptr, const char* buf = nullptr,
                bool byParty = false, float ratio = -1.0f, bool item = false, int int2 = 0) {
    return EffIn{ type, sub, spell, state, ratio, byParty, dbg, buf, item, int2 };
}

static void testTiers() {
    Tiers t;
    KT_CHECK_EQ(tierFor(4, t), 0);
    KT_CHECK_EQ(tierFor(5, t), 1);
    KT_CHECK_EQ(tierFor(11, t), 1);
    KT_CHECK_EQ(tierFor(12, t), 2);
    KT_CHECK_EQ(tierFor(19, t), 2);
    KT_CHECK_EQ(tierFor(20, t), 3);
    KT_CHECK_EQ(tierFor(-3, t), 0);
    Tiers u; u.obvious = 1; u.own = 2; u.full = 3;
    KT_CHECK_EQ(tierFor(2, u), 2);
    KT_CHECK(!drainAt(2));
    KT_CHECK(drainAt(3));
}

static void testClassify() {
    // Fear: type 8 state 8, spell 16, no debuff icon -> Obvious, icon from fallback.
    EffIn fear = mk(0x08, 0, 16, 8);
    KT_CHECK_EQ((int)classify(fear), (int)C_Obvious);
    KT_CHECK_EQ(stateFallbackIcon(0x08, 8), "ip_fear");
    // Stun with no spell
    EffIn stun = mk(0x08, 0, -1, 4);
    KT_CHECK_EQ((int)classify(stun), (int)C_Obvious);
    KT_CHECK_EQ(stateFallbackIcon(0x08, 4), "ip_stun");
    // Other obvious states
    int st[] = { 1, 2, 3, 4, 5, 6, 8, 16 };
    for (int s : st) KT_CHECK_EQ((int)classify(mk(0x08, 0, -1, s)), (int)C_Obvious);
    KT_CHECK_EQ((int)classify(mk(0x08, 0, -1, 7)), (int)C_Never);   // not an obvious state, no icons
    KT_CHECK_EQ((int)classify(mk(0x08, 0, -1, -1)), (int)C_Never);
    KT_CHECK_EQ((int)classify(mk(0x12, 0, -1, -1)), (int)C_Obvious);
    KT_CHECK_EQ((int)classify(mk(0x14, 0, -1, -1)), (int)C_Obvious);
    KT_CHECK_EQ((int)classify(mk(0x1d, 0, -1, -1)), (int)C_Obvious);
    // Never
    KT_CHECK_EQ((int)classify(mk(0x1e, 0, -1, -1, "ip_x", "ip_y")), (int)C_Never);   // visual, even with icons
    KT_CHECK_EQ((int)classify(mk(0x02, 8, -1, -1, "ip_x", nullptr, false, -1.0f, false, 50)), (int)C_Never);   // shield pool (bit 8, int2 > 0)
    KT_CHECK_EQ((int)classify(mk(0x02, 0x0c, -1, -1, nullptr, "ip_buff", false, -1.0f, false, 1)), (int)C_Never);
    KT_CHECK_EQ((int)classify(mk(0x02, 0x09, -1, -1, nullptr, "ip_buff", false, -1.0f, false, 0)), (int)C_Buff); // Energy Resistance: sub 9, int2 0
    KT_CHECK_EQ((int)classify(mk(0x02, 8, -1, -1, nullptr, "ip_buff")), (int)C_Buff);                  // bit 8 but int2 0 -> not a pool
    KT_CHECK_EQ((int)classify(mk(0x02, 0, -1, -1, nullptr, "ip_buff", false, -1.0f, false, 50)), (int)C_Buff); // no bit 8 -> normal buff
    int bk[] = { 0x01, 0x09, 0x13, 0x17, 0x1f, 0x39, 0x3c, 0x44, 0x67 };
    for (int t : bk) KT_CHECK_EQ((int)classify(mk(t, 0, -1, -1, "ip_d", "ip_b")), (int)C_Never);
    // Debuff / Buff / none
    KT_CHECK_EQ((int)classify(mk(0x43, 0, 5, -1, "ip_poison")), (int)C_Debuff);
    KT_CHECK_EQ((int)classify(mk(0x43, 0, 5, -1, "ip_poison", "ip_other")), (int)C_Debuff);
    KT_CHECK_EQ((int)classify(mk(0x50, 0, 5, -1, nullptr, "ip_buff")), (int)C_Buff);
    KT_CHECK_EQ((int)classify(mk(0x50, 0, 5, -1)), (int)C_Never);
    // Fallback icons
    KT_CHECK_EQ(stateFallbackIcon(0x08, 3), "ip_droidstun");
    KT_CHECK_EQ(stateFallbackIcon(0x08, 5), "ip_hold");
    KT_CHECK_EQ(stateFallbackIcon(0x08, 6), "ip_sleep");
    KT_CHECK_EQ(stateFallbackIcon(0x1d, -1), "ip_slow");
    KT_CHECK_EQ(stateFallbackIcon(0x12, -1), "iw_adhsvgren_001");
    KT_CHECK(stateFallbackIcon(0x08, 1) == nullptr);
    KT_CHECK(stateFallbackIcon(0x14, -1) == nullptr);
    KT_CHECK(stateFallbackIcon(0x50, 3) == nullptr);
}

static void testVisibility() {
    // Buff hidden below tier 3, shown at 3.
    for (int t = 0; t <= 2; ++t) { KT_CHECK(!visibleAt(C_Buff, true, t)); KT_CHECK(!visibleAt(C_Buff, false, t)); }
    KT_CHECK(visibleAt(C_Buff, false, 3));
    // Debuff gating by party at tier 2.
    KT_CHECK(!visibleAt(C_Debuff, true, 1));
    KT_CHECK(visibleAt(C_Debuff, true, 2));
    KT_CHECK(!visibleAt(C_Debuff, false, 2));
    KT_CHECK(visibleAt(C_Debuff, true, 3));
    KT_CHECK(visibleAt(C_Debuff, false, 3));
    // Obvious from tier 1.
    KT_CHECK(!visibleAt(C_Obvious, false, 0));
    KT_CHECK(visibleAt(C_Obvious, false, 1));
    KT_CHECK(visibleAt(C_Obvious, false, 3));
    // Never never.
    for (int t = 0; t <= 3; ++t) KT_CHECK(!visibleAt(C_Never, true, t));
}

static void testSlots() {
    Slot out[8];
    // Tier gating end to end: fear obvious, a party debuff, an enemy debuff, a buff.
    EffIn all[] = {
        mk(0x50, 0, 1, -1, nullptr, "ip_buff"),
        mk(0x43, 0, 2, -1, "ip_poison", nullptr, true),
        mk(0x43, 0, 3, -1, "ip_burn", nullptr, false),
        mk(0x08, 0, 16, 8),
    };
    KT_CHECK_EQ(buildSlots(all, 4, 0, out, 8, 5), 0);
    KT_CHECK_EQ(buildSlots(all, 4, 1, out, 8, 5), 1);
    KT_CHECK_EQ(out[0].resref, "ip_fear");
    KT_CHECK(out[0].debuff);
    KT_CHECK_EQ(buildSlots(all, 4, 2, out, 8, 5), 2);
    KT_CHECK_EQ(out[0].resref, "ip_fear");
    KT_CHECK_EQ(out[1].resref, "ip_poison");
    KT_CHECK_EQ(buildSlots(all, 4, 3, out, 8, 5), 4);
    // Order Obvious < Debuff < Buff, stable within class.
    KT_CHECK_EQ(out[0].resref, "ip_fear");
    KT_CHECK_EQ(out[1].resref, "ip_poison");
    KT_CHECK_EQ(out[2].resref, "ip_burn");
    KT_CHECK_EQ(out[3].resref, "ip_buff");
    KT_CHECK(out[1].debuff);
    KT_CHECK(!out[3].debuff);

    // Icon choice: debuffIcon beats state fallback; obvious with buffIcon only uses it; obvious with nothing skipped.
    EffIn pick[] = {
        mk(0x08, 0, 1, 4, "ip_special"),     // debuffIcon wins over ip_stun
        mk(0x14, 0, -1, -1),                 // knockdown, no icon at all -> skipped
        mk(0x14, 0, -1, -1, nullptr, "ip_kd"),
    };
    KT_CHECK_EQ(buildSlots(pick, 3, 1, out, 8, 5), 2);
    KT_CHECK_EQ(out[0].resref, "ip_special");
    KT_CHECK_EQ(out[1].resref, "ip_kd");
    KT_CHECK(out[1].debuff);

    // Dedupe by resref: keep first's flags, ratio = max of known ratios.
    EffIn dup[] = {
        mk(0x43, 0, 2, -1, "ip_poison", nullptr, true, -1.0f, true),
        mk(0x43, 0, 2, -1, "ip_poison", nullptr, true, 0.4f, false),
        mk(0x43, 0, 2, -1, "ip_poison", nullptr, true, 0.25f, false),
    };
    KT_CHECK_EQ(buildSlots(dup, 3, 3, out, 8, 5), 1);
    KT_CHECK(fabsf(out[0].ratio - 0.4f) < 1e-6f);
    KT_CHECK(out[0].item);
    EffIn dup2[] = {
        mk(0x43, 0, 2, -1, "ip_poison", nullptr, true, 0.9f),
        mk(0x43, 0, 2, -1, "ip_poison", nullptr, true, -1.0f),
    };
    KT_CHECK_EQ(buildSlots(dup2, 2, 3, out, 8, 5), 1);
    KT_CHECK(fabsf(out[0].ratio - 0.9f) < 1e-6f);
    // Dedupe across buff/debuff: slots are built Obvious, then Debuff, then Buff, so the debuff copy wins even though the buff is listed first.
    EffIn xd[] = {
        mk(0x50, 0, 1, -1, nullptr, "ip_same"),
        mk(0x43, 0, 2, -1, "ip_same", nullptr, false),
    };
    KT_CHECK_EQ(buildSlots(xd, 2, 3, out, 8, 5), 1);
    KT_CHECK(out[0].debuff);

    // Cap 5 of 7, then clamps of maxIcons.
    static const char* names[7] = { "i0", "i1", "i2", "i3", "i4", "i5", "i6" };
    EffIn seven[7];
    for (int i = 0; i < 7; ++i) seven[i] = mk(0x43, 0, i, -1, names[i], nullptr, true);
    KT_CHECK_EQ(buildSlots(seven, 7, 3, out, 8, 5), 5);
    KT_CHECK_EQ(out[4].resref, "i4");
    KT_CHECK_EQ(buildSlots(seven, 7, 3, out, 8, 0), 1);
    KT_CHECK_EQ(buildSlots(seven, 7, 3, out, 8, -4), 1);
    KT_CHECK_EQ(buildSlots(seven, 7, 3, out, 8, 20), 7);
    EffIn ten[10];
    static const char* n10[10] = { "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9" };
    for (int i = 0; i < 10; ++i) ten[i] = mk(0x43, 0, i, -1, n10[i], nullptr, true);
    KT_CHECK_EQ(buildSlots(ten, 10, 3, out, 8, 20), 8);
    KT_CHECK_EQ(buildSlots(ten, 0, 3, out, 8, 5), 0);
    // Null guards and outCap.
    KT_CHECK_EQ(buildSlots(nullptr, 3, 3, out, 8, 5), 0);
    KT_CHECK_EQ(buildSlots(seven, 7, 3, nullptr, 8, 5), 0);
    KT_CHECK_EQ(buildSlots(seven, -1, 3, out, 8, 5), 0);
    KT_CHECK_EQ(buildSlots(seven, 7, 3, out, 0, 5), 0);
    KT_CHECK_EQ(buildSlots(seven, 7, 3, out, -2, 5), 0);
    KT_CHECK_EQ(buildSlots(seven, 7, 3, out, 3, 5), 3);    // outCap below maxIcons wins
    KT_CHECK_EQ(buildSlots(seven, 7, 3, out, 1, 20), 1);
    KT_CHECK_EQ(buildSlots(seven, 7, 3, out, 100, 20), 7); // outCap above 8 still clamped to 8 icons
    // Dedupe still merges ratio after the cap is full.
    EffIn capdup[3] = { mk(0x43, 0, 1, -1, "x1", nullptr, true), mk(0x43, 0, 2, -1, "x2", nullptr, true),
                        mk(0x43, 0, 3, -1, "x1", nullptr, true, 0.7f) };
    KT_CHECK_EQ(buildSlots(capdup, 3, 3, out, 8, 2), 2);
    KT_CHECK(fabsf(out[0].ratio - 0.7f) < 1e-6f);
}

static void testLayout() {
    // n=1, s=16: rowW=16, x0 = 100+10+40-8 = 142; y = 200+30-4-16 = 210.
    Layout a = layoutRow(100, 200, 10, 30, 80, 20, 1, 16, 4, 2, 3);
    KT_CHECK(a.ok);
    KT_CHECK_EQ(a.n, 1);
    KT_CHECK_EQ(a.icon[0].x, 142);
    KT_CHECK_EQ(a.icon[0].y, 210);
    KT_CHECK_EQ(a.icon[0].w, 16);
    KT_CHECK_EQ(a.icon[0].h, 16);
    KT_CHECK_EQ(a.pill.x, 139);
    KT_CHECK_EQ(a.pill.y, 207);
    KT_CHECK_EQ(a.pill.w, 22);
    KT_CHECK_EQ(a.pill.h, 22);
    // n=3: rowW = 48+4 = 52, x0 = 150-26 = 124; icons at 124, 142, 160.
    Layout b = layoutRow(100, 200, 10, 30, 80, 20, 3, 16, 4, 2, 3);
    KT_CHECK(b.ok);
    KT_CHECK_EQ(b.icon[0].x, 124);
    KT_CHECK_EQ(b.icon[1].x, 142);
    KT_CHECK_EQ(b.icon[2].x, 160);
    KT_CHECK_EQ(b.pill.x, 121);
    KT_CHECK_EQ(b.pill.w, 58);
    // n=5: rowW = 80+8 = 88, x0 = 150-44 = 106; icons at 106,124,142,160,178.
    Layout c = layoutRow(100, 200, 10, 30, 80, 20, 5, 16, 4, 2, 3);
    KT_CHECK_EQ(c.icon[0].x, 106);
    KT_CHECK_EQ(c.icon[2].x, 142);
    KT_CHECK_EQ(c.icon[4].x, 178);
    // Odd rowW: s=33, n=1, nameW=533 -> x0 = 100+10+266-16 = 360 (integer halves truncate: 533/2=266, 33/2=16).
    Layout o = layoutRow(100, 200, 10, 30, 533, 20, 1, 33, 4, 2, 3);
    KT_CHECK_EQ(o.icon[0].x, 360);
    KT_CHECK_EQ(o.pill.x, 357);
    KT_CHECK_EQ(o.pill.w, 39);
    // n clamped to 8.
    Layout d = layoutRow(100, 200, 10, 30, 80, 20, 12, 16, 4, 2, 3);
    KT_CHECK_EQ(d.n, 8);
    KT_CHECK(d.ok);
    // y < 0 -> not ok (menuY+nameY-gap-s = 0+5-4-16 = -15).
    Layout e = layoutRow(0, 0, 10, 5, 80, 20, 2, 16, 4, 2, 3);
    KT_CHECK(!e.ok);
    // y == 0 is fine: 0+20-4-16 = 0.
    Layout e2 = layoutRow(0, 0, 10, 20, 80, 20, 2, 16, 4, 2, 3);
    KT_CHECK(e2.ok);
    KT_CHECK_EQ(e2.icon[0].y, 0);
    // iconPx = 0 uses nameH: s = 18, y = 200+30-4-18 = 208.
    Layout f = layoutRow(100, 200, 10, 30, 80, 18, 1, 0, 4, 2, 3);
    KT_CHECK(f.ok);
    KT_CHECK_EQ(f.icon[0].w, 18);
    KT_CHECK_EQ(f.icon[0].y, 208);
    // n = 0 and n < 0 -> not ok; size too small -> not ok.
    KT_CHECK(!layoutRow(100, 200, 10, 30, 80, 20, 0, 16, 4, 2, 3).ok);
    KT_CHECK(!layoutRow(100, 200, 10, 30, 80, 20, -2, 16, 4, 2, 3).ok);
    KT_CHECK(!layoutRow(100, 200, 10, 30, 80, 3, 2, 0, 4, 2, 3).ok);
    KT_CHECK(layoutRow(100, 200, 10, 30, 80, 20, 2, 4, 4, 2, 3).ok);
}

static void testOverlay() {
    // brightPx: 16*0.5+0.5 = 8; 16*0.97+0.5 = 16.02 -> 16; 16*0.03+0.5 = 0.98 -> 0.
    KT_CHECK_EQ(brightPx(16, 0.0f), 0);
    KT_CHECK_EQ(brightPx(16, 0.5f), 8);
    KT_CHECK_EQ(brightPx(16, 1.0f), 16);
    KT_CHECK_EQ(brightPx(16, 1.5f), 16);
    KT_CHECK_EQ(brightPx(16, -1.0f), 16);   // unknown -> fully bright
    KT_CHECK_EQ(brightPx(16, 0.97f), 16);
    KT_CHECK_EQ(brightPx(16, NAN), 16);                  // NaN -> unknown, no UB cast
    KT_CHECK_EQ(brightPx(16, INFINITY), 16);
    KT_CHECK_EQ(brightPx(16, -INFINITY), 16);
    KT_CHECK_EQ(brightPx(16, 5.0f), 16);
    KT_CHECK_EQ(brightPx(16, 1e30f), 16);
    KT_CHECK_EQ(overlayH(16, NAN), 0);
    KT_CHECK_EQ(brightPx(16, 0.03f), 0);
    KT_CHECK_EQ(brightPx(20, 0.25f), 5);
    KT_CHECK_EQ(overlayH(16, 0.5f), 8);
    KT_CHECK_EQ(overlayH(16, 1.0f), 0);
    KT_CHECK_EQ(overlayH(16, 0.0f), 16);
    KT_CHECK_EQ(overlayH(16, -1.0f), 0);
    KT_CHECK_EQ(overlayH(20, 0.25f), 15);
    // overlayAlpha: 0.65/0.78 = 0.8333...
    KT_CHECK(fabsf(overlayAlpha(0.65f) - 0.83333f) < 1e-4f);
    KT_CHECK(overlayAlpha(1.0f) == 1.0f);
    KT_CHECK(overlayAlpha(-0.5f) == 0.0f);
    KT_CHECK(fabsf(overlayAlpha(0.78f) - 1.0f) < 1e-6f);
    KT_CHECK(fabsf(overlayAlpha(0.39f) - 0.5f) < 1e-6f);
}

int main() {
    testTiers();
    testClassify();
    testVisibility();
    testSlots();
    testLayout();
    testOverlay();
    return kt::Report("enemy_strip_logic");
}
