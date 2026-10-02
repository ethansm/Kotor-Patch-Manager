// SafeRead.h -- exception-free pointer probing + a failure ring buffer for KOTOR2 patch DLLs.
// Load-hang follow-up: patch_manager_mods/09_load_hang_investigation.md section 10 (local-setup/KOTOR-II).
//
// probeRead() never raises an exception: it asks VirtualQuery instead of IsBadReadPtr's try-read, so a
// bad pointer no longer goes through Wine's fault path and every DLL's vectored handler, and guard
// pages are refused instead of being tripped. Readable regions are cached per thread until the next
// beginScope(); call beginScope() at every hook entry, because engine code that runs between hook
// calls can free memory.
//
// Rules: never call probeRead while another thread is suspended by you (VirtualQuery takes an ntdll
// lock that thread may hold). Include from one .cpp per DLL (the state is per translation unit).
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

namespace saferead {

constexpr int CacheSlots = 4;
constexpr int RingSize = 64;

struct Region { uintptr_t lo, hi; };
struct Fail { unsigned site; uintptr_t addr; unsigned len; uintptr_t caller; DWORD tick; };

static thread_local Region t_cache[CacheSlots];
static thread_local int t_next;
static Fail g_ring[RingSize];
static volatile LONG g_ringCount = 0;   // total failures recorded (ring index = count % RingSize)
static bool g_ringOn = false;           // set from the DLL's debug flag

inline void enableRing(bool on) { g_ringOn = on; }

inline void beginScope() {
    for (auto& r : t_cache) r = { 0, 0 };
}

inline bool readableProtect(DWORD p) {
    if (p & (PAGE_GUARD | PAGE_NOACCESS)) return false;
    return (p & (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ |
                 PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) != 0;
}

// [a, end) fully inside committed readable memory? Walks consecutive regions.
inline bool regionReadable(uintptr_t a, uintptr_t end) {
    while (a < end) {
        bool hit = false;
        for (auto& r : t_cache) if (a >= r.lo && a < r.hi) { a = r.hi; hit = true; break; }
        if (hit) continue;
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(reinterpret_cast<LPCVOID>(a), &mbi, sizeof(mbi)) != sizeof(mbi)) return false;
        if (mbi.State != MEM_COMMIT || !readableProtect(mbi.Protect)) return false;
        uintptr_t lo = reinterpret_cast<uintptr_t>(mbi.BaseAddress), hi = lo + mbi.RegionSize;
        t_cache[t_next] = { lo, hi };
        t_next = (t_next + 1) % CacheSlots;
        a = hi;
    }
    return true;
}

inline void recordFail(unsigned site, uintptr_t addr, size_t len, uintptr_t caller) {
    if (!g_ringOn) return;
    LONG i = InterlockedIncrement(&g_ringCount) - 1;
    g_ring[i % RingSize] = { site, addr, static_cast<unsigned>(len), caller, GetTickCount() };
}

// True when [p, p+len) can be read without faulting. `site` is a small caller-chosen id that the
// ring buffer reports next to the caller's return address.
__attribute__((noinline)) static bool probeRead(unsigned site, const void* p, size_t len) {
    uintptr_t a = reinterpret_cast<uintptr_t>(p);
    uintptr_t caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
    if (len == 0) return true;
    if (a < 0x10000 || a + len < a || !regionReadable(a, a + len)) { recordFail(site, a, len, caller); return false; }
    return true;
}

template <typename T> bool readAt(unsigned site, uintptr_t base, int off, T* out) {
    if (base == 0 || !probeRead(site, reinterpret_cast<const void*>(base + off), sizeof(T))) return false;
    *out = *reinterpret_cast<const T*>(base + off);
    return true;
}

// Formats the ring (oldest first) into buf; returns chars written.
inline int dump(char* buf, int cap) {
    LONG count = g_ringCount;
    int n = count < RingSize ? count : RingSize, len = 0;
    len += snprintf(buf + len, cap - len, "saferead: %ld failed probes (showing last %d)\n", count, n);
    for (int k = 0; k < n && len < cap - 1; ++k) {
        const Fail& f = g_ring[(count - n + k) % RingSize];
        len += snprintf(buf + len, cap - len, "  t=%lu site=%u addr=%08x len=%u caller=%08x\n",
                        f.tick, f.site, (unsigned)f.addr, f.len, (unsigned)f.caller);
    }
    return len < cap ? len : cap - 1;
}

} // namespace saferead
