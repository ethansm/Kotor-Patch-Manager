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

struct Config {
    bool enabled = true;
    float bg[3] = { 0.10f, 0.55f, 0.20f };
    float bgAlpha = 0.45f;
    float border[3] = { 0.30f, 0.95f, 0.40f };
    float borderAlpha = 0.9f;
    int borderPx = 2;
    bool waitingDim = true;
    float waitingAlpha = 0.6f;
    bool log = true;
};

struct Style { float bg[3], bgA, border[3], borderA, iconA; int borderPx; };

// Slot 0 while waiting (and waitingDim) is dimmed: icon, background and border alpha all scale by waitingAlpha.
inline Style slotStyle(const Config& c, int slotIndex, bool waiting) {
    Style s;
    s.bg[0] = c.bg[0]; s.bg[1] = c.bg[1]; s.bg[2] = c.bg[2];
    s.border[0] = c.border[0]; s.border[1] = c.border[1]; s.border[2] = c.border[2];
    s.bgA = c.bgAlpha; s.borderA = c.borderAlpha; s.iconA = 1.0f; s.borderPx = c.borderPx;
    if (slotIndex == 0 && waiting && c.waitingDim) {
        s.iconA *= c.waitingAlpha; s.bgA *= c.waitingAlpha; s.borderA *= c.waitingAlpha;
    }
    return s;
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
}  // namespace detail

// One INI line. Blank, ';' / '#' comments and [section] lines are ignored (true). key=value (spaces/tabs trimmed, key case-insensitive):
//   enabled, waiting_dim, log (0/1/true/false/on/off/yes/no); bg_rgb / border_rgb ("r,g,b" floats, each clamped to 0..1);
//   bg_alpha, border_alpha, waiting_alpha (float, clamped to 0..1); border_px (a single digit 1..6; anything else is a bad value).
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
    if (detail::keyEq(kb, ke, "waiting_alpha")) return detail::parseUnit(vb, ve, &c.waitingAlpha);
    if (detail::keyEq(kb, ke, "log")) return detail::parseBool(vb, ve, &c.log);
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

}  // namespace qh
