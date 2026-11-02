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
desc = "Effect: Multi-font headers, +colored keywords, and stat label hierarchy."

[[row]]
id = "inline"
kind = "toggle"
label = "Coloured keywords"
desc = "Effect: On: +Keywords coloured in descriptions. Off: -Plain text."
default = 1
ini = "rich_menu_typography.ini"
section = "General"
key = "InlineColoring"

[[row]]
id = "richdesc"
kind = "toggle"
label = "Multi-font descriptions"
desc = "Effect: On: +Multi-font titles and rules layout. Off: -Uniform body text."
default = 1
ini = "rich_menu_typography.ini"
section = "General"
key = "RichDescBox"

[[row]]
id = "stats"
kind = "toggle"
label = "Stat label hierarchy"
desc = "Effect: On: +Key combat stats highlighted. Off: -Uniform stat labels."
default = 1
ini = "rich_menu_typography.ini"
section = "General"
key = "StatHierarchy"

[[row]]
id = "font"
kind = "list"
label = "Body font"
desc = "Effect: Body font face used for item, feat, and power descriptions."
default = 0
choices = ["fnt_d16x16", "dialogfont10x10", "dialogfont16x16"]

[[row]]
id = "rulesfont"
kind = "list"
label = "Rules font"
desc = "Effect: Font face used for stats, requirements, and rules blocks."
default = 0
choices = ["fnt_d16x16", "dialogfont10x10", "dialogfont16x16"]
)TOML";

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
