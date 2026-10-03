// WizardLogic.h - pure (engine-free) state machine of the level-up path wizard.
// Python twin: research/companion_levelup/wizard_twin.py ; parity: tools/parity/wizard_parity.py
// Rules: candidates = items matching every recorded answer; exactly 1 -> result; otherwise the page is the first page
// after the last recorded one (in kOrder) with >= 2 distinct values (single-valued pages are skipped, NOT recorded); if none,
// a Pick page over the remaining candidates (options = preset ids).
#pragma once
#include <stddef.h>

namespace wiz {

constexpr int MaxItems = 16, MaxOptions = 8, NumQuestions = 5, MaxStack = 6;
enum Page { Role = 0, Style = 1, Alignment = 2, Convert = 3, Skills = 4, Pick = 5 };
// Question order (user 2026-10-02: the Jedi question comes first so the conversion choice is always explicit). Item answers stay indexed by Page.
constexpr int kOrder[NumQuestions] = { Convert, Role, Style, Alignment, Skills };
inline int orderPos(int page) { for (int i = 0; i < NumQuestions; ++i) if (kOrder[i] == page) return i; return NumQuestions; }

struct Item {
    const char* id;
    const char* ans[NumQuestions];   // role, style, alignment, convert, skills
    const char* shortName;
};

struct Answer { int page; int ref; };   // ref = item index whose ans[page] was chosen (Pick: the chosen item)

struct State {
    const Item* items;
    int n;
    int depth;
    Answer stack[MaxStack];
};

struct View {
    bool result;
    int resultItem;
    int page;
    int nOptions;
    const char* options[MaxOptions];   // raw value, or preset id on the Pick page
    int optionItem[MaxOptions];        // Pick: item index, else -1
};

inline char lowerAscii(char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c; }

inline bool ieq(const char* a, const char* b) {
    if (!a || !b) return a == b;
    for (; *a && *b; ++a, ++b) if (lowerAscii(*a) != lowerAscii(*b)) return false;
    return *a == *b;
}

inline bool selectable(const char* const ans[NumQuestions]) {
    for (int i = 0; i < NumQuestions; ++i)
        if (!ans[i] || !*ans[i] || (ans[i][0] == '-' && !ans[i][1])) return false;
    return true;
}

inline void reset(State& s, const Item* items, int n) {
    s.items = items;
    s.n = n < 0 ? 0 : (n > MaxItems ? MaxItems : n);
    s.depth = 0;
}

inline bool canBack(const State& s) { return s.depth > 0; }

inline bool back(State& s) {
    if (s.depth <= 0) return false;
    --s.depth;
    return true;
}

namespace detail {
inline bool matches(const State& s, int i) {
    for (int k = 0; k < s.depth; ++k) {
        const Answer& a = s.stack[k];
        if (a.page == Pick) { if (i != a.ref) return false; }
        else if (!ieq(s.items[i].ans[a.page], s.items[a.ref].ans[a.page])) return false;
    }
    return true;
}
inline int candidates(const State& s, int* out) {
    int c = 0;
    for (int i = 0; i < s.n; ++i) if (matches(s, i)) out[c++] = i;
    return c;
}
}  // namespace detail

inline View view(const State& s) {
    View v;
    v.result = false; v.resultItem = -1; v.page = -1; v.nOptions = 0;
    for (int i = 0; i < MaxOptions; ++i) { v.options[i] = nullptr; v.optionItem[i] = -1; }
    int cand[MaxItems];
    int nc = detail::candidates(s, cand);
    if (nc == 0) return v;
    if (nc == 1) { v.result = true; v.resultItem = cand[0]; return v; }
    int start = s.depth > 0 ? orderPos(s.stack[s.depth - 1].page) + 1 : 0;
    for (int k = start; k < NumQuestions; ++k) {
        const int p = kOrder[k];
        const char* vals[MaxItems];
        int nv = 0;
        for (int k = 0; k < nc; ++k) {
            const char* x = s.items[cand[k]].ans[p];
            bool seen = false;
            for (int j = 0; j < nv; ++j) if (ieq(vals[j], x)) { seen = true; break; }
            if (!seen) vals[nv++] = x;
        }
        if (nv >= 2) {
            v.page = p;
            v.nOptions = nv < MaxOptions ? nv : MaxOptions;
            for (int j = 0; j < v.nOptions; ++j) v.options[j] = vals[j];
            return v;
        }
    }
    v.page = Pick;
    v.nOptions = nc < MaxOptions ? nc : MaxOptions;
    for (int j = 0; j < v.nOptions; ++j) { v.options[j] = s.items[cand[j]].id; v.optionItem[j] = cand[j]; }
    return v;
}

inline bool choose(State& s, int option) {
    View v = view(s);
    if (v.result || v.page < 0 || option < 0 || option >= v.nOptions || s.depth >= MaxStack) return false;
    int ref = -1;
    if (v.page == Pick) ref = v.optionItem[option];
    else {
        int cand[MaxItems];
        int nc = detail::candidates(s, cand);
        for (int k = 0; k < nc && ref < 0; ++k)
            if (ieq(s.items[cand[k]].ans[v.page], v.options[option])) ref = cand[k];
    }
    if (ref < 0) return false;
    s.stack[s.depth].page = v.page;
    s.stack[s.depth].ref = ref;
    ++s.depth;
    return true;
}

inline const char* pageName(int page) {
    static const char* const n[] = {"Role", "Style", "Alignment", "Convert", "Skills", "Pick"};
    return (page >= 0 && page <= Pick) ? n[page] : "None";
}

inline const char* questionText(int page) {
    static const char* const q[] = {"Combat role?", "Weapon style?", "Force alignment?", "Become a Jedi?",
                                    "Skill focus?", "Choose a build:"};
    return (page >= 0 && page <= Pick) ? q[page] : "";
}

inline const char* answerLabel(int page, const char* value) {
    (void)page;
    static const char* const map[][2] = {
        {"melee", "Melee"}, {"ranged", "Ranged"}, {"caster", "Force caster"}, {"support", "Support / healer"},
        {"tank", "Tank"}, {"single", "One weapon"}, {"dual", "Two weapons"}, {"double", "Double-bladed"},
        {"unarmed", "Unarmed"}, {"pistols", "Pistols"}, {"rifle", "Rifle"}, {"none", "None"},
        {"light", "Light side"}, {"dark", "Dark side"}, {"neutral", "Neutral powers"}, {"early", "Early"},
        {"late", "Late"}, {"never", "Never"}, {"n/a", "n/a"}, {"combat", "Combat skills"},
        {"tech", "Tech skills"}, {"stealth", "Stealth skills"}, {"social", "Social skills"}};
    if (!value) return "";
    for (size_t i = 0; i < sizeof(map) / sizeof(map[0]); ++i) if (ieq(map[i][0], value)) return map[i][1];
    return value;
}

}  // namespace wiz
