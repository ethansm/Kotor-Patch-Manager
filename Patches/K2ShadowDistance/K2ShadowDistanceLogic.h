// K2ShadowDistanceLogic.h -- Pure config, math, perf window and ModConfig schema for k2-shadow-distance.
// Header-only and host-testable (no engine addresses or Win32 calls).
#pragma once

#include "../Common/IniKit.h"
#include "../Common/ModConfigDecl.h"
#include <cmath>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace k2shadow {

constexpr const char* kModId = "k2-shadow-distance";
constexpr const char* kVersion = "0.1.0-dev";
constexpr const char* kIniFile = "k2_shadow_distance.ini";
constexpr const char* kIniSection = "Shadows";

// Vanilla swkotor2.exe (Steam 6A522E71) object shadow gates in Scene::DoGobShadows / DoGobShadowsForThisLight:
//   object distance gate: length(view ref - object) > float [shadow dist global] (vanilla 35.0) skips the object
//   view-reference gate:  length^2 vs a .rdata constant of 1100.0 (= 35^2 * 1100/1225, ~33.2 m)
// The mod moves the first to dist and the second to dist^2 * 1100/1225 so the two gates keep the vanilla ratio.
constexpr float kVanillaShadowDist = 35.0f;
constexpr float kVanillaRefSq = 1100.0f;
constexpr float kVanillaDistSq = 1225.0f;   // 35^2

constexpr bool kDefaultEnabled = true;
constexpr float kDefaultShadowDist = 35.0f;   // default stays vanilla until a play test picks a better value
constexpr bool kDefaultPerfLog = true;
constexpr bool kDefaultVerbose = false;

constexpr float kMinShadowDist = 10.0f;
constexpr float kMaxShadowDist = 100.0f;

inline float clampf(float v, float lo, float hi) {
    if (!std::isfinite(v)) return lo;
    return v < lo ? lo : (v > hi ? hi : v);
}

struct Config {
    bool enabled = kDefaultEnabled;
    float shadowDist = kDefaultShadowDist;
    bool perfLog = kDefaultPerfLog;
    bool verbose = kDefaultVerbose;

    void sanitize() { shadowDist = clampf(shadowDist, kMinShadowDist, kMaxShadowDist); }

    bool operator==(const Config& o) const {
        return enabled == o.enabled && std::fabs(shadowDist - o.shadowDist) < 1e-4f && perfLog == o.perfLog &&
               verbose == o.verbose;
    }
    bool operator!=(const Config& o) const { return !(*this == o); }
};

struct EffectiveValues {
    float dist;    // metres, written to the engine float
    float refSq;   // read by the rewritten view-reference FLD
};

inline EffectiveValues computeEffective(const Config& raw) {
    Config c = raw;
    c.sanitize();
    const float d = c.enabled ? c.shadowDist : kVanillaShadowDist;
    return {d, (d * d * kVanillaRefSq) / kVanillaDistSq};
}

constexpr unsigned kPerfWindowMs = 5000;
// A longer gap between shadow-light calls (menu, pause, area load) drops the partial window so per_s counts shadow time only.
constexpr unsigned kPerfGapMs = 1000;

struct PerfWindow {
    unsigned startMs = 0;
    uint32_t calls = 0;
    unsigned lastMs = 0;

    void add(unsigned nowMs) {
        if (calls > 0 && (nowMs - lastMs) > kPerfGapMs) reset(nowMs);
        lastMs = nowMs;
        ++calls;
    }
    // True once kPerfWindowMs has passed since startMs with at least one call (GetTickCount wrap safe).
    bool due(unsigned nowMs) const { return calls > 0 && (nowMs - startMs) >= kPerfWindowMs; }
    float perSec(unsigned nowMs) const {
        const unsigned dt = nowMs - startMs;
        return dt ? calls * 1000.0f / static_cast<float>(dt) : 0.0f;
    }
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

// Parse INI buffer in place (modifies `text`).
inline inikit::LoadStats parseIniText(char* text, Config& cfg) {
    cfg = Config{};
    inikit::LoadStats st = inikit::forEachLine(text, [&](int /*lineNo*/, const char* key, const char* val) -> inikit::IniVerdict {
        bool* flag = nullptr;
        if (strcmp(key, "enabled") == 0) flag = &cfg.enabled;
        else if (strcmp(key, "perf_log") == 0) flag = &cfg.perfLog;
        else if (strcmp(key, "verbose") == 0) flag = &cfg.verbose;
        if (flag) {
            bool b = false;
            if (!parseBoolVal(val, &b)) return inikit::IniVerdictBad;
            *flag = b;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "shadow_dist") == 0 || strcmp(key, "shadowdist") == 0) {
            float f = 0.0f;
            if (!inikit::parseFloats(val, &f, 1)) return inikit::IniVerdictBad;
            cfg.shadowDist = f;
            return inikit::IniVerdictOk;
        }
        return inikit::IniVerdictUnknown;
    });
    cfg.sanitize();
    return st;
}

inline int formatDefaultIni(char* buf, size_t cap, const Config& c = Config{}) {
    return snprintf(
        buf, cap,
        "; K2 Shadow Distance configuration (%s)\n"
        "; Hot-reloaded automatically while the game is running.\n"
        "[%s]\n"
        "enabled = %d\n"
        "; Distance (meters) out to which objects cast shadows (vanilla: 35, range 10..100)\n"
        "shadow_dist = %.0f\n"
        "verbose = %d\n"
        "; Log shadow-light calls per second every 5 s (calibration)\n"
        "perf_log = %d\n",
        kModId, kIniSection, c.enabled ? 1 : 0, c.shadowDist, c.verbose ? 1 : 0, c.perfLog ? 1 : 0);
}

// Declarative Mod Configuration page schema (TomlMini string limit: each string < 96 chars).
constexpr const char* kPageSchema = R"TOML(
[page]
name = "K2 Shadow Distance"
title = "K2 Shadow Distance"
desc = "Extend the distance at which objects cast shadows."

[[row]]
id = "enabled"
kind = "toggle"
label = "Extended Shadow Distance"
desc = "Use the Shadow Distance below (Off restores vanilla 35m)."
default = 1
ini = "k2_shadow_distance.ini"
section = "Shadows"
key = "enabled"

[[row]]
id = "shadow_dist"
kind = "stepper"
style = "slider"
label = "Shadow Distance"
desc = "Meters out to which objects cast shadows (vanilla: 35m)."
default = 35
min = 10
max = 100
step = 5
fmt = "Shadow Distance: %dm"
ini = "k2_shadow_distance.ini"
section = "Shadows"
key = "shadow_dist"
)TOML";

} // namespace k2shadow
