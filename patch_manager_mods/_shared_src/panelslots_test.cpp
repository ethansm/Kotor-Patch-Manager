// Host test of panelslots::PanelSlots (PanelSlots.h): the one slot table behind CharEffectsPanel (bound flag), Inventory3DViewport
// (newest-by-seq, no resources) and EquipCharPreview (destroy on re-bind / evict), plus eviction order, hook counts, release,
// forEach, N=1 and panel 0.
#include "PanelSlots.h"
#include "check.h"
#include <vector>

using namespace panelslots;

// ---- fixture A: CharEffectsPanel (payload carries `bound`; find() is gated on it by the mod) ----
struct SlotA { uintptr_t panel; bool bound; int rows[4]; };
static SlotA* findA(PanelSlots<SlotA, 4>& t, uintptr_t p) { SlotA* s = t.find(p); return s && s->bound ? s : nullptr; }

static void test_fixtureA() {
    PanelSlots<SlotA, 4> t; unsigned seq = 0;
    bool fresh = false;
    SlotA* s = t.claim(0x10, ++seq, &fresh);
    KT_CHECK(s && fresh);
    KT_CHECK(!s->bound && s->panel == 0 && s->rows[3] == 0);   // payload zeroed on fresh claim
    KT_CHECK(findA(t, 0x10) == nullptr);                       // claimed but not bound yet (page unusable path)
    s->panel = 0x10; s->bound = true; s->rows[1] = 7;
    KT_CHECK(findA(t, 0x10) == s);
    SlotA* again = t.claim(0x10, ++seq, &fresh);
    KT_CHECK(again == s && !fresh && again->bound && again->rows[1] == 7);   // re-claim keeps the payload
    s->bound = false; s->panel = 0;                            // the mod's "unbind"
    KT_CHECK(findA(t, 0x10) == nullptr);
    KT_CHECK(t.find(0x10) == s);                               // still claimed in the table until released
    KT_CHECK(t.release(0x10));
    KT_CHECK(t.find(0x10) == nullptr);
}

// ---- fixture I: Inventory3DViewport (seq-only, no hook; 4 slots; oldest evicted silently) ----
struct SlotI { uintptr_t panel; unsigned seq; int lastItem; bool usable; };
static void test_fixtureI() {
    PanelSlots<SlotI, 4> t; unsigned seq = 0;
    for (uintptr_t p = 1; p <= 4; ++p) { SlotI* s = t.claim(p, ++seq); s->panel = p; s->seq = seq; s->lastItem = (int)p * 10; s->usable = true; }
    KT_CHECK_EQ(t.count(), 4);
    SlotI* s5 = t.claim(5, ++seq);                    // all used: evicts panel 1 (seq 1)
    KT_CHECK(t.find(1) == nullptr);
    KT_CHECK(s5 && s5->lastItem == 0 && !s5->usable); // reused entry was reset
    for (uintptr_t p = 2; p <= 5; ++p) KT_CHECK(t.find(p) != nullptr);
    t.claim(2, ++seq);                                // touch panel 2 (now newest): panel 3 is the oldest
    t.claim(6, ++seq);
    KT_CHECK(t.find(3) == nullptr);
    KT_CHECK(t.find(2) && t.find(4) && t.find(5) && t.find(6));
    t.claim(7, ++seq);                                // oldest is now 4 (seq 4), then 5, 2, 6
    KT_CHECK(t.find(4) == nullptr && t.find(5) && t.find(2) && t.find(6) && t.find(7));
}

// ---- fixture E: EquipCharPreview (resources destroyed on evict AND on re-bind) ----
struct SlotE { void* block; int creature; int zoom; };
static std::vector<int> g_destroyed;          // creature ids destroyed, in order
static std::vector<unsigned long long> g_evictCtx;   // (panel << 32 | seq) seen by the hook
static void destroyE(SlotE& s, uintptr_t panel, unsigned seq) {
    if (s.block) g_destroyed.push_back(s.creature);
    g_evictCtx.push_back(((unsigned long long)panel << 32) | seq);
}
static int g_blocks[32];
static void test_fixtureE() {
    g_destroyed.clear(); g_evictCtx.clear();
    PanelSlots<SlotE, 3> t; t.onEvict = destroyE; unsigned seq = 0;
    auto bind = [&](uintptr_t panel, int creature) {
        bool fresh = false;
        SlotE* s = t.claim(panel, ++seq, &fresh);
        if (!fresh && s->block) g_destroyed.push_back(s->creature);   // re-bind teardown, done by the mod via *fresh
        s->block = &g_blocks[creature]; s->creature = creature; s->zoom = 1;
        return s;
    };
    bind(0xA, 1); bind(0xB, 2); bind(0xC, 3);
    KT_CHECK(g_destroyed.empty() && g_evictCtx.empty());   // free entries: no hook
    bind(0xB, 22);                                          // re-bind: mod destroys creature 2, hook not called
    KT_CHECK_EQ((int)g_destroyed.size(), 1); KT_CHECK_EQ(g_destroyed[0], 2);
    KT_CHECK(g_evictCtx.empty());
    bind(0xD, 4);                                           // evicts oldest: 0xA (seq 1)
    KT_CHECK_EQ((int)g_destroyed.size(), 2); KT_CHECK_EQ(g_destroyed[1], 1);
    KT_CHECK_EQ((int)g_evictCtx.size(), 1);
    KT_CHECK_EQ(g_evictCtx[0], (0xAull << 32) | 1ull);      // hook saw the OLD occupant's panel + seq
    KT_CHECK(t.find(0xA) == nullptr);
    SlotE* d = t.find(0xD); KT_CHECK(d && d->creature == 4 && d->zoom == 1);
    bind(0xE, 5);                                           // next oldest: 0xC (seq 3; 0xB was refreshed to seq 4)
    KT_CHECK_EQ(g_destroyed.back(), 3);
    KT_CHECK(t.find(0xC) == nullptr && t.find(0xB) && t.find(0xD) && t.find(0xE));
}

// ---- eviction order, hook counts ----
static int g_hookCalls; static unsigned g_hookSeq; static uintptr_t g_hookPanel; static int g_hookPayload;
struct P { int v; };
static void hook(P& s, uintptr_t panel, unsigned seq) { ++g_hookCalls; g_hookSeq = seq; g_hookPanel = panel; g_hookPayload = s.v; }

static void test_evict_order() {
    PanelSlots<P, 3> t; t.onEvict = hook; g_hookCalls = 0;
    t.claim(1, 30)->v = 100; t.claim(2, 10)->v = 200; t.claim(3, 20)->v = 300;   // seqs need not be claimed in order
    KT_CHECK_EQ(g_hookCalls, 0);
    t.claim(4, 40);
    KT_CHECK_EQ(g_hookCalls, 1); KT_CHECK_EQ(g_hookPanel, (uintptr_t)2); KT_CHECK_EQ(g_hookSeq, 10u); KT_CHECK_EQ(g_hookPayload, 200);
    t.claim(5, 50);
    KT_CHECK_EQ(g_hookCalls, 2); KT_CHECK_EQ(g_hookPanel, (uintptr_t)3); KT_CHECK_EQ(g_hookSeq, 20u); KT_CHECK_EQ(g_hookPayload, 300);
    t.claim(6, 60);
    KT_CHECK_EQ(g_hookCalls, 3); KT_CHECK_EQ(g_hookPanel, (uintptr_t)1); KT_CHECK_EQ(g_hookSeq, 30u); KT_CHECK_EQ(g_hookPayload, 100);
    // A later entry with the smaller seq is picked over an earlier one with a larger seq (no "first used wins").
    PanelSlots<P, 3> u; u.onEvict = hook; g_hookCalls = 0;
    u.claim(1, 5); u.claim(2, 9); u.claim(3, 7);
    u.claim(4, 10);
    KT_CHECK_EQ(g_hookPanel, (uintptr_t)1);
    PanelSlots<P, 3> w; w.onEvict = hook;
    w.claim(1, 9); w.claim(2, 5); w.claim(3, 7);
    w.claim(4, 10); KT_CHECK_EQ(g_hookPanel, (uintptr_t)2);   // smallest in the middle
    PanelSlots<P, 3> x; x.onEvict = hook;
    x.claim(1, 9); x.claim(2, 7); x.claim(3, 5);
    x.claim(4, 10); KT_CHECK_EQ(g_hookPanel, (uintptr_t)3);   // smallest last
    // Ties: lowest index goes first.
    PanelSlots<P, 3> y; y.onEvict = hook;
    y.claim(1, 4); y.claim(2, 4); y.claim(3, 4);
    y.claim(4, 5); KT_CHECK_EQ(g_hookPanel, (uintptr_t)1);
    y.claim(5, 6); KT_CHECK_EQ(g_hookPanel, (uintptr_t)2);
}

static void test_reclaim_no_evict() {
    PanelSlots<P, 2> t; t.onEvict = hook; g_hookCalls = 0;
    P* a = t.claim(1, 1); a->v = 11;
    t.claim(2, 2);
    bool fresh = true;
    P* a2 = t.claim(1, 3, &fresh);   // table is full, but panel 1 already has a slot
    KT_CHECK(a2 == a && !fresh && a2->v == 11);
    KT_CHECK_EQ(g_hookCalls, 0);
    KT_CHECK_EQ(t.count(), 2);
    t.claim(3, 4);                   // seq refresh on re-claim made panel 2 the oldest
    KT_CHECK_EQ(g_hookPanel, (uintptr_t)2); KT_CHECK_EQ(g_hookCalls, 1);
    KT_CHECK(t.find(1) == a && t.find(2) == nullptr);
    // fresh out-param is optional and true on a new binding, free-entry or eviction
    PanelSlots<P, 1> one;
    fresh = false; one.claim(9, 1, &fresh); KT_CHECK(fresh);
    fresh = false; one.claim(8, 2, &fresh); KT_CHECK(fresh);
    KT_CHECK(one.claim(8, 3, nullptr) != nullptr);
}

static void test_release() {
    PanelSlots<P, 2> t; t.onEvict = hook; g_hookCalls = 0;
    t.claim(1, 1)->v = 5; t.claim(2, 2)->v = 6;
    KT_CHECK(!t.release(99));
    KT_CHECK_EQ(t.count(), 2);
    KT_CHECK(t.release(1));
    KT_CHECK(t.find(1) == nullptr);
    KT_CHECK_EQ(t.count(), 1);
    KT_CHECK(!t.release(1));         // second release: nothing to free
    KT_CHECK_EQ(g_hookCalls, 0);     // release never calls the hook
    P* c = t.claim(3, 3);            // takes the freed entry: no eviction, hook silent, payload reset
    KT_CHECK_EQ(g_hookCalls, 0); KT_CHECK_EQ(c->v, 0);
    KT_CHECK(t.find(2) && t.find(2)->v == 6 && t.find(3) == c);
    // a released entry is preferred over evicting, even when it is not the first one
    KT_CHECK(t.release(3));
    t.claim(4, 0);                   // seq 0 is a legal (oldest) seq; still takes the free entry
    KT_CHECK_EQ(g_hookCalls, 0); KT_CHECK(t.find(2) && t.find(4));
    // released then re-claimed panel is fresh (payload reset)
    t.find(4)->v = 77;
    KT_CHECK(t.release(4));
    bool fresh = false; P* r = t.claim(4, 5, &fresh);
    KT_CHECK(fresh && r->v == 0);
}

static void test_foreach() {
    PanelSlots<P, 4> t;
    int n = 0, sum = 0;
    t.forEach([&](P&) { ++n; });
    KT_CHECK_EQ(n, 0);                                   // empty table
    t.claim(1, 1)->v = 1; t.claim(2, 2)->v = 10; t.claim(3, 3)->v = 100;
    t.release(2);
    t.forEach([&](P& s) { ++n; sum += s.v; });
    KT_CHECK_EQ(n, 2); KT_CHECK_EQ(sum, 101);            // released entry (v=10) not visited
    t.forEach([](P& s) { s.v += 1; });                   // mutation through the reference sticks
    KT_CHECK_EQ(t.find(1)->v, 2); KT_CHECK_EQ(t.find(3)->v, 101);
    // index order, not seq order
    PanelSlots<P, 3> u; std::vector<int> order;
    u.claim(7, 30)->v = 1; u.claim(8, 20)->v = 2; u.claim(9, 10)->v = 3;
    u.forEach([&](P& s) { order.push_back(s.v); });
    KT_CHECK_EQ((int)order.size(), 3); KT_CHECK(order == std::vector<int>({1, 2, 3}));
    // forEach is the way to release resources on shutdown: every used one, once
    int freed = 0; u.forEach([&](P&) { ++freed; }); KT_CHECK_EQ(freed, 3);
}

static void test_n1() {
    g_hookCalls = 0;
    PanelSlots<P, 1> t; t.onEvict = hook;
    KT_CHECK(t.find(1) == nullptr);
    P* a = t.claim(1, 1); a->v = 4;
    KT_CHECK(t.find(1) == a); KT_CHECK_EQ(g_hookCalls, 0);
    KT_CHECK(t.claim(1, 2) == a && a->v == 4); KT_CHECK_EQ(g_hookCalls, 0);
    P* b = t.claim(2, 3);                                // evicts the only entry
    KT_CHECK(b == a); KT_CHECK_EQ(g_hookCalls, 1); KT_CHECK_EQ(g_hookPanel, (uintptr_t)1); KT_CHECK_EQ(g_hookSeq, 2u); KT_CHECK_EQ(g_hookPayload, 4);
    KT_CHECK(b->v == 0 && t.find(1) == nullptr && t.find(2) == b);
    KT_CHECK(t.release(2)); KT_CHECK_EQ(t.count(), 0);
    t.claim(3, 4); KT_CHECK_EQ(g_hookCalls, 1);          // free again: no eviction
}

static void test_panel0_and_nohook() {
    PanelSlots<P, 2> t;                                  // no hook set: eviction must not crash
    KT_CHECK(t.find(0) == nullptr);                      // empty entries do not match panel 0
    P* z = t.claim(0, 1); z->v = 3;
    KT_CHECK(t.find(0) == z);                            // panel 0 is a normal key
    KT_CHECK(t.claim(0, 2) == z && z->v == 3);
    KT_CHECK_EQ(t.count(), 1);
    t.claim(1, 3);
    t.claim(2, 4);                                       // evicts panel 0 (seq 2), silently
    KT_CHECK(t.find(0) == nullptr && t.find(1) && t.find(2));
    PanelSlots<P, 2> u; u.claim(5, 1); u.release(5);
    KT_CHECK(u.find(0) == nullptr);                      // a released entry (panel field still 5) / default panel 0 never matches
    u.claim(0, 2); u.release(0);
    KT_CHECK(u.find(0) == nullptr && !u.release(0));
}

// Payload with a non-trivial default: claim resets to Slot(), i.e. the member initialisers apply again.
struct Q { int a = 42; char buf[3] = {1, 2, 3}; };
static void test_reset_default() {
    PanelSlots<Q, 1> t;
    Q* q = t.claim(1, 1);
    KT_CHECK(q->a == 42 && q->buf[2] == 3);
    q->a = 0; q->buf[2] = 9;
    KT_CHECK(t.claim(1, 2)->a == 0);                     // re-claim: untouched
    Q* r = t.claim(2, 3);
    KT_CHECK(r->a == 42 && r->buf[2] == 3);              // fresh: reset to Q()
}

int main() {
    test_fixtureA();
    test_fixtureI();
    test_fixtureE();
    test_evict_order();
    test_reclaim_no_evict();
    test_release();
    test_foreach();
    test_n1();
    test_panel0_and_nohook();
    test_reset_default();
    return kt::Report("panelslots_test");
}
