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
// showInCombat (show_in_combat=1) drops the combat term only; enabled, menu, age and slot gates still apply.
inline bool shouldShow(bool enabled, bool combat, bool inMenu, uint32_t ageMs, int nSlots, bool showInCombat = false) {
    return enabled && (showInCombat || !combat) && !inMenu && ageMs < 250u && nSlots > 0;
}

// Panel hold: keep drawing the last good panel for kPanelHoldMs after a read miss. Never across a menu or a combat hide.
// lastGoodTick == 0 means no good read yet, so nothing is held.
constexpr uint32_t kPanelHoldMs = 400;
inline bool panelHold(uint32_t lastGoodTick, uint32_t now, bool inMenu, bool combatHidden) {
    return !inMenu && !combatHidden && lastGoodTick != 0 && now - lastGoodTick < kPanelHoldMs;
}

// The active slot is needed only when the buffer holds an active action and the engine does not already hold one.
inline bool activeNeedsSlot(bool bufActive, bool engineHolds) { return bufActive && !engineHolds; }

// handoff_clear_first: clear the engine queue before the first handoff send (nothing sent yet this handoff).
// handoff_keep_new (QL10 probe): keepNew skips the clear so the player's new click survives; the engine then orders the queue itself.
inline bool clearBeforeHandoff(bool cfgOn, int sentSoFar, bool keepNew = false) { return cfgOn && !keepNew && sentSoFar == 0; }
// True on the first send of a hand-off that keeps the engine queue (logged as HANDOFF_KEEP).
inline bool keepNewAtHandoff(bool cfgOn, bool keepNew, int sentSoFar) { return cfgOn && keepNew && sentSoFar == 0; }

// handoff_resend_held: the active command the engine still held at the combat edge (a thrown grenade / medpack that triggered combat).
// The hand-off waits while the engine still holds it, then settles by whether it ever ran (QL6c; the old 300 ms clock dropped real casts):
//   held                     -> WAIT, or DROP when it has sat held for kHeldStuckMs of unpaused time (stuck)
//   gone and sawExec         -> DROP   (it ran: aa8/ab8 or a cur subtype 9/10 was seen; never run twice)
//   gone and not sawExec     -> RESEND (wiped before it ran, e.g. a combat-start wipe or a paused eat)
constexpr int kHeldStuckMs = 3000;
enum HeldHeadAction : uint8_t { HELD_WAIT, HELD_RESEND, HELD_DROP };
inline HeldHeadAction heldHeadAction(bool held, bool sawExec, bool paused, int unpausedHeldMs) {
    if (held) return (!paused && unpausedHeldMs > kHeldStuckMs) ? HELD_DROP : HELD_WAIT;
    return sawExec ? HELD_DROP : HELD_RESEND;
}

// Dark teal palette matching custom GUI theme (Session 2)
constexpr float kTealBg[3]     = { 0.05f, 0.25f, 0.28f }; // background
constexpr float kTealBorder[3] = { 0.10f, 0.58f, 0.62f }; // border highlight
constexpr float kTealDim[3]    = { 0.03f, 0.15f, 0.18f }; // waiting / dimmed

// Cancel Queue button. Border and fill share ONE tint field (GuiKit border+0x28), so cwLookLeader passes the same rgb to both.
// Leader: dark green-teal, the measured mean of lbl_miqueue_teal.tga (QH5; user play test 2026-10-11: the old black did not match the menu).
struct CancelStyle { float tint[3]; float hoverTint[3]; float alpha; };
inline CancelStyle cancelStyle(bool leader) {
    if (leader) return CancelStyle{ { 0.033f, 0.233f, 0.182f }, { 0.067f, 0.514f, 0.404f }, 0.85f };
    return CancelStyle{ { kTealBorder[0], kTealBorder[1], kTealBorder[2] }, { 0.15f, 0.75f, 0.80f }, 1.0f };
}

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
    bool traceQueue = true;       // trace_queue: per-command CMD#/WAIT/RESOLVE/DROP/EDGE/QJ log lines
    // 0.2.0: companion queue strips (party members 1-2).
    bool companions = true;
    int stripIconPx = 0;          // 0 = auto (60% of the vit-rect height, min 12)
    int stripSide = 0;            // strip_side: kSideLeft (default) | kSideAbove | kSideBelow
    int stripMax = 4;             // 1..4 icons per strip
    bool kit = true;              // 0.3.0: kit frames + Cancel controls (default on)
    float frameAlpha = 0.9f;      // qh_frame alpha (scaled by waiting_alpha for the dimmed head slot)
    bool cancel = true;           // the Cancel controls (leader + companion strips)
    bool pauseReinject = true;    // pause_reinject: re-inject a paused/eaten action (default on)
    bool dispatchWaitFlags = true; // dispatch_wait_flags: hold a buffered dispatch while the engine's aa8/ab8 flags are set (default on)
    bool traceClicks = true;       // trace_clicks: one CLICK line per left-button press edge (needs trace_queue)
    bool pauseEatenWait = true;   // pause_eaten_wait: while paused, an eaten action waits (no re-inject, no budget) until unpause (default on)
    bool handoffOnCombat = true;  // handoff_on_combat: hand the buffered queue over when combat starts (default on)
    bool showInCombat = true;     // show_in_combat: keep the queue strip drawn during combat (default on)
    bool handoffClearFirst = true;  // handoff_clear_first: clear the engine queue before the first handoff send (default on)
    bool handoffKeepNew = true;     // handoff_keep_new: do NOT clear the engine queue at the hand-off (keeps the new click; 0 = clear-first behaviour)
    bool handoffResendHeld = true;  // handoff_resend_held: re-send an engine-held active command the engine wipes at combat start (default on)
    bool spellReplayPos = true;     // spell_replay_pos: replay a buffered spell with a NULL position when the engine's original was NULL (0 = always pass the zero-filled copy)
    bool hideBannerSoft = true;     // hide_banner_soft: hide the top combat banner while combat is "soft" (only own-party targets, no hostile)
    bool hideRedbarSoft = true;     // hide_redbar_soft: hide the bottom red combat bar (Cancel Combat) while combat is soft
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

// Auto icon size calculation: vit.h * 0.60f (~58px on 96px health bar), clamped [48, 72].
inline int autoIconPx(int vitH) {
    int px = (vitH * 6 + 5) / 10;
    return px < 48 ? 48 : (px > 72 ? 72 : px);
}

// Icon edge in px: the configured size, or auto. Auto, left side: autoIconPx (60% of vit height clamped [48, 72],
// ~58 px on the live 96 px vit). Above / below: 60% of the vit height. Rounded, at least kStripMinIcon.
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

// Vanilla combat layout parity: LBL_COMBATBG3 [310, 498, 180, 90] relative to QUEUE0 [432, 521, 40, 40]
inline Rect combatBgRect(Rect q0) {
    if (q0.w <= 0 || q0.h <= 0) return Rect{ 0, 0, 0, 0 };
    int w = (q0.w * 180 + 20) / 40;
    int h = (q0.h * 90 + 20) / 40;
    int dx = (q0.w * 122 + 20) / 40;
    int dy = (q0.h * 23 + 20) / 40;
    return Rect{ q0.x - dx, q0.y - dy, w, h };
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

// A companion's strip is hidden only for Down (HP <= 0) with readable vitals: unknown vitals (ok=false) keep the strip as before.
inline bool stripHiddenForDown(bool vitalsOk, bool down) { return vitalsOk && down; }

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
    if (detail::keyEq(kb, ke, "trace_queue")) return detail::parseBool(vb, ve, &c.traceQueue);
    if (detail::keyEq(kb, ke, "companions")) return detail::parseBool(vb, ve, &c.companions);
    if (detail::keyEq(kb, ke, "strip_icon_px")) return detail::parseStripPx(vb, ve, &c.stripIconPx);
    if (detail::keyEq(kb, ke, "strip_side")) return detail::parseSide(vb, ve, &c.stripSide);
    if (detail::keyEq(kb, ke, "strip_max")) return detail::parseStripMax(vb, ve, &c.stripMax);
    if (detail::keyEq(kb, ke, "kit")) return detail::parseBool(vb, ve, &c.kit);
    if (detail::keyEq(kb, ke, "frame_alpha")) return detail::parseUnit(vb, ve, &c.frameAlpha);
    if (detail::keyEq(kb, ke, "cancel")) return detail::parseBool(vb, ve, &c.cancel);
    if (detail::keyEq(kb, ke, "pause_reinject")) return detail::parseBool(vb, ve, &c.pauseReinject);
    if (detail::keyEq(kb, ke, "trace_clicks")) return detail::parseBool(vb, ve, &c.traceClicks);
    if (detail::keyEq(kb, ke, "dispatch_wait_flags")) return detail::parseBool(vb, ve, &c.dispatchWaitFlags);
    if (detail::keyEq(kb, ke, "pause_eaten_wait")) return detail::parseBool(vb, ve, &c.pauseEatenWait);
    if (detail::keyEq(kb, ke, "handoff_on_combat")) return detail::parseBool(vb, ve, &c.handoffOnCombat);
    if (detail::keyEq(kb, ke, "show_in_combat")) return detail::parseBool(vb, ve, &c.showInCombat);
    if (detail::keyEq(kb, ke, "handoff_clear_first")) return detail::parseBool(vb, ve, &c.handoffClearFirst);
    if (detail::keyEq(kb, ke, "handoff_keep_new")) return detail::parseBool(vb, ve, &c.handoffKeepNew);
    if (detail::keyEq(kb, ke, "handoff_resend_held")) return detail::parseBool(vb, ve, &c.handoffResendHeld);
    if (detail::keyEq(kb, ke, "spell_replay_pos")) return detail::parseBool(vb, ve, &c.spellReplayPos);
    if (detail::keyEq(kb, ke, "hide_banner_soft")) return detail::parseBool(vb, ve, &c.hideBannerSoft);
    if (detail::keyEq(kb, ke, "hide_redbar_soft")) return detail::parseBool(vb, ve, &c.hideRedbarSoft);
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

// ---- Unified Action Buffer & Follower Wipe Guard (Workstreams 4 & QH4) ----
enum class ActionKind : uint8_t {
    Item = 0,
    Spell = 1
};

struct QueuedAction {
    uintptr_t creature = 0;
    uint32_t clientItem = 0; // itemId for Item, spellId for Spell
    int packedByte = 0;      // packedByte for Item, arg2 for Spell
    uint32_t targetId = 0;
    float pos[3] = {};
    uint32_t queueTime = 0;
    ActionKind kind = ActionKind::Item;
    uint32_t spellId = 0;    // spellId for Spell
    int arg2 = 0;            // arg2 for Spell
    int multispell = 0;
    int metamagic = 0;
    int domain = 0;
    int reInjectCount = 0;
    bool hasPos = false;     // the engine's original call carried a non-null position pointer (Spell capture)
    uint32_t cmdId = 0;      // host command number (CMD#<n>); 0 = unassigned. Keep as the last member.
};
using QueuedItemAction = QueuedAction;

inline QueuedAction makeItemAction(uintptr_t creature, uint32_t itemId, int packedByte, uint32_t targetId,
                                   const float* pos, uint32_t queueTime, uint32_t cmdId = 0) {
    QueuedAction a;
    a.creature = creature;
    a.clientItem = itemId;
    a.packedByte = packedByte;
    a.targetId = targetId;
    if (pos) { a.pos[0] = pos[0]; a.pos[1] = pos[1]; a.pos[2] = pos[2]; }
    a.queueTime = queueTime;
    a.kind = ActionKind::Item;
    a.spellId = 0;
    a.arg2 = 0;
    a.multispell = 0;
    a.metamagic = 0;
    a.domain = 0;
    a.reInjectCount = 0;
    a.cmdId = cmdId;
    return a;
}

inline QueuedAction makeSpellAction(uintptr_t creature, uint32_t spellId, int arg2, uint32_t targetId,
                                    int multispell, int metamagic, int domain, const float* pos, uint32_t queueTime,
                                    uint32_t cmdId = 0) {
    QueuedAction a;
    a.creature = creature;
    a.clientItem = spellId;
    a.packedByte = arg2;
    a.targetId = targetId;
    if (pos) { a.pos[0] = pos[0]; a.pos[1] = pos[1]; a.pos[2] = pos[2]; }
    a.queueTime = queueTime;
    a.kind = ActionKind::Spell;
    a.hasPos = (pos != nullptr);
    a.spellId = spellId;
    a.arg2 = arg2;
    a.multispell = multispell;
    a.metamagic = metamagic;
    a.domain = domain;
    a.reInjectCount = 0;
    a.cmdId = cmdId;
    return a;
}

// Position pointer for the replayed call: a spell the engine itself called with a NULL position is replayed with NULL
// (useOrigPos), never with a pointer to the zero-filled copy; everything else keeps the stored copy.
inline const float* replayPos(const QueuedAction& a, bool useOrigPos) {
    return (a.kind == ActionKind::Spell && useOrigPos && !a.hasPos) ? nullptr : a.pos;
}

constexpr int kMaxActionQueue = 3;
constexpr int kMaxItemQueue = kMaxActionQueue;
constexpr int kMaxReInjectCount = 2;

// Eaten-action verdict timing (activeVerdict). Milliseconds, measured on the caller's tick clock.
constexpr uint32_t kVanishDebounceMs = 150;    // gone this long before it counts as vanished
constexpr uint32_t kMinActiveMs = 500;         // a just-dispatched action is never judged for this long
constexpr int kMaxEatenReInject = 6;           // re-inject budget per eaten episode
constexpr uint32_t kReInjectSpacingMs = 250;   // minimum gap between two re-injects
constexpr uint32_t kEatenWaitMaxMs = 20000;    // unpaused time spent eaten before giving up
constexpr uint32_t kStaleCurMs = 1500;         // a flagless item/spell round node older than this is stale
constexpr uint32_t kReInjectResetMs = 2000;    // a present action this long after the last re-inject clears the budget

struct ActionBuffer {
    QueuedAction queue[kMaxActionQueue];
    int count = 0;
    bool active = false;
    QueuedAction activeAction;
    uint32_t activeStartTick = 0;
    bool reInjectPending = false;
    bool userCancelled = false;
    bool seenInQueue = false;   // active action was seen in the engine queue (RESOLVE seen=); reset by cancel()/clear()
    // eaten-action bookkeeping (appended; cancel() and clear() reset every one of them)
    bool sawExecuting = false;      // the engine showed the active action executing
    bool eaten = false;             // the active action vanished without executing (queue wipe)
    bool staleCurLogged = false;
    bool pausedEaten = false;       // eaten while paused (waiting for unpause; the unpause edge resets the re-inject budget)
    int eatenReInjects = 0;         // re-injects spent in the current eaten episode
    uint32_t goneSinceTick = 0;     // first tick the action was absent in a row (0 = not absent)
    uint32_t lastReInjectTick = 0;
    uint32_t staleCurSince = 0;     // first tick a flagless round node was seen as cur
    uint32_t eatenUnpausedMs = 0;   // accrued unpaused time while eaten
    uint32_t lastVerdictTick = 0;

    bool empty() const { return count == 0; }
    bool full() const { return count >= kMaxActionQueue; }

    bool push(const QueuedAction& a) {
        if (count >= kMaxActionQueue) return false;
        userCancelled = false;
        queue[count++] = a;
        return true;
    }

    bool pop(QueuedAction* out = nullptr) {
        if (count <= 0) return false;
        if (out) *out = queue[0];
        for (int i = 1; i < count; ++i) queue[i - 1] = queue[i];
        --count;
        return true;
    }

    void cancel() {
        count = 0;
        active = false;
        activeAction = QueuedAction{};
        activeStartTick = 0;
        reInjectPending = false;
        userCancelled = true;
        seenInQueue = false;
        sawExecuting = false;
        eaten = false;
        staleCurLogged = false;
        pausedEaten = false;
        eatenReInjects = 0;
        goneSinceTick = 0;
        lastReInjectTick = 0;
        staleCurSince = 0;
        eatenUnpausedMs = 0;
        lastVerdictTick = 0;
    }

    void clear() {
        count = 0;
        active = false;
        activeAction = QueuedAction{};
        activeStartTick = 0;
        reInjectPending = false;
        userCancelled = false;
        seenInQueue = false;
        sawExecuting = false;
        eaten = false;
        staleCurLogged = false;
        pausedEaten = false;
        eatenReInjects = 0;
        goneSinceTick = 0;
        lastReInjectTick = 0;
        staleCurSince = 0;
        eatenUnpausedMs = 0;
        lastVerdictTick = 0;
    }

    // Dispatch stamps the start and clears the per-action eaten flags.
    void markDispatched(uint32_t now) { activeStartTick = now; sawExecuting = false; eaten = false; pausedEaten = false; goneSinceTick = 0; }
    // Adds the unpaused time since the last verdict while eaten; one step is capped at 1000 ms.
    // Pause edges: eaten while paused latches pausedEaten; the first unpaused tick after it gives the action a fresh budget.
    void accrueEaten(bool paused, uint32_t now) {
        if (eaten && paused) pausedEaten = true;
        else if (pausedEaten && !paused) { pausedEaten = false; eatenReInjects = 0; lastReInjectTick = 0; }
        if (eaten && !paused) {
            const uint32_t d = now - lastVerdictTick;
            eatenUnpausedMs += d < 1000u ? d : 1000u;
        }
        lastVerdictTick = now;
    }
    // Starts the gone clock on the first absent frame; a present frame stops it.
    void noteView(bool actionPresent, uint32_t now) {
        if (actionPresent) goneSinceTick = 0;
        else if (goneSinceTick == 0) goneSinceTick = now;
    }
    // Records that the engine showed the action executing; a present frame well after the last re-inject ends the eaten
    // episode (clears the re-inject budget and the unpaused wait, so the next command starts fresh).
    void notePresent(bool executing, bool actionPresent, uint32_t now) {
        if (executing) sawExecuting = true;
        if (actionPresent && now - lastReInjectTick >= kReInjectResetMs) { eatenReInjects = 0; eatenUnpausedMs = 0; }
    }
};

using ItemBuffer = ActionBuffer;

// Every buffered cmdId (queue order), then the active one when active. Writes at most cap ids, returns how many.
// Call before clear()/cancel(): each id is one lost command, so log one DROP line per id.
inline int bufferedCmdIds(const ActionBuffer& buf, uint32_t* out, int cap) {
    int n = 0;
    for (int i = 0; i < buf.count && i < kMaxActionQueue && n < cap; ++i) out[n++] = buf.queue[i].cmdId;
    if (buf.active && n < cap) out[n++] = buf.activeAction.cmdId;
    return n;
}

enum ActionDecision : int {
    DECIDE_DISPATCH_NOW = 0,
    DECIDE_BUFFER = 1,
    DECIDE_DROP_FULL = 2
};

// A caster the party table does not list (scripted NPC, cutscene, unreadable table) is not ours to buffer: pass it to the engine.
inline bool isPartyCaster(int memberK) { return memberK >= 0 && memberK < 3; }

inline ActionDecision decideAction(const ActionBuffer& buf, bool inCombat, int engineQueueLen) {
    if (inCombat) return DECIDE_DISPATCH_NOW;
    if (engineQueueLen == 0 && !buf.active && buf.empty()) return DECIDE_DISPATCH_NOW;
    if (buf.full()) return DECIDE_DROP_FULL;
    return DECIDE_BUFFER;
}

inline ActionDecision decideItemAction(const ActionBuffer& buf, bool inCombat, int engineQueueLen) {
    return decideAction(buf, inCombat, engineQueueLen);
}

inline bool shouldReInject(const ActionBuffer& buf, bool actionInQueue, bool hasFollowLeader, bool paused = false, uint32_t now = 0, bool userCancelled = false) {
    (void)paused; (void)now;
    if (!buf.active || buf.reInjectPending || buf.userCancelled || userCancelled) return false;
    if (buf.activeAction.reInjectCount >= kMaxReInjectCount) return false;
    if (buf.sawExecuting) return false;   // engine showed it executing: it finished, it was not eaten
    if (actionInQueue) return false;
    return hasFollowLeader;
}

inline bool shouldDispatchNext(const ActionBuffer& buf, int engineQueueLen, bool haveCur = false, int nrn = 0) {
    return !buf.active && !buf.empty() && engineQueueLen == 0 && !haveCur && nrn == 0;
}

// ---- QL6b flag gate: after an item/spell the engine keeps aa8/ab8 set for up to ~0.7 s and a dispatch in that window is wiped. ----
constexpr uint32_t kFlagBusyCapMs = 2000;   // hold at most this long, then dispatch anyway (hold=flag_cap)

// One per member. step() is called once per tick, only while every other dispatch gate passes (call reset() otherwise).
// Flags that were never busy during the hold dispatch at once; once busy was seen, the flags must read clear on two ticks
// >= kVanishDebounceMs apart. aa8/ab8 are ignored while paused (sticky in a frozen round), so the caller passes busy=false then.
struct FlagGate {
    uint32_t holdSince = 0;    // first tick of this hold
    uint32_t clearSince = 0;   // first tick of the current clear run
    bool holding = false, clearing = false, sawBusy = false;
    void reset() { holdSince = 0; clearSince = 0; holding = false; clearing = false; sawBusy = false; }
    // true = dispatch now. *capped is set when the dispatch is allowed only because the hold cap ran out.
    bool step(bool busy, uint32_t now, bool* capped = nullptr) {
        if (capped) *capped = false;
        if (!holding) { holding = true; holdSince = now; }
        if (busy) {
            sawBusy = true;
            clearing = false;
            if (now - holdSince >= kFlagBusyCapMs) { if (capped) *capped = true; return true; }
            return false;
        }
        if (!sawBusy) return true;
        if (!clearing) { clearing = true; clearSince = now; return false; }
        return now - clearSince >= kVanishDebounceMs;
    }
};

// ---- Why-codes (LOG GRAMMAR). Reason order = check order of the bool predicate above.
// reInjectWhy(...) == RI_OK  <=>  shouldReInject(...);   dispatchWhy(...) == DS_OK  <=>  shouldDispatchNext(...).
enum ReInjectWhy : uint8_t { RI_OK, RI_INACTIVE, RI_PENDING, RI_CANCEL, RI_MAX, RI_IN_QUEUE, RI_NO_LEADER, RI_EXECUTED };
enum DispatchWhy : uint8_t { DS_OK, DS_ACTIVE, DS_EMPTY, DS_ENGINE_BUSY, DS_CUR_BUSY, DS_NRN_BUSY, DS_FLAG_BUSY };
enum DropWhy : uint8_t { DR_COMBAT_CLEAR, DR_NULL_CREATURE, DR_CANCEL, DR_BUFFER_FULL, DR_UNREADABLE_CREATURE, DR_REINJECT_EXHAUSTED, DR_EATEN_TIMEOUT, DR_IDENTITY_LOST };

inline ReInjectWhy reInjectWhy(const ActionBuffer& buf, bool actionInQueue, bool hasFollowLeader, bool paused = false, uint32_t now = 0, bool userCancelled = false) {
    (void)paused; (void)now;
    if (!buf.active) return RI_INACTIVE;
    if (buf.reInjectPending) return RI_PENDING;
    if (buf.userCancelled || userCancelled) return RI_CANCEL;
    if (buf.activeAction.reInjectCount >= kMaxReInjectCount) return RI_MAX;
    if (buf.sawExecuting) return RI_EXECUTED;
    if (actionInQueue) return RI_IN_QUEUE;
    return hasFollowLeader ? RI_OK : RI_NO_LEADER;
}

inline DispatchWhy dispatchWhy(const ActionBuffer& buf, int engineQueueLen, bool haveCur = false, int nrn = 0, bool flagHold = false) {
    if (buf.active) return DS_ACTIVE;
    if (buf.empty()) return DS_EMPTY;
    if (engineQueueLen != 0) return DS_ENGINE_BUSY;
    if (haveCur) return DS_CUR_BUSY;
    if (nrn != 0) return DS_NRN_BUSY;
    if (flagHold) return DS_FLAG_BUSY;
    return DS_OK;
}

inline const char* whyName(ReInjectWhy w) {
    switch (w) {
        case RI_OK: return "ok";
        case RI_INACTIVE: return "inactive";
        case RI_PENDING: return "pending";
        case RI_CANCEL: return "cancel";
        case RI_MAX: return "max_reinject";
        case RI_IN_QUEUE: return "in_queue";
        case RI_NO_LEADER: return "no_leader";
        case RI_EXECUTED: return "executed";
    }
    return "?";
}

inline const char* whyName(DispatchWhy w) {
    switch (w) {
        case DS_OK: return "ok";
        case DS_ACTIVE: return "active";
        case DS_EMPTY: return "empty";
        case DS_ENGINE_BUSY: return "engine_busy";
        case DS_CUR_BUSY: return "cur_busy";
        case DS_NRN_BUSY: return "nrn_busy";
        case DS_FLAG_BUSY: return "flags";
    }
    return "?";
}

inline const char* whyName(DropWhy w) {
    switch (w) {
        case DR_COMBAT_CLEAR: return "combat_clear";
        case DR_NULL_CREATURE: return "null_creature";
        case DR_CANCEL: return "cancel";
        case DR_BUFFER_FULL: return "buffer_full";
        case DR_UNREADABLE_CREATURE: return "unreadable_creature";
        case DR_REINJECT_EXHAUSTED: return "reinject_exhausted";
        case DR_EATEN_TIMEOUT: return "eaten_timeout";
        case DR_IDENTITY_LOST: return "identity_lost";
    }
    return "?";
}

// ---- Queue signature (QJ log): what the queue looked like on one tick. Equal sigs = nothing new to log. ----
struct QueueSig {
    int ne = 0;              // queue entry count (all entries)
    int nTypes = 0;          // filled entries of types[]: min(ne, 8)
    int types[8] = {};       // first 8 queue action types, in queue order
    int round = 0;           // round counter (+0x10DC)
    int cur = -1;            // current action type; -1 = none
    bool aa8 = false;        // aa8 != 0
    bool ab8 = false;        // ab8 != 0
};

// Keeps the first 8 types; ne counts all entries. q == nullptr keeps no types (ne still counts).
inline QueueSig makeQueueSig(const QEntry* q, int ne, int round, int cur, bool aa8, bool ab8) {
    QueueSig s;
    s.ne = ne < 0 ? 0 : ne;
    s.nTypes = (q && s.ne > 0) ? (s.ne < 8 ? s.ne : 8) : 0;
    for (int i = 0; i < s.nTypes; ++i) s.types[i] = q[i].type;
    s.round = round; s.cur = cur; s.aa8 = aa8; s.ab8 = ab8;
    return s;
}

// Compares ne, the stored types (at most 8), round, cur and both flags. Types past the 8th are not compared.
inline bool sigEqual(const QueueSig& a, const QueueSig& b) {
    if (a.ne != b.ne || a.nTypes != b.nTypes || a.round != b.round || a.cur != b.cur || a.aa8 != b.aa8 || a.ab8 != b.ab8) return false;
    for (int i = 0; i < a.nTypes && i < 8; ++i) if (a.types[i] != b.types[i]) return false;
    return true;
}

// ---- LOG GRAMMAR formatters (INFO text after the KpLog T= stamp). Each writes a NUL-terminated line into out[cap] and
// returns the length it would have had (snprintf style). Output is clamped to kLogLineMax-1 chars, so every line is < 200. ----
constexpr int kLogLineMax = 200;

namespace detail {
// Minimal bounded writer: c=char, s=string (null prints '?'), u=unsigned dec, i=signed dec, x=lowercase hex, mm=member index.
struct LogWr {
    char* b;
    int lim;          // usable bytes in b, including the NUL
    int n = 0;        // chars that would have been written
    LogWr(char* out, int cap) : b(out), lim(cap < kLogLineMax ? cap : kLogLineMax) { if (b && lim > 0) b[0] = '\0'; }
    void c(char ch) { if (b && n + 1 < lim) b[n] = ch; ++n; }
    void s(const char* t) { if (!t) t = "?"; while (*t) c(*t++); }
    void u(unsigned long long v) { char t[24]; int k = 0; do { t[k++] = (char)('0' + v % 10); v /= 10; } while (v); while (k) c(t[--k]); }
    void i(long long v) { if (v < 0) { c('-'); u(0ull - (unsigned long long)v); } else u((unsigned long long)v); }
    void x(unsigned long long v) { char t[20]; int k = 0; do { unsigned d = (unsigned)(v & 0xF); t[k++] = (char)(d < 10 ? '0' + d : 'a' + d - 10); v >>= 4; } while (v); while (k) c(t[--k]); }
    void x8(uint32_t v) { for (int sh = 28; sh >= 0; sh -= 4) { const unsigned d = (v >> sh) & 0xF; c((char)(d < 10 ? '0' + d : 'a' + d - 10)); } }
    void mm(int m) { if (m < 0) c('?'); else i(m); }
    int done() { if (b && lim > 0) b[n < lim ? n : lim - 1] = '\0'; return n; }
};
}  // namespace detail

// CMD#<n> m=<k|?> cr=<hex> kind=item|spell|? id=<hex|-> tgt=<hex|-> dec=<dec> fb=<0|1> engq=<n|unreadable> buf=<n> paused=<0|1> combat=<0|1>
struct CmdLine {
    uint32_t cmdId = 0;
    int m = -1;                              // party member index; < 0 prints '?'
    uintptr_t cr = 0;                        // creature pointer
    int kind = -1;                           // ActionKind value; anything but Item/Spell prints '?'
    bool hasId = false;  uint32_t id = 0;    // id=<hex|->
    bool hasTgt = false; uint32_t tgt = 0;   // tgt=<hex|->
    const char* dec = "?";                   // dispatch_now|buffer|drop_full|pass_combat|pass_unreadable|pass_nonparty
    bool fb = false;
    int engq = 0;                            // queue length; < 0 prints 'unreadable'
    int buf = 0;
    bool paused = false;
    bool combat = false;
};

inline int fmtCmd(char* out, int cap, const CmdLine& c) {
    detail::LogWr w(out, cap);
    w.s("CMD#"); w.u(c.cmdId);
    w.s(" m="); w.mm(c.m);
    w.s(" cr="); w.x((unsigned long long)c.cr);
    w.s(" kind=");
    if (c.kind == (int)ActionKind::Item) w.s("item");
    else if (c.kind == (int)ActionKind::Spell) w.s("spell");
    else w.s("?");
    w.s(" id="); if (c.hasId) w.x(c.id); else w.c('-');
    w.s(" tgt="); if (c.hasTgt) w.x(c.tgt); else w.c('-');
    w.s(" dec="); w.s(c.dec);
    w.s(" fb="); w.c(c.fb ? '1' : '0');
    w.s(" engq="); if (c.engq < 0) w.s("unreadable"); else w.i(c.engq);
    w.s(" buf="); w.i(c.buf);
    w.s(" paused="); w.c(c.paused ? '1' : '0');
    w.s(" combat="); w.c(c.combat ? '1' : '0');
    return w.done();
}

// WAIT cmd=<n> m=<k> why=<reason> buf=<n> engNe=<n> age=<ms>
inline int fmtWait(char* out, int cap, uint32_t cmdId, int m, DispatchWhy why, int buf, int engNe, uint32_t ageMs) {
    detail::LogWr w(out, cap);
    w.s("WAIT cmd="); w.u(cmdId);
    w.s(" m="); w.mm(m);
    w.s(" why="); w.s(whyName(why));
    w.s(" buf="); w.i(buf);
    w.s(" engNe="); w.i(engNe);
    w.s(" age="); w.u(ageMs);
    return w.done();
}

// RESOLVE cmd=<n> m=<k> seen=<0|1> reinj=<n> age=<ms>
inline int fmtResolve(char* out, int cap, uint32_t cmdId, int m, bool seen, int reinj, uint32_t ageMs) {
    detail::LogWr w(out, cap);
    w.s("RESOLVE cmd="); w.u(cmdId);
    w.s(" m="); w.mm(m);
    w.s(" seen="); w.c(seen ? '1' : '0');
    w.s(" reinj="); w.i(reinj);
    w.s(" age="); w.u(ageMs);
    return w.done();
}

// DROP cmd=<n> m=<k> why=combat_clear|null_creature|cancel|buffer_full|unreadable_creature|reinject_exhausted|eaten_timeout|identity_lost n=<count>
inline int fmtDrop(char* out, int cap, uint32_t cmdId, int m, DropWhy why, int n) {
    detail::LogWr w(out, cap);
    w.s("DROP cmd="); w.u(cmdId);
    w.s(" m="); w.mm(m);
    w.s(" why="); w.s(whyName(why));
    w.s(" n="); w.i(n);
    return w.done();
}

// EDGE what=combat|paused|menu old=<0|1> new=<0|1> src=hud|unreadable buf=<n,n,n>
inline int fmtEdge(char* out, int cap, const char* what, bool oldOn, bool newOn, const char* src, const int* buf, int nBuf) {
    detail::LogWr w(out, cap);
    w.s("EDGE what="); w.s(what);
    w.s(" old="); w.c(oldOn ? '1' : '0');
    w.s(" new="); w.c(newOn ? '1' : '0');
    w.s(" src="); w.s(src);
    w.s(" buf=");
    for (int k = 0; buf && k < nBuf; ++k) { if (k) w.c(','); w.i(buf[k]); }
    return w.done();
}

// QJ m=<k> ne=<n> types=[<hex,...,+N>] round=<n> cur=<type|-> aa8=<0|1> ab8=<0|1> paused=<0|1> combat=<0|1> ours=<tag>@<age>ms|external
// oursTag == nullptr prints 'external' (no age). Types are capped at 8; the remainder is shown as +N.
inline int fmtQj(char* out, int cap, int m, const QueueSig& q, bool paused, bool combat, const char* oursTag, uint32_t ageMs) {
    detail::LogWr w(out, cap);
    const int shown = q.nTypes < 0 ? 0 : (q.nTypes > 8 ? 8 : q.nTypes);
    w.s("QJ m="); w.mm(m);
    w.s(" ne="); w.i(q.ne);
    w.s(" types=[");
    for (int k = 0; k < shown; ++k) { if (k) w.c(','); w.x((unsigned long long)(unsigned)q.types[k]); }
    if (q.ne > shown) { if (shown) w.c(','); w.c('+'); w.i(q.ne - shown); }
    w.s("]");
    w.s(" round="); w.i(q.round);
    w.s(" cur="); if (q.cur < 0) w.c('-'); else w.x((unsigned long long)(unsigned)q.cur);
    w.s(" aa8="); w.c(q.aa8 ? '1' : '0');
    w.s(" ab8="); w.c(q.ab8 ? '1' : '0');
    w.s(" paused="); w.c(paused ? '1' : '0');
    w.s(" combat="); w.c(combat ? '1' : '0');
    w.s(" ours=");
    if (!oursTag) w.s("external");
    else { w.s(oursTag); w.c('@'); w.u(ageMs); w.s("ms"); }
    return w.done();
}

// ---- Vanish / handoff / stale-cur / identity-swap grammar. Same LogWr rules as above (caller buffer + cap, clamp 200, returns length). ----
enum VanishKind : uint8_t { VK_EXECUTED, VK_UNSEEN, VK_EATEN_PAUSED, VK_EATEN_WAIT };

inline const char* vanishKindName(VanishKind k) {
    switch (k) {
        case VK_EXECUTED: return "executed";
        case VK_UNSEEN: return "unseen";
        case VK_EATEN_PAUSED: return "eaten_paused";
        case VK_EATEN_WAIT: return "eaten_wait";
    }
    return "?";
}

// VANISH cmd=<n> m=<k> kind=executed|unseen|eaten_paused|eaten_wait paused=<0|1> exec=<0|1> ne=<n> round=<n> cur=<type|-> aa8=<0|1> ab8=<0|1> age=<ms>
// cur uses the QJ convention: hex type, '-' when cur < 0.
inline int fmtVanish(char* out, int cap, uint32_t cmdId, int m, VanishKind kind, bool paused, bool executing, int ne, int round, int cur, bool aa8, bool ab8, uint32_t ageMs) {
    detail::LogWr w(out, cap);
    w.s("VANISH cmd="); w.u(cmdId);
    w.s(" m="); w.mm(m);
    w.s(" kind="); w.s(vanishKindName(kind));
    w.s(" paused="); w.c(paused ? '1' : '0');
    w.s(" exec="); w.c(executing ? '1' : '0');
    w.s(" ne="); w.i(ne);
    w.s(" round="); w.i(round);
    w.s(" cur="); if (cur < 0) w.c('-'); else w.x((unsigned long long)(unsigned)cur);
    w.s(" aa8="); w.c(aa8 ? '1' : '0');
    w.s(" ab8="); w.c(ab8 ? '1' : '0');
    w.s(" age="); w.u(ageMs);
    return w.done();
}

// HANDOFF m=<k> active=<cmd|-> buffered=<n> cmds=<id,id,...|->
// hasActive=false prints active=-. ids/n: the buffered command ids in queue order; a null ids or n <= 0 prints buffered=0 cmds=-.
inline int fmtHandoff(char* out, int cap, int m, bool hasActive, uint32_t activeCmd, const uint32_t* ids, int n) {
    detail::LogWr w(out, cap);
    const int cnt = (ids && n > 0) ? n : 0;
    w.s("HANDOFF m="); w.mm(m);
    w.s(" active="); if (hasActive) w.u(activeCmd); else w.c('-');
    w.s(" buffered="); w.i(cnt);
    w.s(" cmds=");
    if (cnt == 0) w.c('-');
    for (int k = 0; k < cnt; ++k) { if (k) w.c(','); w.u(ids[k]); }
    return w.done();
}

// HANDOFF_ONE cmd=<n> m=<k> seq=<i>/<total>
inline int fmtHandoffOne(char* out, int cap, uint32_t cmdId, int m, int seq, int total) {
    detail::LogWr w(out, cap);
    w.s("HANDOFF_ONE cmd="); w.u(cmdId);
    w.s(" m="); w.mm(m);
    w.s(" seq="); w.i(seq); w.c('/'); w.i(total);
    return w.done();
}

// HANDOFF_CLEAR m=<k> cmd=<n> ne=<n> round=<n>   (handoff_clear_first: the engine queue was cleared before the first send)
inline int fmtHandoffClear(char* out, int cap, int m, uint32_t cmdId, int ne, int round) {
    detail::LogWr w(out, cap);
    w.s("HANDOFF_CLEAR m="); w.mm(m);
    w.s(" cmd="); w.u(cmdId);
    w.s(" ne="); w.i(ne);
    w.s(" round="); w.i(round);
    return w.done();
}

// HANDOFF_KEEP m=<k> cmd=<n> ne=<n> round=<n>   (handoff_keep_new: the engine queue was left as is before the first send)
inline int fmtHandoffKeep(char* out, int cap, int m, uint32_t cmdId, int ne, int round) {
    detail::LogWr w(out, cap);
    w.s("HANDOFF_KEEP m="); w.mm(m);
    w.s(" cmd="); w.u(cmdId);
    w.s(" ne="); w.i(ne);
    w.s(" round="); w.i(round);
    return w.done();
}

// HANDOFF_HELD m=<k> cmd=<n> act=<resend|drop> ms=<n>   (handoff_resend_held: how an engine-held active command was settled; ms = since the edge)
inline int fmtHandoffHeld(char* out, int cap, int m, uint32_t cmdId, HeldHeadAction act, int ms) {
    detail::LogWr w(out, cap);
    w.s("HANDOFF_HELD m="); w.mm(m);
    w.s(" cmd="); w.u(cmdId);
    w.s(" act="); w.s(act == HELD_RESEND ? "resend" : act == HELD_DROP ? "drop" : "wait");
    w.s(" ms="); w.i(ms);
    return w.done();
}

// HANDOFF_OUT m=<k> cmd=<n> exec=<0|1> firstExecMs=<n|-> endMs=<n>   (SPELL_OUT has the same fields)
// Ground truth for a hand-off / spell replay: watched for 8 s after the edge / send. exec = aa8/ab8 or a cur subtype 9/10 was seen unpaused.
inline int fmtOutLine(char* out, int cap, const char* kw, int m, uint32_t cmdId, bool exec, int firstExecMs, int endMs) {
    detail::LogWr w(out, cap);
    w.s(kw); w.s(" m="); w.mm(m);
    w.s(" cmd="); w.u(cmdId);
    w.s(" exec="); w.c(exec ? '1' : '0');
    w.s(" firstExecMs="); if (exec && firstExecMs >= 0) w.i(firstExecMs); else w.c('-');
    w.s(" endMs="); w.i(endMs);
    return w.done();
}
inline int fmtHandoffOut(char* out, int cap, int m, uint32_t cmdId, bool exec, int firstExecMs, int endMs) {
    return fmtOutLine(out, cap, "HANDOFF_OUT", m, cmdId, exec, firstExecMs, endMs);
}
inline int fmtSpellOut(char* out, int cap, int m, uint32_t cmdId, bool exec, int firstExecMs, int endMs) {
    return fmtOutLine(out, cap, "SPELL_OUT", m, cmdId, exec, firstExecMs, endMs);
}

// SPELL_ARGS where=<hook|replay> cmd=<n> m=<k> spell=<hex> arg2=<hex> tgt=<hex> multi=<n> meta=<n> dom=<n> posptr=<hex> pos=<orig|null|zero>
// Every argument of a Force-power cast at capture (the engine's own call) and at replay (our call). posptr = the position pointer
// passed (raw); pos = null (no pointer), orig (a copy of the engine's position) or zero (a pointer to our zero-filled copy).
inline int fmtSpellArgs(char* out, int cap, bool replay, uint32_t cmdId, int m, uint32_t spellId, int arg2, uint32_t targetId,
                        int multispell, int metamagic, int domain, uint32_t posPtrRaw, bool hasPos) {
    detail::LogWr w(out, cap);
    w.s("SPELL_ARGS where="); w.s(replay ? "replay" : "hook");
    w.s(" cmd="); w.u(cmdId);
    w.s(" m="); w.mm(m);
    w.s(" spell="); w.x(spellId);
    w.s(" arg2="); w.x(static_cast<uint32_t>(arg2));
    w.s(" tgt="); w.x(targetId);
    w.s(" multi="); w.i(multispell);
    w.s(" meta="); w.i(metamagic);
    w.s(" dom="); w.i(domain);
    w.s(" posptr="); w.x(posPtrRaw);
    w.s(" pos="); w.s(posPtrRaw == 0 ? "null" : hasPos ? "orig" : "zero");
    return w.done();
}

// CLICK t=<tick ms> x=<n|-> y=<n|->   (one per left-button press edge; x/y are engine cursor coords, '-' when unreadable)
inline int fmtClick(char* out, int cap, uint32_t tickMs, bool haveXy, int x, int y) {
    detail::LogWr w(out, cap);
    w.s("CLICK t="); w.u(tickMs);
    w.s(" x="); if (haveXy) w.i(x); else w.c('-');
    w.s(" y="); if (haveXy) w.i(y); else w.c('-');
    return w.done();
}

// CMBT_FIELDS m=<k> cr520=<hex,..x12> t550=<hex> t564=<hex> t568=<hex>   (server creature, at a combat edge; '?' = unreadable)
// Soft-combat evidence: cr520 is the CrCombat block (kCrCombatDwords dwords), t550/t564/t568 the target fields.
constexpr int kCrCombatDwords = 12;
inline int fmtCmbtFields(char* out, int cap, int m, const uint32_t* cr, const bool* crOk, uint32_t t550, bool ok550,
                         uint32_t t564, bool ok564, uint32_t t568, bool ok568) {
    detail::LogWr w(out, cap);
    w.s("CMBT_FIELDS m="); w.mm(m);
    w.s(" cr520=");
    for (int i = 0; i < kCrCombatDwords; ++i) {
        if (i) w.c(',');
        if (cr && crOk && crOk[i]) w.x8(cr[i]); else w.c('?');
    }
    w.s(" t550="); if (ok550) w.x8(t550); else w.c('?');
    w.s(" t564="); if (ok564) w.x8(t564); else w.c('?');
    w.s(" t568="); if (ok568) w.x8(t568); else w.c('?');
    return w.done();
}

// Soft-combat candidate test: which of a member's target fields (t550/t564/t568) name a hostile, a party member, or nothing.
// "none" = 0 or the sentinel 0x7f000000; a party id matches on the LOW 24 bits (the log shows 1af for 0x800001af); any other id is hostile.
constexpr uint32_t kNoTargetId = 0x7f000000u;
struct SoftCand { bool anyHostile = false; bool anyParty = false; bool allNone = true; };
inline SoftCand softCandidate(const uint32_t* tgt, int n, const uint32_t* partyIds, int np) {
    SoftCand r;
    for (int i = 0; tgt && i < n; ++i) {
        const uint32_t t = tgt[i];
        if (t == 0 || t == kNoTargetId) continue;
        r.allNone = false;
        bool party = false;
        for (int j = 0; partyIds && j < np; ++j)
            if ((partyIds[j] & 0xFFFFFFu) != 0 && (partyIds[j] & 0xFFFFFFu) == (t & 0xFFFFFFu)) party = true;
        if (party) r.anyParty = true; else r.anyHostile = true;
    }
    return r;
}

// SOFT_CAND m=<k> hostile=<0|1> party=<0|1> none=<0|1> cr0=<n|?> cr1=<hex|?>   (cr0/cr1 = dword 0/1 of the cr520 block)
inline int fmtSoftCand(char* out, int cap, int m, const SoftCand& c, uint32_t cr0, bool ok0, uint32_t cr1, bool ok1) {
    detail::LogWr w(out, cap);
    w.s("SOFT_CAND m="); w.mm(m);
    w.s(" hostile="); w.c(c.anyHostile ? '1' : '0');
    w.s(" party="); w.c(c.anyParty ? '1' : '0');
    w.s(" none="); w.c(c.allNone ? '1' : '0');
    w.s(" cr0="); if (ok0) w.u(cr0); else w.c('?');
    w.s(" cr1="); if (ok1) w.x8(cr1); else w.c('?');
    return w.done();
}

// Soft combat = the engine says "in combat" but no member targets a hostile: at least one member targets a party id and none targets a
// non-party id. All-none (nobody targets anything) counts as HARD so the banner stays (conservative).
inline bool softCombat(bool combat, const SoftCand* c, int n) {
    if (!combat || !c || n <= 0) return false;
    bool party = false, hostile = false;
    for (int i = 0; i < n; ++i) { if (c[i].anyParty) party = true; if (c[i].anyHostile) hostile = true; }
    return party && !hostile;
}

// Banner/red-bar action for one frame. The engine sets the visible bits only on the combat-mode toggle, so a hide stays until we undo it.
//   soft && combat            -> HIDE (idempotent; re-applied every frame in case the engine sets the bit again)
//   hidden && combat && !soft -> RESTORE (real combat began while still in combat)
//   otherwise NONE; when hidden && !combat the caller forgets `hidden` (the engine already hid the controls itself, never restore then).
enum BannerAct { BA_NONE = 0, BA_HIDE = 1, BA_RESTORE = 2 };
inline BannerAct bannerAct(bool hidden, bool combat, bool soft) {
    if (soft && combat) return BA_HIDE;
    if (hidden && combat && !soft) return BA_RESTORE;
    return BA_NONE;
}

// BANNER act=<hide|restore|forget|reset> soft=<0|1> ctl=<n> wr=<n>   (ctl = controls managed, wr = flag dwords written this action)
inline int fmtBanner(char* out, int cap, const char* act, int soft, int nCtl, int nWr) {
    detail::LogWr w(out, cap);
    w.s("BANNER act="); w.s(act);
    w.s(" soft="); w.c(soft ? '1' : '0');
    w.s(" ctl="); w.i(nCtl);
    w.s(" wr="); w.i(nWr);
    return w.done();
}

// ROUND m=<k> n=<count> subs=<s,s,...|-> cur=<n|-> ne=<n> types=<hex,hex,...|->
// n is the subs count (null subs or n <= 0 prints n=0 subs=-). cur < 0 prints '-'. Types are hex, zero-padded to two digits.
inline int fmtRound(char* out, int cap, int m, const int* subs, int nSubs, int cur, int ne, const int* types, int nTypes) {
    detail::LogWr w(out, cap);
    const int ns = (subs && nSubs > 0) ? nSubs : 0;
    const int nt = (types && nTypes > 0) ? nTypes : 0;
    w.s("ROUND m="); w.mm(m);
    w.s(" n="); w.i(ns);
    w.s(" subs="); if (ns == 0) w.c('-');
    for (int k = 0; k < ns; ++k) { if (k) w.c(','); w.i(subs[k]); }
    w.s(" cur="); if (cur < 0) w.c('-'); else w.i(cur);
    w.s(" ne="); w.i(ne);
    w.s(" types="); if (nt == 0) w.c('-');
    for (int k = 0; k < nt; ++k) {
        if (k) w.c(',');
        const unsigned t = (unsigned)types[k];
        if (t < 16) w.c('0');
        w.x(t);
    }
    return w.done();
}

// STALE_CUR m=<k> sub=<n> since=<ms> aa8=<0|1> ab8=<0|1> paused=<0|1>
inline int fmtStaleCur(char* out, int cap, int m, int sub, uint32_t sinceMs, bool aa8, bool ab8, bool paused) {
    detail::LogWr w(out, cap);
    w.s("STALE_CUR m="); w.mm(m);
    w.s(" sub="); w.i(sub);
    w.s(" since="); w.u(sinceMs);
    w.s(" aa8="); w.c(aa8 ? '1' : '0');
    w.s(" ab8="); w.c(ab8 ? '1' : '0');
    w.s(" paused="); w.c(paused ? '1' : '0');
    return w.done();
}

// IDENT_SWAP from=<k> to=<k>   (member indices; a negative index prints '?')
inline int fmtIdentSwap(char* out, int cap, int from, int to) {
    detail::LogWr w(out, cap);
    w.s("IDENT_SWAP from="); w.mm(from);
    w.s(" to="); w.mm(to);
    return w.done();
}

// ---- Eaten-action verdict. PURE: the caller fills EngineView from engine memory and acts on the verdict. ----
struct EngineView { bool paused = false, actionPresent = false, executing = false, foreign = false, idle = false; };

enum ActiveVerdict : uint8_t { AV_PRESENT, AV_EXECUTED, AV_UNSEEN, AV_EATEN_WAIT, AV_EATEN_REINJECT, AV_GIVE_UP };

inline const char* verdictName(ActiveVerdict v) {
    switch (v) {
        case AV_PRESENT: return "present";
        case AV_EXECUTED: return "executed";
        case AV_UNSEEN: return "unseen";
        case AV_EATEN_WAIT: return "eaten_wait";
        case AV_EATEN_REINJECT: return "eaten_reinject";
        case AV_GIVE_UP: return "give_up";
    }
    return "?";
}

// Rules are checked in order; the first match wins. pauseEatenWait (ini pause_eaten_wait) adds rule 4b.
inline ActiveVerdict activeVerdict(const ActionBuffer& buf, const EngineView& ev, uint32_t now, bool pauseEatenWait = false) {
    if (ev.actionPresent) return AV_PRESENT;                                              // 1
    if (buf.goneSinceTick == 0 || now - buf.goneSinceTick < kVanishDebounceMs) return AV_PRESENT;
    if (now - buf.activeStartTick <= kMinActiveMs) return AV_PRESENT;
    if (buf.sawExecuting) return AV_EXECUTED;                                             // 2
    if (!buf.eaten && !ev.paused) return AV_UNSEEN;                                       // 3
    if (buf.eatenUnpausedMs >= kEatenWaitMaxMs) return AV_GIVE_UP;                        // 4
    if (pauseEatenWait && ev.paused) return AV_EATEN_WAIT;                                // 4b: the engine eats it again while paused
    if (ev.foreign || !ev.idle) return AV_EATEN_WAIT;                                     // 5
    if (buf.eatenReInjects >= kMaxEatenReInject) return AV_GIVE_UP;                       // 6
    if (now - buf.lastReInjectTick < kReInjectSpacingMs) return AV_EATEN_WAIT;            // 7
    return AV_EATEN_REINJECT;                                                             // 8
}

// Is the engine's current round node a real action? Flagless item/spell nodes go stale after kStaleCurMs unpaused.
inline bool curReal(bool haveCur, int curSub, int aa8, int ab8, bool paused, uint32_t staleSince, uint32_t now) {
    if (!haveCur) return false;
    if (!paused && (aa8 != 0 || ab8 != 0)) return true;
    if (curSub == 9 || curSub == 10) {
        if (!paused && aa8 == 0 && ab8 == 0 && staleSince != 0 && now - staleSince >= kStaleCurMs) return false;
        return true;
    }
    return false;
}

// Builds the EngineView from raw engine reads. aa8/ab8 are ignored while paused (sticky in a frozen round).
inline EngineView makeEngineView(const int* entryTypes, int ne, bool actionPresent, bool haveCur, int curSub, int nrn,
                                 int aa8, int ab8, bool paused, uint32_t staleSince, uint32_t now) {
    bool execNode = false;
    bool foreign = false;
    for (int i = 0; entryTypes != nullptr && i < ne; ++i) {
        const int t = entryTypes[i];
        if (t == 0x2E || t == 0x0F) execNode = true;
        if (t != 0x3F && t != 0x3D && t != 0x2E && t != 0x0F) foreign = true;
    }
    const bool creal = curReal(haveCur, curSub, aa8, ab8, paused, staleSince, now);
    EngineView ev;
    ev.paused = paused;
    ev.actionPresent = actionPresent;
    ev.executing = execNode || (!paused && ((creal && (curSub == 9 || curSub == 10)) || aa8 != 0 || ab8 != 0));
    ev.foreign = foreign;
    ev.idle = !foreign && !execNode && nrn == 0 && !creal;
    return ev;
}

}  // namespace qh

