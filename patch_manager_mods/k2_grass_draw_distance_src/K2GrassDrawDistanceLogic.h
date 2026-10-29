// K2GrassDrawDistanceLogic.h (0.1.9-dev) -- Pure config, math, and ModConfig schema for k2-grass-draw-distance.
// Header-only and host-testable (no engine addresses or Win32 calls).
#pragma once

#include "IniKit.h"
#include "ModConfigDecl.h"
#include <cmath>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace k2grass {

constexpr const char* kModId = "k2-grass-draw-distance";
constexpr const char* kVersion = "0.1.9-dev";
constexpr const char* kIniFile = "k2_grass_draw_distance.ini";
constexpr const char* kIniSection = "Grass";

// Vanilla swkotor2.exe grass parameters:
//   _DAT_00a0f070 = 225.0f (15.0m^2) fade start & CPU wind radius
//   _DAT_00a0f074 = 900.0f (30.0m^2) octree build & fade end radius
//   _DAT_009f5d38 = 0.40f  glAlphaFunc(GL_GREATER, ref) threshold
constexpr float kVanillaFadeStartDist = 15.0f;
constexpr float kVanillaFadeEndDist = 30.0f;
constexpr float kVanillaWindDist = 15.0f;
constexpr float kVanillaAlphaCutoff = 0.40f;

// Extended defaults (5x draw distance = 150m, fade 120m..150m, 20m wind, vanilla 0.40 alpha cutoff).
// 150m played smooth in P3 (GR2: 895 bins at 57-59 fps, no cost over 30m); with the vanilla cutoff opaque
// texels clip at ~138.8m and near grass keeps the vanilla blade edges (a lower cutoff softens every blade).
constexpr bool kDefaultEnabled = true;
constexpr float kDefaultFadeStartDist = 120.0f;
constexpr float kDefaultFadeEndDist = 150.0f;
constexpr float kDefaultWindDist = 20.0f;
constexpr float kDefaultAlphaCutoff = 0.40f;
constexpr bool kDefaultVerbose = false;
constexpr bool kDefaultPerfLog = true;
// 0.1.4: trim the octree collect radius to the distance where grass stops being visible (hidden bins are never drawn),
// and optionally smooth the blade edges with alpha-to-coverage when the GL context has a multisample buffer.
constexpr bool kDefaultTrimHidden = true;
constexpr bool kDefaultSmoothEdges = false;
// Margin (m) added to the alpha-test clip distance when trimming, so bins straddling the clip radius still render.
constexpr float kTrimMarginMeters = 2.0f;
// 0.1.5 experiments (all off by default except the log-only GPU timer; with them off the picture and CPU work equal 0.1.4):
//   gpu_timing   GL_TIME_ELAPSED query around the grass bucket loop (PERF gpu_ms_*), ini only
//   wind_*       PF2b: only every wind_every-th frame animates wind out to wind_dist; in between wind is capped to wind_near
//   far_lod      PF2c: far bins draw only a prefix of their quads (linear from 100% at lod_start to lod_min_keep at the fade end)
constexpr bool kDefaultGpuTiming = true;
constexpr bool kDefaultWindThrottle = false;
constexpr float kDefaultWindNear = 10.0f;
constexpr int kDefaultWindEvery = 2;
constexpr bool kDefaultFarLod = false;
constexpr float kDefaultLodStart = 60.0f;
constexpr float kDefaultLodMinKeep = 0.5f;
constexpr float kMinWindNear = 0.0f;
constexpr float kMaxWindNear = 200.0f;
constexpr int kMinWindEvery = 1;
constexpr int kMaxWindEvery = 8;
constexpr float kMinLodStart = 10.0f;
constexpr float kMaxLodStart = 150.0f;
constexpr float kMinLodMinKeep = 0.1f;
constexpr float kMaxLodMinKeep = 1.0f;
// 0.1.6 variety (doc 50 PV1-PV4); 0.1.9: vary_seed + even_spread default on (GR7 PASS), density_noise off, size_var vanilla. Generation tweaks apply to bins built after the change.
//   vary_seed      PV1: per-face srand seed hashed from the face vertices (vanilla seeds from a rounded coordinate sum)
//   even_spread    PV2: uniform barycentric weights per blade (vanilla clusters toward the face centre)
//   density_noise  PV3: smooth value-noise density patches (density_amount 0..0.9, density_cell metres 2..30)
//   size_var       PV4: blade size span (vanilla 0.4 = the double the size FMUL reads; 0.4..1.2)
constexpr bool kDefaultVarySeed = true;
constexpr bool kDefaultEvenSpread = true;
constexpr bool kDefaultDensityNoise = false;
constexpr float kDefaultDensityAmount = 0.5f;
constexpr float kDefaultDensityCell = 6.0f;
constexpr float kVanillaSizeVar = 0.4f;
constexpr float kDefaultSizeVar = kVanillaSizeVar;
constexpr float kMinDensityAmount = 0.0f;
constexpr float kMaxDensityAmount = 0.9f;
constexpr float kMinDensityCell = 2.0f;
constexpr float kMaxDensityCell = 30.0f;
constexpr float kMinSizeVar = 0.4f;
constexpr float kMaxSizeVar = 1.2f;

// Clamp bounds. 150m is 25x the vanilla 30m grass area; raise only after P3 measures the frame cost.
constexpr float kMinFadeStartDist = 5.0f;
constexpr float kMaxFadeStartDist = 145.0f;
constexpr float kMinFadeEndDist = 10.0f;
constexpr float kMaxFadeEndDist = 150.0f;
constexpr float kMinFadeSpan = 5.0f;
constexpr float kMinWindDist = 0.0f;
constexpr float kMaxWindDist = 200.0f;
constexpr float kMinAlphaCutoff = 0.01f;
constexpr float kMaxAlphaCutoff = 0.90f;

inline float clampf(float v, float lo, float hi) {
    if (!std::isfinite(v)) return lo;
    return v < lo ? lo : (v > hi ? hi : v);
}

struct Config {
    bool enabled = kDefaultEnabled;
    float fadeStartDist = kDefaultFadeStartDist;
    float fadeEndDist = kDefaultFadeEndDist;
    float windDist = kDefaultWindDist;
    float alphaCutoff = kDefaultAlphaCutoff;
    bool verbose = kDefaultVerbose;
    bool perfLog = kDefaultPerfLog;
    bool trimHidden = kDefaultTrimHidden;
    bool smoothEdges = kDefaultSmoothEdges;
    bool gpuTiming = kDefaultGpuTiming;
    bool windThrottle = kDefaultWindThrottle;
    float windNear = kDefaultWindNear;
    int windEvery = kDefaultWindEvery;
    bool farLod = kDefaultFarLod;
    float lodStart = kDefaultLodStart;
    float lodMinKeep = kDefaultLodMinKeep;
    bool varySeed = kDefaultVarySeed;
    bool evenSpread = kDefaultEvenSpread;
    bool densityNoise = kDefaultDensityNoise;
    float densityAmount = kDefaultDensityAmount;
    float densityCell = kDefaultDensityCell;
    float sizeVar = kDefaultSizeVar;

    void sanitize() {
        // fade_end_dist is the cost knob (build radius), so a fade start at or past it pulls the start
        // down rather than pushing the draw distance up behind the menu's back.
        fadeEndDist = clampf(fadeEndDist, kMinFadeEndDist, kMaxFadeEndDist);
        fadeStartDist = clampf(fadeStartDist, kMinFadeStartDist, kMaxFadeStartDist);
        if (fadeStartDist > fadeEndDist - kMinFadeSpan) fadeStartDist = fadeEndDist - kMinFadeSpan;
        windDist = clampf(windDist, kMinWindDist, kMaxWindDist);
        alphaCutoff = clampf(alphaCutoff, kMinAlphaCutoff, kMaxAlphaCutoff);
        windNear = clampf(windNear, kMinWindNear, kMaxWindNear);
        windEvery = windEvery < kMinWindEvery ? kMinWindEvery : (windEvery > kMaxWindEvery ? kMaxWindEvery : windEvery);
        lodStart = clampf(lodStart, kMinLodStart, kMaxLodStart);
        lodMinKeep = clampf(lodMinKeep, kMinLodMinKeep, kMaxLodMinKeep);
        densityAmount = clampf(densityAmount, kMinDensityAmount, kMaxDensityAmount);
        densityCell = clampf(densityCell, kMinDensityCell, kMaxDensityCell);
        sizeVar = clampf(sizeVar, kMinSizeVar, kMaxSizeVar);
    }

    bool operator==(const Config& o) const {
        return enabled == o.enabled &&
               std::fabs(fadeStartDist - o.fadeStartDist) < 1e-4f &&
               std::fabs(fadeEndDist - o.fadeEndDist) < 1e-4f &&
               std::fabs(windDist - o.windDist) < 1e-4f &&
               std::fabs(alphaCutoff - o.alphaCutoff) < 1e-4f &&
               verbose == o.verbose &&
               perfLog == o.perfLog &&
               trimHidden == o.trimHidden &&
               smoothEdges == o.smoothEdges &&
               gpuTiming == o.gpuTiming &&
               windThrottle == o.windThrottle &&
               std::fabs(windNear - o.windNear) < 1e-4f &&
               windEvery == o.windEvery &&
               farLod == o.farLod &&
               std::fabs(lodStart - o.lodStart) < 1e-4f &&
               std::fabs(lodMinKeep - o.lodMinKeep) < 1e-4f &&
               varySeed == o.varySeed &&
               evenSpread == o.evenSpread &&
               densityNoise == o.densityNoise &&
               std::fabs(densityAmount - o.densityAmount) < 1e-4f &&
               std::fabs(densityCell - o.densityCell) < 1e-4f &&
               std::fabs(sizeVar - o.sizeVar) < 1e-4f;
    }
    bool operator!=(const Config& o) const { return !(*this == o); }
};

struct EffectiveValues {
    float fadeStartSq;
    float fadeEndSq;
    float windDistSq;
    float alphaCutoff;
};

inline EffectiveValues computeEffective(const Config& raw) {
    Config c = raw;
    c.sanitize();
    if (!c.enabled) {
        return {
            kVanillaFadeStartDist * kVanillaFadeStartDist,
            kVanillaFadeEndDist * kVanillaFadeEndDist,
            kVanillaWindDist * kVanillaWindDist,
            kVanillaAlphaCutoff,
        };
    }
    return {
        c.fadeStartDist * c.fadeStartDist,
        c.fadeEndDist * c.fadeEndDist,
        c.windDist * c.windDist,
        c.alphaCutoff,
    };
}

// Replicates CAurTriangleBin::RenderGrassPolys alpha calculation for testing/verification.
inline float computeBinAlpha(float binDistSq, const EffectiveValues& ev) {
    if (binDistSq < ev.fadeStartSq) return 1.0f;
    if (binDistSq < ev.fadeEndSq) {
        const float denom = ev.fadeEndSq - ev.fadeStartSq;
        if (denom <= 0.0f) return 0.0f;
        return 1.0f - (binDistSq - ev.fadeStartSq) / denom;
    }
    return 0.0f;
}

// Distance in meters where opaque grass texels (tex_alpha = 1.0) hit glAlphaFunc(GL_GREATER, alphaCutoff).
inline float effectiveClipDistMeters(const EffectiveValues& ev) {
    const float spanSq = ev.fadeEndSq - ev.fadeStartSq;
    if (spanSq <= 0.0f) return std::sqrt(ev.fadeStartSq);
    const float clipSq = ev.fadeStartSq + (1.0f - ev.alphaCutoff) * spanSq;
    return std::sqrt(clipSq > 0.0f ? clipSq : 0.0f);
}

// Squared octree collect radius the patched FLD at 0x00499ADC reads (the fade curve still reads fadeEndSq).
// Trim: bins past (clip distance + margin) are fully hidden by the alpha test, so they need not be collected.
inline float collectDistSq(const EffectiveValues& ev, bool trim) {
    if (!trim) return ev.fadeEndSq;
    const float r = effectiveClipDistMeters(ev) + kTrimMarginMeters;
    const float sq = r * r;
    return sq < ev.fadeEndSq ? sq : ev.fadeEndSq;
}

// PF2b wind throttle. The engine's wind gate has no dt (a bin that passes it advances its sway by a fixed step), so a
// bin skipped on k-1 of k frames sways at 1/k speed; that is the A/B question. Every wind_every-th frame keeps the full
// radius, the frames in between cap it at wind_near, so far bins animate only on the full frames.
inline float windDistSqForFrame(const EffectiveValues& ev, const Config& cfg, uint32_t frame) {
    if (!cfg.windThrottle) return ev.windDistSq;
    int every = cfg.windEvery < kMinWindEvery ? kMinWindEvery : (cfg.windEvery > kMaxWindEvery ? kMaxWindEvery : cfg.windEvery);
    if (frame % static_cast<uint32_t>(every) == 0) return ev.windDistSq;
    const float n = clampf(cfg.windNear, kMinWindNear, kMaxWindNear);
    const float nsq = n * n;
    return nsq < ev.windDistSq ? nsq : ev.windDistSq;
}

// PF2c far LOD: fraction of a bin's quads to draw. 1 at or below lod_start, then linear in distance down to
// lod_min_keep at the fade end (and beyond). A fade end at or below lod_start leaves everything at 1.
inline float lodKeep(float binDistSq, const Config& cfg, const EffectiveValues& ev) {
    if (!cfg.farLod) return 1.0f;
    const float start = clampf(cfg.lodStart, kMinLodStart, kMaxLodStart);
    const float minKeep = clampf(cfg.lodMinKeep, kMinLodMinKeep, kMaxLodMinKeep);
    const float end = std::sqrt(ev.fadeEndSq > 0.0f ? ev.fadeEndSq : 0.0f);
    if (end <= start) return 1.0f;
    const float d = std::sqrt(binDistSq > 0.0f ? binDistSq : 0.0f);
    if (d <= start) return 1.0f;
    if (d >= end) return minKeep;
    return 1.0f - (1.0f - minKeep) * (d - start) / (end - start);
}

// Vertex count to hand glDrawArrays(GL_QUADS) for a bin kept at `keep`: the first ceil(quads * keep) quads, at least 1.
// Counts that are not a whole number of quads (or under one quad) pass through untouched.
inline int32_t thinCount(int32_t count, float keep) {
    if (!(keep < 1.0f) || count < 4 || (count % 4) != 0) return count;
    const int32_t quads = count / 4;
    int32_t k = static_cast<int32_t>(std::ceil(static_cast<double>(quads) * static_cast<double>(keep) - 1e-6));
    if (k < 1) k = 1;
    if (k > quads) k = quads;
    return k * 4;
}

// PF2a path probe: which branch of the engine's grass draw setup the current globals select. Decoded from globals only
// (the game's own helpers cache on first call). F = flags word [0x00A32DF8], N = [0x009F60F0], c60FC / c6118 = ints with
// a -1 "unset" sentinel, flag9F62DC / flag9F60EC = ints whose low bit is the flag.
struct GrassPath {
    int G, H, P, Q, M;
    bool operator==(const GrassPath& o) const { return G == o.G && H == o.H && P == o.P && Q == o.Q && M == o.M; }
    bool operator!=(const GrassPath& o) const { return !(*this == o); }
};

inline GrassPath decodeGrassPath(uint32_t F, int32_t c60FC, int32_t c6118, int32_t N, int32_t flag9F62DC, int32_t flag9F60EC) {
    const int bit1 = static_cast<int>((F >> 1) & 1u);
    const int bit20 = static_cast<int>((F >> 20) & 1u);
    const int bit21 = static_cast<int>((F >> 21) & 1u);
    const int bit29 = static_cast<int>((F >> 29) & 1u);
    const int G = bit20 & (flag9F60EC & 1);
    const int c47 = ((c6118 != -1) ? (c6118 & 1) : bit21) & (flag9F62DC & 1);
    const int H = (((F & 0x20001Bu) == 0x20001Bu) || (c47 && G)) ? 1 : 0;
    int P = 0;
    if (N >= 1) P = (!G || N != 1) ? H : 0;
    const int Q = (c60FC != -1) ? (c60FC == 1 ? 1 : 0) : ((!G || bit29) ? 1 : 0);
    return {G, H, P, Q, bit1};
}

// Per-bin path class for the PERF counters: 0 = A, 1 = B1, 2 = B2. B iff the bin flag 0x08 is set and [0x00A0F07C] != 0.
inline int classifyBinPath(uint8_t binFlags, int32_t f07c, int P) {
    if (!((binFlags & 0x08) && f07c != 0)) return 0;
    return P ? 2 : 1;
}

// ---- 0.1.6 variety (doc 50 PV1-PV4): pure math the three CAurTriangleBin hooks apply to the game's stack slots ----

// PV1: seed for the per-face srand. FNV-1a over the 36 bytes of the nine face floats (v0.xyz, v1.xyz, v2.xyz as raw
// little-endian u32 bits), then the murmur3 fmix32 finalizer; result is non-negative so it is a valid int seed.
inline uint32_t fmix32(uint32_t h) {
    h ^= h >> 16;
    h *= 0x85EBCA6Bu;
    h ^= h >> 13;
    h *= 0xC2B2AE35u;
    h ^= h >> 16;
    return h;
}
inline int32_t faceSeed(const uint32_t bits[9]) {
    uint32_t h = 2166136261u;
    for (int i = 0; i < 9; ++i) {
        for (int b = 0; b < 4; ++b) {
            h ^= (bits[i] >> (8 * b)) & 0xFFu;
            h *= 16777619u;
        }
    }
    return static_cast<int32_t>(fmix32(h) & 0x7FFFFFFFu);
}

// PV2: uniform barycentric weights from the game's two raw blade randoms a, b in [0,1). Reflect when a+b > 1 (the
// classic square-to-triangle fold) so the point is uniform over the face. The weights also drive vertex colour and
// lightmap UVs, so they are clamped to [0,1] and sum to 1.
inline void uniformBary(float a, float b, float* w0, float* w1, float* w2) {
    if (!std::isfinite(a) || !std::isfinite(b)) {
        a = 1.0f / 3.0f;
        b = 1.0f / 3.0f;
    }
    a = a < 0.0f ? 0.0f : (a > 1.0f ? 1.0f : a);
    b = b < 0.0f ? 0.0f : (b > 1.0f ? 1.0f : b);
    if (a + b > 1.0f) {
        a = 1.0f - a;
        b = 1.0f - b;
    }
    float c = 1.0f - (a + b);
    if (c < 0.0f) c = 0.0f;
    *w0 = a;
    *w1 = b;
    *w2 = c;
}

// PV3: value noise over an integer lattice. Hash -> [0,1], smoothstep weights, bilinear blend; result in [0,1] and
// continuous in (x, y).
inline uint32_t latticeHash(int32_t ix, int32_t iy) {
    uint32_t h = static_cast<uint32_t>(ix) * 0x9E3779B1u ^ (static_cast<uint32_t>(iy) * 0x85EBCA77u + 0x27D4EB2Fu);
    return fmix32(h);
}
inline float latticeValue(int32_t ix, int32_t iy) {
    return static_cast<float>(latticeHash(ix, iy) >> 8) / 16777215.0f;   // exactly [0,1]
}
inline float valueNoise2(float x, float y) {
    if (!std::isfinite(x) || !std::isfinite(y)) return 0.5f;
    const double lim = 1.0e9;
    const double fx = std::floor(static_cast<double>(x) < -lim ? -lim : (static_cast<double>(x) > lim ? lim : static_cast<double>(x)));
    const double fy = std::floor(static_cast<double>(y) < -lim ? -lim : (static_cast<double>(y) > lim ? lim : static_cast<double>(y)));
    const int32_t ix = static_cast<int32_t>(fx);
    const int32_t iy = static_cast<int32_t>(fy);
    float tx = static_cast<float>(static_cast<double>(x) - fx);
    float ty = static_cast<float>(static_cast<double>(y) - fy);
    tx = tx < 0.0f ? 0.0f : (tx > 1.0f ? 1.0f : tx);
    ty = ty < 0.0f ? 0.0f : (ty > 1.0f ? 1.0f : ty);
    const float sx = tx * tx * (3.0f - 2.0f * tx);
    const float sy = ty * ty * (3.0f - 2.0f * ty);
    const float v00 = latticeValue(ix, iy);
    const float v10 = latticeValue(ix + 1, iy);
    const float v01 = latticeValue(ix, iy + 1);
    const float v11 = latticeValue(ix + 1, iy + 1);
    const float a = v00 + (v10 - v00) * sx;
    const float b = v01 + (v11 - v01) * sx;
    const float r = a + (b - a) * sy;
    return r < 0.0f ? 0.0f : (r > 1.0f ? 1.0f : r);
}

// Uniform [0,1) from a face centroid's float bits (the stochastic-rounding dither).
inline float centroidDither(float cx, float cy) {
    uint32_t bx = 0, by = 0;
    memcpy(&bx, &cx, sizeof bx);
    memcpy(&by, &cy, sizeof by);
    return static_cast<float>(fmix32(bx * 0x9E3779B1u ^ fmix32(by + 0x7F4A7C15u)) >> 8) / 16777216.0f;
}

// PV3: the face's blade count x (a float; the game's FRNDINT rounds it afterwards) scaled by smooth patch noise and
// dithered so the rounded expectation equals x * f. Clamped to [0, 4x + 4].
inline float densityCount(float x, float cx, float cy, const Config& cfg) {
    if (!std::isfinite(x) || x < 0.0f) return x;   // never feed the game something it did not already hold
    const float amount = clampf(cfg.densityAmount, kMinDensityAmount, kMaxDensityAmount);
    const float cell = clampf(cfg.densityCell, kMinDensityCell, kMaxDensityCell);
    const float n = valueNoise2(cx / cell, cy / cell);
    const float f = 1.0f + amount * (2.0f * n - 1.0f);
    float y = x * f + (centroidDither(cx, cy) - 0.5f);
    const float hi = 4.0f * x + 4.0f;
    if (y > hi) y = hi;
    if (y < 0.0f) y = 0.0f;
    return y;
}

// PV4: the double the patched size FMUL reads. Snapped to 0.001 so the vanilla 0.4f setting yields the exact double 0.4
// the game stores at 0x009921C8.
inline double sizeSpanValue(float sizeVar) {
    const float v = clampf(sizeVar, kMinSizeVar, kMaxSizeVar);
    return std::floor(static_cast<double>(v) * 1000.0 + 0.5) / 1000.0;
}

// Menu link between the Fade Start and Draw Distance sliders (kPageSchema rows fade_start / fade_end): the highest
// Fade Start the menu offers for a Draw Distance, on the row's own 15 m grid and >= kMinFadeSpan below the end.
// Same direction as Config::sanitize (the start moves, never the end), so the sliders show what the engine uses.
constexpr int32_t kMenuFadeStartMin = 15;
constexpr int32_t kMenuFadeStartStep = 15;

inline int32_t menuFadeStartMax(int32_t fadeEnd) {
    const int32_t room = fadeEnd - static_cast<int32_t>(kMinFadeSpan);
    if (room < kMenuFadeStartMin) return kMenuFadeStartMin;
    return kMenuFadeStartMin + (room - kMenuFadeStartMin) / kMenuFadeStartStep * kMenuFadeStartStep;
}
inline int32_t menuLinkFadeStart(int32_t fadeStart, int32_t fadeEnd) {
    const int32_t hi = menuFadeStartMax(fadeEnd);
    return fadeStart > hi ? hi : fadeStart;
}

// P3 calibration window: one sample per grass frame (Scene::CollectAndTraverse hook).
//   binsInRange = previous frame's in-range bin list count (engine global read by the DLL; the per-frame
//                 list swap runs before the octree query refills the current list, doc 48 section 1.5)
constexpr unsigned kPerfWindowMs = 5000;
// A longer gap between grass frames (menu, pause, area load) drops the partial window so fps counts grass time only.
constexpr unsigned kPerfGapMs = 1000;

struct PerfWindow {
    unsigned startMs = 0;
    uint32_t frames = 0;
    uint64_t binsSum = 0;
    uint32_t binsMax = 0;
    unsigned lastMs = 0;
    // Grass-pass CPU time (Scene::DoBinBucket bucket loop, microseconds). The pass can run more than once per frame, so
    // addGrassUs() accumulates into grassUsCur and the next add() (the next frame's collect hook) closes that frame.
    uint64_t grassUsSum = 0;
    uint32_t grassUsMax = 0;
    uint32_t grassCount = 0;
    uint32_t grassUsCur = 0;
    // GPU time of the same pass (GL_TIME_ELAPSED, nanoseconds); results arrive a few frames late and close per result.
    uint64_t gpuNsSum = 0;
    uint64_t gpuNsMax = 0;
    uint32_t gpuCount = 0;
    // Far LOD / path probe counters (grass hooks, per window).
    uint64_t quadsTotal = 0;
    uint64_t quadsDrawn = 0;
    uint32_t pathA = 0;
    uint32_t pathB1 = 0;
    uint32_t pathB2 = 0;
    // 0.1.6 variety counters (per window).
    uint32_t blades = 0;       // PV2 hook calls (blades generated)
    uint32_t seeds = 0;        // PV1 hook calls (faces reseeded)
    uint32_t densFaces = 0;    // PV3 hook calls (faces)
    double densIn = 0.0;       // sum of the game's face counts before the change
    double densOut = 0.0;      // sum after

    void addBlade() { ++blades; }
    void addSeed() { ++seeds; }
    void addDensity(float in, float out) {
        ++densFaces;
        densIn += in;
        densOut += out;
    }
    void addGpuNs(uint64_t ns) {
        gpuNsSum += ns;
        if (ns > gpuNsMax) gpuNsMax = ns;
        ++gpuCount;
    }
    void addQuads(uint32_t total, uint32_t drawn) {
        quadsTotal += total;
        quadsDrawn += drawn;
    }
    void addPath(int cls) {
        if (cls == 2) ++pathB2;
        else if (cls == 1) ++pathB1;
        else ++pathA;
    }
    void addGrassUs(uint32_t us) {
        const uint64_t t = static_cast<uint64_t>(grassUsCur) + us;
        grassUsCur = t > 0xFFFFFFFFull ? 0xFFFFFFFFu : static_cast<uint32_t>(t);
    }
    void add(uint32_t binsInRange, unsigned nowMs) {
        if (frames > 0 && (nowMs - lastMs) > kPerfGapMs) {
            // Grass faces are counted and bins built during the area load (no frames), so the variety counters
            // survive the gap reset; otherwise every load-time hook call is wiped before it is logged.
            const uint32_t b = blades, sd = seeds, df = densFaces;
            const double di = densIn, dout = densOut;
            reset(nowMs);
            blades = b;
            seeds = sd;
            densFaces = df;
            densIn = di;
            densOut = dout;
        }
        if (grassUsCur > 0) {
            grassUsSum += grassUsCur;
            if (grassUsCur > grassUsMax) grassUsMax = grassUsCur;
            ++grassCount;
            grassUsCur = 0;
        }
        lastMs = nowMs;
        ++frames;
        binsSum += binsInRange;
        if (binsInRange > binsMax) binsMax = binsInRange;
    }
    // True once kPerfWindowMs has passed since startMs with at least one frame (GetTickCount wrap safe).
    bool due(unsigned nowMs) const { return frames > 0 && (nowMs - startMs) >= kPerfWindowMs; }
    float fps(unsigned nowMs) const {
        const unsigned dt = nowMs - startMs;
        return dt ? frames * 1000.0f / static_cast<float>(dt) : 0.0f;
    }
    float binsAvg() const { return frames ? static_cast<float>(binsSum) / frames : 0.0f; }
    float grassMsAvg() const { return grassCount ? static_cast<float>(grassUsSum) / grassCount / 1000.0f : 0.0f; }
    float grassMsMax() const { return static_cast<float>(grassUsMax) / 1000.0f; }
    float gpuMsAvg() const { return gpuCount ? static_cast<float>(static_cast<double>(gpuNsSum) / gpuCount / 1.0e6) : 0.0f; }
    float gpuMsMax() const { return static_cast<float>(static_cast<double>(gpuNsMax) / 1.0e6); }
    // Share of grass quads actually drawn (100 when none were counted).
    float quadsPct() const { return quadsTotal ? static_cast<float>(100.0 * static_cast<double>(quadsDrawn) / static_cast<double>(quadsTotal)) : 100.0f; }
    void reset(unsigned nowMs) { *this = PerfWindow{}; startMs = nowMs; }
};

inline bool strEqI(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        if (tolower(static_cast<unsigned char>(*a)) != tolower(static_cast<unsigned char>(*b))) return false;
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

inline bool parseBoolVal(const char* val, bool* out) {
    if (!val || !*val) return false;
    if (strcmp(val, "1") == 0 || strEqI(val, "true") || strEqI(val, "yes") || strEqI(val, "on")) {
        *out = true;
        return true;
    }
    if (strcmp(val, "0") == 0 || strEqI(val, "false") || strEqI(val, "no") || strEqI(val, "off")) {
        *out = false;
        return true;
    }
    return false;
}

// Parse INI buffer in place (modifies `text`). Accepts both float (0.20) and scaled percent (20 -> 0.20) for alpha_cutoff.
inline inikit::LoadStats parseIniText(char* text, Config& cfg) {
    cfg = Config{};
    inikit::LoadStats st = inikit::forEachLine(text, [&](int /*lineNo*/, const char* key, const char* val) -> inikit::IniVerdict {
        if (strcmp(key, "enabled") == 0) {
            bool b = true;
            if (!parseBoolVal(val, &b)) return inikit::IniVerdictBad;
            cfg.enabled = b;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "verbose") == 0) {
            bool b = false;
            if (!parseBoolVal(val, &b)) return inikit::IniVerdictBad;
            cfg.verbose = b;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "perf_log") == 0) {
            bool b = true;
            if (!parseBoolVal(val, &b)) return inikit::IniVerdictBad;
            cfg.perfLog = b;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "trim_hidden") == 0) {
            bool b = true;
            if (!parseBoolVal(val, &b)) return inikit::IniVerdictBad;
            cfg.trimHidden = b;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "smooth_edges") == 0) {
            bool b = false;
            if (!parseBoolVal(val, &b)) return inikit::IniVerdictBad;
            cfg.smoothEdges = b;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "gpu_timing") == 0) {
            bool b = true;
            if (!parseBoolVal(val, &b)) return inikit::IniVerdictBad;
            cfg.gpuTiming = b;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "wind_throttle") == 0) {
            bool b = false;
            if (!parseBoolVal(val, &b)) return inikit::IniVerdictBad;
            cfg.windThrottle = b;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "far_lod") == 0) {
            bool b = false;
            if (!parseBoolVal(val, &b)) return inikit::IniVerdictBad;
            cfg.farLod = b;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "vary_seed") == 0) {
            bool b = false;
            if (!parseBoolVal(val, &b)) return inikit::IniVerdictBad;
            cfg.varySeed = b;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "even_spread") == 0) {
            bool b = false;
            if (!parseBoolVal(val, &b)) return inikit::IniVerdictBad;
            cfg.evenSpread = b;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "density_noise") == 0) {
            bool b = false;
            if (!parseBoolVal(val, &b)) return inikit::IniVerdictBad;
            cfg.densityNoise = b;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "density_amount") == 0) {
            float f = 0.0f;
            if (!inikit::parseFloats(val, &f, 1)) return inikit::IniVerdictBad;
            cfg.densityAmount = f;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "density_cell") == 0) {
            float f = 0.0f;
            if (!inikit::parseFloats(val, &f, 1)) return inikit::IniVerdictBad;
            cfg.densityCell = f;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "size_var") == 0) {
            float f = 0.0f;
            if (!inikit::parseFloats(val, &f, 1)) return inikit::IniVerdictBad;
            // Direct float (0.4..1.2) or the Mod Configuration slider's scaled integer (40..120 -> 0.40..1.20).
            if (f > 2.0f && f <= 200.0f) f *= 0.01f;
            cfg.sizeVar = f;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "wind_near") == 0) {
            float f = 0.0f;
            if (!inikit::parseFloats(val, &f, 1)) return inikit::IniVerdictBad;
            cfg.windNear = f;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "wind_every") == 0) {
            float f = 0.0f;
            if (!inikit::parseFloats(val, &f, 1) || !std::isfinite(f)) return inikit::IniVerdictBad;
            cfg.windEvery = f < -1000.0f ? -1000 : (f > 1000.0f ? 1000 : static_cast<int>(std::lround(f)));
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "lod_start") == 0) {
            float f = 0.0f;
            if (!inikit::parseFloats(val, &f, 1)) return inikit::IniVerdictBad;
            cfg.lodStart = f;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "lod_min_keep") == 0) {
            float f = 0.0f;
            if (!inikit::parseFloats(val, &f, 1)) return inikit::IniVerdictBad;
            cfg.lodMinKeep = f;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "fade_start_dist") == 0 || strcmp(key, "fadestartdist") == 0) {
            float f = 0.0f;
            if (!inikit::parseFloats(val, &f, 1)) return inikit::IniVerdictBad;
            cfg.fadeStartDist = f;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "fade_end_dist") == 0 || strcmp(key, "fadeenddist") == 0 || strcmp(key, "draw_distance") == 0) {
            float f = 0.0f;
            if (!inikit::parseFloats(val, &f, 1)) return inikit::IniVerdictBad;
            cfg.fadeEndDist = f;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "wind_dist") == 0 || strcmp(key, "winddist") == 0) {
            float f = 0.0f;
            if (!inikit::parseFloats(val, &f, 1)) return inikit::IniVerdictBad;
            cfg.windDist = f;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "alpha_cutoff") == 0 || strcmp(key, "alphacutoff") == 0) {
            float f = 0.0f;
            if (!inikit::parseFloats(val, &f, 1)) return inikit::IniVerdictBad;
            // Support both 0.20 (direct float) and 20 (scaled integer from ModConfigDecl scale=100).
            if (f >= 1.0f && f <= 100.0f) f *= 0.01f;
            cfg.alphaCutoff = f;
            return inikit::IniVerdictOk;
        }
        return inikit::IniVerdictUnknown;
    });
    cfg.sanitize();
    return st;
}

inline int formatDefaultIni(char* buf, size_t cap, const Config& c = Config{}) {
    const int alphaPct = static_cast<int>(std::lround(c.alphaCutoff * 100.0f));
    return snprintf(
        buf, cap,
        "; K2 Grass Draw Distance configuration (%s)\n"
        "; Hot-reloaded automatically while the game is running.\n"
        "[%s]\n"
        "enabled = %d\n"
        "; Distance (meters) where grass begins fading (vanilla: 15; kept 5 below fade_end_dist)\n"
        "fade_start_dist = %.0f\n"
        "; Maximum grass draw & octree build distance in meters (vanilla: 30, max: 150)\n"
        "fade_end_dist = %.0f\n"
        "; Distance (meters) for CPU grass wind animation (vanilla: 15, 0 = off)\n"
        "wind_dist = %.0f\n"
        "; Alpha test threshold (0.05..0.60 or 5..60%%; vanilla: 0.40; lower = softer blade edges)\n"
        "alpha_cutoff = %d\n"
        "; Skip collecting grass bins the alpha test hides anyway (1 = on, 0 = collect out to fade_end_dist)\n"
        "trim_hidden = %d\n"
        "; Alpha-to-coverage on grass blade edges; needs a multisampled GL context (1 = on, 0 = off)\n"
        "smooth_edges = %d\n"
        "verbose = %d\n"
        "; Log grass bin counts and frame rate every 5 s (P3 calibration)\n"
        "perf_log = %d\n"
        "; GL timer query around the grass pass, logged as gpu_ms_* in PERF (1 = on, 0 = off; needs perf_log)\n"
        "gpu_timing = %d\n"
        "; Wind throttle: full wind_dist only every wind_every-th frame, wind_near meters in between (1 = on, 0 = off)\n"
        "wind_throttle = %d\n"
        "wind_near = %.0f\n"
        "wind_every = %d\n"
        "; Far grass thinning: draw only a prefix of each far bin's quads; 100%% at lod_start meters, lod_min_keep at the draw distance\n"
        "far_lod = %d\n"
        "lod_start = %.0f\n"
        "lod_min_keep = %.2f\n"
        "; Variety (generation tweaks apply to bins built after the change, e.g. after an area reload)\n"
        "; Per-face random pattern hashed from the face vertices instead of a rounded coordinate sum (1 = on, 0 = vanilla)\n"
        "vary_seed = %d\n"
        "; Blades spread evenly over each face instead of clustering toward its centre (1 = on, 0 = vanilla)\n"
        "even_spread = %d\n"
        "; Smooth density patches: grass count scaled by value noise (1 = on, 0 = off)\n"
        "density_noise = %d\n"
        "; Patch strength 0..0.9 (0.5 = counts vary between 0.5x and 1.5x) and patch size in meters 2..30\n"
        "density_amount = %.2f\n"
        "density_cell = %.0f\n"
        "; Blade size variation 0.4..1.2 (vanilla 0.4; also 40..120 from the menu slider)\n"
        "size_var = %d\n",
        kModId, kIniSection,
        c.enabled ? 1 : 0,
        c.fadeStartDist,
        c.fadeEndDist,
        c.windDist,
        alphaPct,
        c.trimHidden ? 1 : 0,
        c.smoothEdges ? 1 : 0,
        c.verbose ? 1 : 0,
        c.perfLog ? 1 : 0,
        c.gpuTiming ? 1 : 0,
        c.windThrottle ? 1 : 0,
        c.windNear,
        c.windEvery,
        c.farLod ? 1 : 0,
        c.lodStart,
        static_cast<double>(c.lodMinKeep),
        c.varySeed ? 1 : 0,
        c.evenSpread ? 1 : 0,
        c.densityNoise ? 1 : 0,
        static_cast<double>(c.densityAmount),
        static_cast<double>(c.densityCell),
        static_cast<int>(std::lround(c.sizeVar * 100.0f)));
}

// Declarative Mod Configuration page schema (TomlMini string limit: each string < 96 chars).
constexpr const char* kPageSchema = R"TOML(
[page]
name = "K2 Grass Draw Distance"
title = "K2 Grass Draw Distance"
desc = "Grass draw distance, fade, wind radius, alpha cutoff, edge smoothing, variety, and speed tests."

[[row]]
id = "enabled"
kind = "toggle"
label = "Extended Grass Distance"
desc = "Enable extended grass draw distance (Off restores vanilla 15m-30m)."
default = 1
ini = "k2_grass_draw_distance.ini"
section = "Grass"
key = "enabled"

[[row]]
id = "fade_start"
kind = "stepper"
style = "slider"
label = "Fade Start"
desc = "Meters where grass starts fading (vanilla 15m; kept 5m below Draw Distance)."
default = 120
min = 15
max = 135
step = 15
fmt = "Fade Start: %dm"
ini = "k2_grass_draw_distance.ini"
section = "Grass"
key = "fade_start_dist"

[[row]]
id = "fade_end"
kind = "stepper"
style = "slider"
label = "Draw Distance"
desc = "Maximum grass build and fade-out distance in meters (vanilla: 30m)."
default = 150
min = 30
max = 150
step = 15
fmt = "Draw Distance: %dm"
ini = "k2_grass_draw_distance.ini"
section = "Grass"
key = "fade_end_dist"

[[row]]
id = "wind_dist"
kind = "stepper"
style = "slider"
label = "Wind Radius"
desc = "Distance in meters for CPU grass wind sway (vanilla: 15m, 0 = Off)."
default = 20
min = 0
max = 90
step = 5
zero_text = "Off"
fmt = "Wind Radius: %dm"
ini = "k2_grass_draw_distance.ini"
section = "Grass"
key = "wind_dist"

[[row]]
id = "alpha_cutoff"
kind = "stepper"
style = "slider"
label = "Alpha Cutoff"
desc = "OpenGL alpha-test threshold (lower fades smoother; vanilla: 0.40)."
default = 40
min = 5
max = 60
step = 5
scale = 100
ini = "k2_grass_draw_distance.ini"
section = "Grass"
key = "alpha_cutoff"

[[row]]
id = "trim_hidden"
kind = "toggle"
label = "Trim Hidden Grass"
desc = "Skip grass the alpha test already hides (saves CPU; On keeps the picture the same)."
default = 1
ini = "k2_grass_draw_distance.ini"
section = "Grass"
key = "trim_hidden"

[[row]]
id = "smooth_edges"
kind = "toggle"
label = "Smooth Grass Edges"
desc = "Alpha-to-coverage blade edges; needs anti-aliasing (MSAA) in the video options."
default = 0
ini = "k2_grass_draw_distance.ini"
section = "Grass"
key = "smooth_edges"

[[row]]
id = "wind_throttle"
kind = "toggle"
label = "Wind Throttle"
desc = "Test: animate far grass wind on some frames only (saves CPU; far sway runs slower)."
default = 0
ini = "k2_grass_draw_distance.ini"
section = "Grass"
key = "wind_throttle"

[[row]]
id = "far_lod"
kind = "toggle"
label = "Far Grass Thinning"
desc = "Test: draw fewer blades on far grass (saves GPU; far grass may look sparse)."
default = 0
ini = "k2_grass_draw_distance.ini"
section = "Grass"
key = "far_lod"

[[row]]
id = "vary_seed"
kind = "toggle"
label = "Varied Grass Pattern"
desc = "Each patch of grass gets its own random layout (applies to grass built after the change)."
default = 1
ini = "k2_grass_draw_distance.ini"
section = "Grass"
key = "vary_seed"

[[row]]
id = "even_spread"
kind = "toggle"
label = "Even Grass Spread"
desc = "Spread blades evenly over the ground instead of clustering (applies to new grass)."
default = 1
ini = "k2_grass_draw_distance.ini"
section = "Grass"
key = "even_spread"

[[row]]
id = "density_noise"
kind = "toggle"
label = "Grass Density Patches"
desc = "Thicker and sparser patches of grass at the same average amount (applies to new grass)."
default = 0
ini = "k2_grass_draw_distance.ini"
section = "Grass"
key = "density_noise"

[[row]]
id = "size_var"
kind = "stepper"
style = "slider"
label = "Grass Size Variation"
desc = "How much blade size varies (vanilla 0.40; higher = more tall and short blades)."
default = 40
min = 40
max = 120
step = 20
scale = 100
ini = "k2_grass_draw_distance.ini"
section = "Grass"
key = "size_var"
)TOML";

} // namespace k2grass
