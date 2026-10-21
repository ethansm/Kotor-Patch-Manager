// Host tests for HolsterAnimStyle.h (unarmed-style override decision + holster set of the holstered-weapons mod; doc 42).
// Build/run: python3 scripts/run_tests.py holster_animstyle
#include "HolsterAnimStyle.h"
#include "check.h"
#include <stdint.h>

using namespace hs;

static void testIdFilter() {
    KT_CHECK(idInFilter(0x2711));
    KT_CHECK(idInFilter(0x27A4));
    KT_CHECK(idInFilter(0x2712));
    KT_CHECK(idInFilter(0x2713));
    KT_CHECK(idInFilter(0x2714));
    const uint32_t out[] = {0, 1, 0x2710, 0x2715, 0x27A3, 0x27A5, 0x2701, 0x2811, 0x12711, 0x127A4, 0xFFFFFFFFu, 0x27A0, 0x27A2};
    for (size_t i = 0; i < sizeof(out) / sizeof(out[0]); ++i) KT_CHECK(!idInFilter(out[i]));
    // Exact count of accepted ids over a window.
    int n = 0;
    for (uint32_t id = 0; id < 0x3000; ++id) n += idInFilter(id) ? 1 : 0;
    KT_CHECK(n == 5);
}

static void testStyles() {
    for (uint32_t s = 0; s < 16; ++s) {
        const bool want = s == 2 || s == 3 || s == 4 || s == 7 || s == 9;
        KT_CHECK(styleHasOwnLocoRow(s) == want);
    }
    // Style 8 is unarmed itself: no own row.
    KT_CHECK(!styleHasOwnLocoRow(8));
    // Upper garbage bytes are masked off.
    KT_CHECK(styleHasOwnLocoRow(0x102));
    KT_CHECK(styleHasOwnLocoRow(0xFFFFFF03u));
    KT_CHECK(styleHasOwnLocoRow(0x80000004u));
    KT_CHECK(styleHasOwnLocoRow(0x12345607u));
    KT_CHECK(styleHasOwnLocoRow(0x00000109u));
    KT_CHECK(!styleHasOwnLocoRow(0x108));
    KT_CHECK(!styleHasOwnLocoRow(0x100));
    KT_CHECK(!styleHasOwnLocoRow(0xFFFFFF00u));
    KT_CHECK(!styleHasOwnLocoRow(0x101));
    // Bytes above 15 are not accepted either.
    for (uint32_t s = 16; s < 256; ++s) KT_CHECK(!styleHasOwnLocoRow(s));
}

static void testDecide() {
    KT_CHECK(decide(0x2711, 1, 2, true, true) == V_UNARMED);
    // Each condition flipped alone.
    KT_CHECK(decide(0x2711, 0, 2, true, true) == V_KEEP);
    KT_CHECK(decide(0x2711, 2, 2, true, true) == V_KEEP);       // gate must be exactly 1
    KT_CHECK(decide(0x2711, 0xFFFFFFFFu, 2, true, true) == V_KEEP);
    KT_CHECK(decide(0x2715, 1, 2, true, true) == V_KEEP);
    KT_CHECK(decide(0x2711, 1, 8, true, true) == V_KEEP);
    KT_CHECK(decide(0x2711, 1, 2, false, true) == V_KEEP);
    KT_CHECK(decide(0x2711, 1, 2, true, false) == V_KEEP);
    // Full truth table over the 5 conditions.
    const uint32_t ids[2] = {0x2711, 0x2715};
    const uint32_t gates[2] = {1, 0};
    const uint32_t styles[2] = {3, 8};
    for (int a = 0; a < 2; ++a)
        for (int b = 0; b < 2; ++b)
            for (int c = 0; c < 2; ++c)
                for (int d = 0; d < 2; ++d)
                    for (int e = 0; e < 2; ++e) {
                        const bool want = a == 0 && b == 0 && c == 0 && d == 0 && e == 0;
                        const Verdict v = decide(ids[a], gates[b], styles[c], d == 0, e == 0);
                        KT_CHECK(v == (want ? V_UNARMED : V_KEEP));
                    }
    // Every filtered id and every own-row style fires; garbage style bits do not matter.
    const uint32_t fid[5] = {0x2711, 0x27A4, 0x2712, 0x2713, 0x2714};
    const uint32_t sty[5] = {2, 3, 4, 7, 9};
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 5; ++j) {
            KT_CHECK(decide(fid[i], 1, sty[j], true, true) == V_UNARMED);
            KT_CHECK(decide(fid[i], 1, sty[j] | 0x100u, true, true) == V_UNARMED);
        }
    KT_CHECK(V_KEEP == 0 && V_UNARMED == 1);
}

static void testSetBasics() {
    HolsterSet s;
    KT_CHECK(s.count(1000) == 0);
    KT_CHECK(!s.contains(0x10, 0x20, 1000));
    KT_CHECK(s.touch(0x10, 0x20, 1000));
    KT_CHECK(s.contains(0x10, 0x20, 1000));
    KT_CHECK(s.count(1000) == 1);
    KT_CHECK(!s.contains(0x11, 0x20, 1000));            // other owner
    KT_CHECK(!s.contains(0x10, 0x21, 1000));            // base mismatch
    KT_CHECK(!s.contains(0x10, 0, 1000));
    // Update: new base replaces, same slot (count stays 1).
    KT_CHECK(s.touch(0x10, 0x30, 1200));
    KT_CHECK(s.count(1200) == 1);
    KT_CHECK(!s.contains(0x10, 0x20, 1200));
    KT_CHECK(s.contains(0x10, 0x30, 1200));
    // Update refreshes the stamp: age counted from 1200, not 1000.
    KT_CHECK(s.contains(0x10, 0x30, 1700));
    KT_CHECK(!s.contains(0x10, 0x30, 1701));
    // Second owner.
    KT_CHECK(s.touch(0x11, 0x40, 1300));
    KT_CHECK(s.count(1300) == 2);
    KT_CHECK(s.contains(0x11, 0x40, 1300));
    KT_CHECK(s.contains(0x10, 0x30, 1300));
    // Remove.
    s.remove(0x10);
    KT_CHECK(!s.contains(0x10, 0x30, 1300));
    KT_CHECK(s.contains(0x11, 0x40, 1300));
    KT_CHECK(s.count(1300) == 1);
    s.remove(0x10);                                     // double remove is harmless
    s.remove(0x999);                                    // unknown owner is harmless
    s.remove(0);
    KT_CHECK(s.count(1300) == 1);
    // A removed owner can be re-added.
    KT_CHECK(s.touch(0x10, 0x50, 1300));
    KT_CHECK(s.contains(0x10, 0x50, 1300));
    KT_CHECK(s.count(1300) == 2);
}

static void testSetExpiry() {
    HolsterSet s;
    KT_CHECK(kHolsterSetTtlMs == 500);
    KT_CHECK(kHolsterSetCap == 64);
    KT_CHECK(s.touch(0xA, 0xB, 1000));
    KT_CHECK(s.contains(0xA, 0xB, 1000));               // age 0
    KT_CHECK(s.contains(0xA, 0xB, 1499));
    KT_CHECK(s.contains(0xA, 0xB, 1500));               // age 500 inclusive
    KT_CHECK(!s.contains(0xA, 0xB, 1501));              // age 501
    KT_CHECK(s.count(1500) == 1);
    KT_CHECK(s.count(1501) == 0);
    KT_CHECK(!s.contains(0xA, 0xB, 100000));
    // Negative age (now before stamp) is stale/garbage.
    KT_CHECK(!s.contains(0xA, 0xB, 999));
    KT_CHECK(!s.contains(0xA, 0xB, 0));
    KT_CHECK(!s.contains(0xA, 0xB, 1000u - 0x80000000u + 1));
    KT_CHECK(s.count(999) == 0);
    // Huge forward age (just past int32 range) reads as negative -> false.
    KT_CHECK(!s.contains(0xA, 0xB, 1000u + 0x80000000u));
    KT_CHECK(!s.contains(0xA, 0xB, 1000u + 0x7FFFFFFFu));
    // An expired entry of the same owner is revived by touch.
    KT_CHECK(s.touch(0xA, 0xC, 5000));
    KT_CHECK(s.contains(0xA, 0xC, 5000));
    KT_CHECK(s.count(5000) == 1);
}

static void testSetWrap() {
    HolsterSet s;
    KT_CHECK(s.touch(0x1, 0x2, 0xFFFFFF00u));
    KT_CHECK(s.contains(0x1, 0x2, 0xFFFFFF00u));
    KT_CHECK(s.contains(0x1, 0x2, 0xFFFFFFFFu));
    KT_CHECK(s.contains(0x1, 0x2, 0x0u));               // wrapped, age 256
    // 0xFFFFFF00 + 500 = 0x000000F4 (wrapped): boundary.
    KT_CHECK(s.contains(0x1, 0x2, 0xF4u));
    KT_CHECK(!s.contains(0x1, 0x2, 0xF5u));
    KT_CHECK(!s.contains(0x1, 0x2, 0x100u));
    KT_CHECK(s.count(0xF4u) == 1);
    KT_CHECK(s.count(0x100u) == 0);
    // Stamp taken after the wrap, queried before it: negative.
    KT_CHECK(s.touch(0x3, 0x4, 0x10u));
    KT_CHECK(!s.contains(0x3, 0x4, 0xFFFFFF00u));
}

static void testSetCapacity() {
    HolsterSet s;
    for (int i = 0; i < kHolsterSetCap; ++i) KT_CHECK(s.touch((uintptr_t)(0x1000 + i), (uintptr_t)(0x2000 + i), 1000));
    KT_CHECK(s.count(1000) == 64);
    // 65th refused, set unchanged.
    KT_CHECK(!s.touch(0x9999, 0x8888, 1100));
    KT_CHECK(!s.contains(0x9999, 0x8888, 1100));
    KT_CHECK(s.count(1100) == 64);
    for (int i = 0; i < kHolsterSetCap; ++i) KT_CHECK(s.contains((uintptr_t)(0x1000 + i), (uintptr_t)(0x2000 + i), 1100));
    // Updating an existing owner while full still works.
    KT_CHECK(s.touch(0x1000, 0x7000, 1100));
    KT_CHECK(s.contains(0x1000, 0x7000, 1100));
    KT_CHECK(s.touch((uintptr_t)(0x1000 + 63), 0x7001, 1100));
    KT_CHECK(s.contains((uintptr_t)(0x1000 + 63), 0x7001, 1100));
    KT_CHECK(s.count(1100) == 64);
    // Still refused at the age-500 boundary (everything live).
    KT_CHECK(!s.touch(0x9999, 0x8888, 1500));
    // Age 501 for the entries stamped 1000 -> reclaimed, entries refreshed at 1100 stay.
    KT_CHECK(s.touch(0x9999, 0x8888, 1501));
    KT_CHECK(s.contains(0x9999, 0x8888, 1501));
    KT_CHECK(s.contains(0x1000, 0x7000, 1501));          // refreshed at 1100, age 401
    KT_CHECK(s.contains((uintptr_t)(0x1000 + 63), 0x7001, 1501));
    // Only one slot was reclaimed per touch: 63 live remain from the original plus the new one; the evicted one is an expired one.
    KT_CHECK(s.count(1501) == 3);                        // 0x1000, 0x103F, 0x9999 live; rest are age 501 (expired)
    KT_CHECK(s.touch(0x9998, 0x8887, 1501));
    KT_CHECK(s.count(1501) == 4);
    // Live entries are never overwritten: fill with live entries, then fail.
    HolsterSet t;
    for (int i = 0; i < kHolsterSetCap; ++i) KT_CHECK(t.touch((uintptr_t)(i + 1), (uintptr_t)(i + 100), 50 + (uint32_t)i));
    KT_CHECK(!t.touch(0x5000, 0x5001, 200));
    KT_CHECK(t.count(200) == 64);
    // Stamps are 50..113; the oldest is 50; at now=551 only that one (age 501) is expired, so exactly one admission succeeds.
    KT_CHECK(t.touch(0x5000, 0x5001, 551));
    KT_CHECK(!t.contains(1, 100, 551));                  // evicted (was the expired one)
    KT_CHECK(t.contains(2, 101, 551));
    KT_CHECK(t.contains(64, 163, 551));
    KT_CHECK(t.contains(0x5000, 0x5001, 551));
    KT_CHECK(!t.touch(0x5002, 0x5003, 551));            // no expired entry left
    KT_CHECK(t.count(551) == 64);
}

static void testSetFreeSlots() {
    HolsterSet s;
    for (int i = 0; i < kHolsterSetCap; ++i) KT_CHECK(s.touch((uintptr_t)(i + 1), 7, 1000));
    KT_CHECK(!s.touch(0x500, 8, 1000));
    s.remove(33);
    KT_CHECK(s.count(1000) == 63);
    KT_CHECK(!s.contains(33, 7, 1000));
    KT_CHECK(s.touch(0x500, 8, 1000));                   // remove freed a slot
    KT_CHECK(s.contains(0x500, 8, 1000));
    KT_CHECK(s.count(1000) == 64);
    KT_CHECK(!s.touch(0x501, 8, 1000));                  // full again
    // A free slot is preferred over reclaiming an expired one: the expired entries stay present until needed.
    HolsterSet u;
    KT_CHECK(u.touch(1, 1, 1000));
    KT_CHECK(u.touch(2, 2, 2000));
    KT_CHECK(u.count(2000) == 1);
    // Entry 1 expired but not reclaimed (free slots exist) -- contains still false by age.
    KT_CHECK(!u.contains(1, 1, 2000));
    // Re-touching the expired owner works.
    KT_CHECK(u.touch(1, 9, 2000));
    KT_CHECK(u.contains(1, 9, 2000));
    KT_CHECK(u.count(2000) == 2);
}

static void testOwnerZero() {
    HolsterSet s;
    KT_CHECK(!s.touch(0, 5, 1000));
    KT_CHECK(s.count(1000) == 0);
    KT_CHECK(!s.contains(0, 5, 1000));
    KT_CHECK(!s.contains(0, 0, 1000));
    // Refused owner 0 does not consume a slot, even repeatedly.
    for (int i = 0; i < 100; ++i) KT_CHECK(!s.touch(0, 5, 1000));
    for (int i = 0; i < kHolsterSetCap; ++i) KT_CHECK(s.touch((uintptr_t)(i + 1), 1, 1000));
    KT_CHECK(s.count(1000) == 64);
    // Base 0 is a legal value for a real owner.
    HolsterSet t;
    KT_CHECK(t.touch(5, 0, 1000));
    KT_CHECK(t.contains(5, 0, 1000));
    KT_CHECK(!t.contains(5, 1, 1000));
}

static void testFrameOk() {
    KT_CHECK(!frameOk(0));
    KT_CHECK(!frameOk(4));
    KT_CHECK(!frameOk(0xFFFC));          // below 64 KiB
    KT_CHECK(frameOk(0x10000));          // lowest accepted
    KT_CHECK(!frameOk(0x10001));
    KT_CHECK(!frameOk(0x10002));
    KT_CHECK(!frameOk(0x10003));
    KT_CHECK(frameOk(0x10004));
    KT_CHECK(frameOk(0x0019FF50));
    KT_CHECK(!frameOk(0x0019FF52));
    KT_CHECK(frameOk(0xFFFFFFFCu));
    KT_CHECK(!frameOk(0xFFFFFFFFu));
}

static void testAnimId16() {
    KT_CHECK(animId16(0x2714) == 0x2714u);
    KT_CHECK(animId16(0xABCD2714u) == 0x2714u);   // stack garbage above the u16 argument
    KT_CHECK(animId16(0x00012714u) == 0x2714u);
    KT_CHECK(animId16(0xFFFFFFFFu) == 0xFFFFu);
    KT_CHECK(animId16(0) == 0u);
}

static void testOverrideStyle() {
    uint8_t out = 0x55;
    KT_CHECK(overrideStyle(0x2714, 1, 4, true, true, &out));
    KT_CHECK(out == kStyleUnarmed && out == 8);
    // Upper garbage on the id is ignored (masked), upper garbage on the style byte too.
    out = 0x55;
    KT_CHECK(overrideStyle(0xBEEF2712u, 1, 0x107, true, true, &out));
    KT_CHECK(out == 8);
    // Every refusal leaves *out untouched.
    const uint8_t sentinel = 0x77;
    out = sentinel;
    KT_CHECK(!overrideStyle(0x2714, 0, 4, true, true, &out) && out == sentinel);       // gate
    KT_CHECK(!overrideStyle(0x2714, 1, 8, true, true, &out) && out == sentinel);       // already unarmed
    KT_CHECK(!overrideStyle(0x2714, 1, 1, true, true, &out) && out == sentinel);       // single-weapon style has no own row
    KT_CHECK(!overrideStyle(0x2715, 1, 4, true, true, &out) && out == sentinel);       // id outside the filter
    KT_CHECK(!overrideStyle(0x2714, 1, 4, false, true, &out) && out == sentinel);      // not in the holster set
    KT_CHECK(!overrideStyle(0x2714, 1, 4, true, false, &out) && out == sentinel);      // combat flag set
    KT_CHECK(!overrideStyle(0x2714, 1, 4, true, true, 0));                             // null out pointer
    // The id filter looks at the low 16 bits only (a high-bit id 0x12714 is the same id as 0x2714 to the engine's u16 compare).
    out = sentinel;
    KT_CHECK(overrideStyle(0x00012714u, 1, 9, true, true, &out) && out == 8);
    // Sweep: the override fires exactly where decide() says V_UNARMED.
    for (uint32_t id = 0x2700; id < 0x27C0; ++id)
        for (uint32_t st = 0; st < 12; ++st) {
            out = sentinel;
            const bool fired = overrideStyle(id, 1, st, true, true, &out);
            KT_CHECK(fired == (decide(id, 1, st, true, true) == V_UNARMED));
            KT_CHECK(fired ? out == 8 : out == sentinel);
        }
}

// 0.16.0-dev: the resolver hook hands over the BODY object; ownerOfBase maps it back to the client creature that touched the set.
static void testOwnerOfBase() {
    HolsterSet s;
    KT_CHECK(s.ownerOfBase(0x20, 1000) == 0);            // empty set
    KT_CHECK(s.touch(0x10, 0x20, 1000));
    KT_CHECK(s.ownerOfBase(0x20, 1000) == 0x10);         // hit: body -> creature
    KT_CHECK(s.ownerOfBase(0x21, 1000) == 0);            // miss
    KT_CHECK(s.ownerOfBase(0x10, 1000) == 0);            // the creature pointer itself is not a base
    KT_CHECK(s.contains(0x10, 0x20, 1000));              // contains / touch / remove unchanged
    // TTL: live through age 500 inclusive, gone at 501.
    KT_CHECK(s.ownerOfBase(0x20, 1500) == 0x10);
    KT_CHECK(s.ownerOfBase(0x20, 1501) == 0);
    KT_CHECK(s.ownerOfBase(0x20, 999) == 0);             // stamp in the future
    KT_CHECK(s.touch(0x10, 0x20, 1600));                 // refresh revives it
    KT_CHECK(s.ownerOfBase(0x20, 1600) == 0x10);
    // Base 0 never matches, even for a real owner stored with base 0.
    HolsterSet z;
    KT_CHECK(z.touch(5, 0, 1000));
    KT_CHECK(z.ownerOfBase(0, 1000) == 0);
    // Two owners, two bodies.
    HolsterSet t;
    KT_CHECK(t.touch(0x1, 0xA, 1000));
    KT_CHECK(t.touch(0x2, 0xB, 1000));
    KT_CHECK(t.ownerOfBase(0xA, 1000) == 0x1);
    KT_CHECK(t.ownerOfBase(0xB, 1000) == 0x2);
    t.remove(0x1);
    KT_CHECK(t.ownerOfBase(0xA, 1000) == 0);
    KT_CHECK(t.ownerOfBase(0xB, 1000) == 0x2);
    // An expired owner does not shadow a live one with the same base.
    HolsterSet u;
    KT_CHECK(u.touch(0x1, 0xC, 1000));
    KT_CHECK(u.touch(0x2, 0xC, 2000));
    KT_CHECK(u.ownerOfBase(0xC, 2000) == 0x2);
    // Body changed on a re-touch: the old base no longer maps.
    KT_CHECK(u.touch(0x2, 0xD, 2100));
    KT_CHECK(u.ownerOfBase(0xC, 2100) == 0);
    KT_CHECK(u.ownerOfBase(0xD, 2100) == 0x2);
    // Wrap-safe ticks: stamped just before the 32-bit wrap.
    HolsterSet w;
    KT_CHECK(w.touch(0x7, 0x70, 0xFFFFFF00u));
    KT_CHECK(w.ownerOfBase(0x70, 0xFFFFFFFFu) == 0x7);
    KT_CHECK(w.ownerOfBase(0x70, 0x0u) == 0x7);          // wrapped, age 256
    KT_CHECK(w.ownerOfBase(0x70, 0xF4u) == 0x7);         // age 500 inclusive
    KT_CHECK(w.ownerOfBase(0x70, 0xF5u) == 0);           // age 501
    KT_CHECK(w.ownerOfBase(0x70, 0xFFFFFE00u) == 0);     // stamp in the future
    // Agrees with contains for every stored pair.
    HolsterSet f;
    for (int i = 0; i < kHolsterSetCap; ++i) KT_CHECK(f.touch((uintptr_t)(0x100 + i), (uintptr_t)(0x900 + i), 1000));
    for (int i = 0; i < kHolsterSetCap; ++i) {
        KT_CHECK(f.ownerOfBase((uintptr_t)(0x900 + i), 1200) == (uintptr_t)(0x100 + i));
        KT_CHECK(f.contains((uintptr_t)(0x100 + i), (uintptr_t)(0x900 + i), 1200));
    }
}

int main() {
    testIdFilter();
    testStyles();
    testDecide();
    testSetBasics();
    testSetExpiry();
    testSetWrap();
    testSetCapacity();
    testSetFreeSlots();
    testOwnerZero();
    testOwnerOfBase();
    testFrameOk();
    testAnimId16();
    testOverrideStyle();
    return kt::Report("holster_animstyle");
}
