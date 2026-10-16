// Host test of Preview3dLogic.h: viewer token table, camera maths (lifted from Equip3dLogic.h), drag state.
// Expected values are literals worked out by hand from the formulas; no engine data is involved.
// Includes only Preview3dLogic.h (no kpatch_api.h): the API header needs the KPM Common dir, which only the contract test's flags add.
#include "Preview3dLogic.h"
#include "check.h"
#include <math.h>

using namespace p3d;

static bool nearf(float a, float b, float eps = 1e-5f) { return fabsf(a - b) <= eps; }

int main() {
    // ---- fovForZoom: fov / zoom, capped at kMaxSceneFov (150) ----
    KT_CHECK(nearf(kMaxSceneFov, 150.f));
    KT_CHECK(nearf(fovForZoom(90.f, 1.f), 90.f));
    KT_CHECK(nearf(fovForZoom(90.f, 2.f), 45.f));
    KT_CHECK(nearf(fovForZoom(60.f, 0.5f), 120.f));   // 120 < 150: no cap
    KT_CHECK(nearf(fovForZoom(75.f, 0.5f), 150.f));   // exactly the cap
    KT_CHECK(nearf(fovForZoom(90.f, 0.2f), 150.f));   // 450 capped to 150 (fov_deg=90, zoom_min=0.2 case)

    // ---- yawVec: (cos, sin, 0) about +Z ----
    float v[3];
    yawVec(0.f, v);    KT_CHECK(nearf(v[0], 1.f) && nearf(v[1], 0.f) && nearf(v[2], 0.f));
    yawVec(90.f, v);   KT_CHECK(nearf(v[0], 0.f) && nearf(v[1], 1.f) && nearf(v[2], 0.f));
    yawVec(180.f, v);  KT_CHECK(nearf(v[0], -1.f) && nearf(v[1], 0.f));
    yawVec(-90.f, v);  KT_CHECK(nearf(v[0], 0.f) && nearf(v[1], -1.f));
    yawVec(360.f, v);  KT_CHECK(nearf(v[0], 1.f) && nearf(v[1], 0.f, 1e-4f));

    // ---- zoomStep: wheel +120 = zoom in (divide by step per notch); clamped to [zoomMin, zoomMax] ----
    KT_CHECK(nearf(zoomStep(1.f, 0, 1.25f, 0.2f, 5.f), 1.f));
    KT_CHECK(nearf(zoomStep(1.f, 120, 1.25f, 0.2f, 5.f), 0.8f));       // 1 / 1.25
    KT_CHECK(nearf(zoomStep(1.f, -120, 1.25f, 0.2f, 5.f), 1.25f));     // wheel down = zoom out
    KT_CHECK(nearf(zoomStep(1.f, 240, 1.25f, 0.2f, 5.f), 0.64f));      // two notches
    KT_CHECK(nearf(zoomStep(0.5f, 120 * 20, 1.25f, 0.2f, 5.f), 0.2f)); // clamped at zoomMin
    KT_CHECK(nearf(zoomStep(4.f, -120 * 20, 1.25f, 0.2f, 5.f), 5.f));  // clamped at zoomMax

    // ---- yawDrag / panDrag ----
    KT_CHECK(nearf(yawDrag(10.f, 100.f, 90.f, 0.5f), 15.f));
    KT_CHECK(nearf(yawDrag(0.f, 90.f, 100.f, 0.5f), -5.f));
    KT_CHECK(nearf(panDrag(0.f, 110.f, 100.f, 0.01f, 1.f, -1.f, 1.f), 0.1f));
    KT_CHECK(nearf(panDrag(0.f, 110.f, 100.f, 0.01f, 2.f, -1.f, 1.f), 0.05f));  // divided by zoom
    KT_CHECK(nearf(panDrag(0.f, 110.f, 100.f, 0.01f, 0.05f, -1.f, 1.f), 0.1f)); // zoom <= 0.1 counts as 1
    KT_CHECK(nearf(panDrag(0.95f, 200.f, 100.f, 0.01f, 1.f, -1.f, 1.f), 1.f));  // clamped at panMax
    KT_CHECK(nearf(panDrag(-0.95f, 0.f, 100.f, 0.01f, 1.f, -1.f, 1.f), -1.f)); // clamped at panMin
    KT_CHECK(nearf(panDrag(0.f, 110.f, 100.f, -0.01f, 1.f, -1.f, 1.f), -0.1f)); // negative pan_per_px inverts

    // ---- dragAxis: undecided while both deltas are inside lockPx; else the larger delta wins; a tie goes to yaw ----
    KT_CHECK_EQ(dragAxis(0.f, 0.f, 4.f), 0);
    KT_CHECK_EQ(dragAxis(3.99f, 3.99f, 4.f), 0);
    KT_CHECK_EQ(dragAxis(4.f, 0.f, 4.f), 1);     // exactly the lock: decided
    KT_CHECK_EQ(dragAxis(10.f, 2.f, 4.f), 1);
    KT_CHECK_EQ(dragAxis(2.f, -10.f, 4.f), 2);
    KT_CHECK_EQ(dragAxis(5.f, 5.f, 4.f), 1);     // tie -> yaw

    // ---- drag state: the axis locks on the first move past lockPx; then only that axis moves ----
    DragCfg cfg;
    cfg.lockPx = 4.f; cfg.dragDegPx = 0.5f; cfg.panPerPx = 0.01f; cfg.panMin = -1.f; cfg.panMax = 1.f;
    ViewState vs;
    Drag d;
    KT_CHECK(!dragMove(&d, 50.f, 50.f, cfg, &vs));       // never begun: no change
    dragBegin(&d, 100.f, 100.f);
    KT_CHECK(d.active && d.axis == 0);
    KT_CHECK(!dragMove(&d, 103.f, 101.f, cfg, &vs));     // inside the lock: still undecided, no change
    KT_CHECK_EQ(d.axis, 0);
    KT_CHECK(dragMove(&d, 120.f, 101.f, cfg, &vs));      // dx 20 from start -> yaw; 17 px since last sample
    KT_CHECK_EQ(d.axis, 1);
    KT_CHECK(nearf(vs.yaw, 8.5f));
    KT_CHECK(!dragMove(&d, 120.f, 160.f, cfg, &vs));     // locked to yaw: vertical move ignored
    KT_CHECK(nearf(vs.yaw, 8.5f) && nearf(vs.panZ, 0.f));
    dragEnd(&d);
    KT_CHECK(!d.active);
    KT_CHECK(!dragMove(&d, 200.f, 200.f, cfg, &vs));     // ended: no change
    KT_CHECK(nearf(vs.yaw, 8.5f));

    // pan axis, divided by zoom; a new drag starts undecided again
    vs = ViewState();
    vs.zoom = 2.f;
    dragBegin(&d, 0.f, 0.f);
    KT_CHECK(dragMove(&d, 1.f, 30.f, cfg, &vs));         // dy 30 beats dx 1 -> pan
    KT_CHECK_EQ(d.axis, 2);
    KT_CHECK(nearf(vs.panZ, 0.15f));                     // 30 * 0.01 / 2
    KT_CHECK(nearf(vs.yaw, 0.f));
    dragEnd(&d);
    dragBegin(&d, 0.f, 0.f);
    KT_CHECK_EQ(d.axis, 0);

    // ---- viewer tokens: ids 1..4, full table returns 0, removed ids are not reused right away ----
    Viewers tv;
    viewersInit(&tv);
    KT_CHECK_EQ(viewersCount(&tv), 0);
    uint32_t a = viewersAdd(&tv), b = viewersAdd(&tv), c = viewersAdd(&tv), e = viewersAdd(&tv);
    KT_CHECK(a == 1u && b == 2u && c == 3u && e == 4u);
    KT_CHECK_EQ(viewersCount(&tv), 4);
    KT_CHECK(viewersAdd(&tv) == 0u);                     // full: no token
    KT_CHECK_EQ(viewersIndex(&tv, 0u), -1);
    KT_CHECK_EQ(viewersIndex(&tv, 3u), 2);
    KT_CHECK(viewersRemove(&tv, 2u));
    KT_CHECK(!viewersRemove(&tv, 2u));                   // already gone
    KT_CHECK(!viewersRemove(&tv, 0u));
    KT_CHECK_EQ(viewersIndex(&tv, 2u), -1);
    KT_CHECK_EQ(viewersCount(&tv), 3);
    uint32_t f = viewersAdd(&tv);
    KT_CHECK(f == 5u);                                   // counter runs on: 2 is not handed out again
    KT_CHECK(viewersIndex(&tv, 5u) >= 0);
    KT_CHECK_EQ(viewersCount(&tv), 4);

    // ---- wrap: 0 is skipped ----
    Viewers wv;
    viewersInit(&wv);
    wv.nextId = 0xFFFFFFFFu;
    uint32_t w1 = viewersAdd(&wv), w2 = viewersAdd(&wv);
    KT_CHECK(w1 == 0xFFFFFFFFu);
    KT_CHECK(w2 == 1u);

    // ---- wrap onto a live token: the live one is skipped, never handed out twice ----
    Viewers lv;
    viewersInit(&lv);
    uint32_t l1 = viewersAdd(&lv);                       // 1, counter now 2
    lv.nextId = 1u;                                      // as if the counter came back round onto the live token
    uint32_t l2 = viewersAdd(&lv);
    KT_CHECK(l1 == 1u && l2 == 2u);
    KT_CHECK_EQ(viewersIndex(&lv, 1u), 0);
    KT_CHECK_EQ(viewersIndex(&lv, 2u), 1);

    return kt::Report("preview3d_logic_test");
}
