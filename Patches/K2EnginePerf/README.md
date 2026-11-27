# K2 Engine Perf

> **STATUS** Track: EXPERIMENTAL. Version 0.2.0-dev (matches manifest.toml).
> **What it does / explores:** Caches the per-frame `GlobalMemoryStatus` call (250 ms TTL) to cut Wine/Proton CPU overhead, plus an optional per-pass QPC profiler (`pass_profile`, default off).
> **Requires:** KOTOR II Steam/Aspyr exe (SHA-256 `6A522E71...`), Kotor Patch Manager with the KpLog core.
> **Known limits:** the profiler wrappers add QPC overhead and are diagnostic only; metrics undercount if another mod hooks the same IAT slot first (perf_lab). **Testing:** live-tested on Steam.
> **Install:** the cache is safe for normal play; leave `pass_profile` at 0 unless profiling.


Performance optimization patch for KOTOR II (Steam Aspyr Windows x86, SHA-256 `6A522E71...`), targeting Wine / Proton runtime overhead.

## What it does

The engine calls `GlobalMemoryStatus(&ms)` twice per frame inside `0x00478A20` (`SetLeader` / `AurMakeCurrent` area) at call site VA `0x00478A57` (`FF 15 28 61 98 00`, calling through the KERNEL32 IAT slot at `0x00986128`) to compute physical memory availability.

Under Wine and Proton, every invocation of `GlobalMemoryStatus` reads and parses `/proc/meminfo`, consuming ~40 to 50 µs per call. At ~120 calls per second, this consumes ~5 ms per second of CPU time needlessly querying host OS kernel memory.

`k2-engine-perf` redirects the single call at `0x00478A57` to an in-memory cache wrapper (`MemStatusCache.h`) with a 250 ms TTL. Over 96% of calls are served from cache, saving ~4.7 ms/s CPU overhead with zero impact on game mechanics.

### 0.2.0-dev: per-pass QPC profiler (diagnostic, `pass_profile`, default off)

Seven 5-byte `E8 rel32` CALL sites are rewritten (`codepatch::writeChecked`, restored on FreeLibrary) to timing wrappers that forward to the real callee and, when `pass_profile = 1`, add the QPC delta to a per-pass accumulator. With `pass_profile = 0` the wrappers only forward. Nothing about game behaviour changes; it only measures.

| Key in `PERF_PASS` | Site | Callee | Convention |
|---|---|---|---|
| `emit_sim_ms_s` | `0x00469A88` | `0x004EC080` PartEmitter::AnimateParticles | thiscall, 1 stack arg |
| `emit_draw_ms_s` | `0x00469A99` | `0x004E92A0` PartEmitter::RenderFunc | thiscall |
| `blend_ms_s` | `0x004660CA` | `0x004DB1F0` Material::SetBlendingMode | thiscall, 1 arg |
| `dedupe_ub_ms_s` | `0x00466161` | `0x00476620` PtrVec::CountOf | thiscall, 1 arg (81-byte callee in a loop: QPC overhead included, read as an upper bound) |
| `skin_draw_ms_s` | `0x004CD3ED` | `0x004C9640` Global::RenderSkinnedVPBlur | cdecl, 1 arg |
| `anim_ms_s` | `0x004E2ED1` | `0x004DFED0` Global::AnimateHierarchy | cdecl, 6 args |
| `shdw_ms_s` | `0x00468DC1` | `0x00465C90` Global::ProjectDynamicShadow | cdecl, 2 args |

Each value is CPU milliseconds per wall-second in the 5 s window, with the call count in parentheses. Callee ABIs were verified by disassembly of the Steam exe.

### 0.1.2-dev Changes
- **Texture-id scan cache removed:** 0.1.0/0.1.1 attempted to cache `TexMgr::RebuildIdList` (`0x00427970`) across bucket drains. Live telemetry across 139+ sampling windows recorded 0 hits (scan runs only ~1x/frame and takes ~5.5 µs). The 3 CALL hooks, `TexScanCache.h`, and INI keys `tex_scan_cache` / `tex_verify_every` were completely pruned.

## Configuration: `k2_engine_perf.ini`

Hot-reloaded in real-time from the game directory:

```ini
[K2EnginePerf]
mem_status_cache = 1   ; 1 = enable 250 ms cache (default), 0 = bypass directly to real API
mem_status_ms = 250    ; cache validity in milliseconds (clamped 0..5000)
pass_profile = 0       ; 1 = time the 7 render/animation call sites and log PERF_PASS (diagnostic, default 0)
perf_log = 1           ; 1 = emit PERF telemetry line every 5s, 0 = silent
verbose = 0            ; 1 = verbose debug logging
```

## Logging & Telemetry

Logs are written to `k2-engine-perf_log.txt` via `KpLog.h`:
- `patch mem_status @0x00478A57 expect=FF 15 28 61 98 00 seen=FF 15 28 61 98 00 rc=1`
- `PERF 5.0s gms calls=588 real=20 real_us=42.2 saved_us_s=4793`
- with `pass_profile = 1`: 7 more `patch <name> @0x... rc=1` lines at startup and `PERF_PASS 5.0s emit_sim_ms_s=..(calls,max=..) emit_draw_ms_s=..(calls) skin_draw_ms_s=..(calls) anim_ms_s=..(calls) shdw_ms_s=..(calls) blend_ms_s=..(calls) dedupe_ub_ms_s=..(calls)`
- Existing installs keep their `k2_engine_perf.ini` (it is only written when missing): add `pass_profile = 1` by hand to profile.

## Verified Addresses

- Call site: `0x00478A57` (inside `0x00478A20`, `SetLeader` / `Global::AurMakeCurrent`).
- IAT slot: `0x00986128` (KERNEL32 `GlobalMemoryStatus`).
- Engine global: `0x00A309AC` (float target for `ms.dwAvailPhys`).

## Installation & Rollback

- **Install:** Hot-swappable DLL via KPM (`patches/k2-engine-perf.dll`).
- **Rollback:** reinstall the previous DLL through KPM, or set `mem_status_cache = 0` in `k2_engine_perf.ini`.
