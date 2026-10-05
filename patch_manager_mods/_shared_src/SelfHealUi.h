// SelfHealUi.h - pure UI logic for the companion self-heal button/popover (doc 27): button geometry, state priority, steppers, INI line codec, popover layout/hit, INI file model, pending tracker.
// Header-only, std only (no Windows/engine includes).
#pragma once
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace shui {

struct Rect {
    int x, y, w, h;
    bool operator==(const Rect& o) const { return x == o.x && y == o.y && w == o.w && h == o.h; }
};

inline int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

// Button sits left of the vitality bar, vertically centred. {0,0,0,0} = do not draw.
// Small (companion) cards only: live vits are 47x96 small vs 103x208 leader at 3840x1600 (doc 08 probe 1); w >= 70 = leader.
inline bool isLeaderVit(Rect vit) { return vit.w >= 70; }
inline Rect buttonRect(Rect vit) {
    if (vit.w <= 0 || isLeaderVit(vit) || vit.h <= 0 || vit.h > 400) return Rect{0, 0, 0, 0};
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

// ---- popover (3 rows: Auto toggle / HP% stepper / Keep stepper) ----
enum class PopItem { None, AutoToggle, HpMinus, HpPlus, KeepMinus, KeepPlus };

struct PopLayout {
    Rect r[6];   // r[0] = background panel; r[i] == rect of static_cast<PopItem>(i) for i >= 1
};

// Panel is 8*h wide, 3*h tall (h = button.h), top-aligned with the button and placed to its LEFT with a gap of h/4.
// Clamped so x+w <= screenW (when screenW > 0), then x >= 0 (left clamp wins).
inline PopLayout popoverLayout(Rect b, int screenW) {
    int h = b.h, w = 8 * h, gap = h / 4;
    int x = b.x - gap - w;
    if (screenW > 0 && x + w > screenW) x = screenW - w;
    if (x < 0) x = 0;
    int y = b.y;
    PopLayout L;
    L.r[0] = Rect{x, y, w, 3 * h};
    L.r[1] = Rect{x, y, w, h};
    L.r[2] = Rect{x, y + h, h, h};
    L.r[3] = Rect{x + w - h, y + h, h, h};
    L.r[4] = Rect{x, y + 2 * h, h, h};
    L.r[5] = Rect{x + w - h, y + 2 * h, h, h};
    return L;
}

// Span between the minus and plus squares of row 2 (HP) or 3 (Keep); {0,0,0,0} for other rows.
inline Rect popValueRect(const PopLayout& L, int row) {
    if (row != 2 && row != 3) return Rect{0, 0, 0, 0};
    const Rect& m = L.r[row == 2 ? 2 : 4];
    const Rect& p = L.r[row == 2 ? 3 : 5];
    return Rect{m.x + m.w, m.y, p.x - (m.x + m.w), m.h};
}

namespace detail {
inline bool inRect(const Rect& r, int x, int y) { return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h; }
}  // namespace detail

inline PopItem popoverHit(const PopLayout& L, int x, int y) {
    for (int i = 1; i <= 5; ++i)
        if (detail::inRect(L.r[i], x, y)) return static_cast<PopItem>(i);
    return PopItem::None;
}


inline CompCfg applyPop(CompCfg c, PopItem it) {
    switch (it) {
        case PopItem::AutoToggle: c.autoOn = !c.autoOn; break;
        case PopItem::HpMinus: c.hpPct = stepHpPct(c.hpPct, -1); break;
        case PopItem::HpPlus: c.hpPct = stepHpPct(c.hpPct, 1); break;
        case PopItem::KeepMinus: c.keep = stepKeep(c.keep, -1); break;
        case PopItem::KeepPlus: c.keep = stepKeep(c.keep, 1); break;
        case PopItem::None: break;
    }
    return c;
}

inline std::string popLabel(PopItem it, const CompCfg& c) {
    switch (it) {
        case PopItem::AutoToggle: return c.autoOn ? "Auto: ON" : "Auto: OFF";
        case PopItem::HpMinus: case PopItem::KeepMinus: return "-";
        case PopItem::HpPlus: case PopItem::KeepPlus: return "+";
        case PopItem::None: break;
    }
    return "";
}

inline std::string popValueLabel(int row, const CompCfg& c) {
    if (row == 2) return "Use below " + std::to_string(c.hpPct) + "%";
    if (row == 3) return "Keep " + std::to_string(c.keep);
    return "";
}

// ---- INI file model: [defaults] default=..., [companions] <tag>=... ; everything else is preserved verbatim on write ----
struct CfgTable {
    CompCfg def;
    std::vector<std::pair<std::string, CompCfg>> rows;
    bool operator==(const CfgTable& o) const { return def == o.def && rows == o.rows; }
};

namespace detail {
// Splits on '\n', strips a trailing '\r' from each line.
inline void splitLines(const std::string& text, std::vector<std::string>& v) {
    size_t pos = 0;
    while (pos < text.size()) {
        size_t nl = text.find('\n', pos);
        if (nl == std::string::npos) nl = text.size();
        std::string l = text.substr(pos, nl - pos);
        if (!l.empty() && l.back() == '\r') l.pop_back();
        v.push_back(l);
        pos = nl + 1;
    }
}
// If the line is a "[name]" header returns true and sets name (trimmed).
inline bool sectionName(const std::string& line, std::string& name) {
    std::string t = trim(line);
    if (t.size() < 2 || t[0] != '[' || t.back() != ']') return false;
    name = trim(t.substr(1, t.size() - 2));
    return true;
}
}  // namespace detail

inline void upsert(CfgTable& t, const std::string& tag, const CompCfg& c) {
    for (auto& r : t.rows)
        if (r.first == tag) { r.second = c; return; }
    t.rows.emplace_back(tag, c);
}

inline const CompCfg& lookup(const CfgTable& t, const std::string& tag) {
    for (const auto& r : t.rows)
        if (r.first == tag) return r.second;
    return t.def;
}

inline CfgTable parseFile(const std::string& text) {
    CfgTable t;
    std::string sec;
    std::vector<std::string> lines;
    detail::splitLines(text, lines);
    for (const std::string& line : lines) {
        std::string name;
        if (detail::sectionName(line, name)) { sec = name; continue; }
        std::string key;
        CompCfg c;
        if (sec == "defaults") {
            if (parseLine(line, key, c) && key == "default") t.def = c;
        } else if (sec == "companions") {
            if (parseLine(line, key, c)) upsert(t, key, c);
        }
    }
    return t;
}

// Rewrites only the bodies of [defaults] and [companions]; all other lines are kept verbatim (original EOL style kept).
// A repeated section header of the same name is merged into the first. Missing sections are appended.
inline std::string serializeFile(const CfgTable& t, const std::string& original) {
    const std::string eol = original.find("\r\n") != std::string::npos ? "\r\n" : "\n";
    std::vector<std::string> in, out;
    detail::splitLines(original, in);
    auto body = [&](const std::string& sec) {
        if (sec == "defaults") out.push_back(serialize("default", t.def));
        else for (const auto& r : t.rows) out.push_back(serialize(r.first, r.second));
    };
    bool seenDef = false, seenComp = false;
    size_t i = 0;
    while (i < in.size()) {
        std::string name;
        if (!detail::sectionName(in[i], name) || (name != "defaults" && name != "companions")) { out.push_back(in[i++]); continue; }
        bool& seen = name == "defaults" ? seenDef : seenComp;
        size_t j = i + 1;
        while (j < in.size()) { std::string n2; if (detail::sectionName(in[j], n2)) break; ++j; }
        size_t end = j;                                   // keep trailing blank lines of the old body
        while (end > i + 1 && detail::trim(in[end - 1]).empty()) --end;
        if (!seen) {
            seen = true;
            out.push_back(in[i]);
            body(name);
            for (size_t k = end; k < j; ++k) out.push_back(in[k]);
        }                                                  // else: duplicate section dropped (its rows were merged by parseFile)
        i = j;
    }
    for (const char* sec : {"defaults", "companions"}) {
        if (std::string(sec) == "defaults" ? seenDef : seenComp) continue;
        if (!out.empty() && !detail::trim(out.back()).empty()) out.push_back("");
        out.push_back(std::string("[") + sec + "]");
        body(sec);
    }
    std::string s;
    for (const auto& l : out) s += l + eol;
    return s;
}

// ---- pending tracker: after a heal click/auto use, wait for HP up / stack down, else time out ----
constexpr uint32_t kPendingTimeoutMs = 6000;

struct Pending {
    bool on = false;
    uint32_t since = 0;
    int preHp = 0, preStack = 0;
};

inline bool pendingTimedOut(const Pending& p, uint32_t now) { return (uint32_t)(now - p.since) >= kPendingTimeoutMs; }

// stack < 0 means the item is gone (unknown/removed).
inline bool pendingDone(const Pending& p, uint32_t now, int hp, int stack) {
    return hp > p.preHp || (stack >= 0 && stack < p.preStack) || (stack < 0 && p.preStack > 0) || pendingTimedOut(p, now);
}

// ---- look: colour / alpha / spinner ring per state (CS4) ----
constexpr int kRingFrames = 8;
constexpr uint32_t kRingFrameMs = 90;   // 8 frames * 90 ms = ~0.7 s per revolution

struct Look {
    float r, g, b, alpha;
    int ringFrame;
    bool ring;
};

// ringFrame is always computed (wraps with uint32 now, 0 only at frame boundaries); ring says whether to draw it.
inline Look lookFor(BtnState st, bool autoOn, uint32_t now) {
    Look l{0.30f, 0.95f, 0.40f, 1.0f, (int)((now / kRingFrameMs) % (uint32_t)kRingFrames), autoOn && st != BtnState::Hidden};
    const bool gold = autoOn || st == BtnState::AutoOn;
    if (gold) { l.r = 1.00f; l.g = 0.80f; l.b = 0.25f; }
    switch (st) {
        case BtnState::Hidden: l.alpha = 0.0f; break;
        case BtnState::FullHp:
        case BtnState::NoItem: l.r = l.g = l.b = 0.60f; l.alpha = 0.40f; break;
        case BtnState::Pending: l.alpha = 0.65f; break;
        default: break;
    }
    return l;
}

// Square ring centred on the button, side = 1.5 * h rounded to the nearest even number.
inline Rect ringRect(Rect btn) {
    if (btn.h <= 0) return Rect{0, 0, 0, 0};
    int side = ((btn.h * 3 + 2) / 4) * 2;
    return Rect{btn.x + (btn.w - side) / 2, btn.y + (btn.h - side) / 2, side, side};
}

}  // namespace shui
