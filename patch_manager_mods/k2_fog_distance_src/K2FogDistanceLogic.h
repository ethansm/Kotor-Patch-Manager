// K2FogDistanceLogic.h -- Pure config, fog scaling, scene tracking and ModConfig schema for k2-fog-distance.
// Header-only and host-testable (no engine addresses or Win32 calls).
#pragma once

#include "IniKit.h"
#include "ModConfigDecl.h"
#include <cmath>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace k2fog {

constexpr const char* kModId = "k2-fog-distance";
constexpr const char* kVersion = "0.1.0-dev";
constexpr const char* kIniFile = "k2_fog_distance.ini";
constexpr const char* kIniSection = "Fog";

// Scene fog fields (Steam 6A522E71, Scene::DoFog 0x0046A980 / Scene::SetFogRange 0x00464C80):
//   +0x60 int fog on, +0x64 float GL_FOG_START, +0x68 float GL_FOG_END. Each area's .are SunFogNear/SunFogFar
//   (or MoonFog*) lands there; DoFog, the room pass 0x00466700 and 0x004B3D80 all read the two floats.
// Percentages scale the area's own values, so every area keeps its look, only further away.
constexpr int32_t kDefaultFogEndPct = 200;
constexpr int32_t kDefaultFogStartPct = 200;
constexpr int32_t kMinPct = 0;
constexpr int32_t kMaxPct = 400;
// Fog Distance 0% = "No Fog": the same finite huge end KPM Shader Fixes writes for fog-less areas (0x0046AADD,
// GL_FOG_END = 1e6; ARB_fog_linear ignores glDisable(GL_FOG), and FLT_MAX turned surfaces black on AMD, KPM #280).
constexpr float kNoFogEnd = 1.0e6f;
constexpr float kNoFogStart = 0.99e6f;
constexpr float kMaxFogDist = 1.0e6f;
// Applied fog start stays at or below this fraction of the applied end (a start past the end inverts the ramp).
constexpr float kMaxStartFrac = 0.95f;

inline int32_t clampi(int32_t v, int32_t lo, int32_t hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

struct Config {
    bool enabled = true;
    int32_t fogEndPct = kDefaultFogEndPct;
    int32_t fogStartPct = kDefaultFogStartPct;
    bool verbose = false;

    void sanitize() {
        fogEndPct = clampi(fogEndPct, kMinPct, kMaxPct);
        fogStartPct = clampi(fogStartPct, kMinPct, kMaxPct);
    }
    bool operator==(const Config& o) const {
        return enabled == o.enabled && fogEndPct == o.fogEndPct && fogStartPct == o.fogStartPct && verbose == o.verbose;
    }
    bool operator!=(const Config& o) const { return !(*this == o); }
};

struct FogRange {
    float start;
    float end;
};

// Area fog (origStart..origEnd, metres) -> the range this config sends to the engine.
inline FogRange applyScale(float origStart, float origEnd, const Config& raw) {
    Config c = raw;
    c.sanitize();
    if (!c.enabled || !std::isfinite(origStart) || !std::isfinite(origEnd)) return {origStart, origEnd};
    if (c.fogEndPct == 0) return {kNoFogStart, kNoFogEnd};
    float end = origEnd * static_cast<float>(c.fogEndPct) * 0.01f;
    if (end > kMaxFogDist) end = kMaxFogDist;
    float start = origStart * static_cast<float>(c.fogStartPct) * 0.01f;
    if (start > end * kMaxStartFrac) start = end * kMaxStartFrac;
    if (start < 0.0f) start = 0.0f;
    return {start, end};
}

// Remembers each scene's area fog so scaling never compounds: DoFog runs every frame on the same fields we write.
// A value that differs from what we last wrote is new area fog (area load, day/night, a script), so it becomes
// the new original. Fixed small table; scenes past kMaxScenes reuse the oldest slot.
constexpr int kMaxScenes = 4;

struct SceneSlot {
    uintptr_t scene = 0;
    float origStart = 0.0f;
    float origEnd = 0.0f;
    float lastStart = 0.0f;
    float lastEnd = 0.0f;
    uint32_t stamp = 0;
};

struct UpdateResult {
    bool write = false;      // the caller must store `range` into the scene
    bool newOrigin = false;  // area fog changed since our last write (log it)
    FogRange range{0.0f, 0.0f};
    FogRange orig{0.0f, 0.0f};
};

struct FogTracker {
    SceneSlot slots[kMaxScenes];
    uint32_t clock = 0;

    UpdateResult update(uintptr_t scene, float curStart, float curEnd, const Config& cfg) {
        UpdateResult r{};
        SceneSlot* s = find(scene);
        if (!s) {
            s = victim();
            *s = SceneSlot{};
            s->scene = scene;
            r.newOrigin = true;
        } else if (curStart != s->lastStart || curEnd != s->lastEnd) {
            r.newOrigin = true;
        }
        if (r.newOrigin) {
            s->origStart = curStart;
            s->origEnd = curEnd;
        }
        s->stamp = ++clock;
        r.orig = {s->origStart, s->origEnd};
        r.range = applyScale(s->origStart, s->origEnd, cfg);
        r.write = r.range.start != curStart || r.range.end != curEnd;
        s->lastStart = r.range.start;
        s->lastEnd = r.range.end;
        return r;
    }

    SceneSlot* find(uintptr_t scene) {
        for (SceneSlot& s : slots) {
            if (s.scene == scene && scene != 0) return &s;
        }
        return nullptr;
    }
    SceneSlot* victim() {
        SceneSlot* v = &slots[0];
        for (SceneSlot& s : slots) {
            if (s.scene == 0) return &s;
            if (s.stamp < v->stamp) v = &s;
        }
        return v;
    }
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

inline bool parsePct(const char* val, int32_t* out) {
    float f = 0.0f;
    if (!inikit::parseFloats(val, &f, 1) || !std::isfinite(f)) return false;
    *out = static_cast<int32_t>(std::lround(f));
    return true;
}

// Parse INI buffer in place (modifies `text`).
inline inikit::LoadStats parseIniText(char* text, Config& cfg) {
    cfg = Config{};
    inikit::LoadStats st = inikit::forEachLine(text, [&](int /*lineNo*/, const char* key, const char* val) -> inikit::IniVerdict {
        if (strcmp(key, "enabled") == 0 || strcmp(key, "verbose") == 0) {
            bool b = false;
            if (!parseBoolVal(val, &b)) return inikit::IniVerdictBad;
            (key[0] == 'e' ? cfg.enabled : cfg.verbose) = b;
            return inikit::IniVerdictOk;
        }
        if (strcmp(key, "fog_end_pct") == 0) {
            return parsePct(val, &cfg.fogEndPct) ? inikit::IniVerdictOk : inikit::IniVerdictBad;
        }
        if (strcmp(key, "fog_start_pct") == 0) {
            return parsePct(val, &cfg.fogStartPct) ? inikit::IniVerdictOk : inikit::IniVerdictBad;
        }
        return inikit::IniVerdictUnknown;
    });
    cfg.sanitize();
    return st;
}

inline int formatDefaultIni(char* buf, size_t cap, const Config& c = Config{}) {
    return snprintf(
        buf, cap,
        "; K2 Fog Distance configuration (%s)\n"
        "; Hot-reloaded automatically while the game is running.\n"
        "[%s]\n"
        "enabled = %d\n"
        "; Percent of each area's own fog end distance (100 = vanilla, max 400, 0 = no fog)\n"
        "fog_end_pct = %d\n"
        "; Percent of each area's own fog start distance (100 = vanilla, max 400; kept below the fog end)\n"
        "fog_start_pct = %d\n"
        "; Log every area fog change, not only the first per area\n"
        "verbose = %d\n",
        kModId, kIniSection, c.enabled ? 1 : 0, static_cast<int>(c.fogEndPct), static_cast<int>(c.fogStartPct),
        c.verbose ? 1 : 0);
}

// Declarative Mod Configuration page schema (TomlMini string limit: each string < 96 chars).
constexpr const char* kPageSchema = R"TOML(
[page]
name = "K2 Fog Distance"
title = "K2 Fog Distance"
desc = "Push each area's distance fog further out, or turn it off."

[[row]]
id = "enabled"
kind = "toggle"
label = "Extended Fog Distance"
desc = "Scale area fog distances (Off restores each area's vanilla fog)."
default = 1
ini = "k2_fog_distance.ini"
section = "Fog"
key = "enabled"

[[row]]
id = "fog_end"
kind = "stepper"
style = "slider"
label = "Fog Distance"
desc = "Where fog becomes solid, as % of the area's own value (100% = vanilla)."
default = 200
min = 0
max = 400
step = 25
zero_text = "No Fog"
fmt = "Fog Distance: %d%%"
ini = "k2_fog_distance.ini"
section = "Fog"
key = "fog_end_pct"

[[row]]
id = "fog_start"
kind = "stepper"
style = "slider"
label = "Fog Start"
desc = "Where fog begins, as % of the area's own value (kept below Fog Distance)."
default = 200
min = 0
max = 400
step = 25
fmt = "Fog Start: %d%%"
ini = "k2_fog_distance.ini"
section = "Fog"
key = "fog_start_pct"
)TOML";

} // namespace k2fog
