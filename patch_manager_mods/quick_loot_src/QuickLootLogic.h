// QuickLootLogic.h -- pure helpers of quick-loot (doc 52 QL0/QL1a/QL1b/QL2p): event filter, wrap-safe ms, item cap, the pending-container
// tracker, the take-all decision functions (shouldConsume / readyToSend / giveUp / closeFlagFor), the pending-take state, the "Took:" toast
// text / timing / placement (QL1b), the QL2p proximity peek (candidate filter, nearest pick, cooldown / hysteresis, leave decision, suppress
// transitions, key edge, peek state, peek label), 0.3.1-dev label layout (one shared 1200 px rect, 56-char one-line text, latched position A/B,
// x clamp, backing panel rect, Took duration rule, dwell debounce) and the log line formatters. 0.4.0-dev adds the multi-row list layer
// (row text, scroll view, block geometry, bottom clamp, Took duration per row, hover hold) below the fmtPeek section. No game memory, no
// Win32: host-testable (quick_loot_logic_test.cpp; build with -I../_shared_src for TextFit.h).
#pragma once
#include "TextFit.h"   // textfit::textPx: row / block widths use the DLL's own dialogfont10x10 advance table
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

namespace qlogic {

constexpr int EvtOnOpenLoot = 0x16;    // queued by the container's OnOpen script (loot generation) -- question 1
constexpr int EvtClosed = 0x17;
constexpr int EvtInvDisturbed = 0x1B;  // OnInvDisturbed -- question 3
constexpr unsigned MaxItemLines = 64;
constexpr unsigned MaxRepoCount = 4096;     // a repository count above this is garbage, not an inventory
constexpr uint32_t TailMs = 2000;           // keep matching a container this long after Close so a trailing 0x1B is caught
constexpr int RawEventLogs = 3;             // first N EventHandler calls on the pending container are logged raw

inline bool isWatchedEvent(int id) { return id == EvtOnOpenLoot || id == EvtClosed || id == EvtInvDisturbed; }

// Wrap-safe elapsed ms between two 32-bit millisecond counters.
inline uint32_t msSince(uint32_t now, uint32_t then) { return now - then; }

// Number of ITEM lines to log for a repository holding n items: corrupt counts log nothing, big ones are capped.
inline unsigned clampItems(int n) {
    if (n <= 0 || n > static_cast<int>(MaxRepoCount)) return 0;
    return static_cast<unsigned>(n) > MaxItemLines ? MaxItemLines : static_cast<unsigned>(n);
}

// The container the player last opened. matches() is the hot path of the placeable EventHandler hook (every placeable heartbeat).
struct Tracker {
    uintptr_t pending = 0;
    uint32_t showMs = 0;
    bool closed = false;
    uint32_t closeMs = 0;
    int rawLogged = 0;

    void onShow(uintptr_t plc, uint32_t now) { pending = plc; showMs = now; closed = false; closeMs = 0; rawLogged = 0; }
    bool matches(uintptr_t plc, uint32_t now) {
        if (plc == 0 || plc != pending) return false;
        if (closed && msSince(now, closeMs) > TailMs) { pending = 0; closed = false; return false; }
        return true;
    }
    // Close of the pending container: starts the tail window. Returns false when plc is not the pending container.
    bool onClose(uintptr_t plc, uint32_t now) {
        if (plc == 0 || plc != pending) return false;
        closed = true; closeMs = now; return true;
    }
    bool wantRaw() {
        if (rawLogged >= RawEventLogs) return false;
        ++rawLogged;
        return true;
    }
};

// ---- QL1a: take-all decision logic
constexpr int EvtOpen = EvtOnOpenLoot;                 // the OnOpen event: loot generation has run once it was handled
constexpr unsigned OpenSettleFrames = 2;               // HUD frames to wait after OnOpen before sending the Close
constexpr uint32_t SendTimeoutMs = 1000;               // send anyway this long after Show when no OnOpen was seen
constexpr uint32_t GiveUpMs = 5000;                    // stop retrying an unresolvable message object after this long

enum class Reason { Consume = 0, Disabled, NoHud, Shift, NoPlc, Busy };

// Consume (skip the vanilla container menu) only when every condition holds. First failing condition wins, in this order.
// QL1a fix: the Show flag means "the container had items at Show", NOT "loot mode" -- containers that make their loot in OnOpen arrive with
// flag 0 -- so it is no longer a condition (it is still logged in the SHOW line).
inline Reason shouldConsume(bool enabled, bool hudOk, bool shift, bool plcOk, bool busy) {
    if (!enabled) return Reason::Disabled;
    if (!hudOk) return Reason::NoHud;
    if (shift) return Reason::Shift;
    if (!plcOk) return Reason::NoPlc;
    if (busy) return Reason::Busy;
    return Reason::Consume;
}
inline const char* reasonName(Reason r) {
    switch (r) {
        case Reason::Consume: return "consume";
        case Reason::Disabled: return "disabled";
        case Reason::NoHud: return "nohud";
        case Reason::Shift: return "shift";
        case Reason::NoPlc: return "noplc";
        case Reason::Busy: return "busy";
    }
    return "?";
}

// Send the Close when OnOpen was seen at least OpenSettleFrames HUD frames ago, or the Show is SendTimeoutMs old. Wrap-safe.
inline bool readyToSend(bool openSeen, uint32_t frame, uint32_t openFrame, uint32_t nowMs, uint32_t showMs) {
    if (openSeen && static_cast<uint32_t>(frame - openFrame) >= OpenSettleFrames && static_cast<uint32_t>(frame - openFrame) < 0x80000000u) return true;
    return msSince(nowMs, showMs) >= SendTimeoutMs;
}
inline bool giveUp(uint32_t nowMs, uint32_t showMs) { return msSince(nowMs, showMs) >= GiveUpMs; }

// Close flag chosen at send time: 1 (take all) when the container holds items, else 0 (nothing to take; no toast). Unreadable (< 0) -> 0.
inline int closeFlagFor(int itemCount) { return itemCount > 0 ? 1 : 0; }

// The one take-all waiting for its Close.
struct Take {
    bool active = false;
    uint32_t obj = 0;
    uintptr_t plc = 0;
    uint32_t showMs = 0, showFrame = 0;
    bool openSeen = false;
    uint32_t openFrame = 0;
    bool errLogged = false;

    void arm(uint32_t o, uintptr_t p, uint32_t now, uint32_t frame) {
        active = true; obj = o; plc = p; showMs = now; showFrame = frame; openSeen = false; openFrame = 0; errLogged = false;
    }
    void clear() { active = false; plc = 0; obj = 0; openSeen = false; }
    // Placeable event: the first OnOpen of the pending container marks it. Returns true when it did.
    bool onEvent(uintptr_t p, int id, uint32_t frame) {
        if (!active || p == 0 || p != plc || id != EvtOpen || openSeen) return false;
        openSeen = true; openFrame = frame; return true;
    }
};

// ---- QL1b: the "Took (N): ..." toast under the target name tag (0.3.1-dev layout: UX review .claude/reviews/quick_loot_pv1_ux.md)
// The font runs ~20 px/char (measured at 3840x1600), so ONE shared 1200 px rect holds 56 chars on one line. Peek and Took use the same rect.
constexpr unsigned LabelMaxChars = 56;     // the whole label line, "+K more" included (56 x 20.2 px = 1131 px <= 1200)
constexpr unsigned ToastMaxChars = LabelMaxChars;
constexpr unsigned ToastNameMax = 20;      // one item name longer than this is cut: first ToastNameMax-3 chars + "..."
constexpr unsigned NameRawMax = 40;        // sanitizeName (log lines) cuts at this many chars
constexpr unsigned ToastMaxItems = 16;     // names captured per take; further items only count towards "+K more"
constexpr uint32_t ToastMs = 3000;         // how long the line stays up
constexpr uint32_t ToastLongMs = 3500;     // ... when it lists more than ToastLongNames names or ends in "+K more"
constexpr uint32_t ToastCapMs = 4000;      // never longer than this (a longer Took blocks the next peek)
constexpr unsigned ToastLongNames = 3;
constexpr int LabelW = 1200;               // shared peek / Took label width (px)
constexpr int ToastW = LabelW, ToastH = 20;   // label rect size (px)
constexpr int ToastGapBelowName = 20;      // label top = name bottom + this (clears the health bar under the name)
constexpr int ScreenMarginPx = 24;         // position A keeps the label this far from the HUD panel edges
constexpr int BackPadX = 10, BackPadY = 4; // backing panel padding around the label (px)
constexpr float BackAlpha = 0.55f;         // backing panel: black at this alpha
constexpr int PxPerCharEst = 24;           // backing width estimate per char (measured 19..22.2 for wide letters like W; 24 keeps a margin)

// True while the toast is up: strictly less than durMs since it was shown. Wrap-safe (32-bit ms counter).
inline bool toastVisible(uint32_t nowMs, uint32_t shownAtMs, uint32_t durMs) { return msSince(nowMs, shownAtMs) < durMs; }
// How long a Took line stays up: 3 s, 3.5 s when it lists more than 3 names or ends in "+K more", never above the 4 s cap.
inline uint32_t toastDurationMs(unsigned shownNames, unsigned moreCount) {
    uint32_t d = ToastMs;
    if (shownNames > ToastLongNames || moreCount > 0) d = ToastLongMs;
    return d > ToastCapMs ? ToastCapMs : d;
}

struct Rect { int x, y, w, h; };
inline bool operator==(const Rect& a, const Rect& b) { return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h; }
inline bool operator!=(const Rect& a, const Rect& b) { return !(a == b); }

// Toast label rect: centred on the target name label (menu origin + name rect, all px), top = name bottom + gap.
inline Rect toastRect(int menuX, int menuY, int nameX, int nameY, int nameW, int nameH) {
    return { menuX + nameX + nameW / 2 - ToastW / 2, menuY + nameY + nameH + ToastGapBelowName, ToastW, ToastH };
}

// Position A clamp: x in [screenX + 24, screenX + screenW - 24 - w]. screenW <= 0 = unknown (no clamp); a label wider than the panel minus
// the margins is centred on the panel instead.
inline Rect clampLabelX(Rect r, int screenX, int screenW) {
    if (screenW <= 0) return r;
    const int lo = screenX + ScreenMarginPx, hi = screenX + screenW - ScreenMarginPx - r.w;
    if (hi < lo) r.x = screenX + (screenW - r.w) / 2;
    else if (r.x < lo) r.x = lo;
    else if (r.x > hi) r.x = hi;
    return r;
}

// Backing panel behind the text: black, BackAlpha. Width follows the text (chars x PxPerCharEst + 2 pad, at most the label width + 2 pad),
// centred on the label rect; height = label height + 2 BackPadY.
inline Rect backingRect(const Rect& label, unsigned chars) {
    int w = static_cast<int>(chars) * PxPerCharEst + 2 * BackPadX;
    const int maxW = label.w + 2 * BackPadX;
    if (w > maxW) w = maxW;
    return { label.x + label.w / 2 - w / 2, label.y - BackPadY, w, label.h + 2 * BackPadY };
}

// Copies an item name into out (cap >= 2): stops at the first NUL, cuts at NameRawMax, replaces every byte outside 0x20..0x7E with '?',
// trims trailing spaces. A null/empty result becomes "item". Returns the length.
inline unsigned sanitizeName(char* out, unsigned cap, const char* in) {
    unsigned n = 0;
    const unsigned lim = cap - 1 < NameRawMax ? cap - 1 : NameRawMax;
    if (in) {
        while (n < lim && in[n]) {
            const unsigned char c = static_cast<unsigned char>(in[n]);
            out[n] = (c < 0x20 || c > 0x7E) ? '?' : static_cast<char>(c);
            ++n;
        }
    }
    while (n > 0 && out[n - 1] == ' ') --n;
    if (n == 0) { const char* d = "item"; while (d[n]) { out[n] = d[n]; ++n; } }
    out[n] = 0;
    return n;
}

// Label entry for one item: sanitizeName, then a name longer than ToastNameMax is cut to its first ToastNameMax-3 chars, trailing spaces and
// punctuation trimmed, "..." appended ("Lightsaber Crystal: Opila" -> "Lightsaber Crysta..."). cap >= ToastNameMax + 1. Returns the length.
inline unsigned entryName(char* out, unsigned cap, const char* in) {
    char tmp[NameRawMax + 1];
    unsigned n = sanitizeName(tmp, sizeof tmp, in);
    if (n > ToastNameMax && cap > ToastNameMax) {
        n = ToastNameMax - 3;
        while (n > 0) {
            const char c = tmp[n - 1];
            const bool alnum = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
            if (alnum) break;
            --n;
        }
        memcpy(out, tmp, n);
        memcpy(out + n, "...", 3);
        n += 3;
        out[n] = 0;
        return n;
    }
    if (n >= cap) n = cap - 1;
    memcpy(out, tmp, n);
    out[n] = 0;
    return n;
}

// What fmtList / fmtTook / fmtPeek put on the line: names shown and the "+K more" count (0 = none).
struct ListInfo { unsigned shown = 0, more = 0; };

namespace detail {
// "<prefix>A, B x2, C": one entry per item (entryName: names over ToastNameMax are cut with "..."; then " xN" for 2 <= N <= 999). When the line would pass maxChars the entries that fit
// are kept and ", +K more" is appended (K = items left out + `extra`, items beyond the captured ones). count 0 -> out = "" and 0
// (no line). names may hold null pointers; stacks may hold -1 (unknown). The result is cut to cap-1; returns its length.
inline unsigned fmtList(char* out, size_t cap, const char* prefixText, unsigned maxChars, const char* const* names, const int* stacks,
                        unsigned count, unsigned extra, ListInfo* info = nullptr) {
    if (info) *info = ListInfo();
    if (!out || cap == 0) return 0;
    out[0] = 0;
    if (count == 0) return 0;
    if (count > ToastMaxItems) { extra += count - ToastMaxItems; count = ToastMaxItems; }
    char ent[ToastMaxItems][ToastNameMax + 16];
    unsigned len[ToastMaxItems];
    for (unsigned i = 0; i < count; ++i) {
        unsigned n = entryName(ent[i], sizeof ent[i], names ? names[i] : nullptr);
        const int st = stacks ? stacks[i] : -1;
        if (st >= 2 && st <= 999) n += static_cast<unsigned>(snprintf(ent[i] + n, sizeof ent[i] - n, " x%d", st));
        ent[i][n] = 0;
        len[i] = n;
    }
    const unsigned prefix = static_cast<unsigned>(strlen(prefixText));
    unsigned total = prefix;
    for (unsigned i = 0; i < count; ++i) total += len[i] + (i ? 2u : 0u);
    unsigned shown = count;
    unsigned more = extra;
    if (!(extra == 0 && total <= maxChars)) {
        shown = 0;
        unsigned used = prefix;
        for (unsigned i = 0; i < count; ++i) {
            const unsigned add = len[i] + (i ? 2u : 0u);
            const unsigned after = count - i - 1 + extra;   // items still left out if this one is kept
            const unsigned suffix = after ? static_cast<unsigned>(snprintf(nullptr, 0, ", +%u more", after)) : 0u;
            if (used + add + suffix > maxChars) break;
            used += add;
            shown = i + 1;
        }
        if (shown == 0) shown = 1;   // cannot happen with the caps above; never emit an empty list
        more = count - shown + extra;
    }
    if (info) { info->shown = shown; info->more = more; }
    char buf[256 + 64];
    unsigned pos = static_cast<unsigned>(snprintf(buf, sizeof buf, "%s", prefixText));
    for (unsigned i = 0; i < shown; ++i) pos += static_cast<unsigned>(snprintf(buf + pos, sizeof buf - pos, "%s%s", i ? ", " : "", ent[i]));
    if (more) pos += static_cast<unsigned>(snprintf(buf + pos, sizeof buf - pos, ", +%u more", more));
    if (pos >= cap) pos = static_cast<unsigned>(cap - 1);
    memcpy(out, buf, pos);
    out[pos] = 0;
    return pos;
}
}   // namespace detail

// "Took (4): A, B x2, +2 more" (56-char cap; "Took: A" for a single item). N = count + extra. See detail::fmtList.
inline unsigned fmtTook(char* out, size_t cap, const char* const* names, const int* stacks, unsigned count, unsigned extra = 0,
                        ListInfo* info = nullptr) {
    char pre[24];
    const unsigned total = count + extra;
    if (total <= 1) snprintf(pre, sizeof pre, "Took: ");
    else snprintf(pre, sizeof pre, "Took (%u): ", total);
    return detail::fmtList(out, cap, pre, ToastMaxChars, names, stacks, count, extra, info);
}

// ---- QL2p: proximity peek ("show list, I take")
constexpr float DefaultOpenM = 3.0f;          // a container within this 2D distance of the leader is opened for a peek
constexpr float DefaultCloseM = 5.0f;         // the peek closes (items stay) beyond this distance; also the cooldown release distance
constexpr int DefaultTakeKey = 0x46;          // 'F'
constexpr uint32_t DefaultScanMs = 250;
constexpr uint32_t MinScanMs = 50, MaxScanMs = 5000;
constexpr unsigned MaxScanObjects = 2048;     // area object-list cap per scan
constexpr unsigned CooldownCap = 64;
constexpr uint32_t ExpectTimeoutMs = 1500;    // OpenInventory called but no ShowContainerGui for it after this long
constexpr unsigned PeekMaxChars = LabelMaxChars;   // the whole peek line (same cap as the Took line)
constexpr int PeekW = LabelW;                 // peek label width (px); the same rect size as the Took line
constexpr uint32_t DwellMs = 400;             // the leader must stay within OpenM of the same container this long before it is opened
constexpr int PeekTopPercent = 62;            // fixed spot: this % of the HUD panel height

inline float clampOpenM(float v) { return (v > 0.0f && v < 100.0f) ? v : DefaultOpenM; }
// CloseM is at least OpenM + 1 so the hysteresis band is never empty (a NaN falls back to the minimum).
inline float clampCloseM(float openM, float closeM) { return (closeM >= openM + 1.0f) ? closeM : openM + 1.0f; }
inline uint32_t clampScanMs(int v) { return v < static_cast<int>(MinScanMs) ? MinScanMs : (v > static_cast<int>(MaxScanMs) ? MaxScanMs : static_cast<uint32_t>(v)); }
inline int clampTakeKey(int vk) { return (vk >= 1 && vk <= 254) ? vk : DefaultTakeKey; }

inline float dist2d(float ax, float ay, float bx, float by) {
    const float dx = ax - bx, dy = ay - by;
    return sqrtf(dx * dx + dy * dy);
}

// Candidate flags of one placeable (read from the server object; see QuickLoot.cpp offsets).
struct CandFlags {
    bool hasInv = false, locked = false, trap = false, useable = false, open = false, cooldown = false;
};
// First failing candidate condition ("ok" when none). Order: inventory, locked, trap, useable, open, cooldown, range.
inline const char* candidateWhy(const CandFlags& f, float dist, float openM) {
    if (!f.hasInv) return "noinv";
    if (f.locked) return "locked";
    if (f.trap) return "trap";
    if (!f.useable) return "unuse";
    if (f.open) return "open";
    if (f.cooldown) return "cooldown";
    if (!(dist <= openM)) return "far";
    return "ok";
}
inline bool candidateOk(const CandFlags& f, float dist, float openM) { return strcmp(candidateWhy(f, dist, openM), "ok") == 0; }
// Index of the nearest candidate with ok[i] (ties: lowest index), -1 when none.
inline int pickNearest(const float* dist, const bool* ok, int n) {
    int best = -1;
    for (int i = 0; i < n; ++i)
        if (ok[i] && (best < 0 || dist[i] < dist[best])) best = i;
    return best;
}

// Containers the leader has already peeked at: no re-open until the leader has been farther than CloseM from the container
// (hysteresis: enter at OpenM, leave at CloseM). Capped; a full set drops its oldest id. Cleared on an area change.
struct Cooldown {
    uint32_t ids[CooldownCap];
    unsigned n = 0;
    bool contains(uint32_t id) const { for (unsigned i = 0; i < n; ++i) if (ids[i] == id) return true; return false; }
    void add(uint32_t id) {
        if (contains(id)) return;
        if (n == CooldownCap) { memmove(ids, ids + 1, (CooldownCap - 1) * sizeof ids[0]); --n; }
        ids[n++] = id;
    }
    // Removes id when the leader is farther than closeM from it. Returns true when it was removed.
    bool release(uint32_t id, float dist, float closeM) {
        if (!(dist > closeM)) return false;
        for (unsigned i = 0; i < n; ++i)
            if (ids[i] == id) { memmove(ids + i, ids + i + 1, (n - i - 1) * sizeof ids[0]); --n; return true; }
        return false;
    }
    void clear() { n = 0; }
};

// Area change: both pointers known and different (0 = unreadable, never a change).
inline bool areaChanged(uintptr_t prev, uintptr_t cur) { return prev != 0 && cur != 0 && prev != cur; }

// Why the peek ends. Priority: gone > suppress > area > far.
enum class Leave { None = 0, Gone, Suppress, Area, Far };
inline Leave leaveReason(bool plcResolves, bool suppressed, bool areaChangedNow, float dist, float closeM) {
    if (!plcResolves) return Leave::Gone;
    if (suppressed) return Leave::Suppress;
    if (areaChangedNow) return Leave::Area;
    if (dist > closeM) return Leave::Far;
    return Leave::None;
}
inline const char* leaveName(Leave l) {
    switch (l) {
        case Leave::None: return "none";
        case Leave::Gone: return "gone";
        case Leave::Suppress: return "suppress";
        case Leave::Area: return "area";
        case Leave::Far: return "far";
    }
    return "?";
}

// Suppress states (no scan, no new peek; an active peek closes). Priority: noworld > menu > dialog > paused > combat.
enum class Sup { None = 0, NoWorld, Menu, Dialog, Paused, Combat, CombatUnk };
// combat: 1 = in combat, 0 = not, < 0 = unreadable (treated as combat, the QueueHud convention).
inline Sup suppressReason(bool worldOk, bool menu, bool dialog, bool paused, int combat) {
    if (!worldOk) return Sup::NoWorld;
    if (menu) return Sup::Menu;
    if (dialog) return Sup::Dialog;
    if (paused) return Sup::Paused;
    if (combat > 0) return Sup::Combat;
    if (combat < 0) return Sup::CombatUnk;
    return Sup::None;
}
inline const char* supName(Sup s) {
    switch (s) {
        case Sup::None: return "none";
        case Sup::NoWorld: return "noworld";
        case Sup::Menu: return "menu";
        case Sup::Dialog: return "dialog";
        case Sup::Paused: return "paused";
        case Sup::Combat: return "combat";
        case Sup::CombatUnk: return "combat?";
    }
    return "?";
}
// Change detector for the SUPPRESS log line. update() returns true exactly when the state differs from the previous one.
struct SupTracker {
    Sup last = Sup::None;
    bool update(Sup s) { const bool ch = s != last; last = s; return ch; }
};

// Key edge: true exactly once per press (prev tracks the level; update it every frame so a key held when the peek opens never fires).
inline bool risingEdge(bool& prev, bool down) { const bool e = down && !prev; prev = down; return e; }
// Printable key name for the label: letters and digits as themselves, others "key".
inline const char* keyLabel(int vk, char* buf, size_t cap) {
    if (cap < 2) return "";
    if ((vk >= 0x30 && vk <= 0x39) || (vk >= 0x41 && vk <= 0x5A)) { buf[0] = static_cast<char>(vk); buf[1] = 0; }
    else snprintf(buf, cap, "key");
    return buf;
}

// "[F] Take all (4): Medpac x2, Blaster Pistol, +2 more" (56-char cap); one item: "[F] Take: Medpac". total = real repository count,
// count = names captured. total 0 / count 0 -> "" and 0 (no label).
inline unsigned fmtPeek(char* out, size_t cap, const char* key, int total, const char* const* names, const int* stacks, unsigned count,
                        ListInfo* info = nullptr) {
    if (info) *info = ListInfo();
    if (!out || cap == 0) return 0;
    out[0] = 0;
    if (total <= 0 || count == 0) return 0;
    char pre[48];
    if (total == 1) snprintf(pre, sizeof pre, "[%s] Take: ", key ? key : "key");
    else snprintf(pre, sizeof pre, "[%s] Take all (%d): ", key ? key : "key", total);
    const unsigned extra = static_cast<unsigned>(total) > count ? static_cast<unsigned>(total) - count : 0u;
    return detail::fmtList(out, cap, pre, PeekMaxChars, names, stacks, count, extra, info);
}

// Peek label rects. Name-anchored: same placement rule as the toast (centred under the target name label), wider box.
inline Rect peekRectName(int nameX, int nameY, int nameW, int nameH) {
    return { nameX + nameW / 2 - PeekW / 2, nameY + nameH + ToastGapBelowName, PeekW, ToastH };
}
// Fixed spot: horizontally centred on the HUD panel, PeekTopPercent % down its height.
inline Rect peekRectFixed(int px, int py, int pw, int ph) {
    return { px + pw / 2 - PeekW / 2, py + (ph * PeekTopPercent) / 100, PeekW, ToastH };
}


// ---- 0.4.0-dev: multi-row item list (doc 52 QL2L). One block = header row + up to ListRows item rows on a vanilla frame panel.
// Geometry (engine px at 3840x1600, the dev box; all constants measured with scripts/character_page_mockup.py text_size):
//   dialogfont10x10 = 2.7467 px per font texel (1.03 x 1600/600); glyph box 10 texels = 27.5 px tall; the tooltip line pitch is 12.5 texels = 34 px.
//   RowH 32 px keeps the glyph box (27.5) plus 4.5 px of air; 12 rows + header = 13 x 32 = 416 px = 26 % of the screen height.
constexpr int NameCap = 48;                // captured name buffer (rowName reads at most NameCap-1 chars)
constexpr unsigned ListMaxItems = 64;      // names captured per peek / take; items past this only count towards the "+K more" row
constexpr unsigned ListRowsDefault = 8, ListRowsMin = 1, ListRowsMax = 12;   // visible item rows (quick_loot.ini ListRows)
constexpr unsigned RowNameMax = 40;        // an item name longer than this is cut: first RowNameMax-3 chars + "..."
constexpr unsigned RowTextCap = 56;        // one row's text: name (<= 40) + " x999" + NUL fits
constexpr int RowH = 32;                   // row pitch (px); see the measurement above
constexpr int HeaderGapPx = 6;             // air between the header row and the first item row
constexpr int BlockPadX = 28, BlockPadY = 20;   // inside the frame; the vanilla template frame (LBL_MOULDING3, DIMENSION 16) eats 16 px of each edge
constexpr int BlockMinW = 300, BlockMaxW = 900;
constexpr int LabelSlackPx = 12;           // row / header label rects are this much wider than their text budget (a wrapped label is taller than a row)
constexpr int IndicatorGapPx = 36;         // min gap between the header text and the "1-8 of 25" indicator
constexpr float FrameAlpha = 1.0f;         // frame panel alpha: the template fill (uibit_fill_2bt) is already black at alpha 199/255 = 0.78 ("about 0.75"), alpha 1.0 keeps the vanilla look
constexpr int WheelStep = 120;             // wheel delta per row
constexpr uint32_t WheelFreshMs = 250;     // the wheel callback consumes only while the block was drawn within this long
constexpr uint32_t TookBaseMs = 3000, TookRowMs = 500, TookCapMs = 6000;   // Took list: 3 s + 0.5 s per list row past TookFreeRows, cap 6 s
constexpr unsigned TookFreeRows = 3;

inline unsigned clampListRows(int v) {
    if (v < static_cast<int>(ListRowsMin)) return ListRowsMin;
    return v > static_cast<int>(ListRowsMax) ? ListRowsMax : static_cast<unsigned>(v);
}
// Took list duration: 3 s for up to 3 list rows, +0.5 s per row past that, never above 6 s.
inline uint32_t tookDurationMs(unsigned listRows) {
    uint32_t d = TookBaseMs + (listRows > TookFreeRows ? (listRows - TookFreeRows) * TookRowMs : 0u);
    return d > TookCapMs ? TookCapMs : d;
}

// dialogfont10x10 width of t in engine px at 1600 px screen height (rounded up).
inline int textPx10(const char* t) {
    const textfit::TextCtx c = { 1600.0f / 600.0f, 1.03f, 1.0f };
    return static_cast<int>(ceilf(textfit::textPx(c, t, static_cast<int>(strlen(t)), textfit::F10)));
}
constexpr int RowInnerMaxPx = BlockMaxW - 2 * BlockPadX;   // widest row text the block can hold

// Row name: printable bytes (others '?'), at most NameCap-1 of them, trailing spaces trimmed, empty -> "item". Longer than RowNameMax: the first
// RowNameMax-3 chars, trailing non-alphanumerics trimmed, "..." appended ("Mandalorian Heavy Armor Plating Mk IV Upgrade" -> "Mandalorian Heavy Armor Plating Mk IV Up...").
// cap >= RowNameMax + 1. Returns the length.
inline unsigned rowName(char* out, unsigned cap, const char* in) {
    char tmp[NameCap];
    unsigned n = 0;
    if (in) {
        while (n < static_cast<unsigned>(NameCap) - 1 && in[n]) {
            const unsigned char c = static_cast<unsigned char>(in[n]);
            tmp[n] = (c < 0x20 || c > 0x7E) ? '?' : static_cast<char>(c);
            ++n;
        }
    }
    while (n > 0 && tmp[n - 1] == ' ') --n;
    if (n == 0) { memcpy(tmp, "item", 4); n = 4; }
    if (n > RowNameMax) {
        n = RowNameMax - 3;
        while (n > 0) {
            const char c = tmp[n - 1];
            if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) break;
            --n;
        }
        memcpy(tmp + n, "...", 3);
        n += 3;
    }
    if (n >= cap) n = cap - 1;
    memcpy(out, tmp, n);
    out[n] = 0;
    return n;
}
// One item row: rowName + " xN" for 2 <= N <= 999. When the text is wider than maxPx the name is cut further (+ "...") so it never wraps.
inline unsigned rowText(char* out, unsigned cap, const char* name, int stack, int maxPx = RowInnerMaxPx) {
    char nm[RowNameMax + 8];
    unsigned n = rowName(nm, sizeof nm, name);
    char sfx[8] = "";
    if (stack >= 2 && stack <= 999) snprintf(sfx, sizeof sfx, " x%d", stack);
    char buf[RowTextCap + 16];
    snprintf(buf, sizeof buf, "%s%s", nm, sfx);
    if (textPx10(buf) > maxPx) {
        if (n >= 3 && memcmp(nm + n - 3, "...", 3) == 0) n -= 3;
        while (n > 1) {
            --n;
            char t[RowTextCap + 16];
            snprintf(t, sizeof t, "%.*s...%s", static_cast<int>(n), nm, sfx);
            if (textPx10(t) <= maxPx) { memcpy(buf, t, sizeof t); break; }
        }
    }
    unsigned len = static_cast<unsigned>(strlen(buf));
    if (len >= cap) len = cap - 1;
    memcpy(out, buf, len);
    out[len] = 0;
    return len;
}

// The captured list as rows. items = rows for captured names, more = items past the capture (shown as one last "+K more" row).
struct ListData {
    unsigned items = 0, more = 0;
    int maxRowPx = 0;                  // widest row text (incl. the "+K more" row)
    char rows[ListMaxItems][RowTextCap];
    unsigned rowsTotal() const { return items + (more ? 1u : 0u); }
    unsigned totalItems() const { return items + more; }
    bool empty() const { return items == 0; }
    void clear() { items = 0; more = 0; maxRowPx = 0; }
    // count names captured out of `total` items in the container (total < count is treated as count).
    void set(const char* const* names, const int* stacks, unsigned count, unsigned total) {
        clear();
        if (count > ListMaxItems) count = ListMaxItems;
        for (unsigned i = 0; i < count; ++i) {
            rowText(rows[i], RowTextCap, names ? names[i] : nullptr, stacks ? stacks[i] : -1);
            const int w = textPx10(rows[i]);
            if (w > maxRowPx) maxRowPx = w;
        }
        items = count;
        if (count == 0) return;
        more = total > count ? total - count : 0u;
        if (more) {
            char b[RowTextCap];
            moreText(b, sizeof b);
            const int w = textPx10(b);
            if (w > maxRowPx) maxRowPx = w;
        }
    }
    unsigned moreText(char* out, size_t cap) const { return static_cast<unsigned>(snprintf(out, cap, "+%u more", more)); }
    // Row i of the whole list (items, then the "+K more" row); "" past the end.
    void rowAt(unsigned i, char* out, size_t cap) const {
        if (i < items) snprintf(out, cap, "%s", rows[i]);
        else if (i == items && more) moreText(out, cap);
        else if (cap) out[0] = 0;
    }
};

// Scroll state of the list on screen: rows = visible item rows, total = list rows, off = first visible row. The wheel adds up to WheelStep per row.
struct ListView {
    unsigned rows = ListRowsDefault, total = 0, off = 0;
    int acc = 0;                       // wheel delta not yet worth a row (high-resolution wheels send < 120)
    unsigned maxOff() const { return total > rows ? total - rows : 0u; }
    bool scrollable() const { return total > rows; }
    void reset(unsigned newTotal, unsigned newRows) { rows = newRows; total = newTotal; off = 0; acc = 0; }
    void setTotal(unsigned t) { total = t; if (off > maxOff()) off = maxOff(); }
    // delta > 0 = wheel forward = up the list. true when off changed.
    bool scroll(int delta) {
        if (!scrollable()) { acc = 0; return false; }
        acc += delta;
        const int steps = acc / WheelStep;
        acc -= steps * WheelStep;
        long n = static_cast<long>(off) - steps;
        const long hi = static_cast<long>(maxOff());
        if (n < 0) { n = 0; acc = 0; }
        if (n > hi) { n = hi; acc = 0; }
        const bool ch = static_cast<unsigned>(n) != off;
        off = static_cast<unsigned>(n);
        return ch;
    }
};

// "1-8 of 25": first / last item numbers of the visible rows (1-based), total items.
inline int fmtRange(char* out, size_t cap, unsigned first, unsigned last, unsigned total) {
    return snprintf(out, cap, "%u-%u of %u", first, last, total);
}
// "[F] Take all (25)" / "Took (25)". total = items in the container.
inline int fmtHeader(char* out, size_t cap, bool took, const char* key, unsigned total) {
    if (took) return snprintf(out, cap, "Took (%u)", total);
    return snprintf(out, cap, "[%s] Take all (%u)", key ? key : "key", total);
}

// Everything the adapter draws for the block this frame.
struct ListDraw {
    bool took = false, scrollable = false;
    unsigned off = 0, vis = 0, rowsTotal = 0, items = 0;
    char header[40] = {0};
    char indicator[24] = {0};          // "" when the whole list fits
    int headerPx = 0, indicatorPx = 0;
    char rows[ListRowsMax][RowTextCap];
    int w = 0, h = 0;                  // block size (px), constant while scrolling
};

// Builds the block for list d seen through view v. false = nothing to draw (empty list). The width follows the widest of ALL rows (not only the
// visible ones) so scrolling never resizes the block; h follows the visible row count (min(rows, list rows)).
inline bool buildListDraw(const ListData& d, const ListView& v, bool took, const char* key, ListDraw* o) {
    *o = ListDraw();
    if (d.empty()) return false;
    const unsigned total = d.rowsTotal();
    unsigned rows = v.rows < ListRowsMin ? ListRowsMin : (v.rows > ListRowsMax ? ListRowsMax : v.rows);
    const unsigned vis = total < rows ? total : rows;
    unsigned off = v.off;
    const unsigned maxOff = total > rows ? total - rows : 0u;
    if (off > maxOff) off = maxOff;
    o->took = took; o->off = off; o->vis = vis; o->rowsTotal = total; o->items = d.totalItems();
    o->scrollable = total > rows;
    fmtHeader(o->header, sizeof o->header, took, key, d.totalItems());
    o->headerPx = textPx10(o->header);
    int need = d.maxRowPx;
    if (o->scrollable) {
        const unsigned last = (off + vis > d.items) ? d.totalItems() : off + vis;
        fmtRange(o->indicator, sizeof o->indicator, off + 1, last, d.totalItems());
        o->indicatorPx = textPx10(o->indicator);
        char worst[24];   // widest form (digits only grow with off): used for the width so the block stays still
        fmtRange(worst, sizeof worst, maxOff + 1, d.totalItems(), d.totalItems());
        const int hw = o->headerPx + IndicatorGapPx + textPx10(worst);
        if (hw > need) need = hw;
    } else if (o->headerPx > need) need = o->headerPx;
    int w = need + 2 * BlockPadX;
    if (w < BlockMinW) w = BlockMinW;
    if (w > BlockMaxW) w = BlockMaxW;
    o->w = w;
    o->h = 2 * BlockPadY + RowH + HeaderGapPx + static_cast<int>(vis) * RowH;
    for (unsigned i = 0; i < vis; ++i) d.rowAt(off + i, o->rows[i], RowTextCap);
    return true;
}

// ---- 0.3.1-dev: label placement (position A under the target name, B at a fixed spot), latch, Took state, dwell
enum class Pos { None = 0, A, B };
inline const char* posName(Pos p) { return p == Pos::A ? "A" : (p == Pos::B ? "B" : "-"); }

// What the adapter sees of the screen this frame (all px): the target-menu name label (menuShown = on screen now; haveName = a rect was seen
// at some point, `name` is the latest) and the HUD panel extent (havePanel; panel.w <= 0 = unknown).
struct Geometry {
    bool menuShown = false, haveName = false, havePanel = false;
    Rect name = {0, 0, 0, 0}, panel = {0, 0, 0, 0};
};

// Where a new label goes: A (name-anchored, x clamped to the panel) while the target menu is shown, else B (fixed spot on the panel), else A at
// the last seen name rect. false = nothing to anchor to yet.
inline bool choosePlacement(const Geometry& g, Rect* r, Pos* p) {
    const bool panelOk = g.havePanel && g.panel.w > 0 && g.panel.h > 0;
    if (g.menuShown || (!panelOk && g.haveName)) {
        *r = peekRectName(g.name.x, g.name.y, g.name.w, g.name.h);
        *p = Pos::A;
        if (panelOk) *r = clampLabelX(*r, g.panel.x, g.panel.w);
        return true;
    }
    if (panelOk) {
        *r = peekRectFixed(g.panel.x, g.panel.y, g.panel.w, g.panel.h);
        *p = Pos::B;
        *r = clampLabelX(*r, g.panel.x, g.panel.w);
        return true;
    }
    return false;
}

// The position chosen when a label first shows, kept (same rect) until the label is gone, so the line never jumps when the name tag
// appears or vanishes, and the Took line replaces the peek line in the very same rect.
// 0.4.0-dev block anchor (kept by the latch): the unclamped horizontal centre and top of the block, plus the target name's top for the flip.
struct Anchor { int cx = 0, top = 0, nameTop = 0; bool hasName = false; };
struct Latch {
    bool held = false;
    Pos pos = Pos::None;
    Rect rect = {0, 0, 0, 0};
    Anchor anchor;
    void set(Pos p, const Rect& r) { held = true; pos = p; rect = r; }
    void clear() { held = false; pos = Pos::None; rect = {0, 0, 0, 0}; anchor = Anchor(); }
};

// Block anchor, same A/B rule as choosePlacement: A under the target name (centre = name centre, top = name bottom + gap) while the target menu is
// shown or only a name rect is known, else B (centre of the HUD panel, PeekTopPercent % down). false = nothing to anchor to yet.
inline bool chooseAnchor(const Geometry& g, Anchor* a, Pos* p) {
    const bool panelOk = g.havePanel && g.panel.w > 0 && g.panel.h > 0;
    *a = Anchor();
    if (g.menuShown || (!panelOk && g.haveName)) {
        a->cx = g.name.x + g.name.w / 2; a->top = g.name.y + g.name.h + ToastGapBelowName; a->nameTop = g.name.y; a->hasName = true;
        *p = Pos::A;
        return true;
    }
    if (panelOk) {
        a->cx = g.panel.x + g.panel.w / 2; a->top = g.panel.y + (g.panel.h * PeekTopPercent) / 100;
        *p = Pos::B;
        return true;
    }
    return false;
}

// The block rect for anchor a and size w x h. x is clamped to [panel.x + 24, panel.x + panel.w - 24 - w] (centred when the block is wider).
// Bottom clamp: when the block would pass panel bottom - 24, position A with a known name flips above the name (bottom = name top - gap) if that
// fits inside the panel, else (and for B) the block moves up so its bottom sits on the limit. The top never goes above panel.y + 24.
// panel.w <= 0 = unknown: no clamp.
inline Rect blockRect(Pos pos, const Anchor& a, int w, int h, const Rect& panel) {
    Rect r = { a.cx - w / 2, a.top, w, h };
    if (panel.w <= 0 || panel.h <= 0) return r;
    r = clampLabelX(r, panel.x, panel.w);
    const int bottom = panel.y + panel.h - ScreenMarginPx, topLimit = panel.y + ScreenMarginPx;
    if (r.y + h > bottom) {
        int y = bottom - h;
        if (pos == Pos::A && a.hasName) {
            const int flip = a.nameTop - ToastGapBelowName - h;
            if (flip >= topLimit) y = flip;
        }
        r.y = y;
    }
    if (r.y < topLimit) r.y = topLimit;
    return r;
}

// Block-relative rects (add the block origin).
inline Rect headerRect(int w) { return { BlockPadX, BlockPadY, w - 2 * BlockPadX + LabelSlackPx, RowH }; }
inline Rect rowRect(unsigned i, int w) { return { BlockPadX, BlockPadY + RowH + HeaderGapPx + static_cast<int>(i) * RowH, w - 2 * BlockPadX + LabelSlackPx, RowH }; }
// Indicator: text starts at w - BlockPadX - indPx so it ends on the right padding line.
inline Rect indicatorRect(int w, int indPx) { return { w - BlockPadX - indPx, BlockPadY, indPx + LabelSlackPx, RowH }; }
// Row index under the screen point (px, py) for the block at blk with vis visible rows, -1 = none (header, padding, outside).
inline int rowAtPoint(const Rect& blk, unsigned vis, float px, float py) {
    const float x0 = static_cast<float>(blk.x + BlockPadX), x1 = static_cast<float>(blk.x + blk.w - BlockPadX);
    const float y0 = static_cast<float>(blk.y + BlockPadY + RowH + HeaderGapPx);
    if (px < x0 || px >= x1 || py < y0) return -1;
    const int i = static_cast<int>((py - y0) / static_cast<float>(RowH));
    return i < static_cast<int>(vis) ? i : -1;
}
inline bool pointInRect(const Rect& r, float px, float py) {
    return px >= static_cast<float>(r.x) && px < static_cast<float>(r.x + r.w) && py >= static_cast<float>(r.y) && py < static_cast<float>(r.y + r.h);
}

// The Took line: text + how long it stays up.
struct ToastState {
    bool active = false;
    uint32_t shownAt = 0, durMs = ToastMs, lastTick = 0;
    char text[96] = {0};
    void start(const char* t, uint32_t now, uint32_t dur) {
        strncpy(text, t, sizeof text - 1);
        text[sizeof text - 1] = 0;
        shownAt = now; durMs = dur; active = true; lastTick = now;
    }
    bool visible(uint32_t now) const { return active && toastVisible(now, shownAt, durMs); }
    // Per frame: while `hold` (the cursor is over the list) the timer stands still -- shownAt moves forward by the time since the last tick.
    void tick(uint32_t now, bool hold) {
        if (active && hold && visible(now)) shownAt += msSince(now, lastTick);
        lastTick = now;
    }
};

// Dwell debounce for the auto-open: the same best candidate on consecutive scans for at least needMs.
struct Dwell {
    uint32_t id = 0, sinceMs = 0;
    void reset() { id = 0; sinceMs = 0; }
    // cand = the container the scan would open now (0 = none). True when it has been the candidate for >= needMs.
    bool update(uint32_t cand, uint32_t now, uint32_t needMs) {
        if (!cand) { reset(); return false; }
        if (cand != id) { id = cand; sinceMs = now; }
        return msSince(now, sinceMs) >= needMs;
    }
};

// The one peek. Phases: idle -> expecting (OpenInventory called, waiting for its ShowContainerGui) -> active -> idle.
struct Peek {
    // expecting
    uintptr_t expect = 0;
    uint32_t expectMs = 0;
    // shared by expecting and active
    uint32_t serverId = 0;       // area-list object id (cooldown key, plc re-resolve)
    uint32_t opener = 0;         // leader server object id passed to OpenInventory (server-side Close fallback)
    uintptr_t plc = 0;
    uintptr_t area = 0;          // leader area at open
    bool openSeen = false;       // the container's OnOpen (event 0x16) was handled
    uint32_t openFrame = 0;
    // active
    bool active = false;
    uint32_t obj = 0;            // client object id from ShowContainerGui (the Close message takes it)
    uint32_t sinceMs = 0, sinceFrame = 0;
    uint32_t closeStartMs = 0;   // first Close attempt
    bool closeTried = false;
    char text[128] = {0};

    bool busy() const { return active || expect != 0; }
    void begin(uintptr_t p, uint32_t sid, uintptr_t a, uint32_t now) {
        clear();
        expect = p; plc = p; serverId = sid; area = a; expectMs = now;
    }
    // ShowContainerGui for plc: consumed as the peek when it is the expected container. Returns true when it was.
    bool onShow(uint32_t o, uintptr_t p, uint32_t now, uint32_t frame) {
        if (expect == 0 || p != expect) return false;
        expect = 0; active = true; obj = o; sinceMs = now; sinceFrame = frame; closeStartMs = 0; closeTried = false; text[0] = 0;
        return true;
    }
    // Placeable event: the first OnOpen of the peeked container (expecting or active).
    bool onEvent(uintptr_t p, int id, uint32_t frame) {
        if ((!active && expect == 0) || p == 0 || p != plc || id != EvtOpen || openSeen) return false;
        openSeen = true; openFrame = frame; return true;
    }
    bool expired(uint32_t now) const { return expect != 0 && msSince(now, expectMs) >= ExpectTimeoutMs; }
    // The container should be reported empty: OnOpen settled (or SendTimeoutMs after Show) and the repository is still empty.
    bool emptyNow(int itemCount, uint32_t frame, uint32_t now) const {
        return active && itemCount == 0 && readyToSend(openSeen, frame, openFrame, now, sinceMs);
    }
    void clear() {
        expect = 0; expectMs = 0; serverId = 0; opener = 0; plc = 0; area = 0; openSeen = false; openFrame = 0; active = false; obj = 0;
        sinceMs = sinceFrame = 0; closeStartMs = 0; closeTried = false; text[0] = 0;
    }
};

// ---- formatters (fixed formats the play-test row greps for)
inline int fmtShow(char* b, size_t n, uint32_t obj, int flag, uintptr_t plc, int open, int count) {
    return snprintf(b, n, "SHOW obj=%08x flag=%d plc=%08x open=%d n=%d", obj, flag, static_cast<unsigned>(plc), open, count);
}
inline int fmtItem(char* b, size_t n, unsigned i, uint32_t id, int stack) { return snprintf(b, n, "ITEM %u id=%08x stack=%d", i, id, stack); }
inline int fmtEvtRaw(char* b, size_t n, unsigned plc, int a1, int a2, unsigned a3, int id16, int ref) {
    return snprintf(b, n, "EVTRAW plc=%08x a1=%d a2=%d a3=%08x u16=%04x ref=%d", plc, a1, a2, a3, id16 & 0xFFFF, ref);
}
inline int fmtEvt(char* b, size_t n, int id, uint32_t dtMs, int count) { return snprintf(b, n, "EVT id=%x dt=%ums n=%d", id, dtMs, count); }
inline int fmtClose(char* b, size_t n, int flag, uint32_t dtMs, int count, int open) {
    return snprintf(b, n, "CLOSE flag=%d dt=%ums n=%d open=%d", flag, dtMs, count, open);
}
inline int fmtTakeArm(char* b, size_t n, uint32_t obj, uintptr_t plc, int count) {
    return snprintf(b, n, "TAKE-ARM obj=%08x plc=%08x n=%d", obj, static_cast<unsigned>(plc), count);
}
inline int fmtVanilla(char* b, size_t n, Reason r) { return snprintf(b, n, "VANILLA reason=%s", reasonName(r)); }
inline int fmtTakeSend(char* b, size_t n, uint32_t obj, uint32_t dtMs, int open, int count) {
    return snprintf(b, n, "TAKE-SEND obj=%08x dt=%ums open=%d n=%d", obj, dtMs, open, count);
}
inline int fmtTookItem(char* b, size_t n, unsigned i, uint32_t id, const char* name, int stack) {
    char nm[ToastNameMax + 1];
    sanitizeName(nm, sizeof nm, name);
    return snprintf(b, n, "TOOK %u id=%08x name=%s stack=%d", i, id, nm, stack);
}
inline int fmtToast(char* b, size_t n, const char* text) { return snprintf(b, n, "TOAST %s", text); }
inline int fmtTakeEmpty(char* b, size_t n, uint32_t obj, uint32_t dtMs, int open) {
    return snprintf(b, n, "TAKE-EMPTY obj=%08x dt=%ums open=%d", obj, dtMs, open);
}
inline int fmtPeekOpen(char* b, size_t n, uint32_t sid, uintptr_t plc, float dist, int count) {
    return snprintf(b, n, "PEEK-OPEN obj=%08x plc=%08x dist=%.2f n=%d", sid, static_cast<unsigned>(plc), static_cast<double>(dist), count);
}
inline int fmtPeekShow(char* b, size_t n, uint32_t obj, uintptr_t plc, int flag, int count) {
    return snprintf(b, n, "PEEK-SHOW obj=%08x plc=%08x flag=%d n=%d", obj, static_cast<unsigned>(plc), flag, count);
}
inline int fmtPeekLabel(char* b, size_t n, const char* text) { return snprintf(b, n, "PEEK-LABEL %s", text); }
inline int fmtPeekEmpty(char* b, size_t n, uint32_t obj, uint32_t dtMs, int open) {
    return snprintf(b, n, "PEEK-EMPTY obj=%08x dt=%ums open=%d", obj, dtMs, open);
}
inline int fmtPeekTake(char* b, size_t n, uint32_t obj, uint32_t dtMs, int count) {
    return snprintf(b, n, "PEEK-TAKE obj=%08x dt=%ums n=%d", obj, dtMs, count);
}
inline int fmtPeekClose(char* b, size_t n, uint32_t obj, const char* reason, int sent, float dist, int count) {
    return snprintf(b, n, "PEEK-CLOSE obj=%08x reason=%s sent=%d dist=%.2f n=%d", obj, reason, sent, static_cast<double>(dist), count);
}
inline int fmtPeekNoShow(char* b, size_t n, uint32_t sid, uint32_t dtMs, int open) {
    return snprintf(b, n, "PEEK-NOSHOW obj=%08x dt=%ums open=%d", sid, dtMs, open);
}
inline int fmtPeekRefused(char* b, size_t n, uint32_t sid, int open) { return snprintf(b, n, "PEEK-REFUSED obj=%08x open=%d", sid, open); }
inline int fmtSuppress(char* b, size_t n, const char* from, const char* to) { return snprintf(b, n, "SUPPRESS %s>%s", from, to); }
inline int fmtScan(char* b, size_t n, uint32_t sid, float dist, const CandFlags& f, const char* why) {
    return snprintf(b, n, "SCAN nearest obj=%08x dist=%.2f inv=%d lock=%d trap=%d use=%d open=%d cd=%d why=%s", sid, static_cast<double>(dist),
                    f.hasInv, f.locked, f.trap, f.useable, f.open, f.cooldown, why);
}
inline int fmtScanNone(char* b, size_t n) { return snprintf(b, n, "SCAN nearest none"); }
inline int fmtArea(char* b, size_t n, uintptr_t area, int objs) {
    return snprintf(b, n, "AREA area=%08x objs=%d", static_cast<unsigned>(area), objs);
}
inline int fmtCooldown(char* b, size_t n, const char* what, uint32_t sid, float dist, unsigned size) {
    return snprintf(b, n, "COOLDOWN %s obj=%08x dist=%.2f size=%u", what, sid, static_cast<double>(dist), size);
}
inline int fmtLatch(char* b, size_t n, Pos p, const Rect& r) {
    return snprintf(b, n, "LABEL-LATCH pos=%s rect=%d,%d,%d,%d", posName(p), r.x, r.y, r.w, r.h);
}
inline int fmtLabelExtent(char* b, size_t n, unsigned chars, int x, int y, int w, int h) {
    return snprintf(b, n, "LABEL-EXTENT chars=%u extent=%d,%d,%d,%d", chars, x, y, w, h);
}
inline int fmtGiveUpServerClose(char* b, size_t n, uintptr_t plc, int flag) {
    return snprintf(b, n, "GIVEUP-SERVERCLOSE plc=%08x flag=%d", static_cast<unsigned>(plc), flag);
}
inline int fmtListLog(char* b, size_t n, unsigned rows, unsigned items, unsigned off, int w, int h) {
    return snprintf(b, n, "LIST rows=%u items=%u off=%u w=%d h=%d", rows, items, off, w, h);
}
inline int fmtScroll(char* b, size_t n, unsigned off, unsigned maxOff) { return snprintf(b, n, "SCROLL off=%u/%u", off, maxOff); }
inline int fmtWheelSub(char* b, size_t n, int token, unsigned failedTries) {
    return snprintf(b, n, "WHEEL-SUB token=%d after=%u", token, failedTries);
}
inline int fmtErr(char* b, size_t n, const char* what, uint32_t dtMs) { return snprintf(b, n, "ERR %s dt=%ums", what, dtMs); }

}   // namespace qlogic
