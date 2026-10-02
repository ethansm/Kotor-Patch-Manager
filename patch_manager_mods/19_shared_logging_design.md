# 19. Shared logging / probe library for kotor-patch-manager patches (DESIGN, 2026-10-02)

Status: **PROPOSAL / planning only. Nothing implemented.** Revised 10-02 (rev 2): the service lives in the KotorPatcher core, not in a per-patch header or a separate sink DLL (user-confirmed direction). Audience: every contributor to patch DLLs (our sibling sessions and outside contributors). Goal: one logging library, one ini schema, one naming workflow, so logs from different patches can be correlated and switched on/off the same way.

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

## 2. Goal and direction (rev 2, user-confirmed 10-02)
Goal: a **framework-level logging standard for kotor-patch-manager**, so every mod logs compatibly (one clock, one config, one format) and cheaply (no file I/O on game threads, one writer for the whole process).

Direction: **the logging service lives in the patcher core (`KotorPatcher.dll` / `.so` / `.dylib`)**; patches reach it through `Patches/Common`. Rev 1's per-patch `_shared_src/KpLog.h` and optional `kp-log` sink DLL are dropped.

Why the core:
- **It is always there first.** The patcher is loaded before any patch: on Windows it loads every patch DLL itself (`Platform::LoadModule`, patcher.cpp:226/252); on native Linux `KotorPatcher.so` is a `DT_NEEDED` of the game. No load-order question, unlike a sink DLL.
- **One of everything.** One clock origin, one frame counter, one `kplog.ini` reader, one set of file handles, one flush thread for the whole process (instead of up to 16 per-DLL threads/handles). The expensive part of logging under Wine (file I/O) happens once, off the game threads.
- **Precedent.** The core already publishes state to patches (`KOTOR_VERSION_SHA` via `Platform::SetEnv`, patcher.cpp:122) and already has a platform seam (`platform_win32.cpp` / `platform_posix.cpp`) for clocks, files and threads.
- **Existing patches benefit unchanged.** Upstream's `Patches/Common/Common.h` already has `debugLog()` / `debugHexDump()` (OutputDebugString, used by ~29 upstream patches). Routing them through the service gives every upstream patch the common prefix and ini control with no source edits.

## 2a. Upstream context: RFC #180 (checked 10-02)
Upstream issue **LaneDibello/Kotor-Patch-Manager#180 "RFC: how should patches talk to each other?"** (@ProjectSynchro, 2026-08-26, OPEN, 0 comments, not implemented: no `KPatchInit`/`KPatchApi` in upstream/master) proposes the mechanism this design needs:
- A plain-C `KPatchApi { struct_size, abi_version, provide(name, version, iface), require(name, version) }` registry in the patcher.
- **Push, not pull**: after `LoadModule` the patcher calls an optional per-patch export `void KPatchInit(const KPatchApi*)`. No new patcher exports, no platform-specific module lookup; patches without `KPatchInit` are untouched.
- Constraints: plain C across the boundary (MSVC and MinGW patches share one process), callers `require` lazily at first use.
- **Its open question 2 asks exactly our question:** "Should the patcher expose its own services through the same table (logging, `GameVersion`)?"
No other upstream issue or PR covers logging (searched log/logging/debugLog/trace/verbose/export/shared, issues + PRs, all states). Related only loosely: #12 (Feedback scrollback), #28/#42 (debug features).

**Consequence for this design:** logging should be the first patcher-provided service on the #180 registry (interface `"kpatch.log"`, version 1), not a separate `KpLog_GetApi` export. That gives #180 a concrete first consumer and gives us an ABI the maintainers already proposed. Section 3 is written that way; the standalone export is kept only as the fallback if #180 is not adopted.

## 3. Architecture
```
 patch DLL                         Patches/Common/KpLog.h (header)         KotorPatcher core
 KP_LOG(ch,"DRAW x=%d",x) ──► enabled? rate? ──► format into TLS buf ──► Api->Submit(line) ──► ring buffer
                                  (no lock, no I/O)                            │
                                                                     flush thread (1 per process)
                                                                     ├─ <patch>_log.txt (default)
                                                                     ├─ kp_log.txt (merged, opt-in)
                                                                     └─ caps, rotation, kplog.ini reload
```
- **ABI: a `"kpatch.log"` v1 interface on the RFC #180 registry.** The core `provide`s a plain-C table (`struct_size`, `Register`, `Enabled`, `Submit`, `Frame`, `FrameTick`, `Mark`, ...); new functions are appended only. `Patches/Common/KpLog.h` defines `KPatchInit` for the patch (or chains to the patch's own) and `require`s `"kpatch.log"` there.
- **Before `KPatchInit` runs** (the patch's `DllMain` / static init, which execute inside `LoadModule`): lines go to a small static pre-init buffer in the patch, drained into the service when `KPatchInit` arrives (or to `debugLog()` if it never does).
- **Fallback if #180 is not adopted**: the core exports one symbol, `KpLog_GetApi(abiVersion)`, resolved lazily by the header (Windows `GetModuleHandleA("KotorPatcher.dll")` + `GetProcAddress`; Linux/macOS `dlsym(RTLD_DEFAULT, ...)`). Same table either way, so patches do not change.
- **Fallback when the symbol is absent** (older patcher, patch tested standalone): the header logs to `debugLog()` (today's behaviour) or, if the patch opts in, to its own buffered file. A patch never fails to load because logging is missing.
- **Hot path**: `Enabled(channel)` is a read of a flags word the service updates on ini reload; format into a thread-local buffer; `Submit` copies into a lock-free/short-CS ring. No allocation, no I/O, never blocks; when the ring is full the line is dropped and counted (`DROPPED n` reported by the flush thread).
- **Flush thread**: created by the core during init (before patches load), never waited on under the loader lock. On process exit the core flushes synchronously in `CleanupPatcher` / `DLL_PROCESS_DETACH` / the `.so` destructor.
- **Cross-platform**: clock, file I/O and threads go through the core's `Platform::` seam (Win32 QPC / `clock_gettime(CLOCK_MONOTONIC)`). The header uses no Windows-only calls in its fast path.

## 4. Line format
`T=<ms since patcher init, 3 dp> f=<frame#|-> <patch>/<channel> <message>`
- Message text is the patch's existing tag text, unchanged (parsers strip the prefix with one shared regex, see section 7).
- **Frame counter lives in the core.** A patch that already has a per-frame hook calls `KpLog_FrameTick()` (first caller wins; others' calls are ignored); until one does, lines print `f=-`. Later option: the core installs its own per-frame hook from the AddressDatabases per game build.
- One session header per file: wall-clock time, `KOTOR_VERSION_SHA`, patcher version, list of registered patches/channels, so logs map onto Proton/Steam log times.

## 5. Patch-facing API (Patches/Common/KpLog.h)
```cpp
#include "KpLog.h"
KP_LOG_PATCH("buff-duration-hud");               // once per patch (registers name + channels lazily)
KP_CHANNEL(draw); KP_CHANNEL(pool);              // channel declarations
KP_LOG(draw, "DRAW slot=%d x=%.1f", s, x);       // enabled/rate checked before formatting
KP_LOGV(pool, 2, "...");                         // verbose level
KP_LOG_EVERY(draw, 100 /*ms*/, "...");           // per-call-site rate limit
KP_WARN("hook failed at %08X", addr);            // WARN/ERR always on (production default)
KP_MARK("F9");                                   // marker written to every open log
if (KP_ENABLED(draw)) { /* build expensive args (SafeRead) */ }
```
`debugLog()` / `debugHexDump()` keep their signatures and route to channel `debug` of the calling patch (default sink: OutputDebugString, as today, unless `kplog.ini` says otherwise).

## 6. Config: one `kplog.ini` beside `patch_config.toml`, hot-reloaded (mtime, 1000 ms) by the flush thread
```ini
[global]
enabled=0              ; master switch; logging off unless enabled
sink=perpatch          ; perpatch (default) | merged | both
flush_ms=500
max_bytes=8000000      ; per file; on overflow rotate to .1 (keep 1)
max_lines_per_sec=2000 ; per patch; drops counted and reported
debuglog=ods           ; ods (today) | file | both  -- where legacy debugLog() goes
[buff-duration-hud]
enabled=1
channels=draw,pool,tip,capfix,menuslot  ; or * ; -swap to exclude
level=1
```
- Created/edited after launch must take effect (hot reload). Nothing is read only at load.
- **Legacy flag files** (`shieldprobe.txt`, `mouseprobe.txt`, ...) keep working for one release via a mapping table in each migrated patch, then are deprecated.
- **Behavioural kill switches** (`shield_nodraw.txt`, `buffhud_off.txt`, ...) change gameplay/drawing, not logging: out of scope; new ones named `<patch>_off.txt`.

## 7. Conventions for contributors
- Files: `<patch-id>_log.txt` (perpatch), `kp_log.txt` (merged), dumps `<patch-id>_dump.txt`, all beside the game exe.
- Channels: lowercase `[a-z0-9_]`, <= 12 chars; tag text inside lines stays UPPERCASE (`MENUSLOT`) for grep.
- Every probe-only line behind a channel; production default = `WARN`/`ERR` only.
- Never log from a hook without the enabled check; engine reads for arguments go through SafeRead (moved into `Patches/Common`).
- Each patch doc gets a `## Logging` section (channel, rate, meaning, parser).
- Parsers strip the prefix with one shared regex (`scripts/kplog_parse.py` here; a copy in upstream `tools/`), each with a format test.

## 8. Phased plan
| Phase | Work | Where | Gate |
|---|---|---|---|
| 0 | **DONE 10-02.** `SafeRead.h` + `GuiKit.h` canonical in fork `Patches/Common` (KPM aad0a14); `Patches/_shared/*.h` = forwarding headers so `../_shared/` includes build unchanged (BuffDurationHUD + InventorySortFilter rebuilt OK, GuiKit syntax-checked both include styles). Edit in `_shared_src/`, then `scripts/check_shared_headers.py --sync`; plain run = drift check (exit 1). Hotbar `IsBadReadPtr` -> SafeRead NOT done: sibling-owned, hotbar has uncommitted work; left to its owner. | fork | done |
| 0b | **Drafted 10-02: `19a_rfc180_comment_DRAFT.md`. Post only together with the Phase 5 PR (user decision).** Comment on RFC #180: answer its open question 2 ("yes, logging first"), propose `"kpatch.log"` v1 (section 3) + line format (section 4) + answers to its threading question (Submit callable from any thread). **Draft only; posting needs user go.** If #180 needs an implementation first, offer to write the registry in the same PR. | upstream | user |
| 1 | Core: `KpLog_GetApi` + service (ring, flush thread, files, caps/rotation, ini reload, frame counter) in `src/KotorPatcher`, Win32 + POSIX platform pieces; core unit tests (format, caps, rotation, rate limit, ring overflow, ABI version mismatch) in `src/KotorPatcher/tests` | fork `ethansm/local-mods` | Sonnet agent writes, main reviews; existing core tests + mutation gate pass |
| 2 | Header: `Patches/Common/KpLog.h` + `debugLog()` reroute + fallback path; test a patch against (a) new core, (b) old core (fallback) | fork | main |
| 3 | Migrate `mouse-probe` (reference) then the `buff-duration-hud` probe lines (exact tags kept); perf-lab A/B vs per-line fopen under Wine | fork, live install | user play test |
| 4 | Merged sink mode + `kplog_parse.py` + merged timeline viewer; `F9` MARK across patches | fork | after 3 |
| 5 | Upstream PR = #180 registry implementation + `kpatch.log` as its first service + `KpLog.h` + migrated examples + `docs/Logging.md`, posted together with the 0b comment, once 3 is proven | upstream | user go + maintainer |
| 6 | Sibling-owned patches migrate on their owners' schedule (hotbar, inv3d, equip3d, chareffects, levelup, ai, perf-lab) | fork | owners |
| 7 | Contributor doc: `docs/Logging.md` upstream + example patch | upstream | with 5 |

## 9. Decisions
Settled (10-02, user confirmed core direction):
1. Per-patch files by default, merged opt-in (`sink=`).
2. Byte caps + 1 rotation + per-patch rate limit; no line caps.
3. Built for upstream from the start: proven on our fork in upstream's layout, ABI proposed on RFC #180 before Phase 1 code (0b), PR after Phase 3.
4. Frame counter owned by the core; advanced by `KpLog_FrameTick()` from a patch with a frame hook; `f=-` until then.
5. One flush thread, in the core.

Still open:
- Maintainer response on RFC #180 (0b): registry adopted (logging rides on it) vs not (fallback `KpLog_GetApi` export) vs core change declined (keep the service on our fork; header fallback stays upstream-compatible).
- ~~Whether we implement #180's registry ourselves~~: yes (user 10-02), in the same PR as the logging service.
- Ring implementation: lock-free MPSC vs short CRITICAL_SECTION/mutex (decide with perf-lab numbers in Phase 3).

## 10. Risks
- **Core change = every user's patcher.** Default OFF; WARN/ERR only; the flush thread is created only when `enabled=1` or a WARN/ERR is logged. Existing core tests + mutation gate must pass.
- **ABI lock-in**: one versioned symbol, append-only table, size field checked by the header.
- **Loader lock**: core never waits on its thread inside `DllMain`; patches may log from `DllMain` (Submit is non-blocking).
- **Behaviour change in sibling-owned DLLs**: migrate only with owner agreement.
- **Parser breakage**: tag text unchanged; prefix strictly additive; `test_analyze_play_logs.py` must pass.
