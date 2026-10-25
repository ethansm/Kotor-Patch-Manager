// k2_shadow_distance_test.cpp -- Unit tests for K2ShadowDistanceLogic.h.
#define INIW_FILE_API 1
#define MCDECL_FILE_API 1

#include "K2ShadowDistanceLogic.h"
#include "check.h"
#include <cmath>
#include <string.h>

static bool near(float a, float b) { return std::fabs(a - b) < 1e-3f; }

static void test_effective() {
    k2shadow::Config def{};
    KT_CHECK(def.enabled);
    KT_CHECK(def.perfLog);
    KT_CHECK(!def.verbose);
    KT_CHECK(def.shadowDist == 35.0f);

    // Vanilla (default 35 m): exactly the engine's 1100.0 reference constant.
    k2shadow::EffectiveValues ev = k2shadow::computeEffective(def);
    KT_CHECK(ev.dist == 35.0f);
    KT_CHECK(ev.refSq == 1100.0f);

    // 70 m: both gates scale with the distance, 4900 * 1100/1225 = 4400.
    k2shadow::Config far = def;
    far.shadowDist = 70.0f;
    ev = k2shadow::computeEffective(far);
    KT_CHECK(ev.dist == 70.0f);
    KT_CHECK(ev.refSq == 4400.0f);

    // Disabled: vanilla values whatever shadow_dist says.
    far.enabled = false;
    ev = k2shadow::computeEffective(far);
    KT_CHECK(ev.dist == 35.0f && ev.refSq == 1100.0f);

    // Gate ratio stays the vanilla 1100/1225 across the range.
    for (float d = 10.0f; d <= 100.0f; d += 5.0f) {
        k2shadow::Config c = def;
        c.shadowDist = d;
        ev = k2shadow::computeEffective(c);
        KT_CHECK(near(ev.refSq / (ev.dist * ev.dist), 1100.0f / 1225.0f));
    }
}

static void test_clamps() {
    k2shadow::Config c{};
    c.shadowDist = 5.0f;
    KT_CHECK(near(k2shadow::computeEffective(c).dist, 10.0f));
    c.shadowDist = 5000.0f;
    KT_CHECK(near(k2shadow::computeEffective(c).dist, 100.0f));
    KT_CHECK(near(k2shadow::computeEffective(c).refSq, 100.0f * 100.0f * 1100.0f / 1225.0f));
    c.shadowDist = NAN;
    KT_CHECK(near(k2shadow::computeEffective(c).dist, 10.0f));
    c.shadowDist = 100.0f;
    c.sanitize();
    KT_CHECK(c.shadowDist == 100.0f);

    char hi[] = "shadow_dist = 9999\n";
    k2shadow::Config p{};
    k2shadow::parseIniText(hi, p);
    KT_CHECK(p.shadowDist == 100.0f);
    char lo[] = "shadow_dist = -4\n";
    k2shadow::parseIniText(lo, p);
    KT_CHECK(p.shadowDist == 10.0f);
}

static void test_ini_roundtrip() {
    char buf[1024] = {};
    k2shadow::Config orig{};
    orig.enabled = false;
    orig.shadowDist = 65.0f;
    orig.perfLog = false;
    orig.verbose = true;
    const int n = k2shadow::formatDefaultIni(buf, sizeof(buf), orig);
    KT_CHECK(n > 0 && static_cast<size_t>(n) < sizeof(buf));
    k2shadow::Config parsed{};
    inikit::LoadStats st = k2shadow::parseIniText(buf, parsed);
    KT_CHECK_EQ(st.bad, 0);
    KT_CHECK_EQ(st.ok, 4);
    KT_CHECK(parsed == orig);

    // Default config round-trips too.
    k2shadow::Config def{};
    k2shadow::formatDefaultIni(buf, sizeof(buf), def);
    k2shadow::parseIniText(buf, parsed);
    KT_CHECK(parsed == def);

    char bad[] = "shadow_dist = far\nenabled = maybe\nperf_log = 0\nmystery = 1\n";
    k2shadow::Config p2{};
    st = k2shadow::parseIniText(bad, p2);
    KT_CHECK_EQ(st.bad, 2);
    KT_CHECK(p2.shadowDist == 35.0f);
    KT_CHECK(p2.enabled);
    KT_CHECK(!p2.perfLog);
}

static void test_perf_window() {
    k2shadow::PerfWindow w{};
    w.reset(1000);
    for (unsigned t = 1000; t < 6000; t += 100) w.add(t);   // 50 calls over 5 s
    KT_CHECK(!w.due(5999 - 1));
    KT_CHECK(w.due(6000));
    KT_CHECK(near(w.perSec(6000), 10.0f));
    // A gap longer than kPerfGapMs drops the partial window.
    k2shadow::PerfWindow g{};
    g.reset(0);
    g.add(100);
    g.add(200);
    g.add(5000);
    KT_CHECK_EQ(g.calls, 1u);
    KT_CHECK(!g.due(5000));
}

static void test_modconfig_schema() {
    mcdecl::PageDecl pd{};
    char err[96] = {};
    KT_CHECK(mcdecl::parseSchema(k2shadow::kPageSchema, strlen(k2shadow::kPageSchema), pd, err, sizeof(err)));
    KT_CHECK_EQ(pd.nRows, 2);
    KT_CHECK(strcmp(pd.name, "K2 Shadow Distance") == 0);
    for (int i = 0; i < pd.nRows; ++i) {
        const mcdecl::RowDecl& r = pd.rows[i];
        KT_CHECK(strcmp(r.ini, k2shadow::kIniFile) == 0);
        KT_CHECK(strcmp(r.section, k2shadow::kIniSection) == 0);
        KT_CHECK(strlen(r.label) > 0 && strlen(r.label) <= 40);
        KT_CHECK(strlen(r.desc) > 0 && strlen(r.desc) < 95);
    }
    KT_CHECK(strcmp(pd.rows[1].label, "Shadow Distance") == 0);
    KT_CHECK(strcmp(pd.rows[1].key, "shadow_dist") == 0);
}

int main() {
    test_effective();
    test_clamps();
    test_ini_roundtrip();
    test_perf_window();
    test_modconfig_schema();
    return kt::Report("k2_shadow_distance_test");
}
