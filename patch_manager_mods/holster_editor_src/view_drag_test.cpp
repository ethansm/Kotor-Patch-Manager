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

static bool nearf(float a, float b, float eps = 1e-3f) { return fabsf(a - b) <= eps; }

static void testDragEdges() {
    // DragCfg{} defaults pinned
    p3d::DragCfg def;
    KT_CHECK(nearf(def.lockPx, 4.0f)); KT_CHECK(nearf(def.dragDegPx, 0.5f)); KT_CHECK(nearf(def.panPerPx, 0.003f));
    KT_CHECK(nearf(def.panMin, -1.2f)); KT_CHECK(nearf(def.panMax, 1.2f));

    // pan: lock on axis 2, then keep moving; 10 px * 0.003 = 0.03 per step, then clamp both ends
    {
        p3d::Drag d; p3d::ViewState s;
        p3d::dragBegin(&d, 100.0f, 100.0f);
        KT_CHECK(p3d::dragMove(&d, 100.0f, 110.0f, def, &s));
        KT_CHECK_EQ(d.axis, 2);
        KT_CHECK(nearf(s.panZ, 0.03f));
        KT_CHECK(p3d::dragMove(&d, 100.0f, 120.0f, def, &s));
        KT_CHECK(nearf(s.panZ, 0.06f));
        KT_CHECK(p3d::dragMove(&d, 100.0f, 5000.0f, def, &s));
        KT_CHECK(nearf(s.panZ, 1.2f));
        KT_CHECK(p3d::dragMove(&d, 100.0f, -5000.0f, def, &s));
        KT_CHECK(nearf(s.panZ, -1.2f));
    }
    // pan is divided by zoom: 10 px at zoom 2 = 0.015
    {
        p3d::Drag d; p3d::ViewState s; s.zoom = 2.0f;
        p3d::dragBegin(&d, 0.0f, 0.0f);
        KT_CHECK(p3d::dragMove(&d, 0.0f, 10.0f, def, &s));
        KT_CHECK(nearf(s.panZ, 0.015f));
    }
    // zoom 0.1 or less counts as 1
    KT_CHECK(nearf(p3d::panDrag(0.0f, 10.0f, 0.0f, 0.003f, 0.1f, -1.2f, 1.2f), 0.03f));

    // dragAxis
    KT_CHECK_EQ(p3d::dragAxis(5.0f, 5.0f, 4.0f), 1);     // tie -> yaw
    KT_CHECK_EQ(p3d::dragAxis(0.0f, -6.0f, 4.0f), 2);
    KT_CHECK_EQ(p3d::dragAxis(3.9f, 3.9f, 4.0f), 0);     // inside the lock
    KT_CHECK_EQ(p3d::dragAxis(4.0f, 0.0f, 4.0f), 1);     // exactly the lock: decided

    // yaw: 5 px right of start decides yaw, then one 5 px move = 2.5 deg; from 180: 180 + 5 * 0.5 = 182.5
    {
        p3d::Drag d; p3d::ViewState s; s.yaw = 180.0f;
        p3d::dragBegin(&d, 0.0f, 0.0f);
        KT_CHECK(p3d::dragMove(&d, 5.0f, 0.0f, def, &s));
        KT_CHECK_EQ(d.axis, 1);
        KT_CHECK(nearf(s.yaw, 182.5f));
        // unchanged cursor: no change
        KT_CHECK(!p3d::dragMove(&d, 5.0f, 0.0f, def, &s));
        KT_CHECK(nearf(s.yaw, 182.5f));
    }
    // inactive drag changes nothing
    {
        p3d::Drag d; p3d::ViewState s;
        KT_CHECK(!p3d::dragMove(&d, 50.0f, 50.0f, def, &s));
        KT_CHECK(nearf(s.yaw, 0.0f)); KT_CHECK(nearf(s.panZ, 0.0f));
    }

    // zoomStep clamps at both ends
    KT_CHECK(nearf(p3d::zoomStep(2.9f, 1200, 0.9f, 0.5f, 3.0f), 3.0f));
    KT_CHECK(nearf(p3d::zoomStep(0.6f, -1200, 0.9f, 0.5f, 3.0f), 0.5f));
    // zoomStep(1, +1200 = 10 notches in, step 0.9): 1 / 0.9^10 = 2.868; (2, 0) = 2
    KT_CHECK(nearf(p3d::zoomStep(1.0f, 1200, 0.9f, 0.5f, 4.0f), 2.868f, 0.01f));
    KT_CHECK(nearf(p3d::zoomStep(2.0f, 0, 0.9f, 0.5f, 3.0f), 2.0f));
}

int main() {
    testCameraMaths();
    testDragLogic();
    testDragEdges();
    return kt::Report("view_drag");
}
