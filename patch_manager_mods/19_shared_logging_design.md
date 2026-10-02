# 19. Shared logging / probe library for kotor-patch-manager patches (DESIGN, 2026-10-02)

Status: **PROPOSAL / planning only. Nothing implemented.** Audience: every contributor to patch DLLs (our sibling sessions and outside contributors). Goal: one logging library, one ini schema, one naming workflow, so logs from different patches can be correlated and switched on/off the same way.

## 1. Why
Survey of 16 existing logging DLLs (10-02) found:
- **No common clock.** Formats: raw tick; `HH:MM:SS.mmm`; QPC ms since first call; tick+frame; nothing (inv3d, inventory_sort_filter, buffhud). Cross-log correlation is manual.
- **~10 DLLs `fopen`/`fclose` per line** (expensive under Wine); only mouse-probe (held handle) and companion-ai (buffered, rate-limited) avoid it.
- **Inconsistent caps/rotation**: line caps differ (5000..60000), some none; truncate-at-load vs append-forever vs rotate (`.1`, `.old`). Line caps starve other lines (shield bar: `SWAP` = 19.5k of 20k lines).
- **Inconsistent switches**: `*_debug.txt`, `*.txt` with `enabled=0`, `*_off.txt`, `*_nodraw.txt`, `*_dump.txt`, `.off` rename, ini keys; polled every 1000 ms / 250 ms / 100 ms / 30 frames / 30 calls. Some are read only in DllMain (flag created after launch does nothing).
- **~12 hand-copied `logf` helpers** (3 shapes), copied ini-reload helper, per-DLL SafeRead ring.
- **Parsers depend on exact printf strings** (`analyze_play_logs.py` for ai_trace/ranged/sort-filter, `check_effect_decode.py`, `perf_lab_src/resolve_sites.py`); shield-bar tags (`MENUSLOT DRAW POOL CAPFIX ...`) parsed only by hand.
- **Include path mismatch**: sources use `../_shared/SafeRead.h`; repo dir is `_shared_src/`; framework has `Patches/_shared/` (lacks GuiKit.h). Copies can drift. `ForcePowerHotbar.cpp` still calls `IsBadReadPtr`.
- Framework has no logging API for patches (`Platform::Log` -> OutputDebugString / `KPATCH_LOG`), no CONTRIBUTING file, no cross-DLL import precedent (but patches share one process, so `GetProcAddress` between DLLs is possible; load order not guaranteed).

## 2. Design principles
1. **Header-only first** (`_shared_src/KpLog.h`), zero runtime dependency between DLLs. Each patch keeps its own hooks and state. Works for outside contributors with only the header.
2. **Optional shared sink** (phase 2): a tiny `kp-log` DLL that owns ONE merged file and ini. Patches look it up with `GetModuleHandle/GetProcAddress` at first log call; if absent they fall back to their own per-patch file. Never a hard dependency, never a load-order assumption (resolve lazily, retry).
3. **Backward compatible formats**: tag/line text stays exactly as is; the library only adds a common prefix and sink. Analyzer regexes must keep working (add prefix-stripping in parsers, run `test_analyze_play_logs.py`).
4. **Hot path never does file I/O**: format into a thread-local/ring buffer, flush on a timer/size, or by the sink thread. Never block a render hook. Never allocate in hooks.
5. **Safe by default**: logging OFF unless enabled; one kill switch; caps by bytes; read engine memory only through SafeRead.

## 3. Proposed API (sketch)
```cpp
#include "KpLog.h"
KP_LOG_INIT("buff-duration-hud");               // DllMain: names the patch, reads ini
KP_LOG(ch_draw, "DRAW slot=%d x=%.1f", s, x);    // channel + printf; rate/level checked first
KP_LOGV(ch_pool, 2, "...");                      // verbose level
KP_LOG_EVERY(ch_cam, 100 /*ms*/, "...");         // built-in rate limit
KP_MARK("F9");                                   // marker line in all patches
KpLog::enabled(ch_draw);                         // cheap check before building expensive args
KP_LOG_SHUTDOWN();
```
- Every line: `T=<QPC ms since process start, 3 dp> f=<frame#> tick=<GetTickCount> <patch>/<channel> <message>`. Frame counter is exported by a patch that owns a frame hook (or tick only). One session header with wall-clock + tick so logs map to Proton log time.
- Channels registered per patch (`KP_CHANNEL(draw)`), toggled individually in the ini.

## 4. Unified ini schema (`kplog.ini` in the game dir, hot-reloaded every 1000 ms by mtime)
```ini
[global]
enabled=0              ; master switch (replaces most *_debug.txt flag files)
sink=perpatch          ; perpatch | merged
flush_ms=500
max_bytes=8000000      ; per file; rotate to .1 (keep 1)
max_lines_per_sec=2000 ; per patch, drops counted and reported
[buff-duration-hud]
enabled=1
channels=draw,pool,tip,capfix,menuslot  ; or * ; -swap to exclude
level=1
[mouse-probe]
enabled=1
```
- **Legacy flag files keep working** for one release (map: `shieldprobe.txt` -> buff-duration-hud level 2, `mouseprobe.txt` -> mouse-probe enabled, ...), then deprecate. **Behavioural kill switches that change gameplay/drawing** (`shield_nodraw.txt`, `buffhud_off.txt`, `shield_noswap.txt`) are NOT logging switches: keep them, but name new ones `<patch>_off.txt` per convention.
- Flag/ini created after launch must work (hot reload), never DllMain-only.

## 5. Conventions for contributors (workflow doc, to be published)
- Log file names: `<patch-id>_log.txt` (perpatch) or `kp_log.txt` (merged); crash/diagnostic dumps `<patch-id>_dump.txt`.
- Channel names: lowercase `[a-z0-9_]`, <=12 chars; tag text inside lines stays UPPERCASE (`MENUSLOT`) so `grep` works.
- Every probe-only line must be behind a channel; production default logs only `WARN`/`ERR` (installation, hook failures, CAPFIX-like recoveries).
- Never write a log line from a hook without the enabled() check; format arguments that need engine reads via SafeRead.
- Document each patch's channels in its doc under `## Logging` (channel, rate, meaning, parser).
- Parsers (`scripts/`/`mods678_research`) must strip the common prefix with a single shared regex (`scripts/kplog_parse.py`, to be written) and have a format test.
- Include path: one canonical `_shared_src/` copied by the build script into each patch build dir (fix the `_shared` vs `_shared_src` split; add a drift check).

## 6. Phased plan
| Phase | Work | Owner/gate |
|---|---|---|
| 0 | Fix include-path split + drift check; remove direct `IsBadReadPtr` in hotbar (sibling-owned: ask). Agree this doc with sibling sessions/contributors. | main + user |
| 1 | Write `KpLog.h` (per-patch sink, ring buffer + flush thread, QPC clock, caps, ini hot reload, legacy flag shim) + unit test host build (Linux g++ test of formatting/caps/rate limiting) | Sonnet agent, main reviews |
| 2 | Migrate `mouse-probe` (new, no conflicts) as reference implementation; measure cost vs per-line fopen under Wine (perf-lab) | main |
| 3 | Migrate `buff-duration-hud` probe lines (keep exact tags; document in doc 08/02) | main |
| 4 | Optional `kp-log` merged-sink DLL + `kplog_parse.py` + merged timeline viewer; `F9` MARK across patches | decide after 1-3 |
| 5 | Sibling-owned patches migrate on their own schedule (hotbar, inv3d, equip3d, chareffects, levelup, ai, perf-lab); small fix patches share one CS-based helper | owners |
| 6 | Contributor-facing doc (README section / CONTRIBUTING for upstream), example patch | main |

## 7. Open decisions (for the user)
1. Merged single file by default, or per-patch files by default with merged as opt-in? (Recommend per-patch default, merged opt-in.)
2. Byte cap + 1 rotation vs line caps (Recommend bytes + rate limit; fixes cap starvation.)
3. Upstream: propose `KpLog.h` to the kotor-patch-manager project (needs coordination with its maintainer) vs keeping it in our `_shared_src` first. (Recommend ours first, upstream after Phase 3 proves it.)
4. Frame counter source: a shared export from one patch, or each patch counts its own hook calls?
5. Threading: single flush thread per DLL vs sink DLL thread (affects Wine cost).

## 8. Risks
- Behaviour change in sibling-owned DLLs: migrate only with owner agreement; never edit their sources from another thread.
- Perf: prove with perf-lab before touching hot paths; default OFF.
- Parser breakage: keep tag text stable; prefix is strictly additive.
- Wine: file I/O from a helper thread; CRITICAL_SECTION or lock-free ring; no logging from DllMain loader-lock paths.
