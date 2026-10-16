// diversity_head_items_test.cpp -- unit tests for DiversityHeadItems.h
#include <cstddef>
#include <cstdint>

#include "DiversityHeadItems.h"
#include "check.h"

namespace {
using diversity_head_items::HeadAppearance;
using diversity_head_items::findAppearance;
using diversity_head_items::itemAllowed;
using diversity_head_items::kHeadAppearanceCount;
using diversity_head_items::kHeadAppearances;

// Every table entry is found by its own row, and the pointer is that entry.
void testFindAppearanceRoundTrip() {
    for (unsigned i = 0; i < kHeadAppearanceCount; ++i) {
        KT_CHECK(findAppearance(kHeadAppearances[i].row) == &kHeadAppearances[i]);
    }
}

// Rows without an entry return nullptr (353 sits between 352 and 366; 65535 is past the end).
void testFindAppearanceAbsent() {
    KT_CHECK(findAppearance(353) == nullptr);
    KT_CHECK(findAppearance(65535) == nullptr);
}

// Binary search needs strictly ascending rows; the count constant must match the array.
void testHeadAppearanceSortedAndCount() {
    KT_CHECK_EQ(sizeof(kHeadAppearances) / sizeof(kHeadAppearances[0]),
                static_cast<std::size_t>(kHeadAppearanceCount));
    for (unsigned i = 1; i < kHeadAppearanceCount; ++i) {
        KT_CHECK(kHeadAppearances[i - 1].row < kHeadAppearances[i].row);
    }
}

// itemAllowed reads single bits and rejects idx >= 64 (synthetic entry, independent of the table).
void testItemAllowedBits() {
    HeadAppearance a{};
    a.allowed = 1ULL << 5;
    KT_CHECK(itemAllowed(a, 5));
    KT_CHECK(!itemAllowed(a, 4));
    KT_CHECK(!itemAllowed(a, 6));
    a.allowed = 1ULL << 63;
    KT_CHECK(itemAllowed(a, 63));
    KT_CHECK(!itemAllowed(a, 62));
    KT_CHECK(!itemAllowed(a, 64));
}

} // namespace

int main() {
    testFindAppearanceRoundTrip();
    testFindAppearanceAbsent();
    testHeadAppearanceSortedAndCount();
    testItemAllowedBits();
    return kt::Report("diversity_head_items_test");
}
