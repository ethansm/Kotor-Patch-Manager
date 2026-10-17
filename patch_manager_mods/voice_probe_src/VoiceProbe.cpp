// Voice Probe 0.2.0 (Steam Aspyr build 6A522E71...). Diagnostics only: no gameplay change, spawns nothing, default off.
// Question: how often does Global::ExecuteCommandPlaySound run, and in what order relative to the script-side PC messages?
//
// Hook (detour, 5-byte prologue 55 8B EC 6A FF = PUSH EBP / MOV EBP,ESP / PUSH -1, whole instructions):
//   P1 Global::ExecuteCommandPlaySound, address in kotor2-steam-aspyr.hooks.toml (thiscall, ECX = this). The handler reads ECX only.
//
// The sound resref is NOT read here. Stack arguments are never touched and a resref cannot be read safely from this hook.
// The script side logs the resref via SendMessageToPC; match the two logs by call order and tick.
//
// Switch: voice_probe.ini [probe] probe=0 (default off). Read ONCE, lazily, on the first handler call, via
// GetPrivateProfileIntA on an absolute path in the game working dir. When probe=0 the handler returns before doing any work.
//
// Output: voice_probe_log.txt via KpLog.h since 0.1.1 (channel main; core prefix `T=<s.ms> f=- voice-probe/main`; old core: same file name,
// fallback at INFO, appended), capped at kMaxLogLines. Needs a `[voice-probe]` section in kplog.ini on a new core. Lines are short (< 232 B core cut).
//
// Hotkey trigger (0.2.0, doc 41 V-C2, kvoice/V-C2_TRIGGER_DESIGN.md): runs ONE named script with the PC as OBJECT_SELF, with no loose
// game file. voice_probe.ini [trigger] key=<VK> (0 = off, default) ctrl=1 shift=0 cooldown_ms=1000 script=<resref>. Polled on the
// npc-world K2NW_TICK (engine thread, no hook of our own); fires once per fresh press with the game in the foreground. It CALLS
// CVirtualMachine::RunScript (thiscall on the VM global; script CExoString*, object self, bOidValid = 1; RET 12; the engine deep-copies
// the name). companion-ai owns a detour on RunScript's entry; calling through it is fine. Logic and log lines: TriggerCore.h.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "kpatch_api.h"
#include "KpLog.h"
#include "SafeRead.h"
#include "Party.h"
#include "NpcWorld.h"
#include "TriggerCore.h"

KP_LOG_PATCH("voice-probe");

namespace {

constexpr const char* kLogName = "voice_probe_log.txt";
constexpr const char* kIniName = "voice_probe.ini";
constexpr int kMaxLogLines = 500;
constexpr int kMaxTrigLines = 200;   // own cap: PLAYSOUND lines cannot crowd out the TRIGGER lines

int g_logLines = 0;
int g_trigLines = 0;
long g_callCount = 0;        // P1 calls since the probe was switched on
int g_probeState = -1;       // -1 = ini not read yet, 0 = off, 1 = on

// ---- logging: KpLog.h channel main --------------------------------------------------------------------------
KP_CHANNEL(chMain, "main");
// Only stores the path and the level (no I/O): safe from a static initialiser. Same file name as before (runbook and tooling read it).
const bool g_fallbackSet = (KpLog_SetFallbackFile(kLogName), KpLog_SetFallbackLevel(KPLOG_INFO), true);

void logf(const char* fmt, ...) {
    if (g_logLines >= kMaxLogLines) return;
    ++g_logLines;
    char buf[200];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    KP_LOG(chMain, "%s", buf);
}

void trigLog(const char* line) {
    if (g_trigLines >= kMaxTrigLines) return;
    ++g_trigLines;
    KP_LOG(chMain, "%s", line);
}

// A bare ini name is searched in the Windows dir, so build an absolute path from the working dir.
void iniPath(char* path, size_t cap) {
    path[0] = '\0';
    DWORD n = GetCurrentDirectoryA(MAX_PATH, path);
    if (n == 0 || n >= MAX_PATH || n + 2 + strlen(kIniName) > cap) path[0] = '\0';
    else strcat(path, "\\");
    strcat(path, kIniName);
}

// Reads voice_probe.ini once (first call), then returns the cached switch.
bool probeEnabled() {
    if (g_probeState < 0) {
        char path[MAX_PATH + 32];
        iniPath(path, sizeof path);
        g_probeState = GetPrivateProfileIntA("probe", "probe", 0, path) != 0 ? 1 : 0;
    }
    return g_probeState == 1;
}

// ---- hotkey trigger ------------------------------------------------------------------------------------------------------------
constexpr uintptr_t kVmGlobal = 0x00A1B4A8;    // CVirtualMachine* (engine callers: MOV ECX,[global] right before the RunScript call)
constexpr uintptr_t kRunScript = 0x006FD8D0;   // CVirtualMachine::RunScript, thiscall RET 12
constexpr unsigned kSiteTrigger = 0x6C40;      // saferead site id

struct ExoString { const char* s; uint32_t len; };   // CExoString: char* + length; RunScript reads s only and copies it
using RunScriptFn = int(__thiscall*)(void* vm, ExoString* script, uint32_t objectSelf, int oidValid);

k2nw::Client g_nw;
vpt::Config g_trigCfg;
vpt::Trigger g_trig;
bool g_trigLoaded = false;
bool g_areaReady = false;     // AREA_STABLE seen since the last AREA_LOAD (no fire mid-transition on a stale PC id)

void loadTrigger() {
    g_trigLoaded = true;
    char path[MAX_PATH + 32];
    iniPath(path, sizeof path);
    char script[64] = "";
    GetPrivateProfileStringA("trigger", "script", "", script, sizeof script, path);
    g_trigCfg = vpt::makeConfig(static_cast<int>(GetPrivateProfileIntA("trigger", "key", 0, path)),
                                static_cast<int>(GetPrivateProfileIntA("trigger", "ctrl", 1, path)),
                                static_cast<int>(GetPrivateProfileIntA("trigger", "shift", 0, path)),
                                static_cast<int>(GetPrivateProfileIntA("trigger", "cooldown_ms", vpt::DefaultCooldownMs, path)), script);
    char line[vpt::MaxLine];
    vpt::configLine(g_trigCfg, line, sizeof line);
    trigLog(line);
}

bool gameForeground() {
    HWND fg = GetForegroundWindow();
    DWORD pid = 0;
    if (!fg) return false;
    GetWindowThreadProcessId(fg, &pid);
    return pid == GetCurrentProcessId();
}

bool keyDown(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }

struct EngineEnv : vpt::Env {
    uint32_t pcObjectId(const char** why) override {
        party::Fail f = party::F_None;
        const uintptr_t client = party::controlledCreature(&f);
        if (!client) { *why = f == party::F_None ? "no controlled creature" : party::failText(f); return 0; }
        const uintptr_t server = party::serverCreature(client);
        if (!server) { *why = "no server creature"; return 0; }
        uint32_t id = 0;
        if (!saferead::readAt<uint32_t>(kSiteTrigger, server, 4, &id)) { *why = "server creature id unreadable"; return 0; }
        return id;
    }
    int runScript(const char* script, uint32_t oid, bool* called) override {
        uint32_t vm = 0;
        *called = false;
        if (!saferead::readAt<uint32_t>(kSiteTrigger, kVmGlobal, 0, &vm) || !vm) return 0;
        static char name[vpt::MaxResref + 1];
        memcpy(name, script, strlen(script) + 1);
        ExoString es = { name, static_cast<uint32_t>(strlen(name)) };
        *called = true;
        return reinterpret_cast<RunScriptFn>(kRunScript)(reinterpret_cast<void*>(vm), &es, oid, 1);
    }
    void log(const char* line) override { trigLog(line); }
};

void KPATCH_CALL onNwEvent(const K2NwEvent* ev, void*) {
    if (!ev) return;
    if (ev->kind == K2NW_AREA_LOAD) { g_areaReady = false; return; }
    if (ev->kind == K2NW_AREA_STABLE) { g_areaReady = true; return; }
    if (ev->kind != K2NW_TICK) return;
    if (!g_trigLoaded) loadTrigger();
    if (!g_trigCfg.enabled()) return;
    vpt::Input in;
    in.key = keyDown(g_trigCfg.vk);   // key first: the other Win32 queries run only while it is held
    if (in.key) {
        in.areaReady = g_areaReady;
        in.foreground = gameForeground();
        in.ctrl = keyDown(VK_CONTROL);
        in.shift = keyDown(VK_SHIFT);
    }
    const DWORD now = GetTickCount();
    const vpt::Step st = g_trig.step(g_trigCfg, in, now);
    if (st == vpt::Step::Idle) return;
    if (st == vpt::Step::Cooldown) { trigLog("TRIGGER press ignored (cooldown)"); return; }
    if (st == vpt::Step::NotReady) { trigLog("TRIGGER press ignored (area not stable)"); return; }
    saferead::beginScope();
    EngineEnv env;
    if (vpt::fire(env, g_trigCfg, g_trig.fires, now)) g_trig.markRan(now);
}

void onNwConnect(bool ok, const char* why) {
    if (!ok) KP_WARN("TRIGGER npc-world not connected (%s): hotkey trigger inactive", why ? why : "?");
}

}  // namespace

extern "C" {

// P1: Global::ExecuteCommandPlaySound (thiscall, ECX = this). One line per call while the probe is on; nothing otherwise.
void __cdecl VoiceProbePlaySound(void* self) {
    g_nw.poll();   // late npc-world: retries connect every 256 sounds until the tick registration lands
    if (!probeEnabled()) return;
    ++g_callCount;
    uintptr_t obj = reinterpret_cast<uintptr_t>(self);
    logf("PLAYSOUND call #%ld tick=%lu this=%08lx",
         g_callCount,
         static_cast<unsigned long>(GetTickCount()),
         static_cast<unsigned long>(obj));
}

}  // extern "C"

// KpLog_Init + npc-world tick registration; no file I/O under the loader lock (both ini files are read lazily). The probe stays off
// unless voice_probe.ini [probe] probe=1; the trigger stays off unless [trigger] key and script are set.
extern "C" __declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    g_nw.init(api, "voice-probe");
    g_nw.onConnect = onNwConnect;
    g_nw.add(K2NW_TICK | K2NW_AREA_LOAD | K2NW_AREA_STABLE, onNwEvent);
    g_nw.connect();
}
