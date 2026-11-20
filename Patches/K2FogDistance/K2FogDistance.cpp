// K2FogDistance.cpp -- K2 Fog Distance (0.1.1-dev)
// 0.1.1: Scene::SetFogRange hook records the engine-set area range, so an area whose vanilla fog equals the last scaled
// value written to the same Scene is still scaled (R12 review 1.5a).
// Scales each area's distance fog (Scene +0x64 start / +0x68 end) by configurable percentages, or removes it.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define INIW_FILE_API 1
#define MCDECL_FILE_API 1

#include "../Common/FileWatch.h"
#include "K2FogDistanceLogic.h"
#include "../Common/KpLog.h"
#include "../Common/ModConfigApi.h"
#include "../Common/ModConfigDecl.h"
#include "../Common/ProviderClient.h"
#include "../Common/SafeRead.h"

KP_LOG_PATCH("k2-fog-distance");

namespace {

// Scene fields (Steam Aspyr build 6A522E71...; Scene::SetFogRange 0x00464C80 stores +0x64/+0x68,
// Scene::DoFog 0x0046A980 reads +0x60 and passes +0x64/+0x68 to glFogf(GL_FOG_START/GL_FOG_END)).
constexpr uintptr_t OffFogOn = 0x60;
constexpr uintptr_t OffFogStart = 0x64;
constexpr uintptr_t OffFogEnd = 0x68;

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chCfg, "cfg");
KP_CHANNEL(chArea, "area");

const bool g_fallbackSet = (
    KpLog_SetFallbackFile("k2-fog-distance_log.txt"),
    KpLog_SetFallbackLevel(KPLOG_INFO),
    true);

k2fog::Config g_cfg{};
k2fog::FogTracker g_tracker{};
filewatch::ReloadPoll g_iniPoll{};
bool g_initialized = false;
bool g_bannerLogged = false;
uint32_t g_fogCalls = 0;
uint32_t g_setRangeCalls = 0;
uint32_t g_reloadCount = 0;
uint32_t g_areaLogs = 0;
// Last logged area fog, so a day/night flip back and forth logs once per distinct value pair.
float g_loggedStart = -1.0f;
float g_loggedEnd = -1.0f;
constexpr uint32_t kMaxAreaLogs = 200;

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

void ensureDefaultIniFile() {
    FILE* f = fopen(k2fog::kIniFile, "rb");
    if (f) {
        fclose(f);
        return;
    }
    char buf[1024];
    const int n = k2fog::formatDefaultIni(buf, sizeof(buf), g_cfg);
    if (n > 0 && static_cast<size_t>(n) < sizeof(buf)) {
        filewatch::atomicWrite(k2fog::kIniFile, buf, static_cast<size_t>(n));
    }
}

void loadIni(bool initial) {
    ensureDefaultIniFile();
    FILE* f = fopen(k2fog::kIniFile, "rb");
    k2fog::Config nextCfg{};
    inikit::LoadStats st{};
    bool readOk = false;
    if (f) {
        char buf[4096];
        const size_t n = fread(buf, 1, sizeof(buf) - 1, f);
        buf[n] = '\0';
        fclose(f);
        st = k2fog::parseIniText(buf, nextCfg);
        readOk = true;
    }
    const bool changed = initial || (nextCfg != g_cfg);
    g_cfg = nextCfg;
    ++g_reloadCount;

    if (!g_bannerLogged) {
        g_bannerLogged = true;
        KP_LOG(chMain, "%s %s loaded: enabled=%d fog_end=%d%% fog_start=%d%% verbose=%d ini=%s(ok=%d,bad=%d)",
               k2fog::kModId, k2fog::kVersion, g_cfg.enabled ? 1 : 0, static_cast<int>(g_cfg.fogEndPct),
               static_cast<int>(g_cfg.fogStartPct), g_cfg.verbose ? 1 : 0, readOk ? "read" : "defaults", st.ok, st.bad);
    } else if (changed) {
        KP_LOG(chCfg, "INI reload #%u: enabled=%d fog_end=%d%% fog_start=%d%%", g_reloadCount, g_cfg.enabled ? 1 : 0,
               static_cast<int>(g_cfg.fogEndPct), static_cast<int>(g_cfg.fogStartPct));
        g_loggedStart = g_loggedEnd = -1.0f;   // log the next applied range under the new settings
    }
}

void pollModConfig() {
    auto clients = k2pc::makeSet(g_mcCfg, g_mcMenu);
    if (clients.anyMissing()) {
        clients.poll();
    }
    if (!g_pageReady) {
        char err[96] = {};
        if (g_page.load(k2fog::kPageSchema, strlen(k2fog::kPageSchema), err, sizeof(err))) {
            g_page.menu = &g_mcMenu;
            g_page.io = mcdecl::fileIo();
            g_mcEntry = g_page.entry(k2fog::kModId);
        } else {
            KP_WARN("ModConfig schema parse failed: %s", err);
        }
        g_pageReady = true;
    }
    if (g_mcEntry.id && g_mcMenu.ok()) {
        k2pc::registerOnce(g_mcReg, g_mcCfg, g_mcEntry, mcLog);
    }
}

void applyFog(uintptr_t scene) {
    int32_t fogOn = 0;
    float start = 0.0f, end = 0.0f;
    if (!saferead::readAt<int32_t>(1, scene, OffFogOn, &fogOn) || !saferead::readAt<float>(2, scene, OffFogStart, &start) ||
        !saferead::readAt<float>(3, scene, OffFogEnd, &end)) {
        return;
    }
    if (fogOn == 0) return;   // fog-less area: KPM Shader Fixes owns that path (0x0046AADD)
    const k2fog::UpdateResult r = g_tracker.update(scene, start, end, g_cfg);
    if (r.write) {
        *reinterpret_cast<float*>(scene + OffFogStart) = r.range.start;
        *reinterpret_cast<float*>(scene + OffFogEnd) = r.range.end;
    }
    const bool distinct = r.range.start != g_loggedStart || r.range.end != g_loggedEnd;
    if ((r.newOrigin || distinct) && (distinct || g_cfg.verbose) && g_areaLogs < kMaxAreaLogs) {
        ++g_areaLogs;
        g_loggedStart = r.range.start;
        g_loggedEnd = r.range.end;
        KP_LOG(chArea, "area fog scene=0x%08X vanilla=%.1f..%.1fm applied=%.1f..%.1fm (end=%d%% start=%d%%)",
               static_cast<unsigned>(scene), r.orig.start, r.orig.end, r.range.start, r.range.end,
               static_cast<int>(g_cfg.fogEndPct), static_cast<int>(g_cfg.fogStartPct));
    }
}

} // namespace

extern "C" {

void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    auto clients = k2pc::makeSet(g_mcCfg, g_mcMenu);
    clients.init(api);
    filewatch::changed(g_iniPoll, k2fog::kIniFile, GetTickCount(), true);
    loadIni(true);
    g_initialized = true;
    pollModConfig();
}

// Hook 1: Scene::DoFog (0x0046A989, ECX = Scene), before it reads the fog fields.
void __cdecl K2FogOnDoFog(void* scene) {
    const unsigned now = GetTickCount();
    if (!g_initialized) {
        filewatch::changed(g_iniPoll, k2fog::kIniFile, now, true);
        loadIni(true);
        g_initialized = true;
    } else if (filewatch::changed(g_iniPoll, k2fog::kIniFile, now)) {
        loadIni(false);
    }
    if (!g_mcReg.registered) {
        pollModConfig();
    }
    if (scene) applyFog(reinterpret_cast<uintptr_t>(scene));
    ++g_fogCalls;
}

// Hook 2 (0.1.1): Scene::SetFogRange (0x00464C80, virtual, __thiscall(start, end)) at 0x00464C8A, before the stores:
// [EBP-4] = Scene, [EBP+8] = start, [EBP+0xC] = end. Records the engine's range as the area origin.
void __cdecl K2FogOnSetFogRange(uint8_t* ebp) {
    if (ebp == nullptr) return;
    saferead::beginScope();
    uint32_t scene = 0;
    float start = 0.0f, end = 0.0f;
    const uintptr_t base = reinterpret_cast<uintptr_t>(ebp);
    if (!saferead::readAt<uint32_t>(4, base - 4, 0, &scene) || !saferead::readAt<float>(5, base, 8, &start) ||
        !saferead::readAt<float>(6, base, 0xC, &end)) {
        return;
    }
    g_tracker.noteEngineRange(scene, start, end);
    if (g_cfg.verbose || g_setRangeCalls < 20) {
        KP_LOG(chArea, "SetFogRange scene=0x%08X %.1f..%.1fm (call %u)", static_cast<unsigned>(scene), start, end,
               static_cast<unsigned>(g_setRangeCalls + 1));
    }
    ++g_setRangeCalls;
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
    }
    return TRUE;
}
