// diversity_logic_test.cpp -- Unit tests for DiversityLogic.h
#include "DiversityLogic.h"
#include "check.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

namespace {

using NpcDiversity::kCommonerSlots;
using NpcDiversity::kCommonerSlotCount;

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

void testComputeSlotGolden() {
    KT_CHECK_EQ(NpcDiversity::ComputeSlot(12345u), 9);  // depends on kCommonerSlots content
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
    uint32_t hash = 123456789;

    bool resM = NpcDiversity::ShouldDiversifyTexture("N_CommM01", hash, outTex, sizeof(outTex));
    KT_CHECK(resM);
    KT_CHECK(std::strncmp(outTex, "N_CommM", 7) == 0);
    KT_CHECK(std::strlen(outTex) == 9); // N_CommM + 2 digits

    bool resF = NpcDiversity::ShouldDiversifyTexture("n_commf01", hash, outTex, sizeof(outTex));
    KT_CHECK(resF);
    KT_CHECK(std::strncmp(outTex, "N_CommF", 7) == 0);
    KT_CHECK(std::strlen(outTex) == 9);

    bool resNonComm = NpcDiversity::ShouldDiversifyTexture("P_AttonB", hash, outTex, sizeof(outTex));
    KT_CHECK(!resNonComm);

    KT_CHECK(!NpcDiversity::ShouldDiversifyTexture(nullptr, hash, outTex, sizeof(outTex)));
    KT_CHECK(!NpcDiversity::ShouldDiversifyTexture("N_CommM01", hash, outTex, 15));

    // Every generated name is N_Comm{M,F}NN with NN drawn from kCommonerSlots
    bool namesOk = true;
    for (uint32_t i = 0; i < 1000; ++i) {
        bool ok = NpcDiversity::ShouldDiversifyTexture("N_CommF01", DrawHash(i, 185), outTex, sizeof(outTex));
        if (!ok || std::strlen(outTex) != 9 || std::strncmp(outTex, "N_CommF", 7) != 0) namesOk = false;
        int v = (outTex[7] - '0') * 10 + (outTex[8] - '0');
        bool member = false;
        for (size_t k = 0; k < kCommonerSlotCount; ++k) {
            if (kCommonerSlots[k] == v) member = true;
        }
        if (!member) namesOk = false;
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

void testComputeSlotCoverage() {
    constexpr uint32_t kSlotDraws = 10000;
    int counts[256] = {};
    for (uint32_t i = 0; i < kSlotDraws; ++i) {
        int s = NpcDiversity::ComputeSlot(DrawHash(i, 190));
        KT_CHECK(s >= 0 && s < 256);
        if (s >= 0 && s < 256) counts[s]++;
    }

    bool member[256] = {};
    for (size_t k = 0; k < kCommonerSlotCount; ++k) member[kCommonerSlots[k]] = true;

    bool noneOutside = true;
    int reached = 0;
    for (int v = 0; v < 256; ++v) {
        if (counts[v] > 0 && !member[v]) noneOutside = false;
        if (member[v] && counts[v] > 0) ++reached;
    }
    KT_CHECK(noneOutside);
    KT_CHECK_EQ(reached, static_cast<int>(kCommonerSlotCount));

    // Roughly uniform: expected 10000 / 35 = 285.7 per slot, accept 171..400
    bool uniform = true;
    for (size_t k = 0; k < kCommonerSlotCount; ++k) {
        int c = counts[kCommonerSlots[k]];
        if (c < 171 || c > 400) uniform = false;
    }
    KT_CHECK(uniform);
}

} // namespace

int main() {
    testFmix32Golden();
    testMixGoldenAndIndependence();
    testHashDescriptorGolden();
    testHashSeedFallback();
    testComputeSlotGolden();
    testComputeSlotCoverage();
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
    return kt::Report("diversity_logic_test");
}
