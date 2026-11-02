#pragma once

#include <cstddef>
#include <cstdint>
#include <kpatch_api.h>

#ifndef __cdecl
#if defined(_MSC_VER) || defined(__MINGW32__)
#define __cdecl __cdecl
#elif defined(__i386__)
#define __cdecl __attribute__((cdecl))
#else
#define __cdecl
#endif
#endif

extern "C" {
void __cdecl RmtOverrideGlyphColor(uintptr_t ebp);
int __cdecl RmtAddControls2(void* listbox, void** pControls, int count,
                           int bAutoDelete, int bRestoreExtents, int bVariableItemHeights,
                           uintptr_t origFnAddr);
int __cdecl RmtSetLabelBonusPenaltyColour(void* pLabel, int delta);
void __cdecl RmtSetStats(void* panel);
void __cdecl RmtStyleInitControl(void* pControl, const char* tagStr);
void __cdecl KPatchInit(const KPatchApi* api);
}

namespace rmt {

enum class MenuLabelRole : uint8_t {
    None         = 0, // Untouched (bars, portraits, 3D views, backgrounds, buttons)
    TitleOrValue = 1, // Panel titles & primary stat/numeric values -> kCream
    CaptionKey   = 2, // Static section captions, column headers, stat keys -> kTealDim
};

// CSWGuiInGameCharacter embedded CSWGuiLabel offsets (each label size = 0x148)
// Primary Title & Non-Delta Stat Values (styled kCream in RmtSetStats)
constexpr size_t kCharOff_LblForceMastery   = 0x0068;
constexpr size_t kCharOff_LblExperienceStat = 0x0AA8;
constexpr size_t kCharOff_LblNeededXp       = 0x0D38;
constexpr size_t kCharOff_LblDefenseStat    = 0x0E80;
constexpr size_t kCharOff_LblTitle          = 0x48D8;

// Delta-Aware Stat & Modifier Values (styled via RmtSetLabelBonusPenaltyColour)
constexpr size_t kCharOff_LblFortitudeStat  = 0x0588;
constexpr size_t kCharOff_LblReflexStat     = 0x06D0;
constexpr size_t kCharOff_LblWillStat       = 0x0818;
constexpr size_t kCharOff_LblForceStat      = 0x0FC8;
constexpr size_t kCharOff_LblVitalityStat   = 0x1110;
constexpr size_t kCharOff_LblStr            = 0x1630;
constexpr size_t kCharOff_LblStrMod         = 0x18C0;
constexpr size_t kCharOff_LblWis            = 0x1A08;
constexpr size_t kCharOff_LblWisMod         = 0x1C98;
constexpr size_t kCharOff_LblCha            = 0x1DE0;
constexpr size_t kCharOff_LblChaMod         = 0x2070;
constexpr size_t kCharOff_LblInt            = 0x21B8;
constexpr size_t kCharOff_LblIntMod         = 0x2448;
constexpr size_t kCharOff_LblCon            = 0x2590;
constexpr size_t kCharOff_LblConMod         = 0x2820;
constexpr size_t kCharOff_LblDex            = 0x2968;
constexpr size_t kCharOff_LblDexMod         = 0x2BF8;

// Static Caption / Key Labels (styled kTealDim in RmtSetStats)
constexpr size_t kCharOff_LblFortitude      = 0x01B0;
constexpr size_t kCharOff_LblReflex         = 0x02F8;
constexpr size_t kCharOff_LblWill           = 0x0440;
constexpr size_t kCharOff_LblExperience     = 0x0960;
constexpr size_t kCharOff_LblNextLevel      = 0x0BF0;
constexpr size_t kCharOff_LblDefense        = 0x1258;
constexpr size_t kCharOff_LblForce          = 0x13A0;
constexpr size_t kCharOff_LblVitality       = 0x14E8;
constexpr size_t kCharOff_LblStrength       = 0x1778;
constexpr size_t kCharOff_LblWisdom         = 0x1B50;
constexpr size_t kCharOff_LblCharisma       = 0x1F28;
constexpr size_t kCharOff_LblIntelligence   = 0x2300;
constexpr size_t kCharOff_LblConstitution   = 0x26D8;
constexpr size_t kCharOff_LblDexterity      = 0x2AB0;
constexpr size_t kCharOff_LblMore           = 0x2FE0;
constexpr size_t kCharOff_LblLight          = 0x3128;
constexpr size_t kCharOff_LblDark           = 0x3270;

MenuLabelRole classifyMenuLabelTag(const char* tag) noexcept;

void setInlineColoring(bool enabled) noexcept;
bool isInlineColoring() noexcept;
void setRichDescBox(bool enabled) noexcept;
bool isRichDescBox() noexcept;
void setStatHierarchy(bool enabled) noexcept;
bool isStatHierarchy() noexcept;
void setVerbose(bool verbose) noexcept;
bool isVerbose() noexcept;
void resetOverrideCount() noexcept;
uint32_t getOverrideCount() noexcept;
void resetDescPopulateCount() noexcept;
uint32_t getDescPopulateCount() noexcept;
void resetStatStyleCount() noexcept;
uint32_t getStatStyleCount() noexcept;

bool pageRegistered() noexcept;
void pagePollForTest() noexcept;
int32_t applyPageRowForTest(const char* id, int32_t requested) noexcept;
void pageOnEnterForTest() noexcept;
int32_t getPageRowValueForTest(const char* id) noexcept;
} // namespace rmt
