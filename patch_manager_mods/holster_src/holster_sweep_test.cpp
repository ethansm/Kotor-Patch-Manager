// Host-only offline draw sweep (HW32 / W6b; doc 42). Research tool, not shipped: one simulated draw per family x mount x hand x via.
// Position and orientation follow the runtime carry: the weapon sits on the mount while carryW is 0, then blendPoseLocked2 with
// fPos = carryW and fRot = oriF (held at 0 until carryW > 0), as Holster.cpp:1637-1662 and HolsterReach.h do. The static mount pose
// (carryW 0) is reported separately. Every back-mount number depends on the GUESS mount quat (45 deg, or --back-deg). Head, torso,
// orientation and mount proxies are GUESSes unless marked MEASURED. Draw only (no stow). Numbers: .claude/research/holster_hw32/README.md
// Build/run: python3 scripts/run_tests.py holster_sweep                                      (asserts)
//            <binary> --table                                                               (per-category table, stdout)
//            <binary> --tsv <path> [--table] [--back-deg <d>] [--extract-back <f>] [--extract-hip <f>]   (TSV, no asserts)
#include "HolsterReach.h"
#include "HolsterShape.h"
#include "check.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdint.h>
#include <string>
#include <vector>

using namespace hs;

namespace {

constexpr float kPi = 3.14159265358979f;
constexpr float kL1 = 0.298f, kL2 = 0.332f;          // upper arm / forearm, derived (r3_blend_proxies.md:55)
constexpr float kHeadR = 0.105f;                       // GUESS head sphere radius (r3_blend_proxies.md:61)
constexpr float kTorsoR = 0.13f;                       // GUESS torso capsule radius (r3_blend_proxies.md:62)
constexpr uint32_t kDrawMs = 520u, kLastMs = 519u;     // draw length; last sampled tick
constexpr int kSteps = 200;                            // samples per draw, i = 0..199 over 0..kLastMs
constexpr int kBack = 0, kHip = 1, kThigh = 2, kMounts = 3;
constexpr int kPartTip = 0, kPartButt = 1, kPartShaft = 2;

const char* const kMountName[kMounts] = { "back", "hip", "thigh" };

// Hand and shoulder points, [hand]: 0 = right, 1 = left.
const float kShoulder[2][3] = { { 0.176f, -0.022f, 1.468f }, { -0.164f, -0.017f, 1.462f } };   // MEASURED (r3_blend_proxies.md:50)
const float kRest[2][3] = { { 0.25f, 0.35f, 1.10f }, { -0.25f, 0.35f, 1.10f } };               // GUESS hand rest pose
const float kOut3[2][3] = { { 1.f, 0.f, 0.f }, { -1.f, 0.f, 0.f } };                           // Holster.cpp:1524 (body-local)
const float kRestPole[3] = { 0.f, -0.5f, -1.f };                                               // GUESS
const float kUpDir[3] = { 0.f, 0.f, 1.f };                                                     // world up (Holster.cpp:1524)
const float kFwdDir[3] = { 0.f, 1.f, 0.f };                                                    // body forward
const float kHeadC[3] = { 0.f, 0.f, 1.50f };                                                   // GUESS (head_g unmeasured)
const float kCapA[3] = { 0.f, 0.f, 1.13f }, kCapB[3] = { 0.f, 0.f, 1.44f };                    // GUESS torso capsule axis

// Mount points, [mount][hand]. Back uses one point for both hands (dblsaber T3 offset, GUESS placement).
const float kMountPos[kMounts][2][3] = {
    { { -0.106f, -0.082f, 1.028f }, { -0.106f, -0.082f, 1.028f } },   // back: torsoUpr_g + T3 offset (r3_blend_proxies.md:68)
    { { 0.15f, -0.05f, 1.08f }, { -0.15f, -0.05f, 1.08f } },          // hip: z MEASURED belt 1.08 (r3:67), x/y GUESS
    { { 0.104f, 0.f, 0.982f }, { -0.104f, 0.f, 0.982f } },            // thigh: x MEASURED (r3:59), y/z GUESS
};

struct SweepOpts {
    float extract[kMounts];   // extract share per mount: back 0.20, hip 0.10 (plan W6), thigh 0.10 (GUESS, not in the plan)
    float backDeg;            // GUESS back mount: long axis at this angle from +z toward +x (45); valid 0..90
};

struct Clear { float min; int step; };   // minimum clearance over the draw and the sample index where it occurs

struct Row {
    std::string family;
    Category cat = CatUnknown;
    int mount = 0;
    int hand = 0;
    bool via = false;
    int status = 0;             // 0 ok, 1 nan (arm unsolved or non-finite), 2 skip_no_shape
    Clear acc[2][3];            // draw: [proxy head, torso][tip, butt, shaft]
    float stat[2] = { 0.f, 0.f };   // static mount pose (carryW 0): min over the 9 shaft samples, [head, torso]
};

// Unit vector along model axis (0 x, 1 y, 2 z).
void unitAxis(int axis, float* u) {
    u[0] = 0.f; u[1] = 0.f; u[2] = 0.f;
    u[axis] = 1.f;
}

// GUESS back mount direction: deg from +z toward +x, in the xz plane (behind the torso).
void backDir(float deg, float* d) {
    const float r = deg * kPi / 180.f;
    d[0] = std::sin(r);
    d[1] = 0.f;
    d[2] = std::cos(r);
}

// Shortest-arc rotation from unit a to unit b. Only called with a = model axis and b in {fwd, up, back direction}: never antiparallel.
Q qFromTo(const float* a, const float* b) {
    const float c[3] = { a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0] };
    const float d = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    const float n = std::sqrt(c[0] * c[0] + c[1] * c[1] + c[2] * c[2]);
    if (n < 1e-6f) return Q{ 1.f, 0.f, 0.f, 0.f };
    const float ax[3] = { c[0] / n, c[1] / n, c[2] / n };
    return qaxis(ax, std::atan2(n, d) * 180.f / kPi);
}

// Family = stem up to and including its last '_' (w_blstrpstl_007 -> w_blstrpstl_).
std::string familyOf(const char* stem) {
    const std::string s(stem);
    const size_t p = s.rfind('_');
    return p == std::string::npos ? s : s.substr(0, p + 1);
}

float headClear(const float* P) {
    const float dx = P[0] - kHeadC[0], dy = P[1] - kHeadC[1], dz = P[2] - kHeadC[2];
    return std::sqrt(dx * dx + dy * dy + dz * dz) - kHeadR;
}

// Distance from P to the capsule axis segment a..b, minus the radius.
float torsoClear(const float* P) {
    float ab[3], ap[3];
    for (int j = 0; j < 3; ++j) { ab[j] = kCapB[j] - kCapA[j]; ap[j] = P[j] - kCapA[j]; }
    const float len2 = ab[0] * ab[0] + ab[1] * ab[1] + ab[2] * ab[2];
    float t = (ap[0] * ab[0] + ap[1] * ab[1] + ap[2] * ab[2]) / len2;
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;
    float d2 = 0.f;
    for (int j = 0; j < 3; ++j) {
        const float q = kCapA[j] + ab[j] * t;
        d2 += (P[j] - q) * (P[j] - q);
    }
    return std::sqrt(d2) - kTorsoR;
}

// World point of shaft sample k (0 butt end .. 8 tip end) for pose w.
void samplePoint(const Pose& w, const ShapeEntry& e, const float* u, int k, float* P) {
    const float sk = -1.f + 0.25f * static_cast<float>(k);
    float loc[3], r[3];
    for (int j = 0; j < 3; ++j) loc[j] = e.c[j] + u[j] * (0.5f * e.len) * sk;
    qrotate(w.q, loc, r);
    for (int j = 0; j < 3; ++j) P[j] = w.pos[j] + r[j];
}

void update(Clear& c, float v, int step) {
    if (v < c.min) { c.min = v; c.step = step; }
}

void setNaN(Row& r) {
    const float nan = std::numeric_limits<float>::quiet_NaN();
    for (int k = 0; k < 2; ++k)
        for (int p = 0; p < 3; ++p) r.acc[k][p] = Clear{ nan, -1 };
    r.stat[0] = nan;
    r.stat[1] = nan;
}

// One draw. Returns false when reachHinge fails or a clearance is non-finite (the case is then NaN).
bool simulate(const ShapeEntry& e, int mount, int hand, bool viaOn, float extract, float backDeg, Row& row) {
    float u[3];
    unitAxis(e.axis, u);
    const Q qHand = qFromTo(u, kFwdDir);     // GUESS: model long axis -> body forward (+y)
    const Q qVia = qFromTo(u, kUpDir);       // GUESS: model long axis -> world up (+z); weapon outside the shoulder not modelled
    float bd[3];
    backDir(backDeg, bd);
    Pose mp;
    for (int j = 0; j < 3; ++j) mp.pos[j] = kMountPos[mount][hand][j];
    mp.q = (mount == kBack) ? qFromTo(u, bd) : Q{ 1.f, 0.f, 0.f, 0.f };   // GUESS: back diagonal; hip/thigh identity

    // Static mount pose: the weapon sits on the mount while carryW is 0 (reach phase in the runtime).
    row.stat[0] = std::numeric_limits<float>::infinity();
    row.stat[1] = std::numeric_limits<float>::infinity();
    for (int k = 0; k < 9; ++k) {
        float P[3];
        samplePoint(mp, e, u, k, P);
        const float clr[2] = { headClear(P), torsoClear(P) };
        for (int pr = 0; pr < 2; ++pr) {
            if (!std::isfinite(clr[pr])) return false;
            if (clr[pr] < row.stat[pr]) row.stat[pr] = clr[pr];
        }
    }

    Slide s;
    s.reset(false, 0);
    s.target(true, 0, kDrawMs, kDrawMs, 0.f);
    for (int k = 0; k < 2; ++k)
        for (int p = 0; p < 3; ++p) row.acc[k][p] = Clear{ std::numeric_limits<float>::infinity(), -1 };

    for (int i = 0; i < kSteps; ++i) {
        const uint32_t now = static_cast<uint32_t>(i) * kLastMs / static_cast<uint32_t>(kSteps - 1);
        const ReachPlan pl = reachPlan(s, now, 0.3f, extract);
        if (!pl.active) continue;
        float T[3] = { 0.f, 0.f, 0.f }, pole[3] = { 0.f, 0.f, 0.f };
        if (!reachHinge(kShoulder[hand], kL1, kL2, kUpDir, kFwdDir, kOut3[hand], mp.pos, kRest[hand], kRestPole, pl.pathF, T, pole)) return false;
        // Extract offset on the hand target, after the arm solve (plan W7 applies it before the solve; see README).
        // Skipped for exitSign 0 (29 of 39 families: no measured exit direction).
        if (e.exitSign != 0 && pl.extractK > 0.f) {
            const float sg = static_cast<float>(e.exitSign);
            const float us[3] = { u[0] * sg, u[1] * sg, u[2] * sg };
            float r[3];
            qrotate(mp.q, us, r);
            for (int j = 0; j < 3; ++j) T[j] += r[j] * e.exitLen * pl.extractK;
        }
        Pose hp;
        for (int j = 0; j < 3; ++j) hp.pos[j] = T[j];
        hp.q = qHand;
        // Runtime carry: the weapon stays on the mount while carryW is 0; orientation holds until carryW > 0 (Holster.cpp:1637-1662).
        const float fRot = pl.carryW > 0.f ? pl.oriF : 0.f;
        const Pose w = blendPoseLocked2(s, mp, hp, pl.carryW, fRot, viaOn ? &qVia : nullptr);
        for (int k = 0; k < 9; ++k) {
            float P[3];
            samplePoint(w, e, u, k, P);
            const float clr[2] = { headClear(P), torsoClear(P) };
            for (int pr = 0; pr < 2; ++pr) {
                if (!std::isfinite(clr[pr])) return false;
                update(row.acc[pr][kPartShaft], clr[pr], i);
                if (k == 8) update(row.acc[pr][kPartTip], clr[pr], i);
                if (k == 0) update(row.acc[pr][kPartButt], clr[pr], i);
            }
        }
    }
    return true;
}

// One row per (family, mount, hand, via), built from the first kShapes row of each family. Families are contiguous (table is byte-sorted).
void runSweep(const SweepOpts& o, std::vector<Row>& rows, int& families, int& skipped) {
    families = 0;
    skipped = 0;
    const int n = static_cast<int>(kShapeCount);
    int i = 0;
    while (i < n) {
        const std::string fam = familyOf(kShapes[i].stem);
        int j = i;
        while (j < n && familyOf(kShapes[j].stem) == fam) ++j;
        ++families;
        const bool hasShape = shapeOf(kShapes[i].stem) != nullptr;
        if (!hasShape) ++skipped;
        const Category cat = classify(kShapes[i].stem, 0);
        for (int mount = 0; mount < kMounts; ++mount)
            for (int hand = 0; hand < 2; ++hand)
                for (int v = 0; v < 2; ++v) {
                    Row r;
                    r.family = fam;
                    r.cat = cat;
                    r.mount = mount;
                    r.hand = hand;
                    r.via = v == 1;
                    if (!hasShape) {
                        r.status = 2;
                        setNaN(r);
                    } else {
                        const bool ok = simulate(kShapes[i], mount, hand, r.via, o.extract[mount], o.backDeg, r);
                        r.status = ok ? 0 : 1;
                        if (!ok) setNaN(r);
                    }
                    rows.push_back(r);
                }
        i = j;
    }
}

const char* statusName(int s) {
    return s == 0 ? "ok" : (s == 1 ? "nan" : "skip_no_shape");
}

bool writeTsv(const char* path, const std::vector<Row>& rows) {
    FILE* f = std::fopen(path, "w");
    if (!f) return false;
    std::fprintf(f, "family\tmount\thand\tvia\tstatus\thead_tip\thead_tip_step\thead_butt\thead_butt_step\thead_shaft\thead_shaft_step"
                    "\ttorso_tip\ttorso_tip_step\ttorso_butt\ttorso_butt_step\ttorso_shaft\ttorso_shaft_step\tstatic_head\tstatic_torso\n");
    for (const Row& r : rows) {
        std::fprintf(f, "%s\t%s\t%s\t%s\t%s", r.family.c_str(), kMountName[r.mount], r.hand == 0 ? "R" : "L", r.via ? "on" : "off", statusName(r.status));
        for (int pr = 0; pr < 2; ++pr)
            for (int p = 0; p < 3; ++p) {
                const Clear& c = r.acc[pr][p];
                if (r.status != 0) std::fprintf(f, "\tnan\t-1");
                else std::fprintf(f, "\t%.4f\t%d", static_cast<double>(c.min), c.step);
            }
        if (r.status != 0) std::fprintf(f, "\tnan\tnan\n");
        else std::fprintf(f, "\t%.4f\t%.4f\n", static_cast<double>(r.stat[0]), static_cast<double>(r.stat[1]));
    }
    return std::fclose(f) == 0;
}

const Row* findRow(const std::vector<Row>& rows, const char* fam, int mount, int hand, bool via) {
    for (const Row& r : rows)
        if (r.family == fam && r.mount == mount && r.hand == hand && r.via == via) return &r;
    return nullptr;
}

// Minimum over tip, butt and shaft, head and torso of the draw. NaN when the row is missing or any value is non-finite.
float caseMin(const Row* r) {
    if (!r) return std::numeric_limits<float>::quiet_NaN();
    float m = std::numeric_limits<float>::infinity();
    for (int pr = 0; pr < 2; ++pr)
        for (int p = 0; p < 3; ++p) {
            const float v = r->acc[pr][p].min;
            if (!std::isfinite(v)) return std::numeric_limits<float>::quiet_NaN();
            if (v < m) m = v;
        }
    return m;
}

// Minimum over the draw and the static mount pose.
float allMin(const Row* r) {
    const float d = caseMin(r);
    if (!std::isfinite(d) || !std::isfinite(r->stat[0]) || !std::isfinite(r->stat[1])) return std::numeric_limits<float>::quiet_NaN();
    const float s = r->stat[0] < r->stat[1] ? r->stat[0] : r->stat[1];
    return d < s ? d : s;
}

// True when any draw column of a and b differs (the static pose does not depend on via, so the draw is what must move).
bool drawDiffers(const Row* a, const Row* b) {
    if (!a || !b) return false;
    for (int pr = 0; pr < 2; ++pr)
        for (int p = 0; p < 3; ++p)
            if (std::fabs(a->acc[pr][p].min - b->acc[pr][p].min) > 1e-6f) return true;
    return false;
}

// Min over rows of value(row), NaN if any value is non-finite or no row matches.
template <typename F>
float minOver(const std::vector<Row>& rows, Category cat, int mount, int viaSel, F value) {
    float m = std::numeric_limits<float>::infinity();
    bool any = false;
    for (const Row& r : rows) {
        if (r.cat != cat || r.mount != mount || (viaSel >= 0 && r.via != (viaSel == 1))) continue;
        const float v = value(r);
        any = true;
        if (!std::isfinite(v)) return std::numeric_limits<float>::quiet_NaN();
        if (v < m) m = v;
    }
    return any ? m : std::numeric_limits<float>::quiet_NaN();
}

// Per-category table (stdout): min over families and hands. draw = all draw columns, via off or on; static = the mount pose.
void printTable(const std::vector<Row>& rows) {
    const Category order[] = { CatLightsaber, CatMelee, CatQStaff, CatPistol, CatRifle, CatHeavy, CatUnknown };
    std::printf("%-10s", "category");
    for (int m = 0; m < kMounts; ++m)
        std::printf(" %s_off %s_on %s_static", kMountName[m], kMountName[m], kMountName[m]);
    std::printf("\n");
    for (Category c : order) {
        std::printf("%-10s", c == CatUnknown ? "unknown" : categoryName(c));
        for (int m = 0; m < kMounts; ++m) {
            const float off = minOver(rows, c, m, 0, [](const Row& r) { return caseMin(&r); });
            const float on = minOver(rows, c, m, 1, [](const Row& r) { return caseMin(&r); });
            const float st = minOver(rows, c, m, 0, [](const Row& r) { return r.stat[0] < r.stat[1] ? r.stat[0] : r.stat[1]; });
            std::printf(" %+.4f %+.4f %+.4f", static_cast<double>(off), static_cast<double>(on), static_cast<double>(st));
        }
        std::printf("\n");
    }
}

}  // namespace

int main(int argc, char** argv) {
    SweepOpts o = { { 0.2f, 0.1f, 0.1f }, 45.f };
    const char* tsv = nullptr;
    bool table = false, overridden = false;
    for (int a = 1; a < argc; ++a) {
        const bool more = a + 1 < argc;
        if (!std::strcmp(argv[a], "--tsv") && more) tsv = argv[++a];
        else if (!std::strcmp(argv[a], "--table")) table = true;
        else if (!std::strcmp(argv[a], "--extract-back") && more) { o.extract[kBack] = static_cast<float>(std::atof(argv[++a])); overridden = true; }
        else if (!std::strcmp(argv[a], "--extract-hip") && more) { o.extract[kHip] = static_cast<float>(std::atof(argv[++a])); overridden = true; }
        else if (!std::strcmp(argv[a], "--back-deg") && more) { o.backDeg = static_cast<float>(std::atof(argv[++a])); overridden = true; }
        else {
            std::fprintf(stderr, "usage: holster_sweep_test [--table] [--tsv <path>] [--back-deg <d>] [--extract-back <f>] [--extract-hip <f>]\n");
            return 2;
        }
    }
    if (overridden && !tsv && !table) {
        std::fprintf(stderr, "--back-deg / --extract-back / --extract-hip need --tsv or --table (the asserts assume the defaults)\n");
        return 2;
    }

    std::vector<Row> rows;
    int families = 0, skipped = 0;
    runSweep(o, rows, families, skipped);

    if (tsv) {
        if (!writeTsv(tsv, rows)) {
            std::fprintf(stderr, "cannot write %s\n", tsv);
            return 1;
        }
        std::printf("wrote %s: %zu rows, %d families, %d skipped\n", tsv, rows.size(), families, skipped);
    }
    if (table) printTable(rows);
    if (tsv || table) return 0;

    // Key case: qtrstaff on the back, right hand, via off. It clips under the GUESS 45 deg mount quat (static pose and draw).
    const Row* keyOff = findRow(rows, "w_qtrstaff_", kBack, 0, false);
    const Row* keyOn = findRow(rows, "w_qtrstaff_", kBack, 0, true);
    KT_CHECK(keyOff != nullptr && allMin(keyOff) < 0.f);
    // Via must change the draw. Its sign is not asserted (README).
    KT_CHECK(drawDiffers(keyOff, keyOn));

    // Every pistol on the hip is clear, draw and static, both hands and both via states. Pistols = CatPistol (HolsterLogic.h:66-67).
    int pistolCases = 0;
    bool pistolClear = true;
    for (const Row& r : rows) {
        if (r.cat != CatPistol || r.mount != kHip) continue;
        ++pistolCases;
        if (!(allMin(&r) > 0.f)) pistolClear = false;
    }
    KT_CHECK(pistolCases > 0);
    KT_CHECK(pistolClear);

    KT_CHECK(skipped == 0);
    return kt::Report("holster_sweep");
}
