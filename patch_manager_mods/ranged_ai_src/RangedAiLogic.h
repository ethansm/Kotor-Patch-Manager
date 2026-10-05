// RangedAiLogic.h -- the pure (host-testable, no <windows.h>) helpers of ranged-ai: the stance mask parser and the
// E-H1/E-H3 weapon range test. Same semantics as cai::parseMask / cai::inWeaponRange in
// companion_ai_src/CompanionAiLogic.h (KPM builds each patch dir alone, so this is a copy; rai_logic_test.cpp is the
// drift guard). Host tests: rai_logic_test.cpp. Used by RangedAI.cpp (one translation unit).
#pragma once

namespace rai {

// "11,12,16" -> bit mask (values 0..31). F27: null-safe, and a digit run is capped at 1000 (no int overflow; such a
// value is >= 32 and ignored).
inline unsigned parseMask(const char* s) {
    unsigned m = 0;
    for (const char* p = s; p && *p;) {
        if (*p >= '0' && *p <= '9') { int v = 0; while (*p >= '0' && *p <= '9') { v = v * 10 + (*p++ - '0'); if (v > 1000) v = 1000; } if (v < 32) m |= 1u << v; }
        else ++p;
    }
    return m;
}

// E-H3 range test: same formula as E-H1's inline test (a zero/garbage weapon range never counts as in range)
inline bool inWeaponRange(float maxR, float dist2) {
    float lim = maxR + 0.1f; lim *= lim;
    return maxR > 1.0f && dist2 <= lim;
}

}  // namespace rai
