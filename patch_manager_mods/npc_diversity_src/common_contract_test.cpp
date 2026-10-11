// common_contract_test.cpp -- Common CONTRACT test for Master NPC Diversity Dispatcher
// Pins the engine layout constants (NpcDivLayout.h) and the Common row/exclusion APIs the mod relies on.
#include "DiversityLogic.h"
#include "DiversityRows.h"
#include "NpcDivLayout.h"
#include "check.h"
#include <cstdint>
#include <cstddef>

#define CCHECK(why, expr) ::kt::Check("npc_diversity relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define CEQ(why, a, b) ::kt::CheckEq("npc_diversity relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

namespace {

namespace L = NpcDiversity::layout;

void testRowIndices() {
    CEQ("Fat Male commoner row index", NpcDiversity::APP_COMMONER_FAT_MALE, 802);
    CEQ("Fat Female commoner row index", NpcDiversity::APP_COMMONER_FAT_FEMALE, 803);
    CEQ("Czerka Officer Female row index", NpcDiversity::APP_CZERKA_OFFICER_FEMALE, 804);
    CEQ("Twilek Male 02 A row index", NpcDiversity::APP_TWILEK_COMMONER_M_02_A, 805);
    CEQ("Twilek Male 02 B row index", NpcDiversity::APP_TWILEK_COMMONER_M_02_B, 806);
    CEQ("Twilek Male 02 C row index", NpcDiversity::APP_TWILEK_COMMONER_M_02_C, 807);
    CEQ("Twilek Male 02 D row index", NpcDiversity::APP_TWILEK_COMMONER_M_02_D, 808);
    CEQ("Twilek Female Dancer Boot row index", NpcDiversity::APP_TWILEK_FEMALE_DANCER_BOOT, 809);
    CEQ("Twilek Female Dancer row index", NpcDiversity::APP_TWILEK_FEMALE_DANCER, 810);
    CEQ("Twilek Female Underwear row index", NpcDiversity::APP_TWILEK_FEMALE_UNDERWEAR, 811);
    CEQ("Restored head Ahrnell row index", NpcDiversity::HEAD_LLK_AHRNELL, 192);
}

void testDescriptorOffsets() {
    using AppearanceDesc = L::AppearanceDesc;
    CEQ("AppearanceDesc::textureVar offset", offsetof(AppearanceDesc, textureVar), L::kDescOffTextureVar);
    CEQ("AppearanceDesc::descMode14 offset", offsetof(AppearanceDesc, descMode14), L::kDescOffMode14);
    CEQ("AppearanceDesc::appearance offset", offsetof(AppearanceDesc, appearance), L::kDescOffAppearance);
    CEQ("AppearanceDesc::headOverride offset", offsetof(AppearanceDesc, headOverride), L::kDescOffHeadOverride);
    CEQ("AppearanceDesc::gender offset", offsetof(AppearanceDesc, gender), L::kDescOffGender);
    CEQ("AppearanceDesc::phenotype offset", offsetof(AppearanceDesc, phenotype), L::kDescOffPhenotype);
    CCHECK("AppearanceDesc covers +0x00..+0x1C", sizeof(AppearanceDesc) >= 0x1D);
}

void testCreatureOffsets() {
    CEQ("CSWCObject m_nObjectID offset (VERIFIED play-test)", L::kCreOffObjectId, 0x04);
    CEQ("CSWCObject +0x18 pointer offset (NOT a CExoString; tag unresolved)", L::kCreOffTagPtr, 0x18);
    CEQ("CSWCObject +0x1C pointer offset (NOT a CExoString; tag unresolved)", L::kCreOffTagLen, 0x1C);
    CEQ("CSWCObject m_vPosition.x offset (VERIFIED CRE0 dump)", L::kCreOffPosX, 0x20);
    CEQ("CSWCObject m_vPosition.y offset (VERIFIED CRE0 dump)", L::kCreOffPosY, 0x24);
    CEQ("CSWCObject m_vPosition.z offset (VERIFIED CRE0 dump)", L::kCreOffPosZ, 0x28);
    CEQ("CSWCCreature pending appearance counter offset (UNVERIFIED)", L::kCreOffAppCounter, 0x328);
}

void testHookFrameOffsets() {
    CEQ("hook 1 pDesc arg [EBP+0x08] (VERIFIED)", L::kFrameOffPDesc, 0x08);
    CEQ("hook 1 out texture arg [EBP+0x10] (VERIFIED)", L::kFrameOffOutTexture, 0x10);
    CEQ("engine resref buffer size is 16 bytes", sizeof(L::ResRef), 16);
}

void testExclusionContract() {
    // Verify critical exclusion behavior contract
    CCHECK("Kreia companion tag excluded", NpcDiversity::IsExcludedFromDiversification("kreia"));
    CCHECK("Atton companion tag excluded", NpcDiversity::IsExcludedFromDiversification("atton"));
    CCHECK("Chodo story NPC tag excluded", NpcDiversity::IsExcludedFromDiversification("chodo"));
    CCHECK("Generic commoner tag allowed", !NpcDiversity::IsExcludedFromDiversification("citizen_m"));
}

} // namespace

int main() {
    testRowIndices();
    testDescriptorOffsets();
    testCreatureOffsets();
    testHookFrameOffsets();
    testExclusionContract();
    return kt::Report("npc_diversity_common_contract_test");
}
