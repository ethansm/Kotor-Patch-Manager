// rich_desc_box_test.cpp -- Unit and contract tests for RichDescBox.h and RmtAddControls2 (Doc 53 RMT2).
#include "RichDescBox.h"
#include "RichMenuTypography.h"
#include "check.h"
#include "windows.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <sys/mman.h>

namespace {

struct AddControls2Record {
    void* listbox = nullptr;
    void* controls[rmt::kMaxPooledRows] = {nullptr};
    int count = 0;
    int bAutoDelete = -1;
    int bRestoreExtents = -1;
    int bVariableItemHeights = -1;
    int callCount = 0;

    void reset() {
        listbox = nullptr;
        memset(controls, 0, sizeof(controls));
        count = 0;
        bAutoDelete = -1;
        bRestoreExtents = -1;
        bVariableItemHeights = -1;
        callCount = 0;
    }
};

static AddControls2Record g_rec;

static void fakeOrigAddControls2(void* listbox, void** pControls, int count,
                                 int bAutoDelete, int bRestoreExtents, int bVariableItemHeights) {
    g_rec.listbox = listbox;
    g_rec.count = count;
    g_rec.bAutoDelete = bAutoDelete;
    g_rec.bRestoreExtents = bRestoreExtents;
    g_rec.bVariableItemHeights = bVariableItemHeights;
    ++g_rec.callCount;
    for (int i = 0; i < count && i < static_cast<int>(rmt::kMaxPooledRows); ++i) {
        g_rec.controls[i] = pControls[i];
    }
}

static int mockMeasureTextHeight(uintptr_t /*ctl*/, const char* text, int width, const char* font = nullptr) {
    if (!text || !*text) return 0;
    int lineHeight = 14;
    if (font) {
        if (strcmp(font, rmt::kFontSub) == 0 || strstr(font, "10x10") != nullptr) {
            lineHeight = 12;
        } else if (strcmp(font, rmt::kFontTitle) == 0 || (strstr(font, "16x16") != nullptr && strstr(font, "dialog") != nullptr)) {
            lineHeight = 16;
        } else if (strcmp(font, rmt::kFontDesc) == 0 || strstr(font, "fnt_d") != nullptr || strstr(font, "fnt_") != nullptr) {
            lineHeight = 14;
        }
    }
    const int charsPerLine = (width > 35) ? (width / 7) : 28;
    const int halfLine = lineHeight / 2;
    int total = 0;
    const char* p = text;
    while (*p) {
        const char* start = p;
        while (*p && *p != '\n') ++p;
        const int plen = static_cast<int>(p - start);
        if (plen == 0) total += halfLine;
        else total += ((plen + charsPerLine - 1) / charsPerLine) * lineHeight;
        if (*p == '\n') ++p;
    }
    if (total < lineHeight) total = lineHeight;
    return total;
}

struct MockListBox {
    alignas(16) uint8_t raw[0x300];

    MockListBox(uintptr_t parentPanel = 0x1000, int clientW = 260, uint8_t pad = 10) {
        memset(raw, 0, sizeof(raw));
        *reinterpret_cast<uintptr_t*>(raw + rmt::kCtlOff_ParentPanel) = parentPanel;
        *reinterpret_cast<int*>(raw + rmt::kListBoxOff_ClientW) = clientW;
        *reinterpret_cast<uint8_t*>(raw + rmt::kListBoxOff_Padding) = pad;
        *reinterpret_cast<uint16_t*>(raw + rmt::kListBoxOff_TopItem) = 5;
    }

    void* voidPtr() { return reinterpret_cast<void*>(raw); }
    uintptr_t ptr() { return reinterpret_cast<uintptr_t>(raw); }

    void setParentPanel(uintptr_t p) {
        *reinterpret_cast<uintptr_t*>(raw + rmt::kCtlOff_ParentPanel) = p;
    }

    void setExtent(int x, int y, int w, int h) {
        *reinterpret_cast<int*>(raw + guikit::Ctl_X) = x;
        *reinterpret_cast<int*>(raw + guikit::Ctl_Y) = y;
        *reinterpret_cast<int*>(raw + guikit::Ctl_W) = w;
        *reinterpret_cast<int*>(raw + guikit::Ctl_H) = h;
    }

    uint16_t getTopItem() const {
        return *reinterpret_cast<const uint16_t*>(raw + rmt::kListBoxOff_TopItem);
    }
};

struct MockLabel {
    alignas(16) uint8_t raw[guikit::LabelSize];

    MockLabel(const char* text, int x = 0, int y = 0, int w = 240, int h = 30) {
        memset(raw, 0, sizeof(raw));
        *reinterpret_cast<uint32_t*>(raw + 0) = guikit::LabelVtable;
        *reinterpret_cast<int*>(raw + guikit::Ctl_X) = x;
        *reinterpret_cast<int*>(raw + guikit::Ctl_Y) = y;
        *reinterpret_cast<int*>(raw + guikit::Ctl_W) = w;
        *reinterpret_cast<int*>(raw + guikit::Ctl_H) = h;
        *reinterpret_cast<uint32_t*>(raw + guikit::Label_Border) = guikit::BorderVtable;
        *reinterpret_cast<uint32_t*>(raw + guikit::Label_Text) = guikit::TextVtable;
        *reinterpret_cast<const char**>(raw + rmt::kLabelOff_TextParams) = text;
    }

    void* voidPtr() { return reinterpret_cast<void*>(raw); }
    uintptr_t ptr() { return reinterpret_cast<uintptr_t>(raw); }

    void setVtable(uint32_t vt) {
        *reinterpret_cast<uint32_t*>(raw + 0) = vt;
    }

    void setText(const char* text) {
        *reinterpret_cast<const char**>(raw + rmt::kLabelOff_TextParams) = text;
    }
};

alignas(16) static uint8_t g_testArena[64 * 1024];
static size_t g_arenaOffset = 0;

static void* testArenaAlloc(unsigned n) {
    n = (n + 15) & ~15u;
    if (g_arenaOffset + n > sizeof(g_testArena)) {
        return nullptr;
    }
    void* p = g_testArena + g_arenaOffset;
    g_arenaOffset += n;
    return p;
}

void setupEnvironment() {
    g_stubVirtualQueryAll = true;
    rmt::g_measureTextHeight = mockMeasureTextHeight;

    void* vp = mmap(reinterpret_cast<void*>(0x00987000), 0x10000,
                    PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (vp != MAP_FAILED) {
        *reinterpret_cast<uint32_t*>(guikit::LabelVtable + 4) = 0x1234;
    }

    guikit::g_gk.callSetRect = [](uint32_t /*fn*/, uintptr_t ctl, int* r) {
        *reinterpret_cast<int*>(ctl + guikit::Ctl_X) = r[0];
        *reinterpret_cast<int*>(ctl + guikit::Ctl_Y) = r[1];
        *reinterpret_cast<int*>(ctl + guikit::Ctl_W) = r[2];
        *reinterpret_cast<int*>(ctl + guikit::Ctl_H) = r[3];
    };

    guikit::g_gk.alloc = testArenaAlloc;
    guikit::g_gk.labelCtor = [](void*) {};
    guikit::g_gk.exoCtor = [](void* e, const char* s) -> void* {
        memset(e, 0, 16);
        if (s) strncpy(static_cast<char*>(e), s, 15);
        return e;
    };
    guikit::g_gk.exoDtor = [](void*) {};
    guikit::g_gk.textSet = [](void*, void*) {};
    guikit::g_gk.setFont = [](void*, const char*) {};
    guikit::g_gk.setAlign = [](void*, unsigned) {};
    guikit::g_gk.textColor = [](void*, float*) {};
    guikit::g_gk.loadImage = [](const char*) -> void* { return reinterpret_cast<void*>(0x5000); };
}

void testTV7_SectionSplitItem() {
    rmt::resetPool();
    g_rec.reset();
    rmt::setRichDescBox(true);

    MockListBox listbox(0x1000, 260, 10);
    const char* itemDesc = "Fully Upgradeable\n\nDamage: Energy, 4-16\nAttack Modifier: +3\n\n{30}This blaster pistol was common during the Mandalorian Wars.";
    MockLabel srcLabel(itemDesc, 0, 0, 240, 30);
    void* controls[1] = { srcLabel.voidPtr() };

    const int res = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0,
                                    reinterpret_cast<uintptr_t>(fakeOrigAddControls2));
    kt::CheckEq("TV7: RmtAddControls2 intercepts item description and returns 1", res, 1);
    kt::CheckEq("TV7: origAddControls2 invoked exactly once", g_rec.callCount, 1);
    kt::CheckEq("TV7: origAddControls2 bAutoDelete == 0", g_rec.bAutoDelete, 0);
    kt::CheckEq("TV7: origAddControls2 bVariableItemHeights == 1", g_rec.bVariableItemHeights, 1);
    kt::CheckEq("TV7: origAddControls2 bRestoreExtents == 0", g_rec.bRestoreExtents, 0);
    kt::CheckEq("TV7: origAddControls2 count == 3", g_rec.count, 3);
    kt::CheckEq("TV7: ListBox topItem reset to 0", static_cast<int>(listbox.getTopItem()), 0);

    const rmt::LastPopulateInfo& info = rmt::getLastPopulateInfo();
    kt::CheckEq("TV7: Snapshot rowCount is 3", info.rowCount, 3);

    // Row 0: Subtitle ("Fully Upgradeable", dialogfont10x10, kTealDim, align 9, height fh + 0 = 12 + 0 = 12)
    kt::CheckEq("TV7: Row 0 is not divider", info.rowIsDivider[0], false);
    kt::Check("TV7: Row 0 font is dialogfont10x10", strcmp(info.rowFonts[0], rmt::kFontSub) == 0);
    kt::CheckEq("TV7: Row 0 align is TopLeft (9)", info.rowAligns[0], rmt::kAlignTopLeft);
    kt::CheckEq("TV7: Row 0 height is fh + 0 = 12", info.rowHeights[0], 12);
    kt::Check("TV7: Row 0 text is Fully Upgradeable", strcmp(info.rowTexts[0], "Fully Upgradeable") == 0);
    kt::Check("TV7: Row 0 color is kTealDim", info.rowColors[0][0] == rmt::kTealDim[0]);

    // Row 1: Rules (Damage: Energy, 4-16\nAttack Modifier: +3, getRulesFont() fnt_d16x16, kTealText, align 9, height 2*14 + 2 = 30)
    kt::CheckEq("TV7: Row 1 is not divider", info.rowIsDivider[1], false);
    kt::Check("TV7: Row 1 font is rulesFont", strcmp(info.rowFonts[1], rmt::getRulesFont()) == 0);
    kt::CheckEq("TV7: Row 1 align is TopLeft (9)", info.rowAligns[1], rmt::kAlignTopLeft);
    kt::CheckEq("TV7: Row 1 height is fh + 2 = 30", info.rowHeights[1], 30);
    kt::Check("TV7: Row 1 color is kTealText", info.rowColors[1][0] == rmt::kTealText[0]);

    // Row 2: Body (lore prose with {tag} stripped, fnt_d16x16, kTealText, align 9, 2 lines * 14 + 2 = 30)
    kt::CheckEq("TV7: Row 2 is not divider", info.rowIsDivider[2], false);
    kt::Check("TV7: Row 2 font is fnt_d16x16", strcmp(info.rowFonts[2], rmt::kFontDesc) == 0);
    kt::CheckEq("TV7: Row 2 align is TopLeft (9)", info.rowAligns[2], rmt::kAlignTopLeft);
    kt::CheckEq("TV7: Row 2 height is fh + 2 = 30", info.rowHeights[2], 30);
    kt::Check("TV7: Row 2 text has {tag} stripped",
              strcmp(info.rowTexts[2], "This blaster pistol was common during the Mandalorian Wars.") == 0);
    kt::Check("TV7: Row 2 color is kTealText", info.rowColors[2][0] == rmt::kTealText[0]);
}

void testTV8_SectionSplitJournal() {
    rmt::resetPool();
    g_rec.reset();
    rmt::setRichDescBox(true);

    MockListBox listbox(0x1000, 260, 10);
    const char* journalDesc = "Nar Shaddaa:\nRefugee Sector troubles have escalated.";
    MockLabel srcLabel(journalDesc, 0, 0, 240, 30);
    void* controls[1] = { srcLabel.voidPtr() };

    const int res = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0,
                                    reinterpret_cast<uintptr_t>(fakeOrigAddControls2));
    kt::CheckEq("TV8: RmtAddControls2 intercepts journal description and returns 1", res, 1);
    kt::CheckEq("TV8: origAddControls2 count == 2", g_rec.count, 2);

    const rmt::LastPopulateInfo& info = rmt::getLastPopulateInfo();
    kt::CheckEq("TV8: Snapshot rowCount is 2", info.rowCount, 2);

    // Row 0: Title ("Nar Shaddaa", dialogfont16x16, kCream, align 9, height 16 + 0 = 16)
    kt::CheckEq("TV8: Row 0 is not divider", info.rowIsDivider[0], false);
    kt::Check("TV8: Row 0 font is dialogfont16x16", strcmp(info.rowFonts[0], rmt::kFontTitle) == 0);
    kt::CheckEq("TV8: Row 0 align is TopLeft (9)", info.rowAligns[0], rmt::kAlignTopLeft);
    kt::CheckEq("TV8: Row 0 height is fh + 0 = 16", info.rowHeights[0], 16);
    kt::Check("TV8: Row 0 text is Nar Shaddaa (trailing colon stripped)",
              strcmp(info.rowTexts[0], "Nar Shaddaa") == 0);
    kt::Check("TV8: Row 0 color is kCream", info.rowColors[0][0] == rmt::kCream[0]);

    // Row 1: Body (Refugee Sector troubles have escalated., fnt_d16x16, kTealText, align 9, 2 lines * 14 + 2 = 30)
    kt::CheckEq("TV8: Row 1 is not divider", info.rowIsDivider[1], false);
    kt::Check("TV8: Row 1 font is fnt_d16x16", strcmp(info.rowFonts[1], rmt::kFontDesc) == 0);
    kt::CheckEq("TV8: Row 1 align is TopLeft (9)", info.rowAligns[1], rmt::kAlignTopLeft);
    kt::CheckEq("TV8: Row 1 height is fh + 2 = 30", info.rowHeights[1], 30);
    kt::Check("TV8: Row 1 text is Refugee Sector troubles...",
              strcmp(info.rowTexts[1], "Refugee Sector troubles have escalated.") == 0);
    kt::Check("TV8: Row 1 color is kTealText", info.rowColors[1][0] == rmt::kTealText[0]);
}

void testInterceptionFilterGuards() {
    rmt::resetPool();
    g_rec.reset();
    rmt::setRichDescBox(true);

    MockListBox listbox(0x1000);
    MockLabel label("Damage: Energy, 4-16\n");
    void* controls[3] = { label.voidPtr(), label.voidPtr(), label.voidPtr() };
    const uintptr_t fn = reinterpret_cast<uintptr_t>(fakeOrigAddControls2);

    // 1. count != 1 (count = 3) -> NOT intercepted (returns 0)
    g_rec.reset();
    int r = RmtAddControls2(listbox.voidPtr(), controls, 3, 0, 0, 0, fn);
    kt::CheckEq("Guard: count == 3 returns 0", r, 0);
    kt::CheckEq("Guard: origAddControls2 was not called", g_rec.callCount, 0);

    // 2. bAutoDelete != 0 (bAutoDelete = 1) -> NOT intercepted (returns 0)
    g_rec.reset();
    r = RmtAddControls2(listbox.voidPtr(), controls, 1, 1, 0, 0, fn);
    kt::CheckEq("Guard: bAutoDelete == 1 returns 0", r, 0);
    kt::CheckEq("Guard: origAddControls2 was not called", g_rec.callCount, 0);

    // 3. bVariableItemHeights != 0 (bVariableItemHeights = 1) -> NOT intercepted (returns 0)
    g_rec.reset();
    r = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 1, fn);
    kt::CheckEq("Guard: bVariableItemHeights == 1 returns 0", r, 0);
    kt::CheckEq("Guard: origAddControls2 was not called", g_rec.callCount, 0);

    // 4. Non-label control (vtable != LabelVtable) -> NOT intercepted (returns 0)
    g_rec.reset();
    label.setVtable(0x12345678);
    r = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0, fn);
    kt::CheckEq("Guard: invalid control vtable returns 0", r, 0);
    kt::CheckEq("Guard: origAddControls2 was not called", g_rec.callCount, 0);
    label.setVtable(guikit::LabelVtable);

    // 5. Empty string -> NOT intercepted (returns 0)
    g_rec.reset();
    label.setText("");
    r = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0, fn);
    kt::CheckEq("Guard: empty string returns 0", r, 0);
    kt::CheckEq("Guard: origAddControls2 was not called", g_rec.callCount, 0);

    // 6. Null string -> NOT intercepted (returns 0)
    g_rec.reset();
    label.setText(nullptr);
    r = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0, fn);
    kt::CheckEq("Guard: null string returns 0", r, 0);
    kt::CheckEq("Guard: origAddControls2 was not called", g_rec.callCount, 0);

    // 7. RichDescBox=0 INI toggle -> NOT intercepted (returns 0)
    label.setText("Damage: Energy, 4-16\n");
    rmt::setRichDescBox(false);
    kt::CheckEq("RichDescBox is disabled", rmt::isRichDescBox(), false);
    g_rec.reset();
    r = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0, fn);
    kt::CheckEq("Guard: RichDescBox=0 returns 0", r, 0);
    kt::CheckEq("Guard: origAddControls2 was not called", g_rec.callCount, 0);

    // Re-enable RichDescBox
    rmt::setRichDescBox(true);
    kt::CheckEq("RichDescBox is re-enabled", rmt::isRichDescBox(), true);
    r = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0, fn);
    kt::CheckEq("Guard: RichDescBox=1 returns 1", r, 1);
    kt::CheckEq("Guard: origAddControls2 called", g_rec.callCount, 1);
}

void testPoolReuseAndResolutionChange() {
    rmt::resetPool();
    g_rec.reset();
    rmt::setRichDescBox(true);

    MockListBox listbox(0x1000, 260, 10);
    MockLabel label("Damage: Energy, 4-16\n");
    void* controls[1] = { label.voidPtr() };
    const uintptr_t fn = reinterpret_cast<uintptr_t>(fakeOrigAddControls2);

    // First call: populates listbox, allocates labels
    g_rec.reset();
    int r = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0, fn);
    kt::CheckEq("Pool: Call 1 succeeds", r, 1);
    void* const firstControl0 = g_rec.controls[0];
    kt::Check("Pool: Call 1 allocated control 0", firstControl0 != nullptr);

    // Second call on SAME listbox with SAME parentPanel (0x1000): reuses exact same pooled label!
    g_rec.reset();
    r = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0, fn);
    kt::CheckEq("Pool: Call 2 succeeds", r, 1);
    kt::CheckEq("Pool: Call 2 reuses exact same pooled label pointer", g_rec.controls[0], firstControl0);

    // Resolution change: parent panel changed to 0x2000!
    listbox.setParentPanel(0x2000);
    g_rec.reset();
    r = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0, fn);
    kt::CheckEq("Pool: Call 3 (resolution change) succeeds", r, 1);
    void* const freshControl0 = g_rec.controls[0];
    kt::Check("Pool: Call 3 allocated fresh control", freshControl0 != nullptr);
    kt::Check("Pool: Fresh control pointer differs from invalidated stale pointer",
              freshControl0 != firstControl0);

    // Fourth call on 0x2000: reuses freshControl0
    g_rec.reset();
    r = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0, fn);
    kt::CheckEq("Pool: Call 4 succeeds", r, 1);
    kt::CheckEq("Pool: Call 4 reuses fresh control", g_rec.controls[0], freshControl0);
}

void testArmorCompactLayout() {
    rmt::resetPool();
    g_rec.reset();
    rmt::setRichDescBox(true);

    MockListBox listbox(0x1000, 260, 10);
    const char* armorDesc =
        "FEATS REQUIRED\n"
        "Armor Proficiency - Heavy\n\n"
        "Defense Bonus: 12\n"
        "Max Dexterity Bonus: +0\n\n"
        "Mandalorian heavy battle armor.";
    MockLabel srcLabel(armorDesc, 0, 0, 240, 30);
    void* controls[1] = { srcLabel.voidPtr() };

    const int res = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0,
                                    reinterpret_cast<uintptr_t>(fakeOrigAddControls2));
    kt::CheckEq("Armor: RmtAddControls2 succeeds", res, 1);
    kt::CheckEq("Armor: Row count is 2", g_rec.count, 2);

    const rmt::LastPopulateInfo& info = rmt::getLastPopulateInfo();
    // Row 0 is Rules block (containing FEATS REQUIRED and Armor Proficiency)
    kt::CheckEq("Armor: Row 0 is not divider", info.rowIsDivider[0], false);
    kt::Check("Armor: Row 0 font is rulesFont", strcmp(info.rowFonts[0], rmt::getRulesFont()) == 0);
    kt::CheckEq("Armor: Row 0 height is 58 (4 lines * 14 + 2)", info.rowHeights[0], 58);
    kt::Check("Armor: Row 0 text contains FEATS REQUIRED", strstr(info.rowTexts[0], "FEATS REQUIRED") != nullptr);
    kt::Check("Armor: Row 0 text contains Armor Proficiency", strstr(info.rowTexts[0], "Armor Proficiency - Heavy") != nullptr);
    kt::Check("Armor: Row 0 text contains Defense Bonus", strstr(info.rowTexts[0], "Defense Bonus: 12") != nullptr);
    kt::Check("Armor: Row 0 text contains Max Dexterity Bonus", strstr(info.rowTexts[0], "Max Dexterity Bonus: +0") != nullptr);

    // Row 1 is Body lore
    kt::CheckEq("Armor: Row 1 is not divider", info.rowIsDivider[1], false);
    kt::Check("Armor: Row 1 font is fnt_d16x16", strcmp(info.rowFonts[1], rmt::kFontDesc) == 0);
    kt::CheckEq("Armor: Row 1 height is 16 (1 line * 14 + 2)", info.rowHeights[1], 16);
    kt::Check("Armor: Row 1 text is body prose", strcmp(info.rowTexts[1], "Mandalorian heavy battle armor.") == 0);
}

void testHeightClampingAgainstListBox() {
    rmt::resetPool();
    g_rec.reset();
    rmt::setRichDescBox(true);

    // Listbox with physical height lbH = 60.
    // Target max visible = lbH - 10 = 50.
    MockListBox listbox(0x1000, 260, 10);
    listbox.setExtent(0, 0, 240, 60);

    // A description whose natural sum is 58 (<= 60, but > 50).
    // Row 0: 3 lines * 14 + 2 = 44.
    // Row 1: 1 line * 14 + 2 = 16.
    // Sum = 60. targetMax = 50. Excess = 10.
    const char* desc = "Damage: 5\nAttack: 2\nRange: 20\n\nShort lore.";
    MockLabel srcLabel(desc, 0, 0, 240, 30);
    void* controls[1] = { srcLabel.voidPtr() };
    int r = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0,
                            reinterpret_cast<uintptr_t>(fakeOrigAddControls2));
    kt::CheckEq("Clamp: RmtAddControls2 succeeds", r, 1);
    const rmt::LastPopulateInfo& info = rmt::getLastPopulateInfo();
    const int clampedSum = info.rowHeights[0] + info.rowHeights[1];
    kt::Check("Clamp: sumHeights clamped to <= lbH - 10 (50)", clampedSum <= 50);
    kt::Check("Clamp: sumHeights <= lbH (60)", clampedSum <= 60);
}

void testCalibratedMeasureTextHeight() {
    rmt::g_measureTextHeight = nullptr; // use native logic

    // 12px for dialogfont10x10
    int h1 = rmt::measureTextHeight(0, "Line 1", 240, rmt::kFontSub);
    kt::CheckEq("Calib: 1 line 10x10 is 12px", h1, 12);

    // 14px for fnt_d16x16
    int h2 = rmt::measureTextHeight(0, "Line 1", 240, rmt::kFontDesc);
    kt::CheckEq("Calib: 1 line fnt_d16x16 is 14px", h2, 14);

    // 16px for dialogfont16x16
    int h3 = rmt::measureTextHeight(0, "Line 1", 240, rmt::kFontTitle);
    kt::CheckEq("Calib: 1 line 16x16 is 16px", h3, 16);

    // Empty line ("\n\n") adds half line height: 14 + 7 + 14 = 35px
    int hEmpty = rmt::measureTextHeight(0, "Line 1\n\nLine 2", 240, rmt::kFontDesc);
    kt::CheckEq("Calib: empty line adds half line height (35px)", hEmpty, 35);

    // charsPerLine = width / 7: width = 70 -> charsPerLine = 10.
    // 25 chars -> 3 lines -> 3 * 14 = 42px
    int hWrap = rmt::measureTextHeight(0, "1234567890123456789012345", 70, rmt::kFontDesc);
    kt::CheckEq("Calib: wrap with charsPerLine=width/7 is 3 lines (42px)", hWrap, 42);

    rmt::g_measureTextHeight = mockMeasureTextHeight; // restore mock
}

} // namespace

int main() {
    setupEnvironment();
    testTV7_SectionSplitItem();
    testTV8_SectionSplitJournal();
    testArmorCompactLayout();
    testHeightClampingAgainstListBox();
    testCalibratedMeasureTextHeight();
    testInterceptionFilterGuards();
    testPoolReuseAndResolutionChange();
    return kt::Report("rich_desc_box_test");
}
