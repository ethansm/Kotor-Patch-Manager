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
#include <string.h>

namespace saferead {

constexpr int CacheSlots = 4;
constexpr int RingSize = 64;

struct Region { uintptr_t lo, hi; };
struct Fail { unsigned site; uintptr_t addr; unsigned len; uintptr_t caller; DWORD tick; };

static thread_local Region t_cache[CacheSlots];
static thread_local int t_next;
static thread_local Region t_wcache[CacheSlots];   // writable regions (probeWrite); separate so a read-only region never answers a write probe
static thread_local int t_wnext;
static Fail g_ring[RingSize];
static volatile LONG g_ringCount = 0;   // total failures recorded (ring index = count % RingSize)
static bool g_ringOn = false;           // set from the DLL's debug flag

inline void enableRing(bool on) { g_ringOn = on; }

inline void beginScope() {
    for (auto& r : t_cache) r = { 0, 0 };
    for (auto& r : t_wcache) r = { 0, 0 };
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
        if (hi <= a) return false;   // degenerate answer (region end wrapped): refuse rather than loop forever
        t_cache[t_next] = { lo, hi };
        t_next = (t_next + 1) % CacheSlots;
        a = hi;
    }
    return true;
}

inline bool writableProtect(DWORD p) {
    if (p & (PAGE_GUARD | PAGE_NOACCESS)) return false;
    return (p & (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) != 0;
}

// [a, end) fully inside committed writable memory? Same walk as regionReadable, own cache. Never IsBadWritePtr.
inline bool regionWritable(uintptr_t a, uintptr_t end) {
    while (a < end) {
        bool hit = false;
        for (auto& r : t_wcache) if (a >= r.lo && a < r.hi) { a = r.hi; hit = true; break; }
        if (hit) continue;
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(reinterpret_cast<LPCVOID>(a), &mbi, sizeof(mbi)) != sizeof(mbi)) return false;
        if (mbi.State != MEM_COMMIT || !writableProtect(mbi.Protect)) return false;
        uintptr_t lo = reinterpret_cast<uintptr_t>(mbi.BaseAddress), hi = lo + mbi.RegionSize;
        if (hi <= a) return false;   // degenerate answer: refuse rather than loop
        t_wcache[t_wnext] = { lo, hi };
        t_wnext = (t_wnext + 1) % CacheSlots;
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

// True when [p, p+len) is committed, writable memory (PAGE_READWRITE/WRITECOPY/EXECUTE_READWRITE/EXECUTE_WRITECOPY, no GUARD/NOACCESS).
// Same shape, cache scope (beginScope) and failure ring as probeRead; uses VirtualQuery only (IsBadWritePtr fault-storms under Wine).
__attribute__((noinline, unused)) static bool probeWrite(unsigned site, void* p, size_t len) {
    uintptr_t a = reinterpret_cast<uintptr_t>(p);
    uintptr_t caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
    if (len == 0) return true;
    if (a < 0x10000 || a + len < a || !regionWritable(a, a + len)) { recordFail(site, a, len, caller); return false; }
    return true;
}

template <typename T> bool readAt(unsigned site, uintptr_t base, int off, T* out) {
    // base + off wraps mod 2^N: off >= 0 must not lower the address, off < 0 must not raise it.
    uintptr_t a = base + uintptr_t(off);
    if (base == 0 || (off >= 0 ? a < base : a > base) || !probeRead(site, reinterpret_cast<const void*>(a), sizeof(T))) return false;
    *out = *reinterpret_cast<const T*>(a);
    return true;
}

// Reads an engine CExoString {char* p @+0; int len @+4} (32-bit fields) into buf as a NUL-terminated string.
// Returns the length (trailing NULs stripped; engine lengths include the NUL), or 0 with buf[0]=0 on any failure:
// bad read, len <= 0 or > maxLen, len >= cap, empty after stripping, or (printableOnly) a byte outside 0x20..0x7E.
inline int readExo(unsigned site, uintptr_t addr, char* buf, int cap, int maxLen = 80, bool printableOnly = true) {
    if (!buf || cap <= 0) return 0;
    buf[0] = 0;
    uint32_t p = 0;
    int len = 0;
    if (!readAt(site, addr, 0, &p) || !readAt(site, addr, 4, &len)) return 0;
    if (len <= 0 || len > maxLen || p == 0 || len >= cap) return 0;
    const void* src = reinterpret_cast<const void*>(static_cast<uintptr_t>(p));
    if (!probeRead(site, src, len)) return 0;
    memcpy(buf, src, len);
    buf[len] = 0;
    int n = len;
    while (n > 0 && buf[n - 1] == 0) --n;
    if (n == 0) return 0;
    if (printableOnly)
        for (int i = 0; i < n; ++i) {
            unsigned char c = static_cast<unsigned char>(buf[i]);
            if (c < 0x20 || c > 0x7E) { buf[0] = 0; return 0; }
        }
    return n;
}

// Tolerant CExoString copy for logging: p/len already read by the caller (len < 0 = unknown). No printable filter, no length cap.
// out is "" (p == 0, cap < 2, or len == 0), the bytes (truncated to cap-1; to the page end when the rest is unreadable), or "?" (unreadable).
// Returns the byte count copied, 0 for "", -1 for "?".
inline int readExoLoose(unsigned site, uint32_t p, int len, char* out, int cap) {
    if (!out || cap <= 0) return 0;
    out[0] = 0;
    if (!p || cap < 2) return 0;
    int n = cap - 1;
    if (len >= 0 && len < n) n = len;
    if (n <= 0) return 0;
    const void* src = reinterpret_cast<const void*>(static_cast<uintptr_t>(p));
    if (!probeRead(site, src, n)) {
        int toPage = static_cast<int>(((p | 0xFFFu) + 1u) - p);   // bytes left on p's page (the next page may be unmapped)
        if (toPage < n && probeRead(site, src, toPage)) n = toPage;
        else { out[0] = '?'; out[1] = 0; return -1; }
    }
    memcpy(out, src, n);
    out[n] = 0;
    return n;
}

// 16-byte CResRef (not NUL-terminated) -> NUL-terminated out[17]. Stops at the first NUL; any byte outside 0x20..0x7E becomes '?'.
enum : unsigned {
    RR_LOWER = 1,       // lowercase A-Z
    RR_SPACE_BAD = 2,   // space is not printable either (0x21..0x7E only)
    RR_EMPTY_Q = 4,     // an empty name reads "?" instead of ""
};
inline void formatResRef16(const void* raw16, char out[17], unsigned flags) {
    const unsigned char* r = static_cast<const unsigned char*>(raw16);
    const unsigned lo = (flags & RR_SPACE_BAD) ? 0x21u : 0x20u;
    int n = 0;
    for (; n < 16 && r[n]; ++n) {
        unsigned c = r[n];
        if ((flags & RR_LOWER) && c >= 'A' && c <= 'Z') c += 32;
        else if (c < lo || c > 0x7E) c = '?';
        out[n] = static_cast<char>(c);
    }
    if (n == 0 && (flags & RR_EMPTY_Q)) out[n++] = '?';
    out[n] = 0;
}

// probeRead 16 bytes at addr, then formatResRef16. On addr == 0 or unreadable: out = RR_EMPTY_Q ? "?" : "", returns false.
inline bool readResRef16(unsigned site, uintptr_t addr, char out[17], unsigned flags) {
    char raw[16];
    if (!addr || !probeRead(site, reinterpret_cast<const void*>(addr), 16)) {
        if (flags & RR_EMPTY_Q) { out[0] = '?'; out[1] = 0; } else out[0] = 0;
        return false;
    }
    memcpy(raw, reinterpret_cast<const void*>(addr), 16);
    formatResRef16(raw, out, flags);
    return true;
}

// Formats the ring (oldest first) into buf; returns chars written (NUL-terminated, never more than cap-1; 0 for a null buf or cap <= 0).
inline int dump(char* buf, int cap) {
    if (!buf || cap <= 0) return 0;
    buf[0] = 0;
    LONG count = g_ringCount;
    int n = count < RingSize ? count : RingSize, len = 0;
    auto put = [&](int w) {   // advance by what snprintf wrote, clamped to the room left (snprintf returns the untruncated length)
        if (w < 0) w = 0;
        len = w < cap - len ? len + w : cap - 1;
    };
    put(snprintf(buf, cap, "saferead: %ld failed probes (showing last %d)\n", count, n));
    for (int k = 0; k < n && len < cap - 1; ++k) {
        const Fail& f = g_ring[(count - n + k) % RingSize];
        put(snprintf(buf + len, cap - len, "  t=%lu site=%u addr=%08x len=%u caller=%08x\n",
                     f.tick, f.site, (unsigned)f.addr, f.len, (unsigned)f.caller));
    }
    return len;
}

} // namespace saferead
