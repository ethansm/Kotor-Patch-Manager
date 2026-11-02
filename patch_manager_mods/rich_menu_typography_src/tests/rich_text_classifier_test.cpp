// rich_text_classifier_test.cpp -- Host unit & boundary tests for RichTextClassifier.h and ItemProps.h (Doc 53 RMT0).
// Covers TV1..TV8 from Doc 53 section 4.2 plus ItemProps.h shared item/rules parser edge cases.

#include "RichTextClassifier.h"
#include "ItemProps.h"
#include "check.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace {

// Helper to construct a mock CAurGUIStringInternal buffer with m_pszStr at +0x14 and float RGB at +0x30..+0x38.
struct MockGuiString {
    uint8_t raw[0x60];

    MockGuiString(const char* str, float r = rmt::kTealText[0], float g = rmt::kTealText[1], float b = rmt::kTealText[2]) {
        memset(raw, 0, sizeof(raw));
        memcpy(raw + rmt::kStrOff_PszStr, &str, sizeof(const char*));
        memcpy(raw + rmt::kStrOff_ColorR, &r, sizeof(float));
        memcpy(raw + rmt::kStrOff_ColorG, &g, sizeof(float));
        memcpy(raw + rmt::kStrOff_ColorB, &b, sizeof(float));
    }
};

constexpr uint32_t kAlphaE0     = 0xE0000000u;
constexpr uint32_t kDefaultBase = kAlphaE0 | rmt::kRgbTealText;

void testTV1_HeaderLine() {
    const char* s = "Attributes Required:\n";
    MockGuiString gs(s);
    const rmt::LineSpan ls = rmt::classifyLine(s);
    kt::Check("TV1: classified as Header", ls.kind == rmt::LineKind::Header);
    kt::CheckEq("TV1: single span", ls.nSpans, static_cast<uint16_t>(1));

    bool allCream = true;
    for (size_t i = 0; s[i] && s[i] != '\n'; ++i) {
        const uint32_t col = rmt::resolveGlyphColor(gs.raw, s + i, 0, kDefaultBase);
        if (col != (kAlphaE0 | rmt::kRgbCream)) allCream = false;
    }
    kt::Check("TV1: all chars on Header line colored kRgbCream with caller alpha preserved", allCream);
}

void testTV2_KeyValueBasic() {
    const char* s = "Damage: Energy, 4-16\n";
    MockGuiString gs(s);
    const rmt::LineSpan ls = rmt::classifyLine(s);
    kt::Check("TV2: classified as KeyValue", ls.kind == rmt::LineKind::KeyValue);
    kt::CheckEq("TV2: colonPos at index 6", ls.colonPos, static_cast<uint16_t>(6));

    // "Damage:" (indices 0..6) -> kRgbTealDim
    bool keyOk = true;
    for (size_t i = 0; i <= 6; ++i) {
        if (rmt::resolveGlyphColor(gs.raw, s + i, 0, kDefaultBase) != (kAlphaE0 | rmt::kRgbTealDim)) keyOk = false;
    }
    kt::Check("TV2: 'Damage:' colored kRgbTealDim", keyOk);

    // " Energy, 4-16" (indices 7..19) -> kRgbCream ("-16" inside dice range is NOT red)
    bool valOk = true;
    for (size_t i = 7; s[i] && s[i] != '\n'; ++i) {
        if (rmt::resolveGlyphColor(gs.raw, s + i, 0, kDefaultBase) != (kAlphaE0 | rmt::kRgbCream)) valOk = false;
    }
    kt::Check("TV2: ' Energy, 4-16' colored kRgbCream ('-16' in range NOT red)", valOk);
}

void testTV3_KeyValueBonusPenalty() {
    const char* s = "Attack Modifier: +3 (-2 Offhand)\n";
    MockGuiString gs(s);
    const rmt::LineSpan ls = rmt::classifyLine(s);
    kt::Check("TV3: classified as KeyValue", ls.kind == rmt::LineKind::KeyValue);

    // "Attack Modifier:" (0..15) -> kRgbTealDim
    bool keyOk = true;
    for (size_t i = 0; i <= 15; ++i) {
        if (rmt::resolveGlyphColor(gs.raw, s + i, 0, kDefaultBase) != (kAlphaE0 | rmt::kRgbTealDim)) keyOk = false;
    }
    kt::Check("TV3: 'Attack Modifier:' colored kRgbTealDim", keyOk);

    // "+3" at indices 17..18 -> kRgbBonusAmber
    const char* plus3 = strstr(s, "+3");
    kt::Check("TV3: '+3' colored kRgbBonusAmber",
              plus3 &&
              rmt::resolveGlyphColor(gs.raw, plus3, 0, kDefaultBase) == (kAlphaE0 | rmt::kRgbBonusAmber) &&
              rmt::resolveGlyphColor(gs.raw, plus3 + 1, 0, kDefaultBase) == (kAlphaE0 | rmt::kRgbBonusAmber));

    // "(" before "-2" -> kRgbCream; "-2" -> kRgbPenaltyRed; " Offhand)" -> kRgbCream
    const char* paren = strchr(s, '(');
    kt::Check("TV3: '(' is kRgbCream",
              paren && rmt::resolveGlyphColor(gs.raw, paren, 0, kDefaultBase) == (kAlphaE0 | rmt::kRgbCream));
    kt::Check("TV3: '-2' is kRgbPenaltyRed",
              paren &&
              rmt::resolveGlyphColor(gs.raw, paren + 1, 0, kDefaultBase) == (kAlphaE0 | rmt::kRgbPenaltyRed) &&
              rmt::resolveGlyphColor(gs.raw, paren + 2, 0, kDefaultBase) == (kAlphaE0 | rmt::kRgbPenaltyRed));
    kt::Check("TV3: ' Offhand)' is kRgbCream",
              paren && rmt::resolveGlyphColor(gs.raw, paren + 4, 0, kDefaultBase) == (kAlphaE0 | rmt::kRgbCream));

    // Comma-separated penalty without space ("+2 Str,-1 Dex")
    const char* commaPen = "Modifiers: +2 Str,-1 Dex\n";
    MockGuiString gsComma(commaPen);
    const char* minus1 = strstr(commaPen, "-1");
    kt::Check("TV3b: comma-preceded '-1' is kRgbPenaltyRed",
              minus1 && rmt::resolveGlyphColor(gsComma.raw, minus1, 0, kDefaultBase) == (kAlphaE0 | rmt::kRgbPenaltyRed));
}

void testTV4_WrappedContinuation() {
    // 4a: Soft wrap (CAurGUIStringInternal wraps across 2 visual lines without inserting '\n')
    const char* soft = "Immunity: Mind-Affecting, Poison, Critical Hits";
    MockGuiString gsSoft(soft);
    const char* critSoft = strstr(soft, "Critical Hits");
    kt::Check("TV4a (soft wrap): line 2 scans back to 'Immunity: ' -> kRgbCream",
              critSoft && rmt::resolveGlyphColor(gsSoft.raw, critSoft, 1, kDefaultBase) == (kAlphaE0 | rmt::kRgbCream));
    // Also verify hyphenated word "Mind-Affecting" is NOT penalty red
    const char* hyphen = strchr(soft, '-');
    kt::Check("TV4a: hyphen in 'Mind-Affecting' is kRgbCream, not red",
              hyphen && rmt::resolveGlyphColor(gsSoft.raw, hyphen, 0, kDefaultBase) == (kAlphaE0 | rmt::kRgbCream));

    // 4b: Indented hard wrap ("Immunity: Mind-Affecting, Poison,\n  Critical Hits")
    const char* hard = "Immunity: Mind-Affecting, Poison,\n  Critical Hits";
    MockGuiString gsHard(hard);
    const char* critHard = strstr(hard, "Critical Hits");
    kt::Check("TV4b (indented continuation): colored kRgbCream",
              critHard && rmt::resolveGlyphColor(gsHard.raw, critHard, 1, kDefaultBase) == (kAlphaE0 | rmt::kRgbCream));
}

void testTV5_ProseParagraph() {
    const char* s = "This blaster pistol was common during the Mandalorian Wars. It fires bolts: fast and accurate.";
    MockGuiString gs(s);
    const rmt::LineSpan ls = rmt::classifyLine(s);
    kt::Check("TV5: classified as Prose despite colon later in paragraph", ls.kind == rmt::LineKind::Prose);

    bool allTeal = true;
    for (size_t i = 0; s[i]; ++i) {
        if (rmt::resolveGlyphColor(gs.raw, s + i, 0, kDefaultBase) != (kAlphaE0 | rmt::kRgbTealText)) allTeal = false;
    }
    kt::Check("TV5: entire prose paragraph retains kRgbTealText", allTeal);

    // Short sentence with '.' before ':' within first 32 chars
    const char* shortProse = "Wait. Here: take this blaster.";
    const rmt::LineSpan ls2 = rmt::classifyLine(shortProse);
    kt::Check("TV5b: sentence punctuation before ':' within 32 chars -> Prose", ls2.kind == rmt::LineKind::Prose);
}

void testTV6_NonTealPassthrough() {
    const char* s = "Damage: Energy, 4-16\n";
    MockGuiString gsAmber(s, rmt::kBonusAmber[0], rmt::kBonusAmber[1], rmt::kBonusAmber[2]);
    const uint32_t amberPacked = rmt::packRGB(242, 168, 76, 0xFF);
    kt::CheckEq("TV6: non-teal base color (amber) passes through unchanged on key",
                rmt::resolveGlyphColor(gsAmber.raw, s, 0, amberPacked), amberPacked);
    kt::CheckEq("TV6: non-teal base color (amber) passes through unchanged on value",
                rmt::resolveGlyphColor(gsAmber.raw, s + 8, 0, amberPacked), amberPacked);
}

void testTV7_SectionSplitItem() {
    const char* itemDesc =
        "{08}Fully Upgradeable\n"
        "Damage: Energy, 4-16\n"
        "Critical Threat: 19-20,x2\n"
        "Attack Modifier: +3\n\n"
        "This blaster pistol was common during the Mandalorian Wars.";
    rmt::DescSections sec;
    const bool ok = rmt::splitDescSections(itemDesc, &sec);
    kt::Check("TV7: splitDescSections succeeds", ok);
    kt::Check("TV7: strips {08} and extracts subtitle 'Fully Upgradeable'",
              strcmp(sec.subtitle, "Fully Upgradeable") == 0);
    kt::Check("TV7: rules block contains Damage and Attack Modifier",
              strstr(sec.rules, "Damage: Energy, 4-16") != nullptr &&
              strstr(sec.rules, "Attack Modifier: +3") != nullptr);
    kt::Check("TV7: lore body separated from rules",
              strcmp(sec.body, "This blaster pistol was common during the Mandalorian Wars.") == 0);
    kt::Check("TV7: has top and bottom 2px dividers", sec.hasTopDivider && sec.hasBottomDivider && sec.nDividers == 2u);
}

void testTV8_SectionSplitJournal() {
    const char* journalDesc =
        "Nar Shaddaa:\n"
        "Refugee Sector troubles have escalated since the Exchange tightened its grip on the landing pad.";
    rmt::DescSections sec;
    const bool ok = rmt::splitDescSections(journalDesc, &sec);
    kt::Check("TV8: splitDescSections succeeds on Journal text", ok);
    kt::Check("TV8: extracts 'Nar Shaddaa' into title/subtitle",
              strcmp(sec.title, "Nar Shaddaa") == 0 && strcmp(sec.subtitle, "Nar Shaddaa") == 0);
    kt::Check("TV8: rules block is empty for pure journal entry", sec.rules[0] == 0);
    kt::Check("TV8: body holds journal prose",
              strstr(sec.body, "Refugee Sector troubles") != nullptr);
    kt::Check("TV8: has 1 divider (top divider between header and body)",
              sec.hasTopDivider && !sec.hasBottomDivider && sec.nDividers == 1u);
}

void testItemPropsParser() {
    char stripped[128];
    itemprops::stripTags(stripped, sizeof(stripped), "{light 04}  Mandalorian   Combat Suit ");
    kt::Check("ItemProps: stripTags removes {light 04} and collapses spaces",
              strcmp(stripped, "Mandalorian Combat Suit") == 0);

    char dice[64];
    itemprops::diceFix(dice, sizeof(dice), "2d6 Physical + 1d10 Ion");
    kt::Check("ItemProps: diceFix converts 2d6 -> 2-12 and 1d10 -> 1-10",
              strcmp(dice, "2-12 Physical + 1-10 Ion") == 0);

    char dc[64];
    itemprops::fixDC(dc, sizeof(dc), "Stun 25% for 2 rounds, DC22 Fortitude");
    kt::Check("ItemProps: fixDC inserts space in DC22 -> DC 22",
              strcmp(dc, "Stun 25% for 2 rounds, DC 22 Fortitude") == 0);

    itemprops::RulesIn in;
    memset(&in, 0, sizeof(in));
    in.baseRow    = 38; // Light Armor
    in.baseAC     = 5;
    in.dexBonus   = 4;
    in.critThreat = 1;
    in.critMult   = 2;
    in.desc       = "Fully Upgradeable\nDamage Immunity: 10% vs. Energy\n\nLight combat suit worn by Mandalorian scouts.";
    snprintf(in.props[0].label, sizeof(in.props[0].label), "DecreaseAC");
    snprintf(in.props[0].val, sizeof(in.props[0].val), "1");
    in.nProps = 1;

    itemprops::ItemTipModel model;
    const bool built = itemprops::buildItemTipModel(in, "{08}Mandalorian Scout Armor", 1, &model);
    kt::Check("ItemProps: buildItemTipModel succeeds", built);
    kt::Check("ItemProps: title stripped of {08}", strcmp(model.title, "Mandalorian Scout Armor") == 0);
    kt::Check("ItemProps: Light Armor category", strcmp(model.sub, "Light Armor") == 0);
    kt::Check("ItemProps: Defense folded with DecreaseAC (+5 - 1 = +4)",
              model.nRules >= 1 && strcmp(model.rules[0].label, "Defense") == 0 && strcmp(model.rules[0].value, "+4") == 0);

    // Weapon with damage words, bonus damage folding, repeated immunity folding, Upgradeable (Scope), and long flavor
    itemprops::RulesIn win;
    memset(&win, 0, sizeof(win));
    win.baseRow    = 12;
    win.numDice    = 2;
    win.die        = 8;
    win.critThreat = 2;
    win.critMult   = 2;
    win.dmgFlags   = 0x1u | (1u << 4);
    win.baseName   = "{01}Blaster Rifle";
    win.dmgWords[0] = "Physical";
    win.dmgWords[4] = "Energy";
    win.desc =
        "Upgradeable (Scope)\n"
        "Bonus Feat:\n"
        "Rapid Shot\n"
        "Damage: None\n\n"
        "A heavy repeating blaster rifle built for sustained fire across open battlefields in the Outer Rim territories where standard carbines fall short of armor penetration requirements and long-range accuracy.";
    snprintf(win.props[0].label, sizeof(win.props[0].label), "Damage");
    snprintf(win.props[0].sub, sizeof(win.props[0].sub), "Ion");
    snprintf(win.props[0].val, sizeof(win.props[0].val), "1d6");
    snprintf(win.props[1].label, sizeof(win.props[1].label), "DamageImmunity");
    snprintf(win.props[1].name, sizeof(win.props[1].name), "Damage Immunity");
    snprintf(win.props[1].sub, sizeof(win.props[1].sub), "Cold");
    snprintf(win.props[1].val, sizeof(win.props[1].val), "10%%");
    snprintf(win.props[2].label, sizeof(win.props[2].label), "DamageImmunity");
    snprintf(win.props[2].name, sizeof(win.props[2].name), "Damage Immunity");
    snprintf(win.props[2].sub, sizeof(win.props[2].sub), "Fire");
    snprintf(win.props[2].val, sizeof(win.props[2].val), "10%%");
    snprintf(win.props[3].label, sizeof(win.props[3].label), "CastSpell");
    snprintf(win.props[4].label, sizeof(win.props[4].label), "UseLimitationRacial");
    snprintf(win.props[4].sub, sizeof(win.props[4].sub), "Human");
    win.nProps = 5;

    itemprops::ItemTipModel wmodel;
    kt::Check("ItemProps: weapon buildItemTipModel succeeds",
              itemprops::buildItemTipModel(win, "Charric Rifle", 1, &wmodel));
    kt::Check("ItemProps: weapon Damage folded and diceFixed",
              wmodel.nRules >= 2 && strcmp(wmodel.rules[0].value, "2-16 Physical/Energy + 1-6 Ion") == 0);
    kt::Check("ItemProps: weapon Critical 19-20 / x2",
              strcmp(wmodel.rules[1].label, "Critical") == 0 && strcmp(wmodel.rules[1].value, "19-20 / x2") == 0);
    kt::Check("ItemProps: weapon Immunity folded 'Cold, Fire 10%'",
              strcmp(wmodel.rules[2].label, "Immunity") == 0 && strcmp(wmodel.rules[2].value, "Cold, Fire 10%") == 0);
    kt::Check("ItemProps: armorWeight Medium and Heavy",
              strcmp(itemprops::armorWeight(40), "Medium Armor") == 0 &&
              strcmp(itemprops::armorWeight(42), "Heavy Armor") == 0);

    // Null / empty safety checks
    kt::Check("RichTextClassifier: null self with default teal packed baseCol works",
              rmt::resolveGlyphColor(nullptr, "Damage: 5", 0, kDefaultBase) == (kAlphaE0 | rmt::kRgbTealDim));
    kt::Check("RichTextClassifier: null curChar returns baseCol",
              rmt::resolveGlyphColor(nullptr, nullptr, 0, kDefaultBase) == kDefaultBase);
}

void testRequirementHeadersWithoutColons() {
    const char* s1 = "FEATS REQUIRED\n";
    MockGuiString gs1(s1);
    const rmt::LineSpan ls1 = rmt::classifyLine(s1);
    kt::Check("ReqHeader: 'FEATS REQUIRED' classified as Header", ls1.kind == rmt::LineKind::Header);

    bool allCream1 = true;
    for (size_t i = 0; s1[i] && s1[i] != '\n'; ++i) {
        if (rmt::resolveGlyphColor(gs1.raw, s1 + i, 0, kDefaultBase) != (kAlphaE0 | rmt::kRgbCream)) allCream1 = false;
    }
    kt::Check("ReqHeader: 'FEATS REQUIRED' colored kRgbCream", allCream1);

    const char* s2 = "FEAT REQUIRED\n";
    MockGuiString gs2(s2);
    const rmt::LineSpan ls2 = rmt::classifyLine(s2);
    kt::Check("ReqHeader: 'FEAT REQUIRED' classified as Header", ls2.kind == rmt::LineKind::Header);

    bool allCream2 = true;
    for (size_t i = 0; s2[i] && s2[i] != '\n'; ++i) {
        if (rmt::resolveGlyphColor(gs2.raw, s2 + i, 0, kDefaultBase) != (kAlphaE0 | rmt::kRgbCream)) allCream2 = false;
    }
    kt::Check("ReqHeader: 'FEAT REQUIRED' colored kRgbCream", allCream2);
}

void testArmorAndRobeSectionSplits() {
    // 1. Item with Title & Category at the top
    const char* armor1 =
        "FEATS REQUIRED\n"
        "Armor Proficiency - Heavy\n\n"
        "Defense Bonus: 12\n"
        "Max Dexterity Bonus: +0\n\n"
        "Mandalorian heavy battle armor.";
    rmt::DescSections sec1;
    const bool ok1 = rmt::splitDescSections(armor1, &sec1);
    kt::Check("ArmorSplit: splitDescSections succeeds for armor1", ok1);
    kt::Check("ArmorSplit: title is 'FEATS REQUIRED'", strcmp(sec1.title, "FEATS REQUIRED") == 0);
    kt::Check("ArmorSplit: subtitle is 'Armor Proficiency - Heavy'", strcmp(sec1.subtitle, "Armor Proficiency - Heavy") == 0);
    kt::Check("ArmorSplit: rules contains Defense Bonus", strstr(sec1.rules, "Defense Bonus: 12") != nullptr);
    kt::Check("ArmorSplit: rules contains Max Dexterity Bonus", strstr(sec1.rules, "Max Dexterity Bonus: +0") != nullptr);
    kt::Check("ArmorSplit: body contains lore text", strcmp(sec1.body, "Mandalorian heavy battle armor.") == 0);
    kt::Check("ArmorSplit: has top and bottom dividers", sec1.hasTopDivider && sec1.hasBottomDivider && sec1.nDividers == 2u);

    // 2. Robe with Cannot Equip, Restricted, and User Limitation in the middle of rules
    const char* robe =
        "Defense Bonus: 2\n"
        "Max Dexterity Bonus: +8\n\n"
        "Cannot Equip:\n"
        "Droid\n\n"
        "Restricted:\n"
        "Jedi Only\n\n"
        "User Limitation:\n"
        "Light Side\n\n"
        "Defense Bonus: +1 vs. Dark Side\n\n"
        "Ancient Jedi Master robes imbued with the Force.";
    rmt::DescSections secRobe;
    const bool okRobe = rmt::splitDescSections(robe, &secRobe);
    kt::Check("RobeSplit: splitDescSections succeeds for robe", okRobe);
    kt::Check("RobeSplit: title is empty", secRobe.title[0] == 0);
    kt::Check("RobeSplit: subtitle is empty", secRobe.subtitle[0] == 0);
    kt::Check("RobeSplit: rules has Defense Bonus: 2", strstr(secRobe.rules, "Defense Bonus: 2") != nullptr);
    kt::Check("RobeSplit: rules has Max Dexterity Bonus", strstr(secRobe.rules, "Max Dexterity Bonus: +8") != nullptr);
    kt::Check("RobeSplit: rules has Cannot Equip: Droid",
              strstr(secRobe.rules, "Cannot Equip:\nDroid") != nullptr);
    kt::Check("RobeSplit: rules has Restricted: Jedi Only",
              strstr(secRobe.rules, "Restricted:\nJedi Only") != nullptr);
    kt::Check("RobeSplit: rules has User Limitation: Light Side",
              strstr(secRobe.rules, "User Limitation:\nLight Side") != nullptr);
    kt::Check("RobeSplit: rules has Defense Bonus: +1 vs. Dark Side (not in body)",
              strstr(secRobe.rules, "Defense Bonus: +1 vs. Dark Side") != nullptr &&
              strstr(secRobe.body, "Defense Bonus: +1 vs. Dark Side") == nullptr);
    kt::Check("RobeSplit: body contains only lore prose",
              strcmp(secRobe.body, "Ancient Jedi Master robes imbued with the Force.") == 0);

    // 3. Armor with requirements inside rules
    const char* armor2 =
        "Defense Bonus: 12\n"
        "Max Dexterity Bonus: +0\n\n"
        "FEATS REQUIRED\n"
        "Armor Proficiency - Heavy\n\n"
        "Heavy battle suit.";
    rmt::DescSections sec2;
    const bool ok2 = rmt::splitDescSections(armor2, &sec2);
    kt::Check("ArmorSplit2: splitDescSections succeeds", ok2);
    kt::Check("ArmorSplit2: rules has Defense Bonus", strstr(sec2.rules, "Defense Bonus: 12") != nullptr);
    kt::Check("ArmorSplit2: rules has FEATS REQUIRED", strstr(sec2.rules, "FEATS REQUIRED\nArmor Proficiency - Heavy") != nullptr);
    kt::Check("ArmorSplit2: body is lore only", strcmp(sec2.body, "Heavy battle suit.") == 0);

    // 4. Verify no unnecessary blank lines in rules
    kt::Check("NoBlankLinesInRules: rules does not have consecutive newlines",
              strstr(secRobe.rules, "\n\n") == nullptr && strstr(sec1.rules, "\n\n") == nullptr);
}

} // namespace

int main() {
    testTV1_HeaderLine();
    testTV2_KeyValueBasic();
    testTV3_KeyValueBonusPenalty();
    testTV4_WrappedContinuation();
    testTV5_ProseParagraph();
    testTV6_NonTealPassthrough();
    testTV7_SectionSplitItem();
    testTV8_SectionSplitJournal();
    testItemPropsParser();
    testRequirementHeadersWithoutColons();
    testArmorAndRobeSectionSplits();
    return kt::Report("rich_text_classifier_test");
}
