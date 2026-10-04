// Smoke test for perf-lab 0.4.0/0.5.0 hwmip=1 (hardware mipmaps behind the gluBuild2DMipmaps IAT hook).
// Build: i686-w64-mingw32-g++ -std=c++17 -O1 -o smoke_hwmip.exe smoke_hwmip.cpp -lgdi32 -lglu32
//   (dynamic CRT; needs libstdc++-6.dll, libgcc_s_dw2-1.dll, libwinpthread-1.dll next to the exe; no .bss pad: hwmip touches no engine address)
// Run:   smoke_hwmip.exe <run|metrics|off|bare>   (dir needs the DLL from PerfLab.kpatch as perflab.dll). Exit code 0 = all checks passed.
//   run     = hwmip=1, metrics=0: the GLU hook must be installed anyway; every case below; HWMIP summary line from the helper thread
//   metrics = hwmip=1, metrics=1: the IAT is patched once (rGluMip == the real GLU entry), ` hwmip=1` at the end of the #METRICS header, hwmip_*
//             fields at the end of the GL line (all earlier fields intact), mipmaps= counts both paths
//   off     = metrics=1, hwmip absent: byte-identical 0.3.0 behaviour (GLU only, hwmip counters 0, header hwmip=0)
//   bare    = neither metrics nor hwmip: the IAT is not touched (rGluMip stays NULL)
// The harness calls gluBuild2DMipmaps through its own IAT (perf-lab patches it like the game's), with stub GL entry points and a stub for the
// real GLU function injected through PerfLabTestSetGl (no GL context needed). Stubs record calls in order.
// hwmip_hw counts 2D uploads done on HW; cube faces done on HW count in hwmip_cube, cube faces that went to GLU in hwmip_cube_fallback.
#include <windows.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <string>

static int g_fail = 0;
static void check(bool ok, const char* what) { printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++g_fail; }

static HMODULE g_dll;
template <typename T> T fn(const char* n) { T f = (T)GetProcAddress(g_dll, n); if (!f) { printf("missing export %s\n", n); exit(3); } return f; }

// ---- stub GL ----------------------------------------------------------------------------------------------------------
static std::string g_log;            // ordered call log, ';' separated
static unsigned g_err = 0;           // pending GL error (glGetError returns it once)
static unsigned g_failTex = 0;       // glTexImage2D on this target sets GL_INVALID_OPERATION (0 = never)
static bool g_failGen = false;       // glGenerateMipmap sets an error
static int g_errCalls = 0;
static void add(const char* fmt, ...) {
    char b[160]; va_list ap; va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
    if (!g_log.empty()) g_log += ';';
    g_log += b;
}
static void WINAPI sTex(unsigned t, int lvl, int ifmt, int w, int h, int, unsigned fmt, unsigned type, const void*) {
    add("TEX %x %d %dx%d if%d f%x t%x", t, lvl, w, h, ifmt, fmt, type);
    if (g_failTex && t == g_failTex) g_err = 0x502;
}
static void WINAPI sPar(unsigned t, unsigned p, int v) { add("PAR %x %x %d", t, p, v); }
static unsigned WINAPI sErr() { ++g_errCalls; unsigned e = g_err; g_err = 0; return e; }
static void WINAPI sGeti(unsigned, int* v) { *v = 0; }
static void WINAPI sGen(unsigned t) { add("GEN %x", t); if (g_failGen) g_err = 0x506; }
static int WINAPI sGlu(unsigned t, int, int w, int h, unsigned, unsigned, const void*) { add("GLU %x %dx%d", t, w, h); return 0x1234; }

static char g_data[1 << 16];
static void setGl(bool gen, int maxSize) {
    void* f[7] = { (void*)sTex, (void*)sPar, (void*)sErr, (void*)sGeti, gen ? (void*)sGen : nullptr, (void*)sGlu, (void*)(intptr_t)maxSize };
    ((void (__cdecl*)(void**))GetProcAddress(g_dll, "PerfLabTestSetGl"))(f);
    g_log.clear(); g_err = 0; g_failTex = 0; g_failGen = false; g_errCalls = 0;
}
enum { C_CALLS, C_HW, C_NPOT, C_SIZE, C_TYPE, C_TARGET, C_NULL, C_NOGL, C_GLERR, C_CUBE, C_CUBEFB, C_N };
static long long g_c[C_N];
static void counters() { ((int (__cdecl*)(long long*, int))GetProcAddress(g_dll, "PerfLabTestHwmipCounters"))(g_c, C_N); }
static bool cnt(long long calls, long long hw, long long npot, long long size, long long type, long long target, long long nul, long long nogl, long long glerr, long long cube, long long cubefb) {
    counters();
    long long want[C_N] = { calls, hw, npot, size, type, target, nul, nogl, glerr, cube, cubefb };
    for (int i = 0; i < C_N; ++i) if (g_c[i] != want[i]) {
        printf("   counters:"); for (int j = 0; j < C_N; ++j) printf(" %lld", g_c[j]);
        printf("\n   expected:"); for (int j = 0; j < C_N; ++j) printf(" %lld", want[j]); printf("\n");
        return false;
    }
    return true;
}
static void expectLog(const char* what, const char* want) {
    bool ok = g_log == want; check(ok, what);
    if (!ok) printf("   log:  %s\n   want: %s\n", g_log.c_str(), want);
}
static int g_nmip = 0;   // every gluBuild2DMipmaps call this harness makes
static int mip(unsigned target, int comps, int w, int h, unsigned fmt = GL_RGBA, unsigned type = GL_UNSIGNED_BYTE, const void* data = g_data) {
    ++g_nmip;
    return gluBuild2DMipmaps(target, comps, w, h, fmt, type, data);   // through this exe's IAT = perf-lab's hook
}
// one cube set: faces in `order`, w x h each (face index i of the order list is 0x8515 + order[i])
static void cubeFaces(const unsigned* order, int n, int w = 16, int h = 16) { for (int i = 0; i < n; ++i) mip(order[i], 3, w, h, GL_RGB); }
static std::string texLine(unsigned t, int w, int h, int ifmt, unsigned fmt) { char b[96]; snprintf(b, sizeof b, "TEX %x 0 %dx%d if%d f%x t1401", t, w, h, ifmt, fmt); return b; }
static std::string fileText(const char* name) {
    FILE* f = fopen(name, "r"); if (!f) return ""; std::string s; char b[4096]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) s.append(b, n); fclose(f); return s;
}
static std::string lastBlock() {
    std::string all = fileText("perflab_metrics.txt"); size_t p = all.rfind("#METRICS "); return p == std::string::npos ? "" : all.substr(p);
}
static std::string lineOf(const std::string& blk, const char* prefix) {
    size_t p = (blk.compare(0, strlen(prefix), prefix) == 0) ? 0 : blk.find(std::string("\n") + prefix + "");
    if (p == std::string::npos) return "";
    if (p) ++p;
    size_t e = blk.find('\n', p); return blk.substr(p, e - p);
}
static long long field(const std::string& l, const char* key) {
    size_t p = l.find(std::string(" ") + key + "="); if (p == std::string::npos) return -999999;
    return atoll(l.c_str() + p + strlen(key) + 2);
}
// 0.5.0 appends ` module=<name>` to the END of the #METRICS header: strip it before the older "ends with" checks.
static std::string noModule(const std::string& h) { size_t p = h.rfind(" module="); return p == std::string::npos ? h : h.substr(0, p); }
static bool endsWith(const std::string& s, const char* suf) { size_t n = strlen(suf); return s.size() >= n && !s.compare(s.size() - n, n, suf); }

static const unsigned kSeq[6] = { 0x8515, 0x8516, 0x8517, 0x8518, 0x8519, 0x851A };      // vanilla order of 0x436250
static const unsigned kEngine[6] = { 0x8516, 0x8515, 0x8517, 0x8518, 0x8519, 0x851A };   // order of 0x432FA0 (0x8516 first)

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "run";
    bool run = !strcmp(mode, "run"), metrics = !strcmp(mode, "metrics"), off = !strcmp(mode, "off"), bare = !strcmp(mode, "bare");
    if (!run && !metrics && !off && !bare) { printf("unknown mode %s\n", mode); return 2; }
    bool hw = run || metrics;
    FILE* cf = fopen("perflab_debug.txt", "w");
    if (!cf) return 2;
    fprintf(cf, "alloc=0\nflush_ms=1000\n");
    if (run) fprintf(cf, "hwmip=1\n");
    if (metrics) fprintf(cf, "hwmip=1\nmetrics=1\n");
    if (off) fprintf(cf, "metrics=1\n");
    fclose(cf);
    DeleteFileA("perflab_metrics.txt"); DeleteFileA("perflab_log.txt");

    HMODULE glu = GetModuleHandleA("glu32.dll");
    void* realGlu = glu ? (void*)GetProcAddress(glu, "gluBuild2DMipmaps") : nullptr;
    if (!realGlu) { printf("glu32 gluBuild2DMipmaps not resolvable\n"); return 2; }
    g_dll = LoadLibraryA("perflab.dll"); if (!g_dll) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    auto rglu = fn<void* (__cdecl*)()>("PerfLabTestRGluMip");
    fn<void (__cdecl*)(void**)>("PerfLabTestSetGl"); fn<int (__cdecl*)(long long*, int)>("PerfLabTestHwmipCounters"); fn<int (__cdecl*)(void**)>("PerfLabTestResolveEntries");
    Sleep(150);
    std::string log0 = fileText("perflab_log.txt");

    // ---- install / ordering ----
    check(log0.find("perf-lab 0.5.0 attach") != std::string::npos, "0.5.0 version in the attach line");
    check(log0.find(hw ? "hwmip=ON" : "hwmip=OFF") != std::string::npos, hw ? "attach line records hwmip=ON" : "attach line records hwmip=OFF");
    if (bare) {
        check(rglu() == nullptr, "bare: neither metrics nor hwmip -> GLU IAT slot untouched (rGluMip NULL)");
        check(fileText("perflab_log.txt").find("hwmip: gluBuild2DMipmaps IAT hook") == std::string::npos, "bare: no hwmip install line");
        printf("%s: %d failures\n", mode, g_fail); return g_fail ? 1 : 0;
    }
    // patchIat stores the original into rGluMip BEFORE it flips the slot, so it is non-NULL and the real GLU entry as soon as DllMain returned
    check(rglu() != nullptr && rglu() == realGlu, "rGluMip == the real glu32 entry right after attach (set before the IAT flip)");
    if (run) check(log0.find("hwmip: gluBuild2DMipmaps IAT hook ON") != std::string::npos, "metrics=0: hwmip installed the GLU IAT hook");
    if (metrics) check(log0.find("gluBuild2DMipmaps=1") != std::string::npos && log0.find("IAT hook ON") != std::string::npos, "metrics=1+hwmip=1: hook installed (once)");
    // real entry-point resolution (no GL calls): the four core entries come from opengl32.dll
    { void* r[5] = {0}; int ok = fn<int (__cdecl*)(void**)>("PerfLabTestResolveEntries")(r);
      check(ok && r[0] && r[1] && r[2] && r[3], "lazy resolver finds glTexImage2D/glTexParameteri/glGetError/glGetIntegerv in opengl32.dll");
      printf("INFO glGenerateMipmap via wglGetProcAddress without a context: %p (NULL expected outside a game)\n", r[4]); }

    if (off) {
        setGl(true, 2048);
        int r = mip(GL_TEXTURE_2D, 4, 64, 64);
        expectLog("off: eligible 2D call goes straight to GLU", "GLU de1 64x64");
        check(r == 0x1234, "off: GLU return value passes through");
        mip(GL_TEXTURE_2D, 4, 100, 64); mip(0x8515, 3, 16, 16, GL_RGB);
        check(cnt(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), "off: every hwmip counter stays 0");
        check(g_errCalls == 0, "off: no GL entry point is called");
        Sleep(100);
        ((void (__cdecl*)())GetProcAddress(g_dll, "PerfLabMetricsNow"))();
        for (int i = 0; i < 30 && lastBlock().find("#END") == std::string::npos; ++i) Sleep(100);
        std::string b = lastBlock(), hdr = b.substr(0, b.find('\n')), gl = lineOf(b, "GL ");
        check(endsWith(noModule(hdr), " hwmip=0") && hdr.find(" module=") != std::string::npos, "off: #METRICS header ends with hwmip=0 (then the 0.5.0 module= field)");
        check(field(gl, "mipmaps") == 3 && field(gl, "hwmip_calls") == 0 && field(gl, "hwmip_hw") == 0 && field(gl, "hwmip_cube_fallback") == 0, "off: GL line mipmaps=3, hwmip_* fields present and 0");
        check(fileText("perflab_log.txt").find("HWMIP ") == std::string::npos, "off: no HWMIP log lines");
        printf("%s: %d failures\n", mode, g_fail); return g_fail ? 1 : 0;
    }

    // ---- (a) 2D power of two: level 0 + glGenerateMipmap, no GLU ----
    setGl(true, 2048);
    int r = mip(GL_TEXTURE_2D, 4, 64, 32, GL_RGBA);
    expectLog("(a) 2D POT: glTexImage2D level 0 then glGenerateMipmap(0x0DE1), no GLU", (texLine(0xDE1, 64, 32, 4, GL_RGBA) + ";GEN de1").c_str());
    check(r == 0, "(a) HW path returns 0");
    check(cnt(1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0), "(a) counters calls=1 hw=1");
    mip(GL_TEXTURE_2D, 0x190A, 256, 256, 0x190A);   // the spec-LUT shape (comps = enum, passed through)
    check(g_log.find("TEX de1 0 256x256 if6410 f190a t1401;GEN de1") != std::string::npos, "(a) internalformat enum passed through unchanged");
    // stale error before the call is drained
    setGl(true, 2048); g_err = 0x500; mip(GL_TEXTURE_2D, 3, 8, 8, GL_RGB);
    check(cnt(1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0) && g_log.find("GLU") == std::string::npos, "(a) stale GL error drained before the upload (not mistaken for a failure)");

    // ---- (b) NPOT / zero size -> GLU ----
    setGl(true, 2048);
    r = mip(GL_TEXTURE_2D, 4, 100, 64);
    expectLog("(b) NPOT width: GLU only", "GLU de1 100x64");
    check(r == 0x1234, "(b) fallback returns GLU's value");
    mip(GL_TEXTURE_2D, 4, 64, 60); mip(GL_TEXTURE_2D, 4, 0, 0);
    check(cnt(3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0), "(b) npot counter 3 (NPOT height, zero size included), no HW");
    // ---- (b2) NULL data ----
    setGl(true, 2048); mip(GL_TEXTURE_2D, 4, 64, 64, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    expectLog("(b2) data == NULL: GLU only", "GLU de1 64x64");
    check(cnt(1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0), "(b2) hwmip_fallback_null counted (own counter)");

    // ---- (c) over max texture size ----
    setGl(true, 2048); mip(GL_TEXTURE_2D, 4, 4096, 64); mip(GL_TEXTURE_2D, 4, 64, 4096); mip(GL_TEXTURE_2D, 4, 2048, 2048);
    expectLog("(c) 4096 wide / 4096 high with max 2048: GLU; 2048x2048 allowed", ("GLU de1 4096x64;GLU de1 64x4096;" + texLine(0xDE1, 2048, 2048, 4, GL_RGBA) + ";GEN de1").c_str());
    check(cnt(3, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0), "(c) size counter 2, hw 1");

    // ---- (d) GL error after the upload -> GLU re-specifies ----
    setGl(true, 2048); g_failTex = 0xDE1; r = mip(GL_TEXTURE_2D, 4, 32, 32);
    expectLog("(d) injected error after glTexImage2D: GLU called afterwards", (texLine(0xDE1, 32, 32, 4, GL_RGBA) + ";GEN de1;GLU de1 32x32").c_str());
    check(r == 0x1234 && cnt(1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0), "(d) glerror counted, GLU value returned, hw not counted");
    setGl(true, 2048); g_failGen = true; mip(GL_TEXTURE_2D, 4, 32, 32);
    check(g_log == (texLine(0xDE1, 32, 32, 4, GL_RGBA) + ";GEN de1;GLU de1 32x32") && cnt(1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0), "(d) error from glGenerateMipmap also falls back to GLU");

    // ---- (e) cube: every face is self-contained: texparam(0x8513,0x8191,1), TexImage(face, level 0), texparam(..., 0); never glGenerateMipmap ----
    auto cubeWant = [](const unsigned* order, int n, int w = 16, int h = 16) {
        std::string x; for (int i = 0; i < n; ++i) { if (i) x += ";"; x += "PAR 8513 8191 1;" + texLine(order[i], w, h, 3, GL_RGB) + ";PAR 8513 8191 0"; } return x; };
    for (int gen = 1; gen >= 0; --gen) {   // with and without glGenerateMipmap: cube faces take the same path
        const char* tag = gen ? "(e) [glGenerateMipmap available]" : "(e) [glGenerateMipmap unavailable]";
        char name[160];
        setGl(gen != 0, 2048); cubeFaces(kSeq, 6);
        snprintf(name, sizeof name, "%s cube 0x8515..0x851A: per face flag on, level-0 upload, flag off; no glGenerateMipmap call", tag);
        expectLog(name, cubeWant(kSeq, 6).c_str());
        check(g_log.find("GEN") == std::string::npos && cnt(6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0), "(e) counters cube=6 hw=0 cube_fallback=0");
        setGl(gen != 0, 2048); cubeFaces(kEngine, 6);
        snprintf(name, sizeof name, "%s engine face order (0x8516 first, 0x432FA0): same per-face sequence", tag);
        expectLog(name, cubeWant(kEngine, 6).c_str());
        check(cnt(6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0), "(e) engine order counters cube=6");
    }
    setGl(true, 2048); cubeFaces(kSeq, 6); cubeFaces(kSeq, 6);
    check(g_log == (cubeWant(kSeq, 6) + ";" + cubeWant(kSeq, 6)) && cnt(12, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0), "(e) two consecutive sets / repeated faces: no cross-call state");

    // ---- (e2) a face that is NPOT / errors / ineligible: GLU for that face only, the other 5 faces HW ----
    setGl(true, 2048);
    for (int i = 0; i < 6; ++i) mip(kSeq[i], 3, i == 2 ? 20 : 16, i == 2 ? 20 : 16, GL_RGB);
    { std::string w; for (int i = 0; i < 6; ++i) { if (i) w += ";"; w += i == 2 ? "GLU 8517 20x20" : cubeWant(&kSeq[i], 1); }
      expectLog("(e2) NPOT face 3 (0x8517): GLU for that face only, the other 5 on HW", w.c_str()); }
    check(cnt(6, 0, 1, 0, 0, 0, 0, 0, 0, 5, 1), "(e2) counters cube=5 npot=1 cube_fallback=1");
    setGl(true, 2048); g_failTex = 0x8517; cubeFaces(kSeq, 6);
    { std::string w; for (int i = 0; i < 6; ++i) { if (i) w += ";"; w += cubeWant(&kSeq[i], 1); if (i == 2) w += ";GLU 8517 16x16"; }
      expectLog("(e2) GL error on face 3: that face re-specified by GLU right away, the others untouched", w.c_str()); }
    check(cnt(6, 0, 0, 0, 0, 0, 0, 0, 1, 5, 1), "(e2) counters cube=5 glerror=1 cube_fallback=1");
    setGl(true, 2048); mip(0x8515, 3, 24, 24, GL_RGB); for (int i = 1; i < 6; ++i) mip(kSeq[i], 3, 16, 16, GL_RGB);
    check(g_log.find("GLU 8515 24x24;") == 0 && cnt(6, 0, 1, 0, 0, 0, 0, 0, 0, 5, 1), "(e2) first face NPOT: only that face GLU, 5 HW");
    setGl(true, 2048); mip(0x8515, 3, 16, 16, GL_RGB, GL_UNSIGNED_BYTE, nullptr); mip(0x8516, 3, 16, 16, GL_RGB, 0x1403);
    check(g_log == "GLU 8515 16x16;GLU 8516 16x16" && cnt(2, 0, 0, 0, 1, 0, 1, 0, 0, 0, 2), "(e2) NULL data / wrong type on cube faces: GLU, cube_fallback counts every GLU face");
    // cube faces interleaved with 2D calls: every call self-contained
    setGl(true, 2048); mip(0x8515, 3, 16, 16, GL_RGB); mip(GL_TEXTURE_2D, 4, 8, 8); mip(0x8516, 3, 16, 16, GL_RGB); mip(GL_TEXTURE_2D, 4, 100, 8); mip(0x8515, 3, 16, 16, GL_RGB);
    expectLog("(e3) cube faces interleaved with 2D calls (and a repeated face): each call complete on its own",
              (cubeWant(&kSeq[0], 1) + ";" + texLine(0xDE1, 8, 8, 4, GL_RGBA) + ";GEN de1;" + cubeWant(&kSeq[1], 1) + ";GLU de1 100x8;" + cubeWant(&kSeq[0], 1)).c_str());
    check(cnt(5, 1, 1, 0, 0, 0, 0, 0, 0, 3, 0), "(e3) counters hw=1 (2D) cube=3 npot=1");

    // ---- (f) glGenerateMipmap unavailable: GL_GENERATE_MIPMAP texparam path ----
    setGl(false, 2048); r = mip(GL_TEXTURE_2D, 4, 64, 64);
    expectLog("(f) 2D without glGenerateMipmap: 0x8191=1 before the upload, 0x8191=0 after", ("PAR de1 8191 1;" + texLine(0xDE1, 64, 64, 4, GL_RGBA) + ";PAR de1 8191 0").c_str());
    check(r == 0 && cnt(1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0), "(f) 2D texparam path counted as hw (hwmip_hw = 2D uploads only), returns 0");
    setGl(false, 2048); g_failTex = 0xDE1; r = mip(GL_TEXTURE_2D, 4, 64, 64);
    expectLog("(f) 2D texparam path with an error: flag still reset, then GLU", ("PAR de1 8191 1;" + texLine(0xDE1, 64, 64, 4, GL_RGBA) + ";PAR de1 8191 0;GLU de1 64x64").c_str());
    check(r == 0x1234 && cnt(1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0), "(f) 2D texparam error counted");
    // (f) cube faces take the per-face texparam path with or without glGenerateMipmap: covered by (e) in both variants

    // ---- (g) hwmip=0 is covered by mode `off` ----
    // ---- (h) type / target mismatch ----
    setGl(true, 2048); mip(GL_TEXTURE_2D, 4, 64, 64, GL_RGBA, 0x1403);    // GL_UNSIGNED_SHORT
    expectLog("(h) type != GL_UNSIGNED_BYTE: GLU", "GLU de1 64x64");
    mip(0x806F, 4, 64, 64);                                               // GL_TEXTURE_3D
    mip(0x8513, 3, 16, 16);                                               // the cube enum itself is not a face
    check(cnt(3, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0), "(h) type=1 target=2 (3D, cube base enum)");

    // ---- counters concurrent with metrics=0, log output ----
    if (run) {
        check(!fileText("perflab_metrics.txt").size() || fileText("perflab_metrics.txt").find("#METRICS") == std::string::npos, "metrics=0: no #METRICS block is written");
        std::string lg; for (int i = 0; i < 50; ++i) { lg = fileText("perflab_log.txt"); if (lg.find("HWMIP FLUSH hwmip_calls=") != std::string::npos) break; Sleep(100); }
        check(lg.find("HWMIP FLUSH hwmip_calls=") != std::string::npos && lg.find("hwmip_hw=") != std::string::npos && lg.find("hwmip_cube_fallback=") != std::string::npos,
              "metrics=0: helper logs `HWMIP FLUSH hwmip_calls=.. hwmip_hw=..` at the flush");
        check(lg.find("HWMIP GL resolved:") == std::string::npos, "injected GL: no resolver event (resolver ran only in the game path)");
        check(lg.find("HWMIP first GL error") != std::string::npos, "once-only event line for the first GL error fallback");
    }
    if (metrics) {
        setGl(true, 2048); mip(GL_TEXTURE_2D, 4, 64, 64); mip(GL_TEXTURE_2D, 4, 100, 64); cubeFaces(kSeq, 6);
        counters(); long long snap[C_N]; memcpy(snap, g_c, sizeof snap);
        ((void (__cdecl*)())GetProcAddress(g_dll, "PerfLabMetricsNow"))();
        std::string b; for (int i = 0; i < 40; ++i) { b = lastBlock(); if (b.find("#END") != std::string::npos && b.find(" MANUAL ") != std::string::npos) break; Sleep(100); }
        std::string hdr = b.substr(0, b.find('\n')), gl = lineOf(b, "GL ");
        check(endsWith(noModule(hdr), " g1skip=0 hwmip=1") || endsWith(noModule(hdr), " hwmip=1"), "#METRICS header ends with hwmip=1 (then the 0.5.0 module= field)");
        check(hdr.find("tx=0 g1skip=0 hwmip=1") != std::string::npos, "header keeps its existing fields in order (tx= g1skip=) before hwmip=");
        check(gl.find("GL glimage=") == 0 && gl.find(" mip_mpix=") != std::string::npos && gl.find(" mip_mpix=") < gl.find(" hwmip_calls="), "GL line keeps its existing fields; hwmip_* appended after mip_mpix");
        check(field(gl, "hwmip_calls") == snap[C_CALLS] && field(gl, "hwmip_hw") == snap[C_HW] && field(gl, "hwmip_fallback_npot") == snap[C_NPOT] &&
              field(gl, "hwmip_cube") == snap[C_CUBE] && field(gl, "hwmip_glerror") == snap[C_GLERR] && field(gl, "hwmip_cube_fallback") == snap[C_CUBEFB] &&
              field(gl, "hwmip_fallback_size") == 0 && field(gl, "hwmip_fallback_type") == 0 && field(gl, "hwmip_fallback_target") == 0, "GL line hwmip_* values equal the counters");
        check(field(gl, "hwmip_calls") == 8 && field(gl, "hwmip_hw") == 1 && field(gl, "hwmip_cube") == 6, "GL line values calls=8 hw=1 cube=6 cube_fallback=0");
        { size_t p = gl.rfind(" hwmip_"); check(p != std::string::npos && gl.compare(p, 21, " hwmip_cube_fallback=") == 0, "hwmip_cube_fallback is the last GL field"); }
        check(field(gl, "mipmaps") == g_nmip, "mipmaps= counts every call (HW and GLU paths timed)");
        check(fileText("perflab_log.txt").find("HWMIP MANUAL hwmip_calls=") != std::string::npos, "HWMIP summary logged at each metrics write");
    }
    printf("%s: %d failures\n", mode, g_fail);
    return g_fail ? 1 : 0;
}
