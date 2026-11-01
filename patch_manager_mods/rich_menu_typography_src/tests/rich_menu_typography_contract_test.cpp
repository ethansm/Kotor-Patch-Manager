// rich_menu_typography_contract_test.cpp -- Contract tests for Hook Site 1 and RichMenuTypography.cpp (Doc 53 RMT1).
#include "RichMenuTypography.h"
#include "RichTextClassifier.h"
#include "check.h"
#include "FakeKPatchApi.h"
#include "KpLog.h"

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
    uint8_t raw[0x80];

    MockStringInternal(const char* str, float r = 0.1020f, float g = 0.6980f, float b = 0.5490f) {
        memset(raw, 0, sizeof(raw));
        *reinterpret_cast<const char**>(raw + rmt::kStrOff_PszStr) = str;
        *reinterpret_cast<float*>(raw + rmt::kStrOff_ColorR) = r;
        *reinterpret_cast<float*>(raw + rmt::kStrOff_ColorG) = g;
        *reinterpret_cast<float*>(raw + rmt::kStrOff_ColorB) = b;
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

} // namespace

int main() {
    testHookBytesAndAdjacency();
    testStructOffsets();
    testNormalCreamText();
    testNegativePenalty();
    testNonCreamBaseColor();
    testNullSafety();
    testIniGate();
    testKpLogContract();
    return kt::Report("rich_menu_typography_contract_test");
}
