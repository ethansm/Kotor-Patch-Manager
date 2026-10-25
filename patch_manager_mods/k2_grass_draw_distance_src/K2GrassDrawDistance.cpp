// K2GrassDrawDistance.cpp -- K2 Grass Draw Distance (0.1.0-dev)
// Extends grass octree build/collect radius, fade-start/fade-end distances, alpha-test threshold,
// and decouples CPU wind animation radius.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define INIW_FILE_API 1
#define MCDECL_FILE_API 1

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
//   0x00A0F070: float _DAT_00a0f070 (vanilla 225.0f = 15m^2; repurposed for wind_dist^2 at RenderGrassPolys wind check)
//   0x00A0F074: float _DAT_00a0f074 (vanilla 900.0f = 30m^2; octree sphere query & RenderGrassPolys fade_end_sq)
//   0x009F5D38: float _DAT_009f5d38 (vanilla 0.40f; Scene::DoBinBucket glAlphaFunc threshold)
constexpr uintptr_t AddrGrassWindDistSq = 0x00A0F070;
constexpr uintptr_t AddrGrassFadeEndSq = 0x00A0F074;
constexpr uintptr_t AddrGrassAlphaCutoff = 0x009F5D38;

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chCfg, "cfg");

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
    if (saferead::probeRead(1, reinterpret_cast<const void*>(AddrGrassWindDistSq), sizeof(float))) {
        *reinterpret_cast<float*>(AddrGrassWindDistSq) = ev.windDistSq;
    }
    if (saferead::probeRead(2, reinterpret_cast<const void*>(AddrGrassFadeEndSq), sizeof(float))) {
        *reinterpret_cast<float*>(AddrGrassFadeEndSq) = ev.fadeEndSq;
    }
    if (saferead::probeRead(3, reinterpret_cast<const void*>(AddrGrassAlphaCutoff), sizeof(float))) {
        *reinterpret_cast<float*>(AddrGrassAlphaCutoff) = ev.alphaCutoff;
    }
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
               "%s %s loaded: enabled=%d fade=%.0fm..%.0fm (sq=%.0f..%.0f) wind=%.0fm (sq=%.0f) alpha=%.2f clip=%.1fm ini=%s(ok=%d,bad=%d)",
               k2grass::kModId, k2grass::kVersion,
               g_cfg.enabled ? 1 : 0,
               g_cfg.fadeStartDist, g_cfg.fadeEndDist,
               g_eff.fadeStartSq, g_eff.fadeEndSq,
               g_cfg.windDist, g_eff.windDistSq,
               g_eff.alphaCutoff,
               k2grass::effectiveClipDistMeters(g_eff),
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
    g_initialized = true;
    pollModConfig();
}

// Hook 1: Scene::CollectAndTraverse (at 0x00499ABC, once per frame when grass is enabled in the scene).
void __cdecl K2GrassOnSceneCollect() {
    const unsigned now = GetTickCount();
    if (!g_initialized) {
        filewatch::changed(g_iniPoll, k2grass::kIniFile, now, true);
        loadAndApplyIni(true);
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
    ++g_collectCalls;
}

// Hook 2: CAurTriangleBin::RenderGrassPolys (at 0x0050DB55, sets [EBP-0x28] = fadeStartSq).
void __cdecl K2GrassOnBinFadeInit(uintptr_t ebp) {
    if (ebp != 0) {
        *reinterpret_cast<float*>(ebp - 0x28) = g_eff.fadeStartSq;
    }
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    (void)lpvReserved;
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
    }
    return TRUE;
}
