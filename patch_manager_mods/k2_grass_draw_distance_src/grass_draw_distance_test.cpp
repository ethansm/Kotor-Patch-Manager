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

static void test_collect_dist_trim() {
    k2grass::Config def{};
    KT_CHECK(def.trimHidden);
    KT_CHECK(!def.smoothEdges);
    const k2grass::EffectiveValues ev = k2grass::computeEffective(def);

    // Defaults: clip ~138.78 m + 2 m margin = ~140.78 m, so sq ~19818.8 < 22500 (fade end).
    const float trimmed = k2grass::collectDistSq(ev, true);
    KT_CHECK(trimmed < ev.fadeEndSq);
    KT_CHECK(std::fabs(std::sqrt(trimmed) - 140.78f) < 0.01f);
    KT_CHECK(trimmed > 19800.0f && trimmed < 19840.0f);
    // Trim off collects out to the fade end.
    KT_CHECK(std::fabs(k2grass::collectDistSq(ev, false) - 22500.0f) < 1e-3f);

    // Vanilla values (enabled=0): clip 25.10 m + 2 m = 27.10 m, sq ~734.4 < 900.
    k2grass::Config off = def;
    off.enabled = false;
    const k2grass::EffectiveValues vEv = k2grass::computeEffective(off);
    const float vTrim = k2grass::collectDistSq(vEv, true);
    KT_CHECK(std::fabs(std::sqrt(vTrim) - 27.1f) < 0.01f);
    KT_CHECK(vTrim > 730.0f && vTrim < 740.0f);
    KT_CHECK(std::fabs(k2grass::collectDistSq(vEv, false) - 900.0f) < 1e-3f);

    // Never above the fade end: a cutoff near zero clips at ~the fade end, and the margin must not push past it.
    k2grass::Config thin = def;
    thin.alphaCutoff = k2grass::kMinAlphaCutoff;
    const k2grass::EffectiveValues tEv = k2grass::computeEffective(thin);
    KT_CHECK(k2grass::collectDistSq(tEv, true) <= tEv.fadeEndSq);
    KT_CHECK(k2grass::collectDistSq(tEv, true) > k2grass::collectDistSq(ev, true));
    // Never below the fade start (the clip distance is at least the start).
    KT_CHECK(k2grass::collectDistSq(ev, true) >= ev.fadeStartSq);
    KT_CHECK(k2grass::collectDistSq(vEv, true) >= vEv.fadeStartSq);
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

static void test_perf_window_grass_time() {
    k2grass::PerfWindow w{};
    w.reset(1000);
    KT_CHECK(std::fabs(w.grassMsAvg()) < 1e-6f);   // no samples: zeros, no divide by zero
    KT_CHECK(std::fabs(w.grassMsMax()) < 1e-6f);
    w.add(100, 1000);
    // Frame 1: the bucket loop ran twice (1000 + 500 us); it closes on the next frame's add().
    w.addGrassUs(1000);
    w.addGrassUs(500);
    KT_CHECK_EQ(w.grassCount, 0u);
    w.add(100, 1016);
    KT_CHECK_EQ(w.grassCount, 1u);
    KT_CHECK_EQ(w.grassUsMax, 1500u);
    // Frame 2: 500 us.
    w.addGrassUs(500);
    w.add(100, 1033);
    KT_CHECK_EQ(w.grassCount, 2u);
    KT_CHECK(std::fabs(w.grassMsAvg() - 1.0f) < 1e-4f);   // (1500 + 500) / 2 us = 1.00 ms
    KT_CHECK(std::fabs(w.grassMsMax() - 1.5f) < 1e-4f);
    // A frame with no grass pass adds no sample.
    w.add(100, 1050);
    KT_CHECK_EQ(w.grassCount, 2u);
    // Saturating accumulator.
    w.addGrassUs(0xFFFFFFF0u);
    w.addGrassUs(100u);
    KT_CHECK_EQ(w.grassUsCur, 0xFFFFFFFFu);
    // reset() clears the grass window too.
    w.reset(2000);
    KT_CHECK_EQ(w.grassCount, 0u);
    KT_CHECK_EQ(w.grassUsSum, 0u);
    KT_CHECK_EQ(w.grassUsMax, 0u);
    KT_CHECK_EQ(w.grassUsCur, 0u);
}

static void test_wind_throttle() {
    k2grass::Config c{};   // wind 20 m -> 400
    const k2grass::EffectiveValues ev = k2grass::computeEffective(c);
    // Off: the full radius on every frame.
    for (uint32_t f = 0; f < 6; ++f) KT_CHECK(std::fabs(k2grass::windDistSqForFrame(ev, c, f) - 400.0f) < 1e-3f);
    // On, every 2nd frame full, in between near^2 (10 m -> 100).
    c.windThrottle = true;
    KT_CHECK(std::fabs(k2grass::windDistSqForFrame(ev, c, 0) - 400.0f) < 1e-3f);
    KT_CHECK(std::fabs(k2grass::windDistSqForFrame(ev, c, 1) - 100.0f) < 1e-3f);
    KT_CHECK(std::fabs(k2grass::windDistSqForFrame(ev, c, 2) - 400.0f) < 1e-3f);
    KT_CHECK(std::fabs(k2grass::windDistSqForFrame(ev, c, 3) - 100.0f) < 1e-3f);
    // every = 4: full on 0, 4, 8 only.
    c.windEvery = 4;
    int full = 0;
    for (uint32_t f = 0; f < 12; ++f) {
        if (std::fabs(k2grass::windDistSqForFrame(ev, c, f) - 400.0f) < 1e-3f) ++full;
    }
    KT_CHECK_EQ(full, 3);
    // The cap never raises the radius: near 50 m on a 20 m wind stays at 400; wind 0 stays 0.
    c.windNear = 50.0f;
    KT_CHECK(std::fabs(k2grass::windDistSqForFrame(ev, c, 1) - 400.0f) < 1e-3f);
    k2grass::Config nw = c;
    nw.windDist = 0.0f;
    KT_CHECK(std::fabs(k2grass::windDistSqForFrame(k2grass::computeEffective(nw), nw, 1)) < 1e-6f);
    // An out-of-range every (0) behaves as 1: full radius every frame.
    k2grass::Config z = c;
    z.windEvery = 0;
    KT_CHECK(std::fabs(k2grass::windDistSqForFrame(ev, z, 3) - 400.0f) < 1e-3f);
}

static void test_lod_keep_and_thin_count() {
    k2grass::Config c{};   // fade end 150 m
    c.farLod = true;
    c.lodStart = 60.0f;
    c.lodMinKeep = 0.5f;
    const k2grass::EffectiveValues ev = k2grass::computeEffective(c);
    KT_CHECK(std::fabs(k2grass::lodKeep(0.0f, c, ev) - 1.0f) < 1e-6f);
    KT_CHECK(std::fabs(k2grass::lodKeep(59.0f * 59.0f, c, ev) - 1.0f) < 1e-6f);
    KT_CHECK(std::fabs(k2grass::lodKeep(60.0f * 60.0f, c, ev) - 1.0f) < 1e-6f);
    KT_CHECK(std::fabs(k2grass::lodKeep(150.0f * 150.0f, c, ev) - 0.5f) < 1e-5f);
    KT_CHECK(std::fabs(k2grass::lodKeep(200.0f * 200.0f, c, ev) - 0.5f) < 1e-5f);
    KT_CHECK(std::fabs(k2grass::lodKeep(105.0f * 105.0f, c, ev) - 0.75f) < 1e-5f);   // midpoint of 60..150
    // Off: always 1. Fade end at or below lod_start: always 1.
    k2grass::Config off = c;
    off.farLod = false;
    KT_CHECK(std::fabs(k2grass::lodKeep(140.0f * 140.0f, off, ev) - 1.0f) < 1e-6f);
    k2grass::Config narrow = c;
    narrow.fadeEndDist = 50.0f;
    narrow.fadeStartDist = 20.0f;
    KT_CHECK(std::fabs(k2grass::lodKeep(49.0f * 49.0f, narrow, k2grass::computeEffective(narrow)) - 1.0f) < 1e-6f);

    // thinCount: keep 1 unchanged; 0.5 of 100 quads (400 vertices) -> 200; at least one quad; odd shapes pass through.
    KT_CHECK_EQ(k2grass::thinCount(400, 1.0f), 400);
    KT_CHECK_EQ(k2grass::thinCount(400, 0.5f), 200);
    KT_CHECK_EQ(k2grass::thinCount(400, 0.1f), 40);
    KT_CHECK_EQ(k2grass::thinCount(40, 0.3f), 12);    // ceil(10 * 0.3) = 3 quads
    KT_CHECK_EQ(k2grass::thinCount(4, 0.1f), 4);      // one quad stays
    KT_CHECK_EQ(k2grass::thinCount(8, 0.0f), 4);      // never zero
    KT_CHECK_EQ(k2grass::thinCount(3, 0.5f), 3);      // under one quad
    KT_CHECK_EQ(k2grass::thinCount(0, 0.5f), 0);
    KT_CHECK_EQ(k2grass::thinCount(402, 0.5f), 402);  // not a whole number of quads
    KT_CHECK_EQ(k2grass::thinCount(-8, 0.5f), -8);
    KT_CHECK_EQ(k2grass::thinCount(400, 1.5f), 400);
    // Every result is a whole number of quads, within [4, count].
    for (int q = 1; q <= 64; ++q) {
        for (float k = 0.1f; k < 1.0f; k += 0.07f) {
            const int32_t t = k2grass::thinCount(q * 4, k);
            KT_CHECK(t >= 4 && t <= q * 4 && (t % 4) == 0);
        }
    }
}

static void test_decode_grass_path() {
    // F bit 20 + flag9F60EC low bit -> G; N = 2 -> P follows H.
    const uint32_t F20 = 1u << 20;
    k2grass::GrassPath a = k2grass::decodeGrassPath(F20, -1, -1, 2, 0, 1);
    KT_CHECK_EQ(a.G, 1);
    KT_CHECK_EQ(a.H, 0);
    KT_CHECK_EQ(a.P, a.H);
    KT_CHECK_EQ(a.Q, 0);   // c60FC unset: !G || bit29 -> 0 with G=1 and bit29 clear
    KT_CHECK_EQ(a.M, 0);
    // Same with the C47 term true (c6118 = 1, flag9F62DC = 1): H = 1, P = H.
    k2grass::GrassPath b = k2grass::decodeGrassPath(F20, -1, 1, 2, 1, 1);
    KT_CHECK_EQ(b.G, 1);
    KT_CHECK_EQ(b.H, 1);
    KT_CHECK_EQ(b.P, 1);
    // N == 1 with G: P forced 0; N < 1: P = 0 regardless of H.
    KT_CHECK_EQ(k2grass::decodeGrassPath(F20, -1, 1, 1, 1, 1).P, 0);
    KT_CHECK_EQ(k2grass::decodeGrassPath(0x20001Bu, -1, -1, 0, 0, 0).P, 0);
    // F = 0x20001B -> H = 1 (no G); N = 1 and !G -> P = H.
    k2grass::GrassPath c = k2grass::decodeGrassPath(0x20001Bu, -1, -1, 1, 0, 0);
    KT_CHECK_EQ(c.G, 0);
    KT_CHECK_EQ(c.H, 1);
    KT_CHECK_EQ(c.P, 1);
    KT_CHECK_EQ(c.M, 1);   // bit 1 of 0x1B
    KT_CHECK_EQ(c.Q, 1);   // !G
    KT_CHECK_EQ(k2grass::decodeGrassPath(0x20001Au, -1, -1, 1, 0, 0).H, 0);   // one required bit missing
    // c60FC overrides Q; bit 29 sets it under G.
    KT_CHECK_EQ(k2grass::decodeGrassPath(0, 1, -1, 0, 0, 0).Q, 1);
    KT_CHECK_EQ(k2grass::decodeGrassPath(0, 0, -1, 0, 0, 0).Q, 0);
    KT_CHECK_EQ(k2grass::decodeGrassPath(F20 | (1u << 29), -1, -1, 0, 0, 1).Q, 1);
    // c6118 = 0 (set) beats bit 21: C47 = 0, so H stays 0 even with G and bit 21.
    KT_CHECK_EQ(k2grass::decodeGrassPath(F20 | (1u << 21), -1, 0, 2, 1, 1).H, 0);
    KT_CHECK_EQ(k2grass::decodeGrassPath(F20 | (1u << 21), -1, -1, 2, 1, 1).H, 1);
    // Per-bin class: A unless flag 0x08 and f07c != 0; B2 iff P.
    KT_CHECK_EQ(k2grass::classifyBinPath(0x08, 0, 1), 0);
    KT_CHECK_EQ(k2grass::classifyBinPath(0x00, 5, 1), 0);
    KT_CHECK_EQ(k2grass::classifyBinPath(0x08, 5, 0), 1);
    KT_CHECK_EQ(k2grass::classifyBinPath(0x0F, 5, 1), 2);
}

static void test_perf_window_gpu_and_counters() {
    k2grass::PerfWindow w{};
    w.reset(1000);
    KT_CHECK(std::fabs(w.gpuMsAvg()) < 1e-6f);   // no samples: zeros
    KT_CHECK(std::fabs(w.gpuMsMax()) < 1e-6f);
    KT_CHECK(std::fabs(w.quadsPct() - 100.0f) < 1e-4f);
    w.addGpuNs(2000000ull);   // 2.0 ms
    w.addGpuNs(500000ull);    // 0.5 ms
    w.addGpuNs(1500000ull);   // 1.5 ms
    KT_CHECK_EQ(w.gpuCount, 3u);
    KT_CHECK(std::fabs(w.gpuMsAvg() - 4.0f / 3.0f) < 1e-4f);
    KT_CHECK(std::fabs(w.gpuMsMax() - 2.0f) < 1e-4f);
    w.addQuads(100, 50);
    w.addQuads(100, 100);
    KT_CHECK(std::fabs(w.quadsPct() - 75.0f) < 1e-4f);
    w.addPath(0);
    w.addPath(1);
    w.addPath(2);
    w.addPath(2);
    KT_CHECK_EQ(w.pathA, 1u);
    KT_CHECK_EQ(w.pathB1, 1u);
    KT_CHECK_EQ(w.pathB2, 2u);
    // Frame closing does not disturb them; reset clears them.
    w.add(10, 1000);
    KT_CHECK_EQ(w.gpuCount, 3u);
    w.reset(2000);
    KT_CHECK_EQ(w.gpuCount, 0u);
    KT_CHECK_EQ(w.gpuNsMax, 0ull);
    KT_CHECK_EQ(w.quadsTotal, 0ull);
    KT_CHECK_EQ(w.pathB2, 0u);
}

static void test_ini_roundtrip_and_direct_float_alpha() {
    char buf[4096] = {};
    k2grass::Config orig{};
    orig.fadeStartDist = 75.0f;
    orig.fadeEndDist = 135.0f;
    orig.windDist = 30.0f;
    orig.alphaCutoff = 0.25f;
    orig.perfLog = false;
    orig.trimHidden = false;
    orig.smoothEdges = true;
    orig.gpuTiming = false;
    orig.windThrottle = true;
    orig.windNear = 25.0f;
    orig.windEvery = 4;
    orig.farLod = true;
    orig.lodStart = 80.0f;
    orig.lodMinKeep = 0.25f;
    orig.varySeed = true;
    orig.evenSpread = true;
    orig.densityNoise = true;
    orig.densityAmount = 0.7f;
    orig.densityCell = 12.0f;
    orig.sizeVar = 0.8f;
    const int n = k2grass::formatDefaultIni(buf, sizeof(buf), orig);
    KT_CHECK(n > 0 && static_cast<size_t>(n) < sizeof(buf));

    k2grass::Config parsed{};
    inikit::LoadStats st = k2grass::parseIniText(buf, parsed);
    KT_CHECK_EQ(st.bad, 0);
    KT_CHECK(parsed == orig);
    KT_CHECK(parsed.varySeed && parsed.evenSpread && parsed.densityNoise);
    KT_CHECK(std::fabs(parsed.densityAmount - 0.7f) < 1e-4f && std::fabs(parsed.densityCell - 12.0f) < 1e-4f);
    KT_CHECK(std::fabs(parsed.sizeVar - 0.8f) < 1e-4f);
    KT_CHECK(!parsed.trimHidden && parsed.smoothEdges);
    KT_CHECK(!parsed.gpuTiming && parsed.windThrottle && parsed.farLod);
    KT_CHECK(std::fabs(parsed.windNear - 25.0f) < 1e-4f && parsed.windEvery == 4);
    KT_CHECK(std::fabs(parsed.lodStart - 80.0f) < 1e-4f && std::fabs(parsed.lodMinKeep - 0.25f) < 1e-4f);

    // Defaults round-trip too (trim_hidden = 1, smooth_edges = 0), and each new key counts as a known key.
    char dbuf[4096] = {};
    const int dn = k2grass::formatDefaultIni(dbuf, sizeof(dbuf));
    KT_CHECK(dn > 0 && static_cast<size_t>(dn) < sizeof(dbuf));
    k2grass::Config dparsed{};
    inikit::LoadStats dst = k2grass::parseIniText(dbuf, dparsed);
    KT_CHECK_EQ(dst.bad, 0);
    KT_CHECK_EQ(dst.unknown, 0);
    KT_CHECK(dparsed == k2grass::Config{});
    KT_CHECK(dparsed.trimHidden && !dparsed.smoothEdges);
    // 0.1.9 defaults: vary_seed + even_spread on, density_noise off, vanilla size span.
    KT_CHECK(dparsed.varySeed && dparsed.evenSpread && !dparsed.densityNoise);
    KT_CHECK(std::fabs(dparsed.densityAmount - 0.5f) < 1e-4f && std::fabs(dparsed.densityCell - 6.0f) < 1e-4f);
    KT_CHECK(std::fabs(dparsed.sizeVar - 0.4f) < 1e-4f);
    // 0.1.5 defaults: GPU timer on, both experiments off.
    KT_CHECK(dparsed.gpuTiming && !dparsed.windThrottle && !dparsed.farLod);
    KT_CHECK(std::fabs(dparsed.windNear - 10.0f) < 1e-4f && dparsed.windEvery == 2);
    KT_CHECK(std::fabs(dparsed.lodStart - 60.0f) < 1e-4f && std::fabs(dparsed.lodMinKeep - 0.5f) < 1e-4f);
    char keys[] = "trim_hidden = off\nsmooth_edges = yes\n";
    k2grass::Config kc{};
    inikit::LoadStats kst = k2grass::parseIniText(keys, kc);
    KT_CHECK_EQ(kst.ok, 2);
    KT_CHECK(!kc.trimHidden && kc.smoothEdges);
    // The 7 new keys: clamps apply (wind_near 0..200, wind_every 1..8, lod_start 10..150, lod_min_keep 0.1..1.0).
    char nk[] = "gpu_timing = 0\nwind_throttle = on\nwind_near = 999\nwind_every = 99\nfar_lod = 1\nlod_start = 1\nlod_min_keep = 0\n";
    k2grass::Config nc{};
    inikit::LoadStats nst = k2grass::parseIniText(nk, nc);
    KT_CHECK_EQ(nst.ok, 7);
    KT_CHECK_EQ(nst.bad, 0);
    KT_CHECK(!nc.gpuTiming && nc.windThrottle && nc.farLod);
    KT_CHECK(std::fabs(nc.windNear - 200.0f) < 1e-4f && nc.windEvery == 8);
    KT_CHECK(std::fabs(nc.lodStart - 10.0f) < 1e-4f && std::fabs(nc.lodMinKeep - 0.1f) < 1e-4f);
    char nk2[] = "wind_every = 0\nlod_min_keep = 5\n";
    k2grass::Config nc2{};
    k2grass::parseIniText(nk2, nc2);
    KT_CHECK_EQ(nc2.windEvery, 1);
    KT_CHECK(std::fabs(nc2.lodMinKeep - 1.0f) < 1e-4f);
    char nbad[] = "far_lod = maybe\n";
    k2grass::Config nbc{};
    KT_CHECK_EQ(k2grass::parseIniText(nbad, nbc).bad, 1);
    KT_CHECK(!nbc.farLod);
    char bad[] = "trim_hidden = maybe\n";
    k2grass::Config bc{};
    KT_CHECK_EQ(k2grass::parseIniText(bad, bc).bad, 1);
    KT_CHECK(bc.trimHidden);

    // Also verify direct float alpha_cutoff = 0.18
    char floatIni[] = "fade_start_dist = 50\nfade_end_dist = 100\nalpha_cutoff = 0.18\nwind_dist = 0\n";
    k2grass::Config parsed2{};
    inikit::LoadStats st2 = k2grass::parseIniText(floatIni, parsed2);
    KT_CHECK_EQ(st2.ok, 4);
    KT_CHECK_EQ(st2.bad, 0);
    KT_CHECK(std::fabs(parsed2.alphaCutoff - 0.18f) < 1e-5f);
    KT_CHECK(std::fabs(parsed2.windDist - 0.0f) < 1e-5f);
}

static void test_variety_ini_keys_and_clamps() {
    char k[] = "vary_seed = 1\neven_spread = on\ndensity_noise = yes\ndensity_amount = 5\ndensity_cell = 1\nsize_var = 9\n";
    k2grass::Config c{};
    inikit::LoadStats st = k2grass::parseIniText(k, c);
    KT_CHECK_EQ(st.ok, 6);
    KT_CHECK_EQ(st.bad, 0);
    KT_CHECK(c.varySeed && c.evenSpread && c.densityNoise);
    KT_CHECK(std::fabs(c.densityAmount - 0.9f) < 1e-5f);   // clamp 0..0.9
    KT_CHECK(std::fabs(c.densityCell - 2.0f) < 1e-5f);     // clamp 2..30
    KT_CHECK(std::fabs(c.sizeVar - 0.4f) < 1e-5f);         // 9 is not a scaled value (>2 -> 0.09) -> clamps up to 0.4
    // size_var: direct float and menu-scaled integer both land on the same value; range clamps 0.4..1.2.
    const char* srcs[] = {"size_var = 1.0\n", "size_var = 100\n", "size_var = 120\n", "size_var = 0.1\n", "size_var = 7.5\n", "size_var = 40\n"};
    const float want[] = {1.0f, 1.0f, 1.2f, 0.4f, 0.4f, 0.4f};   // 7.5 is read as scaled (0.075) -> clamps up
    for (int i = 0; i < 6; ++i) {
        char buf[32];
        strcpy(buf, srcs[i]);
        k2grass::Config t{};
        k2grass::parseIniText(buf, t);
        KT_CHECK(std::fabs(t.sizeVar - want[i]) < 1e-4f);
    }
    k2grass::Config big{};
    big.densityAmount = 3.0f;
    big.densityCell = 99.0f;
    big.sizeVar = 5.0f;
    big.sanitize();
    KT_CHECK(big.densityAmount == 0.9f && big.densityCell == 30.0f && big.sizeVar == 1.2f);
    big.sizeVar = 0.0f;
    big.sanitize();
    KT_CHECK(big.sizeVar == 0.4f);
    // Vanilla setting yields exactly the game's double 0.4; 1.0 is exact.
    KT_CHECK(k2grass::sizeSpanValue(0.4f) == 0.4);
    KT_CHECK(k2grass::sizeSpanValue(1.0f) == 1.0);
    KT_CHECK(k2grass::sizeSpanValue(9.0f) == 1.2);
}

static void test_face_seed() {
    const float fa[9] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    uint32_t a[9], b[9], c[9];
    memcpy(a, fa, sizeof a);
    memcpy(b, fa, sizeof b);
    KT_CHECK(k2grass::faceSeed(a) == k2grass::faceSeed(b));
    KT_CHECK(k2grass::faceSeed(a) >= 0);
    // Two faces with the same vanilla rounded sum round(v0.x + v1.y + v2.z) get different seeds.
    const float f1[9] = {10.0f, 0.0f, 0.0f, 0.0f, 20.0f, 0.0f, 0.0f, 0.0f, 30.0f};
    const float f2[9] = {20.0f, 5.0f, 0.0f, 3.0f, 10.0f, 1.0f, 2.0f, 2.0f, 30.0f};
    KT_CHECK(std::lround(f1[0] + f1[4] + f1[8]) == std::lround(f2[0] + f2[4] + f2[8]));
    memcpy(b, f1, sizeof b);
    memcpy(c, f2, sizeof c);
    KT_CHECK(k2grass::faceSeed(b) != k2grass::faceSeed(c));
    // Many distinct faces with one shared rounded sum still spread over many seeds.
    int distinct = 0;
    int32_t seen[64];
    for (int i = 0; i < 64; ++i) {
        const float fv[9] = {static_cast<float>(i), 0.0f, 0.0f, 0.0f, static_cast<float>(63 - i), 0.0f, 0.0f, 0.0f, 0.0f};
        memcpy(a, fv, sizeof a);
        const int32_t sd = k2grass::faceSeed(a);
        bool dup = false;
        for (int j = 0; j < distinct; ++j) dup = dup || seen[j] == sd;
        if (!dup) seen[distinct++] = sd;
    }
    KT_CHECK_EQ(distinct, 64);
}

static void test_uniform_bary() {
    double maxErr = 0.0;
    bool inRange = true;
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 100; ++j) {
            const float a = static_cast<float>(i) / 100.0f;
            const float b = static_cast<float>(j) / 100.0f;
            float w0, w1, w2;
            k2grass::uniformBary(a, b, &w0, &w1, &w2);
            const double err = std::fabs(static_cast<double>(w0) + w1 + w2 - 1.0);
            if (err > maxErr) maxErr = err;
            inRange = inRange && w0 >= 0.0f && w0 <= 1.0f && w1 >= 0.0f && w1 <= 1.0f && w2 >= 0.0f && w2 <= 1.0f;
        }
    }
    KT_CHECK(maxErr < 1e-6);
    KT_CHECK(inRange);
    // Reflection: a + b > 1 folds to (1-a, 1-b); a + b <= 1 passes through.
    float w0, w1, w2;
    k2grass::uniformBary(0.9f, 0.8f, &w0, &w1, &w2);
    KT_CHECK(std::fabs(w0 - 0.1f) < 1e-6f && std::fabs(w1 - 0.2f) < 1e-6f && std::fabs(w2 - 0.7f) < 1e-6f);
    k2grass::uniformBary(0.2f, 0.3f, &w0, &w1, &w2);
    KT_CHECK(w0 == 0.2f && w1 == 0.3f && std::fabs(w2 - 0.5f) < 1e-6f);
    // Non-finite input still yields valid weights.
    k2grass::uniformBary(NAN, 0.5f, &w0, &w1, &w2);
    KT_CHECK(std::fabs(static_cast<double>(w0) + w1 + w2 - 1.0) < 1e-6 && w0 >= 0.0f && w2 >= 0.0f);
}

static void test_value_noise_and_density() {
    // Range and continuity (small steps never jump, including across lattice cell edges).
    bool inRange = true;
    float maxStep = 0.0f;
    float prev = k2grass::valueNoise2(-20.0f, 3.3f);
    for (int i = 1; i <= 8000; ++i) {
        const float x = -20.0f + static_cast<float>(i) * 0.005f;
        const float v = k2grass::valueNoise2(x, 3.3f);
        inRange = inRange && v >= 0.0f && v <= 1.0f;
        const float d = std::fabs(v - prev);
        if (d > maxStep) maxStep = d;
        prev = v;
    }
    KT_CHECK(inRange);
    KT_CHECK(maxStep < 0.02f);   // slope <= 1.5 per unit * 0.005 step
    // Exactly at and just beside a lattice edge the value agrees.
    KT_CHECK(std::fabs(k2grass::valueNoise2(4.0f, 2.0f) - k2grass::valueNoise2(3.9999f, 2.0f)) < 1e-3f);
    KT_CHECK(std::fabs(k2grass::valueNoise2(4.0f, 2.0f) - k2grass::valueNoise2(4.0f, 1.9999f)) < 1e-3f);
    // Not constant: the lattice has both low and high corners.
    float lo = 1.0f, hi = 0.0f;
    for (int i = 0; i < 400; ++i) {
        const float v = k2grass::valueNoise2(static_cast<float>(i) * 0.7f, static_cast<float>(i) * 0.3f);
        if (v < lo) lo = v;
        if (v > hi) hi = v;
    }
    KT_CHECK(lo < 0.25f && hi > 0.75f);
    KT_CHECK(k2grass::valueNoise2(NAN, 1.0f) == 0.5f);

    // densityCount: amount 0 only dithers within +-0.5 of x.
    k2grass::Config c{};
    c.densityNoise = true;
    c.densityAmount = 0.0f;
    bool within = true;
    for (int i = 0; i < 500; ++i) {
        const float x = 7.0f + static_cast<float>(i % 5);
        const float y = k2grass::densityCount(x, static_cast<float>(i) * 1.37f, static_cast<float>(i) * -0.61f, c);
        within = within && y >= x - 0.5f && y <= x + 0.5f;
    }
    KT_CHECK(within);
    // amount 0.5: mean over a grid of centroids stays within 3% of x; never negative; bounded above.
    c.densityAmount = 0.5f;
    c.densityCell = 6.0f;
    const float x = 20.0f;
    double sum = 0.0;
    int count = 0;
    bool nonNeg = true;
    float mn = 1e9f, mx = -1e9f;
    for (int i = 0; i < 200; ++i) {
        for (int j = 0; j < 200; ++j) {
            const float cx = -170.0f + static_cast<float>(i) * 1.7f;
            const float cy = -170.0f + static_cast<float>(j) * 1.7f;
            const float y = k2grass::densityCount(x, cx, cy, c);
            nonNeg = nonNeg && y >= 0.0f && y <= 4.0f * x + 4.0f;
            if (y < mn) mn = y;
            if (y > mx) mx = y;
            sum += y;
            ++count;
        }
    }
    KT_CHECK(nonNeg);
    KT_CHECK(std::fabs(sum / count - x) < 0.03 * x);
    KT_CHECK(mn < 0.75f * x && mx > 1.25f * x);   // patches actually vary
    // Small and zero counts never go negative.
    bool smallOk = true;
    for (int i = 0; i < 300; ++i) {
        smallOk = smallOk && k2grass::densityCount(0.0f, static_cast<float>(i), 0.5f * static_cast<float>(i), c) >= 0.0f;
        smallOk = smallOk && k2grass::densityCount(0.3f, static_cast<float>(i), 1.5f * static_cast<float>(i), c) >= 0.0f;
    }
    KT_CHECK(smallOk);
    // Garbage count passes through untouched.
    KT_CHECK(k2grass::densityCount(-1.0f, 0.0f, 0.0f, c) == -1.0f);
}

static void test_perf_window_variety_counters() {
    k2grass::PerfWindow w;
    w.reset(100);
    w.addBlade();
    w.addBlade();
    w.addDensity(10.0f, 12.0f);
    w.addDensity(5.0f, 4.0f);
    KT_CHECK_EQ(w.blades, 2u);
    KT_CHECK_EQ(w.densFaces, 2u);
    KT_CHECK(std::fabs(w.densIn - 15.0) < 1e-9 && std::fabs(w.densOut - 16.0) < 1e-9);
    w.reset(200);
    KT_CHECK_EQ(w.blades, 0u);
    KT_CHECK_EQ(w.densFaces, 0u);
    // Load-time hook calls (no frames) survive the gap reset in add().
    w.add(10, 300);
    w.addSeed();
    w.addDensity(7.0f, 9.0f);
    w.add(10, 300 + k2grass::kPerfGapMs + 500);
    KT_CHECK_EQ(w.frames, 1u);
    KT_CHECK_EQ(w.seeds, 1u);
    KT_CHECK_EQ(w.densFaces, 1u);
    KT_CHECK(std::fabs(w.densIn - 7.0) < 1e-9 && std::fabs(w.densOut - 9.0) < 1e-9);
}

static void test_modconfig_schema_bindings() {
    mcdecl::PageDecl pd{};
    char err[96] = {};
    KT_CHECK(mcdecl::parseSchema(k2grass::kPageSchema, strlen(k2grass::kPageSchema), pd, err, sizeof(err)));
    KT_CHECK_EQ(pd.nRows, 13);
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
    test_collect_dist_trim();
    test_bin_alpha_curve();
    test_sanitize_clamps_and_span_guard();
    test_menu_fade_link();
    test_perf_window();
    test_perf_window_grass_time();
    test_wind_throttle();
    test_lod_keep_and_thin_count();
    test_decode_grass_path();
    test_perf_window_gpu_and_counters();
    test_ini_roundtrip_and_direct_float_alpha();
    test_variety_ini_keys_and_clamps();
    test_face_seed();
    test_uniform_bary();
    test_value_noise_and_density();
    test_perf_window_variety_counters();
    test_modconfig_schema_bindings();
    return kt::Report("grass_draw_distance_test");
}
