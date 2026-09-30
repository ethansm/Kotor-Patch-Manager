#include <windows.h>
#include <stdio.h>
typedef void (__cdecl *HookFn)(void*);
static char g_fakeInternal[0x1000];
__attribute__((noinline)) void stuckSpin(DWORD ms) { volatile unsigned x = 0; DWORD t0 = GetTickCount(); while (GetTickCount() - t0 < ms) ++x; }
int main() {
    CloseHandle(CreateFileA("hangprobe_debug.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0));
    HMODULE m = LoadLibraryA("windows_x86.dll");
    if (!m) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    HookFn hook = (HookFn)GetProcAddress(m, "HangProbeMainLoop");
    printf("hook=%p\n", (void*)hook); fflush(stdout);
    for (int i = 0; i < 180; ++i) { hook(g_fakeInternal); Sleep(16); }        // ~3 s of frames
    CloseHandle(CreateFileA("hangprobe_dump.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0));
    for (int i = 0; i < 120; ++i) { hook(g_fakeInternal); Sleep(16); }        // 2 s, manual report fires
    CopyFileA("hangprobe_report.txt", "report_manual.txt", FALSE);
    stuckSpin(21000);                                                          // no frames: MAIN_STALL after 15 s
    printf("done\n");
    return 0;
}
