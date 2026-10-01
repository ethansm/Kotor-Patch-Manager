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
                           "fm1_rethink_ms", "fm4", "a4", "c1", "c1_party_only", "a3", "log"};
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
    CHECK(funcs == 18 && missing == 0);                          // 18 hooks (C1 x3 share one function)
    int exports = 0; for (const char* p = strstr(d, "EXPORTS\n"); p && (p = strstr(p + 1, "\n    ")); ) ++exports;
    CHECK(exports == 16);                                        // 16 distinct functions
    CHECK(!strstr(t, "Fm3") && !strstr(d, "Fm3"));               // FM3 probe not shipped
}

int main() {
    testGlob(); testNumbers(); testIni(); testShippedIni(); testGate(); testThrottles(); testDecisions(); testHandlerDecisions();
    testExports();
    printf("%d checks, %d failed\n%s\n", g_checks, g_fail, g_fail ? "TESTS FAILED" : "ALL PASS");
    return g_fail ? 1 : 0;
}
