// Preview3dLogic.h -- pure logic of the k2.preview3d provider (equipment-3d-character preview): the viewer token table, the camera
// maths and the drag state. No engine calls, no Win32 types, no KpLog: the provider .cpp keeps all engine access and logging.
//
// Each maths function is a verbatim lift of the expression in patch_manager_mods/equipment_3d_character_src/Equip3dLogic.h
// (fovForZoom, yawVec, zoomStep, yawDrag, panDrag, dragAxis, kMaxSceneFov). Equip3dLogic.h itself is NOT included: it owns the
// eq3d weapon table and the .cpp-side config, which the provider does not need.
#pragma once
#include <math.h>
#include <stdint.h>

namespace p3d {

// Ceiling for the camera fov after the zoom divide (Equip3dLogic.h:126).
constexpr float kMaxSceneFov = 150.f;

// ---------------------------------------------------------------- viewer tokens
// A fixed table of live viewers. A token is never 0 and is never handed out while another live viewer holds it. Tokens are not
// reused right after a removal: the counter runs on, so a stale token from a destroyed view cannot alias a new one.
constexpr int kMaxViewers = 4;

struct Viewers {
    uint32_t ids[kMaxViewers];   // live tokens; 0 = free slot
    uint32_t nextId;             // next counter value to try (never 0 after init)
};

inline void viewersInit(Viewers* v) {
    for (int i = 0; i < kMaxViewers; ++i) v->ids[i] = 0;
    v->nextId = 1;
}

// Index of the live slot holding `token`, or -1 (token 0 is never live).
inline int viewersIndex(const Viewers* v, uint32_t token) {
    if (token == 0) return -1;
    for (int i = 0; i < kMaxViewers; ++i)
        if (v->ids[i] == token) return i;
    return -1;
}

inline int viewersCount(const Viewers* v) {
    int n = 0;
    for (int i = 0; i < kMaxViewers; ++i)
        if (v->ids[i] != 0) ++n;
    return n;
}

// New token, or 0 when every slot is in use. The counter skips 0 on wrap and skips any value still held by a live viewer.
inline uint32_t viewersAdd(Viewers* v) {
    int slot = -1;
    for (int i = 0; i < kMaxViewers && slot < 0; ++i)
        if (v->ids[i] == 0) slot = i;
    if (slot < 0) return 0;
    for (int tries = 0; tries <= kMaxViewers; ++tries) {   // at most kMaxViewers live ids, so one extra try always finds a free value
        uint32_t t = v->nextId;
        v->nextId = (t == 0xFFFFFFFFu) ? 1u : t + 1u;
        if (t != 0 && viewersIndex(v, t) < 0) {
            v->ids[slot] = t;
            return t;
        }
    }
    return 0;
}

// True when `token` was live and is now removed.
inline bool viewersRemove(Viewers* v, uint32_t token) {
    int i = viewersIndex(v, token);
    if (i < 0) return false;
    v->ids[i] = 0;
    return true;
}

// ---------------------------------------------------------------- camera maths
// Scene camera fov for a zoom factor: fov / zoom, capped at kMaxSceneFov. Zoom is not guarded here (callers keep it > 0).
inline float fovForZoom(float fov, float zoom) { return fminf(fov / zoom, kMaxSceneFov); }

// Facing vector for a yaw in degrees, rotating about +Z (x, y, 0).
inline void yawVec(float yawDeg, float v[3]) {
    float rad = yawDeg * 0.0174532925f;
    v[0] = cosf(rad); v[1] = sinf(rad); v[2] = 0.f;
}

// Wheel (+120 per notch up = zoom in) -> new zoom, clamped to [zoomMin, zoomMax]. wheelStep is the zoom factor per notch.
inline float zoomStep(float zoom, long wheel, float wheelStep, float zoomMin, float zoomMax) {
    zoom *= powf(wheelStep, -wheel / 120.0f);
    return fminf(fmaxf(zoom, zoomMin), zoomMax);
}

// ---------------------------------------------------------------- drag
// Horizontal drag -> yaw. dragDegPx is degrees per pixel.
inline float yawDrag(float yaw, float cx, float lastX, float dragDegPx) { return yaw + (cx - lastX) * dragDegPx; }

// Vertical drag -> creature Z offset; divided by zoom so the model tracks the cursor at any FOV. A zoom of 0.1 or less counts as 1.
// Negative panPerPx inverts the direction.
inline float panDrag(float panZ, float cy, float lastY, float panPerPx, float zoom, float panMin, float panMax) {
    float z = zoom > 0.1f ? zoom : 1.0f;
    return fminf(fmaxf(panZ + (cy - lastY) * panPerPx / z, panMin), panMax);
}

// Drag axis, decided once per drag: 0 = undecided (both deltas inside lockPx), 1 = yaw (X), 2 = pan (Y). A tie goes to yaw.
inline int dragAxis(float dx, float dy, float lockPx) {
    if (fabsf(dx) < lockPx && fabsf(dy) < lockPx) return 0;
    return fabsf(dx) >= fabsf(dy) ? 1 : 2;
}

// The camera state the drag edits.
struct ViewState {
    float yaw = 0.f, zoom = 1.f, panZ = 0.f;
};

// Tuning for dragMove (from the provider's config).
struct DragCfg {
    float lockPx = 4.f, dragDegPx = 0.5f, panPerPx = 0.01f, panMin = -1.f, panMax = 1.f;
};

// Drag state of one held mouse button. axis: 0 undecided, 1 yaw, 2 pan. Mirrors the EquipCharPreview.cpp drag block.
struct Drag {
    bool active = false;
    int axis = 0;
    float startX = 0.f, startY = 0.f, lastX = 0.f, lastY = 0.f;
};

inline void dragBegin(Drag* d, float cx, float cy) {
    d->active = true;
    d->axis = 0;
    d->startX = d->lastX = cx;
    d->startY = d->lastY = cy;
}

inline void dragEnd(Drag* d) { d->active = false; }

// One move sample while the button is held. The axis is decided from the start point (once, inside lockPx); after that only the
// locked axis moves, and only when the cursor moved on it. Returns true when yaw or panZ changed (the provider then re-applies the
// view). An inactive drag returns false and changes nothing.
inline bool dragMove(Drag* d, float cx, float cy, const DragCfg& c, ViewState* v) {
    if (!d->active) return false;
    bool changed = false;
    if (!d->axis) d->axis = dragAxis(cx - d->startX, cy - d->startY, c.lockPx);
    if (d->axis == 1 && cx != d->lastX) {
        v->yaw = yawDrag(v->yaw, cx, d->lastX, c.dragDegPx);
        changed = true;
    } else if (d->axis == 2 && cy != d->lastY) {
        v->panZ = panDrag(v->panZ, cy, d->lastY, c.panPerPx, v->zoom, c.panMin, c.panMax);
        changed = true;
    }
    d->lastX = cx;
    d->lastY = cy;
    return changed;
}

}  // namespace p3d
