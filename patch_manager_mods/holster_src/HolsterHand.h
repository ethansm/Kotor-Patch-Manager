// HolsterHand.h -- hand record of the holstered-weapons mod (doc 42, review F1 / r6 recon): which hand a weapon Gob was attached to.
// Pure logic, header-only, no engine calls, no STL. Host-tested by holster_hand_test.cpp.
//   HandRing    16 entries, gob -> hand, the oldest entry is overwritten when full; re-recording a known gob updates it in place
//   SeenSet     up to 64 gobs already reported (HANDMAP dedupe); a full set reports false for new gobs (the log cap)
// Not locked: both users run on the engine's main thread (attach site and the per-frame attach refresh).
#pragma once
#include <stdint.h>

namespace hs {

enum Hand : int { HandNone = 0, HandR = 1, HandL = 2 };

inline char handChar(int h) { return h == HandR ? 'R' : h == HandL ? 'L' : '?'; }

struct HandRing {
    static const int kCap = 16;
    uintptr_t gob[kCap];
    int hand[kCap];
    int next;   // slot the next new gob goes to
    int n;      // filled slots
    HandRing() : next(0), n(0) {
        for (int i = 0; i < kCap; ++i) { gob[i] = 0; hand[i] = HandNone; }
    }
};

inline int handFind(const HandRing& r, uintptr_t g) {
    if (!g) return -1;
    for (int i = 0; i < r.n; ++i)
        if (r.gob[i] == g) return i;
    return -1;
}

// Records g -> hand. A null gob or a hand other than R/L is ignored (false).
inline bool handRecord(HandRing& r, uintptr_t g, int hand) {
    if (!g || (hand != HandR && hand != HandL)) return false;
    const int i = handFind(r, g);
    if (i >= 0) { r.hand[i] = hand; return true; }
    r.gob[r.next] = g;
    r.hand[r.next] = hand;
    r.next = (r.next + 1) % HandRing::kCap;
    if (r.n < HandRing::kCap) ++r.n;
    return true;
}

// Hand of g, HandNone when unknown.
inline int handLookup(const HandRing& r, uintptr_t g) {
    const int i = handFind(r, g);
    return i < 0 ? static_cast<int>(HandNone) : r.hand[i];
}

struct SeenSet {
    static const int kCap = 64;
    uintptr_t gob[kCap];
    int n;
    SeenSet() : n(0) {
        for (int i = 0; i < kCap; ++i) gob[i] = 0;
    }
};

// True exactly once per gob (the caller logs then); false for a null gob, a repeat, or when the set is full.
inline bool seenFirst(SeenSet& s, uintptr_t g) {
    if (!g) return false;
    for (int i = 0; i < s.n; ++i)
        if (s.gob[i] == g) return false;
    if (s.n >= SeenSet::kCap) return false;
    s.gob[s.n++] = g;
    return true;
}

}  // namespace hs
