// K2ShadowDistance.cpp -- K2 Shadow Distance (0.1.0-dev)
// Extends the object shadow distance gates in Scene::DoGobShadows / DoGobShadowsForThisLight: one .data float
// (re-asserted every shadow-light call) and one operand rewrite at KPatchInit (code_patches.toml).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define INIW_FILE_API 1
#define MCDECL_FILE_API 1

#include "../Common/CodePatch.h"
#include "../Common/FileWatch.h"
#include "K2ShadowDistanceLogic.h"
#include "../Common/KpLog.h"
#include "../Common/ModConfigApi.h"
#include "../Common/ModConfigDecl.h"
#include "../Common/ProviderClient.h"
#include "../Common/SafeRead.h"

KP_LOG_PATCH("k2-shadow-distance");

namespace {

// Engine globals (Steam Aspyr build 6A522E71...):
//   0x009F5D50: float (vanilla 35.0, .data): object distance gate, read at 0x00468984 (DoGobShadows) and 0x00468D76
//               (DoGobShadowsForThisLight)
constexpr uintptr_t AddrShadowDist = 0x009F5D50;
// DoGobShadowsForThisLight view-reference gate `FLD float ptr [0x0098BA98]` (1100.0, .rdata); only the imm32
// changes, to &g_shadowRefSq.
constexpr uintptr_t AddrRefFld = 0x00468CDB;
const uint8_t kRefFldOrig[6] = {0xD9, 0x05, 0x98, 0xBA, 0x98, 0x00};
// Read only (init log), never written by this mod:
constexpr uintptr_t AddrLightCap = 0x00A0F040;    // int, vanilla 1: shadow-casting light cap
constexpr uintptr_t AddrCutFlag = 0x009F5D54;     // int, vanilla 1: object distance cut on
constexpr uintptr_t AddrShadowsOn = 0x009F5D30;   // int: master Shadows option
constexpr uintptr_t AddrSoftShadows = 0x009F5DA8; // int: Soft Shadows option

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chCfg, "cfg");
KP_CHANNEL(chPerf, "perf");

const bool g_fallbackSet = (
    KpLog_SetFallbackFile("k2-shadow-distance_log.txt"),
    KpLog_SetFallbackLevel(KPLOG_INFO),
    true);

k2shadow::Config g_cfg{};
k2shadow::EffectiveValues g_eff = k2shadow::computeEffective(g_cfg);
filewatch::ReloadPoll g_iniPoll{};
bool g_initialized = false;
bool g_bannerLogged = false;
bool g_engineLogged = false;
uint32_t g_lightCalls = 0;
uint32_t g_reloadCount = 0;
// View-reference cutoff (squared m) the patched FLD reads; vanilla 1100.0.
float g_shadowRefSq = k2shadow::kVanillaRefSq;
uint8_t g_refFldNew[6] = {};
int g_refPatch = 0;   // codepatch::writeChecked result; 1 = patched
k2shadow::PerfWindow g_perf{};

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

void writeEngineGlobals(const k2shadow::EffectiveValues& ev) {
    g_shadowRefSq = ev.refSq;
    if (saferead::probeRead(1, reinterpret_cast<const void*>(AddrShadowDist), sizeof(float))) {
        *reinterpret_cast<float*>(AddrShadowDist) = ev.dist;
    }
}

// Point the view-reference FLD at g_shadowRefSq. On a byte mismatch nothing is written and that gate stays at the
// vanilla 1100.0 (logged).
void patchRefOperand() {
    if (g_refPatch == 1) return;
    memcpy(g_refFldNew, kRefFldOrig, 2);
    const uint32_t a = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&g_shadowRefSq));
    memcpy(g_refFldNew + 2, &a, 4);
    uint8_t seen[6] = {};
    g_refPatch = codepatch::writeChecked(AddrRefFld, kRefFldOrig, g_refFldNew, 6, seen);
    KP_LOG(chMain, "patch ref_dist_fld @0x%08X rc=%d seen=%02X %02X %02X %02X %02X %02X -> &g_shadowRefSq=0x%08X",
           static_cast<unsigned>(AddrRefFld), g_refPatch, seen[0], seen[1], seen[2], seen[3], seen[4], seen[5], a);
    if (g_refPatch != 1) {
        KP_WARN("ref_dist_fld NOT patched (rc=%d): the view-reference gate stays at the vanilla 1100", g_refPatch);
    }
}

// FreeLibrary only: put the vanilla operand back before g_shadowRefSq goes away.
void unpatchRefOperand() {
    if (g_refPatch != 1) return;
    uint8_t seen[6] = {};
    if (codepatch::writeChecked(AddrRefFld, g_refFldNew, kRefFldOrig, 6, seen) == 1) g_refPatch = 0;
}

// One-time read-only snapshot of the related engine state (nothing here is ever written).
void logEngineState() {
    if (g_engineLogged) return;
    g_engineLogged = true;
    int32_t cap = -1, cut = -1, on = -1, soft = -1;
    const bool okCap = saferead::readAt<int32_t>(2, AddrLightCap, 0, &cap);
    const bool okCut = saferead::readAt<int32_t>(3, AddrCutFlag, 0, &cut);
    const bool okOn = saferead::readAt<int32_t>(4, AddrShadowsOn, 0, &on);
    const bool okSoft = saferead::readAt<int32_t>(5, AddrSoftShadows, 0, &soft);
    KP_LOG(chMain, "engine state (read only): light_cap[0x%08X]=%d%s cut_flag[0x%08X]=%d%s shadows_on[0x%08X]=%d%s soft[0x%08X]=%d%s",
           static_cast<unsigned>(AddrLightCap), static_cast<int>(cap), okCap ? "" : "(unreadable)",
           static_cast<unsigned>(AddrCutFlag), static_cast<int>(cut), okCut ? "" : "(unreadable)",
           static_cast<unsigned>(AddrShadowsOn), static_cast<int>(on), okOn ? "" : "(unreadable)",
           static_cast<unsigned>(AddrSoftShadows), static_cast<int>(soft), okSoft ? "" : "(unreadable)");
}

void perfSample(unsigned now) {
    if (!g_cfg.perfLog) return;
    if (g_perf.startMs == 0) {
        g_perf.reset(now);
    }
    g_perf.add(now);
    if (!g_perf.due(now)) return;
    KP_LOG(chPerf, "PERF shadow_lights=%u per_s=%.0f dist=%.0fm", g_perf.calls, g_perf.perSec(now), g_eff.dist);
    g_perf.reset(now);
}

void ensureDefaultIniFile() {
    FILE* f = fopen(k2shadow::kIniFile, "rb");
    if (f) {
        fclose(f);
        return;
    }
    char buf[1024];
    const int n = k2shadow::formatDefaultIni(buf, sizeof(buf), g_cfg);
    if (n > 0 && static_cast<size_t>(n) < sizeof(buf)) {
        filewatch::atomicWrite(k2shadow::kIniFile, buf, static_cast<size_t>(n));
    }
}

void loadAndApplyIni(bool initial) {
    ensureDefaultIniFile();
    FILE* f = fopen(k2shadow::kIniFile, "rb");
    k2shadow::Config nextCfg{};
    inikit::LoadStats st{};
    bool readOk = false;
    if (f) {
        char buf[4096];
        const size_t n = fread(buf, 1, sizeof(buf) - 1, f);
        buf[n] = '\0';
        fclose(f);
        st = k2shadow::parseIniText(buf, nextCfg);
        readOk = true;
    }
    const bool changed = initial || (nextCfg != g_cfg);
    g_cfg = nextCfg;
    g_eff = k2shadow::computeEffective(g_cfg);
    writeEngineGlobals(g_eff);
    ++g_reloadCount;

    if (!g_bannerLogged) {
        g_bannerLogged = true;
        KP_LOG(chMain, "%s %s loaded: enabled=%d shadow_dist=%.0fm (ref_sq=%.0f) perf_log=%d verbose=%d ini=%s(ok=%d,bad=%d)",
               k2shadow::kModId, k2shadow::kVersion, g_cfg.enabled ? 1 : 0, g_eff.dist, g_eff.refSq, g_cfg.perfLog ? 1 : 0,
               g_cfg.verbose ? 1 : 0, readOk ? "read" : "defaults", st.ok, st.bad);
    } else if (changed || g_cfg.verbose) {
        KP_LOG(chCfg, "INI reload #%u: enabled=%d shadow_dist=%.0fm (ref_sq=%.0f)", g_reloadCount, g_cfg.enabled ? 1 : 0,
               g_eff.dist, g_eff.refSq);
    }
}

void pollModConfig() {
    auto clients = k2pc::makeSet(g_mcCfg, g_mcMenu);
    if (clients.anyMissing()) {
        clients.poll();
    }
    if (!g_pageReady) {
        char err[96] = {};
        if (g_page.load(k2shadow::kPageSchema, strlen(k2shadow::kPageSchema), err, sizeof(err))) {
            g_page.menu = &g_mcMenu;
            g_page.io = mcdecl::fileIo();
            g_mcEntry = g_page.entry(k2shadow::kModId);
        } else {
            KP_WARN("ModConfig schema parse failed: %s", err);
        }
        g_pageReady = true;
    }
    if (g_mcEntry.id && g_mcMenu.ok()) {
        k2pc::registerOnce(g_mcReg, g_mcCfg, g_mcEntry, mcLog);
    }
}

} // namespace

extern "C" {

void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    auto clients = k2pc::makeSet(g_mcCfg, g_mcMenu);
    clients.init(api);
    filewatch::changed(g_iniPoll, k2shadow::kIniFile, GetTickCount(), true);
    loadAndApplyIni(true);
    logEngineState();
    patchRefOperand();
    g_initialized = true;
    pollModConfig();
}

// Hook 1: Scene::DoGobShadowsForThisLight (0x00468B50, entry; once per shadow-casting light per frame).
void __cdecl K2ShadowOnLight() {
    const unsigned now = GetTickCount();
    if (!g_initialized) {
        filewatch::changed(g_iniPoll, k2shadow::kIniFile, now, true);
        loadAndApplyIni(true);
        logEngineState();
        patchRefOperand();
        g_initialized = true;
    } else if (filewatch::changed(g_iniPoll, k2shadow::kIniFile, now)) {
        loadAndApplyIni(false);
    } else {
        // Re-assert the global each call so area loads cannot clobber it.
        writeEngineGlobals(g_eff);
    }
    if (!g_mcReg.registered) {
        pollModConfig();
    }
    perfSample(now);
    ++g_lightCalls;
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
    } else if (fdwReason == DLL_PROCESS_DETACH && lpvReserved == nullptr) {
        unpatchRefOperand();
    }
    return TRUE;
}
