// Host test of saferead::readExo / readExoLoose / formatResRef16 / readResRef16 (SafeRead.h) on fake CExoStrings in a MAP_32BIT arena.
// Windows is the stub windows.h; g_stubVirtualQueryAll makes the arena read as committed memory.
#include <windows.h>
#include <sys/mman.h>
#include <initializer_list>
#include <vector>
#include <unistd.h>
// Protection faking: the host stub's VirtualQuery reports every address as one committed PAGE_READWRITE region. Route SafeRead.h's
// VirtualQuery through a wrapper that can override State/Protect for a chosen range (and clip RegionSize at its edges) while
// forwarding everything else to the stub, so probeWrite's protection rules are testable without touching the shared stub.
struct FakeRange { uintptr_t lo, hi; DWORD protect, state; };
static FakeRange g_fake[8];
static int g_fakeN = 0;
static auto g_realVQ = &VirtualQuery;
static int g_vqCalls = 0;   // how many times SafeRead asked the OS (cache behaviour is observable only through this)
static int g_vqMode = 0;    // 0 normal; 1 fail (returns 0 but leaves a committed-looking answer behind); 2 degenerate (RegionSize 0)
static SIZE_T fakeVQ(LPCVOID a, MEMORY_BASIC_INFORMATION* m, SIZE_T n) {
    ++g_vqCalls;
    if (g_vqMode) {
        m->BaseAddress = const_cast<void*>(a); m->RegionSize = g_vqMode == 1 ? 0x1000 : 0;
        m->State = MEM_COMMIT; m->Protect = PAGE_READWRITE;
        return g_vqMode == 1 ? 0 : sizeof *m;
    }
    SIZE_T r = g_realVQ(a, m, n);
    if (!r) return r;
    uintptr_t p = reinterpret_cast<uintptr_t>(a);
    for (int i = 0; i < g_fakeN; ++i) {
        const FakeRange& f = g_fake[i];
        if (p >= f.lo && p < f.hi) {
            m->BaseAddress = reinterpret_cast<void*>(f.lo); m->RegionSize = f.hi - f.lo;
            m->Protect = f.protect; m->State = f.state;
            return r;
        }
        if (p < f.lo && p + m->RegionSize > f.lo) m->RegionSize = f.lo - p;   // end the default region where a fake one starts
    }
    return r;
}
#define VirtualQuery fakeVQ
#include "SafeRead.h"
#include "check.h"

static uint8_t* g_arena = nullptr;
static size_t g_used = 0;
constexpr size_t ArenaSize = 1u << 16;

static uintptr_t arenaAlloc(size_t n) {
    n = (n + 15) & ~size_t(15);
    if (g_used + n > ArenaSize) { std::printf("arena full\n"); std::exit(4); }
    uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used);
    std::memset(g_arena + g_used, 0, n);
    g_used += n;
    return a;
}

// Fake CExoString {uint32 p; int32 len} pointing at a copy of `bytes` (n bytes). Returns the struct address.
static uintptr_t mkExo(const void* bytes, int n, int len) {
    uintptr_t data = arenaAlloc(n > 0 ? n : 1);
    if (n > 0) std::memcpy(reinterpret_cast<void*>(data), bytes, n);
    uintptr_t st = arenaAlloc(8);
    *reinterpret_cast<uint32_t*>(st) = static_cast<uint32_t>(data);
    *reinterpret_cast<int32_t*>(st + 4) = len;
    return st;
}
static uintptr_t mkExoRaw(uint32_t p, int len) {
    uintptr_t st = arenaAlloc(8);
    *reinterpret_cast<uint32_t*>(st) = p;
    *reinterpret_cast<int32_t*>(st + 4) = len;
    return st;
}

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#ifdef MAP_32BIT
    flags |= MAP_32BIT;
#endif
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, flags, -1, 0);
    if (m == MAP_FAILED) { std::perror("mmap"); return 5; }
    if (static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(m)) + ArenaSize > 0xFFFFFFFFull) { std::printf("arena above 4GB\n"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;

    char buf[160];
    auto fill = [&] { std::memset(buf, 'X', sizeof buf); };

    // "Foo\0" len 4 -> 3
    { uintptr_t s = mkExo("Foo", 4, 4); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 3); KT_CHECK_EQ(buf, "Foo"); }
    // no trailing NUL, len 3
    { uintptr_t s = mkExo("Bar", 3, 3); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 3); KT_CHECK_EQ(buf, "Bar"); }

    // len 0 / -1 / 81 / 80
    { uintptr_t s = mkExo("Foo", 4, 0); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 0); KT_CHECK_EQ(buf[0], 0); }
    { uintptr_t s = mkExo("Foo", 4, -1); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 0); KT_CHECK_EQ(buf[0], 0); }
    char s100[101]; std::memset(s100, 'a', 100); s100[100] = 0;
    { uintptr_t s = mkExo(s100, 81, 81); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 0); KT_CHECK_EQ(buf[0], 0); }
    { uintptr_t s = mkExo(s100, 80, 80); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 80);
      KT_CHECK_EQ(buf[80], 0); KT_CHECK_EQ(std::strlen(buf), 80u); KT_CHECK_EQ(buf[79], 'a'); }

    // pointer / struct address problems
    { uintptr_t s = mkExoRaw(0, 4); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 0); KT_CHECK_EQ(buf[0], 0); }
    { uintptr_t s = mkExoRaw(0x10, 4); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 0); KT_CHECK_EQ(buf[0], 0); }
    { fill();
      KT_CHECK_EQ(saferead::readExo(1, 0x20, buf, 96), 0); KT_CHECK_EQ(buf[0], 0); }
    { fill();
      KT_CHECK_EQ(saferead::readExo(1, 0, buf, 96), 0); KT_CHECK_EQ(buf[0], 0); }
    { uintptr_t s = mkExo("Foo", 4, 4); fill();
      g_stubVirtualQueryAll = false; saferead::beginScope();   // drop the per-thread region cache
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 0); KT_CHECK_EQ(buf[0], 0);
      g_stubVirtualQueryAll = true; saferead::beginScope(); }
    // works again after restoring the stub
    { uintptr_t s = mkExo("Foo", 4, 4); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 3); KT_CHECK_EQ(buf, "Foo"); }

    // non-printable
    { const char raw[] = { 'a', 0x07, 'b' }; uintptr_t s = mkExo(raw, 3, 3); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 0); KT_CHECK_EQ(buf[0], 0);
      fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96, 80, false), 3);
      KT_CHECK_EQ(buf[0], 'a'); KT_CHECK_EQ(buf[1], 0x07); KT_CHECK_EQ(buf[2], 'b'); KT_CHECK_EQ(buf[3], 0); }
    { const char raw[] = { 'a', (char)0x80 }; uintptr_t s = mkExo(raw, 2, 2); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 0); KT_CHECK_EQ(buf[0], 0); }
    { const char raw[] = { 'a', 0x7F }; uintptr_t s = mkExo(raw, 2, 2); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 0); KT_CHECK_EQ(buf[0], 0); }
    { const char raw[] = { ' ', '~' }; uintptr_t s = mkExo(raw, 2, 2); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 2); KT_CHECK_EQ(buf, " ~"); }

    // small cap
    { uintptr_t s = mkExo("Foo", 4, 4); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 4), 0); KT_CHECK_EQ(buf[0], 0);
      fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 5), 3); KT_CHECK_EQ(buf, "Foo"); }

    // maxLen override
    { uintptr_t s = mkExo(s100, 100, 100); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 128, 120), 100); KT_CHECK_EQ(buf, s100);
      fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 128), 0); KT_CHECK_EQ(buf[0], 0);
      fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 128, 100), 100); }

    // all-NUL
    { const char raw[3] = { 0, 0, 0 }; uintptr_t s = mkExo(raw, 3, 3); fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 0); KT_CHECK_EQ(buf[0], 0); }

    // null buf / cap 0 / negative cap
    { uintptr_t s = mkExo("Foo", 4, 4);
      KT_CHECK_EQ(saferead::readExo(1, s, nullptr, 96), 0);
      fill();
      KT_CHECK_EQ(saferead::readExo(1, s, buf, 0), 0); KT_CHECK_EQ(buf[0], 'X');
      KT_CHECK_EQ(saferead::readExo(1, s, buf, -5), 0); KT_CHECK_EQ(buf[0], 'X'); }

    // ---- dump(): cap clamping (cap <= 0 used to pass a negative length to snprintf as a huge size_t)
    {
        { char z[128]; KT_CHECK_EQ(saferead::dump(z, sizeof z), 43); KT_CHECK_EQ(z, "saferead: 0 failed probes (showing last 0)\n"); }   // fresh ring: count starts at 0
        saferead::enableRing(true);
        saferead::probeRead(7, reinterpret_cast<const void*>(0x20), 4);   // one recorded failure
        { char z[256]; saferead::dump(z, sizeof z);
          KT_CHECK(std::strstr(z, "1 failed probes (showing last 1)") != nullptr); KT_CHECK(std::strstr(z, "site=7 ") != nullptr); }   // count<RingSize: entry index is count-n+k
        char g[64]; auto gfill = [&] { std::memset(g, 'G', sizeof g); };
        gfill(); KT_CHECK_EQ(saferead::dump(g, 0), 0);  KT_CHECK_EQ(g[0], 'G');
        gfill(); KT_CHECK_EQ(saferead::dump(g, -1), 0); KT_CHECK_EQ(g[0], 'G'); KT_CHECK_EQ(g[10], 'G');
        gfill(); KT_CHECK_EQ(saferead::dump(nullptr, 16), 0);
        gfill(); KT_CHECK_EQ(saferead::dump(g, 1), 0);  KT_CHECK_EQ(g[0], 0); KT_CHECK_EQ(g[1], 'G');
        gfill(); KT_CHECK_EQ(saferead::dump(g, 10), 9); KT_CHECK_EQ(g[9], 0); KT_CHECK_EQ(std::strncmp(g, "saferead:", 9), 0); KT_CHECK_EQ(g[10], 'G');
        // cap just under the header, just over it (entry line truncated), and roomy
        char full[4096]; int fl = saferead::dump(full, sizeof full);
        KT_CHECK(fl > 50); KT_CHECK_EQ((int)std::strlen(full), fl);
        for (int cap : { fl - 1, fl, fl + 1, 55, 60, 80 }) {
            gfill(); std::memset(full, 'G', 200);
            int n = saferead::dump(full, cap);
            KT_CHECK(n >= 0 && n <= cap - 1); KT_CHECK_EQ((int)std::strlen(full), n); KT_CHECK_EQ(full[cap], 'G');
        }
        // exact truncation: the result is always min(full length, cap-1)
        for (int cap = 2; cap <= fl + 2; ++cap) {
            char big[256]; std::memset(big, 'G', sizeof big);
            int n = saferead::dump(big, cap);
            KT_CHECK_EQ(n, cap - 1 < fl ? cap - 1 : fl); KT_CHECK_EQ((int)std::strlen(big), n);
        }
        saferead::enableRing(false);
    }

    // ---- ring wrap: 100 failures into a 64-entry ring; dump shows the last 64, oldest first
    {
        saferead::enableRing(true);
        char t[8192];
        for (unsigned i = 0; i < 100; ++i) saferead::probeRead(1000 + i, reinterpret_cast<const void*>(0x20), 4);
        int n = saferead::dump(t, sizeof t);
        int lines = 0; for (int i = 0; i < n; ++i) lines += t[i] == '\n';
        KT_CHECK_EQ(lines, 65);                       // header + 64 entries
        KT_CHECK(std::strstr(t, "(showing last 64)") != nullptr);
        KT_CHECK(std::strstr(t, "site=1036 ") != nullptr);     // oldest kept: failure #101 - 64 = site 1036 (earlier site-7 probe aged out)
        KT_CHECK(std::strstr(t, "site=1035 ") == nullptr);
        KT_CHECK(std::strstr(t, "site=1099 ") != nullptr);
        KT_CHECK(std::strstr(t, "site=7 ") == nullptr);
        const char* a = std::strstr(t, "site=1036 "); const char* b = std::strstr(t, "site=1099 ");
        KT_CHECK(a && b && a < b);                     // oldest first
        // ring off: nothing recorded
        saferead::enableRing(false);
        for (int i = 0; i < 5; ++i) saferead::probeRead(2000, reinterpret_cast<const void*>(0x20), 4);
        saferead::enableRing(true);
        saferead::probeRead(2001, reinterpret_cast<const void*>(0x20), 4);
        n = saferead::dump(t, sizeof t); (void)n;
        KT_CHECK(std::strstr(t, "site=2000 ") == nullptr); KT_CHECK(std::strstr(t, "site=2001 ") != nullptr);
        saferead::enableRing(false);
    }

    // ---- probeRead / readAt edge cases
    {
        saferead::beginScope();
        uintptr_t a = arenaAlloc(32);
        KT_CHECK(saferead::probeRead(1, reinterpret_cast<const void*>(a), 0));                                   // len 0 always true
        KT_CHECK(!saferead::probeRead(1, reinterpret_cast<const void*>(~uintptr_t(0) - 3), 8));                  // a+len wraps
        KT_CHECK(!saferead::probeRead(1, reinterpret_cast<const void*>(a), ~size_t(0)));                         // len huge -> wraps
        KT_CHECK(!saferead::probeRead(1, reinterpret_cast<const void*>(0xFFFF), 4));                             // just below the low-address floor
        KT_CHECK(saferead::probeRead(1, reinterpret_cast<const void*>(a), 32));
        uint32_t o = 0x5A5A5A5A; *reinterpret_cast<uint32_t*>(a + 8) = 0x1234;
        KT_CHECK(!saferead::readAt(1, a, -(int)a, &o)); KT_CHECK_EQ(o, 0x5A5A5A5Au);            // base+off == 0 (wraps to null)
        KT_CHECK(!saferead::readAt(1, a, -(int)a - 8, &o)); KT_CHECK_EQ(o, 0x5A5A5A5Au);        // wraps below zero
        KT_CHECK(!saferead::readAt(1, a, -0x7FFFFFFF - 1, &o)); KT_CHECK_EQ(o, 0x5A5A5A5Au);    // INT_MIN
        KT_CHECK(saferead::readAt(1, a + 16, -8, &o)); KT_CHECK_EQ(o, 0x1234u);                 // a legal negative offset still works
    }

    // ---- readAt base+off wrap: a wrapped address fails before any read and leaves out untouched
    {
        saferead::beginScope();
        uintptr_t s = arenaAlloc(8);
        *reinterpret_cast<uint32_t*>(s) = 0xC0DEu;                                    // readable sentinel in the arena
        uintptr_t base = uintptr_t(0) - 0x10000;                                      // base + off wraps to s on any pointer width
        int off = static_cast<int>(s + 0x10000);
        uint32_t o = 0x5A5A5A5Au;
        KT_CHECK(!saferead::readAt(1, base, off, &o)); KT_CHECK_EQ(o, 0x5A5A5A5Au);   // wrapped: must fail, out untouched
        KT_CHECK(saferead::readAt(1, s, 0, &o)); KT_CHECK_EQ(o, 0xC0DEu);            // unwrapped read of s still works
        KT_CHECK(saferead::readAt(1, s - 16, 16, &o)); KT_CHECK_EQ(o, 0xC0DEu);       // legal positive offset (no wrap) still works
        // negative off wrapping below 0 (1 - 0x30000 -> high address): rejected before any probe (the VirtualQuery count is the observable)
        g_fake[0] = { uintptr_t(0) - 0x10000, uintptr_t(0) - 0x8000, PAGE_NOACCESS, MEM_COMMIT }; g_fakeN = 1; int calls = g_vqCalls;
        KT_CHECK(!saferead::readAt(1, 0x20000, -0x30000, &o)); KT_CHECK_EQ(o, 0xC0DEu); KT_CHECK_EQ(g_vqCalls, calls); g_fakeN = 0;
    }

    // ---- readExo exact fit: len == cap-1 fits (NUL at buf[cap-1]); len == cap does not
    {
        char b6[8]; std::memset(b6, 'X', sizeof b6);
        uintptr_t s = mkExo("abcde", 5, 5);
        KT_CHECK_EQ(saferead::readExo(1, s, b6, 6), 5); KT_CHECK_EQ(b6, "abcde"); KT_CHECK_EQ(b6[6], 'X');
        std::memset(b6, 'X', sizeof b6);
        uintptr_t s6 = mkExo("abcdef", 6, 6);
        KT_CHECK_EQ(saferead::readExo(1, s6, b6, 6), 0); KT_CHECK_EQ(b6[0], 0); KT_CHECK_EQ(b6[6], 'X');
        std::memset(b6, 'X', sizeof b6);
        uintptr_t s7 = mkExo("abcdef\0", 7, 7);                 // engine len includes the NUL: 7 > cap-1 -> refused even though 6 chars would fit
        KT_CHECK_EQ(saferead::readExo(1, s7, b6, 7), 0);
        std::memset(b6, 'X', sizeof b6);
        KT_CHECK_EQ(saferead::readExo(1, s7, b6, 8), 6); KT_CHECK_EQ(b6, "abcdef");
    }

    // ---- readExo edge cases found by the mutation gate (doc 20e)
    {
        fill(); { uintptr_t s = mkExo("Foo", 4, 4); KT_CHECK_EQ(saferead::readExo(1, s, buf, 1), 0); KT_CHECK_EQ(buf[0], 0); }   // cap 1: buf[0] still cleared
        fill(); { uintptr_t s = mkExo("a", 1, 1); KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 1); KT_CHECK_EQ(buf, "a"); }       // one-byte string is valid
        fill(); { const char z[1] = { 0 }; uintptr_t s = mkExo(z, 1, 1);
                  KT_CHECK_EQ(saferead::readExo(1, s, buf, 96, 80, false), 0); KT_CHECK_EQ(buf[0], 0); }                         // lone NUL strips to empty
        fill(); { const char r[] = { 0x07, 'b', 'c' }; uintptr_t s = mkExo(r, 3, 3);
                  KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 0); KT_CHECK_EQ(buf[0], 0); }                                    // first byte is checked for printability
        fill(); { const char r[] = { 'a', 0x1F }; uintptr_t s = mkExo(r, 2, 2);
                  KT_CHECK_EQ(saferead::readExo(1, s, buf, 96), 0); KT_CHECK_EQ(buf[0], 0); }                                    // 0x1F is the last control byte
    }

    // ---- probe floor / len 0 / len 1 / region geometry (mutation gate additions)
    {
        g_fakeN = 0; saferead::beginScope();
        auto Rd = [&](uintptr_t p, size_t n) { return saferead::probeRead(1, reinterpret_cast<const void*>(p), n); };
        auto Wr = [&](uintptr_t p, size_t n) { return saferead::probeWrite(1, reinterpret_cast<void*>(p), n); };
        KT_CHECK(Rd(0x20, 0)); KT_CHECK(Rd(0, 0)); KT_CHECK(Wr(0x20, 0)); KT_CHECK(Wr(0, 0));   // len 0 is true even for a bad address
        KT_CHECK(Rd(0x10000, 4)); KT_CHECK(Wr(0x10000, 4));                                      // the floor itself is allowed
        KT_CHECK(!Rd(0xFFFF, 1)); KT_CHECK(!Wr(0xFFFF, 1));
        const uintptr_t base = 0x300000;
        g_fake[0] = { base, base + 0x1000, PAGE_NOACCESS, MEM_COMMIT }; g_fakeN = 1; saferead::beginScope();
        KT_CHECK(!Rd(base, 1)); KT_CHECK(!Wr(base, 1));                                          // len 1 is really probed
        g_fake[0] = { base, base + 0x1000, PAGE_READONLY, MEM_COMMIT };
        KT_CHECK(Rd(base, 1)); KT_CHECK(!Wr(base, 1));
        // regionReadable/Writable called directly: an empty range is true without asking the OS; a cleared cache has no phantom entries
        g_fake[0] = { 0, 0x1000, PAGE_NOACCESS, MEM_COMMIT }; saferead::beginScope();
        KT_CHECK(saferead::regionReadable(0x20, 0x20)); KT_CHECK(saferead::regionWritable(0x20, 0x20));
        KT_CHECK(!saferead::regionReadable(0, 1)); KT_CHECK(!saferead::regionWritable(0, 1));
        // OS failure / degenerate answers are refused (never an infinite loop: alarm kills a hung mutant)
        g_fakeN = 0; saferead::beginScope(); uintptr_t a = arenaAlloc(32);
        g_vqMode = 1; KT_CHECK(!Rd(a, 8)); KT_CHECK(!Wr(a, 8));
        g_vqMode = 2; saferead::beginScope(); alarm(3);
        KT_CHECK(!Rd(a, 8)); KT_CHECK(!Wr(a, 8));
        alarm(0); g_vqMode = 0; saferead::beginScope();
        KT_CHECK(Rd(a, 8)); KT_CHECK(Wr(a, 8));
    }

    // ---- cache geometry via VirtualQuery call counts: 4 slots, round-robin eviction, hit at the region's first byte
    {
        auto lo = [](int i) { return uintptr_t(0x200000) + uintptr_t(i) * 0x10000; };
        g_fakeN = 5; for (int i = 0; i < 5; ++i) g_fake[i] = { lo(i), lo(i) + 0x1000, PAGE_READWRITE, MEM_COMMIT };
        auto run = [&](auto Q) {
            saferead::beginScope(); g_vqCalls = 0;
            KT_CHECK(Q(0)); KT_CHECK(Q(0)); KT_CHECK_EQ(g_vqCalls, 1);                 // second probe at the region start is a cache hit
            for (int i = 1; i < 5; ++i) KT_CHECK(Q(i));
            KT_CHECK_EQ(g_vqCalls, 5);
            for (int i = 4; i >= 1; --i) KT_CHECK(Q(i));
            KT_CHECK_EQ(g_vqCalls, 5);                                                 // the 4 newest regions are all cached
            KT_CHECK(Q(0)); KT_CHECK_EQ(g_vqCalls, 6);                                 // the oldest was evicted
            KT_CHECK(Q(0)); KT_CHECK_EQ(g_vqCalls, 6);
        };
        run([&](int i) { return saferead::probeRead(1, reinterpret_cast<const void*>(lo(i)), 8); });
        run([&](int i) { return saferead::probeWrite(1, reinterpret_cast<void*>(lo(i)), 8); });
        g_fakeN = 0; saferead::beginScope();
    }

    // ---- probeWrite
    {
        saferead::beginScope();
        uintptr_t a = arenaAlloc(64);
        auto P = [&](uintptr_t p, size_t n) { return saferead::probeWrite(1, reinterpret_cast<void*>(p), n); };
        KT_CHECK(P(a, 64));                                    // stub default: committed PAGE_READWRITE
        KT_CHECK(P(a, 0));                                     // len 0 -> true (probeRead shape)
        KT_CHECK(!P(0, 4)); KT_CHECK(!P(0x20, 4)); KT_CHECK(!P(0xFFFF, 4));
        KT_CHECK(!P(~uintptr_t(0) - 3, 8)); KT_CHECK(!P(a, ~size_t(0)));   // p+len wraps
        // per-protection verdicts on a fake 0x1000-byte region at a (keep clear of the arena data via distinct fake bounds)
        uintptr_t lo = a & ~uintptr_t(0xFFF), hi = lo + 0x1000;
        struct { DWORD prot; bool rd, wr; } cases[] = {
            { PAGE_NOACCESS, false, false }, { PAGE_READONLY, true, false }, { PAGE_READWRITE, true, true },
            { PAGE_WRITECOPY, true, true }, { PAGE_EXECUTE_READ, true, false }, { PAGE_EXECUTE_READWRITE, true, true },
            { PAGE_EXECUTE_WRITECOPY, true, true }, { PAGE_READWRITE | PAGE_GUARD, false, false },
            { PAGE_EXECUTE_READWRITE | PAGE_GUARD, false, false }, { 0, false, false },
        };
        for (auto& c : cases) {
            g_fake[0] = { lo, hi, c.prot, MEM_COMMIT }; g_fakeN = 1;
            saferead::beginScope();
            KT_CHECK_EQ(saferead::probeRead(1, reinterpret_cast<const void*>(a), 8), c.rd);
            KT_CHECK_EQ(P(a, 8), c.wr);
        }
        // uncommitted (MEM_RESERVE 0x2000) is neither readable nor writable
        g_fake[0] = { lo, hi, PAGE_READWRITE, 0x2000 }; g_fakeN = 1; saferead::beginScope();
        KT_CHECK(!P(a, 8)); KT_CHECK(!saferead::probeRead(1, reinterpret_cast<const void*>(a), 8));
        // cache sharing: a readable-but-read-only region cached by probeRead must not answer probeWrite true, in either order
        g_fake[0] = { lo, hi, PAGE_READONLY, MEM_COMMIT }; g_fakeN = 1; saferead::beginScope();
        KT_CHECK(saferead::probeRead(1, reinterpret_cast<const void*>(a), 8));      // caches the region
        KT_CHECK(!P(a, 8)); KT_CHECK(saferead::probeRead(1, reinterpret_cast<const void*>(a), 8));
        // and a writable region cached by probeWrite still answers probeRead true
        g_fake[0] = { lo, hi, PAGE_READWRITE, MEM_COMMIT }; saferead::beginScope();
        KT_CHECK(P(a, 8)); KT_CHECK(P(a, 8)); KT_CHECK(saferead::probeRead(1, reinterpret_cast<const void*>(a), 8));
        // a range that spans a writable page into a read-only page
        g_fake[0] = { lo, hi, PAGE_READWRITE, MEM_COMMIT }; g_fake[1] = { hi, hi + 0x1000, PAGE_READONLY, MEM_COMMIT }; g_fakeN = 2; saferead::beginScope();
        KT_CHECK(P(hi - 8, 8)); KT_CHECK(!P(hi - 8, 16)); KT_CHECK(saferead::probeRead(1, reinterpret_cast<const void*>(hi - 8), 16));
        KT_CHECK(!P(hi, 4));
        // writable | writable spanning two fake regions
        g_fake[1] = { hi, hi + 0x1000, PAGE_EXECUTE_READWRITE, MEM_COMMIT }; saferead::beginScope();
        KT_CHECK(P(hi - 8, 16));
        // failure is ring-recorded like probeRead
        saferead::enableRing(true);
        g_fake[1] = { hi, hi + 0x1000, PAGE_READONLY, MEM_COMMIT }; saferead::beginScope();
        P(hi, 4);
        char t[8192]; saferead::dump(t, sizeof t);
        KT_CHECK(std::strstr(t, "site=1 ") != nullptr);
        saferead::enableRing(false);
        g_fakeN = 0; saferead::beginScope();
        KT_CHECK(P(a, 64));
    }

    // ---- readExoLoose / formatResRef16 / readResRef16
    {
        g_fakeN = 0; saferead::beginScope();
        // Legacy bodies the helpers replace (kept verbatim, minus logging/sanitize) for the golden comparison below.
        auto legacy_copyStr = [](int p, int len, char* out, int cap) {   // companion_ai CompanionAI.cpp copyStr, minus cai::sanitize
            out[0] = 0;
            if (!p || cap < 2) return;
            int n = cap - 1;
            if (len >= 0 && len < n) n = len;
            if (n <= 0) return;
            if (!saferead::probeRead(3, reinterpret_cast<const void*>(p), n)) {
                int toPage = int(((static_cast<unsigned>(p) | 0xFFFu) + 1u) - static_cast<unsigned>(p));
                if (toPage < n && saferead::probeRead(3, reinterpret_cast<const void*>(p), toPage)) n = toPage;
                else { out[0] = '?'; out[1] = 0; return; }
            }
            std::memcpy(out, reinterpret_cast<const void*>(p), n);
            out[n] = 0;
        };
        auto legacy_perflabResRef = [](uintptr_t ke, char* out) {   // perf_lab PerfLab.cpp readResRef after the CRes+0x14 deref
            out[0] = '?'; out[1] = 0;
            char raw[16];
            if (!ke || !saferead::probeRead(62, reinterpret_cast<const void*>(ke), 16)) return;
            std::memcpy(raw, reinterpret_cast<const void*>(ke), 16);
            int n = 0;
            for (; n < 16 && raw[n]; ++n) { char ch = raw[n]; out[n] = (ch >= 'A' && ch <= 'Z') ? ch + 32 : (ch > 32 && ch < 127 ? ch : '?'); }
            if (!n) { out[0] = '?'; n = 1; }
            out[n] = 0;
        };
        auto legacy_resrefName = [](const void* r, char out[17]) {   // equip_icon_fix EquipIconFix.cpp resrefName
            std::memcpy(out, r, 16); out[16] = 0;
            for (int i = 0; i < 16; ++i) if (out[i] && (out[i] < 0x20 || out[i] > 0x7e)) out[i] = '?';
        };

        // Page layout inside the arena: [pg, pg+0x1000) readable, [pg+0x1000, pg+0x2000) faked NOACCESS (an unreadable next page).
        const uintptr_t pg = reinterpret_cast<uintptr_t>(g_arena) + 0x8000;
        auto setTail = [&](bool on) { g_fakeN = on ? 1 : 0; g_fake[0] = { pg + 0x1000, pg + 0x2000, PAGE_NOACCESS, MEM_COMMIT }; saferead::beginScope(); };
        const uintptr_t tail = pg + 0x1000 - 5;   // 5 bytes before the unreadable page
        std::memcpy(reinterpret_cast<void*>(tail), "abcde", 5);
        std::memset(reinterpret_cast<void*>(pg + 0x1000), 'Z', 32);

        char o[64];
        auto ofill = [&] { std::memset(o, 'X', sizeof o); };
        auto L = [&](uintptr_t p, int len, int cap) { ofill(); return saferead::readExoLoose(5, static_cast<uint32_t>(p), len, o, cap); };

        // p == 0 / cap < 2 / len == 0: ""
        KT_CHECK_EQ(L(0, 4, 16), 0); KT_CHECK_EQ(o[0], 0); KT_CHECK_EQ(o[1], 'X');
        uintptr_t h = arenaAlloc(32); std::memcpy(reinterpret_cast<void*>(h), "hello world", 12);
        KT_CHECK_EQ(L(h, 5, 1), 0); KT_CHECK_EQ(o[0], 0); KT_CHECK_EQ(o[1], 'X');
        KT_CHECK_EQ(L(h, 5, 0), 0); KT_CHECK_EQ(o[0], 'X');                       // cap 0: nothing written
        KT_CHECK_EQ(L(h, 0, 16), 0); KT_CHECK_EQ(o[0], 0); KT_CHECK_EQ(o[1], 'X');
        ofill(); KT_CHECK_EQ(saferead::readExoLoose(5, (uint32_t)h, 5, nullptr, 16), 0); KT_CHECK_EQ(o[0], 'X');   // null out: no write
        // exact copy / unknown len reads cap-1 / len above cap-1 truncates / len == cap-1
        KT_CHECK_EQ(L(h, 5, 16), 5); KT_CHECK_EQ(o, "hello"); KT_CHECK_EQ(o[6], 'X');
        KT_CHECK_EQ(L(h, -1, 6), 5); KT_CHECK_EQ(o, "hello"); KT_CHECK_EQ(o[6], 'X');
        KT_CHECK_EQ(L(h, 11, 6), 5); KT_CHECK_EQ(o, "hello"); KT_CHECK_EQ(o[6], 'X');
        KT_CHECK_EQ(L(h, 5, 6), 5); KT_CHECK_EQ(o, "hello");
        KT_CHECK_EQ(L(h, -1, 2), 1); KT_CHECK_EQ(o, "h"); KT_CHECK_EQ(o[2], 'X');
        // no printable filter: control and high bytes pass through, an embedded NUL is copied and counted
        { uintptr_t c = arenaAlloc(8); const char raw[] = { 'a', 0x07, (char)0x80, 0, 'b' }; std::memcpy(reinterpret_cast<void*>(c), raw, 5);
          KT_CHECK_EQ(L(c, 5, 16), 5); KT_CHECK(std::memcmp(o, raw, 5) == 0); KT_CHECK_EQ(o[5], 0); KT_CHECK_EQ(o[6], 'X'); }
        // unreadable: "?" and -1 (low address, and an unreadable page with the whole range outside it)
        KT_CHECK_EQ(L(0x20, 4, 16), -1); KT_CHECK_EQ(o, "?"); KT_CHECK_EQ(o[2], 'X');
        setTail(true);
        KT_CHECK_EQ(L(pg + 0x1000, 4, 16), -1); KT_CHECK_EQ(o, "?");
        // partial page: readable only to the page end -> truncated there (5 bytes), whatever len/cap asked for
        KT_CHECK_EQ(L(tail, -1, 32), 5); KT_CHECK_EQ(o, "abcde"); KT_CHECK_EQ(o[6], 'X');
        KT_CHECK_EQ(L(tail, 20, 32), 5); KT_CHECK_EQ(o, "abcde");
        KT_CHECK_EQ(L(tail, 6, 32), 5); KT_CHECK_EQ(o, "abcde");                  // one past the page end
        KT_CHECK_EQ(L(tail, 5, 32), 5); KT_CHECK_EQ(o, "abcde");                  // exactly to the page end: plain probe passes
        KT_CHECK_EQ(L(tail, 3, 32), 3); KT_CHECK_EQ(o, "abc");
        KT_CHECK_EQ(L(tail, -1, 4), 3); KT_CHECK_EQ(o, "abc");                    // cap limits before the page does
        setTail(false);
        KT_CHECK_EQ(L(tail, -1, 32), 31); KT_CHECK_EQ(o[5], 'Z');                 // whole next page readable again: no page-end cut
        // a page-end cut must not run with the page itself unreadable
        g_fake[0] = { pg, pg + 0x1000, PAGE_NOACCESS, MEM_COMMIT }; g_fakeN = 1; saferead::beginScope();
        KT_CHECK_EQ(L(tail, -1, 32), -1); KT_CHECK_EQ(o, "?");
        setTail(false);

        // golden: legacy copyStr vs readExoLoose, byte for byte over the whole buffer
        {
            int plen[] = { -5, -1, 0, 1, 2, 4, 5, 6, 7, 15, 16, 17, 40 };
            int pcap[] = { -1, 0, 1, 2, 3, 5, 6, 7, 16, 33 };
            uintptr_t ptrs[] = { 0, 0x20, 0xFFFF, tail, tail + 2, tail + 4, pg + 0x1000, pg + 0x1000 - 1, pg, h };
            for (int tailOn = 0; tailOn < 2; ++tailOn) {
                setTail(tailOn);
                for (uintptr_t p : ptrs) for (int len : plen) for (int cap : pcap) {
                    if (cap <= 0) continue;   // legacy writes out[0] unconditionally; real callers pass real buffers
                    char a[48], b[48]; std::memset(a, 'X', sizeof a); std::memset(b, 'X', sizeof b);
                    legacy_copyStr(static_cast<int>(p), len, a, cap);
                    int r = saferead::readExoLoose(3, static_cast<uint32_t>(p), len, b, cap);
                    KT_CHECK(std::memcmp(a, b, sizeof a) == 0);
                    KT_CHECK(r == -1 ? (b[0] == '?' && b[1] == 0) : (r >= 0 && b[r] == 0));
                }
            }
            setTail(false);
        }

        // formatResRef16
        auto F = [&](const char* r16, unsigned fl) { std::memset(o, 'X', sizeof o); saferead::formatResRef16(r16, o, fl); };
        F("abcdefghijklmnop", 0);        KT_CHECK_EQ(o, "abcdefghijklmnop"); KT_CHECK_EQ(o[16], 0); KT_CHECK_EQ(o[17], 'X');   // 16 chars, no NUL
        { char r[16] = { 'a', 'b', 0, 'c', 'd' }; F(r, 0); KT_CHECK_EQ(o, "ab"); KT_CHECK_EQ(o[3], 'X'); }                    // stops at the first NUL
        F("AbC_09-Z", 0);                KT_CHECK_EQ(o, "AbC_09-Z");
        F("AbC_09-Z", saferead::RR_LOWER); KT_CHECK_EQ(o, "abc_09-z");
        { char r[16] = { '@', '[', '`', '{', 'A', 'Z' }; F(r, saferead::RR_LOWER); KT_CHECK_EQ(o, "@[`{az"); }              // only A-Z lowercase
        F("a b", 0);                     KT_CHECK_EQ(o, "a b");
        F("a b", saferead::RR_SPACE_BAD); KT_CHECK_EQ(o, "a?b");
        F(" ", saferead::RR_SPACE_BAD);  KT_CHECK_EQ(o, "?");
        { char r[16] = { 'a', (char)0x80, (char)0xFF, 0x1F, 0x7F, '~', '!' }; F(r, 0); KT_CHECK_EQ(o, "a????~!"); F(r, saferead::RR_SPACE_BAD); KT_CHECK_EQ(o, "a????~!"); }
        { char r[16] = { 0x1F }; F(r, 0); KT_CHECK_EQ(o, "?"); }
        { char r[16] = { 0x20 }; F(r, 0); KT_CHECK_EQ(o, " "); }
        { char r[16] = { 0 }; F(r, 0); KT_CHECK_EQ(o, ""); KT_CHECK_EQ(o[1], 'X'); F(r, saferead::RR_EMPTY_Q); KT_CHECK_EQ(o, "?"); KT_CHECK_EQ(o[2], 'X');
          F(r, saferead::RR_LOWER | saferead::RR_SPACE_BAD); KT_CHECK_EQ(o, ""); }
        { char r[16] = { 'a' }; F(r, saferead::RR_EMPTY_Q); KT_CHECK_EQ(o, "a"); }                                          // EMPTY_Q only for empty

        // readResRef16
        uintptr_t rr = arenaAlloc(16); std::memcpy(reinterpret_cast<void*>(rr), "Foo BAR\0xyz\0\0\0\0\0", 16);
        auto R = [&](uintptr_t a, unsigned fl) { std::memset(o, 'X', sizeof o); return saferead::readResRef16(6, a, o, fl); };
        KT_CHECK(R(rr, 0));                                                        KT_CHECK_EQ(o, "Foo BAR");
        KT_CHECK(R(rr, saferead::RR_LOWER | saferead::RR_SPACE_BAD | saferead::RR_EMPTY_Q)); KT_CHECK_EQ(o, "foo?bar");
        KT_CHECK(!R(0, 0));                                                        KT_CHECK_EQ(o, ""); KT_CHECK_EQ(o[1], 'X');
        KT_CHECK(!R(0, saferead::RR_EMPTY_Q));                                     KT_CHECK_EQ(o, "?"); KT_CHECK_EQ(o[2], 'X');
        KT_CHECK(!R(0x20, 0));                                                     KT_CHECK_EQ(o, "");
        KT_CHECK(!R(0x20, saferead::RR_EMPTY_Q));                                  KT_CHECK_EQ(o, "?");
        setTail(true);
        KT_CHECK(!R(pg + 0x1000 - 8, 0));                                          KT_CHECK_EQ(o, "");   // straddles into the unreadable page: all 16 or nothing
        KT_CHECK(R(pg + 0x1000 - 16, saferead::RR_EMPTY_Q));
        { uintptr_t z = arenaAlloc(16); KT_CHECK(R(z, 0)); KT_CHECK_EQ(o, ""); KT_CHECK(R(z, saferead::RR_EMPTY_Q)); KT_CHECK_EQ(o, "?"); }   // readable but empty
        setTail(false);

        // golden: legacy perf-lab readResRef (LOWER|SPACE_BAD|EMPTY_Q) and equip-icon resrefName (flags 0, up to the first NUL)
        {
            struct Row { unsigned char b[16]; };
            std::vector<Row> rows;
            auto add = [&](std::initializer_list<int> v) { Row r; std::memset(r.b, 0, 16); int i = 0; for (int x : v) r.b[i++] = (unsigned char)x; rows.push_back(r); };
            { Row r; std::memcpy(r.b, "ABCDEFGHIJKLMNOP", 16); rows.push_back(r); }
            { Row r; std::memcpy(r.b, "abcdefghijklmnop", 16); rows.push_back(r); }
            add({}); add({ 0x20 }); add({ 0x21 }); add({ 0x1F }); add({ 0x7E }); add({ 0x7F }); add({ 0x80 }); add({ 0xFF }); add({ 'A', 0, 'B' });
            add({ 'a', ' ', 'b' }); add({ ' ', ' ', ' ' }); add({ '@', '[', '`', '{', 'Z', 'a' }); add({ 0, 'a' }); add({ 'a', 0x09, 'b', 0x0A });
            add({ 'i', 'w', '_', 'b', 'l', 'a', 's', 't', 'e', 'r', '0', '1', 'A', 0x80, 0x7F, 0x20 });
            for (int c = 0; c < 256; ++c) add({ 'x', c, 'y' });
            for (int c = 0; c < 256; ++c) add({ c });
            for (const Row& r : rows) {
                uintptr_t a = arenaAlloc(16); std::memcpy(reinterpret_cast<void*>(a), r.b, 16);
                char x[17], y[17], z[17], w[17];
                std::memset(x, 'X', 17); std::memset(y, 'Y', 17);
                legacy_perflabResRef(a, x);
                KT_CHECK(saferead::readResRef16(62, a, y, saferead::RR_LOWER | saferead::RR_SPACE_BAD | saferead::RR_EMPTY_Q));
                KT_CHECK_EQ(std::strcmp(x, y), 0);
                std::memset(z, 'X', 17); std::memset(w, 'Y', 17);
                legacy_resrefName(r.b, z);
                saferead::formatResRef16(r.b, w, 0);
                KT_CHECK_EQ(std::strcmp(z, w), 0);   // legacy keeps bytes after an embedded NUL; the callers only print it with %s, so equality is up to the first NUL
            }
            // unreadable address: legacy perf-lab gives "?" too
            char x[17], y[17]; legacy_perflabResRef(0x20, x); saferead::readResRef16(62, 0x20, y, saferead::RR_LOWER | saferead::RR_SPACE_BAD | saferead::RR_EMPTY_Q);
            KT_CHECK_EQ(std::strcmp(x, y), 0);
            legacy_perflabResRef(0, x); saferead::readResRef16(62, 0, y, saferead::RR_LOWER | saferead::RR_SPACE_BAD | saferead::RR_EMPTY_Q);
            KT_CHECK_EQ(std::strcmp(x, y), 0);
        }
    }

    return kt::Report("saferead_test");
}
