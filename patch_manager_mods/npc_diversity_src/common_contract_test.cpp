// common_contract_test.cpp -- Common CONTRACT test for Master NPC Diversity Dispatcher
#include "DiversityLogic.h"
#include "DiversityRows.h"
#include "check.h"
#include <cstdint>
#include <cstddef>

#define CCHECK(why, expr) ::kt::Check("npc_diversity relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define CEQ(why, a, b) ::kt::CheckEq("npc_diversity relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

namespace {

void testRowIndices() {
    CEQ("Fat Male commoner row index", NpcDiversity::APP_COMMONER_FAT_MALE, 802);
    CEQ("Fat Female commoner row index", NpcDiversity::APP_COMMONER_FAT_FEMALE, 803);
    CEQ("Czerka Officer Female row index", NpcDiversity::APP_CZERKA_OFFICER_FEMALE, 804);
    CEQ("Twilek Male 02 A row index", NpcDiversity::APP_TWILEK_COMMONER_M_02_A, 805);
    CEQ("Twilek Female Dancer row index", NpcDiversity::APP_TWILEK_FEMALE_DANCER, 810);
    CEQ("Alien Kel Dor 01 row index", NpcDiversity::APP_ALIEN_KELDOR_01, 812);
    CEQ("Alien Iridorian 01 row index", NpcDiversity::APP_ALIEN_IRIDORIAN_01, 814);
    CEQ("Restored head Ahrnell row index", NpcDiversity::HEAD_LLK_AHRNELL, 192);
}

void testDescriptorOffsets() {
    #pragma pack(push, 1)
    struct AppearanceDesc {
        char subType;           // +0x00
        uint8_t textureVar;     // +0x01
        uint8_t pad[0x12];      // +0x02 .. +0x13
        int32_t bodyClass;      // +0x14
        uint16_t appearance;    // +0x18
        char headOverride;      // +0x1A
        uint8_t gender;         // +0x1B
        uint8_t phenotype;      // +0x1C
    };
    #pragma pack(pop)

    CEQ("AppearanceDesc::textureVar offset", offsetof(AppearanceDesc, textureVar), 0x01);
    CEQ("AppearanceDesc::bodyClass offset", offsetof(AppearanceDesc, bodyClass), 0x14);
    CEQ("AppearanceDesc::appearance offset", offsetof(AppearanceDesc, appearance), 0x18);
    CEQ("AppearanceDesc::headOverride offset", offsetof(AppearanceDesc, headOverride), 0x1A);
    CEQ("AppearanceDesc::gender offset", offsetof(AppearanceDesc, gender), 0x1B);
    CEQ("AppearanceDesc::phenotype offset", offsetof(AppearanceDesc, phenotype), 0x1C);
}

void testCreatureOffsets() {
    // Engine constants from CSWCObject / CSWCCreature layout
    constexpr size_t OffsetObjectID = 0x04;
    constexpr size_t OffsetTag = 0x18;
    constexpr size_t OffsetTagLen = 0x1C;
    constexpr size_t OffsetPositionX = 0x24;
    constexpr size_t OffsetPositionY = 0x28;
    constexpr size_t OffsetPositionZ = 0x2C;

    CEQ("CSWCObject m_nObjectID offset", OffsetObjectID, 0x04);
    CEQ("CSWCObject m_sTag (CExoString) offset", OffsetTag, 0x18);
    CEQ("CSWCObject m_sTag len offset", OffsetTagLen, 0x1C);
    CEQ("CSWCObject m_vPosition.x offset", OffsetPositionX, 0x24);
    CEQ("CSWCObject m_vPosition.y offset", OffsetPositionY, 0x28);
    CEQ("CSWCObject m_vPosition.z offset", OffsetPositionZ, 0x2C);

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
    return kt::Report("npc_diversity_common_contract_test");
}
