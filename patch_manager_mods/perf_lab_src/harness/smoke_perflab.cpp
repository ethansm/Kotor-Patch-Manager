// Smoke test for PerfLab 1a: threads hammer HeapAlloc/HeapReAlloc/HeapFree through the exe's own IAT,
// timing before/after the DLL attaches, then checks accounting returns to zero.
// Build: i686-w64-mingw32-g++ -O1 -o smoke_perflab.exe smoke_perflab.cpp   (run dir needs the DLL as perflab.dll)
// Exit code 0 = all checks passed (kt::Report). Checks: balanced run -> live_bytes==0 && live_blocks==0; leak sim -> live_bytes and live_blocks each rise
// by >=100000 / >=1000 relative to a stats() snapshot taken just before the leak loop (delta check, no absolute thresholds); allocs>0; untracked==0 && dropped==0. The timing line is informational only.
// Other exit codes: 1 = LoadLibrary failed (also a failed check), 3 = missing export.
#include "check.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
typedef int (__cdecl *StatsFn)(long long*, int);
typedef void (__cdecl *VoidFn)();
static HANDLE g_h;
__attribute__((noinline)) void* siteA(size_t n) { return HeapAlloc(g_h, 0, n); }
__attribute__((noinline)) void* siteB(size_t n) { return HeapAlloc(g_h, HEAP_ZERO_MEMORY, n); }
__attribute__((noinline)) void* siteC(void* p, size_t n) { return HeapReAlloc(g_h, 0, p, n); }
struct Arg { int seed, iters; };
static DWORD WINAPI worker(LPVOID a) {
    Arg* A = (Arg*)a; unsigned s = A->seed; void* live[512] = {0};
    for (int i = 0; i < A->iters; ++i) {
        s = s * 1664525u + 1013904223u; int k = (s >> 8) & 511; size_t n = 8 + ((s >> 4) % 4096);
        if ((s >> 20) % 900 == 0) n = 300000 + (s % 500000);          // large blocks
        if (live[k]) {
            if ((s & 3) == 0) { void* q = siteC(live[k], n); if (q) live[k] = q; }
            else { HeapFree(g_h, 0, live[k]); live[k] = 0; }
        } else live[k] = (s & 1) ? siteA(n) : siteB(n);
    }
    for (int k = 0; k < 512; ++k) if (live[k]) HeapFree(g_h, 0, live[k]);
    return 0;
}
static double run(int threads, int iters) {
    LARGE_INTEGER f, a, b; QueryPerformanceFrequency(&f); QueryPerformanceCounter(&a);
    HANDLE th[8]; Arg args[8];
    for (int i = 0; i < threads; ++i) { args[i] = { i * 7919 + 1, iters }; th[i] = CreateThread(0, 0, worker, &args[i], 0, 0); }
    WaitForMultipleObjects(threads, th, TRUE, INFINITE);
    QueryPerformanceCounter(&b); return double(b.QuadPart - a.QuadPart) / f.QuadPart;
}
int main() {
    g_h = HeapCreate(0, 0x1000, 0);
    FILE* f = fopen("perflab_debug.txt", "w"); fprintf(f, "heapfilter=0\nflush_ms=1000\n"); fclose(f);
    const int T = 1, N = 1500000;
    run(T, N); double base = run(T, N); double base2 = run(T, N); base = base < base2 ? base : base2;
    HMODULE m = LoadLibraryA("perflab.dll");
    if (!m) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    StatsFn stats = (StatsFn)GetProcAddress(m, "PerfLabStats"); VoidFn snap = (VoidFn)GetProcAddress(m, "PerfLabSnapshot");
    if (!stats || !snap) { printf("missing export: stats=%p snap=%p\n", (void*)stats, (void*)snap); return 3; }
    run(T, N); double hooked = run(T, N); double h2 = run(T, N); hooked = hooked < h2 ? hooked : h2;
    long long st[6] = {0}; stats(st, 6);
    printf("no hook: %.3fs  hooked: %.3fs  (%.0f ns/op extra over %d ops)\n", base, hooked, (hooked - base) * 1e9 / (double(T) * N), T * N);
    printf("live_bytes=%lld live_blocks=%lld allocs=%lld frees=%lld untracked=%lld dropped=%lld\n", st[0], st[1], st[2], st[3], st[4], st[5]);
    KT_CHECK_EQ(st[0], 0);          // balanced run: every tracked byte freed
    KT_CHECK_EQ(st[1], 0);
    KT_CHECK(st[2] > 0);            // allocs observed at all
    KT_CHECK_EQ(st[4], 0);          // untracked
    KT_CHECK_EQ(st[5], 0);          // dropped
    // leak simulation: 1000 x 100 bytes at siteA must show as live at that site (asserted as a delta over the pre-leak snapshot)
    long long pre[6] = {0}; stats(pre, 6);
    for (int i = 0; i < 1000; ++i) siteA(100);
    snap(); Sleep(1500);
    stats(st, 6); printf("after leak sim: live_bytes=%lld (+%lld, expect >=100000) blocks=%lld (+%lld, expect >=1000)\n", st[0], st[0] - pre[0], st[1], st[1] - pre[1]);
    KT_CHECK(st[0] - pre[0] >= 100000);
    KT_CHECK(st[1] - pre[1] >= 1000);
    Sleep(1200);
    return kt::Report("smoke_perflab");
}
