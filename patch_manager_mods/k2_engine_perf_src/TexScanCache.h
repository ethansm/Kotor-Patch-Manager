// TexScanCache.h -- Reuse cache + call-site patcher for the per-frame texture-id scan (TexMgr::RebuildIdList) in k2-engine-perf.
// Header-only, host-testable, namespace texscan. No engine globals are read here; the caller (K2EnginePerf.cpp) passes values in.
//
// Engine facts (Steam swkotor2.exe 6A522E71):
//   TexMgr::RebuildIdList 0x00427970 (no args, returns &vec in EAX; vec = {begin,count,cap} at 0x00A1BC48/4C/50). It zeroes the count
//   [0x00A1BC4C] and max id [0x00A1BB84], walks every sheet x cell and push_backs each non-zero cell id. Scene::Render calls it once
//   at the start of each of 3 render-bucket drains (E8 rel32 at kSiteAddr[]). Only EAX is used after the call, so a `void* __cdecl`
//   replacement is register-safe. The only other writer of the vector is the pending-texture flush inside TexMgr::Tick, which ends
//   with ++[0x00A1BBBC]. Hence the result is unchanged between two calls with the same tick counter, same sheet count, pending == 0.
//
// How K2EnginePerf.cpp must use this API (replacement function installed over the 3 call sites):
//   extern "C" void* __cdecl K2EP_TexScan() {
//       uint32_t tick; int32_t sheets, pending; uint32_t cnt, mx;
//       // saferead (committed-memory probe) the tick counter, sheet count and pending count; on any read failure just call the original.
//       // read tick = [kTickAddr], sheets = [kSheetCountAddr], pending = [kPendingAddr]
//       ++g_stats.calls;
//       if (texscan::canReuse(g_state, tick, sheets, pending, cfg.enabled)) {
//           if (!texscan::onHit(g_stats, cfg.verifyEvery)) return (void*)texscan::kVecAddr;   // plain hit: do not touch the vector
//           // verify hit: fall through to the real scan, then compare with what the cache saw
//           void* r = origScan();                    // original 0x00427970 via function pointer
//           cnt = [kVecCountAddr]; mx = [kMaxIdAddr];
//           bool same = texscan::sameResult(g_state.count, g_state.maxId, cnt, mx);
//           texscan::onVerify(g_stats, same);        // counts verifies / mismatches
//           if (!same) texscan::remember(g_state, tick, sheets, pending, cnt, mx);   // re-key on the fresh result
//           return r;
//       }
//       LARGE_INTEGER t0, t1; QueryPerformanceCounter(&t0);
//       void* r = origScan();
//       QueryPerformanceCounter(&t1);
//       texscan::onMiss(g_stats, (uint64_t)(t1.QuadPart - t0.QuadPart));
//       cnt = [kVecCountAddr]; mx = [kMaxIdAddr];
//       texscan::remember(g_state, tick, sheets, pending, cnt, mx);   // invalidates itself when pending != 0
//       return r;
//   }
// Install: texscan::Sites s; texscan::initSites(s, texscan::kSiteAddr, texscan::kOrigFn, (uintptr_t)&K2EP_TexScan);
//   texscan::install(s); log each site with s.site[i].rc / expect / seen ("patch tex_scan_<n> @0x... expect=.. seen=.. rc=..");
//   on unload texscan::uninstall(s) (restores only sites whose install rc == 1). Also texscan::invalidate(g_state) on area load/unload.
#pragma once

#include "CodePatch.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace texscan {

// ---- engine addresses (Steam exe 6A522E71) ---------------------------------------------------------------------------------
constexpr uintptr_t kOrigFn = 0x00427970;          // TexMgr::RebuildIdList
constexpr uintptr_t kVecAddr = 0x00A1BC48;         // &vec (begin), count at +4, cap at +8
constexpr uintptr_t kVecCountAddr = 0x00A1BC4C;
constexpr uintptr_t kMaxIdAddr = 0x00A1BB84;
constexpr uintptr_t kSheetCountAddr = 0x00A1BC70;
constexpr uintptr_t kTickAddr = 0x00A1BBBC;        // TexMgr::Tick frame counter
constexpr uintptr_t kPendingAddr = 0x00A1BC30;     // pending-texture count (int)
constexpr int kSiteCount = 3;
constexpr uintptr_t kSiteAddr[kSiteCount] = {0x00466033, 0x0046630E, 0x0046673F};
constexpr uint8_t kSiteBytes[kSiteCount][5] = {
    {0xE8, 0x38, 0x19, 0xFC, 0xFF}, {0xE8, 0x5D, 0x16, 0xFC, 0xFF}, {0xE8, 0x2C, 0x12, 0xFC, 0xFF}};

// ---- reuse decision --------------------------------------------------------------------------------------------------------
struct State {
    uint32_t tick = 0;
    int32_t sheets = 0;
    bool valid = false;
    uint32_t count = 0;   // vector count / max id seen by the real scan that filled the cache (for verify)
    uint32_t maxId = 0;
};

inline void invalidate(State& s) { s = State(); }

// Cache the key of a real scan. Pending textures at that moment mean the flush could still change the vector, so no caching then.
inline void remember(State& s, uint32_t tick, int32_t sheets, int32_t pending = 0, uint32_t count = 0, uint32_t maxId = 0) {
    if (pending != 0) { invalidate(s); return; }
    s.tick = tick;
    s.sheets = sheets;
    s.count = count;
    s.maxId = maxId;
    s.valid = true;
}

inline bool canReuse(const State& s, uint32_t tick, int32_t sheets, int32_t pending, bool enabled) {
    return enabled && s.valid && pending == 0 && s.tick == tick && s.sheets == sheets;
}

// ---- stats + verify --------------------------------------------------------------------------------------------------------
struct Stats {
    uint64_t calls = 0, hits = 0, misses = 0, verifies = 0, mismatches = 0;
    uint64_t missTicks = 0;   // QPC ticks summed over real scans (misses + verify scans are not included)
    void reset() { *this = Stats(); }
};

// every-th hit (1-based, hits already counted) is a verify hit; every == 0 never verifies.
inline bool dueVerify(uint64_t hits, uint32_t every) { return every != 0 && hits != 0 && (hits % every) == 0; }

inline bool sameResult(uint32_t countA, uint32_t maxA, uint32_t countB, uint32_t maxB) { return countA == countB && maxA == maxB; }

// Count a hit; true when this hit must also be verified against a real scan.
inline bool onHit(Stats& st, uint32_t verifyEvery) {
    ++st.hits;
    return dueVerify(st.hits, verifyEvery);
}
inline void onMiss(Stats& st, uint64_t ticks) { ++st.misses; st.missTicks += ticks; }
inline void onVerify(Stats& st, bool same) { ++st.verifies; if (!same) ++st.mismatches; }

// PERF line fragment: "texscan calls=N hits=N misses=N verifies=N mismatches=N miss_ticks=N"
inline int formatStats(char* buf, size_t n, const Stats& st) {
    return snprintf(buf, n, "texscan calls=%llu hits=%llu misses=%llu verifies=%llu mismatches=%llu miss_ticks=%llu",
                    (unsigned long long)st.calls, (unsigned long long)st.hits, (unsigned long long)st.misses,
                    (unsigned long long)st.verifies, (unsigned long long)st.mismatches, (unsigned long long)st.missTicks);
}

// ---- call-site patching ----------------------------------------------------------------------------------------------------
// rel32 for an E8 at `site`: target - (site + 5), 32-bit wrap (matches the engine's own encoding).
inline int32_t rel32(uintptr_t site, uintptr_t target) { return static_cast<int32_t>(static_cast<uint32_t>(target) - static_cast<uint32_t>(site + 5)); }

inline void buildCall(uintptr_t site, uintptr_t target, uint8_t out[5]) {
    out[0] = 0xE8;
    int32_t d = rel32(site, target);
    memcpy(out + 1, &d, 4);
}

struct Site {
    uintptr_t at = 0;
    uint8_t expect[5] = {0, 0, 0, 0, 0};   // original call to origFn
    uint8_t repl[5] = {0, 0, 0, 0, 0};     // call to the replacement
    uint8_t seen[5] = {0, 0, 0, 0, 0};     // bytes found at install time
    int rc = 0;                            // 0 = not attempted, else codepatch::writeChecked result (1 ok, -1 mismatch, -2, -4)
    int restoreRc = 0;                     // uninstall result (0 = not attempted)
};

struct Sites {
    Site site[kSiteCount];
};

// Fill the sites: expect = E8 to origFn, repl = E8 to newFn. addrs has kSiteCount entries (the game's kSiteAddr, or host buffer addresses).
inline void initSites(Sites& s, const uintptr_t* addrs, uintptr_t origFn, uintptr_t newFn) {
    for (int i = 0; i < kSiteCount; ++i) {
        s.site[i] = Site();
        s.site[i].at = addrs[i];
        buildCall(addrs[i], origFn, s.site[i].expect);
        buildCall(addrs[i], newFn, s.site[i].repl);
    }
}

// Patch each site independently; a mismatching site stays untouched. Returns the number of sites patched.
inline int install(Sites& s) {
    int ok = 0;
    for (int i = 0; i < kSiteCount; ++i) {
        Site& p = s.site[i];
        p.rc = codepatch::writeChecked(p.at, p.expect, p.repl, 5, p.seen);
        if (p.rc == 1) ++ok;
    }
    return ok;
}

// Restore only sites whose install rc was 1 (and that still hold our call; a changed site is left alone, restoreRc = -1).
// Returns the number restored.
inline int uninstall(Sites& s) {
    int ok = 0;
    for (int i = 0; i < kSiteCount; ++i) {
        Site& p = s.site[i];
        if (p.rc != 1) continue;
        uint8_t seen[5];
        p.restoreRc = codepatch::writeChecked(p.at, p.repl, p.expect, 5, seen);
        if (p.restoreRc == 1) { ++ok; p.rc = 0; }
    }
    return ok;
}

}  // namespace texscan
