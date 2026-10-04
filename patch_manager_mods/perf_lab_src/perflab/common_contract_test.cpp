// common_contract_test.cpp -- Patches/Common CONTRACT test for Perf Lab (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on, not the header's own behaviour (that is _shared_src/saferead_test.cpp). Every message starts with
// "perf_lab relies on:" so a failure names the consumer and the reason. Built against ../../_shared_src (see test.toml), so ANY
// edit to SafeRead.h re-runs this test: a Common change that would break Perf Lab fails here first.
//
// Mod use (PerfLab.cpp): beginScope at every hook/snapshot entry; readAt<T> with T = uint16_t, uint32_t, int, unsigned, uint8_t,
// char, float, uintptr_t; probeRead(site, ptr, 16) for the CResRef copy. The mod never uses dump / enableRing / probeWrite / readExo.
// uintptr_t is 8 bytes on this host and 4 on the target, so only size-independent facts are pinned for it.
// Host-only, deterministic.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
// Route SafeRead.h's VirtualQuery through a wrapper that can mark one range unreadable (the stub reports everything readable).
static auto g_realVQ = &VirtualQuery;
static uintptr_t g_badLo = 0, g_badHi = 0;
static SIZE_T fakeVQ(LPCVOID a, MEMORY_BASIC_INFORMATION* m, SIZE_T n) {
    SIZE_T r = g_realVQ(a, m, n);
    if (!r) return r;
    uintptr_t p = reinterpret_cast<uintptr_t>(a);
    if (g_badHi > g_badLo) {
        if (p >= g_badLo && p < g_badHi) { m->BaseAddress = reinterpret_cast<void*>(g_badLo); m->RegionSize = g_badHi - g_badLo; m->State = MEM_COMMIT; m->Protect = PAGE_NOACCESS; }
        else if (p < g_badLo && p + m->RegionSize > g_badLo) m->RegionSize = g_badLo - p;
    }
    return r;
}
#define VirtualQuery fakeVQ
#include "SafeRead.h"
#include "check.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define HCHECK(why, expr) ::kt::Check("perf_lab relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("perf_lab relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))
#define U(x) static_cast<unsigned long long>(x)

constexpr size_t ArenaSize = 1u << 20;   // 1 MiB: room for the +0x2000C / +0x20000 net ring offsets
static uint8_t* g_arena = nullptr;
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

// PerfLab.cpp readName (413): partial-read loop over 32-bit words; returns words delivered (0 = whole read failed -> false).
static int readNameWords(uintptr_t src, int words, uint32_t* w) {
    saferead::beginScope();
    int got = 0;
    for (int i = 0; i < words; ++i) { if (!saferead::readAt(50, src, i * 4, &w[i])) { if (!i) return -1; break; } ++got; }
    return got;
}
// PerfLab.cpp copyExo (522-527): CExoString {p, len}, char-at-a-time until NUL or failure.
static int copyExoLike(uintptr_t exo, char* out, int cap) {
    out[0] = 0;
    uint32_t p = 0; int len = 0;
    saferead::beginScope();
    if (!exo || !saferead::readAt(63, exo, 0, &p) || !saferead::readAt(64, exo, 4, &len) || !p || len <= 0) return 0;
    int n = len < cap - 1 ? len : cap - 1, k = 0;
    for (; k < n; ++k) {
        char ch = 0;
        if (!saferead::readAt(65, p, k, &ch) || !ch) break;
        out[k] = ch;
    }
    out[k] = 0;
    return k;
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
    uintptr_t a = reinterpret_cast<uintptr_t>(g_arena);

    // ---- beginScope (399, 412, 522, 549, 861, 921, 1127, 1376, 1398, 1418, 1426, 1466, 1495, 1516, 1526)
    uint32_t v32 = 0; at<uint32_t>(a, 0x10) = 0xCAFEF00Du;
    bool rep = true; for (int i = 0; i < 15; ++i) { saferead::beginScope(); v32 = 0; rep = rep && saferead::readAt(1, a, 0x10, &v32) && v32 == 0xCAFEF00Du; }
    HCHECK("beginScope is callable at every hook entry (15 sites, repeatedly) and reads still work (PerfLab.cpp:399)", rep);

    // ---- readAt with deduced T: value + sizeof(T) bytes
    at<uint32_t>(a, 0x20) = 0x11223344u;
    uint16_t t16 = 0; uint8_t e4 = 0; int i32 = 0; unsigned u32 = 0; char ch = 0;
    HCHECK("readAt<uint16_t> delivers a CRes type word (restypeOf ke+0x1A, :400)", saferead::readAt(20, a, 0x20, &t16) && t16 == 0x3344);
    HCHECK("readAt<uint8_t> delivers one byte (requeue e4 flag, :1427)", saferead::readAt(53, a, 0x20, &e4) && e4 == 0x44);
    HCHECK("readAt<char> delivers one byte (name / module-name loops, :527,:557)", saferead::readAt(65, a, 0x23, &ch) && ch == 0x11);
    at<int>(a, 0x24) = -5;
    HCHECK("readAt<int> delivers a signed value (aiMaster counts, ModalCount, :1517,:1131)", saferead::readAt(57, a, 0x24, &i32) && i32 == -5);
    at<unsigned>(a, 0x28) = 0x80000001u;
    HCHECK("readAt<unsigned> delivers an unsigned value (ring read index rd, :1496)", saferead::readAt(56, a, 0x28, &u32) && u32 == 0x80000001u);
    float fl = 0; at<float>(a, 0x2C) = 30.5f;
    HCHECK("readAt<float> delivers the frame-limit global (writeGlobals, :863)", saferead::readAt(30, a, 0x2C, &fl) && fl == 30.5f);
    at<uint32_t>(a, 0x30) = 0xAABBCCDDu;
    uint32_t w = 0; saferead::readAt(1, a, 0x30, &w);
    HEQ("readAt<uint32_t> copies exactly 4 bytes (a 32-bit pointer field read into uint32_t, :523)", U(w), 0xAABBCCDDull);
    uintptr_t up = 0; at<uintptr_t>(a, 0x40) = static_cast<uintptr_t>(0x00ABCDEFu);
    HCHECK("readAt<uintptr_t> returns the stored pointer-sized value (CRes pointer chain, :1399,:1419)", saferead::readAt(51, a, 0x40, &up) && up == 0x00ABCDEFu);

    // ---- failures: false + *out untouched (pre-seeded defaults: uintptr_t cres = 0; int size = 0; unsigned rd = 0)
    uintptr_t cres = 0x1234; int size = 77; unsigned rd = 88; uint8_t e4s = 9; uint16_t ts = 0x4242;
    HCHECK("readAt false on base 0 (ke / resman / res null check, :400,:1399)", !saferead::readAt(51, 0, 0x28, &cres));
    HEQ("readAt base 0 leaves *out untouched (cres pre-seeded, :1398)", U(cres), 0x1234ull);
    HCHECK("readAt false below 0x10000 (a bogus small 'pointer', :1419)", !saferead::readAt(52, 0x18, 0, &size));
    HEQ("readAt below 0x10000 leaves *out untouched (size pre-seeded 0, :1419)", (long long)size, 77ll);
    HCHECK("readAt false when base+off is still below 0x10000", !saferead::readAt(56, 0x100, 0x20, &rd));
    HEQ("readAt low base+off leaves *out untouched (rd, :1496)", U(rd), 88ull);
    HCHECK("readAt false when base is 0 even if off puts the address above 0x10000 (base 0 is refused, not offset math)", !saferead::readAt(53, 0, 0x20000, &e4s));
    HEQ("readAt base-0 failure leaves uint8_t untouched (:1427)", U(e4s), 9ull);
    HCHECK("readAt<uint16_t> false on base 0 (restypeOf returns RtUnknown, :400)", !saferead::readAt(20, 0, 0x1A, &ts) && ts == 0x4242);

    // ---- large offsets (1496: obj+0x2000C, then peer+0x20000)
    uintptr_t obj = a + 0x1000;
    uintptr_t peer2 = a;   // peer+0x20000 must land inside the arena
    at<uint32_t>(obj, 0x2000C) = static_cast<uint32_t>(peer2);
    at<unsigned>(peer2, 0x20000) = 0x00ABCDEFu;
    uint32_t peer = 0; unsigned rdv = 0;
    HCHECK("readAt base+off arithmetic with off 0x2000C (net ring peer, :1496)", saferead::readAt(55, obj, 0x2000C, &peer) && peer == static_cast<uint32_t>(peer2));
    HCHECK("readAt chained base+off with off 0x20000 (ring read index, :1496)", saferead::readAt(56, static_cast<uintptr_t>(peer), 0x20000, &rdv) && rdv == 0x00ABCDEFu);
    HEQ("the 0x20000/0x2000C offsets do not wrap or truncate (value read from the exact address)", U(rdv), U(at<unsigned>(a, 0x20000)));
    unsigned rdb = 123;
    g_badLo = a + 0x20000; g_badHi = a + 0x21000; saferead::beginScope();
    HCHECK("readAt false when base+off lands in an unreadable page (peer+0x20000 hole, :1496)", !saferead::readAt(56, a, 0x20000, &rdb));
    HEQ("readAt hole leaves rd untouched so newW > rd+0x10000 uses the seeded 0", U(rdb), 123ull);
    g_badLo = g_badHi = 0; saferead::beginScope();

    // ---- partial read loop (readName 413)
    for (int i = 0; i < 6; ++i) at<uint32_t>(a, 0x100 + 4 * i) = 0x61616161u + i;
    uint32_t nw[6];
    HEQ("readName loop reads all words on readable memory", (long long)readNameWords(a + 0x100, 6, nw), 6ll);
    HCHECK("readName words hold the data in order", nw[0] == 0x61616161u && nw[5] == 0x61616166u);
    g_badLo = a + 0x110; g_badHi = a + 0x1000; saferead::beginScope();
    memset(nw, 0, sizeof nw);
    HEQ("readName: a later word failing keeps the earlier words (4 of 6)", (long long)readNameWords(a + 0x100, 6, nw), 4ll);
    HCHECK("readName: earlier words delivered, failed words untouched (0)", nw[3] == 0x61616164u && nw[4] == 0 && nw[5] == 0);
    HEQ("readName: first word failing -> whole read fails (returns false, :413)", (long long)readNameWords(a + 0x110, 6, nw), -1ll);
    HEQ("readName: src 0 -> first word fails too", (long long)readNameWords(0, 6, nw), -1ll);
    g_badLo = g_badHi = 0; saferead::beginScope();

    // ---- probeRead(62, ke, 16) (509)
    saferead::beginScope();
    HCHECK("probeRead true for a readable 16-byte CResRef", saferead::probeRead(62, reinterpret_cast<const void*>(a + 0x200), 16));
    HCHECK("probeRead false for ke below 0x10000", !saferead::probeRead(62, reinterpret_cast<const void*>(0x1000), 16));
    HCHECK("probeRead false for a null ke", !saferead::probeRead(62, nullptr, 16));
    g_badLo = a + 0x2F8; g_badHi = a + 0x1000; saferead::beginScope();
    HCHECK("probeRead false when the 16 bytes straddle into unreadable memory (the whole span, not just the start)", !saferead::probeRead(62, reinterpret_cast<const void*>(a + 0x2F4), 16));
    HCHECK("probeRead true when the 16 bytes end exactly at the hole", saferead::probeRead(62, reinterpret_cast<const void*>(a + 0x2E8), 16));
    g_badLo = g_badHi = 0; saferead::beginScope();

    // ---- exo-string loop (522-527)
    const char txt[] = "hello\0junk";
    uintptr_t data = a + 0x400, exo = a + 0x440;
    memcpy(reinterpret_cast<void*>(data), txt, sizeof txt);
    at<uint32_t>(exo, 0) = static_cast<uint32_t>(data); at<int>(exo, 4) = 10;
    char out[32];
    HEQ("exo loop stops at the NUL (len 10 but text 'hello')", (long long)copyExoLike(exo, out, sizeof out), 5ll);
    HEQ("exo loop copy is the text", out, "hello");
    at<int>(exo, 4) = 3;
    HEQ("exo loop honours len when shorter than the text", (long long)copyExoLike(exo, out, sizeof out), 3ll);
    at<int>(exo, 4) = 10;
    uintptr_t data2 = a + 0x7FC; memcpy(reinterpret_cast<void*>(data2), "abcd", 4);
    at<uint32_t>(exo, 0) = static_cast<uint32_t>(data2); at<int>(exo, 4) = 8;
    g_badLo = a + 0x800; g_badHi = a + 0x1000; saferead::beginScope();
    HEQ("exo loop: chars before an unreadable page are kept (abcd), then the loop stops cleanly on the failed read", (long long)copyExoLike(exo, out, sizeof out), 4ll);
    HEQ("exo loop text before the hole", out, "abcd");
    g_badLo = g_badHi = 0; saferead::beginScope();
    at<uint32_t>(exo, 0) = 0;
    HEQ("exo loop: null char* -> nothing copied", (long long)copyExoLike(exo, out, sizeof out), 0ll);
    at<uint32_t>(exo, 0) = static_cast<uint32_t>(data); at<int>(exo, 4) = 0;
    HEQ("exo loop: len <= 0 -> nothing copied", (long long)copyExoLike(exo, out, sizeof out), 0ll);
    HEQ("exo loop: exo pointer 0 -> nothing copied", (long long)copyExoLike(0, out, sizeof out), 0ll);
    return kt::Report("common_contract_test (perf_lab)");
}
