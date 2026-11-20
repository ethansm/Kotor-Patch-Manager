// PassProfiler.h -- Per-pass QPC timing accumulators and 5-byte E8 rel32 call-patch helpers for k2-engine-perf (EP4).
// Header-only and host-testable (no engine addresses or direct Win32 timing calls).
#pragma once

#include "../Common/CodePatch.h"
#include <stdint.h>
#include <string.h>

namespace passprof {

enum PassId : int {
    kEmitSim = 0,
    kEmitDraw = 1,
    kBlendMode = 2,
    kMeshDedupe = 3,
    kSkinDraw = 4,
    kGobAnim = 5,
    kShdwProj = 6,
    kPassCount = 7
};

struct PassAcc {
    uint64_t ticks = 0;
    uint64_t maxTicks = 0;
    uint32_t calls = 0;

    void record(uint64_t dt) {
        ticks += dt;
        if (dt > maxTicks) maxTicks = dt;
        ++calls;
    }

    void reset() {
        ticks = 0;
        maxTicks = 0;
        calls = 0;
    }
};

struct WindowState {
    PassAcc pass[kPassCount]{};

    void record(PassId id, uint64_t dt) {
        if (id >= 0 && id < kPassCount) pass[id].record(dt);
    }

    void reset() {
        for (int i = 0; i < kPassCount; ++i) pass[i].reset();
    }
};

struct PassSummary {
    double msPerSec = 0.0;
    double maxCallMs = 0.0;
    uint32_t calls = 0;
};

inline PassSummary summarize(const PassAcc& a, int64_t freq, float windowSecs) {
    PassSummary s{};
    s.calls = a.calls;
    if (freq <= 0) return s;
    const double totalMs = static_cast<double>(a.ticks) * 1000.0 / static_cast<double>(freq);
    s.maxCallMs = static_cast<double>(a.maxTicks) * 1000.0 / static_cast<double>(freq);
    if (windowSecs > 0.0f) s.msPerSec = totalMs / static_cast<double>(windowSecs);
    return s;
}

// ---- 5-byte E8 rel32 CALL-target rewrite helper ----
constexpr int kCallLen = 5;

inline void buildCallRel32(uintptr_t siteAddr, uintptr_t targetAddr, uint8_t out[kCallLen]) {
    out[0] = 0xE8;
    codepatch::putRel32(out + 1, siteAddr + 1, targetAddr);
}

struct CallPatchResult {
    int rc = 0;
    uint8_t seen[kCallLen]{};
};

inline CallPatchResult installCall(uintptr_t siteAddr, const uint8_t expect[kCallLen], uintptr_t targetAddr) {
    CallPatchResult r{};
    uint8_t repl[kCallLen];
    buildCallRel32(siteAddr, targetAddr, repl);
    r.rc = codepatch::writeChecked(siteAddr, expect, repl, kCallLen, r.seen);
    return r;
}

inline CallPatchResult uninstallCall(uintptr_t siteAddr, const uint8_t orig[kCallLen], uintptr_t targetAddr, int installRc) {
    CallPatchResult r{};
    if (installRc != 1) {
        r.rc = 0;
        return r;
    }
    uint8_t patched[kCallLen];
    buildCallRel32(siteAddr, targetAddr, patched);
    r.rc = codepatch::writeChecked(siteAddr, patched, orig, kCallLen, r.seen);
    return r;
}

} // namespace passprof
