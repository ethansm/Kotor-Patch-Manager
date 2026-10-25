// fog_distance_test.cpp -- Unit tests for K2FogDistanceLogic.h.
#define INIW_FILE_API 1
#define MCDECL_FILE_API 1

#include "K2FogDistanceLogic.h"
#include "check.h"
#include <cmath>
#include <string.h>

static bool near(float a, float b) { return std::fabs(a - b) < 1e-3f; }

static void test_apply_scale() {
    k2fog::Config def{};
    KT_CHECK(def.enabled);
    KT_CHECK_EQ(def.fogEndPct, 200);
    KT_CHECK_EQ(def.fogStartPct, 200);

    // Default 200%: 20..60 m area fog -> 40..120 m.
    k2fog::FogRange r = k2fog::applyScale(20.0f, 60.0f, def);
    KT_CHECK(near(r.start, 40.0f) && near(r.end, 120.0f));

    // Disabled: the area's own values, untouched.
    k2fog::Config off = def;
    off.enabled = false;
    r = k2fog::applyScale(20.0f, 60.0f, off);
    KT_CHECK(r.start == 20.0f && r.end == 60.0f);

    // 0% = No Fog: the Shader Fixes fog-less end.
    k2fog::Config none = def;
    none.fogEndPct = 0;
    r = k2fog::applyScale(20.0f, 60.0f, none);
    KT_CHECK(r.end == k2fog::kNoFogEnd && r.start < r.end);

    // Start past the end is pulled to 95% of the end.
    k2fog::Config inv = def;
    inv.fogEndPct = 100;
    inv.fogStartPct = 400;
    r = k2fog::applyScale(50.0f, 60.0f, inv);
    KT_CHECK(near(r.end, 60.0f) && near(r.start, 57.0f));

    // Out-of-range percentages clamp to 0..400.
    k2fog::Config big = def;
    big.fogEndPct = 9000;
    big.fogStartPct = -50;
    r = k2fog::applyScale(10.0f, 100.0f, big);
    KT_CHECK(near(r.end, 400.0f) && near(r.start, 0.0f));
}

static void test_tracker_no_compound() {
    k2fog::FogTracker t{};
    k2fog::Config c{};
    const uintptr_t sceneA = 0x1000;

    // First sight: area values become the origin, scaled range written.
    k2fog::UpdateResult u = t.update(sceneA, 20.0f, 60.0f, c);
    KT_CHECK(u.newOrigin && u.write);
    KT_CHECK(near(u.range.start, 40.0f) && near(u.range.end, 120.0f));

    // Next frame the scene holds our values: no rescale, no write.
    u = t.update(sceneA, u.range.start, u.range.end, c);
    KT_CHECK(!u.newOrigin && !u.write);
    KT_CHECK(near(u.range.end, 120.0f));

    // Settings change: rescale from the remembered origin, not from 120.
    c.fogEndPct = 300;
    u = t.update(sceneA, 40.0f, 120.0f, c);
    KT_CHECK(!u.newOrigin && u.write && near(u.range.end, 180.0f));

    // Area load writes new fog: new origin.
    u = t.update(sceneA, 5.0f, 30.0f, c);
    KT_CHECK(u.newOrigin && near(u.orig.end, 30.0f) && near(u.range.end, 90.0f));

    // Disable: the stored vanilla values go back.
    c.enabled = false;
    u = t.update(sceneA, u.range.start, u.range.end, c);
    KT_CHECK(u.write && u.range.start == 5.0f && u.range.end == 30.0f);
    // Re-enable from vanilla in the scene: same origin, no new origin.
    c.enabled = true;
    u = t.update(sceneA, 5.0f, 30.0f, c);
    KT_CHECK(!u.newOrigin && u.write && near(u.range.end, 90.0f));
}

static void test_tracker_slots() {
    k2fog::FogTracker t{};
    k2fog::Config c{};
    for (uintptr_t s = 1; s <= static_cast<uintptr_t>(k2fog::kMaxScenes); ++s) t.update(s * 0x100, 10.0f, 50.0f, c);
    // A fifth scene evicts the least recently used one (scene 0x100); the others keep their origins.
    t.update(0x100 * 2, 20.0f, 100.0f, c);   // touch scene 2 (its fields now hold our 20..100 write)
    t.update(0x9900, 1.0f, 2.0f, c);
    KT_CHECK(t.find(0x100) == nullptr);
    KT_CHECK(t.find(0x9900) != nullptr);
    KT_CHECK(t.find(0x200) != nullptr && near(t.find(0x200)->origEnd, 50.0f));
    KT_CHECK(t.find(0) == nullptr);
}

static void test_ini_roundtrip() {
    char buf[1024] = {};
    k2fog::Config orig{};
    orig.enabled = false;
    orig.fogEndPct = 325;
    orig.fogStartPct = 150;
    orig.verbose = true;
    const int n = k2fog::formatDefaultIni(buf, sizeof(buf), orig);
    KT_CHECK(n > 0 && static_cast<size_t>(n) < sizeof(buf));
    k2fog::Config parsed{};
    inikit::LoadStats st = k2fog::parseIniText(buf, parsed);
    KT_CHECK_EQ(st.bad, 0);
    KT_CHECK_EQ(st.ok, 4);
    KT_CHECK(parsed == orig);

    char bad[] = "fog_end_pct = lots\nfog_start_pct = 87.6\nenabled = maybe\n";
    k2fog::Config p2{};
    st = k2fog::parseIniText(bad, p2);
    KT_CHECK_EQ(st.bad, 2);
    KT_CHECK_EQ(p2.fogEndPct, k2fog::kDefaultFogEndPct);
    KT_CHECK_EQ(p2.fogStartPct, 88);
    KT_CHECK(p2.enabled);
}

static void test_modconfig_schema() {
    mcdecl::PageDecl pd{};
    char err[96] = {};
    KT_CHECK(mcdecl::parseSchema(k2fog::kPageSchema, strlen(k2fog::kPageSchema), pd, err, sizeof(err)));
    KT_CHECK_EQ(pd.nRows, 3);
    for (int i = 0; i < pd.nRows; ++i) {
        const mcdecl::RowDecl& r = pd.rows[i];
        KT_CHECK(strcmp(r.ini, k2fog::kIniFile) == 0);
        KT_CHECK(strcmp(r.section, k2fog::kIniSection) == 0);
        KT_CHECK(strlen(r.label) > 0 && strlen(r.label) <= 40);
        KT_CHECK(strlen(r.desc) > 0 && strlen(r.desc) < 95);
    }
}

int main() {
    test_apply_scale();
    test_tracker_no_compound();
    test_tracker_slots();
    test_ini_roundtrip();
    test_modconfig_schema();
    return kt::Report("fog_distance_test");
}
