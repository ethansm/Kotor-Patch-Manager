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
//                   place_{rot,off,bone}_w_<x>  per-weapon-model override; the model name is "w_<x>" (resref, 2..16 chars [A-Za-z0-9_])
//                   place_{rot,off,bone}_l_<cat|w_x>  the same keys for the left hand. A left value that is not set mirrors the right one.
//                 Key shape after place_<kind>_: [body_<name>] [l_] [w_<x> | <cat>]. body_ is only valid for place_off.
//                 Any parse failure, wrong count, range violation, unknown key or full table returns false and leaves the cfg
//                 unchanged. A rejected vector is never half-applied. Tables: 48 model entries (left and right together), 24 bodies.
//                 isTuneKey is syntactic: it stays true for a well-formed key even when its table is full.
//   resolveFull   per weapon, per hand: model > type > global precedence, body delta, mirrored left values (see Resolved).
//                 A bone of 0 means "no table bone at any level": the caller uses defaultBone(cfg), which is not mirrored.
#pragma once
#include <stdint.h>
#include <ctype.h>
#include "IniKit.h"
#include "HolsterLogic.h"

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

// Per-body offset delta, right-hand frame (the left hand negates x).
struct BodyTune {
    char body[17] = {0};
    float off[3] = {0.f, 0.f, 0.f};
};

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
inline bool tuneParseKey(const char* key, TuneKeyRef& r) {
    if (!key) return false;
    const char* p = nullptr;
    if (tunePrefixCi(key, "place_rot_", p)) r.kind = TkRot;
    else if (tunePrefixCi(key, "place_off_", p)) r.kind = TkOff;
    else if (tunePrefixCi(key, "place_bone_", p)) r.kind = TkBone;
    else return false;
    if (r.kind == TkOff && tunePrefixCi(p, "body_", p)) {
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

// Keys this header owns (the ini reader routes them to applyTuneKey; place_gobvt is NOT one of them).
inline bool isTuneKey(const char* key) {
    if (!key) return false;
    if (tuneEqCi(key, "place") || tuneEqCi(key, "place_rot") || tuneEqCi(key, "place_off")) return true;
    TuneKeyRef r;
    return tuneParseKey(key, r);
}

// Drop every override (a reload re-reads them, so a deleted key falls back to the global value). Keeps place, place_rot, place_off and bootBoneIdx.
inline void clearCatTune(TuneCfg& c) {
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

// Resolved placement for one weapon on one hand. src[0] is the rot source, src[1] the off source, src[2] the bone source:
// 0 global, 1 category (type), 2 weapon model. src[1] has |4 when a body delta was added.
struct Resolved {
    float rot[3] = {0.f, 0.f, 0.f};
    float off[3] = {0.f, 0.f, 0.f};
    uint8_t boneIdx = 0;
    uint8_t src[3] = {0, 0, 0};
};

struct TuneLevel { const CatTune* t; bool mirror; uint8_t src; };

inline void tunePushLevel(TuneLevel* lv, int& n, const CatTune* t, bool mirror, uint8_t src) {
    if (!t) return;
    lv[n].t = t;
    lv[n].mirror = mirror;
    lv[n].src = src;
    ++n;
}

// Right hand: model > type > global. Left hand: left-model > model (mirrored) > left-type > type (mirrored) > global (mirrored).
// Each of rot, off and bone takes the first level that has it. A body delta (x negated on the left) is added to off unless excludeBody.
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
    if (rotL->mirror) mirrorRot(out.rot);
    if (offL->mirror) mirrorOff(out.off);
    out.src[0] = rotL->src;
    out.src[1] = offL->src;
    out.boneIdx = 0;
    out.src[2] = 0;
    if (boneL) {
        out.boneIdx = boneL->t->boneIdx;
        if (boneL->mirror) mirrorBone(out.boneIdx);
        out.src[2] = boneL->src;
    }
    const BodyTune* b = excludeBody ? nullptr : findBody(c, body);
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
        for (int i = 0; i < 3; ++i) b->off[i] = v[i];
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
    TuneKeyRef r;
    if (!tuneParseKey(key, r)) return false;
    return tuneApplyParsed(c, r, val);
}

} // namespace hs
