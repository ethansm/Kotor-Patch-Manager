// HolsterPreview.h -- pure state of the holstered-weapons editor preview (doc 42, HE2): the preview Gob registry and the edit
// overlay (pending values, scope, Revert seed). Header-only, no windows.h, no engine calls, no engine types. Gobs, parents and
// nodes are opaque void* owned by the caller. Host-tested by holster_preview_test.cpp.
//
// Registry  previewRegister / previewUnregister / previewSweep. At most kPreviewMax (8) entries. The sweep drops an entry whose
//           parent or node changed, or whose gob is gone.
// Overlay   overlayBegin seeds from resolveFull (body delta included). Nudges and overlaySetValues edit the shown values only.
//           overlayWrite stores them into a TuneCfg for the current scope. Angles are degrees. Off is metres.
//           Body scope edits only the body delta (rot and bone are read-only there). Other scopes edit the level value.
//           Stored components are capped: rot |v| <= 720, off |v| <= 3 m. The shown absolute off is the stored level plus
//           the body part, so it can differ from typed input when a cap applies.
#pragma once
#include <cmath>
#include <stdint.h>
#include "HolsterEdit.h"
#include "HolsterTune.h"

namespace hs {

// ---- Preview registry ----------------------------------------------------------------------------------------------------

enum : int { kPreviewMax = 8 };

struct PreviewEntry {
    void* gob = nullptr;
    void* parent = nullptr;
    void* node = nullptr;
    bool left = false;
};

struct PreviewRegistry {
    PreviewEntry e[kPreviewMax];
    int n = 0;
};

// Index of a registered gob, or -1.
inline int previewFind(const PreviewRegistry& r, const void* gob) {
    for (int i = 0; i < r.n; ++i)
        if (r.e[i].gob == gob) return i;
    return -1;
}

// Adds a gob. False for a null gob, a gob that is already registered, or a full registry.
inline bool previewRegister(PreviewRegistry& r, void* gob, void* parent, void* node, bool left) {
    if (!gob || previewFind(r, gob) >= 0 || r.n >= kPreviewMax) return false;
    PreviewEntry& p = r.e[r.n++];
    p.gob = gob;
    p.parent = parent;
    p.node = node;
    p.left = left;
    return true;
}

// Removes entry i and keeps the rest in order. Caller guarantees 0 <= i < n.
inline void previewRemoveAt(PreviewRegistry& r, int i) {
    for (int k = i; k + 1 < r.n; ++k) r.e[k] = r.e[k + 1];
    --r.n;
    r.e[r.n] = PreviewEntry();
}

inline bool previewUnregister(PreviewRegistry& r, const void* gob) {
    const int i = previewFind(r, gob);
    if (i < 0) return false;
    previewRemoveAt(r, i);
    return true;
}

// Drops every entry whose gob is gone or whose live parent or node differs from the registered one. live(gob, parent, node)
// writes the gob's current parent and node and returns false when the gob no longer exists. Returns the number dropped.
template <typename Live>
inline int previewSweep(PreviewRegistry& r, Live live) {
    int dropped = 0;
    int i = 0;
    while (i < r.n) {
        void* parent = nullptr;
        void* node = nullptr;
        const bool keep = live(r.e[i].gob, parent, node) && parent == r.e[i].parent && node == r.e[i].node;
        if (keep) {
            ++i;
            continue;
        }
        previewRemoveAt(r, i);
        ++dropped;
    }
    return dropped;
}

// ---- Edit overlay --------------------------------------------------------------------------------------------------------

enum OverlayScope : uint8_t { ScopeWeapon = 0, ScopeType = 1, ScopeBody = 2 };

inline constexpr float kPreviewRotCap = 720.f;
inline constexpr float kPreviewOffCap = 3.f;

// Clamps to [-lim, lim]. NaN becomes 0.
inline float previewCap(float v, float lim) {
    if (std::isnan(v)) return 0.f;
    return v > lim ? lim : (v < -lim ? -lim : v);
}

struct EditOverlay {
    bool active = false;
    bool dirty = false;
    bool left = false;
    bool hidden = false;                 // flag only; its semantics are not defined in this unit
    OverlayScope scope = ScopeWeapon;
    Category cat = CatUnknown;
    char model[17] = {0};
    char body[17] = {0};
    Resolved cur;                        // shown rot, absolute off (level + body part), bone; src is passed through
    Q placeQ{1, 0, 0, 0};                // canonical placement; the shown euler is re-derived from it
    float baseOff[3] = {0.f, 0.f, 0.f};  // off without the body delta (the level value)
    Resolved seed;                       // BeginEdit values, restored by overlayRevert
    float seedBaseOff[3] = {0.f, 0.f, 0.f};
};

// Re-derives the shown euler from placeQ. The last shown euler is the continuity reference for eulerNear.
inline void overlaySync(EditOverlay& o) {
    float e[3];
    eulerNear(o.placeQ, o.cur.rot, e);
    for (int i = 0; i < 3; ++i) o.cur.rot[i] = e[i];
}

// Writes an absolute off from the value pane. Under body scope the body part changes. Otherwise the level value changes and
// the body part is kept. Each stored component is capped at kPreviewOffCap.
inline void overlaySetAbsOff(EditOverlay& o, const float x[3]) {
    for (int i = 0; i < 3; ++i) {
        if (o.scope == ScopeBody) {
            const float d = previewCap(x[i] - o.baseOff[i], kPreviewOffCap);
            o.cur.off[i] = o.baseOff[i] + d;
        } else {
            const float bodyPart = o.cur.off[i] - o.baseOff[i];
            o.baseOff[i] = previewCap(x[i] - bodyPart, kPreviewOffCap);
            o.cur.off[i] = o.baseOff[i] + bodyPart;
        }
    }
}

// Starts an edit for one weapon on one hand. The seed is the full resolve (body delta included). The base is the resolve
// without the body delta. Scope starts at ScopeWeapon.
inline void overlayBegin(EditOverlay& o, const TuneCfg& c, const char* model, Category cat, const char* body, bool left) {
    Resolved full, base;
    resolveFull(c, model, cat, body, left, false, full);
    resolveFull(c, model, cat, body, left, true, base);
    o = EditOverlay();
    o.active = true;
    o.left = left;
    o.cat = cat;
    tuneCopyName(o.model, model ? model : "");
    tuneCopyName(o.body, body ? body : "");
    o.cur = full;
    o.seed = full;
    for (int i = 0; i < 3; ++i) {
        o.baseOff[i] = base.off[i];
        o.seedBaseOff[i] = base.off[i];
    }
    o.placeQ = qbase(full.rot[0], full.rot[1], full.rot[2]);
}

// Switches the write target. Does not change any value or the dirty flag. False for an invalid scope or an inactive overlay.
inline bool overlaySetScope(EditOverlay& o, int scope) {
    if (!o.active || scope < 0 || scope > 2) return false;
    o.scope = static_cast<OverlayScope>(scope);
    return true;
}

// Rotates the placement by a world-space drag dqWorld about the bone's world orientation boneQ. Rotation is not body-scoped.
inline bool overlayNudgeWorldRot(EditOverlay& o, const Q& boneQ, const Q& dqWorld) {
    if (!o.active || o.scope == ScopeBody) return false;
    o.placeQ = worldDeltaToLocal(boneQ, o.placeQ, dqWorld);
    overlaySync(o);
    o.dirty = true;
    return true;
}

// Moves the offset by a world-space drag dpWorld (metres). Works under every scope.
inline bool overlayNudgeWorldMove(EditOverlay& o, const Q& boneQ, const float dpWorld[3]) {
    if (!o.active) return false;
    float x[3];
    worldMoveToLocal(boneQ, o.cur.off, dpWorld, x);
    overlaySetAbsOff(o, x);
    o.dirty = true;
    return true;
}

// Rotates the placement about one local axis (0 x, 1 y, 2 z) by deg. Rotation is not body-scoped.
inline bool overlayNudgeLocal(EditOverlay& o, int axis, float deg) {
    if (!o.active || o.scope == ScopeBody || axis < 0 || axis > 2) return false;
    o.placeQ = localAxisNudge(o.placeQ, axis, deg);
    overlaySync(o);
    o.dirty = true;
    return true;
}

// Writes the value pane. Under body scope rot is ignored, because a body delta carries only off. Otherwise rot is capped at
// 720 deg per component and placeQ is rebuilt from it.
inline bool overlaySetValues(EditOverlay& o, const float rot[3], const float off[3]) {
    if (!o.active) return false;
    if (o.scope != ScopeBody) {
        for (int i = 0; i < 3; ++i) o.cur.rot[i] = previewCap(rot[i], kPreviewRotCap);
        o.placeQ = qbase(o.cur.rot[0], o.cur.rot[1], o.cur.rot[2]);
    }
    overlaySetAbsOff(o, off);
    o.dirty = true;
    return true;
}

// Steps the mount bone to the next (+1) or previous (-1) editor bone, wrapping over 1..kBoneCount. From 0 (no bone), +1 gives 1
// and -1 gives kBoneCount. Not body-scoped.
inline bool overlayCycleBone(EditOverlay& o, int step) {
    if (!o.active || o.scope == ScopeBody || (step != 1 && step != -1)) return false;
    int b = (o.cur.boneIdx >= 1 && o.cur.boneIdx <= kBoneCount) ? static_cast<int>(o.cur.boneIdx) : 0;
    b += step;
    if (b < 1) b = kBoneCount;
    else if (b > kBoneCount) b = 1;
    if (!boneNameOf(static_cast<uint8_t>(b))) return false;
    o.cur.boneIdx = static_cast<uint8_t>(b);
    o.dirty = true;
    return true;
}

// Restores the BeginEdit values and clears the dirty flag. The scope is kept.
inline void overlayRevert(EditOverlay& o) {
    if (!o.active) return;
    o.cur = o.seed;
    for (int i = 0; i < 3; ++i) o.baseOff[i] = o.seedBaseOff[i];
    o.placeQ = qbase(o.seed.rot[0], o.seed.rot[1], o.seed.rot[2]);
    o.dirty = false;
}

struct EditView {
    Resolved r;                          // rot, off, boneIdx and src (src passed through from resolveFull)
    bool dirty = false;
    bool left = false;
    OverlayScope scope = ScopeWeapon;
};

// Refreshes the shown euler from placeQ and fills the view.
inline void overlayGetEdit(EditOverlay& o, EditView& v) {
    if (o.active) overlaySync(o);
    v.r = o.cur;
    v.dirty = o.dirty;
    v.left = o.left;
    v.scope = o.scope;
}

// Stores one level's values (rot, level off, bone). The level is in the hand's own frame, so nothing is mirrored here.
inline void overlayStoreLevel(CatTune& t, const EditOverlay& o) {
    t.hasRot = true;
    t.hasOff = true;
    for (int i = 0; i < 3; ++i) {
        t.rot[i] = o.cur.rot[i];
        t.off[i] = o.baseOff[i];
    }
    t.boneIdx = o.cur.boneIdx;
}

// Writes the pending edit into c for the current scope. False when the target is missing: no model or body name, an invalid
// category for type scope, an inactive overlay, or a full table.
//   Weapon  per-model entry for this hand (left entries are stored in the left frame).
//   Type    cat[] for the right hand, catL[] for the left.
//   Body    body delta = absolute off - baseOff. resolveFull negates x on the left, so the stored x is negated there too.
inline bool overlayWrite(const EditOverlay& o, TuneCfg& c) {
    if (!o.active) return false;
    switch (o.scope) {
    case ScopeWeapon: {
        if (!o.model[0]) return false;
        ModelTune* m = findOrAddModel(c, o.model, o.left);
        if (!m) return false;
        overlayStoreLevel(m->t, o);
        return true;
    }
    case ScopeType: {
        if (!(o.cat > CatUnknown && o.cat < CatCount)) return false;
        overlayStoreLevel(o.left ? c.catL[o.cat] : c.cat[o.cat], o);
        return true;
    }
    case ScopeBody: {
        if (!o.body[0]) return false;
        BodyTune* b = findOrAddBody(c, o.body);
        if (!b) return false;
        float d[3];
        for (int i = 0; i < 3; ++i) d[i] = o.cur.off[i] - o.baseOff[i];
        if (o.left) mirrorOff(d);
        for (int i = 0; i < 3; ++i) b->off[i] = d[i];
        return true;
    }
    }
    return false;
}

}  // namespace hs
