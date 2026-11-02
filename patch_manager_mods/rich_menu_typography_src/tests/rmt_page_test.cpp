// rmt_page_test.cpp -- Host test of the Rich Menu Typography Mod Configuration page (RichMenuTypographyPage.h, Doc 53 Section 6):
// Shipped schema loads via mcdecl::Page::load, 4 rows, string budgets <= 95 bytes, row ids <= 31 bytes,
// pure applyRow clamping, fontIndexFor case-insensitivity, defaults match rich_menu_typography.ini,
// and iniw::setKey preserves comments and Verbose=0 when writing BodyFont.

#include "ModConfigDecl.h"
#include "RichMenuTypographyPage.h"
#include "IniWrite.h"
#include "check.h"
#include <stdio.h>
#include <string.h>

using namespace rmtpage;

static mcdecl::Page g_page; // ~35 KB static storage

static void test_schema_load() {
    char err[96] = {};
    const bool loaded = g_page.load(kPageSchema, strlen(kPageSchema), err, sizeof(err));
    if (!loaded) {
        printf("Schema load error: %s\n", err);
    }
    kt::Check("schema loads via mcdecl::Page::load", loaded);
    kt::CheckEq("schema declares 5 rows", g_page.decl.nRows, 5);
    kt::Check("page name is Rich Menu Typography", strcmp(g_page.decl.name, "Rich Menu Typography") == 0);
    kt::Check("page title is Rich Menu Typography", strcmp(g_page.decl.title, "Rich Menu Typography") == 0);

    // Row id constants and order
    const char* expectedIds[5] = { kRowInline, kRowRichDesc, kRowStats, kRowFont, kRowRulesFont };
    for (int i = 0; i < 5 && i < g_page.decl.nRows; ++i) {
        kt::Check("row id matches constant", strcmp(g_page.decl.rows[i].id, expectedIds[i]) == 0);
    }

    // Toggle 1: inline
    const mcdecl::RowDecl& r0 = g_page.decl.rows[0];
    kt::Check("row 0 is toggle bound to InlineColoring",
              r0.kind == mcdecl::K_TOGGLE && r0.def == 1 &&
              strcmp(r0.ini, kIniFile) == 0 &&
              strcmp(r0.section, kSection) == 0 &&
              strcmp(r0.key, "InlineColoring") == 0);

    // Toggle 2: richdesc
    const mcdecl::RowDecl& r1 = g_page.decl.rows[1];
    kt::Check("row 1 is toggle bound to RichDescBox",
              r1.kind == mcdecl::K_TOGGLE && r1.def == 1 &&
              strcmp(r1.ini, kIniFile) == 0 &&
              strcmp(r1.section, kSection) == 0 &&
              strcmp(r1.key, "RichDescBox") == 0);

    // Toggle 3: stats
    const mcdecl::RowDecl& r2 = g_page.decl.rows[2];
    kt::Check("row 2 is toggle bound to StatHierarchy",
              r2.kind == mcdecl::K_TOGGLE && r2.def == 1 &&
              strcmp(r2.ini, kIniFile) == 0 &&
              strcmp(r2.section, kSection) == 0 &&
              strcmp(r2.key, "StatHierarchy") == 0);

    // List 4: font (unbound)
    const mcdecl::RowDecl& r3 = g_page.decl.rows[3];
    kt::Check("row 3 is unbound list with 3 choices and def=0",
              r3.kind == mcdecl::K_LIST && r3.def == 0 &&
              r3.nChoices == kFontCount &&
              r3.ini[0] == '\0' && r3.section[0] == '\0' && r3.key[0] == '\0' &&
              strcmp(r3.choices[0], "fnt_d16x16") == 0 &&
              strcmp(r3.choices[1], "dialogfont10x10") == 0 &&
              strcmp(r3.choices[2], "dialogfont16x16") == 0);

    // List 5: rulesfont (unbound)
    const mcdecl::RowDecl& r4 = g_page.decl.rows[4];
    kt::Check("row 4 is unbound list with 3 choices and def=0",
              r4.kind == mcdecl::K_LIST && r4.def == 0 &&
              r4.nChoices == kFontCount &&
              r4.ini[0] == '\0' && r4.section[0] == '\0' && r4.key[0] == '\0' &&
              strcmp(r4.choices[0], "fnt_d16x16") == 0 &&
              strcmp(r4.choices[1], "dialogfont10x10") == 0 &&
              strcmp(r4.choices[2], "dialogfont16x16") == 0);
}

static void test_string_budgets() {
    kt::Check("page name <= 95 bytes", strlen(g_page.decl.name) <= 95);
    kt::Check("page title <= 95 bytes", strlen(g_page.decl.title) <= 95);
    kt::Check("page desc <= 95 bytes", strlen(g_page.decl.desc) <= 95);

    for (int i = 0; i < g_page.decl.nRows; ++i) {
        const mcdecl::RowDecl& r = g_page.decl.rows[i];
        kt::Check("row id <= 31 bytes", strlen(r.id) <= 31);
        kt::Check("row label <= 95 bytes", strlen(r.label) <= 95);
        kt::Check("row desc <= 95 bytes", strlen(r.desc) <= 95);
        kt::Check("row ini <= 95 bytes", strlen(r.ini) <= 95);
        kt::Check("row section <= 95 bytes", strlen(r.section) <= 95);
        kt::Check("row key <= 95 bytes", strlen(r.key) <= 95);
        for (int c = 0; c < r.nChoices; ++c) {
            kt::Check("choice text <= 95 bytes", strlen(r.choices[c]) <= 95);
        }
    }
}

static void test_apply_row() {
    Live live;
    // initial state
    kt::Check("initial live defaults", live.inlineColoring && live.richDesc && live.stats && live.fontIdx == 0 && live.rulesFontIdx == 0);

    // toggle 0/1 round-trip
    kt::CheckEq("inline toggle off", applyRow(live, kRowInline, 0), 0);
    kt::Check("inline field is false", !live.inlineColoring);
    kt::CheckEq("inline toggle on", applyRow(live, kRowInline, 1), 1);
    kt::Check("inline field is true", live.inlineColoring);

    kt::CheckEq("richdesc toggle off", applyRow(live, kRowRichDesc, 0), 0);
    kt::Check("richdesc field is false", !live.richDesc);
    kt::CheckEq("richdesc toggle on", applyRow(live, kRowRichDesc, 1), 1);
    kt::Check("richdesc field is true", live.richDesc);

    kt::CheckEq("stats toggle off", applyRow(live, kRowStats, 0), 0);
    kt::Check("stats field is false", !live.stats);
    kt::CheckEq("stats toggle on", applyRow(live, kRowStats, 1), 1);
    kt::Check("stats field is true", live.stats);

    // 2 and -1 clamp for toggles
    kt::CheckEq("inline 2 clamps to 1", applyRow(live, kRowInline, 2), 1);
    kt::Check("inline is true after 2 clamp", live.inlineColoring);
    kt::CheckEq("inline -1 clamps to 0", applyRow(live, kRowInline, -1), 0);
    kt::Check("inline is false after -1 clamp", !live.inlineColoring);

    kt::CheckEq("richdesc 2 clamps to 1", applyRow(live, kRowRichDesc, 2), 1);
    kt::Check("richdesc is true after 2 clamp", live.richDesc);
    kt::CheckEq("richdesc -1 clamps to 0", applyRow(live, kRowRichDesc, -1), 0);
    kt::Check("richdesc is false after -1 clamp", !live.richDesc);

    kt::CheckEq("stats 2 clamps to 1", applyRow(live, kRowStats, 2), 1);
    kt::Check("stats is true after 2 clamp", live.stats);
    kt::CheckEq("stats -1 clamps to 0", applyRow(live, kRowStats, -1), 0);
    kt::Check("stats is false after -1 clamp", !live.stats);

    // font -1 and kFontCount clamp
    kt::CheckEq("font -1 clamps to 0", applyRow(live, kRowFont, -1), 0);
    kt::CheckEq("live fontIdx is 0", live.fontIdx, 0);
    kt::CheckEq("font 1 valid", applyRow(live, kRowFont, 1), 1);
    kt::CheckEq("live fontIdx is 1", live.fontIdx, 1);
    kt::CheckEq("font 2 valid", applyRow(live, kRowFont, 2), 2);
    kt::CheckEq("live fontIdx is 2", live.fontIdx, 2);
    kt::CheckEq("font kFontCount (3) clamps to 2", applyRow(live, kRowFont, kFontCount), kFontCount - 1);
    kt::CheckEq("live fontIdx is 2 after kFontCount clamp", live.fontIdx, kFontCount - 1);
    kt::CheckEq("font 99 clamps to 2", applyRow(live, kRowFont, 99), kFontCount - 1);
    kt::CheckEq("live fontIdx is 2 after 99 clamp", live.fontIdx, kFontCount - 1);

    // rulesfont -1 and kFontCount clamp
    kt::CheckEq("rulesfont -1 clamps to 0", applyRow(live, kRowRulesFont, -1), 0);
    kt::CheckEq("live rulesFontIdx is 0", live.rulesFontIdx, 0);
    kt::CheckEq("rulesfont 1 valid", applyRow(live, kRowRulesFont, 1), 1);
    kt::CheckEq("live rulesFontIdx is 1", live.rulesFontIdx, 1);
    kt::CheckEq("rulesfont 2 valid", applyRow(live, kRowRulesFont, 2), 2);
    kt::CheckEq("live rulesFontIdx is 2", live.rulesFontIdx, 2);
    kt::CheckEq("rulesfont kFontCount (3) clamps to 2", applyRow(live, kRowRulesFont, kFontCount), kFontCount - 1);
    kt::CheckEq("live rulesFontIdx is 2 after kFontCount clamp", live.rulesFontIdx, kFontCount - 1);
    kt::CheckEq("rulesfont 99 clamps to 2", applyRow(live, kRowRulesFont, 99), kFontCount - 1);
    kt::CheckEq("live rulesFontIdx is 2 after 99 clamp", live.rulesFontIdx, kFontCount - 1);

    // unknown id passthrough
    kt::CheckEq("unknown id passthrough", applyRow(live, "some_unknown_row", 42), 42);
    kt::Check("live state untouched by unknown id", live.fontIdx == 2 && !live.stats);

    // null id passthrough
    kt::CheckEq("null id passthrough", applyRow(live, nullptr, 77), 77);
    kt::Check("live state untouched by null id", live.fontIdx == 2 && !live.stats);
}

static void test_font_helpers() {
    // each name
    kt::CheckEq("fontIndexFor fnt_d16x16", fontIndexFor("fnt_d16x16"), 0);
    kt::CheckEq("fontIndexFor dialogfont10x10", fontIndexFor("dialogfont10x10"), 1);
    kt::CheckEq("fontIndexFor dialogfont16x16", fontIndexFor("dialogfont16x16"), 2);

    // uppercase
    kt::CheckEq("fontIndexFor FNT_D16X16", fontIndexFor("FNT_D16X16"), 0);
    kt::CheckEq("fontIndexFor DIALOGFONT10X10", fontIndexFor("DIALOGFONT10X10"), 1);
    kt::CheckEq("fontIndexFor DIALOGFONT16X16", fontIndexFor("DIALOGFONT16X16"), 2);

    // unknown
    kt::CheckEq("fontIndexFor unknown font", fontIndexFor("not_a_font"), 0);

    // null
    kt::CheckEq("fontIndexFor null", fontIndexFor(nullptr), 0);

    // empty
    kt::CheckEq("fontIndexFor empty string", fontIndexFor(""), 0);

    // fontName clamping
    kt::Check("fontName(0)", strcmp(fontName(0), "fnt_d16x16") == 0);
    kt::Check("fontName(1)", strcmp(fontName(1), "dialogfont10x10") == 0);
    kt::Check("fontName(2)", strcmp(fontName(2), "dialogfont16x16") == 0);
    kt::Check("fontName(-1) clamps to 0", strcmp(fontName(-1), "fnt_d16x16") == 0);
    kt::Check("fontName(3) clamps to 2", strcmp(fontName(3), "dialogfont16x16") == 0);
    kt::Check("fontName(99) clamps to 2", strcmp(fontName(99), "dialogfont16x16") == 0);
}

static void test_defaults_match_ini() {
    // Assert schema defaults match shipped rich_menu_typography.ini:
    // InlineColoring=1, RichDescBox=1, StatHierarchy=1, BodyFont=fnt_d16x16 (index 0)
    kt::CheckEq("schema InlineColoring def=1", g_page.decl.rows[0].def, 1);
    kt::CheckEq("schema RichDescBox def=1", g_page.decl.rows[1].def, 1);
    kt::CheckEq("schema StatHierarchy def=1", g_page.decl.rows[2].def, 1);
    kt::CheckEq("schema BodyFont choice def=0", g_page.decl.rows[3].def, 0);

    kt::CheckEq("BodyFont default fnt_d16x16 maps to index 0", fontIndexFor("fnt_d16x16"), 0);

    Live live;
    kt::Check("Live struct defaults match ini",
              live.inlineColoring == true &&
              live.richDesc == true &&
              live.stats == true &&
              live.fontIdx == 0);
}

static void test_ini_write_bodyfont() {
    const char sampleIni[] =
        "; rich_menu_typography.ini config file\n"
        "[General]\n"
        "; Section comment: text styling\n"
        "InlineColoring=1\n"
        "RichDescBox=1\n"
        "StatHierarchy=1\n"
        "BodyFont=fnt_d16x16\n"
        "Verbose=0\n";

    char outBuf[512] = {};
    size_t outLen = 0;
    const bool ok = iniw::setKey(sampleIni, strlen(sampleIni), kSection, "BodyFont", kFonts[1], outBuf, sizeof(outBuf), &outLen);
    kt::Check("iniw::setKey succeeds updating BodyFont", ok);
    kt::Check("comments preserved at top", strstr(outBuf, "; rich_menu_typography.ini config file\n") != nullptr);
    kt::Check("comments preserved in section", strstr(outBuf, "; Section comment: text styling\n") != nullptr);
    kt::Check("Verbose=0 line preserved", strstr(outBuf, "Verbose=0\n") != nullptr);
    kt::Check("BodyFont updated to dialogfont10x10", strstr(outBuf, "BodyFont=dialogfont10x10\n") != nullptr);
    kt::Check("InlineColoring=1 preserved", strstr(outBuf, "InlineColoring=1\n") != nullptr);
    kt::Check("RichDescBox=1 preserved", strstr(outBuf, "RichDescBox=1\n") != nullptr);
    kt::Check("StatHierarchy=1 preserved", strstr(outBuf, "StatHierarchy=1\n") != nullptr);

    // Commented default case: if BodyFont is commented, setKey adds it and preserves comments
    const char sampleWithCommented[] =
        "[General]\n"
        ";BodyFont=fnt_d16x16\n"
        "Verbose=0\n";
    char outBuf2[512] = {};
    size_t outLen2 = 0;
    const bool ok2 = iniw::setKey(sampleWithCommented, strlen(sampleWithCommented), kSection, "BodyFont", kFonts[2], outBuf2, sizeof(outBuf2), &outLen2);
    kt::Check("iniw::setKey succeeds with commented default", ok2);
    kt::Check("commented line preserved", strstr(outBuf2, ";BodyFont=fnt_d16x16\n") != nullptr);
    kt::Check("Verbose=0 preserved with commented default", strstr(outBuf2, "Verbose=0\n") != nullptr);
    kt::Check("BodyFont=dialogfont16x16 written", strstr(outBuf2, "BodyFont=dialogfont16x16\n") != nullptr);
}

int main() {
    test_schema_load();
    test_string_budgets();
    test_apply_row();
    test_font_helpers();
    test_defaults_match_ini();
    test_ini_write_bodyfont();
    return kt::Report("rmt_page_test");
}
