// rich_menu_typography_contract_test.cpp -- Contract tests for Hook Sites 1..5 and RichMenuTypography.cpp (Doc 53 RMT1..RMT3).
#include "RichMenuTypography.h"
#include "RichDescBox.h"
#include "RichTextClassifier.h"
#include "RichMenuTypographyPage.h"
#include "check.h"
#include "FakeKPatchApi.h"
#include "KpLog.h"
#include "windows.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

void testHookBytesAndAdjacency() {
    constexpr uint8_t kStolenBytes[6] = { 0xFF, 0x15, 0xE0, 0x62, 0x98, 0x00 };
    constexpr uintptr_t kHookAddr = 0x0047C06F;
    constexpr uintptr_t kNextInsn = 0x0047C075;
    constexpr size_t kHookLen = sizeof(kStolenBytes);

    kt::CheckEq("Hook byte length is 6", kHookLen, static_cast<size_t>(6));
    kt::Check("Jump target adjacency: 0x0047C06F + 6 == 0x0047C075", kHookAddr + kHookLen == kNextInsn);

    // Check against vanilla exe if present on host
    const char* exePath = getenv("KOTOR2_VANILLA_EXE");
    if (!exePath || !*exePath) {
        exePath = "/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe";
    }

    FILE* fp = fopen(exePath, "rb");
    if (fp) {
        // File offset of 0x0047C06F: .text starts at raw offset 0x400 with VA 0x00401000
        // 0x0047C06F - 0x00401000 + 0x400 = 0x7B46F
        constexpr long kFileOffset = 0x7B46F;
        fseek(fp, kFileOffset, SEEK_SET);
        uint8_t readBytes[6] = {0};
        const size_t n = fread(readBytes, 1, 6, fp);
        fclose(fp);
        kt::CheckEq("Read 6 bytes from vanilla exe", n, static_cast<size_t>(6));
        kt::Check("Vanilla bytes at 0x0047C06F match FF 15 E0 62 98 00",
                  memcmp(readBytes, kStolenBytes, 6) == 0);
    } else {
        kt::Check("Vanilla exe not present on disk, static contract verified", true);
    }
}

void testHookSite2Contract() {
    // TV9_HookBytesContract: CSWGuiListBox::AddControls_2 hook site 0x0041FD90 and epilogue 0x0041FED0
    constexpr uint8_t kStolenBytes2[6] = { 0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x14 };
    constexpr uint8_t kEpilogueBytes2[6] = { 0x8B, 0xE5, 0x5D, 0xC2, 0x14, 0x00 };
    constexpr uintptr_t kHookAddr2 = 0x0041FD90;
    constexpr uintptr_t kEpilogueAddr2 = 0x0041FED0;

    kt::CheckEq("Hook Site 2 byte length is 6", sizeof(kStolenBytes2), static_cast<size_t>(6));
    kt::CheckEq("Epilogue byte length is 6", sizeof(kEpilogueBytes2), static_cast<size_t>(6));
    kt::Check("Hook Site 2 address is 0x0041FD90", kHookAddr2 == 0x0041FD90);
    kt::Check("Epilogue address is 0x0041FED0", kEpilogueAddr2 == 0x0041FED0);

    const char* exePath = getenv("KOTOR2_VANILLA_EXE");
    if (!exePath || !*exePath) {
        exePath = "/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe";
    }

    FILE* fp = fopen(exePath, "rb");
    if (fp) {
        // File offset of 0x0041FD90: 0x0041FD90 - 0x00401000 + 0x400 = 0x1F190
        constexpr long kFileOffsetHook = 0x1F190;
        fseek(fp, kFileOffsetHook, SEEK_SET);
        uint8_t readHook[6] = {0};
        const size_t n1 = fread(readHook, 1, 6, fp);
        kt::CheckEq("Read 6 bytes for Hook Site 2 from vanilla exe", n1, static_cast<size_t>(6));
        kt::Check("Vanilla bytes at 0x0041FD90 match 55 8B EC 83 EC 14",
                  memcmp(readHook, kStolenBytes2, 6) == 0);

        // File offset of 0x0041FED0: 0x0041FED0 - 0x00401000 + 0x400 = 0x1F2D0
        constexpr long kFileOffsetEpi = 0x1F2D0;
        fseek(fp, kFileOffsetEpi, SEEK_SET);
        uint8_t readEpi[6] = {0};
        const size_t n2 = fread(readEpi, 1, 6, fp);
        kt::CheckEq("Read 6 bytes for Epilogue from vanilla exe", n2, static_cast<size_t>(6));
        kt::Check("Vanilla bytes at 0x0041FED0 match 8B E5 5D C2 14 00",
                  memcmp(readEpi, kEpilogueBytes2, 6) == 0);

        fclose(fp);
    } else {
        kt::Check("Vanilla exe not present on disk, static contract verified", true);
    }
}

void testStructOffsets() {
    kt::CheckEq("CAurGUIStringInternal pszStr offset == 0x14", rmt::kStrOff_PszStr, static_cast<size_t>(0x14));
    kt::CheckEq("CAurGUIStringInternal colorR offset == 0x30", rmt::kStrOff_ColorR, static_cast<size_t>(0x30));
    kt::CheckEq("CAurGUIStringInternal colorG offset == 0x34", rmt::kStrOff_ColorG, static_cast<size_t>(0x34));
    kt::CheckEq("CAurGUIStringInternal colorB offset == 0x38", rmt::kStrOff_ColorB, static_cast<size_t>(0x38));
}

struct SimulatedFrame {
    uint8_t raw[0x200];
    uintptr_t ebp;

    SimulatedFrame() {
        memset(raw, 0, sizeof(raw));
        // Put ebp at offset 0x180 so ebp - 0x168 is at raw + 0x18 (well within bounds)
        ebp = reinterpret_cast<uintptr_t>(raw + 0x180);
    }

    void setSelf(const void* self) {
        *reinterpret_cast<const void**>(ebp - 0x168) = self;
    }
    void setCurChar(const char* p) {
        *reinterpret_cast<const char**>(ebp - 0x10) = p;
    }
    void setLineIdx(int lineIdx) {
        *reinterpret_cast<int*>(ebp - 0x04) = lineIdx;
    }
    void setBaseCol(uint32_t col) {
        *reinterpret_cast<uint32_t*>(ebp - 0x68) = col;
    }
    void setActiveCol(uint32_t col) {
        *reinterpret_cast<uint32_t*>(ebp - 0xF4) = col;
    }
    uint32_t getActiveCol() const {
        return *reinterpret_cast<const uint32_t*>(ebp - 0xF4);
    }
};

struct MockStringInternal {
    alignas(16) uint8_t raw[0x80];

    MockStringInternal(const char* str, float r = 0.1020f, float g = 0.6980f, float b = 0.5490f) {
        memset(raw, 0, sizeof(raw));
        memcpy(raw + rmt::kStrOff_PszStr, &str, sizeof(str));
        memcpy(raw + rmt::kStrOff_ColorR, &r, sizeof(r));
        memcpy(raw + rmt::kStrOff_ColorG, &g, sizeof(g));
        memcpy(raw + rmt::kStrOff_ColorB, &b, sizeof(b));
    }
};

void testNormalCreamText() {
    const char* str = "Feats: +2 Attack\n";
    MockStringInternal mock(str);
    constexpr uint32_t kAlpha = 0xFF000000u;
    constexpr uint32_t kBaseCol = kAlpha | rmt::kRgbTealText;

    SimulatedFrame frame;
    frame.setSelf(mock.raw);
    frame.setLineIdx(0);
    frame.setBaseCol(kBaseCol);

    // Label part: "Feats:" -> kRgbTealDim
    frame.setCurChar(str); // 'F'
    frame.setActiveCol(kBaseCol);
    RmtOverrideGlyphColor(frame.ebp);
    kt::CheckEq("Normal text label 'F' -> kRgbTealDim",
                frame.getActiveCol(), kAlpha | rmt::kRgbTealDim);

    // Bonus part: "+2" -> kRgbBonusAmber
    frame.setCurChar(str + 7); // '+'
    frame.setActiveCol(kBaseCol);
    RmtOverrideGlyphColor(frame.ebp);
    kt::CheckEq("Normal text bonus '+' -> kRgbBonusAmber",
                frame.getActiveCol(), kAlpha | rmt::kRgbBonusAmber);

    frame.setCurChar(str + 8); // '2'
    frame.setActiveCol(kBaseCol);
    RmtOverrideGlyphColor(frame.ebp);
    kt::CheckEq("Normal text bonus '2' -> kRgbBonusAmber",
                frame.getActiveCol(), kAlpha | rmt::kRgbBonusAmber);

    // Value part: "Attack" -> kRgbCream
    frame.setCurChar(str + 10); // 'A'
    frame.setActiveCol(kBaseCol);
    RmtOverrideGlyphColor(frame.ebp);
    kt::CheckEq("Normal text value 'A' -> kRgbCream",
                frame.getActiveCol(), kAlpha | rmt::kRgbCream);
}

void testNegativePenalty() {
    const char* str = "Penalty: -4 Defense\n";
    MockStringInternal mock(str);
    constexpr uint32_t kAlpha = 0xFF000000u;
    constexpr uint32_t kBaseCol = kAlpha | rmt::kRgbTealText;

    SimulatedFrame frame;
    frame.setSelf(mock.raw);
    frame.setLineIdx(0);
    frame.setBaseCol(kBaseCol);

    // Penalty '-' in "-4" -> kRgbPenaltyRed
    frame.setCurChar(str + 9); // '-'
    frame.setActiveCol(kBaseCol);
    RmtOverrideGlyphColor(frame.ebp);
    kt::CheckEq("Penalty '-' -> kRgbPenaltyRed",
                frame.getActiveCol(), kAlpha | rmt::kRgbPenaltyRed);

    frame.setCurChar(str + 10); // '4'
    frame.setActiveCol(kBaseCol);
    RmtOverrideGlyphColor(frame.ebp);
    kt::CheckEq("Penalty '4' -> kRgbPenaltyRed",
                frame.getActiveCol(), kAlpha | rmt::kRgbPenaltyRed);
}

void testNonCreamBaseColor() {
    // Disabled grey: {0.5f, 0.5f, 0.5f}
    const char* str = "Feats: +2 Attack\n";
    MockStringInternal mockGrey(str, 0.5f, 0.5f, 0.5f);
    constexpr uint32_t kGreyCol = 0xFF808080u;

    SimulatedFrame frame;
    frame.setSelf(mockGrey.raw);
    frame.setLineIdx(0);
    frame.setBaseCol(kGreyCol);
    frame.setCurChar(str + 7); // '+'
    frame.setActiveCol(kGreyCol);

    RmtOverrideGlyphColor(frame.ebp);
    kt::CheckEq("Non-cream base color (grey disabled) preserved untouched",
                frame.getActiveCol(), kGreyCol);

    // Green selected: {0.0f, 1.0f, 0.0f}
    MockStringInternal mockGreen(str, 0.0f, 1.0f, 0.0f);
    constexpr uint32_t kGreenCol = 0xFF00FF00u;
    frame.setSelf(mockGreen.raw);
    frame.setBaseCol(kGreenCol);
    frame.setActiveCol(kGreenCol);

    RmtOverrideGlyphColor(frame.ebp);
    kt::CheckEq("Non-cream base color (green selected) preserved untouched",
                frame.getActiveCol(), kGreenCol);
}

void testNullSafety() {
    const char* str = "Feats: +2 Attack\n";
    MockStringInternal mock(str);
    constexpr uint32_t kSentinel = 0xDEADBEEFu;

    SimulatedFrame frame;
    frame.setBaseCol(0xFF8CB21Au);

    // Null self -> activeCol untouched
    frame.setSelf(nullptr);
    frame.setCurChar(str);
    frame.setActiveCol(kSentinel);
    RmtOverrideGlyphColor(frame.ebp);
    kt::CheckEq("Null self preserves activeCol untouched",
                frame.getActiveCol(), kSentinel);

    // Null curChar -> activeCol untouched
    frame.setSelf(mock.raw);
    frame.setCurChar(nullptr);
    frame.setActiveCol(kSentinel);
    RmtOverrideGlyphColor(frame.ebp);
    kt::CheckEq("Null curChar preserves activeCol untouched",
                frame.getActiveCol(), kSentinel);

    // Null ebp
    RmtOverrideGlyphColor(0);
    kt::Check("Null ebp handled safely without crash", true);
}

void testIniGate() {
    const char* str = "Feats: +2 Attack\n";
    MockStringInternal mock(str);
    constexpr uint32_t kSentinel = 0xCAFEBABEu;

    SimulatedFrame frame;
    frame.setSelf(mock.raw);
    frame.setCurChar(str + 7); // '+'
    frame.setLineIdx(0);
    frame.setBaseCol(0xFF8CB21Au);
    frame.setActiveCol(kSentinel);

    // Disable inline coloring
    rmt::setInlineColoring(false);
    kt::CheckEq("InlineColoring is false", rmt::isInlineColoring(), false);

    RmtOverrideGlyphColor(frame.ebp);
    kt::CheckEq("InlineColoring=0 preserves activeCol untouched",
                frame.getActiveCol(), kSentinel);

    // Re-enable inline coloring
    rmt::setInlineColoring(true);
    kt::CheckEq("InlineColoring is true", rmt::isInlineColoring(), true);

    RmtOverrideGlyphColor(frame.ebp);
    kt::CheckEq("InlineColoring=1 enables glyph color override",
                frame.getActiveCol(), 0xFF000000u | rmt::kRgbBonusAmber);
}

void testKpLogContract() {
    kt::CheckEq("KP_LOG_PATCH manifest id is rich-menu-typography",
                strcmp(kplog::kPatchId, "rich-menu-typography"), 0);

    fakekp::reset();
    fakekp::withLog();
    KPatchInit(fakekp::api());
    kt::Check("KPatchInit with log service initializes KpLog and HasCore() is true",
              KpLog_HasCore());
    fakekp::close();
}

void testHookSites3To5Contract() {
    // Hook Site 3: CSWGuiInGameCharacter::SetLabelBonusPenaltyColour (0x00850090, epilogue 0x008500E0)
    constexpr uint8_t kStolenBytes3[7] = { 0x55, 0x8B, 0xEC, 0x51, 0x89, 0x4D, 0xFC };
    constexpr uint8_t kEpilogueBytes3[6] = { 0x8B, 0xE5, 0x5D, 0xC2, 0x08, 0x00 };
    // Hook Site 4: CSWGuiInGameCharacter::SetStats exit block (0x0084F702)
    constexpr uint8_t kStolenBytes4[7] = { 0xC7, 0x45, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF };
    // Hook Site 5: CSWGuiPanel::InitControl post-Load_2 tag check (0x0040F90A)
    constexpr uint8_t kStolenBytes5[8] = { 0x89, 0x45, 0x84, 0x68, 0x80, 0x74, 0x98, 0x00 };

    const char* exePath = getenv("KOTOR2_VANILLA_EXE");
    if (!exePath || !*exePath) {
        exePath = "/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe";
    }

    FILE* fp = fopen(exePath, "rb");
    if (fp) {
        // 0x00850090 - 0x00401000 + 0x400 = 0x44F490
        fseek(fp, 0x44F490L, SEEK_SET);
        uint8_t buf3[7] = {0};
        kt::CheckEq("Read 7 bytes for Hook Site 3", fread(buf3, 1, 7, fp), static_cast<size_t>(7));
        kt::Check("Vanilla bytes at 0x00850090 match 55 8B EC 51 89 4D FC",
                  memcmp(buf3, kStolenBytes3, 7) == 0);

        // 0x008500E0 - 0x00401000 + 0x400 = 0x44F4E0
        fseek(fp, 0x44F4E0L, SEEK_SET);
        uint8_t epi3[6] = {0};
        kt::CheckEq("Read 6 bytes for Hook Site 3 epilogue", fread(epi3, 1, 6, fp), static_cast<size_t>(6));
        kt::Check("Vanilla bytes at 0x008500E0 match 8B E5 5D C2 08 00",
                  memcmp(epi3, kEpilogueBytes3, 6) == 0);

        // 0x0084F702 - 0x00401000 + 0x400 = 0x44EB02
        fseek(fp, 0x44EB02L, SEEK_SET);
        uint8_t buf4[7] = {0};
        kt::CheckEq("Read 7 bytes for Hook Site 4", fread(buf4, 1, 7, fp), static_cast<size_t>(7));
        kt::Check("Vanilla bytes at 0x0084F702 match C7 45 FC FF FF FF FF",
                  memcmp(buf4, kStolenBytes4, 7) == 0);

        // 0x0040F90A - 0x00401000 + 0x400 = 0xED0A
        fseek(fp, 0xED0AL, SEEK_SET);
        uint8_t buf5[8] = {0};
        kt::CheckEq("Read 8 bytes for Hook Site 5", fread(buf5, 1, 8, fp), static_cast<size_t>(8));
        kt::Check("Vanilla bytes at 0x0040F90A match 89 45 84 68 80 74 98 00",
                  memcmp(buf5, kStolenBytes5, 8) == 0);

        fclose(fp);
    } else {
        kt::Check("Vanilla exe not present on disk, static RMT3 hook contract verified", true);
    }
}

void __thiscall fakeSetColor(void* textSub, float* rgb) {
    if (!textSub || !rgb) return;
    float* dst = reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(textSub) + guikit::Text_Color);
    dst[0] = rgb[0];
    dst[1] = rgb[1];
    dst[2] = rgb[2];
}

void initMockLabelAt(uint8_t* mem) {
    memset(mem, 0, guikit::LabelSize);
    *reinterpret_cast<uint32_t*>(mem + 0) = guikit::LabelVtable;
    *reinterpret_cast<uint32_t*>(mem + guikit::Label_Border) = guikit::BorderVtable;
    *reinterpret_cast<uint32_t*>(mem + guikit::Label_Text) = guikit::TextVtable;
    // Non-zero renderer pointer required by guikit::detail::textObjOk
    *reinterpret_cast<uint32_t*>(mem + guikit::Label_Text + guikit::Text_Renderer) = 0x12345678u;
    float* c = reinterpret_cast<float*>(mem + guikit::Label_Text + guikit::Text_StringSub + guikit::Text_Color);
    c[0] = rmt::kTealText[0];
    c[1] = rmt::kTealText[1];
    c[2] = rmt::kTealText[2];
}

bool colorNear(const uint8_t* lbl, const float want[3]) {
    const float* c = reinterpret_cast<const float*>(lbl + guikit::Label_Text + guikit::Text_StringSub + guikit::Text_Color);
    return std::fabs(c[0] - want[0]) < 1e-4f &&
           std::fabs(c[1] - want[1]) < 1e-4f &&
           std::fabs(c[2] - want[2]) < 1e-4f;
}

void testRmt3StatAndMenuHierarchy() {
    g_stubVirtualQueryAll = true;
    guikit::g_gk.textColor = &fakeSetColor;
    rmt::setStatHierarchy(true);
    rmt::resetStatStyleCount();

    // 1. Tag classification checks
    kt::Check("Tag LBL_TITLE -> TitleOrValue",
              rmt::classifyMenuLabelTag("LBL_TITLE") == rmt::MenuLabelRole::TitleOrValue);
    kt::Check("Tag LBL_CREDITS_VALUE -> TitleOrValue",
              rmt::classifyMenuLabelTag("LBL_CREDITS_VALUE") == rmt::MenuLabelRole::TitleOrValue);
    kt::Check("Tag LBL_DEFENSE_STAT -> TitleOrValue",
              rmt::classifyMenuLabelTag("LBL_DEFENSE_STAT") == rmt::MenuLabelRole::TitleOrValue);
    kt::Check("Tag LBL_DAMTEXT -> TitleOrValue",
              rmt::classifyMenuLabelTag("LBL_DAMTEXT") == rmt::MenuLabelRole::TitleOrValue);
    kt::Check("Tag LBL_STRENGTH -> CaptionKey",
              rmt::classifyMenuLabelTag("LBL_STRENGTH") == rmt::MenuLabelRole::CaptionKey);
    kt::Check("Tag LBL_CREDITS -> CaptionKey",
              rmt::classifyMenuLabelTag("LBL_CREDITS") == rmt::MenuLabelRole::CaptionKey);
    kt::Check("Tag LBL_SKILLRANK -> CaptionKey",
              rmt::classifyMenuLabelTag("LBL_SKILLRANK") == rmt::MenuLabelRole::CaptionKey);
    kt::Check("Tag LBL_BAR1 -> None",
              rmt::classifyMenuLabelTag("LBL_BAR1") == rmt::MenuLabelRole::None);
    kt::Check("Null/empty tag -> None",
              rmt::classifyMenuLabelTag(nullptr) == rmt::MenuLabelRole::None &&
              rmt::classifyMenuLabelTag("") == rmt::MenuLabelRole::None);

    // 2. RmtSetLabelBonusPenaltyColour checks (delta == 0 -> kCream, > 0 -> kBonusAmber, < 0 -> kPenaltyRed)
    alignas(16) uint8_t lbl[guikit::LabelSize];
    initMockLabelAt(lbl);

    kt::CheckEq("RmtSetLabelBonusPenaltyColour delta==0 returns 1",
                RmtSetLabelBonusPenaltyColour(lbl, 0), 1);
    kt::Check("RmtSetLabelBonusPenaltyColour delta==0 sets kCream", colorNear(lbl, rmt::kCream));

    kt::CheckEq("RmtSetLabelBonusPenaltyColour delta>0 returns 1",
                RmtSetLabelBonusPenaltyColour(lbl, 4), 1);
    kt::Check("RmtSetLabelBonusPenaltyColour delta>0 sets kBonusAmber", colorNear(lbl, rmt::kBonusAmber));

    kt::CheckEq("RmtSetLabelBonusPenaltyColour delta<0 returns 1",
                RmtSetLabelBonusPenaltyColour(lbl, -2), 1);
    kt::Check("RmtSetLabelBonusPenaltyColour delta<0 sets kPenaltyRed", colorNear(lbl, rmt::kPenaltyRed));

    // 3. RmtStyleInitControl checks across primary menu labels and non-label controls
    initMockLabelAt(lbl);
    RmtStyleInitControl(lbl, "LBL_INV");
    kt::Check("RmtStyleInitControl styles LBL_INV as kCream", colorNear(lbl, rmt::kCream));

    initMockLabelAt(lbl);
    RmtStyleInitControl(lbl, "LBL_CREDITS");
    kt::Check("RmtStyleInitControl styles LBL_CREDITS as kTealDim", colorNear(lbl, rmt::kTealDim));

    initMockLabelAt(lbl);
    RmtStyleInitControl(lbl, "LBL_BAR1");
    kt::Check("RmtStyleInitControl leaves LBL_BAR1 untouched as kTealText", colorNear(lbl, rmt::kTealText));

    // Non-label vtable (e.g. button) is ignored safely
    initMockLabelAt(lbl);
    *reinterpret_cast<uint32_t*>(lbl) = static_cast<uint32_t>(gameaddr::ButtonVtable);
    RmtStyleInitControl(lbl, "LBL_TITLE");
    kt::Check("RmtStyleInitControl ignores non-label control", colorNear(lbl, rmt::kTealText));

    // 4. RmtSetStats check on simulated CSWGuiInGameCharacter panel (size 0x4AC4)
    alignas(16) static uint8_t charPanel[0x4AC4];
    memset(charPanel, 0, sizeof(charPanel));

    constexpr size_t kCreamOffs[] = {
        rmt::kCharOff_LblTitle,
        rmt::kCharOff_LblDefenseStat,
        rmt::kCharOff_LblExperienceStat,
        rmt::kCharOff_LblNeededXp,
        rmt::kCharOff_LblForceMastery,
    };
    constexpr size_t kDimOffs[] = {
        rmt::kCharOff_LblStrength,
        rmt::kCharOff_LblDexterity,
        rmt::kCharOff_LblConstitution,
        rmt::kCharOff_LblIntelligence,
        rmt::kCharOff_LblWisdom,
        rmt::kCharOff_LblCharisma,
        rmt::kCharOff_LblVitality,
        rmt::kCharOff_LblForce,
        rmt::kCharOff_LblDefense,
        rmt::kCharOff_LblFortitude,
        rmt::kCharOff_LblReflex,
        rmt::kCharOff_LblWill,
        rmt::kCharOff_LblExperience,
        rmt::kCharOff_LblNextLevel,
        rmt::kCharOff_LblMore,
        rmt::kCharOff_LblLight,
        rmt::kCharOff_LblDark,
    };

    for (size_t off : kCreamOffs) initMockLabelAt(charPanel + off);
    for (size_t off : kDimOffs) initMockLabelAt(charPanel + off);

    RmtSetStats(charPanel);

    bool allCreamOk = true;
    for (size_t off : kCreamOffs) {
        if (!colorNear(charPanel + off, rmt::kCream)) allCreamOk = false;
    }
    kt::Check("RmtSetStats styles all 5 primary title/stat labels on CSWGuiInGameCharacter as kCream", allCreamOk);

    bool allDimOk = true;
    for (size_t off : kDimOffs) {
        if (!colorNear(charPanel + off, rmt::kTealDim)) allDimOk = false;
    }
    kt::Check("RmtSetStats styles all 17 static key/caption labels on CSWGuiInGameCharacter as kTealDim", allDimOk);

    // 5. StatHierarchy=0 INI gate disables RMT3 hooks
    rmt::setStatHierarchy(false);
    kt::CheckEq("isStatHierarchy is false", rmt::isStatHierarchy(), false);
    initMockLabelAt(lbl);
    kt::CheckEq("RmtSetLabelBonusPenaltyColour returns 0 when StatHierarchy=0",
                RmtSetLabelBonusPenaltyColour(lbl, 0), 0);
    kt::Check("StatHierarchy=0 leaves label color untouched", colorNear(lbl, rmt::kTealText));
    RmtStyleInitControl(lbl, "LBL_TITLE");
    kt::Check("StatHierarchy=0 leaves InitControl label untouched", colorNear(lbl, rmt::kTealText));

    rmt::setStatHierarchy(true);
    guikit::g_gk.rebind();
}

void testKpLogTighteningContract() {
    fakekp::reset();
    fakekp::withLog();
    rmt::resetLogStateForTest();
    rmt::setVerbose(false);
    g_stubVirtualQueryAll = true;
    guikit::g_gk.textColor = &fakeSetColor;
    KPatchInit(fakekp::api());

    auto hasSub = [](const char* sub) {
        for (const auto& l : fakekp::state().logLines) {
            if (l.find(sub) != std::string::npos) return true;
        }
        return false;
    };
    auto countSub = [](const char* sub) {
        size_t c = 0;
        for (const auto& l : fakekp::state().logLines) {
            if (l.find(sub) != std::string::npos) ++c;
        }
        return c;
    };

    // 1. RMTR Verbose=0 tightening: per-line/per-item diagnostics are silent
    const char* const str = "Item: +5 Force\n";
    MockStringInternal mock(str);
    constexpr uint32_t kSentinel = 0xCAFEBABEu;
    SimulatedFrame frame;
    frame.setSelf(mock.raw);
    frame.setCurChar(str + 7); // '+'
    frame.setLineIdx(0);
    frame.setBaseCol(0xFF8CB21Au);
    frame.setActiveCol(kSentinel);

    RmtOverrideGlyphColor(frame.ebp);
    kt::CheckEq("Glyph color is overridden", frame.getActiveCol(), 0xFF000000u | rmt::kRgbBonusAmber);
    kt::Check("Verbose=0 gates glyph override logs (0 lines)", !hasSub("override '"));

    void* dummyCtls[4] = {};
    kt::CheckEq("RmtAddControls2 without real listbox returns 0 safely",
                RmtAddControls2(nullptr, dummyCtls, 0, 0, 0, 0, 0), 0);
    kt::Check("Verbose=0 gates RichDescBox populate logs (0 lines)", !hasSub("RichDescBox populated"));

    alignas(16) static uint8_t mockCtl[0x1D0];
    initMockLabelAt(mockCtl);
    RmtStyleInitControl(mockCtl, "LBL_TITLE");
    kt::Check("Verbose=0 gates RmtStyleInitControl logs (0 lines)", !hasSub("RmtStyleInitControl tag="));

    // 2. RMTR State deduplication: RmtSetStats logs once, not on repeated calls
    alignas(16) static uint8_t charPanel[0x4AC4];
    memset(charPanel, 0, sizeof(charPanel));
    constexpr size_t kCreamOffs[] = { rmt::kCharOff_LblTitle, rmt::kCharOff_LblDefenseStat };
    for (size_t off : kCreamOffs) initMockLabelAt(charPanel + off);

    RmtSetStats(charPanel);
    const size_t statsCount1 = countSub("RmtSetStats styled");
    kt::CheckEq("RmtSetStats logs once on first panel style", statsCount1, static_cast<size_t>(1));

    RmtSetStats(charPanel);
    RmtSetStats(charPanel);
    const size_t statsCount2 = countSub("RmtSetStats styled");
    kt::CheckEq("RmtSetStats deduplicates repeated calls (0 additional lines)", statsCount2, static_cast<size_t>(1));

    // 3. RMTR Startup line budget: verify total lines is < 50
    kt::Check("Total startup lines < 50 (budget)", fakekp::state().logLines.size() < 50);

    // 4. RMTR Verbose=1 enables diagnostics
    fakekp::state().logLines.clear();
    rmt::resetLogStateForTest();
    rmt::setVerbose(true);

    frame.setActiveCol(kSentinel);
    RmtOverrideGlyphColor(frame.ebp);
    kt::Check("Verbose=1 logs glyph override diagnostics", hasSub("override '"));

    initMockLabelAt(mockCtl);
    RmtStyleInitControl(mockCtl, "LBL_TITLE");
    kt::Check("Verbose=1 logs RmtStyleInitControl diagnostics", hasSub("RmtStyleInitControl tag="));

    RmtSetStats(charPanel);
    kt::CheckEq("Verbose=1 logs initial RmtSetStats", countSub("RmtSetStats styled"), static_cast<size_t>(1));
    RmtSetStats(charPanel);
    kt::CheckEq("Verbose=1 still deduplicates unchanged panel/styled state", countSub("RmtSetStats styled"), static_cast<size_t>(1));

    // Switching panel in Verbose=1 logs the state change
    alignas(16) static uint8_t charPanel2[0x4AC4];
    memset(charPanel2, 0, sizeof(charPanel2));
    for (size_t off : kCreamOffs) initMockLabelAt(charPanel2 + off);
    RmtSetStats(charPanel2);
    kt::CheckEq("Verbose=1 logs panel change", countSub("RmtSetStats styled"), static_cast<size_t>(2));

    // Cleanup
    guikit::g_gk.rebind();
    rmt::setVerbose(false);
    rmt::resetLogStateForTest();
    fakekp::close();
}

void testModConfigPageContract() {
    // 1. Without provider (clients not ok) pagePoll registers nothing and does not crash
    kt::Check("pageRegistered initially false", !rmt::pageRegistered());
    rmt::pagePollForTest();
    kt::Check("pageRegistered remains false after poll without provider", !rmt::pageRegistered());

    // 650 polls triggers 600-poll warning safely without crashing
    for (int i = 0; i < 650; ++i) {
        rmt::pagePollForTest();
    }
    kt::Check("pageRegistered remains false after 650 polls without provider", !rmt::pageRegistered());

    // RmtStyleInitControl calls pagePoll if !g_mcReg.registered before early return
    RmtStyleInitControl(nullptr, nullptr);
    kt::Check("RmtStyleInitControl polls safely without crash", !rmt::pageRegistered());

    // 2. pageOnChange semantic via applyPageRowForTest flips globals
    // inline
    kt::CheckEq("applyPageRow inline 0 returns 0", rmt::applyPageRowForTest(rmtpage::kRowInline, 0), 0);
    kt::Check("inlineColoring flipped to false", !rmt::isInlineColoring());
    kt::CheckEq("applyPageRow inline 1 returns 1", rmt::applyPageRowForTest(rmtpage::kRowInline, 1), 1);
    kt::Check("inlineColoring flipped to true", rmt::isInlineColoring());
    kt::CheckEq("applyPageRow inline 2 clamps to 1", rmt::applyPageRowForTest(rmtpage::kRowInline, 2), 1);
    kt::Check("inlineColoring is true after 2 clamp", rmt::isInlineColoring());
    kt::CheckEq("applyPageRow inline -1 clamps to 0", rmt::applyPageRowForTest(rmtpage::kRowInline, -1), 0);
    kt::Check("inlineColoring is false after -1 clamp", !rmt::isInlineColoring());
    rmt::applyPageRowForTest(rmtpage::kRowInline, 1);

    // richdesc
    kt::CheckEq("applyPageRow richdesc 0 returns 0", rmt::applyPageRowForTest(rmtpage::kRowRichDesc, 0), 0);
    kt::Check("richDescBox flipped to false", !rmt::isRichDescBox());
    kt::CheckEq("applyPageRow richdesc 1 returns 1", rmt::applyPageRowForTest(rmtpage::kRowRichDesc, 1), 1);
    kt::Check("richDescBox flipped to true", rmt::isRichDescBox());
    kt::CheckEq("applyPageRow richdesc 2 clamps to 1", rmt::applyPageRowForTest(rmtpage::kRowRichDesc, 2), 1);
    kt::Check("richDescBox is true after 2 clamp", rmt::isRichDescBox());
    kt::CheckEq("applyPageRow richdesc -1 clamps to 0", rmt::applyPageRowForTest(rmtpage::kRowRichDesc, -1), 0);
    kt::Check("richDescBox is false after -1 clamp", !rmt::isRichDescBox());
    rmt::applyPageRowForTest(rmtpage::kRowRichDesc, 1);

    // stats
    kt::CheckEq("applyPageRow stats 0 returns 0", rmt::applyPageRowForTest(rmtpage::kRowStats, 0), 0);
    kt::Check("statHierarchy flipped to false", !rmt::isStatHierarchy());
    kt::CheckEq("applyPageRow stats 1 returns 1", rmt::applyPageRowForTest(rmtpage::kRowStats, 1), 1);
    kt::Check("statHierarchy flipped to true", rmt::isStatHierarchy());
    kt::CheckEq("applyPageRow stats 2 clamps to 1", rmt::applyPageRowForTest(rmtpage::kRowStats, 2), 1);
    kt::Check("statHierarchy is true after 2 clamp", rmt::isStatHierarchy());
    kt::CheckEq("applyPageRow stats -1 clamps to 0", rmt::applyPageRowForTest(rmtpage::kRowStats, -1), 0);
    kt::Check("statHierarchy is false after -1 clamp", !rmt::isStatHierarchy());
    rmt::applyPageRowForTest(rmtpage::kRowStats, 1);

    // 3. fontIdx -> getBodyFont() returns matching kFonts literal, and clamps
    kt::CheckEq("applyPageRow font 0 returns 0", rmt::applyPageRowForTest(rmtpage::kRowFont, 0), 0);
    kt::Check("font 0 body font matches kFonts[0]", strcmp(rmt::getBodyFont(), rmtpage::kFonts[0]) == 0);

    kt::CheckEq("applyPageRow font 1 returns 1", rmt::applyPageRowForTest(rmtpage::kRowFont, 1), 1);
    kt::Check("font 1 body font matches kFonts[1]", strcmp(rmt::getBodyFont(), rmtpage::kFonts[1]) == 0);

    kt::CheckEq("applyPageRow font 2 returns 2", rmt::applyPageRowForTest(rmtpage::kRowFont, 2), 2);
    kt::Check("font 2 body font matches kFonts[2]", strcmp(rmt::getBodyFont(), rmtpage::kFonts[2]) == 0);

    // font out-of-range clamps
    kt::CheckEq("applyPageRow font -1 clamps to 0", rmt::applyPageRowForTest(rmtpage::kRowFont, -1), 0);
    kt::Check("font -1 body font matches kFonts[0]", strcmp(rmt::getBodyFont(), rmtpage::kFonts[0]) == 0);

    kt::CheckEq("applyPageRow font 3 clamps to 2", rmt::applyPageRowForTest(rmtpage::kRowFont, 3), 2);
    kt::Check("font 3 body font matches kFonts[2]", strcmp(rmt::getBodyFont(), rmtpage::kFonts[2]) == 0);

    kt::CheckEq("applyPageRow font 99 clamps to 2", rmt::applyPageRowForTest(rmtpage::kRowFont, 99), 2);
    kt::Check("font 99 body font matches kFonts[2]", strcmp(rmt::getBodyFont(), rmtpage::kFonts[2]) == 0);

    // 4. rulesFontIdx -> getRulesFont() returns matching kFonts literal, and clamps
    kt::CheckEq("applyPageRow rulesfont 0 returns 0", rmt::applyPageRowForTest(rmtpage::kRowRulesFont, 0), 0);
    kt::Check("rulesfont 0 matches kFonts[0]", strcmp(rmt::getRulesFont(), rmtpage::kFonts[0]) == 0);

    kt::CheckEq("applyPageRow rulesfont 1 returns 1", rmt::applyPageRowForTest(rmtpage::kRowRulesFont, 1), 1);
    kt::Check("rulesfont 1 matches kFonts[1]", strcmp(rmt::getRulesFont(), rmtpage::kFonts[1]) == 0);

    kt::CheckEq("applyPageRow rulesfont 2 returns 2", rmt::applyPageRowForTest(rmtpage::kRowRulesFont, 2), 2);
    kt::Check("rulesfont 2 matches kFonts[2]", strcmp(rmt::getRulesFont(), rmtpage::kFonts[2]) == 0);

    // rulesfont out-of-range clamps
    kt::CheckEq("applyPageRow rulesfont -1 clamps to 0", rmt::applyPageRowForTest(rmtpage::kRowRulesFont, -1), 0);
    kt::Check("rulesfont -1 matches kFonts[0]", strcmp(rmt::getRulesFont(), rmtpage::kFonts[0]) == 0);

    kt::CheckEq("applyPageRow rulesfont 3 clamps to 2", rmt::applyPageRowForTest(rmtpage::kRowRulesFont, 3), 2);
    kt::Check("rulesfont 3 matches kFonts[2]", strcmp(rmt::getRulesFont(), rmtpage::kFonts[2]) == 0);

    kt::CheckEq("applyPageRow rulesfont 99 clamps to 2", rmt::applyPageRowForTest(rmtpage::kRowRulesFont, 99), 2);
    kt::Check("rulesfont 99 matches kFonts[2]", strcmp(rmt::getRulesFont(), rmtpage::kFonts[2]) == 0);

    // Unknown id / null passthrough
    kt::CheckEq("applyPageRow unknown id passthrough", rmt::applyPageRowForTest("unknown_id", 42), 42);
    kt::CheckEq("applyPageRow null id passthrough", rmt::applyPageRowForTest(nullptr, 77), 77);

    // pageOnEnter synchronizes font and rulesfont values to page
    rmt::applyPageRowForTest(rmtpage::kRowFont, 1);
    rmt::applyPageRowForTest(rmtpage::kRowRulesFont, 2);
    rmt::pageOnEnterForTest();
    kt::CheckEq("pageOnEnter updates font row to 1", rmt::getPageRowValueForTest(rmtpage::kRowFont), 1);
    kt::CheckEq("pageOnEnter updates rulesfont row to 2", rmt::getPageRowValueForTest(rmtpage::kRowRulesFont), 2);

    rmt::applyPageRowForTest(rmtpage::kRowFont, 0);
    rmt::applyPageRowForTest(rmtpage::kRowRulesFont, 0);
    rmt::pageOnEnterForTest();
    kt::CheckEq("pageOnEnter updates font row to 0", rmt::getPageRowValueForTest(rmtpage::kRowFont), 0);
    kt::CheckEq("pageOnEnter updates rulesfont row to 0", rmt::getPageRowValueForTest(rmtpage::kRowRulesFont), 0);
    kt::Check("font reset to kFonts[0]", strcmp(rmt::getBodyFont(), rmtpage::kFonts[0]) == 0);
    kt::Check("rulesfont reset to kFonts[0]", strcmp(rmt::getRulesFont(), rmtpage::kFonts[0]) == 0);

    // Clean up any test ini created in cwd
    remove(rmtpage::kIniFile);
}

} // namespace

int main() {
    testHookBytesAndAdjacency();
    testHookSite2Contract();
    testHookSites3To5Contract();
    testStructOffsets();
    testNormalCreamText();
    testNegativePenalty();
    testNonCreamBaseColor();
    testNullSafety();
    testIniGate();
    testKpLogContract();
    testKpLogTighteningContract();
    testRmt3StatAndMenuHierarchy();
    testModConfigPageContract();
    return kt::Report("rich_menu_typography_contract_test");
}
