// NpcDiversityDispatcher.cpp -- Master NPC Diversity Dispatcher runtime detour hook
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "DiversityLogic.h"
#include "DiversityRows.h"

namespace {

constexpr size_t ResRefLength = 16;

struct ResRef {
    char value[ResRefLength];
};

#pragma pack(push, 1)
struct AppearanceDesc {
    char subType;           // +0x00
    uint8_t textureVar;     // +0x01
    uint8_t pad[0x12];      // +0x02 .. +0x13
    int32_t bodyClass;      // +0x14 (0 = None/Clothing, 1 = Light, 2 = Medium, 3 = Heavy)
    uint16_t appearance;    // +0x18 row index into appearance.2da
    char headOverride;      // +0x1A
    uint8_t gender;         // +0x1B (0 = Male, 1 = Female)
    uint8_t phenotype;      // +0x1C
};
#pragma pack(pop)

static_assert(sizeof(ResRef) == 16, "ResRef must be 16 bytes");
static_assert(offsetof(AppearanceDesc, textureVar) == 0x01, "textureVar offset mismatch");
static_assert(offsetof(AppearanceDesc, bodyClass) == 0x14, "bodyClass offset mismatch");
static_assert(offsetof(AppearanceDesc, appearance) == 0x18, "appearance offset mismatch");

} // namespace

extern "C" {

__declspec(dllexport) void NpcDiversity_Init() {
    // Initializer hook called by KPatch on DLL load
}

// Intercepts CSWCCreatureAppearance::GetBTypeModelAndTextureResRef after vanilla texture lookup
__declspec(dllexport) void __cdecl OverrideNpcTexture(void* resolverFrame) {
    if (!resolverFrame) return;

    auto** ppDesc = reinterpret_cast<const AppearanceDesc**>(static_cast<char*>(resolverFrame) + 0x08);
    auto** ppTexture = reinterpret_cast<ResRef**>(static_cast<char*>(resolverFrame) + 0x10);

    if (!ppDesc || !*ppDesc || !ppTexture || !*ppTexture) return;

    const AppearanceDesc* pDesc = *ppDesc;
    ResRef* pTexture = *ppTexture;

    // Only diversify civilian clothing (bodyClass 0), never armor
    if (pDesc->bodyClass != 0) return;

    // Safely copy existing texture name with null terminator
    char curTex[ResRefLength + 1] = {};
    std::memcpy(curTex, pTexture->value, ResRefLength);
    curTex[ResRefLength] = '\0';

    // Compute deterministic hash for this creature appearance invocation
    uint32_t hash = NpcDiversity::HashDescriptor(
        pDesc->appearance,
        pDesc->gender,
        pDesc->textureVar,
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(resolverFrame))
    );

    char newTex[32] = {};
    if (NpcDiversity::ShouldDiversifyTexture(curTex, hash, newTex, sizeof(newTex))) {
        // Zero out target buffer and copy diversified texture name
        std::memset(pTexture->value, 0, ResRefLength);
        size_t len = std::strlen(newTex);
        if (len > ResRefLength) len = ResRefLength;
        std::memcpy(pTexture->value, newTex, len);
    }
}

// Remap NPC appearance based on creature position and appearance descriptor
__declspec(dllexport) void __cdecl RemapNpcAppearance(void* pCreature, AppearanceDesc* pDesc) {
    if (!pCreature || !pDesc) return;
    // Only civilian clothing (bodyClass 0), never armor
    if (pDesc->bodyClass != 0) return;

    // Check creature tag against exclusion list (companions, story NPCs)
    const char* tagStr = *reinterpret_cast<const char* const*>(static_cast<const char*>(pCreature) + 0x18);
    uint32_t tagLen = *reinterpret_cast<const uint32_t*>(static_cast<const char*>(pCreature) + 0x1C);
    if (tagStr && tagLen > 0 && NpcDiversity::IsExcludedFromDiversification(tagStr)) {
        return;
    }

    // Read creature position and object ID from memory offsets
    float x = *(const float*)((const char*)pCreature + 0x24);
    float y = *(const float*)((const char*)pCreature + 0x28);
    uint32_t objectId = *(const uint32_t*)((const char*)pCreature + 0x04);

    uint32_t hash = NpcDiversity::HashCreaturePosition(x, y, objectId, pDesc->appearance);
    uint16_t newApp = NpcDiversity::RemapAppearance(pDesc->appearance, hash);
    if (newApp != pDesc->appearance) {
        pDesc->appearance = newApp;
    }
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    (void)lpvReserved;
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
    }
    return TRUE;
}
