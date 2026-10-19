// HolsterPreview.h -- pure state of the holstered-weapons editor preview (doc 42, HE2): the preview Gob registry and the edit
// overlay (pending values, scope, Revert seed). Header-only, no windows.h, no engine calls, no engine types. Gobs, parents and
// nodes are opaque void* owned by the caller. Host-tested by holster_preview_test.cpp.
//
// Registry  previewRegister / previewUnregister / previewSweep. At most kPreviewMax (8) entries. The sweep drops an entry whose
//           parent or node changed, or whose gob is gone.
// Overlay   overlayBegin seeds from resolveFull (body delta included). Nudges and overlaySetValues edit the shown values only.
//           overlayWrite stores them into a TuneCfg for the current scope. Angles are degrees. Off is metres.
//           Body scope edits only the body delta (offset and rotation; the bone is read-only there). Other scopes edit the level value.
//           The body rotation delta is kept apart from the level rotation exactly as the body offset is kept apart from baseOff:
//           baseQ is the placement without the body delta, bodyQ the hand-frame body delta, and
//           placeQ = E * bodyQ * conj(E) * baseQ, with E the left_extra_rot tilt (identity unless the seed carried one).
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
    Q baseQ{1, 0, 0, 0};                 // placement without the body rotation delta (shown frame, left_extra included)
    Q bodyQ{1, 0, 0, 0};                 // body rotation delta in the hand's own frame (identity = none)
    float baseRot[3] = {0.f, 0.f, 0.f};  // euler of baseQ (continuity reference)
    Q seedBodyQ{1, 0, 0, 0};
    float seedBaseRot[3] = {0.f, 0.f, 0.f};
};

// The left_extra_rot tilt of the open edit as a quaternion (identity when the seed carried none).
inline Q overlayExtraQ(const EditOverlay& o) {
    const float xAxis[3] = {1.f, 0.f, 0.f};
    return o.seed.extraDeg != 0.f ? qaxis(xAxis, o.seed.extraDeg) : Q{1.f, 0.f, 0.f, 0.f};
}

inline bool overlayBodyRotSet(const EditOverlay& o) {
    return std::fabs(o.bodyQ.x) > 1e-6f || std::fabs(o.bodyQ.y) > 1e-6f || std::fabs(o.bodyQ.z) > 1e-6f;
}

// Call after placeQ changed by an edit. Body scope: the change belongs to the body delta (baseQ stays). Other scopes: the change
// belongs to the level (bodyQ stays) and baseQ follows, so the body part is never baked into a weapon or type level.
inline void overlayRotChanged(EditOverlay& o) {
    const Q e = overlayExtraQ(o), ec = qconj(e);
    if (o.scope == ScopeBody) {
        o.bodyQ = qnormalize(qmul(qmul(qmul(ec, o.placeQ), qconj(o.baseQ)), e));
    } else {
        o.baseQ = qnormalize(qmul(qmul(qmul(e, qconj(o.bodyQ)), ec), o.placeQ));
        float r[3];
        eulerNear(o.baseQ, o.baseRot, r);
        for (int i = 0; i < 3; ++i) o.baseRot[i] = r[i];
    }
}

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
    o.baseQ = qbase(base.rot[0], base.rot[1], base.rot[2]);
    for (int i = 0; i < 3; ++i) {
        o.baseRot[i] = base.rot[i];
        o.seedBaseRot[i] = base.rot[i];
    }
    const BodyTune* bt = findBody(c, body);
    if (bt && bt->hasRot) {
        float d[3] = {bt->rot[0], bt->rot[1], bt->rot[2]};
        if (left) mirrorRot(d);
        o.bodyQ = qbase(d[0], d[1], d[2]);
    }
    o.seedBodyQ = o.bodyQ;
}

// Switches the write target. Does not change any value or the dirty flag. False for an invalid scope or an inactive overlay.
inline bool overlaySetScope(EditOverlay& o, int scope) {
    if (!o.active || scope < 0 || scope > 2) return false;
    o.scope = static_cast<OverlayScope>(scope);
    return true;
}

// Rotates the placement by a world-space drag dqWorld about the bone's world orientation boneQ. Under body scope the body delta turns.
inline bool overlayNudgeWorldRot(EditOverlay& o, const Q& boneQ, const Q& dqWorld) {
    if (!o.active) return false;
    o.placeQ = worldDeltaToLocal(boneQ, o.placeQ, dqWorld);
    overlaySync(o);
    overlayRotChanged(o);
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

// Rotates the placement about one local axis (0 x, 1 y, 2 z) by deg. Under body scope the body delta turns.
inline bool overlayNudgeLocal(EditOverlay& o, int axis, float deg) {
    if (!o.active || axis < 0 || axis > 2) return false;
    o.placeQ = localAxisNudge(o.placeQ, axis, deg);
    overlaySync(o);
    overlayRotChanged(o);
    o.dirty = true;
    return true;
}

// Writes the value pane: the shown absolute rot (capped at 720 deg per component; placeQ is rebuilt from it) and the shown absolute
// off. Under body scope the difference to the base becomes the body delta, otherwise the level changes.
inline bool overlaySetValues(EditOverlay& o, const float rot[3], const float off[3]) {
    if (!o.active) return false;
    for (int i = 0; i < 3; ++i) o.cur.rot[i] = previewCap(rot[i], kPreviewRotCap);
    o.placeQ = qbase(o.cur.rot[0], o.cur.rot[1], o.cur.rot[2]);
    overlayRotChanged(o);
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
    o.bodyQ = o.seedBodyQ;
    o.baseQ = qbase(o.seedBaseRot[0], o.seedBaseRot[1], o.seedBaseRot[2]);
    for (int i = 0; i < 3; ++i) o.baseRot[i] = o.seedBaseRot[i];
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

// True when the shown rot still equals the seed within the printed precision (0.06 deg per component).
inline bool overlayRotUnchanged(const EditOverlay& o) {
    for (int i = 0; i < 3; ++i)
        if (std::fabs(wrap180(o.cur.rot[i] - o.seed.rot[i])) > 0.06f) return false;
    return true;
}

// Stores one level's values (rot, level off, bone). The level is in the hand's own frame, so nothing is mirrored here.
// left_extra_rot contract: when the seed's left rot was inherited from the mirror and carried an extra tilt (seed.extraDeg != 0) and the
// user did not change the rot, rot is NOT written (hasRot and rot stay as they were), so the level stays "inherited" and the extra keeps
// applying live. A changed rot is written as shown (WYSIWYG: an explicit l_ rot is never given the extra again).
inline void overlayStoreLevel(CatTune& t, const EditOverlay& o) {
    t.hasOff = true;
    const bool keepInherited = o.left && o.seed.extraDeg != 0.f && overlayRotUnchanged(o);
    if (!keepInherited) {
        t.hasRot = true;
        float r[3] = {o.cur.rot[0], o.cur.rot[1], o.cur.rot[2]};
        if (overlayBodyRotSet(o)) {   // the level is the shown placement without the body rotation delta (never baked into the level)
            const Q lq = qnormalize(qmul(qconj(o.bodyQ), o.placeQ));
            eulerNear(lq, o.cur.rot, r);
        }
        for (int i = 0; i < 3; ++i) t.rot[i] = r[i];
    }
    for (int i = 0; i < 3; ++i) t.off[i] = o.baseOff[i];
    t.boneIdx = o.cur.boneIdx;
}

// Writes the pending edit into c for the current scope. False when the target is missing: no model or body name, an invalid
// category for type scope, an inactive overlay, or a full table.
//   Weapon  per-model entry for this hand (left entries are stored in the left frame).
//   Type    cat[] for the right hand, catL[] for the left.
//   Body    body delta = absolute off - baseOff, and rot D = eulerNear(bodyQ, 0) (bodyQ = conj(E) * placeQ * conj(baseQ) * E). resolveFull
//           negates x (off) and mirrors y, z (rot) on the left, so the stored values are mirrored back there too. A D within the
//           printed precision of zero is stored as no rot (hasRot false), so the key is removed.
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
        const float zero[3] = {0.f, 0.f, 0.f};
        float dr[3];
        eulerNear(o.bodyQ, zero, dr);
        if (o.left) mirrorRot(dr);
        const bool any = std::fabs(dr[0]) >= 0.05f || std::fabs(dr[1]) >= 0.05f || std::fabs(dr[2]) >= 0.05f;
        b->hasRot = any;
        for (int i = 0; i < 3; ++i) b->rot[i] = any ? dr[i] : 0.f;
        return true;
    }
    }
    return false;
}

}  // namespace hs
