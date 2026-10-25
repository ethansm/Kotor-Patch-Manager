// K2GrassDrawDistance.cpp -- K2 Grass Draw Distance (0.1.5-dev)
// Extends grass octree build/collect radius, fade-start/fade-end distances, alpha-test threshold,
// and decouples CPU wind animation radius (one operand rewrite at KPatchInit, code_patches.toml).
// 0.1.4: a second operand rewrite trims the octree collect radius to the visible clip distance (trim_hidden), two
// detours inside Scene::DoBinBucket time the grass bucket loop (PERF grass_ms_*) and optionally enable alpha-to-coverage
// around it (smooth_edges, only when the GL context has a multisample buffer).
// 0.1.5: speed experiments, all measured in the PERF line and off by default except the log-only timers:
//   PF2d gpu_timing    GL_TIME_ELAPSED query around the bucket loop (gpu_ms_*), never blocks on a result
//   PF2b wind_throttle full wind radius only every wind_every-th frame (g_windDistSq set once per frame, no new hook)
//   PF2c far_lod       RenderGrassPolys entry hook picks a per-bin keep fraction; the two glDrawArrays(GL_QUADS) calls in
//                      the grass draw helper get a smaller count (nothing else is touched: the helper's offset argument
//                      is also the normals/UV offset)
//   PF2a path probe    decodes which grass draw setup branch the engine globals select (log + per-window bin counts)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cmath>

#define INIW_FILE_API 1
#define MCDECL_FILE_API 1

#include "CodePatch.h"
#include "FileWatch.h"
#include "K2GrassDrawDistanceLogic.h"
#include "KpLog.h"
#include "ModConfigApi.h"
#include "ModConfigDecl.h"
#include "ProviderClient.h"
#include "SafeRead.h"

KP_LOG_PATCH("k2-grass-draw-distance");

namespace {

// Engine .data globals (Steam Aspyr build 6A522E71...). The 0.1.5 path probe globals are listed after AddrGrassPrevBinCount.
//   0x00A0F070: float _DAT_00a0f070 (vanilla 225.0f = 15m^2; RenderGrassPolys fade start, and the wind check
//               until patchWindOperand points that FLD at g_windDistSq)
//   0x00A0F074: float _DAT_00a0f074 (vanilla 900.0f = 30m^2; octree sphere query & RenderGrassPolys fade_end_sq)
//   0x009F5D38: float _DAT_009f5d38 (vanilla 0.40f; Scene::DoBinBucket glAlphaFunc threshold)
constexpr uintptr_t AddrGrassFadeStartSq = 0x00A0F070;
constexpr uintptr_t AddrGrassFadeEndSq = 0x00A0F074;
constexpr uintptr_t AddrGrassAlphaCutoff = 0x009F5D38;
// RenderGrassPolys wind gate `FLD float ptr [0x00A0F070]`; only the imm32 changes, to &g_windDistSq.
constexpr uintptr_t AddrWindFld = 0x0050DCC8;
const uint8_t kWindFldOrig[6] = {0xD9, 0x05, 0x70, 0xF0, 0xA0, 0x00};
// Scene::CollectAndTraverse octree sphere query radius `FLD float ptr [0x00A0F074]`; the imm32 becomes &g_collectSq
// (0x0050DB4C keeps reading the real fade end, so the fade curve is unchanged).
constexpr uintptr_t AddrCollectFld = 0x00499ADC;
const uint8_t kCollectFldOrig[6] = {0xD9, 0x05, 0x74, 0xF0, 0xA0, 0x00};
// Game import slots holding the __stdcall GL entry points the engine itself calls through.
constexpr uintptr_t AddrSlotGlEnable = 0x00986268;
constexpr uintptr_t AddrSlotGlDisable = 0x009862A0;
constexpr uintptr_t AddrSlotGlGetIntegerv = 0x009862CC;
constexpr unsigned GlSampleAlphaToCoverage = 0x809E;
constexpr unsigned GlSampleBuffers = 0x80A8;
constexpr unsigned GlSamples = 0x80A9;
// int: count of the previous frame's in-range grass bin list (list = {ptr, count}, count at +4). The per-frame
// list swap in Scene::CollectAndTraverse copies the current list there before hook 1 runs (doc 48 section 1.5).
constexpr uintptr_t AddrGrassPrevBinCount = 0x00A73304;
// PF2a path probe inputs (read with saferead only; the game functions that derive the path cache on first call, so the
// probe never calls them): F = u32 flags word, N = int, the two flag ints (low bit), the two -1-sentinel ints, plus the
// int that splits bins into path A / B and a byte logged for context.
constexpr uintptr_t AddrPathFlags = 0x00A32DF8;
constexpr uintptr_t AddrPathN = 0x009F60F0;
constexpr uintptr_t AddrPathFlag60EC = 0x009F60EC;
constexpr uintptr_t AddrPathFlag62DC = 0x009F62DC;
constexpr uintptr_t AddrPathC60FC = 0x009F60FC;
constexpr uintptr_t AddrPathC6118 = 0x009F6118;
constexpr uintptr_t AddrPathF07C = 0x00A0F07C;
constexpr uintptr_t AddrPathBac4 = 0x00A7BAC4;
// RenderGrassPolys entry (bin in ECX; float dist^2 at bin+0x04, byte flags at bin+0x02).
constexpr uintptr_t kBinOffDistSq = 0x04;
constexpr uintptr_t kBinOffFlags = 0x02;
constexpr unsigned GlQuads = 7;              // GL_QUADS, the mode the grass draw helper passes to glDrawArrays
constexpr unsigned GlExtensions = 0x1F03;    // GL_EXTENSIONS
constexpr unsigned GlTimeElapsed = 0x88BF;   // GL_TIME_ELAPSED
constexpr unsigned GlQueryResult = 0x8866;
constexpr unsigned GlQueryResultAvailable = 0x8867;
constexpr int kGpuRing = 8;

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chCfg, "cfg");
KP_CHANNEL(chPerf, "perf");

const bool g_fallbackSet = (
    KpLog_SetFallbackFile("k2-grass-draw-distance_log.txt"),
    KpLog_SetFallbackLevel(KPLOG_INFO),
    true);

k2grass::Config g_cfg{};
k2grass::EffectiveValues g_eff = k2grass::computeEffective(g_cfg);
filewatch::ReloadPoll g_iniPoll{};
bool g_initialized = false;
bool g_bannerLogged = false;
uint32_t g_collectCalls = 0;
uint32_t g_reloadCount = 0;
// Wind cutoff (squared m) the patched FLD reads. 0 = no wind (bin d^2 is never < 0).
float g_windDistSq = 225.0f;
// Octree collect radius (squared m) the patched FLD at AddrCollectFld reads; the vanilla value until the first apply.
float g_collectSq = 900.0f;
uint8_t g_windFldNew[6] = {};
int g_windPatch = 0;   // codepatch::writeChecked result; 1 = patched
uint8_t g_collectFldNew[6] = {};
int g_collectPatch = 0;
k2grass::PerfWindow g_perf{};
// Grass bucket loop timing / alpha-to-coverage state (render thread only).
LARGE_INTEGER g_qpcFreq{};
LARGE_INTEGER g_loopT0{};
bool g_loopTiming = false;
bool g_a2cChecked = false;
bool g_a2cCapable = false;
bool g_a2cOn = false;     // a2c is enabled between this frame's Begin and End
// True between Begin and End of the bucket loop (far-LOD draw hooks act only then).
bool g_inGrass = false;
// Keep fraction of the bin RenderGrassPolys is currently drawing (set by K2GrassOnBin, 1 = draw everything).
float g_binKeep = 1.0f;
uint32_t g_windFrame = 0;   // grass frames seen by the collect hook (wind throttle phase)

// PF2d GPU timer: GL_TIME_ELAPSED queries in a ring; a result is read only once the driver says it is available.
using GlGetStringFn = const unsigned char*(__stdcall*)(unsigned);
using WglGetProcFn = void*(__stdcall*)(const char*);
using GlGenQueriesFn = void(__stdcall*)(int, unsigned*);
using GlBeginQueryFn = void(__stdcall*)(unsigned, unsigned);
using GlEndQueryFn = void(__stdcall*)(unsigned);
using GlGetQueryObjectivFn = void(__stdcall*)(unsigned, unsigned, int*);
using GlGetQueryObjectui64vFn = void(__stdcall*)(unsigned, unsigned, uint64_t*);
struct GpuTimer {
    bool checked = false;
    bool ok = false;
    bool idsReady = false;
    bool active = false;     // a query was begun and not yet ended
    int head = 0;            // oldest pending slot
    int pending = 0;         // ended queries whose result has not been read
    unsigned ids[kGpuRing] = {};
    GlGenQueriesFn genQueries = nullptr;
    GlBeginQueryFn beginQuery = nullptr;
    GlEndQueryFn endQuery = nullptr;
    GlGetQueryObjectivFn getObjectiv = nullptr;
    GlGetQueryObjectui64vFn getObjectui64v = nullptr;
};
GpuTimer g_gpu{};

// PF2a path probe state: last decoded path and raw inputs (logged on change), and the per-bin split input.
bool g_pathSeen = false;
uint32_t g_pathLastF = 0;
k2grass::GrassPath g_pathLast{};
bool g_pathReadable = false;   // all probe globals were readable at the last Begin (K2GrassOnBin may then read AddrPathF07C)

// Optional Mod Configuration menu integration (k2.modconfig@1 / k2.menuui@1):
k2mc::ConfigClient g_mcCfg;
k2mc::MenuClient g_mcMenu;
k2pc::RegisterOnce g_mcReg;
mcdecl::Page g_page;
K2ModConfigEntry g_mcEntry{};
bool g_pageReady = false;

void mcLog(int level, const char* msg) {
    KP_LOGV(chMain, level, "%s", msg);
}

void writeEngineGlobals(const k2grass::EffectiveValues& ev) {
    g_windDistSq = ev.windDistSq;
    if (saferead::probeRead(1, reinterpret_cast<const void*>(AddrGrassFadeStartSq), sizeof(float))) {
        *reinterpret_cast<float*>(AddrGrassFadeStartSq) = ev.fadeStartSq;
    }
    if (saferead::probeRead(2, reinterpret_cast<const void*>(AddrGrassFadeEndSq), sizeof(float))) {
        *reinterpret_cast<float*>(AddrGrassFadeEndSq) = ev.fadeEndSq;
    }
    if (saferead::probeRead(3, reinterpret_cast<const void*>(AddrGrassAlphaCutoff), sizeof(float))) {
        *reinterpret_cast<float*>(AddrGrassAlphaCutoff) = ev.alphaCutoff;
    }
}

// Rewrite the imm32 of a `FLD float ptr [abs]` (D9 05 imm32) to point at `target`. On a byte mismatch nothing is
// written (logged); the engine keeps reading its own global.
int patchFldOperand(const char* name, uintptr_t at, const uint8_t* orig, uint8_t* repl, const float* target) {
    memcpy(repl, orig, 2);
    const uint32_t a = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(target));
    memcpy(repl + 2, &a, 4);
    uint8_t seen[6] = {};
    const int rc = codepatch::writeChecked(at, orig, repl, 6, seen);
    KP_LOG(chMain, "patch %s @0x%08X rc=%d seen=%02X %02X %02X %02X %02X %02X new_operand=0x%08X", name,
           static_cast<unsigned>(at), rc, seen[0], seen[1], seen[2], seen[3], seen[4], seen[5], a);
    return rc;
}

// Point the wind gate's FLD at g_windDistSq so wind_dist and fade_start_dist are independent. On a byte
// mismatch nothing is written and wind stays coupled to fade start (vanilla behaviour, logged).
void patchWindOperand() {
    if (g_windPatch == 1) return;
    g_windPatch = patchFldOperand("wind_fld", AddrWindFld, kWindFldOrig, g_windFldNew, &g_windDistSq);
    if (g_windPatch != 1) KP_WARN("wind operand NOT patched: wind radius follows fade start");
}

// Point the octree query's FLD at g_collectSq (trim_hidden). On a mismatch the collect radius stays the fade end.
void patchCollectOperand() {
    if (g_collectPatch == 1) return;
    g_collectPatch = patchFldOperand("collect_fld", AddrCollectFld, kCollectFldOrig, g_collectFldNew, &g_collectSq);
    if (g_collectPatch != 1) KP_WARN("collect operand NOT patched: octree collect radius stays at the fade end");
}

// FreeLibrary only: put the vanilla operands back before the globals go away.
void unpatchOperands() {
    uint8_t seen[6] = {};
    if (g_windPatch == 1 && codepatch::writeChecked(AddrWindFld, g_windFldNew, kWindFldOrig, 6, seen) == 1) g_windPatch = 0;
    if (g_collectPatch == 1 && codepatch::writeChecked(AddrCollectFld, g_collectFldNew, kCollectFldOrig, 6, seen) == 1) g_collectPatch = 0;
}

using GlCapFn = void(__stdcall*)(unsigned);
using GlGetIntFn = void(__stdcall*)(unsigned, int*);

// Call one of the game's GL import slots (glEnable / glDisable); skipped when the slot is unreadable or empty.
void glCap(uintptr_t slot, unsigned cap) {
    uint32_t fn = 0;
    if (!saferead::readAt<uint32_t>(5, slot, 0, &fn) || fn == 0) return;
    (*reinterpret_cast<GlCapFn*>(slot))(cap);
}

// First Begin only (GL context is current): alpha-to-coverage needs a multisample buffer. Logged once.
void checkA2cCapability() {
    g_a2cChecked = true;
    uint32_t fn = 0;
    if (!saferead::readAt<uint32_t>(6, AddrSlotGlGetIntegerv, 0, &fn) || fn == 0) {
        KP_WARN("a2c capability unknown: glGetIntegerv slot unreadable");
        return;
    }
    int sampleBuffers = 0;
    int samples = 0;
    GlGetIntFn get = *reinterpret_cast<GlGetIntFn*>(AddrSlotGlGetIntegerv);
    get(GlSampleBuffers, &sampleBuffers);
    get(GlSamples, &samples);
    g_a2cCapable = sampleBuffers >= 1;
    KP_LOG(chMain, "a2c capability sample_buffers=%d samples=%d", sampleBuffers, samples);
}

// wglGetProcAddress returns small sentinels (1, 2, 3, -1) on some drivers instead of null.
void* validGlProc(void* p) {
    const uintptr_t v = reinterpret_cast<uintptr_t>(p);
    return (v <= 3 || v == ~static_cast<uintptr_t>(0)) ? nullptr : p;
}

// Try `base`, `baseARB` and (when asked) `baseEXT` through wglGetProcAddress.
void* gpuProc(WglGetProcFn wgl, const char* base, bool tryExt) {
    static const char* const kSuffix[3] = {"", "ARB", "EXT"};
    for (int i = 0; i < (tryExt ? 3 : 2); ++i) {
        char name[64];
        snprintf(name, sizeof name, "%s%s", base, kSuffix[i]);
        if (void* p = validGlProc(wgl(name))) return p;
    }
    return nullptr;
}

// First Begin only (GL context is current): look for GL_ARB_timer_query / GL_EXT_timer_query and load its entry points.
// Without them gpu stays 0 in PERF. Logged once.
void gpuInit() {
    g_gpu.checked = true;
    const char* extName = "none";
    HMODULE gl = GetModuleHandleA("opengl32.dll");
    if (gl) {
        auto getString = reinterpret_cast<GlGetStringFn>(reinterpret_cast<void*>(GetProcAddress(gl, "glGetString")));
        auto wgl = reinterpret_cast<WglGetProcFn>(reinterpret_cast<void*>(GetProcAddress(gl, "wglGetProcAddress")));
        const char* exts = getString ? reinterpret_cast<const char*>(getString(GlExtensions)) : nullptr;
        if (exts && wgl) {
            const bool arb = strstr(exts, "GL_ARB_timer_query") != nullptr;
            const bool ext = strstr(exts, "GL_EXT_timer_query") != nullptr;
            if (arb || ext) {
                extName = arb ? "ARB" : "EXT";
                g_gpu.genQueries = reinterpret_cast<GlGenQueriesFn>(gpuProc(wgl, "glGenQueries", false));
                g_gpu.beginQuery = reinterpret_cast<GlBeginQueryFn>(gpuProc(wgl, "glBeginQuery", false));
                g_gpu.endQuery = reinterpret_cast<GlEndQueryFn>(gpuProc(wgl, "glEndQuery", false));
                g_gpu.getObjectiv = reinterpret_cast<GlGetQueryObjectivFn>(gpuProc(wgl, "glGetQueryObjectiv", false));
                g_gpu.getObjectui64v = reinterpret_cast<GlGetQueryObjectui64vFn>(gpuProc(wgl, "glGetQueryObjectui64v", ext));
                g_gpu.ok = g_gpu.genQueries && g_gpu.beginQuery && g_gpu.endQuery && g_gpu.getObjectiv && g_gpu.getObjectui64v;
            }
        }
    }
    KP_LOG(chMain, "gpu timer ext=%s ok=%d", extName, g_gpu.ok ? 1 : 0);
}

// Read every finished query, oldest first. Never blocks: an unavailable result ends the loop (it is read on a later call).
void gpuPoll() {
    while (g_gpu.ok && g_gpu.pending > 0) {
        const unsigned q = g_gpu.ids[g_gpu.head];
        int avail = 0;
        g_gpu.getObjectiv(q, GlQueryResultAvailable, &avail);
        if (!avail) break;
        uint64_t ns = 0;
        g_gpu.getObjectui64v(q, GlQueryResult, &ns);
        g_perf.addGpuNs(ns);
        g_gpu.head = (g_gpu.head + 1) % kGpuRing;
        --g_gpu.pending;
    }
}

void gpuBegin() {
    if (!g_cfg.perfLog || !g_cfg.gpuTiming) return;
    if (!g_gpu.checked) gpuInit();
    if (!g_gpu.ok) return;
    gpuPoll();
    if (g_gpu.active || g_gpu.pending >= kGpuRing) return;   // no free slot (or a stray nested Begin): skip this pass
    if (!g_gpu.idsReady) {
        g_gpu.genQueries(kGpuRing, g_gpu.ids);
        g_gpu.idsReady = true;
        for (int i = 0; i < kGpuRing; ++i) {
            if (g_gpu.ids[i] == 0) g_gpu.ok = false;
        }
        if (!g_gpu.ok) {
            KP_WARN("gpu timer: glGenQueries returned a null id; GPU timing off");
            return;
        }
    }
    g_gpu.beginQuery(GlTimeElapsed, g_gpu.ids[(g_gpu.head + g_gpu.pending) % kGpuRing]);
    g_gpu.active = true;
}

// Only ends a query this module began.
void gpuEnd() {
    if (!g_gpu.active) return;
    g_gpu.endQuery(GlTimeElapsed);
    g_gpu.active = false;
    ++g_gpu.pending;
}

// PF2a: decode the grass draw path from the engine globals (no game calls) and log it when it changes.
void probeGrassPath() {
    uint32_t F = 0;
    int32_t N = 0, f60EC = 0, f62DC = 0, c60FC = -1, c6118 = -1, f07c = 0;
    uint8_t bac4 = 0;
    const bool ok = saferead::readAt<uint32_t>(7, AddrPathFlags, 0, &F) &&
                    saferead::readAt<int32_t>(8, AddrPathN, 0, &N) &&
                    saferead::readAt<int32_t>(9, AddrPathFlag60EC, 0, &f60EC) &&
                    saferead::readAt<int32_t>(10, AddrPathFlag62DC, 0, &f62DC) &&
                    saferead::readAt<int32_t>(11, AddrPathC60FC, 0, &c60FC) &&
                    saferead::readAt<int32_t>(12, AddrPathC6118, 0, &c6118) &&
                    saferead::readAt<int32_t>(13, AddrPathF07C, 0, &f07c) &&
                    saferead::readAt<uint8_t>(14, AddrPathBac4, 0, &bac4);
    g_pathReadable = ok;
    if (!ok) return;
    const k2grass::GrassPath p = k2grass::decodeGrassPath(F, c60FC, c6118, N, f62DC, f60EC);
    if (g_pathSeen && F == g_pathLastF && p == g_pathLast) return;
    g_pathSeen = true;
    g_pathLastF = F;
    g_pathLast = p;
    KP_LOG(chMain, "path probe F=0x%08X G=%d H=%d P=%d Q=%d M=%d f07c=%d bac4=%d", F, p.G, p.H, p.P, p.Q, p.M, f07c,
           static_cast<int>(bac4));
}

void perfSample(unsigned now) {
    if (!g_cfg.perfLog) return;
    if (g_perf.startMs == 0) {
        g_perf.reset(now);
        return;
    }
    int32_t bins = 0;
    if (!saferead::readAt<int32_t>(4, AddrGrassPrevBinCount, 0, &bins) || bins < 0) bins = 0;
    g_perf.add(static_cast<uint32_t>(bins), now);
    if (!g_perf.due(now)) return;
    const bool trimmed = g_collectPatch == 1 && g_cfg.trimHidden;
    KP_LOG(chPerf,
           "PERF fps=%.1f frames=%u bins_avg=%.0f bins_max=%u end=%.0fm start=%.0fm wind=%.0fm alpha=%.2f "
           "grass_ms_avg=%.2f grass_ms_max=%.2f collect=%.0fm trim=%d a2c=%d "
           "gpu_ms_avg=%.2f gpu_ms_max=%.2f gpu=%d lod=%d quads_pct=%.0f pathA=%u pathB1=%u pathB2=%u",
           g_perf.fps(now), g_perf.frames, g_perf.binsAvg(), g_perf.binsMax,
           std::sqrt(g_eff.fadeEndSq), std::sqrt(g_eff.fadeStartSq), std::sqrt(g_eff.windDistSq), g_eff.alphaCutoff,
           g_perf.grassMsAvg(), g_perf.grassMsMax(),
           std::sqrt(g_collectPatch == 1 ? g_collectSq : g_eff.fadeEndSq), trimmed ? 1 : 0,
           (g_a2cCapable && g_cfg.smoothEdges) ? 1 : 0,
           g_perf.gpuMsAvg(), g_perf.gpuMsMax(), (g_gpu.ok && g_cfg.gpuTiming) ? 1 : 0,
           g_cfg.farLod ? 1 : 0, g_perf.quadsPct(), g_perf.pathA, g_perf.pathB1, g_perf.pathB2);
    g_perf.reset(now);
}

void ensureDefaultIniFile() {
    FILE* f = fopen(k2grass::kIniFile, "rb");
    if (f) {
        fclose(f);
        return;
    }
    char buf[4096];
    const int n = k2grass::formatDefaultIni(buf, sizeof(buf), g_cfg);
    if (n > 0 && static_cast<size_t>(n) < sizeof(buf)) {
        filewatch::atomicWrite(k2grass::kIniFile, buf, static_cast<size_t>(n));
    }
}

void loadAndApplyIni(bool initial) {
    ensureDefaultIniFile();
    FILE* f = fopen(k2grass::kIniFile, "rb");
    k2grass::Config nextCfg{};
    inikit::LoadStats st{};
    bool readOk = false;
    if (f) {
        char buf[8192];
        const size_t n = fread(buf, 1, sizeof(buf) - 1, f);
        buf[n] = '\0';
        fclose(f);
        st = k2grass::parseIniText(buf, nextCfg);
        readOk = true;
    }
    const bool changed = initial || (nextCfg != g_cfg);
    g_cfg = nextCfg;
    g_eff = k2grass::computeEffective(g_cfg);
    writeEngineGlobals(g_eff);
    g_collectSq = k2grass::collectDistSq(g_eff, g_cfg.trimHidden);
    ++g_reloadCount;

    if (!g_bannerLogged) {
        g_bannerLogged = true;
        KP_LOG(chMain,
               "%s %s loaded: enabled=%d fade=%.0fm..%.0fm (sq=%.0f..%.0f) wind=%.0fm (sq=%.0f) alpha=%.2f clip=%.1fm trim=%d collect=%.1fm (sq=%.0f) smooth_edges=%d perf_log=%d gpu_timing=%d wind_throttle=%d wind_near=%.0fm wind_every=%d far_lod=%d lod_start=%.0fm lod_min_keep=%.2f ini=%s(ok=%d,bad=%d)",
               k2grass::kModId, k2grass::kVersion,
               g_cfg.enabled ? 1 : 0,
               g_cfg.fadeStartDist, g_cfg.fadeEndDist,
               g_eff.fadeStartSq, g_eff.fadeEndSq,
               g_cfg.windDist, g_eff.windDistSq,
               g_eff.alphaCutoff,
               k2grass::effectiveClipDistMeters(g_eff),
               g_cfg.trimHidden ? 1 : 0, std::sqrt(g_collectSq), g_collectSq,
               g_cfg.smoothEdges ? 1 : 0,
               g_cfg.perfLog ? 1 : 0,
               g_cfg.gpuTiming ? 1 : 0, g_cfg.windThrottle ? 1 : 0, g_cfg.windNear, g_cfg.windEvery,
               g_cfg.farLod ? 1 : 0, g_cfg.lodStart, static_cast<double>(g_cfg.lodMinKeep),
               readOk ? "read" : "defaults", st.ok, st.bad);
    } else if (changed || g_cfg.verbose) {
        KP_LOG(chCfg,
               "INI reload #%u: enabled=%d fade=%.0fm..%.0fm wind=%.0fm alpha=%.2f clip=%.1fm trim=%d collect=%.1fm smooth_edges=%d "
               "gpu_timing=%d wind_throttle=%d wind_near=%.0fm wind_every=%d far_lod=%d lod_start=%.0fm lod_min_keep=%.2f",
               g_reloadCount,
               g_cfg.enabled ? 1 : 0,
               g_cfg.fadeStartDist, g_cfg.fadeEndDist,
               g_cfg.windDist, g_eff.alphaCutoff,
               k2grass::effectiveClipDistMeters(g_eff),
               g_cfg.trimHidden ? 1 : 0, std::sqrt(g_collectSq), g_cfg.smoothEdges ? 1 : 0,
               g_cfg.gpuTiming ? 1 : 0, g_cfg.windThrottle ? 1 : 0, g_cfg.windNear, g_cfg.windEvery,
               g_cfg.farLod ? 1 : 0, g_cfg.lodStart, static_cast<double>(g_cfg.lodMinKeep));
    }
}

// Silent ini write for a row the code moved (Page::setValue writes no ini), so the next page build reads it back.
void writeRowIni(const char* id, int32_t v) {
    const int i = g_page.index(id);
    if (i < 0 || !g_page.io.write) return;
    const mcdecl::RowDecl& r = g_page.decl.rows[i];
    if (!r.ini[0]) return;
    char b[16];
    snprintf(b, sizeof b, "%d", static_cast<int>(v));
    g_page.io.write(r.ini, r.section[0] ? r.section : nullptr, r.key, b);
}

// Keep Fade Start on or below menuFadeStartMax(Draw Distance) and show it: the start row moves, never the end row.
void linkFadeStart(const char* why, int32_t fadeEnd) {
    const int32_t s = g_page.get("fade_start");
    const int32_t ns = k2grass::menuLinkFadeStart(s, fadeEnd);
    if (ns == s) return;
    g_page.setValue("fade_start", ns);
    writeRowIni("fade_start", ns);
    KP_LOG(chCfg, "menu link (%s): fade start %dm -> %dm for draw distance %dm", why, static_cast<int>(s), static_cast<int>(ns),
           static_cast<int>(fadeEnd));
}

int32_t onRowChange(void*, int, const char* id, int32_t requested, int) {
    if (strcmp(id, "fade_start") == 0) return k2grass::menuLinkFadeStart(requested, g_page.get("fade_end"));
    if (strcmp(id, "fade_end") == 0) linkFadeStart("draw distance", requested);
    return requested;
}

void onPageBuilt(void*, uint32_t, void*) {
    linkFadeStart("page build", g_page.get("fade_end"));   // a hand-edited ini can hold start >= end
}

void pollModConfig() {
    auto clients = k2pc::makeSet(g_mcCfg, g_mcMenu);
    if (clients.anyMissing()) {
        clients.poll();
    }
    if (!g_pageReady) {
        char err[96] = {};
        if (g_page.load(k2grass::kPageSchema, strlen(k2grass::kPageSchema), err, sizeof(err))) {
            g_page.menu = &g_mcMenu;
            g_page.io = mcdecl::fileIo();
            g_page.hooks.onChange = onRowChange;
            g_page.hooks.onBuilt = onPageBuilt;
            g_mcEntry = g_page.entry(k2grass::kModId);
            g_pageReady = true;
        } else {
            KP_WARN("ModConfig schema parse failed: %s", err);
            g_pageReady = true;
        }
    }
    if (g_pageReady && g_mcEntry.id && g_mcMenu.ok()) {
        k2pc::registerOnce(g_mcReg, g_mcCfg, g_mcEntry, mcLog);
    }
}

// The three values the game pushed for glDrawArrays(mode, first, count); the hook gets a pointer to them (esp+0).
struct DrawArraysArgs {
    uint32_t mode;
    int32_t first;
    int32_t count;
};

// Far LOD: draw only the first K quads of the bin. Prefix thinning: if quads are spatially ordered inside a bin this
// leaves bald patches (a play-test question); there is no alpha compensation in this version. Only the pushed count
// changes: the helper's offset argument (count * 0x30) is also the normals/UV offset, so it must stay as the game set it.
void thinDrawArrays(DrawArraysArgs* args) {
    if (!g_inGrass || args == nullptr || args->mode != GlQuads || args->count < 0) return;
    const int32_t before = args->count;
    int32_t after = before;
    if (g_cfg.farLod) {
        after = k2grass::thinCount(before, g_binKeep);
        args->count = after;
    }
    if (g_cfg.perfLog) g_perf.addQuads(static_cast<uint32_t>(before / 4), static_cast<uint32_t>(after / 4));
}

} // namespace

extern "C" {

void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    auto clients = k2pc::makeSet(g_mcCfg, g_mcMenu);
    clients.init(api);
    const unsigned now = GetTickCount();
    filewatch::changed(g_iniPoll, k2grass::kIniFile, now, true);
    loadAndApplyIni(true);
    patchWindOperand();
    patchCollectOperand();
    g_initialized = true;
    pollModConfig();
}

// Hook 1: Scene::CollectAndTraverse (at 0x00499ABC, once per frame when grass is enabled in the scene).
void __cdecl K2GrassOnSceneCollect() {
    const unsigned now = GetTickCount();
    if (!g_initialized) {
        filewatch::changed(g_iniPoll, k2grass::kIniFile, now, true);
        loadAndApplyIni(true);
        patchWindOperand();
        patchCollectOperand();
        g_initialized = true;
    } else if (filewatch::changed(g_iniPoll, k2grass::kIniFile, now)) {
        loadAndApplyIni(false);
    } else {
        // Re-assert globals each grass frame so area loads cannot clobber them.
        writeEngineGlobals(g_eff);
    }
    if (!g_mcReg.registered) {
        pollModConfig();
    }
    // PF2b: the wind gate reads g_windDistSq; one value per frame (the throttle phase advances once per grass frame).
    // Wind has no dt, so a throttled bin sways at 1/k speed; whether that reads as slow-motion grass is the A/B question.
    g_windDistSq = k2grass::windDistSqForFrame(g_eff, g_cfg, g_windFrame++);
    gpuPoll();
    perfSample(now);
    ++g_collectCalls;
}

// Hook 2: Scene::DoBinBucket (0x004664E0) at 0x0046661B, the bucket loop's counter init (outer loop; the bins are the
// inner loop at 0x0046664B). Times the whole grass pass.
void __cdecl K2GrassBinLoopBegin() {
    if (!g_a2cChecked) checkA2cCapability();
    probeGrassPath();
    g_inGrass = true;
    g_binKeep = 1.0f;
    gpuBegin();   // before the CPU timer so the query call is not counted as grass CPU time
    if (g_cfg.perfLog) {
        if (!g_qpcFreq.QuadPart) QueryPerformanceFrequency(&g_qpcFreq);
        g_loopTiming = g_qpcFreq.QuadPart > 0 && QueryPerformanceCounter(&g_loopT0) != 0;
    } else {
        g_loopTiming = false;
    }
    if (g_a2cCapable && g_cfg.smoothEdges) {
        glCap(AddrSlotGlEnable, GlSampleAlphaToCoverage);
        g_a2cOn = true;
    }
}

// Hook 3: Scene::DoBinBucket at 0x00466698, the bucket loop's only exit (JGE at 0x00466636).
void __cdecl K2GrassBinLoopEnd() {
    g_inGrass = false;
    if (g_a2cOn) {
        glCap(AddrSlotGlDisable, GlSampleAlphaToCoverage);
        g_a2cOn = false;
    }
    if (g_loopTiming) {
        g_loopTiming = false;
        LARGE_INTEGER t1{};
        if (QueryPerformanceCounter(&t1) && t1.QuadPart >= g_loopT0.QuadPart) {
            const uint64_t us = static_cast<uint64_t>(t1.QuadPart - g_loopT0.QuadPart) * 1000000ull /
                                static_cast<uint64_t>(g_qpcFreq.QuadPart);
            g_perf.addGrassUs(us > 0xFFFFFFFFull ? 0xFFFFFFFFu : static_cast<uint32_t>(us));
        }
    }
    gpuEnd();   // after the CPU timer stop, same reason as in Begin
}

// Hook 4 (PF2c / PF2a): CAurTriangleBin::RenderGrassPolys entry at 0x0050DAC0, bin in ECX (the game's live object).
// Picks the keep fraction for this bin's draw calls and counts the bin by draw path for the PERF line.
void __cdecl K2GrassOnBin(const uint8_t* bin) {
    g_binKeep = 1.0f;
    if (bin == nullptr || reinterpret_cast<uintptr_t>(bin) < 0x10000) return;
    if (g_cfg.farLod) {
        float distSq = 0.0f;
        memcpy(&distSq, bin + kBinOffDistSq, sizeof distSq);
        g_binKeep = k2grass::lodKeep(distSq, g_cfg, g_eff);
    }
    if (g_cfg.perfLog) {
        // [AddrPathF07C] is static engine data already probed readable at this pass's Begin.
        const int32_t f07c = g_pathReadable ? *reinterpret_cast<const volatile int32_t*>(AddrPathF07C) : 0;
        g_perf.addPath(k2grass::classifyBinPath(bin[kBinOffFlags], f07c, g_pathLast.P));
    }
}

// Hook 5 / 6: the two CALL dword ptr [glDrawArrays] in the grass draw helper FUN_004369A0 (grass-only; callers
// 0x0050E16B, 0x0050E487, 0x0050E695), at 0x00436A95 and 0x00436B86.
void __cdecl K2GrassDrawArraysA(DrawArraysArgs* args) { thinDrawArrays(args); }
void __cdecl K2GrassDrawArraysB(DrawArraysArgs* args) { thinDrawArrays(args); }

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
    } else if (fdwReason == DLL_PROCESS_DETACH && lpvReserved == nullptr) {
        unpatchOperands();
    }
    return TRUE;
}
