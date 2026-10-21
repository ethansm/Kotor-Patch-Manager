// Host tests for HolsterShape.h (holstered-weapons shape table lookup, HW30 / C2b; doc 42).
// Build/run: python3 scripts/run_tests.py holster_shape
#include "HolsterShape.h"
#include "check.h"
#include <math.h>

using namespace hs;

static void testExactRows() {
    const ShapeEntry* s = shapeOf("w_lghtsbr_001");
    KT_CHECK(s != nullptr && s->axis == 2 && fabsf(s->len - 1.320f) < 0.01f);
    s = shapeOf("w_blstrpstl_001");
    KT_CHECK(s != nullptr && s->axis == 1 && fabsf(s->len - 0.405f) < 0.01f);
    s = shapeOf("w_dblsbr_001");
    KT_CHECK(s != nullptr && s->axis == 2 && fabsf(s->len - 2.464f) < 0.01f);
}

static void testCaseAndMisses() {
    const ShapeEntry* a = shapeOf("W_LghtSbr_001");
    KT_CHECK(a != nullptr && a == shapeOf("w_lghtsbr_001"));
    KT_CHECK(shapeOf("w_nothing_001") == nullptr);
    KT_CHECK(shapeOf(nullptr) == nullptr);
    KT_CHECK(shapeOf("") == nullptr);
    KT_CHECK(shapeOf("w_lghtsbr_001_padding_padding_padding_xx") == nullptr);   // 32+ characters
    KT_CHECK(shapeOf("w_lghtsbr_") == nullptr);                                 // no _NNN suffix, no family
}

// Builds "w_lghtsbr_" + zeros + '1', exactly len characters long (len < 40).
static void buildStem(char* out, int len) {
    const char* head = "w_lghtsbr_";
    int n = 0;
    while (head[n]) { out[n] = head[n]; ++n; }
    while (n < len - 1) out[n++] = '0';
    out[n++] = '1';
    out[n] = 0;
}

static void testFamilyFallback() {
    const ShapeEntry* f = shapeOf("w_lghtsbr_099");
    KT_CHECK(f != nullptr && f == shapeOf("w_lghtsbr_001") && f->axis == 2 && fabsf(f->len - 1.320f) < 0.02f);
    KT_CHECK(shapeOf("w_blstrpstl_099") == nullptr);   // kNoFallback: blaster pistol
    KT_CHECK(shapeOf("w_qtrstaff_099") == nullptr);    // kNoFallback: quarterstaff
    KT_CHECK(shapeOf("w_ionrfl_099") == nullptr);      // kNoFallback: ion rifle, spread 12.1%
}

static void testFamilyBoundary() {
    char a[40];
    char b[40];
    buildStem(a, 31);   // 31 characters: last length the key buffer accepts
    buildStem(b, 32);   // 32 characters: rejected
    KT_CHECK(shapeOf(a) != nullptr && shapeOf(a) == shapeOf("w_lghtsbr_001"));
    KT_CHECK(shapeOf(b) == nullptr);
}

static bool tableSorted() {
    for (unsigned i = 1; i < kShapeCount; ++i)
        if (shapeCmp(kShapes[i - 1].stem, kShapes[i].stem) >= 0) return false;
    return true;
}

static void testTableSorted() {
    KT_CHECK(kShapeCount > 0);
    KT_CHECK(tableSorted());
}

// Family length of a stem: the chars through the '_' before the trailing _NNN digits (0 when there is no suffix).
static int familyLenOf(const char* stem) {
    int n = 0;
    while (stem[n]) ++n;
    int d = n;
    while (d > 0 && stem[d - 1] >= '0' && stem[d - 1] <= '9') --d;
    if (d == n || d < 2 || stem[d - 1] != '_') return 0;
    return d;
}

// True when rows a and b share a family prefix.
static bool sameFamily(unsigned a, unsigned b) {
    int la = familyLenOf(kShapes[a].stem);
    if (la == 0 || la != familyLenOf(kShapes[b].stem)) return false;
    for (int k = 0; k < la; ++k)
        if (kShapes[a].stem[k] != kShapes[b].stem[k]) return false;
    return true;
}

// True when row a's family key ("w_ionrfl_" for w_ionrfl_001) is listed in kNoFallback.
static bool inNoFallback(unsigned a) {
    int la = familyLenOf(kShapes[a].stem);
    if (la <= 0 || la >= 32) return false;
    char key[32];
    for (int k = 0; k < la; ++k) key[k] = kShapes[a].stem[k];
    key[la] = 0;
    for (int k = 0; k < kNoFallbackCount; ++k)
        if (shapeCmp(kNoFallback[k], key) == 0) return true;
    return false;
}

// Every family whose lengths spread more than 5% must be in kNoFallback. The counts must also match, so a stale
// entry (a family that no longer spreads that much) fails too.
static void testNoFallbackMatchesSpreadRule() {
    unsigned wide = 0;
    unsigned i = 0;
    while (i < kShapeCount) {
        float lo = kShapes[i].len;
        float hi = kShapes[i].len;
        unsigned j = i + 1;
        while (j < kShapeCount && sameFamily(i, j)) {
            if (kShapes[j].len < lo) lo = kShapes[j].len;
            if (kShapes[j].len > hi) hi = kShapes[j].len;
            ++j;
        }
        if (familyLenOf(kShapes[i].stem) > 0 && hi - lo > 0.05f * hi) {
            ++wide;
            KT_CHECK(inNoFallback(i));
        }
        i = j;
    }
    KT_CHECK(wide > 0);
    KT_CHECK(wide == static_cast<unsigned>(kNoFallbackCount));
}

int main() {
    testExactRows();
    testCaseAndMisses();
    testFamilyFallback();
    testFamilyBoundary();
    testTableSorted();
    testNoFallbackMatchesSpreadRule();
    return kt::Report("holster_shape");
}
