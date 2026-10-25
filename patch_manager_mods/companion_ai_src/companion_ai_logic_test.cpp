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
    g_t.parse("[CompanionAI]\ntrace=0\nscripts=0\nengine_points=0\na1=0\np1=0\nfm1=0\nfm4=0\na4=0\nc1=0\na3=0\n", "CompanionAI");
    Config v; parseConfig(g_t, v);
    CHECK(!v.trace && !v.scripts && !v.enginePoints && !v.a1 && !v.p1 && !v.fm1 && v.fm4 == 0 && !v.a4 && !v.c1 && !v.a3);
    // word values (review SF1): off/false/no = 0, on/true/yes = 1, case-insensitive
    g_t.parse("[CompanionAI]\na1=off\nfm1=FALSE\nc1=No\na4=no\nfm4=off\np1=on\na3=True\ntrace=yes\n", "CompanionAI");
    Config w; parseConfig(g_t, w);
    CHECK(!w.a1 && !w.fm1 && !w.c1 && !w.a4 && w.fm4 == 0 && w.p1 && w.a3 && w.trace); CHECK(g_t.bad[0] == 0);
    // unparseable flag -> OFF (vanilla) and reported; unparseable number -> default and reported
    g_t.parse("[CompanionAI]\na1=of\nfm4=maybe\np1_interval_ms=fast\nc1=1\n", "CompanionAI");
    Config u; parseConfig(g_t, u);
    CHECK(!u.a1 && u.fm4 == 0 && u.p1IntervalMs == 250 && u.c1);
    CHECK(strstr(g_t.bad, "a1") && strstr(g_t.bad, "fm4") && strstr(g_t.bad, "p1_interval_ms") && !strstr(g_t.bad, "c1"));
    // empty value = default
    g_t.parse("[CompanionAI]\na1=\n", "CompanionAI"); Config z; parseConfig(g_t, z); CHECK(z.a1 && g_t.bad[0] == 0);
    // probes (PROBE_DESIGN.md section 4): default on, per-flag off, number clamp, unparseable number -> 400 + reported
    Config pd; CHECK(pd.probeFm2c && pd.probeFm2f && pd.probeFm2r && pd.probeFm3 && pd.probeRs && pd.probeP2 && pd.probePf && pd.probeRar &&
                     pd.probeShx && pd.probeSs1 && pd.probeGca && pd.probeParty && pd.probeA1x && pd.probeLines == 400);
    g_t.parse("[CompanionAI]\nprobe_rs=0\nprobe_p2=off\nprobe_rar=no\nprobe_lines=0x20\n", "CompanionAI");
    Config pr; parseConfig(g_t, pr);
    CHECK(!pr.probeRs && !pr.probeP2 && !pr.probeRar && pr.probeFm2c && pr.probeA1x && pr.probeLines == 32 && g_t.bad[0] == 0);
    g_t.parse("[CompanionAI]\nprobe_lines=x\nprobe_pf=maybe\n", "CompanionAI"); Config pb; parseConfig(g_t, pb);
    CHECK(pb.probeLines == 400 && strstr(g_t.bad, "probe_lines")); CHECK(!pb.probePf && strstr(g_t.bad, "probe_pf"));   // flag -> off, number -> default
    g_t.parse("[CompanionAI]\nprobe_lines=999999999\n", "CompanionAI"); Config pc; parseConfig(g_t, pc); CHECK(pc.probeLines == 100000);
    g_t.parse("[CompanionAI]\nprobe_lines=-5\n", "CompanionAI"); Config pn; parseConfig(g_t, pn); CHECK(pn.probeLines == 0);
    // sameConfig sees every probe key
    {
        bool Config::*flags[] = {&Config::probeFm2c, &Config::probeFm2f, &Config::probeFm2r, &Config::probeFm3, &Config::probeRs, &Config::probeP2,
                                 &Config::probePf, &Config::probeRar, &Config::probeShx, &Config::probeSs1, &Config::probeGca, &Config::probeParty,
                                 &Config::probeA1x};
        for (auto f : flags) { Config a, b; b.*f = false; CHECK(!sameConfig(a, b)); }
        Config a, b; b.probeLines = 399; CHECK(!sameConfig(a, b));
    }
}

static void testShippedIni() {
    FILE* f = fopen("companion_ai.ini", "rb");
    CHECK(f != nullptr);
    if (!f) return;
    static char buf[32768]; size_t n = fread(buf, 1, sizeof buf - 1, f); fclose(f); buf[n] = 0;
    g_t.parse(buf, "CompanionAI");
    CHECK(g_t.sectionFound);
    Config c; parseConfig(g_t, c);
    CHECK(g_t.bad[0] == 0);   // every shipped value parses
    if (g_t.bad[0]) printf("unparseable in companion_ai.ini: %s\n", g_t.bad);
    // shipped defaults (task brief): trace=1 scripts=1 engine_points=1 party_only=1, a1=1 a1_retarget=1 fm1=1 fm4=1 c1=1 a4=1,
    // p1=0 p1_interval_ms=250, a3=0
    CHECK(c.trace && c.scripts && c.enginePoints && c.partyOnly);
    CHECK(c.a1 && c.a1Retarget && c.fm1 && c.fm4 == 1 && c.c1 && c.a4);
    CHECK(!c.p1 && c.p1IntervalMs == 250 && !c.a3);
    CHECK(c.debugLog);                           // shipped on: debug_log=1
    CHECK(c.a1Stances == maskOf3(11, 12, 16) && c.a1RetargetStances == maskOf3(11, 12, 16) && c.a1PartyOnly && c.a1RethinkMs == 3000);
    CHECK(c.c1PartyOnly && c.fm1RethinkMs == 0 && c.log && !c.logAll && c.enginePrint && c.logAur && !c.printsAllScripts);
    CHECK(c.shipScope == ScopeAi && c.tags.empty() && c.traceScripts.n == 5);
    CHECK(c.dedupeMs == 1000 && c.maxLinesPerSec == 200 && c.maxFileMb == 64 && c.flushMs == 250);
    CHECK(c.a1BurstThreshold == 4 && c.a1RateThreshold == 400 && !c.engineVerbose && c.summarySec == 10 && c.p1DetailLines == 20);
    CHECK(!c.a1RetargetStrict);                  // shipped off: a1_retarget_strict=0
    // every key in the shipped file is one the parser knows (no typo silently ignored)
    const char* known[] = {"trace", "scripts", "engine_points", "trace_scripts", "shipbuild_scope", "prints_all_scripts", "party_only", "tags",
                           "log_all", "engine_print", "log_aur", "dedupe_ms", "max_lines_per_sec", "max_file_mb", "flush_ms",
                           "a1_burst_threshold", "a1_rate_threshold", "engine_verbose", "summary_sec", "p1_detail_lines", "a1", "a1_stances",
                           "a1_party_only", "a1_retarget", "a1_retarget_stances", "a1_retarget_strict", "a1_rethink_ms", "p1", "p1_interval_ms", "fm1",
                           "fm1_rethink_ms", "fm4", "a4", "c1", "c1_party_only", "a3", "cast_family_guard", "cast_queue_newest", "log", "debug_log",
                           "probe_fm2c", "probe_fm2f", "probe_fm2r", "probe_fm3", "probe_rs", "probe_p2", "probe_pf", "probe_rar", "probe_shx",
                           "probe_ss1", "probe_gca", "probe_party", "probe_a1x", "probe_lines"};
    int knownN = int(sizeof known / sizeof known[0]);
    CHECK(g_t.n == knownN); CHECK(knownN == 54);
    for (int i = 0; i < g_t.n; ++i) {
        bool ok = false;
        for (int k = 0; k < knownN; ++k) if (!strcmp(g_t.key[i], known[k])) ok = true;
        if (!ok) printf("unknown key in companion_ai.ini: %s\n", g_t.key[i]);
        CHECK(ok);
    }
    // the probe build ships every probe on with the 400-line budget
    CHECK(c.probeFm2c && c.probeFm2f && c.probeFm2r && c.probeFm3 && c.probeRs && c.probeP2 && c.probePf && c.probeRar && c.probeShx &&
          c.probeSs1 && c.probeGca && c.probeParty && c.probeA1x && c.probeLines == 400);
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
    // FM1 why: first failing gate in handler order; parity: why==keep <=> fm1 && party && listOk && onlyFollowTypes (the handler's keep)
    CHECK(fm1KeepWhy(false, 1, true, q1, 1) == Fm1Off); CHECK(fm1KeepWhy(true, 0, true, q1, 1) == Fm1NotParty);
    CHECK(fm1KeepWhy(true, 1, false, q1, 1) == Fm1UnreadableList); CHECK(fm1KeepWhy(true, 1, true, q3, 2) == Fm1NonFollowTypes);
    CHECK(fm1KeepWhy(true, 1, true, q1, 0) == Fm1NonFollowTypes); CHECK(fm1KeepWhy(true, 1, true, q2, 3) == Fm1Keep);
    CHECK(!strcmp(fm1WhyName(Fm1Keep), "keep") && !strcmp(fm1WhyName(Fm1NotParty), "not-party") && !strcmp(fm1WhyName(Fm1UnreadableList), "unreadable-list"));
    uint16_t* qs[] = {q1, q2, q3, q4}; int qn[] = {1, 3, 2, 2};
    for (int f = 0; f < 2; ++f) for (int p = 0; p < 2; ++p) for (int l = 0; l < 2; ++l) for (int k = 0; k < 4; ++k) {
        bool keepOld = f && p && l && onlyFollowTypes(qs[k], qn[k]);
        CHECK((fm1KeepWhy(f, p, l, qs[k], qn[k]) == Fm1Keep) == keepOld);
    }
    // line keys
    CHECK(lineKey(1, "a", "b") != lineKey(2, "a", "b")); CHECK(lineKey(1, "a", "b") == lineKey(1, "a", "b"));
    CHECK(lineKey(1, "ab", "") != lineKey(1, "a", "b")); CHECK(lineKey(0, nullptr, nullptr) != 0);
    char s[] = "a\nb\tc"; sanitize(s); CHECK(!strcmp(s, "a b c"));
}

// Review SF3: the behaviour-changing handler decisions.
static void testHandlerDecisions() {
    Config on;                                   // shipped defaults: a1=1 stances/retarget 11,12,16, party only, rethink 3000
    // ---- A1 gate
    CHECK(a1SpinCandidate(11, 0, 0)); CHECK(a1SpinCandidate(12, 0, 0));
    CHECK(!a1SpinCandidate(16, 0, 0));           // stance 16 kept by ranged-ai 0.3.0 (out of range/melee) = Aggressive approach
    CHECK(!a1SpinCandidate(9, 0, 0)); CHECK(!a1SpinCandidate(11, 1, 0)); CHECK(!a1SpinCandidate(12, 0, 1));
    CHECK(a1Applies(on, 11, 1)); CHECK(a1Applies(on, 12, 1)); CHECK(a1Applies(on, 16, 1));   // raw 16 mapped to eff 11
    CHECK(!a1Applies(on, 11, 0));                // enemies / non-party: vanilla (party only)
    CHECK(!a1Applies(on, 13, 1));                // raw stance not listed
    Config off = on; off.a1 = false; CHECK(!a1Applies(off, 11, 1));
    Config any = on; any.a1PartyOnly = false; CHECK(a1Applies(any, 11, 0));
    Config onlyR = on; onlyR.a1Stances = parseMask("11"); CHECK(a1Applies(onlyR, 11, 1)); CHECK(!a1Applies(onlyR, 12, 1));
    CHECK(a1TryRetarget(on, 12, 20.f));          // Stationary also retargets (user decision)
    CHECK(!a1TryRetarget(on, 11, 0.5f)); CHECK(a1TryRetarget(on, 11, 0.6f));
    Config noRt = on; noRt.a1Retarget = false; CHECK(!a1TryRetarget(noRt, 11, 20.f));
    Config rtR = on; rtR.a1RetargetStances = parseMask("11"); CHECK(!a1TryRetarget(rtR, 12, 20.f));
    CHECK(a1NewTarget(0) == Invalid); CHECK(a1NewTarget(Invalid) == Invalid); CHECK(a1NewTarget(0x80000431) == (int)0x80000431);
    CHECK(!a1RethinkWanted(on, Invalid));        // 3000 = engine value: never written
    Config rt = on; rt.a1RethinkMs = 1500;
    CHECK(a1RethinkWanted(rt, Invalid)); CHECK(!a1RethinkWanted(rt, 0x80000431));   // only after an END
    CHECK(a1RethinkWrite(rt, 3000)); CHECK(!a1RethinkWrite(rt, 1500)); CHECK(!a1RethinkWrite(rt, 200));
    // ---- A1 strict-retarget gate (a1_retarget_strict; a1GateTarget). `on` is shipped (gate off); gOn is the gate-on copy
    Config gOn = on; gOn.a1RetargetStrict = true;
    CHECK(a1GateTarget(on, 0x80000431, false) == (int)0x80000431);    // gate off: passthrough even when !strictOk
    CHECK(a1GateTarget(on, 0x80000431, true) == (int)0x80000431);
    CHECK(a1GateTarget(gOn, 0x80000431, false) == Invalid);           // gate on, !strictOk: END path (WI-106: any newId)
    CHECK(a1GateTarget(gOn, 0x80000431, true) == (int)0x80000431);    // gate on, strictOk: keep
    CHECK(a1GateTarget(gOn, Invalid, false) == Invalid);              // Invalid passes through unchanged
    CHECK(a1GateTarget(on, Invalid, true) == Invalid);
    // ---- A4 result mapping (a4=1) and vanilla mapping (a4=0)
    A4Out o = a4Decide(false, 0);    CHECK(o.ret == 1 && o.local == 0 && o.guarded);    // NULL/unreadable row
    o = a4Decide(true, 0);           CHECK(o.ret == 1 && o.local == 0 && !o.guarded);   // row+0x154 == 0: vanilla JZ target
    o = a4Decide(true, 7);           CHECK(o.ret == 0 && o.local == 7 && !o.guarded);   // non-zero: resume, CMP/JZ falls through
    CHECK(a4VanillaResult(0) == 1 && a4VanillaResult(-3) == 0);
    // ---- C1 restart index
    Config c1on = on;
    CHECK(c1Who(c1on, 5, 1)); CHECK(!c1Who(c1on, 5, 0)); CHECK(!c1Who(c1on, 9, 1));       // party creatures only
    Config c1any = on; c1any.c1PartyOnly = false; CHECK(c1Who(c1any, -1, 0));
    CHECK(c1Value(true, true, true) == -1); CHECK(c1Value(false, true, true) == 0);
    CHECK(c1Value(true, false, true) == 0); CHECK(c1Value(true, true, false) == 0);       // cap reached: vanilla 0
    // ---- P1
    CHECK(p1ParityReset(false, 5, 1000)); CHECK(!p1ParityReset(false, 0, 0)); CHECK(!p1ParityReset(true, 5, 1000));
    CHECK(p1Local(true) == 0xFA0u && p1Local(false) == 0u);
    // ---- FM1 fallback cap
    Config f = on; CHECK(!fm1RethinkWrite(f, 1, 1, 3000));                                   // shipped 0 = never
    f.fm1RethinkMs = 800; CHECK(fm1RethinkWrite(f, 1, 1, 3000)); CHECK(!fm1RethinkWrite(f, 0, 1, 3000));
    CHECK(!fm1RethinkWrite(f, 1, 0, 3000)); CHECK(!fm1RethinkWrite(f, 1, 1, 500)); f.fm1 = false; CHECK(!fm1RethinkWrite(f, 1, 1, 3000));
    // ---- FM4 snapshot/restore pairing
    Fm4Pair p;
    CHECK(p.begin(1, false)); p.commit(0x1000); CHECK(p.take(0x1000)); CHECK(!p.take(0x1000));   // used once
    CHECK(p.begin(0, true));                                     // fm4=0 + log: snapshot for the log only
    CHECK(!p.begin(0, false));                                   // nothing to do
    p.begin(1, false); p.commit(0x1000); CHECK(!p.take(0x2000)); CHECK(!p.take(0x1000));   // other creature clears the slot
    p.begin(1, false); p.commit(0x1000); p.begin(1, false); CHECK(!p.take(0x1000));         // a missed restore is dropped
    p.begin(1, false); CHECK(!p.take(0x1000));                                                // snapshot failed: no restore
    CHECK(fm4ShouldRestore(1, true)); CHECK(!fm4ShouldRestore(0, true)); CHECK(!fm4ShouldRestore(1, false));   // hot flip 1->0
    // ---- INI reload rules
    CHECK(reloadDue(true, 5, 5)); CHECK(!reloadDue(false, 1500, 600)); CHECK(reloadDue(false, 1600, 600));
    CHECK(reloadDue(false, 0x10, 0xFFFFFC00u));                  // GetTickCount wrap
    CHECK(reloadPre(true, false, false, false) == ReloadPre::MissingDefaultsLog);   // load without an INI: shipped defaults
    CHECK(reloadPre(false, false, true, false) == ReloadPre::MissingKeepLog);       // deleted while running: keep, log once
    CHECK(reloadPre(false, false, false, false) == ReloadPre::MissingSilent);       // still missing: silent
    CHECK(reloadPre(false, true, true, true) == ReloadPre::Skip);                   // unchanged time+size
    CHECK(reloadPre(false, true, true, false) == ReloadPre::Read);
    CHECK(reloadPre(false, true, false, true) == ReloadPre::Read);                  // reappeared / retry after a failed open
    CHECK(reloadPre(true, true, true, true) == ReloadPre::Read);
    CHECK(reloadPost(false, false, false) == ReloadPost::RetryLater); CHECK(reloadPost(true, false, false) == ReloadPost::RetryLater);
    CHECK(reloadPost(false, true, false) == ReloadPost::KeepBadSection);            // half-written file: keep the config
    CHECK(reloadPost(true, true, false) == ReloadPost::Apply);                      // at load: built-in defaults
    CHECK(reloadPost(false, true, true) == ReloadPost::Apply);
    // ---- per-subject throttle (FM1/E2/A4 lines)
    SubjectThrottle st;
    int n = 0; for (unsigned i = 0; i < 50; ++i) if (st.allow(1, 0x100, 1000 + i, 0, 20, 10000)) ++n;
    CHECK(n == 20);
    CHECK(st.allow(2, 0x100, 1100, 0, 20, 10000));               // other kind, same creature: own budget
    CHECK(st.allow(1, 0x200, 1100, 0, 20, 10000));               // other creature: own budget
    CHECK(!st.allow(1, 0x100, 5000, 0, 20, 10000)); CHECK(st.allow(1, 0x100, 11100, 0, 20, 10000));   // period
    CHECK(st.allow(1, 0x100, 11101, 1, 20, 10000));              // signature change
    for (int k = 0; k < 100; ++k) st.allow(3, 0x1000 + k, 20000 + k, 0, 1, 10000);   // LRU eviction never fails
    CHECK(st.allow(3, 0x1000 + 99, 20200, 0, 1, 10000) == false);   // the newest subject kept its state
}

// Phase-4 probe helpers (PROBE_DESIGN.md section 4)
static void testProbeHelpers() {
    // rsSite
    CHECK(rsSite(0x58F514) == 1 && rsSite(0x569A6C) == 2 && rsSite(0x59E909) == 3 && rsSite(0x5709F5) == 4 && rsSite(0x57074A) == 5);
    CHECK(rsSite(0x570645) == 6 && rsSite(0x57069F) == 6);
    CHECK(rsSite(0x58F50F) == 0 && rsSite(0) == 0 && rsSite(0x58F515) == 0 && rsSite(0xFFFFFFFFu) == 0);
    CHECK(!strcmp(rsName(1), "S6A") && !strcmp(rsName(2), "S6B") && !strcmp(rsName(3), "S6C") && !strcmp(rsName(4), "S7") &&
          !strcmp(rsName(5), "S12") && !strcmp(rsName(6), "S1") && !strcmp(rsName(0), "-") && !strcmp(rsName(9), "-"));
    // pfSite: the 15 CALL sites + 5
    const unsigned pf[PfSites] = {0x574537, 0x575170, 0x599AF4, 0x5F89AA, 0x5F8A13, 0x5F8EB9, 0x5F93B4, 0x5FEAC7, 0x6306A5, 0x630B17,
                                  0x66CA5A, 0x6900C1, 0x69B901, 0x89FE45, 0x8A050E};
    for (int i = 0; i < PfSites; ++i) { CHECK(pfSite(pf[i]) == i); CHECK(pfSite(pf[i] - 5) == -1); CHECK(strcmp(pfName(i), "?") != 0); }
    CHECK(pfSite(0x574532) == -1 && pfSite(0) == -1 && !strcmp(pfName(-1), "?") && !strcmp(pfName(15), "?"));
    CHECK(!strcmp(pfName(0), "SAVE_A") && !strcmp(pfName(1), "SAVE_B") && !strcmp(pfName(3), "XFER_A") && !strcmp(pfName(4), "XFER_B"));
    for (int i = -1; i < PfSites; ++i) CHECK(pfTransient(i) == (i == 0 || i == 1 || i == 3 || i == 4));   // SAVE_A/B, XFER_A/B only
    // p2Runs
    CHECK(p2Runs(0, 1, 4000)); CHECK(!p2Runs(0, 1, 3999)); CHECK(!p2Runs(1, 1, 9000)); CHECK(!p2Runs(0, 2, 9000)); CHECK(p2Runs(0, 1, 0xFFFFFFFFu));
    // shoutExit
    CHECK(shoutExit(false, 0, 0, false) == ShoutNoArea); CHECK(shoutExit(false, 3, 5, true) == ShoutNoArea);
    CHECK(shoutExit(true, -1, 5, true) == ShoutAbortStart); CHECK(shoutExit(true, 5, 5, true) == ShoutEnd);
    CHECK(shoutExit(true, 2, 5, true) == ShoutXBreak); CHECK(shoutExit(true, 2, 5, false) == ShoutAbortMid);
    CHECK(shoutExit(true, 0, 0, false) == ShoutEnd);   // empty list: index 0 >= count 0
    CHECK(!strcmp(shoutName(ShoutNoArea), "noarea") && !strcmp(shoutName(ShoutAbortStart), "ABORT-START") && !strcmp(shoutName(ShoutEnd), "end") &&
          !strcmp(shoutName(ShoutXBreak), "xbreak") && !strcmp(shoutName(ShoutAbortMid), "ABORT-MID"));
    // fm2cReason
    CHECK(!strcmp(fm2cReason(1, 0, 3), "client")); CHECK(!strcmp(fm2cReason(1, -1, 2), "client"));
    CHECK(!strcmp(fm2cReason(0, -1, 3), "code-1")); CHECK(!strcmp(fm2cReason(0, 5, 2), "partial")); CHECK(!strcmp(fm2cReason(0, 5, 3), "blocked"));
    CHECK(!strcmp(fm2cReason(0, 5, 7), "other"));
    // fm3Lost
    CHECK(!fm3Lost(4, 4, false)); CHECK(fm3Lost(4, 2, false)); CHECK(fm3Lost(2, 2, true));
    // a1Strict: on the bound = in, a hair over = out; zero range still has the 0.1 slack
    float lim = 2.0f + 0.1f;
    CHECK(a1Strict(0, 0, lim, 0, 2.0f)); CHECK(a1Strict(0, 0, 0, -lim, 2.0f)); CHECK(!a1Strict(0, 0, lim + 0.01f, 0, 2.0f));
    CHECK(a1Strict(5, 5, 5 + 0.05f, 5, 0.0f)); CHECK(!a1Strict(5, 5, 5 + 0.2f, 5, 0.0f));
    CHECK(!a1Strict(0, 0, 3.0f, 3.0f, 4.0f)); CHECK(a1Strict(0, 0, 3.0f, 3.0f, 4.2f));   // 2D: sqrt(18) = 4.24 vs 4.3
    // pairBump
    PairCount p;
    CHECK(pairBump(p, 1, 2, 1000, 12000) && p.n == 1);                // first event starts the window
    CHECK(!pairBump(p, 1, 2, 5000, 12000) && p.n == 2);
    CHECK(!pairBump(p, 1, 2, 17000, 12000) && p.n == 3);              // 12000 after the last event: still inside
    p.lastD = 7.f;
    CHECK(pairBump(p, 1, 3, 17500, 12000) && p.n == 1 && p.lastD == 0.f && p.tgt == 3);   // other target: reset
    CHECK(pairBump(p, 1, 3, 17500 + 12001, 12000) && p.n == 1);      // gap > window: reset
    CHECK(pairBump(p, 9, 3, 29502, 12000) && p.cr == 9);              // other creature: reset
    CHECK(!pairBump(p, 9, 3, 29502 + 12000, 12000) && p.n == 2);
    PairCount w;                                                      // GetTickCount wraparound
    CHECK(pairBump(w, 1, 2, 0xFFFFFF00u, 5000) && !pairBump(w, 1, 2, 0x00000100u, 5000) && w.n == 2);   // 512 ms across the wrap
    CHECK(pairBump(w, 1, 2, 0x00000100u + 5001u, 5000) && w.n == 1);
    CHECK(w.first == 0x00000100u + 5001u);
    // a1OrphanDue
    CHECK(!a1OrphanDue(5000, 0, 3000)); CHECK(!a1OrphanDue(8000, 4000, 3000)); CHECK(!a1OrphanDue(5000 + 4000, 5000, 3000)); CHECK(a1OrphanDue(5000 + 4001, 5000, 3000));
    CHECK(a1OrphanDue(0x00000010u + 5000u, 0xFFFFFFF0u, 3000));   // wrap: 5024 ms later
    CHECK(!a1OrphanDue(0x00000010u + 100u, 0xFFFFFFF0u, 3000));
    CHECK(a1OrphanDue(1002, 1, 0)); CHECK(!a1OrphanDue(1001, 1, 0)); CHECK(!a1OrphanDue(1002, 1, 1000));
    // a1Rebase (WI-125 / R2-CA-03): frozen time (pause or a stall with no hook running) does not age a pending A1 end
    CHECK(kA1StallMs == 1000);
    CHECK(a1Rebase(0, 5100, 10000, false) == 0 && a1Rebase(0, 5100, 10000, true) == 0);                // none stays none
    CHECK(a1Rebase(5000, 0, 10000, false) == 5000 && a1Rebase(5000, 0, 10000, true) == 5000);          // no previous sample: unchanged
    // stall / pause gap above rethink+1 s (3000+1000): no ORPHAN, frozen age kept
    CHECK(a1OrphanDue(10000, 5000, 3000));                                                             // BASE: unshifted end is ORPHAN
    { unsigned e = a1Rebase(5000, 5100, 10000, false);                                                 // 4900 ms gap, hooks silent
      CHECK(e == 9900 && !a1OrphanDue(10000, e, 3000));                                                // age at the last sample (100 ms) kept
      CHECK(!a1OrphanDue(10000 + 3900, e, 3000) && a1OrphanDue(10000 + 3901, e, 3000)); }              // aging resumes after the gap
    CHECK(a1Rebase(5100, 5100, 60000, false) == 60000);                                                // end set at the last sample: age 0 after a 55 s gap
    CHECK(a1Rebase(5200, 5100, 10000, false) == 10000);                                                // end set inside the gap: capped at now
    CHECK(a1Rebase(5000, 9500, 10000, true) == 5500);                                                  // pause bit read on a short gap: that gap is frozen
    // normal small gaps age normally (existing ORPHAN still fires)
    CHECK(a1Rebase(5000, 5250, 5500, false) == 5000 && a1Rebase(5000, 5000, 6000, false) == 5000);     // gap 250 and exactly kA1StallMs: unchanged
    CHECK(a1Rebase(5000, 5000, 6001, false) == 6001);                                                  // gap 1001: stalled
    { unsigned e = 5000; for (unsigned t = 5250; t <= 9250; t += 250) e = a1Rebase(e, t - 250, t, false);
      CHECK(e == 5000 && a1OrphanDue(9001, e, 3000) && !a1OrphanDue(9000, e, 3000)); }                // 250 ms ticks: ORPHAN at 4001 ms as before
    { unsigned e = 5000, prev = 5250;   // hooks running while paused (pause bit set), 250 ms samples
      for (unsigned t = 5500; t <= 20000; t += 250) { e = a1Rebase(e, prev, t, true); prev = t; CHECK(!a1OrphanDue(t, e, 3000)); }
      CHECK(e == 19750 && !a1OrphanDue(23750, e, 3000) && a1OrphanDue(23751, e, 3000)); }              // frozen at age 250 for 15 s; aging resumes after the unpause
    // wrap across 0xFFFFFFFF
    CHECK(a1Rebase(0xFFFFFF00u, 0xFFFFFF80u, 0x00001000u, false) == 0x00000F80u);                      // gap 0x1080 wraps: age 0x80 kept
    CHECK(!a1OrphanDue(0x00001000u, a1Rebase(0xFFFFFF00u, 0xFFFFFF80u, 0x00001000u, false), 3000));
    CHECK(a1OrphanDue(0x00001000u, 0xFFFFFF00u, 0));                                                   // BASE: unshifted wrap end is ORPHAN
    CHECK(a1Rebase(0xFFFFFF80u, 0xFFFFFF00u, 0x00001000u, false) == 0x00001000u);                      // end set inside a wrapped gap: capped at now
    // a shift never yields 0 (= none)
    CHECK(a1Rebase(0xFFFFF000u, 0x00000010u, 0x00001010u, false) == 1u);                               // endAt + gap wraps to exactly 0 -> 1
    CHECK(a1Rebase(0xFFFFF000u, 0x00000010u, 0x00001010u, false) != 0u);
    CHECK(a1Rebase(0xFFFFFFF0u, 0xFFFFE000u, 0u, false) == 1u);                                        // stalled, capped at now == 0 -> 1
    CHECK(a1Rebase(0x7FFFFFFFu, 0x10u, 0x5000u, false) == 0x7FFFFFFFu);                                // end in the future (clock stepped back): untouched
    // clock stepped backwards (now < lastClock): not a stall, even with the pause bit; pending end unchanged
    CHECK(!a1Frozen(10000, 5000, false) && !a1Frozen(10000, 5000, true));
    CHECK(!a1Frozen(0xFFFFFF00u, 0x00000100u, false) && a1Frozen(1, 0x7FFFFFFFu, false) && !a1Frozen(1, 0x80000002u, false));   // small forward wrap: normal; big forward gap: stall; ~2^31+ is backward
    CHECK(a1Rebase(5000, 10000, 6000, false) == 5000 && a1Rebase(5000, 10000, 6000, true) == 5000);
    CHECK(a1Rebase(4000, 0x40000000u, 0x10u, false) == 4000);
    // rsProbeIdle: probe_rs=0 keeps the A1X follow-up alive
    CHECK(rsProbeIdle(false, false)); CHECK(!rsProbeIdle(false, true)); CHECK(!rsProbeIdle(true, false)); CHECK(!rsProbeIdle(true, true));
    // rarIndex
    CHECK(rarIndex(1, 2) == 7); CHECK(rarIndex(0x47, 4) == 359); CHECK(rarIndex(0x48, 1) == 1); CHECK(rarIndex(5, 9) == 25);
    CHECK(rarIndex(0, 0) == 0 && rarIndex(-3, 4) == 4 && rarIndex(0x47, 0) == 355 && rarIndex(0x47, 5) == 355 && rarIndex(1, -1) == 5);
    for (int ty = -2; ty < 0x60; ++ty) for (int r = -2; r < 8; ++r) { int i = rarIndex(ty, r); CHECK(i >= 0 && i < RarBuckets); }
    // ProbeCap: 400 takes, the 401st refused, the marker exactly once
    ProbeCap cap; int granted = 0;
    CHECK(!cap.marker());                                             // nothing refused yet
    for (int i = 0; i < 400; ++i) if (cap.take(400)) ++granted;
    CHECK(granted == 400); CHECK(!cap.marker());
    CHECK(!cap.take(400)); CHECK(cap.marker()); CHECK(!cap.marker()); CHECK(!cap.take(400)); CHECK(!cap.marker());
    ProbeCap zero; CHECK(!zero.take(0)); CHECK(zero.marker()); CHECK(!zero.marker());   // probe_lines=0: summaries only
    ProbeCap big; for (int i = 0; i < 1000; ++i) CHECK(big.take(100000));   // a raised budget (hot reload) keeps granting
    ProbeCap grow; for (int i = 0; i < 5; ++i) grow.take(5); CHECK(!grow.take(5)); CHECK(grow.take(6));   // reload with a larger budget resumes
}

// WI-77 (BG30): a spinning E1 track that stops being seen expires (one END); a recently seen one keeps spinning.
static void testSpinExpiry() {
    CHECK(spinExpired(true, 1000, 1000 + kSpinExpireMs, kSpinExpireMs));          // unseen exactly TTL: expire
    CHECK(spinExpired(true, 0, 10000, kSpinExpireMs));                             // unseen 10 s after death / area change
    CHECK(spinExpired(true, 0xFFFFF000u, 0x00000800u, kSpinExpireMs));             // GetTickCount wrap, unseen 6144 ms
    CHECK(!spinExpired(true, 1000, 1000 + kSpinExpireMs - 1, kSpinExpireMs));      // 1 ms short: still spinning
    CHECK(!spinExpired(true, 9000, 9100, kSpinExpireMs));                          // seen again (track() refreshes seen): no END
    CHECK(!spinExpired(false, 0, 10000, kSpinExpireMs));                           // not spinning (already ENDed / never spun): no END
}

// WI-78 (BG31): PARTY area baseline. Replays FnGetArea frames through the same two helpers partyTick uses (have == true).
template <typename Arr> static int partyAreaFires(const Arr& frames, int* baselineOut = nullptr) {
    int base = 0, fires = 0;
    for (size_t i = 0; i < sizeof frames / sizeof frames[0]; ++i) {
        if (partyAreaChanged(base, frames[i])) ++fires;
        base = partyAreaNext(base, frames[i]);
    }
    if (baselineOut) *baselineOut = base;
    return fires;
}
static void testPartyArea() {
    CHECK(!partyAreaChanged(0, 5)); CHECK(!partyAreaChanged(5, 0)); CHECK(!partyAreaChanged(0, 0)); CHECK(!partyAreaChanged(5, 5));
    CHECK(partyAreaChanged(5, 7));
    CHECK(partyAreaNext(5, 0) == 5); CHECK(partyAreaNext(0, 0) == 0); CHECK(partyAreaNext(0, 7) == 7); CHECK(partyAreaNext(5, 7) == 7);
    static const int repeat[] = {5, 5, 5}, change[] = {5, 7}, zeroSame[] = {5, 0, 5}, zeroThenB[] = {5, 0, 7};
    static const int firstZero[] = {0, 5}, firstZeroThen[] = {0, 5, 7}, zeroRun[] = {5, 0, 0, 0, 7}, swap[] = {5, 7, 5};
    static const int zeroSwap[] = {5, 0, 7, 0, 7}, leadingZeros[] = {0, 0, 0};
    int base = -1;
    CHECK(partyAreaFires(repeat) == 0);                             // A->A: no change
    CHECK(partyAreaFires(change) == 1);                             // A->B: why=area
    CHECK(partyAreaFires(zeroSame, &base) == 0 && base == 5);       // A->0->A: no why, baseline kept
    CHECK(partyAreaFires(zeroThenB, &base) == 1 && base == 7);      // A->0->B: why=area (kills the unconditional store)
    CHECK(partyAreaFires(firstZero, &base) == 0 && base == 5);      // first-ever 0 then A: no baseline yet, no why
    CHECK(partyAreaFires(firstZeroThen) == 1);                      // ...then a real change after that fires
    CHECK(partyAreaFires(zeroRun) == 1);                            // A->0->0->0->B: repeated unreadable frames keep A
    CHECK(partyAreaFires(swap) == 2);                               // A->B->A: each real change fires
    CHECK(partyAreaFires(zeroSwap) == 1);                           // A->0->B->0->B: only the first B fires
    CHECK(partyAreaFires(leadingZeros, &base) == 0 && base == 0);   // unreadable from the start: no baseline
}

// G1 cast family guard (WI-136): family table, drop decision, INI key
static void testCastGuard() {
    // buff families 1-10: every id of party_ai_fixes.nss GN_PartyPowerFamily's buff rows
    CHECK(castFamily(8) == 1 && castFamily(34) == 1 && castFamily(37) == 1);                 // Speed
    CHECK(castFamily(17) == 2 && castFamily(18) == 2 && castFamily(24) == 2);                 // Armor
    CHECK(castFamily(22) == 3 && castFamily(33) == 3 && castFamily(36) == 3);                 // Valor
    CHECK(castFamily(40) == 4 && castFamily(42) == 4 && castFamily(133) == 4);                // Energy Resist
    CHECK(castFamily(135) == 5 && castFamily(136) == 5 && castFamily(137) == 5);              // Barrier
    CHECK(castFamily(138) == 6 && castFamily(139) == 6 && castFamily(140) == 6);              // Battle Meditation
    CHECK(castFamily(164) == 7 && castFamily(165) == 7 && castFamily(166) == 7);              // Fury
    for (int s = 167; s <= 172; ++s) CHECK(castFamily(s) == 8);                              // Inspire Followers
    CHECK(castFamily(150) == 9 && castFamily(151) == 9 && castFamily(152) == 9);              // Force Body
    CHECK(castFamily(20) == 10 && castFamily(41) == 10);                                      // Force Immunity / Resist Force
    int tracked = 0; for (int s = 0; s < 256; ++s) tracked += castFamily(s) != 0;
    CHECK(tracked == 32);                                                                     // no other id in 0..255 is tracked
    // untracked: debuff families (WI-137), area powers, Kill, unknown and invalid ids
    CHECK(castFamily(45) == 0 && castFamily(7) == 0 && castFamily(38) == 0);                  // single-target debuffs
    CHECK(castFamily(30) == 0 && castFamily(31) == 0 && castFamily(32) == 0);                 // Horror / Insanity / Kill
    CHECK(castFamily(0) == 0 && castFamily(-1) == 0 && castFamily(0x7F000000) == 0);
    // drop decision: same family active (any tier, the same spell too) drops; other family, nothing active, untracked keep
    const int speedHi[] = {37}, speedSame[] = {8}, armor[] = {17}, mixed[] = {22, 137, 40}, none[] = {0};
    CHECK(castGuardDrops(true, false, 8, speedHi, 1));        // lower tier cast, higher tier active: drop
    CHECK(castGuardDrops(true, false, 37, speedSame, 1));     // higher tier cast, lower tier active: drop
    CHECK(castGuardDrops(true, false, 8, speedSame, 1));      // same spell active: drop
    CHECK(castGuardDrops(true, false, 135, mixed, 3));        // Barrier cast: family found among other families
    CHECK(!castGuardDrops(true, false, 135, mixed, 1));       // only the first entry (Valor) scanned: no Barrier
    CHECK(!castGuardDrops(true, false, 8, armor, 1));         // different family active: keep
    CHECK(!castGuardDrops(true, false, 8, none, 1));          // no tracked effect: keep
    CHECK(!castGuardDrops(true, false, 8, speedHi, 0));       // empty effect list: keep
    CHECK(!castGuardDrops(true, false, 8, nullptr, 3));       // unreadable list: keep
    CHECK(!castGuardDrops(true, true, 8, speedHi, 1));        // party leader (player-issued): never dropped
    CHECK(!castGuardDrops(true, true, 37, speedSame, 1));
    CHECK(!castGuardDrops(true, false, 45, speedHi, 1));      // untracked spell (debuff): keep
    CHECK(!castGuardDrops(true, false, -1, speedHi, 1));      // unknown spell row: keep
    CHECK(!castGuardDrops(false, false, 8, speedHi, 1));      // cast_family_guard=0: vanilla
    // INI key: default on (shipped), 0 turns it off, sameConfig sees it
    Config d; CHECK(d.castFamilyGuard);
    g_t.parse("[CompanionAI]\ncast_family_guard=0\n", "CompanionAI"); Config off; parseConfig(g_t, off);
    CHECK(!off.castFamilyGuard && g_t.bad[0] == 0);
    Config e = d; e.castFamilyGuard = false; CHECK(!sameConfig(d, e)); CHECK(sameConfig(d, Config()));
    g_t.parse("[CompanionAI]\ncast_family_guard=maybe\n", "CompanionAI"); Config bad; parseConfig(g_t, bad);
    CHECK(!bad.castFamilyGuard && strstr(g_t.bad, "cast_family_guard") != nullptr);   // unparseable flag = off, like every flag
}

// F4 cast queue newest (WI-143): stale spell pick, drop decision inputs, INI key
static void testCastQueue() {
    int out[8];
    // same-run test: queued on this tick or 15 ms ago = same run (kept); 16 ms+ or never seen = an older run (stale)
    CHECK(f4SameRun(1000, 1000, 16) && f4SameRun(1000, 1015, 16));
    CHECK(!f4SameRun(1000, 1016, 16) && !f4SameRun(0, 5, 16) && !f4SameRun(1000, 1040, 16));
    // empty list: nothing
    const int none_t[1] = {9}, none_a[1] = {5};
    CHECK(staleSpellPick(none_t, none_a, 0, 0, out, 8) == 0);
    // one stale spell: picked
    CHECK(staleSpellPick(none_t, none_a, 1, 0, out, 8) == 1 && out[0] == 0);
    // spell equal to the in-progress action: kept
    CHECK(staleSpellPick(none_t, none_a, 1, 5, out, 8) == 0);
    // mixed types (1, 9, 9): the two spell nodes, in list order; the non-spell node is never picked
    const int mix_t[3] = {1, 9, 9}, mix_a[3] = {7, 8, 9};
    CHECK(staleSpellPick(mix_t, mix_a, 3, 0, out, 8) == 2 && out[0] == 1 && out[1] == 2);
    // the in-progress one is kept, the other spell is still picked
    CHECK(staleSpellPick(mix_t, mix_a, 3, 8, out, 8) == 1 && out[0] == 2);
    // outCap 1 caps the count at 1 (the first stale spell)
    const int tri_t[3] = {9, 9, 9}, tri_a[3] = {1, 2, 3};
    CHECK(staleSpellPick(tri_t, tri_a, 3, 0, out, 1) == 1 && out[0] == 0);
    // no spell nodes at all, and null / empty arguments: nothing
    const int non_t[2] = {1, 2}, non_a[2] = {4, 5};
    CHECK(staleSpellPick(non_t, non_a, 2, 0, out, 8) == 0);
    CHECK(staleSpellPick(nullptr, none_a, 1, 0, out, 8) == 0 && staleSpellPick(none_t, none_a, 1, 0, out, 0) == 0);
    // INI key: default on (shipped), 0 turns it off, sameConfig sees it
    Config d; CHECK(d.castQueueNewest);
    g_t.parse("[CompanionAI]\ncast_queue_newest=0\n", "CompanionAI"); Config off; parseConfig(g_t, off);
    CHECK(!off.castQueueNewest && g_t.bad[0] == 0 && off.castFamilyGuard);
    Config e = d; e.castQueueNewest = false; CHECK(!sameConfig(d, e)); CHECK(sameConfig(d, Config()));
    g_t.parse("[CompanionAI]\ncast_queue_newest=maybe\n", "CompanionAI"); Config bad; parseConfig(g_t, bad);
    CHECK(!bad.castQueueNewest && strstr(g_t.bad, "cast_queue_newest") != nullptr);   // unparseable flag = off, like every flag
}

// exports.def == the hooks.toml function names (run from the source directory)
static void testExports() {
    static char t[65536], d[4096];
    FILE* f = fopen("kotor2-steam-aspyr.hooks.toml", "rb"); CHECK(f != nullptr); if (!f) return;
    size_t n = fread(t, 1, sizeof t - 1, f); fclose(f); t[n] = 0;
    f = fopen("exports.def", "rb"); CHECK(f != nullptr); if (!f) return;
    n = fread(d, 1, sizeof d - 1, f); fclose(f); d[n] = 0;
    int funcs = 0, missing = 0;
    for (const char* p = strstr(t, "function = \""); p; p = strstr(p + 1, "function = \"")) {
        const char* a = p + 12; const char* e = strchr(a, '"'); if (!e) break;
        char name[64] = {}; size_t l = size_t(e - a) < sizeof name - 1 ? size_t(e - a) : sizeof name - 1; memcpy(name, a, l);
        ++funcs;
        char line[80]; snprintf(line, sizeof line, "    %s\n", name);
        if (!strstr(d, line)) { printf("exports.def lacks %s\n", name); ++missing; }
    }
    CHECK(funcs == 33 && missing == 0);                          // 33 hooks: 19 + the 13 Phase-4 probes (C1 x3 share one function; G1 = CompanionAiCastGuard; F4 = CompanionAiCastQueueNewest)
    int exports = 0; for (const char* p = strstr(d, "EXPORTS\n"); p && (p = strstr(p + 1, "\n    ")); ) ++exports;
    CHECK(exports == 32);                                        // 32 exports: 31 distinct hook functions + KPatchInit (KpLog)
    CHECK(strstr(t, "0x005917C1") != nullptr);                   // F4 hook address (AddSWSpellAction before AddAction)
    CHECK(!strstr(t, "0x0051DBE7"));                             // the hot FM3 probe window stays unshipped (the cold FM3S/E/R ship)
    CHECK(strstr(t, "0x0051DBE1") && strstr(t, "0x0051DD33") && strstr(t, "0x0051E0C9"));
}

// WI-144 RetHist (RSX site-0 histogram): slot reuse, overflow to other, top-k order (count desc, then ret asc).
static void testRetHist() {
    RetHist e; unsigned r[16], n[16];
    CHECK(e.top(r, n, 5) == 0);                                  // empty: nothing written
    for (int i = 0; i < 3; ++i) e.add(0x58F514u);
    CHECK(e.n[0] == 3 && e.ret[0] == 0x58F514u && e.other == 0);   // same ret 3x -> one slot, n=3
    CHECK(e.top(r, n, 0) == 0);                                  // k=0 writes nothing
    RetHist o;
    for (unsigned i = 0; i < 17; ++i) o.add(0x400000u + i);      // 16 distinct fill the table, the 17th -> other
    CHECK(o.other == 1);
    o.add(0x400000u);                                            // a stored ret still counts in its slot, not in other
    CHECK(o.n[0] == 2 && o.other == 1);
    CHECK(o.top(r, n, 16) == 16);
    RetHist t;
    t.add(0x300u); t.add(0x300u); t.add(0x100u); t.add(0x100u);
    for (int i = 0; i < 5; ++i) t.add(0x200u);
    t.add(0x50u);
    CHECK(t.top(r, n, 2) == 2);                                  // 0x200 (5) first, then the 2-way tie at 2 -> lower ret 0x100
    CHECK(r[0] == 0x200u && n[0] == 5 && r[1] == 0x100u && n[1] == 2);
    CHECK(t.top(r, n, 10) == 4);                                 // only 4 used slots
    CHECK(r[2] == 0x300u && n[2] == 2 && r[3] == 0x50u && n[3] == 1);
}

// debug_log (0.3.2-probe): config key, D-line reason names / dedupe key, A1 skip label
static void testDebugLog() {
    Config d; CHECK(d.debugLog);                                  // default on
    IniTable t; t.parse("[CompanionAI]\ndebug_log=0\n", "CompanionAI");
    Config c; parseConfig(t, c);
    CHECK(!c.debugLog && !sameConfig(c, d));                      // a reload that flips only debug_log is a config change
    IniTable t1; t1.parse("[CompanionAI]\ndebug_log=1\n", "CompanionAI");
    Config c1; parseConfig(t1, c1);
    CHECK(c1.debugLog && sameConfig(c1, d));
    CHECK(!strcmp(dbgKeepName(DkLeader), "leader") && !strcmp(dbgKeepName(DkNotParty), "not-party"));
    CHECK(!strcmp(dbgKeepName(DkUntracked), "untracked-spell") && !strcmp(dbgKeepName(DkFamilyNotActive), "family-not-active"));
    CHECK(!strcmp(dbgKeepName(DkGuardOff), "guard-off") && !strcmp(dbgKeepName(DkBadRead), "bad-read"));
    CHECK(!strcmp(dbgKeepName(DkNoStale), "no-stale-nodes") && !strcmp(dbgKeepName(DkCount), "?") && !strcmp(dbgKeepName(-1), "?"));
    CHECK(dbgKey(0, 0, 0) != 0);                                  // 0 would mean "no dedupe"
    CHECK(dbgKey(0x7F000010, 8, DkLeader) == dbgKey(0x7F000010, 8, DkLeader));
    CHECK(dbgKey(0x7F000010, 8, DkLeader) != dbgKey(0x7F000010, 8, DkNotParty));
    CHECK(dbgKey(0x7F000010, 8, DkLeader) != dbgKey(0x7F000011, 8, DkLeader));
    CHECK(dbgKey(0x7F000010, 8, DkLeader) != dbgKey(0x7F000010, 9, DkLeader));
    // A1 skip label: party_only is named only when the stance IS listed and the party_only gate blocked
    Config a;                                                     // a1=1, stances 11/12/16, a1_party_only=1
    CHECK(!strcmp(a1SkipLabel(a, 11, 0), "party_only"));
    CHECK(!strcmp(a1SkipLabel(a, 3, 0), "spin (not listed)"));    // stance not listed: the stance is the cause
    CHECK(!strcmp(a1SkipLabel(a, 11, 1), "spin (not listed)"));   // party member, listed stance: not reachable via a1Applies, label stays generic
    a.a1PartyOnly = false; CHECK(!strcmp(a1SkipLabel(a, 3, 0), "spin (not listed)"));
    a.a1 = false; CHECK(!strcmp(a1SkipLabel(a, 11, 0), "spin (a1=0)"));
}

int main() {
    testGlob(); testNumbers(); testIni(); testShippedIni(); testGate(); testThrottles(); testDecisions(); testHandlerDecisions();
    testProbeHelpers(); testSpinExpiry(); testCastGuard(); testCastQueue(); testExports(); testPartyArea(); testRetHist(); testDebugLog();
    printf("%d checks, %d failed\n%s\n", g_checks, g_fail, g_fail ? "TESTS FAILED" : "ALL PASS");
    return g_fail ? 1 : 0;
}
