// diversity_keep_test.cpp -- unit tests for DiversityKeep.h (keep-default tag and position lookup).
#include "DiversityKeep.h"
#include "check.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

namespace nk = NpcDiversity;
namespace nr = NpcDiversity::roles;

namespace {

// Stored position (0.1 m units) as metres.
float Metres(int32_t v10) { return static_cast<float>(v10) * 0.1f; }

char ToUpper(char c) { return (c >= 'a' && c <= 'z') ? static_cast<char>(c - ('a' - 'A')) : c; }

// Stored position x10/y10 within tolerance of (qx, qy) on both axes.
bool NearStored(int32_t qx, int32_t qy) {
    const int32_t tol = nr::kKeepPosTol10;
    for (std::size_t i = 0; i < nr::kKeepPositionCount; ++i) {
        const int32_t dx = qx - nr::kKeepPositions[i].x10;
        const int32_t dy = qy - nr::kKeepPositions[i].y10;
        if (dx >= -tol && dx <= tol && dy >= -tol && dy <= tol) return true;
    }
    return false;
}

bool StoredTagEquals(const char* s) {
    for (std::size_t i = 0; i < nr::kKeepTagCount; ++i) {
        if (std::strcmp(s, nr::kKeepTags[i]) == 0) return true;
    }
    return false;
}

void testPinnedCounts() {
    KT_CHECK_EQ(nr::kKeepTagCount, 112);
    KT_CHECK_EQ(nr::kKeepPositionCount, 158);
    KT_CHECK_EQ(sizeof(nr::kKeepTags) / sizeof(nr::kKeepTags[0]), nr::kKeepTagCount);
    KT_CHECK_EQ(sizeof(nr::kKeepPositions) / sizeof(nr::kKeepPositions[0]), nr::kKeepPositionCount);
}

void testEveryTagMatches() {
    int lenFail = 0, exactFail = 0, upperFail = 0, mixedFail = 0;
    char up[nk::kKeepTagMaxChars + 1];
    for (std::size_t i = 0; i < nr::kKeepTagCount; ++i) {
        const char* t = nr::kKeepTags[i];
        const std::size_t n = std::strlen(t);
        if (n == 0 || n > nk::kKeepTagMaxChars - 1) ++lenFail;
        if (!nk::IsKeepDefaultTag(t)) ++exactFail;

        for (std::size_t c = 0; c <= n; ++c) up[c] = ToUpper(t[c]);
        if (!nk::IsKeepDefaultTag(up)) ++upperFail;

        // Alternate upper/lower by position: "bArTeNdEr"-style mixed case.
        for (std::size_t c = 0; c <= n; ++c) up[c] = (c % 2 == 0) ? ToUpper(t[c]) : t[c];
        if (!nk::IsKeepDefaultTag(up)) ++mixedFail;
    }
    KT_CHECK_EQ(lenFail, 0);
    KT_CHECK_EQ(exactFail, 0);
    KT_CHECK_EQ(upperFail, 0);
    KT_CHECK_EQ(mixedFail, 0);
}

void testNearMissAndWholeString() {
    int nearHits = 0, prefixMismatch = 0;
    char buf[nk::kKeepTagMaxChars + 1];
    for (std::size_t i = 0; i < nr::kKeepTagCount; ++i) {
        const char* t = nr::kKeepTags[i];
        const std::size_t n = std::strlen(t);

        // tag + "x" must not match.
        std::memcpy(buf, t, n);
        buf[n] = 'x';
        buf[n + 1] = '\0';
        if (nk::IsKeepDefaultTag(buf)) ++nearHits;

        // tag minus its last character: matches only if that shorter string is itself stored.
        std::memcpy(buf, t, n - 1);
        buf[n - 1] = '\0';
        const bool expected = StoredTagEquals(buf);
        if (nk::IsKeepDefaultTag(buf) != expected) ++prefixMismatch;
    }
    KT_CHECK_EQ(nearHits, 0);
    KT_CHECK_EQ(prefixMismatch, 0);
}

void testEmptyNullAndOverlong() {
    KT_CHECK(!nk::IsKeepDefaultTag(nullptr));
    KT_CHECK(!nk::IsKeepDefaultTag(""));
    KT_CHECK(!nk::IsKeepDefaultTag("notakeeptag"));

    // 50 characters: over the 32-character cap, must return false without overrunning.
    char big[51];
    std::memset(big, 'b', 50);
    big[50] = '\0';
    KT_CHECK(!nk::IsKeepDefaultTag(big));
}

void testEveryPositionMatchesWithinTolerance() {
    const int32_t tol = nr::kKeepPosTol10;
    int missFail = 0;
    for (std::size_t i = 0; i < nr::kKeepPositionCount; ++i) {
        const nr::KeepPos p = nr::kKeepPositions[i];
        for (int32_t dx = -tol; dx <= tol; dx += tol) {
            for (int32_t dy = -tol; dy <= tol; dy += tol) {
                if (!nk::IsKeepDefaultPos(Metres(p.x10 + dx), Metres(p.y10 + dy))) ++missFail;
            }
        }
    }
    KT_CHECK_EQ(missFail, 0);
}

void testPositionsOutsideToleranceMiss() {
    const int32_t tol = nr::kKeepPosTol10;
    const int32_t out = tol + 1;  // 4: exceeds tolerance once rounded
    int tested = 0, skipped = 0, farHits = 0;
    for (std::size_t i = 0; i < nr::kKeepPositionCount; ++i) {
        const nr::KeepPos p = nr::kKeepPositions[i];
        const int32_t offsets[4][2] = {{out, 0}, {-out, 0}, {0, out}, {0, -out}};
        for (const auto& o : offsets) {
            const int32_t qx = p.x10 + o[0];
            const int32_t qy = p.y10 + o[1];
            // Another stored point may legitimately cover the shifted spot; skip those.
            if (NearStored(qx, qy)) {
                ++skipped;
                continue;
            }
            ++tested;
            if (nk::IsKeepDefaultPos(Metres(qx), Metres(qy))) ++farHits;
        }
    }
    KT_CHECK(tested > static_cast<int>(nr::kKeepPositionCount));
    KT_CHECK_EQ(farHits, 0);
}

void testRejectsNonFiniteAndHuge() {
    const nr::KeepPos p = nr::kKeepPositions[0];
    const float px = Metres(p.x10);
    const float py = Metres(p.y10);
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float inf = std::numeric_limits<float>::infinity();

    KT_CHECK(nk::IsKeepDefaultPos(px, py));  // sanity: the stored point itself matches

    KT_CHECK(!nk::IsKeepDefaultPos(nan, py));
    KT_CHECK(!nk::IsKeepDefaultPos(px, nan));
    KT_CHECK(!nk::IsKeepDefaultPos(nan, nan));
    KT_CHECK(!nk::IsKeepDefaultPos(inf, py));
    KT_CHECK(!nk::IsKeepDefaultPos(-inf, py));
    KT_CHECK(!nk::IsKeepDefaultPos(px, inf));
    KT_CHECK(!nk::IsKeepDefaultPos(px, -inf));
    KT_CHECK(!nk::IsKeepDefaultPos(1e9f, py));
    KT_CHECK(!nk::IsKeepDefaultPos(-1e9f, py));
    KT_CHECK(!nk::IsKeepDefaultPos(px, 1e9f));
    KT_CHECK(!nk::IsKeepDefaultPos(px, -1e9f));
}

void testCombinedGate() {
    const nr::KeepPos p = nr::kKeepPositions[0];
    const float px = Metres(p.x10);
    const float py = Metres(p.y10);
    const float nan = std::numeric_limits<float>::quiet_NaN();

    KT_CHECK(nk::IsKeepDefault("notakeeptag", px, py, true));
    KT_CHECK(!nk::IsKeepDefault("notakeeptag", px, py, false));  // xyOk suppresses the position branch
    KT_CHECK(nk::IsKeepDefault(nr::kKeepTags[0], nan, nan, false));  // tag branch alone
    KT_CHECK(!nk::IsKeepDefault("notakeeptag", nan, py, true));
    KT_CHECK(!nk::IsKeepDefault(nullptr, 1e9f, 1e9f, true));
}

} // namespace

int main() {
    testPinnedCounts();
    testEveryTagMatches();
    testNearMissAndWholeString();
    testEmptyNullAndOverlong();
    testEveryPositionMatchesWithinTolerance();
    testPositionsOutsideToleranceMiss();
    testRejectsNonFiniteAndHuge();
    testCombinedGate();
    return kt::Report("diversity_keep_test");
}
