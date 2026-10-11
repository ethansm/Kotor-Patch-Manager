// CmdqProbe.cpp -- cmdq-probe 0.1.0 (Steam build): LOG-ONLY probe for the command-queue investigation (patch_manager_mods/29_paused_command_queue.md).
// Handlers are plain extern "C" void __cdecl functions. The detour wrappers in kotor2-steam-aspyr.hooks.toml call them with the
// stolen game arguments BEFORE the stolen bytes run. Hard rules: no writes to game memory or registers, no engine calls, reads only
// through saferead, and every handler returns at once while its channel is off (cmdq_probe.ini: probe=1 AND ch_<name>=1).
// Lines: PAUSE (edge-only), CMD, FIRE, ROUND, CLR, REEQ, MED. All channels share one line budget (max_lines).
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ActQueue.h"
#include "SafeRead.h"
#include "KpLog.h"

KP_LOG_PATCH("cmdq-probe");

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chPause, "pause");
KP_CHANNEL(chCmd, "cmd");
KP_CHANNEL(chFire, "fire");
KP_CHANNEL(chRound, "round");
KP_CHANNEL(chClr, "clr");
KP_CHANNEL(chMed, "med");

// Fallback sink for an old core or a failed KpLog_Init (same pattern as the texture-memory mod). Stores the path and level only.
const bool g_fallbackSet = (KpLog_SetFallbackFile("cmdq_probe_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

// Config. Written once in KPatchInit (before the handlers do anything); defaults are silent.
static bool g_probe = false;
static bool g_onPause = false, g_onCmd = false, g_onFire = false, g_onRound = false, g_onClr = false, g_onMed = false;   // probe=1 AND ch_<name>=1
static LONG g_maxLines = 2000;
static bool g_partyOnly = true;        // party_only=1: CLR needs creature+0x11AC != 0 (meaning UNVERIFIED, doc 29); 0 drops that test
static volatile LONG g_lines = 0;      // lines written by all channels (cap = g_maxLines)
static bool g_pauseSeen = false;       // PAUSE edge state: last value logged
static int g_pauseLast = 0;

// saferead site tags (failure ring only). actq::read uses siteQueue+0..+5 and siteRound+0..+9.
static const unsigned kSiteRoundOwner = 0xCD0020u;
static const unsigned kSiteClrVictim = 0xCD0030u, kSiteClrParty = 0xCD0031u;
static const unsigned kSiteClrQueue = 0xCD0040u, kSiteClrRound = 0xCD0060u;
static const unsigned kSiteMedEntry = 0xCD0080u, kSiteMedCanUse = 0xCD0090u;
static const int kClrSlots = 4;        // queue slots (doc 29): the CLR filter and the q= list both use slots 0..3

static inline unsigned U(const void* p) { return static_cast<unsigned>(reinterpret_cast<uintptr_t>(p)); }

// Shared line budget. Called only after the channel gate. The first line past the cap says so once; later lines are dropped.
static bool takeLine() {
    LONG n = InterlockedIncrement(&g_lines);
    if (n <= g_maxLines) return true;
    if (n == g_maxLines + 1) KP_LOG(chMain, "cap reached max_lines=%ld: further lines dropped", static_cast<long>(g_maxLines));
    return false;
}

// Trims spaces, tabs and CR/LF in place; returns the first non-blank character.
static char* trimIni(char* s) {
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') ++s;
    size_t n = strlen(s);
    while (n && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\r' || s[n - 1] == '\n')) s[--n] = 0;
    return s;
}

// Strict on: only 1 / on / true / yes switch a key on. Anything else (including a blank value) is off.
static bool flagValue(const char* v) {
    return !_stricmp(v, "1") || !_stricmp(v, "on") || !_stricmp(v, "true") || !_stricmp(v, "yes");
}

// Reads cmdq_probe.ini from the game dir (cwd). A missing file or a bad value leaves the silent default. Called once, from KPatchInit.
static void readConfig() {
    bool probe = false, pause = false, cmd = false, fire = false, rnd = false, clr = false, med = false;
    FILE* f = fopen("cmdq_probe.ini", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            char* p = trimIni(line);
            if (*p == 0 || *p == ';' || *p == '#' || *p == '[') continue;
            char* eq = strchr(p, '=');
            if (!eq) continue;
            *eq = 0;
            char* key = trimIni(p);
            char* val = trimIni(eq + 1);
            if (!_stricmp(key, "probe")) probe = flagValue(val);
            else if (!_stricmp(key, "ch_pause")) pause = flagValue(val);
            else if (!_stricmp(key, "ch_cmd")) cmd = flagValue(val);
            else if (!_stricmp(key, "ch_fire")) fire = flagValue(val);
            else if (!_stricmp(key, "ch_round")) rnd = flagValue(val);
            else if (!_stricmp(key, "ch_clr")) clr = flagValue(val);
            else if (!_stricmp(key, "ch_med")) med = flagValue(val);
            else if (!_stricmp(key, "party_only")) g_partyOnly = flagValue(val);
            else if (!_stricmp(key, "max_lines")) {
                char* end = nullptr;
                unsigned long v = strtoul(val, &end, 10);
                if (end != val && v >= 1 && v <= 1000000ul) g_maxLines = static_cast<LONG>(v);
            }
        }
        fclose(f);
    }
    g_probe = probe;
    g_onPause = probe && pause;
    g_onCmd = probe && cmd;
    g_onFire = probe && fire;
    g_onRound = probe && rnd;
    g_onClr = probe && clr;
    g_onMed = probe && med;
}

static void addName(char* buf, size_t cap, bool on, const char* name) {
    if (!on) return;
    size_t n = strlen(buf);
    snprintf(buf + n, cap - n, "%s%s", n ? "," : "", name);
}

// ---- handlers: called from the detour wrappers, before the stolen bytes run. Read-only. ----

extern "C" void __cdecl CmdqProbePause(int mask, int val) {
    if (!g_onPause || (mask & 2) == 0) return;
    saferead::beginScope();
    if (!KP_ENABLED(chPause, KPLOG_INFO) || (g_pauseSeen && val == g_pauseLast) || !takeLine()) return;
    g_pauseSeen = true;
    g_pauseLast = val;
    KP_LOG(chPause, "PAUSE mask=%d val=%d", mask, val);
}

extern "C" void __cdecl CmdqProbeCast(void* actor, uint32_t spell, int tgt, int real) {
    if (!g_onCmd) return;
    saferead::beginScope();
    if (!KP_ENABLED(chCmd, KPLOG_INFO) || !takeLine()) return;
    KP_LOG(chCmd, "CMD kind=spell actor=%x id=%x tgt=%x real=%d", U(actor), static_cast<unsigned>(spell), static_cast<unsigned>(tgt), real);
}

extern "C" void __cdecl CmdqProbeItemCast(void* actor, uint32_t item, int tgt, int real) {
    if (!g_onCmd) return;
    saferead::beginScope();
    if (!KP_ENABLED(chCmd, KPLOG_INFO) || !takeLine()) return;
    KP_LOG(chCmd, "CMD kind=item actor=%x id=%x tgt=%x real=%d", U(actor), static_cast<unsigned>(item), static_cast<unsigned>(tgt), real);
}

extern "C" void __cdecl CmdqProbeFireCast(uint32_t actor) {
    if (!g_onFire) return;
    saferead::beginScope();
    if (!KP_ENABLED(chFire, KPLOG_INFO) || !takeLine()) return;
    KP_LOG(chFire, "FIRE kind=spell actor=%x", static_cast<unsigned>(actor));
}

extern "C" void __cdecl CmdqProbeFireItem(uint32_t actor) {
    if (!g_onFire) return;
    saferead::beginScope();
    if (!KP_ENABLED(chFire, KPLOG_INFO) || !takeLine()) return;
    KP_LOG(chFire, "FIRE kind=item actor=%x", static_cast<unsigned>(actor));
}

extern "C" void __cdecl CmdqProbeRoundStart(void* roundObj) {
    if (!g_onRound) return;
    saferead::beginScope();
    if (!KP_ENABLED(chRound, KPLOG_INFO) || !takeLine()) return;
    uint32_t owner = 0;   // stays 0 when round is null or unreadable
    (void)saferead::readAt(kSiteRoundOwner, reinterpret_cast<uintptr_t>(roundObj), 0xB04, &owner);
    KP_LOG(chRound, "ROUND ev=start round=%x owner=%x", U(roundObj), static_cast<unsigned>(owner));
}

extern "C" void __cdecl CmdqProbeRoundEnd(void* roundObj) {
    if (!g_onRound) return;
    saferead::beginScope();
    if (!KP_ENABLED(chRound, KPLOG_INFO) || !takeLine()) return;
    uint32_t owner = 0;
    (void)saferead::readAt(kSiteRoundOwner, reinterpret_cast<uintptr_t>(roundObj), 0xB04, &owner);
    KP_LOG(chRound, "ROUND ev=end round=%x owner=%x", U(roundObj), static_cast<unsigned>(owner));
}

extern "C" void __cdecl CmdqProbeClear(void* victim, uint32_t caller) {
    if (!g_onClr) return;
    saferead::beginScope();
    if (!KP_ENABLED(chClr, KPLOG_INFO)) return;
    // Filter first, in order: victim readable; party flag (+0x11AC) nonzero; queue slots 0..3 hold 0x3F, 0x0F or 0x2E.
    if (!saferead::probeRead(kSiteClrVictim, victim, 4)) return;
    const uintptr_t v = reinterpret_cast<uintptr_t>(victim);
    uint32_t party = 0;
    if (!saferead::readAt(kSiteClrParty, v, actq::Cr11AC, &party)) return;
    if (g_partyOnly && party == 0) return;
    actq::Snapshot s;
    if (!actq::read(v, &s, kSiteClrQueue, kSiteClrRound)) return;
    const int n = s.ne < kClrSlots ? s.ne : kClrSlots;
    bool hit = false;
    for (int i = 0; i < n; ++i) {
        const int t = s.e[i].type;
        if (t == 0x3F || t == 0x0F || t == 0x2E) hit = true;
    }
    if (!hit || !takeLine()) return;
    char q[64] = "";
    for (int i = 0; i < n; ++i) {
        size_t used = strlen(q);
        snprintf(q + used, sizeof(q) - used, "%s%x", i ? "," : "", static_cast<unsigned>(s.e[i].type));
    }
    KP_LOG(chClr, "CLR victim=%x caller=%x q=%s", U(victim), static_cast<unsigned>(caller), q);
}

extern "C" void __cdecl CmdqProbeReequip(void* actor) {
    if (!g_onClr) return;
    saferead::beginScope();
    if (!KP_ENABLED(chClr, KPLOG_INFO) || !takeLine()) return;
    KP_LOG(chClr, "REEQ actor=%x", U(actor));
}

extern "C" void __cdecl CmdqProbeMedEntry(void* self, uint32_t arg, uint32_t arg2) {
    (void)arg2;   // not in the grammar
    if (!g_onMed) return;
    saferead::beginScope();
    if (!KP_ENABLED(chMed, KPLOG_INFO) || !takeLine()) return;
    uint32_t base = 0;   // read only when arg looks like a plain pointer
    if ((arg & 0x40000000u) == 0 && arg >= 0x10000u) (void)saferead::readAt(kSiteMedEntry, arg, 0xC, &base);
    KP_LOG(chMed, "MED site=entry this=%x arg=%x base=%x flag=0", U(self), static_cast<unsigned>(arg), static_cast<unsigned>(base));
}

extern "C" void __cdecl CmdqProbeMedCanUse(int result, uint32_t item) {
    if (!g_onMed) return;
    saferead::beginScope();
    if (!KP_ENABLED(chMed, KPLOG_INFO) || !takeLine()) return;
    uint32_t base = 0;   // stays 0 when item is null or unreadable
    (void)saferead::readAt(kSiteMedCanUse, item, 0xC, &base);
    KP_LOG(chMed, "MED site=canuse this=0 arg=%x base=%x flag=%d", static_cast<unsigned>(item), static_cast<unsigned>(base), result != 0 ? 1 : 0);
}

// Written out literally (create-patch's export scan reads the source text) and listed in exports.def.
// Reads the config here (file I/O is fine in KPatchInit), then starts the logger and writes the one load line (not budgeted).
extern "C" void __cdecl KPatchInit(const KPatchApi* api) {
    readConfig();
    KpLog_Init(api);
    char names[96] = "";
    addName(names, sizeof(names), g_onPause, "pause");
    addName(names, sizeof(names), g_onCmd, "cmd");
    addName(names, sizeof(names), g_onFire, "fire");
    addName(names, sizeof(names), g_onRound, "round");
    addName(names, sizeof(names), g_onClr, "clr");
    addName(names, sizeof(names), g_onMed, "med");
    KP_LOG(chMain, "cmdq-probe 0.1.0 loaded: probe=%d channels=%s max_lines=%ld", g_probe ? 1 : 0, names[0] ? names : "none", static_cast<long>(g_maxLines));
}

// No I/O here (loader lock). The config is read in KPatchInit.
BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
