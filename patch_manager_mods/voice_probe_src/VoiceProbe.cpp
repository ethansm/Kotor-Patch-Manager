// Voice Probe 0.1.1 (Steam Aspyr build 6A522E71...). Diagnostics only: no gameplay change, spawns nothing, default off.
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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "kpatch_api.h"
#include "KpLog.h"

KP_LOG_PATCH("voice-probe");

namespace {

constexpr const char* kLogName = "voice_probe_log.txt";
constexpr const char* kIniName = "voice_probe.ini";
constexpr int kMaxLogLines = 500;

int g_logLines = 0;
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

// Reads voice_probe.ini once (first call), then returns the cached switch.
bool probeEnabled() {
    if (g_probeState < 0) {
        // A bare ini name is searched in the Windows dir, so build an absolute path from the working dir.
        char path[MAX_PATH + 32] = "";
        DWORD n = GetCurrentDirectoryA(MAX_PATH, path);
        if (n == 0 || n >= MAX_PATH) path[0] = '\0';
        else strcat(path, "\\");
        strcat(path, kIniName);
        g_probeState = GetPrivateProfileIntA("probe", "probe", 0, path) != 0 ? 1 : 0;
    }
    return g_probeState == 1;
}

}  // namespace

extern "C" {

// P1: Global::ExecuteCommandPlaySound (thiscall, ECX = this). One line per call while the probe is on; nothing otherwise.
void __cdecl VoiceProbePlaySound(void* self) {
    if (!probeEnabled()) return;
    ++g_callCount;
    uintptr_t obj = reinterpret_cast<uintptr_t>(self);
    logf("PLAYSOUND call #%ld tick=%lu this=%08lx",
         g_callCount,
         static_cast<unsigned long>(GetTickCount()),
         static_cast<unsigned long>(obj));
}

}  // extern "C"

// KpLog_Init only; no file I/O under the loader lock. The probe stays off unless voice_probe.ini [probe] probe=1.
extern "C" __declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) { KpLog_Init(api); }
