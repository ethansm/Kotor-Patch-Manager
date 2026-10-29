// QuickLootRules.h -- the data rules of the 0.7.0-dev (QL9) tooltip: item facts (baseitems.2da row + itempropdef names + the description text) in,
// TipModel (QuickLootLogic.h) out. A port of scripts/quick_loot_tip_mockup.py (dmg_words, dice_range, base_rules, armor_class, prop_rules,
// LABELS, split_desc, short_flavor, build_rules). No engine calls, no heap, no std::string: fixed buffers, host-testable
// (quick_loot_rules_test.cpp; build with -I../_shared_src).
#pragma once
#include "QuickLootLogic.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace qlogic {

// One item property, already resolved to text by the caller: label = itempropdef.2da 'label' column ("Damage", "DecreaseAC", "BonusFeats",
// "CastSpell", "UseLimitationRacial"), name = its TLK name, sub = the subtype's TLK name, val = the cost-table value's TLK name ("" = none).
struct RulesProp { char label[24]; char name[32]; char sub[32]; char val[32]; };

// baseRow = baseitems.2da row; numDice / die / critThreat / critMult / baseAC / dexBonus / dmgFlags = its numdice / dietoroll / critthreat /
// crithitmult / baseac / dexbonus / damageflags columns; desc = DescIdentified text (null = none); baseName = the baseitems TLK name;
// dmgWords[0] = word for any of bits 0-2 (TLK 38552 "Physical"), [b] for bit b = 3..12 (TLK 38553 + b - 3), [13] for bit 13 (0xA3AF).
struct RulesIn {
    int baseRow, numDice, die, critThreat, critMult, baseAC, dexBonus;
    uint32_t dmgFlags;
    RulesProp props[16];
    unsigned nProps;
    const char* desc;
    const char* baseName;
    const char* dmgWords[14];
};

namespace rules_detail {

constexpr unsigned MaxRules = 10;      // TipModel::rules
constexpr unsigned LineCap = 640;      // one description line after tag stripping
constexpr unsigned FlavorCap = 600;    // TipModel::flavor
constexpr unsigned ShortFlavorMax = 170;

inline bool isDigit(char c) { return c >= '0' && c <= '9'; }
inline bool isWord(char c) { return isDigit(c) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
inline char lower(char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c; }

inline bool eqNoCase(const char* a, const char* b) {
    for (; *a && *b; ++a, ++b) if (lower(*a) != lower(*b)) return false;
    return *a == *b;
}

// Copies at most cap-1 chars of src (null = "") and terminates.
inline void copyTrunc(char* dst, unsigned cap, const char* src) {
    unsigned n = 0;
    if (src) while (src[n] && n + 1 < cap) { dst[n] = src[n]; ++n; }
    dst[n] = 0;
}

// stripTags of a fixed-size field that might not be NUL-terminated: reads at most inCap chars.
inline void cleanField(char* out, unsigned cap, const char* in, unsigned inCap) {
    char tmp[48];
    unsigned n = 0;
    while (n < inCap && n + 1 < sizeof tmp && in[n]) { tmp[n] = in[n]; ++n; }
    tmp[n] = 0;
    stripTags(out, cap, tmp);
}

// BioWare's own row names -> the short tooltip labels (mockup LABELS).
inline const char* mapLabel(const char* k) {
    static const char* const pairs[][2] = {
        { "Damage Immunity", "Immunity" }, { "User Limitation", "Restricted" }, { "Activate Item", "Use" }, { "Area of Effect", "Area" },
        { "Save (Will)", "Will Save" }, { "Save (Reflex)", "Reflex Save" }, { "Save (Fort)", "Fort Save" }, { "Secondary", "Effect" },
        { "Attack Bonus", "Attack" }, { "Ability Bonus", "Ability" }, { "Skill Bonus", "Skill" } };
    for (const auto& p : pairs) if (strcmp(k, p[0]) == 0) return p[1];
    return k;
}

// "1d10" reads as 1010 in the game font: every NdM becomes "N-(N*M)" ("2d6" -> "2-12"). Whole-word match like the mockup's \b(\d+)d(\d+)\b.
inline void diceFix(char* out, unsigned cap, const char* in) {
    unsigned n = 0;
    unsigned i = 0;
    while (in[i] && n + 1 < cap) {
        if (isDigit(in[i]) && (i == 0 || !isWord(in[i - 1]))) {
            unsigned j = i;
            unsigned long long a = 0;
            while (isDigit(in[j])) { if (j - i < 9) a = a * 10 + static_cast<unsigned>(in[j] - '0'); ++j; }
            if ((in[j] == 'd' || in[j] == 'D') && isDigit(in[j + 1]) && j - i <= 9) {
                unsigned k = j + 1;
                unsigned long long b = 0;
                while (isDigit(in[k])) { if (k - j - 1 < 9) b = b * 10 + static_cast<unsigned>(in[k] - '0'); ++k; }
                if (!isWord(in[k]) && k - j - 1 <= 9) {
                    char tmp[48];
                    snprintf(tmp, sizeof tmp, "%llu-%llu", a, a * b);
                    for (unsigned t = 0; tmp[t] && n + 1 < cap; ++t) out[n++] = tmp[t];
                    i = k;
                    continue;
                }
            }
            while (i < j && n + 1 < cap) out[n++] = in[i++];   // a plain number: copy the digit run
            continue;
        }
        out[n++] = in[i++];
    }
    out[n] = 0;
}

// "DC22" -> "DC 22" (mockup re.sub(r"\bDC(\d)", r"DC \1")).
inline void fixDC(char* out, unsigned cap, const char* in) {
    unsigned n = 0;
    for (unsigned i = 0; in[i] && n + 1 < cap; ++i) {
        out[n++] = in[i];
        if (in[i] == 'D' && in[i + 1] == 'C' && isDigit(in[i + 2]) && (i == 0 || !isWord(in[i - 1]))) {
            out[n++] = 'C';
            if (n + 1 < cap) out[n++] = ' ';
            ++i;
        }
    }
    out[n] = 0;
}

inline const char* armorWeight(int row) {
    switch (row) {
        case 38: case 39: return "Light Armor";
        case 40: case 41: return "Medium Armor";
        case 42: case 43: return "Heavy Armor";
        default: return nullptr;
    }
}

// Damage words joined with "/": Physical (bits 0-2), bits 3..12, bit 13. Null / empty words are skipped.
inline void joinDmgWords(char* out, unsigned cap, uint32_t flags, const char* const* w) {
    unsigned n = 0;
    out[0] = 0;
    for (unsigned b = 0; b < 14; ++b) {
        const bool on = b == 0 ? (flags & 0x7u) != 0 : (b >= 3 && (flags & (1u << b)) != 0);
        if (!on || !w[b]) continue;
        char word[48];
        stripTags(word, sizeof word, w[b]);
        if (!word[0]) continue;
        if (n && n + 1 < cap) out[n++] = '/';
        for (unsigned i = 0; word[i] && n + 1 < cap; ++i) out[n++] = word[i];
        out[n] = 0;
    }
}

// Adds a row (label mapped through mapLabel, cut to 15 chars). A repeated label folds into the existing row ("Bludgeoning 10%" + "Piercing 10%"
// -> "Bludgeoning, Piercing 10%"; unequal trailing words -> "a, b"), except "Damage" which always starts its own row. Returns the row index, or
// -1 when dropped (empty label, or a new label with the 10 rows full).
inline int addRule(TipModel* m, const char* labelRaw, const char* value) {
    char label[sizeof m->rules[0].label];
    copyTrunc(label, sizeof label, mapLabel(labelRaw));
    if (!label[0]) return -1;
    if (strcmp(label, "Damage") != 0) {
        for (unsigned i = 0; i < m->nRules; ++i) {
            if (strcmp(m->rules[i].label, label) != 0) continue;
            char* h = m->rules[i].value;
            const char* ha = strrchr(h, ' ');
            const char* va = strrchr(value, ' ');
            char tmp[320];
            if (ha && va && ha[1] && strcmp(ha + 1, va + 1) == 0)
                snprintf(tmp, sizeof tmp, "%.*s, %.*s %s", static_cast<int>(ha - h), h, static_cast<int>(va - value), value, ha + 1);
            else
                snprintf(tmp, sizeof tmp, "%s, %s", h, value);
            copyTrunc(h, sizeof m->rules[0].value, tmp);
            return static_cast<int>(i);
        }
    }
    if (m->nRules >= MaxRules) return -1;
    TipRule& r = m->rules[m->nRules];
    copyTrunc(r.label, sizeof r.label, label);
    copyTrunc(r.value, sizeof r.value, value);
    return static_cast<int>(m->nRules++);
}

// Appends to a char buffer with truncation.
inline void appendTrunc(char* dst, unsigned cap, const char* s) {
    unsigned n = static_cast<unsigned>(strlen(dst));
    for (unsigned i = 0; s[i] && n + 1 < cap; ++i) dst[n++] = s[i];
    dst[n] = 0;
}

// First sentence; a second one when the first is under 70 chars and both fit in 170; cut at 170 on a word with "..." when one sentence is
// longer; text without any sentence end is cut at 170 chars (mockup short_flavor). A sentence ends at ".", "!" or "?" runs followed by a blank or the end.
inline void shortFlavor(char* out, unsigned cap, const char* text) {
    struct Span { unsigned b, e; };
    Span s[2] = { { 0, 0 }, { 0, 0 } };
    unsigned ns = 0;
    const unsigned len = static_cast<unsigned>(strlen(text));
    unsigned i = 0;
    while (i < len && ns < 2) {
        while (i < len && (text[i] == ' ' || text[i] == '\t' || text[i] == '\n')) ++i;
        const unsigned b = i;
        unsigned j = i;
        bool found = false;
        while (j < len) {
            const char c = text[j];
            if (c == '.' || c == '!' || c == '?') {
                unsigned k = j;
                while (k < len && (text[k] == '.' || text[k] == '!' || text[k] == '?')) ++k;
                if (j > b && (k >= len || text[k] == ' ' || text[k] == '\t' || text[k] == '\n')) { s[ns++] = { b, k }; i = k; found = true; break; }
                j = k;
                continue;
            }
            ++j;
        }
        if (!found) break;
    }
    if (ns == 0) {
        const unsigned lim = (cap - 1 < ShortFlavorMax) ? cap - 1 : ShortFlavorMax;
        unsigned n = 0;
        while (n < lim && text[n]) { out[n] = text[n]; ++n; }
        out[n] = 0;
        return;
    }
    out[0] = 0;
    const unsigned l0 = s[0].e - s[0].b;
    unsigned n = 0;
    for (unsigned k = s[0].b; k < s[0].e && n + 1 < cap; ++k) out[n++] = text[k];
    out[n] = 0;
    if (l0 < 70 && ns > 1) {
        const unsigned l1 = s[1].e - s[1].b;
        if (l0 + l1 + 1 <= ShortFlavorMax && n + 1 + l1 + 1 <= cap) {
            out[n++] = ' ';
            for (unsigned k = s[1].b; k < s[1].e; ++k) out[n++] = text[k];
            out[n] = 0;
        }
    }
    if (n > ShortFlavorMax) {
        n = ShortFlavorMax - 3;
        while (n > 0 && out[n] != ' ') --n;      // rsplit(" ", 1)[0] of the first 167 chars
        if (n == 0 && out[0] != ' ') n = ShortFlavorMax - 3;
        out[n] = 0;
        appendTrunc(out, cap, "...");
    }
}

}  // namespace rules_detail

// Builds the tooltip model of one item. false only when out is null.
inline bool buildTipModel(const RulesIn& in, const char* name, int stack, TipModel* out) {
    using namespace rules_detail;
    if (!out) return false;
    memset(out, 0, sizeof *out);
    stripTags(out->title, sizeof out->title, name);
    out->stack = stack;

    // category line: armor weight class, else the baseitems name
    const char* weight = armorWeight(in.baseRow);
    if (weight) copyTrunc(out->sub, sizeof out->sub, weight);
    else stripTags(out->sub, sizeof out->sub, in.baseName);

    // rules from baseitems.2da: weapons Damage + Critical, armor Defense + Max Dex (no Range)
    int dmgIdx = -1, defIdx = -1;
    int defAC = 0;
    char buf[160];
    if (in.numDice > 0 && in.die > 0) {
        char words[96];
        joinDmgWords(words, sizeof words, in.dmgFlags, in.dmgWords);
        const long long nd = in.numDice, die = in.die;
        snprintf(buf, sizeof buf, "%lld-%lld%s%s", nd, nd * die, words[0] ? " " : "", words);
        dmgIdx = addRule(out, "Damage", buf);
        const long long ct = in.critThreat < 1 ? 1 : (in.critThreat > 20 ? 20 : in.critThreat);
        const long long cm = in.critMult < 1 ? 2 : in.critMult;
        if (ct > 1) snprintf(buf, sizeof buf, "%lld-20 / x%lld", 21 - ct, cm);
        else snprintf(buf, sizeof buf, "20 / x%lld", cm);
        addRule(out, "Critical", buf);
    }
    if (in.baseAC > 0) {
        defAC = in.baseAC;
        snprintf(buf, sizeof buf, "+%d", defAC);
        defIdx = addRule(out, "Defense", buf);
        if (in.dexBonus >= 0 && in.dexBonus < 99) {
            snprintf(buf, sizeof buf, "+%d", in.dexBonus);
            addRule(out, "Max Dex", buf);
        }
    }

    // item properties: itempropdef name + subtype + value
    const unsigned np = in.nProps < 16 ? in.nProps : 16;
    for (unsigned i = 0; i < np; ++i) {
        const RulesProp& p = in.props[i];
        char lab[sizeof p.label], nm[sizeof p.name], sb[sizeof p.sub], vl[sizeof p.val];
        cleanField(lab, sizeof lab, p.label, sizeof p.label);
        cleanField(nm, sizeof nm, p.name, sizeof p.name);
        cleanField(sb, sizeof sb, p.sub, sizeof p.sub);
        cleanField(vl, sizeof vl, p.val, sizeof p.val);
        // the description says what a use does, and every party member counts as Human
        if (strcmp(lab, "CastSpell") == 0) continue;
        if (strcmp(lab, "UseLimitationRacial") == 0 && strcmp(sb, "Human") == 0) continue;
        char value[72];
        snprintf(value, sizeof value, "%s%s%s", sb, (sb[0] && vl[0]) ? " " : "", vl);
        if (!value[0]) copyTrunc(value, sizeof value, "Yes");
        if (strcmp(lab, "Damage") == 0 && dmgIdx >= 0) {
            // bonus damage folds into the Damage line: "1-10 Physical + 2 Unstoppable"
            char bonus[72];
            snprintf(bonus, sizeof bonus, "%s%s%s", vl, (vl[0] && sb[0]) ? " " : "", sb);
            if (!bonus[0]) copyTrunc(bonus, sizeof bonus, "Yes");
            char tmp[200];
            snprintf(tmp, sizeof tmp, "%s + %s", out->rules[dmgIdx].value, bonus);
            copyTrunc(out->rules[dmgIdx].value, sizeof out->rules[dmgIdx].value, tmp);
            continue;
        }
        if (strcmp(lab, "DecreaseAC") == 0 && defIdx >= 0) {
            // a Defense Penalty folds into Defense
            int dec = 0;
            for (unsigned k = 0; vl[k]; ++k) if (isDigit(vl[k]) && dec < 100000) dec = dec * 10 + (vl[k] - '0');
            defAC -= dec;
            snprintf(buf, sizeof buf, defAC >= 0 ? "+%d" : "%d", defAC);
            copyTrunc(out->rules[defIdx].value, sizeof out->rules[defIdx].value, buf);
            continue;
        }
        addRule(out, strcmp(lab, "BonusFeats") == 0 ? "Bonus Feat" : (nm[0] ? nm : lab), value);
    }

    // description: [tag line] [Label: value lines] flavor
    char tag[48] = "";
    char flavor[FlavorCap] = "";
    {
        char cur[LineCap], nxt[LineCap];
        const char* p = in.desc;
        // reads the next line (tags stripped, CR dropped); false at the end of the text
        auto readLine = [&p](char* dst) -> bool {
            if (!p || !*p) { dst[0] = 0; return false; }
            char raw[LineCap];
            unsigned n = 0;
            while (*p && *p != '\n') { if (*p != '\r' && n + 1 < LineCap) raw[n++] = *p; ++p; }
            raw[n] = 0;
            if (*p == '\n') ++p;
            stripTags(dst, LineCap, raw);
            return true;
        };
        bool hc = readLine(cur);
        while (hc && !cur[0]) hc = readLine(cur);
        if (hc) {
            const size_t l = strlen(cur);
            // a short first line without ':' that is not a sentence is the tag ("Not Upgradeable", "Upgradeable (Scope)")
            if (!strchr(cur, ':') && l <= 32 && cur[l - 1] != '.') {
                copyTrunc(tag, sizeof tag, cur);
                hc = readLine(cur);
            }
        }
        while (hc && cur[0]) {
            char* colon = strchr(cur, ':');
            if (!colon) break;
            *colon = 0;
            char* key = cur;
            for (size_t k = strlen(key); k > 0 && (key[k - 1] == ' ' || key[k - 1] == '\t'); --k) key[k - 1] = 0;
            char* v = colon + 1;
            while (*v == ' ' || *v == '\t') ++v;
            bool consumed = false, nxtValid = false;
            if (!*v) {   // a bare "Label:" takes the next line as its value
                nxtValid = readLine(nxt);
                if (nxtValid && nxt[0] && !strchr(nxt, ':')) { v = nxt; consumed = true; }
            }
            if (key[0] && !(strcmp(key, "Damage") == 0 && eqNoCase(v, "none"))) {
                char fixed[LineCap + 8];
                fixDC(fixed, sizeof fixed, v);
                addRule(out, key, fixed);
            }
            if (consumed) hc = readLine(cur);
            else if (nxtValid) { memcpy(cur, nxt, strlen(nxt) + 1); hc = true; }
            else hc = readLine(cur);
        }
        while (hc) {
            if (cur[0]) {
                if (flavor[0]) appendTrunc(flavor, sizeof flavor, " ");
                appendTrunc(flavor, sizeof flavor, cur);
            }
            hc = readLine(cur);
        }
    }

    // Upgrades row from the tag; "Not Upgradeable" adds nothing
    if (tag[0] && !strstr(tag, "Not")) {
        char slots[48];
        if (strncmp(tag, "Fully", 5) == 0) {
            copyTrunc(slots, sizeof slots, "All slots");
        } else if (strchr(tag, '(')) {
            char body[48];
            unsigned n = 0;
            for (unsigned i = 0; tag[i] && n + 1 < sizeof body;) {
                if (strncmp(tag + i, "Upgradeable", 11) == 0) { i += 11; continue; }
                body[n++] = tag[i++];
            }
            body[n] = 0;
            unsigned b = 0;
            while (body[b] == ' ' || body[b] == '(' || body[b] == ')') ++b;
            while (n > b && (body[n - 1] == ' ' || body[n - 1] == '(' || body[n - 1] == ')')) --n;
            body[n] = 0;
            snprintf(slots, sizeof slots, "%s%s", body + b, body[b] ? " only" : "only");
        } else {
            copyTrunc(slots, sizeof slots, "Yes");
        }
        addRule(out, "Upgrades", slots);
    }

    // dice in the values read as one number in the game font: 1d10 -> 1-10
    for (unsigned i = 0; i < out->nRules; ++i) {
        char tmp[sizeof out->rules[0].value];
        diceFix(tmp, sizeof tmp, out->rules[i].value);
        copyTrunc(out->rules[i].value, sizeof out->rules[i].value, tmp);
    }

    if (out->nRules > 0) {
        shortFlavor(out->flavor, sizeof out->flavor, flavor);
        out->full = false;
    } else {
        copyTrunc(out->flavor, sizeof out->flavor, flavor);   // no rules: the whole description (it often IS the rules, e.g. the medpac formula)
        out->full = true;
    }
    out->foot[0] = 0;
    return true;
}

}  // namespace qlogic
