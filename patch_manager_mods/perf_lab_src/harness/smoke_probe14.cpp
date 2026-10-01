// Smoke test for load-hang-probe 1.4.1 (1.4.1: RestoreAfterMovie / PlayMovies exit logged on tuple change only, calls= counter): 1.3.0 window/activation logging + guarded re-activation fix (now with pass/fail checks) and the
// 1.4.0 probe-log points (ACTIVATEAPP arrival with lParam/foreground discriminator, movie-thread events, RestoreAfterMovie, PlayMovies
// exit/cut-off, MainLoop load-screen decision logged on change only). Exit code 0 = all checks passed.
// Build: i686-w64-mingw32-g++ -O1 -o smoke_probe14.exe smoke_probe14.cpp   (dynamic CRT, default base 0x400000: the 7 MB .bss pad makes the
// image cover 0x400000-0xB00000 so the engine addresses 0xA1B4E0 etc. and 0x40BCF0 are ordinary memory of this exe)
// Run:   smoke_probe14.exe <off|auto|manual>   (dir needs windows_x86.dll from LoadHangProbe.kpatch). Fakes the engine globals at their
// real addresses (0xA1B4E0 ...) and a stub AppActivated at 0x40BCF0 (sets 0xA1B4E0 = 1).
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <string>
static int g_fail = 0;
static void check(bool ok, const char* what) { printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++g_fail; }
static std::string fileText(const char* n) { FILE* f = fopen(n, "r"); if (!f) return ""; std::string s; char b[4096]; size_t k; while ((k = fread(b, 1, sizeof b, f)) > 0) s.append(b, k); fclose(f); return s; }
typedef void (__cdecl *V0)(); typedef void (__cdecl *VP)(void*); typedef void (__cdecl *VII)(int, int); typedef void (__cdecl *VArr)(void*, int, int);
typedef void (__cdecl *MainFn)(void*);
typedef void (__cdecl *WndFn)(void*, unsigned, void*, void*);
typedef void (__cdecl *ActFn)(void*, int);
static char pad[0x700000];   // covers the engine's data addresses
static void unprotect(unsigned addr, unsigned size) { DWORD o; if (!VirtualProtect((void*)addr, size, PAGE_EXECUTE_READWRITE, &o)) { printf("VirtualProtect %08x failed %lu\n", addr, GetLastError()); exit(2); } }
#define G(a) (*(volatile int*)(a))
int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "off";
    FILE* cf = fopen("hangprobe_debug.txt", "w"); fprintf(cf, "io=0\n"); if (!strcmp(mode, "auto")) fprintf(cf, "fix=auto\nfixdelay=1500\n"); fclose(cf);
    DeleteFileA("hangprobe_log.txt");
    MEMORY_BASIC_INFORMATION mb; VirtualQuery((void*)0x40BCF0, &mb, sizeof mb);
    if ((unsigned)pad > 0x9F4000 || (unsigned)pad + sizeof pad < 0xA1C000 || mb.State != MEM_COMMIT) { printf("pad at %p does not cover the engine addresses\n", (void*)pad); return 2; }
    unprotect(0x40B000, 0x1000); unprotect(0xA1B000, 0x1000); unprotect(0x9F4000, 0x1000);
    unsigned char stub[] = { 0xC7, 0x05, 0xE0, 0xB4, 0xA1, 0x00, 0x01, 0x00, 0x00, 0x00, 0xC3 }; memcpy((void*)0x40BCF0, stub, sizeof stub);
    // fake engine objects
    static char internal[0x800], ls[64], mgr[0x200]; static void* modals[2];
    *(unsigned*)ls = 0x009A5C84; modals[0] = ls; *(void**)(internal + 0x278) = ls; *(void**)(internal + 0x274) = mgr; *(void**)(mgr + 0x94) = modals; *(int*)(mgr + 0x98) = 1;
    *(int*)(internal + 0x288) = 1; *(int*)(internal + 0x18) = 1; *(unsigned*)(internal + 0x20) = 0x7f000000;
    // a real window; use whatever Wine reports as foreground as the "game" window so fgIsGame can be true
    WNDCLASSA wc = {}; wc.lpfnWndProc = DefWindowProcA; wc.hInstance = GetModuleHandleA(0); wc.lpszClassName = "smoke"; RegisterClassA(&wc);
    HWND w = CreateWindowA("smoke", "smoke game", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 200, 100, 0, 0, wc.hInstance, 0);
    SetForegroundWindow(w); Sleep(200);
    HWND fg = GetForegroundWindow(); printf("window=%p foreground=%p\n", w, fg);
    *(HWND*)0xA1B484 = w; G(0xA1B4E0) = 0; G(0xA1B4D4) = 2; G(0xA1B48C) = 1;
    HMODULE m = LoadLibraryA("windows_x86.dll"); if (!m) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    MainFn loop = (MainFn)GetProcAddress(m, "HangProbeMainLoop"); WndFn wnd = (WndFn)GetProcAddress(m, "HangProbeWndProc");
    ActFn act = (ActFn)GetProcAddress(m, "HangProbeActivated"), deact = (ActFn)GetProcAddress(m, "HangProbeDeactivated");
    printf("exports: %p %p %p %p\n", (void*)loop, (void*)wnd, (void*)act, (void*)deact); if (!loop || !wnd || !act || !deact) return 3;
    for (int i = 0; i < 10; ++i) { loop(internal); Sleep(16); }
    // ---- 1.4.0 points ----
    VArr arrive = (VArr)GetProcAddress(m, "HangProbeActAppArrive"); V0 mwd = (V0)GetProcAddress(m, "HangProbeMovieWndDeact"), m76 = (V0)GetProcAddress(m, "HangProbeMovie76c"),
        syn = (V0)GetProcAddress(m, "HangProbeSynthPost"), cut = (V0)GetProcAddress(m, "HangProbePumpCutoff"), lsd = (V0)GetProcAddress(m, "HangProbeLsDecision");
    VP rest = (VP)GetProcAddress(m, "HangProbeRestoreAfterMovie"); VII mexit = (VII)GetProcAddress(m, "HangProbeMovieExit");
    check(arrive && mwd && m76 && syn && cut && lsd && rest && mexit, "1.4.0 exports present");
    G(0xA1B764) = 7; G(0xA1B768) = 8; G(0xA1B76C) = 9;
    arrive(w, 0, 0);            // synthetic deactivate while our window is foreground
    arrive(w, 0, 0x1234);       // real deactivate (another app's thread id)
    mwd(); m76(); syn(); rest((void*)0x0040C0DE); mexit(5, 0); cut();
    for (int i = 0; i < 4; ++i) { rest((void*)0x0040C0DE); mexit(5, 0); }   // 1.4.1: identical tuples -> no new lines
    G(0xA1B76C) = 0; rest((void*)0x0040C0DE); rest((void*)0x0040C0DE);       // one change (76c) -> one new line, calls=6
    mexit(5, 1); mexit(5, 1);                                                 // one change (cut-off latch) -> one new line, calls=6
    G(0xA1B76C) = 9;
    for (int i = 0; i < 5; ++i) lsd();   // same state 5x -> one event
    G(0xA1B4E0) = 1; lsd(); G(0xA1B4E0) = 0; lsd(); lsd();   // two changes
    wnd(w, 0x1c, (void*)0, (void*)1234); deact((void*)0x0040A9AA, 0); wnd(w, 0x200, 0, 0 /* WM_MOUSEMOVE: must NOT be logged */);
    wnd(w, 0x1c, (void*)1, (void*)1234); wnd(w, 0x1c, (void*)0, (void*)1234);
    int frames = 0;
    for (int t = 0; t < 400; ++t) {   // ~6.4 s of frames with the load screen up and active=0
        loop(internal); Sleep(16); ++frames;
        if (t == 200 && !strcmp(mode, "manual")) CloseHandle(CreateFileA("hangprobe_activate.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0));
        if (G(0xA1B4E0) && t > 5) { printf("a1b4e0 became 1 at frame %d (mode %s)\n", t, mode); break; }
    }
    printf("final a1b4e0=%d after %d frames (mode %s)\n", G(0xA1B4E0), frames, mode);
    Sleep(800);
    CloseHandle(CreateFileA("hangprobe_dump.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0)); for (int i = 0; i < 260; ++i) { loop(internal); Sleep(16); }
    Sleep(600);
    std::string log = fileText("hangprobe_log.txt");
    check(log.find("load-hang-probe 1.4.1 loaded") != std::string::npos, "1.4.1 version line");
    bool fgOk = GetForegroundWindow() == w;
    check(log.find("ACTIVATEAPP-ARRIVE(0x40B4AD) hwnd=") != std::string::npos && log.find("wp=00000000 lp=00000000") != std::string::npos, "ACTIVATEAPP arrival logged with wParam/lParam");
    if (fgOk) check(log.find("lp=00000000 fgOurs=1") != std::string::npos && log.find("SYNTHETIC DEACTIVATE while foreground is ours") != std::string::npos, "synthetic-deactivate discriminator (wParam 0, lParam 0, our foreground) flagged");
    else printf("SKIP discriminator check: harness window is not foreground under this Wine session\n");
    size_t synthN = 0; for (size_t p = 0; (p = log.find("SYNTHETIC DEACTIVATE", p)) != std::string::npos; ++p) ++synthN;
    check(synthN <= 1, "only the lParam==0 arrival is flagged synthetic");
    check(log.find("MOVIEWND-DEACTIVATE(0x40C6B3)") != std::string::npos && log.find("MOVIE-SETS-76C(0x40D637)") != std::string::npos &&
          log.find("MOVIE-SYNTH-POST-1C(0x40D692)") != std::string::npos && log.find("PUMP-CUTOFF(0x798A77)") != std::string::npos, "movie-thread events + pump cut-off");
    check(log.find("RESTORE-AFTER-MOVIE(0x40BF40) a=00000007 b=00000008 c=00000009 ret=0040c0de") != std::string::npos, "RestoreAfterMovie gates + caller");
    check(log.find("PLAYMOVIES-EXIT(0x798B39) a=00000005 b=00000000") != std::string::npos, "PlayMovies exit locals");
    size_t restN = 0, exitN = 0; for (size_t p = 0; (p = log.find("RESTORE-AFTER-MOVIE", p)) != std::string::npos; ++p) ++restN;
    for (size_t p = 0; (p = log.find("PLAYMOVIES-EXIT", p)) != std::string::npos; ++p) ++exitN;
    check(restN == 2 && exitN == 2, "1.4.1: RestoreAfterMovie / PlayMovies exit logged on change only (2 lines each for 7 calls)");
    check(log.find("RESTORE-AFTER-MOVIE(0x40BF40) a=00000007 b=00000008 c=00000000 ret=0040c0de") != std::string::npos && log.find("PLAYMOVIES-EXIT(0x798B39) a=00000005 b=00000001") != std::string::npos,
          "1.4.1: changed tuple logged with the new values");
    size_t c6 = 0; for (size_t p = 0; (p = log.find("calls=6\n", p)) != std::string::npos; ++p) ++c6;
    check(log.find("calls=1\n") != std::string::npos && c6 == 2, "1.4.1: calls= counts every call (1 on first line, 6 on the change)");
    size_t lsN = 0; for (size_t p = 0; (p = log.find("LS-DECISION-CHANGE", p)) != std::string::npos; ++p) ++lsN;
    check(lsN == 3, "load-screen decision logged on change only (3 changes in 8 calls)");
    check(log.find("WM_?(200)") == std::string::npos, "1.3.0: WM_MOUSEMOVE not logged");
    if (!strcmp(mode, "off")) check(G(0xA1B4E0) == 0 && log.find("FIX AUTO") == std::string::npos, "1.3.0 off: no re-activation");
    else check(G(0xA1B4E0) == 1, "1.3.0 auto/manual: re-activation reached AppActivated");
    printf("%s: %d failures\n", mode, g_fail);
    return g_fail ? 1 : 0;
}
