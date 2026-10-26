// mem_status_cache_test.cpp -- Unit tests for MemStatusCache.h.
#include "MemStatusCache.h"
#include "check.h"
#include <string.h>

static MEMORYSTATUS mk(unsigned avail) {
    MEMORYSTATUS m;
    memset(&m, 0, sizeof(m));
    m.dwLength = 0;   // the real API sets it; the cache must too
    m.dwMemoryLoad = 42;
    m.dwTotalPhys = 8000;
    m.dwAvailPhys = avail;
    m.dwTotalVirtual = 2000;
    m.dwAvailVirtual = 1500;
    return m;
}

static void test_interval() {
    memstat::State s{};
    KT_CHECK(memstat::needRefresh(s, 1000, 250, true));   // nothing cached yet
    memstat::store(s, mk(111), 1000);
    KT_CHECK(!memstat::needRefresh(s, 1000, 250, true));
    KT_CHECK(!memstat::needRefresh(s, 1249, 250, true));
    KT_CHECK(memstat::needRefresh(s, 1250, 250, true));
    KT_CHECK(memstat::needRefresh(s, 5000, 250, true));
    memstat::store(s, mk(222), 1250);
    KT_CHECK(!memstat::needRefresh(s, 1300, 250, true));
}

static void test_wrap() {
    memstat::State s{};
    memstat::store(s, mk(1), 0xFFFFFF00u);
    KT_CHECK(!memstat::needRefresh(s, 0xFFFFFF80u, 250, true));
    KT_CHECK(!memstat::needRefresh(s, 0x00000050u, 500, true));   // 0x150 = 336 ms elapsed
    KT_CHECK(memstat::needRefresh(s, 0x00000050u, 250, true));
    KT_CHECK(memstat::needRefresh(s, 0x00000200u, 500, true));    // 0x300 = 768 ms
}

static void test_always_refresh() {
    memstat::State s{};
    memstat::store(s, mk(1), 1000);
    KT_CHECK(memstat::needRefresh(s, 1000, 0, true));      // interval 0
    KT_CHECK(memstat::needRefresh(s, 1001, 0, true));
    KT_CHECK(memstat::needRefresh(s, 1000, 250, false));   // disabled
    KT_CHECK(memstat::needRefresh(s, 1000, 0, false));
    // Over-large interval is clamped inside needRefresh too.
    KT_CHECK(!memstat::needRefresh(s, 1000 + 4999, 99999, true));
    KT_CHECK(memstat::needRefresh(s, 1000 + 5000, 99999, true));
}

static void test_clamp() {
    KT_CHECK_EQ(memstat::clampInterval(-5), 0u);
    KT_CHECK_EQ(memstat::clampInterval(0), 0u);
    KT_CHECK_EQ(memstat::clampInterval(250), 250u);
    KT_CHECK_EQ(memstat::clampInterval(5000), 5000u);
    KT_CHECK_EQ(memstat::clampInterval(5001), 5000u);
    KT_CHECK_EQ(memstat::clampInterval(1000000), 5000u);
    KT_CHECK_EQ(memstat::kDefaultIntervalMs, 250u);
}

static void test_copyout() {
    memstat::State s{};
    MEMORYSTATUS out;
    memset(&out, 0xAB, sizeof(out));
    memstat::copyOut(s, &out);   // nothing cached: zeroed, length set
    KT_CHECK_EQ(static_cast<unsigned>(out.dwLength), static_cast<unsigned>(sizeof(MEMORYSTATUS)));
    KT_CHECK_EQ(static_cast<unsigned long long>(out.dwAvailPhys), 0ull);

    memstat::store(s, mk(12345), 10);
    memset(&out, 0xAB, sizeof(out));
    memstat::copyOut(s, &out);
    KT_CHECK_EQ(static_cast<unsigned>(out.dwLength), static_cast<unsigned>(sizeof(MEMORYSTATUS)));
    KT_CHECK_EQ(static_cast<unsigned>(out.dwMemoryLoad), 42u);
    KT_CHECK_EQ(static_cast<unsigned long long>(out.dwTotalPhys), 8000ull);
    KT_CHECK_EQ(static_cast<unsigned long long>(out.dwAvailPhys), 12345ull);
    KT_CHECK_EQ(static_cast<unsigned long long>(out.dwTotalVirtual), 2000ull);
    KT_CHECK_EQ(static_cast<unsigned long long>(out.dwAvailVirtual), 1500ull);
    memstat::copyOut(s, nullptr);   // must not crash
}

static void test_stats() {
    memstat::Stats st{};
    st.calls = 10; st.realCalls = 3; st.realTicks = 999;
    st.reset();
    KT_CHECK(st.calls == 0 && st.realCalls == 0 && st.realTicks == 0);
}

static void test_patch_bytes() {
    uint8_t b[memstat::kPatchLen];
    memstat::buildPatch(0x10203040u, b);
    const uint8_t want[6] = { 0xFF, 0x15, 0x40, 0x30, 0x20, 0x10 };
    KT_CHECK(memcmp(b, want, 6) == 0);
    memstat::buildPatch(0xDEADBEEFu, b);
    KT_CHECK(b[2] == 0xEF && b[3] == 0xBE && b[4] == 0xAD && b[5] == 0xDE);
    const uint8_t orig[6] = { 0xFF, 0x15, 0x28, 0x61, 0x98, 0x00 };
    KT_CHECK(memcmp(memstat::kOrigCall, orig, 6) == 0);
}

static void test_install_uninstall() {
    g_stubVirtualQueryAll = true;   // host stub: every address reads as committed
    alignas(8) uint8_t buf[16];
    memset(buf, 0x90, sizeof(buf));
    memcpy(buf + 4, memstat::kOrigCall, 6);
    uintptr_t site = reinterpret_cast<uintptr_t>(buf + 4);
    const uint32_t var = 0x11223344u;
    const uint8_t patched[6] = { 0xFF, 0x15, 0x44, 0x33, 0x22, 0x11 };

    memstat::PatchResult r = memstat::install(site, memstat::kOrigCall, var);
    KT_CHECK_EQ(r.rc, 1);
    KT_CHECK(memcmp(r.seen, memstat::kOrigCall, 6) == 0);
    KT_CHECK(memcmp(buf + 4, patched, 6) == 0);
    KT_CHECK(buf[3] == 0x90 && buf[10] == 0x90);   // neighbours untouched

    // Second install: site no longer matches -> -1, unchanged, seen reports the patched bytes.
    r = memstat::install(site, memstat::kOrigCall, var);
    KT_CHECK_EQ(r.rc, -1);
    KT_CHECK(memcmp(r.seen, patched, 6) == 0);
    KT_CHECK(memcmp(buf + 4, patched, 6) == 0);

    // Uninstall with a failed install rc does nothing.
    memstat::PatchResult u = memstat::uninstall(site, memstat::kOrigCall, var, -1);
    KT_CHECK_EQ(u.rc, 0);
    KT_CHECK(memcmp(buf + 4, patched, 6) == 0);

    // Uninstall after rc==1 restores the original bytes.
    u = memstat::uninstall(site, memstat::kOrigCall, var, 1);
    KT_CHECK_EQ(u.rc, 1);
    KT_CHECK(memcmp(buf + 4, memstat::kOrigCall, 6) == 0);

    // Uninstall when someone else changed the site: refuses (-1), leaves it.
    buf[6] = 0x77;
    u = memstat::uninstall(site, memstat::kOrigCall, var, 1);
    KT_CHECK_EQ(u.rc, -1);
    KT_CHECK(buf[6] == 0x77);

    // Mismatching bytes at install: unchanged, -1.
    uint8_t other[6] = { 0xE8, 1, 2, 3, 4, 5 };
    memcpy(buf + 4, other, 6);
    r = memstat::install(site, memstat::kOrigCall, var);
    KT_CHECK_EQ(r.rc, -1);
    KT_CHECK(memcmp(buf + 4, other, 6) == 0);
    KT_CHECK(memcmp(r.seen, other, 6) == 0);

    // Not committed (stub probe off): -4, seen zeroed, nothing written.
    g_stubVirtualQueryAll = false;
    memcpy(buf + 4, memstat::kOrigCall, 6);
    r = memstat::install(site, memstat::kOrigCall, var);
    KT_CHECK_EQ(r.rc, -4);
    KT_CHECK(memcmp(buf + 4, memstat::kOrigCall, 6) == 0);
    g_stubVirtualQueryAll = true;
}

int main() {
    test_interval();
    test_wrap();
    test_always_refresh();
    test_clamp();
    test_copyout();
    test_stats();
    test_patch_bytes();
    test_install_uninstall();
    return kt::Report("mem_status_cache_test");
}
