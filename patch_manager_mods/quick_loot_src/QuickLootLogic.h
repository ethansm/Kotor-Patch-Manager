// QuickLootLogic.h -- pure helpers of the quick-loot probe (doc 52 QL0): event filter, wrap-safe ms, item cap, the pending-container
// tracker and the log line formatters. No game memory, no Win32: host-testable (quick_loot_logic_test.cpp).
#pragma once
#include <stdint.h>
#include <stdio.h>

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

}   // namespace qlogic
