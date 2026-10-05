// Host test of SelfHealUi.h: button geometry, state priority, steppers, INI codec.
#include "SelfHealUi.h"
#include "check.h"

using namespace shui;

static bool parse(const char* l, std::string& k, CompCfg& c) { return parseLine(l, k, c); }

int main() {
    // ---- buttonRect ----
    { Rect r = buttonRect({100, 50, 47, 36}); KT_CHECK(r == (Rect{100 - 2 - 16, 50 + (36 - 16) / 2, 16, 16})); }   // s=9->16, gap 2
    { Rect r = buttonRect({100, 50, 47, 64}); KT_CHECK_EQ(r.w, 16); KT_CHECK_EQ(r.h, 16); KT_CHECK_EQ(r.x, 100 - 4 - 16); KT_CHECK_EQ(r.y, 50 + 24); }   // gap round(3.56)=4
    { Rect r = buttonRect({100, 50, 47, 96}); KT_CHECK_EQ(r.w, 24); KT_CHECK_EQ(r.h, 24); KT_CHECK_EQ(r.x, 100 - 5 - 24); KT_CHECK_EQ(r.y, 50 + 36); }   // gap round(5.33)=5
    { Rect r = buttonRect({100, 0, 47, 200}); KT_CHECK_EQ(r.w, 24); KT_CHECK_EQ(r.x, 100 - 11 - 24); KT_CHECK_EQ(r.y, 88); }   // gap round(11.1)=11
    { Rect r = buttonRect({100, 0, 47, 80}); KT_CHECK_EQ(r.w, 20); KT_CHECK_EQ(r.x, 100 - 4 - 20); }   // s=20 unclamped, gap round(4.44)=4
    { Rect r = buttonRect({100, 0, 47, 82}); KT_CHECK_EQ(r.w, 21); }   // round(20.5)=21
    { Rect r = buttonRect({100, 0, 47, 81}); KT_CHECK_EQ(r.w, 20); }   // round(20.25)=20
    { Rect r = buttonRect({100, 0, 47, 400}); KT_CHECK_EQ(r.w, 24); KT_CHECK_EQ(r.x, 100 - 22 - 24); }   // h=400 allowed
    { Rect z{0, 0, 0, 0}; KT_CHECK(buttonRect({100, 50, 70, 36}) == z); KT_CHECK(buttonRect({100, 50, 103, 208}) == z); KT_CHECK(buttonRect({100, 50, 0, 36}) == z); KT_CHECK(!(buttonRect({100, 50, 69, 36}) == z)); }   // leader (w >= 70) / empty; 69 = small card
    { Rect r = buttonRect({3671, 1285, 47, 96}); KT_CHECK(r == (Rect{3642, 1321, 24, 24})); }   // live small card, 3840x1600 (doc 08 probe 1)
    { KT_CHECK(isLeaderVit({3511, 1386, 103, 208})); KT_CHECK(!isLeaderVit({3671, 1285, 47, 96})); }
    { Rect z{0, 0, 0, 0}; KT_CHECK(buttonRect({100, 50, 47, 0}) == z); KT_CHECK(buttonRect({100, 50, 47, -5}) == z); KT_CHECK(buttonRect({100, 50, 47, 401}) == z); }
    { Rect z{0, 0, 0, 0}; KT_CHECK(buttonRect({17, 50, 47, 36}) == z); KT_CHECK(!(buttonRect({18, 50, 47, 36}) == z)); }   // x = 18-2-16 = 0 ok; 17 -> -1
    { Rect r = buttonRect({18, 50, 47, 36}); KT_CHECK_EQ(r.x, 0); }
    { Rect r = buttonRect({100, 50, 47, 1}); KT_CHECK_EQ(r.w, 16); KT_CHECK_EQ(r.x, 100 - 2 - 16); }   // tiny h: gap floor 2
    { Rect r = buttonRect({100, 50, 47, 18}); KT_CHECK_EQ(r.x, 100 - 2 - 16); }   // raw gap round(1.0)=1 -> floored to 2
    { Rect r = buttonRect({100, 50, 47, 27}); KT_CHECK_EQ(r.x, 100 - 2 - 16); }   // raw gap 2 (round 1.5) -> 2

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
            {BtnState::Normal, false, 0.30f, 0.95f, 0.40f, 1.0f, false}, {BtnState::Normal, true, 1.00f, 0.80f, 0.25f, 1.0f, true},
            {BtnState::AutoOn, false, 1.00f, 0.80f, 0.25f, 1.0f, false}, {BtnState::AutoOn, true, 1.00f, 0.80f, 0.25f, 1.0f, true},
            {BtnState::Pending, false, 0.30f, 0.95f, 0.40f, 0.65f, false}, {BtnState::Pending, true, 1.00f, 0.80f, 0.25f, 0.65f, true},
            {BtnState::FullHp, false, 0.60f, 0.60f, 0.60f, 0.40f, false}, {BtnState::FullHp, true, 0.60f, 0.60f, 0.60f, 0.40f, true},
            {BtnState::NoItem, false, 0.60f, 0.60f, 0.60f, 0.40f, false}, {BtnState::NoItem, true, 0.60f, 0.60f, 0.60f, 0.40f, true},
            {BtnState::Hidden, false, 0.30f, 0.95f, 0.40f, 0.0f, false}, {BtnState::Hidden, true, 1.00f, 0.80f, 0.25f, 0.0f, false},
        };
        for (const C& c : cs) {
            Look l = lookFor(c.st, c.au, 0);
            KT_CHECK(l.r == c.r); KT_CHECK(l.g == c.g); KT_CHECK(l.b == c.b); KT_CHECK(l.alpha == c.a); KT_CHECK(l.ring == c.ring);
        }
    }
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
    KT_CHECK(ringRect({100, 50, 18, 18}) == (Rect{95, 45, 28, 28}));   // 27 -> 28 (half rounds up)
    return kt::Report("selfheal_ui_test");
}
