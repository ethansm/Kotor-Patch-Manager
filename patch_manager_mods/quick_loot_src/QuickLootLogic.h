// QuickLootLogic.h -- pure helpers of quick-loot (doc 52 QL0/QL1a/QL1b): event filter, wrap-safe ms, item cap, the pending-container
// tracker, the take-all decision functions (shouldConsume / readyToSend / giveUp), the pending-take state, the "Took:" toast text / timing /
// placement (QL1b) and the log line formatters. No game memory, no Win32: host-testable (quick_loot_logic_test.cpp).
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

enum class Reason { Consume = 0, Disabled, NoHud, Flag, Shift, NoPlc, Busy };

// Consume (skip the vanilla container menu) only when every condition holds. First failing condition wins, in this order.
inline Reason shouldConsume(bool enabled, bool hudOk, int flag, bool shift, bool plcOk, bool busy) {
    if (!enabled) return Reason::Disabled;
    if (!hudOk) return Reason::NoHud;
    if (flag != 1) return Reason::Flag;
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
        case Reason::Flag: return "flag";
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

// "Took: A, B x2, C": one entry per item (name, then " xN" for 2 <= N <= 999). When the line would pass ToastMaxChars the entries that
// fit are kept and ", +K more" is appended (K = items left out + `extra`, items beyond the captured ones). count 0 -> out = "" and 0
// (no toast). names may hold null pointers; stacks may hold -1 (unknown). The result is cut to cap-1; returns its length.
inline unsigned fmtTook(char* out, size_t cap, const char* const* names, const int* stacks, unsigned count, unsigned extra = 0) {
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
    const unsigned prefix = 6;   // "Took: "
    unsigned total = prefix;
    for (unsigned i = 0; i < count; ++i) total += len[i] + (i ? 2u : 0u);
    unsigned shown = count;
    unsigned more = extra;
    if (!(extra == 0 && total <= ToastMaxChars)) {
        shown = 0;
        unsigned used = prefix;
        for (unsigned i = 0; i < count; ++i) {
            const unsigned add = len[i] + (i ? 2u : 0u);
            const unsigned after = count - i - 1 + extra;   // items still left out if this one is kept
            const unsigned suffix = after ? static_cast<unsigned>(snprintf(nullptr, 0, ", +%u more", after)) : 0u;
            if (used + add + suffix > ToastMaxChars) break;
            used += add;
            shown = i + 1;
        }
        if (shown == 0) shown = 1;   // cannot happen with the caps above; never emit an empty list
        more = count - shown + extra;
    }
    char buf[ToastMaxChars + 48];
    unsigned pos = static_cast<unsigned>(snprintf(buf, sizeof buf, "Took: "));
    for (unsigned i = 0; i < shown; ++i) pos += static_cast<unsigned>(snprintf(buf + pos, sizeof buf - pos, "%s%s", i ? ", " : "", ent[i]));
    if (more) pos += static_cast<unsigned>(snprintf(buf + pos, sizeof buf - pos, ", +%u more", more));
    if (pos >= cap) pos = static_cast<unsigned>(cap - 1);
    memcpy(out, buf, pos);
    out[pos] = 0;
    return pos;
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
inline int fmtErr(char* b, size_t n, const char* what, uint32_t dtMs) { return snprintf(b, n, "ERR %s dt=%ums", what, dtMs); }

}   // namespace qlogic
