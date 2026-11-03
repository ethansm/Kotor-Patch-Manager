#pragma once
#include <stdint.h>
#include <string.h>

namespace rmtpage {

constexpr const char* kEntryId = "rich-menu-typography";
constexpr const char* kIniFile = "rich_menu_typography.ini";
constexpr const char* kSection = "General";

constexpr const char* kRowInline    = "inline";
constexpr const char* kRowRichDesc  = "richdesc";
constexpr const char* kRowStats     = "stats";
constexpr const char* kRowFont      = "font";
constexpr const char* kRowRulesFont = "rulesfont";
constexpr const char* kRowReset     = "reset";

constexpr int kFontCount = 3;
constexpr const char* const kFonts[kFontCount] = {
    "fnt_d16x16",
    "dialogfont10x10",
    "dialogfont16x16"
};

// Every string is at most 95 bytes (TomlMini rejects longer ones).
constexpr const char* kPageSchema = R"TOML(
[page]
name = "Rich Menu Typography"
title = "Rich Menu Typography"
desc = "Effect: Styled item text, coloured stats and fonts. Change a row to preview."

[[row]]
id = "inline"
kind = "toggle"
label = "Coloured keywords"
desc = "Effect: Bonuses amber, penalties red, keywords tinted. OFF: one colour."
default = 1
ini = "rich_menu_typography.ini"
section = "General"
key = "InlineColoring"

[[row]]
id = "richdesc"
kind = "toggle"
label = "Multi-font descriptions"
desc = "Effect: Title, subtitle, rules and body get own fonts and tight spacing."
default = 1
ini = "rich_menu_typography.ini"
section = "General"
key = "RichDescBox"

[[row]]
id = "stats"
kind = "toggle"
label = "Stat label hierarchy"
desc = "Effect: Stat labels get a bright name and dim value. OFF: flat teal."
default = 1
ini = "rich_menu_typography.ini"
section = "General"
key = "StatHierarchy"

[[row]]
id = "font"
kind = "list"
label = "Body font"
desc = "Effect: Font for item and lore paragraphs. Click to cycle; preview shows it."
default = 0
choices = ["Default (fnt_d16x16)", "Small (dialogfont10x10)", "Large (dialogfont16x16)"]

[[row]]
id = "rulesfont"
kind = "list"
label = "Rules font"
desc = "Effect: Font for stat and requirement lines. Click to cycle; preview shows it."
default = 0
choices = ["Default (fnt_d16x16)", "Small (dialogfont10x10)", "Large (dialogfont16x16)"]

[[row]]
id = "reset"
kind = "toggle"
label = "Reset to Defaults"
desc = "Action: Switch on to restore all toggles and both fonts to defaults."
default = 0
)TOML";

// Sample item texts shown in the description box (item-description format so the
// mod's own section styling applies). Picked by the row that last changed.
constexpr const char* kPreviewWeapon =
    "Useable By:\nJedi Guardian, Jedi Sentinel\nDamage: 3-13 (+1-6 Energy)\nCritical Threat: 19-20,x2\n"
    "Attack Modifier: +3 (-2 Offhand)\n\nA blade of Mandalorian steel, rewarded to the Jedi who finish "
    "their trials. Its hilt is warm to the touch, and the crystal hums when the Force is near.";
constexpr const char* kPreviewArmor =
    "Fully Upgradeable\nFEATS REQUIRED\nArmor Proficiency - Heavy\nDefense Bonus: 12\n"
    "Max Dexterity Bonus: +0\n\nMandalorian heavy battle armor, reforged with Force-resistant plating.";
constexpr const char* kPreviewRobe =
    "Defense Bonus: 2\nMax Dexterity Bonus: +8\n\nCannot Equip:\nDroid\n\nRestricted:\nJedi Only\n\n"
    "Defense Bonus: +1 vs. Dark Side\n\nAncient Jedi Master robes imbued with the Force.";
constexpr const char* kPreviewPower =
    "Force Power:\nForce Lightning\nForce Cost: 4 FP\nRange: 10m Cone\nDamage: 2-8 (+1-4 Dark Side)\n\n"
    "Calls a crackling arc of lightning to strike every foe in a cone. Mastery of this power is a rite "
    "of passage for the Sith.";
constexpr const char* kPreviewLore =
    "Nar Shaddaa:\nRefugee Sector troubles have escalated since the Exchange tightened its grip on the "
    "landing pad. Smugglers now pay a toll to every dock foreman.\nBounty hunters from three systems "
    "have arrived, each claiming a share of the reward for the missing shipment.\nKeep your blaster "
    "holstered and your credits hidden. The Exchange does not forgive debts.";

// Which sample showcases the row that just changed (nullptr/unknown = weapon).
inline const char* previewFor(const char* rowId) {
    if (!rowId) return kPreviewWeapon;
    if (strcmp(rowId, kRowInline) == 0) return kPreviewRobe;
    if (strcmp(rowId, kRowRichDesc) == 0) return kPreviewPower;
    if (strcmp(rowId, kRowStats) == 0) return kPreviewArmor;
    if (strcmp(rowId, kRowFont) == 0) return kPreviewLore;
    if (strcmp(rowId, kRowRulesFont) == 0) return kPreviewWeapon;
    return kPreviewWeapon;
}

struct Live {
    bool inlineColoring = true;
    bool richDesc = true;
    bool stats = true;
    int fontIdx = 0;
    int rulesFontIdx = 0;
};

inline bool iequals(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        char ca = *a;
        char cb = *b;
        if (ca >= 'A' && ca <= 'Z') ca = static_cast<char>(ca + ('a' - 'A'));
        if (cb >= 'A' && cb <= 'Z') cb = static_cast<char>(cb + ('a' - 'A'));
        if (ca != cb) return false;
        ++a;
        ++b;
    }
    return *a == 0 && *b == 0;
}

inline int fontIndexFor(const char* name) {
    if (!name) return 0;
    for (int i = 0; i < kFontCount; ++i) {
        if (iequals(name, kFonts[i])) return i;
    }
    return 0;
}

inline const char* fontName(int idx) {
    if (idx < 0) idx = 0;
    else if (idx >= kFontCount) idx = kFontCount - 1;
    return kFonts[idx];
}

inline int32_t applyRow(Live& live, const char* id, int32_t requested) {
    if (!id) return requested;
    if (strcmp(id, kRowReset) == 0) {
        return (requested != 0) ? 1 : 0;
    }
    if (strcmp(id, kRowInline) == 0) {
        const int32_t v = (requested < 0) ? 0 : ((requested > 1) ? 1 : requested);
        live.inlineColoring = (v != 0);
        return v;
    }
    if (strcmp(id, kRowRichDesc) == 0) {
        const int32_t v = (requested < 0) ? 0 : ((requested > 1) ? 1 : requested);
        live.richDesc = (v != 0);
        return v;
    }
    if (strcmp(id, kRowStats) == 0) {
        const int32_t v = (requested < 0) ? 0 : ((requested > 1) ? 1 : requested);
        live.stats = (v != 0);
        return v;
    }
    if (strcmp(id, kRowFont) == 0) {
        const int32_t v = (requested < 0) ? 0 : ((requested >= kFontCount) ? (kFontCount - 1) : requested);
        live.fontIdx = v;
        return v;
    }
    if (strcmp(id, kRowRulesFont) == 0) {
        const int32_t v = (requested < 0) ? 0 : ((requested >= kFontCount) ? (kFontCount - 1) : requested);
        live.rulesFontIdx = v;
        return v;
    }
    return requested;
}

} // namespace rmtpage
