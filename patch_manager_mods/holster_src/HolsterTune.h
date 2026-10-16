// HolsterTune.h -- ini tuning-key parser for the holstered-weapons mod (doc 42): header-only, no engine calls, no file I/O, no STL beyond IniKit.
// Host-tested by holster_tune_test.cpp. File reload and change stamps live in FileWatch.h, not here.
//
//   applyTuneKey  one "key = value" pair -> TuneCfg. Keys are case-insensitive:
//                   place      1/on/true/yes -> true, 0/off/false/no -> false
//                   place_rot  exactly 3 comma floats, each |v| <= 720 (degrees)
//                   place_off  exactly 3 comma floats, each |v| <= 3.0 (metres)
//                   place_rot_<cat> / place_off_<cat>  per-category override (cat = categoryName: lightsaber, melee, qstaff,
//                              pistol, rifle, heavy); a category without an override uses place_rot / place_off
//                 Any parse failure, wrong count, range violation or unknown key returns false and leaves the cfg unchanged.
//                 A rejected vector is never half-applied.
#pragma once
#include "IniKit.h"
#include "HolsterLogic.h"
#include <ctype.h>

namespace hs {

struct CatTune {
    bool hasRot = false, hasOff = false;
    float rot[3] = {0.f, 0.f, 0.f};
    float off[3] = {0.f, 0.f, 0.f};
};

struct TuneCfg {
    bool place = false;
    float rot[3] = {0.f, 0.f, 0.f};
    float off[3] = {0.f, 0.f, 0.f};
    CatTune cat[CatCount];
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

// "place_rot_pistol" -> (isRot, CatPistol). False for anything that is not a per-category key with a known category name.
inline bool tuneCatKey(const char* key, bool& isRot, Category& cat) {
    const char* rest = nullptr;
    if (stemIs(key, "place_rot") && key[9] == '_') { isRot = true; rest = key + 10; }
    else if (stemIs(key, "place_off") && key[9] == '_') { isRot = false; rest = key + 10; }
    else return false;
    for (int i = CatUnknown + 1; i < CatCount; ++i)
        if (tuneEqCi(rest, categoryName(static_cast<Category>(i)))) { cat = static_cast<Category>(i); return true; }
    return false;
}

// Keys this header owns (the ini reader routes them to applyTuneKey; place_gobvt is NOT one of them).
inline bool isTuneKey(const char* key) {
    bool r; Category c;
    return key && (tuneEqCi(key, "place") || tuneEqCi(key, "place_rot") || tuneEqCi(key, "place_off") || tuneCatKey(key, r, c));
}

// Drop every per-category override (a reload re-reads them, so a deleted key falls back to the global value).
inline void clearCatTune(TuneCfg& c) {
    for (int i = 0; i < CatCount; ++i) c.cat[i] = CatTune();
}

// Effective rotation/offset for a category: its override when set, else the global place_rot / place_off.
inline void resolveTune(const TuneCfg& c, Category cat, float rot[3], float off[3]) {
    const CatTune* t = (cat > CatUnknown && cat < CatCount) ? &c.cat[cat] : nullptr;
    for (int i = 0; i < 3; ++i) {
        rot[i] = (t && t->hasRot) ? t->rot[i] : c.rot[i];
        off[i] = (t && t->hasOff) ? t->off[i] : c.off[i];
    }
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
    bool isRot = false;
    Category cat = CatUnknown;
    if (!tuneCatKey(key, isRot, cat)) return false;
    CatTune& t = c.cat[cat];
    if (isRot) { if (!tuneSetVec3(val, t.rot, 720.f)) return false; t.hasRot = true; }
    else { if (!tuneSetVec3(val, t.off, 3.f)) return false; t.hasOff = true; }
    return true;
}

} // namespace hs
