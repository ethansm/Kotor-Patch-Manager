// K2GrassDrawDistance.cpp -- K2 Grass Draw Distance (0.1.10-dev)
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
// 0.1.6 variety (doc 50 PV1-PV4), all off by default (0.1.9: PV1/PV2 on); with the toggles off and size_var 0.4 the picture equals 0.1.5:
//   PV1 vary_seed      hook in the blade generator FUN_0050EBC0 rewrites the per-face srand seed slot [EBP-0x9C]
//   PV2 even_spread    hook before the blade weight sum rewrites the three barycentric weight slots [EBP-0xC..-0x4]
//   PV3 density_noise  hook before the face count FRNDINT rewrites the float slot [EBP-0x224]
//   PV4 size_var       the size FMUL's double operand is redirected to the DLL's g_sizeSpan (code_patches.toml)
// 0.1.10: far_lod draws an evenly spaced quad subset (own glDrawElements); wind_dist clamp 90 (= menu max); fast_bin_diff
//         hash-based FUN_004987E0 bin list diff (hook 10); fix_b1_pointer (path B1 vertex pointer, code patch).
// 0.1.9: Varied Grass Pattern + Even Grass Spread default on (GR7: looks better, no measured cost).
// 0.1.8: PERF2 seeds=; variety counters survive the PERF gap reset (load-time hook calls were wiped).
// 0.1.7: long log lines split under the core's 232-byte cut (PERF/PERF2, loaded or INI reload + CFG + VARIETY).
// The variety hooks only rewrite the game's own stack slots; the DLL never calls rand/srand. The only game function it
// calls is the bin destroy FUN_0050DA00 (fast_bin_diff, hook 10), with exactly the arguments the engine's own loop uses.
// Generation tweaks apply to bins built after the change (an area reload, or grass newly collected).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cmath>

#define INIW_FILE_API 1
#define MCDECL_FILE_API 1

#include "../Common/CodePatch.h"
#include "../Common/FileWatch.h"
#include "K2GrassDrawDistanceLogic.h"
#include "../Common/KpLog.h"
#include "../Common/ModConfigApi.h"
#include "../Common/ModConfigDecl.h"
#include "../Common/ProviderClient.h"
#include "../Common/SafeRead.h"

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
// Size FMUL `FMUL double ptr [0x009921C8]` (vanilla 0.4); only the imm32 changes, to &g_sizeSpan.
constexpr uintptr_t AddrSizeFmul = 0x0050FEE7;
const uint8_t kSizeFmulOrig[6] = {0xDC, 0x0D, 0xC8, 0x21, 0x99, 0x00};
// FUN_0050EBC0 face vertices (floats x,y,z each) the PV1 seed hash reads: V0 = 0x00A7D6EC/F0/F4, V1 = 0x00A7D6E0/E4/E8,
// V2 = 0x00A7D6D4/D8/DC.
constexpr uintptr_t AddrFaceV0 = 0x00A7D6EC;
constexpr uintptr_t AddrFaceV1 = 0x00A7D6E0;
constexpr uintptr_t AddrFaceV2 = 0x00A7D6D4;
// The PV3 centroid inputs (floats x,y,z each): V0 = 0x00A7D6BC/C0/C4, V1 = 0x00A7D6B0/B4/B8, V2 = 0x00A7D6A4/A8/AC.
constexpr uintptr_t AddrCentV0 = 0x00A7D6BC;
constexpr uintptr_t AddrCentV1 = 0x00A7D6B0;
constexpr uintptr_t AddrCentV2 = 0x00A7D6A4;
// Stack slots, relative to the game's EBP: seed (int), blade weights a/b/c (float), face count (float).
constexpr intptr_t kSlotSeed = -0x9C;
constexpr intptr_t kSlotBladeA = -0xC;
constexpr intptr_t kSlotBladeB = -0x8;
constexpr intptr_t kSlotBladeC = -0x4;
constexpr intptr_t kSlotFaceCount = -0x224;
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
// 0.1.10 fast bin diff: FUN_004987E0 (called once per frame from Scene::CollectAndTraverse) builds the current bins, then
// destroys every previous bin missing from the current list with a linear scan per bin. Lists are {ptr, count} pairs of
// 4-byte bin pointers: previous {0x00A73300, 0x00A73304}, current {0x00A7330C, 0x00A73310}. Destroy = FUN_0050DA00 (__thiscall).
constexpr uintptr_t AddrGrassPrevBinPtr = 0x00A73300;
constexpr uintptr_t AddrGrassCurBinPtr = 0x00A7330C;
constexpr uintptr_t AddrGrassCurBinCount = 0x00A73310;
constexpr uintptr_t AddrBinDestroy = 0x0050DA00;
constexpr intptr_t kSlotDiffCounter = -0x4;   // loop-2 counter of FUN_004987E0, [EBP-4]
// Path B1 pushes `ADD ECX,0x40` (address of bin field +0x40) as the vertex pointer base; paths A and B2 push [bin+0x38].
// The 3-byte `ADD ECX,0x40` at 0x0050E483 (MOV ECX,[EBP-0x114] is at 0x0050E47D) becomes `MOV ECX,[ECX+0x38]`.
constexpr uintptr_t AddrB1Ptr = 0x0050E483;
const uint8_t kB1PtrOrig[3] = {0x83, 0xC1, 0x40};
const uint8_t kB1PtrNew[3] = {0x8B, 0x49, 0x38};
// Draw helper FUN_004369A0 sets the vertex/normal/UV pointers and calls glDrawArrays(7, first, count).
constexpr unsigned GlElementArrayBufferBinding = 0x8895;
constexpr unsigned GlUnsignedInt = 0x1405;
constexpr int32_t kEvenIdxCap = 1 << 18;   // indices; a bin keeping more than 65536 quads falls back to the prefix
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
// PV4 blade size span the patched FMUL reads (vanilla 0.4).
double g_sizeSpan = 0.4;
uint8_t g_sizeFmulNew[6] = {};
int g_sizePatch = 0;
uint8_t g_b1Patched = 0;   // 1 = the B1 vertex pointer fix is written
// Far LOD even selection (render thread only): index list for glDrawElements, and whether this pass may use it.
uint32_t g_evenIdx[kEvenIdxCap];
bool g_elemClear = false;   // no GL_ELEMENT_ARRAY_BUFFER bound at this pass's Begin (client-memory indices are valid)
using GlDrawElementsFn = void(__stdcall*)(unsigned, int, unsigned, const void*);
GlDrawElementsFn g_glDrawElements = nullptr;
bool g_glDrawElementsTried = false;
// Fast bin diff state (render thread only).
k2grass::PtrSet g_diffSet;
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

// Rewrite the imm32 of a 6-byte `op op imm32` absolute-operand instruction (FLD float / FMUL double) to point at `target`. On a byte mismatch nothing is
// written (logged); the engine keeps reading its own global.
int patchFldOperand(const char* name, uintptr_t at, const uint8_t* orig, uint8_t* repl, const void* target) {
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

// PV4: point the size FMUL at g_sizeSpan. On a mismatch the blade size span stays the game's vanilla 0.4.
void patchSizeOperand() {
    if (g_sizePatch == 1) return;
    g_sizePatch = patchFldOperand("size_span_fmul", AddrSizeFmul, kSizeFmulOrig, g_sizeFmulNew, &g_sizeSpan);
    if (g_sizePatch != 1) KP_WARN("size operand NOT patched: blade size span stays vanilla");
}

// 0.1.10 (fix_b1_pointer, read at load): path B1 vertex pointer = [bin+0x38] like paths A and B2. Mismatch: nothing written.
void patchB1Pointer() {
    if (g_b1Patched || !g_cfg.fixB1Pointer) return;
    uint8_t seen[3] = {};
    const int rc = codepatch::writeChecked(AddrB1Ptr, kB1PtrOrig, kB1PtrNew, 3, seen);
    KP_LOG(chMain, "patch b1_vertex_ptr @0x%08X rc=%d seen=%02X %02X %02X", static_cast<unsigned>(AddrB1Ptr), rc, seen[0],
           seen[1], seen[2]);
    if (rc == 1) g_b1Patched = 1;
    else KP_WARN("b1 vertex pointer NOT patched: path B1 keeps the vanilla pointer");
}

// FreeLibrary only: put the vanilla operands back before the globals go away.
void unpatchOperands() {
    uint8_t seen[6] = {};
    if (g_b1Patched && codepatch::writeChecked(AddrB1Ptr, kB1PtrNew, kB1PtrOrig, 3, seen) == 1) g_b1Patched = 0;
    if (g_windPatch == 1 && codepatch::writeChecked(AddrWindFld, g_windFldNew, kWindFldOrig, 6, seen) == 1) g_windPatch = 0;
    if (g_collectPatch == 1 && codepatch::writeChecked(AddrCollectFld, g_collectFldNew, kCollectFldOrig, 6, seen) == 1) g_collectPatch = 0;
    if (g_sizePatch == 1 && codepatch::writeChecked(AddrSizeFmul, g_sizeFmulNew, kSizeFmulOrig, 6, seen) == 1) g_sizePatch = 0;
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
    // The core cuts a line's text at 232 bytes, so the window is logged as two lines (PERF, PERF2).
    KP_LOG(chPerf,
           "PERF fps=%.1f frames=%u bins_avg=%.0f bins_max=%u end=%.0fm start=%.0fm wind=%.0fm alpha=%.2f "
           "grass_ms_avg=%.2f grass_ms_max=%.2f collect=%.0fm trim=%d a2c=%d",
           g_perf.fps(now), g_perf.frames, g_perf.binsAvg(), g_perf.binsMax,
           std::sqrt(g_eff.fadeEndSq), std::sqrt(g_eff.fadeStartSq), std::sqrt(g_eff.windDistSq), g_eff.alphaCutoff,
           g_perf.grassMsAvg(), g_perf.grassMsMax(),
           std::sqrt(g_collectPatch == 1 ? g_collectSq : g_eff.fadeEndSq), trimmed ? 1 : 0,
           (g_a2cCapable && g_cfg.smoothEdges) ? 1 : 0);
    KP_LOG(chPerf,
           "PERF2 gpu_ms_avg=%.2f gpu_ms_max=%.2f gpu=%d lod=%d quads_pct=%.0f pathA=%u pathB1=%u pathB2=%u "
           "seeds=%u blades=%u dens_faces=%u dens_in=%.0f dens_out=%.0f diff_calls=%u diff_destroyed=%u diff_us_avg=%.1f",
           g_perf.gpuMsAvg(), g_perf.gpuMsMax(), (g_gpu.ok && g_cfg.gpuTiming) ? 1 : 0,
           g_cfg.farLod ? 1 : 0, g_perf.quadsPct(), g_perf.pathA, g_perf.pathB1, g_perf.pathB2,
           g_perf.seeds, g_perf.blades, g_perf.densFaces, g_perf.densIn, g_perf.densOut,
           g_perf.diffCalls, g_perf.diffDestroyed, static_cast<double>(g_perf.diffUsAvg()));
    g_perf.reset(now);
}

void ensureDefaultIniFile() {
    FILE* f = fopen(k2grass::kIniFile, "rb");
    if (f) {
        fclose(f);
        return;
    }
    char buf[6144];
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
    g_sizeSpan = k2grass::sizeSpanValue(g_cfg.sizeVar);
    ++g_reloadCount;

    // The core cuts a line's text at 232 bytes: the settings go out as three short lines (main, timing/lod, variety).
    if (!g_bannerLogged) {
        g_bannerLogged = true;
        KP_LOG(chMain,
               "%s %s loaded: enabled=%d fade=%.0fm..%.0fm (sq=%.0f..%.0f) wind=%.0fm (sq=%.0f) alpha=%.2f clip=%.1fm trim=%d "
               "collect=%.1fm (sq=%.0f) ini=%s(ok=%d,bad=%d)",
               k2grass::kModId, k2grass::kVersion,
               g_cfg.enabled ? 1 : 0,
               g_cfg.fadeStartDist, g_cfg.fadeEndDist,
               g_eff.fadeStartSq, g_eff.fadeEndSq,
               g_cfg.windDist, g_eff.windDistSq,
               g_eff.alphaCutoff,
               k2grass::effectiveClipDistMeters(g_eff),
               g_cfg.trimHidden ? 1 : 0, std::sqrt(g_collectSq), g_collectSq,
               readOk ? "read" : "defaults", st.ok, st.bad);
    } else if (changed || g_cfg.verbose) {
        KP_LOG(chCfg,
               "INI reload #%u: enabled=%d fade=%.0fm..%.0fm wind=%.0fm alpha=%.2f clip=%.1fm trim=%d collect=%.1fm",
               g_reloadCount,
               g_cfg.enabled ? 1 : 0,
               g_cfg.fadeStartDist, g_cfg.fadeEndDist,
               g_cfg.windDist, g_eff.alphaCutoff,
               k2grass::effectiveClipDistMeters(g_eff),
               g_cfg.trimHidden ? 1 : 0, std::sqrt(g_collectSq));
    } else {
        return;
    }
    KP_LOG(chCfg,
           "CFG smooth_edges=%d perf_log=%d gpu_timing=%d wind_throttle=%d wind_near=%.0fm wind_every=%d far_lod=%d "
           "lod_start=%.0fm lod_min_keep=%.2f fast_bin_diff=%d b1_fix=%d",
           g_cfg.smoothEdges ? 1 : 0, g_cfg.perfLog ? 1 : 0,
           g_cfg.gpuTiming ? 1 : 0, g_cfg.windThrottle ? 1 : 0, g_cfg.windNear, g_cfg.windEvery,
           g_cfg.farLod ? 1 : 0, g_cfg.lodStart, static_cast<double>(g_cfg.lodMinKeep),
           g_cfg.fastBinDiff ? 1 : 0, g_cfg.fixB1Pointer ? 1 : 0);
    KP_LOG(chCfg,
           "VARIETY vary_seed=%d even_spread=%d density_noise=%d density_amount=%.2f density_cell=%.0fm size_var=%.2f "
           "(applies to grass built after this; reload the save)",
           g_cfg.varySeed ? 1 : 0, g_cfg.evenSpread ? 1 : 0, g_cfg.densityNoise ? 1 : 0,
           static_cast<double>(g_cfg.densityAmount), static_cast<double>(g_cfg.densityCell), g_sizeSpan);
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

// Far LOD: draw K of the bin's N quads. The bin's vertex buffer is written face by face, so a prefix drops whole faces
// (bald holes); instead K quads spaced evenly through the buffer (q_i = floor(i * N / K)) are drawn with our own
// glDrawElements on the client arrays the helper has bound, and the pushed count becomes 0 so the game's glDrawArrays
// draws nothing. When an element buffer is bound, glDrawElements is missing, or the index list does not fit, the old
// prefix (lower the pushed count to K quads) is used. Either way the helper's offset argument (count * 0x30, also the
// normals/UV offset) stays as the game set it, and there is no alpha compensation.
bool drawEvenQuads(int32_t first, int32_t nQuads, int32_t keepQuads) {
    if (!g_elemClear) return false;
    if (!g_glDrawElementsTried) {
        g_glDrawElementsTried = true;
        HMODULE gl = GetModuleHandleA("opengl32.dll");
        if (gl) g_glDrawElements = reinterpret_cast<GlDrawElementsFn>(reinterpret_cast<void*>(GetProcAddress(gl, "glDrawElements")));
        KP_LOG(chMain, "far_lod even thinning: glDrawElements=%s", g_glDrawElements ? "found" : "missing (prefix fallback)");
    }
    if (!g_glDrawElements) return false;
    const int32_t n = k2grass::fillEvenQuadIndices(g_evenIdx, kEvenIdxCap, first, nQuads, keepQuads);
    if (n <= 0) return false;
    g_glDrawElements(GlQuads, n, GlUnsignedInt, g_evenIdx);
    return true;
}

void thinDrawArrays(DrawArraysArgs* args) {
    if (!g_inGrass || args == nullptr || args->mode != GlQuads || args->count < 0) return;
    const int32_t before = args->count;
    int32_t after = before;
    if (g_cfg.farLod) {
        after = k2grass::thinCount(before, g_binKeep);
        if (after < before && drawEvenQuads(args->first, before / 4, after / 4)) args->count = 0;
        else args->count = after;
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
    patchSizeOperand();
    patchB1Pointer();
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
        patchSizeOperand();
        patchB1Pointer();
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
    g_elemClear = false;
    if (g_cfg.farLod) {
        // Client-memory indices are only valid when no element buffer is bound; one query per pass.
        uint32_t fn = 0;
        if (saferead::readAt<uint32_t>(15, AddrSlotGlGetIntegerv, 0, &fn) && fn != 0) {
            int bound = -1;
            (*reinterpret_cast<GlGetIntFn*>(AddrSlotGlGetIntegerv))(GlElementArrayBufferBinding, &bound);
            g_elemClear = bound == 0;
        }
    }
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

// Read three consecutive floats of engine static data as raw u32 bits (fixed .data addresses; no game call).
static void readBits3(uintptr_t at, uint32_t* out) {
    for (int i = 0; i < 3; ++i) out[i] = *reinterpret_cast<const volatile uint32_t*>(at + 4u * static_cast<uintptr_t>(i));
}

// Hook 7 (PV1): blade generator FUN_0050EBC0 at 0x0050EF89 (MOV ECX,[EBP-0x9C]; PUSH ECX; CALL srand). `ebp` is the game's
// frame pointer; the seed slot is overwritten with a hash of the face's nine vertex floats before the game loads it.
void __cdecl K2GrassOnFaceSeed(uint8_t* ebp) {
    if (!g_cfg.varySeed || ebp == nullptr) return;
    uint32_t bits[9];
    readBits3(AddrFaceV0, bits);
    readBits3(AddrFaceV1, bits + 3);
    readBits3(AddrFaceV2, bits + 6);
    *reinterpret_cast<int32_t*>(ebp + kSlotSeed) = k2grass::faceSeed(bits);
    if (g_cfg.perfLog) g_perf.addSeed();
}

// Hook 8 (PV2): per blade at 0x0050F177 (FLD [EBP-0xC]; FADD [EBP-0x8]), before the weight sum and normalise call. The two
// raw randoms (each rand % 10000 / 10000) become uniform barycentric weights, written to all three weight slots.
void __cdecl K2GrassOnBlade(uint8_t* ebp) {
    if (!g_cfg.evenSpread || ebp == nullptr) return;
    float* wa = reinterpret_cast<float*>(ebp + kSlotBladeA);
    float* wb = reinterpret_cast<float*>(ebp + kSlotBladeB);
    float* wc = reinterpret_cast<float*>(ebp + kSlotBladeC);
    float w0 = 0.0f, w1 = 0.0f, w2 = 0.0f;
    k2grass::uniformBary(*wa, *wb, &w0, &w1, &w2);
    *wa = w0;
    *wb = w1;
    *wc = w2;
    if (g_cfg.perfLog) g_perf.addBlade();
}

// Hook 9 (PV3): per face at 0x0050D35C (FLD [EBP-0x224], then FRNDINT / FISTP into the face blade count). The float slot is
// the single source of the count; it is scaled by smooth patch noise at the face centroid.
void __cdecl K2GrassOnFaceCount(uint8_t* ebp) {
    if (!g_cfg.densityNoise || ebp == nullptr) return;
    uint32_t b0[3], b1[3], b2[3];
    readBits3(AddrCentV0, b0);
    readBits3(AddrCentV1, b1);
    readBits3(AddrCentV2, b2);
    float v0[3], v1[3], v2[3];
    memcpy(v0, b0, sizeof v0);
    memcpy(v1, b1, sizeof v1);
    memcpy(v2, b2, sizeof v2);
    const float cx = (v0[0] + v1[0] + v2[0]) * (1.0f / 3.0f);
    const float cy = (v0[1] + v1[1] + v2[1]) * (1.0f / 3.0f);
    float* slot = reinterpret_cast<float*>(ebp + kSlotFaceCount);
    const float x = *slot;
    const float y = k2grass::densityCount(x, cx, cy, g_cfg);
    *slot = y;
    if (g_cfg.perfLog) g_perf.addDensity(x, y);
}

// Hook 10 (0.1.10, fast_bin_diff): FUN_004987E0 at 0x00498829, the loop-2 counter init `MOV dword ptr [EBP-4], 0` (7 bytes;
// the original bytes are skipped, so this handler writes the counter itself on every path). Loop 2 destroys each previous
// bin that is not in the current list, scanning the current list per bin (O(N^2)). Fast path: hash the current bins, destroy
// the missing previous bins in list order with the game's own destroy, then set the counter to the previous count so loop 2
// ends at once. Anything unusual (off, null/short lists, > kMaxDiffBins, null entries, unreadable arrays, full table) leaves
// the counter at 0 before any destroy ran: exactly the vanilla loop.
void __cdecl K2GrassBinDiff(uint8_t* ebp) {
    if (ebp == nullptr) return;
    int32_t* counter = reinterpret_cast<int32_t*>(ebp + kSlotDiffCounter);
    *counter = 0;
    if (!g_cfg.fastBinDiff) return;
    saferead::beginScope();
    uint32_t prevPtr = 0, curPtr = 0;
    int32_t prevCount = 0, curCount = 0;
    if (!saferead::readAt<uint32_t>(16, AddrGrassPrevBinPtr, 0, &prevPtr) ||
        !saferead::readAt<int32_t>(17, AddrGrassPrevBinCount, 0, &prevCount) ||
        !saferead::readAt<uint32_t>(18, AddrGrassCurBinPtr, 0, &curPtr) ||
        !saferead::readAt<int32_t>(19, AddrGrassCurBinCount, 0, &curCount)) return;
    if (prevCount <= 0 || prevCount > k2grass::kMaxDiffBins || curCount < 0 || curCount > k2grass::kMaxDiffBins) return;
    if (prevPtr == 0 || (curCount > 0 && curPtr == 0)) return;
    if (!saferead::probeRead(20, reinterpret_cast<const void*>(static_cast<uintptr_t>(prevPtr)), 4u * static_cast<size_t>(prevCount))) return;
    if (curCount > 0 && !saferead::probeRead(21, reinterpret_cast<const void*>(static_cast<uintptr_t>(curPtr)), 4u * static_cast<size_t>(curCount))) return;
    LARGE_INTEGER t0{};
    bool timing = false;
    if (g_cfg.perfLog) {
        if (!g_qpcFreq.QuadPart) QueryPerformanceFrequency(&g_qpcFreq);
        timing = g_qpcFreq.QuadPart > 0 && QueryPerformanceCounter(&t0) != 0;
    }
    const volatile uint32_t* cur = reinterpret_cast<const volatile uint32_t*>(static_cast<uintptr_t>(curPtr));
    const volatile uint32_t* prev = reinterpret_cast<const volatile uint32_t*>(static_cast<uintptr_t>(prevPtr));
    g_diffSet.clear();
    for (int32_t i = 0; i < curCount; ++i) {
        if (!g_diffSet.insert(cur[i])) return;   // null entry or full table: vanilla loop
    }
    for (int32_t i = 0; i < prevCount; ++i) {
        if (prev[i] == 0) return;
    }
    using BinDestroyFn = void(__thiscall*)(void*);
    const BinDestroyFn destroy = reinterpret_cast<BinDestroyFn>(AddrBinDestroy);
    uint32_t destroyed = 0;
    for (int32_t i = 0; i < prevCount; ++i) {
        const uint32_t bin = prev[i];
        if (g_diffSet.contains(bin)) continue;
        destroy(reinterpret_cast<void*>(static_cast<uintptr_t>(bin)));
        ++destroyed;
    }
    *counter = prevCount;
    if (timing) {
        LARGE_INTEGER t1{};
        if (QueryPerformanceCounter(&t1) && t1.QuadPart >= t0.QuadPart) {
            const uint64_t us = static_cast<uint64_t>(t1.QuadPart - t0.QuadPart) * 1000000ull / static_cast<uint64_t>(g_qpcFreq.QuadPart);
            g_perf.addDiff(destroyed, us > 0xFFFFFFFFull ? 0xFFFFFFFFu : static_cast<uint32_t>(us));
            return;
        }
    }
    g_perf.addDiff(destroyed, 0);
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
