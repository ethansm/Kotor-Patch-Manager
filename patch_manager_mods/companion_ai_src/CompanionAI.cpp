// Companion AI (Steam Aspyr build 6A522E71.../LAA 4AB72FC1...): party-AI trace layer + engine AI fixes, one DLL,
// one INI (companion_ai.ini), one log (ai_trace.txt). Designs: patch_manager_mods/mods678_research/ai_review_2026-10-01/
// designs/design_ai_trace.md and design_ai_engine_fixes.md (user decisions in that folder's README.md).
//
// Every behaviour has an INI flag; all flags 0 = vanilla. The INI is re-checked at most once a second (file time and
// size; re-parsed only when they change), no restart needed.
//
// Hooks (cdecl detours; the stolen bytes run after the handler; for a consumed-exit hook EAX is excluded from the
// restore, so EAX = the handler result on BOTH exits and both exits are EAX-dead, check_windows.py):
//  trace (trace=1 scripts=1 / engine_points=1):
//   T1  0x692C50 AiTraceShipBuild(ebp)          ExecuteCommandShipBuild: [ebp-4]=0 (ShipBuild() FALSE) for traced scripts
//   T2  0x68F301 AiTracePrintString(ebp) -> int  ExecuteCommandPrintString: log; 1 => 0x68F39F (skip the engine print)
//   T3  0x6F62F2 AiTraceAurPostString(ebp)      ExecuteCommandAurPostString (retail stub): log
//   E1  0x6D740F AiTraceAttackEntry(eax)        AIActionPhysicalAttacks entry (+0x570 reset): A1 spin detector
//   E1b 0x6D8BE2 AiTraceAttackApproachExit(ebp) approach-branch exit (return 2): snapshot
//   E2  0x569A08 AiTraceFallbackFires(ebp)      AIUpdate: the slot-6 (end of round) fallback is about to run
//   E3  0x56BDBD AiTracePerceptionMode1(ebp)    SpawnInHeartbeatPerception block 2: UpdatePerception(1) rate
//  fixes:
//   A1  0x6D853E CompanionAiA1Gate(ebp) -> int  Ranged/Stationary self-requeue: 1 => 0x6D7D2F (retarget or end)
//   P1i 0x56BDB4 CompanionAiP1Interval(ebp)     perception throttle decision ([ebp-0x28] for the stolen CMP)
//   P1s 0x56BDE3 CompanionAiP1Stamp(ebp)        perception throttle stamp
//   FM1 0x586F43 CompanionAiBumpClear(ebp)      BumpFriends ClearAllActions(1) (skip-only: the handler makes the call)
//   FM4 0x57451F CompanionAiFm4Snap(eax) / 0x575183 CompanionAiFm4Restore(ebp)   SaveCreature FollowInfo (mode 1)
//   A4  0x6D9E2A CompanionAiA4(ebp) -> int      AIActionCombat NULL spell row (skip-only): 1 => 0x6D9EA4
//   C1  0x5868B9/0x5868F1/0x586929 CompanionAiC1(ebp)   ClearHostileActionsVersus restart index (skip-only)
//   A3  0x6D78B7 CompanionAiA3(ebp)             dead/undetected-target exit: clear the orientation lock (opt, a3=0)
//   FM3 0x51DBE7 CompanionAiFm3Probe(ebp)       scheduler sentinel probe, log only (fm3_probe=0)
//
// Hot handlers (E1, E3, P1i/P1s, FM3) read the live frame and the creature the engine is about to dereference
// directly; every other pointer goes through SafeRead (bad pointer => vanilla).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "../_shared/SafeRead.h"
#include "CompanionAiLogic.h"

namespace {

using cai::Invalid;
constexpr const char* Version = "0.1.0";

// ---- game globals and functions (conventions from the raw .asm)
constexpr int G_VM = 0x00A1B4A8;            // CVirtualMachine*
constexpr int G_App = 0x00A1B4A4;           // CAppManager*: +4 client side, +8 server side
constexpr int G_RunScriptVar = 0x00A7E10C;  // ExecuteScript event id
constexpr int FnGetGameObject = 0x0051C0A0; // thiscall(server=[[0xA1B4A4]+8]; id) RET 4 -> obj or 0
constexpr int FnClientList = 0x0073FB90;    // thiscall([[0xA1B4A4]+4]) RET -> x   (0x6D852C)
constexpr int FnClientAt = 0x007E5DA0;      // thiscall(x; int idx) RET 4 -> p     (0x6D8533)
constexpr int FnControlled = 0x0077D800;    // thiscall(p) RET -> controlled creature or 0 (0x6D8557)
constexpr int FnNearestEnemy = 0x0057C990;  // thiscall(cr; float range, int exclude, int ref) RET 0xC -> id
constexpr int FnClearAllActions = 0x00541080; // thiscall(obj; int clearAttacks) RET 4
constexpr int FnSetLock = 0x0057D370;       // thiscall(cr; int id, int) RET 8 (EndAttackAction's lock clear 0x6DAB40)
constexpr int FnGetArea = 0x005453C0;       // thiscall(cr) RET -> area
using Fn0 = int(__thiscall*)(int);
using Fn1 = int(__thiscall*)(int, int);
using Fn2 = int(__thiscall*)(int, int, int);
using RetargetFn = int(__thiscall*)(int, float, int, int);

// ---- object / creature offsets
constexpr int ObId = 0x04, ObType = 0x08, ObTag = 0x18, ObQueue = 0x100;
constexpr int ObjTypeCreature = 5;
constexpr int CrSlot6 = 0x2A0, CrP1Flag = 0x38C, CrP1Interval = 0x39C, CrStampA = 0x3A0, CrStampB = 0x3A4;
constexpr int CrFollowInfo = 0x500, CrCombat = 0x520, CrTarget = 0x550, CrFallback = 0x570, CrRound = 0x10DC,
              CrIsPC = 0x10E8, CrStats = 0x1198, CrParty = 0x11AC;
constexpr int RoundActive = 0xA84, StatsStance = 0x146, FollowInfoSize = 0x3C;
// VM internals
constexpr int VmInternal = 0x1C, VmiDepth = 0x28, VmiName = 0x44, VmiNameStride = 0x28, VmiSelf = 0x18C;

// AIActionPhysicalAttacks 0x6D7390 frame
constexpr int F_Attacker = -0x49C, F_Stance = -0x174, F_PartyTable = -0x178, F_Backoff = -0x168, F_Ranged = -0x164,
              F_Ctrl = -0x16C, F_Dist2 = -0xBC, F_Max = -0x2C, F_Los = -0xD0, F_ForceJump = -0x3C, F_TargetArea = -0x38,
              F_ActionTarget = -0x14, F_NewTarget = -0xE8, F_DeadExitValid = -0x30;

// ---- frame / memory access
template <typename T> T fr(int ebp, int off) { return *reinterpret_cast<const T*>(ebp + off); }   // live hooked frame
template <typename T> void frw(int ebp, int off, T v) { *reinterpret_cast<T*>(ebp + off) = v; }
template <typename T> T rd(int base, int off, T def = T()) {
    T v = def;
    saferead::readAt<T>(1, static_cast<uintptr_t>(base), off, &v);
    return v;
}
template <typename T> bool wr(int addr, T v) {
    if (!addr || !saferead::probeRead(2, reinterpret_cast<const void*>(addr), sizeof(T))) return false;
    *reinterpret_cast<T*>(addr) = v;
    return true;
}
// Copy an engine string (CExoString char* + length) into out, bounded and sanitized. "" when missing, "?" unreadable.
void copyStr(int p, int len, char* out, int cap) {
    out[0] = 0;
    if (!p || cap < 2) return;
    int n = cap - 1;
    if (len >= 0 && len < n) n = len;
    if (n <= 0) return;
    if (!saferead::probeRead(3, reinterpret_cast<const void*>(p), n)) {
        int toPage = int(((static_cast<unsigned>(p) | 0xFFFu) + 1u) - static_cast<unsigned>(p));
        if (toPage < n && saferead::probeRead(3, reinterpret_cast<const void*>(p), toPage)) n = toPage;
        else { out[0] = '?'; out[1] = 0; return; }
    }
    memcpy(out, reinterpret_cast<const void*>(p), n);
    out[n] = 0;
    cai::sanitize(out);
}
void readTag(int obj, char* out, int cap) {
    copyStr(rd<int>(obj, ObTag), rd<int>(obj, ObTag + 4, -1), out, cap);
    if (!out[0]) { out[0] = '-'; out[1] = 0; }
}

// ---- config, log
cai::Config g_cfg;
cai::IniTable g_ini;
char g_iniText[32768];
char g_iniPath[MAX_PATH + 32] = "companion_ai.ini";
char g_logPath[MAX_PATH + 32] = "ai_trace.txt";
DWORD g_lastIniCheck = 0, g_iniSize = ~0u;
FILETIME g_iniTime = {0, 0};
bool g_iniSeen = false, g_ready = false;

CRITICAL_SECTION g_cs;
FILE* g_log = nullptr;
cai::LogGate g_gate;
DWORD g_lastFlush = 0;
bool g_dirty = false;

struct Lock { Lock() { EnterCriticalSection(&g_cs); } ~Lock() { LeaveCriticalSection(&g_cs); } };

void writeRaw(const char* s, int n) {
    if (n <= 0) return;
    fwrite(s, 1, n, g_log);
    g_gate.wrote(static_cast<unsigned long long>(n));
    g_dirty = true;
}

void vemit(DWORD now, uint32_t key, bool force, const char* fmt, va_list ap) {
    if (!g_log) return;
    Lock lk;
    cai::GateOut o = g_gate.admit(now, key, force);
    char buf[1100];
    bool flushNow = force;
    if (o.capMarker) {
        writeRaw(buf, snprintf(buf, sizeof buf, "-- ai_trace: file cap %d MB reached at t=%lu; further lines dropped (raise max_file_mb to resume) --\n",
                               g_cfg.maxFileMb, now));
        flushNow = true;
    }
    if (o.repeats) writeRaw(buf, snprintf(buf, sizeof buf, "t=%lu R previous line repeated %ux\n", now, o.repeats));
    if (o.droppedMarker) {
        writeRaw(buf, snprintf(buf, sizeof buf, "-- ai_trace: %u lines dropped by the per-second cap (%d/s) at t=%lu --\n",
                               o.droppedMarker, g_cfg.maxLinesPerSec, now));
        flushNow = true;
    }
    if (o.write) {
        int n = vsnprintf(buf, sizeof buf - 1, fmt, ap);
        if (n < 0) n = 0;
        if (n > int(sizeof buf) - 2) n = int(sizeof buf) - 2;
        buf[n++] = '\n';
        writeRaw(buf, n);
    }
    if (g_dirty && (flushNow || now - g_lastFlush >= static_cast<DWORD>(g_cfg.flushMs))) { fflush(g_log); g_dirty = false; g_lastFlush = now; }
}
// trace line: dedupe key (0 = none), subject to the per-second cap
void tline(DWORD now, uint32_t key, const char* fmt, ...) { va_list ap; va_start(ap, fmt); vemit(now, key, false, fmt, ap); va_end(ap); }
// event line: fix events, summaries, config; bypasses the per-second cap (not the file cap), flushed at once
void eline(DWORD now, const char* fmt, ...) { va_list ap; va_start(ap, fmt); vemit(now, 0, true, fmt, ap); va_end(ap); }

void logConfig(DWORD now, const char* why) {
    const cai::Config& c = g_cfg;
    char scripts[600], tags[600];
    c.traceScripts.format(scripts, sizeof scripts);
    c.tags.format(tags, sizeof tags);
    eline(now, "# config (%s) trace=%d scripts=%d engine_points=%d shipbuild_scope=%s trace_scripts=%s prints_all_scripts=%d party_only=%d "
               "tags=%s log_all=%d engine_print=%d log_aur=%d dedupe_ms=%d caps=%d/s,%dMB flush_ms=%d a1_burst/rate=%d/%d engine_verbose=%d "
               "summary_sec=%d p1_detail_lines=%d",
          why, c.trace, c.scripts, c.enginePoints, c.shipScope == cai::ScopeOff ? "off" : c.shipScope == cai::ScopeAll ? "all" : "ai", scripts,
          c.printsAllScripts, c.partyOnly, tags, c.logAll, c.enginePrint, c.logAur, c.dedupeMs, c.maxLinesPerSec, c.maxFileMb, c.flushMs,
          c.a1BurstThreshold, c.a1RateThreshold, c.engineVerbose, c.summarySec, c.p1DetailLines);
    eline(now, "# config fixes a1=%d a1_stances=%x a1_party_only=%d a1_retarget=%d a1_retarget_stances=%x a1_rethink_ms=%d p1=%d p1_interval_ms=%d "
               "fm1=%d fm1_rethink_ms=%d fm4=%d%s a4=%d c1=%d c1_party_only=%d a3=%d fm3_probe=%d log=%d",
          c.a1, c.a1Stances, c.a1PartyOnly, c.a1Retarget, c.a1RetargetStances, c.a1RethinkMs, c.p1, c.p1IntervalMs, c.fm1, c.fm1RethinkMs,
          c.fm4, c.fm4Requested > 1 ? " (requested mode not built: vanilla)" : "", c.a4, c.c1, c.c1PartyOnly, c.a3, c.fm3Probe, c.log);
}

void applyGate() {
    g_gate.configure(static_cast<unsigned>(g_cfg.maxLinesPerSec), static_cast<unsigned>(g_cfg.dedupeMs),
                     static_cast<unsigned long long>(g_cfg.maxFileMb) << 20);
}

void loadIni(DWORD now, bool force) {
    if (!force && now - g_lastIniCheck < 1000) return;
    Lock lk;
    g_lastIniCheck = now;
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExA(g_iniPath, GetFileExInfoStandard, &fad)) {
        if (g_iniSeen || force) {
            if (force) { g_cfg = cai::Config(); applyGate(); }
            eline(now, "# config: %s not found; using %s", g_iniPath, force ? "the built-in defaults (= shipped values)" : "the last config");
            if (force) logConfig(now, "defaults");
        }
        g_iniSeen = false; g_iniSize = ~0u;
        return;
    }
    if (!force && g_iniSeen && CompareFileTime(&fad.ftLastWriteTime, &g_iniTime) == 0 && fad.nFileSizeLow == g_iniSize) return;
    FILE* f = fopen(g_iniPath, "rb");
    if (!f) { g_iniSeen = false; return; }   // e.g. an editor holding it: keep the current config, retry next second
    size_t n = fread(g_iniText, 1, sizeof g_iniText - 1, f);
    fclose(f);
    g_iniText[n] = 0;
    g_iniTime = fad.ftLastWriteTime; g_iniSize = fad.nFileSizeLow; g_iniSeen = true;
    g_ini.parse(g_iniText, "CompanionAI");
    if (!g_ini.sectionFound && !force) {   // half-written or wrong file: keep the current config until it changes again
        eline(now, "# config: no [CompanionAI] section in %s; keeping the current config", g_iniPath);
        return;
    }
    cai::Config c;
    cai::parseConfig(g_ini, c);
    bool changed = force || !cai::sameConfig(c, g_cfg);
    g_cfg = c;
    applyGate();
    if (!g_ini.sectionFound) eline(now, "# config: no [CompanionAI] section in %s; built-in defaults", g_iniPath);
    if (changed) logConfig(now, force ? "load" : "reload");
}

// ---- object info (script OBJECT_SELF), cached 1 s per id
struct ObjInfo { int id = 0; int type = -1; int party = 0; char tag[33] = "-"; DWORD tick = 0; };
ObjInfo g_obj[32];
int g_objNext = 0;

int gameObject(int id) {
    if (id == Invalid || !id) return 0;
    int app = rd<int>(G_App, 0), server = app ? rd<int>(app, 8) : 0;
    if (!server) return 0;
    return reinterpret_cast<Fn1>(FnGetGameObject)(server, id);
}

const ObjInfo& objInfo(int id, DWORD now) {
    for (auto& o : g_obj) if (o.id == id && now - o.tick < 1000) return o;
    ObjInfo* o = nullptr;
    for (auto& e : g_obj) if (e.id == id) { o = &e; break; }
    if (!o) { o = &g_obj[g_objNext]; g_objNext = (g_objNext + 1) % 32; }
    *o = ObjInfo();
    o->id = id; o->tick = now ? now : 1;
    if (int obj = gameObject(id)) {
        o->type = rd<unsigned char>(obj, ObType, 0xFF);
        if (o->type == ObjTypeCreature) o->party = rd<int>(obj, CrParty);
        readTag(obj, o->tag, sizeof o->tag);
    }
    return *o;
}

bool objPasses(const ObjInfo& o) {
    return g_cfg.logAll || (g_cfg.partyOnly && o.type == ObjTypeCreature && o.party) || (!g_cfg.tags.empty() && g_cfg.tags.match(o.tag));
}
// creature pointer filter for the engine points / fix logs
bool crPasses(int cr, int party) {
    if (g_cfg.logAll || (g_cfg.partyOnly && party)) return true;
    if (g_cfg.tags.empty()) return false;
    char tag[33]; readTag(cr, tag, sizeof tag);
    return g_cfg.tags.match(tag);
}

// ---- VM context: running script name, OBJECT_SELF, depth, event id
struct Ctx { int depth = -1; char script[40] = ""; int self = Invalid; int ev = 0; };
bool vmCtx(Ctx& c) {
    int vm = rd<int>(G_VM, 0), vmi = vm ? rd<int>(vm, VmInternal) : 0;
    if (!vmi) return false;
    c.depth = rd<int>(vmi, VmiDepth, -1);
    if (c.depth < 0 || c.depth > 7) return false;
    int slot = VmiName + c.depth * VmiNameStride;
    copyStr(rd<int>(vmi, slot), rd<int>(vmi, slot + 4, -1), c.script, 33);
    c.self = rd<int>(vmi, VmiSelf + c.depth * 4, Invalid);
    c.ev = rd<int>(G_RunScriptVar, 0);
    return c.script[0] != 0 && c.script[0] != '?';
}

// controlled creature id via the same chain AIActionPhysicalAttacks uses (0x6D8515..0x6D856B); false = unknown
bool controlledId(int* out) {
    *out = Invalid;
    int app = rd<int>(G_App, 0), client = app ? rd<int>(app, 4) : 0;
    if (!client) return false;
    int inner = rd<int>(client, 4);
    if (!inner || !saferead::probeRead(4, reinterpret_cast<const void*>(inner + 0x270), 4)) return false;
    int x = reinterpret_cast<Fn0>(FnClientList)(client);
    if (!x || !saferead::probeRead(4, reinterpret_cast<const void*>(x), 0x28)) return false;   // [x] count, [x+0x24] entry 0
    int p = reinterpret_cast<Fn1>(FnClientAt)(x, 0);
    if (!p) return true;   // no player client: vanilla leaves the controlled id at INVALID
    if (!saferead::probeRead(4, reinterpret_cast<const void*>(p), 0x100)) return false;
    int q = reinterpret_cast<Fn0>(FnControlled)(p);
    if (!q) return false;
    return saferead::readAt<int>(4, static_cast<uintptr_t>(q), ObId, out);
}

// ---- statistics
struct Stats {
    unsigned flips = 0, prints = 0, aur = 0, e1 = 0, e1spins = 0, e1b = 0, e2 = 0, e3 = 0;
    unsigned a1Would = 0, a1End = 0, a1Retarget = 0, p1Checks = 0, p1Allowed = 0, p1Throttled = 0, p1Runs = 0, p1Resets = 0;
    unsigned fm1Keep = 0, fm1Clear = 0, fm4Snap = 0, fm4Restore = 0, a4 = 0, c1Restart = 0, c1Vanilla = 0, c1Capped = 0, a3 = 0;
    unsigned fm3Passes = 0, fm3RepeatPasses = 0, fm3MaxRepeats = 0;
};
Stats g_st, g_stLast;
DWORD g_lastSummary = 0;

// ---- E1 / A1 per-creature tracks
struct CrTrack {
    int id = 0; int cr = 0; DWORD seen = 0;
    cai::BurstTrack burst; DWORD secStart = 0; int f570 = 0; int party = 0;
    bool spinning = false; DWORD spinStart = 0;
    // E1b snapshot
    unsigned exits = 0; int xStance = -1, xBackoff = 0, xCtrl = Invalid, xRanged = 0, xLos = 0, xTgt = Invalid; float xDist2 = 0, xMax = 0;
    // A1
    unsigned wouldSpin = 0, ends = 0, retargets = 0; DWORD lastA1 = 0; cai::LineThrottle a1Log;
};
CrTrack g_tr[16];
CrTrack* track(int id, DWORD now) {
    CrTrack* oldest = &g_tr[0];
    for (auto& t : g_tr) { if (t.id == id) { t.seen = now; return &t; } if (now - t.seen > now - oldest->seen) oldest = &t; }
    for (auto& t : g_tr) if (!t.id) { t = CrTrack(); t.id = id; t.seen = now; t.secStart = now; return &t; }
    *oldest = CrTrack(); oldest->id = id; oldest->seen = now; oldest->secStart = now;
    return oldest;
}

// ---- E3 aggregation
cai::RateAgg g_p1Agg;
DWORD g_p1AggStart = 0, g_lastP1Report = 0;
int g_p1Details = 0, g_p1DetailIds[64];

void reportP1(DWORD now) {
    if (!g_p1Agg.total) { g_p1AggStart = now; return; }
    double secs = (now - g_p1AggStart) / 1000.0; if (secs < 0.001) secs = 0.001;
    eline(now, "t=%lu E3 P1 calls/s=%.0f creatures=%d%s maxPerCreature/s=%.1f elapsed>=4000:%d%% (p1=%d interval=%d)", now,
          g_p1Agg.total / secs, g_p1Agg.n, g_p1Agg.overflow ? "+" : "", g_p1Agg.maxPer() / secs, g_p1Agg.sharePct(), g_cfg.p1,
          g_cfg.p1 ? g_cfg.p1IntervalMs : 4000);
    g_p1Agg.reset(); g_p1AggStart = now;
}

unsigned long long g_sumLines = 0, g_sumDropped = 0, g_sumDeduped = 0;
void summary(DWORD now) {
    if (!g_cfg.log && !g_cfg.trace) return;
    const Stats& s = g_st;
    // only when something happened since the last S line (the S line itself is not "something")
    if (memcmp(&s, &g_stLast, sizeof s) || g_gate.totalLines != g_sumLines || g_gate.totalDropped != g_sumDropped ||
        g_gate.totalDeduped != g_sumDeduped) {
        eline(now, "t=%lu S lines=%llu dropped=%llu deduped=%llu capDropped=%llu | flips=%u prints=%u aur=%u e1=%u e1spins=%u e1b=%u e2=%u e3=%u | "
                   "a1{would=%u end=%u retarget=%u} p1{checks=%u allowed=%u throttled=%u runs=%u resets=%u} fm1{keep=%u clear=%u} "
                   "fm4{snap=%u restore=%u} a4=%u c1{restart=%u vanilla=%u capped=%u} a3=%u fm3{passes=%u repeatPasses=%u maxRepeats=%u}",
              now, g_gate.totalLines, g_gate.totalDropped, g_gate.totalDeduped, g_gate.totalCapDropped, s.flips, s.prints, s.aur, s.e1,
              s.e1spins, s.e1b, s.e2, s.e3, s.a1Would, s.a1End, s.a1Retarget, s.p1Checks, s.p1Allowed, s.p1Throttled, s.p1Runs, s.p1Resets,
              s.fm1Keep, s.fm1Clear, s.fm4Snap, s.fm4Restore, s.a4, s.c1Restart, s.c1Vanilla, s.c1Capped, s.a3, s.fm3Passes,
              s.fm3RepeatPasses, s.fm3MaxRepeats);
        g_stLast = s;
    }
    for (auto& t : g_tr)
        if (t.id && (t.wouldSpin || t.ends || t.retargets)) {
            eline(now, "t=%lu A1 summary: cr=%08x would-spin passes=%u/%ds ends=%u retargets=%u", now, t.id, t.wouldSpin, g_cfg.summarySec,
                  t.ends, t.retargets);
            t.wouldSpin = t.ends = t.retargets = 0;
        }
    g_sumLines = g_gate.totalLines; g_sumDropped = g_gate.totalDropped; g_sumDeduped = g_gate.totalDeduped;
}

// Called first by every handler: INI reload (1 Hz), periodic summaries, buffered-log flush.
inline void tick(DWORD now) {
    if (!g_ready) return;
    if (now - g_lastIniCheck >= 1000) loadIni(now, false);
    if (now - g_lastSummary >= static_cast<DWORD>(g_cfg.summarySec) * 1000u) {
        g_lastSummary = now;
        summary(now);
        if (g_cfg.trace && g_cfg.enginePoints && !g_cfg.engineVerbose) reportP1(now);
    }
    if (g_cfg.trace && g_cfg.enginePoints && g_cfg.engineVerbose && now - g_lastP1Report >= 1000) { g_lastP1Report = now; reportP1(now); }
    if (g_dirty && g_log && now - g_lastFlush >= static_cast<DWORD>(g_cfg.flushMs)) { Lock lk; fflush(g_log); g_dirty = false; g_lastFlush = now; }
}

unsigned long long nowUs() {
    static LARGE_INTEGER freq = {};
    if (!freq.QuadPart) QueryPerformanceFrequency(&freq);
    LARGE_INTEGER c; QueryPerformanceCounter(&c);
    return freq.QuadPart ? static_cast<unsigned long long>(c.QuadPart) * 1000000ull / static_cast<unsigned long long>(freq.QuadPart) : 0;
}

int rawStance(int cr) {
    int stats = rd<int>(cr, CrStats);
    return stats ? rd<unsigned short>(stats, StatsStance, 0xFFFF) : -1;
}
int roundActive(int cr) { int r = rd<int>(cr, CrRound); return r ? rd<int>(r, RoundActive, -1) : -1; }

// E1 once-a-second evaluation (secondary pointers via SafeRead)
void e1Evaluate(CrTrack& t, DWORD now) {
    saferead::beginScope();
    int cr = t.cr;
    bool spin = t.burst.spinning(static_cast<unsigned>(g_cfg.a1BurstThreshold), static_cast<unsigned>(g_cfg.a1RateThreshold));
    int round = roundActive(cr);
    bool show = crPasses(cr, t.party);
    if (spin && round != 1) {
        if (!t.spinning) {
            t.spinning = true; t.spinStart = now; ++g_st.e1spins;
            if (show) {
                char tag[33]; readTag(cr, tag, sizeof tag);
                eline(now, "t=%lu E1 SPIN cr=%08x %s party=%d stance=%d entries/s=%u maxBurst=%u round=%d combat=%d f570=%d tgt=%08x "
                           "exit{n=%u stance=%d backoff=%d ctrl=%08x ranged=%d dist2=%.2f max=%.2f los=%d}",
                      now, t.id, tag, t.party, rawStance(cr), t.burst.perSec, t.burst.maxBurst, round, rd<int>(cr, CrCombat), t.f570,
                      t.xTgt, t.exits, t.xStance, t.xBackoff, t.xCtrl, t.xRanged, t.xDist2, t.xMax, t.xLos);
            }
        }
    } else if (t.spinning) {
        t.spinning = false;
        if (show) {
            char tag[33]; readTag(cr, tag, sizeof tag);
            eline(now, "t=%lu E1 END cr=%08x %s spin lasted %.1f s", now, t.id, tag, (now - t.spinStart) / 1000.0);
        }
    }
    if (g_cfg.engineVerbose && show)
        tline(now, 0, "t=%lu E1 RATE cr=%08x party=%d entries/s=%u maxBurst=%u round=%d f570=%d", now, t.id, t.party, t.burst.perSec,
              t.burst.maxBurst, round, t.f570);
    t.burst.resetSecond(); t.exits = 0; t.secStart = now;
}

} // namespace

// =====================================================================================================================
// Trace: VM command sites
// =====================================================================================================================

// T1. ExecuteCommandShipBuild 0x692C40 at 0x692C50: [ebp-4] is the value pushed at 0x692C5A.
extern "C" void __cdecl AiTraceShipBuild(int ebp) {
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    if (!g_cfg.trace || !g_cfg.scripts || g_cfg.shipScope == cai::ScopeOff) return;
    Ctx c; if (!vmCtx(c)) return;
    if (g_cfg.shipScope == cai::ScopeAi && !g_cfg.traceScripts.match(c.script)) return;
    if (!objPasses(objInfo(c.self, now))) return;
    frw<int>(ebp, -4, 0);   // ShipBuild() = FALSE for this call
    ++g_st.flips;
}

// T2. ExecuteCommandPrintString 0x68F260 at 0x68F301 (argc < 2 path, every compiled call). Return 1 => 0x68F39F.
extern "C" int __cdecl AiTracePrintString(int ebp) {
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    if (!g_cfg.trace || !g_cfg.scripts) return 0;
    Ctx c; if (!vmCtx(c)) return 0;
    bool traced = g_cfg.traceScripts.match(c.script);
    if (!traced && !g_cfg.printsAllScripts) return 0;
    const ObjInfo& o = objInfo(c.self, now);
    if (!objPasses(o)) return 0;
    char text[480];
    copyStr(fr<int>(ebp, -0x18), fr<int>(ebp, -0x14), text, sizeof text);
    ++g_st.prints;
    tline(now, cai::lineKey(c.self, c.script, text), "t=%lu P %08x %s %s ev=%d d=%d | %s", now, c.self, o.tag, c.script, c.ev, c.depth, text);
    return (traced && !g_cfg.enginePrint) ? 1 : 0;
}

// T3. ExecuteCommandAurPostString 0x6F6200 (retail stub) at 0x6F62F2, after the four pops succeeded.
extern "C" void __cdecl AiTraceAurPostString(int ebp) {
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    if (!g_cfg.trace || !g_cfg.scripts || !g_cfg.logAur) return;
    Ctx c; if (!vmCtx(c)) return;
    if (!g_cfg.traceScripts.match(c.script) && !g_cfg.printsAllScripts) return;
    const ObjInfo& o = objInfo(c.self, now);
    if (!objPasses(o)) return;
    char text[400];
    copyStr(fr<int>(ebp, -0x20), fr<int>(ebp, -0x1C), text, sizeof text);
    ++g_st.aur;
    tline(now, cai::lineKey(c.self, c.script, text), "t=%lu A %08x %s %s ev=%d d=%d | %s @(%d,%d) life=%.1f", now, c.self, o.tag, c.script,
          c.ev, c.depth, text, fr<int>(ebp, -0x10), fr<int>(ebp, -0x14), fr<float>(ebp, -0x18));
}

// =====================================================================================================================
// Trace: engine log points
// =====================================================================================================================

// E1. AIActionPhysicalAttacks 0x6D7390 at 0x6D740F, EAX = attacker (the stolen MOV [EAX+0x570],0xBB8 follows).
extern "C" void __cdecl AiTraceAttackEntry(int cr) {
    DWORD now = GetTickCount(); tick(now);
    if (!g_cfg.trace || !g_cfg.enginePoints || !cr) return;
    int party = *reinterpret_cast<const int*>(cr + CrParty);   // the engine writes [cr+0x570] next: cr is valid
    if (!party && !g_cfg.logAll && g_cfg.tags.empty()) return;
    ++g_st.e1;
    CrTrack* t = track(*reinterpret_cast<const int*>(cr + ObId), now);
    t->cr = cr; t->party = party;
    t->f570 = *reinterpret_cast<const int*>(cr + CrFallback);
    t->burst.onEntry(nowUs());
    if (now - t->secStart >= 1000) e1Evaluate(*t, now);
}

// E1b. Approach-branch exit 0x6D8BE2 (return 2): snapshot only (E1 prints it).
extern "C" void __cdecl AiTraceAttackApproachExit(int ebp) {
    DWORD now = GetTickCount(); tick(now);
    if (!g_cfg.trace || !g_cfg.enginePoints) return;
    int cr = fr<int>(ebp, F_Attacker);
    if (!cr) return;
    int party = *reinterpret_cast<const int*>(cr + CrParty);
    if (!party && !g_cfg.logAll && g_cfg.tags.empty()) return;
    ++g_st.e1b;
    CrTrack* t = track(*reinterpret_cast<const int*>(cr + ObId), now);
    t->cr = cr; t->party = party; ++t->exits;
    t->xStance = fr<int>(ebp, F_Stance); t->xBackoff = fr<int>(ebp, F_Backoff); t->xCtrl = fr<int>(ebp, F_Ctrl);
    t->xRanged = fr<int>(ebp, F_Ranged); t->xDist2 = fr<float>(ebp, F_Dist2); t->xMax = fr<float>(ebp, F_Max);
    t->xLos = fr<int>(ebp, F_Los); t->xTgt = fr<int>(ebp, F_ActionTarget);
}

// E2. CSWSCreature::AIUpdate 0x569320 at 0x569A08: +0x520==1, +0x570<=0, alive => the slot-6 script runs now.
extern "C" void __cdecl AiTraceFallbackFires(int ebp) {
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    if (!g_cfg.trace || !g_cfg.enginePoints) return;
    int cr = fr<int>(ebp, -0x194);
    int party = rd<int>(cr, CrParty);
    ++g_st.e2;
    if (!crPasses(cr, party)) return;
    char tag[33], script[40];
    readTag(cr, tag, sizeof tag);
    copyStr(rd<int>(cr, CrSlot6), rd<int>(cr, CrSlot6 + 4, -1), script, 33);
    int id = rd<int>(cr, ObId);
    const CrTrack* t = nullptr;
    for (auto& e : g_tr) if (e.id == id) t = &e;
    eline(now, "t=%lu E2 FALLBACK cr=%08x %s party=%d combat=%d round=%d script=%s a1spin=%d lastA1=%ldms", now, id, tag, party,
          rd<int>(cr, CrCombat), roundActive(cr), script[0] ? script : "-", t ? t->spinning : 0,
          (t && t->lastA1) ? long(now - t->lastA1) : -1L);
}

// E3. SpawnInHeartbeatPerception 0x56B8A0 block 2 at 0x56BDBD: UpdatePerception(1) is about to run for [ebp-0xD4].
extern "C" void __cdecl AiTracePerceptionMode1(int ebp) {
    DWORD now = GetTickCount(); tick(now);
    if (!g_cfg.trace || !g_cfg.enginePoints) return;
    int c = fr<int>(ebp, -0xD4);   // the engine passes it as `this` right after the window
    if (!c) return;
    ++g_st.e3;
    unsigned elapsed = fr<unsigned>(ebp, -0x28);
    int id = *reinterpret_cast<const int*>(c + ObId);
    if (!g_p1AggStart) g_p1AggStart = now;
    g_p1Agg.add(id, elapsed);
    if (g_p1Details < g_cfg.p1DetailLines && g_p1Details < 64) {
        for (int i = 0; i < g_p1Details; ++i) if (g_p1DetailIds[i] == id) return;
        g_p1DetailIds[g_p1Details++] = id;
        saferead::beginScope();
        char tag[33]; readTag(c, tag, sizeof tag);
        eline(now, "t=%lu E3 P1 detail cr=%08x %s elapsed=%u stamp=(%d,%d) interval=%d f38c=%d isPC=%d party=%d", now, id, tag, elapsed,
              *reinterpret_cast<const int*>(c + CrStampA), *reinterpret_cast<const int*>(c + CrStampB),
              *reinterpret_cast<const int*>(c + CrP1Interval), *reinterpret_cast<const int*>(c + CrP1Flag),
              *reinterpret_cast<const int*>(c + CrIsPC), *reinterpret_cast<const int*>(c + CrParty));
    }
}

// =====================================================================================================================
// Fixes
// =====================================================================================================================

// A1. AIActionPhysicalAttacks approach branch at 0x6D853E (after ranged-ai E-H3 wrote [ebp-0x174]). Return 1 =>
// 0x6D7D2F, the vanilla PC retarget tail: [ebp-0xE8] = new target (InitializeNewAttackTarget, return 1) or INVALID
// (EndAttackAction(1), return 2). Return 0 => vanilla (stolen MOV [ebp-0x16C],INVALID, resume 0x6D8548).
extern "C" int __cdecl CompanionAiA1Gate(int ebp) {
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    int eff = fr<int>(ebp, F_Stance);
    if (eff != 11 && eff != 12) return 0;                                         // not a spin: vanilla move follows
    if (fr<int>(ebp, F_Backoff) != 0 || fr<int>(ebp, F_ForceJump) != 0) return 0; // back-off / force jump: vanilla
    int attacker = fr<int>(ebp, F_Attacker);
    int id = 0;
    if (!attacker || !saferead::readAt<int>(5, static_cast<uintptr_t>(attacker), ObId, &id)) return 0;
    int ctrl = Invalid, p = fr<int>(ebp, F_PartyTable);
    if (p) {   // = vanilla 0x6D8548..0x6D856B (pure getter)
        int q = reinterpret_cast<Fn0>(FnControlled)(p);
        if (!q || !saferead::readAt<int>(5, static_cast<uintptr_t>(q), ObId, &ctrl)) return 0;   // unknown: vanilla
    }
    if (ctrl == id) return 0;                                                     // controlled: vanilla moves
    int raw = rawStance(attacker), party = rd<int>(attacker, CrParty);
    float dist2 = fr<float>(ebp, F_Dist2), mx = fr<float>(ebp, F_Max);
    int los = fr<int>(ebp, F_Los), ranged = fr<int>(ebp, F_Ranged), tgt = fr<int>(ebp, F_ActionTarget);
    CrTrack* t = track(id, now);
    ++t->wouldSpin; ++g_st.a1Would;
    bool apply = g_cfg.a1 && cai::inMask(g_cfg.a1Stances, raw) && (!g_cfg.a1PartyOnly || party);
    int newId = Invalid;
    if (apply) {
        if (g_cfg.a1Retarget && cai::inMask(g_cfg.a1RetargetStances, raw) && mx > 0.5f) {
            newId = reinterpret_cast<RetargetFn>(FnNearestEnemy)(attacker, mx, tgt, Invalid);
            if (!newId) newId = Invalid;
        }
        frw<int>(ebp, F_NewTarget, newId);
        if (newId == Invalid && g_cfg.a1RethinkMs < 3000) {
            int cur = rd<int>(attacker, CrFallback);
            if (cur > g_cfg.a1RethinkMs) wr<int>(attacker + CrFallback, g_cfg.a1RethinkMs);
        }
        if (newId != Invalid) { ++t->retargets; ++g_st.a1Retarget; } else { ++t->ends; ++g_st.a1End; }
        t->lastA1 = now;
    }
    if (g_cfg.log && crPasses(attacker, party)) {
        int tgtArea = fr<int>(ebp, F_TargetArea), myArea = reinterpret_cast<Fn0>(FnGetArea)(attacker);
        const char* reason = cai::a1Reason(ranged, dist2, mx, los, !tgtArea || myArea == tgtArea);
        unsigned sig = (apply ? 1u : 0u) | (newId != Invalid ? 2u : 0u) | (static_cast<unsigned>(reason[0]) << 8) | (static_cast<unsigned>(raw & 0xFF) << 16);
        if (t->a1Log.allow(now, sig, 10, 2000)) {
            float dist = dist2 > 0 ? __builtin_sqrtf(dist2) : 0.f;
            eline(now, "t=%lu A1 cr=%08x party=%d raw=%d eff=%d reason=%s dist=%.1f max=%.1f los=%d ranged=%d area=%s tgt=%08x -> %s new=%08x (wouldSpin=%u)",
                  now, id, party, raw, eff, reason, dist, mx, los, ranged, (!tgtArea || myArea == tgtArea) ? "same" : "other", tgt,
                  !apply ? (g_cfg.a1 ? "spin (not listed)" : "spin (a1=0)") : newId != Invalid ? "retarget" : "end", newId, t->wouldSpin);
        }
    }
    return apply ? 1 : 0;
}

// P1i. SpawnInHeartbeatPerception block 2 at 0x56BDB4 (non-party, non-IsPC, mode 1). The stolen CMP [ebp-0x28],0xFA0
// and the JC after it decide; this handler writes [ebp-0x28] (no later reader) = 0xFA0 to run or 0 to skip.
extern "C" void __cdecl CompanionAiP1Interval(int ebp) {
    DWORD now = GetTickCount(); tick(now);
    ++g_st.p1Checks;
    int c = fr<int>(ebp, -0xD4);
    if (!c) return;
    unsigned* stamp = reinterpret_cast<unsigned*>(c + CrStampA);   // the engine read both words at 0x56BCF9/0x56BD06
    if (!g_cfg.p1) {
        if (stamp[0] | stamp[1]) {   // stamped while p1 was on: restore the vanilla never-stamped state
            stamp[0] = stamp[1] = 0; frw<unsigned>(ebp, -0x28, 0xFA0); ++g_st.p1Resets;
        }
        return;
    }
    bool run = cai::p1ShouldRun(fr<unsigned>(ebp, -0x24), fr<unsigned>(ebp, -0x20), stamp[0], stamp[1], fr<unsigned>(ebp, -0x1C),
                                fr<unsigned>(ebp, -0x28), static_cast<unsigned>(g_cfg.p1IntervalMs));
    frw<unsigned>(ebp, -0x28, run ? 0xFA0u : 0u);
    if (run) ++g_st.p1Allowed; else ++g_st.p1Throttled;
}

// P1s. Block 2 at 0x56BDE3, right after UpdatePerception(1) ran: store the stamp (pairing as pushed at 0x56BD06/0x56BCF9).
extern "C" void __cdecl CompanionAiP1Stamp(int ebp) {
    ++g_st.p1Runs;
    if (!g_cfg.p1) return;
    int c = fr<int>(ebp, -0xD4);
    if (!c) return;
    *reinterpret_cast<unsigned*>(c + CrStampA) = fr<unsigned>(ebp, -0x24);
    *reinterpret_cast<unsigned*>(c + CrStampB) = fr<unsigned>(ebp, -0x20);
}

// FM1. BumpFriends 0x586B70 at 0x586F43 (skip-only: PUSH 1; MOV ECX,[EBP+8]; CALL ClearAllActions). The handler makes
// the vanilla call itself unless fm1 keeps a party bumpee's follow-only queue (the push-aside move is queued in front).
extern "C" void __cdecl CompanionAiBumpClear(int ebp) {
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    int bumpee = fr<int>(ebp, 8), mover = fr<int>(ebp, -0x158);
    int party = rd<int>(bumpee, CrParty);
    uint16_t types[64]; int n = 0; bool listOk = false;
    if (int list = rd<int>(bumpee, ObQueue)) {   // CExoLinkedList: [list] = head node {+0 prev, +4 next, +8 data}
        listOk = true;
        int node = rd<int>(list, 0);
        while (node && n < 64) {
            int data = 0, next = 0; unsigned short ty = 0;
            if (!saferead::readAt<int>(6, static_cast<uintptr_t>(node), 8, &data) || !data ||
                !saferead::readAt<unsigned short>(6, static_cast<uintptr_t>(data), 0, &ty) ||
                !saferead::readAt<int>(6, static_cast<uintptr_t>(node), 4, &next)) { listOk = false; break; }
            types[n++] = ty; node = next;
        }
        if (node) listOk = false;   // longer than 64: never keep
    }
    bool keep = g_cfg.fm1 && party && listOk && cai::onlyFollowTypes(types, n);
    bool show = (g_cfg.log || (g_cfg.trace && g_cfg.enginePoints)) && crPasses(bumpee, party);
    char q[64 * 5 + 8] = "";
    if (show) {
        int len = 0;
        for (int i = 0; i < n && i < 16 && len < int(sizeof q) - 8; ++i) len += snprintf(q + len, sizeof q - len, "%s%x", i ? "," : "", types[i]);
        if (n > 16) snprintf(q + len, sizeof q - len, ",+%d", n - 16);
    }
    if (keep) ++g_st.fm1Keep;
    else {
        reinterpret_cast<Fn1>(FnClearAllActions)(bumpee, 1);   // vanilla: same this, same argument
        ++g_st.fm1Clear;
        if (g_cfg.fm1 && party && g_cfg.fm1RethinkMs > 0 && rd<int>(bumpee, CrCombat) == 1 && rd<int>(bumpee, CrFallback) > g_cfg.fm1RethinkMs)
            wr<int>(bumpee + CrFallback, g_cfg.fm1RethinkMs);
    }
    if (show) {
        char bt[33], mt[33]; readTag(bumpee, bt, sizeof bt); readTag(mover, mt, sizeof mt);
        eline(now, "t=%lu FM1 bumpee=%08x %s party=%d mover=%08x %s combat=%d tgt=%08x round=%d queue=[%s]%s -> %s%s", now, rd<int>(bumpee, ObId), bt,
              party, rd<int>(mover, ObId), mt, rd<int>(bumpee, CrCombat), rd<int>(bumpee, CrTarget, Invalid), roundActive(bumpee), q,
              listOk ? "" : " (unreadable)", keep ? "keep" : "clear", (!keep && !g_cfg.fm1) ? " (fm1=0)" : "");
    }
}

// FM4 mode 1. SaveCreature 0x574500: snapshot the live FollowInfo before SetPartyMemberFlag(0,1) frees it (0x57451F,
// EAX = creature), restore it into the re-created one after SaveFollowInfo wrote the defaults (0x575183), so the save
// bytes stay vanilla and the live follow state survives.
namespace { unsigned char g_snap[FollowInfoSize]; int g_snapCr = 0; }

extern "C" void __cdecl CompanionAiFm4Snap(int cr) {
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    g_snapCr = 0;
    if (g_cfg.fm4 != 1 && !g_cfg.log) return;
    if (!rd<int>(cr, CrParty)) return;
    int fi = rd<int>(cr, CrFollowInfo);
    if (!fi || !saferead::probeRead(7, reinterpret_cast<const void*>(fi), FollowInfoSize)) return;
    memcpy(g_snap, reinterpret_cast<const void*>(fi), FollowInfoSize);
    g_snapCr = cr; ++g_st.fm4Snap;
}

extern "C" void __cdecl CompanionAiFm4Restore(int ebp) {
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    int cr = fr<int>(ebp, -0x1D4);
    if (!g_snapCr || g_snapCr != cr) { g_snapCr = 0; return; }
    g_snapCr = 0;
    int fi = rd<int>(cr, CrFollowInfo);
    unsigned char saved[FollowInfoSize];
    bool ok = fi && rd<int>(cr, CrParty) && saferead::probeRead(7, reinterpret_cast<const void*>(fi), FollowInfoSize);
    if (ok) memcpy(saved, reinterpret_cast<const void*>(fi), FollowInfoSize);
    bool restored = false;
    if (ok && g_cfg.fm4 == 1) { memcpy(reinterpret_cast<void*>(fi), g_snap, FollowInfoSize); restored = true; ++g_st.fm4Restore; }
    if (g_cfg.log) {
        auto I = [](const unsigned char* b, int o) { int v; memcpy(&v, b + o, 4); return v; };
        auto F = [](const unsigned char* b, int o) { float v; memcpy(&v, b + o, 4); return v; };
        char tag[33]; readTag(cr, tag, sizeof tag);
        eline(now, "t=%lu FM4 cr=%08x %s fi=%08x live: obj=%08x maxSpeed=%.2f safety=%d | saved: obj=%08x maxSpeed=%.2f safety=%d -> %s", now,
              rd<int>(cr, ObId), tag, fi, I(g_snap, 0), F(g_snap, 0x28), I(g_snap, 0x38), ok ? I(saved, 0) : -1, ok ? F(saved, 0x28) : 0.f,
              ok ? I(saved, 0x38) : -1, restored ? "restored (mode 1)" : ok ? "reset kept (fm4=0)" : "no FollowInfo after save");
    }
}

// A4. AIActionCombat 0x6D9500 at 0x6D9E2A (skip-only, 15 bytes incl. the store to [ebp-0x15C]). Return 1 => 0x6D9EA4
// (= vanilla's JZ when row+0x154 == 0), 0 => resume 0x6D9E39 (CMP/JZ re-tests [ebp-0x15C]).
extern "C" int __cdecl CompanionAiA4(int ebp) {
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    int row = fr<int>(ebp, -0x68);
    if (!g_cfg.a4) {   // exact vanilla semantics, including the fault on a NULL row
        if (!row) {
            eline(now, "t=%lu A4 NULL spell row: cr=%08x spell=%d -> vanilla crash (a4=0)", now, rd<int>(fr<int>(ebp, -0x1AC), ObId), fr<int>(ebp, -0x58));
            if (g_log) fflush(g_log);
        }
        int v = *reinterpret_cast<volatile const int*>(row + 0x154);
        frw<int>(ebp, -0x15C, v);
        return v == 0 ? 1 : 0;
    }
    int v = 0;
    if (!row || !saferead::readAt<int>(8, static_cast<uintptr_t>(row), 0x154, &v)) {
        ++g_st.a4;
        frw<int>(ebp, -0x15C, 0);
        eline(now, "t=%lu A4 %s spell row %08x: cr=%08x spell=%d -> skip (not-hostile path 0x6D9EA4)", now, row ? "unreadable" : "NULL", row,
              rd<int>(fr<int>(ebp, -0x1AC), ObId), fr<int>(ebp, -0x58));
        return 1;
    }
    frw<int>(ebp, -0x15C, v);
    return v == 0 ? 1 : 0;
}

// C1. ClearHostileActionsVersus 0x5867E0, after each group delete (0x5868B9 / 0x5868F1 / 0x586929, skip-only):
// vanilla stores i=0 and the loop's ++ skips the group that slid into slot 0; -1 re-examines it.
namespace { cai::RestartCap g_c1Cap; }
extern "C" void __cdecl CompanionAiC1(int ebp) {
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    int v = 0;
    if (g_cfg.c1) {
        int self = fr<int>(ebp, -0x2C);
        bool who = !g_cfg.c1PartyOnly || (rd<unsigned char>(self, ObType, 0xFF) == ObjTypeCreature && rd<int>(self, CrParty) != 0);
        if (who) {
            if (g_c1Cap.allow(self, ebp, now, 256)) v = -1;
            else ++g_st.c1Capped;
        }
    }
    frw<int>(ebp, -0x10, v);
    if (v) ++g_st.c1Restart; else ++g_st.c1Vanilla;
}

// A3 (optional, a3=0). AIActionPhysicalAttacks at 0x6D78B7 (MOV [ebp-0x3C],0 runs after): when [ebp-0x30]==0 the
// action returns 2 without EndAttackAction, leaving the orientation lock on the dead/undetected target. Clear the lock
// the way EndAttackAction does (0x6DAB36: 0x57D370(INVALID, 0)), without its combat-mode/animation side effects.
namespace { cai::LineThrottle g_a3Log; }
extern "C" void __cdecl CompanionAiA3(int ebp) {
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    if (!g_cfg.a3 || fr<int>(ebp, F_DeadExitValid) != 0) return;
    int attacker = fr<int>(ebp, F_Attacker);
    int party = rd<int>(attacker, CrParty), id = rd<int>(attacker, ObId, Invalid);
    if (!party || id == Invalid) return;
    int ctrl;
    if (!controlledId(&ctrl) || ctrl == id) return;
    reinterpret_cast<Fn2>(FnSetLock)(attacker, Invalid, 0);
    ++g_st.a3;
    if (g_cfg.log && g_a3Log.allow(now, static_cast<unsigned>(id), 20, 5000))
        eline(now, "t=%lu A3 cr=%08x dead/undetected target %08x: orientation lock cleared", now, id, fr<int>(ebp, F_ActionTarget));
}

// FM3 probe (log only, fm3_probe=0). AI scheduler 0x51D790 at 0x51DBE7: object [ebp-0x28], sentinel [ebp-0x3C],
// level [ebp-0x34], first-of-level flag [ebp-0x38] (1 for the object that just became the sentinel).
namespace { cai::Fm3Pass g_fm3; int g_fm3Lines = 0; }
extern "C" void __cdecl CompanionAiFm3Probe(int ebp) {
    if (!g_cfg.fm3Probe) return;   // hottest loop of the game: nothing else when off
    DWORD now = GetTickCount(); tick(now);
    int obj = fr<int>(ebp, -0x28), sentinel = fr<int>(ebp, -0x3C), level = fr<int>(ebp, -0x34), first = fr<int>(ebp, -0x38);
    if (first == 1) {
        if (g_fm3.active) {
            ++g_st.fm3Passes;
            if (g_fm3.repeats) {
                ++g_st.fm3RepeatPasses;
                if (static_cast<unsigned>(g_fm3.repeats) > g_st.fm3MaxRepeats) g_st.fm3MaxRepeats = static_cast<unsigned>(g_fm3.repeats);
                if (g_fm3Lines < 200) {
                    ++g_fm3Lines;
                    eline(now, "t=%lu FM3 pass level=%d sentinel=%08x objects=%d distinct=%d repeats=%d", now, g_fm3.level, g_fm3.sentinel,
                          g_fm3.objects, g_fm3.distinct, g_fm3.repeats);
                }
            }
        }
        g_fm3.begin(level, sentinel);
    }
    if (g_fm3.active && obj) g_fm3.object(obj);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        InitializeCriticalSection(&g_cs);
        char cwd[MAX_PATH] = "";
        if (GetCurrentDirectoryA(MAX_PATH, cwd) > 0) {
            snprintf(g_iniPath, sizeof g_iniPath, "%s\\companion_ai.ini", cwd);
            snprintf(g_logPath, sizeof g_logPath, "%s\\ai_trace.txt", cwd);
        }
        g_log = fopen(g_logPath, "w");   // one session per file
        if (g_log) setvbuf(g_log, nullptr, _IOFBF, 64 * 1024);
        DWORD now = GetTickCount();
        time_t tt = time(nullptr); struct tm* lt = localtime(&tt);
        char when[32] = "?";
        if (lt) strftime(when, sizeof when, "%Y-%m-%d %H:%M:%S", lt);
        eline(now, "# companion-ai %s session %s t=%lu pid=%lu cwd=%s ini=%s", Version, when, now, (unsigned long)GetCurrentProcessId(), cwd, g_iniPath);
        loadIni(now, true);
        g_lastSummary = now;
        g_ready = true;
    } else if (reason == DLL_PROCESS_DETACH) {
        if (g_log) fflush(g_log);
    }
    return TRUE;
}
