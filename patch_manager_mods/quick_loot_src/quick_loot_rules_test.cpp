// quick_loot_rules_test.cpp -- host test of QuickLootRules.h (doc 52 QL9): the tooltip data rules (baseitems damage / critical / defense, itempropdef
// rows, description tag + "Label: value" lines, folding, merging, dice, short flavor) on the six items of scripts/quick_loot_tip_mockup.py (inputs
// captured from the live install by that script) plus edge cases.
#include "QuickLootRules.h"
#include "check.h"
#include <stdio.h>
#include <string.h>

using namespace qlogic;

static RulesIn g_in;
static TipModel g_m;

static void begin(int baseRow, const char* baseName, const char* desc) {
    memset(&g_in, 0, sizeof g_in);
    g_in.baseRow = baseRow; g_in.dexBonus = -1; g_in.critThreat = 1; g_in.critMult = 2;
    g_in.baseName = baseName; g_in.desc = desc;
    g_in.dmgWords[0] = "Physical";
    g_in.dmgWords[4] = "Unstoppable";
    g_in.dmgWords[5] = "Ion";
    g_in.dmgWords[10] = "Sonic";
    g_in.dmgWords[13] = "Sith";
}
static void prop(const char* label, const char* name, const char* sub, const char* val) {
    RulesProp& p = g_in.props[g_in.nProps++];
    memset(&p, 0, sizeof p);
    snprintf(p.label, sizeof p.label, "%s", label);
    snprintf(p.name, sizeof p.name, "%s", name);
    snprintf(p.sub, sizeof p.sub, "%s", sub);
    snprintf(p.val, sizeof p.val, "%s", val);
}
static bool rowIs(unsigned i, const char* label, const char* value) {
    return i < g_m.nRules && strcmp(g_m.rules[i].label, label) == 0 && strcmp(g_m.rules[i].value, value) == 0;
}
static void dump(const char* what) {
    if (!kt::Verbose()) return;
    printf("%s: '%s' x%d sub='%s' full=%d\n", what, g_m.title, g_m.stack, g_m.sub, g_m.full);
    for (unsigned i = 0; i < g_m.nRules; ++i) printf("    %s = %s\n", g_m.rules[i].label, g_m.rules[i].value);
    printf("    flavor: %s\n", g_m.flavor);
}

static void carbine() {
    begin(21, "Disruptor Rifle",
          "Not Upgradeable\n\nThis disruptor is even more destructive than its pistol counterpart, and is outlawed on just as many worlds. Disruptors reduce solid "
          "matter to its constituent molecules. Painfully. Unlike typical blasters, disruptors ignore most types of personal energy shields.");
    g_in.numDice = 1; g_in.die = 10; g_in.critThreat = 3; g_in.critMult = 2; g_in.dmgFlags = 16;
    kt::Check("carbine: builds", buildTipModel(g_in, "{08}Disruptor Carbine", 1, &g_m));
    dump("carbine");
    kt::CheckEq("carbine: title without the tag", g_m.title, "Disruptor Carbine");
    kt::CheckEq("carbine: stack", g_m.stack, 1);
    kt::CheckEq("carbine: sub is the base item name", g_m.sub, "Disruptor Rifle");
    kt::CheckEq("carbine: two rows (Not Upgradeable adds none)", g_m.nRules, 2u);
    kt::Check("carbine: Damage 1-10 Unstoppable (never Energy)", rowIs(0, "Damage", "1-10 Unstoppable"));
    kt::Check("carbine: Critical 18-20 / x2", rowIs(1, "Critical", "18-20 / x2"));
    kt::CheckEq("carbine: first sentence only", g_m.flavor,
                "This disruptor is even more destructive than its pistol counterpart, and is outlawed on just as many worlds.");
    kt::Check("carbine: not full, no foot", !g_m.full && g_m.foot[0] == 0);
}

static void suit() {
    begin(38, "Defense Bonus 4", "Even the basic combat attire of the Mandalorians provides a formidable defense. The mesh of this armor absorbs some of the impact of physical blows despite its light weight.");
    g_in.baseAC = 4; g_in.dexBonus = 5;
    prop("DamageImmunity", "Damage Immunity", "Bludgeoning", "10%");
    prop("DamageImmunity", "Damage Immunity", "Piercing", "10%");
    prop("DamageImmunity", "Damage Immunity", "Slashing", "10%");
    prop("DecreaseAC", "Defense Penalty", "DB Armor", "-1");
    prop("UseLimitationRacial", "User Limitation", "Human", "");
    kt::Check("suit: builds", buildTipModel(g_in, "{light 04}Mandalorian Combat Suit", 1, &g_m));
    dump("suit");
    kt::CheckEq("suit: title", g_m.title, "Mandalorian Combat Suit");
    kt::CheckEq("suit: sub is the weight class", g_m.sub, "Light Armor");
    kt::CheckEq("suit: three rows (penalty folded, Human restriction hidden)", g_m.nRules, 3u);
    kt::Check("suit: Defense +3", rowIs(0, "Defense", "+3"));
    kt::Check("suit: Max Dex +5", rowIs(1, "Max Dex", "+5"));
    kt::Check("suit: Immunity merged", rowIs(2, "Immunity", "Bludgeoning, Piercing, Slashing 10%"));
    kt::CheckEq("suit: first sentence only (>= 70 chars)", g_m.flavor, "Even the basic combat attire of the Mandalorians provides a formidable defense.");
}

static void medpac() {
    const char* d = "A medpac contains essential equipment for the treatment of wounds. They cannot be used by droids. Basic medpacs heal 10 vitality points + WIS modifier + user's skill in Treat Injury.";
    begin(55, "Medical Equipment", d);
    prop("CastSpell", "Activate Item", "Medpac I", "Single Use");
    kt::Check("medpac: builds", buildTipModel(g_in, "Medpac", 5, &g_m));
    dump("medpac");
    kt::CheckEq("medpac: stack", g_m.stack, 5);
    kt::CheckEq("medpac: no rules", g_m.nRules, 0u);
    kt::Check("medpac: full description", g_m.full && strcmp(g_m.flavor, d) == 0);
    kt::CheckEq("medpac: sub", g_m.sub, "Medical Equipment");
}

static void sonic() {
    begin(22, "Sonic Rifle", "Upgradeable (Scope)\n\nCombining both sonic and disruptor attacks, this rifle tears through enemy defenses.");
    g_in.numDice = 1; g_in.die = 10; g_in.critThreat = 1; g_in.critMult = 2; g_in.dmgFlags = 1024;
    prop("Damage", "Damage Bonus", "Unstoppable", "1d10");
    prop("OnHit", "On Hit", "Attribute Damage", "DC 22");
    prop("OnHit", "On Hit", "Stun", "DC 18");
    kt::Check("sonic: builds", buildTipModel(g_in, "{27}Sonic Disruptor", 1, &g_m));
    dump("sonic");
    kt::CheckEq("sonic: four rows", g_m.nRules, 4u);
    kt::Check("sonic: Damage with the folded 1d10 as 1-10", rowIs(0, "Damage", "1-10 Sonic + 1-10 Unstoppable"));
    kt::Check("sonic: Critical 20 / x2", rowIs(1, "Critical", "20 / x2"));
    kt::Check("sonic: On Hit merged", rowIs(2, "On Hit", "Attribute Damage DC 22, Stun DC 18"));
    kt::Check("sonic: Upgrades Scope only", rowIs(3, "Upgrades", "Scope only"));
    kt::CheckEq("sonic: flavor", g_m.flavor, "Combining both sonic and disruptor attacks, this rifle tears through enemy defenses.");
}

static void grenade() {
    begin(26, "Stun Grenade",
          "Damage: None\nSecondary: Stun for 9sec\nArea of Effect: 4m\nRange: Long\nSave (Will):\nDC15 for no effect\n\nThis type of grenade explodes in a concussive wave of force that "
          "disrupts the senses of both organic and inorganic targets in the area of effect.");
    prop("CastSpell", "Activate Item", "Grenade: Stun", "Single Use");
    kt::Check("grenade: builds", buildTipModel(g_in, "Concussion Grenade", 1, &g_m));
    dump("grenade");
    kt::CheckEq("grenade: four rows, no Damage: None", g_m.nRules, 4u);
    kt::Check("grenade: Effect", rowIs(0, "Effect", "Stun for 9sec"));
    kt::Check("grenade: Area", rowIs(1, "Area", "4m"));
    kt::Check("grenade: Range from the description", rowIs(2, "Range", "Long"));
    kt::Check("grenade: Will Save with DC spaced, bare label takes the next line", rowIs(3, "Will Save", "DC 15 for no effect"));
    kt::Check("grenade: flavor is the sentence after the stats", strncmp(g_m.flavor, "This type of grenade explodes", 29) == 0);
    kt::CheckEq("grenade: sub", g_m.sub, "Stun Grenade");
}

static void vibroblade() {
    begin(5, "Vibro Blade",
          "Fully Upgradeable\n\nThough believed to have been constructed by the Zabrak, this vibroblade is rarely used by them. The Zabrak feel their combat skills sufficient to "
          "make this weapon's fine balance irrelevant.\n\nIts small size makes this a good off-hand weapon.");
    g_in.numDice = 1; g_in.die = 10; g_in.critThreat = 2; g_in.critMult = 2; g_in.dmgFlags = 2;
    prop("BonusFeats", "Bonus Feat", "Finesse: Melee Weapons", "");
    prop("Damage", "Damage Bonus", "Unstoppable", "2");
    kt::Check("vibroblade: builds", buildTipModel(g_in, "{10}Zabrak Vibroblade", 1, &g_m));
    dump("vibroblade");
    kt::CheckEq("vibroblade: four rows", g_m.nRules, 4u);
    kt::Check("vibroblade: Damage 1-10 Physical + 2 Unstoppable", rowIs(0, "Damage", "1-10 Physical + 2 Unstoppable"));
    kt::Check("vibroblade: Critical 19-20 / x2", rowIs(1, "Critical", "19-20 / x2"));
    kt::Check("vibroblade: Bonus Feat", rowIs(2, "Bonus Feat", "Finesse: Melee Weapons"));
    kt::Check("vibroblade: Upgrades All slots", rowIs(3, "Upgrades", "All slots"));
    kt::CheckEq("vibroblade: flavor is the first sentence", g_m.flavor,
                "Though believed to have been constructed by the Zabrak, this vibroblade is rarely used by them.");
}

static void edges() {
    // null out
    begin(0, nullptr, nullptr);
    kt::Check("null out -> false", !buildTipModel(g_in, "x", 1, nullptr));
    // null desc / names / base name
    kt::Check("null desc builds", buildTipModel(g_in, nullptr, -1, &g_m));
    kt::Check("null everything: empty title, sub, no rules, full empty flavor", g_m.title[0] == 0 && g_m.sub[0] == 0 && g_m.nRules == 0 && g_m.full && g_m.flavor[0] == 0);
    kt::CheckEq("stack -1 passes through", g_m.stack, -1);
    // armor weights
    const int rows[] = { 38, 39, 40, 41, 42, 43, 44 };
    const char* want[] = { "Light Armor", "Light Armor", "Medium Armor", "Medium Armor", "Heavy Armor", "Heavy Armor", "Base" };
    for (int i = 0; i < 7; ++i) {
        begin(rows[i], "Base", nullptr);
        buildTipModel(g_in, "a", 1, &g_m);
        kt::CheckEq("armor weight by row", g_m.sub, want[i]);
    }
    // 16 props, all distinct: capped at 10 rows; every row still sane
    begin(0, "x", nullptr);
    for (int i = 0; i < 16; ++i) { char n[16]; snprintf(n, sizeof n, "Prop%d", i); prop("Foo", n, "s", "v"); }
    buildTipModel(g_in, "many", 1, &g_m);
    kt::CheckEq("16 props -> 10 rules", g_m.nRules, 10u);
    kt::Check("first and last kept rows", rowIs(0, "Prop0", "s v") && rowIs(9, "Prop9", "s v"));
    // more props than the array: nProps clamps
    g_in.nProps = 99;
    kt::Check("nProps beyond 16 is clamped", buildTipModel(g_in, "many", 1, &g_m) && g_m.nRules == 10);
    // prop without sub/val -> Yes; ability label mapped; Human limitation hidden only for Human
    begin(0, "x", nullptr);
    prop("AbilityBonus", "Ability Bonus", "Strength", "+2");
    prop("Foo", "Plain", "", "");
    prop("UseLimitationRacial", "User Limitation", "Wookiee", "");
    prop("UseLimitationRacial", "User Limitation", "Human", "");
    prop("CastSpell", "Activate Item", "Human", "");
    buildTipModel(g_in, "p", 1, &g_m);
    kt::CheckEq("mapped labels, Yes, Human hidden", g_m.nRules, 3u);
    kt::Check("Ability row", rowIs(0, "Ability", "Strength +2"));
    kt::Check("Yes row", rowIs(1, "Plain", "Yes"));
    kt::Check("non-Human restriction kept as Restricted", rowIs(2, "Restricted", "Wookiee"));
    // dice on non-damage values, DC spacing, word-boundary safety
    begin(0, "x", nullptr);
    prop("OnHit", "On Hit", "2d6 fire", "x1d10 4D8");
    prop("Foo", "Num", "", "12 and 3d");
    buildTipModel(g_in, "d", 1, &g_m);
    kt::Check("2d6 -> 2-12, 4D8 -> 4-32, x1d10 untouched", rowIs(0, "On Hit", "2-12 fire x1d10 4-32"));
    kt::Check("plain numbers and 3d untouched", rowIs(1, "Num", "12 and 3d"));
    // defense penalty without a Defense row stays a normal row; penalty past zero goes negative
    begin(0, "x", nullptr);
    prop("DecreaseAC", "Defense Penalty", "DB Armor", "-1");
    buildTipModel(g_in, "d", 1, &g_m);
    kt::Check("DecreaseAC without Defense: a row", rowIs(0, "Defense Penalty", "DB Armor -1"));
    begin(38, "x", nullptr);
    g_in.baseAC = 1;
    prop("DecreaseAC", "Defense Penalty", "DB", "-3");
    buildTipModel(g_in, "d", 1, &g_m);
    kt::Check("Defense below zero shows -2", rowIs(0, "Defense", "-2"));
    // no Damage fold target: the bonus is a row of its own
    begin(0, "x", nullptr);
    prop("Damage", "Damage Bonus", "Unstoppable", "2");
    buildTipModel(g_in, "d", 1, &g_m);
    kt::Check("Damage bonus without a base Damage is its own row", rowIs(0, "Damage Bonus", "Unstoppable 2"));
    // damage words: bits 0-2 once, joined with "/", null / empty skipped, Disruptor never Energy
    begin(21, "w", nullptr);
    g_in.numDice = 2; g_in.die = 6; g_in.dmgFlags = 0x7 | (1u << 5) | (1u << 6) | (1u << 13);
    g_in.dmgWords[6] = "";   // empty word skipped
    buildTipModel(g_in, "w", 1, &g_m);
    kt::Check("2d6 -> 2-12, words joined by /", rowIs(0, "Damage", "2-12 Physical/Ion/Sith"));
    g_in.dmgFlags = 0;
    buildTipModel(g_in, "w", 1, &g_m);
    kt::Check("no flags: dice only", rowIs(0, "Damage", "2-12"));
    // crit range: threat 1 -> 20, threat 20 -> 1-20, multiplier default
    g_in.critThreat = 0; g_in.critMult = 0;
    buildTipModel(g_in, "w", 1, &g_m);
    kt::Check("threat 0 / mult 0 -> 20 / x2", rowIs(1, "Critical", "20 / x2"));
    g_in.critThreat = 99; g_in.critMult = 3;
    buildTipModel(g_in, "w", 1, &g_m);
    kt::Check("threat clamped to 20", rowIs(1, "Critical", "1-20 / x3"));
    // armor with dexBonus 99 or -1 shows no Max Dex
    begin(40, "a", nullptr);
    g_in.baseAC = 7; g_in.dexBonus = 99;
    buildTipModel(g_in, "a", 1, &g_m);
    kt::CheckEq("dex 99: Defense only", g_m.nRules, 1u);
    // description tag kinds
    begin(0, "x", "Upgradeable\n\nSome text.");
    buildTipModel(g_in, "t", 1, &g_m);
    kt::Check("plain Upgradeable -> Yes", rowIs(0, "Upgrades", "Yes"));
    begin(0, "x", "Upgradeable (Scope, Lightsaber)\n\nSome text.");
    buildTipModel(g_in, "t", 1, &g_m);
    kt::Check("two slots", rowIs(0, "Upgrades", "Scope, Lightsaber only"));
    begin(0, "x", "Not Upgradeable (Special)\n\nSome text.");
    buildTipModel(g_in, "t", 1, &g_m);
    kt::CheckEq("Not -> no row", g_m.nRules, 0u);
    // CRLF, tag stripped from description lines, label with a colon-less description, stat rows merging by label
    begin(0, "x", "{a}Fully Upgradeable\r\n{b}Damage: 2d6 Energy\r\nDamage: None\r\nSave (Fort): DC22\r\nSave (Fort): DC18\r\n\r\nFlavor here. More.");
    buildTipModel(g_in, "t", 1, &g_m);
    dump("crlf");
    kt::Check("CRLF: Damage kept (not None), dice converted", rowIs(0, "Damage", "2-12 Energy"));
    kt::Check("CRLF: second Damage: None dropped, saves merged", rowIs(1, "Fort Save", "DC 22, DC 18") || rowIs(1, "Fort Save", "DC 22, 18"));
    kt::Check("CRLF: Upgrades", rowIs(2, "Upgrades", "All slots"));
    kt::CheckEq("CRLF: flavor", g_m.flavor, "Flavor here. More.");
    // a lone first line with a colon is a stat; a sentence first line is not a tag
    begin(0, "x", "Short sentence.\nLabel: value");
    buildTipModel(g_in, "t", 1, &g_m);
    kt::CheckEq("sentence first line is flavor, so no rows", g_m.nRules, 0u);
    kt::Check("... and the whole text stays (full)", g_m.full && strcmp(g_m.flavor, "Short sentence. Label: value") == 0);
    // unknown (over-long) tags are kept, 12-char tags are removed
    begin(0, "x", nullptr);
    buildTipModel(g_in, "{this tag is longer than twelve}Name", 1, &g_m);
    kt::CheckEq("long tag kept", g_m.title, "{this tag is longer than twelve}Name");
    buildTipModel(g_in, "{123456789012}Name", 1, &g_m);
    kt::CheckEq("12-char tag removed", g_m.title, "Name");
    buildTipModel(g_in, "{unclosed Name", 1, &g_m);
    kt::CheckEq("unclosed brace kept", g_m.title, "{unclosed Name");
    // overlong strings
    char big[1500];
    memset(big, 'x', sizeof big - 1); big[sizeof big - 1] = 0;
    for (int i = 0; i < 40; ++i) big[i * 30 + 29] = ' ';
    begin(0, big, big);
    buildTipModel(g_in, big, 1, &g_m);
    kt::Check("overlong name / sub / desc: all NUL-terminated and cut", strlen(g_m.title) == 47 && strlen(g_m.sub) < 40 && strlen(g_m.flavor) < 600);
    kt::Check("overlong desc without rules is full and cut at the buffer", g_m.full && strlen(g_m.flavor) == 599);
    // a field with no NUL at all must not be read past its size
    begin(0, "x", nullptr);
    RulesProp& p = g_in.props[g_in.nProps++];
    memset(&p, 'z', sizeof p);
    buildTipModel(g_in, "n", 1, &g_m);
    kt::Check("unterminated prop fields: one row, bounded", g_m.nRules == 1 && strlen(g_m.rules[0].label) < 16 && strlen(g_m.rules[0].value) < 96);
    // long single sentence cut on a word; no-terminator text cut at 170
    char longs[400];
    longs[0] = 0;
    for (int i = 0; i < 40; ++i) strcat(longs, "word ");
    strcat(longs, "end.");
    begin(0, "x", nullptr);
    g_in.desc = longs;
    prop("Foo", "Row", "", "");
    buildTipModel(g_in, "s", 1, &g_m);
    kt::Check("long sentence cut with ...", strlen(g_m.flavor) <= 170 && strcmp(g_m.flavor + strlen(g_m.flavor) - 3, "...") == 0);
    char nosent[400];
    memset(nosent, 'q', sizeof nosent - 1); nosent[sizeof nosent - 1] = 0;
    g_in.desc = nosent;
    buildTipModel(g_in, "s", 1, &g_m);
    kt::CheckEq("no sentence end: cut at 170", strlen(g_m.flavor), 170u);
    // two short sentences join; a third is dropped; short sentence + long second stays alone
    g_in.desc = "One short. Two short. Three short.";
    buildTipModel(g_in, "s", 1, &g_m);
    kt::CheckEq("two short sentences", g_m.flavor, "One short. Two short.");
    char s2[400];
    snprintf(s2, sizeof s2, "Short one. %s.", "this second sentence is long and keeps going on and on and on and on and on and on and on and on and on and on and on and on and on and on and on and on and on and on and on and on and on and on and on and on");
    g_in.desc = s2;
    buildTipModel(g_in, "s", 1, &g_m);
    kt::CheckEq("second sentence too long: first only", g_m.flavor, "Short one.");
    g_in.desc = "Version 3.5 is out. Done.";
    buildTipModel(g_in, "s", 1, &g_m);
    kt::CheckEq("a '.' inside a number is not a sentence end", g_m.flavor, "Version 3.5 is out. Done.");
    // truncation of a merged value stays terminated
    begin(0, "x", nullptr);
    for (int i = 0; i < 12; ++i) prop("Foo", "Same", "abcdefgh ijklmnop", "");
    buildTipModel(g_in, "m", 1, &g_m);
    kt::Check("merged value cut safely", g_m.nRules == 1 && strlen(g_m.rules[0].value) <= 95);
}

int main() {
    carbine();
    suit();
    medpac();
    sonic();
    grenade();
    vibroblade();
    edges();
    return kt::Report("quick_loot_rules_test");
}
