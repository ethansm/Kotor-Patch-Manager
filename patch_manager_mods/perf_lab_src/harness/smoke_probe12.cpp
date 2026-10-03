// Smoke test for load-hang-probe 1.2.0: SetPlayerId ring, IAT file-I/O ring, report sections.
// Build: i686-w64-mingw32-g++ -O1 -static -o smoke_probe12.exe smoke_probe12.cpp ; run dir needs windows_x86.dll (unzip from LoadHangProbe.kpatch)
// Exit code 0 = all checks passed (kt::Report). Checks: both exports non-null; the two failing CreateFileA calls return INVALID_HANDLE_VALUE;
// hangprobe_log.txt gets (polled up to 5 s each, see wait_line.h) the SETPLAYER lines for ret=0078237c -> 7f000000 and ret=007b3107 -> fffffffd
// (no "(?)" in them) and the "IAT file I/O hooks: ... capture ON" line. Exit 1 = LoadLibrary failed, 3 = missing export.
#include "check.h"
#include "wait_line.h"
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
    KT_CHECK(loop != nullptr); KT_CHECK(setp != nullptr);
    if (!loop || !setp) return 3;
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
    KT_CHECK(bad == INVALID_HANDLE_VALUE); KT_CHECK(bad2 == INVALID_HANDLE_VALUE);
    Sleep(600);
    CloseHandle(CreateFileA("hangprobe_dump.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0));
    for (int i = 0; i < 200; ++i) { loop(g_fake); Sleep(16); }
    // The probe logs from its own thread: poll for each line instead of assuming it is already flushed.
    const char* log = "hangprobe_log.txt";
    std::string l1 = waitForLine(log, "SETPLAYER ... ret=0078237c ... -> 7f000000", 5000);
    std::string l2 = waitForLine(log, "SETPLAYER ... ret=007b3107 ... -> fffffffd", 5000);
    std::string l3 = waitForLine(log, "IAT file I/O hooks: ... capture ON", 5000);
    KT_CHECK(!l1.empty()); KT_CHECK(!l2.empty()); KT_CHECK(!l3.empty());
    KT_CHECK(l1.find("(?)") == std::string::npos); KT_CHECK(l2.find("(?)") == std::string::npos);
    if (l1.empty()) printf("missing: SETPLAYER ret=0078237c\n");
    if (l2.empty()) printf("missing: SETPLAYER ret=007b3107\n");
    if (l3.empty()) printf("missing: IAT file I/O hooks line\n");
    printf("done\n"); return kt::Report("smoke_probe12");
}
