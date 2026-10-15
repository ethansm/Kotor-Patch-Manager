// T1 host tests for HolsterQuat.h (the placement math of Holster.cpp; doc 42).
// Build/run: python3 scripts/run_tests.py holster_quat
#include "HolsterQuat.h"
#include "check.h"
#include <cmath>
#include <limits>

using namespace hs;

static bool closeTo(float a, float b) { return std::fabs(a - b) <= 1e-5f; }

// Column j of R (R e_j) must equal the euler_matrix basis column j in .claude/waves/w1-42-HWB/u05.md (rows printed as m[i][j]).
struct EulerCase { float rx, ry, rz; float m[3][3]; };
static const EulerCase kEuler[] = {
    {0, 0, 0,   {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}},
    {90, 0, 0,  {{1, 0, 0}, {0, 0, -1}, {0, 1, 0}}},
    {0, 90, 0,  {{0, 0, 1}, {0, 1, 0}, {-1, 0, 0}}},
    {0, 0, 90,  {{0, -1, 0}, {1, 0, 0}, {0, 0, 1}}},
    {-90, 0, 0, {{1, 0, 0}, {0, 0, 1}, {0, -1, 0}}},
    {180, 0, 0, {{1, 0, 0}, {0, -1, 0}, {0, 0, -1}}},
};

static void testBaseMatchesEulerMatrix() {
    for (const EulerCase& c : kEuler) {
        const Q q = qbase(c.rx, c.ry, c.rz);
        for (int j = 0; j < 3; ++j) {
            float e[3] = {0.f, 0.f, 0.f};
            e[j] = 1.f;
            float out[3];
            qrotate(q, e, out);
            for (int i = 0; i < 3; ++i) KT_CHECK(closeTo(out[i], c.m[i][j]));
        }
    }
    // -90 about x: +y (barrel) -> -z, +z (saber emitter) -> +y, the same pair test_k1_port_tex_preview.py pins.
    const Q q = qbase(-90.f, 0.f, 0.f);
    float ey[3] = {0.f, 1.f, 0.f}, ez[3] = {0.f, 0.f, 1.f}, out[3];
    qrotate(q, ey, out);
    KT_CHECK(closeTo(out[0], 0.f) && closeTo(out[1], 0.f) && closeTo(out[2], -1.f));
    qrotate(q, ez, out);
    KT_CHECK(closeTo(out[0], 0.f) && closeTo(out[1], 1.f) && closeTo(out[2], 0.f));
}

static void testIdentityPlacementIsNoOp() {
    const Pose node = {{1.f, 2.f, 3.f}, qbase(30.f, -45.f, 60.f)};
    const Placement id = {{0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}};
    const Pose o = applyPlacement(node, id);
    for (int i = 0; i < 3; ++i) KT_CHECK(closeTo(o.pos[i], node.pos[i]));
    KT_CHECK(closeTo(o.q.w, node.q.w) && closeTo(o.q.x, node.q.x) && closeTo(o.q.y, node.q.y) && closeTo(o.q.z, node.q.z));
}

static void testChildOffsetRotatesWithNode() {
    // node rotated 90 deg about z: the local offset (0.2,0,0) becomes (0,0.2,0) in the world.
    const Pose node = {{1.f, 2.f, 3.f}, qbase(0.f, 0.f, 90.f)};
    const Placement pl = {{0.2f, 0.f, 0.f}, {0.f, 0.f, 0.f}};
    const Pose o = applyPlacement(node, pl);
    KT_CHECK(closeTo(o.pos[0], 1.f) && closeTo(o.pos[1], 2.2f) && closeTo(o.pos[2], 3.f));
    KT_CHECK(closeTo(o.q.w, node.q.w) && closeTo(o.q.x, node.q.x) && closeTo(o.q.y, node.q.y) && closeTo(o.q.z, node.q.z));
}

static void testLocalRotationAppliesAfterNode() {
    // node Rz(90), local Rx(90). Applied to e_y: Rz90 * Rx90 * e_y = Rz90 * e_z = e_z. The reversed order would give -e_x.
    const Pose node = {{0.f, 0.f, 0.f}, qbase(0.f, 0.f, 90.f)};
    const Placement pl = {{0.f, 0.f, 0.f}, {90.f, 0.f, 0.f}};
    const Pose o = applyPlacement(node, pl);
    float e[3] = {0.f, 1.f, 0.f}, out[3];
    qrotate(o.q, e, out);
    KT_CHECK(closeTo(out[0], 0.f) && closeTo(out[1], 0.f) && closeTo(out[2], 1.f));
}

static void testOutputQuatIsUnit() {
    const Pose unitNode = {{0.f, 0.f, 0.f}, qbase(10.f, 20.f, 30.f)};
    const Placement pl = {{0.1f, -0.2f, 0.3f}, {45.f, -30.f, 170.f}};
    const Pose a = applyPlacement(unitNode, pl);
    float na = std::sqrt(a.q.w * a.q.w + a.q.x * a.q.x + a.q.y * a.q.y + a.q.z * a.q.z);
    KT_CHECK(closeTo(na, 1.f));
    // a non-unit node quaternion must still give a unit output
    const Pose scaledNode = {{0.f, 0.f, 0.f}, {2.f, 0.f, 0.f, 0.f}};
    const Pose b = applyPlacement(scaledNode, pl);
    float nb = std::sqrt(b.q.w * b.q.w + b.q.x * b.q.x + b.q.y * b.q.y + b.q.z * b.q.z);
    KT_CHECK(closeTo(nb, 1.f));
    KT_CHECK(poseSane(a) && poseSane(b));
}

static void testPoseSane() {
    const Pose ok = {{1.f, 2.f, 3.f}, qbase(10.f, 20.f, 30.f)};
    KT_CHECK(poseSane(ok));
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float inf = std::numeric_limits<float>::infinity();
    Pose p = ok;
    p.pos[1] = nan;
    KT_CHECK(!poseSane(p));
    p = ok;
    p.pos[2] = inf;
    KT_CHECK(!poseSane(p));
    p = ok;
    p.q.x = nan;
    KT_CHECK(!poseSane(p));
    p = ok;
    p.q.w = -inf;
    KT_CHECK(!poseSane(p));
    p = ok;
    p.q = {2.f, 0.f, 0.f, 0.f};          // length 2: not unit
    KT_CHECK(!poseSane(p));
    p.q = {1.002f, 0.f, 0.f, 0.f};       // length off by 2e-3: outside the 1e-3 tolerance
    KT_CHECK(!poseSane(p));
    p.q = {1.0005f, 0.f, 0.f, 0.f};      // length off by 5e-4: inside the tolerance
    KT_CHECK(poseSane(p));
}

int main() {
    testBaseMatchesEulerMatrix();
    testIdentityPlacementIsNoOp();
    testChildOffsetRotatesWithNode();
    testLocalRotationAppliesAfterNode();
    testOutputQuatIsUnit();
    testPoseSane();
    return kt::Report("holster_quat");
}
