// common_contract_test.cpp -- Patches/Common CONTRACT test for Companion AI.
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod (doc 20 section 4.1; rule of 2026-10-03).
// It pins the semantics THIS mod relies on, not the header's own behaviour (that is _shared_src/saferead_test.cpp). Every message starts with
// "companion_ai relies on:" so a failure names the consumer and the reason. Built against ../_shared_src (see test.toml), so ANY edit to
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
#include "ModConfigDecl.h"
#include "CompanionAiLogic.h"
#include "check.h"

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


#define HCHECK(why, expr) ::kt::Check("companion_ai relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("companion_ai relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

// CompanionAI.cpp:96-99 rd<T>: `T v = def; readAt<T>(1, base, off, &v); return v;` -- the def survives every failure.
template <typename T> static T rd(int base, int off, T def = T()) {
    T v = def; saferead::readAt<T>(1, static_cast<uintptr_t>(base), off, &v); return v;
}
static int I(uintptr_t a) { return static_cast<int>(a); }   // the mod holds 32-bit engine addresses in int (arena is MAP_32BIT)

static void readat_contract() {
    setup();
    put<int>(0x100 + 0x04, 0x1234);       // ObId
    put<int>(0x100 + 0x154, -77);         // row +0x154 (CompanionAI.cpp:1066)
    put<float>(0x100 + 0x94, 1.5f);       // CrPos x
    put<float>(0x100 + 0x98, -2.25f);     // CrPos y
    put<float>(0x100 + 0x9C, 3.0f);       // CrPos z
    int id = 0;
    HCHECK("readAt<int> returns true and delivers the value on readable memory (CompanionAI.cpp:320,879,883)", saferead::readAt<int>(5, A(0x100), 0x04, &id) && id == 0x1234);
    int v = 0;
    HCHECK("readAt<int> at a large offset (row+0x154) delivers the value (CompanionAI.cpp:1066)", saferead::readAt<int>(8, A(0x100), 0x154, &v) && v == -77);
    float x = 0, y = 0;
    HCHECK("readAt<float> delivers x/y of a creature position (CompanionAI.cpp:906-909)", saferead::readAt<float>(9, A(0x100), 0x94, &x) && saferead::readAt<float>(9, A(0x100), 0x94 + 4, &y) && x == 1.5f && y == -2.25f);
    HCHECK("readAt<float> in a CrPos + 4*i loop reads x,y,z in order (CompanionAI.cpp:1170-1171)", [&] { float a[3] = {}; for (int i = 0; i < 3; ++i) if (!saferead::readAt<float>(10, A(0x100), 0x94 + 4 * i, &a[i])) return false; return a[0] == 1.5f && a[1] == -2.25f && a[2] == 3.0f; }());
    // generic list walk: node{+4 next, +8 data}, data{+0 unsigned short type}
    put<int>(0x200 + 8, I(A(0x300))); put<int>(0x200 + 4, I(A(0x240))); put<unsigned short>(0x300, 0x0104);
    int data = 0, next = 0; unsigned short ty = 0;
    HCHECK("readAt<int>+readAt<unsigned short> walk a node->data->type chain (CompanionAI.cpp:981-983)",
           saferead::readAt<int>(6, A(0x200), 8, &data) && data && saferead::readAt<unsigned short>(6, static_cast<uintptr_t>(data), 0, &ty) && saferead::readAt<int>(6, A(0x200), 4, &next) && ty == 0x0104 && next == I(A(0x240)));
    HEQ("readAt<unsigned short> reads exactly 2 bytes (neighbour byte untouched)", static_cast<unsigned long long>(ty), 0x0104ull);
    // rd<T> wrapper
    HEQ("rd<int> returns the stored value on readable memory (CompanionAI.cpp:96-99)", rd<int>(I(A(0x100)), 0x04), 0x1234);
    HCHECK("rd<float> returns the stored value (CompanionAI.cpp:96-99)", rd<float>(I(A(0x100)), 0x98) == -2.25f);
    HEQ("rd<int> default T() == 0 on base 0 (CompanionAI.cpp:96-99)", rd<int>(0, 4), 0);
    HEQ("rd<int> keeps the caller's default on base 0 (e.g. rd<int>(obj, ObTag+4, -1), CompanionAI.cpp:1226)", rd<int>(0, 4, -1), -1);
    HEQ("rd<int> keeps the default below 0x10000", rd<int>(0x20, 0, -1), -1);
    HEQ("rd<int> keeps the default on a NOACCESS page", rd<int>(I(A(kBad)), 0, -1), -1);
    HEQ("rd<int> keeps the default on an uncommitted page", rd<int>(I(A(kFree)), 0, -1), -1);
    HCHECK("rd<float> keeps the default on unreadable memory", rd<float>(I(A(kBad)), 0, 9.5f) == 9.5f);
    // failure semantics
    int out = 0x5A5A;
    HCHECK("readAt returns false on base 0 (CompanionAI.cpp:879 null attacker/queue)", !saferead::readAt<int>(5, 0, 4, &out));
    HEQ("readAt leaves *out untouched on base 0 (rd/controlledId pre-seed it)", out, 0x5A5A);
    HCHECK("readAt returns false below 0x10000 (small-int 'pointers' in engine fields)", !saferead::readAt<int>(5, 0x20, 0, &out) && out == 0x5A5A);
    HCHECK("readAt returns false just below the 0x10000 low-address limit (0xFFF0)", !saferead::readAt<int>(5, 0xFFF0, 0, &out) && out == 0x5A5A);
    HCHECK("readAt returns false on a NOACCESS page and leaves *out (CompanionAI.cpp:883,981)", !saferead::readAt<int>(6, A(kBad), 8, &out) && out == 0x5A5A);
    HCHECK("readAt returns false on an uncommitted page and leaves *out", !saferead::readAt<int>(6, A(kFree), 0, &out) && out == 0x5A5A);
    float fo = 7.f;
    HCHECK("readAt<float> false on unreadable memory leaves *out (CompanionAI.cpp:906: ax/ay pre-seeded)", !saferead::readAt<float>(9, A(kBad), 0x10, &fo) && fo == 7.f);
    unsigned short so = 0xBEEF;
    HCHECK("readAt<unsigned short> false on unreadable memory leaves *out (CompanionAI.cpp:982)", !saferead::readAt<unsigned short>(6, A(kBad), 0, &so) && so == 0xBEEF);
    // straddle: the value spans readable -> NOACCESS
    HCHECK("readAt<int> straddling the end of the readable page into NOACCESS is false (partial value never delivered)", !saferead::readAt<int>(5, A(kBad - 2), 0, &out) && out == 0x5A5A);
    HCHECK("readAt<int> ending exactly at the page end is true", saferead::readAt<int>(5, A(kBad - 4), 0, &out));
    HCHECK("readAt<int> via base+off crossing from readable into NOACCESS (field offset pushes it over) is false", !saferead::readAt<int>(5, A(kBad - 0x20), 0x1E, &out));
}

static void scope_contract() {
    setup();
    int v = 0;
    HCHECK("beginScope then read works (every hook entry: CompanionAI.cpp:587,700,737,...)", (saferead::beginScope(), saferead::readAt<int>(1, A(0x100), 0x154, &v)));
    bool ok = true;
    for (int i = 0; i < 6; ++i) { saferead::beginScope(); saferead::beginScope(); ok = ok && saferead::readAt<int>(1, A(0x100), 0x04, &v) && saferead::probeRead(2, reinterpret_cast<void*>(A(0x100)), 4); }
    HCHECK("repeated / nested beginScope calls (hooks re-enter: CompanionAI.cpp:856,873) leave reads working", ok);
    // cache is per scope: a read warms it, beginScope must drop it so freed memory is re-queried
    saferead::beginScope();
    HCHECK("a NOACCESS page fails within a scope", !saferead::probeRead(3, reinterpret_cast<void*>(A(kBad + 0x10)), 4));
    g_fakeN = 0;   // 'engine freed/remapped': the page becomes readable between hook calls
    HCHECK("a stale (unreadable) answer is never cached: next read in the same scope sees the new state", saferead::probeRead(3, reinterpret_cast<void*>(A(kBad + 0x10)), 4));
    setup();
    saferead::probeRead(3, reinterpret_cast<void*>(A(0x100)), 4);          // warm the cache for the readable region
    g_fake[0] = { A(0), A(0x2000), PAGE_NOACCESS, MEM_COMMIT };             // engine 'frees' it between hook calls
    HCHECK("without beginScope the cached region still answers (why every hook entry must call beginScope)", saferead::probeRead(3, reinterpret_cast<void*>(A(0x100)), 4));
    saferead::beginScope();
    HCHECK("after beginScope the cache is dropped and the freed region now fails (CompanionAI.cpp:587 etc.)", !saferead::probeRead(3, reinterpret_cast<void*>(A(0x100)), 4));
    setup();
}

static void probe_contract() {
    setup();
    auto P = [](unsigned s, uintptr_t a, size_t n) { return saferead::probeRead(s, reinterpret_cast<const void*>(a), n); };
    HCHECK("probeRead sizeof(int)=4 true on readable memory (wr<int>, CompanionAI.cpp:102)", P(2, A(0x100), sizeof(int)));
    HCHECK("probeRead sizeof(float)=4 / sizeof(short)=2 / sizeof(char)=1 true (wr<T>, CompanionAI.cpp:102)", P(2, A(0x100), sizeof(float)) && P(2, A(0x100), sizeof(short)) && P(2, A(0x100), 1));
    HCHECK("probeRead len 0 is true even for a null pointer (copyStr n computed from len, CompanionAI.cpp:113)", P(3, 0, 0) && P(3, A(kBad), 0));
    HCHECK("probeRead len 4 on controlledId's inner+0x270 is true on readable memory (CompanionAI.cpp:312)", P(4, A(0x100) + 0x70, 4));
    HCHECK("probeRead 0x28 (client list header) is true on readable memory (CompanionAI.cpp:314)", P(4, A(0x400), 0x28));
    HCHECK("probeRead 0x100 (player client) is true on readable memory (CompanionAI.cpp:317)", P(4, A(0x400), 0x100));
    HCHECK("probeRead FollowInfoSize=0x3C is true on readable memory (CompanionAI.cpp:1024,1036)", P(7, A(0x400), 0x3C));
    HCHECK("probeRead cnt*4 for cnt in 1..4096 is true on readable memory (CompanionAI.cpp:1224)", P(11, A(0x4000), 4) && P(11, A(0x4000), 4096ull * 4));
    HCHECK("probeRead false on null (callers pass `if (!addr || ...)` but the API must agree)", !P(2, 0, 4));
    HCHECK("probeRead false below 0x10000 (CompanionAI.cpp:102,312,314)", !P(2, 0x8, 4) && !P(2, 0xFFF0, 4));
    HCHECK("probeRead false on a NOACCESS page for each length the mod uses (4, 0x28, 0x100, 0x3C)", !P(4, A(kBad), 4) && !P(4, A(kBad), 0x28) && !P(4, A(kBad), 0x100) && !P(7, A(kBad), 0x3C));
    HCHECK("probeRead false on an uncommitted page (freed memory)", !P(4, A(kFree), 4));
    HCHECK("probeRead false when pointer + len wraps the address space", !P(4, ~uintptr_t(0) - 2, 16));
    // crossing: readable prefix -> unreadable tail
    uintptr_t p = A(kBad) - 8; size_t n = 0x20;
    HCHECK("probeRead over a range crossing from readable into NOACCESS is false (copyStr first attempt, CompanionAI.cpp:113)", !P(3, p, n));
    // copyStr's retry (lines 114-115): toPage = distance to the next 4K boundary
    uintptr_t toPage = ((p | 0xFFFu) + 1u) - p;
    HEQ("page-clamp arithmetic: bytes to the page end are 8 here", static_cast<unsigned long long>(toPage), 8ull);
    HCHECK("probeRead of the readable prefix up to the page end is true (the copyStr page-clamp retry, CompanionAI.cpp:115)", P(3, p, toPage));
    HCHECK("probeRead of one byte past the page end is false (the clamp must be exact)", !P(3, p, toPage + 1));
    HCHECK("probeRead of the last byte of the readable page is true", P(3, A(kBad) - 1, 1));
    HCHECK("probeRead starting exactly at the NOACCESS page is false even for 1 byte", !P(3, A(kBad), 1));
    // multi-region walk: readable -> readable across a region boundary created by a fake range of the same kind
    g_fake[g_fakeN++] = { A(Page), A(2 * Page), PAGE_READONLY, MEM_COMMIT };
    saferead::beginScope();
    HCHECK("probeRead spanning two adjacent readable regions (RW then READONLY) is true (regionReadable walks regions, CompanionAI.cpp:1224)", P(11, A(Page) - 0x10, 0x20));
    g_fake[g_fakeN - 1] = { A(Page), A(2 * Page), PAGE_GUARD | PAGE_READWRITE, MEM_COMMIT };
    saferead::beginScope();
    HCHECK("probeRead refuses a PAGE_GUARD page (guard pages are never tripped)", !P(11, A(Page) + 0x10, 4));
    setup();
}

// CompanionAI.cpp copyStr (107-110): `saferead::readExoLoose(3, p, len, out, cap); cai::sanitize(out);` -- the helper only copies; sanitizing stays in the mod.
static void exoloose_contract() {
    setup();
    char out[16];
    memcpy(g_arena + 0x600, "TagValue\0junk", 13);
    memset(out, 'x', sizeof out);
    int n = saferead::readExoLoose(3, static_cast<uint32_t>(A(0x600)), 8, out, sizeof out);
    HCHECK("readExoLoose copies exactly len bytes and NUL-terminates (CompanionAI.cpp:107 copyStr)", n == 8 && !strcmp(out, "TagValue"));
    n = saferead::readExoLoose(3, static_cast<uint32_t>(A(0x600)), 8, out, 5);
    HCHECK("readExoLoose truncates to cap-1 bytes and NUL-terminates (c.script is char[33], tags are char[cap])", n == 4 && !strcmp(out, "TagV"));
    n = saferead::readExoLoose(3, static_cast<uint32_t>(A(0x600)), -1, out, 7);
    HCHECK("readExoLoose with len -1 (unknown, rd default) copies up to cap-1 bytes", n == 6 && !strcmp(out, "TagVal"));
    n = saferead::readExoLoose(3, 0, 8, out, sizeof out);
    HCHECK("readExoLoose gives \"\" on p == 0 (readTag then falls back to \"-\", CompanionAI.cpp:112)", n == 0 && out[0] == 0);
    n = saferead::readExoLoose(3, static_cast<uint32_t>(A(0x600)), 0, out, sizeof out);
    HCHECK("readExoLoose gives \"\" for len 0", n == 0 && out[0] == 0);
    n = saferead::readExoLoose(3, static_cast<uint32_t>(A(0x600)), 8, out, 1);
    HCHECK("readExoLoose gives \"\" for cap < 2", n == 0 && out[0] == 0);
    n = saferead::readExoLoose(3, static_cast<uint32_t>(A(kBad)), 8, out, sizeof out);
    HCHECK("readExoLoose gives \"?\" (-1) on an unreadable page", n == -1 && !strcmp(out, "?"));
    n = saferead::readExoLoose(3, static_cast<uint32_t>(A(kFree)), 8, out, sizeof out);
    HCHECK("readExoLoose gives \"?\" (-1) on an uncommitted page", n == -1 && !strcmp(out, "?"));
    // page-end fallback: 8 readable bytes then the NOACCESS page; a 12-byte request falls back to the 8 bytes up to the page end
    memcpy(g_arena + kBad - 8, "ABCDEFGH", 8);
    char big[32]; memset(big, 'x', sizeof big);
    n = saferead::readExoLoose(3, static_cast<uint32_t>(A(kBad - 8)), 12, big, sizeof big);
    HCHECK("readExoLoose falls back to the bytes up to the page end when the rest is unreadable (CompanionAI.cpp:114-115 semantics)", n == 8 && !strcmp(big, "ABCDEFGH"));
    n = saferead::readExoLoose(3, static_cast<uint32_t>(A(kBad - 8)), 8, big, sizeof big);
    HCHECK("readExoLoose reads a string ending exactly at the page end in full", n == 8 && !strcmp(big, "ABCDEFGH"));
    // the helper does NOT sanitize: control bytes pass through (cai::sanitize in copyStr handles them)
    memcpy(g_arena + 0x700, "a\x01\x7f" "b\xe9", 5);
    n = saferead::readExoLoose(3, static_cast<uint32_t>(A(0x700)), 5, out, sizeof out);
    HCHECK("readExoLoose does NOT sanitize: control / high bytes are copied verbatim (copyStr must still call cai::sanitize)", n == 5 && !memcmp(out, "a\x01\x7f" "b\xe9", 6));
    setup();
}

// CompanionAiLogic.h kPageSchema (Options -> Mod Configuration): parses, every row is an ini-bound [CompanionAI] toggle whose default is the
// shipped Config default, and the bound keys are ones parseConfig reads (a key written by the page must change g_cfg after loadIni).
static void moddecl_contract() {
    mcdecl::PageDecl pd{};
    char err[96] = {};
    HCHECK("kPageSchema parses with ModConfigDecl (CompanionAI.cpp pollModConfig)", mcdecl::parseSchema(cai::kPageSchema, strlen(cai::kPageSchema), pd, err, sizeof err));
    HEQ("kPageSchema has 11 rows", pd.nRows, 11);
    HCHECK("page name is Companion AI", !strcmp(pd.name, "Companion AI"));
    const cai::Config d;
    struct { const char* key; bool def; } want[] = {
        {"cast_family_guard", d.castFamilyGuard}, {"cast_queue_newest", d.castQueueNewest}, {"a1", d.a1}, {"a1_retarget", d.a1Retarget},
        {"fm1", d.fm1}, {"fm4", d.fm4 != 0}, {"c1", d.c1}, {"a3", d.a3}, {"trace", d.trace}, {"log", d.log}, {"debug_log", d.debugLog}};
    for (const auto& w : want) {
        const mcdecl::RowDecl* r = nullptr;
        for (int i = 0; i < pd.nRows; ++i) if (!strcmp(pd.rows[i].key, w.key)) r = &pd.rows[i];
        HCHECK("every page key has a row", r != nullptr);
        if (!r) continue;
        HCHECK("each row is a toggle bound to companion_ai.ini [CompanionAI]",
               r->kind == mcdecl::K_TOGGLE && !strcmp(r->ini, "companion_ai.ini") && !strcmp(r->section, "CompanionAI"));
        HEQ("row default = shipped Config default", r->def, w.def ? 1 : 0);
        char text[64];
        snprintf(text, sizeof text, "[CompanionAI]\n%s=%d\n", w.key, w.def ? 0 : 1);
        cai::Config c;
        static cai::IniTable t;
        t.parse(text, "CompanionAI");
        cai::parseConfig(t, c);
        HCHECK("a page write of the non-default value changes the parsed Config (loadIni applies it)", !cai::sameConfig(c, d));
    }
}

int main() {
    moddecl_contract();
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS
#ifdef MAP_32BIT
                   | MAP_32BIT
#endif
                   , -1, 0);
    if (m == MAP_FAILED) { perror("mmap"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    readat_contract();
    scope_contract();
    probe_contract();
    exoloose_contract();
    return kt::Report("common_contract_test (companion_ai)");
}
