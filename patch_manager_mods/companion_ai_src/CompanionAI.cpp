// Companion AI (Steam Aspyr build 6A522E71...): party-AI trace layer + engine AI fixes, one DLL,
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
//   (FM3 hot sentinel probe 0x51DBE7: designed and validated, NOT shipped -- wrapper cost in the hottest loop; the cold FM3 sites below ship)
//  Phase-4 probes (0.2.0-probe; log only, plain detours, INI probe_<name>, "PR"/"PRS" lines; engine_ai_atlas/PROBE_DESIGN.md):
//   FM2C 0x55B69A CompanionAiProbeFm2c(ebp)            PlotPathInArea result 2 (partial) vs 3 (failed)
//   FM2F 0x5C2438 CompanionAiProbeFm2Fail(ebp)         MoveToPoint FAIL leg
//   FM2R 0x5BC5F4 CompanionAiProbeFm2Reapproach(ebp)   action 0x11 re-approach
//   FM3S 0x51DBE1 CompanionAiProbeFm3Sentinel(ebp)     UpdateState sentinel set (cold)
//   FM3E 0x51DD33 CompanionAiProbeFm3LevelEnd(ebp)     UpdateState level end (cold)
//   FM3R 0x51E0C9 CompanionAiProbeFm3Remove(ebp)       RemoveObject (cold)
//   RS   0x6FD8D0 CompanionAiProbeRunScript(ret, slot, self)   RunScript entry, sites by return address (hot-ish)
//   P2   0x56BE79 CompanionAiProbeP2(ebp)              party perception-0 cadence (HOT)
//   PF   0x58A35E CompanionAiProbePartyFlag(ebp)       SetPartyMemberFlag store of +0x11AC
//   RAR  0x5D6A60 CompanionAiProbeRunActions(ebp)      RunActions per-action result (HOT)
//   SHX  0x5475E8 CompanionAiProbeShout(ebp)           shout delivery single exit
//   SS1  0x59D966 CompanionAiProbeSetState(ebp)        OnApplySetState commit
//   GCA  0x67B204 CompanionAiProbeGetCurrentAction(ebp) GetCurrentAction (cmd 522)
//   PARTY / A1X: no window (tick + PF / A1 gate + E2 + RS)
//
// Hot handlers (E1, E3, P1i/P1s) read the live frame and the creature the engine is about to dereference
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
constexpr const char* Version = "0.2.0-probe";

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
               "fm1=%d fm1_rethink_ms=%d fm4=%d%s a4=%d c1=%d c1_party_only=%d a3=%d log=%d",
          c.a1, c.a1Stances, c.a1PartyOnly, c.a1Retarget, c.a1RetargetStances, c.a1RethinkMs, c.p1, c.p1IntervalMs, c.fm1, c.fm1RethinkMs,
          c.fm4, c.fm4Requested > 1 ? " (requested mode not built: vanilla)" : "", c.a4, c.c1, c.c1PartyOnly, c.a3, c.log);
    eline(now, "# config probes fm2c=%d fm2f=%d fm2r=%d fm3=%d rs=%d p2=%d pf=%d rar=%d shx=%d ss1=%d gca=%d party=%d a1x=%d lines=%d", c.probeFm2c,
          c.probeFm2f, c.probeFm2r, c.probeFm3, c.probeRs, c.probeP2, c.probePf, c.probeRar, c.probeShx, c.probeSs1, c.probeGca, c.probeParty,
          c.probeA1x, c.probeLines);
}

void applyGate() {
    g_gate.configure(static_cast<unsigned>(g_cfg.maxLinesPerSec), static_cast<unsigned>(g_cfg.dedupeMs),
                     static_cast<unsigned long long>(g_cfg.maxFileMb) << 20);
}

void loadIni(DWORD now, bool force) {
    if (!cai::reloadDue(force, now, g_lastIniCheck)) return;
    Lock lk;
    g_lastIniCheck = now;
    WIN32_FILE_ATTRIBUTE_DATA fad;
    bool exists = GetFileAttributesExA(g_iniPath, GetFileExInfoStandard, &fad) != 0;
    bool unchanged = exists && CompareFileTime(&fad.ftLastWriteTime, &g_iniTime) == 0 && fad.nFileSizeLow == g_iniSize;
    switch (cai::reloadPre(force, exists, g_iniSeen, unchanged)) {
    case cai::ReloadPre::Skip: return;
    case cai::ReloadPre::MissingDefaultsLog:
        g_cfg = cai::Config(); applyGate();
        eline(now, "# config: %s not found; using the built-in defaults (= shipped values)", g_iniPath);
        logConfig(now, "defaults");
        g_iniSeen = false; g_iniSize = ~0u;
        return;
    case cai::ReloadPre::MissingKeepLog:
        eline(now, "# config: %s not found; using the last config", g_iniPath);
        g_iniSeen = false; g_iniSize = ~0u;
        return;
    case cai::ReloadPre::MissingSilent:
        g_iniSeen = false; g_iniSize = ~0u;
        return;
    case cai::ReloadPre::Read: break;
    }
    FILE* f = fopen(g_iniPath, "rb");
    size_t n = 0;
    if (f) { n = fread(g_iniText, 1, sizeof g_iniText - 1, f); fclose(f); }
    g_iniText[n] = 0;
    if (f) { g_iniTime = fad.ftLastWriteTime; g_iniSize = fad.nFileSizeLow; g_iniSeen = true; g_ini.parse(g_iniText, "CompanionAI"); }
    switch (cai::reloadPost(force, f != nullptr, f && g_ini.sectionFound)) {
    case cai::ReloadPost::RetryLater: g_iniSeen = false; return;   // e.g. an editor holding it: keep the config, retry next second
    case cai::ReloadPost::KeepBadSection:                          // half-written or wrong file: keep it until it changes again
        eline(now, "# config: no [CompanionAI] section in %s; keeping the current config", g_iniPath);
        return;
    case cai::ReloadPost::Apply: break;
    }
    cai::Config c;
    cai::parseConfig(g_ini, c);
    bool changed = force || !cai::sameConfig(c, g_cfg);
    g_cfg = c;
    applyGate();
    if (!g_ini.sectionFound) eline(now, "# config: no [CompanionAI] section in %s; built-in defaults", g_iniPath);
    if (g_ini.bad[0]) eline(now, "# config: unparseable value for: %s (flags -> 0/off, numbers -> default)", g_ini.bad);
    if (changed) logConfig(now, force ? "load" : "reload");
}

// ---- object info (script OBJECT_SELF), cached 1 s per id
struct ObjInfo { int id = 0; int type = -1; int party = 0; int ispc = 0; char tag[33] = "-"; DWORD tick = 0; };
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
        if (o->type == ObjTypeCreature) { o->party = rd<int>(obj, CrParty); o->ispc = rd<int>(obj, CrIsPC); }
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
    DWORD pendingA1 = 0;   // A1X: tick of the last A1 end/retarget with no slot-6 follow-up yet (0 = none)
    cai::LineThrottle strictLog; unsigned strictN = 0, strictRej = 0;   // A1X STRICT: counts per summary window, throttled line (review SF2)
};
CrTrack g_tr[32];
// review N6: with tags=/log_all=1 big fights must not evict the party's tracks (A1 throttle, would-spin counts):
// evict the least recently seen non-party track first
CrTrack* track(int id, DWORD now) {
    CrTrack* victim = nullptr;
    for (auto& t : g_tr) {
        if (t.id == id) { t.seen = now; return &t; }
        if (!t.id) { if (!victim || victim->id) victim = &t; continue; }
        if (victim && !victim->id) continue;
        if (!victim || (victim->party && !t.party) || (!!victim->party == !!t.party && now - t.seen > now - victim->seen)) victim = &t;
    }
    *victim = CrTrack(); victim->id = id; victim->seen = now; victim->secStart = now;
    return victim;
}
cai::SubjectThrottle g_subj;   // per-creature throttle for E2 / FM1 / A4 lines (review N1)
enum SubjKind { SkE2 = 1, SkFm1 = 2, SkA4 = 3 };
DWORD g_loadTid = 0;
bool g_t1TidLogged = false;

// ---- E3 aggregation
int g_p1iCr = 0; unsigned g_p1iElapsed = 0;   // the engine's elapsed for the creature P1i just rewrote
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
                   "fm4{snap=%u restore=%u} a4=%u c1{restart=%u vanilla=%u capped=%u} a3=%u",
              now, g_gate.totalLines, g_gate.totalDropped, g_gate.totalDeduped, g_gate.totalCapDropped, s.flips, s.prints, s.aur, s.e1,
              s.e1spins, s.e1b, s.e2, s.e3, s.a1Would, s.a1End, s.a1Retarget, s.p1Checks, s.p1Allowed, s.p1Throttled, s.p1Runs, s.p1Resets,
              s.fm1Keep, s.fm1Clear, s.fm4Snap, s.fm4Restore, s.a4, s.c1Restart, s.c1Vanilla, s.c1Capped, s.a3);
        g_stLast = s;
    }
    for (auto& t : g_tr)
        if (t.id && (t.wouldSpin || t.ends || t.retargets)) {
            eline(now, "t=%lu A1 summary: cr=%08x would-spin passes=%u/%ds ends=%u retargets=%u strict=%u/%u", now, t.id, t.wouldSpin, g_cfg.summarySec,
                  t.ends, t.retargets, t.strictN, t.strictRej);
            t.wouldSpin = t.ends = t.retargets = t.strictN = t.strictRej = 0;
        }
    g_sumLines = g_gate.totalLines; g_sumDropped = g_gate.totalDropped; g_sumDeduped = g_gate.totalDeduped;
}

// =====================================================================================================================
// Phase-4 probes (engine_ai_atlas/PROBE_DESIGN.md): log only, plain detours, behaviour stays vanilla. "PR" = event line
// (per-second cap, probe_lines budget per probe), "PRS" = summary every summary_sec (only when something changed).
// =====================================================================================================================
constexpr int FnPartyTable = 0x0051C8D0;   // thiscall(serverApp) RET -> *(serverApp+4)+0x1F0B4: [+0] count, +8+4i npc idx, +0x1C+4*idx ids
constexpr int FnPlayerId = 0x0051C8F0;     // thiscall(serverApp) RET -> id of the controlled leader (player+0x28), 0x7F000000 if none
constexpr int CrPos = 0x94, CrMoving = 0x11BC, CrCombatExit = 0x524, CrSetState = 0xFF5, CrListenerSpeaker = 0x1A0, CrListenerPattern = 0x1BC;

enum PId { PiFm2c, PiFm2f, PiFm2r, PiFm3, PiRs, PiP2, PiPf, PiRar, PiShx, PiSs1, PiGca, PiParty, PiA1x, PiA1xF, PiCount };
const char* const PNames[PiCount] = {"FM2C", "FM2F", "FM2R", "FM3", "RS", "P2", "PF", "RAR", "SHX", "SS1", "GCA", "PARTY", "A1X", "A1XF"};
cai::ProbeCap g_pcap[PiCount];
enum PSubj { PkFm2f = 1 };
cai::SubjectThrottle g_psubj;      // per-subject throttles of the probe lines (own table: never evicts E2/FM1/A4 state)
int g_pcSeen = 0;                  // last creature id seen with +0x10E8 == 1 (RAR / P2 / RS / PARTY)

// One probe line: the probe's session budget first, then the shared gate (tline: per-second cap; pev: event line).
bool pbudget(int pid, DWORD now) {
    if (g_pcap[pid].take(static_cast<unsigned>(g_cfg.probeLines))) return true;
    if (g_pcap[pid].marker()) eline(now, "-- probe %s line budget reached --", PNames[pid]);
    return false;
}
void pline(int pid, DWORD now, const char* fmt, ...) {
    if (!pbudget(pid, now)) return;
    va_list ap; va_start(ap, fmt); vemit(now, 0, false, fmt, ap); va_end(ap);
}
void pev(int pid, DWORD now, const char* fmt, ...) {
    if (!pbudget(pid, now)) return;
    va_list ap; va_start(ap, fmt); vemit(now, 0, true, fmt, ap); va_end(ap);
}

// ---- FM2: per (creature, target) pair counters (FM2C 30 s, FM2F / FM2R 12 s)
struct Fm2Pair {
    int cr = 0, tgt = 0; DWORD seen = 0;
    cai::PairCount c2, c3, fl, ra;
    cai::LineThrottle lt; unsigned rlogged = 0;
};
Fm2Pair g_fm2[16];
Fm2Pair& fm2Pair(int cr, int tgt, DWORD now) {
    Fm2Pair* v = &g_fm2[0];
    for (auto& e : g_fm2) {
        if (e.seen && e.cr == cr && e.tgt == tgt) { e.seen = now ? now : 1; return e; }
        if (!e.seen) v = &e; else if (v->seen && now - e.seen > now - v->seen) v = &e;
    }
    *v = Fm2Pair(); v->cr = cr; v->tgt = tgt; v->seen = now ? now : 1;
    return *v;
}

// ---- FM3: scheduler wrap sentinel (log only; never writes [ebp-0x40] or the cursor)
struct Fm3State { bool active = false; int level = -1; int sentinel = 0; bool removed = false; };
Fm3State g3;
struct Fm3Cnt { unsigned passes = 0, lost = 0, removes = 0, beforeCursor = 0; } g_fm3c, g_fm3cLast;
unsigned g_fm3LLines = 0;
cai::LineThrottle g_fm3xLog;

// ---- RS: RunScript return-address sites per OBJECT_SELF
struct RsTrack {
    int id = 0; DWORD seen = 0;
    unsigned n[7] = {}, nLast[7] = {}, logged[7] = {};
    DWORD sec = 0; unsigned cur12 = 0, max12 = 0;
};
RsTrack g_rs[32];
unsigned g_rsS7Lines = 0, g_rsS7E9Lines = 0, g_e9 = 0, g_e9Last = ~0u;   // g_e9: pat-15 deliveries from non-party creatures to party listeners (budget-free, PRS E9)
RsTrack* rsTrack(int id, DWORD now) {
    RsTrack* v = &g_rs[0];
    for (auto& e : g_rs) {
        if (e.seen && e.id == id) { e.seen = now ? now : 1; return &e; }
        if (!e.seen) v = &e; else if (v->seen && now - e.seen > now - v->seen) v = &e;
    }
    *v = RsTrack(); v->id = id; v->seen = now ? now : 1; v->sec = now;
    return v;
}

// ---- P2: real perception-0 cadence per creature
struct P2Track { int id = 0; DWORD seen = 0, last = 0; unsigned runs = 0, runsLast = 0, gaps = 0, sumGap = 0, maxGap = 0, ctrlRuns = 0, logged = 0; };
P2Track g_p2[16];
P2Track* p2Track(int id, DWORD now) {
    P2Track* v = &g_p2[0];
    for (auto& e : g_p2) {
        if (e.seen && e.id == id) { e.seen = now ? now : 1; return &e; }
        if (!e.seen) v = &e; else if (v->seen && now - e.seen > now - v->seen) v = &e;
    }
    *v = P2Track(); v->id = id; v->seen = now ? now : 1;
    return v;
}

// ---- PF: who writes +0x11AC
struct PfCnt { unsigned saveA = 0, saveB = 0, xferA = 0, xferB = 0, other = 0; } g_pfc, g_pfcLast;
bool g_partyPf = false;            // a non-transient PF write happened: PARTY snapshot due at the next tick

// ---- RAR: action histogram per creature (type x result)
struct RarTrack {
    int id = 0; DWORD seen = 0; int ispc = 0; int lastType = 0, runType = -1;
    unsigned n = 0, nLast = 0, run = 0, moveLogged = 0, reqLogged = 0;
    unsigned cnt[cai::RarBuckets] = {};
};
RarTrack g_rar[16];
RarTrack* rarTrack(int id, DWORD now) {
    static RarTrack* mru = nullptr;
    if (mru && mru->seen && mru->id == id) { mru->seen = now ? now : 1; return mru; }
    RarTrack* v = &g_rar[0];
    for (auto& e : g_rar) {
        if (e.seen && e.id == id) { e.seen = now ? now : 1; return mru = &e; }
        if (!e.seen) v = &e; else if (v->seen && now - e.seen > now - v->seen) v = &e;
    }
    *v = RarTrack(); v->id = id; v->seen = now ? now : 1;
    return mru = v;
}
// "type:r1/r2/r3/r4" (+"+oN" for other results) of the 8 most frequent types
void rarFormat(const RarTrack& t, char* out, int cap) {
    out[0] = 0;
    unsigned tot[cai::RarTypes]; bool used[cai::RarTypes] = {};
    for (int ty = 0; ty < cai::RarTypes; ++ty) { tot[ty] = 0; for (int r = 0; r < 5; ++r) tot[ty] += t.cnt[ty * 5 + r]; }
    int len = 0;
    for (int k = 0; k < 8; ++k) {
        int best = -1;
        for (int ty = 0; ty < cai::RarTypes; ++ty) if (!used[ty] && tot[ty] && (best < 0 || tot[ty] > tot[best])) best = ty;
        if (best < 0 || len >= cap - 48) break;
        used[best] = true;
        const unsigned* c = &t.cnt[best * 5];
        len += snprintf(out + len, cap - len, "%s%x:%u/%u/%u/%u", k ? " " : "", best, c[1], c[2], c[3], c[4]);
        if (c[0]) len += snprintf(out + len, cap - len, "+o%u", c[0]);
    }
    if (!used[0] && tot[0] && len < cap - 48) {   // the type-0 bucket (PR-RAR FAIL input) is always printed (review N7)
        const unsigned* c = &t.cnt[0];
        len += snprintf(out + len, cap - len, "%s0:%u/%u/%u/%u", len ? " " : "", c[1], c[2], c[3], c[4]);
        if (c[0]) len += snprintf(out + len, cap - len, "+o%u", c[0]);
    }
}

// ---- SHX: shout delivery exits
struct ShxCnt { unsigned shouts = 0, r35 = 0, partySpk = 0, end = 0, xbreak = 0, noarea = 0, abortStartCr = 0, abortStartOther = 0, abortMid = 0; } g_shx, g_shxLast;
unsigned g_shxLines = 0, g_ss1Lines = 0;

// ---- GCA: GetCurrentAction results per creature
struct GcaTrack {
    int id = 0; DWORD seen = 0; int lastRes = 0x7FFFFFFF; unsigned logged = 0, total = 0, totalLast = 0; int nres = 0; int res[6] = {}; unsigned cnt[6] = {};
};
GcaTrack g_gca[16];
GcaTrack* gcaTrack(int id, DWORD now) {
    GcaTrack* v = &g_gca[0];
    for (auto& e : g_gca) {
        if (e.seen && e.id == id) { e.seen = now ? now : 1; return &e; }
        if (!e.seen) v = &e; else if (v->seen && now - e.seen > now - v->seen) v = &e;
    }
    *v = GcaTrack(); v->id = id; v->seen = now ? now : 1;
    return v;
}
int rarLastType(int id) { for (auto& e : g_rar) if (e.seen && e.id == id) return e.lastType; return 0; }

// ---- PARTY: party flag / controlled-creature snapshot
unsigned g_partyLines = 0;
DWORD g_lastPartyChk = 0, g_lastOrphan = 0;
int g_lastCtrl = 0, g_lastCtrlArea = 0;
bool g_partyLoaded = false;

void partyEmit(DWORD now, const char* why, int ctrl) {
    if (g_partyLines >= 100) return;
    int app = rd<int>(G_App, 0), server = app ? rd<int>(app, 8) : 0;
    if (!server || !rd<int>(server, 4)) return;   // 0x51C620 (inside both getters) dereferences [server+4] (review N5)
    int tbl = reinterpret_cast<Fn0>(FnPartyTable)(server);
    int pid = reinterpret_cast<Fn0>(FnPlayerId)(server);
    int cobj = ctrl != Invalid ? gameObject(ctrl) : 0, pobj = pid != Invalid ? gameObject(pid) : 0;
    int count = rd<int>(tbl, 0, -1);
    char mem[300] = ""; int len = 0;
    for (int i = 0; i < count && i < 12 && len < int(sizeof mem) - 48; ++i) {
        int idx = rd<int>(tbl, 8 + 4 * i, -1);
        int id = (idx >= 0 && idx < 64) ? rd<int>(tbl, 0x1C + 4 * idx, 0) : 0;
        int o = id ? gameObject(id) : 0;
        int ispc = o ? rd<int>(o, CrIsPC, -1) : -1;
        if (ispc == 1) g_pcSeen = id;
        len += snprintf(mem + len, sizeof mem - len, "%s%d:%08x:%d:%d", i ? "," : "", idx, id, o ? rd<int>(o, CrParty, -1) : -1, ispc);
    }
    if (cobj && rd<int>(cobj, CrIsPC) == 1) g_pcSeen = ctrl;
    ++g_partyLines;
    pline(PiParty, now, "t=%lu PR PARTY why=%s ctrl=%08x f=%d/%d pobj=%08x f=%d/%d pc=%08x tbl=%d [%s]", now, why, ctrl,
          cobj ? rd<int>(cobj, CrParty, -1) : -1, cobj ? rd<int>(cobj, CrIsPC, -1) : -1, pid,
          pobj ? rd<int>(pobj, CrParty, -1) : -1, pobj ? rd<int>(pobj, CrIsPC, -1) : -1, g_pcSeen, count, mem);
}
// Triggers (at most one line each): the first tick with a controlled creature (load), a controlled-id change (leader), an area
// change of the controlled creature (area, checked at <= 1 Hz) and a non-transient PF write (pf).
void partyTick(DWORD now) {
    bool pf = g_partyPf;
    g_partyPf = false;
    if (!pf && now - g_lastPartyChk < 1000) return;
    g_lastPartyChk = now;
    saferead::beginScope();
    int ctrl = Invalid;
    bool have = controlledId(&ctrl) && ctrl != Invalid;
    if (!have && !pf) return;
    int cobj = have ? gameObject(ctrl) : 0;
    int area = cobj ? reinterpret_cast<Fn0>(FnGetArea)(cobj) : 0;
    const char* why = nullptr;
    if (have && !g_partyLoaded) why = "load";
    else if (have && g_lastCtrl && ctrl != g_lastCtrl) why = "leader";
    else if (have && area && g_lastCtrlArea && area != g_lastCtrlArea) why = "area";
    else if (pf) why = "pf";
    if (have) { g_partyLoaded = true; g_lastCtrl = ctrl; g_lastCtrlArea = area; }
    if (why) partyEmit(now, why, have ? ctrl : Invalid);
}

// ---- A1X: follow-up after an A1 end/retarget (a slot-6 run: RS S6A/S6B/S6C or E2) and orphans
void a1xFollowup(int id, const char* site, DWORD now) {
    if (!g_cfg.probeA1x || !id) return;
    for (auto& t : g_tr)
        if (t.id == id) {
            if (t.pendingA1) {
                long after = long(now - t.pendingA1);
                t.pendingA1 = 0;
                pev(PiA1xF, now, "t=%lu PR A1X FOLLOWUP cr=%08x site=%s after=%ldms", now, id, site, after);
            }
            return;
        }
}
void orphanTick(DWORD now) {
    g_lastOrphan = now;
    for (auto& t : g_tr)
        if (t.id && t.pendingA1 && cai::a1OrphanDue(now, t.pendingA1, g_cfg.a1RethinkMs)) {
            long since = long(now - t.pendingA1);
            t.pendingA1 = 0;
            pev(PiA1xF, now, "t=%lu PR A1X ORPHAN cr=%08x since=%ldms", now, t.id, since);
        }
}

// PRS summaries, called from the periodic block of tick (every summary_sec): only entries that changed since the last report.
void probeSummary(DWORD now) {
    if (memcmp(&g_fm3c, &g_fm3cLast, sizeof g_fm3c)) {
        eline(now, "t=%lu PRS FM3 passes=%u lost=%u removes=%u beforeCursor=%u", now, g_fm3c.passes, g_fm3c.lost, g_fm3c.removes, g_fm3c.beforeCursor);
        g_fm3cLast = g_fm3c;
    }
    for (auto& t : g_rs)
        if (t.seen && memcmp(t.n, t.nLast, sizeof t.n)) {
            eline(now, "t=%lu PRS RS cr=%08x s6a=%u s6b=%u s6c=%u s7=%u s12=%u s1=%u max12ps=%u", now, t.id, t.n[1], t.n[2], t.n[3], t.n[4], t.n[5], t.n[6], t.max12);
            memcpy(t.nLast, t.n, sizeof t.n);
        }
    if (g_cfg.probeRs && g_e9 != g_e9Last) {   // first summary prints deliveries=0 too: absence of a line then means no data, not zero
        eline(now, "t=%lu PRS E9 deliveries=%u", now, g_e9);
        g_e9Last = g_e9;
    }
    for (auto& t : g_p2)
        if (t.seen && t.runs != t.runsLast) {
            eline(now, "t=%lu PRS P2 cr=%08x runs=%u avg=%u max=%u ctrlRuns=%u", now, t.id, t.runs, t.gaps ? t.sumGap / t.gaps : 0, t.maxGap, t.ctrlRuns);
            t.runsLast = t.runs;
        }
    if (memcmp(&g_pfc, &g_pfcLast, sizeof g_pfc)) {
        eline(now, "t=%lu PRS PF saveA=%u saveB=%u xferA=%u xferB=%u other=%u", now, g_pfc.saveA, g_pfc.saveB, g_pfc.xferA, g_pfc.xferB, g_pfc.other);
        g_pfcLast = g_pfc;
    }
    for (auto& t : g_rar)
        if (t.seen && t.n != t.nLast) {
            char h[400]; rarFormat(t, h, sizeof h);
            eline(now, "t=%lu PRS RAR cr=%08x ispc=%d n=%u %s", now, t.id, t.ispc, t.n, h);
            t.nLast = t.n;
        }
    if (memcmp(&g_shx, &g_shxLast, sizeof g_shx)) {
        eline(now, "t=%lu PRS SHX shouts=%u r35=%u partySpk=%u end=%u xbreak=%u noarea=%u abortStartCr=%u abortStartOther=%u abortMid=%u", now, g_shx.shouts,
              g_shx.r35, g_shx.partySpk, g_shx.end, g_shx.xbreak, g_shx.noarea, g_shx.abortStartCr, g_shx.abortStartOther, g_shx.abortMid);
        g_shxLast = g_shx;
    }
    for (auto& t : g_gca)
        if (t.seen && t.total != t.totalLast) {
            char h[160]; int len = 0; h[0] = 0;
            for (int i = 0; i < t.nres && len < int(sizeof h) - 24; ++i) len += snprintf(h + len, sizeof h - len, "%sr%d:%u", i ? " " : "", t.res[i], t.cnt[i]);
            eline(now, "t=%lu PRS GCA cr=%08x %s", now, t.id, h);
            t.totalLast = t.total;
        }
}

// Called first by every handler: INI reload (1 Hz), periodic summaries, buffered-log flush.
inline void tick(DWORD now) {
    if (!g_ready) return;
    if (now - g_lastIniCheck >= 1000) loadIni(now, false);
    if (now - g_lastSummary >= static_cast<DWORD>(g_cfg.summarySec) * 1000u) {
        g_lastSummary = now;
        summary(now);
        if (g_cfg.trace && g_cfg.enginePoints && !g_cfg.engineVerbose) reportP1(now);
        probeSummary(now);
    }
    if (g_cfg.trace && g_cfg.enginePoints && g_cfg.engineVerbose && now - g_lastP1Report >= 1000) { g_lastP1Report = now; reportP1(now); }
    if (g_cfg.probeParty && (g_partyPf || now - g_lastPartyChk >= 1000)) partyTick(now);
    if (g_cfg.probeA1x && now - g_lastOrphan >= 250) orphanTick(now);
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
    if (!g_t1TidLogged) {   // review N8: the VM, AI and save hooks are assumed to run on the main thread
        g_t1TidLogged = true;
        eline(now, "# T1 first ShipBuild flip: tid=%lu (load tid=%lu) script=%s", (unsigned long)GetCurrentThreadId(), (unsigned long)g_loadTid, c.script);
    }
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
    if (g_cfg.probeA1x) a1xFollowup(rd<int>(fr<int>(ebp, -0x194), ObId), "E2", now);   // A1X: slot-6 run seen (RS S6B follows)
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
    if (!g_subj.allow(SkE2, id, now, 0, 20, 10000)) return;   // first 20 per creature, then one per 10 s (S line counts all)
    tline(now, 0, "t=%lu E2 FALLBACK cr=%08x %s party=%d combat=%d round=%d script=%s a1spin=%d lastA1=%ldms", now, id, tag, party,
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
    if (c == g_p1iCr) elapsed = g_p1iElapsed;   // p1=1: P1i replaced [ebp-0x28] with 0xFA0/0 (review N4)
    g_p1iCr = 0;
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
    if (!cai::a1SpinCandidate(eff, fr<int>(ebp, F_Backoff), fr<int>(ebp, F_ForceJump))) return 0;   // vanilla move / back-off / jump
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
    bool apply = cai::a1Applies(g_cfg, raw, party);
    int newId = Invalid;
    if (apply) {
        if (cai::a1TryRetarget(g_cfg, raw, mx))
            newId = cai::a1NewTarget(reinterpret_cast<RetargetFn>(FnNearestEnemy)(attacker, mx, tgt, Invalid));
        frw<int>(ebp, F_NewTarget, newId);
        if (cai::a1RethinkWanted(g_cfg, newId) && cai::a1RethinkWrite(g_cfg, rd<int>(attacker, CrFallback)))
            wr<int>(attacker + CrFallback, g_cfg.a1RethinkMs);
        if (newId != Invalid) { ++t->retargets; ++g_st.a1Retarget; } else { ++t->ends; ++g_st.a1End; }
        t->lastA1 = now;
        if (g_cfg.probeA1x) {   // log only: strict 2D test of the retarget candidate, and the follow-up watch (RS S6*/E2 clear it)
            t->pendingA1 = now ? now : 1;
            if (newId != Invalid && crPasses(attacker, party)) {
                float ax = 0, ay = 0, tx = 0, ty = 0;
                int cand = gameObject(newId);
                if (cand && saferead::readAt<float>(9, static_cast<uintptr_t>(attacker), CrPos, &ax) &&
                    saferead::readAt<float>(9, static_cast<uintptr_t>(attacker), CrPos + 4, &ay) &&
                    saferead::readAt<float>(9, static_cast<uintptr_t>(cand), CrPos, &tx) &&
                    saferead::readAt<float>(9, static_cast<uintptr_t>(cand), CrPos + 4, &ty)) {
                    float lim = mx + 0.1f;
                    bool ok = cai::a1Strict(ax, ay, tx, ty, mx);
                    ++t->strictN; if (!ok) ++t->strictRej;   // always counted (A1 summary strict=N/rejected); the line is throttled
                    if (t->strictLog.allow(now, ok ? 1u : 0u, 10, 5000))   // first 10, then on an ok/would-reject change or every 5 s
                        pev(PiA1x, now, "t=%lu PR A1X STRICT cr=%08x new=%08x d2=%.2f lim2=%.2f -> %s", now, id, newId,
                            (ax - tx) * (ax - tx) + (ay - ty) * (ay - ty), lim * lim, ok ? "ok" : "would-reject");
                }
            }
        }
    }
    if (g_cfg.log && crPasses(attacker, party)) {
        int tgtArea = fr<int>(ebp, F_TargetArea), myArea = reinterpret_cast<Fn0>(FnGetArea)(attacker);
        const char* reason = cai::a1Reason(ranged, dist2, mx, los, !tgtArea || myArea == tgtArea);
        unsigned sig = (apply ? 1u : 0u) | (newId != Invalid ? 2u : 0u) | (static_cast<unsigned>(reason[0]) << 8) | (static_cast<unsigned>(raw & 0xFF) << 16);
        if (t->a1Log.allow(now, sig, 10, 2000)) {
            float dist = dist2 > 0 ? __builtin_sqrtf(dist2) : 0.f;
            char a1x[64] = "";   // A1X suffix (probe_a1x): combat-exit ms, fallback timer, combat flag after this pass
            if (g_cfg.probeA1x)
                snprintf(a1x, sizeof a1x, " c524=%d f570=%d combat=%d", rd<int>(attacker, CrCombatExit), rd<int>(attacker, CrFallback), rd<int>(attacker, CrCombat));
            eline(now, "t=%lu A1 cr=%08x party=%d raw=%d eff=%d reason=%s dist=%.1f max=%.1f los=%d ranged=%d area=%s tgt=%08x -> %s new=%08x (wouldSpin=%u)%s",
                  now, id, party, raw, eff, reason, dist, mx, los, ranged, (!tgtArea || myArea == tgtArea) ? "same" : "other", tgt,
                  !apply ? (g_cfg.a1 ? "spin (not listed)" : "spin (a1=0)") : newId != Invalid ? "retarget" : "end", newId, t->wouldSpin, a1x);
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
        if (cai::p1ParityReset(g_cfg.p1, stamp[0], stamp[1])) {   // stamped while p1 was on: back to the vanilla never-stamped state
            stamp[0] = stamp[1] = 0; frw<unsigned>(ebp, -0x28, cai::p1Local(true)); ++g_st.p1Resets;
        }
        return;
    }
    g_p1iCr = c; g_p1iElapsed = fr<unsigned>(ebp, -0x28);   // for E3 (review N4)
    bool run = cai::p1ShouldRun(fr<unsigned>(ebp, -0x24), fr<unsigned>(ebp, -0x20), stamp[0], stamp[1], fr<unsigned>(ebp, -0x1C),
                                fr<unsigned>(ebp, -0x28), static_cast<unsigned>(g_cfg.p1IntervalMs));
    frw<unsigned>(ebp, -0x28, cai::p1Local(run));
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
        if (g_cfg.fm1 && party && g_cfg.fm1RethinkMs > 0 &&
            cai::fm1RethinkWrite(g_cfg, party, rd<int>(bumpee, CrCombat), rd<int>(bumpee, CrFallback)))
            wr<int>(bumpee + CrFallback, g_cfg.fm1RethinkMs);
    }
    int bid = show ? rd<int>(bumpee, ObId) : 0;
    if (show && g_subj.allow(SkFm1, bid, now, keep ? 1u : 0u, 20, 2000)) {   // first 20 per bumpee, then on change / 2 s
        char bt[33], mt[33]; readTag(bumpee, bt, sizeof bt); readTag(mover, mt, sizeof mt);
        tline(now, 0, "t=%lu FM1 bumpee=%08x %s party=%d mover=%08x %s combat=%d tgt=%08x round=%d queue=[%s]%s -> %s%s", now, bid, bt,
              party, rd<int>(mover, ObId), mt, rd<int>(bumpee, CrCombat), rd<int>(bumpee, CrTarget, Invalid), roundActive(bumpee), q,
              listOk ? "" : " (unreadable)", keep ? "keep" : "clear", (!keep && !g_cfg.fm1) ? " (fm1=0)" : "");
    }
}

// FM4 mode 1. SaveCreature 0x574500: snapshot the live FollowInfo before SetPartyMemberFlag(0,1) frees it (0x57451F,
// EAX = creature), restore it into the re-created one after SaveFollowInfo wrote the defaults (0x575183), so the save
// bytes stay vanilla and the live follow state survives.
namespace { unsigned char g_snap[FollowInfoSize]; cai::Fm4Pair g_fm4; }

extern "C" void __cdecl CompanionAiFm4Snap(int cr) {
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    if (!g_fm4.begin(g_cfg.fm4, g_cfg.log)) return;
    if (!rd<int>(cr, CrParty)) return;
    int fi = rd<int>(cr, CrFollowInfo);
    if (!fi || !saferead::probeRead(7, reinterpret_cast<const void*>(fi), FollowInfoSize)) return;
    memcpy(g_snap, reinterpret_cast<const void*>(fi), FollowInfoSize);
    g_fm4.commit(cr); ++g_st.fm4Snap;
}

extern "C" void __cdecl CompanionAiFm4Restore(int ebp) {
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    int cr = fr<int>(ebp, -0x1D4);
    if (!g_fm4.take(cr)) return;
    int fi = rd<int>(cr, CrFollowInfo);
    unsigned char saved[FollowInfoSize];
    bool ok = fi && rd<int>(cr, CrParty) && saferead::probeRead(7, reinterpret_cast<const void*>(fi), FollowInfoSize);
    if (ok) memcpy(saved, reinterpret_cast<const void*>(fi), FollowInfoSize);
    bool restored = false;
    if (cai::fm4ShouldRestore(g_cfg.fm4, ok)) { memcpy(reinterpret_cast<void*>(fi), g_snap, FollowInfoSize); restored = true; ++g_st.fm4Restore; }
    if (g_cfg.log) {
        auto I = [](const unsigned char* b, int o) { int v; memcpy(&v, b + o, 4); return v; };
        auto F = [](const unsigned char* b, int o) { float v; memcpy(&v, b + o, 4); return v; };
        char tag[33]; readTag(cr, tag, sizeof tag);
        eline(now, "t=%lu FM4 cr=%08x %s tid=%lu fi=%08x live: obj=%08x maxSpeed=%.2f safety=%d | saved: obj=%08x maxSpeed=%.2f safety=%d -> %s", now,
              rd<int>(cr, ObId), tag, (unsigned long)GetCurrentThreadId(), fi, I(g_snap, 0), F(g_snap, 0x28), I(g_snap, 0x38), ok ? I(saved, 0) : -1, ok ? F(saved, 0x28) : 0.f,
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
        return cai::a4VanillaResult(v);
    }
    int v = 0;
    bool readable = row && saferead::readAt<int>(8, static_cast<uintptr_t>(row), 0x154, &v);
    cai::A4Out o = cai::a4Decide(readable, v);
    frw<int>(ebp, -0x15C, o.local);
    if (o.guarded) {
        ++g_st.a4;
        int crId = rd<int>(fr<int>(ebp, -0x1AC), ObId);
        if (g_subj.allow(SkA4, crId, now, 0, 5, 10000))   // first 5 per creature, then one per 10 s
            eline(now, "t=%lu A4 %s spell row %08x: cr=%08x spell=%d -> skip (not-hostile path 0x6D9EA4)", now, row ? "unreadable" : "NULL", row,
                  crId, fr<int>(ebp, -0x58));
    }
    return o.ret;
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
        int type = -1, party = 0;
        if (g_cfg.c1PartyOnly) { type = rd<unsigned char>(self, ObType, 0xFF); if (type == ObjTypeCreature) party = rd<int>(self, CrParty); }
        bool who = cai::c1Who(g_cfg, type, party);
        bool capOk = who && g_c1Cap.allow(self, ebp, now, 256);
        if (who && !capOk) ++g_st.c1Capped;
        v = cai::c1Value(g_cfg.c1, who, capOk);
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

// =====================================================================================================================
// Phase-4 probe handlers (log only; every one returns void and leaves the frame untouched)
// =====================================================================================================================

// FM2C. PlotPathInArea 0x55AC80 at 0x55B69A (MOV ECX,[0x99B5A0]): the result [EBP-0x18] is 2 (partial granted) or 3 (failed).
// PI = [EBP+8]: +0x2C creature id, +0x30 target id, +0x3C call count, +0x19C cached first code, +0 client-path flag.
extern "C" void __cdecl CompanionAiProbeFm2c(int ebp) {
    if (!g_cfg.probeFm2c) return;
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    int res = fr<int>(ebp, -0x18), code = fr<int>(ebp, -0x20), pi = fr<int>(ebp, 8);
    int cr = rd<int>(pi, 0x2C), tgt = rd<int>(pi, 0x30);
    if (cr) { ObjInfo o = objInfo(cr, now); if (!objPasses(o)) return; }   // cr 0 (unreadable PI) is always logged: the analyzer flags it
    Fm2Pair& p = fm2Pair(cr, tgt, now);
    if (res == 2) cai::pairBump(p.c2, cr, tgt, now, 30000);
    else if (res == 3) cai::pairBump(p.c3, cr, tgt, now, 30000);
    if (!p.lt.allow(now, static_cast<unsigned>(res), 10, 5000)) return;   // first 10 per pair, then on a res change / every 5 s
    unsigned n2 = (p.c2.n && now - p.c2.last <= 30000) ? p.c2.n : 0, n3 = (p.c3.n && now - p.c3.last <= 30000) ? p.c3.n : 0;
    pline(PiFm2c, now, "t=%lu PR FM2C cr=%08x tgt=%08x res=%d code=%d cached=%d cp=%d calls=%d n2=%u n3=%u", now, cr, tgt, res, code,
          rd<int>(pi, 0x19C), rd<int>(pi, 0), rd<int>(pi, 0x3C), n2, n3);
}

// FM2F. MoveToPoint 0x5BFC00 FAIL leg at 0x5C2438: creature [EBP-0x5A8], PI [EBP-0x598] (+0x30 target id), node [EBP+8] (u16 +0x6C = group).
extern "C" void __cdecl CompanionAiProbeFm2Fail(int ebp) {
    if (!g_cfg.probeFm2f) return;
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    int cr = fr<int>(ebp, -0x5A8), pi = fr<int>(ebp, -0x598), node = fr<int>(ebp, 8);
    int id = rd<int>(cr, ObId), party = rd<int>(cr, CrParty);
    if (!id || !crPasses(cr, party)) return;
    int tgt = rd<int>(pi, 0x30);
    Fm2Pair& p = fm2Pair(id, tgt, now);
    cai::pairBump(p.fl, id, tgt, now, 12000);
    if (!g_psubj.allow(PkFm2f, id, now, p.fl.n, 20, 60000)) return;   // first 20 per creature, then on an n change
    int ctrl = Invalid;
    bool isCtrl = controlledId(&ctrl) && ctrl == id;
    pline(PiFm2f, now, "t=%lu PR FM2F cr=%08x tgt=%08x grp=%u n=%u span=%lums ctrl=%d", now, id, tgt, unsigned(rd<unsigned short>(node, 0x6C)),
          p.fl.n, static_cast<unsigned long>(now - p.fl.first), isCtrl ? 1 : 0);
}

// FM2R. Action 0x11 handler 0x5BC1D0 at 0x5BC5F4: creature [EBP-0xA4], target id [EBP-0x28], otherArea [EBP-0x40], inRange [EBP-0x2C],
// range f32 [EBP-0x3C]. d = 3D distance via +0x94; prog = previous d - d for the same pair.
extern "C" void __cdecl CompanionAiProbeFm2Reapproach(int ebp) {
    if (!g_cfg.probeFm2r) return;
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    int cr = fr<int>(ebp, -0xA4), tgt = fr<int>(ebp, -0x28);
    int id = rd<int>(cr, ObId), party = rd<int>(cr, CrParty);
    if (!id || !crPasses(cr, party)) return;
    float d = -1.f;
    if (int to = gameObject(tgt)) {
        float a[3], b[3]; bool ok = true;
        for (int i = 0; i < 3 && ok; ++i) ok = saferead::readAt<float>(10, static_cast<uintptr_t>(cr), CrPos + 4 * i, &a[i]) &&
                                               saferead::readAt<float>(10, static_cast<uintptr_t>(to), CrPos + 4 * i, &b[i]);
        if (ok) d = __builtin_sqrtf((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]) + (a[2] - b[2]) * (a[2] - b[2]));
    }
    Fm2Pair& p = fm2Pair(id, tgt, now);
    cai::pairBump(p.ra, id, tgt, now, 12000);
    float prog = (p.ra.n > 1 && p.ra.lastD > 0.f && d >= 0.f) ? p.ra.lastD - d : 0.f;
    p.ra.lastD = d >= 0.f ? d : 0.f;
    if (p.rlogged >= 20 && p.ra.n % 5 != 0) return;   // first 20 per pair, then every 5th n
    ++p.rlogged;
    pline(PiFm2r, now, "t=%lu PR FM2R cr=%08x tgt=%08x other=%d inr=%d range=%.2f d=%.2f prog=%.2f n=%u", now, id, tgt, fr<int>(ebp, -0x40),
          fr<int>(ebp, -0x2C), fr<float>(ebp, -0x3C), d, prog, p.ra.n);
}

// FM3S. UpdateState 0x51D790 at 0x51DBE1, reached only while the level's "first" flag [EBP-0x38] == 1: the first object becomes the sentinel.
extern "C" void __cdecl CompanionAiProbeFm3Sentinel(int ebp) {
    if (!g_cfg.probeFm3) return;
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    g3.active = true; g3.level = fr<int>(ebp, -0x34); g3.sentinel = fr<int>(ebp, -0x28); g3.removed = false;
}

// FM3E. UpdateState level end at 0x51DD33 (the stolen SUB re-runs after this): did the sentinel leave its level / get removed?
extern "C" void __cdecl CompanionAiProbeFm3LevelEnd(int ebp) {
    if (!g_cfg.probeFm3) return;
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    int level = fr<int>(ebp, -0x34);
    if (!g3.active || level != g3.level) { g3.active = false; return; }
    g3.active = false;
    ++g_fm3c.passes;
    int nowLevel = g3.sentinel ? rd<int>(g3.sentinel, 0x78, -1) : -1;
    if (!g3.sentinel || !cai::fm3Lost(level, nowLevel, g3.removed)) return;
    ++g_fm3c.lost;
    if (g_fm3LLines >= 200) return;
    ++g_fm3LLines;
    unsigned long long elapsed = ((static_cast<unsigned long long>(fr<unsigned>(ebp, -0x2C)) << 32) | fr<unsigned>(ebp, -0x30)) -
                                 ((static_cast<unsigned long long>(fr<unsigned>(ebp, -0xC)) << 32) | fr<unsigned>(ebp, -0x10));
    unsigned long long slice = (static_cast<unsigned long long>(fr<unsigned>(ebp, -0x1C)) << 32) | fr<unsigned>(ebp, -0x20);
    pline(PiFm3, now, "t=%lu PR FM3L level=%d sentinel=%08x now=%d removed=%d done=%d elapsed=%llu slice=%llu", now, level, g3.sentinel, nowLevel,
          g3.removed ? 1 : 0, fr<int>(ebp, -0x40), elapsed, slice);
}

// FM3R. RemoveObject 0x51E0C0 at 0x51E0C9: object [EBP+8], master [EBP-0x10]; list = master + 4 + 16 * level (+0 id array, +4 count, +0xC cursor).
extern "C" void __cdecl CompanionAiProbeFm3Remove(int ebp) {
    if (!g_cfg.probeFm3) return;
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    int obj = fr<int>(ebp, 8), master = fr<int>(ebp, -0x10);
    int level = rd<int>(obj, 0x78, -1), id = rd<int>(obj, ObId);
    int idx = -1, cursor = -1;
    if (level >= 0 && level <= 4 && master) {
        int list = master + 4 + 16 * level, arr = rd<int>(list, 0), cnt = rd<int>(list, 4, -1);
        cursor = rd<int>(list, 0xC, -1);
        if (arr && cnt > 0 && cnt <= 4096 && saferead::probeRead(11, reinterpret_cast<const void*>(arr), static_cast<size_t>(cnt) * 4)) {
            const int* ids = reinterpret_cast<const int*>(arr);
            for (int i = 0; i < cnt; ++i) if (ids[i] == id) { idx = i; break; }
        }
    }
    ++g_fm3c.removes;
    if (idx >= 0 && idx <= cursor) ++g_fm3c.beforeCursor;
    bool isSentinel = g3.active && obj == g3.sentinel && level == g3.level;
    if (isSentinel) g3.removed = true;
    if (g_fm3xLog.allow(now, 0, 50, 1000))   // first 50, then one per second
        pline(PiFm3, now, "t=%lu PR FM3X id=%08x level=%d idx=%d cursor=%d sentinel=%d", now, id, level, idx, cursor, isSentinel ? 1 : 0);
}

// RS. RunScript 0x6FD8D0 entry (thiscall; ret = return address, script = CExoString* slot (may be 0), self = OBJECT_SELF id). Only the
// 7 probed call sites pass the first test; everything else returns after at most 7 compares.
extern "C" void __cdecl CompanionAiProbeRunScript(int ret, int script, int self) {
    if (!g_cfg.probeRs) return;
    int site = cai::rsSite(static_cast<unsigned>(ret));
    if (!site) return;
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    if (site <= 3) a1xFollowup(self, cai::rsName(site), now);   // slot-6 sites: the A1 end was followed up (before the filter: review N3)
    ObjInfo o = objInfo(self, now);
    if (!objPasses(o)) return;
    RsTrack* t = rsTrack(self, now);
    ++t->n[site];
    if (o.ispc == 1) g_pcSeen = self;
    if (site == 5) {   // OnBlocked runs per second
        if (now - t->sec >= 1000) { t->sec = now; t->cur12 = 0; }
        if (++t->cur12 > t->max12) t->max12 = t->cur12;
    }
    char sfx[128] = "";
    bool e9 = false;
    if (site == 4) {   // OnShout: the speaker, its shout pattern; E9 = pattern 15 from a non-party creature to a party listener (counted before any line budget)
        int obj = gameObject(self), spk = obj ? rd<int>(obj, CrListenerSpeaker, Invalid) : Invalid;
        int pat = obj ? rd<int>(obj, CrListenerPattern, -1) : -1;
        ObjInfo so = objInfo(spk, now);
        e9 = pat == 15 && so.type == ObjTypeCreature && !so.party && o.party;
        if (e9) ++g_e9;
        snprintf(sfx, sizeof sfx, " spk=%08x spkParty=%d pat=%d spkTag=%s", spk, so.party, pat, so.tag);
    }
    // lines: first 20 per (cr, site); S7: first 10 per creature (50 per session), plus every E9-case line (300 per session, own budget)
    if (site == 4) {
        if (e9) { if (g_rsS7E9Lines >= 300) return; ++g_rsS7E9Lines; }
        else { if (t->logged[4] >= 10 || g_rsS7Lines >= 50) return; ++t->logged[4]; ++g_rsS7Lines; }
    } else {
        if (t->logged[site] >= 20) return;
        ++t->logged[site];
    }
    int empty = (!script || !rd<int>(script, 0) || rd<int>(script, 4, 0) <= 0) ? 1 : 0;
    pline(PiRs, now, "t=%lu PR RS site=%s cr=%08x %s party=%d empty=%d%s", now, cai::rsName(site), self, o.tag, o.party, empty, sfx);
}

// P2. SpawnInHeartbeatPerception 0x56B8A0 at 0x56BE79 (HOT: every AIUpdate of every creature). The engine runs UpdatePerception(0) iff
// [cr+0x10E8] == 0 && mode [EBP+8] == 1 && elapsed [EBP-0x18] >= 4000: this logs those runs and the gap between them per creature.
extern "C" void __cdecl CompanionAiProbeP2(int ebp) {
    if (!g_cfg.probeP2) return;
    int cr = fr<int>(ebp, -0xD4);
    if (!cr) return;
    int party = *reinterpret_cast<const int*>(cr + CrParty);
    if (!party && !g_cfg.logAll && g_cfg.tags.empty()) return;
    int isPC = *reinterpret_cast<const int*>(cr + CrIsPC);
    if (isPC == 1) g_pcSeen = *reinterpret_cast<const int*>(cr + ObId);
    unsigned elapsed = fr<unsigned>(ebp, -0x18);
    if (!cai::p2Runs(isPC, fr<int>(ebp, 8), elapsed)) return;
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    if (!crPasses(cr, party)) return;
    int id = *reinterpret_cast<const int*>(cr + ObId);
    P2Track* t = p2Track(id, now);
    if (t->runs && now - t->last <= 30000) {   // longer gaps = the creature was away (area change): not a cadence sample
        unsigned gap = now - t->last;
        ++t->gaps; t->sumGap += gap; if (gap > t->maxGap) t->maxGap = gap;
    }
    t->last = now; ++t->runs;
    int ctrl = Invalid;
    bool isCtrl = controlledId(&ctrl) && ctrl == id;
    if (isCtrl) ++t->ctrlRuns;
    if (t->logged >= 10) return;
    ++t->logged;
    pline(PiP2, now, "t=%lu PR P2 cr=%08x ctrl=%d elapsed=%u combat=%d", now, id, isCtrl ? 1 : 0, elapsed, *reinterpret_cast<const int*>(cr + CrCombat));
}

// PF. SetPartyMemberFlag 0x58A330 at 0x58A35E, before the store of +0x11AC: creature [EBP-0x48], new [EBP+8], apply [EBP+0xC], caller [EBP+4].
extern "C" void __cdecl CompanionAiProbePartyFlag(int ebp) {
    if (!g_cfg.probePf) return;
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    int cr = fr<int>(ebp, -0x48), nw = fr<int>(ebp, 8), apply = fr<int>(ebp, 0xC), caller = fr<int>(ebp, 4);
    int old = rd<int>(cr, CrParty, -1);
    int site = cai::pfSite(static_cast<unsigned>(caller));
    switch (site) {
    case 0: ++g_pfc.saveA; break;
    case 1: ++g_pfc.saveB; break;
    case 3: ++g_pfc.xferA; break;
    case 4: ++g_pfc.xferB; break;
    default: ++g_pfc.other; break;
    }
    bool transient = cai::pfTransient(site);
    if (!transient) g_partyPf = true;
    if (transient && (old == nw || !crPasses(cr, old | nw))) return;   // transient pairs: party members whose flag really changes
    char tag[33]; readTag(cr, tag, sizeof tag);
    pline(PiPf, now, "t=%lu PR PF cr=%08x %s old=%d new=%d apply=%d site=%s", now, rd<int>(cr, ObId), tag, old, nw, apply, cai::pfName(site));
}

// RAR. RunActions 0x5D56B0 at 0x5D6A60 (HOT: per executed action; the stolen CMP re-runs after this): object [EBP-0x1A8], action type
// [EBP-0x18], result [EBP-0x1C]. Histogram per creature (PRS RAR), a queued move under a SetState (MOVESTATE) and stuck requeues (REQ).
extern "C" void __cdecl CompanionAiProbeRunActions(int ebp) {
    if (!g_cfg.probeRar) return;
    int obj = fr<int>(ebp, -0x1A8);
    if (!obj || *reinterpret_cast<const unsigned char*>(obj + ObType) != ObjTypeCreature) return;
    int party = *reinterpret_cast<const int*>(obj + CrParty);
    if (!party && !g_cfg.logAll && g_cfg.tags.empty()) return;
    DWORD now = GetTickCount(); tick(now);
    if (!(g_cfg.logAll || (g_cfg.partyOnly && party))) { saferead::beginScope(); if (!crPasses(obj, party)) return; }
    int id = *reinterpret_cast<const int*>(obj + ObId), type = fr<int>(ebp, -0x18), res = fr<int>(ebp, -0x1C);
    RarTrack* t = rarTrack(id, now);
    t->ispc = *reinterpret_cast<const int*>(obj + CrIsPC);
    if (t->ispc == 1) g_pcSeen = id;
    ++t->n; ++t->cnt[cai::rarIndex(type, res)]; t->lastType = type;
    if (res == 1 && type == t->runType) ++t->run; else { t->runType = type; t->run = res == 1 ? 1u : 0u; }
    if (t->run == 200 && t->reqLogged < 10) {
        ++t->reqLogged;
        pline(PiRar, now, "t=%lu PR RAR REQ cr=%08x type=%x n=%u", now, id, type, t->run);
    }
    if ((type == 1 || type == 0x11) && t->moveLogged < 20) {
        unsigned state = *(reinterpret_cast<const unsigned char*>(obj) + CrSetState);
        if (state) { ++t->moveLogged; pline(PiRar, now, "t=%lu PR RAR MOVESTATE cr=%08x type=%x res=%d state=%d", now, id, type, res, int(state)); }
    }
}

// SHX. Shout delivery 0x547100, single exit 0x5475E8 (all 5 exits land here): speaker [EBP-0xC0], &string [EBP+8], radius f32 [EBP+0xC], bSeen
// [EBP+0x10], area [EBP-0x30], list [EBP-0x14] (area+0x194), index [EBP-0x1C], last candidate id [EBP-0x10] (uninitialised on early exits).
extern "C" void __cdecl CompanionAiProbeShout(int ebp) {
    if (!g_cfg.probeShx) return;
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    int spk = fr<int>(ebp, -0xC0), area = fr<int>(ebp, -0x30), list = fr<int>(ebp, -0x14), idx = fr<int>(ebp, -0x1C), last = fr<int>(ebp, -0x10);
    int count = area ? rd<int>(list, 4, -1) : -1;
    bool lastOk = area && idx >= 0 && idx < count && gameObject(last) != 0;
    int kind = cai::shoutExit(area != 0, idx, count, lastOk);
    float r = fr<float>(ebp, 0xC);
    int stype = rd<unsigned char>(spk, ObType, 0xFF);
    int party = stype == ObjTypeCreature ? rd<int>(spk, CrParty, 0) : 0;   // +0x11AC only exists on creatures (review SF1)
    ++g_shx.shouts;
    if (r >= 34.5f && r <= 35.5f) ++g_shx.r35;
    if (party) ++g_shx.partySpk;
    switch (kind) {
    case cai::ShoutEnd: ++g_shx.end; break;
    case cai::ShoutXBreak: ++g_shx.xbreak; break;
    case cai::ShoutNoArea: ++g_shx.noarea; break;
    case cai::ShoutAbortStart: if (stype == ObjTypeCreature) ++g_shx.abortStartCr; else ++g_shx.abortStartOther; break;
    default: ++g_shx.abortMid; break;
    }
    bool isAbort = kind == cai::ShoutAbortStart || kind == cai::ShoutAbortMid;
    static unsigned s_abortLines = 0; static cai::LineThrottle s_abortLog;
    if (isAbort) { if (s_abortLines >= 100 && !s_abortLog.allow(now, static_cast<unsigned>(kind), 0, 1000)) return; ++s_abortLines; }   // first 100 ABORT, then 1/s per kind
    else { if (g_shxLines >= 30) return; ++g_shxLines; }                                                                              // first 30 others
    int sp = fr<int>(ebp, 8);
    char text[33], tag[33];
    copyStr(rd<int>(sp, 0), rd<int>(sp, 4, -1), text, sizeof text);
    readTag(spk, tag, sizeof tag);
    const char* fmt = "t=%lu PR SHX %s spk=%08x type=%d %s party=%d r=%.0f seen=%d idx=%d count=%d last=%08x text=%s";
    if (isAbort) eline(now, fmt, now, cai::shoutName(kind), rd<int>(spk, ObId), stype, tag, party, r, fr<int>(ebp, 0x10), idx, count, last, text);   // own cap, not the budget
    else pline(PiShx, now, fmt, now, cai::shoutName(kind), rd<int>(spk, ObId), stype, tag, party, r, fr<int>(ebp, 0x10), idx, count, last, text);
}

// SS1. OnApplySetState 0x59CBA0 at 0x59D966 (stolen MOV [EDX+0xFF5],AL; the wrapper restores EAX): creature [EBP-0x20], new state [EBP-0x1C],
// old = u8 [cr+0xFF5].
extern "C" void __cdecl CompanionAiProbeSetState(int ebp) {
    if (!g_cfg.probeSs1) return;
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    int cr = fr<int>(ebp, -0x20), party = rd<int>(cr, CrParty);
    if (!crPasses(cr, party) || g_ss1Lines >= 50) return;   // party, first 50
    ++g_ss1Lines;
    char tag[33]; readTag(cr, tag, sizeof tag);
    pline(PiSs1, now, "t=%lu PR SS1 cr=%08x %s state %d->%d combat=%d", now, rd<int>(cr, ObId), tag, int(rd<unsigned char>(cr, CrSetState)),
          fr<int>(ebp, -0x1C), rd<int>(cr, CrCombat));
}

// GCA. GetCurrentAction (command 522) 0x67B100 at 0x67B204: object id [EBP-4], result [EBP-8] (0xFFFF init, 0xFFFE empty). [EBP-0xC],
// [EBP-0x14] and [EBP-0x20] are uninitialised on the early paths: never read.
extern "C" void __cdecl CompanionAiProbeGetCurrentAction(int ebp) {
    if (!g_cfg.probeGca) return;
    saferead::beginScope();
    DWORD now = GetTickCount(); tick(now);
    int id = fr<int>(ebp, -4), res = fr<int>(ebp, -8);
    ObjInfo o = objInfo(id, now);
    if (!objPasses(o)) return;
    GcaTrack* t = gcaTrack(id, now);
    ++t->total;
    int slot = -1;
    for (int i = 0; i < t->nres; ++i) if (t->res[i] == res) { slot = i; break; }
    if (slot < 0 && t->nres < 6) { slot = t->nres++; t->res[slot] = res; }
    if (slot >= 0) ++t->cnt[slot];
    if (res == t->lastRes || t->logged >= 40) return;   // on a res change per creature, first 40
    t->lastRes = res; ++t->logged;
    int obj = gameObject(id);
    pline(PiGca, now, "t=%lu PR GCA cr=%08x res=%d lastRun=%x moving=%d", now, id, res, rarLastType(id), obj ? rd<int>(obj, CrMoving, -1) : -1);
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
        g_loadTid = GetCurrentThreadId();
        time_t tt = time(nullptr); struct tm* lt = localtime(&tt);
        char when[32] = "?";
        if (lt) strftime(when, sizeof when, "%Y-%m-%d %H:%M:%S", lt);
        eline(now, "# companion-ai %s session %s t=%lu pid=%lu tid=%lu cwd=%s ini=%s", Version, when, now, (unsigned long)GetCurrentProcessId(),
              (unsigned long)g_loadTid, cwd, g_iniPath);
        loadIni(now, true);
        g_lastSummary = now;
        g_ready = true;
    } else if (reason == DLL_PROCESS_DETACH) {
        if (g_log) fflush(g_log);
    }
    return TRUE;
}
