// HolsterToggle.h -- pure logic of the holster toggle key (draw / stow on a key press; doc 42; no Win32, host-testable; tests in holster_toggle_test.cpp).
//   KeyEdge          rising-edge detector: update(down) is true only on the up -> down transition (a held key fires once, release re-arms)
//   ToggleAct        what a key press does: None / Draw / Stow / Refused (hostile nearby)
//   ToggleIn         inputs of the decision (all gathered by the caller; see field comments)
//   toggleDecide     press -> action. Stow caller clears the hold and, if clientCombat != 0, writes server +0x524 = 0
//   clampKey         virtual-key code from the ini: 1..0xFE kept, anything else 0 (off)
//   parseKey         "119" | "0x77" -> key code 0..0xFE (0 = off); rejects empty, trailing garbage, out of range
//   clampDrawHold    draw-hold length in ms: 0 stays 0 (until the next press), else 1000..60000
// Failure mode rule: anything unknown (not ready, combat flag unreadable) does nothing; no write happens on an unknown state.
#pragma once
#include <stdint.h>

namespace hs {

struct KeyEdge {
    bool prev = false;
    bool update(bool down) { bool r = down && !prev; prev = down; return r; }
};

enum ToggleAct : int { ToggleNone = 0, ToggleDraw = 1, ToggleStow = 2, ToggleRefused = 3 };

struct ToggleIn {
    bool ready;            // controlled creature and party table valid
    bool holdOn;           // holdActive for the controlled owner
    uint32_t clientCombat; // client +0x138
    bool combatKnown;      // client flag readable
    bool hostileNear;      // fresh hostile check result
};

// Not drawn (no hold, client combat flag 0) -> Draw. Drawn: hostile near -> Refused (stays drawn), else Stow. Unknown -> None.
inline ToggleAct toggleDecide(const ToggleIn& in) {
    if (!in.ready || !in.combatKnown) return ToggleNone;
    const bool drawn = in.holdOn || in.clientCombat != 0;
    if (!drawn) return ToggleDraw;
    if (in.hostileNear) return ToggleRefused;
    return ToggleStow;
}

inline int clampKey(long v) { return (v < 1 || v > 0xFE) ? 0 : static_cast<int>(v); }

// Decimal or 0x/0X hex, digits only (no sign, no spaces, no suffix). 0 is allowed and means off. Range 0..0xFE. On failure `out` is untouched.
inline bool parseKey(const char* s, int* out) {
    if (!s || !out) return false;
    unsigned base = 10;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) { base = 16; s += 2; }
    if (!*s) return false;
    long v = 0;
    for (; *s; ++s) {
        unsigned d;
        if (*s >= '0' && *s <= '9') d = static_cast<unsigned>(*s - '0');
        else if (base == 16 && *s >= 'a' && *s <= 'f') d = static_cast<unsigned>(*s - 'a') + 10u;
        else if (base == 16 && *s >= 'A' && *s <= 'F') d = static_cast<unsigned>(*s - 'A') + 10u;
        else return false;
        v = v * static_cast<long>(base) + static_cast<long>(d);
        if (v > 0xFE) return false;   // also bounds the accumulator, no overflow on long digit strings
    }
    *out = static_cast<int>(v);
    return true;
}

inline uint32_t clampDrawHold(long ms) {
    if (ms == 0) return 0;
    if (ms < 1000) return 1000;
    if (ms > 60000) return 60000;
    return static_cast<uint32_t>(ms);
}

}  // namespace hs
