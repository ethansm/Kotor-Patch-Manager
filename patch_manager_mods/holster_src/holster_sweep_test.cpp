// Host-only offline slide sweep (HW32 / W6b, HW32c; doc 42). Research tool, not shipped: one simulated slide per family x mount x hand x via x dir.
// The weapon follows the runtime carry (Holster.cpp carryShownBase, HolsterSlide.h hs::slideResid*, hs::slideViaProgress): on the mount while
// carryW is 0; then the hand-frame residual (ResidDraw / ResidStow, reachResidualVia with the via in the hand frame) when that branch applies,
// else the world blend (carryBlend). The aimed point is reachHinge's point, plus the via's lateral bump while the via is on, plus the extract
// lift, and the two-bone solve clamps it to reach before the hand is placed. The static mount pose (carryW 0) is reported separately.
// Back-mount numbers depend on the mount quat: the GUESS (qFromTo of --back-deg, default 45), --back-quat, or a logged rel from --mounts-json
// (src column). Head, torso, orientation and mount proxies are GUESSes unless marked MEASURED. Numbers: .claude/research/holster_hw32/README.md
// Build/run: python3 scripts/run_tests.py holster_sweep                                      (asserts)
//            <binary> --table [--dir draw|stow|both]                                        (per-category table, stdout)
//            <binary> --tsv <path> [--table] [--dir ...] [--back-deg <d> | --back-quat w,x,y,z | --mounts-json <file>]
//                     [--extract-back <f>] [--extract-hip <f>] [--pole-grip <a>]           (TSV, no asserts)
#include "HolsterReach.h"
#include "HolsterShape.h"
#include "check.h"
#include <cctype>
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
constexpr uint32_t kDrawMs = 520u, kLastMs = 519u;     // draw length (melee / rifle, HW28 table); last sampled tick
constexpr uint32_t kStowMs = 600u, kStowLastMs = 599u; // stow length (melee / rifle, HW28 table); last sampled tick
constexpr int kSteps = 200;                            // samples per slide, i = 0..199 over 0..last
constexpr float kGrab = 0.3f;                          // grab share: reachPlan g and the stow window (the sweep's fixed value)
constexpr int kBack = 0, kHip = 1, kThigh = 2, kMounts = 3;
constexpr int kPartTip = 0, kPartButt = 1, kPartShaft = 2;
constexpr int kDirDraw = 0, kDirStow = 1, kDirBoth = 2;      // SweepOpts::dirSel; a row's dir is kDirDraw or kDirStow
constexpr int kSrcNone = 0, kSrcLog = 1, kSrcGuess = 2, kSrcQuat = 3;   // Row::src (back mounts only)

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

// One logged mount entry (holster_log_check.py --mounts --json). rel is normalized when hasRel; cat is the category name.
struct MountRec {
    std::string cat, bone;
    int count = 0;
    bool hasRel = false;
    Q rel{ 1.f, 0.f, 0.f, 0.f };
};

struct SweepOpts {
    float extract[kMounts] = { 0.f, 0.f, 0.f };   // extract share per mount (defaults: back 0.20, hip 0.10, thigh 0.10 in defaultOpts)
    float backDeg = 45.f;                          // GUESS back mount: long axis at this angle from +z toward +x (45); valid 0..90
    bool backQuatSet = false;
    Q backQuat{ 1.f, 0.f, 0.f, 0.f };             // --back-quat: a body-relative mount quat for back mounts (overrides backDeg)
    bool mountsSet = false;
    std::vector<MountRec> mounts;                  // --mounts-json: back-mount rel per (cat, bone); missing -> the GUESS
    int dirSel = kDirBoth;                         // --dir
    float poleGrip = 0.f;                          // --pole-grip: HW29b pole bias toward the grip, hip and thigh only (0..1)
    float viaOut = kReachViaOutM;                  // --via-out: lateral shoulder offset of the via pose, m (0..0.30)
    float slowX = 1.f;                             // --slow: slow_holster_x factor scaling draw, stow and residual ms (1..8)
};

SweepOpts defaultOpts() {
    SweepOpts o;
    o.extract[kBack] = 0.2f;
    o.extract[kHip] = 0.1f;
    o.extract[kThigh] = 0.1f;
    return o;
}

struct Clear { float min; int step; };   // minimum clearance over the slide and the sample index where it occurs

struct Row {
    std::string family;
    Category cat = CatUnknown;
    int mount = 0;
    int hand = 0;
    bool via = false;
    int dir = kDirDraw;
    int src = kSrcNone;         // back mounts: log | quat | guess; hip and thigh: none
    bool resid = false;         // some frame took the hand-frame residual branch (ResidDraw or ResidStow)
    int status = 0;             // 0 ok, 1 nan (arm unsolved or non-finite), 2 skip_no_shape
    Clear acc[2][3];            // slide: [proxy head, torso][tip, butt, shaft]
    float stat[2] = { 0.f, 0.f };   // static mount pose (carryW 0): min over the 9 shaft samples, [head, torso]
};

// HW32c carry capture (Holster.cpp CarryResid, draw fields): the hand-frame offset to the mount, taken on the first carry frame of a draw.
struct ResidCap { Q q; float pos[3]; uint32_t startMs; uint32_t msEff; uint8_t on; };

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

// --back-quat "w,x,y,z": four finite numbers with a non-zero norm. Normalized on success; out is untouched on failure.
bool parseQuatArg(const char* s, Q& out) {
    float v[4];
    const char* p = s;
    for (int i = 0; i < 4; ++i) {
        char* end = nullptr;
        const double d = std::strtod(p, &end);
        if (end == p || !std::isfinite(d)) return false;
        v[i] = static_cast<float>(d);
        p = end;
        if (i < 3) {
            if (*p != ',') return false;
            ++p;
        }
    }
    if (*p != 0) return false;
    const float n = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
    if (!(n > 1e-6f) || !std::isfinite(n)) return false;
    out = Q{ v[0] / n, v[1] / n, v[2] / n, v[3] / n };
    return true;
}

// ---- --mounts-json reader (no dependencies). Reads JSON values (objects, arrays, strings, numbers, true / false / null); the mount keys are
// checked and other keys are skipped. The holster_log_check.py emitter writes cat as the Category index (0..6); a name is accepted too.

struct JVal {
    enum Kind { KNull, KBool, KNum, KStr, KArr, KObj } kind = KNull;
    double num = 0.0;
    std::string str;
    std::vector<JVal> arr;               // KArr elements
    std::vector<std::string> keys;       // KObj: keys[i] -> vals[i]
    std::vector<JVal> vals;
};

void jsonWs(const char*& p) {
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') ++p;
}

// Escapes other than \" \\ \/ are rejected (exit 2). The emitter writes ASCII names only, so the valid escapes \n \t \b \f \r \uXXXX
// are not needed; a bone or category name with one is a documented limit, not a crash.
bool jsonString(const char*& p, std::string& s) {
    if (*p != '"') return false;
    ++p;
    s.clear();
    while (*p != '"') {
        if (*p == 0) return false;
        if (*p == '\\') {
            ++p;
            if (*p != '"' && *p != '\\' && *p != '/') return false;
        }
        s.push_back(*p++);
    }
    ++p;
    return true;
}

bool jsonValue(const char*& p, JVal& v, int depth) {
    if (depth > 8) return false;
    jsonWs(p);
    if (*p == '"') {
        v.kind = JVal::KStr;
        return jsonString(p, v.str);
    }
    if (*p == '[') {
        v.kind = JVal::KArr;
        ++p;
        jsonWs(p);
        if (*p == ']') { ++p; return true; }
        for (;;) {
            JVal e;
            if (!jsonValue(p, e, depth + 1)) return false;
            v.arr.push_back(e);
            jsonWs(p);
            if (*p == ',') { ++p; continue; }
            if (*p == ']') { ++p; return true; }
            return false;
        }
    }
    if (*p == '{') {
        v.kind = JVal::KObj;
        ++p;
        jsonWs(p);
        if (*p == '}') { ++p; return true; }
        for (;;) {
            jsonWs(p);
            std::string k;
            if (!jsonString(p, k)) return false;
            jsonWs(p);
            if (*p != ':') return false;
            ++p;
            JVal e;
            if (!jsonValue(p, e, depth + 1)) return false;
            v.keys.push_back(k);
            v.vals.push_back(e);
            jsonWs(p);
            if (*p == ',') { ++p; continue; }
            if (*p == '}') { ++p; return true; }
            return false;
        }
    }
    if (!std::strncmp(p, "null", 4)) { p += 4; v.kind = JVal::KNull; return true; }
    if (!std::strncmp(p, "true", 4)) { p += 4; v.kind = JVal::KBool; return true; }
    if (!std::strncmp(p, "false", 5)) { p += 5; v.kind = JVal::KBool; return true; }
    // number: -?digits(.digits)?([eE][+-]?digits)?
    const char* q = p;
    if (*q == '-') ++q;
    if (!std::isdigit(static_cast<unsigned char>(*q))) return false;
    while (std::isdigit(static_cast<unsigned char>(*q))) ++q;
    if (*q == '.') {
        ++q;
        if (!std::isdigit(static_cast<unsigned char>(*q))) return false;
        while (std::isdigit(static_cast<unsigned char>(*q))) ++q;
    }
    if (*q == 'e' || *q == 'E') {
        ++q;
        if (*q == '+' || *q == '-') ++q;
        if (!std::isdigit(static_cast<unsigned char>(*q))) return false;
        while (std::isdigit(static_cast<unsigned char>(*q))) ++q;
    }
    const std::string tok(p, q);
    v.num = std::strtod(tok.c_str(), nullptr);
    if (!std::isfinite(v.num)) return false;
    v.kind = JVal::KNum;
    p = q;
    return true;
}

const JVal* findKey(const JVal& obj, const char* key) {
    for (size_t i = 0; i < obj.keys.size(); ++i)
        if (obj.keys[i] == key) return &obj.vals[i];
    return nullptr;
}

// Entries with count 0 are dropped (nothing was logged). Required per entry: cat, bone, count, rel (null or [w,x,y,z]). spread and nofacing are
// optional and type-checked when present. A null rel keeps the entry out of use (the GUESS stays). Any error -> false with err set.
bool parseMountsText(const std::string& text, std::vector<MountRec>& out, std::string& err) {
    out.clear();
    const char* p = text.c_str();
    JVal root;
    if (!jsonValue(p, root, 0)) {
        err = "malformed JSON at byte " + std::to_string(static_cast<long>(p - text.c_str()));
        return false;
    }
    jsonWs(p);
    if (*p != 0) {
        err = "trailing text at byte " + std::to_string(static_cast<long>(p - text.c_str()));
        return false;
    }
    if (root.kind != JVal::KArr) {
        err = "top level must be an array of mount entries";
        return false;
    }
    std::vector<MountRec> recs;
    for (size_t i = 0; i < root.arr.size(); ++i) {
        const JVal& e = root.arr[i];
        const std::string where = "entry " + std::to_string(i) + ": ";
        if (e.kind != JVal::KObj) { err = where + "not an object"; return false; }
        const JVal* cat = findKey(e, "cat");
        const JVal* bone = findKey(e, "bone");
        const JVal* count = findKey(e, "count");
        const JVal* rel = findKey(e, "rel");
        MountRec m;
        if (cat && cat->kind == JVal::KStr) {
            m.cat = cat->str;
        } else if (cat && cat->kind == JVal::KNum && cat->num == std::floor(cat->num) && cat->num >= 0.0 && cat->num < static_cast<double>(CatCount)) {
            m.cat = categoryName(static_cast<Category>(static_cast<int>(cat->num)));
        } else {
            err = where + "cat must be a category name or its index 0..6";
            return false;
        }
        if (!bone || bone->kind != JVal::KStr) { err = where + "bone must be a string"; return false; }
        if (!count || count->kind != JVal::KNum || count->num < 0.0 || count->num > 1e9 || count->num != std::floor(count->num)) {
            err = where + "count must be a whole number 0..1e9";
            return false;
        }
        m.bone = bone->str;
        m.count = static_cast<int>(count->num);
        if (!rel || (rel->kind != JVal::KNull && rel->kind != JVal::KArr)) { err = where + "rel must be null or [w,x,y,z]"; return false; }
        if (rel->kind == JVal::KArr) {
            if (rel->arr.size() != 4) { err = where + "rel must have 4 numbers"; return false; }
            float v[4];
            for (int k = 0; k < 4; ++k) {
                if (rel->arr[k].kind != JVal::KNum) { err = where + "rel must hold numbers only"; return false; }
                v[k] = static_cast<float>(rel->arr[k].num);
            }
            const float n = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
            if (!(n > 1e-6f) || !std::isfinite(n)) { err = where + "rel has zero norm"; return false; }
            m.hasRel = true;
            m.rel = Q{ v[0] / n, v[1] / n, v[2] / n, v[3] / n };
        }
        const JVal* spread = findKey(e, "spread");
        if (spread && spread->kind != JVal::KNum && spread->kind != JVal::KNull) { err = where + "spread must be a number or null"; return false; }
        const JVal* nofacing = findKey(e, "nofacing");
        if (nofacing && nofacing->kind != JVal::KNum) { err = where + "nofacing must be a number"; return false; }
        if (m.count > 0) recs.push_back(m);
    }
    out = recs;
    return true;
}

bool loadMountsJson(const char* path, std::vector<MountRec>& out, std::string& err) {
    FILE* f = std::fopen(path, "rb");
    if (!f) { err = "cannot open"; return false; }
    std::string text;
    char buf[4096];
    size_t n = 0;
    while ((n = std::fread(buf, 1, sizeof buf, f)) > 0) text.append(buf, n);
    std::fclose(f);
    return parseMountsText(text, out, err);
}

// The back bones (torsoUpr_g, torso_g): reachIsBackBone, Holster.cpp:1470, which is _stricmp, so the match ignores ASCII case.
bool isBackBone(const std::string& b) {
    std::string l = b;
    for (char& c : l) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return l == "torsoupr_g" || l == "torso_g";
}

// Back-mount rel for a category: the (cat, bone) entry with the most samples over the back bones. A tie keeps the first entry. False when no
// usable entry exists.
bool logBackRel(const std::vector<MountRec>& recs, Category cat, Q& out) {
    const std::string name = categoryName(cat);
    const MountRec* best = nullptr;
    for (const MountRec& r : recs) {
        if (!r.hasRel || r.cat != name) continue;
        if (!isBackBone(r.bone)) continue;
        if (!best || r.count > best->count) best = &r;
    }
    if (!best) return false;
    out = best->rel;
    return true;
}

// Source of a back-mount quat (same precedence as backQuat): log, then --back-quat, then the GUESS.
int backSrc(const SweepOpts& o, Category cat) {
    Q q{ 1.f, 0.f, 0.f, 0.f };
    if (o.mountsSet && logBackRel(o.mounts, cat, q)) return kSrcLog;
    return o.backQuatSet ? kSrcQuat : kSrcGuess;
}

// Back-mount quat for a category and model axis u: the log entry, else --back-quat, else the GUESS (qFromTo of --back-deg).
Q backQuat(const SweepOpts& o, const float* u, Category cat) {
    Q q{ 1.f, 0.f, 0.f, 0.f };
    if (o.mountsSet && logBackRel(o.mounts, cat, q)) return q;
    if (o.backQuatSet) return o.backQuat;
    float bd[3];
    backDir(o.backDeg, bd);
    return qFromTo(u, bd);
}

const char* statusName(int s) {
    return s == 0 ? "ok" : (s == 1 ? "nan" : "skip_no_shape");
}

const char* srcName(int s) {
    return s == kSrcLog ? "log" : (s == kSrcGuess ? "guess" : (s == kSrcQuat ? "quat" : "-"));
}

const char* dirName(int d) {
    return d == kDirStow ? "stow" : "draw";
}

const char* pathName(const Row& r) {
    if (r.status == 2) return "-";
    return r.resid ? "resid" : "blend";
}

// GUESS animation elbow for the solve's lengths (|E-S| = kL1, |H-E| = kL2 with H = kRest[hand]), on the kRestPole side.
void restElbow(int hand, float* E) {
    const float* S = kShoulder[hand];
    const float* H = kRest[hand];
    float d[3], dir[3];
    for (int j = 0; j < 3; ++j) d[j] = H[j] - S[j];
    const float D = std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
    for (int j = 0; j < 3; ++j) dir[j] = d[j] / D;
    const float a = (kL1 * kL1 - kL2 * kL2 + D * D) / (2.f * D);
    const float dp = kRestPole[0] * dir[0] + kRestPole[1] * dir[1] + kRestPole[2] * dir[2];
    float pp[3];
    for (int j = 0; j < 3; ++j) pp[j] = kRestPole[j] - dp * dir[j];
    const float np = std::sqrt(pp[0] * pp[0] + pp[1] * pp[1] + pp[2] * pp[2]);
    const float hh = kL1 * kL1 - a * a;
    const float h = hh > 0.f ? std::sqrt(hh) : 0.f;
    for (int j = 0; j < 3; ++j) E[j] = S[j] + dir[j] * a + (np > 1e-6f ? pp[j] / np : 0.f) * h;
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

// The world blend of the carry (Holster.cpp carryBlend): plain blendPoseLocked when neither the extract share nor the via is on.
Pose carryBlendSweep(Slide& lock, const Pose& mount, const Pose& hand, float carryW, float oriF, bool exOn, const Q* via) {
    if (!exOn && !via) return blendPoseLocked(lock, mount, hand, carryW);
    const float fRot = exOn ? (carryW > 0.f ? oriF : 0.f) : carryW;
    return blendPoseLocked2(lock, mount, hand, carryW, fRot, via);
}

// The hand-frame residual of the carry (Holster.cpp carryShownBase, ResidDraw / ResidStow). Returns false when the pose is not sane (the caller
// falls back to carryBlendSweep). A stow outside its landing window gives the hand pose itself (the weapon is in the hand).
bool carryResidSweep(const Slide& s, ResidCap& cap, SlideResidBranch br, const Pose& mount, const Pose& carried, uint32_t now, float grab,
                     uint32_t residMs, const Q* via, Pose& out) {
    const Q cq = qnormalize(carried.q), inv = reachConj(cq);
    const int32_t d = static_cast<int32_t>(now - s.t0);
    const float resMs = static_cast<float>(residMs);
    float t = 0.f, ms = resMs;
    float off[3] = { 0.f, 0.f, 0.f };
    Q rres{ 1.f, 0.f, 0.f, 0.f };
    if (br == ResidDraw) {   // draw: fixed offset captured at the first carry frame
        if (!cap.on || static_cast<int32_t>(now - cap.startMs) > static_cast<int32_t>(s.len) + 100) {
            const float dm[3] = { mount.pos[0] - carried.pos[0], mount.pos[1] - carried.pos[1], mount.pos[2] - carried.pos[2] };
            cap.q = qmul(inv, mount.q);
            qrotate(inv, dm, cap.pos);
            cap.startMs = now;
            const float left = static_cast<float>(static_cast<int32_t>(s.len) - d);
            cap.msEff = static_cast<uint32_t>(left < resMs ? (left > 1.f ? left : 1.f) : resMs);
            cap.on = 1;
        }
        slideResidDrawWindow(now, cap.startMs, cap.msEff, t, ms);
        rres = cap.q;
        for (int i = 0; i < 3; ++i) off[i] = cap.pos[i];
    } else {                 // stow: the offset from the hand to the mount, weighted by the time left in the carry
        cap.on = 0;
        slideResidStowWindow(now, s.t0, s.len, grab, residMs, t, ms);
        if (!(ms > 1.f) || t >= ms) {
            out = carried;
            return true;
        }
        const float dm[3] = { mount.pos[0] - carried.pos[0], mount.pos[1] - carried.pos[1], mount.pos[2] - carried.pos[2] };
        rres = qmul(inv, mount.q);
        qrotate(inv, dm, off);
    }
    const float k = 1.f - easeSmooth(t / ms);   // share of the offset that is still there
    float o3[3], w3[3];
    for (int i = 0; i < 3; ++i) o3[i] = off[i] * k;
    qrotate(cq, o3, w3);
    Pose o;
    for (int i = 0; i < 3; ++i) o.pos[i] = carried.pos[i] + w3[i];
    // HW31b: with a via, the residual turns through it (the via in the hand frame is conj(carried) * via).
    const Q viaH = via ? qmul(inv, *via) : Q{ 1.f, 0.f, 0.f, 0.f };
    o.q = qnormalize(qmul(cq, via ? reachResidualVia(rres, &viaH, t, ms) : reachResidual(rres, t, ms)));
    if (!poseSane(o)) {
        cap.on = 0;
        return false;
    }
    out = o;
    return true;
}

// One slide (dir kDirDraw or kDirStow). Returns false when reachHinge or the solve fails, or a clearance is non-finite (the case is then NaN).
bool simulate(const ShapeEntry& e, int mount, int hand, bool viaOn, int dir, const SweepOpts& o, Category cat, Row& row) {
    float u[3];
    unitAxis(e.axis, u);
    const Q qHand = qFromTo(u, kFwdDir);     // GUESS: model long axis -> body forward (+y)
    const float extract = o.extract[mount];
    const bool exOn = extract > 0.f;
    Pose mp;
    for (int j = 0; j < 3; ++j) mp.pos[j] = kMountPos[mount][hand][j];
    mp.q = (mount == kBack) ? backQuat(o, u, cat) : Q{ 1.f, 0.f, 0.f, 0.f };
    // The via quat, as Holster.cpp reachViaFor (:1480-1489; static there, so this is a literal copy): the weapon's long axis in the mount
    // frame, turned to world up by arcQuat, then times the mount quat, which keeps the mount's roll. Hip and thigh use the identity mount GUESS.
    const Q mq = qnormalize(mp.q);
    float wv[3];
    qrotate(mq, u, wv);
    const Q qVia = qnormalize(qmul(arcQuat(wv, kUpDir), mq));

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

    const uint32_t drawMs = slowMs(kDrawMs, o.slowX), stowMs = slowMs(kStowMs, o.slowX), residMs = slowMs(kReachResidMs, o.slowX);
    Slide s, lock;
    if (dir == kDirDraw) {
        s.reset(false, 0);
        s.target(true, 0, drawMs, stowMs, 0.f);
    } else {
        s.reset(true, 0);                    // fully in hand, carryFrom 1: a full stow
        s.target(false, 0, drawMs, stowMs, 0.f);
    }
    lock.reset(false, 0);
    const uint32_t lastMs = (dir == kDirDraw ? drawMs : stowMs) - 1u;
    for (int k = 0; k < 2; ++k)
        for (int p = 0; p < 3; ++p) row.acc[k][p] = Clear{ std::numeric_limits<float>::infinity(), -1 };

    float restE[3];
    restElbow(hand, restE);
    const Q* viaP = viaOn ? &qVia : nullptr;
    ResidCap cap{};
    row.resid = false;
    const float arm = kL1 + kL2;
    const float fxA = (mp.pos[0] - kShoulder[hand][0]) / arm;
    const float fyA = (mp.pos[1] - kShoulder[hand][1]) / arm;
    const float dz = mp.pos[2] - kShoulder[hand][2], dzA = dz / arm;
    const ReachTopo topo = reachTopology3D(fxA, fyA, dzA, reachHingeFits(dz, arm));
    const float uW = reachOverWeight3D(fxA, fyA, dzA, topo);
    const ReachStyle st = reachDefaultStyle(static_cast<int>(cat));

    for (int i = 0; i < kSteps; ++i) {
        const uint32_t now = static_cast<uint32_t>(i) * lastMs / static_cast<uint32_t>(kSteps - 1);
        const ReachPlan pl = reachPlan(s, now, kGrab, extract);
        if (!pl.active) continue;
        // HW31b via clock: reads the previous frame's carry capture, as carryShownBase's caller does.
        float viaS = 0.f;
        if (viaOn) viaS = slideViaProgress(pl.active, residMs, pl.carryW, pl.oriF, exOn, s.dir != 0, s.from, s.carryFrom, now, s.t0, s.len,
                                           kGrab, cap.on != 0, cap.startMs, cap.msEff);
        float T[3] = { 0.f, 0.f, 0.f }, pole[3] = { 0.f, 0.f, 0.f };
        // R5 parity with Holster.cpp: back mounts take reachHinge; hip and thigh mounts take the bowed carry target + 3D pole.
        if (mount == kBack) {
            if (!reachHinge(kShoulder[hand], kL1, kL2, kUpDir, kFwdDir, kOut3[hand], mp.pos, kRest[hand], kRestPole, pl.pathF, T, pole)) return false;
        } else {
            float effBulge[3], effOver = uW;
            reachAdaptBulge(st, uW, fxA, fyA, dzA, hand != 0, effBulge, effOver);
            reachCarryTarget(mp.pos, kRest[hand], kUpDir, kFwdDir, kOut3[hand], effBulge, effOver, arm, pl.pathF, T);
            if (!reachPole3D(st, uW, fxA, fyA, dzA, hand != 0, Q{ 1.f, 0.f, 0.f, 0.f }, kRestPole, reachAnimMix(pl.pathF), pole)) return false;
        }
        // HW31b: while the via is on, the aimed point moves outward (the arm's own side) by the via bump (reachViaOffset, 0 at both ends).
        if (viaS > 0.f) {
            const float bump = reachViaOffset(viaS, 1.f, o.viaOut);
            for (int j = 0; j < 3; ++j) T[j] += kOut3[hand][j] * bump;
        }
        // HW31 extract: lift the aimed point out of the mount along the weapon's long axis, before the solve (Holster.cpp:1692). Skipped for
        // exitSign 0 (29 of 39 families: no measured exit direction).
        if (e.exitSign != 0 && pl.extractK > 0.f) {
            const float sg = static_cast<float>(e.exitSign);
            const float us[3] = { u[0] * sg, u[1] * sg, u[2] * sg };
            float r[3];
            qrotate(mp.q, us, r);
            for (int j = 0; j < 3; ++j) T[j] += r[j] * e.exitLen * pl.extractK;
        }
        // HW29b pole bias: hip and thigh only, toward the grip. GUESS grip: the animation forearm (hand -> elbow, hand frame = qHand) carried by the mount.
        if (o.poleGrip > 0.f && mount != kBack) {
            float he[3], hl[3], gw[3];
            for (int j = 0; j < 3; ++j) he[j] = restE[j] - kRest[hand][j];
            qrotate(reachConj(qHand), he, hl);
            qrotate(mp.q, hl, gw);
            const float gl = r3len(gw);
            if (gl > 1e-6f && std::isfinite(gl)) {
                for (int j = 0; j < 3; ++j) gw[j] /= gl;
                const float before[3] = { pole[0], pole[1], pole[2] };
                reachPoleBias(before, gw, pl.pathF, o.poleGrip, kPoleBiasDeg, pole);
            }
        }
        // Two-bone solve: the target is clamped to reach. solveTwoBone's reach is d_clamped / d, so the hand sits at S + (T - S) * reach.
        float En[3], reach = 1.f;
        if (!solveTwoBone(kShoulder[hand], restE, kRest[hand], T, En, &reach, pole)) return false;
        float Hd[3];
        for (int j = 0; j < 3; ++j) Hd[j] = kShoulder[hand][j] + (T[j] - kShoulder[hand][j]) * reach;
        Pose hp;
        for (int j = 0; j < 3; ++j) hp.pos[j] = Hd[j];
        hp.q = qHand;

        // Carry (Holster.cpp carryShownBase): the residual branch when one applies, else the world blend.
        const SlideResidBranch br = slideResidBranch(s.dir != 0, s.from, s.carryFrom, pl.carryW, pl.active, residMs, qfinite(hp.q));
        Pose w{};
        bool took = false;
        if (br != ResidNone) took = carryResidSweep(s, cap, br, mp, hp, now, kGrab, residMs, viaP, w);
        if (took) {
            row.resid = true;
        } else {
            cap.on = 0;
            w = carryBlendSweep(lock, mp, hp, pl.carryW, pl.oriF, exOn, viaP);
        }

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

// One row per (family, dir, mount, hand, via), built from the first kShapes row of each family. Families are contiguous (table is byte-sorted).
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
        for (int dir = kDirDraw; dir <= kDirStow; ++dir) {
            if (o.dirSel != kDirBoth && o.dirSel != dir) continue;
            for (int mount = 0; mount < kMounts; ++mount)
                for (int hand = 0; hand < 2; ++hand)
                    for (int v = 0; v < 2; ++v) {
                        Row r;
                        r.family = fam;
                        r.cat = cat;
                        r.mount = mount;
                        r.hand = hand;
                        r.via = v == 1;
                        r.dir = dir;
                        r.src = mount == kBack ? backSrc(o, cat) : kSrcNone;
                        if (!hasShape) {
                            r.status = 2;
                            setNaN(r);
                        } else {
                            const bool ok = simulate(kShapes[i], mount, hand, r.via, dir, o, cat, r);
                            r.status = ok ? 0 : 1;
                            if (!ok) setNaN(r);
                        }
                        rows.push_back(r);
                    }
        }
        i = j;
    }
}

bool writeTsv(const char* path, const std::vector<Row>& rows) {
    FILE* f = std::fopen(path, "w");
    if (!f) return false;
    std::fprintf(f, "family\tmount\thand\tvia\tstatus\thead_tip\thead_tip_step\thead_butt\thead_butt_step\thead_shaft\thead_shaft_step"
                    "\ttorso_tip\ttorso_tip_step\ttorso_butt\ttorso_butt_step\ttorso_shaft\ttorso_shaft_step\tstatic_head\tstatic_torso\tsrc\tdir\tpath\n");
    for (const Row& r : rows) {
        std::fprintf(f, "%s\t%s\t%s\t%s\t%s", r.family.c_str(), kMountName[r.mount], r.hand == 0 ? "R" : "L", r.via ? "on" : "off", statusName(r.status));
        for (int pr = 0; pr < 2; ++pr)
            for (int p = 0; p < 3; ++p) {
                const Clear& c = r.acc[pr][p];
                if (r.status != 0) std::fprintf(f, "\tnan\t-1");
                else std::fprintf(f, "\t%.4f\t%d", static_cast<double>(c.min), c.step);
            }
        if (r.status != 0) std::fprintf(f, "\tnan\tnan");
        else std::fprintf(f, "\t%.4f\t%.4f", static_cast<double>(r.stat[0]), static_cast<double>(r.stat[1]));
        std::fprintf(f, "\t%s\t%s\t%s\n", srcName(r.src), dirName(r.dir), pathName(r));
    }
    return std::fclose(f) == 0;
}

const Row* findRow(const std::vector<Row>& rows, const char* fam, int mount, int hand, bool via, int dir) {
    for (const Row& r : rows)
        if (r.family == fam && r.mount == mount && r.hand == hand && r.via == via && r.dir == dir) return &r;
    return nullptr;
}

// Minimum over tip, butt and shaft, head and torso of the slide. NaN when the row is missing or any value is non-finite.
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

// Minimum over the slide and the static mount pose.
float allMin(const Row* r) {
    const float d = caseMin(r);
    if (!std::isfinite(d) || !std::isfinite(r->stat[0]) || !std::isfinite(r->stat[1])) return std::numeric_limits<float>::quiet_NaN();
    const float s = r->stat[0] < r->stat[1] ? r->stat[0] : r->stat[1];
    return d < s ? d : s;
}

// True when any slide column of a and b differs (the static pose does not depend on via, so the slide is what must move).
bool drawDiffers(const Row* a, const Row* b) {
    if (!a || !b) return false;
    for (int pr = 0; pr < 2; ++pr)
        for (int p = 0; p < 3; ++p)
            if (std::fabs(a->acc[pr][p].min - b->acc[pr][p].min) > 1e-6f) return true;
    return false;
}

// Min over rows of value(row) for one category and mount; viaSel and dirSel: -1 = any, else 0/1 (via off/on, dir draw/stow). NaN if any value is
// non-finite or no row matches.
template <typename F>
float minOver(const std::vector<Row>& rows, Category cat, int mount, int viaSel, int dirSel, F value) {
    float m = std::numeric_limits<float>::infinity();
    bool any = false;
    for (const Row& r : rows) {
        if (r.cat != cat || r.mount != mount || (viaSel >= 0 && r.via != (viaSel == 1)) || (dirSel >= 0 && r.dir != dirSel)) continue;
        const float v = value(r);
        any = true;
        if (!std::isfinite(v)) return std::numeric_limits<float>::quiet_NaN();
        if (v < m) m = v;
    }
    return any ? m : std::numeric_limits<float>::quiet_NaN();
}

// Per-category table (stdout): one block per dir present, min over families and hands. off / on = via state; static = the mount pose (carryW 0).
// src = back-mount quat source (- when none); path = resid / blend over the category's rows (mixed when both).
void printTable(const std::vector<Row>& rows) {
    const Category order[] = { CatLightsaber, CatMelee, CatQStaff, CatPistol, CatRifle, CatHeavy, CatUnknown };
    std::printf("%-10s %-5s", "category", "dir");
    for (int m = 0; m < kMounts; ++m)
        std::printf(" %s_off %s_on %s_static", kMountName[m], kMountName[m], kMountName[m]);
    std::printf(" %-6s %-6s\n", "src", "path");
    for (int dir = kDirDraw; dir <= kDirStow; ++dir) {
        bool any = false;
        for (const Row& r : rows) any = any || r.dir == dir;
        if (!any) continue;
        for (Category c : order) {
            std::printf("%-10s %-5s", c == CatUnknown ? "unknown" : categoryName(c), dirName(dir));
            for (int m = 0; m < kMounts; ++m) {
                const float off = minOver(rows, c, m, 0, dir, [](const Row& r) { return caseMin(&r); });
                const float on = minOver(rows, c, m, 1, dir, [](const Row& r) { return caseMin(&r); });
                const float st = minOver(rows, c, m, 0, dir, [](const Row& r) { return r.stat[0] < r.stat[1] ? r.stat[0] : r.stat[1]; });
                std::printf(" %+.4f %+.4f %+.4f", static_cast<double>(off), static_cast<double>(on), static_cast<double>(st));
            }
            const char* srcTxt = "-";
            int nRes = 0, nBlend = 0;
            for (const Row& r : rows) {
                if (r.cat != c || r.dir != dir) continue;
                if (r.status != 2) {
                    if (r.resid) ++nRes;
                    else ++nBlend;
                }
                if (r.mount == kBack && std::strcmp(srcTxt, "-") == 0) srcTxt = srcName(r.src);
            }
            const char* pathTxt = (nRes && nBlend) ? "mixed" : (nRes ? "resid" : (nBlend ? "blend" : "-"));
            std::printf(" %-6s %-6s\n", srcTxt, pathTxt);
        }
    }
}

}  // namespace

int main(int argc, char** argv) {
    SweepOpts o = defaultOpts();
    const char* tsv = nullptr;
    bool table = false, overridden = false;
    for (int a = 1; a < argc; ++a) {
        const bool more = a + 1 < argc;
        const char* arg = argv[a];
        if (!std::strcmp(arg, "--tsv") && more) tsv = argv[++a];
        else if (!std::strcmp(arg, "--table")) table = true;
        else if (!std::strcmp(arg, "--extract-back") && more) { o.extract[kBack] = static_cast<float>(std::atof(argv[++a])); overridden = true; }
        else if (!std::strcmp(arg, "--extract-hip") && more) { o.extract[kHip] = static_cast<float>(std::atof(argv[++a])); overridden = true; }
        else if (!std::strcmp(arg, "--back-deg") && more) { o.backDeg = static_cast<float>(std::atof(argv[++a])); overridden = true; }
        else if (!std::strcmp(arg, "--back-quat") && more) {
            const char* v = argv[++a];
            if (!parseQuatArg(v, o.backQuat)) {
                std::fprintf(stderr, "--back-quat wants w,x,y,z: four finite numbers with a non-zero norm (got \"%s\")\n", v);
                return 2;
            }
            o.backQuatSet = true;
            overridden = true;
        }
        else if (!std::strcmp(arg, "--mounts-json") && more) {
            const char* v = argv[++a];
            std::string err;
            if (!loadMountsJson(v, o.mounts, err)) {
                std::fprintf(stderr, "--mounts-json %s: %s\n", v, err.c_str());
                return 2;
            }
            o.mountsSet = true;
            overridden = true;
        }
        else if (!std::strcmp(arg, "--dir") && more) {
            const char* v = argv[++a];
            if (!std::strcmp(v, "draw")) o.dirSel = kDirDraw;
            else if (!std::strcmp(v, "stow")) o.dirSel = kDirStow;
            else if (!std::strcmp(v, "both")) o.dirSel = kDirBoth;
            else {
                std::fprintf(stderr, "--dir wants draw, stow or both (got \"%s\")\n", v);
                return 2;
            }
            overridden = true;
        }
        else if (!std::strcmp(arg, "--pole-grip") && more) {
            const char* v = argv[++a];
            char* end = nullptr;
            const double g = std::strtod(v, &end);
            if (end == v || *end != 0 || !(g >= 0.0 && g <= 1.0)) {
                std::fprintf(stderr, "--pole-grip wants a number in 0..1 (got \"%s\")\n", v);
                return 2;
            }
            o.poleGrip = static_cast<float>(g);
            overridden = true;
        }
        else if (!std::strcmp(arg, "--via-out") && more) {
            const char* v = argv[++a];
            char* end = nullptr;
            const double m = std::strtod(v, &end);
            if (end == v || *end != 0 || !(m >= 0.0 && m <= 0.30)) {
                std::fprintf(stderr, "--via-out wants a number in 0..0.30 (got \"%s\")\n", v);
                return 2;
            }
            o.viaOut = static_cast<float>(m);
            overridden = true;
        }
        else if (!std::strcmp(arg, "--slow") && more) {
            const char* v = argv[++a];
            char* end = nullptr;
            const double sx = std::strtod(v, &end);
            if (end == v || *end != 0 || !(sx >= 1.0 && sx <= 8.0)) {
                std::fprintf(stderr, "--slow wants a number in 1..8 (got \"%s\")\n", v);
                return 2;
            }
            o.slowX = static_cast<float>(sx);
            overridden = true;
        }
        else {
            std::fprintf(stderr, "usage: holster_sweep_test [--table] [--tsv <path>] [--dir draw|stow|both] [--back-deg <d> | --back-quat w,x,y,z | "
                                 "--mounts-json <file>] [--extract-back <f>] [--extract-hip <f>] [--pole-grip <a>] [--via-out <m>] [--slow <x>]\n");
            return 2;
        }
    }
    if (overridden && !tsv && !table) {
        std::fprintf(stderr, "--back-deg / --back-quat / --mounts-json / --dir / --pole-grip / --via-out / --slow / --extract-* need --tsv or --table (the asserts assume the defaults)\n");
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

    // Key case: qtrstaff on the back, right hand, via off, draw. It clips under the GUESS 45 deg mount quat (static pose and slide).
    const Row* keyOff = findRow(rows, "w_qtrstaff_", kBack, 0, false, kDirDraw);
    const Row* keyOn = findRow(rows, "w_qtrstaff_", kBack, 0, true, kDirDraw);
    KT_CHECK(keyOff != nullptr && allMin(keyOff) < 0.f);
    // Via must change the draw. Its sign is not asserted (README).
    KT_CHECK(drawDiffers(keyOff, keyOn));

    // Every pistol on the hip is clear, draw and static, both hands and both via states. Pistols = CatPistol (HolsterLogic.h:66-67).
    int pistolCases = 0;
    bool pistolClear = true;
    for (const Row& r : rows) {
        if (r.cat != CatPistol || r.mount != kHip || r.dir != kDirDraw) continue;
        ++pistolCases;
        if (!(allMin(&r) > 0.f)) pistolClear = false;
    }
    KT_CHECK(pistolCases > 0);
    KT_CHECK(pistolClear);

    // HW32c parsers: --back-quat normalizes and rejects zero / malformed input; --mounts-json takes the sample shape, drops count 0, rejects bad input.
    Q q{ 0.f, 0.f, 0.f, 0.f };
    KT_CHECK(parseQuatArg("2,0,0,0", q) && std::fabs(q.w - 1.f) < 1e-6f && std::fabs(q.x) < 1e-6f && std::fabs(q.y) < 1e-6f && std::fabs(q.z) < 1e-6f);
    KT_CHECK(parseQuatArg("0.92,0,0.38,0", q) && std::fabs(q.w * q.w + q.y * q.y - 1.f) < 1e-5f);
    Q keep{ 0.5f, 0.5f, 0.5f, 0.5f };
    KT_CHECK(!parseQuatArg("0,0,0,0", keep) && keep.w == 0.5f);
    KT_CHECK(!parseQuatArg("nan,0,0,0", keep));
    KT_CHECK(!parseQuatArg("1,0,0", keep));
    KT_CHECK(!parseQuatArg("1,0,0,0,0", keep));
    KT_CHECK(!parseQuatArg("1,0,0,0x", keep));

    const std::string sample =
        "[{\"cat\":\"rifle\",\"bone\":\"torsoUpr_g\",\"count\":12,\"nofacing\":0,\"rel\":[0.92,0,0.38,0],\"spread\":1.2},"
        " {\"bone\":\"torso_g\",\"count\":3,\"rel\":null,\"cat\":\"pistol\"},"
        " {\"cat\":4,\"bone\":\"pelvis_g\",\"count\":9,\"nofacing\":1,\"rel\":[1,0,0,0],\"spread\":null}]";
    std::vector<MountRec> recs;
    std::string err;
    KT_CHECK(parseMountsText(sample, recs, err) && recs.size() == 3);
    KT_CHECK(recs.size() == 3 && recs[0].cat == "rifle" && recs[0].count == 12 && recs[0].hasRel && recs[1].cat == "pistol" && !recs[1].hasRel);
    KT_CHECK(recs.size() == 3 && recs[2].cat == "pistol");   // numeric cat 4 is the pistol index
    Q got{ 0.f, 0.f, 0.f, 0.f };
    KT_CHECK(logBackRel(recs, CatRifle, got) && std::fabs(got.w * got.w + got.x * got.x + got.y * got.y + got.z * got.z - 1.f) < 1e-5f);
    KT_CHECK(!logBackRel(recs, CatPistol, got));   // null rel: the GUESS stays
    KT_CHECK(!logBackRel(recs, CatHeavy, got));    // no entry: the GUESS stays
    KT_CHECK(parseMountsText("[{\"cat\":\"melee\",\"bone\":\"torsoUpr_g\",\"count\":0,\"rel\":[0.5,0.5,0.5,0.5]}]", recs, err) && recs.empty());
    KT_CHECK(!parseMountsText("[{\"cat\":\"rifle\"", recs, err));
    KT_CHECK(!parseMountsText("{}", recs, err));
    KT_CHECK(!parseMountsText("[] x", recs, err));
    KT_CHECK(!parseMountsText("[{\"cat\":\"rifle\",\"bone\":\"torsoUpr_g\",\"count\":1}]", recs, err));                 // rel missing
    KT_CHECK(!parseMountsText("[{\"cat\":\"rifle\",\"bone\":\"torsoUpr_g\",\"count\":1.5,\"rel\":null}]", recs, err));   // fractional count
    KT_CHECK(!parseMountsText("[{\"cat\":\"rifle\",\"bone\":\"torsoUpr_g\",\"count\":1,\"rel\":[0,0,0,0]}]", recs, err)); // zero rel
    KT_CHECK(!parseMountsText("[{\"cat\":\"rifle\",\"bone\":\"torsoUpr_g\",\"count\":3000000000,\"rel\":null}]", recs, err)); // past 1e9: int overflow
    // The bone match ignores ASCII case (reachIsBackBone is _stricmp).
    KT_CHECK(parseMountsText("[{\"cat\":\"rifle\",\"bone\":\"TORSOUPR_G\",\"count\":2,\"rel\":[1,0,0,0]}]", recs, err) && logBackRel(recs, CatRifle, got));

    // HW32c --dir: stow alone gives only stow rows; draw alone only draw rows; both is the two sets.
    SweepOpts stowOnly = defaultOpts();
    stowOnly.dirSel = kDirStow;
    std::vector<Row> stowRows;
    int sf = 0, ss = 0;
    runSweep(stowOnly, stowRows, sf, ss);
    bool allStow = !stowRows.empty();
    for (const Row& r : stowRows) allStow = allStow && r.dir == kDirStow;
    KT_CHECK(allStow);
    KT_CHECK(stowRows.size() * 2 == rows.size());

    // R6: --via-out and --slow options produce finite, non-NaN rows and --via-out 0.25 changes the via-on trajectory relative to 0.05.
    SweepOpts voLo = defaultOpts(), voHi = defaultOpts();
    voLo.dirSel = kDirDraw; voLo.viaOut = 0.05f;
    voHi.dirSel = kDirDraw; voHi.viaOut = 0.25f; voHi.slowX = 2.f;
    std::vector<Row> rLo, rHi;
    int fLo = 0, sLo = 0, fHi = 0, sHi = 0;
    runSweep(voLo, rLo, fLo, sLo);
    runSweep(voHi, rHi, fHi, sHi);
    KT_CHECK(sLo == 0 && sHi == 0 && rLo.size() == rHi.size());
    const Row* qLo = findRow(rLo, "w_qtrstaff_", kBack, 0, true, kDirDraw);
    const Row* qHi = findRow(rHi, "w_qtrstaff_", kBack, 0, true, kDirDraw);
    KT_CHECK(qLo != nullptr && qHi != nullptr && drawDiffers(qLo, qHi));

    // R5 + 3D dynamic editor offset grid sweep: across front-chest, cross-draw, thigh and lumbar placements, the 3D reach solver
    // produces 100% finite, reachable two-bone solutions across all path steps f in [0, 1] for both hands.
    {
        const float arm = kL1 + kL2;
        const float fxGrid[4] = { -0.30f, -0.10f, 0.15f, 0.30f };
        const float fyGrid[4] = { -0.25f, -0.10f, 0.0f, 0.18f };
        const float dzGrid[4] = { -0.78f, -0.54f, -0.36f, -0.05f };
        int solvedGrid = 0;
        bool gridOk = true;
        for (int hand = 0; hand < 2; ++hand) {
            float restE[3];
            restElbow(hand, restE);
            for (float fxA : fxGrid)
                for (float fyA : fyGrid)
                    for (float dzA : dzGrid) {
                        const float mpPos[3] = {
                            kShoulder[hand][0] + fxA * arm,
                            kShoulder[hand][1] + fyA * arm,
                            kShoulder[hand][2] + dzA * arm
                        };
                        const float dz = dzA * arm;
                        const ReachTopo topo = reachTopology3D(fxA, fyA, dzA, reachHingeFits(dz, arm));
                        const float uW = reachOverWeight3D(fxA, fyA, dzA, topo);
                        const ReachStyle st = reachDefaultStyle(CatPistol);
                        for (int step = 0; step <= 10; ++step) {
                            const float f = static_cast<float>(step) / 10.f;
                            float T[3], pole[3], En[3], reach = 1.f;
                            if (topo == ReachTopo::HingeBack) {
                                gridOk = gridOk && reachHinge(kShoulder[hand], kL1, kL2, kUpDir, kFwdDir, kOut3[hand], mpPos, kRest[hand], kRestPole, f, T, pole);
                            } else {
                                float effBulge[3], effOver = uW;
                                reachAdaptBulge(st, uW, fxA, fyA, dzA, hand != 0, effBulge, effOver);
                                reachCarryTarget(mpPos, kRest[hand], kUpDir, kFwdDir, kOut3[hand], effBulge, effOver, arm, f, T);
                                gridOk = gridOk && reachPole3D(st, uW, fxA, fyA, dzA, hand != 0, Q{ 1.f, 0.f, 0.f, 0.f }, kRestPole, reachAnimMix(f), pole);
                            }
                            gridOk = gridOk && solveTwoBone(kShoulder[hand], restE, kRest[hand], T, En, &reach, pole) && std::isfinite(reach);
                            ++solvedGrid;
                        }
                    }
        }
        KT_CHECK(gridOk && solvedGrid == 2 * 4 * 4 * 4 * 11);
    }

    KT_CHECK(skipped == 0);
    return kt::Report("holster_sweep");
}
