// Host test of saferead::readExo (SafeRead.h) on fake CExoStrings in a MAP_32BIT arena.
// Windows is the stub windows.h; g_stubVirtualQueryAll makes the arena read as committed memory.
#include "SafeRead.h"
#include "check.h"
#include <sys/mman.h>

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

    return kt::Report("saferead_test");
}
