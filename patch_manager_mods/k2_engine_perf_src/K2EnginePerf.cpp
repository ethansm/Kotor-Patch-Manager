// K2EnginePerf.cpp -- K2 Engine Perf (0.1.1-dev)
// Two small per-frame CPU savings, each behind an ini switch, measured in a PERF line every 5 s:
//   1. tex_scan_cache: the 3 render-bucket drains of a scene each rebuild the full texture-id list (TexMgr::RebuildIdList
//      0x00427970); the result cannot change between them, so the 2nd and 3rd reuse it (TexScanCache.h).
//   2. mem_status_cache: 0x00478A20 calls GlobalMemoryStatus twice per frame for one float; under Wine each call reads
//      /proc/meminfo. The result is cached for mem_status_ms (MemStatusCache.h).
// No KPM detours: 4 call sites are rewritten at KPatchInit and restored on FreeLibrary (code_patches.toml).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "CodePatch.h"
#include "FileWatch.h"
#include "K2EnginePerfLogic.h"
#include "KpLog.h"
#include "MemStatusCache.h"
#include "SafeRead.h"
#include "TexScanCache.h"

KP_LOG_PATCH("k2-engine-perf");

extern "C" void* __cdecl K2EP_TexScan();
extern "C" void WINAPI K2EP_GlobalMemoryStatus(LPMEMORYSTATUS out);

namespace {

constexpr uintptr_t AddrGmsSite = 0x00478A57;   // CALL dword ptr [0x00986128] in 0x00478A20
constexpr uintptr_t AddrGmsIat = 0x00986128;    // KERNEL32 GlobalMemoryStatus IAT slot

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chCfg, "cfg");
KP_CHANNEL(chPerf, "perf");

const bool g_fallbackSet = (
    KpLog_SetFallbackFile("k2-engine-perf_log.txt"),
    KpLog_SetFallbackLevel(KPLOG_INFO),
    true);

using ScanFn = void*(__cdecl*)();
using GmsFn = void(WINAPI*)(LPMEMORYSTATUS);

k2ep::Config g_cfg{};
filewatch::ReloadPoll g_iniPoll{};
bool g_bannerLogged = false;
uint32_t g_reloadCount = 0;
k2ep::PerfWindow g_perf{};
int64_t g_qpcFreq = 0;

// Texture-id scan cache.
texscan::Sites g_scanSites{};
texscan::State g_scanState{};
texscan::Stats g_scanStats{};
bool g_scanGlobalsOk = false;   // the 5 engine globals were readable at install (exe image, never unmapped after)
bool g_scanTripped = false;     // a verify mismatch turned the cache off for this session
const ScanFn g_origScan = reinterpret_cast<ScanFn>(texscan::kOrigFn);

// GlobalMemoryStatus cache.
GmsFn g_realGms = nullptr;
GmsFn g_pfnGms = &K2EP_GlobalMemoryStatus;   // the patched CALL reads its target from here
memstat::State g_gmsState{};
memstat::Stats g_gmsStats{};
memstat::PatchResult g_gmsPatch{};
bool g_installed = false;

uint64_t qpcNow() {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return static_cast<uint64_t>(t.QuadPart);
}

void ensureDefaultIniFile() {
    FILE* f = fopen(k2ep::kIniFile, "rb");
    if (f) {
        fclose(f);
        return;
    }
    char buf[2048];
    const int n = k2ep::formatDefaultIni(buf, sizeof(buf), g_cfg);
    if (n > 0 && static_cast<size_t>(n) < sizeof(buf)) {
        filewatch::atomicWrite(k2ep::kIniFile, buf, static_cast<size_t>(n));
    }
}

void loadAndApplyIni(bool initial) {
    ensureDefaultIniFile();
    FILE* f = fopen(k2ep::kIniFile, "rb");
    k2ep::Config next{};
    inikit::LoadStats st{};
    bool readOk = false;
    if (f) {
        char buf[4096];
        const size_t n = fread(buf, 1, sizeof(buf) - 1, f);
        buf[n] = '\0';
        fclose(f);
        st = k2ep::parseIniText(buf, next);
        readOk = true;
    }
    const bool changed = initial || (next != g_cfg);
    if (next.texScanCache != g_cfg.texScanCache) texscan::invalidate(g_scanState);
    g_cfg = next;
    ++g_reloadCount;
    if (!g_bannerLogged) {
        g_bannerLogged = true;
        KP_LOG(chMain, "%s %s loaded: tex_scan_cache=%d tex_verify_every=%d mem_status_cache=%d mem_status_ms=%d perf_log=%d verbose=%d ini=%s(ok=%d,bad=%d)",
               k2ep::kModId, k2ep::kVersion, g_cfg.texScanCache ? 1 : 0, g_cfg.texVerifyEvery, g_cfg.memStatusCache ? 1 : 0,
               g_cfg.memStatusMs, g_cfg.perfLog ? 1 : 0, g_cfg.verbose ? 1 : 0, readOk ? "read" : "defaults", st.ok, st.bad);
    } else if (changed || g_cfg.verbose) {
        KP_LOG(chCfg, "INI reload #%u: tex_scan_cache=%d tex_verify_every=%d mem_status_cache=%d mem_status_ms=%d perf_log=%d",
               g_reloadCount, g_cfg.texScanCache ? 1 : 0, g_cfg.texVerifyEvery, g_cfg.memStatusCache ? 1 : 0,
               g_cfg.memStatusMs, g_cfg.perfLog ? 1 : 0);
    }
}

void logPatch(const char* name, uintptr_t at, const uint8_t* expect, const uint8_t* seen, int n, int rc) {
    char e[32] = {}, s[32] = {};
    for (int i = 0; i < n; ++i) {
        snprintf(e + i * 3, sizeof(e) - i * 3, "%02X ", expect[i]);
        snprintf(s + i * 3, sizeof(s) - i * 3, "%02X ", seen[i]);
    }
    KP_LOG(chMain, "patch %s @0x%08X expect=%s seen=%s rc=%d", name, static_cast<unsigned>(at), e, s, rc);
    if (rc != 1) KP_WARN("%s NOT patched (rc=%d): that call stays vanilla", name, rc);
}

void installPatches() {
    if (g_installed) return;
    g_installed = true;
    LARGE_INTEGER fq;
    if (QueryPerformanceFrequency(&fq)) g_qpcFreq = fq.QuadPart;

    // Texture-id scan: every global the replacement reads must be readable once; they live in the exe image.
    g_scanGlobalsOk = saferead::probeRead(1, reinterpret_cast<const void*>(texscan::kTickAddr), 4) &&
                      saferead::probeRead(2, reinterpret_cast<const void*>(texscan::kSheetCountAddr), 4) &&
                      saferead::probeRead(3, reinterpret_cast<const void*>(texscan::kPendingAddr), 4) &&
                      saferead::probeRead(4, reinterpret_cast<const void*>(texscan::kVecCountAddr), 4) &&
                      saferead::probeRead(5, reinterpret_cast<const void*>(texscan::kMaxIdAddr), 4);
    if (g_scanGlobalsOk) {
        texscan::initSites(g_scanSites, texscan::kSiteAddr, texscan::kOrigFn, reinterpret_cast<uintptr_t>(&K2EP_TexScan));
        texscan::install(g_scanSites);
        for (int i = 0; i < texscan::kSiteCount; ++i) {
            char name[16];
            snprintf(name, sizeof(name), "tex_scan_%d", i + 1);
            const texscan::Site& p = g_scanSites.site[i];
            logPatch(name, p.at, p.expect, p.seen, 5, p.rc);
        }
    } else {
        KP_WARN("texture-scan globals unreadable: tex_scan sites NOT patched");
    }

    // GlobalMemoryStatus: keep the real IAT target, then point the one CALL operand at g_pfnGms.
    uint32_t iat = 0;
    if (saferead::readAt<uint32_t>(6, AddrGmsIat, 0, &iat) && iat != 0) {
        g_realGms = reinterpret_cast<GmsFn>(static_cast<uintptr_t>(iat));
        g_gmsPatch = memstat::install(AddrGmsSite, memstat::kOrigCall,
                                      static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&g_pfnGms)));
        logPatch("mem_status", AddrGmsSite, memstat::kOrigCall, g_gmsPatch.seen, memstat::kPatchLen, g_gmsPatch.rc);
    } else {
        KP_WARN("GlobalMemoryStatus IAT slot 0x%08X unreadable: mem_status NOT patched", static_cast<unsigned>(AddrGmsIat));
    }
}

// FreeLibrary only: restore the sites before this DLL's code goes away.
void uninstallPatches() {
    texscan::uninstall(g_scanSites);
    memstat::uninstall(AddrGmsSite, memstat::kOrigCall, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&g_pfnGms)),
                       g_gmsPatch.rc);
}

void perfFlush(unsigned now) {
    if (!g_perf.started) {
        g_perf.reset(now);
        return;
    }
    if (!g_perf.due(now)) return;
    const float secs = g_perf.seconds(now);
    if (g_cfg.perfLog) {
        const k2ep::SavedEstimate ts = k2ep::estimateSaved(g_scanStats.missTicks, static_cast<uint32_t>(g_scanStats.misses),
                                                           static_cast<uint32_t>(g_scanStats.hits), g_qpcFreq, secs);
        const k2ep::SavedEstimate gs = k2ep::estimateSaved(g_gmsStats.realTicks, static_cast<uint32_t>(g_gmsStats.realCalls),
                                                           static_cast<uint32_t>(g_gmsStats.calls - g_gmsStats.realCalls),
                                                           g_qpcFreq, secs);
        KP_LOG(chPerf,
               "PERF %.1fs tex calls=%llu hits=%llu misses=%llu scan_us=%.1f saved_us_s=%.0f verify=%llu mism=%llu%s | "
               "gms calls=%llu real=%llu real_us=%.1f saved_us_s=%.0f",
               secs, static_cast<unsigned long long>(g_scanStats.calls), static_cast<unsigned long long>(g_scanStats.hits),
               static_cast<unsigned long long>(g_scanStats.misses), ts.avgUs, ts.savedUsPerSec,
               static_cast<unsigned long long>(g_scanStats.verifies), static_cast<unsigned long long>(g_scanStats.mismatches),
               g_scanTripped ? " TRIPPED" : "", static_cast<unsigned long long>(g_gmsStats.calls),
               static_cast<unsigned long long>(g_gmsStats.realCalls), gs.avgUs, gs.savedUsPerSec);
    }
    g_scanStats.reset();
    g_gmsStats.reset();
    g_perf.reset(now);
}

// Shared per-frame work (both replacements run every frame): ini poll (1 s throttle inside) and the PERF window.
void onFrame() {
    const unsigned now = GetTickCount();
    if (filewatch::changed(g_iniPoll, k2ep::kIniFile, now)) loadAndApplyIni(false);
    perfFlush(now);
}

} // namespace

extern "C" {

void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    filewatch::changed(g_iniPoll, k2ep::kIniFile, GetTickCount(), true);
    loadAndApplyIni(true);
    installPatches();
}

// Replaces `CALL 0x00427970` at the 3 bucket-drain sites. Returns &vec like the original; a plain hit leaves the vector
// exactly as the previous real scan of this tick left it.
void* __cdecl K2EP_TexScan() {
    const uint32_t tick = *reinterpret_cast<const volatile uint32_t*>(texscan::kTickAddr);
    const int32_t sheets = *reinterpret_cast<const volatile int32_t*>(texscan::kSheetCountAddr);
    const int32_t pending = *reinterpret_cast<const volatile int32_t*>(texscan::kPendingAddr);
    ++g_scanStats.calls;
    const bool enabled = g_cfg.texScanCache && !g_scanTripped;
    if (texscan::canReuse(g_scanState, tick, sheets, pending, enabled)) {
        if (!texscan::onHit(g_scanStats, static_cast<uint32_t>(g_cfg.texVerifyEvery))) {
            return reinterpret_cast<void*>(texscan::kVecAddr);
        }
        void* r = g_origScan();
        const uint32_t cnt = *reinterpret_cast<const volatile uint32_t*>(texscan::kVecCountAddr);
        const uint32_t mx = *reinterpret_cast<const volatile uint32_t*>(texscan::kMaxIdAddr);
        const bool same = texscan::sameResult(g_scanState.count, g_scanState.maxId, cnt, mx);
        texscan::onVerify(g_scanStats, same);
        if (!same) {
            // The key missed a change: stop reusing for this session (safety first) and say so once.
            KP_ERR("tex_scan verify MISMATCH tick=%u sheets=%d cached count=%u max=%u real count=%u max=%u: cache OFF for this session",
                    tick, sheets, g_scanState.count, g_scanState.maxId, cnt, mx);
            g_scanTripped = true;
            texscan::invalidate(g_scanState);
        }
        return r;
    }
    const uint64_t t0 = qpcNow();
    void* r = g_origScan();
    texscan::onMiss(g_scanStats, qpcNow() - t0);
    const uint32_t cnt = *reinterpret_cast<const volatile uint32_t*>(texscan::kVecCountAddr);
    const uint32_t mx = *reinterpret_cast<const volatile uint32_t*>(texscan::kMaxIdAddr);
    texscan::remember(g_scanState, tick, sheets, pending, cnt, mx);
    onFrame();
    return r;
}

// Replaces the GlobalMemoryStatus IAT call at 0x00478A57 (stdcall, same signature as the API).
void WINAPI K2EP_GlobalMemoryStatus(LPMEMORYSTATUS out) {
    ++g_gmsStats.calls;
    const unsigned now = GetTickCount();
    const unsigned interval = memstat::clampInterval(g_cfg.memStatusMs);
    if (memstat::needRefresh(g_gmsState, now, interval, g_cfg.memStatusCache)) {
        MEMORYSTATUS ms;
        memset(&ms, 0, sizeof(ms));
        ms.dwLength = sizeof(ms);
        const uint64_t t0 = qpcNow();
        g_realGms(&ms);
        g_gmsStats.realTicks += qpcNow() - t0;
        ++g_gmsStats.realCalls;
        memstat::store(g_gmsState, ms, now);
    }
    memstat::copyOut(g_gmsState, out);
    onFrame();
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
    } else if (fdwReason == DLL_PROCESS_DETACH && lpvReserved == nullptr) {
        uninstallPatches();
    }
    return TRUE;
}
