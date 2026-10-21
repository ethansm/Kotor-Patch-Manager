// HolsterTune.h -- ini tuning-key parser for the holstered-weapons mod (doc 42): header-only, no engine calls, no file I/O, no STL beyond IniKit.
// Host-tested by holster_tune_test.cpp. File reload and change stamps live in FileWatch.h, not here.
//
//   applyTuneKey  one "key = value" pair -> TuneCfg. Keys are case-insensitive:
//                   place      1/on/true/yes -> true, 0/off/false/no -> false
//                   place_rot  exactly 3 comma floats, each |v| <= 720 (degrees)
//                   place_off  exactly 3 comma floats, each |v| <= 3.0 (metres)
//                   place_rot_<cat> / place_off_<cat>  per-category override (cat = categoryName: lightsaber, melee, qstaff,
//                              pistol, rifle, heavy); a category without an override uses place_rot / place_off
//                   place_bone_<cat>  mount bone for that category: one of kBones (case-insensitive: torsoUpr_g, torso_g,
//                              pelvis_g, rthigh_g, lthigh_g). Any other value is rejected. The attach node itself is used when no bone is set.
//                   place_off_body_<name>  body delta added to the offset of every weapon on that body (name = 1..16 chars [A-Za-z0-9_])
//                   place_rot_body_<name>  body rotation delta in degrees (|v| <= 720), pre-multiplied onto the resolved rot (0.4.0-dev)
//                   place_{rot,off,bone}_w_<x>  per-weapon-model override; the model name is "w_<x>" (resref, 2..16 chars [A-Za-z0-9_])
//                   place_{rot,off,bone}_l_<cat|w_x>  the same keys for the left hand. A left value that is not set mirrors the right one.
//                 left_extra_rot / left_extra_rot_<cat>  one float, degrees about the bone X axis (|v| <= 720 or the key is rejected;
//                              the value USED is clamped to +-45, see kLeftExtraMax). Default -8. Applied by resolveFull ONLY when the left
//                              hand's rot is inherited from the right (mirrored) as a whole triple, after the mirror, as a quaternion
//                              (qaxis(X, ex) * qbase(mirrored rot)); never to an explicit place_rot_l_* / place_rot_l_w_* level. The
//                              per-category key (cat = categoryName) wins over the global one. 0 = plain mirror.
//                 Key shape after place_<kind>_: [body_<name>] [l_] [w_<x> | <cat>]. body_ is valid for place_off and place_rot (not place_bone).
//                 Any parse failure, wrong count, range violation, unknown key or full table returns false and leaves the cfg
//                 unchanged. A rejected vector is never half-applied. Tables: 48 model entries (left and right together), 24 bodies.
//                 isTuneKey is syntactic: it stays true for a well-formed key even when its table is full.
//   resolveFull   per weapon, per hand: model > type > global precedence, body delta, mirrored left values (see Resolved).
//                 A bone of 0 means "no table bone at any level": the caller uses defaultBone(cfg), which is not mirrored.
//   boneName      1-based table index -> bone name (boneNameOf, static storage); nullptr for 0 or out of range.
//   scopeKeys     the inverse of applyTuneKey, for one edit scope (ModelTune, Category + hand + CatTune, or BodyTune): writes
//                 KV entries for setKeys and key names for removeKeys (a field the level lacks). Pure: the caller owns the arrays
//                 and the text buffer (kScopeTextMax bytes is enough for one scope). Raw values, printed %.1f (rot) / %.3f (off).
//                 At most 3 set and 3 unset entries per call. Returns false, with both counts 0, on a bad name, a value outside the
//                 caps, or an array or buffer that is too small.
//   draw_slide_ms_<cat>   HW28 draw slide ms, 0..570, clamped at set time (SlideTune below; the key parse is not in this header yet)
//   stow_slide_ms_<cat>   HW28 stow slide ms, 0..1000, clamped at set time (same)
//   reach_grab_pct_<cat>  HW28 reach grab percent, 0..60, clamped at set time (same; percent, not the g_reachGrab fraction)
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <cmath>
#include "IniKit.h"
#include "HolsterIniWrite.h"
#include "HolsterLogic.h"
#include "HolsterSlide.h"
#include "HolsterEdit.h"

namespace hs {

enum : int { kBoneCount = 5, kMaxMdl = 48, kMaxBody = 24 };

// ASCII case-insensitive equality (keys, bool words, bone and model names).
inline bool tuneEqCi(const char* a, const char* b) {
    for (; *a && *b; ++a, ++b)
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
    return *a == *b;
}

// Mount-bone table, 1-based (0 = none). rthigh_g = 4 and lthigh_g = 5 are the two bones mirrorBone swaps.
inline const char* boneNameOf(uint8_t idx) {
    static const char* const kBones[kBoneCount] = {"torsoUpr_g", "torso_g", "pelvis_g", "rthigh_g", "lthigh_g"};
    return (idx >= 1 && idx <= kBoneCount) ? kBones[idx - 1] : nullptr;
}

// Bone name of a 1-based table index: the boneNameOf string (static storage), nullptr for 0 or out of range.
inline const char* boneName(uint8_t idx) { return boneNameOf(idx); }

// Case-insensitive table lookup: 1..kBoneCount, or 0 for a null, empty or non-table name (a custom bone_name is 0).
inline uint8_t boneIndexOf(const char* name) {
    if (!name || !*name) return 0;
    for (uint8_t i = 1; i <= kBoneCount; ++i)
        if (tuneEqCi(name, boneNameOf(i))) return i;
    return 0;
}

struct CatTune {
    bool hasRot = false, hasOff = false;
    uint8_t boneIdx = 0;   // 1-based into the bone table; 0 = no bone at this level
    float rot[3] = {0.f, 0.f, 0.f};
    float off[3] = {0.f, 0.f, 0.f};
};

// Per-weapon-model override. model is the name as written ("w_Blaster_01"), matched case-insensitively; left picks the hand.
struct ModelTune {
    char model[17] = {0};
    bool left = false;
    CatTune t;
};

// Per-body delta, right-hand frame: off (the left hand negates x) and an optional rot (place_rot_body_<name>, degrees; the left
// hand mirrors it with mirrorRot). hasRot false = no rotation delta (rot stays 0).
struct BodyTune {
    char body[17] = {0};
    float off[3] = {0.f, 0.f, 0.f};
    bool hasRot = false;
    float rot[3] = {0.f, 0.f, 0.f};
};

// left_extra_rot: default tilt (degrees about bone X) added after the mirror, and the clamp applied when it is used.
inline constexpr float kLeftExtraDefault = -8.f;
inline constexpr float kLeftExtraMax = 45.f;
inline constexpr float kLeftExtraParseMax = 720.f;

struct TuneCfg {
    bool place = false;
    uint8_t bootBoneIdx = 0;   // table index of bone_name (set by Holster.cpp); 0 = custom bone_name
    float rot[3] = {0.f, 0.f, 0.f};
    float off[3] = {0.f, 0.f, 0.f};
    CatTune cat[CatCount];
    CatTune catL[CatCount];    // left-hand per-category overrides
    ModelTune mdl[kMaxMdl];
    int nMdl = 0;
    BodyTune body[kMaxBody];
    int nBody = 0;
    float leftExtra = kLeftExtraDefault;   // left_extra_rot (raw; leftExtraFor clamps)
    float leftExtraCat[CatCount] = {};     // left_extra_rot_<cat> (raw), valid where hasLeftExtraCat
    bool hasLeftExtraCat[CatCount] = {};
};
static_assert(sizeof(TuneCfg) <= 4096, "TuneCfg is memcpy'd every frame: keep it under 4 KiB");

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

// Length of s when s is 1..maxLen chars of [A-Za-z0-9_]; 0 otherwise (including the empty string).
inline int tuneNameLen(const char* s, int maxLen) {
    int n = 0;
    for (; s[n]; ++n) {
        const char ch = s[n];
        const bool ok = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_';
        if (n >= maxLen || !ok) return 0;
    }
    return n;
}

// Copies a name of at most 16 chars into a char[17] and NUL-terminates it.
inline void tuneCopyName(char* dst, const char* src) {
    int i = 0;
    for (; i < 16 && src[i]; ++i) dst[i] = src[i];
    dst[i] = 0;
}

// Case-insensitive prefix test. On a match rest points just past the prefix.
inline bool tunePrefixCi(const char* s, const char* prefix, const char*& rest) {
    const char* q = s;
    for (; *prefix; ++prefix, ++q)
        if (!*q || tolower((unsigned char)*q) != tolower((unsigned char)*prefix)) return false;
    rest = q;
    return true;
}

enum TuneKind : int { TkRot = 0, TkOff, TkBone };

// One parsed place_<kind>_ key. name holds the body or model name as written (only one of body/model is set).
struct TuneKeyRef {
    TuneKind kind = TkRot;
    bool left = false, body = false, model = false;
    char name[17] = {0};
    Category cat = CatUnknown;   // set only for a plain category key
};

// Parses the key shape [body_<name>] [l_] [w_<x> | <cat>] after place_rot_ / place_off_ / place_bone_. False for anything else.
// body_<name> is accepted after place_rot_ and place_off_ only (place_bone_body_<name> is not a key).
inline bool tuneParseKey(const char* key, TuneKeyRef& r) {
    if (!key) return false;
    const char* p = nullptr;
    if (tunePrefixCi(key, "place_rot_", p)) r.kind = TkRot;
    else if (tunePrefixCi(key, "place_off_", p)) r.kind = TkOff;
    else if (tunePrefixCi(key, "place_bone_", p)) r.kind = TkBone;
    else return false;
    if ((r.kind == TkOff || r.kind == TkRot) && tunePrefixCi(p, "body_", p)) {
        if (tuneNameLen(p, 16) == 0) return false;
        r.body = true;
        tuneCopyName(r.name, p);
        return true;
    }
    if (tunePrefixCi(p, "l_", p)) r.left = true;
    if ((p[0] == 'w' || p[0] == 'W') && p[1] == '_') {
        if (tuneNameLen(p, 16) == 0) return false;
        r.model = true;
        tuneCopyName(r.name, p);
        return true;
    }
    for (int i = CatUnknown + 1; i < CatCount; ++i)
        if (tuneEqCi(p, categoryName(static_cast<Category>(i)))) { r.cat = static_cast<Category>(i); return true; }
    return false;
}

// Syntactic: "left_extra_rot" or "left_extra_rot_<anything>". applyTuneKey rejects a bad suffix or value, so a caller can warn.
inline bool isLeftExtraKey(const char* key) {
    const char* rest = nullptr;
    if (!key) return false;
    return tuneEqCi(key, "left_extra_rot") || tunePrefixCi(key, "left_extra_rot_", rest);
}

// Keys this header owns (the ini reader routes them to applyTuneKey; place_gobvt is NOT one of them).
inline bool isTuneKey(const char* key) {
    if (!key) return false;
    if (tuneEqCi(key, "place") || tuneEqCi(key, "place_rot") || tuneEqCi(key, "place_off")) return true;
    if (isLeftExtraKey(key)) return true;
    TuneKeyRef r;
    return tuneParseKey(key, r);
}

// Drop every override (a reload re-reads them, so a deleted key falls back to the global value). Keeps place, place_rot, place_off and bootBoneIdx.
// left_extra_rot returns to kLeftExtraDefault and the per-category extras are dropped, so a deleted left_extra_rot key also falls back.
inline void clearCatTune(TuneCfg& c) {
    c.leftExtra = kLeftExtraDefault;
    for (int i = 0; i < CatCount; ++i) { c.leftExtraCat[i] = 0.f; c.hasLeftExtraCat[i] = false; }
    for (int i = 0; i < CatCount; ++i) { c.cat[i] = CatTune(); c.catL[i] = CatTune(); }
    for (int i = 0; i < kMaxMdl; ++i) c.mdl[i] = ModelTune();
    for (int i = 0; i < kMaxBody; ++i) c.body[i] = BodyTune();
    c.nMdl = 0;
    c.nBody = 0;
}

// Effective rotation/offset for a category: its override when set, else the global place_rot / place_off.
inline void resolveTune(const TuneCfg& c, Category cat, float rot[3], float off[3]) {
    const CatTune* t = (cat > CatUnknown && cat < CatCount) ? &c.cat[cat] : nullptr;
    for (int i = 0; i < 3; ++i) {
        rot[i] = (t && t->hasRot) ? t->rot[i] : c.rot[i];
        off[i] = (t && t->hasOff) ? t->off[i] : c.off[i];
    }
}

// Mount bone name of a category (right hand), or nullptr when it has none (the weapon stays on its attach node).
inline const char* resolveBone(const TuneCfg& c, Category cat) {
    return (cat > CatUnknown && cat < CatCount) ? boneNameOf(c.cat[cat].boneIdx) : nullptr;
}

// The bone for a weapon whose resolved bone is 0: the boot bone (bone_name), 0 when bone_name is custom.
inline uint8_t defaultBone(const TuneCfg& c) { return c.bootBoneIdx; }

inline const ModelTune* findModel(const TuneCfg& c, const char* model, bool left) {
    if (!model || !*model) return nullptr;
    for (int i = 0; i < c.nMdl && i < kMaxMdl; ++i)
        if (c.mdl[i].left == left && tuneEqCi(c.mdl[i].model, model)) return &c.mdl[i];
    return nullptr;
}

inline const BodyTune* findBody(const TuneCfg& c, const char* body) {
    if (!body || !*body) return nullptr;
    for (int i = 0; i < c.nBody && i < kMaxBody; ++i)
        if (tuneEqCi(c.body[i].body, body)) return &c.body[i];
    return nullptr;
}

// Existing entry (same name and hand), else a new one; nullptr when the table is full.
inline ModelTune* findOrAddModel(TuneCfg& c, const char* name, bool left) {
    for (int i = 0; i < c.nMdl && i < kMaxMdl; ++i)
        if (c.mdl[i].left == left && tuneEqCi(c.mdl[i].model, name)) return &c.mdl[i];
    if (c.nMdl >= kMaxMdl) return nullptr;
    ModelTune& m = c.mdl[c.nMdl++];
    m = ModelTune();
    tuneCopyName(m.model, name);
    m.left = left;
    return &m;
}

inline BodyTune* findOrAddBody(TuneCfg& c, const char* name) {
    for (int i = 0; i < c.nBody && i < kMaxBody; ++i)
        if (tuneEqCi(c.body[i].body, name)) return &c.body[i];
    if (c.nBody >= kMaxBody) return nullptr;
    BodyTune& b = c.body[c.nBody++];
    b = BodyTune();
    tuneCopyName(b.body, name);
    return &b;
}

// Left-hand mirror of a right-hand value (the weapon is reflected across the body's sagittal plane).
// The rotation's x is kept and y, z are negated: qbase(rx,-ry,-rz) is the reflection (w,x,-y,-z) of qbase(rx,ry,rz).
inline void mirrorRot(float rot[3]) { rot[1] = -rot[1]; rot[2] = -rot[2]; }
inline void mirrorOff(float off[3]) { off[0] = -off[0]; }
inline void mirrorBone(uint8_t& boneIdx) {
    enum : uint8_t { kRThigh = 4, kLThigh = 5 };
    if (boneIdx == kRThigh) boneIdx = kLThigh;
    else if (boneIdx == kLThigh) boneIdx = kRThigh;
}
// Mirror of a full right-hand placement. An involution: mirroring twice gives the input back.
inline void mirrorLeft(float rot[3], float off[3], uint8_t& boneIdx) {
    mirrorRot(rot);
    mirrorOff(off);
    mirrorBone(boneIdx);
}

// The left extra tilt in degrees for a category: its own key when set, else the global one, clamped to +-kLeftExtraMax (NaN -> 0).
inline float leftExtraFor(const TuneCfg& c, Category cat) {
    const bool has = cat > CatUnknown && cat < CatCount && c.hasLeftExtraCat[cat];
    const float v = has ? c.leftExtraCat[cat] : c.leftExtra;
    if (std::isnan(v)) return 0.f;
    return v > kLeftExtraMax ? kLeftExtraMax : (v < -kLeftExtraMax ? -kLeftExtraMax : v);
}

// True when a raw left_extra_rot value is outside the clamp (the logger warns).
inline bool leftExtraClamped(float raw) { return std::isnan(raw) || std::fabs(raw) > kLeftExtraMax; }

// Resolved placement for one weapon on one hand. src[0] is the rot source, src[1] the off source, src[2] the bone source:
// 0 global, 1 category (type), 2 weapon model. src[1] has |4 when a body delta was added, src[0] has |16 when a non-zero body rotation
// delta was composed into rot.
// extraDeg is the left_extra_rot tilt that is folded into rot (0 = none: right hand, explicit left rot, or extra 0). leftMirror
// (left hand only) has bit 0 set when rot came from the mirror and bit 1 when off did: the editor save path uses extraDeg to keep
// the extra out of explicit l_ keys, and the logs print leftSrcName.
struct Resolved {
    float rot[3] = {0.f, 0.f, 0.f};
    float off[3] = {0.f, 0.f, 0.f};
    uint8_t boneIdx = 0;
    uint8_t src[3] = {0, 0, 0};
    float extraDeg = 0.f;
    uint8_t leftMirror = 0;
};

// Log name of where a left placement came from: "-" for the right hand, "mirror" (rot and off both inherited), "explicit" (neither) or "mixed".
inline const char* leftSrcName(const Resolved& r, bool left) {
    if (!left) return "-";
    const unsigned m = r.leftMirror & 3u;
    return m == 3u ? "mirror" : (m == 0u ? "explicit" : "mixed");
}

struct TuneLevel { const CatTune* t; bool mirror; uint8_t src; };

inline void tunePushLevel(TuneLevel* lv, int& n, const CatTune* t, bool mirror, uint8_t src) {
    if (!t) return;
    lv[n].t = t;
    lv[n].mirror = mirror;
    lv[n].src = src;
    ++n;
}

// Right hand: model > type > global. Left hand: left-model > model (mirrored) > left-type > type (mirrored) > global (mirrored).
// Each of rot, off and bone takes the first level that has it. A body delta is added unless excludeBody: its off (x negated on the left)
// is added to off, and its rot D (mirrorRot on the left) is pre-multiplied onto the resolved rot, q = qbase(D) * qbase(R), so D turns
// about the bone's own axes like the editor's nudges (HolsterEdit.h localAxisNudge). The left_extra_rot tilt stays outermost. With no
// body rot (or D = 0) the result is bit-identical to the offset-only path.
// model, body may be null or "". cat may be CatUnknown (global only).
inline void resolveFull(const TuneCfg& c, const char* model, Category cat, const char* body, bool left, bool excludeBody, Resolved& out) {
    CatTune global;   // the global level: rot and off always present, no bone
    global.hasRot = global.hasOff = true;
    for (int i = 0; i < 3; ++i) { global.rot[i] = c.rot[i]; global.off[i] = c.off[i]; }
    const bool validCat = cat > CatUnknown && cat < CatCount;
    const CatTune* typeR = validCat ? &c.cat[cat] : nullptr;
    const ModelTune* modelR = findModel(c, model, false);
    const CatTune* mR = modelR ? &modelR->t : nullptr;
    TuneLevel lv[5] = {};
    int n = 0;
    if (!left) {
        tunePushLevel(lv, n, mR, false, 2);
        tunePushLevel(lv, n, typeR, false, 1);
        tunePushLevel(lv, n, &global, false, 0);
    } else {
        const ModelTune* modelL = findModel(c, model, true);
        const CatTune* typeL = validCat ? &c.catL[cat] : nullptr;
        tunePushLevel(lv, n, modelL ? &modelL->t : nullptr, false, 2);
        tunePushLevel(lv, n, mR, true, 2);
        tunePushLevel(lv, n, typeL, false, 1);
        tunePushLevel(lv, n, typeR, true, 1);
        tunePushLevel(lv, n, &global, true, 0);
    }
    const TuneLevel* rotL = nullptr;
    const TuneLevel* offL = nullptr;
    const TuneLevel* boneL = nullptr;
    for (int k = 0; k < n; ++k) {
        if (!rotL && lv[k].t->hasRot) rotL = &lv[k];
        if (!offL && lv[k].t->hasOff) offL = &lv[k];
        if (!boneL && lv[k].t->boneIdx != 0) boneL = &lv[k];
    }
    // The global level is always pushed last and has rot and off, so rotL and offL are never null.
    for (int i = 0; i < 3; ++i) { out.rot[i] = rotL->t->rot[i]; out.off[i] = offL->t->off[i]; }
    out.extraDeg = 0.f;
    out.leftMirror = 0;
    if (rotL->mirror) mirrorRot(out.rot);
    if (offL->mirror) mirrorOff(out.off);
    if (left) out.leftMirror = static_cast<uint8_t>((rotL->mirror ? 1 : 0) | (offL->mirror ? 2 : 0));
    // Body rotation delta (also on an explicit left rot, as the offset delta is): q = qbase(D) * qbase(R).
    const BodyTune* b = excludeBody ? nullptr : findBody(c, body);
    bool recompose = false;
    Q q = Q{1.f, 0.f, 0.f, 0.f};
    if (b && b->hasRot && (b->rot[0] != 0.f || b->rot[1] != 0.f || b->rot[2] != 0.f)) {
        float d[3] = {b->rot[0], b->rot[1], b->rot[2]};
        if (left) mirrorRot(d);
        q = qmul(qbase(d[0], d[1], d[2]), qbase(out.rot[0], out.rot[1], out.rot[2]));
        recompose = true;
    }
    // left_extra_rot: only on a whole inherited rot triple, composed as a quaternion about the bone X axis (never added to rx).
    if (left && rotL->mirror) {
        const float ex = leftExtraFor(c, cat);
        if (ex != 0.f) {
            const float xAxis[3] = {1.f, 0.f, 0.f};
            q = qmul(qaxis(xAxis, ex), recompose ? q : qbase(out.rot[0], out.rot[1], out.rot[2]));
            recompose = true;
            out.extraDeg = ex;
        }
    }
    if (recompose) {
        float e[3];
        eulerNear(q, out.rot, e);
        for (int i = 0; i < 3; ++i) out.rot[i] = e[i];
    }
    out.src[0] = rotL->src;
    if (b && b->hasRot && (b->rot[0] != 0.f || b->rot[1] != 0.f || b->rot[2] != 0.f))
        out.src[0] = static_cast<uint8_t>(out.src[0] | 16);
    out.src[1] = offL->src;
    out.boneIdx = 0;
    out.src[2] = 0;
    if (boneL) {
        out.boneIdx = boneL->t->boneIdx;
        if (boneL->mirror) mirrorBone(out.boneIdx);
        out.src[2] = boneL->src;
    }
    if (b) {
        float d[3] = {b->off[0], b->off[1], b->off[2]};
        if (left) mirrorOff(d);
        for (int i = 0; i < 3; ++i) out.off[i] += d[i];
        out.src[1] = static_cast<uint8_t>(out.src[1] | 4);
    }
}

// Writes the value for one parsed key. The value is checked before a table slot is claimed, so a rejected value never leaves a new entry.
inline bool tuneApplyParsed(TuneCfg& c, const TuneKeyRef& r, const char* val) {
    float v[3] = {0.f, 0.f, 0.f};
    uint8_t bone = 0;
    if (r.kind == TkBone) {
        bone = boneIndexOf(val);
        if (bone == 0) return false;
    } else if (!tuneSetVec3(val, v, r.kind == TkRot ? 720.f : 3.f)) {
        return false;
    }
    if (r.body) {
        BodyTune* b = findOrAddBody(c, r.name);
        if (!b) return false;
        if (r.kind == TkRot) {
            for (int i = 0; i < 3; ++i) b->rot[i] = v[i];
            b->hasRot = true;
        } else {
            for (int i = 0; i < 3; ++i) b->off[i] = v[i];
        }
        return true;
    }
    CatTune* t = nullptr;
    if (r.model) {
        ModelTune* m = findOrAddModel(c, r.name, r.left);
        if (!m) return false;
        t = &m->t;
    } else {
        t = r.left ? &c.catL[r.cat] : &c.cat[r.cat];
    }
    if (r.kind == TkRot) { for (int i = 0; i < 3; ++i) t->rot[i] = v[i]; t->hasRot = true; }
    else if (r.kind == TkOff) { for (int i = 0; i < 3; ++i) t->off[i] = v[i]; t->hasOff = true; }
    else t->boneIdx = bone;
    return true;
}

// left_extra_rot[_<cat>] = one finite float with |v| <= kLeftExtraParseMax. Anything else (bad suffix, junk, range) leaves c unchanged.
inline bool tuneApplyLeftExtra(TuneCfg& c, const char* key, const char* val) {
    float v = 0.f;
    if (!inikit::parseFloats(val, &v, 1) || std::fabs(v) > kLeftExtraParseMax) return false;
    if (tuneEqCi(key, "left_extra_rot")) { c.leftExtra = v; return true; }
    const char* rest = nullptr;
    if (!tunePrefixCi(key, "left_extra_rot_", rest)) return false;
    for (int i = CatUnknown + 1; i < CatCount; ++i)
        if (tuneEqCi(rest, categoryName(static_cast<Category>(i)))) { c.leftExtraCat[i] = v; c.hasLeftExtraCat[i] = true; return true; }
    return false;
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
    if (isLeftExtraKey(key)) return tuneApplyLeftExtra(c, key, val);
    TuneKeyRef r;
    if (!tuneParseKey(key, r)) return false;
    return tuneApplyParsed(c, r, val);
}

// ---- scopeKeys: one edit scope -> holster.ini keys (the inverse of applyTuneKey) ----

// Text bytes one scope needs at most: 3 keys (29-30 bytes each) and the rot and off values (21 bytes each), bone names are static.
enum : int { kScopeTextMax = 160 };

// Caller-owned output of scopeKeys. Every call resets nSet, nUnset and bufUsed first.
//   set / nSet      setKeys entries; key points into buf, val into buf or at boneName's static string
//   unset / nUnset  key names for removeKeys; they point into buf
//   buf / bufUsed   key and value text, NUL-terminated strings packed from offset 0
struct ScopeOut {
    KV* set;
    int setCap;
    int nSet;
    const char** unset;
    int unsetCap;
    int nUnset;
    char* buf;
    size_t bufCap;
    size_t bufUsed;
};

inline bool scopeFail(ScopeOut& o) {
    o.nSet = 0;
    o.nUnset = 0;
    return false;
}

// Appends "place_<kind>_<tail>" to o.buf. Returns its start, or nullptr when it does not fit (nothing is written past bufCap).
inline const char* scopeKey(ScopeOut& o, const char* kind, const char* tail) {
    const size_t room = o.bufCap - o.bufUsed;
    const int n = snprintf(o.buf + o.bufUsed, room, "place_%s_%s", kind, tail);
    if (n < 0 || static_cast<size_t>(n) >= room) return nullptr;
    const char* s = o.buf + o.bufUsed;
    o.bufUsed += static_cast<size_t>(n) + 1;
    return s;
}

// Appends "a,b,c" to o.buf: rot as %.1f, off as %.3f. Each value must be finite with |v| <= the cap applyTuneKey accepts.
// A value that rounds to zero prints as 0, never -0. Returns its start, or nullptr on a bad value or a full buffer.
inline const char* scopeVec(ScopeOut& o, const float* v, bool rot) {
    const float limit = rot ? 720.f : 3.f;
    for (int i = 0; i < 3; ++i)
        if (!std::isfinite(v[i]) || std::fabs(v[i]) > limit) return nullptr;
    const float half = rot ? 0.05f : 0.0005f;   // half a unit of the last printed digit
    float x[3];
    for (int i = 0; i < 3; ++i) x[i] = std::fabs(v[i]) < half ? 0.f : v[i];
    const size_t room = o.bufCap - o.bufUsed;
    const int n = rot ? snprintf(o.buf + o.bufUsed, room, "%.1f,%.1f,%.1f", x[0], x[1], x[2])
                      : snprintf(o.buf + o.bufUsed, room, "%.3f,%.3f,%.3f", x[0], x[1], x[2]);
    if (n < 0 || static_cast<size_t>(n) >= room) return nullptr;
    const char* s = o.buf + o.bufUsed;
    o.bufUsed += static_cast<size_t>(n) + 1;
    return s;
}

inline bool scopeSet(ScopeOut& o, const char* key, const char* val) {
    if (!o.set || o.nSet >= o.setCap) return false;
    o.set[o.nSet++] = KV{key, val};
    return true;
}

inline bool scopeUnset(ScopeOut& o, const char* key) {
    if (!o.unset || o.nUnset >= o.unsetCap) return false;
    o.unset[o.nUnset++] = key;
    return true;
}

// True when "place_<kind>_<tail>" parses with tuneParseKey, so the serialized key is one applyTuneKey accepts.
inline bool scopeParses(const char* kind, const char* tail, TuneKeyRef& r) {
    char probe[64];
    const int n = snprintf(probe, sizeof probe, "place_%s_%s", kind, tail);
    return n > 0 && static_cast<size_t>(n) < sizeof probe && tuneParseKey(probe, r);
}

// One weapon or type level. tail is "[l_]<name>", the key text after place_<kind>_. Each field the level has goes to set;
// each field it lacks (no rot, no off, or bone 0) goes to unset, so setKeys and removeKeys together mirror the level.
inline bool scopeLevel(ScopeOut& o, const char* tail, const CatTune& t) {
    const char* key = scopeKey(o, "rot", tail);
    if (!key) return false;
    if (t.hasRot) {
        const char* val = scopeVec(o, t.rot, true);
        if (!val || !scopeSet(o, key, val)) return false;
    } else if (!scopeUnset(o, key)) {
        return false;
    }
    key = scopeKey(o, "off", tail);
    if (!key) return false;
    if (t.hasOff) {
        const char* val = scopeVec(o, t.off, false);
        if (!val || !scopeSet(o, key, val)) return false;
    } else if (!scopeUnset(o, key)) {
        return false;
    }
    key = scopeKey(o, "bone", tail);
    if (!key) return false;
    if (const char* bone = boneName(t.boneIdx)) return scopeSet(o, key, bone);
    return scopeUnset(o, key);
}

// Weapon override (ModelTune): key tail [l_]w_<x>. The name must be 1..16 chars of [A-Za-z0-9_] and start with "w_".
inline bool scopeKeys(const ModelTune& m, ScopeOut& o) {
    o.nSet = 0;
    o.nUnset = 0;
    o.bufUsed = 0;
    if (tuneNameLen(m.model, 16) == 0) return false;
    char tail[24];
    snprintf(tail, sizeof tail, "%s%s", m.left ? "l_" : "", m.model);
    TuneKeyRef r;
    if (!scopeParses("rot", tail, r) || !r.model || r.left != m.left) return false;
    if (!scopeLevel(o, tail, m.t)) return scopeFail(o);
    return true;
}

// Category override (CatTune of cat[cat], or catL[cat] when left): key tail [l_]<categoryName>. cat must be a real category.
inline bool scopeKeys(Category cat, bool left, const CatTune& t, ScopeOut& o) {
    o.nSet = 0;
    o.nUnset = 0;
    o.bufUsed = 0;
    if (cat <= CatUnknown || cat >= CatCount) return false;
    char tail[24];
    snprintf(tail, sizeof tail, "%s%s", left ? "l_" : "", categoryName(cat));
    TuneKeyRef r;
    if (!scopeParses("rot", tail, r) || r.model || r.body || r.cat != cat || r.left != left) return false;
    if (!scopeLevel(o, tail, t)) return scopeFail(o);
    return true;
}

// Body delta (BodyTune): always one set entry place_off_body_<name> (a body entry always has an off), plus place_rot_body_<name>:
// set when hasRot, else unset (so a body without a rotation delta removes a stale key).
inline bool scopeKeys(const BodyTune& b, ScopeOut& o) {
    o.nSet = 0;
    o.nUnset = 0;
    o.bufUsed = 0;
    if (tuneNameLen(b.body, 16) == 0) return false;
    char tail[24];
    snprintf(tail, sizeof tail, "body_%s", b.body);
    TuneKeyRef r;
    if (!scopeParses("off", tail, r) || !r.body) return false;
    const char* key = scopeKey(o, "off", tail);
    if (!key) return scopeFail(o);
    const char* val = scopeVec(o, b.off, false);
    if (!val || !scopeSet(o, key, val)) return scopeFail(o);
    if (!scopeParses("rot", tail, r) || !r.body) return scopeFail(o);
    key = scopeKey(o, "rot", tail);
    if (!key) return scopeFail(o);
    if (b.hasRot) {
        val = scopeVec(o, b.rot, true);
        if (!val || !scopeSet(o, key, val)) return scopeFail(o);
    } else if (!scopeUnset(o, key)) {
        return scopeFail(o);
    }
    return true;
}

// HW28 slide timing: per-category draw/stow ms and reach grab percent. Kept out of TuneCfg (memcpy'd every frame).
// Index 0 (CatUnknown) holds the global key; indexes 1..6 hold the categories. A category value beats the global, which beats
// slideDefault. Every set call, clamped or not, sets the has-flag and stores the clamped value, so a stored 0 is a real value.
struct SlideTune {
    bool hasDraw[CatCount] = {};
    bool hasStow[CatCount] = {};
    bool hasGrab[CatCount] = {};
    uint32_t draw[CatCount] = {};   // ms
    uint32_t stow[CatCount] = {};   // ms
    uint32_t grab[CatCount] = {};   // percent (0..60)
};

// Array index for a category: a real category maps to itself; anything outside (CatUnknown, CatCount) maps to CatUnknown (the global).
inline int slideIdx(Category cat) {
    return (cat > CatUnknown && cat < CatCount) ? static_cast<int>(cat) : static_cast<int>(CatUnknown);
}

inline constexpr uint32_t kSlideGrabPctMax = 60;   // reach_grab_pct_<cat> range, inclusive

// Setters: each call stores the clamped value, sets the has-flag, and returns true when raw was out of range (the caller warns).
inline bool slideSetDraw(SlideTune& s, Category cat, long raw) {
    const int i = slideIdx(cat);
    const uint32_t stored = clampDrawSlideMs(raw);
    s.draw[i] = stored;
    s.hasDraw[i] = true;
    return static_cast<long>(stored) != raw;
}

inline bool slideSetStow(SlideTune& s, Category cat, long raw) {
    const int i = slideIdx(cat);
    const uint32_t stored = clampStowSlideMs(raw);
    s.stow[i] = stored;
    s.hasStow[i] = true;
    return static_cast<long>(stored) != raw;
}

inline bool slideSetGrab(SlideTune& s, Category cat, long raw) {
    const int i = slideIdx(cat);
    const uint32_t stored = raw < 0 ? 0u : (raw > static_cast<long>(kSlideGrabPctMax) ? kSlideGrabPctMax : static_cast<uint32_t>(raw));
    s.grab[i] = stored;
    s.hasGrab[i] = true;
    return static_cast<long>(stored) != raw;
}

struct SlideRes { uint32_t drawMs, stowMs, grabPct; };

// Built-in timing, rows in Category enum order (CatUnknown, Lightsaber, Melee, QStaff, Pistol, Rifle, Heavy).
// CatUnknown keeps the pre-HW28 globals (350/350/30). A cat outside (CatUnknown, CatCount) maps to the CatUnknown row.
inline SlideRes slideDefault(Category cat) {
    static const SlideRes kRows[CatCount] = {
        {350, 350, 30},   // CatUnknown
        {350, 350, 30},   // CatLightsaber
        {520, 600, 45},   // CatMelee
        {570, 700, 45},   // CatQStaff
        {300, 350, 30},   // CatPistol
        {520, 600, 45},   // CatRifle
        {570, 700, 45},   // CatHeavy
    };
    return kRows[slideIdx(cat)];
}

// Per field: the category's own value when its has-flag is set, else the global (index 0) when set, else slideDefault(cat). Never clamps.
inline uint32_t slideDrawMsFor(const SlideTune& s, Category cat) {
    const int i = slideIdx(cat);
    if (s.hasDraw[i]) return s.draw[i];
    if (s.hasDraw[CatUnknown]) return s.draw[CatUnknown];
    return slideDefault(cat).drawMs;
}

inline uint32_t slideStowMsFor(const SlideTune& s, Category cat) {
    const int i = slideIdx(cat);
    if (s.hasStow[i]) return s.stow[i];
    if (s.hasStow[CatUnknown]) return s.stow[CatUnknown];
    return slideDefault(cat).stowMs;
}

inline uint32_t grabPctFor(const SlideTune& s, Category cat) {
    const int i = slideIdx(cat);
    if (s.hasGrab[i]) return s.grab[i];
    if (s.hasGrab[CatUnknown]) return s.grab[CatUnknown];
    return slideDefault(cat).grabPct;
}

inline SlideRes slideResolve(const SlideTune& s, Category cat) {
    SlideRes r;
    r.drawMs = slideDrawMsFor(s, cat);
    r.stowMs = slideStowMsFor(s, cat);
    r.grabPct = grabPctFor(s, cat);
    return r;
}

} // namespace hs
