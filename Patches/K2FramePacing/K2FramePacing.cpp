// K2FramePacing.cpp -- K2 Frame Pacing (0.1.0-dev)
// The WinMain loop (0x00407920) has no sleep anywhere: pacing is SwapBuffers/V-Sync only, so a minimized game (no
// swap) spins a full core. One detour at the end of every loop iteration waits instead: a fixed Sleep while minimized,
// an FPS cap while in the background, and an optional user FPS cap (Sleep, then a short QPC spin).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define INIW_FILE_API 1
#define MCDECL_FILE_API 1

#include "../Common/FileWatch.h"
#include "K2FramePacingLogic.h"
#include "../Common/KpLog.h"
#include "../Common/ModConfigApi.h"
#include "../Common/ModConfigDecl.h"
#include "../Common/ProviderClient.h"
#include "../Common/SafeRead.h"

KP_LOG_PATCH("k2-frame-pacing");

namespace {

// Engine global (Steam Aspyr build 6A522E71...): main window HWND, written by the window setup 0x00408AF0 (0x00408C02)
// and passed to GetDC just before SwapBuffers in the loop (0x0040840E).
constexpr uintptr_t AddrMainHwnd = 0x00A1B484;
// Foreground/minimized state is re-read at most this often (GetForegroundWindow / IsIconic are wineserver calls).
constexpr unsigned kModePollMs = 100;

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chCfg, "cfg");
KP_CHANNEL(chPerf, "perf");

const bool g_fallbackSet = (
    KpLog_SetFallbackFile("k2-frame-pacing_log.txt"),
    KpLog_SetFallbackLevel(KPLOG_INFO),
    true);

k2fp::Config g_cfg{};
filewatch::ReloadPoll g_iniPoll{};
bool g_initialized = false;
bool g_bannerLogged = false;
bool g_hwndOk = false;   // the HWND global was readable at init (exe .data, never unmapped after)
uint32_t g_reloadCount = 0;
k2fp::PerfWindow g_perf{};
int64_t g_qpcFreq = 0;
uint64_t g_lastEnd = 0;   // QPC at the end of the previous handler call (0 = none yet)
k2fp::Mode g_mode = k2fp::Mode::Active;
unsigned g_modeMs = 0;
bool g_modePolled = false;
uint32_t g_modeChanges = 0;

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

uint64_t qpcNow() {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return static_cast<uint64_t>(t.QuadPart);
}

int64_t ticksToUs(uint64_t ticks) {
    return g_qpcFreq > 0 ? static_cast<int64_t>(ticks * 1000000ull / static_cast<uint64_t>(g_qpcFreq)) : 0;
}

void ensureDefaultIniFile() {
    FILE* f = fopen(k2fp::kIniFile, "rb");
    if (f) {
        fclose(f);
        return;
    }
    char buf[2048];
    const int n = k2fp::formatDefaultIni(buf, sizeof(buf), g_cfg);
    if (n > 0 && static_cast<size_t>(n) < sizeof(buf)) {
        filewatch::atomicWrite(k2fp::kIniFile, buf, static_cast<size_t>(n));
    }
}

void loadAndApplyIni(bool initial) {
    ensureDefaultIniFile();
    FILE* f = fopen(k2fp::kIniFile, "rb");
    k2fp::Config next{};
    inikit::LoadStats st{};
    bool readOk = false;
    if (f) {
        char buf[4096];
        const size_t n = fread(buf, 1, sizeof(buf) - 1, f);
        buf[n] = '\0';
        fclose(f);
        st = k2fp::parseIniText(buf, next);
        readOk = true;
    }
    const bool changed = initial || (next != g_cfg);
    g_cfg = next;
    ++g_reloadCount;
    if (!g_bannerLogged) {
        g_bannerLogged = true;
        KP_LOG(chMain, "%s %s loaded: enabled=%d minimized_sleep=%d(%dms) background_fps=%d fps_cap=%d spin_us=%d perf_log=%d verbose=%d hwnd_ok=%d ini=%s(ok=%d,bad=%d)",
               k2fp::kModId, k2fp::kVersion, g_cfg.enabled ? 1 : 0, g_cfg.minimizedSleep ? 1 : 0, g_cfg.minimizedSleepMs,
               g_cfg.backgroundFps, g_cfg.fpsCap, g_cfg.spinUs, g_cfg.perfLog ? 1 : 0, g_cfg.verbose ? 1 : 0,
               g_hwndOk ? 1 : 0, readOk ? "read" : "defaults", st.ok, st.bad);
    } else if (changed || g_cfg.verbose) {
        KP_LOG(chCfg, "INI reload #%u: enabled=%d minimized_sleep=%d(%dms) background_fps=%d fps_cap=%d spin_us=%d",
               g_reloadCount, g_cfg.enabled ? 1 : 0, g_cfg.minimizedSleep ? 1 : 0, g_cfg.minimizedSleepMs,
               g_cfg.backgroundFps, g_cfg.fpsCap, g_cfg.spinUs);
    }
}

void pollModConfig() {
    auto clients = k2pc::makeSet(g_mcCfg, g_mcMenu);
    if (clients.anyMissing()) {
        clients.poll();
    }
    if (!g_pageReady) {
        char err[96] = {};
        if (g_page.load(k2fp::kPageSchema, strlen(k2fp::kPageSchema), err, sizeof(err))) {
            g_page.menu = &g_mcMenu;
            g_page.io = mcdecl::fileIo();
            g_mcEntry = g_page.entry(k2fp::kModId);
        } else {
            KP_WARN("ModConfig schema parse failed: %s", err);
        }
        g_pageReady = true;
    }
    if (g_mcEntry.id && g_mcMenu.ok()) {
        k2pc::registerOnce(g_mcReg, g_mcCfg, g_mcEntry, mcLog);
    }
}

void initOnce() {
    if (g_initialized) return;
    g_initialized = true;
    LARGE_INTEGER fq;
    if (QueryPerformanceFrequency(&fq)) g_qpcFreq = fq.QuadPart;
    g_hwndOk = saferead::probeRead(1, reinterpret_cast<const void*>(AddrMainHwnd), sizeof(HWND));
    filewatch::changed(g_iniPoll, k2fp::kIniFile, GetTickCount(), true);
    loadAndApplyIni(true);
    if (!g_hwndOk) KP_WARN("main HWND global 0x%08X unreadable: mode stays active (no minimized/background waits)",
                           static_cast<unsigned>(AddrMainHwnd));
}

// Minimized > background > active. A foreground window owned by the game (e.g. a dialog) still counts as active.
k2fp::Mode readMode() {
    if (!g_hwndOk) return k2fp::Mode::Active;
    const HWND hwnd = *reinterpret_cast<const volatile HWND*>(AddrMainHwnd);
    if (!hwnd) return k2fp::Mode::Active;
    if (IsIconic(hwnd)) return k2fp::Mode::Minimized;
    const HWND fg = GetForegroundWindow();
    if (fg == hwnd || (fg && GetAncestor(fg, GA_ROOTOWNER) == hwnd)) return k2fp::Mode::Active;
    return k2fp::Mode::Background;
}

void updateMode(unsigned nowMs) {
    if (g_modePolled && (nowMs - g_modeMs) < kModePollMs) return;
    g_modePolled = true;
    g_modeMs = nowMs;
    const k2fp::Mode m = readMode();
    if (m != g_mode) {
        ++g_modeChanges;
        KP_LOG(chMain, "mode %s -> %s (#%u)", k2fp::modeName(g_mode), k2fp::modeName(m), g_modeChanges);
        g_mode = m;
    }
}

} // namespace

extern "C" {

void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    auto clients = k2pc::makeSet(g_mcCfg, g_mcMenu);
    clients.init(api);
    initOnce();
    pollModConfig();
}

// Hook 1: end of every WinMain loop iteration (0x0040846E), after the present/skip block.
void __cdecl K2FramePacingOnLoopEnd() {
    initOnce();
    const unsigned nowMs = GetTickCount();
    if (filewatch::changed(g_iniPoll, k2fp::kIniFile, nowMs)) loadAndApplyIni(false);
    if (!g_mcReg.registered) pollModConfig();
    updateMode(nowMs);

    const uint64_t start = qpcNow();
    const int64_t workUs = g_lastEnd ? ticksToUs(start - g_lastEnd) : 0;
    int64_t sleptUs = 0, spunUs = 0;

    if (g_cfg.enabled) {
        const uint32_t fixedMs = k2fp::fixedSleepMs(g_cfg, g_mode);
        if (fixedMs) Sleep(fixedMs);
        const int64_t target = k2fp::targetFrameUs(g_cfg, g_mode);
        if (target > 0 && g_lastEnd && g_qpcFreq > 0) {   // no QPC frequency: ticksToUs is 0, the spin would never end
            const uint32_t ms = k2fp::capSleepMs(target - workUs, g_cfg.spinUs);
            if (ms) Sleep(ms);
            const uint64_t afterSleep = qpcNow();
            sleptUs = ticksToUs(afterSleep - start);
            // Short spin for the last spin_us so the cap does not overshoot by Sleep granularity.
            uint64_t t = afterSleep;
            while (ticksToUs(t - g_lastEnd) < target) {
                YieldProcessor();
                t = qpcNow();
            }
            spunUs = ticksToUs(t - afterSleep);
        } else {
            sleptUs = ticksToUs(qpcNow() - start);
        }
    }

    g_lastEnd = qpcNow();
    if (g_cfg.perfLog) {
        if (!g_perf.started) g_perf.reset(nowMs);
        g_perf.addFrame(g_mode, static_cast<uint32_t>(workUs > 0xFFFFFFFFll ? 0xFFFFFFFFll : workUs),
                        static_cast<uint32_t>(sleptUs), static_cast<uint32_t>(spunUs));
        if (g_perf.due(nowMs)) {
            char line[256];
            g_perf.format(line, sizeof(line), nowMs);
            KP_LOG(chPerf, "%s", line);
            g_perf.reset(nowMs);
        }
    }
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID /*lpvReserved*/) {
    if (fdwReason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(hinstDLL);
    return TRUE;
}
