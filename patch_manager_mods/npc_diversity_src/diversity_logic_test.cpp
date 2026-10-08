// diversity_logic_test.cpp -- Unit tests for DiversityLogic.h
#include "DiversityLogic.h"
#include "check.h"
#include <cstdio>
#include <cstring>

namespace {

void testHashDeterminism() {
    uint32_t h1 = NpcDiversity::HashCreatureSpawn("201TEL", 12.5f, -34.8f, "citizen_m");
    uint32_t h2 = NpcDiversity::HashCreatureSpawn("201TEL", 12.5f, -34.8f, "citizen_m");
    KT_CHECK_EQ(h1, h2);

    uint32_t hDiffPos = NpcDiversity::HashCreatureSpawn("201TEL", 12.6f, -34.8f, "citizen_m");
    KT_CHECK(h1 != hDiffPos);

    uint32_t hDiffArea = NpcDiversity::HashCreatureSpawn("202TEL", 12.5f, -34.8f, "citizen_m");
    KT_CHECK(h1 != hDiffArea);

    uint32_t hDiffTag = NpcDiversity::HashCreatureSpawn("201TEL", 12.5f, -34.8f, "citizen_f");
    KT_CHECK(h1 != hDiffTag);
}

void testDescriptorHash() {
    uint32_t d1 = NpcDiversity::HashDescriptor(802, 0, 1, 0x1234);
    uint32_t d2 = NpcDiversity::HashDescriptor(802, 0, 1, 0x1234);
    KT_CHECK_EQ(d1, d2);

    uint32_t dDiffGender = NpcDiversity::HashDescriptor(802, 1, 1, 0x1234);
    KT_CHECK(d1 != dDiffGender);

    uint32_t dDiffApp = NpcDiversity::HashDescriptor(803, 0, 1, 0x1234);
    KT_CHECK(d1 != dDiffApp);
}

void testComputeSlot() {
    for (uint32_t seed = 0; seed < 1000; ++seed) {
        int slot = NpcDiversity::ComputeSlot(seed * 2654435761u, 1, 35);
        KT_CHECK(slot >= 1);
        KT_CHECK(slot <= 35);
    }
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

void testGenericAppearanceChecks() {
    KT_CHECK(NpcDiversity::IsGenericMaleCommoner(300));
    KT_CHECK(NpcDiversity::IsGenericMaleCommoner(310));
    KT_CHECK(NpcDiversity::IsGenericMaleCommoner(17));  // Commoner_Mal_Asian_01_A
    KT_CHECK(NpcDiversity::IsGenericMaleCommoner(190)); // Commoner_Mal_White_01_A
    KT_CHECK(NpcDiversity::IsGenericMaleCommoner(591)); // Commoner_Mal_Asian_01_B
    KT_CHECK(!NpcDiversity::IsGenericMaleCommoner(802));
    KT_CHECK(!NpcDiversity::IsGenericMaleCommoner(500));

    KT_CHECK(NpcDiversity::IsGenericFemaleCommoner(350));
    KT_CHECK(NpcDiversity::IsGenericFemaleCommoner(360));
    KT_CHECK(NpcDiversity::IsGenericFemaleCommoner(14));  // Commoner_Fem_Asian_01_A
    KT_CHECK(NpcDiversity::IsGenericFemaleCommoner(185)); // Commoner_Fem_White_01_A
    KT_CHECK(NpcDiversity::IsGenericFemaleCommoner(585)); // Commoner_Fem_Asian_01_B
    KT_CHECK(!NpcDiversity::IsGenericFemaleCommoner(803));
    KT_CHECK(!NpcDiversity::IsGenericFemaleCommoner(550));

    KT_CHECK(NpcDiversity::IsGenericTwilekMale(500));
    KT_CHECK(NpcDiversity::IsGenericTwilekMale(510));
    KT_CHECK(NpcDiversity::IsGenericTwilekMale(416)); // Twilek_Commoner_M_01_A
    KT_CHECK(!NpcDiversity::IsGenericTwilekMale(300));
    KT_CHECK(!NpcDiversity::IsGenericTwilekMale(805));

    KT_CHECK(NpcDiversity::IsGenericTwilekFemale(550));
    KT_CHECK(NpcDiversity::IsGenericTwilekFemale(560));
    KT_CHECK(NpcDiversity::IsGenericTwilekFemale(446)); // Twilek_Commoner_F_01_A
    KT_CHECK(!NpcDiversity::IsGenericTwilekFemale(350));
    KT_CHECK(!NpcDiversity::IsGenericTwilekFemale(810));
}

void testHashCreaturePosition() {
    uint32_t h1 = NpcDiversity::HashCreaturePosition(12.5f, -34.8f, 42, 300);
    uint32_t h2 = NpcDiversity::HashCreaturePosition(12.5f, -34.8f, 42, 300);
    KT_CHECK_EQ(h1, h2);

    uint32_t hDiffX = NpcDiversity::HashCreaturePosition(12.6f, -34.8f, 42, 300);
    KT_CHECK(h1 != hDiffX);

    uint32_t hDiffObj = NpcDiversity::HashCreaturePosition(12.5f, -34.8f, 99, 300);
    KT_CHECK(h1 != hDiffObj);

    uint32_t hDiffApp = NpcDiversity::HashCreaturePosition(12.5f, -34.8f, 42, 350);
    KT_CHECK(h1 != hDiffApp);
}

void testRemapAppearanceDistribution() {
    // Male commoner: 15% fat, 5% kel dor, 5% iridorian, 75% unchanged
    int maleFat = 0, maleKel = 0, maleIri = 0, maleUnchanged = 0;
    for (uint32_t h = 0; h < 1000; ++h) {
        uint16_t result = NpcDiversity::RemapAppearance(300, h);
        if (result == 802) maleFat++;
        else if (result == 812 || result == 813) maleKel++;
        else if (result == 814 || result == 815) maleIri++;
        else if (result == 300) maleUnchanged++;
    }
    KT_CHECK(maleFat > 0);
    KT_CHECK(maleKel > 0);
    KT_CHECK(maleIri > 0);
    KT_CHECK(maleUnchanged > 0);

    // Female commoner: 15% fat, 5% czerka, 80% unchanged
    int femaleFat = 0, femaleCzerka = 0, femaleUnchanged = 0;
    for (uint32_t h = 0; h < 1000; ++h) {
        uint16_t result = NpcDiversity::RemapAppearance(350, h);
        if (result == 803) femaleFat++;
        else if (result == 804) femaleCzerka++;
        else if (result == 350) femaleUnchanged++;
    }
    KT_CHECK(femaleFat > 0);
    KT_CHECK(femaleCzerka > 0);
    KT_CHECK(femaleUnchanged > 0);

    // Male Twi'lek: 4 variants
    int twiMaleA = 0, twiMaleB = 0, twiMaleC = 0, twiMaleD = 0;
    for (uint32_t h = 0; h < 1000; ++h) {
        uint16_t result = NpcDiversity::RemapAppearance(500, h);
        if (result == 805) twiMaleA++;
        else if (result == 806) twiMaleB++;
        else if (result == 807) twiMaleC++;
        else if (result == 808) twiMaleD++;
    }
    KT_CHECK(twiMaleA > 0);
    KT_CHECK(twiMaleB > 0);
    KT_CHECK(twiMaleC > 0);
    KT_CHECK(twiMaleD > 0);

    // Female Twi'lek: 20% dancer variants, 80% unchanged
    int twiFemDancer = 0, twiFemUnchanged = 0;
    for (uint32_t h = 0; h < 1000; ++h) {
        uint16_t result = NpcDiversity::RemapAppearance(550, h);
        if (result == 809 || result == 810 || result == 811) twiFemDancer++;
        else if (result == 550) twiFemUnchanged++;
    }
    KT_CHECK(twiFemDancer > 0);
    KT_CHECK(twiFemUnchanged > 0);

    // Non-generic appearance unchanged
    KT_CHECK_EQ(NpcDiversity::RemapAppearance(999, 12345), 999);
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
}

void testExclusionTagLookups() {
    // Test companion exclusion tags (case-insensitive)
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("kreia"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("KREIA"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("Kreia"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("atton"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("ATTON"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("bao_dur"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("BAO_DUR"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("mira"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("MIRA"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("hanharr"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("HANHARR"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("mandalore"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("MANDALORE"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("visas"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("VISAS"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("disciple"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("DISCIPLE"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("handmaiden"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("HANDMAIDEN"));

    // Test major story NPC exclusion tags (case-insensitive)
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("chodo"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("CHODO"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("luxa"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("LUXA"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("grenn"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("GRENN"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("opo_chano"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("OPO_CHANO"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("lorso"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("LORSO"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("b4d4"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("B4D4"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("ralon"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("RALON"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("moza"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("MOZA"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("vrook"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("VROOK"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("zreztik"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("ZREZTIK"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("geeda"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("GEEDA"));

    // Test additional aliases and companion variants
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("baodur"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("t3m4"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("t3_m4"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("hk47"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("g0t0"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("player"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("opochano"));

    // Test creature tag prefix stripping (n_, c_, npc_)
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("n_chodo"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("n_luxa"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("c_grenn"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("npc_atton"));
    KT_CHECK(NpcDiversity::IsExcludedFromDiversification("n_kreia"));

    // Test that generic commoners are NOT excluded
    KT_CHECK(!NpcDiversity::IsExcludedFromDiversification("citizen_m"));
    KT_CHECK(!NpcDiversity::IsExcludedFromDiversification("citizen_f"));
    KT_CHECK(!NpcDiversity::IsExcludedFromDiversification("citizen"));
    KT_CHECK(!NpcDiversity::IsExcludedFromDiversification("commoner"));
    KT_CHECK(!NpcDiversity::IsExcludedFromDiversification("n_commoner"));
    KT_CHECK(!NpcDiversity::IsExcludedFromDiversification("c_bystander"));
    KT_CHECK(!NpcDiversity::IsExcludedFromDiversification("civ"));

    // Test that non-excluded NPCs are not excluded
    KT_CHECK(!NpcDiversity::IsExcludedFromDiversification("random_npc"));
    KT_CHECK(!NpcDiversity::IsExcludedFromDiversification("test"));
    KT_CHECK(!NpcDiversity::IsExcludedFromDiversification(""));
    KT_CHECK(!NpcDiversity::IsExcludedFromDiversification(nullptr));
}

} // namespace

int main() {
    testHashDeterminism();
    testDescriptorHash();
    testComputeSlot();
    testIsCommonerTexture();
    testShouldDiversifyTexture();
    testGenericAppearanceChecks();
    testHashCreaturePosition();
    testRemapAppearanceDistribution();
    testExclusionTagLookups();
    return kt::Report("diversity_logic_test");
}