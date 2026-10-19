// Host test of SelfHealUi.h: button geometry, state priority, steppers, INI codec.
#include "SelfHealUi.h"
#include "check.h"

using namespace shui;

static bool parse(const char* l, std::string& k, CompCfg& c) { return parseLine(l, k, c); }

int main() {
    // ---- buttonRect ----
    // ---- discDiameter / buttonRect: disc square = queue strip icon (autoIconPx), cross button = 2/3 of it, disc right edge `gap` left of vit.x ----
    // WI-38 C219: disc is fixed at kDefaultDisc (58) for every bar height (no 48-72 clamp)
    KT_CHECK_EQ(discDiameter(96), 58); KT_CHECK_EQ(discDiameter(36), 58); KT_CHECK_EQ(discDiameter(64), 58); KT_CHECK_EQ(discDiameter(80), 58);
    KT_CHECK_EQ(discDiameter(200), 58); KT_CHECK_EQ(discDiameter(0), 58); KT_CHECK_EQ(discDiameter(1000), 58);
    { Rect r = buttonRect({3671, 1285, 47, 96}); KT_CHECK(r == (Rect{3617, 1313, 39, 39}));   // live small card, 3840x1600 (doc 08 probe 1)
      Rect rg = ringRect(r); KT_CHECK_EQ(rg.w, 58); KT_CHECK_EQ(rg.h, 58); KT_CHECK_EQ(rg.x + rg.w + 5, 3671);   // disc = 58 px, gap round(5.33)=5
      KT_CHECK_EQ(rg.y + rg.h / 2, 1285 + 96 / 2); }                                                          // centred on the bar
    for (int h = 1; h <= 400; ++h) {
        Rect r = buttonRect({1000, 100, 47, h}); if (r.w <= 0) continue;
        Rect rg = ringRect(r);
        KT_CHECK(rg.w - discDiameter(h) >= -1 && rg.w - discDiameter(h) <= 1);      // ring square == disc diameter (+/-1)
        KT_CHECK(r.w == r.h); KT_CHECK(rg.x + rg.w <= 1000 - 2);                    // clear of the bar, gap >= 2
    }
    { Rect r = buttonRect({100, 0, 47, 400}); KT_CHECK_EQ(r.w, 39); KT_CHECK_EQ(ringRect(r).w, 58); }   // h=400 allowed, disc fixed at 58
    { Rect z{0, 0, 0, 0}; KT_CHECK(buttonRect({100, 50, 70, 36}) == z); KT_CHECK(buttonRect({100, 50, 103, 208}) == z); KT_CHECK(buttonRect({100, 50, 0, 36}) == z); KT_CHECK(!(buttonRect({100, 50, 69, 96}) == z)); }   // leader (w >= 70) / empty; 69 = small card
    { KT_CHECK(isLeaderVit({3511, 1386, 103, 208})); KT_CHECK(!isLeaderVit({3671, 1285, 47, 96})); }
    { Rect z{0, 0, 0, 0}; KT_CHECK(buttonRect({100, 50, 47, 0}) == z); KT_CHECK(buttonRect({100, 50, 47, -5}) == z); KT_CHECK(buttonRect({100, 50, 47, 401}) == z); }
    { Rect z{0, 0, 0, 0}; KT_CHECK(buttonRect({59, 50, 47, 36}) == z); KT_CHECK(!(buttonRect({60, 50, 47, 36}) == z)); }   // disc 58 + gap 2: left edge 0 ok, -1 refused
    { Rect r = buttonRect({60, 50, 47, 36}); KT_CHECK_EQ(ringRect(r).x, 0); }

    // ---- resolveState priority ----
    // args: hasCreature,isLeader,cur,max,havePick,pending,autoOn
    KT_CHECK(resolveState(false, false, 1, 10, true, true, true) == BtnState::Hidden);
    KT_CHECK(resolveState(true, true, 1, 10, true, true, true) == BtnState::Hidden);
    KT_CHECK(resolveState(true, false, 1, 0, true, true, true) == BtnState::Hidden);
    KT_CHECK(resolveState(true, false, 0, -1, false, false, false) == BtnState::Hidden);
    KT_CHECK(resolveState(true, false, 10, 10, false, true, true) == BtnState::FullHp);
    KT_CHECK(resolveState(true, false, 11, 10, true, true, true) == BtnState::FullHp);
    KT_CHECK(resolveState(true, false, 9, 10, false, true, true) == BtnState::NoItem);
    KT_CHECK(resolveState(true, false, 9, 10, true, true, true) == BtnState::Pending);
    KT_CHECK(resolveState(true, false, 9, 10, true, false, true) == BtnState::AutoOn);
    KT_CHECK(resolveState(true, false, 9, 10, true, false, false) == BtnState::Normal);
    KT_CHECK(resolveState(true, false, 1, 1, true, false, false) == BtnState::FullHp);
    // down (unconscious) beats everything except Hidden; waking restores the previous state (cfg untouched)
    KT_CHECK(resolveState(true, false, 0, 10, true, true, true, true) == BtnState::Down);
    KT_CHECK(resolveState(true, false, 0, 10, false, false, false, true) == BtnState::Down);
    KT_CHECK(resolveState(true, false, 10, 10, true, false, false, true) == BtnState::Down);
    KT_CHECK(resolveState(false, false, 0, 10, true, false, false, true) == BtnState::Hidden);
    KT_CHECK(resolveState(true, true, 0, 10, true, false, false, true) == BtnState::Hidden);
    KT_CHECK(resolveState(true, false, 0, 0, true, false, false, true) == BtnState::Hidden);
    KT_CHECK(resolveState(true, false, 4, 10, true, false, true, false) == BtnState::AutoOn);   // awake again: previous look

    // ---- click ----
    KT_CHECK(!clickAllowed(BtnState::Hidden)); KT_CHECK(!clickAllowed(BtnState::FullHp)); KT_CHECK(!clickAllowed(BtnState::Pending)); KT_CHECK(!clickAllowed(BtnState::Down));
    KT_CHECK(clickAllowed(BtnState::NoItem)); KT_CHECK(clickAllowed(BtnState::AutoOn)); KT_CHECK(clickAllowed(BtnState::Normal));
    KT_CHECK_EQ(clickMinCount(BtnState::Normal), 1); KT_CHECK_EQ(clickMinCount(BtnState::AutoOn), 1);

    // ---- steppers ----
    KT_CHECK_EQ(stepHpPct(35, 1), 40); KT_CHECK_EQ(stepHpPct(35, -1), 30); KT_CHECK_EQ(stepHpPct(35, 0), 35);
    KT_CHECK_EQ(stepHpPct(95, 1), 95); KT_CHECK_EQ(stepHpPct(90, 1), 95); KT_CHECK_EQ(stepHpPct(5, -1), 5); KT_CHECK_EQ(stepHpPct(10, -1), 5);
    KT_CHECK_EQ(stepHpPct(200, 1), 95); KT_CHECK_EQ(stepHpPct(-50, -1), 5); KT_CHECK_EQ(stepHpPct(200, 0), 95); KT_CHECK_EQ(stepHpPct(0, 0), 5);
    KT_CHECK_EQ(stepHpPct(35, 7), 40); KT_CHECK_EQ(stepHpPct(35, -7), 30);
    KT_CHECK_EQ(stepKeep(2, 1), 3); KT_CHECK_EQ(stepKeep(2, -1), 1); KT_CHECK_EQ(stepKeep(2, 0), 2);
    KT_CHECK_EQ(stepKeep(20, 1), 20); KT_CHECK_EQ(stepKeep(19, 1), 20); KT_CHECK_EQ(stepKeep(0, -1), 0); KT_CHECK_EQ(stepKeep(1, -1), 0);
    KT_CHECK_EQ(stepKeep(99, 1), 20); KT_CHECK_EQ(stepKeep(-9, -1), 0); KT_CHECK_EQ(stepKeep(99, 0), 20); KT_CHECK_EQ(stepKeep(-9, 0), 0);
    KT_CHECK_EQ(stepKeep(2, 5), 3); KT_CHECK_EQ(stepKeep(2, -5), 1);

    // ---- INI ----
    { CompCfg d; KT_CHECK(!d.autoOn); KT_CHECK_EQ(d.hpPct, 35); KT_CHECK_EQ(d.keep, 2); }
    { std::string k; CompCfg c; KT_CHECK(parse("bastila=auto:1,hp:50,keep:4", k, c)); KT_CHECK(k == "bastila"); KT_CHECK(c.autoOn); KT_CHECK_EQ(c.hpPct, 50); KT_CHECK_EQ(c.keep, 4); }
    { std::string k; CompCfg c; KT_CHECK(parse("  t3 =  auto : 1 , hp : 20 , keep : 0  \r\n", k, c)); KT_CHECK(k == "t3"); KT_CHECK(c.autoOn); KT_CHECK_EQ(c.hpPct, 20); KT_CHECK_EQ(c.keep, 0); }
    { std::string k; CompCfg c; c.autoOn = true; KT_CHECK(parse("a=auto:0", k, c)); KT_CHECK(!c.autoOn); KT_CHECK_EQ(c.hpPct, 35); KT_CHECK_EQ(c.keep, 2); }
    { std::string k; CompCfg c; KT_CHECK(parse("a=hp:1,keep:99", k, c)); KT_CHECK_EQ(c.hpPct, 5); KT_CHECK_EQ(c.keep, 20); }
    { std::string k; CompCfg c; KT_CHECK(parse("a=hp:100,keep:-3", k, c)); KT_CHECK_EQ(c.hpPct, 95); KT_CHECK_EQ(c.keep, 0); }
    { std::string k; CompCfg c; KT_CHECK(parse("a=hp:+50,keep:+3", k, c)); KT_CHECK_EQ(c.hpPct, 50); KT_CHECK_EQ(c.keep, 3); }
    { std::string k; CompCfg c; KT_CHECK(parse("a=hp:abc,keep:x1,auto:", k, c)); KT_CHECK_EQ(c.hpPct, 35); KT_CHECK_EQ(c.keep, 2); KT_CHECK(!c.autoOn); }
    { std::string k; CompCfg c; KT_CHECK(parse("a=hp:-,keep:", k, c)); KT_CHECK_EQ(c.hpPct, 35); KT_CHECK_EQ(c.keep, 2); }
    { std::string k; CompCfg c; KT_CHECK(parse("a=hp:99999999999999,keep:99999999999999", k, c)); KT_CHECK_EQ(c.hpPct, 95); KT_CHECK_EQ(c.keep, 20); }
    { std::string k; CompCfg c; KT_CHECK(parse("a=hp:-99999999999999", k, c)); KT_CHECK_EQ(c.hpPct, 5); }
    { std::string k; CompCfg c; KT_CHECK(parse("a=foo:9,bar,hp:40,,keep:1,auto:2", k, c)); KT_CHECK_EQ(c.hpPct, 40); KT_CHECK_EQ(c.keep, 1); KT_CHECK(c.autoOn); }
    { std::string k; CompCfg c; KT_CHECK(parse("a=", k, c)); KT_CHECK(k == "a"); KT_CHECK(c == CompCfg()); }
    { std::string k; CompCfg c; KT_CHECK(parse("a=b=c,hp:60", k, c)); KT_CHECK(k == "a"); KT_CHECK_EQ(c.hpPct, 60); }
    { std::string k = "keep"; CompCfg c;
      KT_CHECK(!parse("", k, c)); KT_CHECK(!parse("   ", k, c)); KT_CHECK(!parse("no equals", k, c)); KT_CHECK(!parse("=auto:1", k, c)); KT_CHECK(!parse("  =auto:1", k, c));
      KT_CHECK(!parse("; a=auto:1", k, c)); KT_CHECK(!parse("# a=auto:1", k, c)); KT_CHECK(!parse(";a=auto:1", k, c)); KT_CHECK(!parse("#a=hp:9", k, c));
      KT_CHECK(k == "keep"); KT_CHECK(c == CompCfg()); }
    { CompCfg c; c.autoOn = true; c.hpPct = 35; c.keep = 2; KT_CHECK(serialize("k", c) == "k=auto:1,hp:35,keep:2"); }
    { CompCfg c; KT_CHECK(serialize("zz", c) == "zz=auto:0,hp:35,keep:2"); }
    for (int a = 0; a < 2; ++a) for (int hp = 5; hp <= 95; hp += 5) for (int kp = 0; kp <= 20; kp += 5) {
        CompCfg x; x.autoOn = a; x.hpPct = hp; x.keep = kp;
        std::string k; CompCfg y; y.autoOn = !a; y.hpPct = 1; y.keep = 1;
        KT_CHECK(parse(serialize("tag", x).c_str(), k, y)); KT_CHECK(k == "tag"); KT_CHECK(y == x);
    }

    // ---- popover layout ----
    { Rect b{200, 50, 16, 16}; PopLayout L = popoverLayout(b, 1920);
      KT_CHECK(L.r[0] == (Rect{200 - 4 - 128, 50, 128, 48}));   // gap h/4 = 4, w = 8h
      KT_CHECK(L.r[1] == (Rect{68, 50, 128, 16}));
      KT_CHECK(L.r[2] == (Rect{68, 66, 16, 16})); KT_CHECK(L.r[3] == (Rect{68 + 112, 66, 16, 16}));
      KT_CHECK(L.r[4] == (Rect{68, 82, 16, 16})); KT_CHECK(L.r[5] == (Rect{180, 82, 16, 16}));
      KT_CHECK(popValueRect(L, 2) == (Rect{84, 66, 96, 16})); KT_CHECK(popValueRect(L, 3) == (Rect{84, 82, 96, 16}));
      KT_CHECK(popValueRect(L, 1) == (Rect{0, 0, 0, 0})); KT_CHECK(popValueRect(L, 4) == (Rect{0, 0, 0, 0})); }
    { Rect b{400, 10, 24, 24}; PopLayout L = popoverLayout(b, 0);
      KT_CHECK(L.r[0] == (Rect{400 - 6 - 192, 10, 192, 72})); KT_CHECK(L.r[5] == (Rect{202 + 168, 58, 24, 24})); }
    { PopLayout L = popoverLayout(Rect{50, 5, 16, 16}, 1920); KT_CHECK_EQ(L.r[0].x, 0); KT_CHECK_EQ(L.r[3].x, 112); }   // left clamp
    { PopLayout L = popoverLayout(Rect{300, 5, 16, 16}, 200); KT_CHECK_EQ(L.r[0].x, 72); KT_CHECK_EQ(L.r[0].x + L.r[0].w, 200); }   // right clamp
    { PopLayout L = popoverLayout(Rect{300, 5, 16, 16}, 100); KT_CHECK_EQ(L.r[0].x, 0); }   // left wins
    { PopLayout L = popoverLayout(Rect{300, 5, 16, 16}, 1000); KT_CHECK_EQ(L.r[0].x, 300 - 4 - 128); }   // no clamp when it fits
    { PopLayout L = popoverLayout(Rect{300, 5, 16, 16}, 1); KT_CHECK_EQ(L.r[0].x, 0); }   // tiny screen still clamps
    { PopLayout L = popoverLayout(Rect{200, 50, 16, 16}, 1920); KT_CHECK(L.r[0].x + L.r[0].w == 200 - 4); }
    // ---- popover hit ----
    for (int h : {16, 24}) {
        PopLayout L = popoverLayout(Rect{400, 50, h, h}, 1920);
        for (int i = 1; i <= 5; ++i) {
            const Rect& r = L.r[i];
            KT_CHECK(popoverHit(L, r.x, r.y) == static_cast<PopItem>(i));
            KT_CHECK(popoverHit(L, r.x + r.w - 1, r.y + r.h - 1) == static_cast<PopItem>(i));
            KT_CHECK(popoverHit(L, r.x + r.w / 2, r.y + r.h / 2) == static_cast<PopItem>(i));
        }
        KT_CHECK(popoverHit(L, L.r[0].x + h + 1, L.r[0].y + h + 1) == PopItem::None);   // value text area
        KT_CHECK(popoverHit(L, L.r[0].x + h + 1, L.r[0].y + 2 * h + 1) == PopItem::None);
        KT_CHECK(popoverHit(L, L.r[0].x - 1, L.r[0].y) == PopItem::None);
        KT_CHECK(popoverHit(L, L.r[0].x + L.r[0].w, L.r[0].y) == PopItem::None);
        KT_CHECK(popoverHit(L, L.r[0].x, L.r[0].y - 1) == PopItem::None);
        KT_CHECK(popoverHit(L, L.r[0].x, L.r[0].y + 3 * h) == PopItem::None);
        KT_CHECK(popoverHit(L, L.r[2].x + h, L.r[2].y) == PopItem::None);
        KT_CHECK(popoverHit(L, L.r[3].x - 1, L.r[3].y) == PopItem::None);
    }
    // ---- applyPop / labels ----
    { CompCfg c; c = applyPop(c, PopItem::AutoToggle); KT_CHECK(c.autoOn); c = applyPop(c, PopItem::AutoToggle); KT_CHECK(!c.autoOn); }
    { CompCfg c; c = applyPop(c, PopItem::HpPlus); KT_CHECK_EQ(c.hpPct, 40); c = applyPop(c, PopItem::HpMinus); c = applyPop(c, PopItem::HpMinus); KT_CHECK_EQ(c.hpPct, 30);
      KT_CHECK_EQ(c.keep, 2); KT_CHECK(!c.autoOn); }
    { CompCfg c; c = applyPop(c, PopItem::KeepPlus); KT_CHECK_EQ(c.keep, 3); c = applyPop(c, PopItem::KeepMinus); c = applyPop(c, PopItem::KeepMinus); KT_CHECK_EQ(c.keep, 1);
      KT_CHECK_EQ(c.hpPct, 35); }
    { CompCfg c; c.hpPct = 95; c = applyPop(c, PopItem::HpPlus); KT_CHECK_EQ(c.hpPct, 95); c.hpPct = 5; c = applyPop(c, PopItem::HpMinus); KT_CHECK_EQ(c.hpPct, 5);
      c.keep = 20; c = applyPop(c, PopItem::KeepPlus); KT_CHECK_EQ(c.keep, 20); c.keep = 0; c = applyPop(c, PopItem::KeepMinus); KT_CHECK_EQ(c.keep, 0); }
    { CompCfg c; c.autoOn = true; KT_CHECK(applyPop(c, PopItem::None) == c); }
    { CompCfg c; KT_CHECK(popLabel(PopItem::AutoToggle, c) == "Auto: OFF"); c.autoOn = true; KT_CHECK(popLabel(PopItem::AutoToggle, c) == "Auto: ON");
      KT_CHECK(popLabel(PopItem::HpMinus, c) == "-"); KT_CHECK(popLabel(PopItem::KeepMinus, c) == "-");
      KT_CHECK(popLabel(PopItem::HpPlus, c) == "+"); KT_CHECK(popLabel(PopItem::KeepPlus, c) == "+"); KT_CHECK(popLabel(PopItem::None, c) == ""); }
    { CompCfg c; KT_CHECK(popValueLabel(2, c) == "Use below 35%"); KT_CHECK(popValueLabel(3, c) == "Keep 2"); KT_CHECK(popValueLabel(1, c) == ""); KT_CHECK(popValueLabel(4, c) == ""); }

    // ---- popover: text-fit unit, width-parameterised layout, row hit, auto caption ----
    KT_CHECK_EQ(popUnitForText(0, 20, 16), 30); KT_CHECK_EQ(popUnitForText(0, 5, 16), 16);   // ceil(1.5*20)=30; ceil(7.5)=8 < 16
    KT_CHECK_EQ(popUnitForText(100, 20, 16), 30);                                             // maxLabelPx does not change the result
    KT_CHECK_EQ(popUnitForText(0, 14, 16), 21); KT_CHECK_EQ(popUnitForText(0, 11, 16), 17); KT_CHECK_EQ(popUnitForText(0, 10.5f, 16), 16);   // ceil(16.5)=17, ceil(15.75)=16
    KT_CHECK_EQ(popUnitForText(0, 20, 40), 40); KT_CHECK_EQ(popUnitForText(0, 0, 0), 0);
    { PopLayout L = popoverLayoutW(Rect{400, 50, 58, 58}, 1920, 24, 200);   // gap 24/4 = 6, x = 400 - 6 - 200 = 194
      KT_CHECK(L.r[0] == (Rect{194, 50, 200, 72})); KT_CHECK(L.r[1] == (Rect{194, 50, 200, 24}));
      KT_CHECK(L.r[2] == (Rect{194, 74, 24, 24})); KT_CHECK(L.r[3] == (Rect{370, 74, 24, 24}));
      KT_CHECK(L.r[4] == (Rect{194, 98, 24, 24})); KT_CHECK(L.r[5] == (Rect{370, 98, 24, 24}));
      KT_CHECK(popValueRect(L, 2) == (Rect{218, 74, 152, 24})); KT_CHECK(popValueRect(L, 3) == (Rect{218, 98, 152, 24}));   // span = 200 - 2*24
      PopLayout a0 = popoverLayoutW(Rect{400, 50, 24, 24}, 1920, 0, 200), a1 = popoverLayoutW(Rect{400, 50, 24, 24}, 1920, 24, 200);
      for (int i = 0; i < 6; ++i) KT_CHECK(a0.r[i] == a1.r[i]); }                              // unit 0 = b.h
    { PopLayout L = popoverLayoutW(Rect{300, 5, 58, 58}, 250, 24, 200); KT_CHECK_EQ(L.r[0].x, 50); KT_CHECK_EQ(L.r[0].x + L.r[0].w, 250); KT_CHECK_EQ(L.r[3].x, 226); }   // right clamp: 94 + 200 > 250 -> 50
    { PopLayout L = popoverLayoutW(Rect{50, 5, 58, 58}, 1920, 24, 200); KT_CHECK_EQ(L.r[0].x, 0); KT_CHECK_EQ(L.r[3].x, 176); }   // left clamp: 50-6-200 < 0 -> 0
    { PopLayout L = popoverLayoutW(Rect{300, 5, 58, 58}, 100, 24, 200); KT_CHECK_EQ(L.r[0].x, 0); }   // left wins: 100 - 200 < 0 -> 0
    { PopLayout L = popoverLayoutW(Rect{400, 50, 58, 58}, 1920, 24, 200);
      KT_CHECK_EQ((int)popoverRowAt(L, 200, 55), (int)Row_Auto);      // r[1] Auto row
      KT_CHECK_EQ((int)popoverRowAt(L, 206, 86), (int)Row_HpM);       // r[2] HP minus
      KT_CHECK_EQ((int)popoverRowAt(L, 382, 86), (int)Row_HpP);       // r[3] HP plus
      KT_CHECK_EQ((int)popoverRowAt(L, 206, 110), (int)Row_KeepM);    // r[4] Keep minus
      KT_CHECK_EQ((int)popoverRowAt(L, 382, 110), (int)Row_KeepP);    // r[5] Keep plus
      KT_CHECK_EQ((int)popoverRowAt(L, 300, 86), (int)Row_None);      // HP value span
      KT_CHECK_EQ((int)popoverRowAt(L, 300, 110), (int)Row_None);     // Keep value span
      KT_CHECK_EQ((int)popoverRowAt(L, 193, 60), (int)Row_None);      // left of panel
      KT_CHECK_EQ((int)popoverRowAt(L, 394, 60), (int)Row_None);      // right of panel
      KT_CHECK_EQ((int)popoverRowAt(L, 300, 49), (int)Row_None);      // above panel
      KT_CHECK_EQ((int)popoverRowAt(L, 300, 122), (int)Row_None);     // below panel
      KT_CHECK_EQ((int)popoverRowAt(L, 393, 73), (int)Row_Auto);      // last pixel inside r[1]
      KT_CHECK_EQ((int)popoverRowAt(L, 393, 121), (int)Row_KeepP);    // last pixel inside r[5]
      for (int x = 190; x <= 398; ++x) for (int y = 46; y <= 126; ++y)
          KT_CHECK_EQ((int)popoverRowAt(L, x, y), (int)popoverHit(L, x, y));   // PopItem n == Row n, so same mapping everywhere
    }
    KT_CHECK_EQ((int)Row_None, 0); KT_CHECK_EQ((int)Row_KeepP, 5);
    KT_CHECK(std::string(popAutoCaption()) == "Auto-heal");

    // ---- file model ----
    {
        const std::string orig =
            "; header comment\r\n[General]\r\nlog=1\r\n\r\n[defaults]\r\n; old\r\ndefault=auto:1,hp:50,keep:3\r\n\r\n[probe]\r\n# keep me\r\ncall=1\r\n"
            "[companions]\r\n  bastila = auto:1,hp:60,keep:1 \r\nt3=auto:0\r\nbastila=auto:0,hp:25,keep:4\r\nbad line\r\n;c=auto:1\r\n";
        CfgTable t = parseFile(orig);
        KT_CHECK(t.def == (CompCfg{true, 50, 3}));
        KT_CHECK_EQ((int)t.rows.size(), 2);
        KT_CHECK(t.rows[0].first == "bastila"); KT_CHECK(t.rows[0].second == (CompCfg{false, 25, 4}));   // later duplicate wins, in place
        KT_CHECK(t.rows[1].first == "t3");
        KT_CHECK(lookup(t, "t3") == CompCfg()); KT_CHECK(lookup(t, "zzz") == t.def); KT_CHECK(lookup(t, "bastila").keep == 4);
        upsert(t, "hk47", CompCfg{true, 10, 0}); upsert(t, "t3", CompCfg{true, 70, 5});
        KT_CHECK_EQ((int)t.rows.size(), 3); KT_CHECK(t.rows[1].second == (CompCfg{true, 70, 5}));
        std::string w = serializeFile(t, orig);
        KT_CHECK(parseFile(w) == t);
        KT_CHECK(w.find("; header comment\r\n[General]\r\nlog=1\r\n\r\n[defaults]\r\n") == 0);
        KT_CHECK(w.find("[probe]\r\n# keep me\r\ncall=1\r\n") != std::string::npos);
        KT_CHECK(w.find("; old") == std::string::npos);            // comment inside rewritten section dropped
        KT_CHECK(w.find("default=auto:1,hp:50,keep:3\r\n\r\n[probe]") != std::string::npos);   // blank separator kept
        KT_CHECK(w.find("\r\n") != std::string::npos);
        for (size_t i = 0; i < w.size(); ++i) if (w[i] == '\n') KT_CHECK(i > 0 && w[i - 1] == '\r');
        KT_CHECK(serializeFile(t, w) == w);                         // idempotent
    }
    {   // LF, no trailing newline, other-section comments, no companions/defaults sections
        const std::string orig = "[General]\n; note\nx=1\n[probe]\ncall=0";
        CfgTable t; t.def = CompCfg{true, 20, 1}; upsert(t, "a", CompCfg{false, 90, 9});
        std::string w = serializeFile(t, orig);
        KT_CHECK(w == "[General]\n; note\nx=1\n[probe]\ncall=0\n\n[defaults]\ndefault=auto:1,hp:20,keep:1\n\n[companions]\na=auto:0,hp:90,keep:9\n");
        KT_CHECK(parseFile(w) == t);
    }
    {   // empty original; only defaults missing; only companions missing
        CfgTable t; t.def = CompCfg{true, 5, 0};
        std::string w = serializeFile(t, "");
        KT_CHECK(w == "[defaults]\ndefault=auto:1,hp:5,keep:0\n\n[companions]\n"); KT_CHECK(parseFile(w) == t);
        std::string w2 = serializeFile(t, "[companions]\nq=auto:1\n");
        KT_CHECK(parseFile(w2) == t);
        std::string w3 = serializeFile(t, "[defaults]\ndefault=auto:0\n");
        KT_CHECK(w3 == "[defaults]\ndefault=auto:1,hp:5,keep:0\n\n[companions]\n"); KT_CHECK(parseFile(w3) == t);
    }
    {   // duplicate sections merge; other keys in [defaults] ignored; stray lines before any section ignored
        const std::string orig = "default=auto:1\n[defaults]\ndefault=hp:40\n[companions]\na=hp:10\n[defaults]\ndefault=keep:7\nfoo=hp:99\n[companions]\nb=hp:20\n[ companions ]\nc=hp:30\n";
        CfgTable t = parseFile(orig);
        KT_CHECK_EQ(t.def.keep, 7); KT_CHECK_EQ(t.def.hpPct, 35);
        KT_CHECK_EQ((int)t.rows.size(), 3);
        std::string w = serializeFile(t, orig);
        KT_CHECK(parseFile(w) == t);
        size_t first = w.find("[defaults]"); KT_CHECK(first != std::string::npos); KT_CHECK(w.find("[defaults]", first + 1) == std::string::npos);
        KT_CHECK(w.find("[companions]") != std::string::npos); KT_CHECK(w.find("[companions]", w.find("[companions]") + 1) == std::string::npos);
        KT_CHECK(w.find("default=auto:1\n[defaults]") == 0);        // pre-section stray line preserved verbatim
    }
    {   // adjacent headers, blank-only body, "[]" is a real (unnamed) section header
        CfgTable t; t.def = CompCfg{true, 5, 0}; upsert(t, "q", CompCfg{false, 10, 1});
        KT_CHECK(serializeFile(t, "[defaults]\n[companions]\n") == "[defaults]\ndefault=auto:1,hp:5,keep:0\n[companions]\nq=auto:0,hp:10,keep:1\n");
        KT_CHECK(serializeFile(t, "[defaults]\n\n[probe]\nx=1\n") == "[defaults]\ndefault=auto:1,hp:5,keep:0\n\n[probe]\nx=1\n\n[companions]\nq=auto:0,hp:10,keep:1\n");
        KT_CHECK(parseFile("[companions]\n[]\nq=auto:1\n").rows.empty());
        KT_CHECK(parseFile("[companions]\n[\nq=auto:1\n").rows.size() == 1);
        KT_CHECK(parseFile("[companions]\n]\nq=auto:1\n").rows.size() == 1);
    }
    {   // lookup/upsert on empty table
        CfgTable t; KT_CHECK(lookup(t, "x") == CompCfg()); upsert(t, "x", CompCfg{true, 5, 5}); KT_CHECK(lookup(t, "x").hpPct == 5);
        CfgTable u = t; KT_CHECK(u == t); u.def.keep = 9; KT_CHECK(!(u == t)); u = t; u.rows[0].second.keep = 1; KT_CHECK(!(u == t));
    }

    // ---- pending ----
    KT_CHECK_EQ((int)kPendingTimeoutMs, 6000);
    { Pending p; p.on = true; p.since = 1000; p.preHp = 10; p.preStack = 3;
      KT_CHECK(!pendingDone(p, 1100, 10, 3));
      KT_CHECK(pendingDone(p, 1100, 11, 3));                     // hp up
      KT_CHECK(!pendingDone(p, 1100, 9, 3));                     // hp down alone
      KT_CHECK(pendingDone(p, 1100, 10, 2));                     // stack down
      KT_CHECK(pendingDone(p, 1100, 10, 0));                     // stack to zero
      KT_CHECK(!pendingDone(p, 1100, 10, 4));                    // stack up
      KT_CHECK(pendingDone(p, 1100, 10, -1));                    // item gone
      KT_CHECK(!pendingDone(p, 6999, 10, 3)); KT_CHECK(pendingDone(p, 7000, 10, 3)); KT_CHECK(pendingDone(p, 99999, 10, 3));
      KT_CHECK(!pendingTimedOut(p, 6999)); KT_CHECK(pendingTimedOut(p, 7000)); }
    { Pending p; p.preStack = 1; KT_CHECK(pendingDone(p, 0, 0, -1)); KT_CHECK(!pendingDone(p, 0, 0, 1)); }
    { Pending p; p.preStack = 0; KT_CHECK(!pendingDone(p, 0, 0, -1)); KT_CHECK(!pendingDone(p, 0, 0, 0)); }   // no stack known: gone != consumed
    { Pending p; p.since = 0xFFFFFF00u; p.preHp = 5; p.preStack = 1;
      KT_CHECK(!pendingDone(p, 0xFFFFFF10u, 5, 1)); KT_CHECK(!pendingDone(p, 100, 5, 1)); KT_CHECK(!pendingDone(p, 0x00001000u, 5, 1)); KT_CHECK(pendingDone(p, 0x00001800u, 5, 1));
      KT_CHECK(pendingDone(p, 0xFFFFFF00u + 6000u, 5, 1)); KT_CHECK(!pendingDone(p, 0xFFFFFF00u + 5999u, 5, 1)); }
    { Pending p; KT_CHECK(!p.on); KT_CHECK_EQ((int)p.since, 0); KT_CHECK_EQ(p.preHp, 0); KT_CHECK_EQ(p.preStack, 0); }
    // ---- lookFor / ringRect ----
    {
        struct C { BtnState st; bool au; float r, g, b, a; bool ring; } cs[] = {
            {BtnState::Normal, false, 1.0f, 1.0f, 1.0f, 1.0f, false}, {BtnState::Normal, true, 1.0f, 1.0f, 1.0f, 1.0f, true},
            {BtnState::AutoOn, false, 1.0f, 1.0f, 1.0f, 1.0f, false}, {BtnState::AutoOn, true, 1.0f, 1.0f, 1.0f, 1.0f, true},
            {BtnState::Pending, false, 1.0f, 1.0f, 1.0f, 0.65f, false}, {BtnState::Pending, true, 1.0f, 1.0f, 1.0f, 0.65f, true},
            {BtnState::FullHp, false, 0.55f, 0.55f, 0.55f, 0.40f, false}, {BtnState::FullHp, true, 0.55f, 0.55f, 0.55f, 0.40f, true},
            {BtnState::NoItem, false, 0.55f, 0.55f, 0.55f, 0.40f, false}, {BtnState::NoItem, true, 0.55f, 0.55f, 0.55f, 0.40f, true},
            {BtnState::Hidden, false, 1.0f, 1.0f, 1.0f, 0.0f, false}, {BtnState::Hidden, true, 1.0f, 1.0f, 1.0f, 0.0f, false},
            {BtnState::Down, false, 0.55f, 0.55f, 0.55f, 0.40f, false}, {BtnState::Down, true, 0.55f, 0.55f, 0.55f, 0.40f, false},   // no ring even with auto ON
        };
        for (const C& c : cs) {
            Look l = lookFor(c.st, c.au, 0);
            KT_CHECK(l.r == c.r); KT_CHECK(l.g == c.g); KT_CHECK(l.b == c.b); KT_CHECK(l.alpha == c.a); KT_CHECK(l.ring == c.ring);
        }
    }
    for (uint32_t t : {0u, 90u, 450u, 12345u}) KT_CHECK_EQ(lookFor(BtnState::Down, true, t).ringFrame, 0);   // Down: no animation
    KT_CHECK(kRingR == 1.00f && kRingG == 0.80f && kRingB == 0.25f);   // ring stays gold
    KT_CHECK_EQ(kRingFrames, 8); KT_CHECK_EQ((int)kRingFrameMs, 90);
    KT_CHECK_EQ(lookFor(BtnState::Normal, true, 0).ringFrame, 0);
    KT_CHECK_EQ(lookFor(BtnState::Normal, true, 89).ringFrame, 0);
    KT_CHECK_EQ(lookFor(BtnState::Normal, true, 90).ringFrame, 1);
    KT_CHECK_EQ(lookFor(BtnState::Normal, true, 629).ringFrame, 6);
    KT_CHECK_EQ(lookFor(BtnState::Normal, true, 630).ringFrame, 7);
    KT_CHECK_EQ(lookFor(BtnState::Normal, true, 719).ringFrame, 7);
    KT_CHECK_EQ(lookFor(BtnState::Normal, true, 720).ringFrame, 0);   // wrap at 8
    KT_CHECK_EQ(lookFor(BtnState::Normal, true, 810).ringFrame, 1);
    KT_CHECK_EQ(lookFor(BtnState::Normal, false, 90).ringFrame, 1);   // computed even when ring off
    KT_CHECK_EQ(lookFor(BtnState::Hidden, true, 180).ringFrame, 2);
    KT_CHECK_EQ(lookFor(BtnState::Normal, true, 0xFFFFFFFFu).ringFrame, 2);   // 4294967295/90 = 47721858 -> %8 = 2
    KT_CHECK_EQ(lookFor(BtnState::Normal, true, 0xFFFFFFFFu + 1u).ringFrame, 0);   // uint32 wraps to 0
    KT_CHECK_EQ(lookFor(BtnState::AutoOn, true, 0xFFFFFFFFu - 89u).ringFrame, 1);
    { Rect z{0, 0, 0, 0}; KT_CHECK(ringRect({10, 10, 16, 0}) == z); KT_CHECK(ringRect({10, 10, 16, -4}) == z); }
    KT_CHECK(ringRect({100, 50, 16, 16}) == (Rect{96, 46, 24, 24}));
    KT_CHECK(ringRect({100, 50, 24, 24}) == (Rect{94, 44, 36, 36}));
    KT_CHECK(ringRect({100, 50, 20, 20}) == (Rect{95, 45, 30, 30}));
    KT_CHECK(ringRect({100, 50, 17, 17}) == (Rect{96, 46, 26, 26}));   // 25.5 -> 26
    KT_CHECK(ringRect({100, 50, 1, 1}) == (Rect{100, 50, 2, 2}));     // 1.5 -> 2; (1-2)/2 truncates to 0
    KT_CHECK_EQ(ringRect({0, 0, 3, 3}).w, 4);    // 4.5 -> 4 (not 6)
    KT_CHECK_EQ(ringRect({0, 0, 7, 7}).w, 10);   // 10.5 -> 10 (not 12)
    // ---- leaderButtonRect(card, vit, diameter): disc left of the leader vit bar (starts 80 px left of the card), kLeaderClear left of the shield bar, top aligned with card.y ----
    { Rect card{3591, 1394, 191, 192}, vit{3511, 1386, 103, 208}; Rect r = leaderButtonRect(card, vit, 58);   // live leader (MAP member=0)
      KT_CHECK(r == (Rect{3445, 1403, 39, 39}));
      Rect rg = ringRect(r); KT_CHECK_EQ(rg.w, 58); KT_CHECK_EQ(rg.y, card.y);
      KT_CHECK_EQ(rg.x + rg.w + 11 + kLeaderClear, vit.x);                       // gap lround(2*192/36)=11, then the clear margin
      KT_CHECK(rg.x + rg.w < vit.x);                                              // never overlaps the shield/vitality bars
      KT_CHECK_EQ(leaderStripReserve(card, vit, 58), 80 + 11 + kLeaderClear + 58 + kReserveGap);
      KT_CHECK(leaderButtonRect(card, vit) == r);                                 // diameter 0 = default 58
      KT_CHECK_EQ(ringRect(leaderButtonRect(card, vit, 1000)).w, 72); KT_CHECK_EQ(ringRect(leaderButtonRect(card, vit, 10)).w, 48); }   // diameter clamped
    { Rect card{100, 500, 106, 106}; Rect r = leaderButtonRect(card, Rect{0, 0, 0, 0}, 58);   // no/invalid vit: anchor = card.x
      KT_CHECK_EQ(ringRect(r).x + 58 + 6 + kLeaderClear, 100);
      KT_CHECK_EQ(leaderStripReserve(card, Rect{0, 0, 0, 0}, 58), 6 + kLeaderClear + 58 + kReserveGap);
      Rect r2 = leaderButtonRect(card, Rect{150, 500, 80, 100}, 58); KT_CHECK(r2 == r); }   // vit right of the card: anchor = card.x
    { Rect z{0, 0, 0, 0}; Rect nv{0, 0, 0, 0}; KT_CHECK(leaderButtonRect({20, 500, 106, 106}, nv) == z); KT_CHECK(leaderButtonRect({0, 500, 106, 106}, nv) == z);   // would fall off the left edge
      KT_CHECK(leaderButtonRect({100, 500, 0, 106}, nv) == z); KT_CHECK(leaderButtonRect({100, 500, 106, 0}, nv) == z);
      KT_CHECK(!(leaderButtonRect({70, 500, 106, 106}, nv) == z)); KT_CHECK(leaderButtonRect({69, 500, 106, 106}, nv) == z); }   // ring left edge 70-6-6-58 = 0 ok, -1 refused

    // ---- stripReserve: gap (ceil) + disc diameter + kReserveGap; covers the real disc at every bar height (wastes at most 1 gap px) ----
    KT_CHECK_EQ(stripReserve({3671, 1285, 47, 96}), 66);     // gap 6 + disc 58 + kReserveGap 2
    KT_CHECK_EQ(stripReserve({0, 0, 47, 8}), 2 + 58 + kReserveGap);   // gap floor 2, disc fixed 58
    for (int h = 8; h <= 400; ++h) {
        const Rect bt = buttonRect({1000, 100, 47, h});
        if (bt.w <= 0) continue;
        const Rect rg = ringRect(bt);
        const int need = 1000 - (bt.x < rg.x ? bt.x : rg.x);
        const int rs = stripReserve({1000, 100, 47, h});
        KT_CHECK(rs - kReserveGap >= need - 1 && rs - kReserveGap <= need + 3);
    }
    // popover keeps its pre-disc size: popUnit(btn) = 24 for the live 39 px button; unit overrides b.h
    KT_CHECK_EQ(popUnit(Rect{0, 0, 39, 39}), 24); KT_CHECK_EQ(popUnit(Rect{0, 0, 10, 10}), 16); KT_CHECK_EQ(popUnit(Rect{0, 0, 90, 90}), 24);
    { PopLayout L = popoverLayout(Rect{400, 50, 58, 58}, 1920, 24); KT_CHECK_EQ(L.r[0].w, 192); KT_CHECK_EQ(L.r[0].h, 72); KT_CHECK_EQ(L.r[0].x + L.r[0].w + 6, 400);
      KT_CHECK(L.r[0].y == 50); KT_CHECK(popoverLayout(Rect{400, 50, 58, 58}, 1920) .r[0].w == 8 * 58); }   // unit 0 = legacy b.h
    KT_CHECK(ringRect({100, 50, 18, 18}) == (Rect{95, 45, 28, 28}));   // 27 -> 28 (half rounds up)
    // pendingDone2 / useAllowed (CS12)
    {
        Pending p; p.on = true; p.since = 1000; p.preHp = 10; p.preStack = 3;
        uint32_t u = 0;
        KT_CHECK(pendingDone2(p, 1100, 11, 3, true, &u));          // hp up wins even while queued
        KT_CHECK(pendingDone2(p, 1100, 10, 2, true, &u));          // stack down
        KT_CHECK(pendingDone2(p, 1100, 10, -1, false, &u));        // item gone
        u = 0; KT_CHECK(!pendingDone2(p, 1100, 10, 3, true, &u));
        KT_CHECK_EQ(u, 0u);
        u = 0; KT_CHECK(!pendingDone2(p, 900000, 10, 3, true, &u));   // queued forever never times out
        KT_CHECK(!pendingDone2(p, 0xFFFFFF00u, 10, 3, true, &u));
        u = 0;
        KT_CHECK(!pendingDone2(p, 10000, 10, 3, false, &u));       // stretch starts at 10000
        KT_CHECK_EQ(u, 10000u);
        KT_CHECK(!pendingDone2(p, 15999, 10, 3, false, &u));       // 5999 ms
        KT_CHECK(pendingDone2(p, 16000, 10, 3, false, &u));        // 6000 ms
        u = 0; pendingDone2(p, 10000, 10, 3, false, &u);
        KT_CHECK(!pendingDone2(p, 14000, 10, 3, true, &u));        // requeued: timer resets
        KT_CHECK_EQ(u, 0u);
        KT_CHECK(!pendingDone2(p, 15000, 10, 3, false, &u));       // new stretch from 15000
        KT_CHECK(!pendingDone2(p, 20999, 10, 3, false, &u));
        KT_CHECK(pendingDone2(p, 21000, 10, 3, false, &u));
        KT_CHECK(pendingDone2(p, 7000, 10, 3, false, nullptr));    // no tracker: falls back to the old since-based timeout
        KT_CHECK(!pendingDone2(p, 6999, 10, 3, false, nullptr));
        // tick 0 is the sentinel: arming at now==0 must not time out at once (SH-3b-05 / BG41)
        u = 0; KT_CHECK(!pendingDone2(p, 0, 10, 3, false, &u));
        KT_CHECK_EQ(u, 1u);
        KT_CHECK(!pendingDone2(p, 0, 10, 3, false, &u));          // same tick again: 0 ms elapsed
        KT_CHECK(!pendingDone2(p, 6000, 10, 3, false, &u));       // tick 0 read as 1: 5999 ms
        KT_CHECK(pendingDone2(p, 6001, 10, 3, false, &u));        // 6000 ms
        // wrap: stretch armed just below 0xFFFFFFFF, clock wraps past 0 (unsigned delta)
        u = 0; KT_CHECK(!pendingDone2(p, 0xFFFFFFF0u, 10, 3, false, &u));
        KT_CHECK_EQ(u, 0xFFFFFFF0u);
        KT_CHECK(!pendingDone2(p, 0x00000F00u, 10, 3, false, &u));   // 3856 ms across the wrap
        KT_CHECK(!pendingDone2(p, 0x0000175Fu, 10, 3, false, &u));   // 5999 ms
        KT_CHECK(pendingDone2(p, 0x00001760u, 10, 3, false, &u));    // 6000 ms
        KT_CHECK(useAllowed(false) == UseRefusal::None);
        KT_CHECK(useAllowed(true) == UseRefusal::AlreadyQueued);
        KT_CHECK(std::string(useRefusalText(useAllowed(true))) == "already queued");
        KT_CHECK(std::string(useRefusalText(UseRefusal::None)).empty());
    }
    {
        // WI-75 (BG28): leader settings follow the leader's tag; slot0 is only the untagged fallback.
        KT_CHECK(shui::leaderKey("Carth") == "Carth");
        KT_CHECK(shui::leaderKey("player") == "player");
        KT_CHECK(shui::leaderKey("") == "slot0");                          // empty tag -> fallback

        shui::CfgTable t;
        shui::CompCfg s0; s0.hpPct = 30;                                   // pre-fix leader row (slot0)
        shui::upsert(t, "slot0", s0);
        KT_CHECK(shui::migrateLeaderRow(t, "player"));                     // frame 1: copy slot0 once
        KT_CHECK(shui::lookup(t, shui::leaderKey("player")).hpPct == 30);
        KT_CHECK(!shui::migrateLeaderRow(t, "player"));                    // repeat frame: no second copy
        shui::CompCfg pA; pA.hpPct = 70;
        shui::upsert(t, "player", pA);                                     // player edits own row
        KT_CHECK(!shui::migrateLeaderRow(t, "player"));                    // an existing tag row is never overwritten
        KT_CHECK(shui::lookup(t, "player").hpPct == 70);
        KT_CHECK(shui::migrateLeaderRow(t, "Carth"));                      // leader swap to Carth
        KT_CHECK(shui::lookup(t, shui::leaderKey("Carth")).hpPct == 30);   // not player's 70
        shui::CompCfg cC; cC.hpPct = 50;
        shui::upsert(t, "Carth", cC);                                      // Carth edits own row
        KT_CHECK(shui::lookup(t, shui::leaderKey("player")).hpPct == 70);  // swap back: player's row
        KT_CHECK(shui::lookup(t, shui::leaderKey("Carth")).hpPct == 50);
        KT_CHECK(!shui::migrateLeaderRow(t, ""));                          // empty tag: no migration
        KT_CHECK(!shui::migrateLeaderRow(t, "slot0"));
        KT_CHECK(shui::lookup(t, shui::leaderKey("")).hpPct == 30);        // slot0 fallback row kept
        KT_CHECK(t.rows.size() == 3);                                      // slot0, player, Carth: no duplicates
        shui::CfgTable e;                                                  // no slot0 row: nothing to copy
        KT_CHECK(!shui::migrateLeaderRow(e, "Carth"));
        KT_CHECK(e.rows.empty());
    }
    {   // BG36 / CO-2d-04: slot owner. Unreadable keeps, first sight adopts, same keeps, a different creature resets.
        KT_CHECK(ownerStep(0x1000, 0) == OwnerStep::Keep);
        KT_CHECK(ownerStep(0, 0) == OwnerStep::Keep);
        KT_CHECK(ownerStep(0, 0x1000) == OwnerStep::Adopt);
        KT_CHECK(ownerStep(0x1000, 0x1000) == OwnerStep::Keep);
        KT_CHECK(ownerStep(0x1000, 0x2000) == OwnerStep::Reset);
        // frame sequence: A armed, unreadable frame, A again, swap to B, B again, swap back to A -> exactly two resets, owner follows
        uintptr_t owner = 0;
        int resets = 0, adopts = 0;
        const uintptr_t frames[] = {0x1000, 0, 0x1000, 0x2000, 0x2000, 0x1000};
        for (uintptr_t cur : frames) {
            const OwnerStep s = ownerStep(owner, cur);
            if (s == OwnerStep::Reset) ++resets;
            if (s == OwnerStep::Adopt) ++adopts;
            if (s != OwnerStep::Keep) owner = cur;
        }
        KT_CHECK(adopts == 1);
        KT_CHECK(resets == 2);
        KT_CHECK(owner == 0x1000);
    }
    {   // FU-texinit / SH-3a-05: per-slot texture policy. A null slot is asked again on every build; a non-null handle is kept, never looked up again.
        int asks = 0;
        void* ready = nullptr;   // fake engine: null until the texture exists
        auto lookup = [&]() -> void* { ++asks; return ready; };
        void* cross = shui::resolveTexSlot(nullptr, lookup);   // build 1 (ensure ok, texture not ready yet)
        KT_CHECK(cross == nullptr);
        KT_CHECK(asks == 1);
        cross = shui::resolveTexSlot(cross, lookup);           // build 2: still absent, asked again
        KT_CHECK(cross == nullptr);
        KT_CHECK(asks == 2);
        ready = reinterpret_cast<void*>(uintptr_t{0x2000});
        cross = shui::resolveTexSlot(cross, lookup);           // build 3 (g_texInit already 1): the null is re-queried and found
        KT_CHECK(cross == reinterpret_cast<void*>(uintptr_t{0x2000}));
        KT_CHECK(asks == 3);
        ready = reinterpret_cast<void*>(uintptr_t{0x3000});
        cross = shui::resolveTexSlot(cross, lookup);           // build 4: non-null kept, the engine is not asked again (pool leak)
        cross = shui::resolveTexSlot(cross, lookup);
        KT_CHECK(cross == reinterpret_cast<void*>(uintptr_t{0x2000}));
        KT_CHECK(asks == 3);
    }
    {   // FU-texinit: a miss stays nullptr (no sentinel stored), so the next build still sees a null slot and asks again.
        int asks = 0;
        void* s = shui::resolveTexSlot(nullptr, [&]() -> void* { ++asks; return nullptr; });
        KT_CHECK(s == nullptr);
        KT_CHECK(asks == 1);
        s = shui::resolveTexSlot(s, [&]() -> void* { ++asks; return nullptr; });
        KT_CHECK(s == nullptr);
        KT_CHECK(asks == 2);
    }
    {   // MCF4: erase / eraseAll / hasRow: removal = "Follow global" (lookup falls back to def) and persists through serializeFile.
        CfgTable t;
        t.def = CompCfg{true, 50, 3};
        upsert(t, "atton", CompCfg{false, 20, 1});
        upsert(t, "kreia", CompCfg{true, 80, 0});
        upsert(t, "player", CompCfg{true, 10, 9});
        KT_CHECK(hasRow(t, "atton") && !hasRow(t, "t3m4"));
        KT_CHECK(erase(t, "kreia"));
        KT_CHECK(!hasRow(t, "kreia") && t.rows.size() == 2);
        KT_CHECK(lookup(t, "kreia") == t.def);                           // follows the defaults again
        KT_CHECK(!erase(t, "kreia"));                                    // already gone: false, no change
        KT_CHECK(!erase(t, ""));
        KT_CHECK(t.rows.size() == 2 && t.rows[0].first == "atton" && t.rows[1].first == "player");   // order of the rest is kept
        const std::string orig = "[defaults]\ndefault=auto:0,hp:35,keep:2\n\n[companions]\natton=auto:0,hp:20,keep:1\nkreia=auto:1,hp:80,keep:0\nplayer=auto:1,hp:10,keep:9\n[other]\nx=1\n";
        const std::string w = serializeFile(t, orig);
        KT_CHECK(w.find("kreia") == std::string::npos);                  // the removed row is gone from the file
        KT_CHECK(w.find("[other]\nx=1\n") != std::string::npos);         // foreign sections untouched
        KT_CHECK(parseFile(w) == t);                                     // round trip
        KT_CHECK_EQ((int)eraseAll(t), 2);
        KT_CHECK(t.rows.empty() && t.def == (CompCfg{true, 50, 3}));     // def kept
        KT_CHECK_EQ((int)eraseAll(t), 0);
        const std::string w2 = serializeFile(t, w);
        KT_CHECK(w2.find("atton") == std::string::npos && w2.find("[companions]") != std::string::npos);
        KT_CHECK(parseFile(w2) == t);
        KT_CHECK(lookup(t, "atton") == t.def && lookup(t, "anyone") == t.def);
    }
    {   // MCF4: tags match case-insensitively; the first spelling stays.
        CfgTable t;
        upsert(t, "BaoDur", CompCfg{true, 40, 1});
        KT_CHECK(lookup(t, "baodur").hpPct == 40 && hasRow(t, "BAODUR"));
        upsert(t, "baodur", CompCfg{false, 60, 2});
        KT_CHECK(t.rows.size() == 1 && t.rows[0].first == "BaoDur" && t.rows[0].second.hpPct == 60);
        KT_CHECK(erase(t, "BAODUR") && t.rows.empty());
        t.def = CompCfg{true, 10, 0};
        KT_CHECK(lookup(t, "baodur") == t.def);
        CfgTable m;
        upsert(m, "slot0", CompCfg{true, 70, 4});
        KT_CHECK(migrateLeaderRow(m, "Player") && hasRow(m, "player") && m.rows.size() == 2);
        KT_CHECK(!migrateLeaderRow(m, "PLAYER"));
    }
    return kt::Report("selfheal_ui_test");
}
