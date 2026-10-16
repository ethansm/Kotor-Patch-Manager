// Host tests for HolsterHand.h (gob -> hand ring and HANDMAP dedupe set of the holstered-weapons mod; doc 42).
// Build/run: python3 scripts/run_tests.py holster_hand
#include "HolsterHand.h"
#include "check.h"
#include <stdint.h>

using namespace hs;

static void testInsertLookup() {
    HandRing r;
    KT_CHECK(handLookup(r, 0x1000) == HandNone);
    KT_CHECK(handRecord(r, 0x1000, HandR));
    KT_CHECK(handRecord(r, 0x2000, HandL));
    KT_CHECK(handLookup(r, 0x1000) == HandR);
    KT_CHECK(handLookup(r, 0x2000) == HandL);
    KT_CHECK(handLookup(r, 0x3000) == HandNone);
    KT_CHECK(r.n == 2);
}

static void testRejectsBadInput() {
    HandRing r;
    KT_CHECK(!handRecord(r, 0, HandR));
    KT_CHECK(!handRecord(r, 0x1000, HandNone));
    KT_CHECK(!handRecord(r, 0x1000, 7));
    KT_CHECK(r.n == 0);
    KT_CHECK(handLookup(r, 0) == HandNone);
}

static void testUpdateInPlace() {
    HandRing r;
    KT_CHECK(handRecord(r, 0x1000, HandR));
    KT_CHECK(handRecord(r, 0x1000, HandL));   // same gob re-attached to the other hand
    KT_CHECK(r.n == 1);
    KT_CHECK(handLookup(r, 0x1000) == HandL);
}

static void testOverwriteOldest() {
    HandRing r;
    for (int i = 0; i < HandRing::kCap; ++i) KT_CHECK(handRecord(r, 0x1000u + 0x10u * i, (i & 1) ? HandL : HandR));
    KT_CHECK(r.n == HandRing::kCap);
    KT_CHECK(handLookup(r, 0x1000) == HandR);
    KT_CHECK(handRecord(r, 0x9000, HandL));   // 17th entry evicts the oldest (0x1000)
    KT_CHECK(r.n == HandRing::kCap);
    KT_CHECK(handLookup(r, 0x1000) == HandNone);
    KT_CHECK(handLookup(r, 0x1010) == HandL);
    KT_CHECK(handLookup(r, 0x9000) == HandL);
    KT_CHECK(handRecord(r, 0x9010, HandR));   // next eviction is the second oldest (0x1010)
    KT_CHECK(handLookup(r, 0x1010) == HandNone);
    KT_CHECK(handLookup(r, 0x1020) == HandR);
    for (int i = 0; i < 100; ++i) KT_CHECK(handRecord(r, 0x20000u + i, HandR));   // wraps many times, never overflows
    KT_CHECK(r.n == HandRing::kCap);
    KT_CHECK(handLookup(r, 0x20000u + 99) == HandR);
    KT_CHECK(handLookup(r, 0x20000u + 83) == HandNone);   // only the last 16 inserts (84..99) remain
    KT_CHECK(handLookup(r, 0x20000u + 84) == HandR);
}

static void testSeenDedupeAndCap() {
    SeenSet s;
    KT_CHECK(!seenFirst(s, 0));
    KT_CHECK(seenFirst(s, 0x1000));
    KT_CHECK(!seenFirst(s, 0x1000));
    KT_CHECK(seenFirst(s, 0x2000));
    for (int i = 2; i < SeenSet::kCap; ++i) KT_CHECK(seenFirst(s, 0x3000u + i));
    KT_CHECK(s.n == SeenSet::kCap);
    KT_CHECK(!seenFirst(s, 0xFFFF0));   // full: the cap drops new gobs
    KT_CHECK(!seenFirst(s, 0x1000));
}

static void testHandChar() {
    KT_CHECK(handChar(HandR) == 'R' && handChar(HandL) == 'L' && handChar(HandNone) == '?');
}

int main() {
    testInsertLookup();
    testRejectsBadInput();
    testUpdateInPlace();
    testOverwriteOldest();
    testSeenDedupeAndCap();
    testHandChar();
    return kt::Report("holster_hand");
}
