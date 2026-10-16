// common_contract_test.cpp -- Common CONTRACT test for Master NPC Diversity Dispatcher
// Pins the engine layout constants (NpcDivLayout.h) and the Common row/exclusion APIs the mod relies on.
#include "DiversityLogic.h"
#include "DiversityRoles.h"
#include "DiversityRows.h"
#include "DiversitySaltSource.h"
#include "NpcWorld.h"
#include "NpcDivLayout.h"
#include "check.h"
#include <cstdint>
#include <cstddef>
#include <cstring>

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
    CEQ("CSWCObject +0x1C pointer offset (sibling object; NOT a tag)", L::kCreOffPtr1C, 0x1C);
    CEQ("CSWCObject +0x20 pointer offset (separate alloc; tag unresolved)", L::kCreOffPtr20, 0x20);
    CEQ("CSWCObject +0x5C candidate pointer offset (probe only)", L::kCreOffPtr5C, 0x5C);
    CEQ("CSWCObject +0x6C candidate pointer offset (probe only)", L::kCreOffPtr6C, 0x6C);
    CEQ("CSWCObject m_vPosition.x offset (VERIFIED raw CRE0 bytes)", L::kCreOffPosX, 0x24);
    CEQ("CSWCObject m_vPosition.y offset (VERIFIED raw CRE0 bytes)", L::kCreOffPosY, 0x28);
    CEQ("CSWCObject m_vPosition.z offset (VERIFIED raw CRE0 bytes)", L::kCreOffPosZ, 0x2C);
    CEQ("AppearanceDesc subType offset", L::kDescOffSubType, 0x00);
    CEQ("subType guard: max commoner subType", static_cast<size_t>(L::kDescSubTypeMaxCommoner), 2);
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

void testKeepPosTolerance() {
    // Keep-default position fallback tolerance (0.1 m units) that DiversityKeep.h relies on.
    CEQ("keep-default position tolerance is 3 (0.3 m)", NpcDiversity::roles::kKeepPosTol10, 3);
}

// NB1: the soft k2.npcworld client. The salt API this mod relies on, and the "no provider -> salt 0" guarantee.
void testNpcWorldClient() {
    CCHECK("interface name is k2.npcworld", std::strcmp(K2NW_IFACE, "k2.npcworld") == 0);
    CEQ("interface version is 1", K2NW_VERSION, 1);
    CCHECK("VisitSalt is a field of the v1 table, before SpawnCap", offsetof(K2NpcWorldApi, VisitSalt) < offsetof(K2NpcWorldApi, SpawnCap));
    k2nw::Client inert;   // never init()ed: no KPatchApi, no provider
    CCHECK("an inert client does not connect", !inert.connect());
    CEQ("an inert client reports salt 0", inert.visitSalt(), 0u);
    CEQ("visit_variety on without a provider gives salt 0", NpcDiversity::VisitSaltFrom(true, inert), 0u);
    CEQ("visit_variety off gives salt 0", NpcDiversity::VisitSaltFrom(false, inert), 0u);
}

} // namespace

int main() {
    testRowIndices();
    testDescriptorOffsets();
    testCreatureOffsets();
    testHookFrameOffsets();
    testExclusionContract();
    testKeepPosTolerance();
    testNpcWorldClient();
    return kt::Report("npc_diversity_common_contract_test");
}
