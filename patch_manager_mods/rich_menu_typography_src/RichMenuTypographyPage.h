#pragma once
#include <stdint.h>
#include <string.h>

namespace rmtpage {

constexpr const char* kEntryId = "rich-menu-typography";
constexpr const char* kIniFile = "rich_menu_typography.ini";
constexpr const char* kSection = "General";

constexpr const char* kRowSample    = "sample";
constexpr const char* kRowInline    = "inline";
constexpr const char* kRowRichDesc  = "richdesc";
constexpr const char* kRowStats     = "stats";
constexpr const char* kRowSemantic  = "semantic";
constexpr const char* kRowDividers  = "dividers";
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
id = "sample"
kind = "list"
label = "Preview sample"
desc = "Effect: Which sample item the preview shows. Every option below applies to it."
default = 0
choices = ["Showcase (all styles)", "Melee weapon", "Heavy armor", "Robe + restrictions", "Force power"]

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
id = "semantic"
kind = "toggle"
label = "Semantic colours"
desc = "Effect: Damage types blue, Jedi/Sith/Droid Only red inside stat values. OFF: cream."
default = 1
ini = "rich_menu_typography.ini"
section = "General"
key = "SemanticColors"

[[row]]
id = "dividers"
kind = "list"
label = "Section dividers"
desc = "Effect: Thin rule lines between description sections. Click to cycle; preview shows it."
default = 1
choices = ["Off", "Between sections", "All boundaries"]

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

// Sample item texts shown in the preview (item-description format so the mod's own section styling applies). ONE sample is shown for every
// option row (picked by the "Preview sample" row), and each carries a title, subtitle, rules with +/- numbers, a damage type, a restriction,
// a "vs." line and a lore body, so every option can be seen acting on the same text.
constexpr int kSampleCount = 5;
constexpr const char* kPreviewShowcase =
    "Dawnfang Vibroblade:\nFully Upgradeable\nDamage: 3-13 (+1-6 Energy)\nCritical Threat: 19-20,x2\n"
    "Attack Modifier: +3 (-2 Offhand)\nDamage Bonus: +4 vs. Droids\nEquip: Jedi Only\n\n"
    "A duelling blade of the Echani, balanced for the fastest strike. Its edge hums at a pitch only a master can hear, and the crystal sings when the Force is near.";
constexpr const char* kPreviewMelee =
    "Mandalorian Blade:\nTwo-handed\nDamage: 3-13 (+1-6 Sonic)\nCritical Threat: 19-20,x2\n"
    "Attack Modifier: +3 (-2 Offhand)\nDamage Bonus: +2 vs. Droids\nUseable By:\nJedi Guardian, Jedi Sentinel\n\n"
    "A blade of Mandalorian steel, rewarded to the Jedi who finish their trials. Its hilt is warm to the touch, and the crystal hums when the Force is near.";
constexpr const char* kPreviewArmorFeats =
    "Heavy Battle Armor:\nFully Upgradeable\nFEATS REQUIRED\nArmor Proficiency - Heavy\nDefense Bonus: 12\n"
    "Max Dexterity Bonus: +0 (-2 Stealth)\nResistance: +5 vs. Fire\n\n"
    "Mandalorian heavy battle armor, reforged with Force-resistant plating that turns aside blaster fire and lightsaber sparks alike.";
constexpr const char* kPreviewRobeBlocks =
    "Ancient Jedi Robes:\nLight Armor\nDefense Bonus: 2\nMax Dexterity Bonus: +8 (-2 Stealth)\nCannot Equip:\nDroid\n\n"
    "Restricted:\nJedi Only\n\nResistance: +2 vs. Cold\n\n"
    "Ancient Jedi Master robes imbued with the Force, still warm from a Jedi who wore them to the very end.";
constexpr const char* kPreviewForcePower =
    "Force Power:\nForce Lightning\nForce Cost: 4 FP (-1 Dark Side)\nRange: 10m Cone\nDamage: 2-8 (+1-4 Electrical)\n"
    "Damage Bonus: +2 vs. Droids\nRestricted:\nSith Only\n\n"
    "Calls a crackling arc of lightning to strike every foe in a cone. Mastery of this power is a rite of passage for the Sith.";
constexpr const char* const kSamples[kSampleCount] = {
    kPreviewShowcase, kPreviewMelee, kPreviewArmorFeats, kPreviewRobeBlocks, kPreviewForcePower
};

// The sample for an index from the "Preview sample" row (out of range clamps; 0 = Showcase).
inline const char* previewFor(int sampleIdx) {
    if (sampleIdx < 0 || sampleIdx >= kSampleCount) sampleIdx = 0;
    return kSamples[sampleIdx];
}

struct Live {
    int sampleIdx = 0;
    bool inlineColoring = true;
    bool richDesc = true;
    bool stats = true;
    bool semantic = true;
    int dividers = 1;   // 0 off, 1 between title|rules and rules|body, 2 also between rule groups
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
    if (strcmp(id, kRowSample) == 0) {
        const int32_t v = (requested < 0) ? 0 : ((requested >= kSampleCount) ? (kSampleCount - 1) : requested);
        live.sampleIdx = v;
        return v;
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
    if (strcmp(id, kRowDividers) == 0) {
        const int32_t v = (requested < 0) ? 0 : ((requested > 2) ? 2 : requested);
        live.dividers = v;
        return v;
    }
    if (strcmp(id, kRowSemantic) == 0) {
        const int32_t v = (requested < 0) ? 0 : ((requested > 1) ? 1 : requested);
        live.semantic = (v != 0);
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
