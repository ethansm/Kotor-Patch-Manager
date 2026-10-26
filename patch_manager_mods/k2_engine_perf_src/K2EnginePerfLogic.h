// K2EnginePerfLogic.h -- Pure config, ini parsing and perf window for k2-engine-perf.
// Header-only and host-testable (no engine addresses or Win32 calls). The two optimisations live in
// TexScanCache.h (texture-id scan reuse) and MemStatusCache.h (GlobalMemoryStatus cache).
#pragma once

#include "IniKit.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace k2ep {

constexpr const char* kModId = "k2-engine-perf";
constexpr const char* kVersion = "0.1.0-dev";
constexpr const char* kIniFile = "k2_engine_perf.ini";
constexpr const char* kIniSection = "EnginePerf";

constexpr bool kDefaultTexScanCache = true;
constexpr int kDefaultTexVerifyEvery = 240;   // one full re-scan compare per 240 cache hits; 0 = never
constexpr bool kDefaultMemStatusCache = true;
constexpr int kDefaultMemStatusMs = 250;
constexpr bool kDefaultPerfLog = true;
constexpr bool kDefaultVerbose = false;

constexpr int kMaxTexVerifyEvery = 100000;
constexpr int kMaxMemStatusMs = 5000;

inline int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

struct Config {
    bool texScanCache = kDefaultTexScanCache;
    int texVerifyEvery = kDefaultTexVerifyEvery;
    bool memStatusCache = kDefaultMemStatusCache;
    int memStatusMs = kDefaultMemStatusMs;
    bool perfLog = kDefaultPerfLog;
    bool verbose = kDefaultVerbose;

    void sanitize() {
        texVerifyEvery = clampi(texVerifyEvery, 0, kMaxTexVerifyEvery);
        memStatusMs = clampi(memStatusMs, 0, kMaxMemStatusMs);
    }

    bool operator==(const Config& o) const {
        return texScanCache == o.texScanCache && texVerifyEvery == o.texVerifyEvery &&
               memStatusCache == o.memStatusCache && memStatusMs == o.memStatusMs && perfLog == o.perfLog &&
               verbose == o.verbose;
    }
    bool operator!=(const Config& o) const { return !(*this == o); }
};

constexpr unsigned kPerfWindowMs = 5000;

// Wall-clock PERF window driven from the per-frame wrappers (GetTickCount, wrap safe).
struct PerfWindow {
    unsigned startMs = 0;
    bool started = false;

    bool due(unsigned nowMs) const { return started && (nowMs - startMs) >= kPerfWindowMs; }
    float seconds(unsigned nowMs) const { return static_cast<float>(nowMs - startMs) / 1000.0f; }
    void reset(unsigned nowMs) {
        startMs = nowMs;
        started = true;
    }
};

// Microseconds from QPC ticks; 0 when the frequency is unknown.
inline double ticksToUs(uint64_t ticks, int64_t freq) {
    return freq > 0 ? static_cast<double>(ticks) * 1e6 / static_cast<double>(freq) : 0.0;
}

// Average cost of one real call and the time the cache saved per second of the window:
// saved = hits * avg real cost / seconds.
struct SavedEstimate {
    double avgUs;
    double savedUsPerSec;
};

inline SavedEstimate estimateSaved(uint64_t realTicks, uint32_t realCalls, uint32_t hits, int64_t freq, float seconds) {
    SavedEstimate e{0.0, 0.0};
    if (realCalls == 0) return e;
    e.avgUs = ticksToUs(realTicks, freq) / static_cast<double>(realCalls);
    if (seconds > 0.0f) e.savedUsPerSec = e.avgUs * static_cast<double>(hits) / static_cast<double>(seconds);
    return e;
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

inline bool parseIntVal(const char* val, int* out) {
    if (!val || !*val) return false;
    char* end = nullptr;
    const long v = strtol(val, &end, 10);
    while (end && (*end == ' ' || *end == '\t')) ++end;
    if (!end || *end != '\0') return false;
    *out = static_cast<int>(v);
    return true;
}

// Parse INI buffer in place (modifies `text`).
inline inikit::LoadStats parseIniText(char* text, Config& cfg) {
    cfg = Config{};
    inikit::LoadStats st = inikit::forEachLine(text, [&](int /*lineNo*/, const char* key, const char* val) -> inikit::IniVerdict {
        bool* flag = nullptr;
        if (strcmp(key, "tex_scan_cache") == 0) flag = &cfg.texScanCache;
        else if (strcmp(key, "mem_status_cache") == 0) flag = &cfg.memStatusCache;
        else if (strcmp(key, "perf_log") == 0) flag = &cfg.perfLog;
        else if (strcmp(key, "verbose") == 0) flag = &cfg.verbose;
        if (flag) {
            bool b = false;
            if (!parseBoolVal(val, &b)) return inikit::IniVerdictBad;
            *flag = b;
            return inikit::IniVerdictOk;
        }
        int* num = nullptr;
        if (strcmp(key, "tex_verify_every") == 0) num = &cfg.texVerifyEvery;
        else if (strcmp(key, "mem_status_ms") == 0) num = &cfg.memStatusMs;
        if (num) {
            int v = 0;
            if (!parseIntVal(val, &v)) return inikit::IniVerdictBad;
            *num = v;
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
        "; K2 Engine Perf configuration (%s)\n"
        "; Hot-reloaded automatically while the game is running. 0 = vanilla behaviour for each switch.\n"
        "[%s]\n"
        "; Reuse the texture-id list within one scene render instead of rebuilding it for each of the 3 bucket passes\n"
        "tex_scan_cache = %d\n"
        "; Safety check: every Nth reuse also runs the real scan and compares (0 = never)\n"
        "tex_verify_every = %d\n"
        "; Cache GlobalMemoryStatus (called 2x per frame; Wine reads /proc/meminfo each time)\n"
        "mem_status_cache = %d\n"
        "; Refresh interval of the cached memory status in ms (0..5000)\n"
        "mem_status_ms = %d\n"
        "; Log call counts and the measured saving every 5 s\n"
        "perf_log = %d\n"
        "verbose = %d\n",
        kModId, kIniSection, c.texScanCache ? 1 : 0, c.texVerifyEvery, c.memStatusCache ? 1 : 0, c.memStatusMs,
        c.perfLog ? 1 : 0, c.verbose ? 1 : 0);
}

} // namespace k2ep
