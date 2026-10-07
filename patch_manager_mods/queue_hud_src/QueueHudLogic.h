// QueueHudLogic.h -- pure logic for the queue-hud mod (a GREEN copy of the vanilla combat action-queue panel, drawn out of combat).
// No engine calls, no Windows headers, no allocation: host-testable (queue_hud_logic_test.cpp). Header-only, C++17.
//
// Source of truth: patch_manager_mods/30_queue_hud_evidence/RE.md (vanilla 0x74DC80 panel filler, 0x74E0B0 icon resolver).
// The DLL reads the SERVER creature of party member 0: queue list (+0x100) -> QEntry[], round (+0x10DC): current node (+0xB18),
// pending node list (+0xB00) -> RNode[]. This header decides WHICH (entry, node) pairs fill the 4 slots; the DLL resolves icons
// by calling 0x74E0B0(round, action, node, ...) for each pair and caches them by Slot::key.
//
// Deliberate deviations from vanilla 0x74DC80 (each pinned by a test):
//   D1  vanilla only shows a 0x3F entry while combat flag F is set; we show it out of combat (that is the point of the mod).
//   D2  vanilla expands the round list once per 0x3F entry (duplicates); we expand only for the FIRST 0x3F entry.
//   D3  vanilla repeats the current node inside the expanded list; we skip the round node whose ptr equals cur->ptr when cur
//       already produced a slot.
//   D4  vanilla with a lone 0x3F and nothing to show would draw i_attack (0x74E572); out of combat that is meaningless, so no slot.
//   D5  vanilla's slot filler (0x74E7F0, force=0) does not consume a slot whose icon resolves blank; we mirror that by testing the
//       node sub-type against the per-type blank table (also for the cur node), so a blank icon never takes a slot.
#pragma once
#include <stdint.h>
#include <string.h>
#include "SelfHealUi.h"   // shui::buttonRect: the slim left-side icon size is the selfheal button side

namespace qh {

// ---- inputs (filled by the DLL from SafeRead-probed engine memory) ----
struct QEntry { int type; uint32_t param0; };   // queue action: type dword [A+0], first param [A+0x38] (spell id / item object id)
struct RNode {                                  // round action node
    uint8_t subType;                            // byte [N+0x10]
    uint32_t spellId;                           // [N+0x24]
    uint32_t itemId;                            // [N+0x64]
    uint32_t featId;                            // word [N+0x5C] (only the low 16 bits are significant)
    uint32_t ptr;                               // the node's address (nonzero); identity for dedupe
};

enum NodeSel : int { NODE_NONE = -1, NODE_CUR = -2 };   // 0..3 = index into the round list

struct Slot { int entry; int node; uint32_t key; };     // entry = index into the (capped) entry array

constexpr int kMaxSlots = 4;
constexpr int kMaxEntries = 16;
constexpr int kMaxRound = 4;

// Queue action types the vanilla panel shows.
constexpr int T_ROUND_ACTION = 0x0C;   // icon from the node (blank without one)
constexpr int T_CAST_SPELL = 0x0F;     // icon from entry param0 (spell id) only
constexpr int T_USE_ITEM = 0x2E;       // icon from entry param0 (item object id) only
constexpr int T_COMBAT = 0x3F;         // icon from the node only

inline bool isQualifying(int type) { return type == T_ROUND_ACTION || type == T_CAST_SPELL || type == T_USE_ITEM || type == T_COMBAT; }

// Round node sub-type -> does 0x74E0B0 produce a non-blank icon for it? (RE.md section 2 jump tables.)
//   type 0x0C table 0x74E780: 1, 6, 7, 9, 10, 11.   type 0x3F table 0x74E7AC: 1, 6, 7, 9, 10, 11, 13, 14.
inline bool nodeIconValid(int entryType, uint8_t sub) {
    switch (sub) {
        case 1: case 6: case 7: case 9: case 10: case 11: return entryType == T_ROUND_ACTION || entryType == T_COMBAT;
        case 13: case 14: return entryType == T_COMBAT;
        default: return false;
    }
}

// Stable cache key, never 0. FNV-1a over (tag, a): the same icon inputs always give the same key.
inline uint32_t mixKey(uint32_t tag, uint32_t a) {
    uint32_t h = 2166136261u;
    for (int i = 0; i < 4; ++i) { h ^= (tag >> (8 * i)) & 0xFFu; h *= 16777619u; }
    for (int i = 0; i < 4; ++i) { h ^= (a >> (8 * i)) & 0xFFu; h *= 16777619u; }
    return h ? h : 1u;
}

// Key of an entry-driven icon (0x0F spell / 0x2E item: depends only on the entry's first param).
inline uint32_t entryKey(const QEntry& e) { return mixKey(static_cast<uint32_t>(e.type), e.param0); }

// Key of a node-driven icon (0x0C / 0x3F): sub-type plus the id that sub-type's icon depends on (9 spell, 10 item, 11 feat word).
inline uint32_t nodeKey(const RNode& n) {
    uint32_t id = 0;
    if (n.subType == 9) id = n.spellId;
    else if (n.subType == 10) id = n.itemId;
    else if (n.subType == 11) id = n.featId & 0xFFFFu;
    return mixKey(0x100u | n.subType, id);
}

namespace detail {
// Callers guarantee n < kMaxSlots (every loop below tests it before a push).
inline void push(Slot out[kMaxSlots], int& n, int entry, int node, uint32_t key) {
    out[n].entry = entry; out[n].node = node; out[n].key = key;
    ++n;
}
}  // namespace detail

// Fills out[0..return) with up to 4 slots (slot 0 = the big one). Entry order = queue order; the first qualifying entry gets the
// current round node `cur` (nullable); the first 0x3F entry expands the pending round list. Inputs are capped: ne <= 16, nrn <= 4.
inline int buildSlots(const QEntry* e, int ne, const RNode* rn, int nrn, const RNode* cur, Slot out[kMaxSlots]) {
    if (!out || !e) return 0;
    if (!rn) nrn = 0;
    int n = 0;
    bool curTaken = false, expanded = false, curUsed = false;
    for (int i = 0; i < ne && i < kMaxEntries && n < kMaxSlots; ++i) {
        const int t = e[i].type;
        if (!isQualifying(t)) continue;
        const bool spellOrItem = (t == T_CAST_SPELL || t == T_USE_ITEM);
        if (cur && !curTaken) {
            // The current round node belongs to the first qualifying entry, even if its icon turns out blank (vanilla sets the flag).
            curTaken = true;
            if (spellOrItem) { detail::push(out, n, i, NODE_CUR, entryKey(e[i])); curUsed = true; }
            else if (nodeIconValid(t, cur->subType)) { detail::push(out, n, i, NODE_CUR, nodeKey(*cur)); curUsed = true; }
        } else if (spellOrItem) {
            // Later entries (or any entry when there is no current node) show the entry's own icon. 0x0C / 0x3F have none without a node.
            detail::push(out, n, i, NODE_NONE, entryKey(e[i]));
        }
        if (t == T_COMBAT && !expanded) {
            expanded = true;
            for (int k = 0; k < nrn && k < kMaxRound && n < kMaxSlots; ++k) {
                if (curUsed && rn[k].ptr == cur->ptr) continue;
                if (!nodeIconValid(T_COMBAT, rn[k].subType)) continue;
                detail::push(out, n, i, k, nodeKey(rn[k]));
            }
        }
    }
    return n;
}

// The head action is "waiting" (vanilla panel shows it dimmed while the creature is paused / has a delayed action).
inline bool isWaiting(bool paused, int ab8, int aa8) { return paused || ab8 >= 1 || aa8 != 0; }

// Draw only outside combat, outside menu frames, for a fresh snapshot (< 250 ms old) with something to show.
inline bool shouldShow(bool enabled, bool combat, bool inMenu, uint32_t ageMs, int nSlots) {
    return enabled && !combat && !inMenu && ageMs < 250u && nSlots > 0;
}

// Dark teal palette matching custom GUI theme (Session 2)
constexpr float kTealBg[3]     = { 0.05f, 0.25f, 0.28f }; // background
constexpr float kTealBorder[3] = { 0.10f, 0.58f, 0.62f }; // border highlight
constexpr float kTealDim[3]    = { 0.03f, 0.15f, 0.18f }; // waiting / dimmed

struct Config {
    bool enabled = true;
    float bg[3] = { kTealBg[0], kTealBg[1], kTealBg[2] };
    float bgAlpha = 0.45f;
    float border[3] = { kTealBorder[0], kTealBorder[1], kTealBorder[2] };
    float borderAlpha = 0.9f;
    int borderPx = 2;
    bool waitingDim = true;
    float waitingAlpha = 0.6f;
    float waitingBg[3] = { kTealDim[0], kTealDim[1], kTealDim[2] };
    bool log = true;
    // 0.2.0: companion queue strips (party members 1-2).
    bool companions = true;
    int stripIconPx = 0;          // 0 = auto (60% of the vit-rect height, min 12)
    int stripSide = 0;            // strip_side: kSideLeft (default) | kSideAbove | kSideBelow
    int stripMax = 4;             // 1..4 icons per strip
    bool kit = true;              // 0.3.0: kit frames + Cancel controls (default on)
    float frameAlpha = 0.9f;      // qh_frame alpha (scaled by waiting_alpha for the dimmed head slot)
    bool cancel = true;           // the Cancel controls (leader + companion strips)
};

struct Style { float bg[3], bgA, border[3], borderA, iconA, frameA; int borderPx; };

// Slot 0 while waiting (and waitingDim) is dimmed: icon, background and border alpha all scale by waitingAlpha; bg recoloured to waitingBg.
inline Style slotStyle(const Config& c, int slotIndex, bool waiting) {
    Style s;
    s.bg[0] = c.bg[0]; s.bg[1] = c.bg[1]; s.bg[2] = c.bg[2];
    s.border[0] = c.border[0]; s.border[1] = c.border[1]; s.border[2] = c.border[2];
    s.bgA = c.bgAlpha; s.borderA = c.borderAlpha; s.iconA = 1.0f; s.frameA = c.frameAlpha; s.borderPx = c.borderPx;
    if (slotIndex == 0 && waiting && c.waitingDim) {
        s.bg[0] = c.waitingBg[0]; s.bg[1] = c.waitingBg[1]; s.bg[2] = c.waitingBg[2];
        s.iconA *= c.waitingAlpha; s.bgA *= c.waitingAlpha; s.borderA *= c.waitingAlpha; s.frameA *= c.waitingAlpha;
    }
    return s;
}

// ---- companion strips (0.2.0) ----
struct Rect { int x, y, w, h; };

// strip_side. left (default): right of the row ends left of the vit bar, clear of the selfheal button + ring zone, vertically centred
// on the bar. above / below: on the card's top / bottom edge (live 3840x1600: small cards are 85 px tall and 104 px apart, so above /
// below only fit with a small strip_icon_px).
enum : int { kSideLeft = 0, kSideAbove = 1, kSideBelow = 2 };

constexpr int kStripGap = 2;      // px between icons, and between the strip and the card
constexpr int kStripMinIcon = 12;

// Auto icon size calculation: vit.h * 0.40f (~38px on 96px health bar), clamped [32, 48].
inline int autoIconPx(int vitH) {
    int px = (vitH * 40) / 100;
    return px < 32 ? 32 : (px > 48 ? 48 : px);
}

// Icon edge in px: the configured size, or auto. Auto, left side: 40% of vit height clamped [32, 48]
// (~38 px on the live 47x96 small vit). Above / below: 60% of the vit height. Rounded, at least kStripMinIcon.
inline int stripIconSize(Rect vit, const Config& c) {
    if (c.stripIconPx > 0) return c.stripIconPx;
    int s = 0;
    if (c.stripSide == kSideLeft) {
        s = autoIconPx(vit.h);
    } else {
        s = (vit.h * 6 + 5) / 10;
    }
    return s < kStripMinIcon ? kStripMinIcon : s;
}

// Row width of n icons (n clamped to strip_max and kMaxSlots), 2px apart: the width queue-hud registers at hud-host strip order 1.
inline int stripRowWidth(Rect vit, int n, const Config& c) {
    if (n <= 0) return 0;
    const int cap = c.stripMax < 1 ? 1 : (c.stripMax > kMaxSlots ? kMaxSlots : c.stripMax);
    if (n > cap) n = cap;
    return n * stripIconSize(vit, c) + (n - 1) * kStripGap;
}

// A horizontal row of n square icons (n clamped to strip_max and 4), 2px apart, in queue order. left: the row ends `reserve`
// px left of vit.x (the selfheal footprint: hud-host strip slots, or shui::stripReserve(vit) as fallback), centred on the vit bar's height. above / below: left-aligned on the card, bottom edge 2px above the card top /
// top edge 2px below the card bottom. screenW > 0 shifts the row left so it stays on screen (never
// left of x=0). Returns the icon count written to out[0..); 0 for n <= 0 or an unusable card / vit rect.
inline int stripRects(Rect card, Rect vit, int n, const Config& c, Rect out[kMaxSlots], int screenW, int reserve) {
    if (!out || n <= 0 || card.w <= 0 || card.h <= 0 || vit.h <= 0) return 0;
    int cap = c.stripMax < 1 ? 1 : (c.stripMax > kMaxSlots ? kMaxSlots : c.stripMax);
    if (n > cap) n = cap;
    const int s = stripIconSize(vit, c);
    const int rowW = n * s + (n - 1) * kStripGap;
    int y = 0, x0 = 0;
    if (c.stripSide == kSideAbove || c.stripSide == kSideBelow) {
        y = c.stripSide == kSideBelow ? card.y + card.h + kStripGap : card.y - kStripGap - s;
        x0 = card.x;
    } else {
        y = vit.y + (vit.h - s) / 2;
        x0 = vit.x - reserve - rowW;
        if (x0 < 0) x0 = 0;
    }
    if (screenW > 0) {
        const int over = x0 + rowW - screenW;
        if (over > 0) x0 -= over;
        if (x0 < 0) x0 = 0;
    }
    for (int i = 0; i < n; ++i) { out[i].x = x0 + i * (s + kStripGap); out[i].y = y; out[i].w = s; out[i].h = s; }
    return n;
}

// ---- 0.3.0: kit frame + Cancel geometry ----
// The icon sits 15% inside the qh_frame art on every side (floor, so a 20 px slot insets 3 px and keeps a 14 px icon).
inline Rect insetRect(Rect r) {
    const int dx = r.w * 15 / 100, dy = r.h * 15 / 100;
    return Rect{ r.x + dx, r.y + dy, r.w - 2 * dx, r.h - 2 * dy };
}

// Left pointer chevron indicator pointing right into Slot 0 (vanilla combat layout parity)
inline Rect chevronRect(Rect q0) {
    if (q0.w <= 0 || q0.h <= 0) return Rect{ 0, 0, 0, 0 };
    int w = (q0.w * 18 + 20) / 40;
    int h = (q0.h * 26 + 20) / 40;
    if (w < 8) w = 8;
    if (h < 10) h = 10;
    int gap = (q0.w * 4 + 20) / 40;
    int right = q0.x - gap;
    int x = right - w;
    int y = q0.y + (q0.h - h) / 2;
    return Rect{ x, y, w, h };
}

// Leader Cancel: wide rounded bar beneath Slot 0 matching native combat BTN_CLEARALL [317,570,166,24] vs QUEUE0 [432,521,40,40].
inline Rect cancelRect(Rect q0, Rect q1) {
    (void)q1;
    if (q0.w <= 0 || q0.h <= 0) return Rect{ 0, 0, 0, 0 };
    int w = (q0.w * 166 + 20) / 40;
    int h = (q0.h * 24 + 20) / 40;
    int gap = (q0.h * 9 + 20) / 40;
    int right = q0.x + q0.w + (q0.w * 11 + 20) / 40;
    int x = right - w;
    int y = q0.y + q0.h + gap;
    return Rect{ x, y, w, h };
}

// Companion Cancel: a square of the strip icon size placed just LEFT of the strip's first icon (gap = max(icon / 4, kStripGap)).
// strip_side left: the row already ends `reserve` left of the vit bar, so the cancel square lies further left, clear of the
// selfheal button + ring; when it would leave the screen (x < 0) there is no cancel (never move it into the reserve). above / below:
// the row starts at the card's left edge, nothing of selfheal is there; at x < 0 it goes just RIGHT of the last icon instead. A square
// that would pass screenW (> 0) is refused. {0,0,0,0} = none.
inline Rect stripCancelRect(const Rect* icons, int n, int side, int screenW = 0) {
    if (!icons || n <= 0 || icons[0].w <= 0 || icons[0].h <= 0) return Rect{ 0, 0, 0, 0 };
    const int s = icons[0].w;
    int gap = s / 4;
    if (gap < kStripGap) gap = kStripGap;
    Rect r{ icons[0].x - gap - s, icons[0].y, s, s };
    if (r.x < 0) {
        if (side == kSideLeft) return Rect{ 0, 0, 0, 0 };
        const Rect& last = icons[n - 1];
        r.x = last.x + last.w + gap;
    }
    if (screenW > 0 && r.x + r.w > screenW) return Rect{ 0, 0, 0, 0 };
    return r;
}

// A Cancel control is shown exactly when its queue is (the same out-of-combat rule); `drawn` = icons actually drawn for that queue.
inline bool cancelVisible(bool enabled, bool cancelOn, bool combat, bool inMenu, uint32_t ageMs, int drawn) {
    return cancelOn && shouldShow(enabled, combat, inMenu, ageMs, drawn);
}

// ---- INI ----
namespace detail {
inline bool isSp(char ch) { return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n'; }
inline char lower(char ch) { return (ch >= 'A' && ch <= 'Z') ? static_cast<char>(ch - 'A' + 'a') : ch; }
inline void trim(const char*& b, const char*& e) {
    while (b < e && isSp(*b)) ++b;
    while (e > b && isSp(e[-1])) --e;
}
inline const char* findCh(const char* b, const char* e, char ch) { while (b < e && *b != ch) ++b; return b; }
inline bool keyEq(const char* b, const char* e, const char* lit) {   // case-insensitive [b,e) == lit
    long n = 0;
    while (lit[n]) ++n;
    if (e - b != n) return false;
    for (long i = 0; i < n; ++i) if (lower(b[i]) != lit[i]) return false;
    return true;
}
// [+-]?digits[.digits], whole (trimmed) range consumed: no exponent / inf / nan / hex.
inline bool parseFloatRange(const char* b, const char* e, float* out) {
    trim(b, e);
    bool neg = false;
    if (b < e && (*b == '+' || *b == '-')) { neg = (*b == '-'); ++b; }
    double v = 0.0, scale = 1.0;
    int digits = 0;
    for (; b < e && *b >= '0' && *b <= '9'; ++b, ++digits) v = v * 10.0 + (*b - '0');
    if (b < e && *b == '.') {
        ++b;
        for (; b < e && *b >= '0' && *b <= '9'; ++b, ++digits) { scale /= 10.0; v += (*b - '0') * scale; }
    }
    if (digits == 0 || b != e) return false;
    *out = static_cast<float>(neg ? -v : v);
    return true;
}
inline float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
inline bool parseUnit(const char* b, const char* e, float* out) {
    float v;
    if (!parseFloatRange(b, e, &v)) return false;
    *out = clamp01(v);
    return true;
}
// "r,g,b": exactly three comma-separated floats, each clamped; nothing is written unless all three parse.
inline bool parseRgb(const char* b, const char* e, float out[3]) {
    const char* c1 = findCh(b, e, ',');
    if (c1 == e) return false;
    const char* c2 = findCh(c1 + 1, e, ',');
    if (c2 == e) return false;
    if (findCh(c2 + 1, e, ',') != e) return false;   // a 4th component
    float v0, v1, v2;
    if (!parseUnit(b, c1, &v0) || !parseUnit(c1 + 1, c2, &v1) || !parseUnit(c2 + 1, e, &v2)) return false;
    out[0] = v0; out[1] = v1; out[2] = v2;
    return true;
}
inline bool parseBool(const char* b, const char* e, bool* out) {
    if (keyEq(b, e, "1") || keyEq(b, e, "true") || keyEq(b, e, "on") || keyEq(b, e, "yes")) { *out = true; return true; }
    if (keyEq(b, e, "0") || keyEq(b, e, "false") || keyEq(b, e, "off") || keyEq(b, e, "no")) { *out = false; return true; }
    return false;
}
// A single digit 1..6.
inline bool parsePx(const char* b, const char* e, int* out) {
    if (e - b != 1 || *b < '1' || *b > '6') return false;
    *out = *b - '0';
    return true;
}
inline bool parseStripPx(const char* b, const char* e, int* out) {   // 0 (auto) or 8..96 integer px
    trim(b, e);
    if (b == e || e - b > 2) return false;
    int v = 0;
    for (const char* p = b; p < e; ++p) { if (*p < '0' || *p > '9') return false; v = v * 10 + (*p - '0'); }
    if (v != 0 && (v < 8 || v > 96)) return false;
    *out = v;
    return true;
}
inline bool parseStripMax(const char* b, const char* e, int* out) {   // a single digit 1..4
    if (e - b != 1 || *b < '1' || *b > '4') return false;
    *out = *b - '0';
    return true;
}
inline bool parseSide(const char* b, const char* e, int* side) {
    if (keyEq(b, e, "left")) { *side = kSideLeft; return true; }
    if (keyEq(b, e, "above")) { *side = kSideAbove; return true; }
    if (keyEq(b, e, "below")) { *side = kSideBelow; return true; }
    return false;
}
}  // namespace detail

// One INI line. Blank, ';' / '#' comments and [section] lines are ignored (true). key=value (spaces/tabs trimmed, key case-insensitive):
//   enabled, waiting_dim, log (0/1/true/false/on/off/yes/no); bg_rgb / border_rgb ("r,g,b" floats, each clamped to 0..1);
//   bg_alpha, border_alpha, waiting_alpha (float, clamped to 0..1); border_px (a single digit 1..6; anything else is a bad value).
//   0.2.0: companions (bool), strip_icon_px (0 = auto, else 8..96), strip_side (left | above | below), strip_max (a single digit 1..4).
//   0.3.0: kit (bool), frame_alpha (float 0..1), cancel (bool).
// Unknown key, missing '=' or a bad value returns false and leaves the field unchanged.
inline bool parseIniLine(Config& c, const char* line) {
    if (!line) return false;
    const char* b = line;
    const char* e = line;
    while (*e) ++e;
    detail::trim(b, e);
    if (b == e || *b == ';' || *b == '#' || *b == '[') return true;
    const char* eq = detail::findCh(b, e, '=');
    if (eq == e) return false;
    const char* kb = b; const char* ke = eq;
    detail::trim(kb, ke);
    const char* vb = eq + 1; const char* ve = e;
    detail::trim(vb, ve);
    if (detail::keyEq(kb, ke, "enabled")) return detail::parseBool(vb, ve, &c.enabled);
    if (detail::keyEq(kb, ke, "bg_rgb")) return detail::parseRgb(vb, ve, c.bg);
    if (detail::keyEq(kb, ke, "bg_alpha")) return detail::parseUnit(vb, ve, &c.bgAlpha);
    if (detail::keyEq(kb, ke, "border_rgb")) return detail::parseRgb(vb, ve, c.border);
    if (detail::keyEq(kb, ke, "border_alpha")) return detail::parseUnit(vb, ve, &c.borderAlpha);
    if (detail::keyEq(kb, ke, "border_px")) return detail::parsePx(vb, ve, &c.borderPx);
    if (detail::keyEq(kb, ke, "waiting_dim")) return detail::parseBool(vb, ve, &c.waitingDim);
    if (detail::keyEq(kb, ke, "waiting_dim_rgb")) return detail::parseRgb(vb, ve, c.waitingBg);
    if (detail::keyEq(kb, ke, "waiting_alpha")) return detail::parseUnit(vb, ve, &c.waitingAlpha);
    if (detail::keyEq(kb, ke, "log")) return detail::parseBool(vb, ve, &c.log);
    if (detail::keyEq(kb, ke, "companions")) return detail::parseBool(vb, ve, &c.companions);
    if (detail::keyEq(kb, ke, "strip_icon_px")) return detail::parseStripPx(vb, ve, &c.stripIconPx);
    if (detail::keyEq(kb, ke, "strip_side")) return detail::parseSide(vb, ve, &c.stripSide);
    if (detail::keyEq(kb, ke, "strip_max")) return detail::parseStripMax(vb, ve, &c.stripMax);
    if (detail::keyEq(kb, ke, "kit")) return detail::parseBool(vb, ve, &c.kit);
    if (detail::keyEq(kb, ke, "frame_alpha")) return detail::parseUnit(vb, ve, &c.frameAlpha);
    if (detail::keyEq(kb, ke, "cancel")) return detail::parseBool(vb, ve, &c.cancel);
    return false;
}

// Whole INI text (\n or \r\n lines). Bad lines are skipped (their fields keep the previous value). Lines over 255 bytes are ignored.
inline void parseIni(Config& c, const char* text) {
    if (!text) return;
    char buf[256];
    const char* p = text;
    while (*p) {
        const char* q = p;
        while (*q && *q != '\n') ++q;
        const unsigned len = static_cast<unsigned>(q - p);
        if (len < sizeof buf) {
            memcpy(buf, p, len);
            buf[len] = 0;
            parseIniLine(c, buf);
        }
        p = *q ? q + 1 : q;
    }
}

// ---- Multi-Item Action Buffer & Follower Wipe Guard (Workstream 4) ----
struct QueuedItemAction {
    uintptr_t creature = 0;
    uint32_t clientItem = 0;
    int packedByte = 0;
    uint32_t targetId = 0;
    float pos[3] = {};
    uint32_t queueTime = 0;
};

constexpr int kMaxItemQueue = 3;

struct ItemBuffer {
    QueuedItemAction queue[kMaxItemQueue];
    int count = 0;
    bool active = false;
    QueuedItemAction activeAction;
    uint32_t activeStartTick = 0;
    bool reInjectPending = false;

    bool empty() const { return count == 0; }
    bool full() const { return count >= kMaxItemQueue; }

    bool push(const QueuedItemAction& a) {
        if (count >= kMaxItemQueue) return false;
        queue[count++] = a;
        return true;
    }

    bool pop(QueuedItemAction* out = nullptr) {
        if (count <= 0) return false;
        if (out) *out = queue[0];
        for (int i = 1; i < count; ++i) queue[i - 1] = queue[i];
        --count;
        return true;
    }

    void clear() {
        count = 0;
        active = false;
        activeAction = QueuedItemAction{};
        activeStartTick = 0;
        reInjectPending = false;
    }
};

enum ActionDecision : int {
    DECIDE_DISPATCH_NOW = 0,
    DECIDE_BUFFER = 1,
    DECIDE_DROP_FULL = 2
};

inline ActionDecision decideItemAction(const ItemBuffer& buf, bool inCombat, int engineQueueLen) {
    if (inCombat) return DECIDE_DISPATCH_NOW;
    if (engineQueueLen == 0 && !buf.active && buf.empty()) return DECIDE_DISPATCH_NOW;
    if (buf.full()) return DECIDE_DROP_FULL;
    return DECIDE_BUFFER;
}

inline bool shouldReInject(const ItemBuffer& buf, bool itemInQueue, bool hasFollowLeader, bool paused, uint32_t now) {
    if (!buf.active || buf.reInjectPending) return false;
    if (itemInQueue) return false;
    uint32_t elapsed = now - buf.activeStartTick;
    return hasFollowLeader || (paused && elapsed < 3000);
}

inline bool shouldDispatchNext(const ItemBuffer& buf, int engineQueueLen) {
    return !buf.active && !buf.empty() && engineQueueLen == 0;
}

}  // namespace qh
