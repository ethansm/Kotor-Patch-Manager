// MemStatusCache.h -- GlobalMemoryStatus result cache + CALL-operand rewrite helpers for k2-engine-perf.
// Header-only, host-testable: no engine addresses and no Win32 calls (the game-side wrapper lives in K2EnginePerf.cpp).
//
// Engine fact (Steam swkotor2.exe 6A522E71): 0x00478A20 does GlobalMemoryStatus(&ms); [0x00A309AC] = (float)ms.dwAvailPhys /
// [0x0098BFE0]; TexMgr::Tick(); and runs twice per frame. The call at 0x00478A57 is `FF 15 28 61 98 00`
// (CALL dword ptr [0x00986128], the KERNEL32 GlobalMemoryStatus IAT slot). Under Wine each real call reads /proc/meminfo.
// The mod re-points that one CALL operand at a DLL-owned function-pointer variable that holds &K2EP_GlobalMemoryStatus.
//
// How K2EnginePerf.cpp should use this API (g_realGms = original IAT pointer read from [0x00986128] at install,
// g_pfnGms = &K2EP_GlobalMemoryStatus, g_state = memstat::State, g_stats = memstat::Stats, both zero-initialised):
//
//   void WINAPI K2EP_GlobalMemoryStatus(LPMEMORYSTATUS out) {
//       g_stats.calls++;
//       unsigned now = GetTickCount();
//       if (memstat::needRefresh(g_state, now, g_intervalMs, g_cacheEnabled)) {
//           MEMORYSTATUS ms; ms.dwLength = sizeof(ms);
//           LARGE_INTEGER t0, t1; QueryPerformanceCounter(&t0);
//           g_realGms(&ms);
//           QueryPerformanceCounter(&t1);
//           g_stats.realCalls++;
//           g_stats.realTicks += (uint64_t)(t1.QuadPart - t0.QuadPart);
//           memstat::store(g_state, ms, now);
//       }
//       memstat::copyOut(g_state, out);   // always; sets out->dwLength = sizeof(MEMORYSTATUS) like the real API
//   }
//
// (g_intervalMs: memstat::clampInterval(ini value); 0 = no caching, every call goes to the real function.)
// Patch: memstat::install(0x00478A57, memstat::kOrigCall, (uint32_t)(uintptr_t)&g_pfnGms, r) then log
// `patch mem_status @0x00478A57 expect=.. seen=.. rc=..` from r.rc / r.seen; on unload memstat::uninstall(...) with the install rc.
#pragma once

#include "CodePatch.h"
#include <stdint.h>
#include <string.h>
#include <windows.h>

#ifdef GUIKIT_HOST
// The host stub windows.h has no MEMORYSTATUS; layout-compatible with <winbase.h> (DWORD/SIZE_T as the stub defines them).
struct _MEMORYSTATUS {
    DWORD dwLength, dwMemoryLoad;
    SIZE_T dwTotalPhys, dwAvailPhys, dwTotalPageFile, dwAvailPageFile, dwTotalVirtual, dwAvailVirtual;
};
typedef _MEMORYSTATUS MEMORYSTATUS;
typedef _MEMORYSTATUS* LPMEMORYSTATUS;
#endif

namespace memstat {

constexpr unsigned kDefaultIntervalMs = 250;
constexpr unsigned kMaxIntervalMs = 5000;

inline unsigned clampInterval(long v) {
    if (v < 0) return 0;
    if (v > static_cast<long>(kMaxIntervalMs)) return kMaxIntervalMs;
    return static_cast<unsigned>(v);
}

struct State {
    MEMORYSTATUS cached;
    bool valid;
    unsigned lastMs;   // GetTickCount() at the last store
};

// True when the caller must run the real GlobalMemoryStatus: caching off, interval 0, nothing cached yet, or the interval
// elapsed. Elapsed time is an unsigned subtraction, so a GetTickCount wrap (0xFFFFFF00 -> 0x00000050) reads as 0x150 ms.
inline bool needRefresh(const State& s, unsigned nowMs, unsigned intervalMs, bool enabled) {
    if (!enabled || !s.valid) return true;
    unsigned iv = intervalMs > kMaxIntervalMs ? kMaxIntervalMs : intervalMs;
    if (iv == 0) return true;
    return static_cast<unsigned>(nowMs - s.lastMs) >= iv;
}

inline void store(State& s, const MEMORYSTATUS& ms, unsigned nowMs) {
    s.cached = ms;
    s.valid = true;
    s.lastMs = nowMs;
}

// Fill the caller's struct like the real API: all fields from the cache, dwLength = sizeof(MEMORYSTATUS).
// With nothing cached the fields are zeroed (the wrapper always refreshes first, so this is a safety net).
inline void copyOut(const State& s, MEMORYSTATUS* out) {
    if (!out) return;
    if (s.valid) *out = s.cached;
    else memset(out, 0, sizeof(*out));
    out->dwLength = sizeof(MEMORYSTATUS);
}

struct Stats {
    uint64_t calls;       // wrapper invocations
    uint64_t realCalls;   // forwarded to the real GlobalMemoryStatus
    uint64_t realTicks;   // QueryPerformanceCounter ticks summed over real calls
    void reset() { calls = 0; realCalls = 0; realTicks = 0; }
};

// ---- CALL operand rewrite: FF 15 <imm32 = address of the DLL-owned function-pointer variable> ----
constexpr int kPatchLen = 6;
inline const uint8_t kOrigCall[kPatchLen] = { 0xFF, 0x15, 0x28, 0x61, 0x98, 0x00 };   // CALL [0x00986128]

inline void buildPatch(uint32_t ptrVarAddr, uint8_t out[kPatchLen]) {
    out[0] = 0xFF; out[1] = 0x15;
    out[2] = static_cast<uint8_t>(ptrVarAddr);
    out[3] = static_cast<uint8_t>(ptrVarAddr >> 8);
    out[4] = static_cast<uint8_t>(ptrVarAddr >> 16);
    out[5] = static_cast<uint8_t>(ptrVarAddr >> 24);
}

struct PatchResult {
    int rc;                    // codepatch::writeChecked: 1 ok | -1 mismatch | -2 protect failed | -4 not committed
    uint8_t seen[kPatchLen];   // bytes found at the site before the compare (zeroed on -4)
};

// Rewrite the 6-byte CALL at siteAddr to call through *ptrVarAddr, only if the site holds `expect`.
inline PatchResult install(uintptr_t siteAddr, const uint8_t expect[kPatchLen], uint32_t ptrVarAddr) {
    PatchResult r;
    memset(r.seen, 0, sizeof(r.seen));
    uint8_t repl[kPatchLen];
    buildPatch(ptrVarAddr, repl);
    r.rc = codepatch::writeChecked(siteAddr, expect, repl, kPatchLen, r.seen);
    return r;
}

// Restore the original bytes, only when install returned rc == 1 and the site still holds our patch. Returns
// 0 (nothing to do: install did not succeed) or the writeChecked rc (1 restored, -1 site changed since, -2, -4).
inline PatchResult uninstall(uintptr_t siteAddr, const uint8_t orig[kPatchLen], uint32_t ptrVarAddr, int installRc) {
    PatchResult r;
    memset(r.seen, 0, sizeof(r.seen));
    if (installRc != 1) { r.rc = 0; return r; }
    uint8_t patched[kPatchLen];
    buildPatch(ptrVarAddr, patched);
    r.rc = codepatch::writeChecked(siteAddr, patched, orig, kPatchLen, r.seen);
    return r;
}

}  // namespace memstat
