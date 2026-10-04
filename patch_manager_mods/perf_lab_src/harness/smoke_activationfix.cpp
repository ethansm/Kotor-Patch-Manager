// Smoke test for k2-activation-fix 1.1.0 (DLL-only; DllMain rewrites the movie thread's CALL [PostMessageA] at 0x40D69E).
// Build: i686-w64-mingw32-g++ -O1 -static-libgcc -static-libstdc++ -o smoke_activationfix.exe smoke_activationfix.cpp
//        (default base 0x400000; the 7 MB .bss pad makes the image cover 0x400000-0xB00000, so 0x40D692, the IAT slot
//        0x98646C and the engine globals 0xA1B4xx are ordinary memory of this exe; check `objdump -h`: .text must end
//        below 0x40D000)
// Run:   smoke_activationfix.exe <run|disabled|refuse>   (dir needs windows_x86.dll from ActivationFix.kpatch). Exit 0 = pass.
// The fake movie-thread fragment at 0x40D692 is the real 18 bytes (PUSH 0 / PUSH 0 / PUSH 0x1C / MOV EAX,[0xA1B484] /
// PUSH EAX / CALL [0x98646C]) + RET; the IAT slot points at FakePost, which records each call.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static int g_fail = 0;
static void check(bool ok, const char* what) { printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++g_fail; }
static char pad[0x700000];   // covers 0x40D000 .. 0xA1C000
static void unprotect(unsigned addr, unsigned size) { DWORD o; if (!VirtualProtect((void*)addr, size, PAGE_EXECUTE_READWRITE, &o)) { printf("VirtualProtect %08x failed %lu\n", addr, GetLastError()); exit(2); } }
static volatile int g_posts = 0; static HWND g_lastHwnd; static UINT g_lastMsg; static WPARAM g_lastWp; static LPARAM g_lastLp;
static BOOL __stdcall FakePost(HWND h, UINT m, WPARAM w, LPARAM l) { ++g_posts; g_lastHwnd = h; g_lastMsg = m; g_lastWp = w; g_lastLp = l; return TRUE; }
static char* g_log;   // heap: a static buffer would sit in .bss in front of pad and push it past 0x40D692
static void readLog() { if (!g_log) g_log = (char*)malloc(16384); g_log[0] = 0; FILE* f = fopen("activationfix_log.txt", "r"); if (!f) return; size_t k = fread(g_log, 1, 16383, f); g_log[k] = 0; fclose(f); }
static int count(const char* s) { int n = 0; for (const char* p = g_log; (p = strstr(p, s)) != nullptr; ++p) ++n; return n; }
static void checkLoadOnce() { int n = 0; if (g_log) for (const char* p = g_log; (p = strstr(p, " 1.1.0 loaded: ")) != nullptr; ++p) ++n; check(n == 1, "log: load line appears exactly once"); }
// Calls the fake fragment and returns ESP after minus ESP before (0 = stack balanced, i.e. the replacement pops 16 bytes).
static int callSite() {
    int delta;
    __asm__ __volatile__(
        "pushl %%ebx\n\t"
        "movl %%esp, %%ebx\n\t"
        "movl $0x0040D692, %%eax\n\t"
        "call *%%eax\n\t"
        "movl %%esp, %%eax\n\t"
        "subl %%ebx, %%eax\n\t"
        "movl %%ebx, %%esp\n\t"
        "popl %%ebx\n\t"
        : "=a"(delta) : : "ecx", "edx", "memory");
    return delta;
}
typedef void (__cdecl *SetFg)(int); typedef int (__cdecl *GetSt)();
int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "run";
    MEMORY_BASIC_INFORMATION mb; VirtualQuery((void*)0x40D692, &mb, sizeof mb);
    if ((unsigned)pad > 0x40D600 || (unsigned)pad + sizeof pad < 0xA1C000 || mb.State != MEM_COMMIT) { printf("pad at %p does not cover 0x40D600-0xA1C000\n", (void*)pad); return 2; }
    unprotect(0x40D000, 0x1000); unprotect(0x986000, 0x1000); unprotect(0xA1B000, 0x1000);
    static const unsigned char frag[19] = { 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x1C, 0xA1, 0x84, 0xB4, 0xA1, 0x00, 0x50, 0xFF, 0x15, 0x6C, 0x64, 0x98, 0x00, 0xC3 };
    memcpy((void*)0x40D692, frag, sizeof frag);
    if (!strcmp(mode, "refuse")) *(unsigned char*)0x40D6A3 = 0x01;   // CALL [0x0198646C]: wrong bytes
    *(void**)0x98646C = (void*)&FakePost;
    WNDCLASSA wc = {}; wc.lpfnWndProc = DefWindowProcA; wc.hInstance = GetModuleHandleA(0); wc.lpszClassName = "smokeaf"; RegisterClassA(&wc);
    HWND w = CreateWindowA("smokeaf", "smoke game", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 200, 100, 0, 0, wc.hInstance, 0);
    SetForegroundWindow(w); Sleep(200);
    *(HWND*)0xA1B484 = w; *(volatile int*)0xA1B4E0 = 1; *(volatile int*)0xA1B76C = 1;
    DeleteFileA("activationfix_log.txt");
    if (!strcmp(mode, "disabled")) { FILE* cf = fopen("activationfix.txt", "w"); fprintf(cf, "enabled=0\n"); fclose(cf); } else DeleteFileA("activationfix.txt");
    unsigned char before[6]; memcpy(before, (void*)0x40D69E, 6);
    HMODULE m = LoadLibraryA("windows_x86.dll"); if (!m) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    { typedef void (__cdecl *InitFn)(const void*); InitFn ini = (InitFn)GetProcAddress(m, "KPatchInit"); check(ini != nullptr, "KPatchInit export present"); if (ini) ini(nullptr); }
    SetFg setFg = (SetFg)GetProcAddress(m, "ActFixTestSetForeground"); GetSt state = (GetSt)GetProcAddress(m, "ActFixTestState");
    check(setFg && state, "test exports present");
    if (!setFg || !state) return 3;
    const unsigned char* site = (const unsigned char*)0x40D69E;
    if (!strcmp(mode, "run")) {
        check(state() == 1, "state = patched");
        int rel; memcpy(&rel, site + 1, 4);
        unsigned target = 0x40D6A3 + rel, lo = (unsigned)m; MEMORY_BASIC_INFORMATION dm; VirtualQuery((void*)target, &dm, sizeof dm);
        check(site[0] == 0xE8 && site[5] == 0x90 && target > lo && (unsigned)dm.AllocationBase == lo, "site rewritten to CALL rel32 (into the patch DLL) + NOP");
        check(memcmp((void*)0x40D692, frag, 12) == 0 && site[6] == 0xC3, "bytes around the site untouched");
        setFg(1); g_posts = 0; int d1 = callSite();
        check(g_posts == 0 && d1 == 0, "foreground ours: synthetic WM_ACTIVATEAPP(0,0) dropped, stack balanced");
        setFg(0); g_posts = 0; int d2 = callSite();
        check(g_posts == 1 && g_lastHwnd == w && g_lastMsg == 0x1C && g_lastWp == 0 && g_lastLp == 0 && d2 == 0, "foreground not ours: post forwarded unchanged (hwnd, 0x1C, 0, 0), stack balanced");
        setFg(-1); g_posts = 0; bool fgOurs = GetForegroundWindow() == w; int d3 = callSite();
        if (fgOurs) check(g_posts == 0 && d3 == 0, "real foreground check (harness window in front): dropped");
        else printf("SKIP real foreground check: harness window is not foreground under this Wine session (posts=%d)\n", g_posts);
        readLog(); checkLoadOnce();
        check(strstr(g_log, "k2-activation-fix 1.1.0 loaded: site 0x0040d69e patched") != nullptr, "log: patched line");
        check(count("IGNORED synthetic WM_ACTIVATEAPP(0,0)") == (fgOurs ? 2 : 1) && count("PASSED movie-thread post msg=001c") == 1, "log: IGNORED/PASSED lines");
        check(strstr(g_log, "a1b4e0=1 76c=1") != nullptr, "log: engine flags recorded");
    } else if (!strcmp(mode, "disabled")) {
        check(state() == 0 && memcmp(site, before, 6) == 0, "disabled: site untouched");
        g_posts = 0; int d = callSite();
        check(g_posts == 1 && d == 0, "disabled: original CALL [PostMessageA] runs");
        readLog(); checkLoadOnce(); check(strstr(g_log, "DISABLED by activationfix.txt") != nullptr, "log: disabled line");
        DeleteFileA("activationfix.txt");
    } else {
        check(state() == -1 && memcmp(site, before, 6) == 0, "refuse: wrong bytes -> refused, site untouched");
        readLog(); checkLoadOnce(); check(strstr(g_log, "REFUSED(bytes ff 15 6c 64 98 01") != nullptr, "log: REFUSED(bytes ...) line");
    }
    printf("%s: %d failures\n", mode, g_fail);
    return g_fail ? 1 : 0;
}
