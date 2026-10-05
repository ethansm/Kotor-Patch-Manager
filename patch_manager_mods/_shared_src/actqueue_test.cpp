// Host test of ActQueue.h: the pure helpers (hasType / headType / starved / useQueued) and read() on a fake creature in a MAP_32BIT arena.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "ActQueue.h"
#include "check.h"
#include <sys/mman.h>
#include <string.h>
#include <initializer_list>

using namespace actq;

static_assert(CrQueue == 0x100 && CrRound == 0x10DC, "creature offsets");
static_assert(RdList == 0xB00 && RdCur == 0xB18 && RdAB8 == 0xAB8 && RdAA8 == 0xAA8, "round offsets");
static_assert(RnItem == 0x64 && ActParam0 == 0x38, "node / action offsets");

static Snapshot snapOf(std::initializer_list<int> types) {
    Snapshot s;
    for (int t : types) { s.e[s.ne].type = t; ++s.ne; }
    return s;
}

static uint8_t* g_a;
static uintptr_t A(size_t off) { return reinterpret_cast<uintptr_t>(g_a + off); }
template <typename T> static void put(size_t off, T v) { memcpy(g_a + off, &v, sizeof v); }

int main() {
    // ---- pure helpers
    static_assert(CrRunning == 0x20C && Cr11AC == 0x11AC && RdA84 == 0xA84, "R8 gate offsets");
    auto withRound = [](Snapshot s) { s.rn[0].itemId = 0x7F000009u; s.nrn = 1; return s; };
    KT_CHECK(starved(withRound(snapOf({0x3D, 0x3F}))));                  // k=1
    KT_CHECK_EQ(firstIndex(snapOf({0x3D, 0x3D, 0x3F}), 0x3F), 2);
    KT_CHECK(starved(withRound(snapOf({0x3D, 0x3D, 0x3F}))));            // k=2
    KT_CHECK(starved(withRound(snapOf({0x3D, 0x3D, 0x3D, 0x3F}))));      // k=3
    KT_CHECK(!starved(withRound(snapOf({0x3D, 0x3D, 0x3D, 0x3D, 0x3F}))));   // k=4: out of the rotate window
    KT_CHECK(!starved(withRound(snapOf({0x3D, 0x0C, 0x3F}))));           // a non-0x3D ahead of the 0x3F
    KT_CHECK(!starved(withRound(snapOf({0x3F, 0x3D}))));                 // 0x3F already at the head (k=0)
    KT_CHECK(!starved(withRound(snapOf({0x03, 0x40, 0x3D, 0x3F}))));     // MoveToPoint / 0x40 ahead: wait
    KT_CHECK(!starved(snapOf({0x3D, 0x3F})));                            // leftover 0x3F, empty round: harmless (R8 Q3)
    KT_CHECK(!useQueued(snapOf({0x3D, 0x3F})));
    KT_CHECK(!starved(withRound(snapOf({0x3D}))));
    KT_CHECK(!starved(withRound(snapOf({0x3D, 0x3D}))));
    KT_CHECK(!starved(Snapshot()));
    KT_CHECK_EQ(firstIndex(Snapshot(), 0x3F), -1);
    KT_CHECK_EQ(firstIndex(snapOf({0x3D}), 0x3F), -1);
    KT_CHECK_EQ(headType(Snapshot()), -1);
    KT_CHECK_EQ(headType(snapOf({0x3D, 0x3F})), 0x3D);
    KT_CHECK(hasType(snapOf({1, 2, 0x3F}), 0x3F));
    KT_CHECK(!hasType(snapOf({1, 2}), 0x3F));
    KT_CHECK(!hasType(Snapshot(), 0x3F));
    { Snapshot s = withRound(snapOf({0x3D, 0x3F})); s.ne = 1; KT_CHECK(!starved(s)); }   // entries beyond ne are ignored
    KT_CHECK(!useQueued(snapOf({0x3F})));
    KT_CHECK(!useQueued(snapOf({0x3D})));
    KT_CHECK(!useQueued(Snapshot()));
    { Snapshot s; s.rn[0].itemId = 0; s.rn[1].itemId = 0x7F000009u; s.nrn = 2; KT_CHECK(useQueued(s)); }   // via the round only
    { Snapshot s; s.rn[0].itemId = 0; s.rn[1].itemId = 5; s.nrn = 1; KT_CHECK(!useQueued(s)); }          // node past nrn ignored
    { Snapshot s; s.cur.itemId = 9; s.haveCur = true; KT_CHECK(useQueued(s)); }                            // the current node with an item id counts
    { Snapshot s; s.cur.itemId = 9; s.haveCur = false; KT_CHECK(!useQueued(s)); }
    { Snapshot s; s.cur.itemId = 0; s.haveCur = true; KT_CHECK(!useQueued(s)); }

    // ---- read() on a fake creature
    void* m = mmap(nullptr, 0x10000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS
#ifdef MAP_32BIT
                   | MAP_32BIT
#endif
                   , -1, 0);
    if (m == MAP_FAILED) { perror("mmap"); return 5; }
    g_a = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    const size_t cr = 0x1000, lst = 0x2000, n1 = 0x2100, n2 = 0x2110, d1 = 0x2200, d2 = 0x2300, rnd = 0x3000, rl = 0x3200, rin = 0x3210, rn1 = 0x3220, rd1 = 0x3300, curN = 0x3400;
    put<uint32_t>(cr + CrQueue, (uint32_t)A(lst)); put<uint32_t>(lst, (uint32_t)A(n1));
    put<uint32_t>(n1 + NodeNext, (uint32_t)A(n2)); put<uint32_t>(n1 + NodeData, (uint32_t)A(d1));
    put<uint32_t>(n2 + NodeNext, 0); put<uint32_t>(n2 + NodeData, (uint32_t)A(d2));
    put<uint32_t>(d1 + ActType, 0x3D); put<uint32_t>(d1 + ActParam0, 77);
    put<uint32_t>(d2 + ActType, 0x3F); put<uint32_t>(d2 + ActParam0, 88);
    put<uint32_t>(cr + CrRound, (uint32_t)A(rnd));
    put<uint32_t>(rnd + RdList, (uint32_t)A(rl)); put<uint32_t>(rl, (uint32_t)A(rin)); put<uint32_t>(rin, (uint32_t)A(rn1));
    put<uint32_t>(rn1 + NodeNext, 0); put<uint32_t>(rn1 + NodeData, (uint32_t)A(rd1));
    put<uint8_t>(rd1 + RnSub, 10); put<uint32_t>(rd1 + RnSpell, 3); put<uint32_t>(rd1 + RnItem, 0x55); put<uint16_t>(rd1 + RnFeat, 4);
    put<uint32_t>(rnd + RdCur, (uint32_t)A(curN)); put<uint8_t>(curN + RnSub, 9); put<uint32_t>(curN + RnSpell, 21);
    put<int32_t>(rnd + RdAB8, 2); put<int32_t>(rnd + RdAA8, 1);

    Snapshot s;
    KT_CHECK(!read(0, &s, 0x7100, 0x7120));
    KT_CHECK(read(A(cr), &s, 0x7100, 0x7120));
    KT_CHECK_EQ(s.ne, 2);
    KT_CHECK_EQ(s.e[0].type, 0x3D); KT_CHECK_EQ((int)s.e[0].param0, 77); KT_CHECK(s.eData[0] == A(d1));
    KT_CHECK_EQ(s.e[1].type, 0x3F); KT_CHECK_EQ((int)s.e[1].param0, 88);
    KT_CHECK(starved(s)); KT_CHECK(useQueued(s));
    KT_CHECK_EQ(s.nrn, 1); KT_CHECK_EQ((int)s.rn[0].subType, 10); KT_CHECK_EQ((int)s.rn[0].itemId, 0x55); KT_CHECK_EQ((int)s.rn[0].featId, 4); KT_CHECK(s.rn[0].ptr == (uint32_t)A(rd1));
    KT_CHECK(s.haveCur); KT_CHECK_EQ((int)s.cur.subType, 9); KT_CHECK_EQ((int)s.cur.spellId, 21);
    KT_CHECK_EQ(s.ab8, 2); KT_CHECK_EQ(s.aa8, 1);
    // unreadable action data keeps its index with type -1
    put<uint32_t>(n2 + NodeData, 0x10);   // low address: unmapped for the MAP_32BIT arena
    KT_CHECK(read(A(cr), &s, 0x7100, 0x7120));
    KT_CHECK_EQ(s.ne, 2); KT_CHECK_EQ(s.e[1].type, -1); KT_CHECK(!starved(s));
    // no queue, no round
    put<uint32_t>(cr + CrQueue, 0); put<uint32_t>(cr + CrRound, 0);
    KT_CHECK(read(A(cr), &s, 0x7100, 0x7120));
    KT_CHECK_EQ(s.ne, 0); KT_CHECK_EQ(s.nrn, 0); KT_CHECK(!s.haveCur); KT_CHECK(!useQueued(s));
    // playerPaused() walks the fixed engine address AppGlobal (not mappable in the host arena), so it is not host-testable; QueueHud's tests keep the behaviour pinned.
    return kt::Report("actqueue_test");
}
