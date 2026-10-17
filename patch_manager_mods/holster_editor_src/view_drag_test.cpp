// view_drag_test.cpp -- test for ViewDrag.h camera maths and drag state machine.
#include "ViewDrag.h"
#include "check.h"
#include <math.h>

static void testCameraMaths() {
    // fovForZoom
    KT_CHECK(fabsf(p3d::fovForZoom(60.0f, 1.0f) - 60.0f) < 0.001f);
    KT_CHECK(fabsf(p3d::fovForZoom(60.0f, 2.0f) - 30.0f) < 0.001f);
    KT_CHECK(fabsf(p3d::fovForZoom(60.0f, 0.1f) - 150.0f) < 0.001f); // clamped to 150

    // yawVec
    float v[3] = {0, 0, 0};
    p3d::yawVec(0.0f, v);
    KT_CHECK(fabsf(v[0] - 1.0f) < 0.001f);
    KT_CHECK(fabsf(v[1]) < 0.001f);
    KT_CHECK(fabsf(v[2]) < 0.001f);

    p3d::yawVec(90.0f, v);
    KT_CHECK(fabsf(v[0]) < 0.001f);
    KT_CHECK(fabsf(v[1] - 1.0f) < 0.001f);

    // zoomStep
    float z = p3d::zoomStep(1.0f, 120, 0.9f, 0.5f, 3.0f);
    KT_CHECK(fabsf(z - 1.1111f) < 0.01f); // 1.0 / 0.9 = 1.111
    float zOut = p3d::zoomStep(1.0f, -120, 0.9f, 0.5f, 3.0f);
    KT_CHECK(fabsf(zOut - 0.9f) < 0.001f);
}

static void testDragLogic() {
    p3d::Drag d;
    p3d::DragCfg cfg;
    cfg.lockPx = 4.0f;
    cfg.dragDegPx = 0.5f;
    cfg.panPerPx = 0.003f;
    cfg.panMin = -1.2f;
    cfg.panMax = 1.2f;

    p3d::ViewState s = {180.0f, 1.0f, 0.0f};

    p3d::dragBegin(&d, 100.0f, 100.0f);
    KT_CHECK(d.active);
    KT_CHECK_EQ(d.axis, 0);

    // Minor movement within lockPx
    bool changed = p3d::dragMove(&d, 102.0f, 101.0f, cfg, &s);
    KT_CHECK(!changed);
    KT_CHECK_EQ(d.axis, 0);

    // Move horizontally past lockPx -> locks axis 1 (yaw)
    changed = p3d::dragMove(&d, 110.0f, 102.0f, cfg, &s);
    KT_CHECK(changed);
    KT_CHECK_EQ(d.axis, 1);
    KT_CHECK(fabsf(s.yaw - 184.0f) < 0.01f); // 180 + (110 - 102)*0.5 = 184

    // Further vertical motion is ignored because axis is locked to 1
    changed = p3d::dragMove(&d, 110.0f, 150.0f, cfg, &s);
    KT_CHECK(!changed);
    KT_CHECK(fabsf(s.panZ) < 0.001f);

    p3d::dragEnd(&d);
    KT_CHECK(!d.active);
}

int main() {
    testCameraMaths();
    testDragLogic();
    return kt::Report("view_drag");
}
