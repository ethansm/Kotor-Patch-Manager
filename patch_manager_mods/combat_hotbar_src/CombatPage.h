// Combat Action Hotbar -- Mod Configuration page (doc 46 declarative layer). Pure text, no Win32, so combat_page_test.cpp parses the shipped
// schema on the host. Four layout rows, each bound 1:1 to a combat_hotbar.ini [Buttons] key: ModConfigDecl writes the accepted value there,
// and the mod's own 1 s ini poll (next target-menu frame) applies it. Every [Buttons] key is live since 0.10.0-dev. Enabled, Cols, MaxRows,
// Replace and HideEmpty are master/rollback/capacity switches, so they get no row (see the mod-config value test, doc 45).
#pragma once
#include "CombatPreview.h"

namespace chpage {

constexpr const char* kEntryId = "combat-hotbar";
constexpr const char* kIniFile = "combat_hotbar.ini";

// Row ids, in order.
constexpr const char* kRowSize = "size";
constexpr const char* kRowCols = "cols";
constexpr const char* kRowPad = "pad";
constexpr const char* kRowGap = "gap";

// default = the mod's own default (CombatLogic.h kDefault*, CombatHotbar.cpp loadIni); the test pins them together.
constexpr const char* kPageSchema = R"TOML(
[page]
name = "Combat Hotbar"
title = "Combat Hotbar Layout"
desc = "Size and spacing of the target-menu action buttons. Applies on the next target."

[[row]]
id = "size"
kind = "stepper"
label = "Button size"
desc = "Button size in pixels. Auto fits the target menu. Too large a size is shrunk to fit."
default = 0
min = 0
max = 64
step = 8
zero_text = "Auto"
ini = "combat_hotbar.ini"
section = "Buttons"
key = "BtnPx"

[[row]]
id = "cols"
kind = "stepper"
label = "Columns (all groups)"
desc = "Columns shared by the visible groups: 6 gives 2 per group with three groups. Fewer = taller."
default = 6
min = 2
max = 12
step = 1
ini = "combat_hotbar.ini"
section = "Buttons"
key = "ColsTotal"

[[row]]
id = "pad"
kind = "stepper"
label = "Button spacing"
desc = "Gap in pixels between buttons. A large gap shrinks the buttons."
default = 8
min = 0
max = 16
step = 2
ini = "combat_hotbar.ini"
section = "Buttons"
key = "Pad"

[[row]]
id = "gap"
kind = "stepper"
label = "Group spacing"
desc = "Gap in pixels between the group backgrounds."
default = 10
min = 0
max = 32
step = 2
ini = "combat_hotbar.ini"
section = "Buttons"
key = "GroupGap"
)TOML";

// ---- preview + test-state modal (doc 46 MCF10/CH7) --------------------------------------------------------------------------------------------
// The gear on the page opens a modal whose rows set the DEMO target of the preview (chprev::Demo, memory only: never written to an ini, never read by the
// live HUD). Row kinds are mod-config's MUI_ROW_* values (0 toggle, 1 stepper); the glue in CombatHotbar.cpp turns this table into K2MuiRowSpec.
constexpr const char* kModalTitle = "Test state";
constexpr const char* kGearDesc = "Test state: set what the preview shows (how many attacks, force powers and items, unusable entries, screen size) to check the layout. Nothing here changes the game.";
constexpr int kPreviewPermille = 700;   // preview share of the description pane (mod-config clamps to 600..850)

enum ModalRow { MR_ATTACKS = 0, MR_POWERS, MR_ITEMS, MR_UNUSABLE, MR_SCREEN, MR_RESET, MR_COUNT };

constexpr const char* kUnusableChoices[3] = {"None", "Some", "All"};
constexpr const char* kScreenChoices[chprev::kNumScreens] = {"1280x720", "1920x1080", "2560x1080", "3440x1440"};

struct ModalRowDef {
    int kind;                    // 0 toggle, 1 stepper
    const char* label;
    const char* desc;
    int minV, maxV, step, def;
    bool wrap;
    const char* const* choices;  // null = numeric
    int nChoices;
};

constexpr ModalRowDef kModalRows[MR_COUNT] = {
    {1, "Attacks", "Entries in the first group (feats and attacks) of the demo target, 0 to 16.", 0, 16, 1, 4, false, nullptr, 0},
    {1, "Force powers", "Entries in the force power group of the demo target, 0 to 16. 0 hides that group's pill.", 0, 16, 1, 3, false, nullptr, 0},
    {1, "Items", "Entries in the item group of the demo target, 0 to 16. Items are listed by stack size.", 0, 16, 1, 2, false, nullptr, 0},
    {1, "Unusable", "Dim none, some (every second one) or all entries, as when the party cannot act.", 0, 2, 1, 0, true, kUnusableChoices, 3},
    {1, "Screen", "Screen size the preview pretends to be, to check ultrawide and small screens.", 0, chprev::kNumScreens - 1, 1, 1, true, kScreenChoices, chprev::kNumScreens},
    {0, "Reset demo", "Back to the starting demo target (4 attacks, 3 force powers, 2 items, none unusable, 1920x1080).", 0, 1, 1, 0, false, nullptr, 0},
};

// The demo field a modal row edits. MR_RESET and out-of-range rows are not fields (value ignored).
inline void demoApply(chprev::Demo& d, int row, int value) {
    switch (row) {
        case MR_ATTACKS: case MR_POWERS: case MR_ITEMS: d.count[row] = chprev::clampi(value, 0, 16); break;
        case MR_UNUSABLE: d.unusable = chprev::clampi(value, 0, 2); break;
        case MR_SCREEN: d.screen = chprev::clampi(value, 0, chprev::kNumScreens - 1); break;
        default: break;
    }
}
inline int demoValue(const chprev::Demo& d, int row) {
    switch (row) {
        case MR_ATTACKS: case MR_POWERS: case MR_ITEMS: return d.count[row];
        case MR_UNUSABLE: return d.unusable;
        case MR_SCREEN: return d.screen;
        default: return 0;
    }
}

}  // namespace chpage
