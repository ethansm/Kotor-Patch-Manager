// tex_scan_cache_test.cpp -- Unit tests for TexScanCache.h (reuse decision, stats/verify cadence, rel32 and call-site patching).
#include "TexScanCache.h"
#include "check.h"
#include <string.h>

static void test_reuse() {
    texscan::State s;
    KT_CHECK(!s.valid);
    KT_CHECK(!texscan::canReuse(s, 0, 0, 0, true));          // nothing cached yet
    texscan::remember(s, 100, 7, 0, 500, 321);
    KT_CHECK(texscan::canReuse(s, 100, 7, 0, true));
    KT_CHECK(!texscan::canReuse(s, 101, 7, 0, true));        // new tick
    KT_CHECK(!texscan::canReuse(s, 100, 8, 0, true));        // sheet count changed
    KT_CHECK(!texscan::canReuse(s, 100, 7, 1, true));        // textures pending
    KT_CHECK(!texscan::canReuse(s, 100, 7, -1, true));       // any non-zero pending
    KT_CHECK(!texscan::canReuse(s, 100, 7, 0, false));       // disabled always misses
    KT_CHECK(s.count == 500 && s.maxId == 321);
    // tick 0 / sheets 0 are legal keys once remembered
    texscan::remember(s, 0, 0);
    KT_CHECK(texscan::canReuse(s, 0, 0, 0, true));
}

static void test_invalidate() {
    texscan::State s;
    texscan::remember(s, 5, 2, 0, 10, 20);
    texscan::invalidate(s);
    KT_CHECK(!s.valid);
    KT_CHECK(!texscan::canReuse(s, 5, 2, 0, true));
    // remember with pending != 0 does not cache (and drops any older entry)
    texscan::remember(s, 5, 2, 0, 10, 20);
    texscan::remember(s, 6, 2, 3, 10, 20);
    KT_CHECK(!s.valid);
    KT_CHECK(!texscan::canReuse(s, 6, 2, 0, true));
    KT_CHECK(!texscan::canReuse(s, 5, 2, 0, true));
}

static void test_verify_and_stats() {
    KT_CHECK(!texscan::dueVerify(5, 0));                     // 0 = never
    KT_CHECK(!texscan::dueVerify(0, 4));
    KT_CHECK(texscan::dueVerify(4, 4) && texscan::dueVerify(8, 4));
    KT_CHECK(!texscan::dueVerify(3, 4) && !texscan::dueVerify(5, 4));
    KT_CHECK(texscan::dueVerify(1, 1) && texscan::dueVerify(2, 1));

    KT_CHECK(texscan::sameResult(10, 99, 10, 99));
    KT_CHECK(!texscan::sameResult(10, 99, 11, 99));
    KT_CHECK(!texscan::sameResult(10, 99, 10, 98));

    texscan::Stats st;
    int due = 0;
    for (int i = 0; i < 10; ++i) if (texscan::onHit(st, 3)) ++due;
    KT_CHECK(st.hits == 10 && due == 3);                     // hits 3, 6, 9
    due = 0;
    texscan::Stats st0;
    for (int i = 0; i < 10; ++i) if (texscan::onHit(st0, 0)) ++due;
    KT_CHECK(due == 0 && st0.hits == 10);

    texscan::onMiss(st, 1000);
    texscan::onMiss(st, 234);
    KT_CHECK(st.misses == 2 && st.missTicks == 1234);
    texscan::onVerify(st, true);
    texscan::onVerify(st, false);
    KT_CHECK(st.verifies == 2 && st.mismatches == 1);
    st.calls = 12;

    char buf[200];
    texscan::formatStats(buf, sizeof buf, st);
    KT_CHECK(strstr(buf, "calls=12 hits=10 misses=2 verifies=2 mismatches=1 miss_ticks=1234") != nullptr);

    st.reset();
    KT_CHECK(st.calls == 0 && st.hits == 0 && st.misses == 0 && st.verifies == 0 && st.mismatches == 0 && st.missTicks == 0);
}

static void test_rel32() {
    // Reproduce the three original engine call bytes (negative displacements to 0x00427970).
    for (int i = 0; i < texscan::kSiteCount; ++i) {
        uint8_t out[5];
        texscan::buildCall(texscan::kSiteAddr[i], texscan::kOrigFn, out);
        KT_CHECK(memcmp(out, texscan::kSiteBytes[i], 5) == 0);
    }
    uint8_t out[5];
    texscan::buildCall(0x00466033, 0x00427970, out);
    const uint8_t want[5] = {0xE8, 0x38, 0x19, 0xFC, 0xFF};
    KT_CHECK(memcmp(out, want, 5) == 0);
    KT_CHECK(texscan::rel32(0x00466033, 0x00427970) == -0x3E6C8 );
    KT_CHECK(texscan::rel32(0x1000, 0x2000) == 0x0FFB);      // positive displacement
    KT_CHECK(texscan::rel32(0x1000, 0x1005) == 0);
}

// Host buffer standing in for the exe: three E8 call sites at distinct offsets.
struct Fake {
    uint8_t mem[64];
    uintptr_t addr[texscan::kSiteCount];
    static constexpr uintptr_t kOrig = 0x00427970, kNew = 0x10001000;
    void init() {
        memset(mem, 0x90, sizeof mem);
        for (int i = 0; i < texscan::kSiteCount; ++i) {
            addr[i] = reinterpret_cast<uintptr_t>(mem + 8 + i * 16);
            texscan::buildCall(addr[i], kOrig, mem + 8 + i * 16);
        }
    }
};

static void test_install_uninstall() {
    codepatch::ops();   // ensure default (host) ops
    g_stubVirtualQueryAll = true;
    Fake f;
    f.init();
    uint8_t orig[64];
    memcpy(orig, f.mem, sizeof orig);

    texscan::Sites s;
    texscan::initSites(s, f.addr, Fake::kOrig, Fake::kNew);
    KT_CHECK(texscan::install(s) == 3);
    for (int i = 0; i < 3; ++i) {
        KT_CHECK(s.site[i].rc == 1);
        uint8_t want[5];
        texscan::buildCall(f.addr[i], Fake::kNew, want);
        KT_CHECK(memcmp(f.mem + 8 + i * 16, want, 5) == 0);
        KT_CHECK(memcmp(s.site[i].seen, orig + 8 + i * 16, 5) == 0);
    }
    KT_CHECK(texscan::uninstall(s) == 3);
    KT_CHECK(memcmp(f.mem, orig, sizeof orig) == 0);
    // second uninstall is a no-op (rc reset)
    KT_CHECK(texscan::uninstall(s) == 0);

    // Mismatch on site 1: it stays unpatched and reports -1, the others still patch; uninstall restores only the patched ones.
    f.init();
    f.mem[8 + 16] = 0xE9;   // site 1 no longer a call
    memcpy(orig, f.mem, sizeof orig);
    texscan::initSites(s, f.addr, Fake::kOrig, Fake::kNew);
    KT_CHECK(texscan::install(s) == 2);
    KT_CHECK(s.site[0].rc == 1 && s.site[1].rc == -1 && s.site[2].rc == 1);
    KT_CHECK(memcmp(f.mem + 8 + 16, orig + 8 + 16, 5) == 0);      // untouched
    KT_CHECK(s.site[1].seen[0] == 0xE9);
    KT_CHECK(texscan::uninstall(s) == 2);
    KT_CHECK(memcmp(f.mem, orig, sizeof orig) == 0);

    // Uninstall leaves a site alone when someone else changed it after install.
    f.init();
    texscan::initSites(s, f.addr, Fake::kOrig, Fake::kNew);
    KT_CHECK(texscan::install(s) == 3);
    f.mem[8] = 0xE9;
    KT_CHECK(texscan::uninstall(s) == 2);
    KT_CHECK(s.site[0].restoreRc == -1 && f.mem[8] == 0xE9);
    g_stubVirtualQueryAll = false;

    // Not committed (stub VirtualQuery off): -4, nothing written.
    f.init();
    memcpy(orig, f.mem, sizeof orig);
    texscan::initSites(s, f.addr, Fake::kOrig, Fake::kNew);
    KT_CHECK(texscan::install(s) == 0);
    KT_CHECK(s.site[0].rc == -4 && memcmp(f.mem, orig, sizeof orig) == 0);
    KT_CHECK(texscan::uninstall(s) == 0);
}

static void test_cadence_scenario() {
    // 3 drains per frame, one tick per frame: first drain misses, next two hit; verify every 4th hit.
    texscan::State s;
    texscan::Stats st;
    uint32_t verifyEvery = 4;
    int verified = 0;
    for (uint32_t frame = 1; frame <= 8; ++frame) {
        for (int d = 0; d < 3; ++d) {
            ++st.calls;
            if (texscan::canReuse(s, frame, 5, 0, true)) {
                if (texscan::onHit(st, verifyEvery)) {
                    texscan::onVerify(st, texscan::sameResult(s.count, s.maxId, 40, 77));
                    ++verified;
                }
            } else {
                texscan::onMiss(st, 10);
                texscan::remember(s, frame, 5, 0, 40, 77);
            }
        }
    }
    KT_CHECK(st.calls == 24 && st.misses == 8 && st.hits == 16);
    KT_CHECK(verified == 4 && st.verifies == 4 && st.mismatches == 0);
    KT_CHECK(st.calls == st.hits + st.misses);
}

int main() {
    test_reuse();
    test_invalidate();
    test_verify_and_stats();
    test_rel32();
    test_install_uninstall();
    test_cadence_scenario();
    return kt::Report("tex_scan_cache_test");
}
