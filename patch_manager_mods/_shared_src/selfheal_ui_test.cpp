// Host test of SelfHealUi.h: button geometry, state priority, steppers, INI codec.
#include "SelfHealUi.h"
#include "check.h"

using namespace shui;

static bool parse(const char* l, std::string& k, CompCfg& c) { return parseLine(l, k, c); }

int main() {
    // ---- buttonRect ----
    { Rect r = buttonRect({100, 50, 200, 36}); KT_CHECK(r == (Rect{100 - 2 - 16, 50 + (36 - 16) / 2, 16, 16})); }   // s=9->16, gap 2
    { Rect r = buttonRect({100, 50, 200, 64}); KT_CHECK_EQ(r.w, 16); KT_CHECK_EQ(r.h, 16); KT_CHECK_EQ(r.x, 100 - 4 - 16); KT_CHECK_EQ(r.y, 50 + 24); }   // gap round(3.56)=4
    { Rect r = buttonRect({100, 50, 200, 96}); KT_CHECK_EQ(r.w, 24); KT_CHECK_EQ(r.h, 24); KT_CHECK_EQ(r.x, 100 - 5 - 24); KT_CHECK_EQ(r.y, 50 + 36); }   // gap round(5.33)=5
    { Rect r = buttonRect({100, 0, 200, 200}); KT_CHECK_EQ(r.w, 24); KT_CHECK_EQ(r.x, 100 - 11 - 24); KT_CHECK_EQ(r.y, 88); }   // gap round(11.1)=11
    { Rect r = buttonRect({100, 0, 70, 80}); KT_CHECK_EQ(r.w, 20); KT_CHECK_EQ(r.x, 100 - 4 - 20); }   // s=20 unclamped, gap round(4.44)=4
    { Rect r = buttonRect({100, 0, 70, 82}); KT_CHECK_EQ(r.w, 21); }   // round(20.5)=21
    { Rect r = buttonRect({100, 0, 70, 81}); KT_CHECK_EQ(r.w, 20); }   // round(20.25)=20
    { Rect r = buttonRect({100, 0, 70, 400}); KT_CHECK_EQ(r.w, 24); KT_CHECK_EQ(r.x, 100 - 22 - 24); }   // h=400 allowed
    { Rect z{0, 0, 0, 0}; KT_CHECK(buttonRect({100, 50, 69, 36}) == z); KT_CHECK(buttonRect({100, 50, 0, 36}) == z); }   // leader/narrow
    { Rect z{0, 0, 0, 0}; KT_CHECK(buttonRect({100, 50, 200, 0}) == z); KT_CHECK(buttonRect({100, 50, 200, -5}) == z); KT_CHECK(buttonRect({100, 50, 200, 401}) == z); }
    { Rect z{0, 0, 0, 0}; KT_CHECK(buttonRect({17, 50, 200, 36}) == z); KT_CHECK(!(buttonRect({18, 50, 200, 36}) == z)); }   // x = 18-2-16 = 0 ok; 17 -> -1
    { Rect r = buttonRect({18, 50, 200, 36}); KT_CHECK_EQ(r.x, 0); }
    { Rect r = buttonRect({100, 50, 200, 1}); KT_CHECK_EQ(r.w, 16); KT_CHECK_EQ(r.x, 100 - 2 - 16); }   // tiny h: gap floor 2
    { Rect r = buttonRect({100, 50, 200, 18}); KT_CHECK_EQ(r.x, 100 - 2 - 16); }   // raw gap round(1.0)=1 -> floored to 2
    { Rect r = buttonRect({100, 50, 200, 27}); KT_CHECK_EQ(r.x, 100 - 2 - 16); }   // raw gap 2 (round 1.5) -> 2

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

    // ---- click ----
    KT_CHECK(!clickAllowed(BtnState::Hidden)); KT_CHECK(!clickAllowed(BtnState::FullHp)); KT_CHECK(!clickAllowed(BtnState::Pending));
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
    return kt::Report("selfheal_ui_test");
}
