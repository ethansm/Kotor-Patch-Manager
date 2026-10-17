// ViewDrag.h -- pure camera maths and drag tracking shared by preview-3d and holster-editor. equip3d keeps its own eq3d copy (Equip3dLogic.h).
#ifndef VIEW_DRAG_H
#define VIEW_DRAG_H

#include <math.h>
#include <stdint.h>

namespace p3d {

constexpr float kMaxSceneFov = 150.0f;

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

// Tuning for dragMove.
struct DragCfg {
    float lockPx = 4.f, dragDegPx = 0.5f, panPerPx = 0.003f, panMin = -1.2f, panMax = 1.2f;
};

// Drag state of one held mouse button. axis: 0 undecided, 1 yaw, 2 pan. Mirrors EquipCharPreview.cpp.
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
// locked axis moves, and only when the cursor moved on it. Returns true when yaw or panZ changed.
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

} // namespace p3d

#endif // VIEW_DRAG_H
