// HolsterTune.h -- ini tuning-key parser for the holstered-weapons mod (doc 42): header-only, no engine calls, no file I/O, no STL beyond IniKit.
// Host-tested by holster_tune_test.cpp. File reload and change stamps live in FileWatch.h, not here.
//
//   applyTuneKey  one "key = value" pair -> TuneCfg. Keys are case-insensitive:
//                   place      1/on/true/yes -> true, 0/off/false/no -> false
//                   place_rot  exactly 3 comma floats, each |v| <= 720 (degrees)
//                   place_off  exactly 3 comma floats, each |v| <= 3.0 (metres)
//                 Any parse failure, wrong count, range violation or unknown key returns false and leaves the cfg unchanged.
//                 A rejected vector is never half-applied.
#pragma once
#include "IniKit.h"
#include <ctype.h>

namespace hs {

struct TuneCfg {
    bool place = false;
    float rot[3] = {0.f, 0.f, 0.f};
    float off[3] = {0.f, 0.f, 0.f};
};

// ASCII case-insensitive equality (keys and bool words).
inline bool tuneEqCi(const char* a, const char* b) {
    for (; *a && *b; ++a, ++b)
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
    return *a == *b;
}

// Bool word -> out. Returns false (out untouched) for anything outside the eight accepted words.
inline bool tuneParseBool(const char* v, bool& out) {
    if (tuneEqCi(v, "1") || tuneEqCi(v, "on") || tuneEqCi(v, "true") || tuneEqCi(v, "yes")) { out = true; return true; }
    if (tuneEqCi(v, "0") || tuneEqCi(v, "off") || tuneEqCi(v, "false") || tuneEqCi(v, "no")) { out = false; return true; }
    return false;
}

// Exactly 3 finite comma floats with |v| <= limit -> dst. dst is written only when every component passes.
inline bool tuneSetVec3(const char* val, float* dst, float limit) {
    float v[3];
    if (!inikit::parseFloats(val, v, 3)) return false;
    for (int i = 0; i < 3; ++i)
        if (v[i] < -limit || v[i] > limit) return false;
    dst[0] = v[0]; dst[1] = v[1]; dst[2] = v[2];
    return true;
}

inline bool applyTuneKey(TuneCfg& c, const char* key, const char* val) {
    if (!key || !val) return false;
    if (tuneEqCi(key, "place")) {
        bool b = false;
        if (!tuneParseBool(val, b)) return false;
        c.place = b;
        return true;
    }
    if (tuneEqCi(key, "place_rot")) return tuneSetVec3(val, c.rot, 720.f);
    if (tuneEqCi(key, "place_off")) return tuneSetVec3(val, c.off, 3.f);
    return false;
}

} // namespace hs
