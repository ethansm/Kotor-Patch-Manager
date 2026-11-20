// ItemProps.h -- Shared pure C++17 item property, description tag, dice, and rule parser (Doc 53 RMT0).
// Extracted from QuickLootRules.h / QuickLootLogic.h so both quick_loot_src, rich_menu_typography_src
// (RichTextClassifier.h, RichDescBox.h), and ItemHoverCard.h share one canonical zero-heap, zero-Win32 implementation.
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace itemprops {

constexpr unsigned MaxRules       = 10;
constexpr unsigned LineCap        = 640;
constexpr unsigned FlavorCap      = 600;
constexpr unsigned ShortFlavorMax = 170;

struct RulesProp {
    char label[24];
    char name[32];
    char sub[32];
    char val[32];
};

struct RulesIn {
    int baseRow, numDice, die, critThreat, critMult, baseAC, dexBonus;
    uint32_t dmgFlags;
    RulesProp props[16];
    unsigned nProps;
    const char* desc;
    const char* baseName;
    const char* dmgWords[14];
};

struct PropRule {
    char label[16];
    char value[96];
};

struct ItemTipModel {
    char title[48];
    int stack;
    char sub[40];
    PropRule rules[MaxRules];
    unsigned nRules;
    char flavor[FlavorCap];
    bool full;
    char foot[64];
};

inline bool isDigit(char c) noexcept { return c >= '0' && c <= '9'; }
inline bool isWord(char c) noexcept {
    return isDigit(c) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
inline char lower(char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
}

inline bool eqNoCase(const char* a, const char* b) noexcept {
    if (!a || !b) return a == b;
    for (; *a && *b; ++a, ++b) {
        if (lower(*a) != lower(*b)) return false;
    }
    return *a == *b;
}

inline void copyTrunc(char* dst, unsigned cap, const char* src) noexcept {
    if (!dst || cap == 0) return;
    unsigned n = 0;
    if (src) {
        while (src[n] && n + 1 < cap) {
            dst[n] = src[n];
            ++n;
        }
    }
    dst[n] = 0;
}

inline void appendTrunc(char* dst, unsigned cap, const char* s) noexcept {
    if (!dst || cap == 0 || !s) return;
    unsigned n = static_cast<unsigned>(strlen(dst));
    for (unsigned i = 0; s[i] && n + 1 < cap; ++i) dst[n++] = s[i];
    dst[n] = 0;
}

// Strips BioWare "{...}" tags of at most 12 inner chars (no nested braces), collapses runs of spaces/tabs
// to a single space, and trims leading/trailing whitespace. Longer or unclosed braces are preserved as text.
inline unsigned stripTags(char* out, unsigned cap, const char* in) noexcept {
    if (!out || cap == 0) return 0;
    unsigned n = 0;
    bool space = false;
    if (in) {
        for (unsigned i = 0; in[i] && n + 1 < cap; ++i) {
            if (in[i] == '{') {
                unsigned j = i + 1;
                while (in[j] && in[j] != '}' && in[j] != '{' && j - i - 1 <= 12) ++j;
                if (in[j] == '}' && j - i - 1 <= 12) {
                    i = j;
                    continue;
                }
            }
            if (in[i] == ' ' || in[i] == '\t') {
                space = n > 0;
                continue;
            }
            if (space && n + 2 < cap) out[n++] = ' ';
            space = false;
            out[n++] = in[i];
        }
    }
    out[n] = 0;
    return n;
}

// Multi-line variant of stripTags: strips "{...}" tags (<= 12 inner chars) while preserving '\n' line breaks
// and leading indentation on continuation lines.
inline unsigned stripTagsMultiline(char* out, unsigned cap, const char* in) noexcept {
    if (!out || cap == 0) return 0;
    unsigned n = 0;
    if (in) {
        for (unsigned i = 0; in[i] && n + 1 < cap; ++i) {
            if (in[i] == '\r') continue;
            if (in[i] == '{') {
                unsigned j = i + 1;
                while (in[j] && in[j] != '}' && in[j] != '{' && in[j] != '\n' && j - i - 1 <= 12) ++j;
                if (in[j] == '}' && j - i - 1 <= 12) {
                    i = j;
                    continue;
                }
            }
            out[n++] = in[i];
        }
    }
    out[n] = 0;
    return n;
}

inline void cleanField(char* out, unsigned cap, const char* in, unsigned inCap) noexcept {
    char tmp[48];
    unsigned n = 0;
    if (in) {
        while (n < inCap && n + 1 < sizeof tmp && in[n]) {
            tmp[n] = in[n];
            ++n;
        }
    }
    tmp[n] = 0;
    stripTags(out, cap, tmp);
}

inline const char* mapLabel(const char* k) noexcept {
    if (!k) return "";
    static const char* const pairs[][2] = {
        { "Damage Immunity", "Immunity" },
        { "User Limitation", "Restricted" },
        { "Activate Item", "Use" },
        { "Area of Effect", "Area" },
        { "Save (Will)", "Will Save" },
        { "Save (Reflex)", "Reflex Save" },
        { "Save (Fort)", "Fort Save" },
        { "Secondary", "Effect" },
        { "Attack Bonus", "Attack" },
        { "Ability Bonus", "Ability" },
        { "Skill Bonus", "Skill" },
    };
    for (const auto& p : pairs) {
        if (strcmp(k, p[0]) == 0) return p[1];
    }
    return k;
}

// Converts whole-word dice notation "NdM" -> "N-(N*M)" ("2d6" -> "2-12", "1d10" -> "1-10").
inline void diceFix(char* out, unsigned cap, const char* in) noexcept {
    if (!out || cap == 0) return;
    if (!in) {
        out[0] = 0;
        return;
    }
    unsigned n = 0;
    unsigned i = 0;
    while (in[i] && n + 1 < cap) {
        if (isDigit(in[i]) && (i == 0 || !isWord(in[i - 1]))) {
            unsigned j = i;
            unsigned long long a = 0;
            while (isDigit(in[j])) {
                if (j - i < 9) a = a * 10 + static_cast<unsigned>(in[j] - '0');
                ++j;
            }
            if ((in[j] == 'd' || in[j] == 'D') && isDigit(in[j + 1]) && j - i <= 9) {
                unsigned k = j + 1;
                unsigned long long b = 0;
                while (isDigit(in[k])) {
                    if (k - j - 1 < 9) b = b * 10 + static_cast<unsigned>(in[k] - '0');
                    ++k;
                }
                if (!isWord(in[k]) && k - j - 1 <= 9) {
                    char tmp[48];
                    snprintf(tmp, sizeof tmp, "%llu-%llu", a, a * b);
                    for (unsigned t = 0; tmp[t] && n + 1 < cap; ++t) out[n++] = tmp[t];
                    i = k;
                    continue;
                }
            }
            while (i < j && n + 1 < cap) out[n++] = in[i++];
            continue;
        }
        out[n++] = in[i++];
    }
    out[n] = 0;
}

// Converts "DC22" -> "DC 22".
inline void fixDC(char* out, unsigned cap, const char* in) noexcept {
    if (!out || cap == 0) return;
    if (!in) {
        out[0] = 0;
        return;
    }
    unsigned n = 0;
    for (unsigned i = 0; in[i] && n + 1 < cap; ++i) {
        out[n++] = in[i];
        if (in[i] == 'D' && in[i + 1] == 'C' && isDigit(in[i + 2]) && (i == 0 || !isWord(in[i - 1]))) {
            if (n + 1 < cap) out[n++] = 'C';
            if (n + 1 < cap) out[n++] = ' ';
            ++i;
        }
    }
    out[n] = 0;
}

inline const char* armorWeight(int row) noexcept {
    switch (row) {
        case 38:
        case 39: return "Light Armor";
        case 40:
        case 41: return "Medium Armor";
        case 42:
        case 43: return "Heavy Armor";
        default: return nullptr;
    }
}

inline void joinDmgWords(char* out, unsigned cap, uint32_t flags, const char* const* w) noexcept {
    if (!out || cap == 0) return;
    unsigned n = 0;
    out[0] = 0;
    if (!w) return;
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

inline int addRule(ItemTipModel* m, const char* labelRaw, const char* value) noexcept {
    if (!m || !labelRaw || !value) return -1;
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
            if (ha && va && ha[1] && strcmp(ha + 1, va + 1) == 0) {
                snprintf(tmp, sizeof tmp, "%.*s, %.*s %s",
                         static_cast<int>(ha - h), h,
                         static_cast<int>(va - value), value, ha + 1);
            } else {
                snprintf(tmp, sizeof tmp, "%s, %s", h, value);
            }
            copyTrunc(h, sizeof m->rules[0].value, tmp);
            return static_cast<int>(i);
        }
    }
    if (m->nRules >= MaxRules) return -1;
    PropRule& r = m->rules[m->nRules];
    copyTrunc(r.label, sizeof r.label, label);
    copyTrunc(r.value, sizeof r.value, value);
    return static_cast<int>(m->nRules++);
}

inline void shortFlavor(char* out, unsigned cap, const char* text) noexcept {
    if (!out || cap == 0) return;
    if (!text) {
        out[0] = 0;
        return;
    }
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
                if (j > b && (k >= len || text[k] == ' ' || text[k] == '\t' || text[k] == '\n')) {
                    s[ns++] = { b, k };
                    i = k;
                    found = true;
                    break;
                }
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
        while (n < lim && text[n]) {
            out[n] = text[n];
            ++n;
        }
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
        while (n > 0 && out[n] != ' ') --n;
        if (n == 0 && out[0] != ' ') n = ShortFlavorMax - 3;
        out[n] = 0;
        appendTrunc(out, cap, "...");
    }
}

inline bool buildItemTipModel(const RulesIn& in, const char* name, int stack, ItemTipModel* out) noexcept {
    if (!out) return false;
    memset(out, 0, sizeof *out);
    stripTags(out->title, sizeof out->title, name);
    out->stack = stack;

    const char* weight = armorWeight(in.baseRow);
    if (weight) copyTrunc(out->sub, sizeof out->sub, weight);
    else stripTags(out->sub, sizeof out->sub, in.baseName);

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

    const unsigned np = in.nProps < 16 ? in.nProps : 16;
    for (unsigned i = 0; i < np; ++i) {
        const RulesProp& p = in.props[i];
        char lab[sizeof p.label], nm[sizeof p.name], sb[sizeof p.sub], vl[sizeof p.val];
        cleanField(lab, sizeof lab, p.label, sizeof p.label);
        cleanField(nm, sizeof nm, p.name, sizeof p.name);
        cleanField(sb, sizeof sb, p.sub, sizeof p.sub);
        cleanField(vl, sizeof vl, p.val, sizeof p.val);
        if (strcmp(lab, "CastSpell") == 0) continue;
        if (strcmp(lab, "UseLimitationRacial") == 0 && strcmp(sb, "Human") == 0) continue;
        char value[72];
        snprintf(value, sizeof value, "%s%s%s", sb, (sb[0] && vl[0]) ? " " : "", vl);
        if (!value[0]) copyTrunc(value, sizeof value, "Yes");
        if (strcmp(lab, "Damage") == 0 && dmgIdx >= 0) {
            char bonus[72];
            snprintf(bonus, sizeof bonus, "%s%s%s", vl, (vl[0] && sb[0]) ? " " : "", sb);
            if (!bonus[0]) copyTrunc(bonus, sizeof bonus, "Yes");
            char tmp[200];
            snprintf(tmp, sizeof tmp, "%s + %s", out->rules[dmgIdx].value, bonus);
            copyTrunc(out->rules[dmgIdx].value, sizeof out->rules[dmgIdx].value, tmp);
            continue;
        }
        if (strcmp(lab, "DecreaseAC") == 0 && defIdx >= 0) {
            int dec = 0;
            for (unsigned k = 0; vl[k]; ++k) {
                if (isDigit(vl[k]) && dec < 100000) dec = dec * 10 + (vl[k] - '0');
            }
            defAC -= dec;
            snprintf(buf, sizeof buf, defAC >= 0 ? "+%d" : "%d", defAC);
            copyTrunc(out->rules[defIdx].value, sizeof out->rules[defIdx].value, buf);
            continue;
        }
        addRule(out, strcmp(lab, "BonusFeats") == 0 ? "Bonus Feat" : (nm[0] ? nm : lab), value);
    }

    char tag[48] = "";
    char flavor[FlavorCap] = "";
    {
        char cur[LineCap], nxt[LineCap];
        const char* p = in.desc;
        auto readLine = [&p](char* dst) -> bool {
            if (!p || !*p) {
                dst[0] = 0;
                return false;
            }
            char raw[LineCap];
            unsigned n = 0;
            while (*p && *p != '\n') {
                if (*p != '\r' && n + 1 < LineCap) raw[n++] = *p;
                ++p;
            }
            raw[n] = 0;
            if (*p == '\n') ++p;
            stripTags(dst, LineCap, raw);
            return true;
        };
        bool hc = readLine(cur);
        while (hc && !cur[0]) hc = readLine(cur);
        if (hc) {
            const size_t l = strlen(cur);
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
            if (!*v) {
                nxtValid = readLine(nxt);
                if (nxtValid && nxt[0] && !strchr(nxt, ':')) {
                    v = nxt;
                    consumed = true;
                }
            }
            if (key[0] && !(strcmp(key, "Damage") == 0 && eqNoCase(v, "none"))) {
                char fixed[LineCap + 8];
                fixDC(fixed, sizeof fixed, v);
                addRule(out, key, fixed);
            }
            if (consumed) hc = readLine(cur);
            else if (nxtValid) {
                memcpy(cur, nxt, strlen(nxt) + 1);
                hc = true;
            } else {
                hc = readLine(cur);
            }
        }
        while (hc) {
            if (cur[0]) {
                if (flavor[0]) appendTrunc(flavor, sizeof flavor, " ");
                appendTrunc(flavor, sizeof flavor, cur);
            }
            hc = readLine(cur);
        }
    }

    if (tag[0] && !strstr(tag, "Not")) {
        char slots[48];
        if (strncmp(tag, "Fully", 5) == 0) {
            copyTrunc(slots, sizeof slots, "All slots");
        } else if (strchr(tag, '(')) {
            char body[48];
            unsigned n = 0;
            for (unsigned i = 0; tag[i] && n + 1 < sizeof body;) {
                if (strncmp(tag + i, "Upgradeable", 11) == 0) {
                    i += 11;
                    continue;
                }
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

    for (unsigned i = 0; i < out->nRules; ++i) {
        char tmp[sizeof out->rules[0].value];
        diceFix(tmp, sizeof tmp, out->rules[i].value);
        copyTrunc(out->rules[i].value, sizeof out->rules[i].value, tmp);
    }

    if (out->nRules > 0) {
        shortFlavor(out->flavor, sizeof out->flavor, flavor);
        out->full = false;
    } else {
        copyTrunc(out->flavor, sizeof out->flavor, flavor);
        out->full = true;
    }
    out->foot[0] = 0;
    return true;
}

} // namespace itemprops
