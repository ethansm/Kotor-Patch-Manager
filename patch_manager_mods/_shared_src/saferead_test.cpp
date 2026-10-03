// Host test of saferead::readExo (SafeRead.h) on fake CExoStrings in a MAP_32BIT arena.
// Windows is the stub windows.h; g_stubVirtualQueryAll makes the arena read as committed memory.
#include <windows.h>
#include <sys/mman.h>
#include <initializer_list>
// Protection faking: the host stub's VirtualQuery reports every address as one committed PAGE_READWRITE region. Route SafeRead.h's
// VirtualQuery through a wrapper that can override State/Protect for a chosen range (and clip RegionSize at its edges) while
// forwarding everything else to the stub, so probeWrite's protection rules are testable without touching the shared stub.
struct FakeRange { uintptr_t lo, hi; DWORD protect, state; };
static FakeRange g_fake[8];
static int g_fakeN = 0;
static auto g_realVQ = &VirtualQuery;
static SIZE_T fakeVQ(LPCVOID a, MEMORY_BASIC_INFORMATION* m, SIZE_T n) {
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
        saferead::enableRing(true);
        saferead::probeRead(7, reinterpret_cast<const void*>(0x20), 4);   // one recorded failure
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

    return kt::Report("saferead_test");
}
