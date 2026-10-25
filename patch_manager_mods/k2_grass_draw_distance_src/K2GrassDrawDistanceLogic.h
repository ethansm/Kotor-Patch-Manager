// K2GrassDrawDistanceLogic.h -- Pure config, math, and ModConfig schema for k2-grass-draw-distance.
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
constexpr const char* kVersion = "0.1.0-dev";
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

// Extended defaults (3x draw distance = 90m, smooth fade from 60m..90m, 20m wind, 0.20 alpha cutoff):
constexpr bool kDefaultEnabled = true;
constexpr float kDefaultFadeStartDist = 60.0f;
constexpr float kDefaultFadeEndDist = 90.0f;
constexpr float kDefaultWindDist = 20.0f;
constexpr float kDefaultAlphaCutoff = 0.20f;
constexpr bool kDefaultVerbose = false;

// Clamp bounds:
constexpr float kMinFadeStartDist = 5.0f;
constexpr float kMaxFadeStartDist = 500.0f;
constexpr float kMinFadeEndDist = 10.0f;
constexpr float kMaxFadeEndDist = 1000.0f;
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

    void sanitize() {
        fadeStartDist = clampf(fadeStartDist, kMinFadeStartDist, kMaxFadeStartDist);
        fadeEndDist = clampf(fadeEndDist, kMinFadeEndDist, kMaxFadeEndDist);
        if (fadeEndDist < fadeStartDist + kMinFadeSpan) {
            fadeEndDist = clampf(fadeStartDist + kMinFadeSpan, kMinFadeEndDist, kMaxFadeEndDist);
            if (fadeStartDist > fadeEndDist - kMinFadeSpan) {
                fadeStartDist = fadeEndDist - kMinFadeSpan;
            }
        }
        windDist = clampf(windDist, kMinWindDist, kMaxWindDist);
        alphaCutoff = clampf(alphaCutoff, kMinAlphaCutoff, kMaxAlphaCutoff);
    }

    bool operator==(const Config& o) const {
        return enabled == o.enabled &&
               std::fabs(fadeStartDist - o.fadeStartDist) < 1e-4f &&
               std::fabs(fadeEndDist - o.fadeEndDist) < 1e-4f &&
               std::fabs(windDist - o.windDist) < 1e-4f &&
               std::fabs(alphaCutoff - o.alphaCutoff) < 1e-4f &&
               verbose == o.verbose;
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
        "; Distance (meters) where grass begins fading (vanilla: 15)\n"
        "fade_start_dist = %.0f\n"
        "; Maximum grass draw & octree build distance in meters (vanilla: 30)\n"
        "fade_end_dist = %.0f\n"
        "; Distance (meters) for CPU grass wind animation (vanilla: 15, 0 = off)\n"
        "wind_dist = %.0f\n"
        "; Alpha test threshold (0.05..0.60 or 5..60%%; vanilla: 0.40)\n"
        "alpha_cutoff = %d\n"
        "verbose = %d\n",
        kModId, kIniSection,
        c.enabled ? 1 : 0,
        c.fadeStartDist,
        c.fadeEndDist,
        c.windDist,
        alphaPct,
        c.verbose ? 1 : 0);
}

// Declarative Mod Configuration page schema (TomlMini string limit: each string < 96 chars).
constexpr const char* kPageSchema = R"TOML(
[page]
name = "K2 Grass Draw Distance"
title = "K2 Grass Draw Distance"
desc = "Extend grass draw distance, fade start, wind radius, and alpha cutoff."

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
desc = "Distance in meters where grass starts fading (vanilla: 15m)."
default = 60
min = 15
max = 240
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
default = 90
min = 30
max = 300
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
default = 20
min = 5
max = 60
step = 5
scale = 100
ini = "k2_grass_draw_distance.ini"
section = "Grass"
key = "alpha_cutoff"
)TOML";

} // namespace k2grass
