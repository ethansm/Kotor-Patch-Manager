// Voice Probe 0.1.0 (Steam Aspyr build 6A522E71...). Diagnostics only: no gameplay change, spawns nothing, default off.
// Question: how often does Global::ExecuteCommandPlaySound run, and in what order relative to the script-side PC messages?
//
// Hook (detour, 5-byte prologue 55 8B EC 6A FF = PUSH EBP / MOV EBP,ESP / PUSH -1, whole instructions):
//   P1 0x0068EA70 Global::ExecuteCommandPlaySound (thiscall, ECX = this). The handler reads ECX only.
//
// The sound resref is NOT read here. Stack arguments are never touched and a resref cannot be read safely from this hook.
// The script side logs the resref via SendMessageToPC; match the two logs by call order and tick.
//
// Switch: voice_probe.ini [probe] probe=0 (default off). Read ONCE, lazily, on the first handler call, via
// GetPrivateProfileIntA on an absolute path in the game working dir. When probe=0 the handler returns before doing any work.
//
// Output: voice_probe_log.txt in the game's working directory. One open/append/close per line, capped at kMaxLogLines.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

namespace {

constexpr const char* kLogName = "voice_probe_log.txt";
constexpr const char* kIniName = "voice_probe.ini";
constexpr int kMaxLogLines = 500;

int g_logLines = 0;
long g_callCount = 0;        // P1 calls since the probe was switched on
int g_probeState = -1;       // -1 = ini not read yet, 0 = off, 1 = on

// ---- logging: one open/append/close per line -----------------------------------------------------------------
void logf(const char* fmt, ...) {
    if (g_logLines >= kMaxLogLines) return;
    FILE* f = fopen(kLogName, "a");
    if (!f) return;
    ++g_logLines;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fputc('\n', f);
    fclose(f);
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
