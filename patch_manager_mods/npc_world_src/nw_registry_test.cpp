// Host test of the npc-world registry and pure helpers (NwCore.h): token handling, kind masks, dispatch order, snapshot rules for
// re-entrant register/unregister, the slow-consumer guard, string/orientation/salt helpers. World state rules are nw_world_test.cpp.
#include "NwCore.h"
#include "check.h"
#include <math.h>
#include <string.h>
#include <vector>

#if UINTPTR_MAX == 0xffffffff
static_assert(sizeof(K2NwEvent) == 60, "K2NwEvent size (m32)");
static_assert(sizeof(K2NpcWorldApi) == 40, "K2NpcWorldApi size (m32)");
#endif

static std::vector<int> g_order;
static int g_hits = 0, g_lastKind = 0;
static void KPATCH_CALL cbA(const K2NwEvent* ev, void*) { g_order.push_back(1); ++g_hits; g_lastKind = ev->kind; }
static void KPATCH_CALL cbB(const K2NwEvent*, void*) { g_order.push_back(2); ++g_hits; }
static void KPATCH_CALL cbC(const K2NwEvent*, void*) { g_order.push_back(3); ++g_hits; }

static nw::Dispatcher* g_d = nullptr;
static int32_t g_tokB = 0, g_tokNew = 0;
static void KPATCH_CALL cbRegisters(const K2NwEvent*, void*) { g_order.push_back(10); g_tokNew = g_d->add(K2NW_ALL, cbC, nullptr, "late"); }
static void KPATCH_CALL cbKillsB(const K2NwEvent*, void*) { g_order.push_back(11); g_d->remove(g_tokB); }

static uint32_t g_clock = 0, g_step = 0;
static uint32_t fakeNow(void*) { uint32_t t = g_clock; g_clock += g_step; return t; }
static int g_slowSeen = 0; static uint32_t g_slowUs = 0; static int32_t g_slowKind = 0;
static void onSlow(void*, const nw::Entry&, uint32_t us, int32_t kind) { ++g_slowSeen; g_slowUs = us; g_slowKind = kind; }

static K2NwEvent ev(int32_t kind) { K2NwEvent e = {}; e.struct_size = sizeof e; e.kind = kind; e.area = ""; e.tag = ""; return e; }

int main() {
    // 1. add / remove / limits
    {
        nw::Dispatcher d;
        KT_CHECK_EQ(d.add(K2NW_ALL, nullptr, nullptr, "x"), -1);              // null fn
        KT_CHECK_EQ(d.add(0, cbA, nullptr, "x"), -1);                          // no kind bit
        KT_CHECK_EQ(d.add(0x100, cbA, nullptr, "x"), -1);                      // only unknown bits
        const int32_t t1 = d.add(K2NW_AREA_LOAD | 0x100, cbA, nullptr, "a");   // unknown bits dropped
        KT_CHECK_EQ(t1, 1); KT_CHECK_EQ(d.e[0].mask, K2NW_AREA_LOAD);
        KT_CHECK_EQ(d.add(K2NW_ALL, cbB, nullptr, nullptr), 2); KT_CHECK(d.e[1].owner != nullptr);   // null owner -> ""
        KT_CHECK(!d.remove(99)); KT_CHECK(d.remove(1)); KT_CHECK(!d.remove(1)); KT_CHECK_EQ(d.n, 1);
        nw::Dispatcher f;
        for (int i = 0; i < nw::MaxConsumers; ++i) KT_CHECK(f.add(K2NW_TICK, cbA, nullptr, "f") > 0);
        KT_CHECK_EQ(f.add(K2NW_TICK, cbA, nullptr, "f"), -2);                  // table full
        KT_CHECK_EQ(f.count(K2NW_TICK), nw::MaxConsumers); KT_CHECK_EQ(f.count(K2NW_AREA_LOAD), 0);
    }
    // 2. dispatch: mask, order, invalid kinds
    {
        nw::Dispatcher d; g_order.clear(); g_hits = 0;
        d.add(K2NW_AREA_STABLE, cbA, nullptr, "a"); d.add(K2NW_ALL, cbB, nullptr, "b"); d.add(K2NW_TICK, cbC, nullptr, "c");
        KT_CHECK_EQ(d.dispatch(ev(K2NW_AREA_STABLE)), 2);
        KT_CHECK(g_order == (std::vector<int>{1, 2}));
        KT_CHECK_EQ(g_lastKind, K2NW_AREA_STABLE);
        KT_CHECK_EQ(d.dispatch(ev(K2NW_CREATURE_ADDED)), 1);
        KT_CHECK_EQ(d.dispatch(ev(0)), 0);                                     // no bit
        KT_CHECK_EQ(d.dispatch(ev(K2NW_AREA_LOAD | K2NW_TICK)), 0);            // two bits
        KT_CHECK_EQ(d.dispatch(ev(0x40)), 0);                                  // unknown bit
        nw::Dispatcher empty; KT_CHECK_EQ(empty.dispatch(ev(K2NW_TICK)), 0);
    }
    // 3. snapshot rules
    {
        nw::Dispatcher d; g_d = &d; g_order.clear();
        d.add(K2NW_TICK, cbRegisters, nullptr, "r"); g_tokB = d.add(K2NW_TICK, cbB, nullptr, "b");
        KT_CHECK_EQ(d.dispatch(ev(K2NW_TICK)), 2);                             // consumer registered by a callback waits
        KT_CHECK(g_order == (std::vector<int>{10, 2}));
        g_order.clear();
        KT_CHECK_EQ(d.dispatch(ev(K2NW_TICK)), 3);                             // next event: r, b and the consumer r registered before (r registers another)
        d.remove(g_tokNew);
        nw::Dispatcher k; g_d = &k; g_order.clear();
        k.add(K2NW_TICK, cbKillsB, nullptr, "k"); g_tokB = k.add(K2NW_TICK, cbB, nullptr, "b");
        KT_CHECK_EQ(k.dispatch(ev(K2NW_TICK)), 1);                             // B unregistered by the earlier callback: not called
        KT_CHECK(g_order == (std::vector<int>{11}));
    }
    // 4. slow-consumer guard
    {
        nw::Dispatcher d; d.add(K2NW_ALL, cbA, nullptr, "slow");
        d.timing.now = fakeNow; d.timing.slow = onSlow;
        g_clock = 0; g_step = nw::SlowThresholdUs;                             // exactly the limit: not slow
        d.dispatch(ev(K2NW_TICK)); KT_CHECK_EQ(g_slowSeen, 0);
        g_step = nw::SlowThresholdUs + 1;
        d.dispatch(ev(K2NW_AREA_LOAD)); KT_CHECK_EQ(g_slowSeen, 1); KT_CHECK_EQ(g_slowUs, nw::SlowThresholdUs + 1); KT_CHECK_EQ(g_slowKind, K2NW_AREA_LOAD);
        KT_CHECK_EQ(d.slowCalls, 1u);
        g_clock = 0xFFFFFFF0u; g_step = 6000;                                  // wraps at 2^32, still measured right
        d.dispatch(ev(K2NW_TICK)); KT_CHECK_EQ(g_slowUs, 6000u);
        d.timing.now = nullptr; g_slowSeen = 0;
        d.dispatch(ev(K2NW_TICK)); KT_CHECK_EQ(g_slowSeen, 0);                 // guard off
    }
    // 5. helpers
    {
        KT_CHECK(nw::ciEqualsN("MouseDroid", "mousedroid", 32)); KT_CHECK(!nw::ciEqualsN("abc", "abd", 3)); KT_CHECK(nw::ciEqualsN("abc", "abd", 2));
        KT_CHECK(!nw::ciEqualsN("abc", "ab", 5)); KT_CHECK(nw::ciEqualsN("", "", 5));
        char full[17]; memset(full, 'a', 16); full[16] = 0;
        char full2[17]; memcpy(full2, full, 17); full2[15] = 'b';
        KT_CHECK(nw::resrefEquals(full, full)); KT_CHECK(!nw::resrefEquals(full, full2)); KT_CHECK(!nw::resrefEquals(nullptr, "x")); KT_CHECK(!nw::resrefEquals("x", nullptr));
        KT_CHECK(nw::resrefEquals("204TEL", "204tel"));
        const nw::Vec3 o = nw::orientationFromFacing(0.0f); KT_CHECK(o.x == 1.0f && o.y == 0.0f && o.z == 0.0f);
        const nw::Vec3 q = nw::orientationFromFacing(float(M_PI) / 2); KT_CHECK(fabsf(q.x) < 1e-5f && fabsf(q.y - 1.0f) < 1e-5f);
        const nw::Vec3 n = nw::orientationFromFacing(NAN); KT_CHECK(n.x == 1.0f && n.y == 0.0f);
        const nw::Vec3 i = nw::orientationFromFacing(INFINITY); KT_CHECK(i.x == 1.0f && i.y == 0.0f);
        KT_CHECK_EQ(nw::fmix32(0), 0u); KT_CHECK_EQ(nw::fmix32(1), 0x514E28B7u);   // murmur3 fmix32 reference values
        for (uint32_t e = 0; e < 5000; ++e) KT_CHECK(nw::makeVisitSalt(int32_t(e % 7), e) != 0);
        KT_CHECK(nw::makeVisitSalt(1, 5) != nw::makeVisitSalt(2, 5)); KT_CHECK(nw::makeVisitSalt(1, 5) != nw::makeVisitSalt(1, 6));
        KT_CHECK_EQ(nw::makeVisitSalt(3, 9), nw::makeVisitSalt(3, 9));
        char b[6]; memset(b, 'x', sizeof b);
        KT_CHECK_EQ(nw::copyText("abcdefgh", b, sizeof b), 8); KT_CHECK(strcmp(b, "abcde") == 0);
        KT_CHECK_EQ(nw::copyText("", b, sizeof b), 0); KT_CHECK_EQ(b[0], 0);
        KT_CHECK_EQ(nw::copyText("abc", nullptr, 0), 3); KT_CHECK_EQ(nw::copyText("abc", b, 0), 3);
        KT_CHECK(nw::singleKind(K2NW_TICK)); KT_CHECK(!nw::singleKind(0)); KT_CHECK(!nw::singleKind(3)); KT_CHECK(!nw::singleKind(16));
    }
    return kt::Report("nw_registry_test");
}
