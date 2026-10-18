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

static void testConfig() {
    // Defaults equal the shipped enemy_strip.ini and survive sanitize unchanged.
    CfgRaw d;
    KT_CHECK_EQ(d.enabled, 1); KT_CHECK_EQ(d.verbose, 0); KT_CHECK_EQ(d.maxIcons, 5); KT_CHECK_EQ(d.iconSize, 0);
    KT_CHECK_EQ(d.gap, 4); KT_CHECK_EQ(d.spacing, 4);
    KT_CHECK_EQ(d.tierObvious, 5); KT_CHECK_EQ(d.tierOwn, 12); KT_CHECK_EQ(d.tierFull, 20);
    KT_CHECK_EQ(d.backing, 0); KT_CHECK_EQ(d.backingPad, 4); KT_CHECK_EQ(d.forceTier, -1);
    KT_CHECK(d.drainAlpha == 0.65f);
    KT_CHECK(d.backingAlpha == 0.55f);
    CfgRaw s = sanitize(d);
    KT_CHECK_EQ(s.maxIcons, 5); KT_CHECK_EQ(s.tierOwn, 12); KT_CHECK_EQ(s.tierFull, 20); KT_CHECK_EQ(s.forceTier, -1);
    KT_CHECK(s.drainAlpha == 0.65f);

    // Booleans 0..1: -3 -> 0, 7 -> 1.
    { CfgRaw r; r.enabled = -3; r.verbose = 7; r.backing = 2; CfgRaw o = sanitize(r);
      KT_CHECK_EQ(o.enabled, 0); KT_CHECK_EQ(o.verbose, 1); KT_CHECK_EQ(o.backing, 1); }
    // MaxIcons 1..kMaxIcons (8): 0 -> 1, 1, 8, 9 -> 8.
    { CfgRaw r; r.maxIcons = 0; KT_CHECK_EQ(sanitize(r).maxIcons, 1);
      r.maxIcons = 1; KT_CHECK_EQ(sanitize(r).maxIcons, 1);
      r.maxIcons = 8; KT_CHECK_EQ(sanitize(r).maxIcons, 8);
      r.maxIcons = 9; KT_CHECK_EQ(sanitize(r).maxIcons, 8);
      r.maxIcons = -50; KT_CHECK_EQ(sanitize(r).maxIcons, 1); }
    // IconSize 0..256.
    { CfgRaw r; r.iconSize = -1; KT_CHECK_EQ(sanitize(r).iconSize, 0);
      r.iconSize = 0; KT_CHECK_EQ(sanitize(r).iconSize, 0);
      r.iconSize = 256; KT_CHECK_EQ(sanitize(r).iconSize, 256);
      r.iconSize = 257; KT_CHECK_EQ(sanitize(r).iconSize, 256); }
    // Gap 0..200.
    { CfgRaw r; r.gap = -1; KT_CHECK_EQ(sanitize(r).gap, 0);
      r.gap = 200; KT_CHECK_EQ(sanitize(r).gap, 200);
      r.gap = 201; KT_CHECK_EQ(sanitize(r).gap, 200); }
    // Spacing 0..100.
    { CfgRaw r; r.spacing = -9; KT_CHECK_EQ(sanitize(r).spacing, 0);
      r.spacing = 100; KT_CHECK_EQ(sanitize(r).spacing, 100);
      r.spacing = 101; KT_CHECK_EQ(sanitize(r).spacing, 100); }
    // BackingPad 0..32.
    { CfgRaw r; r.backingPad = -1; KT_CHECK_EQ(sanitize(r).backingPad, 0);
      r.backingPad = 32; KT_CHECK_EQ(sanitize(r).backingPad, 32);
      r.backingPad = 33; KT_CHECK_EQ(sanitize(r).backingPad, 32); }
    // Tier values 0..127 (ordered inputs so only the range acts): obvious -4 -> 0; full 500 -> 127; own 500 -> 127 then full raised to 127.
    { CfgRaw r; r.tierObvious = -4; CfgRaw o = sanitize(r); KT_CHECK_EQ(o.tierObvious, 0); KT_CHECK_EQ(o.tierOwn, 12);
      CfgRaw r2; r2.tierFull = 500; KT_CHECK_EQ(sanitize(r2).tierFull, 127);
      CfgRaw r3; r3.tierOwn = 500; CfgRaw o3 = sanitize(r3); KT_CHECK_EQ(o3.tierOwn, 127); KT_CHECK_EQ(o3.tierFull, 127);
      CfgRaw r4; r4.tierObvious = r4.tierOwn = r4.tierFull = 127; CfgRaw o4 = sanitize(r4);
      KT_CHECK_EQ(o4.tierObvious, 127); KT_CHECK_EQ(o4.tierOwn, 127); KT_CHECK_EQ(o4.tierFull, 127); }
    // Tier ordering: the LATER value is raised. Own 3 < Obvious 5 -> 5; Full 8 < Own 12 -> 12.
    { CfgRaw r; r.tierOwn = 3; CfgRaw o = sanitize(r);
      KT_CHECK_EQ(o.tierObvious, 5); KT_CHECK_EQ(o.tierOwn, 5); KT_CHECK_EQ(o.tierFull, 20);
      CfgRaw r2; r2.tierFull = 8; CfgRaw o2 = sanitize(r2);
      KT_CHECK_EQ(o2.tierObvious, 5); KT_CHECK_EQ(o2.tierOwn, 12); KT_CHECK_EQ(o2.tierFull, 12);
      // Cascade: Obvious 30 raises Own to 30 and Full to 30.
      CfgRaw r3; r3.tierObvious = 30; CfgRaw o3 = sanitize(r3);
      KT_CHECK_EQ(o3.tierObvious, 30); KT_CHECK_EQ(o3.tierOwn, 30); KT_CHECK_EQ(o3.tierFull, 30);
      // Equal values are legal and untouched.
      CfgRaw r4; r4.tierObvious = 10; r4.tierOwn = 10; r4.tierFull = 10; CfgRaw o4 = sanitize(r4);
      KT_CHECK_EQ(o4.tierObvious, 10); KT_CHECK_EQ(o4.tierOwn, 10); KT_CHECK_EQ(o4.tierFull, 10); }
    // ForceTier -1..3: 7 -> 3, -5 -> -1, 0 and -1 and 3 kept.
    { CfgRaw r; r.forceTier = 7; KT_CHECK_EQ(sanitize(r).forceTier, 3);
      r.forceTier = -5; KT_CHECK_EQ(sanitize(r).forceTier, -1);
      r.forceTier = 0; KT_CHECK_EQ(sanitize(r).forceTier, 0);
      r.forceTier = -1; KT_CHECK_EQ(sanitize(r).forceTier, -1);
      r.forceTier = 3; KT_CHECK_EQ(sanitize(r).forceTier, 3); }
    // Alphas 0..1: edges kept, out of range clamped, infinities clamp, NaN -> that field's default.
    { CfgRaw r; r.drainAlpha = -0.5f; r.backingAlpha = 1.5f; CfgRaw o = sanitize(r);
      KT_CHECK(o.drainAlpha == 0.0f); KT_CHECK(o.backingAlpha == 1.0f);
      r.drainAlpha = 0.0f; r.backingAlpha = 1.0f; o = sanitize(r);
      KT_CHECK(o.drainAlpha == 0.0f); KT_CHECK(o.backingAlpha == 1.0f);
      r.drainAlpha = INFINITY; r.backingAlpha = -INFINITY; o = sanitize(r);
      KT_CHECK(o.drainAlpha == 1.0f); KT_CHECK(o.backingAlpha == 0.0f);
      r.drainAlpha = NAN; r.backingAlpha = NAN; o = sanitize(r);
      KT_CHECK(o.drainAlpha == 0.65f); KT_CHECK(o.backingAlpha == 0.55f); }
    // Explicit bound vectors (each fails an identity sanitize).
    { CfgRaw r; r.enabled = 5; KT_CHECK_EQ(sanitize(r).enabled, 1); }
    { CfgRaw r; r.verbose = -3; KT_CHECK_EQ(sanitize(r).verbose, 0); }
    { CfgRaw r; r.backing = -1; KT_CHECK_EQ(sanitize(r).backing, 0); }
    { CfgRaw r; r.tierObvious = 500; CfgRaw o = sanitize(r);   // clamp to 127, then Own and Full are raised to 127
      KT_CHECK_EQ(o.tierObvious, 127); KT_CHECK_EQ(o.tierOwn, 127); KT_CHECK_EQ(o.tierFull, 127); }
    { CfgRaw r; r.tierObvious = 0; r.tierOwn = -4; CfgRaw o = sanitize(r);
      KT_CHECK_EQ(o.tierObvious, 0); KT_CHECK_EQ(o.tierOwn, 0); KT_CHECK_EQ(o.tierFull, 20); }
    { CfgRaw r; r.tierObvious = 0; r.tierOwn = 0; r.tierFull = -4; CfgRaw o = sanitize(r);
      KT_CHECK_EQ(o.tierObvious, 0); KT_CHECK_EQ(o.tierOwn, 0); KT_CHECK_EQ(o.tierFull, 0); }
    // Untouched fields stay put when another is out of range.
    { CfgRaw r; r.gap = 999; CfgRaw o = sanitize(r); KT_CHECK_EQ(o.spacing, 4); KT_CHECK_EQ(o.maxIcons, 5); KT_CHECK_EQ(o.backingPad, 4); }
}

static void testParseFloat() {
    KT_CHECK(fabsf(parseFloatStrict("0.65", 0.1f) - 0.65f) < 1e-6f);
    KT_CHECK(parseFloatStrict("0.65abc", 0.1f) == 0.1f);        // trailing junk -> default
    KT_CHECK(fabsf(parseFloatStrict(" 0.5 ", 0.1f) - 0.5f) < 1e-6f);   // surrounding whitespace is fine
    KT_CHECK(fabsf(parseFloatStrict("0.5\r\n", 0.1f) - 0.5f) < 1e-6f);
    KT_CHECK(parseFloatStrict("", 0.1f) == 0.1f);
    KT_CHECK(parseFloatStrict("   ", 0.1f) == 0.1f);
    KT_CHECK(parseFloatStrict("abc", 0.1f) == 0.1f);
    KT_CHECK(parseFloatStrict("0.5 0.6", 0.1f) == 0.1f);
    KT_CHECK(parseFloatStrict("nan", 0.1f) == 0.1f);
    KT_CHECK(parseFloatStrict(nullptr, 0.1f) == 0.1f);
    KT_CHECK(fabsf(parseFloatStrict("-2", 0.1f) + 2.0f) < 1e-6f);   // no clamping here; sanitize does that
    KT_CHECK(parseFloatStrict("1e999", 0.1f) == 1e30f);             // huge -> finite, cast stays defined
    // End to end: junk -> default -> sanitize keeps it; out-of-range number -> clamped.
    CfgRaw r; r.drainAlpha = parseFloatStrict("0.9x", 0.65f); r.backingAlpha = parseFloatStrict("7", 0.55f);
    CfgRaw o = sanitize(r);
    KT_CHECK(o.drainAlpha == 0.65f); KT_CHECK(o.backingAlpha == 1.0f);
}

static void testSettings() {
    // Keys and kinds.
    KT_CHECK(strcmp(settingKey(S_Enabled), "Enabled") == 0);
    KT_CHECK(strcmp(settingKey(S_MaxIcons), "MaxIcons") == 0);
    KT_CHECK(strcmp(settingKey(S_IconSize), "IconSize") == 0);
    KT_CHECK(strcmp(settingKey(S_Gap), "Gap") == 0);
    KT_CHECK(strcmp(settingKey(S_Spacing), "Spacing") == 0);
    KT_CHECK(strcmp(settingKey(S_Backing), "Backing") == 0);
    KT_CHECK(strcmp(settingKey(S_BackingAlpha), "BackingAlpha") == 0);
    KT_CHECK(strcmp(settingKey(S_DrainAlpha), "DrainAlpha") == 0);
    KT_CHECK(settingKey(S_Count) == nullptr);
    KT_CHECK(settingKey(-1) == nullptr);
    KT_CHECK(settingIsToggle(S_Enabled) && settingIsToggle(S_Backing) && !settingIsToggle(S_Gap));
    KT_CHECK(settingIsFloat(S_DrainAlpha) && settingIsFloat(S_BackingAlpha) && !settingIsFloat(S_IconSize));

    // Integer steps: +1 / -1 from the defaults (gap 4, spacing 4, maxIcons 5), clamped at the ends.
    { CfgRaw c; KT_CHECK_EQ(stepSetting(c, S_Gap, 1).gap, 5); KT_CHECK_EQ(stepSetting(c, S_Gap, -1).gap, 3);
      KT_CHECK_EQ(stepSetting(c, S_Spacing, 1).spacing, 5); KT_CHECK_EQ(stepSetting(c, S_Spacing, -1).spacing, 3);
      KT_CHECK_EQ(stepSetting(c, S_MaxIcons, 1).maxIcons, 6); KT_CHECK_EQ(stepSetting(c, S_MaxIcons, -1).maxIcons, 4); }
    { CfgRaw c; c.gap = 0; KT_CHECK_EQ(stepSetting(c, S_Gap, -1).gap, 0); c.gap = 200; KT_CHECK_EQ(stepSetting(c, S_Gap, 1).gap, 200);
      c.spacing = 0; KT_CHECK_EQ(stepSetting(c, S_Spacing, -1).spacing, 0); c.spacing = 100; KT_CHECK_EQ(stepSetting(c, S_Spacing, 1).spacing, 100);
      c.maxIcons = 1; KT_CHECK_EQ(stepSetting(c, S_MaxIcons, -1).maxIcons, 1); c.maxIcons = 8; KT_CHECK_EQ(stepSetting(c, S_MaxIcons, 1).maxIcons, 8); }
    // A step changes only its own field (the other fields, including the tiers and ForceTier, stay put).
    { CfgRaw c; c.forceTier = 2; c.tierOwn = 9; c.verbose = 1; CfgRaw o = stepSetting(c, S_Gap, 1);
      KT_CHECK_EQ(o.forceTier, 2); KT_CHECK_EQ(o.tierOwn, 9); KT_CHECK_EQ(o.verbose, 1); KT_CHECK_EQ(o.spacing, 4); KT_CHECK_EQ(o.maxIcons, 5); }
    // Icon size: auto (0) -> 16 -> 18 ...; at 8 or below a down step returns to auto; 256 is the top; 0 down stays 0.
    { CfgRaw c; KT_CHECK_EQ(stepSetting(c, S_IconSize, 1).iconSize, 16); KT_CHECK_EQ(stepSetting(c, S_IconSize, -1).iconSize, 0);
      c.iconSize = 16; KT_CHECK_EQ(stepSetting(c, S_IconSize, 1).iconSize, 18); KT_CHECK_EQ(stepSetting(c, S_IconSize, -1).iconSize, 14);
      c.iconSize = 9; KT_CHECK_EQ(stepSetting(c, S_IconSize, -1).iconSize, 7);
      c.iconSize = 8; KT_CHECK_EQ(stepSetting(c, S_IconSize, -1).iconSize, 0);
      c.iconSize = 3; KT_CHECK_EQ(stepSetting(c, S_IconSize, -1).iconSize, 0);
      c.iconSize = 255; KT_CHECK_EQ(stepSetting(c, S_IconSize, 1).iconSize, 256);
      c.iconSize = 256; KT_CHECK_EQ(stepSetting(c, S_IconSize, 1).iconSize, 256); }
    // Toggles flip on either direction; dir 0 changes nothing.
    { CfgRaw c; KT_CHECK_EQ(stepSetting(c, S_Enabled, 1).enabled, 0); KT_CHECK_EQ(stepSetting(c, S_Enabled, -1).enabled, 0);
      c.enabled = 0; KT_CHECK_EQ(stepSetting(c, S_Enabled, 1).enabled, 1);
      CfgRaw d; KT_CHECK_EQ(stepSetting(d, S_Backing, 1).backing, 1); d.backing = 1; KT_CHECK_EQ(stepSetting(d, S_Backing, -1).backing, 0);
      KT_CHECK(sameSetting(d, stepSetting(d, S_Backing, 0), S_Backing)); }
    // Alpha: 0.05 steps in whole hundredths (0.65 -> 0.70 -> 0.75, no float drift after many steps), snapped to the grid, clamped 0..1.
    { CfgRaw c; KT_CHECK(stepSetting(c, S_DrainAlpha, 1).drainAlpha == 0.70f); KT_CHECK(stepSetting(c, S_DrainAlpha, -1).drainAlpha == 0.60f);
      KT_CHECK(stepSetting(c, S_BackingAlpha, 1).backingAlpha == 0.60f); KT_CHECK(stepSetting(c, S_BackingAlpha, -1).backingAlpha == 0.50f); }
    { CfgRaw c; c.drainAlpha = 0.0f; for (int i = 0; i < 40; ++i) c = stepSetting(c, S_DrainAlpha, 1);
      KT_CHECK(c.drainAlpha == 1.0f);
      for (int i = 0; i < 20; ++i) c = stepSetting(c, S_DrainAlpha, -1);
      KT_CHECK(c.drainAlpha == 0.0f);
      c = stepSetting(c, S_DrainAlpha, -1); KT_CHECK(c.drainAlpha == 0.0f); }
    { CfgRaw c; c.drainAlpha = 0.67f; KT_CHECK(stepSetting(c, S_DrainAlpha, 1).drainAlpha == 0.70f);   // 0.67 snaps to 0.65 first, then +0.05
      c.drainAlpha = 0.62f; KT_CHECK(stepSetting(c, S_DrainAlpha, -1).drainAlpha == 0.55f); }   // 0.62 snaps to 0.60, then -0.05
    // INI text: ints plain, alphas two decimals; the step result round-trips through parseFloatStrict.
    { CfgRaw c; char b[32];
      formatIniValue(S_Gap, c, b, sizeof b); KT_CHECK(strcmp(b, "4") == 0);
      formatIniValue(S_Enabled, c, b, sizeof b); KT_CHECK(strcmp(b, "1") == 0);
      formatIniValue(S_IconSize, c, b, sizeof b); KT_CHECK(strcmp(b, "0") == 0);
      formatIniValue(S_DrainAlpha, c, b, sizeof b); KT_CHECK(strcmp(b, "0.65") == 0);
      formatIniValue(S_BackingAlpha, c, b, sizeof b); KT_CHECK(strcmp(b, "0.55") == 0);
      CfgRaw u = stepSetting(c, S_DrainAlpha, 1); formatIniValue(S_DrainAlpha, u, b, sizeof b);
      KT_CHECK(strcmp(b, "0.70") == 0); KT_CHECK(parseFloatStrict(b, 0.0f) == 0.70f);
      formatIniValue(S_Count, c, b, sizeof b); KT_CHECK(b[0] == '\0');
      formatIniValue(S_Gap, c, b, 3); KT_CHECK(strcmp(b, "4") == 0);
      char tiny[2]; formatIniValue(S_DrainAlpha, c, tiny, sizeof tiny); KT_CHECK(tiny[0] == '0' && tiny[1] == '\0');   // truncated, NUL kept
      formatIniValue(S_Gap, c, nullptr, 8); formatIniValue(S_Gap, c, b, 0); }
    // Row text.
    { CfgRaw c; char b[64];
      formatRowText(S_Enabled, c, false, b, sizeof b); KT_CHECK(strcmp(b, "Enabled: On") == 0);
      c.enabled = 0; formatRowText(S_Enabled, c, false, b, sizeof b); KT_CHECK(strcmp(b, "Enabled: Off") == 0);
      c.enabled = 1; formatRowText(S_Enabled, c, true, b, sizeof b); KT_CHECK(strcmp(b, "Enabled: On (restart)") == 0);
      formatRowText(S_MaxIcons, c, true, b, sizeof b); KT_CHECK(strcmp(b, "MaxIcons: 5 (restart)") == 0);
      formatRowText(S_IconSize, c, false, b, sizeof b); KT_CHECK(strcmp(b, "IconSize: Auto") == 0);
      c.iconSize = 24; formatRowText(S_IconSize, c, false, b, sizeof b); KT_CHECK(strcmp(b, "IconSize: 24") == 0);
      formatRowText(S_Gap, c, false, b, sizeof b); KT_CHECK(strcmp(b, "Gap: 4") == 0);
      formatRowText(S_Spacing, c, false, b, sizeof b); KT_CHECK(strcmp(b, "Spacing: 4") == 0);
      c.backing = 1; formatRowText(S_Backing, c, false, b, sizeof b); KT_CHECK(strcmp(b, "Backing: On") == 0);
      formatRowText(S_DrainAlpha, c, false, b, sizeof b); KT_CHECK(strcmp(b, "DrainAlpha: 0.65") == 0);
      formatRowText(S_BackingAlpha, c, false, b, sizeof b); KT_CHECK(strcmp(b, "BackingAlpha: 0.55") == 0);
      formatRowText(S_Count, c, false, b, sizeof b); KT_CHECK(b[0] == '\0');
      char small[8]; formatRowText(S_DrainAlpha, c, false, small, sizeof small); KT_CHECK(strlen(small) == 7);   // truncated, NUL kept
      formatRowText(S_Gap, c, false, nullptr, 8); }
}

int main() {
    testConfig();
    testParseFloat();
    testTiers();
    testClassify();
    testVisibility();
    testSlots();
    testLayout();
    testOverlay();
    testSettings();
    return kt::Report("enemy_strip_logic");
}
