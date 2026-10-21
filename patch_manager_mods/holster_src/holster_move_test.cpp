// Host tests for HolsterMove.h (move hold + party combat hold of the holstered-weapons mod, 0.6.0-dev; doc 42).
// Build/run: python3 scripts/run_tests.py holster_move
#include "HolsterMove.h"
#include "check.h"
#include <stdint.h>

using namespace hs;

static void testLocoIds() {
    KT_CHECK(isLocoAnim(0x2712u));
    KT_CHECK(isLocoAnim(0x2713u));
    KT_CHECK(isLocoAnim(0x2714u));
    KT_CHECK(!isLocoAnim(0x2711u));
    KT_CHECK(!isLocoAnim(0x2715u));
    KT_CHECK(!isLocoAnim(0x2710u));
    KT_CHECK(!isLocoAnim(0x0712u));
    KT_CHECK(!isLocoAnim(0u));
    KT_CHECK(!isLocoAnim(0xFFFFu));
    KT_CHECK(isLocoAnim(0xABCD2712u));    // only the low 16 bits count
    KT_CHECK(isLocoAnim(0x00012714u));
    KT_CHECK(!isLocoAnim(0x27120000u));
}

static void testHoldBoundary() {
    MoveTable t;
    t.note(0x1000, 5000);
    KT_CHECK(t.movedWithin(0x1000, 5000, 1500));          // age 0
    KT_CHECK(t.movedWithin(0x1000, 6499, 1500));
    KT_CHECK(t.movedWithin(0x1000, 6500, 1500));          // age == holdMs: inside
    KT_CHECK(!t.movedWithin(0x1000, 6501, 1500));         // holdMs + 1: out
    KT_CHECK(!t.movedWithin(0x1000, 6500, 0));            // holdMs 0 = off
    KT_CHECK(!t.movedWithin(0x1000, 5000, 0));
    KT_CHECK(t.movedWithin(0x1000, 5000, 1));
    KT_CHECK(t.movedWithin(0x1000, 5001, 1));
    KT_CHECK(!t.movedWithin(0x1000, 5002, 1));
    KT_CHECK(!t.movedWithin(0x2000, 5000, 1500));         // unknown owner
    KT_CHECK(!t.movedWithin(0, 5000, 1500));              // owner 0
}

static void testNegativeAge() {
    MoveTable t;
    t.note(0x1000, 5000);
    KT_CHECK(!t.movedWithin(0x1000, 4999, 1500));         // tick from the future
    KT_CHECK(!t.movedWithin(0x1000, 4000, 10000));
}

static void testWrap() {
    MoveTable t;
    t.note(0x1000, 0xFFFFFF00u);
    KT_CHECK(t.movedWithin(0x1000, 0xFFFFFF00u, 1500));
    KT_CHECK(t.movedWithin(0x1000, 0xFFFFFFFFu, 1500));
    KT_CHECK(t.movedWithin(0x1000, 0u, 1500));            // age 256 across the wrap
    KT_CHECK(t.movedWithin(0x1000, 0x100u, 512));         // age 512 == holdMs
    KT_CHECK(!t.movedWithin(0x1000, 0x101u, 512));        // 513
    KT_CHECK(t.movedWithin(0x1000, 0x100u, 1500));
    KT_CHECK(!t.movedWithin(0x1000, 0xFFFFFE00u, 1500));  // before the note: negative age
    t.note(0x1000, 0x50u);                                // renew across the wrap
    KT_CHECK(t.movedWithin(0x1000, 0x50u + 1500u, 1500));
    KT_CHECK(!t.movedWithin(0x1000, 0x50u + 1501u, 1500));
}

static void testRenewAndMany() {
    MoveTable t;
    t.note(0x1000, 100);
    t.note(0x2000, 200);
    t.note(0x1000, 900);                                  // renew, not a second entry
    int used = 0;
    for (int i = 0; i < MoveTable::kCap; ++i) used += t.e[i].used ? 1 : 0;
    KT_CHECK(used == 2);
    KT_CHECK(t.movedWithin(0x1000, 2400, 1500));
    KT_CHECK(!t.movedWithin(0x2000, 2400, 1500));         // 0x2000 last seen at 200
    KT_CHECK(t.movedWithin(0x2000, 1700, 1500));
}

static void testOwnerZeroIgnored() {
    MoveTable t;
    t.note(0, 100);
    for (int i = 0; i < MoveTable::kCap; ++i) KT_CHECK(!t.e[i].used);
    KT_CHECK(!t.movedWithin(0, 100, 1500));
}

static void testOverwriteOldest() {
    MoveTable t;
    // Fill 16 owners with ticks 1000 + i*10 (owner 0x100 + i is the oldest at i == 0).
    for (int i = 0; i < MoveTable::kCap; ++i) t.note(0x100 + i, 1000 + i * 10);
    for (int i = 0; i < MoveTable::kCap; ++i) KT_CHECK(t.movedWithin(0x100 + i, 1200, 1500));
    t.note(0x999, 1300);                                  // table full: evicts the oldest (0x100)
    KT_CHECK(t.movedWithin(0x999, 1300, 10));
    KT_CHECK(!t.movedWithin(0x100, 1300, 5000));
    for (int i = 1; i < MoveTable::kCap; ++i) KT_CHECK(t.movedWithin(0x100 + i, 1300, 5000));
    t.note(0x998, 1310);                                  // next oldest (0x101) goes
    KT_CHECK(!t.movedWithin(0x101, 1310, 5000));
    KT_CHECK(t.movedWithin(0x102, 1310, 5000));
    KT_CHECK(t.movedWithin(0x999, 1310, 5000));
}

static void testOverwriteOldestOrderIndependent() {
    // The oldest entry sits in the middle and at the end of the table, not only at index 0.
    MoveTable t;
    for (int i = 0; i < MoveTable::kCap; ++i) t.note(0x200 + i, 5000 + (i == 7 ? 0 : 100 + i));
    t.note(0x777, 6000);
    KT_CHECK(!t.movedWithin(0x207, 6000, 5000));          // index 7 was oldest
    KT_CHECK(t.movedWithin(0x200, 6000, 5000));
    MoveTable u;
    for (int i = 0; i < MoveTable::kCap; ++i) u.note(0x300 + i, 5000 + (i == MoveTable::kCap - 1 ? 0 : 100 + i));
    u.note(0x778, 6000);
    KT_CHECK(!u.movedWithin(0x300 + MoveTable::kCap - 1, 6000, 5000));   // last index was oldest
    KT_CHECK(u.movedWithin(0x300, 6000, 5000));
}

static void testOverwriteOldestWrap() {
    // Ticks straddle the wrap: the entry noted at 0xFFFFFF00 is older than the one at 0x10.
    MoveTable t;
    for (int i = 0; i < MoveTable::kCap; ++i) t.note(0x400 + i, 0x10u + i);   // young, after the wrap
    t.note(0x400 + 5, 0xFFFFFF00u);                       // renew entry 5 with a pre-wrap tick (it is now the oldest by wrap-safe age)
    t.note(0x888, 0x100u);
    KT_CHECK(!t.movedWithin(0x405, 0x100u, 5000));
    KT_CHECK(t.movedWithin(0x400, 0x100u, 5000));
    KT_CHECK(t.movedWithin(0x888, 0x100u, 5000));
}

static void testFreeSlotPreferred() {
    MoveTable t;
    for (int i = 0; i < MoveTable::kCap - 1; ++i) t.note(0x500 + i, 1000);
    t.note(0x600, 9000);                                  // one free slot left: nothing is evicted
    for (int i = 0; i < MoveTable::kCap - 1; ++i) KT_CHECK(t.movedWithin(0x500 + i, 1000, 0) == false && t.movedWithin(0x500 + i, 1000, 1));
    KT_CHECK(t.movedWithin(0x600, 9000, 1));
}

static void testCapacitySixteen() {
    // Literal 16: sixteen owners all fit, the 17th evicts exactly one (the oldest).
    MoveTable t;
    for (int i = 0; i < 16; ++i) t.note(0x700 + i, 1000 + i);
    for (int i = 0; i < 16; ++i) KT_CHECK(t.movedWithin(0x700 + i, 1100, 1000));
    t.note(0x7FF, 1100);
    int alive = 0;
    for (int i = 0; i < 16; ++i) alive += t.movedWithin(0x700 + i, 1100, 1000) ? 1 : 0;
    KT_CHECK(alive == 15);
    KT_CHECK(!t.movedWithin(0x700, 1100, 1000));
    KT_CHECK(t.movedWithin(0x7FF, 1100, 1000));
}

static void testFreeSlotZeroUsedFirst() {
    // Huge ticks make an unused entry look "old or young" depending on the sign; the free slots (including index 0) must be filled before any eviction.
    MoveTable t;
    const uint32_t base = 0x90000000u;
    for (int i = 0; i < 16; ++i) t.note(0x800 + i, base + i);
    for (int i = 0; i < 16; ++i) KT_CHECK(t.movedWithin(0x800 + i, base + 100, 1000));
}

static void testOldestIndexOne() {
    MoveTable t;
    for (int i = 0; i < 16; ++i) t.note(0x900 + i, 5000 + (i == 1 ? 0 : 100));
    t.note(0x9FF, 6000);
    KT_CHECK(!t.movedWithin(0x901, 6000, 5000));          // index 1 was the oldest
    KT_CHECK(t.movedWithin(0x900, 6000, 5000));
}

static void testOldestTieEvictsFirst() {
    MoveTable t;
    for (int i = 0; i < 16; ++i) t.note(0xA00 + i, 5000);   // all the same age
    t.note(0xAFF, 6000);
    KT_CHECK(!t.movedWithin(0xA00, 6000, 5000));            // a tie keeps the first (lowest index)
    for (int i = 1; i < 16; ++i) KT_CHECK(t.movedWithin(0xA00 + i, 6000, 5000));
}

static void testOwnerOne() {
    MoveTable t;
    t.note(1, 100);
    KT_CHECK(t.movedWithin(1, 100, 10));
    KT_CHECK(!t.movedWithin(2, 100, 10));
}

static void testAnyCombat() {
    const uint32_t none[3] = {0, 0, 0};
    const uint32_t first[3] = {1, 0, 0};
    const uint32_t mid[3] = {0, 7, 0};
    const uint32_t last[3] = {0, 0, 0xFFFFFFFFu};
    KT_CHECK(!anyCombat(none, 3));
    KT_CHECK(anyCombat(first, 3));
    KT_CHECK(anyCombat(mid, 3));
    KT_CHECK(anyCombat(last, 3));
    KT_CHECK(!anyCombat(last, 2));                        // only the first n count
    KT_CHECK(anyCombat(last, 3));
    KT_CHECK(!anyCombat(first, 0));
    KT_CHECK(!anyCombat(first, -1));
    KT_CHECK(!anyCombat(nullptr, 3));
    KT_CHECK(anyCombat(first, 1));
    KT_CHECK(!anyCombat(mid, 1));
    const uint32_t eight[8] = {0, 0, 0, 0, 0, 0, 0, 2};
    KT_CHECK(anyCombat(eight, 8));
    KT_CHECK(!anyCombat(eight, 7));
    const uint32_t nine[9] = {0, 0, 0, 0, 0, 0, 0, 0, 3};
    KT_CHECK(!anyCombat(nine, 9));                        // clamped to 8: the ninth is never read
    const uint32_t nineB[9] = {0, 0, 0, 0, 0, 0, 0, 4, 0};
    KT_CHECK(anyCombat(nineB, 9));                        // the eighth (index 7) is still read after the clamp
}

static void testBlocksCut() {
    KT_CHECK(moveHoldBlocksCut(true, true));
    KT_CHECK(!moveHoldBlocksCut(true, false));
    KT_CHECK(!moveHoldBlocksCut(false, true));
    KT_CHECK(!moveHoldBlocksCut(false, false));
}

int main() {
    testLocoIds();
    testHoldBoundary();
    testNegativeAge();
    testWrap();
    testRenewAndMany();
    testOwnerZeroIgnored();
    testOverwriteOldest();
    testOverwriteOldestOrderIndependent();
    testOverwriteOldestWrap();
    testFreeSlotPreferred();
    testCapacitySixteen();
    testFreeSlotZeroUsedFirst();
    testOldestIndexOne();
    testOldestTieEvictsFirst();
    testOwnerOne();
    testAnyCombat();
    testBlocksCut();
    return kt::Report("holster_move");
}
