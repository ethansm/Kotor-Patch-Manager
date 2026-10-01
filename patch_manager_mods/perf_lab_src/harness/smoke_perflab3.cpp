// Smoke test for PerfLab 0.3.0: all 0.2.0 checks (M1-M8 handlers, tx/g1skip byte experiments, VQ census, 0.1.1 census regression, hook cost)
// plus the 0.3.0 groups: LIVE globals, BR name tables + ring, KT/texture lifetime, NWScript VM, net ring/scene counters, per-handler HOOKCOST_BY,
// UNLOAD snapshot, experiment bytes logged when off, and mode `nogroups` (metrics=1 with live/br/kt/vm/misc=0 -> no 0.3.0 lines).
// Build: i686-w64-mingw32-g++ -O1 -o smoke_perflab3.exe smoke_perflab3.cpp -lgdi32 -lglu32
//   (dynamic CRT, default base 0x400000: the 7 MB .bss pad makes the image cover 0x400000-0xB00000 so the engine addresses
//    0x4261D4, 0x4265F3, 0xA1B490 ... are ordinary memory of this exe; same trick as smoke_probe13.cpp)
// Run:   smoke_perflab3.exe <run|refuse|off|nogroups>   (dir needs the DLL from PerfLab.kpatch as perflab.dll). Exit code 0 = all checks passed.
//   run    = tx=1 g1skip=1 with the right original bytes -> both patched (TX cave executed on fake textures); all counter/format/census/cost checks
//   refuse = tx=1 g1skip=1 with wrong bytes at the sites -> both refused, bytes untouched
//   off    = no tx/g1skip keys -> bytes untouched, arm logged OFF
#include <windows.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>

static char pad[0x700000];   // covers the engine's code/data addresses used below
#define G(a) (*(volatile uint32_t*)(a))
typedef void (__cdecl* V0)();
typedef void (__cdecl* V1)(void*);
typedef void (__cdecl* V2)(void*, void*);
typedef void (__cdecl* VSpin)(int, void*);
typedef int (__cdecl* StatsFn)(long long*, int);
static HMODULE g_dll;
template <typename T> T fn(const char* n) { T f = (T)GetProcAddress(g_dll, n); if (!f) { printf("missing export %s\n", n); exit(3); } return f; }

static int g_fail = 0;
static void check(bool ok, const char* what) { printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++g_fail; }

// Fake CRes (R1 layout): +4 u16 refcount, +8 source (>>30), +0xC flags, +0x10 data, +0x14 key entry (restype u16 at +0x1A), +0x18 size.
struct FakeRes { char b[0x40]; char key[0x20]; };
static void mkRes(FakeRes& r, int restype, int size, uint16_t ref, bool hasKey = true) {
    memset(&r, 0, sizeof r);
    *(uint16_t*)(r.b + 4) = ref; *(void**)(r.b + 0x10) = r.b;   // any non-NULL data pointer
    *(int*)(r.b + 0x18) = size;
    if (hasKey) { *(uint16_t*)(r.key + 0x1A) = (uint16_t)restype; *(void**)(r.b + 0x14) = r.key; }
}

static std::string lastBlock() {   // last #METRICS ... #END block of perflab_metrics.txt
    FILE* f = fopen("perflab_metrics.txt", "r"); if (!f) return "";
    std::string all; char buf[4096]; size_t n; while ((n = fread(buf, 1, sizeof buf, f)) > 0) all.append(buf, n); fclose(f);
    size_t p = all.rfind("#METRICS "); return p == std::string::npos ? "" : all.substr(p);
}
static std::string line(const std::string& blk, const char* prefix) {
    size_t p = blk.find(std::string("\n") + prefix); if (p == std::string::npos) return "";
    size_t e = blk.find('\n', p + 1); return blk.substr(p + 1, e - p - 1);
}
static long long field(const std::string& l, const char* key) {   // "key=123"
    size_t p = l.find(std::string(" ") + key + "="); if (p == std::string::npos) return -999999;
    return atoll(l.c_str() + p + strlen(key) + 2);
}
static std::string fileText(const char* name) {
    FILE* f = fopen(name, "r"); if (!f) return ""; std::string s; char b[4096]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) s.append(b, n); fclose(f); return s;
}
static void waitMetrics(int want) {   // wait until the helper has written block number `want`
    for (int i = 0; i < 60; ++i) { std::string b = lastBlock(); if (!b.empty() && atoi(b.c_str() + 9) >= want) return; Sleep(100); }
}
static double nsPerCall(void (*body)(int), int n) {
    LARGE_INTEGER f, a, b; QueryPerformanceFrequency(&f); QueryPerformanceCounter(&a); body(n); QueryPerformanceCounter(&b);
    return double(b.QuadPart - a.QuadPart) * 1e9 / f.QuadPart / n;
}

// census storm (0.1.1 regression): HeapAlloc/ReAlloc/Free through this exe's IAT
static HANDLE g_h;
__attribute__((noinline)) void* siteA(size_t n) { return HeapAlloc(g_h, 0, n); }
__attribute__((noinline)) void* siteC(void* p, size_t n) { return HeapReAlloc(g_h, 0, p, n); }
static DWORD WINAPI storm(LPVOID a) {
    unsigned s = (unsigned)(uintptr_t)a; void* live[512] = {0};
    for (int i = 0; i < 200000; ++i) {
        s = s * 1664525u + 1013904223u; int k = (s >> 8) & 511; size_t n = 8 + ((s >> 4) % 4096);
        if ((s >> 20) % 900 == 0) n = 300000 + (s % 500000);
        if (live[k]) { if ((s & 3) == 0) { void* q = siteC(live[k], n); if (q) live[k] = q; } else { HeapFree(g_h, 0, live[k]); live[k] = 0; } }
        else live[k] = siteA(n);
    }
    for (int k = 0; k < 512; ++k) if (live[k]) HeapFree(g_h, 0, live[k]);
    return 0;
}

static FakeRes g_cost; static V1 g_demand; static V0 g_bucket; static VSpin g_spin; static V2 g_getimg; static char g_tex[0x100];
static void costDemand(int n) { for (int i = 0; i < n; ++i) g_demand(g_cost.b); }
static void costBucket(int n) { for (int i = 0; i < n; ++i) g_bucket(); }
static void costSpin(int n) { for (int i = 0; i < n; ++i) g_spin(0, nullptr); }
static void costGetImg(int n) { for (int i = 0; i < n; ++i) g_getimg(g_tex, (void*)0x401234); }

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "run";
    bool nogroups = !strcmp(mode, "nogroups"), run = !strcmp(mode, "run") || nogroups, refuse = !strcmp(mode, "refuse");
    if ((uintptr_t)pad > 0x420000 || (uintptr_t)pad + sizeof pad < 0xA82000) { printf("pad %p does not cover 0x420000-0xA82000\n", (void*)pad); return 2; }
    FILE* cf = fopen("perflab_debug.txt", "w");
    fprintf(cf, "alloc=1\nheapfilter=0\nflush_ms=60000\nmetrics=1\nvq=1\n");
    if (run || refuse) fprintf(cf, "tx=1\ng1skip=1\n");
    if (nogroups) fprintf(cf, "live=0\nbr=0\nkt=0\nvm=0\nmisc=0\n");
    fclose(cf);
    DeleteFileA("perflab_metrics.txt"); DeleteFileA("perflab_log.txt"); DeleteFileA("perflab_alloc_snaps.txt");
    // experiment sites. TX: an executable stand-in for BuildAndStoreTexture's tail (thiscall tex, isTpc): entry 0x4261C0 saves ECX at
    // [EBP-4] and loads EAX = isTpc (as IsTPCLoaded would), then the real 7 bytes at 0x4261CF; the FreeImage path 0x4261D6 returns 0xF4EED,
    // the skip path returns isTpc, epilogue at 0x4261E6 (ret 4).
    { DWORD o; if (!VirtualProtect((void*)0x426000, 0x1000, PAGE_EXECUTE_READWRITE, &o)) { printf("VirtualProtect failed\n"); return 2; } }
    static const uint8_t stub[] = { 0x55, 0x8B, 0xEC, 0x51, 0x89, 0x4D, 0xFC, 0x8B, 0x45, 0x08, 0x90, 0x90, 0x90, 0x90, 0x90,
                                    0x0F, 0xB6, 0xC8, 0x85, 0xC9, 0x75, 0x10,
                                    0xB8, 0xED, 0x4E, 0x0F, 0x00, 0xEB, 0x09, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                                    0x8B, 0xE5, 0x5D, 0xC2, 0x04, 0x00 };
    memcpy((void*)0x4261C0, stub, sizeof stub);
    uint8_t* tx = (uint8_t*)0x4261CF; uint8_t* g1 = (uint8_t*)0x4265F3;
    if (refuse) { tx[5] = 0x74; g1[0] = 0x75; g1[1] = 0x06; } else { g1[0] = 0x74; g1[1] = 0x06; }
    uint8_t tx0[7], g10[2] = { g1[0], g1[1] }; memcpy(tx0, tx, 7);
    typedef int (__thiscall* BuildFn)(void*, int);
    BuildFn build = (BuildFn)0x4261C0;
    static char texPlain[0x100], texCtrl[0x100]; *(void**)(texCtrl + 0x34) = texCtrl;   // any non-NULL controller
    auto txFreed = [&](char* t, int tpc) { return build(t, tpc) == 0xF4EED; };
    // engine globals (M7) + fake app chain for load-screen edge detection
    static char resman[0x40], app[16], client[16], internal[0x800], ls[64], mgr[0x200]; static void* modals[2];
    *(int*)(resman + 4) = 16 << 20; *(int*)(resman + 8) = 12345678;
    G(0xA1B490) = (uint32_t)(uintptr_t)resman; G(0x9F6100) = 1; G(0xA1B704) = 0; G(0xA7EB9C) = 0; *(float*)0xA1B710 = 0.0f;
    *(unsigned*)ls = 0x009A5C84; modals[0] = ls; *(void**)(internal + 0x278) = ls; *(void**)(internal + 0x274) = mgr;
    *(void**)(mgr + 0x94) = modals; *(int*)(mgr + 0x98) = 0;
    *(void**)(app + 4) = client; *(void**)(client + 4) = internal; G(0xA1B4A4) = (uint32_t)(uintptr_t)app;

    g_dll = LoadLibraryA("perflab.dll"); if (!g_dll) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    V0 tick = fn<V0>("PerfLabTick");
    tick();   // first tick applies the experiments
    if (run) check(tx[0] == 0xE9 && tx[5] == 0x90 && tx[6] == 0x90 && g1[0] == 0xEB && g1[1] == 0x06, "tx cave jump + g1skip patched the right original bytes");
    else check(!memcmp(tx, tx0, 7) && g1[0] == g10[0] && g1[1] == g10[1], refuse ? "mismatched bytes refused (unchanged)" : "experiments off: bytes unchanged");
    if (run) check(txFreed(texPlain, 0) && txFreed(texPlain, 1) && !txFreed(texCtrl, 1) && txFreed(texCtrl, 0),
                   "TX cave: TGA freed, TPC freed, TPC with controller kept, TGA with controller freed");
    else if (!refuse) check(txFreed(texPlain, 0) && !txFreed(texPlain, 1) && !txFreed(texCtrl, 1) && txFreed(texCtrl, 0), "engine behaviour: only TGA freed");
    Sleep(400);
    std::string log = fileText("perflab_log.txt");
    if (run) check(log.find("tx=ON") != std::string::npos && log.find("g1skip=ON") != std::string::npos, "log records tx=ON g1skip=ON");
    if (refuse) check(log.find("tx=REFUSED(bytes)") != std::string::npos && log.find("g1skip=REFUSED(bytes)") != std::string::npos, "log records REFUSED(bytes)");
    if (!run && !refuse) check(log.find("tx=OFF") != std::string::npos && log.find("g1skip=OFF") != std::string::npos, "log records tx=OFF g1skip=OFF");
    if (!run && !refuse) check(log.find("0x4261CF saw 0f b6 c8 85 c9 75 10") != std::string::npos && log.find("0x4265F3 saw 74 06") != std::string::npos, "0.3.0: site bytes logged with the experiments off");
    check(log.find("perf-lab 0.3.0 attach") != std::string::npos, "0.3.0 version in the attach line");
    if (!run) { printf("%s: %d failures\n", mode, g_fail); return g_fail ? 1 : 0; }

    // ---- M1 ----
    V2 malloc_ = fn<V2>("PerfLabResMalloc"), freed = fn<V2>("PerfLabResFreeData");
    V1 evict = fn<V1>("PerfLabResEvict"), release = fn<V1>("PerfLabResRelease"), demand = fn<V1>("PerfLabResDemand");
    static FakeRes A, B, C, D, E;
    mkRes(A, 3007, 2000000, 1); mkRes(B, 2002, 100000, 2); mkRes(C, 0, 50, 1, false); mkRes(D, 3007, 20 << 20, 0); mkRes(E, 2002, 777, 1);
    *(uint32_t*)(E.b + 8) = 0x40000000;   // source >>30 == 1: engine does not free, must not be counted
    malloc_(resman, A.b); malloc_(resman, B.b); malloc_(resman, C.b); malloc_(resman, D.b);
    demand(A.b); demand(A.b); demand(A.b); demand(B.b);
    release(A.b); release(B.b);
    evict(B.b); freed(B.b, (void*)0x00712245);
    demand(B.b);   // re-demand after eviction
    freed(E.b, (void*)0x00401234);
    // ---- M2 ----
    V0 wb = fn<V0>("PerfLabWaitBegin"), ws = fn<V0>("PerfLabWaitSleep"), we = fn<V0>("PerfLabWaitEnd");
    wb(); for (int i = 0; i < 5; ++i) { ws(); Sleep(2); } we();
    wb();   // in-flight without a wait (resman+0x2C == 0): begin only
    // ---- M4 ----
    V0 gfp = fn<V0>("PerfLabGlFinishPre"), gip = fn<V0>("PerfLabGlImagePost");
    gfp(); Sleep(3); gip(); gip();
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);   // width 0: GLU returns an error, hook still counts
    // ---- M5 ----
    V0 bucket = fn<V0>("PerfLabBucketInsert"), abar = fn<V0>("PerfLabActionBar");
    for (int f = 0; f < 5; ++f) { for (int i = 0; i < 10 + f; ++i) bucket(); abar(); SwapBuffers(nullptr); Sleep(20); }
    // ---- M6 ----
    V0 me = fn<V0>("PerfLabMovieEnter"); VSpin spin = fn<VSpin>("PerfLabMovieSpin");
    static char movieThis[0x300]; *(int*)(movieThis + 0x254) = 1;
    me(); for (int i = 0; i < 600; ++i) spin(0, movieThis); spin(1, movieThis); me();
    // ---- M3 (body per batch T1) ----
    V2 getimg = fn<V2>("PerfLabGetImage");
    static char tpcRet[0x100], tga[0x100];   // TPC, built, GL name, buffer retained / a TGA texture
    tpcRet[0xE9] = 1; tpcRet[0xE4] = 1; *(int*)(tpcRet + 0x58) = 5; *(void**)(tpcRet + 0x44) = tpcRet;
    getimg(tpcRet, (void*)0x433400); getimg(tpcRet, (void*)0x48FF50); getimg(tpcRet, (void*)0x401234); getimg(tga, (void*)0x401234);
    V1 buildHook = fn<V1>("PerfLabBuildTex");
    static char tpcCtrl[0x100]; memcpy(tpcCtrl, tpcRet, sizeof tpcCtrl); *(void**)(tpcCtrl + 0x34) = tpcCtrl;
    buildHook(tpcRet); buildHook(tpcCtrl); buildHook(tga);

    // edges: load screen UP then DOWN (helper polls every 100 ms)
    *(int*)(mgr + 0x98) = 1; Sleep(400); *(int*)(mgr + 0x98) = 0; Sleep(400);
    fn<V0>("PerfLabMetricsNow")(); waitMetrics(3);
    std::string all = fileText("perflab_metrics.txt"), blk = lastBlock();
    check(all.find(" LOAD_UP ") != std::string::npos && all.find(" LOAD_DOWN ") != std::string::npos, "metrics snapshots at load-screen UP and DOWN edges");
    check(blk.find(" MANUAL ") != std::string::npos, "manual metrics snapshot");
    // RT restype allocN allocB evictN evictB freeN freeB relN relZero demN redem60 resident
    long long rt[12];
    auto rtline = [&](const char* t) { std::string l = line(blk, t); memset(rt, 0, sizeof rt);
        return sscanf(l.c_str(), "RT %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld", &rt[0], &rt[1], &rt[2], &rt[3], &rt[4], &rt[5], &rt[6], &rt[7], &rt[8], &rt[9], &rt[10], &rt[11]) == 12; };
    check(rtline("RT 3007 ") && rt[1] == 2 && rt[2] == 2000000 + (20 << 20) && rt[7] == 1 && rt[8] == 1 && rt[9] == 3 && rt[11] == 2000000 + (20 << 20), "M1 TPC(3007): 2 allocs, bytes, 1 release to zero, 3 demands, resident");
    check(rtline("RT 2002 ") && rt[1] == 1 && rt[2] == 100000 && rt[3] == 1 && rt[4] == 100000 && rt[5] == 1 && rt[6] == 100000 && rt[8] == 0 && rt[9] == 2 && rt[10] == 1 && rt[11] == 0,
          "M1 MDL(2002): evict + free counted, source-1 free skipped, re-demand after eviction, resident 0");
    check(rtline("RT -1 ") && rt[1] == 1 && rt[2] == 50, "M1 CRes without key entry -> restype -1");
    std::string res = line(blk, "RES ");
    check(field(res, "malloc") == 4 && field(res, "over_budget") == 1 && field(res, "budget_min") == 12345678 && field(res, "redemand_60s") == 1, "M1 RES totals, over-budget Malloc, budget min, re-demand");
    check(line(blk, "FREECALLER 00712245 1").size() > 0, "M1 FreeResourceData caller attribution");
    std::string w = line(blk, "WAIT ");
    check(field(w, "begin") == 2 && field(w, "end") == 1 && field(w, "sleeps") == 5 && w.find("hist 0 0 0 1 ") != std::string::npos && field(w, "total_ms") >= 5, "M2 wait: begin/end/sleeps/hist/time");
    std::string gl = line(blk, "GL ");
    check(field(gl, "glimage") == 2 && field(gl, "glfinish") == 1 && field(gl, "glfinish_ms") >= 2 && field(gl, "mipmaps") == 1, "M4 glImage/glFinish/gluBuild2DMipmaps");
    std::string fr = line(blk, "FRAME ");
    long long h[10] = {0}; size_t hp = fr.find("hist_ms ");
    if (hp != std::string::npos) sscanf(fr.c_str() + hp + 8, "%lld %lld %lld %lld %lld %lld %lld %lld %lld %lld", &h[0], &h[1], &h[2], &h[3], &h[4], &h[5], &h[6], &h[7], &h[8], &h[9]);
    check(field(fr, "frames") == 5 && h[5] == 4 && field(fr, "bucket_calls") == 60 && field(fr, "actionbar_calls") == 5 && field(fr, "max_bucket_per_frame") == 14 && field(fr, "max_actionbar_per_frame") == 1,
          "M5 frames, frame-time histogram (4 intervals in 16-31 ms), per-frame maxima");
    std::string mv = line(blk, "MOVIE ");
    check(field(mv, "calls") == 2 && field(mv, "spins") == 601 && field(mv, "pump_cutoff_seen") == 1 && field(mv, "last_this254") == 1 && mv.find("spin_hist 0 0 0 0 0 0 0 0 0 0 1 ") != std::string::npos,
          "M6 movie calls/spins/cut-off/this+0x254/spin histogram");
    std::string g = line(blk, "G ");
    check(g.find("budget8=12345678(1)") != std::string::npos && g.find("9f6100=1(1)") != std::string::npos && g.find("a1b710=0(1)") != std::string::npos, "M7 engine globals");
    check(blk.find("\nVQ regions commit=") != std::string::npos && blk.find("\nVQ commit MB image=") != std::string::npos && blk.find("\nVQ free MB") != std::string::npos &&
          blk.find("\nVQTOP ") != std::string::npos && blk.find("largest_free=") != std::string::npos, "M8 VQ census format");
    std::string gi = line(blk, "GETIMG ");
    check(field(gi, "calls") == 4 && field(gi, "tpc") == 3 && field(gi, "tpc_retained") == 3 && field(gi, "upload") == 1 && field(gi, "ctrl") == 1 &&
          field(gi, "ctrl_tpc_retained") == 1 && field(gi, "other") == 2 && field(gi, "other_tpc_retained") == 1 && line(blk, "GETIMGCALLER 00401234 2").size() > 0,
          "M3 GetImage classes (upload/controller/other) + retained-TPC marker + caller table");
    std::string bl = line(blk, "BUILD ");
    check(field(bl, "calls") == 3 && field(bl, "requeue_tpc_retained") == 2 && field(bl, "tpc_with_ctrl") == 1, "C1/C2 BuildAndStoreTexture requeue + controller counters");

    // ---- 0.3.0 groups ----
    G(0xA1BBE0) = 11; G(0xA1BBA0) = 2222; G(0xA1FFAC) = 33; G(0xA1DEE0) = 4444; G(0xA1FF68) = 55; G(0xA1FF60) = 66; G(0xA200F4) = 7; G(0xA20030) = 8; G(0xA7BD28) = 9;
    // BR: TGA load by key-entry resref (16 bytes, not NUL-terminated), build by texture name (+0x7C), GUI setters (CResRef*), texture miss (char*)
    static FakeRes T1, T2; mkRes(T1, 3, 360000, 1); mkRes(T2, 3, 360000, 1);
    memcpy(T1.key, "LBL_MAP749KOR\0\0\0", 16); memcpy(T2.key, "ABCDEFGHIJKLMNOP", 16);
    malloc_(resman, T1.b); malloc_(resman, T1.b); malloc_(resman, T2.b);
    static char texN[0x100]; strcpy(texN + 0x7C, "per_cpanel"); buildHook(texN); buildHook(texN);
    static char rr[16]; memcpy(rr, "MAPTEX0123456789", 16);
    fn<V2>("PerfLabBrSetter1")(rr, (void*)0x750CEC); fn<V2>("PerfLabBrSetter2")(rr, (void*)0x416123); fn<V2>("PerfLabBrTexMiss")((void*)"fx_beam01", (void*)0x4274F0);
    // KT / textures
    static FakeRes AR; mkRes(AR, 3007, 300, 1); static char rm2[0x40]; *(void**)(rm2 + 0x28) = AR.b;
    fn<V1>("PerfLabAsyncStart")(rm2); fn<V0>("PerfLabDestroyTable")(); fn<V1>("PerfLabStillInDemand")(AR.b);
    static FakeRes U1r, U2r; mkRes(U1r, 3007, 10, 1); mkRes(U2r, 3007, 10, 1); *(uint32_t*)(U1r.b + 0xC) = 0x204;
    fn<V1>("PerfLabUafWrite")(U1r.b); fn<V1>("PerfLabUafWrite")(U2r.b);
    fn<V1>("PerfLabTxiTpc")((void*)"tex_a"); fn<V1>("PerfLabTxiTpc")((void*)"TEX_A");
    typedef void (__cdecl* V3)(void*, void*, void*);
    static char rq[0x100]; rq[0xE4] = 1;
    fn<V3>("PerfLabRequeue")((void*)0xA1BC2C, rq, (void*)0x422E77); fn<V3>("PerfLabRequeue")((void*)0xA1BC2C, tga, (void*)0x4269DE); fn<V3>("PerfLabRequeue")((void*)0x123456, rq, (void*)0x1);
    V1 freeImg = fn<V1>("PerfLabFreeImage"); static char tpcNoBuf[0x100]; tpcNoBuf[0xE9] = 1; static char tgaBuf[0x100]; *(void**)(tgaBuf + 0x44) = tgaBuf;
    freeImg(tpcRet); freeImg(tpcNoBuf); freeImg(tgaBuf);
    fn<V0>("PerfLabTexDtor")();
    // VM
    struct ExoStr { const char* t; int n; } es = { "k_def_heartbeat", 15 };
    V1 runScript = fn<V1>("PerfLabRunScript"); for (int i = 0; i < 3; ++i) runScript(&es);
    typedef void (__cdecl* VI)(int); VI exec = fn<VI>("PerfLabExecCmd"); exec(5); exec(5); exec(9); exec(-3); exec(0x36D);
    fn<V0>("PerfLabInstrLimit")(); fn<V0>("PerfLabDelayCommand")(); fn<V0>("PerfLabAdcEnter")(); fn<V0>("PerfLabAdcEnter")(); fn<V0>("PerfLabAdcHandoff")();
    // net / scenes
    VI netSend = fn<VI>("PerfLabNetSend"); netSend(100); netSend(70000);
    char* ring = (char*)VirtualAlloc(0, 0x21000, MEM_COMMIT, PAGE_READWRITE); char* peer = (char*)VirtualAlloc(0, 0x21000, MEM_COMMIT, PAGE_READWRITE);   // not .bss: keeps pad below 0x420000
    *(void**)(ring + 0x2000C) = peer; *(unsigned*)(peer + 0x20000) = 10;
    typedef void (__cdecl* VOvf)(void*, unsigned); VOvf ovf = fn<VOvf>("PerfLabNetOvfTest"); ovf(ring, 0x20000); ovf(ring, 20);
    fn<V0>("PerfLabFocusDummy")(); fn<V0>("PerfLabSceneDtor")();
    // AI / pathfinding / effects (U3): AI master with level counts 3,0,0,0,2 ; two object updates of ~5 ms each; next tick closes the estimate
    static char aim[0x60]; *(int*)(aim + 8) = 3; *(int*)(aim + 8 + 64) = 2;
    V1 aiTick = fn<V1>("PerfLabAiTick"); typedef void (__cdecl* VPI)(void*, int); VPI aiPre = fn<VPI>("PerfLabAiObjPre"); V0 aiPost = fn<V0>("PerfLabAiObjPost");
    static char obj1[16];
    aiTick(aim); aiPre(obj1, 0); Sleep(5); aiPost(); aiPre(obj1, 4); Sleep(5); aiPost(); aiPost(); aiTick(aim);
    V0 pathEnter = fn<V0>("PerfLabPathEnter"); VI pathRes = fn<VI>("PerfLabPathResult");
    pathEnter(); Sleep(2); pathRes(1); pathEnter(); pathRes(2); pathEnter(); pathRes(3);
    fn<V0>("PerfLabEffectApply")(); fn<V0>("PerfLabEffectApply")(); fn<V0>("PerfLabEffectRemove")();
    // UNLOAD snapshot
    fn<V0>("PerfLabUnload")();
    for (int i = 0; i < 30 && fileText("perflab_metrics.txt").find(" UNLOAD ") == std::string::npos; ++i) Sleep(100);
    check(fileText("perflab_metrics.txt").find(" UNLOAD ") != std::string::npos || nogroups, "UnloadModule hook writes an UNLOAD metrics snapshot");
    fn<V0>("PerfLabMetricsNow")(); Sleep(600);
    std::string b3 = lastBlock();
    if (nogroups) {
        check(b3.find("\nLIVE") == std::string::npos && b3.find("\nNAMES") == std::string::npos && b3.find("\nKT ") == std::string::npos &&
              b3.find("\nVM ") == std::string::npos && b3.find("\nNET ") == std::string::npos && b3.find("\nGETIMG ") != std::string::npos,
              "nogroups: no 0.3.0 group lines, 0.2.0 lines still present");
        printf("%s: %d failures\n", mode, g_fail); return g_fail ? 1 : 0;
    }
    check(line(b3, "LIVE").find("textures=11 gl_bytes_est=2222 models=33 mesh_node_bytes=4444 gob_instances=55 node_objects=66 scenes=7 focus_dummies=8 emitters=9") != std::string::npos, "LIVE engine counters");
    check(line(b3, "NAME tga_load 2 lbl_map749kor ").size() && line(b3, "NAME tga_load 1 abcdefghijklmnop ").size(), "BR TGA loads by resref (lower-cased, 16-char unterminated)");
    check(line(b3, "NAME build 2 per_cpanel ").size(), "BR builds by texture name (+0x7C)");
    check(line(b3, "NAME gui_setter1 1 maptex0123456789 ra=00750cec").size() && line(b3, "NAME gui_setter2 1 maptex0123456789 ra=00416123").size() &&
          line(b3, "NAME tex_miss 1 fx_beam01 ra=004274f0").size(), "BR GUI setters + texture miss with caller");
    check(b3.find("BRRING ") != std::string::npos && b3.find(" tex_miss fx_beam01 ra=004274f0") != std::string::npos, "BR event ring");
    std::string kt = line(b3, "KT ");
    check(field(kt, "async_start") == 1 && field(kt, "async_bytes") == 300 && field(kt, "destroy_table") == 1 && field(kt, "still_in_demand") == 1 &&
          field(kt, "uaf_write") == 1 && field(kt, "release_ok") == 1 && field(kt, "txi_via_tpc") == 2 && field(kt, "unload") == 1, "KT counters (async, DestroyTable, still-in-demand, UAF write vs normal, TXI via TPC, unload)");
    check(line(b3, "NAME txi_via_tpc 2 tex_a ").size(), "TXI-via-TPC names");
    std::string tl = line(b3, "TEXLIFE ");
    check(field(tl, "requeue") == 2 && field(tl, "requeue_built") == 1 && field(tl, "freeimage") == 3 && field(tl, "freeimage_tpc") == 2 && field(tl, "freeimage_tpc_buf") == 1 &&
          field(tl, "freeimage_tga_buf") == 1 && field(tl, "tex_dtor") == 1 && line(b3, "REQUEUECALLER 00422e77 1").size(), "texture lifetime (requeue list filter, built, FreeImage split C3, dtor, requeue callers)");
    std::string vm = line(b3, "VM ");
    check(field(vm, "run_script") == 3 && field(vm, "exec_cmd") == 5 && field(vm, "exec_bad_id") == 2 && field(vm, "instr_limit") == 1 && field(vm, "delay_cmd") == 1 &&
          field(vm, "adc_enter") == 2 && field(vm, "adc_handoff") == 1 && field(vm, "adc_leak_upper") == 1, "VM counters (runs, commands, bad ids, instruction limit, delay, ActionDoCommand leak bound)");
    check(line(b3, "VMCMD 5:2 9:1").size() && line(b3, "NAME script 3 k_def_heartbeat ").size(), "VM command histogram + script names");
    std::string nt = line(b3, "NET ");
    check(field(nt, "send") == 2 && field(nt, "bytes") == 70100 && field(nt, "max_len") == 70000 && field(nt, "ovf_test") == 2 && field(nt, "ovf_hit") == 1 &&
          nt.find("focus_dummy=1 scene_dtor=1") != std::string::npos, "net ring + scene counters (overflow test reproduces the engine compare)");
    std::string ai = line(b3, "AI ");
    check(field(ai, "ticks") == 2 && field(ai, "max_n") == 5 && field(ai, "max_n_level") == 3 && field(ai, "obj_updates") == 2 && field(ai, "prepass_iter") == 5 &&
          field(ai, "obj_ms") >= 9 && field(ai, "tick_ms_est") >= 9 && (ai.find("tick_hist 0 0 0 1 ") != std::string::npos || ai.find("tick_hist 0 0 0 0 1 ") != std::string::npos),
          "AI ticks, list sizes, per-object time, derived pre-pass iterations, tick estimate + histogram");
    std::string pa = line(b3, "PATH ");
    check(field(pa, "calls") == 3 && field(pa, "pending") == 1 && field(pa, "ok") == 1 && field(pa, "fail") == 1 && field(pa, "ms") >= 2 && pa.find("EFFECT apply=2 remove=1") != std::string::npos,
          "pathfind results/time + effect apply/remove");
    check(line(b3, "HOOKCOST_BY RunScript calls=3").size() && line(b3, "HOOKCOST_BY Bucket calls=").size() && line(b3, "HOOKCOST_BY ResDemand calls=").size(), "per-handler HOOKCOST_BY lines");

    // ---- 0.1.1 census regression ----
    g_h = HeapCreate(0, 0x1000, 0);
    StatsFn stats = fn<StatsFn>("PerfLabStats"); long long st[6] = {0}; stats(st, 6);
    long long live0 = st[0], blocks0 = st[1];
    HANDLE th[4]; for (int i = 0; i < 4; ++i) th[i] = CreateThread(0, 0, storm, (LPVOID)(uintptr_t)(i * 7919 + 1), 0, 0);
    WaitForMultipleObjects(4, th, TRUE, INFINITE);
    stats(st, 6);
    printf("census: live_bytes=%lld live_blocks=%lld allocs=%lld frees=%lld untracked=%lld dropped=%lld\n", st[0], st[1], st[2], st[3], st[4], st[5]);
    check(st[0] == live0 && st[1] == blocks0 && st[2] > 100000 && st[5] == 0, "0.1.1 census storm: accounting returns to the start");

    // ---- hook cost ----
    mkRes(g_cost, 3007, 1000, 2); g_demand = demand; g_bucket = bucket; g_spin = spin; g_getimg = getimg;
    double cb = nsPerCall(costBucket, 2000000), cs = nsPerCall(costSpin, 2000000), cg = nsPerCall(costGetImg, 2000000), cd = nsPerCall(costDemand, 200000);
    printf("hook cost (handler only, excl. the KPM wrapper): bucket %.1f ns, movie spin %.1f ns, getimage %.1f ns, demand (SafeRead key entry) %.1f ns\n", cb, cs, cg, cd);
    check(cb < 100 && cs < 100 && cg < 100, "counter-only hooks < 100 ns/call");
    printf("%s: %d failures\n", mode, g_fail);
    return g_fail ? 1 : 0;
}
