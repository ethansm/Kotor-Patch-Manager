// Host tests for HolsterLogRate.h (quiet-mode log rate limits of the holstered-weapons mod).
// Build/run: python3 scripts/run_tests.py holster_lograte
#include "HolsterLogRate.h"
#include "check.h"
#include <stdint.h>
#include <math.h>

using namespace hs;

static void testKeyBumpAndWant() {
    KeyCounts<2> t;
    KT_CHECK(keyBump(t, 0) == 0);
    KT_CHECK(keyBump(t, 0x10) == 1);
    KT_CHECK(keyBump(t, 0x10) == 2);
    KT_CHECK(keyBump(t, 0x20) == 1);
    KT_CHECK(keyBump(t, 0x30) == 1);   // full: the oldest slot (0x10) is evicted, the new key starts at 1
    KT_CHECK(keyBump(t, 0x20) == 2);
    KT_CHECK(keyBump(t, 0x30) == 2);
    KT_CHECK(keyBump(t, 0x10) == 1);   // evicted key restarts (evicts 0x20, the next oldest)
    KT_CHECK(creatureKey(0) != 0 && creatureKey(0x40) == 0x40);
    KT_CHECK(!keyedWant(0, 3, 10));
    KT_CHECK(keyedWant(1, 3, 10) && keyedWant(3, 3, 10));
    KT_CHECK(!keyedWant(4, 3, 10) && !keyedWant(9, 3, 10));
    KT_CHECK(keyedWant(10, 3, 10) && keyedWant(20, 3, 10));
    KT_CHECK(!keyedWant(11, 3, 0) && !keyedWant(10, 3, 0));
}

static void testCombVolume() {
    KeyCounts<256> t;
    int logged = 0;
    for (int i = 0; i < 100; ++i) logged += keyedWant(keyBump(t, 0x1000), 3, 10) ? 1 : 0;
    KT_CHECK(logged == 3 + 10);   // hits 1..3, then 10,20,...,100
}

static void testKeyRingNeverDark() {
    KeyCounts<256> t;
    for (int i = 0; i < 1000; ++i) KT_CHECK(keyBump(t, 0x1000u + static_cast<uintptr_t>(i)) == 1);   // creature 1000 still gets its first lines
    KT_CHECK(keyBump(t, creatureKey(0)) == 1 && keyBump(t, creatureKey(0)) == 2);   // null creature is counted
}

static void testCombLogWanted() {
    KT_CHECK(combLogWanted(true, 0, false) && combLogWanted(true, 500, false));   // edges are never rate-limited
    int logged = 0;
    for (uint32_t c = 1; c <= 100; ++c) logged += combLogWanted(false, c, false) ? 1 : 0;
    KT_CHECK(logged == 3 + 10);   // edge=0: first 3, then every 10th
    KT_CHECK(!combLogWanted(false, 4, false) && combLogWanted(false, 10, false));
    KT_CHECK(combLogWanted(false, 4, true) && combLogWanted(false, 0, true));   // verbose logs everything
}

static void testFnvAndMath() {
    KT_CHECK(fnv1a("a") != fnv1a("b"));
    KT_CHECK(fnv1a("abc") == fnv1a("abc"));
    KT_CHECK(fnv1a(nullptr, 5u) == 5u);
    const float id[4] = {1, 0, 0, 0}, neg[4] = {-1, 0, 0, 0}, r90[4] = {0.70710678f, 0.70710678f, 0, 0};
    KT_CHECK(quatAngleDeg(id, id) < 0.01f);
    KT_CHECK(quatAngleDeg(id, neg) < 0.01f);   // q and -q are one rotation
    KT_CHECK(fabsf(quatAngleDeg(id, r90) - 90.f) < 0.1f);
    const float a[3] = {0, 0, 0}, b[3] = {3, 4, 0};
    KT_CHECK(fabsf(vecDist(a, b) - 5.f) < 1e-5f);
}

static void testPlaceSampling() {
    PlaceTable t;
    int logged = 0;
    for (int i = 0; i < 1000; ++i) logged += placeWant(t, 0xA000, 7u, 0.25f, 10.f) ? 1 : 0;
    KT_CHECK(logged == 3 + 5);   // first 3, then 200,400,600,800,1000
}

static void testPlaceChange() {
    PlaceTable t;
    for (int i = 0; i < 10; ++i) (void)placeWant(t, 0xA000, 7u, 0.25f, 10.f);
    KT_CHECK(!placeWant(t, 0xA000, 7u, 0.251f, 10.2f));   // inside tolerance
    KT_CHECK(placeWant(t, 0xA000, 8u, 0.25f, 10.f));      // signature changed
    KT_CHECK(!placeWant(t, 0xA000, 8u, 0.25f, 10.f));
    KT_CHECK(placeWant(t, 0xA000, 8u, 0.30f, 10.f));      // displacement moved
    KT_CHECK(!placeWant(t, 0xA000, 8u, 0.30f, 10.f));
    KT_CHECK(placeWant(t, 0xA000, 8u, 0.30f, 12.f));      // rotation moved by 2 degrees
    KT_CHECK(placeWant(t, 0xB000, 7u, 0.25f, 10.f));      // another gob starts at count 1
}

static void testPlaceChangeCap() {
    PlaceTable t;
    for (int i = 0; i < 5; ++i) (void)placeWant(t, 0xA000, 7u, 0.f, 0.f);
    int logged = 0;
    for (int i = 0; i < 100; ++i) logged += placeWant(t, 0xA000, 7u, 1.f + 0.1f * static_cast<float>(i), 0.f) ? 1 : 0;
    KT_CHECK(logged == static_cast<int>(PlaceTable::kMaxChange));   // every call moves the displacement, but change lines stop at the cap
}

static void testPlaceOverflow() {
    PlaceTable t;
    for (int i = 0; i < PlaceTable::kCap; ++i) KT_CHECK(placeWant(t, 0x1000u + static_cast<uintptr_t>(i), 1u, 0.f, 0.f));
    int logged = 0;
    for (int i = 0; i < 1000; ++i) logged += placeWant(t, 0xFFFF00u, 1u, 0.f, 0.f) ? 1 : 0;
    KT_CHECK(logged == 3 + 5);   // a new gob past the table evicts the oldest and gets the full sampling (first 3, every 200th)
    for (int i = 0; i < 500; ++i) KT_CHECK(placeWant(t, 0x900000u + static_cast<uintptr_t>(i), 1u, 0.f, 0.f));   // every new gob logs its first line
}

static void testRefuse() {
    RefuseTable t;
    uint32_t c = 0;
    KT_CHECK(refuseWant(t, 2, 0x100, &c) && c == 1);        // first hit of the reason
    KT_CHECK(!refuseWant(t, 2, 0x100, &c) && c == 2);       // same pair
    KT_CHECK(refuseWant(t, 2, 0x200, &c) && c == 3);        // new gob
    KT_CHECK(refuseWant(t, 3, 0x100, &c) && c == 1);        // other reason
    int logged = 0;
    for (int i = 0; i < 20000; ++i) logged += refuseWant(t, 2, 0x100, &c) ? 1 : 0;
    KT_CHECK(logged == 2);   // c = 10000 and 20000 summaries (total reached 20003)
    KT_CHECK(c == 20003);
}

static void testRefuseCaps() {
    RefuseTable t;
    int logged = 0;
    uint32_t c = 0;
    for (int i = 0; i < 500; ++i) logged += refuseWant(t, 2, 0x1000u + static_cast<uintptr_t>(i), &c) ? 1 : 0;
    KT_CHECK(logged == 500);                 // every new (reason, gob) pair logs; the oldest pair is evicted past 64
    KT_CHECK(refuseWant(t, 2, 0x1000u, &c)); // evicted pair is new again
    KT_CHECK(!refuseWant(t, 2, 0x1000u + 499u, &c));   // recent pair still remembered
    KT_CHECK(refuseWant(t, -1, 1, &c));      // out-of-range reason folds to 0, still counted
}

int main() {
    testKeyBumpAndWant();
    testCombVolume();
    testKeyRingNeverDark();
    testCombLogWanted();
    testFnvAndMath();
    testPlaceSampling();
    testPlaceChange();
    testPlaceChangeCap();
    testPlaceOverflow();
    testRefuse();
    testRefuseCaps();
    return kt::Report("holster_lograte");
}
