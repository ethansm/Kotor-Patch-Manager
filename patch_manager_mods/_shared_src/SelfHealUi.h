// SelfHealUi.h - pure UI logic for the companion self-heal button/popover (doc 27): button geometry, state priority, steppers, INI line codec.
// Header-only, std only (no Windows/engine includes).
#pragma once
#include <cmath>
#include <string>

namespace shui {

struct Rect {
    int x, y, w, h;
    bool operator==(const Rect& o) const { return x == o.x && y == o.y && w == o.w && h == o.h; }
};

inline int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

// Button sits left of the vitality bar, vertically centred. {0,0,0,0} = do not draw.
inline Rect buttonRect(Rect vit) {
    if (vit.w < 70 || vit.h <= 0 || vit.h > 400) return Rect{0, 0, 0, 0};
    double pxu = vit.h / 36.0;
    int s = clampi((int)std::lround(vit.h * 0.25), 16, 24);
    int gap = (int)std::lround(2 * pxu);
    if (gap < 2) gap = 2;
    int x = vit.x - gap - s;
    if (x < 0) return Rect{0, 0, 0, 0};
    return Rect{x, vit.y + (vit.h - s) / 2, s, s};
}

enum class BtnState { Hidden, FullHp, NoItem, Pending, AutoOn, Normal };

inline BtnState resolveState(bool hasCreature, bool isLeader, int cur, int max, bool havePick, bool pending, bool autoOn) {
    if (!hasCreature || isLeader || max <= 0) return BtnState::Hidden;
    if (cur >= max) return BtnState::FullHp;
    if (!havePick) return BtnState::NoItem;
    if (pending) return BtnState::Pending;
    if (autoOn) return BtnState::AutoOn;
    return BtnState::Normal;
}

inline bool clickAllowed(BtnState s) { return s != BtnState::Hidden && s != BtnState::FullHp && s != BtnState::Pending; }
inline int clickMinCount(BtnState) { return 1; }   // manual click ignores the keep reserve

inline int stepHpPct(int v, int dir) { return clampi(v + (dir > 0 ? 5 : (dir < 0 ? -5 : 0)), 5, 95); }
inline int stepKeep(int v, int dir) { return clampi(v + (dir > 0 ? 1 : (dir < 0 ? -1 : 0)), 0, 20); }

struct CompCfg {
    bool autoOn = false;
    int hpPct = 35;
    int keep = 2;
    bool operator==(const CompCfg& o) const { return autoOn == o.autoOn && hpPct == o.hpPct && keep == o.keep; }
};

namespace detail {
inline std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n')) ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' || s[b - 1] == '\n')) --b;
    return s.substr(a, b - a);
}
// Parses [-]digits (saturating). false if not purely numeric.
inline bool parseInt(const std::string& s, int& out) {
    size_t i = 0;
    bool neg = false;
    if (i < s.size() && (s[i] == '-' || s[i] == '+')) { neg = s[i] == '-'; ++i; }
    if (i >= s.size()) return false;
    long long v = 0;
    for (; i < s.size(); ++i) {
        if (s[i] < '0' || s[i] > '9') return false;
        if (v < 100000) v = v * 10 + (s[i] - '0');
    }
    out = (int)(neg ? -v : v);
    return true;
}
}  // namespace detail

// "<companionTag>=auto:0|1,hp:NN,keep:N". Unknown fields ignored; bad numbers clamp / keep default.
inline bool parseLine(const std::string& line, std::string& key, CompCfg& out) {
    std::string t = detail::trim(line);
    if (t.empty() || t[0] == ';' || t[0] == '#') return false;
    size_t eq = t.find('=');
    if (eq == std::string::npos) return false;
    std::string k = detail::trim(t.substr(0, eq));
    if (k.empty()) return false;
    key = k;
    std::string rest = t.substr(eq + 1);
    size_t pos = 0;
    while (pos <= rest.size()) {
        size_t comma = rest.find(',', pos);
        if (comma == std::string::npos) comma = rest.size();
        std::string f = rest.substr(pos, comma - pos);
        pos = comma + 1;
        size_t c = f.find(':');
        if (c == std::string::npos) continue;
        std::string name = detail::trim(f.substr(0, c)), val = detail::trim(f.substr(c + 1));
        int n;
        if (!detail::parseInt(val, n)) continue;
        if (name == "auto") out.autoOn = n != 0;
        else if (name == "hp") out.hpPct = clampi(n, 5, 95);
        else if (name == "keep") out.keep = clampi(n, 0, 20);
    }
    return true;
}

inline std::string serialize(const std::string& key, const CompCfg& c) {
    return key + "=auto:" + (c.autoOn ? "1" : "0") + ",hp:" + std::to_string(c.hpPct) + ",keep:" + std::to_string(c.keep);
}

}  // namespace shui
