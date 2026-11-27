// CombatTip.h -- CH7a rich tooltip formatter for the combat hotbar (doc 31 §6.2). Pure std, no engine; host test combat_tip_test.cpp.
// The engine tip box does not wrap (GuiTip.h), so every line break is an explicit '\n'.
#pragma once
#include <cstdint>
#include <string>

namespace chtip {

enum Kind { K_Attack, K_Feat, K_Power, K_Grenade, K_Launcher };

// Key encodings (doc 31 key table): feat = 0x10000000|featId, power = 0x20000000|spellId, Attack = 0x3EB.
inline Kind kindOf(int category, uint32_t key) {
    if (category == 2) return K_Grenade;
    if (category == 3) return K_Launcher;
    if (key & 0x20000000u) return K_Power;
    if (key & 0x10000000u) return K_Feat;
    return K_Attack;
}

inline const char* kindLabel(Kind k) {
    switch (k) {
        case K_Feat: return "Attack feat";
        case K_Power: return "Force power";
        case K_Grenade: return "Grenade";
        case K_Launcher: return "Wrist launcher";
        default: return "Attack";
    }
}

struct TipIn {
    std::string name;
    Kind kind = K_Attack;
    uint32_t count = 0;      // stack size (+0x34), 0 = not a stack
    bool usable = true;
    std::string fp;          // spells.2da forcepoints cell (raw), powers only
    std::string desc;        // raw TLK description, "" = none
    std::string hotkey;      // CH7d; "" here
    int moreBelow = 0;       // CH7c (pill tip); 0 here
};

// FNV-1a 32-bit, for the per-button tip cache.
inline uint32_t hash(const std::string& s) {
    uint32_t h = 2166136261u;
    for (unsigned char c : s) { h ^= c; h *= 16777619u; }
    return h;
}

// keep in sync: action_hotbar_src/HotbarLogic.h tipTitleQty (title part).
// "Frag Grenade (5)" -> "Frag Grenade"; the trailing " (...)" is dropped when the name ends ')' and the " (" is not at 0.
inline std::string stripQty(const std::string& name) {
    size_t par = name.rfind(" (");
    if (par == std::string::npos || par == 0 || name.empty() || name.back() != ')') return name;
    return name.substr(0, par);
}

// keep in sync: action_hotbar_src/HotbarLogic.h wrapText.
// Greedy word wrap into '\n'-joined lines (at most maxLines, "..." when cut); a word longer than width stays whole.
inline std::string wrapText(const std::string& in, int width, int maxLines) {
    std::string out, line; int lines = 0; size_t i = 0;
    while (i < in.size()) {
        size_t e = in.find(' ', i); if (e == std::string::npos) e = in.size();
        std::string w = in.substr(i, e - i); i = e + 1;
        if (!line.empty() && (int)(line.size() + 1 + w.size()) > width) {
            if (++lines >= maxLines) { line += "..."; break; }
            out += line + "\n"; line.clear();
        }
        line += (line.empty() ? "" : " ") + w;
    }
    out += line;
    return out;
}

// keep in sync: action_hotbar_src/HotbarLogic.h shortDesc.
// Descriptions start with a category/prerequisite header ("Universal Power Restricted By Armor This power ..."): drop it and keep the first sentence.
inline std::string shortDesc(const std::string& d) {
    size_t p = d.find(" This "); std::string t = (p != std::string::npos && p < 120) ? d.substr(p + 1) : d;
    size_t e = t.find(". "); if (e != std::string::npos) t.resize(e + 1);
    return t;
}

// ASCII separator: the engine tip fonts are bitmap fonts; a Latin-1 middle dot (0xB7) may have no glyph.
constexpr const char* Sep = " - ";

inline bool fpShown(const std::string& fp) { return !fp.empty() && fp != "0" && fp != "****"; }

// detail 0: the bare name (pre-CH7a tip). 1: title + kind line. 2: + wrapped short description + "Not usable right now".
// Empty name -> "" (the caller's Clear path). No trailing '\n'.
inline std::string format(const TipIn& t, int detail) {
    if (t.name.empty()) return std::string();
    if (detail <= 0) return t.name;
    std::string out = stripQty(t.name);
    if (out.empty()) out = t.name;
    out += "\n";
    out += kindLabel(t.kind);
    if (t.kind == K_Power && fpShown(t.fp)) out += Sep + t.fp + " FP";
    if ((t.kind == K_Grenade || t.kind == K_Launcher) && t.count > 0) out += Sep + std::to_string(t.count) + " left";
    if (!t.hotkey.empty()) out += Sep + t.hotkey;
    if (detail >= 2) {
        if (!t.desc.empty()) {
            const std::string d = wrapText(shortDesc(t.desc), 38, 3);
            if (!d.empty()) out += "\n" + d;
        }
        if (!t.usable) out += "\nNot usable right now";
    }
    return out;
}

}  // namespace chtip
