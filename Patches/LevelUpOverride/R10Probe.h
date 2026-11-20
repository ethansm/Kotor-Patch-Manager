// R10Probe.h -- per-panel logger state machine for the R10 (PC level-up filter) LOG-ONLY probe hooks (doc 25 section 8).
// PURE: no windows.h, no STL, no engine reads. LevelUpOverride.cpp handlers do the saferead reads and feed plain values in;
// lines leave through an EmitFn callback (the mod routes them to its logLine). Host-tested by r10_probe_test.cpp.
//
// Sites: F1 = feats grid row add (per candidate), F2 = feats choosable test (per candidate), P1/P2 = the same for powers.
// F1/P1 carry no panel pointer, F2/P2 do. A "session" is one panel build of one kind: rows (F1/P1 calls) + choosable_seen (F2/P2 calls).
// Assumed order inside one panel open: grid rows first, then the choosable build (R10_findings (a): 0x90AEA0 -> grid 0x89B600).
//   - onRow before any panel pointer is known starts a pending session (panel=0) that the first onChoosable of the same kind adopts.
//   - onRow of the same kind after that session already saw a choosable id starts a NEW session (a second panel, e.g. bonus feats).
//   - a kind switch or a different panel pointer flushes the old session first; flush() ends the active one (called when the GUI closes).
// Line budget: hard cap MaxLines per process; verbose per-id lines stop at IdLineLimit so the panel/done summaries always fit.
#pragma once
#include <stdint.h>
#include <stdio.h>

namespace r10 {

constexpr int MaxLines = 400;      // total R10 lines per process, the last one is the cap notice
constexpr int IdLineLimit = 360;   // per-id (verbose) lines only while fewer than this many lines were emitted
enum Kind { Feats = 0, Powers = 1 };

inline const char* kindName(int k) { return k == Powers ? "powers" : "feats"; }

// Values read by the handler for the first choosable call of a panel (-1 / 0 = unreadable).
struct PanelInfo {
    uint32_t panel = 0;
    int bonus = 0;                 // feats only: panel+0x1BE0 bit1
    uint32_t client = 0, server = 0;
    const char* tag = "";          // server creature Tag, "" when empty / unreadable
    int ispc = -1, stats6c = -1, lvl = -1, picks = -1;
};

typedef void (*EmitFn)(void* ctx, const char* line);

struct State {
    bool on = false;               // Config::r10probe
    bool verbose = false;          // Config::verbose
    EmitFn emit = nullptr;
    void* ctx = nullptr;
    int lines = 0;                 // R10 lines emitted so far (never reset by flush)
    bool active = false;
    int kind = Feats;
    uint32_t panel = 0;            // 0 until the first choosable call of the session
    bool panelLogged = false;
    int rows = 0, seen = 0;
};

inline void put(State& s, const char* line) {   // one line through the cap
    if (!s.emit || s.lines >= MaxLines) return;
    if (s.lines == MaxLines - 1) { ++s.lines; s.emit(s.ctx, "R10 line cap reached"); return; }
    ++s.lines;
    s.emit(s.ctx, line);
}

inline void putId(State& s, int kind, const char* site, uint32_t id) {
    if (!s.verbose || s.lines >= IdLineLimit) return;
    char b[96];
    snprintf(b, sizeof b, "R10 id kind=%s site=%s id=%u", kindName(kind), site, (unsigned)id);
    put(s, b);
}

// Ends the active session with its done line (no-op when none). Call when the level-up GUI is gone.
inline void flush(State& s) {
    if (!s.active) return;
    s.active = false;
    if (!s.on) return;
    char b[128];
    snprintf(b, sizeof b, "R10 done kind=%s panel=%08x rows=%d choosable_seen=%d", kindName(s.kind), (unsigned)s.panel, s.rows, s.seen);
    put(s, b);
}

inline void begin(State& s, int kind, uint32_t panel) {
    s.active = true; s.kind = kind; s.panel = panel; s.panelLogged = false; s.rows = 0; s.seen = 0;
}

// F1 / P1: one grid row candidate. id = feat id (F1) or spell id (P1).
inline void onRow(State& s, int kind, uint32_t id) {
    if (!s.on) return;
    if (s.active && (s.kind != kind || s.seen > 0)) flush(s);
    if (!s.active) begin(s, kind, 0);
    ++s.rows;
    putId(s, kind, kind == Powers ? "P1" : "F1", id);
}

// True when onChoosable needs a PanelInfo for (kind, panel): the handler skips the reads on every other call.
inline bool wantInfo(const State& s, int kind, uint32_t panel) {
    return s.on && !(s.active && s.kind == kind && s.panel == panel && s.panelLogged);
}

// F2 / P2: one choosable-test candidate of `panel`. info may be null only when wantInfo() was false.
inline void onChoosable(State& s, int kind, uint32_t panel, uint32_t id, const PanelInfo* info) {
    if (!s.on) return;
    if (s.active && (s.kind != kind || (s.panel != 0 && s.panel != panel))) flush(s);
    if (!s.active) begin(s, kind, panel);
    if (s.panel == 0) s.panel = panel;
    if (!s.panelLogged) {
        s.panelLogged = true;
        PanelInfo d;
        const PanelInfo& i = info ? *info : d;
        char tag[40]; int n = 0;
        for (const char* t = i.tag ? i.tag : ""; *t && n < (int)sizeof tag - 1; ++t) tag[n++] = *t == '"' ? '\'' : *t;
        tag[n] = 0;
        char b[256];
        snprintf(b, sizeof b, "R10 panel kind=%s panel=%08x bonus=%d client=%08x server=%08x tag=\"%s\" ispc=%d stats6c=%d lvl=%d picks=%d",
                 kindName(kind), (unsigned)panel, i.bonus, (unsigned)i.client, (unsigned)i.server, tag, i.ispc, i.stats6c, i.lvl, i.picks);
        put(s, b);
    }
    ++s.seen;
    putId(s, kind, kind == Powers ? "P2" : "F2", id);
}

} // namespace r10
