// Host unit tests for CompanionAiLogic.h (run from this directory: it also parses the shipped companion_ai.ini):
//   g++ -std=c++17 -Wall -Wextra -o /tmp/companion_ai_logic_test companion_ai_logic_test.cpp && /tmp/companion_ai_logic_test
#include "CompanionAiLogic.h"
#include <stdio.h>
#include <stdlib.h>
using namespace cai;

static int g_fail = 0, g_checks = 0;
#define CHECK(c) do { ++g_checks; if (!(c)) { printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); ++g_fail; } } while (0)

static IniTable g_t;   // large: keep it off the stack

static void testGlob() {
    CHECK(globMatch("k_ai_master", "K_AI_MASTER"));
    CHECK(!globMatch("k_ai_master", "k_ai_master2"));
    CHECK(globMatch("k_hen_*dlg", "k_hen_attondlg"));
    CHECK(globMatch("k_hen_*dlg", "k_hen_dlg"));
    CHECK(!globMatch("k_hen_*dlg", "k_hen_heartbt01"));
    CHECK(globMatch("n_*", "n_mandsold"));
    CHECK(globMatch("*", "anything"));
    CHECK(globMatch("a*b*c", "aXXbYYc"));
    CHECK(!globMatch("a*b*c", "aXXbYY"));
    NameList l; l.set(DefaultTraceScripts);
    CHECK(l.n == 5);
    const char* hen[] = {"atton", "baodur", "discip", "g0t0", "hanharr", "hk47", "hndmaid", "kreia", "mand", "mira", "remote", "t3m4", "visas"};
    for (const char* h : hen) { char s[40]; snprintf(s, sizeof s, "k_hen_%sdlg", h); CHECK(l.match(s)); }
    CHECK(l.match("A_ATKONEND")); CHECK(l.match("k_combat_rnd")); CHECK(!l.match("k_hen_heartbt01")); CHECK(!l.match("")); CHECK(!l.match(nullptr));
    NameList e; e.set("  ,  ,"); CHECK(e.empty());
    NameList t; t.set(" Boss_Tag , n_* "); CHECK(t.n == 2); CHECK(t.match("boss_tag")); CHECK(t.match("N_GUARD")); CHECK(!t.match("p_atton"));
    char buf[200]; l.format(buf, sizeof buf); CHECK(!strcmp(buf, DefaultTraceScripts));
}

static void testNumbers() {
    CHECK(parseMask("11,12,16") == ((1u << 11) | (1u << 12) | (1u << 16)));
    CHECK(parseMask(" 16 ") == (1u << 16));
    CHECK(parseMask("") == 0); CHECK(parseMask("40,99999999999,3") == (1u << 3));
    CHECK(inMask(parseMask("11,12"), 12)); CHECK(!inMask(parseMask("11,12"), 16)); CHECK(!inMask(~0u, -1)); CHECK(!inMask(~0u, 40));
    CHECK(parseInt("250", 0) == 250); CHECK(parseInt(" -3 ", 0) == -3); CHECK(parseInt("0x10", 0) == 16); CHECK(parseInt("abc", 7) == 7);
    CHECK(parseInt(nullptr, 9) == 9); CHECK(parseInt("12 ; comment", 0) == 12);
}

static void testIni() {
    const char* txt =
        "; header\n[Other]\na1=0\n[companionai]\r\n  a1 = 0   ; inline comment\r\n"
        "trace_scripts = k_ai_master , k_hen_*dlg ; two\nTAGS=n_boss#x\nshipbuild_scope=ALL\np1_interval_ms=0x100\n"
        "fm4=2\nmax_lines_per_sec=0\na1_rethink_ms=99999\n# hash comment\nnoequals\n[Next]\na3=1\n";
    g_t.parse(txt, "CompanionAI");
    CHECK(g_t.sectionFound);
    CHECK(!strcmp(g_t.get("a1"), "0"));
    CHECK(g_t.get("a3") == nullptr);   // other section
    CHECK(!strcmp(g_t.get("TRACE_SCRIPTS"), "k_ai_master , k_hen_*dlg"));
    CHECK(!strcmp(g_t.get("tags"), "n_boss#x"));   // '#' not after whitespace: part of the value
    Config c; parseConfig(g_t, c);
    CHECK(!c.a1); CHECK(c.traceScripts.n == 2); CHECK(c.traceScripts.match("k_hen_kreiadlg")); CHECK(!c.traceScripts.match("a_atkonend"));
    CHECK(c.shipScope == ScopeAll); CHECK(c.p1IntervalMs == 256); CHECK(c.fm4 == 0 && c.fm4Requested == 2);
    CHECK(c.maxLinesPerSec == 1); CHECK(c.a1RethinkMs == 3000);
    CHECK(c.tags.n == 1 && c.tags.match("N_BOSS#X"));
    // missing section: built-in defaults
    g_t.parse("[Other]\na1=0\n", "CompanionAI"); CHECK(!g_t.sectionFound);
    Config d; parseConfig(g_t, d); CHECK(d.a1 && d.trace && !d.p1);
    // duplicate key: last wins
    g_t.parse("[CompanionAI]\nc1=1\nc1=0\n", "CompanionAI"); Config e; parseConfig(g_t, e); CHECK(!e.c1);
    // sameConfig
    Config x, y; CHECK(sameConfig(x, y)); y.c1 = false; CHECK(!sameConfig(x, y)); y = x; y.tags.set("a"); CHECK(!sameConfig(x, y));
    // all flags 0 = vanilla
    g_t.parse("[CompanionAI]\ntrace=0\nscripts=0\nengine_points=0\na1=0\np1=0\nfm1=0\nfm4=0\na4=0\nc1=0\na3=0\nfm3_probe=0\n", "CompanionAI");
    Config v; parseConfig(g_t, v);
    CHECK(!v.trace && !v.scripts && !v.enginePoints && !v.a1 && !v.p1 && !v.fm1 && v.fm4 == 0 && !v.a4 && !v.c1 && !v.a3 && !v.fm3Probe);
}

static void testShippedIni() {
    FILE* f = fopen("companion_ai.ini", "rb");
    CHECK(f != nullptr);
    if (!f) return;
    static char buf[32768]; size_t n = fread(buf, 1, sizeof buf - 1, f); fclose(f); buf[n] = 0;
    g_t.parse(buf, "CompanionAI");
    CHECK(g_t.sectionFound);
    Config c; parseConfig(g_t, c);
    // shipped defaults (task brief): trace=1 scripts=1 engine_points=1 party_only=1, a1=1 a1_retarget=1 fm1=1 fm4=1 c1=1 a4=1,
    // p1=0 p1_interval_ms=250, a3=0
    CHECK(c.trace && c.scripts && c.enginePoints && c.partyOnly);
    CHECK(c.a1 && c.a1Retarget && c.fm1 && c.fm4 == 1 && c.c1 && c.a4);
    CHECK(!c.p1 && c.p1IntervalMs == 250 && !c.a3 && !c.fm3Probe);
    CHECK(c.a1Stances == maskOf3(11, 12, 16) && c.a1RetargetStances == maskOf3(11, 12, 16) && c.a1PartyOnly && c.a1RethinkMs == 3000);
    CHECK(c.c1PartyOnly && c.fm1RethinkMs == 0 && c.log && !c.logAll && c.enginePrint && c.logAur && !c.printsAllScripts);
    CHECK(c.shipScope == ScopeAi && c.tags.empty() && c.traceScripts.n == 5);
    CHECK(c.dedupeMs == 1000 && c.maxLinesPerSec == 200 && c.maxFileMb == 64 && c.flushMs == 250);
    CHECK(c.a1BurstThreshold == 4 && c.a1RateThreshold == 400 && !c.engineVerbose && c.summarySec == 10 && c.p1DetailLines == 20);
    // every key in the shipped file is one the parser knows (no typo silently ignored)
    const char* known[] = {"trace", "scripts", "engine_points", "trace_scripts", "shipbuild_scope", "prints_all_scripts", "party_only", "tags",
                           "log_all", "engine_print", "log_aur", "dedupe_ms", "max_lines_per_sec", "max_file_mb", "flush_ms",
                           "a1_burst_threshold", "a1_rate_threshold", "engine_verbose", "summary_sec", "p1_detail_lines", "a1", "a1_stances",
                           "a1_party_only", "a1_retarget", "a1_retarget_stances", "a1_rethink_ms", "p1", "p1_interval_ms", "fm1",
                           "fm1_rethink_ms", "fm4", "a4", "c1", "c1_party_only", "a3", "fm3_probe", "log"};
    int knownN = int(sizeof known / sizeof known[0]);
    CHECK(g_t.n == knownN);
    for (int i = 0; i < g_t.n; ++i) {
        bool ok = false;
        for (int k = 0; k < knownN; ++k) if (!strcmp(g_t.key[i], known[k])) ok = true;
        if (!ok) printf("unknown key in companion_ai.ini: %s\n", g_t.key[i]);
        CHECK(ok);
    }
    // the built-in defaults equal the shipped file
    Config def; CHECK(sameConfig(def, c));
}

static void testGate() {
    LogGate g; g.configure(3, 1000, 1000);
    unsigned t = 5000;
    // per-second cap: 3 lines, then drops, then a marker with the count at the next second
    for (int i = 0; i < 3; ++i) { GateOut o = g.admit(t, 0, false); CHECK(o.write && !o.droppedMarker); g.wrote(10); }
    GateOut o = g.admit(t + 10, 0, false); CHECK(!o.write);
    o = g.admit(t + 20, 0, false); CHECK(!o.write);
    o = g.admit(t + 30, 0, true); CHECK(o.write && o.droppedMarker == 2);   // forced bypasses the cap and carries the marker
    g.wrote(10);
    o = g.admit(t + 1000, 0, false); CHECK(o.write && o.droppedMarker == 0); g.wrote(10);
    CHECK(g.totalDropped == 2);
    // dedupe: same key within the window -> suppressed, then one repeat count before the next different line
    LogGate d; d.configure(100, 1000, 1 << 20);
    o = d.admit(100, 42, false); CHECK(o.write); d.wrote(5);
    for (int i = 0; i < 5; ++i) { o = d.admit(200 + i, 42, false); CHECK(!o.write); }
    o = d.admit(300, 43, false); CHECK(o.write && o.repeats == 5);
    o = d.admit(301, 43, false); CHECK(!o.write);
    o = d.admit(1400, 43, false); CHECK(o.write && o.repeats == 1);       // window expired: repeats flushed, line written again
    o = d.admit(1401, 0, false); CHECK(o.write && o.repeats == 0);        // key 0 never deduped
    o = d.admit(1402, 0, false); CHECK(o.write);
    CHECK(d.totalDeduped == 6);
    // file cap: one marker, then nothing; raising the cap resumes
    LogGate c; c.configure(100, 0, 100);
    o = c.admit(1, 0, true); CHECK(o.write); c.wrote(100);
    o = c.admit(2, 0, true); CHECK(!o.write && o.capMarker);
    o = c.admit(3, 0, true); CHECK(!o.write && !o.capMarker);
    CHECK(c.totalCapDropped == 2);
    c.configure(100, 0, 1000);
    o = c.admit(4, 0, false); CHECK(o.write && !o.capMarker);
    // GetTickCount wrap-around
    LogGate w; w.configure(1, 1000, 1 << 20);
    o = w.admit(0xFFFFFF00u, 0, false); CHECK(o.write);
    o = w.admit(0xFFFFFF10u, 0, false); CHECK(!o.write);
    o = w.admit(0x00000010u, 0, false); CHECK(!o.write);                          // 272 ms after the window start (across the wrap)
    o = w.admit(0x00000300u, 0, false); CHECK(o.write && o.droppedMarker == 2);   // 1024 ms: new second, marker for both drops
}

static void testThrottles() {
    LineThrottle t;
    int n = 0; for (unsigned i = 0; i < 100; ++i) if (t.allow(1000 + i, 7, 10, 2000)) ++n;
    CHECK(n == 10);
    CHECK(t.allow(1200, 8, 10, 2000));        // signature change
    CHECK(!t.allow(1201, 8, 10, 2000));
    CHECK(t.allow(3300, 8, 10, 2000));        // period
    unsigned last = 0; CHECK(due(1000, last, 1000)); CHECK(!due(1500, last, 1000)); CHECK(due(2000, last, 1000));
    // bursts: 30 entries 20 us apart = one RunActions loop; then normal one-per-frame entries
    BurstTrack b;
    for (int i = 0; i < 30; ++i) b.onEntry(1000000ull + i * 20);
    CHECK(b.maxBurst == 30 && b.perSec == 30 && b.spinning(4, 400));
    b.resetSecond();
    for (int i = 0; i < 60; ++i) b.onEntry(5000000ull + i * 16667ull);
    CHECK(b.maxBurst == 1 && b.perSec == 60 && !b.spinning(4, 400));
    RateAgg a;
    for (int i = 0; i < 10; ++i) a.add(1, 5000);
    for (int i = 0; i < 5; ++i) a.add(2, 100);
    CHECK(a.total == 15 && a.n == 2 && a.maxPer() == 10 && a.sharePct() == 66);
    for (int i = 0; i < 100; ++i) a.add(1000 + i, 0);
    CHECK(a.n == RateAgg::Slots && a.overflow == 100 - (RateAgg::Slots - 2));
    a.reset(); CHECK(a.total == 0 && a.n == 0);
    RestartCap r;
    int ok = 0; for (int i = 0; i < 300; ++i) if (r.allow(0x1000, 0x2000, 50, 256)) ++ok;
    CHECK(ok == 256);
    CHECK(r.allow(0x1000, 0x2000, 200, 256));   // > 50 ms later: a new call
    CHECK(r.allow(0x1004, 0x2000, 200, 256));   // other object: a new call
}

static void testDecisions() {
    CHECK(inWeaponRange(20.f, 19.f * 19.f)); CHECK(inWeaponRange(20.f, 20.1f * 20.1f)); CHECK(!inWeaponRange(20.f, 20.2f * 20.2f));
    CHECK(!inWeaponRange(0.5f, 0.f)); CHECK(!inWeaponRange(0.f, 0.f));
    CHECK(!strcmp(a1Reason(1, 900.f, 20.f, 1, true), "range"));
    CHECK(!strcmp(a1Reason(1, 100.f, 20.f, 0, true), "los"));
    CHECK(!strcmp(a1Reason(0, 100.f, 20.f, 1, true), "melee"));
    CHECK(!strcmp(a1Reason(1, 100.f, 20.f, 1, false), "area"));
    CHECK(!strcmp(a1Reason(1, 100.f, 20.f, 1, true), "other"));
    // P1: never stamped -> run; clock behind the stamp -> run; else interval / day rollover
    CHECK(p1ShouldRun(5, 1000, 0, 0, 0, 0, 250));
    CHECK(p1ShouldRun(5, 1000, 5, 2000, 0, 0, 250));
    CHECK(p1ShouldRun(5, 1000, 6, 0, 0, 0, 250));
    CHECK(!p1ShouldRun(5, 1100, 5, 1000, 0, 100, 250));
    CHECK(p1ShouldRun(5, 1300, 5, 1000, 0, 300, 250));
    CHECK(p1ShouldRun(6, 10, 5, 86399000, 1, 1010, 250));
    CHECK(p1ShouldRun(5, 1000, 5, 1000, 0, 0, 0));   // interval 0 = every tick
    // FM1 keep set
    uint16_t q1[] = {0x3D}; uint16_t q2[] = {0x3D, 0x40, 0x46}; uint16_t q3[] = {0x3D, 0x0C}; uint16_t q4[] = {0x13, 0x0C};
    CHECK(onlyFollowTypes(q1, 1)); CHECK(onlyFollowTypes(q2, 3)); CHECK(!onlyFollowTypes(q3, 2)); CHECK(!onlyFollowTypes(q4, 2));
    CHECK(!onlyFollowTypes(q1, 0));
    // FM3 pass bookkeeping
    Fm3Pass p; p.begin(4, 0x100);
    p.object(0x100); p.object(0x200); p.object(0x300); p.object(0x200); p.object(0x300);
    CHECK(p.objects == 5 && p.distinct == 3 && p.repeats == 2 && p.level == 4);
    p.begin(2, 0x900); CHECK(p.objects == 0 && p.repeats == 0);
    // line keys
    CHECK(lineKey(1, "a", "b") != lineKey(2, "a", "b")); CHECK(lineKey(1, "a", "b") == lineKey(1, "a", "b"));
    CHECK(lineKey(1, "ab", "") != lineKey(1, "a", "b")); CHECK(lineKey(0, nullptr, nullptr) != 0);
    char s[] = "a\nb\tc"; sanitize(s); CHECK(!strcmp(s, "a b c"));
}

int main() {
    testGlob(); testNumbers(); testIni(); testShippedIni(); testGate(); testThrottles(); testDecisions();
    printf("%d checks, %d failed\n%s\n", g_checks, g_fail, g_fail ? "TESTS FAILED" : "ALL PASS");
    return g_fail ? 1 : 0;
}
