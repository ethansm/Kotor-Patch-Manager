// k2-activation-fix 1.1.0 (Steam Aspyr build 6A522E71...). Real fix for the first-load hang H1
// (doc 09 s18-19, doc 10 Phase 5B).
//
// Mechanism (incidents 16/17): at the end of the intro movie the movie thread (0x40D430, after setting [0xA1B76C]=1)
// posts WM_ACTIVATEAPP(wParam 0, lParam 0) to the main window (0x40D692..0x40D69E). The movie window had seen a
// deactivate that was only activation moving to OUR main window (Wine sends WM_ACTIVATEAPP between threads of one
// process). The main WndProc 0x40A750 then deactivates the engine (AppDeactivated 0x40BC40, a1b4e0 = 0) and minimizes
// the fullscreen window. When Wine keeps the foreground on our window, the restore brings no WM_ACTIVATEAPP(TRUE),
// RestoreAfterMovie skips its re-activation (76c = 1) and MainLoop never hides the load screen (0x781E0B).
//
// Fix: DllMain replaces that one call, `CALL [PostMessageA]` at 0x40D69E (FF 15 6C 64 98 00), with
// `CALL ActFixMoviePost` + NOP. ActFixMoviePost has PostMessageA's stdcall signature (pops 16 bytes; the engine
// ignores the result at 0x40D6A4). It drops the post only while the foreground window belongs to our process, i.e.
// the app is not really inactive; otherwise it calls PostMessageA exactly as before (user alt-tabbed away during the
// movie). The WndProc, Alt+Enter (0x40BDC0 SendMessage WM_ACTIVATEAPP(0,0), deliberate) and the 0x409 movie path are
// untouched. Not chosen: a WndProc gate at 0x40B4BA, which would also swallow the deliberate Alt+Enter minimize.
//
// Config: $G/activationfix.txt with `enabled=0` disables the patch (site left untouched); absent = enabled.
// Log (1.1.0): shared KpLog.h (channel main): one `loaded` line per launch (+ tick=, emitted by KPatchInit) + one line per post.
// Old core: fallback activationfix_log.txt at INFO.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include "CodePatch.h"
#include "IniKit.h"
#include "KpLog.h"
#include "GameAddr.h"

KP_LOG_PATCH("k2-activation-fix");

namespace {

constexpr uintptr_t SitePost = 0x0040D69E;     // movie thread: CALL dword ptr [0x0098646C] (PostMessageA)
constexpr uintptr_t IatPostMessageA = 0x0098646C;   // game IAT slot used by the replaced CALL
constexpr uintptr_t FlagA1B4E0 = gameaddr::AppActiveFlag;   // engine app-active flag
constexpr uintptr_t Gate76C = gameaddr::MovieRestoreSkipFlag;   // movie-thread "restore will not re-activate" flag
const uint8_t SiteOrig[6] = { 0xFF, 0x15, 0x6C, 0x64, 0x98, 0x00 };

volatile LONG g_ignored = 0, g_passed = 0;
int g_state = 0;            // 1 patched, 0 disabled, -1 refused (bytes), -2 refused (protect), -3 refused (unmapped)
uint8_t g_seen[6];
int g_testFg = -1;          // harness only: -1 real foreground check, 0/1 forced

KP_CHANNEL(chMain, "main");

// Only stores the path and the level (no I/O): safe from a static initialiser. Old core: same file as 1.0.0, at INFO.
const bool g_fallbackSet = (KpLog_SetFallbackFile("activationfix_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

// The load line is formatted in DllMain (setLoad) but emitted later, once the logger is known: by KPatchInit (whether or not
// KpLog_Init succeeded), else by the first hook call / helper-thread tick (old core without KPatchInit).
char g_loadLine[400]; int g_loadLevel = KPLOG_INFO;
volatile LONG g_loadEmitted = 0;
void setLoad(int level, const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); vsnprintf(g_loadLine, sizeof(g_loadLine), fmt, ap); va_end(ap);
    size_t n = strlen(g_loadLine);
    snprintf(g_loadLine + n, sizeof(g_loadLine) - n, " tick=%lu", GetTickCount());
    g_loadLevel = level;
}
void emitLoadLine() {
    if (g_loadEmitted) return;                         // cheap fast path (hot after the first call)
    if (InterlockedExchange(&g_loadEmitted, 1)) return;
    KP_LOGV(chMain, g_loadLevel, "%s", g_loadLine);
}

bool configEnabled() { return inikit::enabledFile("activationfix.txt"); }

bool foregroundIsOurs(HWND* fgOut, DWORD* pidOut) {
    HWND fg = GetForegroundWindow(); DWORD pid = 0;
    if (fg) GetWindowThreadProcessId(fg, &pid);
    *fgOut = fg; *pidOut = pid;
    if (g_testFg >= 0) return g_testFg == 1;
    return fg && pid == GetCurrentProcessId();
}

int writeSite(uintptr_t at, const uint8_t* expect, const uint8_t* repl, int n, uint8_t* seen) {
    if (!codepatch::committed(at, n)) return -3;
    return codepatch::writeChecked(at, expect, repl, n, seen);
}

}  // namespace

// Replaces the movie thread's PostMessageA(main hwnd, WM_ACTIVATEAPP, 0, 0) at 0x40D69E (movie thread).
extern "C" BOOL __stdcall ActFixMoviePost(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    emitLoadLine();
    HWND fg; DWORD fgPid;
    bool ours = foregroundIsOurs(&fg, &fgPid);
    int active = *reinterpret_cast<volatile int*>(FlagA1B4E0), f76c = *reinterpret_cast<volatile int*>(Gate76C);
    if (msg == WM_ACTIVATEAPP && wp == 0 && ours) {
        LONG n = InterlockedIncrement(&g_ignored);
        KP_LOG(chMain, "IGNORED synthetic WM_ACTIVATEAPP(0,0) post from movie thread tid=%lu hwnd=%p fg=%p fgPid=%lu (ours) a1b4e0=%d 76c=%d n=%ld",
             GetCurrentThreadId(), hwnd, fg, fgPid, active, f76c, n);
        return TRUE;
    }
    // Forward through the game's own IAT slot, exactly as the replaced CALL [0x0098646C] did (keeps any IAT hook of other patches).
    auto post = *reinterpret_cast<BOOL (WINAPI* volatile*)(HWND, UINT, WPARAM, LPARAM)>(IatPostMessageA);
    BOOL r = post(hwnd, msg, wp, lp);
    LONG n = InterlockedIncrement(&g_passed);
    KP_LOG(chMain, "PASSED movie-thread post msg=%04x wp=%08lx lp=%08lx tid=%lu hwnd=%p fg=%p fgPid=%lu ours=%d a1b4e0=%d 76c=%d ret=%d n=%ld",
         msg, static_cast<unsigned long>(wp), static_cast<unsigned long>(lp), GetCurrentThreadId(), hwnd, fg, fgPid, ours ? 1 : 0, active, f76c, r, n);
    return r;
}

// Harness-only exports (never called by the game).
extern "C" void __cdecl ActFixTestSetForeground(int ours) { g_testFg = ours; }
extern "C" int __cdecl ActFixTestState() { return g_state; }

// Written out literally (create-patch's export scan reads the source text) and listed in exports.def.
extern "C" void __cdecl KPatchInit(const KPatchApi* api) { KpLog_Init(api); emitLoadLine(); }

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason != DLL_PROCESS_ATTACH) return TRUE;
    if (!configEnabled()) {
        g_state = 0;
        setLoad(KPLOG_INFO, "k2-activation-fix 0.1.1 loaded: DISABLED by activationfix.txt (site 0x%08x untouched) pid=%lu", (unsigned)SitePost, GetCurrentProcessId());
        return TRUE;
    }
    uint8_t repl[6] = { 0xE8, 0, 0, 0, 0, 0x90 };
    codepatch::putRel32(repl + 1, SitePost + 1, reinterpret_cast<uintptr_t>(&ActFixMoviePost));
    g_state = writeSite(SitePost, SiteOrig, repl, 6, g_seen);
    if (g_state == 1)
        setLoad(KPLOG_INFO, "k2-activation-fix 0.1.1 loaded: site 0x%08x patched (CALL [PostMessageA] -> ActFixMoviePost %p) pid=%lu",
             (unsigned)SitePost, reinterpret_cast<void*>(&ActFixMoviePost), GetCurrentProcessId());
    else if (g_state == -1)
        setLoad(KPLOG_WARN, "k2-activation-fix 0.1.1 loaded: REFUSED(bytes %02x %02x %02x %02x %02x %02x, expected ff 15 6c 64 98 00) site 0x%08x untouched pid=%lu",
             g_seen[0], g_seen[1], g_seen[2], g_seen[3], g_seen[4], g_seen[5], (unsigned)SitePost, GetCurrentProcessId());
    else
        setLoad(KPLOG_WARN, "k2-activation-fix 0.1.1 loaded: REFUSED(%s) site 0x%08x untouched pid=%lu", g_state == -2 ? "protect" : "unmapped", (unsigned)SitePost, GetCurrentProcessId());
    return TRUE;
}
