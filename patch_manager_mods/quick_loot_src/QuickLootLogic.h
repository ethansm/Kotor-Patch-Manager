// QuickLootLogic.h -- pure helpers of quick-loot (doc 52 QL0/QL1a/QL1b/QL2p): event filter, wrap-safe ms, item cap, the pending-container
// tracker, the take-all decision functions (shouldConsume / readyToSend / giveUp / closeFlagFor), the pending-take state, the "Took:" toast
// text / timing / placement (QL1b), the QL2p proximity peek (candidate filter, nearest pick, cooldown / hysteresis, leave decision, suppress
// transitions, key edge, peek state, peek label) and the log line formatters. No game memory, no Win32: host-testable (quick_loot_logic_test.cpp).
#pragma once
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

// ---- QL1b: the "Took: ..." toast under the target name tag
constexpr unsigned ToastMaxChars = 80;     // the whole toast line, "+K more" included
constexpr unsigned ToastNameMax = 40;      // one item name is cut at this many chars
constexpr unsigned ToastMaxItems = 16;     // names captured per take; further items only count towards "+K more"
constexpr uint32_t ToastMs = 3000;         // how long the line stays up
constexpr int ToastW = 420, ToastH = 20;   // label rect size (px)
constexpr int ToastGapBelowName = 20;      // label top = name bottom + this (clears the health bar under the name)

// True while the toast is up: strictly less than durMs since it was shown. Wrap-safe (32-bit ms counter).
inline bool toastVisible(uint32_t nowMs, uint32_t shownAtMs, uint32_t durMs) { return msSince(nowMs, shownAtMs) < durMs; }

struct Rect { int x, y, w, h; };
inline bool operator==(const Rect& a, const Rect& b) { return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h; }
inline bool operator!=(const Rect& a, const Rect& b) { return !(a == b); }

// Toast label rect: centred on the target name label (menu origin + name rect, all px), top = name bottom + gap.
inline Rect toastRect(int menuX, int menuY, int nameX, int nameY, int nameW, int nameH) {
    return { menuX + nameX + nameW / 2 - ToastW / 2, menuY + nameY + nameH + ToastGapBelowName, ToastW, ToastH };
}

// Copies an item name into out (cap >= 2): stops at the first NUL, cuts at ToastNameMax, replaces every byte outside 0x20..0x7E with '?',
// trims trailing spaces. A null/empty result becomes "item". Returns the length.
inline unsigned sanitizeName(char* out, unsigned cap, const char* in) {
    unsigned n = 0;
    const unsigned lim = cap - 1 < ToastNameMax ? cap - 1 : ToastNameMax;
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

namespace detail {
// "<prefix>A, B x2, C": one entry per item (name, then " xN" for 2 <= N <= 999). When the line would pass maxChars the entries that fit
// are kept and ", +K more" is appended (K = items left out + `extra`, items beyond the captured ones). count 0 -> out = "" and 0
// (no line). names may hold null pointers; stacks may hold -1 (unknown). The result is cut to cap-1; returns its length.
inline unsigned fmtList(char* out, size_t cap, const char* prefixText, unsigned maxChars, const char* const* names, const int* stacks,
                        unsigned count, unsigned extra) {
    if (!out || cap == 0) return 0;
    out[0] = 0;
    if (count == 0) return 0;
    if (count > ToastMaxItems) { extra += count - ToastMaxItems; count = ToastMaxItems; }
    char ent[ToastMaxItems][ToastNameMax + 16];
    unsigned len[ToastMaxItems];
    for (unsigned i = 0; i < count; ++i) {
        char nm[ToastNameMax + 1];
        unsigned n = sanitizeName(nm, sizeof nm, names ? names[i] : nullptr);
        memcpy(ent[i], nm, n);
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

// "Took: A, B x2, C" (80-char cap). See detail::fmtList.
inline unsigned fmtTook(char* out, size_t cap, const char* const* names, const int* stacks, unsigned count, unsigned extra = 0) {
    return detail::fmtList(out, cap, "Took: ", ToastMaxChars, names, stacks, count, extra);
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
constexpr unsigned PeekMaxChars = 100;        // the whole peek line
constexpr int PeekW = 760;                    // peek label width (px); wider than the toast
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

// "3 items  [F] take all: Medpac x2, Blaster Pistol, +1 more" (100-char cap). total = real repository count, count = names captured.
// total 0 / count 0 -> "" and 0 (no label).
inline unsigned fmtPeek(char* out, size_t cap, const char* key, int total, const char* const* names, const int* stacks, unsigned count) {
    if (!out || cap == 0) return 0;
    out[0] = 0;
    if (total <= 0 || count == 0) return 0;
    char pre[48];
    snprintf(pre, sizeof pre, "%d item%s  [%s] take all: ", total, total == 1 ? "" : "s", key ? key : "key");
    const unsigned extra = static_cast<unsigned>(total) > count ? static_cast<unsigned>(total) - count : 0u;
    return detail::fmtList(out, cap, pre, PeekMaxChars, names, stacks, count, extra);
}

// Peek label rects. Name-anchored: same placement rule as the toast (centred under the target name label), wider box.
inline Rect peekRectName(int nameX, int nameY, int nameW, int nameH) {
    return { nameX + nameW / 2 - PeekW / 2, nameY + nameH + ToastGapBelowName, PeekW, ToastH };
}
// Fixed spot: horizontally centred on the HUD panel, PeekTopPercent % down its height.
inline Rect peekRectFixed(int px, int py, int pw, int ph) {
    return { px + pw / 2 - PeekW / 2, py + (ph * PeekTopPercent) / 100, PeekW, ToastH };
}

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
inline int fmtErr(char* b, size_t n, const char* what, uint32_t dtMs) { return snprintf(b, n, "ERR %s dt=%ums", what, dtMs); }

}   // namespace qlogic
