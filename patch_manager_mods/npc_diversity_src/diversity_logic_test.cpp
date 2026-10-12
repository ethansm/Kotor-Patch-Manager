// diversity_logic_test.cpp -- Unit tests for DiversityLogic.h
#include "DiversityLogic.h"
#include "check.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

namespace {

using NpcDiversity::SlotFamily;

constexpr uint32_t kDraws = 100000;

// Per-creature hash through the real pipeline (position, objectId, row).
uint32_t DrawHash(uint32_t i, uint16_t row) {
    return NpcDiversity::HashCreaturePosition(static_cast<float>(i) * 0.37f,
                                              static_cast<float>(i) * 0.11f, i, row);
}

// True when count / total is within [lo, hi] percent. Prints the observed share on failure.
bool ShareInRange(const char* label, long count, long total, double lo, double hi) {
    double pct = total > 0 ? 100.0 * static_cast<double>(count) / static_cast<double>(total) : -1.0;
    bool ok = pct >= lo && pct <= hi;
    if (!ok) {
        std::printf("  %s: %ld/%ld = %.3f%% not in [%.2f, %.2f]\n", label, count, total, pct, lo, hi);
    }
    return ok;
}

// First raw hash i whose band roll equals target (target < 100).
uint32_t HashWithBandRoll(uint32_t target) {
    for (uint32_t h = 0; h < 1000000u; ++h) {
        if (NpcDiversity::BandRoll(h) == target) return h;
    }
    return 0;
}

// True when v is one of the first n entries of a generated allowlist (DiversitySlots.h).
bool InAllow(const uint8_t* list, size_t n, uint8_t v) {
    for (size_t k = 0; k < n; ++k) {
        if (list[k] == v) return true;
    }
    return false;
}

// Draws kDraws creatures on one row: nothing outside the allowlist, every entry within +-25% of uniform.
void CheckUniformOverList(const char* label, uint16_t row, const uint8_t* list, size_t n) {
    long counts[256] = {};
    for (uint32_t oid = 1; oid <= kDraws; ++oid) {
        counts[NpcDiversity::PickTextureVar(oid, row)]++;
    }
    KT_CHECK_EQ(counts[0], 0L);

    const double uniformPct = 100.0 / static_cast<double>(n);
    long total = 0;
    for (size_t k = 0; k < n; ++k) {
        long c = counts[list[k]];
        total += c;
        KT_CHECK(ShareInRange(label, c, kDraws, uniformPct * 0.75, uniformPct * 1.25));
    }
    KT_CHECK_EQ(total, static_cast<long>(kDraws));  // no value outside the allowlist
}

// For every allowlist entry as the current variant, 2000 creatures on the row never get that entry back.
void CheckNeverEqualCurrent(const char* label, uint16_t row, const uint8_t* list, size_t n) {
    bool ok = true;
    for (size_t k = 0; k < n; ++k) {
        const uint8_t cur = list[k];
        for (uint32_t oid = 1; oid <= 2000u; ++oid) {
            const uint8_t v = NpcDiversity::PickTextureVar(oid, row, cur);
            if (v == cur || !InAllow(list, n, v)) ok = false;
        }
    }
    if (!ok) std::printf("  %s: returned the current variant or a value outside the allowlist\n", label);
    KT_CHECK(ok);
}

bool IsTwilekMaleVariant(uint16_t v) { return v >= 805 && v <= 808; }
bool IsDancer(uint16_t v) { return v >= 809 && v <= 811; }

void testFmix32Golden() {
    KT_CHECK_EQ(NpcDiversity::Fmix32(0u), 0x00000000u);
    KT_CHECK_EQ(NpcDiversity::Fmix32(1u), 0x514E28B7u);
    KT_CHECK_EQ(NpcDiversity::Fmix32(0xdeadbeefu), 0x0DE5C6A9u);
}

void testMixGoldenAndIndependence() {
    KT_CHECK_EQ(NpcDiversity::Mix(12345u, 4u), 0xEB5B36F3u);
    KT_CHECK_EQ(NpcDiversity::BandRoll(12345u), 82u);

    // Different salts give nearly independent residues mod 100: P(equal) ~ 1%.
    long equal = 0;
    for (uint32_t i = 0; i < kDraws; ++i) {
        uint32_t h = DrawHash(i, 190);
        if (NpcDiversity::Mix(h, 1) % 100u == NpcDiversity::Mix(h, 2) % 100u) ++equal;
    }
    KT_CHECK(ShareInRange("salt1 vs salt2 residue equality", equal, kDraws, 0.7, 1.3));
}

void testHashDescriptorGolden() {
    uint32_t d1 = NpcDiversity::HashDescriptor(190, 0, 1, 0x1234);
    KT_CHECK_EQ(d1, 0x06C232B2u);
    KT_CHECK_EQ(d1, NpcDiversity::HashDescriptor(190, 0, 1, 0x1234));

    KT_CHECK(d1 != NpcDiversity::HashDescriptor(190, 1, 1, 0x1234));
    KT_CHECK(d1 != NpcDiversity::HashDescriptor(191, 0, 1, 0x1234));
}

void testHashSeedFallback() {
    uint32_t f1 = NpcDiversity::HashSeedFallback(0x1000u, 0x2000u);
    KT_CHECK_EQ(f1, 0x79B53855u);
    KT_CHECK_EQ(f1, NpcDiversity::HashSeedFallback(0x1000u, 0x2000u));
    KT_CHECK(f1 != NpcDiversity::HashSeedFallback(0x1000u, 0x2001u));
    KT_CHECK(f1 != NpcDiversity::HashSeedFallback(0x1001u, 0x2000u));
}

void testShouldVaryGolden() {
    // Vary-gate draw: Mix(HashCreatureId(oid, row), 5) % 100 < kVaryPct (35).
    KT_CHECK_EQ(NpcDiversity::Mix(NpcDiversity::HashCreatureId(1u, 190), 5), 0xA91B8E6Au);
    KT_CHECK(NpcDiversity::ShouldVary(1u, 190));           // 34
    KT_CHECK(!NpcDiversity::ShouldVary(2u, 190));          // 48
    KT_CHECK(!NpcDiversity::ShouldVary(42u, 190));         // 70
    KT_CHECK(!NpcDiversity::ShouldVary(1000u, 190));       // 91
    KT_CHECK(NpcDiversity::ShouldVary(0xDEADBEEFu, 190));  // 22
}

void testIsCommonerTexture() {
    bool isFemale = false;

    KT_CHECK(NpcDiversity::IsCommonerTexture("N_CommM01", isFemale));
    KT_CHECK_EQ(isFemale, false);

    KT_CHECK(NpcDiversity::IsCommonerTexture("n_commm12", isFemale));
    KT_CHECK_EQ(isFemale, false);

    KT_CHECK(NpcDiversity::IsCommonerTexture("N_CommF01", isFemale));
    KT_CHECK_EQ(isFemale, true);

    KT_CHECK(NpcDiversity::IsCommonerTexture("n_commf35", isFemale));
    KT_CHECK_EQ(isFemale, true);

    KT_CHECK(!NpcDiversity::IsCommonerTexture("P_AttonB", isFemale));
    KT_CHECK(!NpcDiversity::IsCommonerTexture("PMBMM", isFemale));
    KT_CHECK(!NpcDiversity::IsCommonerTexture("N_RepOff", isFemale));
    KT_CHECK(!NpcDiversity::IsCommonerTexture("", isFemale));
    KT_CHECK(!NpcDiversity::IsCommonerTexture(nullptr, isFemale));
}

void testShouldDiversifyTexture() {
    char outTex[32] = {};
    const uint32_t hash = 123456789u;

    // Golden: the prefix follows the texture, the slot is drawn from the row's family allowlist.
    KT_CHECK(NpcDiversity::ShouldDiversifyTexture("N_CommM01", 190, hash, outTex, sizeof(outTex)));
    KT_CHECK(std::strcmp(outTex, "N_CommM13") == 0);
    KT_CHECK(NpcDiversity::ShouldDiversifyTexture("n_commf01", 185, hash, outTex, sizeof(outTex)));
    KT_CHECK(std::strcmp(outTex, "N_CommF15") == 0);

    // Fat rows have one slot each: 34 for male, 35 for female.
    KT_CHECK(NpcDiversity::ShouldDiversifyTexture("N_CommM01", 802, hash, outTex, sizeof(outTex)));
    KT_CHECK(std::strcmp(outTex, "N_CommM34") == 0);
    KT_CHECK(NpcDiversity::ShouldDiversifyTexture("N_CommF01", 803, hash, outTex, sizeof(outTex)));
    KT_CHECK(std::strcmp(outTex, "N_CommF35") == 0);

    // Non-commoner texture, row with no family (SlotFamily::None), null pointers, short buffer: no change.
    KT_CHECK(!NpcDiversity::ShouldDiversifyTexture("P_AttonB", 190, hash, outTex, sizeof(outTex)));
    KT_CHECK(!NpcDiversity::ShouldDiversifyTexture("N_CommM01", 300, hash, outTex, sizeof(outTex)));
    KT_CHECK(!NpcDiversity::ShouldDiversifyTexture(nullptr, 190, hash, outTex, sizeof(outTex)));
    KT_CHECK(!NpcDiversity::ShouldDiversifyTexture("N_CommM01", 190, hash, outTex, 15));

    // Every generated name is N_CommF NN with NN drawn from kFemaleSlots.
    bool namesOk = true;
    for (uint32_t i = 0; i < 1000; ++i) {
        if (!NpcDiversity::ShouldDiversifyTexture("N_CommF01", 185, DrawHash(i, 185), outTex, sizeof(outTex))) namesOk = false;
        if (std::strlen(outTex) != 9 || std::strncmp(outTex, "N_CommF", 7) != 0) namesOk = false;
        const int v = (outTex[7] - '0') * 10 + (outTex[8] - '0');
        if (!InAllow(NpcDiversity::kFemaleSlots, NpcDiversity::kFemaleSlotsCount, static_cast<uint8_t>(v))) namesOk = false;
    }
    KT_CHECK(namesOk);
}

void testHashCreaturePosition() {
    uint32_t h1 = NpcDiversity::HashCreaturePosition(12.5f, -34.8f, 42, 190);
    KT_CHECK_EQ(h1, 0xD65B15FDu);
    KT_CHECK_EQ(h1, NpcDiversity::HashCreaturePosition(12.5f, -34.8f, 42, 190));

    KT_CHECK(h1 != NpcDiversity::HashCreaturePosition(12.6f, -34.8f, 42, 190));
    KT_CHECK(h1 != NpcDiversity::HashCreaturePosition(12.5f, -34.7f, 42, 190));
    KT_CHECK(h1 != NpcDiversity::HashCreaturePosition(12.5f, -34.8f, 99, 190));
    KT_CHECK(h1 != NpcDiversity::HashCreaturePosition(12.5f, -34.8f, 42, 350));
}

void testQuantizeCoordGuards() {
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float inf = std::numeric_limits<float>::infinity();

    KT_CHECK_EQ(NpcDiversity::QuantizeCoord(nan), 0);
    KT_CHECK_EQ(NpcDiversity::QuantizeCoord(1.0e6f), 10000000);
    KT_CHECK_EQ(NpcDiversity::QuantizeCoord(inf), 10000000);
    KT_CHECK_EQ(NpcDiversity::QuantizeCoord(1.0e9f), 10000000);
    KT_CHECK_EQ(NpcDiversity::QuantizeCoord(-inf), -10000000);
    KT_CHECK_EQ(NpcDiversity::QuantizeCoord(-1.0e9f), -10000000);
    KT_CHECK_EQ(NpcDiversity::QuantizeCoord(12.5f), 125);

    // Non-finite and huge inputs hash deterministically and do not collapse into the normal range
    KT_CHECK_EQ(NpcDiversity::HashCreaturePosition(nan, nan, 5, 190),
                NpcDiversity::HashCreaturePosition(0.0f, 0.0f, 5, 190));
    KT_CHECK_EQ(NpcDiversity::HashCreaturePosition(inf, -inf, 5, 190),
                NpcDiversity::HashCreaturePosition(1.0e6f, -1.0e6f, 5, 190));
    KT_CHECK_EQ(NpcDiversity::HashCreaturePosition(1.0e9f, -1.0e9f, 5, 190),
                NpcDiversity::HashCreaturePosition(inf, -inf, 5, 190));
    KT_CHECK_EQ(NpcDiversity::HashCreaturePosition(nan, nan, 5, 190),
                NpcDiversity::HashCreaturePosition(nan, nan, 5, 190));
    KT_CHECK(NpcDiversity::HashCreaturePosition(inf, 0.0f, 5, 190) !=
             NpcDiversity::HashCreaturePosition(0.0f, 0.0f, 5, 190));
}

// Band edges: a raw hash whose BandRoll equals target must select the expected branch.
void testBandEdges() {
    // Male commoner 190: 0..14 fat, 15..99 unchanged
    const uint32_t maleTargets[] = {0, 14, 15, 19, 20, 24, 25, 99};
    for (uint32_t t : maleTargets) {
        uint32_t h = HashWithBandRoll(t);
        KT_CHECK_EQ(NpcDiversity::BandRoll(h), t);
        uint16_t want = (t < NpcDiversity::kFatPct) ? NpcDiversity::APP_COMMONER_FAT_MALE : 190;
        KT_CHECK_EQ(NpcDiversity::RemapAppearance(190, 0, h), want);
    }

    // Female commoner 185: 0..14 fat, 15..19 czerka, 20..99 unchanged
    const uint32_t femaleTargets[] = {0, 14, 15, 19, 20, 24, 25, 99};
    for (uint32_t t : femaleTargets) {
        uint32_t h = HashWithBandRoll(t);
        KT_CHECK_EQ(NpcDiversity::BandRoll(h), t);
        uint16_t want = 185;
        if (t < NpcDiversity::kFatPct) want = NpcDiversity::APP_COMMONER_FAT_FEMALE;
        else if (t < NpcDiversity::kFatPct + NpcDiversity::kCzerkaPct) want = NpcDiversity::APP_CZERKA_OFFICER_FEMALE;
        KT_CHECK_EQ(NpcDiversity::RemapAppearance(185, 1, h), want);
    }

    // Male Twi'lek 416: 0..79 remapped to 805..808, 80..99 unchanged
    const uint32_t twiMaleTargets[] = {0, 14, 24, 25, 79, 80, 99};
    for (uint32_t t : twiMaleTargets) {
        uint32_t h = HashWithBandRoll(t);
        KT_CHECK_EQ(NpcDiversity::BandRoll(h), t);
        uint16_t got = NpcDiversity::RemapAppearance(416, 0, h);
        if (t < NpcDiversity::kTwilekMaleRemapPct) {
            KT_CHECK(IsTwilekMaleVariant(got));
        } else {
            KT_CHECK_EQ(got, 416);
        }
    }

    // Female Twi'lek 446: 0..19 dancer (809..811), 20..99 unchanged
    const uint32_t twiFemTargets[] = {0, 14, 19, 20, 24, 25, 99};
    for (uint32_t t : twiFemTargets) {
        uint32_t h = HashWithBandRoll(t);
        KT_CHECK_EQ(NpcDiversity::BandRoll(h), t);
        uint16_t got = NpcDiversity::RemapAppearance(446, 1, h);
        if (t < NpcDiversity::kDancerPct) {
            KT_CHECK(IsDancer(got));
        } else {
            KT_CHECK_EQ(got, 446);
        }
    }
}

void testGenderMismatchReturnsInput() {
    const uint32_t fatMaleHash = HashWithBandRoll(0);      // would change a male commoner
    const uint32_t czerkaHash = HashWithBandRoll(16);      // would change a female commoner
    const uint32_t lowRollHash = HashWithBandRoll(5);      // inside both Twi'lek bands (remap and dancer)

    // Male commoner row: only gender 0 may change it
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(190, 0, fatMaleHash), NpcDiversity::APP_COMMONER_FAT_MALE);
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(190, 1, fatMaleHash), 190);
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(190, 2, fatMaleHash), 190);
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(190, 255, fatMaleHash), 190);

    // Female commoner row: only gender 1 may change it
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(185, 1, czerkaHash), NpcDiversity::APP_CZERKA_OFFICER_FEMALE);
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(185, 0, czerkaHash), 185);
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(185, 2, czerkaHash), 185);
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(185, 255, czerkaHash), 185);

    // Male Twi'lek row: only gender 0 may change it
    KT_CHECK(IsTwilekMaleVariant(NpcDiversity::RemapAppearance(416, 0, lowRollHash)));
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(416, 1, lowRollHash), 416);
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(416, 2, lowRollHash), 416);
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(416, 255, lowRollHash), 416);

    // Female Twi'lek row: only gender 1 may change it
    KT_CHECK(IsDancer(NpcDiversity::RemapAppearance(446, 1, lowRollHash)));
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(446, 0, lowRollHash), 446);
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(446, 2, lowRollHash), 446);
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(446, 255, lowRollHash), 446);

    // Non-generic appearance unchanged for any gender
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(999, 0, 12345u), 999);
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(999, 1, 12345u), 999);
}

void testMaleCommonerDistribution() {
    long fat = 0, unchanged = 0, other = 0, alien = 0;
    long fatEven = 0, evenTotal = 0, fatOdd = 0, oddTotal = 0;
    for (uint32_t i = 0; i < kDraws; ++i) {
        uint32_t h = DrawHash(i, 190);
        uint16_t r = NpcDiversity::RemapAppearance(190, 0, h);
        bool isFat = (r == NpcDiversity::APP_COMMONER_FAT_MALE);
        if (isFat) ++fat;
        else if (r == 190) ++unchanged;
        else ++other;
        if (r >= 812 && r <= 815) ++alien;  // Kel Dor / Iridorian rows must never be produced
        if (h & 1u) { ++oddTotal; if (isFat) ++fatOdd; }
        else        { ++evenTotal; if (isFat) ++fatEven; }
    }
    KT_CHECK(ShareInRange("male fat", fat, kDraws, 14.0, 16.0));
    KT_CHECK(ShareInRange("male unchanged", unchanged, kDraws, 84.0, 86.0));
    KT_CHECK_EQ(other, 0L);
    KT_CHECK_EQ(alien, 0L);

    // Fat share holds on each parity of the hash (no dependence on the low bit)
    KT_CHECK(ShareInRange("male fat, even hash", fatEven, evenTotal, 14.0, 16.0));
    KT_CHECK(ShareInRange("male fat, odd hash", fatOdd, oddTotal, 14.0, 16.0));
}

void testFemaleCommonerDistribution() {
    long fat = 0, czerka = 0, unchanged = 0, other = 0;
    for (uint32_t i = 0; i < kDraws; ++i) {
        uint16_t r = NpcDiversity::RemapAppearance(185, 1, DrawHash(i, 185));
        if (r == NpcDiversity::APP_COMMONER_FAT_FEMALE) ++fat;
        else if (r == NpcDiversity::APP_CZERKA_OFFICER_FEMALE) ++czerka;
        else if (r == 185) ++unchanged;
        else ++other;
    }
    KT_CHECK(ShareInRange("female fat", fat, kDraws, 14.0, 16.0));
    KT_CHECK(ShareInRange("female czerka", czerka, kDraws, 4.3, 5.7));
    KT_CHECK(ShareInRange("female unchanged", unchanged, kDraws, 79.0, 81.0));
    KT_CHECK_EQ(other, 0L);
}

void testTwilekMaleDistribution() {
    long remapped = 0, unchanged = 0, other = 0;
    long variant[4] = {0, 0, 0, 0};
    // Restricted subset: draws with hash % 100 < kTwilekMaleRemapPct (variant must not depend on the raw low digits)
    long subsetRemapped = 0;
    long subsetVariant[4] = {0, 0, 0, 0};
    long remapEven = 0, evenTotal = 0, remapOdd = 0, oddTotal = 0;
    for (uint32_t i = 0; i < kDraws; ++i) {
        uint32_t h = DrawHash(i, 416);
        uint16_t r = NpcDiversity::RemapAppearance(416, 0, h);
        bool isRemapped = IsTwilekMaleVariant(r);
        if (isRemapped) {
            ++remapped;
            ++variant[r - 805];
        } else if (r == 416) {
            ++unchanged;
        } else {
            ++other;
        }
        if (h % 100u < NpcDiversity::kTwilekMaleRemapPct && isRemapped) {
            ++subsetRemapped;
            ++subsetVariant[r - 805];
        }
        if (h & 1u) { ++oddTotal; if (isRemapped) ++remapOdd; }
        else        { ++evenTotal; if (isRemapped) ++remapEven; }
    }
    KT_CHECK(ShareInRange("twi'lek male remap", remapped, kDraws, 79.0, 81.0));
    KT_CHECK(ShareInRange("twi'lek male unchanged", unchanged, kDraws, 19.0, 21.0));
    KT_CHECK_EQ(other, 0L);
    for (int k = 0; k < 4; ++k) {
        KT_CHECK(ShareInRange("twi'lek male variant share of remapped", variant[k], remapped, 23.0, 27.0));
    }
    for (int k = 0; k < 4; ++k) {
        KT_CHECK(ShareInRange("twi'lek male variant share, hash%100<80 subset", subsetVariant[k], subsetRemapped, 20.0, 30.0));
    }
    KT_CHECK(ShareInRange("twi'lek male remap, even hash", remapEven, evenTotal, 79.0, 81.0));
    KT_CHECK(ShareInRange("twi'lek male remap, odd hash", remapOdd, oddTotal, 79.0, 81.0));
}

void testTwilekFemaleDistribution() {
    long dancers = 0, unchanged = 0, other = 0;
    long variant[3] = {0, 0, 0};
    for (uint32_t i = 0; i < kDraws; ++i) {
        uint16_t r = NpcDiversity::RemapAppearance(446, 1, DrawHash(i, 446));
        if (IsDancer(r)) {
            ++dancers;
            ++variant[r - 809];
        } else if (r == 446) {
            ++unchanged;
        } else {
            ++other;
        }
    }
    KT_CHECK(ShareInRange("twi'lek female dancers", dancers, kDraws, 19.0, 21.0));
    KT_CHECK(ShareInRange("twi'lek female unchanged", unchanged, kDraws, 79.0, 81.0));
    KT_CHECK_EQ(other, 0L);
    for (int k = 0; k < 3; ++k) {
        // 1/3 of the dancers, +/- 3 points
        KT_CHECK(ShareInRange("dancer variant share", variant[k], dancers, 100.0 / 3.0 - 3.0, 100.0 / 3.0 + 3.0));
    }
}

// Male and female family allowlists: every entry reached, nothing outside, each within +-25% of uniform.
void testFamilySlotCoverage() {
    CheckUniformOverList("texvar slot, M row 190", 190, NpcDiversity::kMaleSlots, NpcDiversity::kMaleSlotsCount);
    CheckUniformOverList("texvar slot, F row 185", 185, NpcDiversity::kFemaleSlots, NpcDiversity::kFemaleSlotsCount);
}

// Golden values from an independent Python FNV-1a/fmix32 reference (bytes LSB first: oid, then appearance).
void testHashCreatureIdGolden() {
    KT_CHECK_EQ(NpcDiversity::HashCreatureId(1u, 190), 0x8A2319E1u);
    KT_CHECK_EQ(NpcDiversity::HashCreatureId(0x12345678u, 802), 0x8D7671C1u);
    KT_CHECK_EQ(NpcDiversity::HashCreatureId(0xDEADBEEFu, 811), 0xCA471ED4u);
    KT_CHECK_EQ(NpcDiversity::HashCreatureId(1000u, 805), 0x494232EAu);
    KT_CHECK_EQ(NpcDiversity::HashCreatureId(42u, 185), 0xE4F9FA16u);
    KT_CHECK_EQ(NpcDiversity::HashCreatureId(0xFFFFFFFFu, 0xFFFFu), 0xF2FCD5C4u);

    // Determinism, and both inputs feed the hash.
    KT_CHECK_EQ(NpcDiversity::HashCreatureId(1000u, 805), NpcDiversity::HashCreatureId(1000u, 805));
    KT_CHECK(NpcDiversity::HashCreatureId(1000u, 805) != NpcDiversity::HashCreatureId(1001u, 805));
    KT_CHECK(NpcDiversity::HashCreatureId(1000u, 805) != NpcDiversity::HashCreatureId(1000u, 806));
}

void testTexVarEligible() {
    // Generic male and female commoner tables (representative rows from each table).
    KT_CHECK(NpcDiversity::TexVarEligible(17));
    KT_CHECK(NpcDiversity::TexVarEligible(190));
    KT_CHECK(NpcDiversity::TexVarEligible(617));
    KT_CHECK(NpcDiversity::TexVarEligible(14));
    KT_CHECK(NpcDiversity::TexVarEligible(185));
    KT_CHECK(NpcDiversity::TexVarEligible(614));

    // Appended rows: 802 fat male, 803 fat female, 805..808 male Twi'lek commoner variants, 811 underwear.
    KT_CHECK(NpcDiversity::TexVarEligible(802));
    KT_CHECK(NpcDiversity::TexVarEligible(803));
    KT_CHECK(NpcDiversity::TexVarEligible(805));
    KT_CHECK(NpcDiversity::TexVarEligible(806));
    KT_CHECK(NpcDiversity::TexVarEligible(807));
    KT_CHECK(NpcDiversity::TexVarEligible(808));
    KT_CHECK(NpcDiversity::TexVarEligible(811));

    // Excluded: 804 Czerka officer, 809/810 dancers.
    KT_CHECK(!NpcDiversity::TexVarEligible(804));
    KT_CHECK(!NpcDiversity::TexVarEligible(809));
    KT_CHECK(!NpcDiversity::TexVarEligible(810));

    // Story-style row outside every table, the zero row, and the all-ones row.
    KT_CHECK(!NpcDiversity::TexVarEligible(300));
    KT_CHECK(!NpcDiversity::TexVarEligible(0));
    KT_CHECK(!NpcDiversity::TexVarEligible(0xFFFFu));

    // Vanilla Twi'lek rows are table-only (remapped to 805..808 before this check), so they are not eligible.
    KT_CHECK(!NpcDiversity::TexVarEligible(416));
    KT_CHECK(!NpcDiversity::TexVarEligible(446));

    // Exactly 57 eligible rows across the whole uint16 range: 24 male + 26 female + 7 appended.
    int eligible = 0;
    for (uint32_t a = 0; a <= 0xFFFFu; ++a) {
        if (NpcDiversity::TexVarEligible(static_cast<uint16_t>(a))) ++eligible;
    }
    KT_CHECK_EQ(eligible, 57);
}

void testPickTextureVarGateAndGolden() {
    // oid 0 means no creature id: always 0, for any row and any current variant.
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(0u, 190), 0);
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(0u, 802), 0);
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(0u, 0xFFFFu), 0);
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(0u, 190, 13), 0);

    // Rows with no family (SlotFamily::None) get 0 for any nonzero oid.
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(5u, 804), 0);
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(5u, 809), 0);
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(5u, 810), 0);
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(5u, 416), 0);
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(5u, 300), 0);
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(5u, 0), 0);
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(5u, 0xFFFFu), 0);

    // Golden picks: Mix(HashCreatureId(oid, row), 4) % n indexes the family allowlist.
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(1u, 190), 13);          // M, index 12 of 16
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(1000u, 805), 13);       // M (Twi'lek variant row)
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(0x12345678u, 617), 1);  // M
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(42u, 185), 1);          // F
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(0xDEADBEEFu, 811), 8);  // F (underwear row)
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(0x12345678u, 802), 34); // FatM
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(0xDEADBEEFu, 803), 35); // FatF

    // Current-variant skip: a draw equal to currentTv moves to the next allowlist entry.
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(1u, 190, 13), 14);
    KT_CHECK_EQ(NpcDiversity::PickTextureVar(42u, 185, 1), 2);

    // Dispatcher idempotence: the 2-arg pick is a pure function of (oid, final row), so re-applying a row
    // returns the value already written (no write). Passing that value back as currentTv would move to the
    // next slot, which is why the dispatcher must not pass currentTv.
    {
        const uint32_t oids[3] = {1u, 42u, 0x12345678u};
        const uint16_t apps[3] = {190, 185, 617};
        for (int i = 0; i < 3; ++i) {
            const uint8_t a = NpcDiversity::PickTextureVar(oids[i], apps[i]);
            KT_CHECK_EQ(NpcDiversity::PickTextureVar(oids[i], apps[i]), a);
            KT_CHECK(NpcDiversity::PickTextureVar(oids[i], apps[i], a) != a);
        }
    }
}

// Each of the four families draws only from its own allowlist (rows: M 190/805, F 185/811, FatM 802, FatF 803).
void testPickTextureVarRangeAndDeterminism() {
    struct Case {
        uint16_t row;
        const uint8_t* list;
        size_t n;
    };
    const Case cases[] = {
        {190, NpcDiversity::kMaleSlots, NpcDiversity::kMaleSlotsCount},
        {805, NpcDiversity::kMaleSlots, NpcDiversity::kMaleSlotsCount},
        {185, NpcDiversity::kFemaleSlots, NpcDiversity::kFemaleSlotsCount},
        {811, NpcDiversity::kFemaleSlots, NpcDiversity::kFemaleSlotsCount},
        {802, NpcDiversity::kFatMaleSlots, NpcDiversity::kFatMaleSlotsCount},
        {803, NpcDiversity::kFatFemaleSlots, NpcDiversity::kFatFemaleSlotsCount},
    };
    for (const Case& c : cases) {
        bool inList = true;
        for (uint32_t oid = 1; oid <= 10000u; ++oid) {
            if (!InAllow(c.list, c.n, NpcDiversity::PickTextureVar(oid, c.row))) inList = false;
        }
        KT_CHECK(inList);
    }
}

void testPickTextureVarUniform() {
    // Uniform over the family allowlist for the appended Twi'lek male (805) and underwear (811) rows.
    CheckUniformOverList("texvar slot, row 805", 805, NpcDiversity::kMaleSlots, NpcDiversity::kMaleSlotsCount);
    CheckUniformOverList("texvar slot, row 811", 811, NpcDiversity::kFemaleSlots, NpcDiversity::kFemaleSlotsCount);
}

void testDeterminism() {
    bool stable = true;
    for (uint32_t oid = 1; oid <= 10000u; ++oid) {
        if (NpcDiversity::PickTextureVar(oid, 190, 5) != NpcDiversity::PickTextureVar(oid, 190, 5)) stable = false;
        if (NpcDiversity::ShouldVary(oid, 190) != NpcDiversity::ShouldVary(oid, 190)) stable = false;
    }
    KT_CHECK(stable);

    // The row feeds the hash: rows 190 and 805 share a family allowlist but pick differently for many creatures.
    long differ = 0;
    for (uint32_t oid = 1; oid <= 10000u; ++oid) {
        if (NpcDiversity::PickTextureVar(oid, 190) != NpcDiversity::PickTextureVar(oid, 805)) ++differ;
    }
    KT_CHECK(differ > 5000);
}

void testFamilyOfMatchesTexVarEligible() {
    KT_CHECK(NpcDiversity::FamilyOf(190) == SlotFamily::M);
    KT_CHECK(NpcDiversity::FamilyOf(617) == SlotFamily::M);
    KT_CHECK(NpcDiversity::FamilyOf(805) == SlotFamily::M);
    KT_CHECK(NpcDiversity::FamilyOf(808) == SlotFamily::M);
    KT_CHECK(NpcDiversity::FamilyOf(185) == SlotFamily::F);
    KT_CHECK(NpcDiversity::FamilyOf(614) == SlotFamily::F);
    KT_CHECK(NpcDiversity::FamilyOf(811) == SlotFamily::F);
    KT_CHECK(NpcDiversity::FamilyOf(802) == SlotFamily::FatM);
    KT_CHECK(NpcDiversity::FamilyOf(803) == SlotFamily::FatF);
    KT_CHECK(NpcDiversity::FamilyOf(804) == SlotFamily::None);
    KT_CHECK(NpcDiversity::FamilyOf(809) == SlotFamily::None);
    KT_CHECK(NpcDiversity::FamilyOf(810) == SlotFamily::None);
    KT_CHECK(NpcDiversity::FamilyOf(416) == SlotFamily::None);
    KT_CHECK(NpcDiversity::FamilyOf(0xFFFFu) == SlotFamily::None);

    // Over rows 0..815 a family exists exactly when TexVarEligible holds (57 rows in total).
    bool agree = true;
    long eligible = 0;
    for (uint32_t a = 0; a <= 815u; ++a) {
        const uint16_t row = static_cast<uint16_t>(a);
        const bool hasFamily = NpcDiversity::FamilyOf(row) != SlotFamily::None;
        if (hasFamily) ++eligible;
        if (hasFamily != NpcDiversity::TexVarEligible(row)) agree = false;
    }
    KT_CHECK(agree);
    KT_CHECK_EQ(eligible, 57L);

    // The allowlist accessor returns the generated arrays and their sizes; None has none.
    size_t n = 0;
    KT_CHECK(NpcDiversity::FamilySlots(SlotFamily::M, n) == NpcDiversity::kMaleSlots);
    KT_CHECK_EQ(static_cast<long>(n), 16L);
    KT_CHECK(NpcDiversity::FamilySlots(SlotFamily::FatF, n) == NpcDiversity::kFatFemaleSlots);
    KT_CHECK_EQ(static_cast<long>(n), 1L);
    KT_CHECK(NpcDiversity::FamilySlots(SlotFamily::None, n) == nullptr);
    KT_CHECK_EQ(static_cast<long>(n), 0L);
}

void testShouldVaryRate() {
    long varied = 0;
    for (uint32_t oid = 1; oid <= kDraws; ++oid) {
        if (NpcDiversity::ShouldVary(oid, 190)) ++varied;
    }
    KT_CHECK(ShareInRange("vary gate share, row 190", varied, kDraws, 33.0, 37.0));
    KT_CHECK(!NpcDiversity::ShouldVary(0u, 190));
}

void testShouldVaryAppendedRows() {
    bool allVary = true;
    for (uint16_t row = 802; row <= 811; ++row) {
        for (uint32_t oid = 1; oid <= 1000u; ++oid) {
            if (!NpcDiversity::ShouldVary(oid, row)) allVary = false;
        }
    }
    KT_CHECK(allVary);
    KT_CHECK(!NpcDiversity::ShouldVary(0u, 802));
}

void testFatRowsOnly() {
    bool fatMOk = true;
    bool fatFOk = true;
    for (uint32_t oid = 1; oid <= kDraws; ++oid) {
        if (NpcDiversity::PickTextureVar(oid, 802) != 34) fatMOk = false;
        if (NpcDiversity::PickTextureVar(oid, 803) != 35) fatFOk = false;
        // One slot each, so the change rule does not apply: the same value comes back.
        if (NpcDiversity::PickTextureVar(oid, 802, 34) != 34) fatMOk = false;
        if (NpcDiversity::PickTextureVar(oid, 803, 35) != 35) fatFOk = false;
    }
    KT_CHECK(fatMOk);
    KT_CHECK(fatFOk);
}

void testSlots6And9Appear() {
    long m6 = 0, m9 = 0, f6 = 0, f9 = 0;
    for (uint32_t oid = 1; oid <= kDraws; ++oid) {
        const uint8_t m = NpcDiversity::PickTextureVar(oid, 190);
        const uint8_t f = NpcDiversity::PickTextureVar(oid, 185);
        if (m == 6) ++m6;
        if (m == 9) ++m9;
        if (f == 6) ++f6;
        if (f == 9) ++f9;
    }
    KT_CHECK(m6 > 0 && m9 > 0);
    KT_CHECK(f6 > 0 && f9 > 0);
}

void testNeverEqualCurrent() {
    CheckNeverEqualCurrent("M row 190", 190, NpcDiversity::kMaleSlots, NpcDiversity::kMaleSlotsCount);
    CheckNeverEqualCurrent("M row 805", 805, NpcDiversity::kMaleSlots, NpcDiversity::kMaleSlotsCount);
    CheckNeverEqualCurrent("F row 185", 185, NpcDiversity::kFemaleSlots, NpcDiversity::kFemaleSlotsCount);
    CheckNeverEqualCurrent("F row 811", 811, NpcDiversity::kFemaleSlots, NpcDiversity::kFemaleSlotsCount);
}

} // namespace

int main() {
    testFmix32Golden();
    testMixGoldenAndIndependence();
    testHashDescriptorGolden();
    testHashSeedFallback();
    testShouldVaryGolden();
    testShouldVaryRate();
    testShouldVaryAppendedRows();
    testFamilySlotCoverage();
    testIsCommonerTexture();
    testShouldDiversifyTexture();
    testHashCreaturePosition();
    testQuantizeCoordGuards();
    testBandEdges();
    testGenderMismatchReturnsInput();
    testMaleCommonerDistribution();
    testFemaleCommonerDistribution();
    testTwilekMaleDistribution();
    testTwilekFemaleDistribution();
    testHashCreatureIdGolden();
    testTexVarEligible();
    testFamilyOfMatchesTexVarEligible();
    testPickTextureVarGateAndGolden();
    testPickTextureVarRangeAndDeterminism();
    testPickTextureVarUniform();
    testFatRowsOnly();
    testSlots6And9Appear();
    testNeverEqualCurrent();
    testDeterminism();
    return kt::Report("diversity_logic_test");
}
