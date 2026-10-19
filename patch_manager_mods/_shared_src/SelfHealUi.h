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
inline bool isLeaderVitForButton(Rect vit) { return vit.w >= 70; }
// Disc ("circle") diameter. WI-38 C219: fixed at kDefaultDisc (58) for every bar height (was clamp(0.6 * vit.h, 48, 72)).
// btnSizeForDisc inverts ringRect (side ~1.5 * button).
constexpr int kDefaultDisc = 58;   // declared before discDiameter (its body returns it)
inline int discDiameter(int vitH) { (void)vitH; return kDefaultDisc; }
inline int btnSizeForDisc(int d) { return (d * 2 + 1) / 3; }
inline int ringSideFor(int btnH) { return ((btnH * 3 + 2) / 4) * 2; }   // ringRect side (defined below)
inline int gapFor(int h) { int g = (int)std::lround(2 * h / 36.0); return g < 2 ? 2 : g; }
// The disc/ring square sits left of the vitality bar (its right edge `gap` from vit.x), the cross button is centred in it, both vertically
// centred on the bar. {0,0,0,0} = do not draw.
inline Rect buttonRect(Rect vit) {
    if (vit.w <= 0 || isLeaderVit(vit) || vit.h <= 0 || vit.h > 400) return Rect{0, 0, 0, 0};
    const int d = discDiameter(vit.h), s = btnSizeForDisc(d);
    const int rx = vit.x - gapFor(vit.h) - d;   // ring/disc left edge
    if (rx < 0) return Rect{0, 0, 0, 0};
    return Rect{rx + (d - s) / 2, vit.y + (vit.h - s) / 2, s, s};
}

// Width left of vit.x kept free for the disc: gap (rounded up) + disc/ring side + kReserveGap. This is the width companion-selfheal
// registers at hud-host strip order 0 (doc 24 CS10); queue-hud falls back to it when hud-host / selfheal is absent.
constexpr int kReserveGap = 2;
inline int stripReserve(Rect vit) {
    int gap = (vit.h + 17) / 18;
    if (gap < 2) gap = 2;
    return gap + ringSideFor(btnSizeForDisc(discDiameter(vit.h))) + kReserveGap;   // ring side = disc diameter +/-1
}
// Leader (party-leader card) button: OUTSIDE, left of the leader's vitality bar (which starts left of the portrait card) and clear of
// the shield bar (kLeaderClear). `diameter` is the companion disc size (the leader's own vit.h would give the 72 px cap), 0 = default.
// The disc's top edge is aligned with card.y. {0,0,0,0} = do not draw (invalid card, or it would fall off the left screen edge).
constexpr int kLeaderClear = 6;
inline int leaderDisc(int diameter) { return diameter > 0 ? clampi(diameter, 48, 72) : kDefaultDisc; }
inline int leaderAnchorX(Rect card, Rect vit) { return (vit.w > 0 && vit.x < card.x) ? vit.x : card.x; }
inline Rect leaderButtonRect(Rect card, Rect vit, int diameter = 0) {
    if (card.w <= 0 || card.h <= 0) return Rect{0, 0, 0, 0};
    const int d = leaderDisc(diameter), s = btnSizeForDisc(d);
    const int rx = leaderAnchorX(card, vit) - gapFor(card.h) - kLeaderClear - d;
    if (rx < 0) return Rect{0, 0, 0, 0};
    return Rect{rx + (d - s) / 2, card.y + (d - s) / 2, s, s};
}
// Width left of card.x kept free for the leader disc (mirrors stripReserve).
inline int leaderStripReserve(Rect card, Rect vit, int diameter = 0) {
    return (card.x - leaderAnchorX(card, vit)) + gapFor(card.h) + kLeaderClear + leaderDisc(diameter) + kReserveGap;
}

enum class BtnState { Hidden, FullHp, NoItem, Pending, AutoOn, Normal, Down };

// `down` = unconscious (cur <= 0): always Down (dim, no ring, no click). cfg is untouched, so waking restores the previous Normal/AutoOn look.
inline BtnState resolveState(bool hasCreature, bool isLeader, int cur, int max, bool havePick, bool pending, bool autoOn, bool down = false) {
    if (!hasCreature || isLeader || max <= 0) return BtnState::Hidden;
    if (down) return BtnState::Down;
    if (cur >= max) return BtnState::FullHp;
    if (!havePick) return BtnState::NoItem;
    if (pending) return BtnState::Pending;
    if (autoOn) return BtnState::AutoOn;
    return BtnState::Normal;
}

inline bool clickAllowed(BtnState s) { return s != BtnState::Hidden && s != BtnState::FullHp && s != BtnState::Pending && s != BtnState::Down; }
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
// `unit` (> 0) overrides the item height (the button grew with the disc, the popover keeps its size: popUnit).
inline int popUnit(Rect btn) { return clampi((btn.h * 5 + 4) / 8, 16, 24); }
inline PopLayout popoverLayout(Rect b, int screenW, int unit = 0) {
    int h = unit > 0 ? unit : b.h, w = 8 * h, gap = h / 4;
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

// Item height that fits a label: max(btnUnit, ceil(1.5 * linePx)). maxLabelPx is accepted for the signature and does not affect the result.
inline int popUnitForText(float /*maxLabelPx*/, float linePx, int btnUnit) {
    const int need = (int)std::ceil(linePx * 1.5f);
    return btnUnit > need ? btnUnit : need;
}

// Like popoverLayout, but the panel is panelW wide (not 8*unit) and every row is `unit` tall. Value span = panelW - 2*unit (popValueRect).
// x = b.x - unit/4 - panelW, clamped to screenW (when > 0) then to x >= 0. unit <= 0 falls back to b.h, as in popoverLayout.
inline PopLayout popoverLayoutW(Rect b, int screenW, int unit, int panelW) {
    const int u = unit > 0 ? unit : b.h, w = panelW, gap = u / 4;
    int x = b.x - gap - w;
    if (screenW > 0 && x + w > screenW) x = screenW - w;
    if (x < 0) x = 0;
    const int y = b.y;
    PopLayout L;
    L.r[0] = Rect{x, y, w, 3 * u};
    L.r[1] = Rect{x, y, w, u};
    L.r[2] = Rect{x, y + u, u, u};
    L.r[3] = Rect{x + w - u, y + u, u, u};
    L.r[4] = Rect{x, y + 2 * u, u, u};
    L.r[5] = Rect{x + w - u, y + 2 * u, u, u};
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

// Row id for the popover; values 1..5 match PopItem 1..5 (Row_Auto..Row_KeepP). Value text spans and points outside the panel are Row_None.
enum PopRow { Row_None, Row_Auto, Row_HpM, Row_HpP, Row_KeepM, Row_KeepP };
inline PopRow popoverRowAt(const PopLayout& L, int x, int y) {
    switch (popoverHit(L, x, y)) {
        case PopItem::AutoToggle: return Row_Auto;
        case PopItem::HpMinus: return Row_HpM;
        case PopItem::HpPlus: return Row_HpP;
        case PopItem::KeepMinus: return Row_KeepM;
        case PopItem::KeepPlus: return Row_KeepP;
        case PopItem::None: break;
    }
    return Row_None;
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

inline const char* popAutoCaption() { return "Auto-heal"; }

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

namespace detail {
// MCF4: INI tags match ASCII case-insensitively (the engine compares tags with stricmp; the utc says "BaoDur", saves say "baodur"). The first
// spelling written into the table is kept.
inline bool tagEq(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        const char x = a[i] >= 'A' && a[i] <= 'Z' ? (char)(a[i] - 'A' + 'a') : a[i];
        const char y = b[i] >= 'A' && b[i] <= 'Z' ? (char)(b[i] - 'A' + 'a') : b[i];
        if (x != y) return false;
    }
    return true;
}
}  // namespace detail

inline void upsert(CfgTable& t, const std::string& tag, const CompCfg& c) {
    for (auto& r : t.rows)
        if (detail::tagEq(r.first, tag)) { r.second = c; return; }
    t.rows.emplace_back(tag, c);
}

inline const CompCfg& lookup(const CfgTable& t, const std::string& tag) {
    for (const auto& r : t.rows)
        if (detail::tagEq(r.first, tag)) return r.second;
    return t.def;
}

// MCF4: remove one tag's row so lookup falls back to `def` ("Follow global"). false when the tag has no row.
inline bool erase(CfgTable& t, const std::string& tag) {
    for (auto it = t.rows.begin(); it != t.rows.end(); ++it)
        if (detail::tagEq(it->first, tag)) { t.rows.erase(it); return true; }
    return false;
}

// MCF4: remove every per-tag row ("Apply defaults to everyone"); `def` is kept. Returns the number of rows removed.
inline size_t eraseAll(CfgTable& t) {
    const size_t n = t.rows.size();
    t.rows.clear();
    return n;
}

// MCF4: true when `tag` has its own [companions] row (false = it follows `def`).
inline bool hasRow(const CfgTable& t, const std::string& tag) {
    for (const auto& r : t.rows)
        if (detail::tagEq(r.first, tag)) return true;
    return false;
}

// WI-75 (BG28): the leader (member 0) keys its settings by creature tag, like companions.
// "slot0" is only the fallback when no tag could be read.
inline std::string leaderKey(const std::string& tag) {
    return tag.empty() ? std::string("slot0") : tag;
}

// WI-75: first-load migration of the pre-fix leader row. When the tagged leader has no row yet and a
// legacy "slot0" row exists, copy slot0 to the tag key. Returns true when a row was added (the caller
// then persists the table); false for an empty/"slot0" tag, an existing tag row, or no slot0 row.
inline bool migrateLeaderRow(CfgTable& t, const std::string& tag) {
    if (tag.empty() || tag == "slot0") return false;
    bool hasTag = false, hasLegacy = false;
    for (const auto& r : t.rows) {
        if (detail::tagEq(r.first, tag)) hasTag = true;
        if (r.first == "slot0") hasLegacy = true;
    }
    if (hasTag || !hasLegacy) return false;
    const CompCfg legacy = lookup(t, "slot0");   // copy first: upsert may reallocate t.rows
    upsert(t, tag, legacy);
    return true;
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

// pendingDone2 (CS12): while the use is still in the creature's action queue (queued) the item is merely waiting, so the pending
// state never times out; the 6 s timeout runs only over a continuous not-queued stretch. *unqueuedSince (0 = not running; the caller
// keeps it with the Pending and zeroes it when arming) is the tick the not-queued stretch began; a requeue resets it.
inline bool pendingDone2(const Pending& p, uint32_t now, int hp, int stack, bool queued, uint32_t* unqueuedSince) {
    if (hp > p.preHp || (stack >= 0 && stack < p.preStack) || (stack < 0 && p.preStack > 0)) return true;
    if (queued) { if (unqueuedSince) *unqueuedSince = 0; return false; }
    if (!unqueuedSince) return pendingTimedOut(p, now);
    // 0 is the "not running" sentinel, so a tick of 0 is read as 1 in both the stored start and the clock below
    const uint32_t t = now ? now : 1u;
    if (*unqueuedSince == 0) { *unqueuedSince = t; return false; }   // arming call: a fresh stretch has not timed out yet
    return (uint32_t)(t - *unqueuedSince) >= kPendingTimeoutMs;
}

// ---- slot owner (BG36 / CO-2d-04): pending/auto/cooldown state is kept per party slot, so it must follow the creature ----
// What to do with a slot's owner record when the creature now in the slot is `current` (0 = unreadable): Keep the state,
// Adopt `current` as the owner (nothing was recorded yet, nothing to reset), or Reset every per-slot field (another creature moved in).
enum class OwnerStep { Keep, Adopt, Reset };
inline OwnerStep ownerStep(uintptr_t recorded, uintptr_t current) {
    if (!current) return OwnerStep::Keep;
    if (!recorded) return OwnerStep::Adopt;
    return recorded == current ? OwnerStep::Keep : OwnerStep::Reset;
}

enum class UseRefusal { None, AlreadyQueued };
inline const char* useRefusalText(UseRefusal r) { return r == UseRefusal::AlreadyQueued ? "already queued" : ""; }
// A second heal use is refused while one is still queued (it would stack behind the stalled first).
inline UseRefusal useAllowed(bool queued) { return queued ? UseRefusal::AlreadyQueued : UseRefusal::None; }

// ---- look: colour / alpha / spinner ring per state (CS4) ----
constexpr int kRingFrames = 8;
constexpr float kRingR = 1.00f, kRingG = 0.80f, kRingB = 0.25f;   // gold tint for the busy ring
constexpr uint32_t kRingFrameMs = 90;   // 8 frames * 90 ms = ~0.7 s per revolution

struct Look {
    float r, g, b, alpha;
    int ringFrame;
    bool ring;
};

// ringFrame is always computed (wraps with uint32 now, 0 only at frame boundaries); ring says whether to draw it.
inline Look lookFor(BtnState st, bool autoOn, uint32_t now) {
    Look l{1.0f, 1.0f, 1.0f, 1.0f, (int)((now / kRingFrameMs) % (uint32_t)kRingFrames), autoOn && st != BtnState::Hidden};
    // The cross is a pre-shaded texture drawn untinted (white tint); auto mode is signalled by the gold ring (kRingR/G/B), not the cross.
    switch (st) {
        case BtnState::Hidden: l.alpha = 0.0f; break;
        case BtnState::Down: l.r = l.g = l.b = 0.55f; l.alpha = 0.40f; l.ring = false; l.ringFrame = 0; break;   // unconscious: no ring, no animation
        case BtnState::FullHp:
        case BtnState::NoItem: l.r = l.g = l.b = 0.55f; l.alpha = 0.40f; break;
        case BtnState::Pending: l.alpha = 0.65f; break;
        default: break;
    }
    return l;
}

// Square ring centred on the button, side = 1.5 * h rounded to the nearest even number.
inline Rect ringRect(Rect btn) {
    if (btn.h <= 0) return Rect{0, 0, 0, 0};
    int side = ringSideFor(btn.h);
    return Rect{btn.x + (btn.w - side) / 2, btn.y + (btn.h - side) / 2, side, side};
}

// Per-slot texture policy (FU-texinit): a non-null handle is returned as-is and never looked up again (the GuiKit image pool keeps it alive);
// a null handle is asked again on every call, ready or not (imageForRetry does not pool a null). A miss stays null (no sentinel).
// Header-only, std only: `lookup` is any callable returning void* (the caller passes the imageForRetry call for its slot).
template <class Lookup>
inline void* resolveTexSlot(void* cur, Lookup&& lookup) { return cur ? cur : lookup(); }

}  // namespace shui
