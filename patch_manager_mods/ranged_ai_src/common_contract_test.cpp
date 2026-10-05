// common_contract_test.cpp -- Patches/Common CONTRACT test for Ranged AI.
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod (doc 20 section 4.1; rule of 2026-10-03).
// It pins the semantics THIS mod relies on, not the header's own behaviour (that is _shared_src/saferead_test.cpp). Every message starts with
// "ranged_ai relies on:" so a failure names the consumer and the reason. Built against ../_shared_src (see test.toml), so ANY edit to
// SafeRead.h re-runs this test: a Common change that would break the mod fails here first.
//
// Unreadable memory: the host stub's VirtualQuery reports everything as one committed RW region, so (as in _shared_src/saferead_test.cpp)
// SafeRead.h's VirtualQuery is routed through a wrapper that can mark a range NOACCESS / not committed. Host-only, deterministic.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct FakeRange { uintptr_t lo, hi; DWORD protect, state; };
static FakeRange g_fake[4];
static int g_fakeN = 0;
static auto g_realVQ = &VirtualQuery;
static SIZE_T fakeVQ(LPCVOID a, MEMORY_BASIC_INFORMATION* m, SIZE_T n) {
    SIZE_T r = g_realVQ(a, m, n);
    if (!r) return r;
    uintptr_t p = reinterpret_cast<uintptr_t>(a);
    for (int i = 0; i < g_fakeN; ++i) {
        const FakeRange& f = g_fake[i];
        if (p >= f.lo && p < f.hi) { m->BaseAddress = reinterpret_cast<void*>(f.lo); m->RegionSize = f.hi - f.lo; m->Protect = f.protect; m->State = f.state; return r; }
        if (p < f.lo && p + m->RegionSize > f.lo) m->RegionSize = f.lo - p;   // end the default region where a fake one starts
    }
    return r;
}
#define VirtualQuery fakeVQ
#include "SafeRead.h"
#include "check.h"
#include "KpLog.h"
#include <string>
#include <unistd.h>

KP_LOG_PATCH("ranged-ai");
KP_CHANNEL(chMain, "main");

static uint8_t* g_arena = nullptr;
constexpr size_t ArenaSize = 0x10000, Page = 0x1000;
// arena layout: pages 0-1 readable data, page 2 NOACCESS (fake), page 3 not committed (fake), pages 4+ readable
static const uintptr_t kBad = 2 * Page, kFree = 3 * Page;
static uintptr_t A(size_t off) { return reinterpret_cast<uintptr_t>(g_arena) + off; }
template <typename T> static void put(size_t off, T v) { memcpy(g_arena + off, &v, sizeof v); }
static void setup() {
    g_fakeN = 0;
    g_fake[g_fakeN++] = { A(kBad), A(kBad) + Page, PAGE_NOACCESS, MEM_COMMIT };
    g_fake[g_fakeN++] = { A(kFree), A(kFree) + Page, 0, 0 };   // MEM_FREE
    saferead::beginScope();
}


#define HCHECK(why, expr) ::kt::Check("ranged_ai relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("ranged_ai relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

// RangedAI.cpp:73-77 rd<T>: `T v = def; readAt<T>(1, base, off, &v); return v;` -- the default survives every failure.
template <typename T> static T rd(int base, int off, T def = T()) {
    T v = def; saferead::readAt<T>(1, static_cast<uintptr_t>(base), off, &v); return v;
}
// RangedAI.cpp:153,160: 12-byte position struct read in one readAt, `V3 v = {0,0,0}` kept on failure.
struct V3 { float x, y, z; };
static V3 pos(int obj, int off) { V3 v = {0, 0, 0}; saferead::readAt<V3>(2, static_cast<uintptr_t>(obj), off, &v); return v; }
static int I(uintptr_t a) { return static_cast<int>(a); }   // the mod holds 32-bit engine addresses in int (arena is MAP_32BIT)

static void readat_contract() {
    setup();
    put<int>(0x100 + 0x04, 0x1234); put<float>(0x100 + 0x08, 6.5f); put<unsigned char>(0x100 + 0x0C, 0xAB);
    put<float>(0x200 + 0x94, 1.5f); put<float>(0x200 + 0x98, -2.25f); put<float>(0x200 + 0x9C, 3.0f);
    HEQ("rd<int> returns the stored value on readable memory (RangedAI.cpp:73-77)", rd<int>(I(A(0x100)), 0x04), 0x1234);
    HCHECK("rd<float> returns the stored value (RangedAI.cpp:73-77)", rd<float>(I(A(0x100)), 0x08) == 6.5f);
    HEQ("rd<unsigned char> reads exactly 1 byte (RangedAI.cpp:73-77)", rd<unsigned char>(I(A(0x100)), 0x0C), 0xABu);
    HEQ("rd<int> default T() == 0 on base 0", rd<int>(0, 4), 0);
    HEQ("rd<int> keeps the caller's default on base 0", rd<int>(0, 4, -1), -1);
    HEQ("rd<int> keeps the default below 0x10000", rd<int>(0x20, 0, -1), -1);
    HEQ("rd<int> keeps the default on a NOACCESS page", rd<int>(I(A(kBad)), 0, -1), -1);
    HEQ("rd<int> keeps the default on an uncommitted page", rd<int>(I(A(kFree)), 0, -1), -1);
    int out = 0x5A5A;
    HCHECK("readAt returns false on base 0 and leaves *out (RangedAI.cpp:75)", !saferead::readAt<int>(1, 0, 4, &out) && out == 0x5A5A);
    HCHECK("readAt returns false below 0x10000 and leaves *out", !saferead::readAt<int>(1, 0xFFF0, 0, &out) && out == 0x5A5A);
    HCHECK("readAt returns false on NOACCESS memory and leaves *out", !saferead::readAt<int>(1, A(kBad), 8, &out) && out == 0x5A5A);
    HCHECK("readAt<int> straddling the readable page end into NOACCESS is false", !saferead::readAt<int>(1, A(kBad - 2), 0, &out) && out == 0x5A5A);
    HCHECK("readAt<int> ending exactly at the page end is true", saferead::readAt<int>(1, A(kBad - 4), 0, &out));
}

static void v3_contract() {
    setup();
    put<float>(0x200 + 0x94, 1.5f); put<float>(0x200 + 0x98, -2.25f); put<float>(0x200 + 0x9C, 3.0f);
    static_assert(sizeof(V3) == 12, "ranged_ai relies on: V3 is 12 bytes (RangedAI.cpp:153)");
    V3 v = pos(I(A(0x200)), 0x94);
    HCHECK("readAt<V3> delivers all three floats in one 12-byte read (RangedAI.cpp:160)", v.x == 1.5f && v.y == -2.25f && v.z == 3.0f);
    V3 w = pos(0, 0x94);
    HCHECK("readAt<V3> on base 0 keeps {0,0,0} (RangedAI.cpp:160)", w.x == 0 && w.y == 0 && w.z == 0);
    w = pos(0x20, 0x94);
    HCHECK("readAt<V3> below 0x10000 keeps {0,0,0}", w.x == 0 && w.y == 0 && w.z == 0);
    w = pos(I(A(kBad)), 0x94 - 0x94);
    HCHECK("readAt<V3> on NOACCESS memory keeps {0,0,0}", w.x == 0 && w.y == 0 && w.z == 0);
    V3 s = {7, 8, 9};
    HCHECK("readAt<V3> false leaves *out whole (no partial copy)", !saferead::readAt<V3>(2, A(kBad), 0, &s) && s.x == 7 && s.y == 8 && s.z == 9);
    // 12-byte struct whose last float falls into the NOACCESS page: all-or-nothing
    s = {7, 8, 9};
    HCHECK("readAt<V3> whose tail crosses into NOACCESS is false and leaves *out untouched (all-or-nothing)", !saferead::readAt<V3>(2, A(kBad - 8), 0, &s) && s.x == 7 && s.y == 8 && s.z == 9);
    HCHECK("readAt<V3> ending exactly at the page end is true", saferead::readAt<V3>(2, A(kBad - 12), 0, &s));
}

static void probe_contract() {
    setup();
    auto P = [](unsigned s, uintptr_t a, size_t n) { return saferead::probeRead(s, reinterpret_cast<const void*>(a), n); };
    HCHECK("probeRead sizeof(int)/sizeof(float) true on readable memory (wr<T>, RangedAI.cpp:79)", P(2, A(0x100), sizeof(int)) && P(2, A(0x100), sizeof(float)));
    HCHECK("probeRead false on null (wr<T> guards `!addr ||` but the API must agree, RangedAI.cpp:79)", !P(2, 0, 4));
    HCHECK("probeRead false below 0x10000 (RangedAI.cpp:79)", !P(2, 0x8, 4));
    HCHECK("probeRead false on a NOACCESS page (RangedAI.cpp:79)", !P(2, A(kBad), 4));
    HCHECK("probeRead false on an uncommitted page (freed memory)", !P(2, A(kFree), 4));
    HCHECK("probeRead straddling readable -> NOACCESS is false (wr<T> must not write a half-mapped field)", !P(2, A(kBad) - 2, 4));
    HCHECK("probeRead of the last 4 readable bytes is true", P(2, A(kBad) - 4, 4));
    HCHECK("probeRead false when pointer + len wraps the address space", !P(2, ~uintptr_t(0) - 2, 4));
}

static void scope_contract() {
    setup();
    int v = 0;
    HCHECK("beginScope then read works (every hook entry: RangedAI.cpp:173,219,241,...)", (saferead::beginScope(), saferead::readAt<int>(1, A(0x100), 0x04, &v)));
    bool ok = true;
    for (int i = 0; i < 6; ++i) { saferead::beginScope(); saferead::beginScope(); ok = ok && saferead::readAt<int>(1, A(0x100), 0x04, &v) && saferead::probeRead(2, reinterpret_cast<void*>(A(0x100)), 4); }
    HCHECK("repeated / nested beginScope calls leave reads and probes working", ok);
    setup();
    saferead::probeRead(2, reinterpret_cast<void*>(A(0x100)), 4);           // warm the cache for the readable region
    g_fake[0] = { A(0), A(0x2000), PAGE_NOACCESS, MEM_COMMIT };              // engine 'frees' it between hook calls
    HCHECK("without beginScope the cached region still answers (why every hook entry must call beginScope)", saferead::probeRead(2, reinterpret_cast<void*>(A(0x100)), 4));
    saferead::beginScope();
    HCHECK("after beginScope the cache is dropped and the freed region now fails (RangedAI.cpp:173 etc.)", !saferead::probeRead(2, reinterpret_cast<void*>(A(0x100)), 4));
    setup();
}

// RangedAI.cpp log section: static-init fallback sink (path + level only, no I/O). An INFO line lands in the fallback file with the
// "<ms> ranged-ai/main " prefix and the text unchanged; WARN carries the WARN tag (S2 bad frame / not writable).
static void kplog_contract() {
    const char* td = getenv("TMPDIR"); if (!td || !*td) td = "/tmp";
    char path[300]; snprintf(path, sizeof path, "%s/ranged_ai_contract_%d.txt", td, (int)getpid());
    remove(path);
    HCHECK("KpLog_HasCore() is false before KPatchInit (old-core path: emit from the first loadIni / log call)", !KpLog_HasCore());
    KpLog_SetFallbackFile(path); KpLog_SetFallbackLevel(KPLOG_INFO);
    HCHECK("KpLog_SetFallbackFile only stores the path, no I/O (safe from a static initialiser)", access(path, F_OK) != 0);
    HCHECK("fallback level INFO enables INFO on the main channel (every ranged-ai line is INFO or WARN)", KP_ENABLED(chMain, KPLOG_INFO));
    KP_LOGV(chMain, KPLOG_INFO, "%s", "ranged-ai 0.4.0 loaded; cwd=x pid=1 tick=1");
    std::string t; if (FILE* f = fopen(path, "r")) { char b[512]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) t.append(b, n); fclose(f); }
    size_t sp = t.find(' ');
    HCHECK("fallback line starts with a decimal tick count", sp != std::string::npos && sp > 0 && t.find_first_not_of("0123456789") == sp);
    HCHECK("fallback line is \"<ms> ranged-ai/main \" then the text unchanged", t.substr(sp) == " ranged-ai/main ranged-ai 0.4.0 loaded; cwd=x pid=1 tick=1\n");
    KP_LOGV(chMain, KPLOG_WARN, "%s", "S2 bad frame probe");
    std::string u; if (FILE* f = fopen(path, "r")) { char b[512]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) u.append(b, n); fclose(f); }
    HCHECK("a WARN line carries the WARN tag", u.find(" ranged-ai/main WARN S2 bad frame probe\n") != std::string::npos);
    KpLog_SetFallbackFile(nullptr);
    remove(path);
}

int main() {
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS
#ifdef MAP_32BIT
                   | MAP_32BIT
#endif
                   , -1, 0);
    if (m == MAP_FAILED) { perror("mmap"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    readat_contract();
    v3_contract();
    probe_contract();
    scope_contract();
    kplog_contract();
    return kt::Report("common_contract_test (ranged_ai)");
}
