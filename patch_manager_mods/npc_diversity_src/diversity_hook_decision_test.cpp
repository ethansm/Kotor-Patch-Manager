// diversity_hook_decision_test.cpp -- unit tests for HookDecision.h
#include "DiversityRows.h"
#include "HookDecision.h"
#include "NpcDivLayout.h"
#include "check.h"
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace {

using NpcDiversity::SkipReason;
using NpcDiversity::layout::AppearanceDesc;
using NpcDiversity::layout::ResRef;

void testShouldSkipDescriptorBranches() {
    // descMode14 is not a skip gate: in-world commoners carry descMode14 == 1.
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(1, 0, 190), SkipReason::None);   // male commoner, descMode 1
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(5, 1, 185), SkipReason::None);   // female commoner, descMode 5
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(-1, 0, 416), SkipReason::None);  // Twi'lek male, descMode -1
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(1, 1, 446), SkipReason::None);   // Twi'lek female, descMode 1

    // No descMode value produces DescMode, across genders and row classes.
    const int32_t modes[] = {0, 1, 5, -1, INT32_MIN, INT32_MAX};
    const uint8_t genders[] = {0, 1, 2, 255};
    const uint16_t rows[] = {190, 185, 416, 446, 802, 803, 300, 0};
    bool sawDescMode = false;
    for (int32_t m : modes) {
        for (uint8_t g : genders) {
            for (uint16_t r : rows) {
                if (NpcDiversity::ShouldSkipDescriptor(m, g, r) == SkipReason::DescMode) sawDescMode = true;
            }
        }
    }
    KT_CHECK(!sawDescMode);

    // Gender out of range is still a skip, with or without a nonzero descMode14.
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(1, 2, 190), SkipReason::Gender);
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(-1, 255, 185), SkipReason::Gender);

    // Gender out of range wins over the row check.
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 2, 190), SkipReason::Gender);
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 255, 185), SkipReason::Gender);
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 2, 802), SkipReason::Gender);

    // Male class needs gender 0; female class needs gender 1.
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 0, 190), SkipReason::None);   // male commoner, male
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 1, 190), SkipReason::Gender); // male commoner, female
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 0, 185), SkipReason::Gender); // female commoner, male
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 1, 185), SkipReason::None);   // female commoner, female
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 0, 339), SkipReason::Gender); // vanilla female commoner, male
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 1, 339), SkipReason::None);   // vanilla female commoner, female

    // Twi'lek classes follow the same gender rule.
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 0, 416), SkipReason::None);   // Twi'lek male
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 1, 416), SkipReason::Gender);
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 1, 446), SkipReason::None);   // Twi'lek female
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 0, 446), SkipReason::Gender);
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 0, 445), SkipReason::None);   // vanilla Twi'lek male
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 1, 280), SkipReason::None);   // vanilla Twi'lek female

    // Rows in none of the four tables are never diversified (802/803 are the fat commoners).
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 0, 802), SkipReason::NotCommonerRow);
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 1, 803), SkipReason::NotCommonerRow);
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 0, 300), SkipReason::NotCommonerRow); // former test range, not in tables
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 1, 550), SkipReason::NotCommonerRow); // former test range, not in tables
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 0, 0), SkipReason::NotCommonerRow);
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(0, 0, 311), SkipReason::NotCommonerRow);
}

void testShouldSkipDescriptorPointer() {
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(nullptr), SkipReason::NullDesc);

    AppearanceDesc d{};
    d.appearance = 190;
    d.gender = 0;
    d.descMode14 = 0;
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(&d), SkipReason::None);

    d.gender = 1;
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(&d), SkipReason::Gender);

    d.gender = 0;
    d.descMode14 = 1;  // in-world commoner value: not skipped
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(&d), SkipReason::None);

    d.descMode14 = 0;
    d.appearance = 802;
    KT_CHECK_EQ(NpcDiversity::ShouldSkipDescriptor(&d), SkipReason::NotCommonerRow);
}

void testResolveTextureSeed() {
    // Signature has no frame argument: the fallback depends only on (pDescPtr, retAddr).
    static_assert(std::is_same_v<decltype(&NpcDiversity::ResolveTextureSeed),
                                 uint32_t (*)(bool, uint32_t, uint32_t, uint32_t)>,
                  "ResolveTextureSeed must take (bool, stash, pDesc, ret) and nothing else");

    constexpr uint32_t kStash = 0xDEADBEEFu;
    constexpr uint32_t kDesc = 0x00800000u;
    constexpr uint32_t kRet = 0x0085E601u;

    // Stash precedence: a present stash wins, including a stash of 0.
    KT_CHECK_EQ(NpcDiversity::ResolveTextureSeed(true, kStash, kDesc, kRet), kStash);
    KT_CHECK_EQ(NpcDiversity::ResolveTextureSeed(true, 0u, kDesc, kRet), 0u);

    // Without a stash, the stash value is ignored.
    const uint32_t fallback = NpcDiversity::ResolveTextureSeed(false, 0u, kDesc, kRet);
    KT_CHECK_EQ(NpcDiversity::ResolveTextureSeed(false, kStash, kDesc, kRet), fallback);

    // Determinism.
    KT_CHECK_EQ(NpcDiversity::ResolveTextureSeed(false, 0u, kDesc, kRet), fallback);

    // Fallback differs for a different return address and for a different descriptor pointer.
    KT_CHECK(fallback != NpcDiversity::ResolveTextureSeed(false, 0u, kDesc, 0x0085CC00u));
    KT_CHECK(fallback != NpcDiversity::ResolveTextureSeed(false, 0u, 0x00800010u, kRet));

    // Pinned FNV-1a value for these inputs; changing the hash breaks saved texture picks.
    KT_CHECK_EQ(fallback, 0x25CA49E3u);
}

void testPlausibleResRef() {
    ResRef good{};
    std::memcpy(good.value, "N_CommM01", 9);
    KT_CHECK(NpcDiversity::IsPlausibleResRef(good.value));

    ResRef full{};
    std::memcpy(full.value, "ABCDEFGHIJKLMNOP", 16);  // 16 chars, no NUL
    KT_CHECK(NpcDiversity::IsPlausibleResRef(full.value));

    ResRef empty{};
    KT_CHECK(!NpcDiversity::IsPlausibleResRef(empty.value));

    ResRef junk{};
    std::memset(junk.value, 0xFF, sizeof junk.value);
    KT_CHECK(!NpcDiversity::IsPlausibleResRef(junk.value));

    ResRef gap{};  // NUL, then a character
    gap.value[0] = 'N';
    gap.value[2] = 'x';
    KT_CHECK(!NpcDiversity::IsPlausibleResRef(gap.value));

    ResRef ctl{};  // control byte inside the name
    ctl.value[0] = 'N';
    ctl.value[1] = '\x01';
    KT_CHECK(!NpcDiversity::IsPlausibleResRef(ctl.value));

    KT_CHECK(!NpcDiversity::IsPlausibleResRef(nullptr));
}

// Doc 44 VFS characters use appended appearance rows at or above APP_COUNT (Juhani = 818). The
// hook must leave them alone without any characterlib lookup.
void testVfsCharacterRowSkipped() {
    static_assert(818 >= NpcDiversity::APP_COUNT, "Juhani row 818 must lie at or above APP_COUNT");
    KT_CHECK(NpcDiversity::ShouldSkipDescriptor(0, 1, 818) == NpcDiversity::SkipReason::NotCommonerRow);
    KT_CHECK(NpcDiversity::ShouldSkipDescriptor(1, 0, 818) == NpcDiversity::SkipReason::NotCommonerRow);
}

} // namespace

int main() {
    testShouldSkipDescriptorBranches();
    testShouldSkipDescriptorPointer();
    testResolveTextureSeed();
    testPlausibleResRef();
    testVfsCharacterRowSkipped();
    return kt::Report("diversity_hook_decision_test");
}
