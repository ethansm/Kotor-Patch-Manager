// QuickLootLogic.h -- pure helpers of quick-loot (doc 52 QL0/QL1a/QL1b/QL2p): event filter, wrap-safe ms, item cap, the pending-container
// tracker, the take-all decision functions (shouldConsume / readyToSend / giveUp / closeFlagFor), the pending-take state, the "Took:" toast
// text / timing / placement (QL1b), the QL2p proximity peek (candidate filter, nearest pick, cooldown / hysteresis, leave decision, suppress
// transitions, key edge, peek state, peek label), 0.3.1-dev label layout (one shared 1200 px rect, 56-char one-line text, latched position A/B,
// x clamp, backing panel rect, Took duration rule, dwell debounce) and the log line formatters. 0.4.0-dev adds the multi-row list layer
// (row text, scroll view, block geometry, bottom clamp, Took duration per row, hover hold) below the fmtPeek section. 0.5.0-dev (QL2V) restyles
// that block like the vanilla inventory list: bordered protoitem rows with a gap, an item icon per row (resref sanitizer here), RowH 64, and a
// vanilla-style scrollbar (geometry, thumb, hit test and step logic here). No game memory, no
// Win32: host-testable (quick_loot_logic_test.cpp; build with -I../_shared_src for TextFit.h).
#pragma once
#include "TextFit.h"   // textfit::textPx: row / block widths use the DLL's own dialogfont10x10 advance table
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <cmath>

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

// QL6 Give / quest hand-in container offsets
constexpr int Gui_ContainerOffset = 0x54;   // CGuiInGame + 0x54 -> CSWGuiContainer*
constexpr int Container_ModeFlags = 0x70;   // CSWGuiContainer + 0x70, bit 0: 1=Take, 0=Give
constexpr int Plc_OnInvDisturbed = 0x30C;   // CSWSPlaceable + 0x30C, CExoString

enum class Reason { Consume = 0, Disabled, NoHud, Shift, NoPlc, Busy, Give };

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
        case Reason::Give: return "give";
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

// 0.7.0-dev (QL9): BioWare TLK names carry {tag} prefixes ("{08}Disruptor Carbine", "{light 04}Mandalorian Combat Suit") the engine hides.
// Copies in -> out (cap >= 1) without every "{...}" of at most 12 inner chars (no nested braces), runs of blanks collapsed to one, none at
// either end. A longer or unclosed brace is kept as text. Returns the length.
inline unsigned stripTags(char* out, unsigned cap, const char* in) {
    unsigned n = 0;
    bool space = false;
    if (in) {
        for (unsigned i = 0; in[i] && n + 1 < cap; ++i) {
            if (in[i] == '{') {
                unsigned j = i + 1;
                while (in[j] && in[j] != '}' && in[j] != '{' && j - i - 1 <= 12) ++j;
                if (in[j] == '}' && j - i - 1 <= 12) { i = j; continue; }
            }
            if (in[i] == ' ' || in[i] == '\t') { space = n > 0; continue; }
            if (space && n + 2 < cap) out[n++] = ' ';
            space = false;
            out[n++] = in[i];
        }
    }
    out[n] = 0;
    return n;
}

// 0.7.0-dev (QL9) shared tooltip contract: QuickLootRules.h buildTipModel fills it, buildTip lays it out. title = stripped name, stack
// (-1 = not stackable), sub = category line ("Light Armor", "Disruptor Rifle"), rules = label / value rows, flavor = description text
// (full = no rules: the whole description, up to 7 lines), foot = "Equipped: ..." / "In inventory: N" ("" = none).
struct TipRule { char label[16]; char value[96]; };
struct TipModel { char title[48]; int stack; char sub[40]; TipRule rules[10]; unsigned nRules; char flavor[600]; bool full; char foot[64]; };

// Copies an item name into out (cap >= 2): {tags} stripped first (stripTags: log lines show "Disruptor Carbine", not "{08}Disruptor Carbine"), stops
// at the first NUL, cuts at NameRawMax, replaces every byte outside 0x20..0x7E with '?', trims trailing spaces. A null/empty result becomes "item".
// Returns the length.
inline unsigned sanitizeName(char* out, unsigned cap, const char* in) {
    char st[NameRawMax + 64];   // the stripped name; a longer one is cut here, far past NameRawMax
    stripTags(st, sizeof st, in);
    unsigned n = 0;
    const unsigned lim = cap - 1 < NameRawMax ? cap - 1 : NameRawMax;
    while (n < lim && st[n]) {
        const unsigned char c = static_cast<unsigned char>(st[n]);
        out[n] = (c < 0x20 || c > 0x7E) ? '?' : static_cast<char>(c);
        ++n;
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
    if (total <= 1) snprintf(pre, sizeof pre, "Acquired: ");
    else snprintf(pre, sizeof pre, "Acquired (%u): ", total);
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
    bool hasInv = false, locked = false, trap = false, useable = false, open = false, cooldown = false, isGive = false;
};
// First failing candidate condition ("ok" when none). Order: inventory, locked, trap, useable, open, cooldown, give, range.
inline const char* candidateWhy(const CandFlags& f, float dist, float openM) {
    if (!f.hasInv) return "noinv";
    if (f.locked) return "locked";
    if (f.trap) return "trap";
    if (!f.useable) return "unuse";
    if (f.open) return "open";
    if (f.cooldown) return "cooldown";
    if (f.isGive) return "give";
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
enum class Leave { None = 0, Gone, Suppress, Area, Far, Off, Timeout, MouseLeave };   // Off (0.6.0-dev): ProximityEnabled was switched off while a peek was up; Timeout (QL2): auto-close; MouseLeave (QL3): mouse unhover
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
        case Leave::Off: return "off";
        case Leave::Timeout: return "timeout";
        case Leave::MouseLeave: return "mouseleave";
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
    if (total == 1) snprintf(pre, sizeof pre, "[%s] Get Item: ", key ? key : "key");
    else snprintf(pre, sizeof pre, "[%s] Get Items (%d): ", key ? key : "key", total);
    const unsigned extra = static_cast<unsigned>(total) > count ? static_cast<unsigned>(total) - count : 0u;
    return detail::fmtList(out, cap, pre, PeekMaxChars, names, stacks, count, extra, info);
}

// ---- 0.4.0-dev: multi-row item list (doc 52 QL2L); 0.5.0-dev (QL2V) restyle. One block = header row + up to ListRows item rows on a vanilla frame panel.
// Geometry (engine px at 3840x1600, the dev box; all constants measured with scripts/character_page_mockup.py text_size):
//   dialogfont10x10 = 2.7467 px per font texel (1.03 x 1600/600); glyph box 10 texels = 27.5 px tall; the tooltip line pitch is 12.5 texels = 34 px.
//   0.5.0-dev rows are vanilla protoitem buttons (inventory_p: bordered uibit_brdr_16bct / 16bet + uibit_fill_2bt, teal; HILIGHT = the same images
//   with the near-white-green tint): RowH 64 px (vanilla 50 units x 2.667 = 133 px; "a little larger than 0.4.0's 32"), RowGapPx between rows, an item
//   icon square at the left (RowH - 2 x RowIconPad) and the name after it. The header keeps its 32 px row (HeaderH). The scrollbar (SbW wide, only
//   when the list has more rows than ListRows) sits right of the rows, arrows at both ends, a thumb sized visible/total.
constexpr int NameCap = 48;                // captured name buffer (rowName reads at most NameCap-1 chars)
constexpr unsigned ListMaxItems = 64;      // names captured per peek / take; items past this only count towards the "+K more" row
constexpr unsigned ListRowsDefault = 4, ListRowsMin = 1, ListRowsMax = 12;   // visible item rows (quick_loot.ini ListRows)
constexpr unsigned RowNameMax = 40;        // an item name longer than this is cut: first RowNameMax-3 chars + "..."
constexpr unsigned RowTextCap = 56;        // one row's text: name (<= 40) + " x999" + NUL fits
constexpr unsigned ResrefCap = 17;         // icon resref buffer: 16 chars + NUL (the engine CResRef is char[16], NOT NUL-terminated)
constexpr int RowH = 64;                   // item row height (px). Alternatives rendered by the mockup: RowHAlt1 / RowHAlt2
constexpr int RowHAlt1 = 56, RowHAlt2 = 72;
constexpr int RowGapPx = 4;                // air between item rows (the vanilla list shows the adjacent borders; a small gap reads as separate rows)
constexpr int HeaderH = 32;                // header / indicator row height (the 0.4.0 RowH)
constexpr int HeaderGapPx = 6;             // air between the header row and the first item row
constexpr int RowIconPad = 8;              // icon inset from the row's left / top / bottom edge (0.7.0-dev QL10: 48 px icon in the 64 px row; the 16 px border's drawn line is only its outer ~3 px)
constexpr int StackColW = 52;              // 0.7.0-dev QL10: right-aligned stack column ("x2", up to "x999"), taken from the row text
constexpr int IconGapPx = 12;              // icon -> text
constexpr int RowTextPadR = 16;            // text -> row right edge
constexpr int TextGlyphH = 28;             // glyph box (27.5 px), used to centre the text vertically in the row (align 9 = top-left)
constexpr int TextBoxH = 32;               // text label height (a wrapped label is taller than this)
constexpr int SbW = 36;                    // scrollbar column width (engine px); the vanilla bar is 16 units = 77 px wide, rows here are far narrower
constexpr int SbGapPx = 8;                 // rows -> scrollbar
constexpr int SbReserve = SbW + SbGapPx;   // width taken from the rows when the scrollbar shows
constexpr int SbInset = 2;                 // track outline thickness (the outline is a teal label under a dark inset label)
constexpr int SbThumbMin = 24;
constexpr int BlockPadX = 28, BlockPadY = 20;   // inside the frame; the vanilla template frame (LBL_MOULDING3, DIMENSION 16) eats 16 px of each edge
constexpr int BlockMinW = 400, BlockMaxW = 1000;
constexpr int BezelInsetPx = 38;           // 0.8.0-dev (QL14): hardware bezel rail inset on all 4 sides
constexpr int LabelSlackPx = 12;           // row / header label rects are this much wider than their text budget (a wrapped label is taller than a row)
constexpr int IndicatorGapPx = 36;         // min gap between the header text and the "1-8 of 25" indicator
constexpr float FrameAlpha = 1.0f;         // frame panel alpha: the template fill (uibit_fill_2bt) is already black at alpha 199/255 = 0.78 ("about 0.75"), alpha 1.0 keeps the vanilla look
// 0.7.0-dev panel opacity: the adapter draws a SECOND uibit_fill_2bt (baked alpha 199/255) at fillRect(frame) UNDER the frame; two layers give
// 1 - 0.22 x 0.22 = 0.95 opacity. NEVER setAlpha above 1 on the frame itself: 0.94 x 0.78 = 0.73 is MORE transparent (the engine multiplies, it does not add).
// BackAlpha (above) stays for the flat fallback backing.
constexpr float FillBakedAlpha = 199.0f / 255.0f;   // uibit_fill_2bt as shipped
constexpr int FillInsetPx = 6;             // the underlay sits this far inside the frame on every side (inside the border's drawn line)
inline Rect fillRect(const Rect& frame) { return { frame.x + FillInsetPx, frame.y + FillInsetPx, frame.w - 2 * FillInsetPx, frame.h - 2 * FillInsetPx }; }
constexpr int WheelStep = 120;             // wheel delta per row
constexpr uint32_t WheelFreshMs = 250;     // the wheel callback consumes only while the block was drawn within this long
constexpr uint32_t TookBaseMs = 3000, TookRowMs = 500, TookCapMs = 6000;   // Took list: 3 s + 0.5 s per list row past TookFreeRows, cap 6 s
constexpr unsigned TookFreeRows = 3;

// Icon geometry for a row height.
inline int iconPx(int rowH = RowH) { return rowH - 2 * RowIconPad; }
inline int rowChromePx(int rowH = RowH) { return RowIconPad + iconPx(rowH) + IconGapPx + StackColW + RowTextPadR; }   // row width that is not text
// Widest row text the block can hold: BlockMaxW minus the pads, the worst-case chrome (RowHAlt2) and the scrollbar.
constexpr int RowTextMaxPx = BlockMaxW - 2 * BlockPadX - (RowIconPad + (RowHAlt2 - 2 * RowIconPad) + IconGapPx + StackColW + RowTextPadR) - SbReserve;

// Copies a 16-byte engine CResRef (not NUL-terminated) into out (>= ResrefCap): stops at the first NUL, bytes outside 0x20..0x7E become '?',
// trailing spaces are trimmed. Returns the length (0 = no icon). raw may be null.
inline unsigned sanitizeResref(char* out, const char* raw) {
    unsigned n = 0;
    if (raw) {
        while (n < 16 && raw[n]) {
            const unsigned char c = static_cast<unsigned char>(raw[n]);
            out[n] = (c < 0x20 || c > 0x7E) ? '?' : static_cast<char>(c);
            ++n;
        }
    }
    while (n > 0 && out[n - 1] == ' ') --n;
    out[n] = 0;
    return n;
}
// A base item index from [item+0xC] is plausible (baseitems.2da has well under 1024 rows): the adapter refuses an icon call for anything else.
constexpr int MaxBaseItemIdx = 1024;
inline bool baseItemIdxOk(int idx) { return idx >= 0 && idx < MaxBaseItemIdx; }

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

// Row name: {tags} stripped, printable bytes (others '?'), at most NameCap-1 of them, trailing spaces trimmed, empty -> "item". Longer than RowNameMax: the first
// RowNameMax-3 chars, trailing non-alphanumerics trimmed, "..." appended ("Mandalorian Heavy Armor Plating Mk IV Upgrade" -> "Mandalorian Heavy Armor Plating Mk IV Up...").
// cap >= RowNameMax + 1. Returns the length.
inline unsigned rowName(char* out, unsigned cap, const char* in) {
    char tmp[NameCap];
    char st[NameCap + 64];   // {tags} stripped first (0.7.0-dev): "{08}Disruptor Carbine" lists as "Disruptor Carbine"
    stripTags(st, sizeof st, in);
    unsigned n = 0;
    while (n < static_cast<unsigned>(NameCap) - 1 && st[n]) {
        const unsigned char c = static_cast<unsigned char>(st[n]);
        tmp[n] = (c < 0x20 || c > 0x7E) ? '?' : static_cast<char>(c);
        ++n;
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
// One item row's name text: rowName; when it is wider than maxPx the name is cut further (+ "...") so it never wraps. The stack count is NOT in the
// text since 0.7.0-dev (QL10): it has its own right-aligned column (rowStackText / rowStackRect).
inline unsigned rowText(char* out, unsigned cap, const char* name, int maxPx = RowTextMaxPx) {
    char nm[RowNameMax + 8];
    unsigned n = rowName(nm, sizeof nm, name);
    char buf[RowTextCap + 16];
    snprintf(buf, sizeof buf, "%s", nm);
    if (textPx10(buf) > maxPx) {
        if (n >= 3 && memcmp(nm + n - 3, "...", 3) == 0) n -= 3;
        while (n > 1) {
            --n;
            char t[RowTextCap + 16];
            snprintf(t, sizeof t, "%.*s...", static_cast<int>(n), nm);
            if (textPx10(t) <= maxPx) { memcpy(buf, t, sizeof t); break; }
        }
    }
    unsigned len = static_cast<unsigned>(strlen(buf));
    if (len >= cap) len = cap - 1;
    memcpy(out, buf, len);
    out[len] = 0;
    return len;
}

// Stack column text: "x2" for 2 <= N <= 999, else "" (cap >= 6). Returns the length.
inline unsigned rowStackText(char* out, size_t cap, int stack) {
    if (stack >= 2 && stack <= 999) return static_cast<unsigned>(snprintf(out, cap, "x%d", stack));
    if (cap) out[0] = 0;
    return 0;
}

// The captured list as rows. items = rows for captured names, more = items past the capture (shown as one last "+K more" row).
struct ListData {
    unsigned items = 0, more = 0;
    int maxRowPx = 0;                  // widest row text (incl. the "+K more" row)
    char rows[ListMaxItems][RowTextCap];
    char icons[ListMaxItems][ResrefCap];   // 0.5.0-dev: icon resref per item row ("" = none; the "+K more" row has none)
    char stackTxt[ListMaxItems][8];    // 0.7.0-dev: stack column text per item row ("x2", "" = none; kept per item so scrolling keeps them)
    unsigned rowsTotal() const { return items + (more ? 1u : 0u); }
    unsigned totalItems() const { return items + more; }
    bool empty() const { return items == 0; }
    void clear() { items = 0; more = 0; maxRowPx = 0; }
    // count names captured out of `total` items in the container (total < count is treated as count). icons: optional resref per item.
    void set(const char* const* names, const int* stacks, unsigned count, unsigned total, const char* const* iconRes = nullptr) {
        clear();
        if (count > ListMaxItems) count = ListMaxItems;
        for (unsigned i = 0; i < count; ++i) {
            rowText(rows[i], RowTextCap, names ? names[i] : nullptr);
            rowStackText(stackTxt[i], sizeof stackTxt[i], stacks ? stacks[i] : -1);
            sanitizeResref(icons[i], iconRes ? iconRes[i] : nullptr);
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
    const char* iconAt(unsigned i) const { return i < items ? icons[i] : ""; }
    const char* stackAt(unsigned i) const { return i < items ? stackTxt[i] : ""; }
};

// 0.7.0-dev (QL11) list order: plot items first, credits second, then the rest; stable (the container's own order inside each group). A plot item
// that is also a credit counts as plot. plot / credits are per captured item (n of them, either may be null = none); order[0..n) receives the
// source indices in display order (n <= ListMaxItems fits uint8_t).
inline void sortOrder(const uint8_t* plot, const bool* credits, unsigned n, uint8_t* order) {
    unsigned k = 0;
    for (int g = 0; g < 3; ++g) {
        for (unsigned i = 0; i < n; ++i) {
            const int grp = (plot && plot[i]) ? 0 : ((credits && credits[i]) ? 1 : 2);
            if (grp == g) order[k++] = static_cast<uint8_t>(i);
        }
    }
}

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
    // 0.5.0-dev scrollbar: moves the first visible row by `rows` (+ = down the list), clamped. true when off changed.
    bool step(int rows) {
        acc = 0;
        if (!scrollable()) return false;
        long n = static_cast<long>(off) + rows;
        const long hi = static_cast<long>(maxOff());
        if (n < 0) n = 0;
        if (n > hi) n = hi;
        const bool ch = static_cast<unsigned>(n) != off;
        off = static_cast<unsigned>(n);
        return ch;
    }
};

// Scrollbar parts a click can land on. Up / Down = one row, PageUp / PageDown (the track above / below the thumb) = one page (`rows`), Thumb = nothing.
enum class SbPart { None = 0, Up, Down, PageUp, PageDown, Thumb };
inline const char* sbPartName(SbPart p) {
    switch (p) {
        case SbPart::None: return "none";
        case SbPart::Up: return "up";
        case SbPart::Down: return "down";
        case SbPart::PageUp: return "pageup";
        case SbPart::PageDown: return "pagedown";
        case SbPart::Thumb: return "thumb";
    }
    return "?";
}
// The scroll a click on `p` asks for: rows to step (+ = down), 0 = none.
inline int sbStepFor(SbPart p, unsigned pageRows) {
    switch (p) {
        case SbPart::Up: return -1;
        case SbPart::Down: return 1;
        case SbPart::PageUp: return -static_cast<int>(pageRows);
        case SbPart::PageDown: return static_cast<int>(pageRows);
        default: return 0;
    }
}

// "1-8 of 25": first / last item numbers of the visible rows (1-based), total items.
inline int fmtRange(char* out, size_t cap, unsigned first, unsigned last, unsigned total) {
    return snprintf(out, cap, "%u-%u of %u", first, last, total);
}
// "[F] Take all (25)" / "Took (25)". total = items in the container.
inline int fmtHeader(char* out, size_t cap, bool took, const char* key, unsigned total) {
    if (took) return snprintf(out, cap, "Acquired Items (%u)", total);
    return snprintf(out, cap, "[%s] Get Items (%u)", key ? key : "key", total);
}

// ---- 0.5.0-dev: block / row / scrollbar geometry (block-relative px; add the block origin). rowH is a parameter only so the tests and the
// mockup can check the 56 / 72 alternatives; the mod always uses RowH.
// 0.8.0-dev (QL14): Unified Header geometry and hardware bezel padding
inline Rect bezelOuterRect(const Rect& blk, bool hasBezel) {
    return hasBezel ? Rect{ blk.x - BezelInsetPx, blk.y - BezelInsetPx, blk.w + 2 * BezelInsetPx, blk.h + 2 * BezelInsetPx } : blk;
}

inline Rect titleRect(int w, int padX = BlockPadX, int padY = BlockPadY) {
    return { padX, padY, w - 2 * padX, 32 };
}

inline Rect titleDivRect(int w, int padX = BlockPadX, int padY = BlockPadY) {
    return { padX, padY + 32 + 6, w - 2 * padX, 2 };
}

inline Rect actionBtnRect(int w, int padX = BlockPadX, int padY = BlockPadY) {
    return { padX, padY + 32 + 6 + 2 + 8, w - 2 * padX, 40 };
}

inline Rect actionLedRect(const Rect& btnRect) {
    return { btnRect.x + 12, btnRect.y + 12, 16, 16 };
}

inline Rect actionTextRect(const Rect& btnRect, bool hasLed) {
    const int tx = btnRect.x + (hasLed ? 36 : 12);
    return { tx, btnRect.y + 2, btnRect.w - (hasLed ? 48 : 24), 32 };
}

inline int rowsTopPx(bool hasUnified = false) {
    return hasUnified ? (BlockPadY + 96) : (BlockPadY + HeaderH + HeaderGapPx);
}

// Height of the list area for vis rows: rows + the gaps between them.
inline int listHeightPx(unsigned vis, int rowH = RowH) { return vis ? static_cast<int>(vis) * rowH + (static_cast<int>(vis) - 1) * RowGapPx : 0; }
inline int blockHeightPx(unsigned vis, int rowH = RowH, bool hasUnified = false) {
    return 2 * BlockPadY + (hasUnified ? 96 : HeaderH + HeaderGapPx) + listHeightPx(vis, rowH);
}
// The scrollbar needs room for both arrows and a minimum thumb; a list this short (ListRows 1) scrolls with the wheel only.
inline bool sbFits(unsigned vis, int rowH = RowH) { return listHeightPx(vis, rowH) >= 2 * SbW + SbThumbMin; }

struct Sb {
    bool shown = false;
    Rect col = {0, 0, 0, 0}, up = {0, 0, 0, 0}, down = {0, 0, 0, 0}, track = {0, 0, 0, 0}, thumb = {0, 0, 0, 0};   // block-relative
};
// Scrollbar layout for a block of width w showing vis rows of `total` list rows at first row `off`. shown only when total > vis and it fits.
inline Sb sbLayout(int w, unsigned vis, unsigned total, unsigned off, int rowH = RowH, bool hasUnified = false) {
    Sb o;
    if (vis == 0 || total <= vis || !sbFits(vis, rowH)) return o;
    o.shown = true;
    const int listH = listHeightPx(vis, rowH);
    o.col = { w - BlockPadX - SbW, rowsTopPx(hasUnified), SbW, listH };
    o.up = { o.col.x, o.col.y, SbW, SbW };
    o.down = { o.col.x, o.col.y + listH - SbW, SbW, SbW };
    o.track = { o.col.x, o.col.y + SbW, SbW, listH - 2 * SbW };
    const unsigned maxOff = total - vis;
    if (off > maxOff) off = maxOff;
    int th = static_cast<int>((static_cast<long long>(o.track.h) * vis) / total);   // thumb length = track x visible / total
    if (th < SbThumbMin) th = SbThumbMin;
    if (th > o.track.h) th = o.track.h;
    const int ty = o.track.y + static_cast<int>((static_cast<long long>(o.track.h - th) * off) / maxOff);
    o.thumb = { o.track.x + 4, ty, SbW - 8, th };
    return o;
}
// The scrollbar part under the block-relative point (rx, ry); None outside the column. Above / below the thumb on the track = page up / down.
inline SbPart sbHitAt(const Sb& s, float rx, float ry) {
    if (!s.shown) return SbPart::None;
    auto in = [](const Rect& r, float x, float y) { return x >= static_cast<float>(r.x) && x < static_cast<float>(r.x + r.w) && y >= static_cast<float>(r.y) && y < static_cast<float>(r.y + r.h); };
    if (in(s.up, rx, ry)) return SbPart::Up;
    if (in(s.down, rx, ry)) return SbPart::Down;
    if (!in(s.track, rx, ry)) return SbPart::None;
    if (ry < static_cast<float>(s.thumb.y)) return SbPart::PageUp;
    if (ry >= static_cast<float>(s.thumb.y + s.thumb.h)) return SbPart::PageDown;
    return SbPart::Thumb;
}

// Header flash after a per-item take: "Acquired: <name>" (+ " (N)" for a stack of N > 1) for FlashMs, the name cut so it fits maxPx. The scroll
// indicator is hidden while it shows (0.7.0-dev), so maxPx is the whole header width.
constexpr uint32_t FlashMs = 1500;
inline unsigned fmtFlashHeader(char* out, size_t cap, const char* name, int maxPx, int stack = -1) {
    char nm[RowNameMax + 8];
    unsigned n = rowName(nm, sizeof nm, name);
    char sfx[16] = "";
    if (stack > 1) snprintf(sfx, sizeof sfx, " (%d)", stack);
    char buf[RowNameMax + 40];
    snprintf(buf, sizeof buf, "Acquired: %s%s", nm, sfx);
    if (textPx10(buf) > maxPx) {
        if (n >= 3 && memcmp(nm + n - 3, "...", 3) == 0) n -= 3;
        while (n > 1) {
            --n;
            char t[RowNameMax + 40];
            snprintf(t, sizeof t, "Acquired: %.*s...%s", static_cast<int>(n), nm, sfx);
            if (textPx10(t) <= maxPx) { memcpy(buf, t, sizeof t); break; }
        }
    }
    unsigned len = static_cast<unsigned>(strlen(buf));
    if (len >= cap) len = static_cast<unsigned>(cap - 1);
    memcpy(out, buf, len);
    out[len] = 0;
    return len;
}

// Everything the adapter draws for the block this frame.
struct ListDraw {
    bool took = false, scrollable = false;
    bool sb = false;                   // the scrollbar shows (scrollable and it fits): the rows are SbReserve narrower
    bool unified = false;              // 0.8.0-dev (QL14): Unified Header Card (Proposal A+B Hybrid)
    int rowH = RowH;
    unsigned off = 0, vis = 0, rowsTotal = 0, items = 0;
    char header[72] = {0};             // "[F] Get Items (25)", or the 0.6.0-dev "Acquired: <name>" flash
    char indicator[24] = {0};          // "" when the whole list fits
    int headerPx = 0, indicatorPx = 0;
    char rows[ListRowsMax][RowTextCap];
    char icons[ListRowsMax][ResrefCap];   // icon resref per visible row ("" = none)
    char stacks[ListRowsMax][8];       // 0.7.0-dev: stack column text per visible row ("x2"; "" = none, also on the "+K more" row)
    int w = 0, h = 0;                  // block size (px), constant while scrolling
    Rect fill = {0, 0, 0, 0};          // 0.7.0-dev: underlay fill rect, BLOCK-relative (the adapter adds the block origin): fillRect({0,0,w,h})
};

// Builds the block for list d seen through view v. false = nothing to draw (empty list). The width follows the widest of ALL rows (not only the
// visible ones) so scrolling never resizes the block; h follows the visible row count (min(rows, list rows)).
// flashName (peek list only): the take flash replaces the header text, flashStack > 1 appends " (N)", and the scroll indicator is hidden meanwhile.
inline bool buildListDraw(const ListData& d, const ListView& v, bool took, const char* key, ListDraw* o, int rowH = RowH, const char* flashName = nullptr, int flashStack = -1, bool hasUnified = false) {
    *o = ListDraw();
    if (d.empty()) return false;
    const unsigned total = d.rowsTotal();
    unsigned rows = v.rows < ListRowsMin ? ListRowsMin : (v.rows > ListRowsMax ? ListRowsMax : v.rows);
    const unsigned vis = total < rows ? total : rows;
    unsigned off = v.off;
    const unsigned maxOff = total > rows ? total - rows : 0u;
    if (off > maxOff) off = maxOff;
    o->took = took; o->off = off; o->vis = vis; o->rowsTotal = total; o->items = d.totalItems(); o->rowH = rowH;
    o->unified = hasUnified;
    o->scrollable = total > rows;
    o->sb = o->scrollable && sbFits(vis, rowH);
    fmtHeader(o->header, sizeof o->header, took, key, d.totalItems());
    const bool flashing = flashName && !took;
    o->headerPx = textPx10(o->header);
    int need = d.maxRowPx + rowChromePx(rowH) + (o->sb ? SbReserve : 0);
    if (o->scrollable) {
        const unsigned last = (off + vis > d.items) ? d.totalItems() : off + vis;
        char worst[24];   // widest form (digits only grow with off): used for the width so the block stays still
        fmtRange(worst, sizeof worst, maxOff + 1, d.totalItems(), d.totalItems());
        const int hw = o->headerPx + IndicatorGapPx + textPx10(worst);   // the plain header, also while flashing: the flash never shrinks the block
        if (hw > need) need = hw;
        if (!flashing) {
            fmtRange(o->indicator, sizeof o->indicator, off + 1, last, d.totalItems());
            o->indicatorPx = textPx10(o->indicator);
        }
    } else if (o->headerPx > need) need = o->headerPx;
    if (flashing) {   // 0.6.0-dev: the take flash replaces the header text (the block widens for it, up to BlockMaxW); 0.7.0-dev: no indicator, so the whole width
        fmtFlashHeader(o->header, sizeof o->header, flashName, BlockMaxW - 2 * BlockPadX, flashStack);
        o->headerPx = textPx10(o->header);
        if (o->headerPx > need) need = o->headerPx;
    }
    int w = need + 2 * BlockPadX;
    if (w < BlockMinW) w = BlockMinW;
    if (w > BlockMaxW) w = BlockMaxW;
    o->w = w;
    o->h = blockHeightPx(vis, rowH, hasUnified);
    o->fill = fillRect({ 0, 0, o->w, o->h });
    for (unsigned i = 0; i < vis; ++i) {
        d.rowAt(off + i, o->rows[i], RowTextCap);
        snprintf(o->icons[i], ResrefCap, "%s", d.iconAt(off + i));
        snprintf(o->stacks[i], sizeof o->stacks[i], "%s", d.stackAt(off + i));
    }
    return true;
}

// ---- 0.6.0-dev (QL2A): block anchor. The 0.3.1 / 0.4.0 latch is gone: the block is re-placed EVERY frame from the target name tag the engine
// just positioned (K2HUD_TARGET_MENU_POST runs after 0x743850), so it follows the tag when the camera turns.
enum class Pos { None = 0, A, B };
inline const char* posName(Pos p) { return p == Pos::A ? "A" : (p == Pos::B ? "B" : "-"); }

// What the adapter sees of the screen this frame (all px): the target-menu name label (menuShown = its rect was read THIS frame), the HUD panel
// extent (havePanel; panel.w <= 0 = unknown) and the projected container point (Anchor 1; proj.state 0 = not tried, 1 = a value, 2 = the engine
// call was unavailable (note says why)).
struct Proj { int state = 0; float x = 0, y = 0; int ret = 0; const char* note = ""; float wx = 0, wy = 0, wz = 0; };   // wx..wz: the world point projected (PROJ log)   // ret: 1 in front of the camera, 2 behind, 0 unknown
struct Geometry {
    bool menuShown = false, havePanel = false;
    Rect name = {0, 0, 0, 0}, panel = {0, 0, 0, 0};
    Proj proj;
};

// Anchor modes (quick_loot.ini Anchor / the Mod Configuration page): 0 under the target name tag (default), 1 below the container (projected), 2 beside the name tag.
enum class AnchorMode { UnderName = 0, BelowContainer = 1, BesideName = 2 };
constexpr int AnchorModeCount = 3;
constexpr uint32_t NameHoldMs = 250;       // the last name rect is kept this long after the target menu goes away, then the block hides
constexpr int BesideGapPx = 24;            // mode 2: gap between the name tag and the block
constexpr int ProjGapPx = 16;              // mode 1: block top = projected y + this
inline int clampAnchorMode(int v) { return (v >= 0 && v < AnchorModeCount) ? v : 0; }
inline const char* anchorModeName(int m) {
    switch (m) {
        case 0: return "name";
        case 1: return "container";
        case 2: return "beside";
        case 3: return "fixed";
    }
    return "?";
}

// The name rect seen last and when. valid() = seen within NameHoldMs (wrap-safe).
struct NameHold {
    bool have = false;
    Rect rect = {0, 0, 0, 0};
    uint32_t seenMs = 0;
    void see(const Rect& r, uint32_t now) { have = true; rect = r; seenMs = now; }
    bool valid(uint32_t now) const { return have && msSince(now, seenMs) <= NameHoldMs; }
    void clear() { have = false; rect = {0, 0, 0, 0}; seenMs = 0; }
};

// Why a projected container point cannot be used (nullptr = usable): not finite, behind the camera (ret 2), no panel to check against, outside the panel.
inline const char* projReject(float x, float y, int ret, const Rect& panel) {
    if (!std::isfinite(x) || !std::isfinite(y)) return "nan";
    if (ret == 2) return "behind";
    if (panel.w <= 0 || panel.h <= 0) return "nopanel";
    if (x < static_cast<float>(panel.x) || x >= static_cast<float>(panel.x + panel.w) || y < static_cast<float>(panel.y) || y >= static_cast<float>(panel.y + panel.h)) return "outside";
    return nullptr;
}

// Block anchor: the unclamped horizontal centre and top of the block, plus the target name's top for the flip.
struct Anchor { int cx = 0, top = 0, nameTop = 0; bool hasName = false; };

// What anchorBlock decided: the mode actually used (-1 = none), why (static text, change-only logged), and the projection verdict for the PROJ line.
struct AnchorUse { int want = 0, mode = -1; const char* reason = "none"; const char* projWhy = "none"; };

// Change detector for the ANCHOR log line (requested mode, mode used, reason): update() is true exactly when one of them differs from the last call.
struct AnchorTrack {
    bool any = false;
    int want = 0, mode = -1;
    char reason[24] = {0};
    bool update(const AnchorUse& u) {
        const bool ch = !any || want != u.want || mode != u.mode || strcmp(reason, u.reason) != 0;
        any = true; want = u.want; mode = u.mode;
        snprintf(reason, sizeof reason, "%s", u.reason);
        return ch;
    }
    void reset() { any = false; }
};

// PROJ log gate: a verdict change (state + why) always logs; otherwise the first ProjLogMax lines of a session, and only when the projected point moved
// ProjMovePx or more since the last logged one (so the 40 lines are 40 different camera angles, not 40 identical frames).
constexpr unsigned ProjLogMax = 40;
constexpr float ProjMovePx = 24.0f;
struct ProjTrack {
    unsigned n = 0;
    int state = -1;
    char why[16] = {0};
    float lx = 0, ly = 0;
    bool want(int st, const char* w, float x, float y) {
        const bool ch = st != state || strcmp(why, w) != 0;
        state = st;
        snprintf(why, sizeof why, "%s", w);
        const bool moved = n == 0 || fabsf(x - lx) >= ProjMovePx || fabsf(y - ly) >= ProjMovePx;
        if (ch || (n < ProjLogMax && moved)) { if (n < ProjLogMax) ++n; lx = x; ly = y; return true; }
        return false;
    }
};

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

// Mode 2: left edge = name right + BesideGapPx, top = the name's top. Flips to the left of the name when the block would pass the panel's right
// margin; y is clamped inside the panel. panel.w <= 0 = unknown: no clamp, no flip.
inline Rect besideRect(const Rect& name, int w, int h, const Rect& panel) {
    Rect r = { name.x + name.w + BesideGapPx, name.y, w, h };
    if (panel.w <= 0 || panel.h <= 0) return r;
    const int right = panel.x + panel.w - ScreenMarginPx, left = panel.x + ScreenMarginPx;
    if (r.x + w > right) r.x = name.x - BesideGapPx - w;
    if (r.x < left) r.x = left;
    const int bottom = panel.y + panel.h - ScreenMarginPx, topLimit = panel.y + ScreenMarginPx;
    if (r.y + h > bottom) r.y = bottom - h;
    if (r.y < topLimit) r.y = topLimit;
    return r;
}

// This frame's anchor: the requested mode, or the fallback. Mode 1 needs a usable projection and falls back to mode 0 (reason says why) when it
// has none; modes 0 / 2 need the name rect (nameOk = seen within NameHoldMs). false = no anchor (the caller hides the block).
inline bool anchorBlock(int mode, bool nameOk, const Rect& name, const Geometry& g, int w, int h, Rect* out, AnchorUse* use) {
    const Rect panel = g.havePanel ? g.panel : Rect{0, 0, 0, 0};
    AnchorUse u;
    u.want = clampAnchorMode(mode);
    const char* why = nullptr;
    if (u.want == 1) {
        if (g.proj.state == 0) why = "proj-none";
        else if (g.proj.state == 2) why = "proj-unavail";
        else why = projReject(g.proj.x, g.proj.y, g.proj.ret, panel);
        u.projWhy = g.proj.state == 1 ? (why ? why : "ok") : (g.proj.state == 2 ? "unavail" : "none");
        if (g.proj.state == 1 && !why) {
            Anchor a;
            a.cx = static_cast<int>(g.proj.x); a.top = static_cast<int>(g.proj.y) + ProjGapPx;
            *out = blockRect(Pos::B, a, w, h, panel);
            u.mode = 1; u.reason = "ok";
            *use = u;
            return true;
        }
    }
    if (u.want == 2 && nameOk) {
        *out = besideRect(name, w, h, panel);
        u.mode = 2; u.reason = "ok";
        *use = u;
        return true;
    }
    if (nameOk) {
        Anchor a;
        a.cx = name.x + name.w / 2; a.top = name.y + name.h + ToastGapBelowName; a.nameTop = name.y; a.hasName = true;
        *out = blockRect(Pos::A, a, w, h, panel);
        u.mode = 0;
        u.reason = u.want == 0 ? "ok" : "fallback-name";   // mode 1 without a usable projection (projWhy says why)
        *use = u;
        return true;
    }
    // No name tag (the container is not the current target): follow the projected container point when it is usable, else the fixed spot
    // (PeekTopPercent of the panel), so a proximity peek of an untargeted container still shows. No panel = nothing to place against.
    if (g.proj.state == 1 && !projReject(g.proj.x, g.proj.y, g.proj.ret, panel)) {
        Anchor a;
        a.cx = static_cast<int>(g.proj.x); a.top = static_cast<int>(g.proj.y) + ProjGapPx;
        *out = blockRect(Pos::B, a, w, h, panel);
        u.mode = 1; u.reason = "noname-proj";
        *use = u;
        return true;
    }
    if (panel.w > 0 && panel.h > 0) {
        Anchor a;
        a.cx = panel.x + panel.w / 2; a.top = panel.y + panel.h * PeekTopPercent / 100;
        *out = blockRect(Pos::B, a, w, h, panel);
        u.mode = 3; u.reason = "noname-fixed";
        *use = u;
        return true;
    }
    u.reason = "no-name";
    *use = u;
    return false;
}

// Block-relative rects (add the block origin).
inline Rect headerRect(int w) { return { BlockPadX, BlockPadY, w - 2 * BlockPadX + LabelSlackPx, HeaderH }; }
// Item row i (the bordered protoitem frame): width w minus the pads, minus the scrollbar column when it shows.
inline Rect rowRect(unsigned i, int w, bool sb = false, int rowH = RowH, bool hasUnified = false) {
    return { BlockPadX, rowsTopPx(hasUnified) + static_cast<int>(i) * (rowH + RowGapPx), w - 2 * BlockPadX - (sb ? SbReserve : 0), rowH };
}
// Icon square inside a row frame, and the text label rect (vertically centred on the glyph box; align 9 = top-left).
inline Rect rowIconRect(const Rect& r, int rowH = RowH) { return { r.x + RowIconPad, r.y + RowIconPad, iconPx(rowH), iconPx(rowH) }; }
inline Rect rowTextRect(const Rect& r, int rowH = RowH) {
    const int x = r.x + RowIconPad + iconPx(rowH) + IconGapPx;
    return { x, r.y + (rowH - TextGlyphH) / 2, r.x + r.w - RowTextPadR - StackColW - x + LabelSlackPx, TextBoxH };
}
// Stack column text ("x2", align right, ends on the right padding line). The column reserves StackColW (fits "x2"; "x99" is 72 px, "x999" 97 px), but the
// RECT grows to the left to StackRectW so a wide count never wraps in its label (the name text may then meet it: only for a name at the full budget).
constexpr int StackRectW = 112;            // >= "x999" (97 px) + LabelSlackPx
inline Rect rowStackRect(const Rect& r, int rowH = RowH) {
    return { r.x + r.w - RowTextPadR - StackRectW, r.y + (rowH - TextGlyphH) / 2, StackRectW, TextBoxH };
}
// Indicator: text starts at w - BlockPadX - indPx so it ends on the right padding line.
inline Rect indicatorRect(int w, int indPx) { return { w - BlockPadX - indPx, BlockPadY, indPx + LabelSlackPx, HeaderH }; }
// Row index under the screen point (px, py) for the block at blk with vis visible rows, -1 = none (header, padding, a gap between rows, the scrollbar, outside).
inline int rowAtPoint(const Rect& blk, unsigned vis, float px, float py, bool sb = false, int rowH = RowH, bool hasUnified = false) {
    const float x0 = static_cast<float>(blk.x + BlockPadX), x1 = static_cast<float>(blk.x + blk.w - BlockPadX - (sb ? SbReserve : 0));
    const float y0 = static_cast<float>(blk.y + rowsTopPx(hasUnified));
    if (px < x0 || px >= x1 || py < y0) return -1;
    const int pitch = rowH + RowGapPx;
    const int i = static_cast<int>((py - y0) / static_cast<float>(pitch));
    const float inRow = (py - y0) - static_cast<float>(i * pitch);
    if (inRow >= static_cast<float>(rowH)) return -1;   // in the gap
    return i < static_cast<int>(vis) ? i : -1;
}
// 0.6.0-dev (QL4): the item row a left click at (px, py) lands on: only the PEEK list's rows take clicks (a Took list is read-only); -1 = none.
// The adapter makes the row frame labels click-eating under the same rule (rowsEatClicks), so a click on a row never walks the character.
inline bool rowsEatClicks(bool took, bool peekUp) { return !took && peekUp; }
inline int rowClickAt(const Rect& blk, unsigned vis, bool sb, bool took, float px, float py, int rowH = RowH, bool hasUnified = false) {
    return took ? -1 : rowAtPoint(blk, vis, px, py, sb, rowH, hasUnified);
}
inline bool pointInRect(const Rect& r, float px, float py) {
    return px >= static_cast<float>(r.x) && px < static_cast<float>(r.x + r.w) && py >= static_cast<float>(r.y) && py < static_cast<float>(r.y + r.h);
}
inline bool actionBtnHitAt(const Rect& blk, float px, float py, bool hasUnified = true) {
    if (!hasUnified) return false;
    const Rect br = actionBtnRect(blk.w);
    const Rect btn = { blk.x + br.x, blk.y + br.y, br.w, br.h };
    return pointInRect(btn, px, py);
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
    bool isSelect = false;       // QL2: true when peek was triggered by select/click, false for proximity
    bool isHover = false;        // QL3: true when peek was triggered by cursor hover (app+0x4B0)
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
        const bool wasHover = isHover;
        expect = 0; active = true; isSelect = false; isHover = wasHover; obj = o; sinceMs = now; sinceFrame = frame; closeStartMs = 0; closeTried = false; text[0] = 0;
        return true;
    }
    // QL2: start active select peek directly from ShowContainerGui
    void beginSelect(uintptr_t p, uint32_t sid, uint32_t o, uintptr_t a, uint32_t op, uint32_t now, uint32_t frame) {
        clear();
        active = true;
        isSelect = true;
        isHover = false;
        plc = p;
        serverId = sid;
        obj = o;
        area = a;
        opener = op;
        sinceMs = now;
        sinceFrame = frame;
    }
    // QL3: begin hover peek (waiting for ShowContainerGui)
    void beginHover(uintptr_t p, uint32_t sid, uintptr_t a, uint32_t op, uint32_t now) {
        clear();
        expect = p; plc = p; serverId = sid; area = a; opener = op; expectMs = now;
        isHover = true;
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
        expect = 0; expectMs = 0; serverId = 0; opener = 0; plc = 0; area = 0; openSeen = false; openFrame = 0; active = false; isSelect = false; isHover = false; obj = 0;
        sinceMs = sinceFrame = 0; closeStartMs = 0; closeTried = false; text[0] = 0;
    }
};

// ---- 0.6.0-dev (QL4): per-item take. The click takes ONE item with the engine's own server CloseInventory drain sequence (ItemListGetItem,
// RemoveFromRepo, [first-pickup branch], 0x58A2C0(1), InsertOrMerge, SetOwner) for the clicked index instead of count-1.
enum class TakeWhy { Ok = 0, NoRepo, BadIndex, Stale, NoItem, NoCtx, NoDst, Popup, Failed };
inline const char* takeWhyName(TakeWhy w) {
    switch (w) {
        case TakeWhy::Ok: return "ok";
        case TakeWhy::NoRepo: return "norepo";
        case TakeWhy::BadIndex: return "badindex";
        case TakeWhy::Stale: return "stale";
        case TakeWhy::NoItem: return "noitem";
        case TakeWhy::NoCtx: return "noctx";
        case TakeWhy::NoDst: return "nodst";
        case TakeWhy::Popup: return "popup";
        case TakeWhy::Failed: return "failed";
    }
    return "?";
}
// The drain's first-pickup branch (asm 0x5F5881..0x5F5A29) for an item whose base item byte [base+0xAC] is 0x18: on the "lockertsfinv" container it
// sets the item's local boolean 0x5E silently; elsewhere the FIRST pickup (bit clear) sets it and shows the item-description pop-up. The pop-up is a
// dozen GUI calls this mod does not replicate: such a row is refused (F, take all, runs the real drain and shows it).
enum class NoticePlan { None = 0, SetBit, Popup };
constexpr int BaseItemNoticeClass = 0x18, ItemLocalBoolBit = 0x5E;
inline NoticePlan noticePlan(int baseClassByte, bool lockerTag, bool bitSet) {
    if (baseClassByte != BaseItemNoticeClass) return NoticePlan::None;
    if (lockerTag) return NoticePlan::SetBit;
    return bitSet ? NoticePlan::None : NoticePlan::Popup;
}
// A take click is accepted: the peek is up with its open flag set, the leader is within CloseM, no suppress state, the row is a real item.
inline const char* takeRefusal(bool peekUp, int openFlag, bool worldOk, float dist, float closeM, bool suppressed, unsigned absRow, unsigned items) {
    if (!peekUp) return "nopeek";
    if (suppressed) return "suppress";
    if (openFlag != 1) return "notopen";
    if (!worldOk) return "noworld";
    if (!(dist <= closeM)) return "range";
    if (absRow >= items) return "morerow";
    return nullptr;
}

// ---- 0.6.0-dev (QL8): item tooltip beside the list; 0.7.0-dev (QL9) redesign (approved layout: scripts/quick_loot_tip_mockup.py layout()). A fixed-width
// vanilla frame: icon 72 px at the left, title (cream, dialogfont16x16 when it fits else 10x10, " xN" for stacks) and the category line right of it, then up
// to three sections each under a 2 px divider: rules (ONE multi-line label column + ONE multi-line value column), flavor (the description, 3 lines;
// 7 when there are no rules), foot ("Equipped: ..." / "In inventory: N"). QuickLootRules.h buildTipModel fills the TipModel (QuickLootLogic.h, above).
constexpr uint32_t TipDelayMs = 300;
constexpr unsigned TipMaxLines = 10;       // tipWrap's hard cap (also the per-value cap)
constexpr int TipW = 660;                  // frame width (fixed)
constexpr int TipPadX = 26, TipPadY = 20;  // inside the frame
constexpr int TipIconPx = 72, TipIconGapPx = 16;   // icon square, icon -> title text
constexpr int TipTitleH = 34, TipSubH = 28;        // title / category line label heights
constexpr int TipLineH = 34;               // pitch of the rules / flavor / foot lines: the engine's multi-line dialogfont label pitch (0.6.x body), not the mockup's 30
constexpr int TipRuleH = 2;                // divider thickness
constexpr int TipBlockGapPx = 12;          // air around the dividers and between sections
constexpr int TipGapPx = 10;               // list -> tooltip
constexpr int TipLabelColMax = 230, TipLabelColPad = 24;   // label column = min(230, widest label + 24)
constexpr unsigned TipFlavorLines = 3, TipFlavorLinesFull = 7;   // flavor line cap (full = the item has no rules: show the whole description)
constexpr unsigned TipDescCap = 1536, TipBodyCap = 1200;
constexpr unsigned TipLabelsCap = 512, TipValuesCap = 1536;
using TipData = TipModel;                  // the machine keeps this name; a model is {} (stack -1 = not stackable)
// Everything the adapter draws for the tooltip, absolute px. labels / values are ONE multi-line label each: a value that wrapped to K lines gets K-1
// extra "\n " lines in labels so the rows stay aligned. Multi-line label rects are LabelSlackPx wider than their wrap width so CGuiLabel never re-wraps.
struct TipDraw {
    Rect frame = {0, 0, 0, 0}, fill = {0, 0, 0, 0}, icon = {0, 0, 0, 0}, title = {0, 0, 0, 0}, sub = {0, 0, 0, 0};
    Rect dividers[3] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};   // 2 px tall; one per section present, top to bottom
    Rect labels = {0, 0, 0, 0}, values = {0, 0, 0, 0}, flavor = {0, 0, 0, 0}, foot = {0, 0, 0, 0};
    unsigned nDividers = 0;
    bool bigTitle = false, haveRules = false, haveFlavor = false, haveFoot = false, flipped = false, truncated = false;
    int lines = 0;                     // rule lines + flavor lines (the TIP log line)
    char titleText[NameCap + 16] = {0};
    char subText[48] = {0};
    char labelsText[TipLabelsCap] = {0};
    char valuesText[TipValuesCap] = {0};
    char flavorText[TipBodyCap] = {0};
    char footText[64] = {0};
};
inline int textPx16(const char* t) {
    const textfit::TextCtx c = { 1600.0f / 600.0f, 1.03f, 1.0f };
    return static_cast<int>(ceilf(textfit::textPx(c, t, static_cast<int>(strlen(t)), textfit::F16)));
}
// Description text for the label: '\r' dropped, '\n' kept as a line break (the TLK uses them for paragraphs; two in a row = one blank line, more collapse to two,
// none at either end), other control bytes and runs of blanks become one space (none next to a line break), bytes outside 0x20..0x7E '?'. Returns the length.
inline unsigned sanitizeDesc(char* out, unsigned cap, const char* in) {
    unsigned n = 0;
    bool pendingSpace = false;
    int pendingNl = 0;
    if (in) {
        for (unsigned i = 0; in[i] && n + 4 < cap; ++i) {
            const unsigned char c = static_cast<unsigned char>(in[i]);
            if (c == '\r') continue;
            if (c == '\n') { if (n > 0 && pendingNl < 2) ++pendingNl; pendingSpace = false; continue; }
            if (c <= 0x20) { pendingSpace = n > 0 && pendingNl == 0; continue; }
            if (pendingNl) { for (int k = 0; k < pendingNl; ++k) out[n++] = '\n'; pendingNl = 0; pendingSpace = false; }
            else if (pendingSpace) { out[n++] = ' '; pendingSpace = false; }
            out[n++] = c > 0x7E ? '?' : static_cast<char>(c);
        }
    }
    out[n] = 0;
    return n;
}
// Word wrap of text at innerPx (engine measure = textPx10) into out ('\n' between lines; a '\n' in the text is a paragraph break, an empty paragraph a blank
// line), at most maxLines (<= TipMaxLines) lines: the rest is cut and the last line ends in "...". Returns the line count; *widest = the widest line in px; *cut = the text was
// longer. (Own wrapper: TextFit.h's wrapAppendW trips -Werror=format-truncation in this test build and has no paragraph breaks.)
inline int tipWrap(char* out, unsigned cap, const char* text, int innerPx, int* widest, bool* cut, unsigned maxLines = TipMaxLines) {
    if (maxLines < 1) maxLines = 1;
    if (maxLines > TipMaxLines) maxLines = TipMaxLines;
    constexpr unsigned LineCap = 256;
    static char ln[TipMaxLines + 1][LineCap];
    unsigned nl = 0;   // index of the line being filled
    bool any = false;  // the current line holds text (or is a blank paragraph line)
    ln[0][0] = 0;
    *cut = false;
    const char* p = text ? text : "";
    bool full = false;
    auto newLine = [&]() -> bool {
        if (nl + 1 >= maxLines) { *cut = true; full = true; return false; }
        ++nl; ln[nl][0] = 0;
        return true;
    };
    while (*p && !full) {
        const char* e = strchr(p, '\n');
        const unsigned plen = e ? static_cast<unsigned>(e - p) : static_cast<unsigned>(strlen(p));
        if (any && !newLine()) break;          // each paragraph starts on a new line
        any = true;
        const char* q = p;
        const char* pend = p + plen;
        unsigned cur = 0;
        while (q < pend) {
            while (q < pend && *q == ' ') ++q;
            if (q >= pend) break;
            const char* w = q;
            while (q < pend && *q != ' ') ++q;
            unsigned wl = static_cast<unsigned>(q - w);
            if (wl > 120) wl = 120;
            char cand[LineCap + 130];
            snprintf(cand, sizeof cand, "%s%s%.*s", ln[nl], cur ? " " : "", static_cast<int>(wl), w);
            if (cur && textPx10(cand) > innerPx) {   // the word does not fit: start the next line
                if (!newLine()) break;
                cur = 0;
                snprintf(ln[nl], LineCap, "%.*s", static_cast<int>(wl), w);
            } else {
                snprintf(ln[nl], LineCap, "%.*s", static_cast<int>(strlen(cand) < LineCap - 1 ? strlen(cand) : LineCap - 1), cand);
            }
            cur = static_cast<unsigned>(strlen(ln[nl]));
        }
        if (full) break;
        p += plen + (e ? 1 : 0);
        if (!e) break;
        if (!*p) break;
    }
    // text left over after the 10th line (the loop stopped on a full table with input remaining)
    const unsigned lines = nl + 1;
    int w = 0;
    unsigned o = 0;
    out[0] = 0;
    for (unsigned i = 0; i < lines; ++i) {
        char t[LineCap + 8];
        snprintf(t, sizeof t, "%s", ln[i]);
        if (i + 1 == lines && *cut) {   // the 10th line ends in "..." (cut until it fits)
            unsigned n = static_cast<unsigned>(strlen(t));
            snprintf(t, sizeof t, "%s...", ln[i]);
            while (n > 1 && textPx10(t) > innerPx) { --n; while (n > 1 && ln[i][n - 1] == ' ') --n; snprintf(t, sizeof t, "%.*s...", static_cast<int>(n), ln[i]); }
        }
        const int lw = textPx10(t);
        if (lw > w) w = lw;
        const int put = snprintf(out + o, cap > o ? cap - o : 0, "%s%s", i ? "\n" : "", t);
        if (put < 0 || o + static_cast<unsigned>(put) >= cap) { o = cap - 1; out[o] = 0; break; }
        o += static_cast<unsigned>(put);
    }
    *widest = w;
    return static_cast<int>(lines);
}
// out = text + suffix when its textPx10 fits px, else text cut (trailing blanks trimmed) + "..." + suffix until it does (never below 1 char).
inline void tipFit10(char* out, size_t cap, const char* text, int px, const char* suffix = "") {
    char t[256];
    snprintf(t, sizeof t, "%s%s", text, suffix);
    if (textPx10(t) > px) {
        unsigned n = static_cast<unsigned>(strlen(text));
        while (n > 1) {
            --n;
            while (n > 1 && text[n - 1] == ' ') --n;
            snprintf(t, sizeof t, "%.*s...%s", static_cast<int>(n), text, suffix);
            if (textPx10(t) <= px) break;
        }
    }
    snprintf(out, cap, "%s", t);
}
// Appends s to buf (cap bytes, *len used, always NUL-terminated); what does not fit is cut.
inline void tipAppend(char* buf, size_t cap, size_t* len, const char* s) {
    if (*len + 1 >= cap) return;
    const size_t room = cap - *len - 1;
    size_t k = strlen(s);
    if (k > room) k = room;
    memcpy(buf + *len, s, k);
    *len += k;
    buf[*len] = 0;
}

// Builds the tooltip for model d beside block blk (top edge = the block's top; rowAbs, the hovered row, is not used by the 0.7.0-dev layout) inside
// panel (panel.w <= 0 = unknown): right of the list, flipped to its left when it would leave the panel, clamped to the panel margins.
// false = nothing to show (not reachable now: an empty title becomes "item").
inline bool buildTip(const TipData& d, const Rect& blk, const Rect& /*rowAbs*/, const Rect& panel, TipDraw* o, int extraPadX = 0) {
    *o = TipDraw();
    const int inner = TipW - 2 * TipPadX;
    const int headW = inner - TipIconPx - TipIconGapPx;   // title / category text width
    // title: the name (+ " xN" for stacks); 16x16 when it fits, else 10x10 cut with "..." (the stack suffix stays)
    char nm[RowNameMax + 8];
    rowName(nm, sizeof nm, d.title);
    char sfx[16] = "";
    if (d.stack >= 2) snprintf(sfx, sizeof sfx, " x%d", d.stack);
    snprintf(o->titleText, sizeof o->titleText, "%s%s", nm, sfx);
    o->bigTitle = textPx16(o->titleText) <= headW;
    if (!o->bigTitle) tipFit10(o->titleText, sizeof o->titleText, nm, headW, sfx);
    char sub[sizeof o->subText];
    stripTags(sub, sizeof sub, d.sub);
    tipFit10(o->subText, sizeof o->subText, sub, headW);
    // rules: ONE label column and ONE value column; a value that wraps to K lines adds K-1 "\n " lines to the labels so the rows stay aligned
    const unsigned nRules = d.nRules < 10 ? d.nRules : 10;
    unsigned nUsed = 0;
    int col = 0;
    for (unsigned i = 0; i < nRules; ++i) {
        if (!d.rules[i].label[0] && !d.rules[i].value[0]) continue;
        const int w = textPx10(d.rules[i].label) + TipLabelColPad;
        if (w > col) col = w;
        ++nUsed;
    }
    if (col > TipLabelColMax) col = TipLabelColMax;
    int ruleLines = 0;
    size_t nl = 0, nv = 0;
    for (unsigned i = 0; i < nRules; ++i) {
        const TipRule& r = d.rules[i];
        if (!r.label[0] && !r.value[0]) continue;
        char wrapped[512];
        int widest = 0;
        bool cut = false;
        const int k = tipWrap(wrapped, sizeof wrapped, r.value, inner - col, &widest, &cut);
        if (cut) o->truncated = true;
        if (ruleLines) { tipAppend(o->labelsText, sizeof o->labelsText, &nl, "\n"); tipAppend(o->valuesText, sizeof o->valuesText, &nv, "\n"); }
        tipAppend(o->labelsText, sizeof o->labelsText, &nl, r.label);
        for (int j = 1; j < k; ++j) tipAppend(o->labelsText, sizeof o->labelsText, &nl, "\n ");
        tipAppend(o->valuesText, sizeof o->valuesText, &nv, wrapped);
        ruleLines += k;
    }
    o->haveRules = nUsed > 0;
    // flavor: the description as one paragraph (the design wraps by words), 3 lines, 7 when the item has no rules
    char flat[sizeof d.flavor + 8];
    {
        char raw[sizeof d.flavor + 8];
        unsigned n = 0;
        for (; n < sizeof d.flavor && d.flavor[n]; ++n) raw[n] = (static_cast<unsigned char>(d.flavor[n]) < 0x20) ? ' ' : d.flavor[n];
        raw[n] = 0;
        stripTags(flat, sizeof flat, raw);
    }
    int flavLines = 0;
    if (flat[0]) {
        int widest = 0;
        bool cut = false;
        flavLines = tipWrap(o->flavorText, sizeof o->flavorText, flat, inner, &widest, &cut, d.full ? TipFlavorLinesFull : TipFlavorLines);
        if (cut) o->truncated = true;
        o->haveFlavor = flavLines > 0;
    }
    // foot
    char foot[sizeof d.foot];
    stripTags(foot, sizeof foot, d.foot);
    o->haveFoot = foot[0] != 0;
    if (o->haveFoot) tipFit10(o->footText, sizeof o->footText, foot, inner);
    o->lines = ruleLines + flavLines;

    // height: header, then per section [divider][gap][lines]; rules end in a gap always (the mockup), flavor only when a foot follows
    const int headH = TipIconPx > TipTitleH + TipSubH ? TipIconPx : TipTitleH + TipSubH;
    int h = TipPadY + headH + TipBlockGapPx;
    if (o->haveRules) h += TipRuleH + TipBlockGapPx + ruleLines * TipLineH + TipBlockGapPx;
    if (o->haveFlavor) h += TipRuleH + TipBlockGapPx + flavLines * TipLineH + (o->haveFoot ? TipBlockGapPx : 0);
    if (o->haveFoot) h += TipRuleH + TipBlockGapPx + TipLineH;
    h += TipPadY;
    const int w = TipW;
    // position: right of the list, top = the block's top; flip left near the panel edge, then clamp (as in 0.6.0-dev)
    int x = blk.x + blk.w + TipGapPx + extraPadX;
    int y = blk.y;
    if (panel.w > 0 && panel.h > 0) {
        if (x + w > panel.x + panel.w - ScreenMarginPx) { x = blk.x - TipGapPx - extraPadX - w; o->flipped = true; }
        if (x < panel.x + ScreenMarginPx) x = panel.x + ScreenMarginPx;
        const int bottom = panel.y + panel.h - ScreenMarginPx, topLimit = panel.y + ScreenMarginPx;
        if (y + h > bottom) y = bottom - h;
        if (y < topLimit) y = topLimit;
    }
    o->frame = { x, y, w, h };
    o->fill = fillRect(o->frame);
    const int ix = x + TipPadX;
    int cy = y + TipPadY;
    o->icon = { ix, cy, TipIconPx, TipIconPx };
    const int tx = ix + TipIconPx + TipIconGapPx;
    o->title = { tx, cy + 4, headW + LabelSlackPx, TipTitleH };
    o->sub = { tx, cy + 4 + TipTitleH, headW + LabelSlackPx, TipSubH };
    cy += headH + TipBlockGapPx;
    auto divider = [&]() {
        o->dividers[o->nDividers++] = { ix, cy, inner, TipRuleH };
        cy += TipRuleH + TipBlockGapPx;
    };
    if (o->haveRules) {
        divider();
        o->labels = { ix, cy, col - 8 + LabelSlackPx, ruleLines * TipLineH };
        o->values = { ix + col, cy, inner - col + LabelSlackPx, ruleLines * TipLineH };
        cy += ruleLines * TipLineH + TipBlockGapPx;
    }
    if (o->haveFlavor) {
        divider();
        o->flavor = { ix, cy, inner + LabelSlackPx, flavLines * TipLineH };
        cy += flavLines * TipLineH + (o->haveFoot ? TipBlockGapPx : 0);
    }
    if (o->haveFoot) {
        divider();
        o->foot = { ix, cy, inner + LabelSlackPx, TipLineH };
    }
    return o->titleText[0] != 0;
}

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
inline int fmtHoverOpen(char* b, size_t n, uint32_t sid, uintptr_t plc, float dist, int count) {
    return snprintf(b, n, "HOVER-OPEN obj=%08x plc=%08x dist=%.2f n=%d", sid, static_cast<unsigned>(plc), static_cast<double>(dist), count);
}
inline int fmtEmptyCursor(char* b, size_t n, uint32_t sid, bool empty) {
    return snprintf(b, n, "EMPTY-CURSOR obj=%08x empty=%d", sid, empty ? 1 : 0);
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
inline int fmtSbClick(char* b, size_t n, SbPart p, unsigned off, unsigned maxOff) {
    return snprintf(b, n, "SBCLICK part=%s off=%u/%u", sbPartName(p), off, maxOff);
}
inline int fmtWheelSub(char* b, size_t n, int token, unsigned failedTries) {
    return snprintf(b, n, "WHEEL-SUB token=%d after=%u", token, failedTries);
}
inline int fmtAnchor(char* b, size_t n, const AnchorUse& u) {
    return snprintf(b, n, "ANCHOR want=%s used=%s reason=%s proj=%s", anchorModeName(u.want), u.mode < 0 ? "none" : anchorModeName(u.mode), u.reason, u.projWhy);
}
inline int fmtProj(char* b, size_t n, bool nameOk, const Rect& name, const Proj& p, const char* why) {
    return snprintf(b, n, "PROJ name=%d,%d,%d,%d world=%.2f,%.2f,%.2f px=%.1f,%.1f ret=%d state=%d why=%s%s%s", nameOk ? name.x : 0, nameOk ? name.y : 0, nameOk ? name.w : 0, nameOk ? name.h : 0,
                    static_cast<double>(p.wx), static_cast<double>(p.wy), static_cast<double>(p.wz), static_cast<double>(p.x), static_cast<double>(p.y), p.ret, p.state, why, p.note && p.note[0] ? " note=" : "", p.note ? p.note : "");
}
inline int fmtTake1(char* b, size_t n, unsigned i, uint32_t id, const char* name, int stack, int rc, int nBefore, int nAfter, int notice) {
    char nm[ToastNameMax + 1];
    sanitizeName(nm, sizeof nm, name);
    return snprintf(b, n, "TAKE1 i=%u id=%08x name=%s stack=%d rc=%d n_before=%d n_after=%d notice=%d", i, id, nm, stack, rc, nBefore, nAfter, notice);
}
inline int fmtTake1Refused(char* b, size_t n, unsigned i, const char* why, int notice) {
    return snprintf(b, n, "TAKE1-REFUSED i=%u why=%s notice=%d", i, why, notice);
}
inline int fmtTip(char* b, size_t n, unsigned i, int lines, int ex, int ey, int ew, int eh, bool flipped) {
    return snprintf(b, n, "TIP i=%u lines=%d extent=%d,%d,%d,%d flip=%d", i, lines, ex, ey, ew, eh, flipped ? 1 : 0);
}
inline int fmtErr(char* b, size_t n, const char* what, uint32_t dtMs) { return snprintf(b, n, "ERR %s dt=%ums", what, dtMs); }

}   // namespace qlogic
