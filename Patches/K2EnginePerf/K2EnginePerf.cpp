// K2EnginePerf.cpp -- K2 Engine Perf (0.2.0-dev)
// Per-frame CPU savings & pass QPC profiler behind ini switches, measured in PERF / PERF_PASS lines every 5 s:
//   mem_status_cache: 0x00478A20 calls GlobalMemoryStatus twice per frame for one float; under Wine each call reads
//   /proc/meminfo. The result is cached for mem_status_ms (MemStatusCache.h).
//   pass_profile: 5-byte E8 rel32 call-target timing wrappers (PassProfiler.h) across 7 emitter, mesh-bucket,
//   skinning, animation, and shadow projection call sites.
// No KPM detours: call sites are rewritten via codepatch::writeChecked at KPatchInit and restored on FreeLibrary (code_patches.toml).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../Common/CodePatch.h"
#include "../Common/FileWatch.h"
#include "K2EnginePerfLogic.h"
#include "../Common/KpLog.h"
#include "MemStatusCache.h"
#include "PassProfiler.h"
#include "../Common/SafeRead.h"

KP_LOG_PATCH("k2-engine-perf");

extern "C" void WINAPI K2EP_GlobalMemoryStatus(LPMEMORYSTATUS out);

namespace {

constexpr uintptr_t AddrGmsSite = 0x00478A57;   // CALL dword ptr [0x00986128] in 0x00478A20
constexpr uintptr_t AddrGmsIat = 0x00986128;    // KERNEL32 GlobalMemoryStatus IAT slot

constexpr uintptr_t AddrEmitSimSite = 0x00469A88;     // CALL 0x004EC080 (PartEmitter::AnimateParticles, thiscall ret 4)
constexpr uintptr_t AddrEmitSimFn = 0x004EC080;
constexpr uintptr_t AddrEmitDrawSite = 0x00469A99;    // CALL 0x004E92A0 (PartEmitter::RenderFunc, thiscall)
constexpr uintptr_t AddrEmitDrawFn = 0x004E92A0;
constexpr uintptr_t AddrBlendModeSite = 0x004660CA;   // CALL 0x004DB1F0 (Material::SetBlendingMode, thiscall ret 4)
constexpr uintptr_t AddrBlendModeFn = 0x004DB1F0;
constexpr uintptr_t AddrMeshDedupeSite = 0x00466161;  // CALL 0x00476620 (PtrVec::CountOf, thiscall ret 4)
constexpr uintptr_t AddrMeshDedupeFn = 0x00476620;
constexpr uintptr_t AddrSkinDrawSite = 0x004CD3ED;  // CALL 0x004C9640 (Global::RenderSkinnedVPBlur, cdecl 1 arg)
constexpr uintptr_t AddrSkinDrawFn = 0x004C9640;
constexpr uintptr_t AddrGobAnimSite = 0x004E2ED1;     // CALL 0x004DFED0 (Global::AnimateHierarchy, cdecl 6 args)
constexpr uintptr_t AddrGobAnimFn = 0x004DFED0;
constexpr uintptr_t AddrShdwProjSite = 0x00468DC1;    // CALL 0x00465C90 (Global::ProjectDynamicShadow, cdecl 2 args)
constexpr uintptr_t AddrShdwProjFn = 0x00465C90;

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chCfg, "cfg");
KP_CHANNEL(chPerf, "perf");

const bool g_fallbackSet = (
    KpLog_SetFallbackFile("k2-engine-perf_log.txt"),
    KpLog_SetFallbackLevel(KPLOG_INFO),
    true);

using GmsFn = void(WINAPI*)(LPMEMORYSTATUS);

k2ep::Config g_cfg{};
filewatch::ReloadPoll g_iniPoll{};
bool g_bannerLogged = false;
uint32_t g_reloadCount = 0;
k2ep::PerfWindow g_perf{};
int64_t g_qpcFreq = 0;

// GlobalMemoryStatus cache.
GmsFn g_realGms = nullptr;
GmsFn g_pfnGms = &K2EP_GlobalMemoryStatus;   // the patched CALL reads its target from here
memstat::State g_gmsState{};
memstat::Stats g_gmsStats{};
memstat::PatchResult g_gmsPatch{};

// Pass profiler state.
passprof::WindowState g_passWin{};
passprof::CallPatchResult g_passPatches[passprof::kPassCount]{};
bool g_installed = false;

uint64_t qpcNow() {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return static_cast<uint64_t>(t.QuadPart);
}

// ---- Pass timing wrappers (signatures match each patched CALL site ABI exactly) ----
void __fastcall K2EP_EmitSim(void* self, void* /*edx*/, uint32_t dtBits) {
    using Fn = void(__thiscall*)(void*, uint32_t);
    auto fn = reinterpret_cast<Fn>(AddrEmitSimFn);
    if (!g_cfg.passProfile) { fn(self, dtBits); return; }
    const uint64_t t0 = qpcNow();
    fn(self, dtBits);
    g_passWin.record(passprof::kEmitSim, qpcNow() - t0);
}

void __fastcall K2EP_EmitDraw(void* self, void* /*edx*/) {
    using Fn = void(__thiscall*)(void*);
    auto fn = reinterpret_cast<Fn>(AddrEmitDrawFn);
    if (!g_cfg.passProfile) { fn(self); return; }
    const uint64_t t0 = qpcNow();
    fn(self);
    g_passWin.record(passprof::kEmitDraw, qpcNow() - t0);
}

void __fastcall K2EP_BlendMode(void* self, void* /*edx*/, int idx) {
    using Fn = void(__thiscall*)(void*, int);
    auto fn = reinterpret_cast<Fn>(AddrBlendModeFn);
    if (!g_cfg.passProfile) { fn(self, idx); return; }
    const uint64_t t0 = qpcNow();
    fn(self, idx);
    g_passWin.record(passprof::kBlendMode, qpcNow() - t0);
}

// PtrVec::CountOf is an 81-byte callee in a per-item loop: the two QPC reads can cost more than the function, so
// dedupe_ub_ms_s is an UPPER BOUND (includes timer overhead), not a true cost.
int __fastcall K2EP_MeshDedupe(void* self, void* /*edx*/, uint32_t item) {
    using Fn = int(__thiscall*)(void*, uint32_t);
    auto fn = reinterpret_cast<Fn>(AddrMeshDedupeFn);
    if (!g_cfg.passProfile) return fn(self, item);
    const uint64_t t0 = qpcNow();
    const int r = fn(self, item);
    g_passWin.record(passprof::kMeshDedupe, qpcNow() - t0);
    return r;
}

void __cdecl K2EP_SkinDraw(uint32_t arg) {
    using Fn = void(__cdecl*)(uint32_t);
    auto fn = reinterpret_cast<Fn>(AddrSkinDrawFn);
    if (!g_cfg.passProfile) { fn(arg); return; }
    const uint64_t t0 = qpcNow();
    fn(arg);
    g_passWin.record(passprof::kSkinDraw, qpcNow() - t0);
}

void __cdecl K2EP_GobAnim(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6) {
    using Fn = void(__cdecl*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    auto fn = reinterpret_cast<Fn>(AddrGobAnimFn);
    if (!g_cfg.passProfile) { fn(a1, a2, a3, a4, a5, a6); return; }
    const uint64_t t0 = qpcNow();
    fn(a1, a2, a3, a4, a5, a6);
    g_passWin.record(passprof::kGobAnim, qpcNow() - t0);
}

void __cdecl K2EP_ShdwProj(uint32_t a1, uint32_t a2) {
    using Fn = void(__cdecl*)(uint32_t, uint32_t);
    auto fn = reinterpret_cast<Fn>(AddrShdwProjFn);
    if (!g_cfg.passProfile) { fn(a1, a2); return; }
    const uint64_t t0 = qpcNow();
    fn(a1, a2);
    g_passWin.record(passprof::kShdwProj, qpcNow() - t0);
}

struct PassSite {
    const char* name;
    uintptr_t site;
    uint8_t orig[passprof::kCallLen];
    uintptr_t wrapper;
};

const PassSite kPassSites[passprof::kPassCount] = {
    { "emit_sim",    AddrEmitSimSite,    { 0xE8, 0xF3, 0x25, 0x08, 0x00 }, reinterpret_cast<uintptr_t>(&K2EP_EmitSim) },
    { "emit_draw",   AddrEmitDrawSite,   { 0xE8, 0x02, 0xF8, 0x07, 0x00 }, reinterpret_cast<uintptr_t>(&K2EP_EmitDraw) },
    { "blend_mode",  AddrBlendModeSite,  { 0xE8, 0x21, 0x51, 0x07, 0x00 }, reinterpret_cast<uintptr_t>(&K2EP_BlendMode) },
    { "mesh_dedupe", AddrMeshDedupeSite, { 0xE8, 0xBA, 0x04, 0x01, 0x00 }, reinterpret_cast<uintptr_t>(&K2EP_MeshDedupe) },
    { "skin_draw", AddrSkinDrawSite, { 0xE8, 0x4E, 0xC2, 0xFF, 0xFF }, reinterpret_cast<uintptr_t>(&K2EP_SkinDraw) },
    { "gob_anim",    AddrGobAnimSite,    { 0xE8, 0xFA, 0xCF, 0xFF, 0xFF }, reinterpret_cast<uintptr_t>(&K2EP_GobAnim) },
    { "shdw_proj",   AddrShdwProjSite,   { 0xE8, 0xCA, 0xCE, 0xFF, 0xFF }, reinterpret_cast<uintptr_t>(&K2EP_ShdwProj) },
};

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
    g_cfg = next;
    ++g_reloadCount;
    if (!g_bannerLogged) {
        g_bannerLogged = true;
        KP_LOG(chMain, "%s %s loaded: mem_status_cache=%d mem_status_ms=%d pass_profile=%d perf_log=%d verbose=%d ini=%s(ok=%d,bad=%d)",
               k2ep::kModId, k2ep::kVersion, g_cfg.memStatusCache ? 1 : 0, g_cfg.memStatusMs,
               g_cfg.passProfile ? 1 : 0, g_cfg.perfLog ? 1 : 0, g_cfg.verbose ? 1 : 0,
               readOk ? "read" : "defaults", st.ok, st.bad);
    } else if (changed || g_cfg.verbose) {
        KP_LOG(chCfg, "INI reload #%u: mem_status_cache=%d mem_status_ms=%d pass_profile=%d perf_log=%d",
               g_reloadCount, g_cfg.memStatusCache ? 1 : 0, g_cfg.memStatusMs,
               g_cfg.passProfile ? 1 : 0, g_cfg.perfLog ? 1 : 0);
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

    for (int i = 0; i < passprof::kPassCount; ++i) {
        const PassSite& ps = kPassSites[i];
        g_passPatches[i] = passprof::installCall(ps.site, ps.orig, ps.wrapper);
        logPatch(ps.name, ps.site, ps.orig, g_passPatches[i].seen, passprof::kCallLen, g_passPatches[i].rc);
    }
}

// FreeLibrary only: restore the sites before this DLL's code goes away.
void uninstallPatches() {
    memstat::uninstall(AddrGmsSite, memstat::kOrigCall, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&g_pfnGms)),
                       g_gmsPatch.rc);
    for (int i = 0; i < passprof::kPassCount; ++i) {
        const PassSite& ps = kPassSites[i];
        passprof::uninstallCall(ps.site, ps.orig, ps.wrapper, g_passPatches[i].rc);
    }
}

void perfFlush(unsigned now) {
    if (!g_perf.started) {
        g_perf.reset(now);
        return;
    }
    if (!g_perf.due(now)) return;
    const float secs = g_perf.seconds(now);
    if (g_cfg.perfLog) {
        const k2ep::SavedEstimate gs = k2ep::estimateSaved(g_gmsStats.realTicks, static_cast<uint32_t>(g_gmsStats.realCalls),
                                                           static_cast<uint32_t>(g_gmsStats.calls - g_gmsStats.realCalls),
                                                           g_qpcFreq, secs);
        KP_LOG(chPerf, "PERF %.1fs gms calls=%llu real=%llu real_us=%.1f saved_us_s=%.0f",
               secs, static_cast<unsigned long long>(g_gmsStats.calls), static_cast<unsigned long long>(g_gmsStats.realCalls),
               gs.avgUs, gs.savedUsPerSec);
        if (g_cfg.passProfile) {
            const passprof::PassSummary es = passprof::summarize(g_passWin.pass[passprof::kEmitSim], g_qpcFreq, secs);
            const passprof::PassSummary ed = passprof::summarize(g_passWin.pass[passprof::kEmitDraw], g_qpcFreq, secs);
            const passprof::PassSummary sk = passprof::summarize(g_passWin.pass[passprof::kSkinDraw], g_qpcFreq, secs);
            const passprof::PassSummary an = passprof::summarize(g_passWin.pass[passprof::kGobAnim], g_qpcFreq, secs);
            const passprof::PassSummary sh = passprof::summarize(g_passWin.pass[passprof::kShdwProj], g_qpcFreq, secs);
            const passprof::PassSummary st = passprof::summarize(g_passWin.pass[passprof::kBlendMode], g_qpcFreq, secs);
            const passprof::PassSummary dd = passprof::summarize(g_passWin.pass[passprof::kMeshDedupe], g_qpcFreq, secs);
            KP_LOG(chPerf,
                   "PERF_PASS %.1fs emit_sim_ms_s=%.2f(%u,max=%.2f) emit_draw_ms_s=%.2f(%u) skin_draw_ms_s=%.2f(%u) anim_ms_s=%.2f(%u) shdw_ms_s=%.2f(%u) blend_ms_s=%.2f(%u) dedupe_ub_ms_s=%.2f(%u)",
                   secs,
                   es.msPerSec, es.calls, es.maxCallMs,
                   ed.msPerSec, ed.calls,
                   sk.msPerSec, sk.calls,
                   an.msPerSec, an.calls,
                   sh.msPerSec, sh.calls,
                   st.msPerSec, st.calls,
                   dd.msPerSec, dd.calls);
        }
    }
    g_gmsStats.reset();
    g_passWin.reset();
    g_perf.reset(now);
}

// Per-frame work (the replacement runs every frame): ini poll (1 s throttle inside) and the PERF window.
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
