// Smoke test for PerfLab 0.5.0 (measurement-only additions): MOD (module= in the #METRICS header), EVT (7 save/load/options detours + ring),
// TPCTOP/RESTOP (pointer-keyed live-resource table, tombstones, overflow), VA (address-space summary, also with vq=0), the ledger fields appended
// to RT / KT / BUILD / FRAME (redemB, txi_via_tpc_kb, requeue_tpc_retained_kb, gms_calls/gms_ms via the GlobalMemoryStatus IAT hook) and HWV/HWVFB
// (hwmip validation: per-format counts, first 8 fallbacks, level-1 width check).
// Build: i686-w64-mingw32-g++ -std=c++17 -O1 -o smoke_perflab5.exe smoke_perflab5.cpp -lgdi32 -lglu32
//   (dynamic CRT, default base 0x400000: the 7 MB .bss pad makes the image cover 0x400000-0xB00000 so the engine addresses used below (0xA1B4A4 app
//    global, 0xA1B490 resman, 0xA1B... load screen chain) are ordinary memory of this exe; same trick as smoke_perflab3.cpp. Big buffers use VirtualAlloc.)
// Run:   smoke_perflab5.exe <run|off|noev>   (dir needs the DLL from PerfLab.kpatch as perflab.dll). Exit code 0 = all checks passed.
//   run  = metrics=1 hwmip=1 vq=0 (ev default on): every check below
//   off  = no perflab_debug.txt: DLL inert (no files, no GLU hook, EVT/Malloc/TXI handlers do nothing)
//   noev = metrics=1 ev=0: no EVT / TPCTOP / RESTOP lines, EVT handlers inert (no HOOKCOST_BY Evt), MOD + VA + the rest still written
// The GL stubs and the level-query stand-in are injected through the harness-only exports PerfLabTestSetGl / PerfLabTestSetGetTexLevel (no GL context).
#include <windows.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <string>

static char pad[0x700000];   // covers the engine's code/data addresses used below
#define G(a) (*(volatile uint32_t*)(a))
typedef void (__cdecl* V0)();
typedef void (__cdecl* V1)(void*);
typedef void (__cdecl* V2)(void*, void*);
static HMODULE g_dll;
template <typename T> T fn(const char* n) { T f = (T)GetProcAddress(g_dll, n); if (!f) { printf("missing export %s\n", n); exit(3); } return f; }

static int g_fail = 0;
static void check(bool ok, const char* what) { printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++g_fail; }

static std::string fileText(const char* name) {
    FILE* f = fopen(name, "r"); if (!f) return ""; std::string s; char b[4096]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) s.append(b, n); fclose(f); return s;
}
static bool exists(const char* name) { return GetFileAttributesA(name) != INVALID_FILE_ATTRIBUTES; }
static std::string line(const std::string& blk, const char* prefix) {   // first line starting with prefix (the header counts as line 0)
    size_t p = blk.compare(0, strlen(prefix), prefix) == 0 ? 0 : blk.find(std::string("\n") + prefix);
    if (p == std::string::npos) return "";
    if (p) ++p;
    size_t e = blk.find('\n', p); return blk.substr(p, e == std::string::npos ? std::string::npos : e - p);
}
static int countLines(const std::string& blk, const char* prefix) {
    int n = 0; size_t p = 0;
    if (blk.compare(0, strlen(prefix), prefix) == 0) ++n;
    while ((p = blk.find(std::string("\n") + prefix, p)) != std::string::npos) { ++n; ++p; }
    return n;
}
static long long field(const std::string& l, const char* key) {   // "key=123" preceded by a space
    size_t p = l.find(std::string(" ") + key + "="); if (p == std::string::npos) return -999999;
    return atoll(l.c_str() + p + strlen(key) + 2);
}
static bool endsWith(const std::string& s, const char* suf) { size_t n = strlen(suf); return s.size() >= n && !s.compare(s.size() - n, n, suf); }

// ---- metrics file helpers: blocks are "#METRICS <n> <EVENT> tick=..." ... "#END <n>" ----
static int countEv(const char* ev) {   // complete blocks of one event
    std::string all = fileText("perflab_metrics.txt"), key = std::string(" ") + ev + " tick=";
    int n = 0; size_t p = 0;
    while ((p = all.find(key, p)) != std::string::npos) { size_t s = all.rfind("#METRICS ", p); if (s != std::string::npos && all.find("#END", p) != std::string::npos) ++n; ++p; }
    return n;
}
static std::string lastOf(const char* ev) {   // the last complete block of one event
    std::string all = fileText("perflab_metrics.txt"), key = std::string(" ") + ev + " tick=";
    size_t p = all.rfind(key); if (p == std::string::npos) return "";
    size_t s = all.rfind("#METRICS ", p), e = all.find("#END", p);
    return (s == std::string::npos || e == std::string::npos) ? "" : all.substr(s, e - s);
}
static std::string waitNext(const char* ev, int before) {   // wait for one more complete block of `ev`
    for (int i = 0; i < 80; ++i) { if (countEv(ev) > before) return lastOf(ev); Sleep(100); }
    return "";
}
static std::string manual() { int n = countEv("MANUAL"); fn<V0>("PerfLabMetricsNow")(); return waitNext("MANUAL", n); }
static std::string unload() { int n = countEv("UNLOAD"); fn<V0>("PerfLabUnload")(); return waitNext("UNLOAD", n); }

// ---- fake engine objects ----
struct FakeRes { char b[0x40]; char key[0x20]; };   // CRes: +4 u16 refcount, +8 source, +0x10 data, +0x14 key entry (restype u16 at +0x1A, CResRef at +0), +0x18 size
static void mkRes(FakeRes& r, int restype, int size, const char* name16) {
    memset(&r, 0, sizeof r);
    *(uint16_t*)(r.b + 4) = 1; *(void**)(r.b + 0x10) = r.b; *(int*)(r.b + 0x18) = size;
    memset(r.key, 0, 16); if (name16) memcpy(r.key, name16, strnlen(name16, 16));   // 16 bytes, NOT NUL-terminated when full
    *(uint16_t*)(r.key + 0x1A) = (uint16_t)restype; *(void**)(r.b + 0x14) = r.key;
}
struct ExoStr { const char* p; int len; };

// ---- stub GL (as in smoke_hwmip.cpp) ----
static std::string g_log;
static unsigned g_err = 0, g_failTex = 0, g_badLvlTarget = 0;
static unsigned g_tgt[8]; static int g_wv[8]; static int g_nt = 0;
static void add(const char* fmt, ...) { char b[160]; va_list ap; va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap); if (!g_log.empty()) g_log += ';'; g_log += b; }
static void WINAPI sTex(unsigned t, int lvl, int ifmt, int w, int h, int, unsigned fmt, unsigned type, const void*) {
    add("TEX %x %d %dx%d if%d f%x t%x", t, lvl, w, h, ifmt, fmt, type);
    int i = 0; for (; i < g_nt; ++i) if (g_tgt[i] == t) break;
    if (i == g_nt && g_nt < 8) ++g_nt;
    g_tgt[i] = t; g_wv[i] = w;
    if (g_failTex && t == g_failTex) g_err = 0x502;
}
static void WINAPI sPar(unsigned t, unsigned p, int v) { add("PAR %x %x %d", t, p, v); }
static unsigned WINAPI sErr() { unsigned e = g_err; g_err = 0; return e; }
static void WINAPI sGeti(unsigned, int* v) { *v = 0; }
static void WINAPI sGen(unsigned t) { add("GEN %x", t); }
static int WINAPI sGlu(unsigned t, int, int w, int h, unsigned, unsigned, const void*) { add("GLU %x %dx%d", t, w, h); return 0x1234; }
static void WINAPI sLvl(unsigned t, int level, unsigned pname, int* out) {   // glGetTexLevelParameteriv stand-in: width of level 1 = max(1, w/2) unless told otherwise
    add("LVL %x %d", t, level);
    int w = 0; for (int i = 0; i < g_nt; ++i) if (g_tgt[i] == t) w = g_wv[i];
    if (level != 1 || pname != 0x1000) { *out = -7; return; }
    *out = (t == g_badLvlTarget) ? 0 : (w / 2 > 1 ? w / 2 : 1);
}
static char g_data[1 << 14];
static void setGl(bool lvl, int maxSize = 2048) {
    void* f[7] = { (void*)sTex, (void*)sPar, (void*)sErr, (void*)sGeti, (void*)sGen, (void*)sGlu, (void*)(intptr_t)maxSize };
    fn<void (__cdecl*)(void**)>("PerfLabTestSetGl")(f);
    fn<void (__cdecl*)(void*)>("PerfLabTestSetGetTexLevel")(lvl ? (void*)sLvl : nullptr);
    g_log.clear(); g_err = 0; g_failTex = 0; g_badLvlTarget = 0; g_nt = 0;
}
static void mip(unsigned target, int comps, int w, int h, unsigned fmt, unsigned type = GL_UNSIGNED_BYTE, const void* data = g_data) { gluBuild2DMipmaps(target, comps, w, h, fmt, type, data); }

// 4 threads hammer ResMalloc / ResFreeData on disjoint CRes slices while the helper snapshots the table (UNLOAD blocks): the spinlock must keep it consistent.
struct StormArg { V2 mal, fre; void* resman; FakeRes* r; int n, rounds; };
static DWORD WINAPI tpcStorm(LPVOID a) {
    StormArg* x = (StormArg*)a;
    for (int k = 0; k < x->rounds; ++k) {
        for (int i = 0; i < x->n; ++i) x->mal(x->resman, x->r[i].b);
        for (int i = 0; i < x->n; ++i) x->fre(x->r[i].b, (void*)0x00712245);
    }
    return 0;
}

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "run";
    bool run = !strcmp(mode, "run"), off = !strcmp(mode, "off"), noev = !strcmp(mode, "noev");
    if (!run && !off && !noev) { printf("unknown mode %s\n", mode); return 2; }
    if ((uintptr_t)pad > 0xA1B000 || (uintptr_t)pad + sizeof pad < 0xA1C000) { printf("pad %p does not cover 0xA1B000-0xA1C000 (resman, app global)\n", (void*)pad); return 2; }   // no tx/g1skip here, so no 0x4261CF constraint
    DeleteFileA("perflab_debug.txt"); DeleteFileA("perflab_metrics.txt"); DeleteFileA("perflab_log.txt"); DeleteFileA("perflab_alloc_snaps.txt"); DeleteFileA("perflab_alloc.txt");
    if (!off) {
        FILE* cf = fopen("perflab_debug.txt", "w");
        fprintf(cf, "alloc=0\nflush_ms=60000\nmetrics=1\nvq=0\n");
        if (run) fprintf(cf, "hwmip=1\n");
        if (noev) fprintf(cf, "ev=0\n");
        fclose(cf);
    }
    // engine globals: resman, fake app chain (module + load screen)
    static char resman[0x40], app[16], client[16], internal[0x800], ls[64], mgr[0x200], modobj[0x80]; static void* modals[2];
    *(int*)(resman + 4) = 16 << 20; *(int*)(resman + 8) = 12345678; G(0xA1B490) = (uint32_t)(uintptr_t)resman;
    *(unsigned*)ls = 0x009A5C84; modals[0] = ls; *(void**)(internal + 0x278) = ls; *(void**)(internal + 0x274) = mgr; *(void**)(mgr + 0x94) = modals; *(int*)(mgr + 0x98) = 0;
    static const char nameA[] = "testmod_a", nameB[] = "testmod_b";
    *(void**)(modobj + 0x38) = (void*)nameA; *(int*)(modobj + 0x3C) = sizeof nameA;
    *(void**)(internal + 0x18) = modobj; *(void**)(app + 4) = client; *(void**)(client + 4) = internal; G(0xA1B4A4) = (uint32_t)(uintptr_t)app;
    HMODULE glu = GetModuleHandleA("glu32.dll");
    void* realGlu = glu ? (void*)GetProcAddress(glu, "gluBuild2DMipmaps") : nullptr;

    g_dll = LoadLibraryA("perflab.dll"); if (!g_dll) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    V2 malloc_ = fn<V2>("PerfLabResMalloc"), freed = fn<V2>("PerfLabResFreeData");
    V1 evict = fn<V1>("PerfLabResEvict"), demand = fn<V1>("PerfLabResDemand");
    typedef void (__cdecl* VSave)(int, void*);
    VSave saveG = fn<VSave>("PerfLabSaveGame"), loadG = fn<VSave>("PerfLabLoadGame");
    V0 qload = fn<V0>("PerfLabQuickLoad"), qsave = fn<V0>("PerfLabQuickSave"), autosave = fn<V0>("PerfLabAutosave"), optsave = fn<V0>("PerfLabOptSave"), sndapply = fn<V0>("PerfLabSoundApply");
    Sleep(200);

    // ======================================================================================================== off
    if (off) {
        // no perflab_debug.txt: DllMain returned early. Every handler must be inert and nothing may be written or hooked.
        FakeRes* r = (FakeRes*)VirtualAlloc(0, sizeof(FakeRes) * 4, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        for (int i = 0; i < 4; ++i) mkRes(r[i], 3007, 100000 * (i + 1), "offtex");
        ExoStr n1 = { "offsave", 8 };
        saveG(1, &n1); loadG(2, &n1); qload(); qsave(); autosave(); optsave(); sndapply();
        for (int i = 0; i < 4; ++i) malloc_(resman, r[i].b);
        for (int i = 0; i < 4; ++i) freed(r[i].b, (void*)0x401000);
        demand(r[0].b);
        ((void (__cdecl*)(void*, void*))fn<void*>("PerfLabTxiTpc"))((void*)"tex_a", nullptr);
        MEMORYSTATUS ms; GlobalMemoryStatus(&ms);
        fn<V0>("PerfLabMetricsNow")(); fn<V0>("PerfLabUnload")();
        mip(GL_TEXTURE_2D, 4, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);   // width 0: GLU returns an error; nothing may intercept it
        Sleep(600);
        check(!exists("perflab_log.txt") && !exists("perflab_metrics.txt"), "off: no perflab_log.txt / perflab_metrics.txt created");
        check(fn<void* (__cdecl*)()>("PerfLabTestRGluMip")() == nullptr, "off: GLU IAT slot untouched (rGluMip NULL)");
        (void)realGlu;
        printf("%s: %d failures\n", mode, g_fail); return g_fail ? 1 : 0;
    }

    // ======================================================================================================== run / noev
    std::string log0 = fileText("perflab_log.txt");
    check(log0.find("perf-lab 0.5.0 attach") != std::string::npos, "0.5.0 version in the attach line");
    check(log0.find(noev ? "ev=0" : "ev=1") != std::string::npos, noev ? "attach line records ev=0" : "attach line records ev=1");
    check(log0.find("GlobalMemoryStatus=1") != std::string::npos, "GlobalMemoryStatus IAT hook installed (the exe imports it)");
    check(log0.find(noev ? "tpctop=0" : "tpctop=1") != std::string::npos, noev ? "ev=0: TPCTOP table not allocated" : "TPCTOP table allocated");

    // ---- MOD: module name in the header, per block ----
    std::string b1 = manual(); std::string h1 = b1.substr(0, b1.find('\n'));
    check(endsWith(h1, " module=testmod_a") && h1.find(" hwmip=") != std::string::npos && h1.find(" module=") == h1.rfind(" module="), "MOD: header ends with module=testmod_a (existing fields untouched before it)");
    *(void**)(modobj + 0x38) = (void*)nameB; *(int*)(modobj + 0x3C) = sizeof nameB;
    std::string b2 = manual();
    check(endsWith(b2.substr(0, b2.find('\n')), " module=testmod_b"), "MOD: module changes with the engine state (module=testmod_b)");
    *(void**)(internal + 0x18) = nullptr;
    std::string b3 = manual();
    check(endsWith(b3.substr(0, b3.find('\n')), " module=-"), "MOD: NULL module (main menu) -> module=-");
    *(void**)(internal + 0x18) = modobj; *(void**)(modobj + 0x38) = (void*)"A_VERY_LONG_MODULE_NAME_THAT_EXCEEDS_THIRTY_ONE_CHARS"; *(int*)(modobj + 0x3C) = 54;
    std::string b3b = manual(); std::string h3b = b3b.substr(0, b3b.find('\n'));
    check(endsWith(h3b, " module=A_VERY_LONG_MODULE_NAME_THAT_EX"), "MOD: names are cut at 31 characters");
    G(0xA1B4A4) = 0;
    std::string b3c = manual();
    check(endsWith(b3c.substr(0, b3c.find('\n')), " module=?"), "MOD: unreadable chain (app NULL) -> module=?");
    G(0xA1B4A4) = (uint32_t)(uintptr_t)app; *(void**)(modobj + 0x38) = (void*)nameA; *(int*)(modobj + 0x3C) = sizeof nameA;

    // ---- VA: every block, also with vq=0 ----
    std::string va = line(b1, "VA ");
    check(va.size() && field(va, "free_mb") >= 0 && field(va, "largest_free_mb") >= 0 && field(va, "commit_mb") > 0 && field(va, "reserve_mb") >= 0 && field(va, "regions") > 3 &&
          va.find(" largest_free_at=0x") != std::string::npos && field(va, "largest_free_mb") <= field(va, "free_mb"), "VA: line present with metrics on and vq=0 (all six fields, largest <= free)");
    check(b1.find("\nVQ ") == std::string::npos && b1.find("\nVQTOP") == std::string::npos, "VA: vq=0 writes no VQ lines (VQ output unchanged)");
    check(countLines(b3, "VA ") == 1 && countLines(b3c, "VA ") == 1, "VA: exactly one line per block");

    // ---- EVT ----
    if (!noev) {
        std::string pre = manual();   // drain whatever the earlier blocks left (nothing yet)
        ExoStr s1 = { "MySave", 7 }, s2 = { "Other save", 11 }, longName = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 37 }, ctl = { "a\x01" "b", 4 };
        saveG(3, &s1); loadG(5, &s2); qsave(); qload(); autosave(); optsave(); optsave(); sndapply(); saveG(7, nullptr); loadG(0, &longName); saveG(-1, &ctl);
        std::string e1 = manual();
        check(countLines(e1, "EVT kind=") == 11, "EVT: 11 events -> 11 EVT lines");
        check(line(e1, "EVT kind=save ").find(" slot=3 name=MySave") != std::string::npos, "EVT: save slot 3, name read from the CExoString");
        check(e1.find("EVT kind=load tick=") != std::string::npos && e1.find(" slot=5 name=Other save\n") != std::string::npos, "EVT: load slot 5, name with a space kept");
        check(e1.find(" slot=-1 name=-\n") != std::string::npos && line(e1, "EVT kind=qsave ").find("name=-") != std::string::npos && line(e1, "EVT kind=qload ").size() && line(e1, "EVT kind=autosave ").size() &&
              line(e1, "EVT kind=sndapply ").size() && countLines(e1, "EVT kind=optsave ") == 2, "EVT: parameterless kinds (slot=-1, name=-): qsave qload autosave optsave x2 sndapply");
        check(e1.find(" slot=7 name=-\n") != std::string::npos, "EVT: NULL CExoString* -> name=-");
        check(e1.find(" slot=0 name=ABCDEFGHIJKLMNOPQRSTUVWXYZ01234\n") != std::string::npos, "EVT: name cut at 31 characters");
        check(e1.find(" name=a?b\n") != std::string::npos, "EVT: non-printable byte -> ?");
        std::string cn = line(e1, "EVT counts");
        check(field(cn, "save") == 3 && field(cn, "load") == 2 && field(cn, "qsave") == 1 && field(cn, "qload") == 1 && field(cn, "autosave") == 1 && field(cn, "optsave") == 2 && field(cn, "sndapply") == 1 &&
              cn.find("lost=") == std::string::npos, "EVT: counts line save=3 load=2 qsave=1 qload=1 autosave=1 optsave=2 sndapply=1, no lost=");
        { size_t t = e1.find("tick="); check(t != std::string::npos && atoll(e1.c_str() + t + 5) > 0, "EVT: tick (GetTickCount) recorded"); }
        qload();
        std::string e2 = manual();
        check(countLines(e2, "EVT kind=") == 1 && e2.find("EVT kind=qload ") != std::string::npos && field(line(e2, "EVT counts"), "qload") == 2 && field(line(e2, "EVT counts"), "save") == 3,
              "EVT: a block lists only the entries new since the previous block; counts are cumulative");
        std::string e3 = manual();
        check(countLines(e3, "EVT kind=") == 0 && line(e3, "EVT counts").size(), "EVT: no new events -> counts line only");
        for (int i = 0; i < 100; ++i) saveG(i, &s1);
        std::string e4 = manual();
        check(countLines(e4, "EVT kind=") == 64 && field(line(e4, "EVT counts"), "save") == 103 && field(line(e4, "EVT counts"), "lost") == 36 && e4.find(" slot=99 name=MySave\n") != std::string::npos &&
              e4.find(" slot=35 name=") == std::string::npos && e4.find(" slot=36 name=") != std::string::npos, "EVT: 100 events between blocks -> newest 64 listed, lost=36 reported, counters still exact");
        check(line(e4, "HOOKCOST_BY Evt calls=").size(), "EVT: HOOKCOST_BY Evt line (shared cost id)");
    } else {
        ExoStr s1 = { "MySave", 7 };
        saveG(3, &s1); loadG(5, &s1); qsave(); qload(); autosave(); optsave(); sndapply();
        std::string n1 = manual();
        check(countLines(n1, "EVT") == 0 && n1.find("\nEVT") == std::string::npos, "noev: ev=0 -> no EVT lines at all (not even the counts line)");
        check(line(n1, "HOOKCOST_BY Evt").empty(), "noev: EVT handlers inert (no HOOKCOST_BY Evt, counters untouched)");
    }

    // ---- ledger fields appended to RT / KT / BUILD / FRAME ----
    static FakeRes A, B; mkRes(A, 3007, 2000000, "tex_a"); mkRes(B, 2002, 100000, "model_b");
    malloc_(resman, A.b); malloc_(resman, B.b);
    demand(A.b); evict(B.b); freed(B.b, (void*)0x00712245); demand(B.b); freed(A.b, (void*)0x00401000);   // B: re-demand after eviction -> redem60 1, redemB = 100000
    typedef void (__cdecl* VTxi)(void*, void*);
    static FakeRes tpcRes; mkRes(tpcRes, 3007, 1048576, "tex_x"); static char wrapper[0x20]; *(void**)(wrapper + 8) = tpcRes.b;
    VTxi txi = fn<VTxi>("PerfLabTxiTpc");
    txi((void*)"tex_a", wrapper); txi((void*)"tex_b", wrapper); txi((void*)"tex_c", nullptr); txi((void*)"tex_d", (void*)0x10);   // NULL / unreadable wrapper: counted, no bytes
    V1 buildHook = fn<V1>("PerfLabBuildTex");
    static char tpcFlat[0x100], tpcCube[0x100], tpcNone[0x100];
    auto mkTex = [](char* t, int fmt, int w, int h, int base, int mips) {
        t[0xE9] = 1; t[0xE4] = 1; *(void**)(t + 0x44) = t; *(int*)(t + 0x74) = fmt; *(int*)(t + 0x68) = w; *(int*)(t + 0x6C) = h; *(int*)(t + 0x5C) = base; *(int16_t*)(t + 0xD2) = (int16_t)mips;
    };
    mkTex(tpcFlat, 3, 256, 256, 32768, 3);   // 32768 + 128x128 (32*32*8) + 64x64 (16*16*8) = 43008
    mkTex(tpcCube, 4, 64, 384, 4096, 2);     // (4096 + 32x32 face (8*8*16)) * 6 = 30720
    mkTex(tpcNone, 9, 256, 256, 32768, 3);   // unknown block format: counted, 0 bytes
    buildHook(tpcFlat); buildHook(tpcCube); buildHook(tpcNone);
    for (int i = 0; i < 5; ++i) { MEMORYSTATUS ms; GlobalMemoryStatus(&ms); }
    std::string l1 = manual();
    { long long v[13] = {0}; std::string l = line(l1, "RT 2002 ");
      int n = sscanf(l.c_str(), "RT %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], &v[10], &v[11], &v[12]);
      check(n == 13 && v[9] == 1 && v[12] == 100000 && v[11] == 0, "RT: 13th field redemB = bytes of re-demanded resources (100000), the first 12 fields unchanged"); }
    check(line(l1, "# RT ").find("resident redemB") != std::string::npos, "RT: comment header lists redemB last");
    std::string kt = line(l1, "KT ");
    check(field(kt, "txi_via_tpc") == 4 && field(kt, "txi_via_tpc_kb") == 2048 && endsWith(kt, " txi_via_tpc_kb=2048"), "KT: txi_via_tpc_kb = 2 x 1 MB of TPC resource demanded for TXIs (NULL/bad wrappers add nothing), last field");
    std::string bl = line(l1, "BUILD ");
    check(field(bl, "calls") == 3 && field(bl, "requeue_tpc_retained") == 3 && field(bl, "requeue_tpc_retained_kb") == (43008 + 30720) / 1024 && endsWith(bl, "requeue_tpc_retained_kb=72"),
          "BUILD: requeue_tpc_retained_kb = 72 (flat DXT1 43008 B + cube DXT5 30720 B via FreeImage's formula; unknown format counts 0), last field");
    std::string fr = line(l1, "FRAME ");
    check(field(fr, "gms_calls") == 5 && field(fr, "gms_ms") >= 0 && fr.rfind(" gms_calls=") > fr.find("allocs_per_frame_hist") && fr.find(" gms_ms=") == fr.rfind(" gms_"), "FRAME: gms_calls=5 gms_ms= appended after the histograms");
    if (noev) check(line(l1, "RT 3007 ").size() > 0 && line(l1, "VA ").size() > 0, "noev: RT and VA still written");

    // ---- TPCTOP / RESTOP ----
    if (!noev) {
        FakeRes* arr = (FakeRes*)VirtualAlloc(0, sizeof(FakeRes) * 20000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        // scenario 1: 25 TPCs (rank 1 = a 16-char unterminated upper-case name), a 50 MB model, a TGA; LOAD_DOWN block (load-screen edge)
        for (int i = 0; i < 25; ++i) { char nm[17]; snprintf(nm, sizeof nm, "Tpc_%02d", i); mkRes(arr[i], 3007, (i + 1) * 102400, i == 24 ? "BIGTEXTURE_ABCDE" : nm); }
        mkRes(arr[25], 2002, 50 << 20, "BigModel"); mkRes(arr[26], 3, 360000, "lbl_map749kor");
        for (int i = 0; i < 27; ++i) malloc_(resman, arr[i].b);
        malloc_(resman, arr[0].b);   // same CRes allocated again: updated, not duplicated
        int before = countEv("LOAD_DOWN");
        *(int*)(mgr + 0x98) = 1; Sleep(400); *(int*)(mgr + 0x98) = 0;
        std::string d1 = waitNext("LOAD_DOWN", before);
        check(d1.size() > 0, "LOAD_DOWN block written at the load-screen edge");
        check(line(d1, "TPCTOP n=25 total_mb=31.7").size() > 0 && line(d1, "TPCTOP n=25 total_mb=31.7").find("overflow=") == std::string::npos, "TPCTOP: n=25 total_mb=31.7 at LOAD_DOWN, no overflow field");
        check(line(d1, "TPCTOP 1 kb=2500 name=bigtexture_abcde").size() && line(d1, "TPCTOP 2 kb=2400 name=tpc_23").size() && line(d1, "TPCTOP 20 kb=600 name=tpc_05").size(),
              "TPCTOP: sorted by size descending, resref lower-cased, 16-byte unterminated name intact");
        check(line(d1, "TPCTOP 21 ").empty() && countLines(d1, "TPCTOP ") == 21, "TPCTOP: top 20 only (n= line + 20 ranks)");
        check(line(d1, "RESTOP 1 type=2002 kb=51200 name=bigmodel").size() && line(d1, "RESTOP 2 type=3007 kb=2500 name=bigtexture_abcde").size() && countLines(d1, "RESTOP ") == 10 &&
              line(d1, "RESTOP 11 ").empty() && d1.find("type=3 kb=351 name=lbl_map749kor") == std::string::npos, "RESTOP: top 10 across all restypes (model first, TGA below the cut)");
        std::string mn = manual();
        check(mn.find("\nTPCTOP") == std::string::npos && mn.find("\nRESTOP") == std::string::npos, "TPCTOP/RESTOP only at LOAD_DOWN and UNLOAD (not in a MANUAL block)");
        // scenario 2: free the biggest -> gone at UNLOAD
        freed(arr[24].b, (void*)0x00712245); freed(arr[26].b, (void*)0x00401000); freed(arr[25].b, (void*)0x00712245);
        std::string u1 = unload();
        check(line(u1, "TPCTOP n=24 total_mb=29.3").size() && line(u1, "TPCTOP 1 kb=2400 name=tpc_23").size() && u1.find("bigtexture_abcde") == std::string::npos && line(u1, "RESTOP 1 type=3007 kb=2400 name=tpc_23").size(),
              "TPCTOP: freed resources removed (n=24, new rank 1 = tpc_23, model and TGA gone)");
        // source==1 resources are not freed by the engine and must stay
        *(uint32_t*)(arr[3].b + 8) = 0x40000000; freed(arr[3].b, (void*)0x00401000);
        std::string u1b = unload(); check(line(u1b, "TPCTOP n=24 ").size() > 0, "TPCTOP: a CRes whose free the engine skips (source 1) stays in the table");
        *(uint32_t*)(arr[3].b + 8) = 0;
        for (int i = 0; i < 24; ++i) freed(arr[i].b, (void*)0x00712245);
        // scenario 3: tombstone churn (insert-free windows), table must end empty
        for (int i = 0; i < 20000; ++i) { mkRes(arr[i], 3007, 1000 + i, "churn"); malloc_(resman, arr[i].b); if (i >= 8) freed(arr[i - 8].b, (void*)0x00712245); }
        for (int i = 19992; i < 20000; ++i) freed(arr[i].b, (void*)0x00712245);
        std::string u2 = unload();
        check(line(u2, "TPCTOP n=0 total_mb=0.0").size() > 0 && line(u2, "TPCTOP n=0 total_mb=0.0").find("overflow=") == std::string::npos, "TPCTOP: 20000 insert/free cycles with 8 live at a time -> table empty again, no overflow");
        // scenario 4: overflow (> capacity live)
        for (int i = 0; i < 17000; ++i) mkRes(arr[i], 3007, i == 0 ? 99 << 20 : 1000 + i, i == 0 ? "first" : "ovf");
        for (int i = 0; i < 17000; ++i) malloc_(resman, arr[i].b);
        std::string u3 = unload(); std::string tl = line(u3, "TPCTOP n=");
        long long n = field(tl, "n"), ov = field(tl, "overflow");
        check(n > 15000 && n <= 16384 && ov > 0 && ov == 17000 - n, "TPCTOP: 17000 live inserts -> table keeps ~16K, overflow counted (n + overflow == 17000)");
        printf("INFO overflow case: n=%lld overflow=%lld (%s)\n", n, ov, tl.c_str());
        check(line(u3, "TPCTOP 1 kb=101376 name=first").size() > 0, "TPCTOP: the first (largest) entry survived the overflow");
        for (int i = 0; i < 17000; ++i) freed(arr[i].b, (void*)0x00712245);   // frees of overflowed CRes find nothing
        std::string u4 = unload();
        check(line(u4, "TPCTOP n=0 total_mb=0.0").size() > 0 && field(line(u4, "TPCTOP n="), "overflow") == ov, "TPCTOP: after freeing everything n=0; overflow stays cumulative");
        // scenario 5: concurrent writers + helper snapshots (UNLOAD) -> consistent, empty at the end
        for (int i = 0; i < 16000; ++i) mkRes(arr[i], 3007, 4096 + i, "storm");
        StormArg sa[4]; HANDLE th[4];
        for (int t = 0; t < 4; ++t) { sa[t] = { malloc_, freed, resman, arr + t * 4000, 4000, 80 }; th[t] = CreateThread(0, 0, tpcStorm, &sa[t], 0, 0); }
        int snaps = 0; DWORD wr;
        do { std::string mid = unload(); if (mid.size() && line(mid, "TPCTOP n=").size()) ++snaps; wr = WaitForMultipleObjects(4, th, TRUE, 0); } while (wr == WAIT_TIMEOUT && snaps < 40);
        WaitForMultipleObjects(4, th, TRUE, INFINITE);
        std::string u5 = unload();
        check(snaps >= 1 && line(u5, "TPCTOP n=0 total_mb=0.0").size() > 0, "TPCTOP: 4 threads x 80 rounds of 4000 malloc/free with concurrent snapshots -> table consistent (n=0 at the end)");
        printf("INFO concurrent snapshots taken while the storm ran: %d\n", snaps);
    } else {
        FakeRes* arr = (FakeRes*)VirtualAlloc(0, sizeof(FakeRes) * 10, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        for (int i = 0; i < 10; ++i) { mkRes(arr[i], 3007, 100000 * (i + 1), "noevtex"); malloc_(resman, arr[i].b); }
        for (int i = 0; i < 10; ++i) freed(arr[i].b, (void*)0x00712245);
        std::string un = unload();
        check(un.size() > 0 && un.find("\nTPCTOP") == std::string::npos && un.find("\nRESTOP") == std::string::npos && un.find("\nEVT") == std::string::npos, "noev: UNLOAD block has no TPCTOP / RESTOP / EVT lines");
        check(endsWith(un.substr(0, un.find('\n')), " module=testmod_a") && line(un, "VA ").size(), "noev: MOD and VA still written");
    }

    // ---- HWV (hwmip=1, GL stubs) ----
    if (run) {
        setGl(true);
        mip(GL_TEXTURE_2D, 4, 64, 64, 0x1908);
        check(g_log == "TEX de1 0 64x64 if4 f1908 t1401;GEN de1;LVL de1 1", "HWV: 2D RGBA upload then ONE pure level-1 query (no state change, same call)");
        setGl(true);
        mip(0x8515, 3, 16, 16, 0x1907);
        check(g_log == "PAR 8513 8191 1;TEX 8515 0 16x16 if3 f1907 t1401;PAR 8513 8191 0;LVL 8515 1", "HWV: cube face: level-1 query goes to the face target after the flag is reset");
        setGl(true);
        mip(GL_TEXTURE_2D, 4, 64, 64, 0x1908); mip(GL_TEXTURE_2D, 3, 32, 32, 0x1907); mip(GL_TEXTURE_2D, 0x190A, 16, 16, 0x190A); mip(GL_TEXTURE_2D, 1, 8, 8, 0x1909);
        mip(GL_TEXTURE_2D, 4, 4, 4, 0x80E1); mip(0x8515, 3, 16, 16, 0x1907); mip(GL_TEXTURE_2D, 4, 1, 1, 0x1908);
        g_badLvlTarget = 0x8516; mip(0x8516, 3, 16, 16, 0x1907); mip(0x8517, 3, 16, 16, 0x1907);
        g_badLvlTarget = 0x8517; mip(0x8517, 3, 32, 32, 0x1907);
        std::string h = manual(); std::string hv = line(h, "HWV ");
        check(field(hv, "fmt_rgba") == 2 && field(hv, "fmt_rgb") == 5 && field(hv, "fmt_la") == 1 && field(hv, "fmt_l") == 1 && field(hv, "fmt_other") == 1,
              "HWV: per-format counts (RGBA 2, RGB 5, LA 1, L 1, other 1)");
        check(field(hv, "lvl1_ok") == 7 && field(hv, "lvl1_bad") == 2 && hv.find(" lvl1_first_bad=0x8516/16/0") != std::string::npos && endsWith(hv, "lvl1_first_bad=0x8516/16/0"),
              "HWV: level-1 check ok=7 (1x1 skipped), bad=2, first bad = 0x8516/16/0 (only the first is kept)");
        check(h.find("\nHWVFB") == std::string::npos, "HWV: no fallbacks yet -> no HWVFB lines");
        check(fileText("perflab_log.txt").find("HWV fmt_rgba=2 fmt_rgb=5") != std::string::npos, "HWV: also logged next to the HWMIP lines in perflab_log.txt");
        // fallbacks: first 8 recorded, reasons in call order
        setGl(true);
        mip(GL_TEXTURE_2D, 4, 100, 64, 0x1908);                              // 1 npot
        mip(GL_TEXTURE_2D, 4, 64, 64, 0x1908, GL_UNSIGNED_BYTE, nullptr);    // 2 null
        mip(GL_TEXTURE_2D, 4, 64, 64, 0x1908, 0x1403);                       // 3 type
        mip(0x806F, 4, 64, 64, 0x1908);                                      // 4 target
        mip(GL_TEXTURE_2D, 4, 4096, 64, 0x1908);                             // 5 size (max 2048)
        g_failTex = GL_TEXTURE_2D; mip(GL_TEXTURE_2D, 4, 32, 32, 0x1908); g_failTex = 0;   // 6 glerror (GLU re-specifies)
        mip(GL_TEXTURE_2D, 4, 20, 20, 0x1907);                               // 7 npot
        mip(GL_TEXTURE_2D, 4, 0, 0, 0x1907);                                 // 8 npot (zero size)
        mip(GL_TEXTURE_2D, 4, 33, 33, 0x1907);                               // 9 npot: not recorded (only the first 8)
        std::string f = manual(); std::string fv = line(f, "HWV ");
        check(countLines(f, "HWVFB ") == 8 && line(f, "HWVFB 9 ").empty(), "HWV: exactly the first 8 fallbacks are recorded");
        check(line(f, "HWVFB 1 reason=npot target=0xde1 w=100 h=64 fmt=0x1908 type=0x1401").size() && line(f, "HWVFB 2 reason=null target=0xde1 w=64 h=64 fmt=0x1908 type=0x1401").size() &&
              line(f, "HWVFB 3 reason=type target=0xde1 w=64 h=64 fmt=0x1908 type=0x1403").size() && line(f, "HWVFB 4 reason=target target=0x806f w=64 h=64 fmt=0x1908 type=0x1401").size() &&
              line(f, "HWVFB 5 reason=size target=0xde1 w=4096 h=64 fmt=0x1908 type=0x1401").size() && line(f, "HWVFB 6 reason=glerror target=0xde1 w=32 h=32 fmt=0x1908 type=0x1401").size() &&
              line(f, "HWVFB 7 reason=npot target=0xde1 w=20 h=20 fmt=0x1907 type=0x1401").size() && line(f, "HWVFB 8 reason=npot target=0xde1 w=0 h=0 fmt=0x1907 type=0x1401").size(),
              "HWV: HWVFB rows carry reason/target/w/h/fmt/type in call order (npot null type target size glerror npot npot)");
        check(field(fv, "fmt_rgba") == 0 && field(fv, "lvl1_ok") == 0 && fv.find("lvl1_first_bad=-") != std::string::npos, "HWV: counters were reset by the harness setters (fallbacks do not count as HW uploads)");
        // only the first 16 HW uploads are validated
        setGl(true);
        for (int i = 0; i < 20; ++i) mip(GL_TEXTURE_2D, 4, 16, 16, 0x1908);
        std::string c16 = manual();
        check(field(line(c16, "HWV "), "fmt_rgba") == 20 && field(line(c16, "HWV "), "lvl1_ok") == 16 && field(line(c16, "HWV "), "lvl1_bad") == 0, "HWV: 20 HW uploads counted, only the first 16 level-1 checked");
        // no glGetTexLevelParameteriv available: no query at all
        setGl(false);
        mip(GL_TEXTURE_2D, 4, 64, 64, 0x1908);
        check(g_log == "TEX de1 0 64x64 if4 f1908 t1401;GEN de1", "HWV: without glGetTexLevelParameteriv no query is issued");
        std::string nq = manual(); check(field(line(nq, "HWV "), "fmt_rgba") == 1 && field(line(nq, "HWV "), "lvl1_ok") == 0, "HWV: formats still counted, lvl1_ok/bad stay 0");
        // the hwmip_* fields of the GL line are unchanged (appended-only contract)
        std::string gl = line(nq, "GL ");
        check(gl.find("GL glimage=") == 0 && endsWith(gl, ("hwmip_cube_fallback=" + std::to_string(field(gl, "hwmip_cube_fallback"))).c_str()), "HWV: GL line keeps hwmip_cube_fallback as its last field");
        check(nq.find("\nHWV ") > nq.find("\nGL ") && nq.find("\nHWV ") < nq.find("\nFRAME "), "HWV lines sit right after the GL line in the metrics block");
    } else {
        std::string nh = manual();
        check(nh.find("\nHWV") == std::string::npos, "noev (hwmip off): no HWV lines");
    }

    printf("%s: %d failures\n", mode, g_fail);
    return g_fail ? 1 : 0;
}
