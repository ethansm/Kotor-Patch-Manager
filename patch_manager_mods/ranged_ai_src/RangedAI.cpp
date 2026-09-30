// Ranged AI (Steam Aspyr build 6A522E71.../LAA 4AB72FC1...). Engine half of Mod 8: makes ranged party members
// hold position instead of walking into melee when their line of fire is blocked (E-H1), and lets them retarget
// like the player does (E-H2). Research trail and every address: patch_manager_mods/13_ranged_ai_spacing.md.
//
// Hooks (cdecl detours, stolen bytes run first; a non-zero return jumps to consumed_exit_address):
//   E-H1 0x006d83ea RangedAiHold(attacker=ECX, ebp)   inside CSWSCreature::AIActionPhysicalAttacks 0x6d7390,
//                                                     return 1 => end the attack action (0x6d83ff)
//   E-H2 0x006d7c73 RangedAiRetarget(attacker=EAX, ebp) return 1 => behave as IsPC (0x6d7c8c)
//
// Stage 0 (hold=0, retarget=0 in ranged_ai.ini) only logs, so vanilla behaviour is unchanged. Behaviour is switched
// on from the INI (re-read once a second), no restart needed.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "../_shared/SafeRead.h"

namespace {

constexpr int FnRangeWeaponEquipped = 0x0056E1E0;   // thiscall(creature), no stack args, returns bool
constexpr int FnGetArea = 0x005453C0;               // thiscall(creature), returns area*
using RangedFn = int(__thiscall*)(int);
using AreaFn = int(__thiscall*)(int);

// creature offsets
constexpr int CrId = 0x04, CrPos = 0x94, CrEndAttack = 0x52C, CrLastBlocked = 0x558, CrIsPC = 0x10E8,
              CrStats = 0x1198, CrIsParty = 0x11AC;
constexpr int StatsStance = 0x146;
constexpr int ObjType = 0x08, ObjTypeCreature = 5;

// AIActionPhysicalAttacks 0x6d7390 frame (EBP-relative), from the asm (dump_excerpts/006d7390.asm)
constexpr int F_Attacker = -0x49C, F_Los = -0xD0, F_Dist2 = -0xBC, F_Max = -0x2C, F_Desired = -0x4C,
              F_Desired2 = -0xB0, F_TargetArea = -0x38, F_Target = -0x50, F_ApproachFlag = -0x168,
              F_RangedFlag = -0x164, F_ActionObj = -0x14;

// ---- log ----
int g_logLines = 0;
constexpr int MaxLogLines = 6000;
void logf(const char* fmt, ...) {
    if (g_logLines >= MaxLogLines) return;
    FILE* f = fopen("ranged_ai_log.txt", "a");
    if (!f) return;
    ++g_logLines;
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f); fclose(f);
}

template <typename T> T rd(int base, int off, T def = T()) {
    T v = def;
    saferead::readAt<T>(1, static_cast<uintptr_t>(base), off, &v);
    return v;
}

// ---- config ----
struct Config {
    bool hold = false, retarget = false, log = true, logAll = false;
    unsigned holdMask = 1u << 11, retargetMask = 1u << 11;
};
Config g_cfg;
DWORD g_lastIni = 0;
char IniName[MAX_PATH + 32] = "ranged_ai.ini";   // made absolute in DllMain (a bare name is searched in the Windows dir)

unsigned parseMask(const char* s) {
    unsigned m = 0;
    for (const char* p = s; *p;) {
        while (*p == ' ' || *p == ',') ++p;
        if (*p >= '0' && *p <= '9') { int v = 0; while (*p >= '0' && *p <= '9') v = v * 10 + (*p++ - '0'); if (v < 32) m |= 1u << v; }
        else if (*p) ++p;
    }
    return m;
}

void loadIni(bool force) {
    DWORD now = GetTickCount();
    if (!force && g_lastIni && now - g_lastIni < 1000) return;
    g_lastIni = now ? now : 1;
    Config c;
    c.hold = GetPrivateProfileIntA("RangedAI", "hold", 0, IniName) != 0;
    c.retarget = GetPrivateProfileIntA("RangedAI", "retarget", 0, IniName) != 0;
    c.log = GetPrivateProfileIntA("RangedAI", "log", 1, IniName) != 0;
    c.logAll = GetPrivateProfileIntA("RangedAI", "log_all", 0, IniName) != 0;
    char s[128] = {};
    GetPrivateProfileStringA("RangedAI", "hold_stances", "11", s, sizeof s, IniName); c.holdMask = parseMask(s);
    GetPrivateProfileStringA("RangedAI", "retarget_stances", "11", s, sizeof s, IniName); c.retargetMask = parseMask(s);
    if (force || c.hold != g_cfg.hold || c.retarget != g_cfg.retarget || c.holdMask != g_cfg.holdMask || c.retargetMask != g_cfg.retargetMask)
        logf("config: hold=%d retarget=%d hold_stances=%x retarget_stances=%x log=%d log_all=%d", c.hold, c.retarget, c.holdMask, c.retargetMask, c.log, c.logAll);
    g_cfg = c;
}

// ---- per-creature log throttle ----
struct Track {
    int id = 0; unsigned sig = ~0u; int logged = 0; DWORD lastLog = 0; unsigned calls = 0;
};
Track g_tr[8];
Track* trackFor(int id) {
    for (auto& t : g_tr) if (t.id == id) return &t;
    for (auto& t : g_tr) if (!t.id) { t.id = id; return &t; }
    return &g_tr[id & 7];
}
DWORD g_lastSummary = 0;
void summary() {
    DWORD now = GetTickCount();
    if (now - g_lastSummary < 10000) return;
    g_lastSummary = now;
    for (auto& t : g_tr) if (t.id && t.calls) { logf("summary: creature %08x hold-site calls=%u logged=%d", t.id, t.calls, t.logged); t.calls = 0; }
}

struct V3 { float x, y, z; };
V3 pos(int obj, int off) { V3 v = {0, 0, 0}; saferead::readAt<V3>(2, static_cast<uintptr_t>(obj), off, &v); return v; }

int stanceOf(int cr) {
    int stats = rd<int>(cr, CrStats);
    return stats ? rd<unsigned short>(stats, StatsStance, 0xFFFF) : -1;
}

bool haveRanged(int cr) { return reinterpret_cast<RangedFn>(FnRangeWeaponEquipped)(cr) != 0; }

} // namespace

// E-H1. ECX = attacker, EBP = AIActionPhysicalAttacks frame. Return 1 => hold (end the attack action).
extern "C" int __cdecl RangedAiHold(int attacker, int ebp) {
    saferead::beginScope();
    loadIni(false);
    if (!attacker) return 0;
    int party = rd<int>(attacker, CrIsParty), isPC = rd<int>(attacker, CrIsPC);
    int id = rd<int>(attacker, CrId);
    Track* t = trackFor(id);
    ++t->calls;

    bool party1 = party != 0;
    if (!party1 && !g_cfg.logAll) { return 0; }

    int stance = stanceOf(attacker);
    int ranged = haveRanged(attacker) ? 1 : 0;
    int los = rd<int>(ebp, F_Los);
    float dist2 = rd<float>(ebp, F_Dist2), maxR = rd<float>(ebp, F_Max), desired = rd<float>(ebp, F_Desired);
    int myArea = reinterpret_cast<AreaFn>(FnGetArea)(attacker), tgtArea = rd<int>(ebp, F_TargetArea);
    int target = rd<int>(ebp, F_Target);
    int ttype = target ? rd<unsigned char>(target, ObjType, 0xFF) : -1;
    int endFlag = rd<int>(attacker, CrEndAttack);

    float lim = static_cast<float>(static_cast<double>(maxR) + 0.1); lim *= lim;
    bool inRange = dist2 <= lim;
    bool hold = g_cfg.hold && party1 && !isPC && stance >= 0 && stance < 32 && ((g_cfg.holdMask >> stance) & 1) &&
                ranged && los == 0 && inRange && myArea == tgtArea && ttype == ObjTypeCreature;

    if (g_cfg.log) {
        DWORD now = GetTickCount();
        unsigned sig = (los != 0) | (ranged << 1) | ((myArea == tgtArea) << 2) | (hold << 3) | (inRange << 4) | ((endFlag != 0) << 5) |
                       ((rd<int>(ebp, F_ApproachFlag) != 0) << 6);
        if (t->logged < 16 || sig != t->sig || now - t->lastLog > 2000) {
            V3 me = pos(attacker, CrPos), tp = target ? pos(target, CrPos) : V3{0, 0, 0}, lb = pos(attacker, CrLastBlocked);
            logf("H1 t=%lu cr=%08x party=%d pc=%d stance=%d ranged=%d los=%d dist2=%.2f max=%.2f desired=%.2f inRange=%d area=%08x/%08x "
                 "tgt=%08x ttype=%d approach=%d rangedLocal=%d end52c=%08x me=(%.2f,%.2f,%.2f) tgtpos=(%.2f,%.2f,%.2f) lastBlocked=(%.2f,%.2f,%.2f) -> hold=%d",
                 now, id, party, isPC, stance, ranged, los, dist2, maxR, desired, inRange, myArea, tgtArea, target, ttype,
                 rd<int>(ebp, F_ApproachFlag), rd<int>(ebp, F_RangedFlag), endFlag, me.x, me.y, me.z, tp.x, tp.y, tp.z, lb.x, lb.y, lb.z, hold);
            t->sig = sig; t->lastLog = now; ++t->logged;
        }
        summary();
    }
    return hold ? 1 : 0;
}

// E-H2. EAX = attacker, EBP = frame. Reached on the second blocked check at the same position. Return 1 => act as IsPC.
extern "C" int __cdecl RangedAiRetarget(int attacker, int ebp) {
    saferead::beginScope();
    loadIni(false);
    if (!attacker) return 0;
    int party = rd<int>(attacker, CrIsParty), isPC = rd<int>(attacker, CrIsPC);
    if (!party && !g_cfg.logAll) return 0;
    int stance = stanceOf(attacker);
    int ranged = haveRanged(attacker) ? 1 : 0;
    bool gate = g_cfg.retarget && party != 0 && !isPC && stance >= 0 && stance < 32 && ((g_cfg.retargetMask >> stance) & 1) && ranged;
    if (g_cfg.log) {
        V3 me = pos(attacker, CrPos), lb = pos(attacker, CrLastBlocked);
        logf("H2 t=%lu cr=%08x party=%d pc=%d stance=%d ranged=%d los=%d me=(%.2f,%.2f,%.2f) lastBlocked=(%.2f,%.2f,%.2f) -> retarget=%d",
             GetTickCount(), rd<int>(attacker, CrId), party, isPC, stance, ranged, rd<int>(ebp, F_Los), me.x, me.y, me.z, lb.x, lb.y, lb.z, gate);
    }
    return gate ? 1 : 0;
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        char cwd[MAX_PATH] = "?"; GetCurrentDirectoryA(MAX_PATH, cwd);
        logf("ranged-ai 0.1.0-dev loaded; cwd=%s pid=%lu", cwd, (unsigned long)GetCurrentProcessId());
        if (GetCurrentDirectoryA(MAX_PATH, cwd) > 0) snprintf(IniName, sizeof IniName, "%s\\ranged_ai.ini", cwd);
        logf("ini path: %s", IniName);
        loadIni(true);
    }
    return TRUE;
}
