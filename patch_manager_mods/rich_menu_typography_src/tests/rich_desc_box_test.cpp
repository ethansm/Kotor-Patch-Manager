// rich_desc_box_test.cpp -- Unit and contract tests for RichDescBox.h and RmtAddControls2 (Doc 53 RMT2).
#include "RichDescBox.h"
#include "RichMenuTypography.h"
#include "check.h"
#include "windows.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>
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
        *reinterpret_cast<uint32_t*>(raw + guikit::Label_Text + guikit::Text_Renderer) = 0x1000;
        *reinterpret_cast<const char**>(raw + rmt::kLabelOff_TextParams) = text;

        setColor(rmt::kTealText[0], rmt::kTealText[1], rmt::kTealText[2]);
    }

    void setColor(float r, float g, float b) {
        float* col = reinterpret_cast<float*>(raw + guikit::Label_Text + guikit::Text_StringSub + guikit::Text_Color);
        col[0] = r;
        col[1] = g;
        col[2] = b;
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
    rmt::g_measureTextHeight = nullptr; // native calibrated logic (no container inflation)

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

    // Row 0: Subtitle ("Fully Upgradeable", dialogfont10x10, kTealDim, align 9, height fh + 3 (gap) = 11)
    kt::CheckEq("TV7: Row 0 is not divider", info.rowIsDivider[0], false);
    kt::Check("TV7: Row 0 font is dialogfont10x10", strcmp(info.rowFonts[0], rmt::kFontSub) == 0);
    kt::CheckEq("TV7: Row 0 align is TopLeft (9)", info.rowAligns[0], rmt::kAlignTopLeft);
    kt::CheckEq("TV7: Row 0 height is fh = 11", info.rowHeights[0], 11);
    kt::Check("TV7: Row 0 text is Fully Upgradeable", strcmp(info.rowTexts[0], "Fully Upgradeable") == 0);
    kt::Check("TV7: Row 0 color is kTealDim", info.rowColors[0][0] == rmt::kTealDim[0]);

    // Row 1: Rules (Damage: Energy, 4-16\nAttack Modifier: +3, getRulesFont() fnt_d16x16, kTealText, align 9, height 2*14 + 4 (gap) = 32)
    kt::CheckEq("TV7: Row 1 is not divider", info.rowIsDivider[1], false);
    kt::Check("TV7: Row 1 font is rulesFont", strcmp(info.rowFonts[1], rmt::getRulesFont()) == 0);
    kt::CheckEq("TV7: Row 1 align is TopLeft (9)", info.rowAligns[1], rmt::kAlignTopLeft);
    kt::CheckEq("TV7: Row 1 height is fh = 32", info.rowHeights[1], 32);
    kt::Check("TV7: Row 1 color is kTealText", info.rowColors[1][0] == rmt::kTealText[0]);

    // Row 2: Body (lore prose with {tag} stripped, fnt_d16x16, kTealText, align 9, 2 lines * 14 = 28)
    kt::CheckEq("TV7: Row 2 is not divider", info.rowIsDivider[2], false);
    kt::Check("TV7: Row 2 font is fnt_d16x16", strcmp(info.rowFonts[2], rmt::kFontDesc) == 0);
    kt::CheckEq("TV7: Row 2 align is TopLeft (9)", info.rowAligns[2], rmt::kAlignTopLeft);
    kt::CheckEq("TV7: Row 2 height is fh = 28", info.rowHeights[2], 28);
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

    // Row 0: Title ("Nar Shaddaa", dialogfont16x16, kCream, align 9, height 12 + 4 (gap) = 16)
    kt::CheckEq("TV8: Row 0 is not divider", info.rowIsDivider[0], false);
    kt::Check("TV8: Row 0 font is dialogfont16x16", strcmp(info.rowFonts[0], rmt::kFontTitle) == 0);
    kt::CheckEq("TV8: Row 0 align is TopLeft (9)", info.rowAligns[0], rmt::kAlignTopLeft);
    kt::CheckEq("TV8: Row 0 height is fh = 16", info.rowHeights[0], 16);
    kt::Check("TV8: Row 0 text is Nar Shaddaa (trailing colon stripped)",
              strcmp(info.rowTexts[0], "Nar Shaddaa") == 0);
    kt::Check("TV8: Row 0 color is kCream", info.rowColors[0][0] == rmt::kCream[0]);

    // Row 1: Body (Refugee Sector troubles have escalated., fnt_d16x16, kTealText, align 9, 2 lines * 14 = 28)
    kt::CheckEq("TV8: Row 1 is not divider", info.rowIsDivider[1], false);
    kt::Check("TV8: Row 1 font is fnt_d16x16", strcmp(info.rowFonts[1], rmt::kFontDesc) == 0);
    kt::CheckEq("TV8: Row 1 align is TopLeft (9)", info.rowAligns[1], rmt::kAlignTopLeft);
    kt::CheckEq("TV8: Row 1 height is fh = 28", info.rowHeights[1], 28);
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
    kt::CheckEq("Armor: Row 0 height is 60 (4 lines * 14 + 4 gap)", info.rowHeights[0], 60);
    kt::Check("Armor: Row 0 text contains FEATS REQUIRED", strstr(info.rowTexts[0], "FEATS REQUIRED") != nullptr);
    kt::Check("Armor: Row 0 text contains Armor Proficiency", strstr(info.rowTexts[0], "Armor Proficiency - Heavy") != nullptr);
    kt::Check("Armor: Row 0 text contains Defense Bonus", strstr(info.rowTexts[0], "Defense Bonus: 12") != nullptr);
    kt::Check("Armor: Row 0 text contains Max Dexterity Bonus", strstr(info.rowTexts[0], "Max Dexterity Bonus: +0") != nullptr);

    // Row 1 is Body lore (31 chars is 1 line at charW=7: 1 * 14 = 14)
    kt::CheckEq("Armor: Row 1 is not divider", info.rowIsDivider[1], false);
    kt::Check("Armor: Row 1 font is fnt_d16x16", strcmp(info.rowFonts[1], rmt::kFontDesc) == 0);
    kt::CheckEq("Armor: Row 1 height is 14 (1 line * 14)", info.rowHeights[1], 14);
    kt::Check("Armor: Row 1 text is body prose", strcmp(info.rowTexts[1], "Mandalorian heavy battle armor.") == 0);
}

void testHeightClampingAgainstListBox() {
    rmt::resetPool();
    g_rec.reset();
    rmt::setRichDescBox(true);

    MockListBox listbox(0x1000, 260, 10);
    listbox.setExtent(0, 0, 240, 85);

    // Case 1: Natural heights without artificial clamping
    // Row 0: 3 lines * 14 + 4 gap = 46.
    // Row 1: 1 line * 14 = 14.
    // Sum = 60 <= 85.
    const char* desc = "Damage: 5\nAttack: 2\nRange: 20\n\nShort lore.";
    MockLabel srcLabel(desc, 0, 0, 240, 30);
    void* controls[1] = { srcLabel.voidPtr() };
    int r = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0,
                            reinterpret_cast<uintptr_t>(fakeOrigAddControls2));
    kt::CheckEq("Clamp: RmtAddControls2 succeeds", r, 1);
    const rmt::LastPopulateInfo& info = rmt::getLastPopulateInfo();
    const int naturalSum = info.rowHeights[0] + info.rowHeights[1];
    kt::CheckEq("Natural: Row 0 height is 46", info.rowHeights[0], 46);
    kt::CheckEq("Natural: Row 1 height is 14", info.rowHeights[1], 14);
    kt::CheckEq("Natural: sumHeights is 60", naturalSum, 60);

    // Case 2: Natural sum when more lines present
    // Row 0: 4 lines * 14 = 56.
    // Row 1: 1 line * 14 = 14.
    // Sum = 70. Available clientH = 60 < 70 -> needs scrolling, uses standard 4 gap -> 56 + 4 + 14 = 74.
    rmt::resetPool();
    g_rec.reset();
    listbox.setExtent(0, 0, 240, 60);
    const char* descOver = "Damage: 5\nAttack: 2\nRange: 20\nCritical: 20\n\nShort lore.";
    MockLabel srcLabelOver(descOver, 0, 0, 240, 30);
    void* controlsOver[1] = { srcLabelOver.voidPtr() };
    r = RmtAddControls2(listbox.voidPtr(), controlsOver, 1, 0, 0, 0,
                        reinterpret_cast<uintptr_t>(fakeOrigAddControls2));
    kt::CheckEq("ClampOver: RmtAddControls2 succeeds", r, 1);
    const rmt::LastPopulateInfo& infoOver = rmt::getLastPopulateInfo();
    const int sumOver = infoOver.rowHeights[0] + infoOver.rowHeights[1];
    kt::CheckEq("NaturalOver: sumHeights is 74", sumOver, 74);
}

void testCalibratedMeasureTextHeight() {
    rmt::g_measureTextHeight = nullptr; // use native logic

    // 8px for dialogfont10x10
    int h1 = rmt::measureTextHeight(0, "Line 1", 240, rmt::kFontSub);
    kt::CheckEq("Calib: 1 line 10x10 is 8px", h1, 8);

    // 14px for fnt_d16x16
    int h2 = rmt::measureTextHeight(0, "Line 1", 240, rmt::kFontDesc);
    kt::CheckEq("Calib: 1 line fnt_d16x16 is 14px", h2, 14);

    // 12px for dialogfont16x16
    int h3 = rmt::measureTextHeight(0, "Line 1", 240, rmt::kFontTitle);
    kt::CheckEq("Calib: 1 line 16x16 is 12px", h3, 12);

    // Empty line ("\n\n") adds half line height: 14 + 7 + 14 = 35px
    int hEmpty = rmt::measureTextHeight(0, "Line 1\n\nLine 2", 240, rmt::kFontDesc);
    kt::CheckEq("Calib: empty line adds half line height (35px)", hEmpty, 35);

    // charsPerLine = width / charW (width=70 / 7 = 10 chars/line):
    // 25 chars -> 3 lines -> 3 * 14 = 42px
    int hWrap = rmt::measureTextHeight(0, "1234567890123456789012345", 70, rmt::kFontDesc);
    kt::CheckEq("Calib: wrap with charsPerLine=width/charW is 3 lines (42px)", hWrap, 42);

    // Short subtitle and rules calibrated metrics
    int hSub = rmt::measureTextHeight(0, "Fully Upgradeable", 240, rmt::kFontSub);
    kt::CheckEq("Calib: short subtitle is 8px", hSub, 8);

    int hRules = rmt::measureTextHeight(0, "Damage: Energy, 4-16\nAttack Modifier: +3", 240, rmt::getRulesFont());
    kt::CheckEq("Calib: 2-line rules is 28px", hRules, 28);
}

void testThemePaletteAdaptation() {
    rmt::resetPool();
    g_rec.reset();
    rmt::setRichDescBox(true);

    MockListBox listbox(0x1000, 260, 10);
    const char* desc = "Combat Suit\n\nDefense Bonus: 4\nMax Dexterity Bonus: +5\n\nEven the most frugal of mercenaries know they need protection.";
    MockLabel srcLabel(desc, 0, 0, 240, 30);
    // Set non-teal Amber color (Merchant menu: R=0.70, G=0.55, B=0.20)
    srcLabel.setColor(0.70f, 0.55f, 0.20f);
    void* controls[1] = { srcLabel.voidPtr() };

    const int res = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0,
                                    reinterpret_cast<uintptr_t>(fakeOrigAddControls2));
    kt::CheckEq("Theme: RmtAddControls2 intercepts amber merchant item", res, 1);
    const rmt::LastPopulateInfo& info = rmt::getLastPopulateInfo();

    // Verify Row 0 (Subtitle "Combat Suit") adapts to dimmed amber (0.70 * 0.75 = 0.525f)
    kt::Check("Theme: Row 0 color R is dimmed amber (0.525f)", std::abs(info.rowColors[0][0] - 0.525f) < 0.01f);

    // Verify Rules and Body retain original amber color
    kt::Check("Theme: Row 1 color R matches amber 0.70f", std::abs(info.rowColors[1][0] - 0.70f) < 0.01f);
    kt::Check("Theme: Row 1 color G matches amber 0.55f", std::abs(info.rowColors[1][1] - 0.55f) < 0.01f);
    kt::Check("Theme: Row 2 color R matches amber 0.70f", std::abs(info.rowColors[2][0] - 0.70f) < 0.01f);
}

void testShieldContinuationParsing() {
    rmt::resetPool();
    g_rec.reset();
    rmt::setRichDescBox(true);

    MockListBox listbox(0x1000, 260, 10);
    const char* shieldDesc =
        "Deflection: Bludgeoning, Piercing, Slashing, 20pts total\n"
        "Duration: 200 seconds,\n"
        "or max damage taken\n\n"
        "Charges: Using this item consumes one charge.\n\n"
        "Mandalorians don't fear melee combat, but they don't fight it unprotected either.";
    MockLabel srcLabel(shieldDesc, 0, 0, 240, 30);
    void* controls[1] = { srcLabel.voidPtr() };

    const int res = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0,
                                    reinterpret_cast<uintptr_t>(fakeOrigAddControls2));
    kt::CheckEq("Shield: RmtAddControls2 succeeds", res, 1);
    kt::CheckEq("Shield: Row count is 2 (Rules + Body)", g_rec.count, 2);

    const rmt::LastPopulateInfo& info = rmt::getLastPopulateInfo();
    // Verify Row 0 has all rules including continuation and Charges:
    kt::Check("Shield: Row 0 contains Deflection", strstr(info.rowTexts[0], "Deflection:") != nullptr);
    kt::Check("Shield: Row 0 contains Duration", strstr(info.rowTexts[0], "Duration: 200 seconds,") != nullptr);
    kt::Check("Shield: Row 0 contains continuation line", strstr(info.rowTexts[0], "or max damage taken") != nullptr);
    kt::Check("Shield: Row 0 contains Charges", strstr(info.rowTexts[0], "Charges: Using this item") != nullptr);

    // Verify Row 1 has lore prose only
    kt::Check("Shield: Row 1 contains lore prose", strstr(info.rowTexts[1], "Mandalorians don't fear melee combat") != nullptr);
    kt::Check("Shield: Row 1 does not contain Charges", strstr(info.rowTexts[1], "Charges:") == nullptr);
}

void testVibroswordUpgradeTagParsing() {
    rmt::resetPool();
    g_rec.reset();
    rmt::setRichDescBox(true);

    MockListBox listbox(0x1000, 260, 2);
    const char* vibroDesc =
        "Feats Required:\n"
        "Weapon Proficiency - Melee Weapons\n"
        "Damage: Physical, 3-13\n"
        "Ion: +1-6 vs. Droid\n"
        "Damage Bonus: +1 Energy, +1-4 Ion\n"
        "Critical Threat:\n"
        "19-20,x2\n"
        "Attack Modifier: +1\n"
        "Fully Upgradeable\n\n"
        "Ultrasonic generators power this Echani-developed weapon design.\n"
        "A cortosis weave ensures traditional swordplay will endure.";
    MockLabel srcLabel(vibroDesc, 0, 0, 240, 30);
    void* controls[1] = { srcLabel.voidPtr() };

    const int res = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0,
                                    reinterpret_cast<uintptr_t>(fakeOrigAddControls2));
    kt::CheckEq("Vibro: RmtAddControls2 succeeds", res, 1);
    kt::CheckEq("Vibro: Row count is 2 (Rules + Body)", g_rec.count, 2);

    const rmt::LastPopulateInfo& info = rmt::getLastPopulateInfo();
    kt::Check("Vibro: Row 0 contains Attack Modifier", strstr(info.rowTexts[0], "Attack Modifier: +1") != nullptr);
    kt::Check("Vibro: Row 0 contains Fully Upgradeable", strstr(info.rowTexts[0], "Fully Upgradeable") != nullptr);
    kt::Check("Vibro: Row 1 does NOT contain Fully Upgradeable", strstr(info.rowTexts[1], "Fully Upgradeable") == nullptr);
    kt::Check("Vibro: Row 1 contains lore prose", strstr(info.rowTexts[1], "Ultrasonic generators power") != nullptr);
}

void testTV11_MeasurementProbeLogging() {
    rmt::resetPool();
    g_rec.reset();
    rmt::setRichDescBox(true);

    static std::vector<std::string> capturedLogs;
    capturedLogs.clear();
    rmt::g_probeLog = [](const char* msg) {
        if (msg) capturedLogs.push_back(msg);
    };

    MockListBox listbox(0x1000, 260, 10);
    const char* itemDesc = "Fully Upgradeable\n\nDamage: Energy, 4-16\nAttack Modifier: +3\n\n{30}This blaster pistol was common during the Mandalorian Wars.";
    MockLabel srcLabel(itemDesc, 0, 0, 240, 30);
    void* controls[1] = { srcLabel.voidPtr() };

    const int res = RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0,
                                    reinterpret_cast<uintptr_t>(fakeOrigAddControls2));
    kt::CheckEq("TV11: RmtAddControls2 intercepts item description and returns 1", res, 1);
    kt::CheckEq("TV11: origAddControls2 count == 3", g_rec.count, 3);

    const rmt::LastPopulateInfo& info = rmt::getLastPopulateInfo();
    kt::CheckEq("TV11: Snapshot rowCount is 3", info.rowCount, 3);
    kt::CheckEq("TV11: Row 0 height is fh = 11", info.rowHeights[0], 11);
    kt::CheckEq("TV11: Row 1 height is fh = 32", info.rowHeights[1], 32);
    kt::CheckEq("TV11: Row 2 height is fh = 28", info.rowHeights[2], 28);

    bool hasRowProbe = false;
    bool hasSumProbe = false;
    for (const auto& line : capturedLogs) {
        if (line.rfind("RMTPROBE row", 0) == 0) hasRowProbe = true;
        if (line.rfind("RMTPROBE sum", 0) == 0) hasSumProbe = true;
    }
    kt::Check("TV11: Captured at least one RMTPROBE row line", hasRowProbe);
    kt::Check("TV11: Captured at least one RMTPROBE sum line", hasSumProbe);

    // Verify dedupe: second populate call with identical parameters emits no new probe lines
    const size_t prevCount = capturedLogs.size();
    RmtAddControls2(listbox.voidPtr(), controls, 1, 0, 0, 0,
                    reinterpret_cast<uintptr_t>(fakeOrigAddControls2));
    kt::CheckEq("TV11: Dedupe prevents duplicate logs on identical populate", static_cast<int>(capturedLogs.size()), static_cast<int>(prevCount));

    rmt::g_probeLog = nullptr;
}

} // namespace

int main() {
    setupEnvironment();
    testTV7_SectionSplitItem();
    testTV8_SectionSplitJournal();
    testArmorCompactLayout();
    testHeightClampingAgainstListBox();
    testCalibratedMeasureTextHeight();
    testThemePaletteAdaptation();
    testShieldContinuationParsing();
    testVibroswordUpgradeTagParsing();
    testInterceptionFilterGuards();
    testPoolReuseAndResolutionChange();
    testTV11_MeasurementProbeLogging();
    return kt::Report("rich_desc_box_test");
}
