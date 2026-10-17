// CombatTrace.h -- pure change-gated combat/pause/menu edge line shared by the mods ("CSTATE combat=0>1 paused=?>0 menu=0>0").
// Each field is 0, 1, or -1 (unknown to this mod, printed '?'). Formatters are bounded: always NUL-terminated, return the length written, cap<=0 returns 0.
// Pure: no engine dependency. Game-side readers live in CombatTraceRead.h; pause comes from actq::playerPaused (ActQueue.h).
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace ctrace {

struct CombatState { int8_t combat = -1, paused = -1, menu = -1; };   // -1 = unknown to this mod

// 0 -> '0', 1 -> '1', anything else -> '?'.
inline char glyph(int8_t v) { return v == 0 ? '0' : (v == 1 ? '1' : '?'); }

// True when any field differs; copies now into prev.
inline bool combatStateStep(CombatState& prev, const CombatState& now) {
    bool changed = prev.combat != now.combat || prev.paused != now.paused || prev.menu != now.menu;
    prev = now;
    return changed;
}

// Bounded snprintf wrapper: returns the length actually written (clamped to cap-1), 0 when cap<=0.
inline int fmtCap(char* out, int cap, int n) {
    if (cap <= 0) return 0;
    if (n < 0) { out[0] = 0; return 0; }
    return n >= cap ? cap - 1 : n;
}

// "CSTATE combat=<a>><b> paused=<a>><b> menu=<a>><b>" with a/b in 0|1|?.
inline int fmtCState(char* out, int cap, const CombatState& prev, const CombatState& now) {
    if (!out || cap <= 0) return 0;
    int n = snprintf(out, (size_t)cap, "CSTATE combat=%c>%c paused=%c>%c menu=%c>%c",
                     glyph(prev.combat), glyph(now.combat), glyph(prev.paused), glyph(now.paused), glyph(prev.menu), glyph(now.menu));
    return fmtCap(out, cap, n);
}

// " [c=<0|1|?> p=<0|1|?> m=<0|1|?>]" suffix for VIS lines.
inline int fmtCtx(char* out, int cap, const CombatState& s) {
    if (!out || cap <= 0) return 0;
    int n = snprintf(out, (size_t)cap, " [c=%c p=%c m=%c]", glyph(s.combat), glyph(s.paused), glyph(s.menu));
    return fmtCap(out, cap, n);
}

// Per-mod tracker: change-gated and capped; sink receives each formatted line.
struct Tracker { CombatState prev; bool primed = false; int lines = 0; int cap = 500; };

// First call emits (prev all '?'); later calls only on change; at cap emits "CSTATE cap reached" once, then stays silent (prev still updated).
inline void tick(Tracker& t, const CombatState& now, void (*sink)(const char* line)) {
    CombatState old = t.prev;   // all '?' until primed, so the first line reads "?>x"
    bool emit = !t.primed;
    t.primed = true;
    if (combatStateStep(t.prev, now)) emit = true;
    if (!emit || !sink || t.lines > t.cap) return;
    char line[96];
    if (t.lines == t.cap) sink("CSTATE cap reached");
    else fmtCState(line, (int)sizeof line, old, now), sink(line);
    t.lines++;
}

}  // namespace ctrace
