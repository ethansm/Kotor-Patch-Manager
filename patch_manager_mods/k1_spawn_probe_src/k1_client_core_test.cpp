// k1_client_core_test.cpp -- Host tests for ClientCore.h (LogBudget, EventScope, ScopedEvent, DumpGate, ScanBudget, RunStable).
#include "ClientCore.h"
#include "check.h"

#include <cstring>
#include <stdint.h>
#include <string>
#include <type_traits>
#include <vector>

namespace {

using namespace k1spawn;

static_assert(LogBudget::kTotal == 3000 && LogBudget::kAddToArea == 2500, "log budget constants");
static_assert(ScanBudget::kMaxScans == 64, "scan budget constant");
static_assert(!std::is_copy_constructible<ScopedEvent>::value, "ScopedEvent is not copyable");

constexpr int kOut = 64;

// Records everything RunStable and the nested handler do, in order.
struct FakeEnv : Env {
    std::vector<std::string> lines;    // every line() call, in order
    std::vector<std::string> trace;    // lines, flush() and spawn() calls, in order
    std::vector<std::string> present;  // tags that tagPresent() reports
    int opens = 0;
    int closes = 0;
    int flushes = 0;
    int spawnCalls = 0;
    uint32_t spawnRet = 0x7f000100u;  // 0 = refused
    EventScope* nestScope = nullptr;  // when set, the next spawn() runs a nested origin=LIB event on this scope

    void line(const char* text) override {
        lines.push_back(text);
        trace.push_back(text);
    }
    void openLog() override { ++opens; }
    void closeLog() override { ++closes; }
    void flush() override {
        ++flushes;
        trace.push_back("<flush>");
    }
    uint32_t spawn(const char* resref, const char*, float, float, float, float) override {
        ++spawnCalls;
        trace.push_back(std::string("<spawn ") + resref + ">");
        if (nestScope) {
            EventScope* s = nestScope;
            nestScope = nullptr;
            ScopedEvent nested(*s, *this);
            line("ADDTOAREA n=1 c=00000000 id=00000000 origin=LIB");
        }
        return spawnRet;
    }
    bool tagPresent(const char* tag) override {
        for (const std::string& t : present) {
            if (t == tag) return true;
        }
        return false;
    }
};

bool hasEntry(const std::vector<std::string>& v, const char* s) {
    for (const std::string& e : v) {
        if (e == s) return true;
    }
    return false;
}

void testEventScopeTransitions() {
    EventScope s;
    KT_CHECK(s.open());
    KT_CHECK(!s.open());
    KT_CHECK(!s.close());
    KT_CHECK(s.close());
    KT_CHECK(!s.close());  // at depth 0: nothing changes
    KT_CHECK_EQ(s.depth(), 0);
}

void testScopedEventOpensOnce() {
    FakeEnv env;
    EventScope s;
    {
        ScopedEvent outer(s, env);
        {
            ScopedEvent inner(s, env);
            KT_CHECK_EQ(s.depth(), 2);
        }
        KT_CHECK_EQ(env.closes, 0);  // inner end does not close the log
    }
    KT_CHECK_EQ(env.opens, 1);
    KT_CHECK_EQ(env.closes, 1);
    KT_CHECK_EQ(s.depth(), 0);
}

void testScopedEventNestedSpawn() {
    FakeEnv env;
    EventScope scope;
    LogBudget budget;
    env.nestScope = &scope;
    {
        ScopedEvent outer(scope, env);
        RunStable(env, "204tel", 1, 0xABCDu, true, budget);
        KT_CHECK_EQ(scope.depth(), 1);  // the nested event has ended, the outer one is still open
    }
    KT_CHECK_EQ(scope.depth(), 0);
    KT_CHECK_EQ(env.opens, 1);
    KT_CHECK_EQ(env.closes, 1);
    KT_CHECK_EQ(env.spawnCalls, 7);
    const std::vector<std::string> want = {
        "SPAWN stable area=204tel visit=1 salt=0000abcd rows=7 spawn=on",
        "SPAWN begin resref=c_drdmouse01 tag=MouseDroid area=204tel at -39.0,-63.1,0.0",
        "ADDTOAREA n=1 c=00000000 id=00000000 origin=LIB",
        "SPAWN ok resref=c_drdmouse01 id=7f000100",
        "SPAWN begin resref=k1p_juhani tag=k1p_juhani area=204tel at -37.5,-63.1,0.0",
        "SPAWN ok resref=k1p_juhani id=7f000100",
        "SPAWN begin resref=k1p_carth tag=k1p_carth area=204tel at -36.0,-63.1,0.0",
        "SPAWN ok resref=k1p_carth id=7f000100",
        "SPAWN begin resref=k1p_juhanv tag=k1p_juhanv area=204tel at -34.5,-63.1,0.0",
        "SPAWN ok resref=k1p_juhanv id=7f000100",
        "SPAWN begin resref=k1p_mission tag=k1p_mission area=204tel at -40.5,-63.1,0.0",
        "SPAWN ok resref=k1p_mission id=7f000100",
        "SPAWN begin resref=k1p_zaalbaru tag=k1p_zaalbar area=204tel at -42.5,-63.1,0.0",
        "SPAWN ok resref=k1p_zaalbaru id=7f000100",
        "SPAWN begin resref=k1p_bastilla tag=k1p_bastilla area=204tel at -44.5,-63.1,0.0",
        "SPAWN ok resref=k1p_bastilla id=7f000100",
    };
    KT_CHECK(env.lines == want);
}

void testDumpGate() {
    DumpGate g;
    KT_CHECK(!g.wantNpc(false));
    KT_CHECK(g.wantNpc(true));
    KT_CHECK(g.wantNpc(true));  // asking does not commit
    g.commitNpc();
    KT_CHECK(!g.wantNpc(true));
    KT_CHECK(!g.wantNpc(false));

    KT_CHECK(g.wantAny(0x10));
    KT_CHECK(g.wantAny(0x10));  // a refused dump leaves the flags unset
    g.commitAny(0x10);
    KT_CHECK(!g.wantAny(0x10));  // same id refused
    KT_CHECK(g.wantAny(0x20));   // distinct id allowed
    g.commitAny(0x20);
    KT_CHECK(g.wantAny(0x30));
    g.commitAny(0x30);
    KT_CHECK(!g.wantAny(0x40));  // three dumps taken: the 4th id is refused
    g.commitAny(0x40);           // and a refused commit changes nothing
    KT_CHECK(!g.wantAny(0x40));
    KT_CHECK(!g.wantAny(0x10));
}

void testDumpGateRepeatCommitKeepsSlots() {
    DumpGate g;
    g.commitAny(5);
    g.commitAny(5);  // a repeated id takes no second slot
    KT_CHECK(g.wantAny(6));
    g.commitAny(6);
    KT_CHECK(g.wantAny(7));
}

void testLogBudgetAddToAreaCap() {
    LogBudget b;
    KT_CHECK(b.fits(1));
    KT_CHECK(b.addToAreaAllowed());
    KT_CHECK(!b.capped());
    KT_CHECK_EQ(b.used(), 0);
    KT_CHECK_EQ(b.dropped(), 0L);
    for (int i = 0; i < LogBudget::kAddToArea; ++i) KT_CHECK(b.reserve(1, LogKind::AddToArea));
    KT_CHECK_EQ(b.used(), 2500);
    KT_CHECK(!b.addToAreaAllowed());
    KT_CHECK(!b.reserve(1, LogKind::AddToArea));
    KT_CHECK_EQ(b.dropped(), 1L);
    KT_CHECK_EQ(b.used(), 2500);  // the refused line is not charged
    KT_CHECK(b.reserve(1, LogKind::Other));  // Other still passes below the total cap
    KT_CHECK_EQ(b.used(), 2501);
    KT_CHECK(b.reserve(LogBudget::kTotal - b.used(), LogKind::Other));
    KT_CHECK(b.capped());
    KT_CHECK(!b.reserve(1, LogKind::Other));
    KT_CHECK(!b.reserve(1, LogKind::AddToArea));
    KT_CHECK(!b.fits(1));
    KT_CHECK(b.fits(0));
    KT_CHECK_EQ(b.dropped(), 3L);
    KT_CHECK_EQ(b.used(), 3000);
}

void testLogBudgetRefusalChargesNothing() {
    LogBudget b;
    KT_CHECK(b.reserve(2400, LogKind::AddToArea));
    KT_CHECK(!b.reserve(200, LogKind::AddToArea));  // 2600 > 2500: refused as a whole
    KT_CHECK_EQ(b.used(), 2400);
    KT_CHECK_EQ(b.dropped(), 200L);
    KT_CHECK(b.reserve(100, LogKind::Other));  // used 2500, AddToArea still at 2400
    KT_CHECK_EQ(b.used(), 2500);
    KT_CHECK(b.addToAreaAllowed());            // 100 AddToArea slots left
    KT_CHECK(b.reserve(100, LogKind::AddToArea));  // used 2600, AddToArea 2500
    KT_CHECK(!b.addToAreaAllowed());
    KT_CHECK(!b.reserve(2999, LogKind::Other));  // 2600 + 2999 > 3000
    KT_CHECK_EQ(b.dropped(), 3199L);
    KT_CHECK_EQ(b.used(), 2600);
}

void testLogBudgetCapMarker() {
    char out[kOut] = "untouched";
    LogBudget b;
    KT_CHECK(!b.takeCapMarker(7, out, kOut));  // not capped yet
    KT_CHECK(b.reserve(2500, LogKind::AddToArea));
    KT_CHECK(!b.takeCapMarker(8, out, kOut));  // the AddToArea cap is not the total cap
    KT_CHECK(b.reserve(500, LogKind::Other));
    KT_CHECK(b.capped());
    KT_CHECK(b.takeCapMarker(1234, out, kOut));
    KT_CHECK(std::strcmp(out, "CAP reached at n=1234") == 0);
    char again[kOut] = "untouched";
    KT_CHECK(!b.takeCapMarker(1235, again, kOut));  // once only
    KT_CHECK(std::strcmp(again, "untouched") == 0);
}

void testScanBudget() {
    ScanBudget s;
    KT_CHECK(!s.allow(false, true, false));  // library tag names something: no scan
    KT_CHECK(!s.allow(true, false, false));  // tag string names something: no scan
    KT_CHECK(!s.allow(false, false, false));
    KT_CHECK(!s.allow(true, true, true));    // log capped: no scan
    KT_CHECK_EQ(s.scans(), 0);               // refusals are not counted
    for (int i = 0; i < ScanBudget::kMaxScans; ++i) KT_CHECK(s.allow(true, true, false));
    KT_CHECK_EQ(s.scans(), 64);
    KT_CHECK(!s.allow(true, true, false));   // the 65th is refused
    KT_CHECK(!s.allow(true, true, true));
    KT_CHECK_EQ(s.scans(), 64);
}

void testRunStableDisabled() {
    FakeEnv env;
    LogBudget budget;
    RunStable(env, "204tel", 3, 0x1u, false, budget);
    const std::vector<std::string> want = {"SPAWN stable area=204tel visit=3 salt=00000001 rows=7 spawn=off"};
    KT_CHECK(env.lines == want);
    KT_CHECK_EQ(env.spawnCalls, 0);
    KT_CHECK_EQ(env.flushes, 0);
    KT_CHECK_EQ(budget.used(), 1);
}

void testRunStableFormatAndOrder() {
    FakeEnv env;
    LogBudget budget;
    RunStable(env, "204tel", 1, 0xABCDu, true, budget);
    const std::vector<std::string> want = {
        "SPAWN stable area=204tel visit=1 salt=0000abcd rows=7 spawn=on",
        "SPAWN begin resref=c_drdmouse01 tag=MouseDroid area=204tel at -39.0,-63.1,0.0",
        "<flush>",
        "<spawn c_drdmouse01>",
        "SPAWN ok resref=c_drdmouse01 id=7f000100",
        "SPAWN begin resref=k1p_juhani tag=k1p_juhani area=204tel at -37.5,-63.1,0.0",
        "<flush>",
        "<spawn k1p_juhani>",
        "SPAWN ok resref=k1p_juhani id=7f000100",
        "SPAWN begin resref=k1p_carth tag=k1p_carth area=204tel at -36.0,-63.1,0.0",
        "<flush>",
        "<spawn k1p_carth>",
        "SPAWN ok resref=k1p_carth id=7f000100",
        "SPAWN begin resref=k1p_juhanv tag=k1p_juhanv area=204tel at -34.5,-63.1,0.0",
        "<flush>",
        "<spawn k1p_juhanv>",
        "SPAWN ok resref=k1p_juhanv id=7f000100",
        "SPAWN begin resref=k1p_mission tag=k1p_mission area=204tel at -40.5,-63.1,0.0",
        "<flush>",
        "<spawn k1p_mission>",
        "SPAWN ok resref=k1p_mission id=7f000100",
        "SPAWN begin resref=k1p_zaalbaru tag=k1p_zaalbar area=204tel at -42.5,-63.1,0.0",
        "<flush>",
        "<spawn k1p_zaalbaru>",
        "SPAWN ok resref=k1p_zaalbaru id=7f000100",
        "SPAWN begin resref=k1p_bastilla tag=k1p_bastilla area=204tel at -44.5,-63.1,0.0",
        "<flush>",
        "<spawn k1p_bastilla>",
        "SPAWN ok resref=k1p_bastilla id=7f000100",
    };
    KT_CHECK(env.trace.size() == want.size());
    KT_CHECK(env.trace == want);
    KT_CHECK_EQ(budget.used(), 15);
}

void testRunStableTagPresentSkips() {
    FakeEnv env;
    env.present.push_back("MouseDroid");
    LogBudget budget;
    RunStable(env, "204tel", 2, 0x2u, true, budget);
    const std::vector<std::string> want = {
        "SPAWN stable area=204tel visit=2 salt=00000002 rows=7 spawn=on",
        "SPAWN skip resref=c_drdmouse01 tag=MouseDroid already present (library seen-tag list)",
        "SPAWN begin resref=k1p_juhani tag=k1p_juhani area=204tel at -37.5,-63.1,0.0",
        "SPAWN ok resref=k1p_juhani id=7f000100",
        "SPAWN begin resref=k1p_carth tag=k1p_carth area=204tel at -36.0,-63.1,0.0",
        "SPAWN ok resref=k1p_carth id=7f000100",
        "SPAWN begin resref=k1p_juhanv tag=k1p_juhanv area=204tel at -34.5,-63.1,0.0",
        "SPAWN ok resref=k1p_juhanv id=7f000100",
        "SPAWN begin resref=k1p_mission tag=k1p_mission area=204tel at -40.5,-63.1,0.0",
        "SPAWN ok resref=k1p_mission id=7f000100",
        "SPAWN begin resref=k1p_zaalbaru tag=k1p_zaalbar area=204tel at -42.5,-63.1,0.0",
        "SPAWN ok resref=k1p_zaalbaru id=7f000100",
        "SPAWN begin resref=k1p_bastilla tag=k1p_bastilla area=204tel at -44.5,-63.1,0.0",
        "SPAWN ok resref=k1p_bastilla id=7f000100",
    };
    KT_CHECK(env.lines == want);
    KT_CHECK_EQ(env.spawnCalls, 6);
    KT_CHECK(!hasEntry(env.trace, "<spawn c_drdmouse01>"));  // the present tag is never spawned
    KT_CHECK(hasEntry(env.trace, "<spawn k1p_juhani>"));
}

void testRunStableSpawnRefused() {
    FakeEnv env;
    env.spawnRet = 0;
    LogBudget budget;
    RunStable(env, "204tel", 1, 0u, true, budget);
    const std::vector<std::string> want = {
        "SPAWN stable area=204tel visit=1 salt=00000000 rows=7 spawn=on",
        "SPAWN begin resref=c_drdmouse01 tag=MouseDroid area=204tel at -39.0,-63.1,0.0",
        "SPAWN fail resref=c_drdmouse01 refused by npc-world (reason in npc-world_log.txt)",
        "SPAWN begin resref=k1p_juhani tag=k1p_juhani area=204tel at -37.5,-63.1,0.0",
        "SPAWN fail resref=k1p_juhani refused by npc-world (reason in npc-world_log.txt)",
        "SPAWN begin resref=k1p_carth tag=k1p_carth area=204tel at -36.0,-63.1,0.0",
        "SPAWN fail resref=k1p_carth refused by npc-world (reason in npc-world_log.txt)",
        "SPAWN begin resref=k1p_juhanv tag=k1p_juhanv area=204tel at -34.5,-63.1,0.0",
        "SPAWN fail resref=k1p_juhanv refused by npc-world (reason in npc-world_log.txt)",
        "SPAWN begin resref=k1p_mission tag=k1p_mission area=204tel at -40.5,-63.1,0.0",
        "SPAWN fail resref=k1p_mission refused by npc-world (reason in npc-world_log.txt)",
        "SPAWN begin resref=k1p_zaalbaru tag=k1p_zaalbar area=204tel at -42.5,-63.1,0.0",
        "SPAWN fail resref=k1p_zaalbaru refused by npc-world (reason in npc-world_log.txt)",
        "SPAWN begin resref=k1p_bastilla tag=k1p_bastilla area=204tel at -44.5,-63.1,0.0",
        "SPAWN fail resref=k1p_bastilla refused by npc-world (reason in npc-world_log.txt)",
    };
    KT_CHECK(env.lines == want);
    KT_CHECK_EQ(env.spawnCalls, 7);
}

void testRunStableUnknownAndNullArea() {
    FakeEnv env;
    LogBudget budget;
    RunStable(env, "other", 1, 0u, true, budget);
    const std::vector<std::string> want = {"SPAWN stable area=other visit=1 salt=00000000 rows=0 spawn=on"};
    KT_CHECK(env.lines == want);
    KT_CHECK_EQ(env.spawnCalls, 0);

    FakeEnv nullEnv;
    LogBudget nullBudget;
    RunStable(nullEnv, nullptr, 4, 0xffu, true, nullBudget);
    const std::vector<std::string> nullWant = {"SPAWN stable area= visit=4 salt=000000ff rows=0 spawn=on"};
    KT_CHECK(nullEnv.lines == nullWant);
    KT_CHECK_EQ(nullEnv.spawnCalls, 0);
}

void testRunStableBudgetExhausted() {
    FakeEnv env;
    LogBudget budget;
    KT_CHECK(budget.reserve(LogBudget::kTotal, LogKind::Other));
    RunStable(env, "204tel", 1, 0u, true, budget);
    KT_CHECK(env.lines.empty());
    KT_CHECK_EQ(env.spawnCalls, 7);     // the spawns still happen
    KT_CHECK_EQ(env.flushes, 7);
    KT_CHECK_EQ(budget.dropped(), 15L);  // stable line plus begin and ok for each of seven rows
    KT_CHECK_EQ(budget.used(), 3000);
}

}  // namespace

int main() {
    testEventScopeTransitions();
    testScopedEventOpensOnce();
    testScopedEventNestedSpawn();
    testDumpGate();
    testDumpGateRepeatCommitKeepsSlots();
    testLogBudgetAddToAreaCap();
    testLogBudgetRefusalChargesNothing();
    testLogBudgetCapMarker();
    testScanBudget();
    testRunStableDisabled();
    testRunStableFormatAndOrder();
    testRunStableTagPresentSkips();
    testRunStableSpawnRefused();
    testRunStableUnknownAndNullArea();
    testRunStableBudgetExhausted();
    return kt::Report("k1_client_core_test");
}
