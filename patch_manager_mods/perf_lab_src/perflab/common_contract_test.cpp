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
// CodePatch.h (codepatch::): writeChecked (applyTx, applyExperiments), putRel32 (TX cave), patchIat (heap/SwapBuffers/GLU/GMS hooks),
// pinned through injected codepatch::ops() fakes and a hand-built PE32 image.
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
#include "CodePatch.h"
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

// ---- codepatch (CodePatch.h) fakes: every Win32 call goes through codepatch::ops(), replaced here by recorders ------------------
namespace cpfake {
struct ProtCall { void* p; SIZE_T n; DWORD newProt; uint8_t firstByte; int seq; };
struct FlushCall { const void* p; SIZE_T n; int seq; };
static ProtCall prot[8]; static int protN = 0;
static FlushCall flushed[8]; static int flushN = 0;
static int seq = 0;
static bool protFail = false;
static DWORD oldProt = PAGE_EXECUTE_READ;   // what the fake reports as the previous protection
static BOOL protect(LPVOID p, SIZE_T n, DWORD np, PDWORD old) {
    if (protN < 8) prot[protN] = { p, n, np, *static_cast<uint8_t*>(p), seq };
    ++protN; ++seq;
    if (protFail) return 0;
    *old = oldProt;
    return 1;
}
static void flush(const void* p, SIZE_T n) { if (flushN < 8) flushed[flushN] = { p, n, seq }; ++flushN; ++seq; }
static SIZE_T query(LPCVOID, MEMORY_BASIC_INFORMATION*, SIZE_T) { return 0; }
static void reset() { protN = flushN = seq = 0; protFail = false; oldProt = PAGE_EXECUTE_READ; }
}

// ---- hand-built PE32 image for patchIat (offsets by hand, independent of CodePatch.h's structs) -----------------------------
// desc0 = decoy dll importing the same function names (slot values 0x5555000i); desc1 = the mod's dll: slot 0 is an ordinal import,
// slot i+1 is fn[i] (value 0x2222000i). The two descriptors share the name strings, so a wrong-dll match would swap the decoy.
static uint8_t g_img[0x1000] __attribute__((aligned(16)));
static void put32(uint32_t off, uint32_t v) { memcpy(g_img + off, &v, 4); }
static void putStr(uint32_t off, const char* s) { memcpy(g_img + off, s, strlen(s) + 1); }
static uint32_t get32(uint32_t off) { uint32_t v; memcpy(&v, g_img + off, 4); return v; }
constexpr uint32_t E_LFANEW = 0x80, DIR_IMPORT = E_LFANEW + 24 + 96 + 8;      // PE sig(4)+file hdr(20); data dirs at opt+96; dir 1 = import
constexpr uint32_t DESC_D = 0x200, DESC_G = DESC_D + 20;
constexpr uint32_t OFT_D = 0x400, OFT_G = 0x440, FT_D = 0x500, FT_G = 0x540, NAMES = 0x600;
constexpr uint32_t N_FN = 3;
static const char* const FN[N_FN] = { "HeapAlloc", "SwapBuffers", "HeapFree" };
static void buildImage(const char* dll, const char* decoy, bool withImportDir) {
    memset(g_img, 0, sizeof g_img);
    put32(0x3C, E_LFANEW);
    memcpy(g_img + E_LFANEW, "PE\0\0", 4);
    if (withImportDir) { put32(DIR_IMPORT, DESC_D); put32(DIR_IMPORT + 4, 60); }
    putStr(0x300, decoy); putStr(0x320, dll);
    for (uint32_t i = 0; i < N_FN; ++i) putStr(NAMES + i * 0x20 + 2, FN[i]);                   // hint(2) + name
    put32(DESC_D + 0, OFT_D); put32(DESC_D + 12, 0x300); put32(DESC_D + 16, FT_D);
    put32(DESC_G + 0, OFT_G); put32(DESC_G + 12, 0x320); put32(DESC_G + 16, FT_G);
    put32(OFT_G, 0x80000000u | 16); put32(FT_G, 0x11110000u);
    for (uint32_t i = 0; i < N_FN; ++i) {
        put32(OFT_D + i * 4, NAMES + i * 0x20); put32(FT_D + i * 4, 0x55550000u + i);
        put32(OFT_G + (i + 1) * 4, NAMES + i * 0x20); put32(FT_G + (i + 1) * 4, 0x22220000u + i);
    }                                                                                           // terminators stay zero
}
static uint32_t slotG(uint32_t i) { return FT_G + (i + 1) * 4; }
static uint32_t slotD(uint32_t i) { return FT_D + i * 4; }

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

    // ---- readResRef16 (PerfLab.cpp readResRef: RR_LOWER | RR_SPACE_BAD | RR_EMPTY_Q, site 62)
    {
        const unsigned F = saferead::RR_LOWER | saferead::RR_SPACE_BAD | saferead::RR_EMPTY_Q;
        char o[17]; uintptr_t rr = a + 0x500;
        memset(reinterpret_cast<void*>(rr), 0, 16); memcpy(reinterpret_cast<void*>(rr), "N_Hk_Door01", 11);
        saferead::beginScope();
        HCHECK("readResRef16 reads a readable CResRef and lower-cases A-Z (TPCTOP names are lower-case)", saferead::readResRef16(62, rr, o, F) && !strcmp(o, "n_hk_door01"));
        memset(reinterpret_cast<void*>(rr), 0, 16); memcpy(reinterpret_cast<void*>(rr), "a b\x01" "c\xe9", 6);
        HCHECK("readResRef16 maps space, control and high bytes to '?' with RR_SPACE_BAD", saferead::readResRef16(62, rr, o, F) && !strcmp(o, "a?b?c?"));
        memset(reinterpret_cast<void*>(rr), 'Z', 16);
        HCHECK("readResRef16 handles a full 16-byte name with no NUL (17th byte terminated)", saferead::readResRef16(62, rr, o, F) && strlen(o) == 16 && o[0] == 'z' && o[15] == 'z');
        memset(reinterpret_cast<void*>(rr), 0, 16);
        HCHECK("readResRef16 turns an empty name into \"?\" with RR_EMPTY_Q (readable, returns true)", saferead::readResRef16(62, rr, o, F) && !strcmp(o, "?"));
        memset(o, 'x', sizeof o);
        HCHECK("readResRef16 gives \"?\" and false for a null address", !saferead::readResRef16(62, 0, o, F) && !strcmp(o, "?"));
        memset(o, 'x', sizeof o);
        HCHECK("readResRef16 gives \"?\" and false below 0x10000", !saferead::readResRef16(62, 0x1000, o, F) && !strcmp(o, "?"));
        g_badLo = a + 0x2F8; g_badHi = a + 0x1000; saferead::beginScope();
        memset(o, 'x', sizeof o);
        HCHECK("readResRef16 gives \"?\" and false when the 16 bytes straddle into unreadable memory", !saferead::readResRef16(62, a + 0x2F4, o, F) && !strcmp(o, "?"));
        g_badLo = g_badHi = 0; saferead::beginScope();
    }

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
    // ---- codepatch::writeChecked / putRel32: applyTx and applyExperiments (PerfLab.cpp)
    {
        codepatch::Ops savedOps = codepatch::ops();
        codepatch::ops() = { cpfake::query, cpfake::protect, cpfake::flush };
        // TX patch shape: 7 bytes 0F B6 C8 85 C9 75 10 -> E9 rel32 90 90; the byte after the site is not part of the write
        uint8_t code[8] = { 0x0F, 0xB6, 0xC8, 0x85, 0xC9, 0x75, 0x10, 0x55 };
        const uint8_t orig[7] = { 0x0F, 0xB6, 0xC8, 0x85, 0xC9, 0x75, 0x10 };
        const uint8_t jmp[7] = { 0xE9, 0x11, 0x22, 0x33, 0x44, 0x90, 0x90 };
        uint8_t seen[7]; memset(seen, 0xEE, sizeof seen);
        cpfake::reset();
        HEQ("writeChecked returns 1 when the site holds the expected bytes (g_txState = 1)", codepatch::writeChecked(reinterpret_cast<uintptr_t>(code), orig, jmp, 7, seen), 1);
        HCHECK("writeChecked fills seen with the bytes found before the write (g_txSeen is logged)", memcmp(seen, orig, 7) == 0);
        HCHECK("writeChecked writes exactly n bytes (the byte after the site is untouched)", memcmp(code, jmp, 7) == 0 && code[7] == 0x55);
        HEQ("writeChecked does exactly two protect calls (RWX, then restore)", cpfake::protN, 2);
        HCHECK("writeChecked makes the site RWX first", cpfake::prot[0].p == code && cpfake::prot[0].n == 7 && cpfake::prot[0].newProt == PAGE_EXECUTE_READWRITE);
        HEQ("writeChecked restores the previous protection", cpfake::prot[1].newProt, static_cast<DWORD>(PAGE_EXECUTE_READ));
        HCHECK("writeChecked flushes the instruction cache once, over the patched bytes", cpfake::flushN == 1 && cpfake::flushed[0].p == code && cpfake::flushed[0].n == 7);
        HCHECK("writeChecked flushes after the protection is restored", cpfake::flushed[0].seq > cpfake::prot[1].seq);

        // mismatch (-1): `code` now holds jmp, not orig
        uint8_t found[8]; memcpy(found, code, 8);
        cpfake::reset(); memset(seen, 0xEE, sizeof seen);
        HEQ("writeChecked returns -1 on a byte mismatch (g_*State = -1)", codepatch::writeChecked(reinterpret_cast<uintptr_t>(code), orig, jmp, 7, seen), -1);
        HCHECK("writeChecked fills seen BEFORE the compare, so a mismatch leaves the found bytes (the log prints them)", memcmp(seen, found, 7) == 0);
        HCHECK("writeChecked leaves memory untouched on mismatch", memcmp(code, found, 8) == 0);
        HEQ("writeChecked touches neither protection nor the icache on mismatch", cpfake::protN + cpfake::flushN, 0);
        uint8_t last[7]; memcpy(last, orig, 7); last[6] ^= 1;
        cpfake::reset();
        HEQ("writeChecked compares all n bytes (a last-byte difference is a mismatch)", codepatch::writeChecked(reinterpret_cast<uintptr_t>(last), orig, jmp, 7, seen), -1);

        // protect failure (-2)
        uint8_t c3[7]; memcpy(c3, orig, 7);
        cpfake::reset(); cpfake::protFail = true; memset(seen, 0xEE, sizeof seen);
        HEQ("writeChecked returns -2 when protect fails (g_*State = -2; the mod's own -3 stays caller-side)", codepatch::writeChecked(reinterpret_cast<uintptr_t>(c3), orig, jmp, 7, seen), -2);
        HCHECK("writeChecked has already filled seen on a protect failure", memcmp(seen, orig, 7) == 0);
        HCHECK("writeChecked leaves the bytes untouched on a protect failure", memcmp(c3, orig, 7) == 0);
        HEQ("writeChecked tries protect once and never flushes on a protect failure", cpfake::protN * 10 + cpfake::flushN, 10);

        // G1 patch shape: 2 bytes 74 06 -> EB 06
        uint8_t g1[3] = { 0x74, 0x06, 0xAA };
        const uint8_t g1Orig[2] = { 0x74, 0x06 }, g1New[2] = { 0xEB, 0x06 };
        uint8_t g1Seen[2]; cpfake::reset();
        HEQ("writeChecked handles the 2-byte g1skip patch", codepatch::writeChecked(reinterpret_cast<uintptr_t>(g1), g1Orig, g1New, 2, g1Seen), 1);
        HCHECK("the 2-byte patch lands and stops at n", g1[0] == 0xEB && g1[1] == 0x06 && g1[2] == 0xAA);
        codepatch::ops() = savedOps;

        // putRel32 as the TX cave builds it: field staged at cave+17/+22, displacement relative to the field's own site
        uint8_t cave[32]; memset(cave, 0xCC, sizeof cave);
        const uintptr_t caveBase = 0x01230000, txFree = 0x004261D6, txSkip = 0x004261E6;   // 32-bit target addresses
        cave[16] = 0xE9; codepatch::putRel32(cave + 17, caveBase + 17, txFree);
        cave[21] = 0xE9; codepatch::putRel32(cave + 22, caveBase + 22, txSkip);
        int32_t d1, d2; memcpy(&d1, cave + 17, 4); memcpy(&d2, cave + 22, 4);
        HEQ("putRel32 writes target - (fieldSite + 4) into the field (TxFree jump)", static_cast<long long>(d1), static_cast<long long>(static_cast<int32_t>(txFree - (caveBase + 17 + 4))));
        HEQ("putRel32 writes target - (fieldSite + 4) into the field (TxSkip jump)", static_cast<long long>(d2), static_cast<long long>(static_cast<int32_t>(txSkip - (caveBase + 22 + 4))));
        HCHECK("putRel32 writes exactly the 4 field bytes (opcode bytes and neighbours untouched)", cave[16] == 0xE9 && cave[21] == 0xE9 && cave[26] == 0xCC && cave[15] == 0xCC);
        uint8_t f[4]; codepatch::putRel32(f, 0x1000, 0x1100);
        HCHECK("putRel32 forward displacement: 0x1100 - 0x1004 = 0xFC", f[0] == 0xFC && f[1] == 0 && f[2] == 0 && f[3] == 0);
        codepatch::putRel32(f, 0x2000, 0x1000);
        HCHECK("putRel32 backward displacement is negative two's complement (-0x1004)", f[0] == 0xFC && f[1] == 0xEF && f[2] == 0xFF && f[3] == 0xFF);
    }

    // ---- codepatch::patchIat: the heap, SwapBuffers, gluBuild2DMipmaps and GlobalMemoryStatus hooks (PerfLab.cpp installGluHook, init)
    {
        codepatch::Ops savedOps = codepatch::ops();
        codepatch::ops() = { cpfake::query, cpfake::protect, cpfake::flush };
        buildImage("GDI32.dll", "KERNEL32.dll", true);
        HMODULE mod = g_img;
        void* orig = nullptr;
        void* const hook = reinterpret_cast<void*>(static_cast<uintptr_t>(0x12345678));
        cpfake::reset();
        HCHECK("patchIat swaps the named IAT slot, matching the dll name case-insensitively (\"gdi32.DLL\")", codepatch::patchIat(mod, "gdi32.DLL", "SwapBuffers", hook, &orig));
        HEQ("patchIat writes the hook into the named slot", get32(slotG(1)), 0x12345678u);
        HEQ("patchIat returns the old slot value in *orig (rSwap / rGluMip / rGms are called through it)", (unsigned long long)reinterpret_cast<uintptr_t>(orig), 0x22220001ull);
        HCHECK("patchIat leaves neighbouring slots and the ordinal entry alone", get32(slotG(0)) == 0x22220000u && get32(slotG(2)) == 0x22220002u && get32(FT_G) == 0x11110000u);
        HEQ("patchIat does not touch the other dll's same-named slot", get32(slotD(1)), 0x55550001u);
        HCHECK("patchIat opens just the 4-byte slot RW and restores the previous protection", cpfake::protN == 2 && cpfake::prot[0].n == 4 && cpfake::prot[0].newProt == PAGE_READWRITE && cpfake::prot[1].newProt == PAGE_EXECUTE_READ);
        void* o2 = reinterpret_cast<void*>(0x1);
        HCHECK("patchIat is false for a function the dll does not import, *orig untouched", !codepatch::patchIat(mod, "GDI32.dll", "NotImported", hook, &o2) && o2 == reinterpret_cast<void*>(0x1));
        HCHECK("patchIat is false for a dll the exe does not import (g_gluState = -1 path)", !codepatch::patchIat(mod, "GLU32.dll", "SwapBuffers", hook, &o2));
        cpfake::reset(); cpfake::protFail = true;
        HCHECK("patchIat is false when protect fails, *orig untouched", !codepatch::patchIat(mod, "GDI32.dll", FN[2], hook, &o2) && o2 == reinterpret_cast<void*>(0x1));
        HEQ("patchIat leaves the slot untouched when protect fails", get32(slotG(2)), 0x22220002u);
        cpfake::protFail = false;
        buildImage("GDI32.dll", "KERNEL32.dll", false);
        HCHECK("patchIat is false for an image without an import directory", !codepatch::patchIat(mod, "GDI32.dll", "SwapBuffers", hook, &o2));
        codepatch::ops() = savedOps;
    }
    return kt::Report("common_contract_test (perf_lab)");
}
