// Smoke test for load-hang-probe 1.3.0: window/activation logging + guarded re-activation fix.
// Build: i686-w64-mingw32-g++ -O1 -o smoke_probe13.exe smoke_probe13.cpp   (dynamic CRT, default base 0x400000: the 7 MB .bss pad makes the
// image cover 0x400000-0xB00000 so the engine addresses 0xA1B4E0 etc. and 0x40BCF0 are ordinary memory of this exe)
// Run:   smoke_probe13.exe <off|auto|manual>   (dir needs windows_x86.dll from LoadHangProbe.kpatch). Fakes the engine globals at their
// real addresses (0xA1B4E0 ...) and a stub AppActivated at 0x40BCF0 (sets 0xA1B4E0 = 1).
#include <windows.h>
#include <stdio.h>
#include <string.h>
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
    return 0;
}
