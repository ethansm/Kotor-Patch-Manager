// spawn_logic_test.cpp -- Unit tests for ClientLogic.h (roster, resref/tag matching, harvest tags)
#include "ClientLogic.h"
#include "check.h"

#include <cmath>
#include <cstddef>
#include <cstring>

namespace {

void testResrefEqualsCaseAndLength() {
    KT_CHECK(k1spawn::ResrefEquals("n_commm01", "N_COMMM01"));
    KT_CHECK(k1spawn::ResrefEquals("Ntr_Guard", "ntr_guard"));
    KT_CHECK(!k1spawn::ResrefEquals("n_commm01", "n_commm02"));
    KT_CHECK(!k1spawn::ResrefEquals("n_commm", "n_commm01"));  // shorter string is not a prefix match
    KT_CHECK(!k1spawn::ResrefEquals("n_commm01", "n_commm"));
    KT_CHECK(k1spawn::ResrefEquals("", ""));
    KT_CHECK(!k1spawn::ResrefEquals("", "a"));
}

void testResrefEqualsSixteenCharNoNul() {
    char a[16];
    char b[16];
    char c[16];
    char d[16];
    std::memcpy(a, "0123456789abcdef", 16);  // exactly 16 chars, no NUL
    std::memcpy(b, "0123456789ABCDEF", 16);  // same chars, other case
    std::memcpy(c, "0123456789abcdeX", 16);  // differs at char 15
    std::memcpy(d, "0123456789abcde", 16);   // 15 chars plus NUL
    KT_CHECK(k1spawn::ResrefEquals(a, b));
    KT_CHECK(k1spawn::ResrefEquals(a, "0123456789ABCDEF"));
    KT_CHECK(!k1spawn::ResrefEquals(a, c));
    KT_CHECK(!k1spawn::ResrefEquals(a, d));
    KT_CHECK(!k1spawn::ResrefEquals(d, a));
    KT_CHECK(k1spawn::ResrefEquals(d, "0123456789ABCDE"));
    // Only the first 16 chars count: a longer string that differs at char 17 and beyond still matches.
    KT_CHECK(k1spawn::ResrefEquals(a, "0123456789abcdefGHIJ"));
    KT_CHECK(k1spawn::ResrefEquals("0123456789abcdefX", "0123456789abcdefY"));
}

void testResrefEqualsNull() {
    KT_CHECK(!k1spawn::ResrefEquals(nullptr, "a"));
    KT_CHECK(!k1spawn::ResrefEquals("a", nullptr));
    KT_CHECK(!k1spawn::ResrefEquals(nullptr, nullptr));
}

void testTagEqualsThirtyTwoChars() {
    char tag[32];
    std::memcpy(tag, "abcdefghijklmnopqrstuvwxyz012345", 32);  // 32 chars, no NUL
    KT_CHECK(k1spawn::TagEquals(tag, "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345"));
    KT_CHECK(!k1spawn::TagEquals(tag, "abcdefghijklmnopqrstuvwxyz01234"));
    KT_CHECK(k1spawn::TagEquals("abcdefghijklmnopqrstuvwxyz012345X", "abcdefghijklmnopqrstuvwxyz012345Y"));  // only 32 count
    KT_CHECK(!k1spawn::TagEquals(nullptr, "a"));
    KT_CHECK(!k1spawn::TagEquals("a", nullptr));
    KT_CHECK(!k1spawn::TagEquals("ntr_guar", "ntr_guard"));  // prefix is not a match
}

void testRowsForAreaMatch() {
    int idx[k1spawn::kMaxRowsPerArea];
    KT_CHECK_EQ(k1spawn::RowsForArea("204tel", idx, k1spawn::kMaxRowsPerArea), 5);
    KT_CHECK_EQ(idx[0], 0);
    KT_CHECK_EQ(idx[1], 1);
    KT_CHECK_EQ(idx[2], 2);
    KT_CHECK_EQ(k1spawn::RowsForArea("204TEL", idx, k1spawn::kMaxRowsPerArea), 5);  // case does not matter
    KT_CHECK_EQ(k1spawn::RowsForArea("other", idx, k1spawn::kMaxRowsPerArea), 0);
    KT_CHECK_EQ(k1spawn::RowsForArea("", idx, k1spawn::kMaxRowsPerArea), 0);
    KT_CHECK_EQ(k1spawn::RowsForArea("204tel2", idx, k1spawn::kMaxRowsPerArea), 0);  // a longer name is not a match
    KT_CHECK_EQ(k1spawn::RowsForArea("204te", idx, k1spawn::kMaxRowsPerArea), 0);    // a prefix is not a match
}

void testRowsForAreaBounds() {
    int idx[k1spawn::kMaxRowsPerArea];
    KT_CHECK_EQ(k1spawn::RowsForArea(nullptr, idx, k1spawn::kMaxRowsPerArea), 0);
    KT_CHECK_EQ(k1spawn::RowsForArea("204tel", nullptr, 4), 0);
    KT_CHECK_EQ(k1spawn::RowsForArea("204tel", idx, 0), 0);
    KT_CHECK_EQ(k1spawn::RowsForArea("204tel", idx, -1), 0);
    KT_CHECK_EQ(k1spawn::RowsForArea("204tel", idx, 1), 1);  // capped at the buffer size, roster order kept
    KT_CHECK_EQ(idx[0], 0);
}

void testRowsForAreaSixteenCharNoNul() {
    char area[16];
    std::memcpy(area, "204tel__________", 16);  // 16 chars, no NUL: must not be read past index 15 and must not match "204tel"
    int idx[k1spawn::kMaxRowsPerArea];
    KT_CHECK_EQ(k1spawn::RowsForArea(area, idx, k1spawn::kMaxRowsPerArea), 0);
}

void testHarvestTags() {
    KT_CHECK(k1spawn::IsHarvestTag("k1p_native_npc"));
    KT_CHECK(k1spawn::IsHarvestTag("K1P_PROBE_NPC"));
    KT_CHECK(k1spawn::IsHarvestTag("k1p_probe_npc"));
    KT_CHECK(k1spawn::IsHarvestTag("MouseDroid"));
    KT_CHECK(k1spawn::IsHarvestTag("mousedroid"));
    KT_CHECK(k1spawn::IsHarvestTag("k1p_carth"));
    KT_CHECK(!k1spawn::IsHarvestTag("k1p_carthx"));
    KT_CHECK(k1spawn::IsHarvestTag("k1p_juhanv"));
    KT_CHECK(!k1spawn::IsHarvestTag("MouseDroid2"));
    KT_CHECK(!k1spawn::IsHarvestTag("k1p_native"));
    KT_CHECK(!k1spawn::IsHarvestTag("ntr_guard"));
    KT_CHECK(!k1spawn::IsHarvestTag(""));
    KT_CHECK(!k1spawn::IsHarvestTag(nullptr));
}

void testRosterIntegrity() {
    KT_CHECK(k1spawn::kRosterCount >= 2);
    for (int i = 0; i < k1spawn::kRosterCount; ++i) {
        const k1spawn::RosterRow& r = k1spawn::kRoster[i];
        KT_CHECK(r.resref[0] != 0 && std::strlen(r.resref) <= 16);
        KT_CHECK(r.tag[0] != 0 && std::strlen(r.tag) <= 32);
        KT_CHECK(r.area[0] != 0 && std::strlen(r.area) <= 16);
        KT_CHECK(std::isfinite(r.x) && std::isfinite(r.y) && std::isfinite(r.z) && std::isfinite(r.facing));
        // every roster tag is a harvest tag, so the NPC dump covers what the client spawns
        KT_CHECK(k1spawn::IsHarvestTag(r.tag));
        // tags are unique: the duplicate guard is per tag
        for (int j = i + 1; j < k1spawn::kRosterCount; ++j) KT_CHECK(!k1spawn::TagEquals(r.tag, k1spawn::kRoster[j].tag));
    }
    // the rows the play tests were written for (0.2.0: pet + native NPC; 0.3.2: K1 Juhani)
    KT_CHECK(k1spawn::ResrefEquals(k1spawn::kRoster[0].resref, "c_drdmouse01"));
    KT_CHECK(k1spawn::ResrefEquals(k1spawn::kRoster[1].resref, "k1p_native_npc"));
    KT_CHECK(k1spawn::ResrefEquals(k1spawn::kRoster[2].resref, "k1p_juhani"));
    KT_CHECK(k1spawn::ResrefEquals(k1spawn::kRoster[4].resref, "k1p_juhanv"));
}

}  // namespace

int main() {
    testResrefEqualsCaseAndLength();
    testResrefEqualsSixteenCharNoNul();
    testResrefEqualsNull();
    testTagEqualsThirtyTwoChars();
    testRowsForAreaMatch();
    testRowsForAreaBounds();
    testRowsForAreaSixteenCharNoNul();
    testHarvestTags();
    testRosterIntegrity();
    return kt::Report("spawn_logic_test");
}
