// K2FramePacingLogic.h -- Pure config, pacing math, perf window and ModConfig schema for k2-frame-pacing.
// Header-only and host-testable (no engine addresses or Win32 calls).
#pragma once

#include "IniKit.h"
#include "ModConfigDecl.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace k2fp {

constexpr const char* kModId = "k2-frame-pacing";
constexpr const char* kVersion = "0.1.0-dev";
constexpr const char* kIniFile = "k2_frame_pacing.ini";
constexpr const char* kIniSection = "FramePacing";

constexpr int kMinMinimizedSleepMs = 5;
constexpr int kMaxMinimizedSleepMs = 200;
constexpr int kMinBackgroundFps = 5;
constexpr int kMaxBackgroundFps = 120;
constexpr int kMinFpsCap = 20;
constexpr int kMaxFpsCap = 360;
constexpr int kMaxSpinUs = 5000;

inline int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

// A "0 = off" value: <=0 -> 0, 1..lo-1 -> lo, above hi -> hi.
inline int clampOffOr(int v, int lo, int hi) { return v <= 0 ? 0 : clampi(v, lo, hi); }

struct Config {
    bool enabled = true;
    bool minimizedSleep = true;
    int minimizedSleepMs = 50;
    int backgroundFps = 30;   // 0 = no cap while in the background
    int fpsCap = 0;           // 0 = off (V-Sync paces)
    int spinUs = 1500;
    bool perfLog = true;
    bool verbose = false;

    void sanitize() {
        minimizedSleepMs = clampi(minimizedSleepMs, kMinMinimizedSleepMs, kMaxMinimizedSleepMs);
        backgroundFps = clampOffOr(backgroundFps, kMinBackgroundFps, kMaxBackgroundFps);
        fpsCap = clampOffOr(fpsCap, kMinFpsCap, kMaxFpsCap);
        spinUs = clampi(spinUs, 0, kMaxSpinUs);
    }

    bool operator==(const Config& o) const {
        return enabled == o.enabled && minimizedSleep == o.minimizedSleep && minimizedSleepMs == o.minimizedSleepMs &&
               backgroundFps == o.backgroundFps && fpsCap == o.fpsCap && spinUs == o.spinUs && perfLog == o.perfLog &&
               verbose == o.verbose;
    }
    bool operator!=(const Config& o) const { return !(*this == o); }
};

enum class Mode : int { Active = 0, Background = 1, Minimized = 2 };

inline const char* modeName(Mode m) {
    switch (m) {
        case Mode::Active: return "active";
        case Mode::Background: return "background";
        case Mode::Minimized: return "minimized";
    }
    return "active";
}

// Target frame period (us) for capped modes: 0 = no cap. Minimized is a fixed sleep, not a cap.
inline int64_t targetFrameUs(const Config& raw, Mode m) {
    if (!raw.enabled) return 0;
    Config c = raw;
    c.sanitize();
    int fps = 0;
    if (m == Mode::Active) fps = c.fpsCap;
    else if (m == Mode::Background) fps = c.backgroundFps;
    return fps > 0 ? static_cast<int64_t>(1000000 / fps) : 0;
}

// Fixed Sleep (ms) before any cap logic.
inline uint32_t fixedSleepMs(const Config& raw, Mode m) {
    if (!raw.enabled || !raw.minimizedSleep || m != Mode::Minimized) return 0;
    Config c = raw;
    c.sanitize();
    return static_cast<uint32_t>(c.minimizedSleepMs);
}

// Whole ms to Sleep before spinning out the last spinUs.
inline uint32_t capSleepMs(int64_t remainingUs, int spinUs) {
    if (spinUs < 0) spinUs = 0;
    if (remainingUs <= spinUs) return 0;
    return static_cast<uint32_t>((remainingUs - spinUs) / 1000);
}

constexpr unsigned kPerfWindowMs = 5000;

struct PerfWindow {
    unsigned startMs = 0;
    bool started = false;
    uint32_t frames[3] = {0, 0, 0};
    uint64_t workUs = 0;
    uint32_t workMaxUs = 0;
    uint64_t sleptUs = 0;
    uint64_t spunUs = 0;

    void reset(unsigned nowMs) {
        *this = PerfWindow{};
        startMs = nowMs;
        started = true;
    }
    // GetTickCount wrap safe (unsigned subtraction).
    bool due(unsigned nowMs) const { return started && (nowMs - startMs) >= kPerfWindowMs; }
    void addFrame(Mode m, uint32_t work, uint32_t slept, uint32_t spun) {
        const int i = static_cast<int>(m);
        if (i >= 0 && i < 3) ++frames[i];
        workUs += work;
        if (work > workMaxUs) workMaxUs = work;
        sleptUs += slept;
        spunUs += spun;
    }
    uint32_t totalFrames() const { return frames[0] + frames[1] + frames[2]; }
    int format(char* buf, size_t cap, unsigned nowMs) const {
        const unsigned dt = nowMs - startMs;
        const uint32_t total = totalFrames();
        const double fps = dt ? total * 1000.0 / dt : 0.0;
        const double workAvg = total ? static_cast<double>(workUs) / total / 1000.0 : 0.0;
        return snprintf(buf, cap,
                        "PERF fps=%.1f active=%u bg=%u min=%u work_ms_avg=%.2f work_ms_max=%.2f slept_ms=%llu spun_ms=%llu",
                        fps, frames[0], frames[1], frames[2], workAvg, workMaxUs / 1000.0,
                        static_cast<unsigned long long>(sleptUs / 1000), static_cast<unsigned long long>(spunUs / 1000));
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

// Whole-string integer parse (no trailing junk); out-of-int-range counts as bad.
inline bool parseIntVal(const char* val, int* out) {
    if (!val || !*val) return false;
    char* end = nullptr;
    const long v = strtol(val, &end, 10);
    if (end == val || *end != '\0') return false;
    if (v < -1000000L || v > 1000000L) return false;
    *out = static_cast<int>(v);
    return true;
}

// Parse INI buffer in place (modifies `text`).
inline inikit::LoadStats parseIniText(char* text, Config& cfg) {
    cfg = Config{};
    inikit::LoadStats st = inikit::forEachLine(text, [&](int /*lineNo*/, const char* key, const char* val) -> inikit::IniVerdict {
        bool* flag = nullptr;
        if (strcmp(key, "enabled") == 0) flag = &cfg.enabled;
        else if (strcmp(key, "minimized_sleep") == 0) flag = &cfg.minimizedSleep;
        else if (strcmp(key, "perf_log") == 0) flag = &cfg.perfLog;
        else if (strcmp(key, "verbose") == 0) flag = &cfg.verbose;
        if (flag) {
            bool b = false;
            if (!parseBoolVal(val, &b)) return inikit::IniVerdictBad;
            *flag = b;
            return inikit::IniVerdictOk;
        }
        int* num = nullptr;
        if (strcmp(key, "minimized_sleep_ms") == 0) num = &cfg.minimizedSleepMs;
        else if (strcmp(key, "background_fps") == 0) num = &cfg.backgroundFps;
        else if (strcmp(key, "fps_cap") == 0) num = &cfg.fpsCap;
        else if (strcmp(key, "spin_us") == 0) num = &cfg.spinUs;
        if (num) {
            int n = 0;
            if (!parseIntVal(val, &n)) return inikit::IniVerdictBad;
            *num = n;
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
        "; K2 Frame Pacing configuration (%s)\n"
        "; Hot-reloaded automatically while the game is running.\n"
        "[%s]\n"
        "; 0 = vanilla (no waiting at all)\n"
        "enabled = %d\n"
        "; Sleep while the window is minimized (vanilla spins a full core)\n"
        "minimized_sleep = %d\n"
        "; Milliseconds slept per loop while minimized (5..200)\n"
        "minimized_sleep_ms = %d\n"
        "; FPS cap while the game is in the background (0 = no cap, 5..120)\n"
        "background_fps = %d\n"
        "; FPS cap while active (0 = off, V-Sync paces; 20..360)\n"
        "fps_cap = %d\n"
        "; Microseconds spun (not slept) at the end of a capped frame (0..5000)\n"
        "spin_us = %d\n"
        "verbose = %d\n"
        "; Log frame pacing stats every 5 s (calibration)\n"
        "perf_log = %d\n",
        kModId, kIniSection, c.enabled ? 1 : 0, c.minimizedSleep ? 1 : 0, c.minimizedSleepMs, c.backgroundFps, c.fpsCap,
        c.spinUs, c.verbose ? 1 : 0, c.perfLog ? 1 : 0);
}

// Declarative Mod Configuration page schema (TomlMini string limit: each string < 96 chars).
constexpr const char* kPageSchema = R"TOML(
[page]
name = "K2 Frame Pacing"
title = "K2 Frame Pacing"
desc = "Stop the game burning a CPU core while minimized or in the background."

[[row]]
id = "minimized_sleep"
kind = "toggle"
label = "Sleep While Minimized"
desc = "Sleep instead of spinning a full CPU core while the window is minimized."
default = 1
ini = "k2_frame_pacing.ini"
section = "FramePacing"
key = "minimized_sleep"

[[row]]
id = "background_fps"
kind = "stepper"
style = "slider"
label = "Background FPS Cap"
desc = "Frame rate cap while the game is not the active window (0 = no cap)."
default = 30
min = 0
max = 120
step = 5
fmt = "Background FPS: %d"
ini = "k2_frame_pacing.ini"
section = "FramePacing"
key = "background_fps"

[[row]]
id = "fps_cap"
kind = "stepper"
style = "slider"
label = "FPS Cap"
desc = "Frame rate cap while active (0 = off, V-Sync paces the game)."
default = 0
min = 0
max = 240
step = 5
fmt = "FPS Cap: %d"
ini = "k2_frame_pacing.ini"
section = "FramePacing"
key = "fps_cap"
)TOML";

} // namespace k2fp
