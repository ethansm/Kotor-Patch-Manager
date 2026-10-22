// Tests for EnemyStripLogic.h (doc 39): effect classification, awareness tiers, slot selection, row layout, settings-page preview layout, cooldown overlay maths.
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
    KT_CHECK(d.drainAlpha == 0.0f);   // drain shading OFF by default
    KT_CHECK(d.backingAlpha == 0.55f);
    CfgRaw s = sanitize(d);
    KT_CHECK_EQ(s.maxIcons, 5); KT_CHECK_EQ(s.tierOwn, 12); KT_CHECK_EQ(s.tierFull, 20); KT_CHECK_EQ(s.forceTier, -1);
    KT_CHECK(s.drainAlpha == 0.0f);

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
      KT_CHECK(o.drainAlpha == 0.0f); KT_CHECK(o.backingAlpha == 0.55f); }
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
    // DrainAlpha from the OFF default: 0 -> 0.05 -> 0 exactly (no float drift), down from 0 stays 0, 13 steps up reach 0.65 exactly.
    { CfgRaw c; KT_CHECK(c.drainAlpha == 0.0f);
      CfgRaw u = stepSetting(c, S_DrainAlpha, 1); KT_CHECK(u.drainAlpha == 0.05f && !overlayVisible(c.drainAlpha) && overlayVisible(u.drainAlpha));
      KT_CHECK(stepSetting(u, S_DrainAlpha, -1).drainAlpha == 0.0f); KT_CHECK(stepSetting(c, S_DrainAlpha, -1).drainAlpha == 0.0f);
      for (int i = 0; i < 12; ++i) u = stepSetting(u, S_DrainAlpha, 1);
      KT_CHECK(u.drainAlpha == 0.65f);
      for (int i = 0; i < 13; ++i) u = stepSetting(u, S_DrainAlpha, -1);
      KT_CHECK(u.drainAlpha == 0.0f); }
    // overlayVisible: only a visibly non-zero alpha (> 0.001).
    KT_CHECK(!overlayVisible(0.0f) && !overlayVisible(-0.5f) && !overlayVisible(0.001f) && !overlayVisible(NAN) && overlayVisible(0.002f) &&
             overlayVisible(0.05f) && overlayVisible(0.65f) && overlayVisible(1.0f));
    // Alpha: 0.05 steps in whole hundredths (0.65 -> 0.70 -> 0.75, no float drift after many steps), snapped to the grid, clamped 0..1.
    { CfgRaw c; c.drainAlpha = 0.65f; KT_CHECK(stepSetting(c, S_DrainAlpha, 1).drainAlpha == 0.70f); KT_CHECK(stepSetting(c, S_DrainAlpha, -1).drainAlpha == 0.60f);
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
      formatIniValue(S_DrainAlpha, c, b, sizeof b); KT_CHECK(strcmp(b, "0.00") == 0);
      formatIniValue(S_BackingAlpha, c, b, sizeof b); KT_CHECK(strcmp(b, "0.55") == 0);
      CfgRaw u = stepSetting(c, S_DrainAlpha, 1); formatIniValue(S_DrainAlpha, u, b, sizeof b);
      KT_CHECK(strcmp(b, "0.05") == 0); KT_CHECK(parseFloatStrict(b, 0.0f) == 0.05f);
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
      formatRowText(S_DrainAlpha, c, false, b, sizeof b); KT_CHECK(strcmp(b, "DrainAlpha: 0.00") == 0);
      formatRowText(S_BackingAlpha, c, false, b, sizeof b); KT_CHECK(strcmp(b, "BackingAlpha: 0.55") == 0);
      formatRowText(S_Count, c, false, b, sizeof b); KT_CHECK(b[0] == '\0');
      char small[8]; formatRowText(S_DrainAlpha, c, false, small, sizeof small); KT_CHECK(strlen(small) == 7);   // truncated, NUL kept
      formatRowText(S_Gap, c, false, nullptr, 8); }
}

static bool rr(const RowRect& r, int x, int y, int w, int h) { return r.x == x && r.y == y && r.w == w && r.h == h; }
static bool insideRect(const RowRect& r, const RowRect& p) { return r.x >= p.x && r.y >= p.y && r.x + r.w <= p.x + p.w && r.y + r.h <= p.y + p.h; }

// previewLayout: hand-computed vectors (auto icon edge 24, name bar 200x24, gap 4, spacing 4, pillPad 4 unless stated).
static void testPreview() {
    const RowRect pane = { 100, 50, 300, 200 };
    // Sample table: the strip's real icons, ratios spread over the bar, clamped index.
    KT_CHECK(strcmp(previewSample(0).resref, kIconFear) == 0 && previewSample(0).ratio == 0.25f);
    KT_CHECK(strcmp(previewSample(1).resref, kIconStun) == 0 && previewSample(1).ratio == 0.5f);
    KT_CHECK(strcmp(previewSample(2).resref, kIconSlow) == 0 && previewSample(2).ratio == 0.75f);
    KT_CHECK(strcmp(previewSample(3).resref, kIconHold) == 0 && previewSample(3).ratio == 0.1f);
    KT_CHECK(strcmp(previewSample(4).resref, kIconSleep) == 0 && previewSample(4).ratio == 0.9f);
    KT_CHECK(strcmp(previewSample(-1).resref, kIconFear) == 0 && strcmp(previewSample(99).resref, kIconSleep) == 0);
    KT_CHECK_EQ(kPreviewControls, 13);
    // Health fill inside the 200x24 bar at (150,152): inset 24/12 = 2, hh = 24*30/100 = 7, y = 152+24-7-2 = 167, x = 152, w = (200-4)*720/1000 = 141.
    { RowRect h = previewHealthRect({ 150, 152, 200, 24 }); KT_CHECK(rr(h, 152, 167, 141, 7)); }
    // Zoomed bar 880x106: inset 8, hh 31, y = y0+106-31-8, w = (880-16)*0.72 = 622; always inside the bar. A tiny bar (h 5) still yields a rect inside it.
    { const RowRect b = { 0, 100, 880, 106 }; RowRect h = previewHealthRect(b); KT_CHECK(rr(h, 8, 167, 622, 31) && insideRect(h, b)); }
    { const RowRect b = { 10, 10, 40, 5 }; RowRect h = previewHealthRect(b); KT_CHECK(h.w == 0 || insideRect(h, b)); }
    { const RowRect b = { 10, 10, 1, 24 }; RowRect h = previewHealthRect(b); KT_CHECK(h.w == 0); }
    // 1. Centred, 5 icons, auto edge, no backing: bw 200, bh 24+4+24 = 52, block (150,124); name y 124+52-24 = 152; row x0 = 150+100-68 = 182, y = 152-4-24 = 124.
    { PreviewLayout p = previewLayout(pane, 1, 5, 0, 4, 4, 4, false);
      KT_CHECK(p.ok && p.n == 5 && !p.pill);
      KT_CHECK(rr(p.name, 150, 152, 200, 24));
      KT_CHECK_EQ(p.row.n, 5);
      KT_CHECK(rr(p.row.icon[0], 182, 124, 24, 24) && rr(p.row.icon[1], 210, 124, 24, 24) && rr(p.row.icon[4], 294, 124, 24, 24));
      KT_CHECK_EQ(p.row.icon[0].x + (p.row.icon[4].x + 24 - p.row.icon[0].x) / 2, 250);   // row centred on the pane centre x = 100+150
      KT_CHECK_EQ(p.name.x + p.name.w / 2, 250);
      KT_CHECK_EQ((124 + 152 + 24) / 2 - (pane.y + pane.h / 2), 0); }   // block vertical centre: rows span 124..176, centre 150 = pane centre y 50+100
    // 2. Backing on: the pill padding joins the block (bh 56, by 122); the pill sits 4 px around the row.
    { PreviewLayout p = previewLayout(pane, 1, 5, 0, 4, 4, 4, true);
      KT_CHECK(p.ok && p.n == 5 && p.pill);
      KT_CHECK(rr(p.name, 150, 154, 200, 24));
      KT_CHECK(rr(p.row.icon[0], 182, 126, 24, 24));
      KT_CHECK(rr(p.row.pill, 178, 122, 144, 32)); }
    // 3. Row wider than the pane: 100 wide pane, 30 px icons, spacing 5: 5 -> 170, 4 -> 135, 3 -> 100 fits. Name bar narrows to the pane.
    { const RowRect narrow = { 0, 0, 100, 200 };
      PreviewLayout p = previewLayout(narrow, 1, 5, 30, 4, 5, 4, false);
      KT_CHECK(p.ok && p.n == 3);
      KT_CHECK(rr(p.name, 0, 105, 100, 24));
      KT_CHECK(rr(p.row.icon[0], 0, 71, 30, 30) && rr(p.row.icon[1], 35, 71, 30, 30) && rr(p.row.icon[2], 70, 71, 30, 30)); }
    // 3b. The pill padding counts toward the width: same pane with backing -> 2*4 more, so 2 icons (65+8 = 73; 3 icons = 108 > 100).
    { const RowRect narrow = { 0, 0, 100, 200 };
      PreviewLayout p = previewLayout(narrow, 1, 5, 30, 4, 5, 4, true);
      KT_CHECK(p.ok && p.n == 2 && p.pill && insideRect(p.row.pill, narrow)); }
    // 4. Zero / negative area: not ok, nothing to draw.
    KT_CHECK(!previewLayout({ 10, 10, 0, 100 }, 1, 5, 0, 4, 4, 4, false).ok);
    KT_CHECK(!previewLayout({ 10, 10, 100, 0 }, 1, 5, 0, 4, 4, 4, false).ok);
    KT_CHECK(!previewLayout({ 10, 10, -5, -5 }, 1, 5, 0, 4, 4, 4, true).ok);
    // 5. maxIcons 1 / 5 / 8 / 0 / negative: min(maxIcons, 5); one icon sits centred (x0 = 150+100-12 = 238).
    { PreviewLayout p1 = previewLayout(pane, 1, 1, 0, 4, 4, 4, false);
      KT_CHECK(p1.ok && p1.n == 1 && rr(p1.row.icon[0], 238, 124, 24, 24));
      KT_CHECK_EQ(previewLayout(pane, 1, 5, 0, 4, 4, 4, false).n, 5);
      KT_CHECK_EQ(previewLayout(pane, 1, 8, 0, 4, 4, 4, false).n, 5);
      KT_CHECK_EQ(previewLayout(pane, 1, 0, 0, 4, 4, 4, false).n, 0);
      KT_CHECK_EQ(previewLayout(pane, 1, -3, 0, 4, 4, 4, false).n, 0); }
    // 6. Enabled off: no row, no pill; the name bar alone is centred (by = 50 + (200-24)/2 = 138).
    { PreviewLayout p = previewLayout(pane, 0, 5, 0, 4, 4, 4, true);
      KT_CHECK(p.ok && p.n == 0 && !p.pill && rr(p.name, 150, 138, 200, 24)); }
    // 7. Pane too short for the row: row hidden, name bar kept (centred: (40-24)/2 = 8); a pane shorter than the bar shrinks the bar.
    { PreviewLayout p = previewLayout({ 0, 0, 300, 40 }, 1, 5, 0, 4, 4, 4, false);
      KT_CHECK(p.ok && p.n == 0 && rr(p.name, 50, 8, 200, 24));
      PreviewLayout q = previewLayout({ 0, 0, 300, 10 }, 1, 5, 0, 4, 4, 4, false);
      KT_CHECK(q.ok && q.n == 0 && rr(q.name, 50, 0, 200, 10)); }
    // 8. Icon edge below 4 px: no row. Negative gap / spacing / pad behave like 0.
    KT_CHECK_EQ(previewLayout(pane, 1, 5, 2, 4, 4, 4, false).n, 0);
    { PreviewLayout a = previewLayout(pane, 1, 5, 0, -9, -9, -9, true), b = previewLayout(pane, 1, 5, 0, 0, 0, 0, true);
      KT_CHECK(a.n == 5 && b.n == 5 && rr(a.name, b.name.x, b.name.y, b.name.w, b.name.h) && rr(a.row.icon[4], b.row.icon[4].x, b.row.icon[4].y, 24, 24)); }
    // 9. Wide backed row (40 px icons, 400 wide pane): 5*40+16 = 216, +8 = 224 > 200, so the block is the pill width; name x = 88+12 = 100.
    { const RowRect wide = { 0, 0, 400, 200 };
      PreviewLayout p = previewLayout(wide, 1, 5, 40, 4, 4, 4, true);
      KT_CHECK(p.ok && p.n == 5);
      KT_CHECK(rr(p.name, 100, 112, 200, 24));
      KT_CHECK(rr(p.row.icon[0], 92, 68, 40, 40));
      KT_CHECK(rr(p.row.pill, 88, 64, 224, 48)); }
    // 10. Sweep: whatever the pane and settings, every rect drawn stays inside the pane (icons, pill when drawn, name bar).
    { int bad = 0, shown = 0;
      const int ws[] = { 1, 7, 50, 99, 100, 151, 200, 201, 333, 640 }, hs[] = { 1, 9, 24, 30, 51, 52, 56, 100, 223, 500 };
      const int is[] = { 0, 4, 16, 17, 31, 64 }, gs[] = { 0, 4, 13 }, ss[] = { 0, 3, 4 }, ms[] = { 0, 1, 3, 5, 8 };
      for (int w : ws) for (int h : hs) for (int i : is) for (int g : gs) for (int sp : ss) for (int m : ms) for (int bk = 0; bk < 2; ++bk) {
          const RowRect pn = { 37, 11, w, h };
          PreviewLayout p = previewLayout(pn, 1, m, i, g, sp, 4, bk != 0);
          if (!p.ok || !insideRect(p.name, pn)) { ++bad; continue; }
          for (int k = 0; k < p.n; ++k) { ++shown; if (!insideRect(p.row.icon[k], pn) || (p.pill && !insideRect(p.row.pill, pn))) ++bad; }
          if (p.n > 0 && p.row.icon[0].y + p.row.icon[0].h + g != p.name.y) ++bad;   // the gap between row and bar is the setting
      }
      KT_CHECK_EQ(bad, 0);
      KT_CHECK(shown > 1000); }
}

// previewZoomPct / previewLayoutZoomed: hand-computed. zw = pane.w*55/200; zr = pane.w*90/rowW; zh = pane.h*90/blockH; clamp 100..600; sc(v) = (v*z+50)/100.
static void testPreviewZoom() {
    const RowRect big = { 0, 0, 1600, 900 };
    // Defaults (icon 14, gap 4, spacing 4, no backing): zw = 440, zr = 144000/86 = 1674, zh = 81000/42 = 1928 -> 440.
    KT_CHECK_EQ(previewZoomPct(big, 5, 14, 4, 4, 4, false), 440);
    { PreviewLayout p = previewLayoutZoomed(big, 1, 5, 14, 4, 4, 4, false);
      // icon (14*440+50)/100 = 62, gap 18, spacing 18, name 880 x 106 (24*440+50 = 10610 -> 106).
      KT_CHECK(p.ok && p.n == 5 && p.name.w == 880 && p.name.h == 106);
      KT_CHECK(p.row.icon[0].w == 62 && p.row.icon[0].h == 62);
      KT_CHECK_EQ(p.row.icon[1].x - p.row.icon[0].x, 62 + 18);
      KT_CHECK_EQ(p.name.y - (p.row.icon[0].y + p.row.icon[0].h), 18); }
    // A big icon edge shrinks the zoom so the row fits: icon 100 -> rowW 516, zr = 144000/516 = 279, zh = 81000/128 = 632 -> 279.
    KT_CHECK_EQ(previewZoomPct(big, 5, 100, 4, 4, 4, false), 279);
    // Backing adds 2*pad to the row width: rowW 94 -> zr = 144000/94 = 1531, still 440 (zw wins).
    KT_CHECK_EQ(previewZoomPct(big, 5, 14, 4, 4, 4, true), 440);
    // Small panes never go below 1:1, and an empty pane answers 100.
    KT_CHECK_EQ(previewZoomPct({ 0, 0, 300, 200 }, 5, 14, 4, 4, 4, false), 100);
    KT_CHECK_EQ(previewZoomPct({ 0, 0, 0, 200 }, 5, 14, 4, 4, 4, false), 100);
    // The ceiling is 600: pane 5000 wide -> zw 1375 -> 600.
    KT_CHECK_EQ(previewZoomPct({ 0, 0, 5000, 3000 }, 5, 14, 4, 4, 4, false), 600);
    // At 100 % the zoomed layout equals the 1:1 layout (same name bar, icons, pill).
    { const RowRect small = { 100, 50, 300, 200 };
      PreviewLayout a = previewLayout(small, 1, 5, 0, 4, 4, 4, true);
      PreviewLayout b = previewLayoutZoomed(small, 1, 5, 0, 4, 4, 4, true);
      KT_CHECK(a.ok == b.ok && a.n == b.n && a.pill == b.pill && rr(b.name, a.name.x, a.name.y, a.name.w, a.name.h));
      KT_CHECK(rr(b.row.pill, a.row.pill.x, a.row.pill.y, a.row.pill.w, a.row.pill.h));
      for (int i = 0; i < a.n; ++i) KT_CHECK(rr(b.row.icon[i], a.row.icon[i].x, a.row.icon[i].y, a.row.icon[i].w, a.row.icon[i].h)); }
    // Zoomed blocks stay inside the pane for every setting combination on a coarse grid.
    for (int s = 0; s <= 64; s += 16) for (int g = 0; g <= 16; g += 8) for (int sp = 0; sp <= 16; sp += 8) for (int bk = 0; bk < 2; ++bk) {
        PreviewLayout p = previewLayoutZoomed(big, 1, 5, s, g, sp, 4, bk != 0);
        if (!p.ok) continue;
        KT_CHECK(insideRect(p.name, big));
        for (int i = 0; i < p.n; ++i) KT_CHECK(insideRect(p.row.icon[i], big));
        if (p.pill) KT_CHECK(insideRect(p.row.pill, big));
    }
}

// Staged edits (0.4.3): slider mapping, setSliderValue, copySetting, pageDiffers, pageDefaults.
static void testStagedEdits() {
    // sliderHundredths: track x 100..300 (w 200): 0 at/below the left edge, 100 at/beyond the right, 0.05 grid in between.
    KT_CHECK_EQ(sliderHundredths(50, 100, 200), 0);
    KT_CHECK_EQ(sliderHundredths(100, 100, 200), 0);
    KT_CHECK_EQ(sliderHundredths(300, 100, 200), 100);
    KT_CHECK_EQ(sliderHundredths(999, 100, 200), 100);
    KT_CHECK_EQ(sliderHundredths(200, 100, 200), 50);    // half way
    KT_CHECK_EQ(sliderHundredths(130, 100, 200), 15);    // 15 %
    KT_CHECK_EQ(sliderHundredths(131, 100, 200), 15);    // 15.5 -> raw 16 -> nearest 0.05 = 15
    KT_CHECK_EQ(sliderHundredths(133, 100, 200), 15);    // 16.5 -> raw 17 -> 15
    KT_CHECK_EQ(sliderHundredths(134, 100, 200), 15);    // raw 17 -> still 15 (17/5 rounds down)
    KT_CHECK_EQ(sliderHundredths(135, 100, 200), 20);    // raw 18 -> 20
    KT_CHECK_EQ(sliderHundredths(150, 0, 0), 0);          // empty track
    KT_CHECK_EQ(sliderHundredths(150, 0, -5), 0);
    // sliderFillPx: rounds, clamps, NaN -> 0.
    KT_CHECK_EQ(sliderFillPx(0.0f, 98), 0);
    KT_CHECK_EQ(sliderFillPx(1.0f, 98), 98);
    KT_CHECK_EQ(sliderFillPx(0.65f, 98), 64);
    KT_CHECK_EQ(sliderFillPx(2.0f, 98), 98);
    KT_CHECK_EQ(sliderFillPx(-1.0f, 98), 0);
    KT_CHECK_EQ(sliderFillPx(0.5f, 0), 0);
    { const float nan = 0.0f / 0.0f; KT_CHECK_EQ(sliderFillPx(nan, 98), 0); }
    // setSliderValue touches only its own alpha and clamps 0..100.
    { CfgRaw c; CfgRaw a = setSliderValue(c, S_DrainAlpha, 70);
      KT_CHECK(a.drainAlpha == 0.70f && a.backingAlpha == c.backingAlpha);
      CfgRaw b = setSliderValue(c, S_BackingAlpha, 130);
      KT_CHECK(b.backingAlpha == 1.0f && b.drainAlpha == c.drainAlpha);
      CfgRaw d = setSliderValue(c, S_DrainAlpha, -5);
      KT_CHECK(d.drainAlpha == 0.0f);
      CfgRaw e = setSliderValue(c, S_Gap, 50);   // not a slider: unchanged
      KT_CHECK(!pageDiffers(c, e)); }
    KT_CHECK(settingIsSlider(S_BackingAlpha) && settingIsSlider(S_DrainAlpha) && !settingIsSlider(S_Gap) && !settingIsSlider(S_Backing));
    // copySetting + pageDiffers: every page setting individually; INI-only fields never count.
    { CfgRaw a, b;
      b.enabled = 0; b.maxIcons = 8; b.iconSize = 20; b.gap = 9; b.spacing = 7; b.backing = 1; b.backingAlpha = 0.9f; b.drainAlpha = 0.4f;
      b.verbose = 1; b.tierObvious = 9; b.forceTier = 2; b.backingPad = 11;
      for (int s = 0; s < S_Count; ++s) {
          CfgRaw c = copySetting(a, b, s);
          KT_CHECK(pageDiffers(a, c));
          KT_CHECK(!sameSetting(a, c, s) && sameSetting(c, b, s));
          int others = 0;
          for (int k = 0; k < S_Count; ++k) if (k != s && !sameSetting(a, c, k)) ++others;
          KT_CHECK_EQ(others, 0);
      }
      CfgRaw iniOnly = a; iniOnly.verbose = 1; iniOnly.tierObvious = 9; iniOnly.forceTier = 2; iniOnly.backingPad = 11;
      KT_CHECK(!pageDiffers(a, iniOnly));
      KT_CHECK(!pageDiffers(b, copySetting(copySetting(copySetting(copySetting(copySetting(copySetting(copySetting(copySetting(a, b, 0), b, 1), b, 2), b, 3), b, 4), b, 5), b, 6), b, 7)));
      KT_CHECK(copySetting(a, b, 99).gap == a.gap); }
    // pageDefaults: page settings at the defaults, INI-only fields kept.
    { CfgRaw k; k.enabled = 0; k.maxIcons = 8; k.gap = 9; k.drainAlpha = 0.9f; k.verbose = 1; k.forceTier = 3; k.tierFull = 33; k.backingPad = 6;
      CfgRaw d = pageDefaults(k);
      const CfgRaw def;
      KT_CHECK(!pageDiffers(d, def));
      KT_CHECK(d.verbose == 1 && d.forceTier == 3 && d.tierFull == 33 && d.backingPad == 6); }
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
    testPreview();
    testPreviewZoom();
    testStagedEdits();
    return kt::Report("enemy_strip_logic");
}
