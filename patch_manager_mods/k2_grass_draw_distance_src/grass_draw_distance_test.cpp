// grass_draw_distance_test.cpp -- Unit tests for K2GrassDrawDistanceLogic.h (doc 48).
#define INIW_FILE_API 1
#define MCDECL_FILE_API 1

#include "K2GrassDrawDistanceLogic.h"
#include "check.h"
#include <cmath>
#include <string.h>

static void test_defaults_and_effective_math() {
    k2grass::Config def{};
    KT_CHECK(def.enabled);
    KT_CHECK(std::fabs(def.fadeStartDist - 120.0f) < 1e-5f);
    KT_CHECK(std::fabs(def.fadeEndDist - 150.0f) < 1e-5f);
    KT_CHECK(std::fabs(def.windDist - 20.0f) < 1e-5f);
    KT_CHECK(std::fabs(def.alphaCutoff - 0.40f) < 1e-5f);
    KT_CHECK(def.perfLog);

    k2grass::EffectiveValues ev = k2grass::computeEffective(def);
    KT_CHECK(std::fabs(ev.fadeStartSq - 14400.0f) < 1e-3f);
    KT_CHECK(std::fabs(ev.fadeEndSq - 22500.0f) < 1e-3f);
    KT_CHECK(std::fabs(ev.windDistSq - 400.0f) < 1e-3f);
    KT_CHECK(std::fabs(ev.alphaCutoff - 0.40f) < 1e-5f);

    // Disabled config restores exact vanilla constants (15m=225, 30m=900, 15m=225, 0.40f).
    k2grass::Config off = def;
    off.enabled = false;
    k2grass::EffectiveValues vEv = k2grass::computeEffective(off);
    KT_CHECK(std::fabs(vEv.fadeStartSq - 225.0f) < 1e-3f);
    KT_CHECK(std::fabs(vEv.fadeEndSq - 900.0f) < 1e-3f);
    KT_CHECK(std::fabs(vEv.windDistSq - 225.0f) < 1e-3f);
    KT_CHECK(std::fabs(vEv.alphaCutoff - 0.40f) < 1e-5f);

    // Vanilla alpha-test clip (~25.10m) vs default 120..150m with the same cutoff: sqrt(14400 + 0.6 x 8100) = ~138.78m.
    const float vanillaClip = k2grass::effectiveClipDistMeters(vEv);
    const float extClip = k2grass::effectiveClipDistMeters(ev);
    KT_CHECK(vanillaClip > 25.0f && vanillaClip < 25.2f);
    KT_CHECK(extClip > 138.7f && extClip < 138.9f);

    // A 0.20 cutoff on a 60..90 window clips at sqrt(7200) = ~84.85m.
    k2grass::Config soft = def;
    soft.fadeStartDist = 60.0f;
    soft.fadeEndDist = 90.0f;
    soft.alphaCutoff = 0.20f;
    const float softClip = k2grass::effectiveClipDistMeters(k2grass::computeEffective(soft));
    KT_CHECK(softClip > 84.8f && softClip < 84.9f);
}

static void test_bin_alpha_curve() {
    k2grass::Config def{};
    def.fadeStartDist = 60.0f;
    def.fadeEndDist = 90.0f;
    k2grass::EffectiveValues ev = k2grass::computeEffective(def); // 3600 .. 8100

    KT_CHECK(std::fabs(k2grass::computeBinAlpha(0.0f, ev) - 1.0f) < 1e-5f);
    KT_CHECK(std::fabs(k2grass::computeBinAlpha(3599.0f, ev) - 1.0f) < 1e-5f);
    KT_CHECK(std::fabs(k2grass::computeBinAlpha(3600.0f, ev) - 1.0f) < 1e-5f);
    // Midpoint in squared distance: (3600 + 8100)/2 = 5850 -> alpha = 0.5
    KT_CHECK(std::fabs(k2grass::computeBinAlpha(5850.0f, ev) - 0.5f) < 1e-5f);
    KT_CHECK(std::fabs(k2grass::computeBinAlpha(8100.0f, ev) - 0.0f) < 1e-5f);
    KT_CHECK(std::fabs(k2grass::computeBinAlpha(12000.0f, ev) - 0.0f) < 1e-5f);
}

static void test_sanitize_clamps_and_span_guard() {
    // Inverted fade_start >= fade_end pulls the start down (draw distance is the cost knob, never raised).
    k2grass::Config c{};
    c.fadeStartDist = 100.0f;
    c.fadeEndDist = 80.0f;
    c.windDist = -10.0f;
    c.alphaCutoff = 5.0f;
    c.sanitize();
    KT_CHECK(std::fabs(c.fadeStartDist - 75.0f) < 1e-5f);
    KT_CHECK(std::fabs(c.fadeEndDist - 80.0f) < 1e-5f);
    KT_CHECK(std::fabs(c.windDist - 0.0f) < 1e-5f);
    KT_CHECK(std::fabs(c.alphaCutoff - k2grass::kMaxAlphaCutoff) < 1e-5f);

    // Extreme upper clamp
    c.fadeStartDist = 9999.0f;
    c.fadeEndDist = 9999.0f;
    c.sanitize();
    KT_CHECK(c.fadeStartDist <= k2grass::kMaxFadeStartDist);
    KT_CHECK(std::fabs(c.fadeEndDist - 150.0f) < 1e-5f);
    KT_CHECK(c.fadeEndDist >= c.fadeStartDist + k2grass::kMinFadeSpan);

    // Menu case: Fade Start slider at its max (135) with Draw Distance 90 keeps 90.
    c.fadeStartDist = 135.0f;
    c.fadeEndDist = 90.0f;
    c.sanitize();
    KT_CHECK(std::fabs(c.fadeEndDist - 90.0f) < 1e-5f);
    KT_CHECK(std::fabs(c.fadeStartDist - 85.0f) < 1e-5f);

    // Lowest draw distance still leaves a valid span above the start floor.
    c.fadeStartDist = 0.0f;
    c.fadeEndDist = 0.0f;
    c.sanitize();
    KT_CHECK(std::fabs(c.fadeEndDist - k2grass::kMinFadeEndDist) < 1e-5f);
    KT_CHECK(std::fabs(c.fadeStartDist - k2grass::kMinFadeStartDist) < 1e-5f);
}

static void test_menu_fade_link() {
    // Highest start on the 15 m grid, >= 5 m below the end.
    KT_CHECK_EQ(k2grass::menuFadeStartMax(90), 75);
    KT_CHECK_EQ(k2grass::menuFadeStartMax(150), 135);
    KT_CHECK_EQ(k2grass::menuFadeStartMax(30), 15);
    KT_CHECK_EQ(k2grass::menuFadeStartMax(45), 30);
    KT_CHECK_EQ(k2grass::menuFadeStartMax(10), 15);   // below the menu range: floor, never under the row min
    // Draw Distance dropped under Fade Start: the start follows down.
    KT_CHECK_EQ(k2grass::menuLinkFadeStart(135, 90), 75);
    KT_CHECK_EQ(k2grass::menuLinkFadeStart(45, 30), 15);
    // Already consistent: unchanged.
    KT_CHECK_EQ(k2grass::menuLinkFadeStart(45, 90), 45);
    KT_CHECK_EQ(k2grass::menuLinkFadeStart(75, 90), 75);
    // Every menu pair agrees with sanitize (the engine sees exactly what the sliders show).
    for (int32_t end = 30; end <= 150; end += 15) {
        for (int32_t start = 15; start <= 135; start += 15) {
            k2grass::Config c{};
            c.fadeEndDist = static_cast<float>(end);
            c.fadeStartDist = static_cast<float>(k2grass::menuLinkFadeStart(start, end));
            const k2grass::Config before = c;
            c.sanitize();
            KT_CHECK(c == before);
        }
    }
}

static void test_perf_window() {
    k2grass::PerfWindow w{};
    w.reset(1000);
    KT_CHECK(!w.due(1000));
    w.add(400, 1000);
    w.add(600, 1900);
    KT_CHECK(!w.due(5999));
    KT_CHECK(w.due(6000));
    KT_CHECK_EQ(w.frames, 2u);
    KT_CHECK_EQ(w.binsMax, 600u);
    KT_CHECK(std::fabs(w.binsAvg() - 500.0f) < 1e-3f);
    KT_CHECK(std::fabs(w.fps(6000) - 0.4f) < 1e-4f);
    // GetTickCount wrap: start near UINT_MAX, now past zero.
    w.reset(0xFFFFF000u);
    w.add(1, 0xFFFFF000u);
    KT_CHECK(w.due(0x00000400u + 0x1000u));
    // A grass gap over kPerfGapMs (menu, area load) drops the partial window instead of diluting fps.
    w.reset(10000);
    w.add(500, 10000);
    w.add(500, 11000);           // exactly the gap limit: kept
    KT_CHECK_EQ(w.frames, 2u);
    w.add(700, 12001);           // 1001 ms gap: window restarts at this frame
    KT_CHECK_EQ(w.frames, 1u);
    KT_CHECK_EQ(w.startMs, 12001u);
    KT_CHECK_EQ(w.binsMax, 700u);
    for (unsigned t = 12017; t < 17001; t += 17) w.add(700, t);
    KT_CHECK(w.due(17001));
    KT_CHECK(w.fps(17001) > 58.0f && w.fps(17001) < 60.0f);
    w.reset(7000);
    KT_CHECK_EQ(w.frames, 0u);
    KT_CHECK_EQ(w.startMs, 7000u);
}

static void test_ini_roundtrip_and_direct_float_alpha() {
    char buf[1024] = {};
    k2grass::Config orig{};
    orig.fadeStartDist = 75.0f;
    orig.fadeEndDist = 135.0f;
    orig.windDist = 30.0f;
    orig.alphaCutoff = 0.25f;
    orig.perfLog = false;
    const int n = k2grass::formatDefaultIni(buf, sizeof(buf), orig);
    KT_CHECK(n > 0 && static_cast<size_t>(n) < sizeof(buf));

    k2grass::Config parsed{};
    inikit::LoadStats st = k2grass::parseIniText(buf, parsed);
    KT_CHECK_EQ(st.bad, 0);
    KT_CHECK(parsed == orig);

    // Also verify direct float alpha_cutoff = 0.18
    char floatIni[] = "fade_start_dist = 50\nfade_end_dist = 100\nalpha_cutoff = 0.18\nwind_dist = 0\n";
    k2grass::Config parsed2{};
    inikit::LoadStats st2 = k2grass::parseIniText(floatIni, parsed2);
    KT_CHECK_EQ(st2.ok, 4);
    KT_CHECK_EQ(st2.bad, 0);
    KT_CHECK(std::fabs(parsed2.alphaCutoff - 0.18f) < 1e-5f);
    KT_CHECK(std::fabs(parsed2.windDist - 0.0f) < 1e-5f);
}

static void test_modconfig_schema_bindings() {
    mcdecl::PageDecl pd{};
    char err[96] = {};
    KT_CHECK(mcdecl::parseSchema(k2grass::kPageSchema, strlen(k2grass::kPageSchema), pd, err, sizeof(err)));
    KT_CHECK_EQ(pd.nRows, 5);
    for (int i = 0; i < pd.nRows; ++i) {
        const mcdecl::RowDecl& r = pd.rows[i];
        KT_CHECK(strcmp(r.ini, k2grass::kIniFile) == 0);
        KT_CHECK(strcmp(r.section, k2grass::kIniSection) == 0);
        KT_CHECK(strlen(r.label) > 0 && strlen(r.label) <= 40);
        KT_CHECK(strlen(r.desc) > 0 && strlen(r.desc) < 95);
    }
}

int main() {
    test_defaults_and_effective_math();
    test_bin_alpha_curve();
    test_sanitize_clamps_and_span_guard();
    test_menu_fade_link();
    test_perf_window();
    test_ini_roundtrip_and_direct_float_alpha();
    test_modconfig_schema_bindings();
    return kt::Report("grass_draw_distance_test");
}
