// K2GrassDrawDistance.cpp -- K2 Grass Draw Distance (0.1.1-dev)
// Extends grass octree build/collect radius, fade-start/fade-end distances, alpha-test threshold,
// and decouples CPU wind animation radius (one operand rewrite at KPatchInit, code_patches.toml).
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

// Engine .data globals (Steam Aspyr build 6A522E71...):
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
// int: count of the previous frame's in-range grass bin list (list = {ptr, count}, count at +4). The per-frame
// list swap in Scene::CollectAndTraverse copies the current list there before hook 1 runs (doc 48 section 1.5).
constexpr uintptr_t AddrGrassPrevBinCount = 0x00A73304;

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
uint8_t g_windFldNew[6] = {};
int g_windPatch = 0;   // codepatch::writeChecked result; 1 = patched
k2grass::PerfWindow g_perf{};

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

// Point the wind gate's FLD at g_windDistSq so wind_dist and fade_start_dist are independent. On a byte
// mismatch nothing is written and wind stays coupled to fade start (vanilla behaviour, logged).
void patchWindOperand() {
    if (g_windPatch == 1) return;
    memcpy(g_windFldNew, kWindFldOrig, 2);
    const uint32_t a = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&g_windDistSq));
    memcpy(g_windFldNew + 2, &a, 4);
    uint8_t seen[6] = {};
    g_windPatch = codepatch::writeChecked(AddrWindFld, kWindFldOrig, g_windFldNew, 6, seen);
    if (g_windPatch == 1) {
        KP_LOG(chMain, "wind operand patched at 0x%08X -> &g_windDistSq=0x%08X", static_cast<unsigned>(AddrWindFld), a);
    } else {
        KP_WARN("wind operand NOT patched (rc=%d seen=%02X %02X %02X %02X %02X %02X): wind radius follows fade start",
                g_windPatch, seen[0], seen[1], seen[2], seen[3], seen[4], seen[5]);
    }
}

// FreeLibrary only: put the vanilla operand back before g_windDistSq goes away.
void unpatchWindOperand() {
    if (g_windPatch != 1) return;
    uint8_t seen[6] = {};
    if (codepatch::writeChecked(AddrWindFld, g_windFldNew, kWindFldOrig, 6, seen) == 1) g_windPatch = 0;
}

void perfSample(unsigned now) {
    if (!g_cfg.perfLog) return;
    if (g_perf.startMs == 0) {
        g_perf.reset(now);
        return;
    }
    int32_t bins = 0;
    if (!saferead::readAt<int32_t>(4, AddrGrassPrevBinCount, 0, &bins) || bins < 0) bins = 0;
    g_perf.add(static_cast<uint32_t>(bins));
    if (!g_perf.due(now)) return;
    KP_LOG(chPerf,
           "PERF fps=%.1f frames=%u bins_avg=%.0f bins_max=%u end=%.0fm start=%.0fm wind=%.0fm alpha=%.2f",
           g_perf.fps(now), g_perf.frames, g_perf.binsAvg(), g_perf.binsMax,
           std::sqrt(g_eff.fadeEndSq), std::sqrt(g_eff.fadeStartSq), std::sqrt(g_eff.windDistSq), g_eff.alphaCutoff);
    g_perf.reset(now);
}

void ensureDefaultIniFile() {
    FILE* f = fopen(k2grass::kIniFile, "rb");
    if (f) {
        fclose(f);
        return;
    }
    char buf[1024];
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
    ++g_reloadCount;

    if (!g_bannerLogged) {
        g_bannerLogged = true;
        KP_LOG(chMain,
               "%s %s loaded: enabled=%d fade=%.0fm..%.0fm (sq=%.0f..%.0f) wind=%.0fm (sq=%.0f) alpha=%.2f clip=%.1fm perf_log=%d ini=%s(ok=%d,bad=%d)",
               k2grass::kModId, k2grass::kVersion,
               g_cfg.enabled ? 1 : 0,
               g_cfg.fadeStartDist, g_cfg.fadeEndDist,
               g_eff.fadeStartSq, g_eff.fadeEndSq,
               g_cfg.windDist, g_eff.windDistSq,
               g_eff.alphaCutoff,
               k2grass::effectiveClipDistMeters(g_eff),
               g_cfg.perfLog ? 1 : 0,
               readOk ? "read" : "defaults", st.ok, st.bad);
    } else if (changed || g_cfg.verbose) {
        KP_LOG(chCfg,
               "INI reload #%u: enabled=%d fade=%.0fm..%.0fm wind=%.0fm alpha=%.2f clip=%.1fm",
               g_reloadCount,
               g_cfg.enabled ? 1 : 0,
               g_cfg.fadeStartDist, g_cfg.fadeEndDist,
               g_cfg.windDist, g_eff.alphaCutoff,
               k2grass::effectiveClipDistMeters(g_eff));
    }
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
    perfSample(now);
    ++g_collectCalls;
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
    } else if (fdwReason == DLL_PROCESS_DETACH && lpvReserved == nullptr) {
        unpatchWindOperand();
    }
    return TRUE;
}
