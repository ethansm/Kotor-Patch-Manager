// Smoke test for load-hang-probe 1.2.0: SetPlayerId ring, IAT file-I/O ring, report sections.
// Build: i686-w64-mingw32-g++ -O1 -static -o smoke_probe12.exe smoke_probe12.cpp ; run dir needs windows_x86.dll (unzip from LoadHangProbe.kpatch)
#include <windows.h>
#include <stdio.h>
typedef void (__cdecl *MainFn)(void*);
typedef void (__cdecl *SetFn)(void*, void*, int);
static char g_fake[0x1000];
int main() {
    CloseHandle(CreateFileA("hangprobe_debug.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0));
    HMODULE m = LoadLibraryA("windows_x86.dll");
    if (!m) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    MainFn loop = (MainFn)GetProcAddress(m, "HangProbeMainLoop"); SetFn setp = (SetFn)GetProcAddress(m, "HangProbeSetPlayerId");
    printf("exports: %p %p\n", (void*)loop, (void*)setp);
    for (int i = 0; i < 30; ++i) { loop(g_fake); Sleep(16); }
    setp(g_fake, (void*)0x0078237C, 0x7f000000); setp(g_fake, (void*)0x007B3107, (int)0xfffffffd);
    // file I/O through this exe's own imports
    HANDLE h = CreateFileA("io_test.bin", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0); char buf[4096] = {1}; DWORD w; for (int i = 0; i < 8; ++i) WriteFile(h, buf, sizeof buf, &w, 0); CloseHandle(h);
    h = CreateFileA("io_test.bin", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    DWORD got; for (int i = 0; i < 5; ++i) ReadFile(h, buf, 4096, &got, 0);
    SetFilePointer(h, 100, 0, FILE_BEGIN); ReadFile(h, buf, 4096, &got, 0);  // read 4096 of remaining 32768-100: full
    SetFilePointer(h, -10, 0, FILE_END); ReadFile(h, buf, 4096, &got, 0);    // short read (10 bytes)
    CloseHandle(h);
    HANDLE bad = CreateFileA("does_not_exist.bin", GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);        // FNF: counted only
    HANDLE bad2 = CreateFileA(".", GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);                         // other failure (dir)
    printf("bad=%p bad2=%p err=%lu\n", bad, bad2, GetLastError());
    Sleep(600);
    CloseHandle(CreateFileA("hangprobe_dump.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0));
    for (int i = 0; i < 200; ++i) { loop(g_fake); Sleep(16); }
    printf("done\n"); return 0;
}
