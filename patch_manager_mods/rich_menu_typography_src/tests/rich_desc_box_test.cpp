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

static int mockMeasureTextHeight(uintptr_t /*ctl*/, const char* text, int /*width*/) {
    if (!text || !*text) return 0;
    int lines = 1;
    for (const char* p = text; *p; ++p) {
        if (*p == '\n') ++lines;
    }
    return lines * 18;
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
    kt::CheckEq("TV7: origAddControls2 count == 5", g_rec.count, 5);
    kt::CheckEq("TV7: ListBox topItem reset to 0", static_cast<int>(listbox.getTopItem()), 0);

    const rmt::LastPopulateInfo& info = rmt::getLastPopulateInfo();
    kt::CheckEq("TV7: Snapshot rowCount is 5", info.rowCount, 5);

    // Row 0: Subtitle ("Fully Upgradeable", dialogfont10x10, kTealDim, align 9, height fh + 4 = 18 + 4 = 22)
    kt::CheckEq("TV7: Row 0 is not divider", info.rowIsDivider[0], false);
    kt::Check("TV7: Row 0 font is dialogfont10x10", strcmp(info.rowFonts[0], rmt::kFontSub) == 0);
    kt::CheckEq("TV7: Row 0 align is TopLeft (9)", info.rowAligns[0], rmt::kAlignTopLeft);
    kt::CheckEq("TV7: Row 0 height is fh + 4 = 22", info.rowHeights[0], 22);
    kt::Check("TV7: Row 0 text is Fully Upgradeable", strcmp(info.rowTexts[0], "Fully Upgradeable") == 0);
    kt::Check("TV7: Row 0 color is kTealDim", info.rowColors[0][0] == rmt::kTealDim[0]);

    // Row 1: Top Divider (2px, isDivider true, kTealFrame)
    kt::CheckEq("TV7: Row 1 is divider", info.rowIsDivider[1], true);
    kt::CheckEq("TV7: Row 1 height is kDividerHeight (2)", info.rowHeights[1], rmt::kDividerHeight);
    kt::CheckEq("TV7: Row 1 align is TopLeft (9)", info.rowAligns[1], rmt::kAlignTopLeft);
    kt::Check("TV7: Row 1 color is kTealFrame", info.rowColors[1][0] == rmt::kTealFrame[0]);

    // Row 2: Rules (Damage: Energy, 4-16\nAttack Modifier: +3, dialogfont10x10, kTealText, align 9, height fh + 6 = 36 + 6 = 42)
    kt::CheckEq("TV7: Row 2 is not divider", info.rowIsDivider[2], false);
    kt::Check("TV7: Row 2 font is dialogfont10x10", strcmp(info.rowFonts[2], rmt::kFontSub) == 0);
    kt::CheckEq("TV7: Row 2 align is TopLeft (9)", info.rowAligns[2], rmt::kAlignTopLeft);
    kt::CheckEq("TV7: Row 2 height is fh + 6 = 42", info.rowHeights[2], 42);
    kt::Check("TV7: Row 2 color is kTealText", info.rowColors[2][0] == rmt::kTealText[0]);

    // Row 3: Bottom Divider (2px, isDivider true, kTealFrame)
    kt::CheckEq("TV7: Row 3 is divider", info.rowIsDivider[3], true);
    kt::CheckEq("TV7: Row 3 height is kDividerHeight (2)", info.rowHeights[3], rmt::kDividerHeight);
    kt::CheckEq("TV7: Row 3 align is TopLeft (9)", info.rowAligns[3], rmt::kAlignTopLeft);

    // Row 4: Body (lore prose with {tag} stripped, fnt_d16x16, kTealText, align 9, height fh + 8 = 18 + 8 = 26)
    kt::CheckEq("TV7: Row 4 is not divider", info.rowIsDivider[4], false);
    kt::Check("TV7: Row 4 font is fnt_d16x16", strcmp(info.rowFonts[4], rmt::kFontDesc) == 0);
    kt::CheckEq("TV7: Row 4 align is TopLeft (9)", info.rowAligns[4], rmt::kAlignTopLeft);
    kt::CheckEq("TV7: Row 4 height is fh + 8 = 26", info.rowHeights[4], 26);
    kt::Check("TV7: Row 4 text has {tag} stripped",
              strcmp(info.rowTexts[4], "This blaster pistol was common during the Mandalorian Wars.") == 0);
    kt::Check("TV7: Row 4 color is kTealText", info.rowColors[4][0] == rmt::kTealText[0]);
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
    kt::CheckEq("TV8: origAddControls2 count == 3", g_rec.count, 3);

    const rmt::LastPopulateInfo& info = rmt::getLastPopulateInfo();
    kt::CheckEq("TV8: Snapshot rowCount is 3", info.rowCount, 3);

    // Row 0: Title ("Nar Shaddaa", dialogfont16x16, kCream, align 9, height 18 + 4 = 22)
    kt::CheckEq("TV8: Row 0 is not divider", info.rowIsDivider[0], false);
    kt::Check("TV8: Row 0 font is dialogfont16x16", strcmp(info.rowFonts[0], rmt::kFontTitle) == 0);
    kt::CheckEq("TV8: Row 0 align is TopLeft (9)", info.rowAligns[0], rmt::kAlignTopLeft);
    kt::CheckEq("TV8: Row 0 height is fh + 4 = 22", info.rowHeights[0], 22);
    kt::Check("TV8: Row 0 text is Nar Shaddaa (trailing colon stripped)",
              strcmp(info.rowTexts[0], "Nar Shaddaa") == 0);
    kt::Check("TV8: Row 0 color is kCream", info.rowColors[0][0] == rmt::kCream[0]);

    // Row 1: Top Divider (2px, isDivider true)
    kt::CheckEq("TV8: Row 1 is divider", info.rowIsDivider[1], true);
    kt::CheckEq("TV8: Row 1 height is 2", info.rowHeights[1], rmt::kDividerHeight);
    kt::CheckEq("TV8: Row 1 align is TopLeft (9)", info.rowAligns[1], rmt::kAlignTopLeft);

    // Row 2: Body (Refugee Sector troubles have escalated., fnt_d16x16, kTealText, align 9, height 18 + 8 = 26)
    kt::CheckEq("TV8: Row 2 is not divider", info.rowIsDivider[2], false);
    kt::Check("TV8: Row 2 font is fnt_d16x16", strcmp(info.rowFonts[2], rmt::kFontDesc) == 0);
    kt::CheckEq("TV8: Row 2 align is TopLeft (9)", info.rowAligns[2], rmt::kAlignTopLeft);
    kt::CheckEq("TV8: Row 2 height is fh + 8 = 26", info.rowHeights[2], 26);
    kt::Check("TV8: Row 2 text is Refugee Sector troubles...",
              strcmp(info.rowTexts[2], "Refugee Sector troubles have escalated.") == 0);
    kt::Check("TV8: Row 2 color is kTealText", info.rowColors[2][0] == rmt::kTealText[0]);
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

} // namespace

int main() {
    setupEnvironment();
    testTV7_SectionSplitItem();
    testTV8_SectionSplitJournal();
    testInterceptionFilterGuards();
    testPoolReuseAndResolutionChange();
    return kt::Report("rich_desc_box_test");
}
