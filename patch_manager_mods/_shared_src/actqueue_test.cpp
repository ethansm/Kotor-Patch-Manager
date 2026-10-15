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
    // truncation: 20 queue nodes keep the first 16 (kMaxEntries), 6 round nodes keep the first 4 (kMaxRound), in list order
    {
        const size_t tcr = 0x4000, tint = 0x4200, tnode = 0x4300, tdata = 0x4500;
        const size_t trnd = 0x5100, trl = 0x6000, trin = 0x6040, trnode = 0x6100, trdata = 0x6400;
        put<uint32_t>(tcr + CrQueue, (uint32_t)A(tint)); put<uint32_t>(tint, (uint32_t)A(tnode));
        for (int i = 0; i < 20; ++i) {
            const size_t nd = tnode + 0x10 * i, dd = tdata + 0x40 * i;
            put<uint32_t>(nd + NodeNext, i + 1 < 20 ? (uint32_t)A(nd + 0x10) : 0u); put<uint32_t>(nd + NodeData, (uint32_t)A(dd));
            put<uint32_t>(dd + ActType, 100 + i); put<uint32_t>(dd + ActParam0, i);
        }
        put<uint32_t>(tcr + CrRound, (uint32_t)A(trnd));
        put<uint32_t>(trnd + RdList, (uint32_t)A(trl)); put<uint32_t>(trl, (uint32_t)A(trin)); put<uint32_t>(trin, (uint32_t)A(trnode));
        for (int i = 0; i < 6; ++i) {
            const size_t nd = trnode + 0x10 * i, dd = trdata + 0x80 * i;
            put<uint32_t>(nd + NodeNext, i + 1 < 6 ? (uint32_t)A(nd + 0x10) : 0u); put<uint32_t>(nd + NodeData, (uint32_t)A(dd));
            put<uint8_t>(dd + RnSub, (uint8_t)(i + 1)); put<uint32_t>(dd + RnSpell, 0x100 + i);
            put<uint32_t>(dd + RnItem, 0x200 + i); put<uint16_t>(dd + RnFeat, (uint16_t)i);
        }
        KT_CHECK(read(A(tcr), &s, 0x7300, 0x7320));
        KT_CHECK_EQ(s.ne, 16); KT_CHECK_EQ(s.e[0].type, 100); KT_CHECK_EQ(s.e[15].type, 115); KT_CHECK_EQ((int)s.e[15].param0, 15);
        KT_CHECK_EQ(s.nrn, 4); KT_CHECK_EQ((int)s.rn[0].spellId, 0x100); KT_CHECK_EQ((int)s.rn[3].itemId, 0x203); KT_CHECK_EQ((int)s.rn[3].subType, 4);
        KT_CHECK(!s.haveCur);
    }
    // playerPaused() walks the fixed engine address AppGlobal: map that page (best effort; skipped when the host process already owns it)
    // and a second 32-bit region for the chain, then pin each hop and the bit-2 test.
    {
        const uintptr_t ag = gameaddr::AppGlobal;
        void* gp = mmap(reinterpret_cast<void*>(ag & ~(uintptr_t)0xFFF), 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
        void* ch = mmap(nullptr, 0x20000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS
#ifdef MAP_32BIT
                        | MAP_32BIT
#endif
                        , -1, 0);
        if (gp != MAP_FAILED && ch != MAP_FAILED && reinterpret_cast<uintptr_t>(gp) == (ag & ~(uintptr_t)0xFFF) && reinterpret_cast<uintptr_t>(ch) < 0x80000000u) {
            uint8_t* c = static_cast<uint8_t*>(ch);
            const uint32_t cApp = (uint32_t)reinterpret_cast<uintptr_t>(c), cSrv = cApp + 0x100, cInt = cApp + 0x200;   // internal + 0x10078 stays inside the 0x20000 region
            auto w32 = [&](uintptr_t addr, uint32_t v) { memcpy(reinterpret_cast<void*>(addr), &v, sizeof v); };
            KT_CHECK(!playerPaused(0x7400));                                   // AppGlobal -> NULL app: not paused
            w32(ag, cApp);
            KT_CHECK(!playerPaused(0x7400));                                   // app+8 (server) still NULL
            w32(cApp + 8, cSrv);
            KT_CHECK(!playerPaused(0x7400));                                   // server+4 (internal) still NULL
            w32(cSrv + 4, cInt);
            KT_CHECK(!playerPaused(0x7400));                                   // pause byte 0
            c[0x200 + PauseInternal] = 2;  KT_CHECK(playerPaused(0x7400));     // bit 2 set: paused
            c[0x200 + PauseInternal] = 0xFD; KT_CHECK(!playerPaused(0x7400));  // every other bit set, bit 2 clear: not paused
            c[0x200 + PauseInternal] = 0xFF; KT_CHECK(playerPaused(0x7400));
            w32(cSrv + 4, 0);              KT_CHECK(!playerPaused(0x7400));    // chain broken again: not paused
            munmap(gp, 0x1000); munmap(ch, 0x20000);
        } else {
            printf("actqueue_test: AppGlobal page not mappable on this host, playerPaused chain case skipped\n");
            if (gp != MAP_FAILED && reinterpret_cast<uintptr_t>(gp) == (ag & ~(uintptr_t)0xFFF)) munmap(gp, 0x1000);
            if (ch != MAP_FAILED) munmap(ch, 0x20000);
        }
    }
    return kt::Report("actqueue_test");
}
