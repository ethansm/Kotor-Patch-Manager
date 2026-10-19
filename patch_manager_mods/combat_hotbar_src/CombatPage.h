// Combat Action Hotbar -- Mod Configuration page (doc 46 declarative layer). Pure text, no Win32, so combat_page_test.cpp parses the shipped
// schema on the host. Four layout rows, each bound 1:1 to a combat_hotbar.ini [Buttons] key: ModConfigDecl writes the accepted value there,
// and the mod's own 1 s ini poll (next target-menu frame) applies it. Every [Buttons] key is live since 0.10.0-dev. Enabled, Cols, MaxRows,
// Replace and HideEmpty are master/rollback/capacity switches, so they get no row (see the mod-config value test, doc 45).
#pragma once

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

}  // namespace chpage
