// Smoke test for load-hang-probe (originally 1.1.0, kept current for 1.4.1: manual dump report and MAIN_STALL report). Formerly smoke.cpp,
// renamed smoke_probe11.cpp to match smoke_probe12/14 (probe 1.1 = the first version with these two report conditions).
// Build: i686-w64-mingw32-g++ -O1 -static -o smoke_probe11.exe smoke_probe11.cpp   (-I <dir of check.h and wait_line.h>)
//        run dir needs windows_x86.dll (unzip from LoadHangProbe.kpatch). Takes ~27 s.
// Exit code 0 = all checks passed (kt::Report). 1 = LoadLibrary failed, 3 = missing export. Checks:
//   hook export non-null; hangprobe_log.txt has the "load-hang-probe ... loaded" line;
//   manual trigger (touch hangprobe_dump.txt): hangprobe_report.txt gets a "hangprobe report: MANUAL" header (probe samples 2.5 s then reports, so
//     the poll deadline is 6 s, not 3 s) and is copied to report_manual.txt (the old smoke copied it BEFORE it was written, so report_manual.txt
//     never existed and nothing was verified);
//   no frames for 15 s (stuckSpin): hangprobe_report.txt gets a "hangprobe report: MAIN_STALL" header within 25 s of starting the spin
//     (needs one stack to dominate the samples; if that proves flaky it is downgraded to a printed WARN, see below).
// MAIN_STALL flakiness: none seen in 3 consecutive runs under Proton Experimental Wine, so it is a hard check.
#include "check.h"
#include "wait_line.h"
#include <windows.h>
#include <stdio.h>
typedef void (__cdecl *HookFn)(void*);
static char g_fakeInternal[0x1000];
__attribute__((noinline)) void stuckSpin(DWORD ms) { volatile unsigned x = 0; DWORD t0 = GetTickCount(); while (GetTickCount() - t0 < ms) ++x; }
int main() {
    CloseHandle(CreateFileA("hangprobe_debug.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0));
    DeleteFileA("hangprobe_report.txt"); DeleteFileA("report_manual.txt");
    HMODULE m = LoadLibraryA("windows_x86.dll");
    if (!m) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    HookFn hook = (HookFn)GetProcAddress(m, "HangProbeMainLoop");
    printf("hook=%p\n", (void*)hook); fflush(stdout);
    KT_CHECK(hook != nullptr);
    if (!hook) return 3;
    for (int i = 0; i < 180; ++i) { hook(g_fakeInternal); Sleep(16); }        // ~3 s of frames
    KT_CHECK(!waitForLine("hangprobe_log.txt", "load-hang-probe ... loaded", 3000).empty());
    CloseHandle(CreateFileA("hangprobe_dump.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0));
    for (int i = 0; i < 120; ++i) { hook(g_fakeInternal); Sleep(16); }        // 2 s of frames while the probe samples
    bool manual = !waitForLine("hangprobe_report.txt", "hangprobe report: MANUAL", 6000).empty();
    KT_CHECK(manual);
    KT_CHECK(CopyFileA("hangprobe_report.txt", "report_manual.txt", FALSE) != 0);
    KT_CHECK(!waitForLine("report_manual.txt", "hangprobe report: MANUAL", 3000).empty());
    DWORD t0 = GetTickCount();
    stuckSpin(18000);                                                          // no frames: MAIN_STALL after 15 s
    int left = 25000 - (int)(GetTickCount() - t0); if (left < 0) left = 0;
    KT_CHECK(!waitForLine("hangprobe_report.txt", "hangprobe report: MAIN_STALL", left).empty());
    printf("done\n");
    return kt::Report("smoke_probe11");
}
