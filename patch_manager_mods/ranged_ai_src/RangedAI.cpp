// Ranged AI (Steam Aspyr build 6A522E71.../LAA 4AB72FC1...). Engine half of Mod 8: makes ranged party members
// hold position instead of walking into melee when their line of fire is blocked (E-H1), and lets them retarget
// like the player does (E-H2). Research trail and every address: patch_manager_mods/13_ranged_ai_spacing.md.
//
// Hooks (cdecl detours; the stolen bytes run after the handler; a non-zero return jumps to consumed_exit_address):
//   E-H1 0x006d83ea RangedAiHold(attacker=ECX, ebp)   inside CSWSCreature::AIActionPhysicalAttacks 0x6d7390,
//                                                     return 1 => end the attack action (0x6d83ff)
//   E-H2 0x006d7c79 RangedAiRetarget(attacker=EAX, ebp) return 1 => behave as IsPC (0x6d7c8c)
//        (not 0x6d7c73: stolen bytes run AFTER the handler with EAX = its return value, so they must not use EAX)
//   E-H3 0x006d850f RangedAiStanceRules(eax, ebp)     skip-only MOV [ebp-0x174],EAX: that local is read only by the
//        "Ranged/Stationary companion does not approach" tests (0x6d876a, 0x6d8a7e); store 11 for stance 16 so the
//        kiting stance gets vanilla Ranged's engine rule (review 2026-09-30). 0.3.0 kite_approach=1: map 16 -> 11 only
//        with a ranged weapon AND the target in weapon range; otherwise keep 16 = the vanilla Aggressive approach
//        (ranged: walks until in range; melee: closes in). design_ai_engine_fixes.md section 3, A1 part B
// Stance 16 "Ranged (Kiting)" in the party stance menu (0.2.0; all four windows EAX-free):
//   S-H1 0x0077aa5e RangedAiStanceClamp(state=ECX)     AddStanceActions 0x77a9e0: return 1 => keep 16 (skip reset-to-9, 0x77aab4)
//   S-H2 0x0077acbc RangedAiStanceRow(ebp)             row for aistate 16: id 0x415 + icon; return 1 => 0x77ae77 (next row)
//   S-H3 0x0077d8d4 RangedAiStanceClick(ebp)           StanceActionClicked 0x77d890: id 0x415 => stance 16; return 1 => 0x77d9ca
//   S-H4 0x0074dbdd RangedAiHudStance(ebp)             HUD refresh 0x74da40 default branch (skip-only): marker id for stance 16
//
// Behaviour applies only to the stances in hold_stances / retarget_stances (shipped: 16), so the vanilla stances stay
// vanilla. Everything is re-read from ranged_ai.ini once a second, no restart needed.
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

constexpr int KiteStance = 16, KiteActionId = 0x415;
constexpr int RowStride = 0x3C, RowActionId = 0x08, RowIcon = 0x20;
constexpr int KiteTutorialRow = 0x3F;   // tutorial.2da row StanceActionClicked passes to 0x740a20 for "Ranged" (shown once ever)
// AddStanceActions 0x77a9e0 frame: [ebp+8] = action list (*list = row array), [ebp-0x14] = row aistate, [ebp-0x1c] = row index
constexpr int FA_List = 8, FA_State = -0x14, FA_RowIdx = -0x1C;
// StanceActionClicked 0x77d890 frame: [ebp+8] = action id, [ebp-0x10] = stats, [ebp-8] = message strref, [ebp-4] = tutorial row
constexpr int FC_Id = 8, FC_Stats = -0x10, FC_MsgStrref = -0x8, FC_Tutorial = -0x4;
// AIActionPhysicalAttacks: [ebp-0x174] = stance copy compared with 0xB/0xC (Ranged/Stationary) before the approach move
constexpr int F_StanceRules = -0x174, VanillaRangedStance = 11;
// HUD refresh 0x74da40 frame: [ebp-0x28] = stance, [ebp-0x1c] = remembered stance action id
constexpr int FH_Stance = -0x28, FH_ActionId = -0x1C;

// AIActionPhysicalAttacks 0x6d7390 frame (EBP-relative), from the asm (dump_excerpts/006d7390.asm)
constexpr int F_Attacker = -0x49C, F_Los = -0xD0, F_Dist2 = -0xBC, F_Max = -0x2C, F_Desired = -0x4C,
              F_Desired2 = -0xB0, F_TargetArea = -0x38, F_Target = -0x50, F_BackoffFlag = -0x168,
              F_RangedFlag = -0x164, F_ActionObj = -0x14;

// ---- log ----
int g_logLines = 0;
constexpr int MaxLogLines = 6000;
void logf(const char* fmt, ...) {
    if (g_logLines > MaxLogLines) return;
    FILE* f = fopen("ranged_ai_log.txt", "a");
    if (!f) return;
    if (++g_logLines > MaxLogLines) { fputs("-- log cap reached, further lines dropped --\n", f); fclose(f); return; }
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f); fclose(f);
}

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

// ---- config ----
struct Config {
    bool hold = true, retarget = true, log = true, logAll = false;
    unsigned holdMask = 1u << KiteStance, retargetMask = 1u << KiteStance;
    bool rangedRules = true;         // E-H3: stance 16 uses the vanilla Ranged "no approach" rule
    bool kiteApproach = true;        // E-H3 0.3.0: ...but only with a ranged weapon and the target in range
    int msgStrref = 123710;          // floaty text on selecting stance 16; fallback = vanilla "Ranged Behavior" message
    char icon[17] = "ib_ranged";     // stance menu / HUD icon ResRef (<= 16 chars)
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
    c.hold = GetPrivateProfileIntA("RangedAI", "hold", 1, IniName) != 0;
    c.retarget = GetPrivateProfileIntA("RangedAI", "retarget", 1, IniName) != 0;
    c.rangedRules = GetPrivateProfileIntA("RangedAI", "ranged_rules", 1, IniName) != 0;
    c.kiteApproach = GetPrivateProfileIntA("RangedAI", "kite_approach", 1, IniName) != 0;
    c.msgStrref = GetPrivateProfileIntA("RangedAI", "kite_strref_msg", 123710, IniName);
    if (c.msgStrref <= 0) c.msgStrref = 123710;
    c.log = GetPrivateProfileIntA("RangedAI", "log", 1, IniName) != 0;
    c.logAll = GetPrivateProfileIntA("RangedAI", "log_all", 0, IniName) != 0;
    char s[128] = {};
    GetPrivateProfileStringA("RangedAI", "hold_stances", "16", s, sizeof s, IniName); c.holdMask = parseMask(s);
    GetPrivateProfileStringA("RangedAI", "retarget_stances", "16", s, sizeof s, IniName); c.retargetMask = parseMask(s);
    GetPrivateProfileStringA("RangedAI", "kite_icon", "ib_ranged", s, sizeof s, IniName);
    for (int i = 0; i < 16; ++i) { char ch = s[i]; c.icon[i] = (ch >= 'A' && ch <= 'Z') ? char(ch + 32) : ch; if (!ch) break; }
    c.icon[16] = 0;
    if (!c.icon[0]) strcpy(c.icon, "ib_ranged");
    if (force || c.hold != g_cfg.hold || c.retarget != g_cfg.retarget || c.holdMask != g_cfg.holdMask || c.retargetMask != g_cfg.retargetMask ||
        c.msgStrref != g_cfg.msgStrref || strcmp(c.icon, g_cfg.icon) || c.rangedRules != g_cfg.rangedRules || c.kiteApproach != g_cfg.kiteApproach)
        logf("config: hold=%d retarget=%d hold_stances=%x retarget_stances=%x ranged_rules=%d kite_approach=%d kite_strref_msg=%d kite_icon=%s log=%d log_all=%d",
             c.hold, c.retarget, c.holdMask, c.retargetMask, c.rangedRules, c.kiteApproach, c.msgStrref, c.icon, c.log, c.logAll);
    g_cfg = c;
}

// ---- per-creature log throttle ----
struct Track {
    int id = 0; unsigned sig = ~0u; int logged = 0; DWORD lastLog = 0; unsigned calls = 0, holds = 0;
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
    for (auto& t : g_tr)
        if (t.id && t.calls) { logf("summary: creature %08x hold-site calls=%u holds=%u logged=%d", t.id, t.calls, t.holds, t.logged); t.calls = t.holds = 0; }
}

struct V3 { float x, y, z; };
// E-H3 range test: same formula as E-H1's inline test (a zero/garbage weapon range never counts as in range)
bool inWeaponRange(float maxR, float dist2) {
    float lim = maxR + 0.1f; lim *= lim;
    return maxR > 1.0f && dist2 <= lim;
}

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

    bool party1 = party != 0;
    if (!party1 && !g_cfg.logAll) { return 0; }
    Track* t = trackFor(id);
    if (t->id != id) { *t = Track{}; t->id = id; }   // slot shared by a colliding id: reset its throttle state
    ++t->calls;

    int stance = stanceOf(attacker);
    int ranged = haveRanged(attacker) ? 1 : 0;
    int los = rd<int>(ebp, F_Los);
    float dist2 = rd<float>(ebp, F_Dist2), maxR = rd<float>(ebp, F_Max), desired = rd<float>(ebp, F_Desired);
    int myArea = reinterpret_cast<AreaFn>(FnGetArea)(attacker), tgtArea = rd<int>(ebp, F_TargetArea);
    int target = rd<int>(ebp, F_Target);
    int ttype = target ? rd<unsigned char>(target, ObjType, 0xFF) : -1;
    int endFlag = rd<int>(attacker, CrEndAttack);

    float lim = maxR + 0.1f; lim *= lim;
    bool inRange = maxR > 1.0f && dist2 <= lim;   // a zero/garbage weapon range never counts as in range
    bool hold = g_cfg.hold && party1 && !isPC && stance >= 0 && stance < 32 && ((g_cfg.holdMask >> stance) & 1) &&
                ranged && los == 0 && inRange && myArea && myArea == tgtArea && ttype == ObjTypeCreature;
    if (hold) ++t->holds;

    if (g_cfg.log) {
        DWORD now = GetTickCount();
        unsigned sig = (los != 0) | (ranged << 1) | ((myArea == tgtArea) << 2) | (hold << 3) | (inRange << 4) | ((endFlag != 0) << 5) |
                       ((rd<int>(ebp, F_BackoffFlag) != 0) << 6);
        if (t->logged < 16 || sig != t->sig || now - t->lastLog > 2000) {
            V3 me = pos(attacker, CrPos), tp = target ? pos(target, CrPos) : V3{0, 0, 0}, lb = pos(attacker, CrLastBlocked);
            logf("H1 t=%lu cr=%08x party=%d pc=%d stance=%d ranged=%d los=%d dist2=%.2f max=%.2f desired=%.2f inRange=%d area=%08x/%08x "
                 "tgt=%08x ttype=%d backoff=%d rangedLocal=%d end52c=%08x me=(%.2f,%.2f,%.2f) tgtpos=(%.2f,%.2f,%.2f) lastBlocked=(%.2f,%.2f,%.2f) -> hold=%d",
                 now, id, party, isPC, stance, ranged, los, dist2, maxR, desired, inRange, myArea, tgtArea, target, ttype,
                 rd<int>(ebp, F_BackoffFlag), rd<int>(ebp, F_RangedFlag), endFlag, me.x, me.y, me.z, tp.x, tp.y, tp.z, lb.x, lb.y, lb.z, hold);
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

// AddStanceActions runs on every action-bar rebuild (several times a second): log S1/S2 only when what they do changes.
int g_s1Logged = 0, g_s2LastRow = -1, g_s2Logged = 0;
constexpr int MenuLogBudget = 20;

// S-H1. ECX = the shown creature's stance, about to be clamped to {9,11,12,13}. Return 1 => keep it (stance 16).
extern "C" int __cdecl RangedAiStanceClamp(int state) {
    saferead::beginScope();
    if (state != KiteStance) return 0;
    loadIni(false);
    if (g_cfg.log && g_s1Logged < MenuLogBudget) { ++g_s1Logged; logf("S1 t=%lu stance menu build: stance %d kept (vanilla clamp skipped)", GetTickCount(), state); }
    return 1;
}

// S-H2. Row for the aistate.2da row being added. For aistate 16 fill what the vanilla case bodies fill (action id +
// icon ResRef; name, callback and flags were set by the common path) and skip the vanilla switch.
extern "C" int __cdecl RangedAiStanceRow(int ebp) {
    saferead::beginScope();
    if (rd<int>(ebp, FA_State, -1) != KiteStance) return 0;
    loadIni(false);
    int list = rd<int>(ebp, FA_List), base = list ? rd<int>(list, 0) : 0, idx = rd<int>(ebp, FA_RowIdx, -1);
    if (!base || idx < 0 || idx > 64) { logf("S2 bad frame: list=%08x base=%08x idx=%d (row left empty)", list, base, idx); return 1; }
    int row = base + idx * RowStride;
    if (!wr<int>(row + RowActionId, KiteActionId)) { logf("S2 row %08x not writable", row); return 1; }
    char ref[16] = {0};
    for (int i = 0; i < 16 && g_cfg.icon[i]; ++i) ref[i] = g_cfg.icon[i];   // CResRef: 16 bytes, lower case, zero padded (as 0x710eb0)
    for (int i = 0; i < 16; i += 4) { int w; memcpy(&w, ref + i, 4); wr<int>(row + RowIcon + i, w); }
    if (g_cfg.log && (idx != g_s2LastRow || g_s2Logged < 3) && g_s2Logged < MenuLogBudget) {
        ++g_s2Logged; g_s2LastRow = idx;
        logf("S2 t=%lu stance row %d (%08x): id=0x%x icon=%s", GetTickCount(), idx, row, KiteActionId, g_cfg.icon);
    }
    return 1;
}

// S-H3. Stance menu click. Action id 0x415 => stance 16 + its message; the tail (tutorial, message box) is vanilla.
extern "C" int __cdecl RangedAiStanceClick(int ebp) {
    saferead::beginScope();
    int id = rd<int>(ebp, FC_Id, -1);
    loadIni(false);
    int stats = rd<int>(ebp, FC_Stats);
    if (id != KiteActionId) {
        if (g_cfg.log) logf("S3 t=%lu stance click: id=0x%x (vanilla) stance before=%d", GetTickCount(), id, stats ? rd<unsigned short>(stats, StatsStance, 0xFFFF) : -1);
        return 0;
    }
    if (!stats) return 0;
    int before = rd<unsigned short>(stats, StatsStance, 0xFFFF);
    wr<unsigned short>(stats + StatsStance, KiteStance);
    wr<int>(ebp + FC_MsgStrref, g_cfg.msgStrref);
    wr<int>(ebp + FC_Tutorial, KiteTutorialRow);
    if (g_cfg.log) logf("S3 t=%lu stance click: id=0x%x stance %d -> %d msg strref=%d", GetTickCount(), id, before, rd<unsigned short>(stats, StatsStance, 0xFFFF), g_cfg.msgStrref);
    return 1;
}

// E-H3. Replaces MOV [ebp-0x174],EAX (EAX = the attacker's stance) in AIActionPhysicalAttacks.
// 0.3.0 kite_approach: [ebp-0x164] (ranged, 0x6d836b), [ebp-0xbc] (dist^2) and [ebp-0x2c] (max weapon range) are written
// before this point on every path; with a melee weapon or the target out of range stance 16 keeps the Aggressive
// approach instead of inheriting Ranged's "never approach" (which re-queued the attack forever, review A1).
namespace {
struct H3Log { int id = 0; int last = -1; DWORD t = 0; };
H3Log g_h3[8];
int g_h3Logged = 0;
}
extern "C" void __cdecl RangedAiStanceRules(int stance, int ebp) {
    saferead::beginScope();
    bool map = stance == KiteStance && g_cfg.rangedRules;
    int ranged = -1, inRange = -1;
    if (map && g_cfg.kiteApproach) {
        // live frame slots of the hooked function: read directly (as the store below)
        ranged = *reinterpret_cast<const int*>(ebp + F_RangedFlag) != 0;
        inRange = inWeaponRange(*reinterpret_cast<const float*>(ebp + F_Max), *reinterpret_cast<const float*>(ebp + F_Dist2));
        if (!ranged || !inRange) map = false;   // melee or out of range: vanilla Aggressive approach (move range = max if ranged)
    }
    int eff = map ? VanillaRangedStance : stance;
    // [ebp-0x174] is a slot of the live hooked frame; the replaced instruction is skipped, so always store it.
    *reinterpret_cast<int*>(ebp + F_StanceRules) = eff;
    if (stance == KiteStance && g_cfg.log) {   // first 20 lines, then only when the decision changes for a creature
        int attacker = *reinterpret_cast<const int*>(ebp + F_Attacker);   // `this` of the hooked function
        int id = attacker ? *reinterpret_cast<const int*>(attacker + CrId) : 0;
        H3Log* h = nullptr;
        for (auto& e : g_h3) if (e.id == id) h = &e;
        if (!h) { h = &g_h3[id & 7]; h->id = id; h->last = -1; h->t = 0; }
        int code = eff | (ranged + 1) << 8 | (inRange + 1) << 12;
        DWORD now = GetTickCount();
        if (g_h3Logged < 20 || (code != h->last && now - h->t >= 250)) {   // changes at most 4/s per creature
            ++g_h3Logged; h->last = code; h->t = now;
            logf("H3 t=%lu cr=%08x stance=16 ranged=%d inRange=%d kite_approach=%d -> %d%s", now, id, ranged, inRange,
                 g_cfg.kiteApproach, eff, eff == KiteStance ? " (approach)" : " (no approach)");
        }
    }
}

// S-H4. HUD refresh on character change, default branch of the stance -> action id switch (replaces MOV [ebp-0x1c],-1).
extern "C" void __cdecl RangedAiHudStance(int ebp) {
    saferead::beginScope();
    int stance = rd<int>(ebp, FH_Stance, -1);
    wr<int>(ebp + FH_ActionId, stance == KiteStance ? KiteActionId : -1);
    if (stance == KiteStance && g_cfg.log) logf("S4 t=%lu HUD stance marker -> 0x%x", GetTickCount(), KiteActionId);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        if (FILE* f = fopen("ranged_ai_log.txt", "w")) fclose(f);   // one session per file (cwd = game folder)
        char cwd[MAX_PATH] = "?"; GetCurrentDirectoryA(MAX_PATH, cwd);
        logf("ranged-ai 0.3.0 loaded; cwd=%s pid=%lu", cwd, (unsigned long)GetCurrentProcessId());
        if (GetCurrentDirectoryA(MAX_PATH, cwd) > 0) snprintf(IniName, sizeof IniName, "%s\\ranged_ai.ini", cwd);
        logf("ini path: %s", IniName);
        loadIni(true);
    }
    return TRUE;
}
