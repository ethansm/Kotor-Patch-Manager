// common_contract_test.cpp -- Patches/Common CONTRACT test for Inventory Sort & Filter (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on, not the header's own behaviour (that is _shared_src/saferead_test.cpp). Every message starts with
// "inventory_sort_filter relies on:" so a failure names the consumer and the reason. Built against ../_shared_src (see test.toml),
// so ANY edit to SafeRead.h re-runs this test: a Common change that would break the mod fails here first.
//
// Mod use (InventorySortFilter.cpp): beginScope at the four hook entries; rd<T> = readAt<T> with a pre-seeded default (T = int,
// unsigned char, unsigned, unsigned short); wr32 = probeRead(addr, 4); dumpMem = readAt<int> per word. Host-only, deterministic.
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
#include "KpLog.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define HCHECK(why, expr) ::kt::Check("inventory_sort_filter relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("inventory_sort_filter relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))
KP_LOG_PATCH("inventory-sort-filter");
KP_CHANNEL(chMain, "main");

#define U(x) static_cast<unsigned long long>(x)

static uint8_t* g_arena = nullptr; constexpr size_t ArenaSize = 1u << 16;
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

// The mod's own wrappers, verbatim (InventorySortFilter.cpp:81-90 and 92-103), built only from saferead calls.
template <typename T> static T rd(int base, int off, T def = T()) {
    T v = def;
    saferead::readAt<T>(1, static_cast<uintptr_t>(base), off, &v);
    return v;
}
static bool wr32(int addr, int v) {
    if (!saferead::probeRead(2, reinterpret_cast<const void*>(static_cast<intptr_t>(addr)), 4)) return false;
    *reinterpret_cast<int*>(addr) = v;
    return true;
}
static int dumpWords(int addr, int bytes, bool* got) {   // returns words read; got[k] per word (mirrors dumpMem's per-word readAt<int>)
    int n = 0;
    for (int j = 0; j < bytes; j += 4) {
        int v = 0;
        got[j / 4] = saferead::readAt<int>(3, static_cast<uintptr_t>(addr), j, &v);
        if (got[j / 4]) ++n;
    }
    return n;
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
    const int ib = static_cast<int>(a);   // the mod holds 32-bit addresses in ints

    // ---- beginScope at every hook entry (272/294/352/381)
    saferead::beginScope();
    at<int>(a, 0x10) = 0x12345678; at<unsigned char>(a, 0x20) = 0x7B; at<unsigned>(a, 0x24) = 0xDEADBEEFu; at<unsigned short>(a, 0x28) = 0xBEEF;
    bool rep = true; for (int i = 0; i < 4; ++i) { saferead::beginScope(); rep = rep && rd<int>(ib, 0x10, -1) == 0x12345678; }
    HCHECK("beginScope is callable at each hook entry (4 sites) and reads still work afterwards (InventorySortFilter.cpp:272)", rep);

    // ---- rd<T> (83): value on readable memory for each T the mod uses
    HEQ("rd<int> delivers the value (panel/item/ebp field reads, :134,:145,:222)", (long long)rd<int>(ib, 0x10), 0x12345678ll);
    HEQ("rd<unsigned char> delivers one byte (FilterModeOff / BaseGenderReqOff, :139,:193)", U(rd<unsigned char>(ib, 0x20, 0xFF)), 0x7Bull);
    HEQ("rd<unsigned> delivers 32 bits (BaseSlotMaskOff, :189)", U(rd<unsigned>(ib, 0x24)), 0xDEADBEEFull);
    HEQ("rd<unsigned short> delivers 16 bits (gender fields, :195)", U(rd<unsigned short>(ib, 0x28)), 0xBEEFull);
    at<unsigned>(a, 0x30) = 0x11223344u;
    HEQ("readAt<unsigned char> reads sizeof(T)=1 byte only (low byte of a little-endian word)", U(rd<unsigned char>(ib, 0x30)), 0x44ull);
    HEQ("readAt<unsigned short> reads sizeof(T)=2 bytes only", U(rd<unsigned short>(ib, 0x30)), 0x3344ull);
    at<int>(a, 0x50) = 11; at<int>(a, 0x54) = 22; at<int>(a, 0x58) = 33;
    HEQ("readAt honours base+off (slot array walk base + 4*i, :179,:223)", (long long)(rd<int>(ib + 0x50, 8) * 100 + rd<int>(ib, 0x50 + 4)), 3322ll);

    // ---- failure keeps the pre-seeded default (the mod relies on this to fail open / fail unknown)
    HEQ("rd<int> on base 0 returns the seeded default (:274 TabClickedOff, -1)", (long long)rd<int>(0, 0x10, -1), -1ll);
    HEQ("rd<unsigned char> on base 0 returns 0xFF = unknown filter mode, never a cycle mode (:139)", U(rd<unsigned char>(0, 0x20, 0xFF)), 0xFFull);
    HEQ("rd<int> below 0x10000 returns the seeded default (-1 = open upgrade slot, fail-open, :179)", (long long)rd<int>(0x20, 0, -1), -1ll);
    HEQ("rd<int> failure with no default is 0 (:134 holder chain)", (long long)rd<int>(0, 0), 0ll);
    HEQ("rd<unsigned> on base 0 is 0 (:189)", U(rd<unsigned>(0, 0)), 0ull);
    int v = 0x55555555;
    HCHECK("readAt returns false on base 0", !saferead::readAt<int>(1, 0, 4, &v));
    HEQ("readAt failure leaves *out untouched", (long long)v, 0x55555555ll);
    HCHECK("readAt returns false below 0x10000", !saferead::readAt<int>(1, 0x8000, 0, &v));
    HEQ("readAt failure below 0x10000 leaves *out untouched", (long long)v, 0x55555555ll);

    // ---- unreadable memory (page-sized hole in the arena)
    g_badLo = a + 0x1000; g_badHi = a + 0x2000; saferead::beginScope();
    at<int>(a, 0x0FFC) = 0x0BADF00D;
    HEQ("rd<int> on an unreadable page returns the seeded default", (long long)rd<int>(ib + 0x1000, 0, -1), -1ll);
    HEQ("rd<int> just below an unreadable page still reads", (long long)rd<int>(ib, 0x0FFC), 0x0BADF00Dll);
    HCHECK("readAt that straddles into the unreadable page fails (4-byte read at -2 of the hole)", !saferead::readAt<int>(1, a, 0x0FFE, &v));
    HEQ("straddling readAt leaves *out untouched", (long long)v, 0x55555555ll);

    // ---- wr32 (87): probeRead(addr, 4) gate
    at<int>(a, 0x40) = 0;
    HCHECK("wr32 writes through when the 4 bytes are readable (:87, ebp+F_SelRow patch)", wr32(ib + 0x40, 0x7777) && at<int>(a, 0x40) == 0x7777);
    HCHECK("wr32 refuses an unreadable target and does not write", !wr32(ib + 0x1000, 0x1234) && at<int>(a, 0x1000) == 0);
    HCHECK("wr32 refuses a null/low target (no fault)", !wr32(0, 1) && !wr32(0x20, 1));
    HCHECK("wr32 refuses a word that starts readable but ends in the hole", !wr32(ib + 0x0FFE, 1));
    HEQ("wr32 refusal left the readable bytes before the hole alone", (long long)at<int>(a, 0x0FFC), 0x0BADF00Dll);

    // ---- dump loop (97): tail crosses into unreadable memory
    g_badLo = a + 0x140; g_badHi = a + 0x1000; saferead::beginScope();
    for (int i = 0; i < 0x120; i += 4) at<int>(a, 0x100 + i) = 0x1000 + i;
    bool got[0x120 / 4]; int n = dumpWords(ib + 0x100, 0x120, got);
    HEQ("dump loop reads exactly the readable words (0x40 bytes = 16 of 72)", (long long)n, 16ll);
    bool headOk = true, tailBad = true;
    for (int k = 0; k < 16; ++k) headOk = headOk && got[k];
    for (int k = 16; k < 0x120 / 4; ++k) tailBad = tailBad && !got[k];
    HCHECK("dump loop: words before the hole succeed (earlier words are kept)", headOk);
    HCHECK("dump loop: words in the hole fail cleanly, loop continues to the end (dumpMem prints placeholders)", tailBad);
    int w = -1; HCHECK("dump loop: last readable word value is intact", saferead::readAt<int>(3, a, 0x13C, &w) && w == 0x1000 + 0x3C);
    g_badLo = g_badHi = 0; saferead::beginScope();
    n = dumpWords(ib + 0x100, 0x120, got);
    HEQ("dump loop reads all 72 words once the hole is gone (scope reset drops the cache)", (long long)n, 72ll);

    // ---- KpLog fallback sink (logf forwards here): SetFallbackFile only stores the path, so the DLL's static initialiser does no I/O.
    {
        const char* td = getenv("TMPDIR"); if (!td || !*td) td = "/tmp";
        char path[300]; snprintf(path, sizeof path, "%s/inventory_sort_filter_log.txt", td);
        remove(path);
        KpLog_SetFallbackFile(path); KpLog_SetFallbackLevel(KPLOG_INFO);
        HCHECK("KpLog_SetFallbackFile only stores the path, no I/O (safe from a static initialiser)", access(path, F_OK) != 0);
        KpLog_SetFallbackFile(nullptr);
        remove(path);
    }
    return kt::Report("common_contract_test (inventory_sort_filter)");
}
