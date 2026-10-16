# 19e. Phase 2 plan: KpLog.h (2026-10-03)

Status: in progress. Approved plan below; execution log at the end.


## Context
Phase 1 is done: the RFC #180 registry and the `kpatch.log` v1 service live in the KotorPatcher core.
- Branch `feat/patch-registry-logging`, e36afa4..4fd1d12, worktree `~/Games/KOTOR2_Mod_Workspace/tools/kpm-registry`.
- A sibling session already merged it into the fork branch `ethansm/local-mods` as **fa67a3b**; the fork's HEAD is now fa653d0.
- **It is not installed live.** The live `KotorPatcher.dll` dates from Sep 28 and contains no kplog code.

The goal is a framework-level logging standard. Phase 2 gives patches the convenience header that sits on top of the service, and proves it on the reference patch mouse-probe. The phase also covers:
- the shared parser;
- the upstream PR draft;
- a 32-bit verification pass;
- a shared memory that tells every Claude session on this machine to convert its mods to the new logger in our local build.

**New rollout rule (user, 10-03):** whichever mod ships first with the logger triggers the upstream PR for the logger (registry + `kpatch.log` + `KpLog.h`). It goes up together with that mod. Posting still needs the user's explicit go.

**Resume docs** (local-setup repo `/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/`):
- 19c: session context, "Next" + "Session 2 hand-off".
- 19d: Phase 1 plan and execution log.
- 19: design.
- 19a: the held RFC comment.
- 19b: the cross-patch survey.
- Memory: `kotor2-shared-logging`.

User decisions (10-03):
- The `debugLog()` reroute is **opt-in only**. It applies only to patches that include `KpLog.h` and define `KPatchInit`; unmigrated patches behave exactly as today.
- Scope includes mouse-probe, `kplog_parse.py`, the PR draft and the 32-bit run.
- `KpLog.h` lives in the KPM repo next to `kpatch_api.h`, not in the `_shared_src` flow.

## Ground rules (carry over)
- **Delegation:**
  - Sonnet `general-purpose` subagents (`model: "sonnet"`) do each mechanical chunk, one at a time.
  - Opus re-runs the acceptance checks, reviews the diff and commits.
  - Brief each agent with: this plan's path plus its chunk, the worktree, the allowed files, the acceptance commands, "do not commit, no git stash; report a diff summary and test output", and the conventions.
- **Commit style:** `patcher:`/`Common:`/`docs:` subject, then a prose paragraph, then `- path: change` bullets, then the `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>` trailer.
- **Before each build/test:** `ps aux | grep -E 'make|cc1'`.
- **Gate:** `scripts/kpm_registry_gate.sh` in local-setup, which hardcodes the `kpm-registry` path.
  - Point it at `$W/src/KotorPatcher` through a `W=` env override (Opus adds that override in Step 0).
  - It tolerates only missing-multilib errors. With multilib present it demands full `make all test`.
- **Never launch the game. Never touch live files** without the user's go.
- **Repo boundaries:**
  - Sibling sessions edit the fork clone and local-setup. Never touch untracked `Patches/MouseProbe/` in the fork clone except in chunk M. Commit only our own paths.
  - Never edit sibling-owned mods' sources.
- **Upstream:** never post upstream (PR or #180 comment) without the user's go.
- **Persistence:** after every chunk, append to `19e_phase2_plan.md` "Execution log" and commit (local-setup).

## Step 0 (Opus)
1. Copy this plan to `patch_manager_mods/19e_phase2_plan.md` and commit it in local-setup.
2. Branch: in `kpm-registry`, run `git switch -c feat/kplog-header feat/patch-registry-logging`. It stacks on the registry branch, so the upstream PR can be the stack. Call this worktree `$W`.
3. Fix stale docs. Doc 19 status (line 3) and Phase 1 row (line 116) say "not merged into local-mods". Correct both to "merged fa67a3b (sibling session), not installed live". Commit.
4. Add the `W=` env override to `scripts/kpm_registry_gate.sh` (default stays the `kpm-registry` path) and commit it.
5. Baseline: run `W=$W scripts/kpm_registry_gate.sh` and record the result. If `/usr/lib/gcc/x86_64-linux-gnu/13/32` exists (multilib installed), run chunk T first.

## Chunk T: 32-bit full verification (Sonnet; only if multilib is installed, otherwise record "skipped" and re-check at the end)
- **Run:** in `$W`, `make -C src/KotorPatcher all test` must be fully green, i386 rows included, and the Linux `KotorPatcher.so` must build.
- **Real 32-bit smoke:** recreate the Phase 1 host smoke (19d "Chunks C, D, E" describes it) with a 32-bit driver.
  1. `gcc -m32` driver; it `dlopen`s `build/KotorPatcher.so`.
  2. The temp dir holds `patch_config.toml` with DLL_ONLY entries for `build/tests/fixtures/32/{provider,logger}.so`, plus `kplog.ini`.
  3. The driver resolves `_ZN12KotorPatcher14CleanupPatcherEv` and calls it before `_exit`. This works around the pre-existing dlclose-at-exit crash, which is not ours.
  4. Expect `logger-fixture_log.txt` with the header and the WARN/INFO lines.
  - Driver and script go in the session scratchpad.
- **Fix i386-only failures:** allowed files are the core and tests.
- **Accept:** gate prints "GATE PASS (full, multilib)" and the smoke files are correct.

## Chunk K: `Patches/Common/KpLog.h` + `KpDebugHook.h` (Sonnet)
Files: `Patches/Common/KpLog.h`, `Patches/Common/KpDebugHook.h`. No core changes.

**Constraints:**
- C++17; compiles with `-fno-exceptions -fno-rtti` (Linux/mac patch flags).
- No `windows.h`. Declare `extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char*);` under `_WIN32` instead.
- No `##__VA_ARGS__`/`__VA_OPT__`, because of the MSVC traditional preprocessor.
- On non-Windows, wrap all header state in `#pragma GCC visibility push(hidden)`/`pop`. GCC otherwise makes inline variables and statics `STB_GNU_UNIQUE`, which merges them across `RTLD_LOCAL` `.so`s.
- No 64-bit atomics. Use `std::atomic<uint32_t>` ms, because i386 would otherwise need libatomic.
- Use `fopen_s` under `_MSC_VER`.
- Comments explain why. Document that `KPatchInit` must be written literally in a `.cpp`; a macro cannot satisfy create-patch's export scan.

**API:**
- `KP_LOG_PATCH("id")`
  - Expands to a `static_assert(ValidId)` plus `namespace kplog { extern const char kPatchId[] = id; }`.
  - Exactly one `.cpp` uses it; a missing one is a link error.
- `int KpLog_Init(const KPatchApi* api)`
  - The patch calls it from its own `extern "C" void __cdecl KPatchInit(const KPatchApi* api)`.
  - It requires `"kpatch.log"` v1, needs `struct_size >= sizeof(KPatchLogApi)`, and `Register`s the id.
  - On success it sets the api atomically (release) and installs the debug hook, then returns 1.
  - On any failure it returns 0 and the patch stays in fallback. It is DllMain-safe: no I/O.
- `bool KpLog_HasCore()`, `void KpLog_SetFallbackLevel(int)`, `void KpLog_SetFallbackFile(const char* path)` (plus the `KP_LOG_FALLBACK_FILE` / `KP_LOG_FALLBACK_LEVEL` / `KP_LOG_FALLBACK_MAX_LINES` macros; defaults are WARN and 60000).
- `KP_CHANNEL(var, "name")`
  - Declares a constexpr-constructed `kplog::Channel { const char* name; std::atomic<int32_t> id{-2}; }`, at namespace scope or function-local.
  - Resolution is lazy: it stays -2 while there is no api; with an api it calls `Channel()` and caches the id.
  - A result of -1 is cached as dead and produces one fallback WARN `channel '<n>' rejected`.
- `KP_LOG(ch, ...)` is INFO. `KP_LOGV(ch, lvl, ...)` is `do { if (On(ch,lvl)) Write(ch,lvl,__VA_ARGS__); } while(0)`.
  - `Write` formats into a **stack** `char[512]` with the printf attribute on GCC/clang.
  - It splits on embedded `\n`, one `Submit` per piece.
- `KP_WARN(...)`/`KP_ERR(...)` use the built-in `main` channel.
- `KP_LOG_EVERY(ch, ms, ...)` uses a per-call-site `static std::atomic<uint32_t>` with CAS; the first call always logs.
- `KP_ENABLED(ch[, lvl])`, `KP_MARK(label)`, `KP_FRAME_TICK()`. Mark and FrameTick are no-ops in fallback.

**Fallback rule:** no api means the fallback sink on every call until init. There is no pre-init ring and no timeout heuristic.
- On the new core, `KPatchInit` precedes every hook, so only static-ctor/DllMain lines fall back.
- On the old core everything falls back, gated by `level <= fallbackLevel`.

**Fallback sink:**
- Uses the fallback file if one was set, opened lazily. Otherwise ODS on Windows and stderr elsewhere.
- Line: `<ms steady_clock> <id>/<ch> [WARN |ERR ]text\n`, written with one `fwrite` + `fflush`, which is thread-safe through the CRT lock.
- After the cap, writes `--- log cap reached ---` once.

**`KpDebugHook.h`:**
- `using KpDebugHookFn = void(*)(const char*, size_t); inline std::atomic<KpDebugHookFn> kp_debug_hook{nullptr};`
- `inline void kp_debug_emit(const char* text, size_t len)` calls the hook if set, else ODS on Windows / `fputs` to stderr elsewhere.
- Hidden visibility on non-Windows.
- `KpLog_Init`'s hook submits on channel `debug` at `KPLOG_DEBUG` (split on `\n`). It also emits ODS when `KP_DEBUGLOG_ODS`, which defaults to 1.

**Accept:**
- Chunk H tests pass.
- Compile checks: the `make kplog-check` target from chunk H, run with MinGW win32 and posix alike, `-Wall -Werror -fno-exceptions -fno-rtti`.

## Chunk C: `Common.h` opt-in reroute (Sonnet, small)
- `Patches/Common/Common.h`: `#include "KpDebugHook.h"`.
  - `debugLog` keeps its `vsnprintf_s` into `char[512]`, then calls `kp_debug_emit(buffer, strlen)` instead of `OutputDebugStringA`.
  - The `debugHexDump` rows do the same.
- With no hook set the behaviour is byte-identical (ODS).
- Document the caveat: `debugLog("a"); debugLog("b\n")` concatenated in ODS but becomes two file lines once rerouted.
- **Accept:** `tests/compile/kplog_common.cpp`, from chunk H, includes `Common.h` + `KpLog.h` in both orders and calls `debugLog`. It must compile with `i686-w64-mingw32-g++` and with `-posix`. Also build one upstream patch that uses `debugLog` (e.g. `Patches/ModOptions`) through `create-patch.py` with MinGW, unchanged, as a regression check.

## Chunk H: host tests for the header (Sonnet)
**Makefile (`src/KotorPatcher`):**
- `FIXTURES_CXX := $(basename $(notdir $(wildcard tests/fixtures/*.cpp)))`.
- In the width loop: `g++ $$m -std=c++17 -Wall -Wextra -fno-exceptions -fno-rtti -shared -fPIC -I../../Patches/Common`.
- Add a `kplog-check` target for the compile checks. It skips a toolchain that is missing.

**Fixtures:**
- `tests/fixtures/kplog_a.cpp` (id `kplog-a`) and `kplog_b.cpp` (`kplog-b`), sharing `tests/fixtures/kplog_body.inc` (kept outside the `*.cpp` glob).
- Exports: `KPatchInit`, `fx_log`, `fx_debuglog`, `fx_every`, `fx_threads`, `fx_set_fallback`, `fx_has_core`, plus a static-ctor line.

**`tests/any_kplog_header.cpp`:** each scenario runs in a `fork()` child, because `dlopen` state persists per process. The child does `LogService::Init(tmp, {startThread=false})`, writes the ini, and calls `Registry::Provide`/`OnModuleLoaded`.

| # | Scenario | Expect |
|---|---|---|
| 1 | INFO/DEBUG with ini enabled | lines in the `kplog-a/<ch>` file |
| 2 | no ini | WARN/ERR only |
| 3 | old core (never init) | fallback file + prefix, `fx_has_core()==0`, INFO gated |
| 4 | old core, no fallback file | stderr shows `kplog-a/` |
| 5 | static-ctor line | goes to fallback, not the core file |
| 6 | fake api: null require / short `struct_size` | stays fallback, no crash |
| 7 | **two fixtures with the same channel names** | separate files (GNU_UNIQUE guard) |
| 8 | 8 threads, first use of one channel | one id, no lost lines |
| 9 | `KP_LOG_EVERY(50ms)` ×1000 over ~200 ms | 4-6 lines |
| 10 | debuglog with/without init, embedded `\n` | debug channel / split / stderr |
| 11 | 300-byte line | `[...]` |
| 12 | 257th channel or bad name | one rejection WARN, then no-ops |

Also add `tests/compile/kplog_common.cpp`, used by chunk C.

**Accept:** gate PASS with the new row, 20× loop clean, TSan (64-bit, `setarch -R`) clean.

## Chunk D: docs (Sonnet drafts, Opus edits)
- **`docs/PatchRegistry.md`:** replace the "KpLog.h planned" note (around line 275) with a "Logging with KpLog.h" section:
  - minimal example (`KP_LOG_PATCH`, `KPatchInit` → `KpLog_Init`, `KP_CHANNEL`, `KP_LOG`, `KP_ENABLED` for expensive arguments);
  - fallback behaviour on old cores;
  - debugLog opt-in, plus the ODS concatenation caveat;
  - rate-limit and `KP_LOG_EVERY` advice;
  - channel naming (`[a-z0-9_]`, ≤12 chars, tags UPPERCASE, kept stable);
  - the exports.def note.
- **`src/KotorPatcher/README.md`:** a short pointer.
- **`tests/README.md`:** the C++ fixtures and `kplog-check`.
- **local-setup `19f_kplog_migration_guide.md`:** the checklist sibling sessions follow (see chunk A):
  1. Include path. Fork builds: `#include "KpLog.h"` (Windows `-I Common`); Linux/mac native: `"../Common/KpLog.h"`.
  2. Add `KPatchInit` to the `.cpp` and to a committed `exports.def`.
  3. Map each helper's tags to channels; keep tag text.
  4. Legacy flag file: gate it to `!KpLog_HasCore()` and drive `KpLog_SetFallbackLevel`.
  5. Behavioural kill switches stay.
  6. Parser impact: `ranged_ai`/`ai_trace` regexes are `^`-anchored in `mods678_research/analyze_play_logs.py:125-185`, so update the parser alongside or use `kplog_parse.py`'s prefix strip.
  7. Never call `KP_FRAME_TICK` from a hook that is not frame-global.
  8. Verify: build, no-ini run shows only WARN, ini run shows channels.

## Chunk M: mouse-probe migration (Sonnet; source of truth = local-setup `patch_manager_mods/mouse_probe_src/`, build copy = fork `Patches/MouseProbe/`)
1. `KP_LOG_PATCH("mouse-probe")` and a literal `extern "C" void __cdecl KPatchInit(const KPatchApi* api) { KpLog_Init(api); }`. Add `KPatchInit` to the committed `exports.def`. Bump the manifest version to 0.2.0.
2. Channels:
   - `main`: START/STOP
   - `raw`: RAW
   - `polls`: POLLS
   - `norm`: NORM
   - `drag`: DRAGSTATE; STUCKDRAG? goes out at **WARN**
   - `cam`: CAM
   Keep every tag word and field layout. The core prefix replaces the raw-QPC `%.3f` clock; note this in doc 18.
3. Gating:
   - Per-channel early-outs (`KP_ENABLED(raw)||KP_ENABLED(polls)`, …). Always track drag state.
   - The legacy `mouseprobe.txt` poll runs only when `!KpLog_HasCore()`. It toggles `KpLog_SetFallbackLevel(INFO/WARN)` and logs START/STOP.
   - `KpLog_SetFallbackFile("mouseprobe_log.txt")` with the 60000 cap, so on the live old core it behaves as today.
   - On the new core, log `START mouseprobe 0.2.0` once and use no STOP.
   - **No `KP_FRAME_TICK`**: the camera hook is in-world only, and first-caller ownership would lock out a better ticker. At most gate it behind `-DMOUSEPROBE_FRAMETICK`.
4. Build: the fork clone needs `KpLog.h` there first, so Opus merges `feat/kplog-header` into `ethansm/local-mods` after K/C/H/D are committed. Rebase is not allowed (shared branch); a merge commit is fine. Check that the sibling's latest commits are intact.
   Then sync `mouse_probe_src` → `Patches/MouseProbe/` and run `python3 Patches/create-patch.py MouseProbe` (MinGW). Stage the `.kpatch`; **do not install**.
5. Offline proof without the game (the Windows DLL can't be loaded natively on Linux; the KpLog behaviour itself is covered by chunk H's tests):
   - `objdump -p` must show the exports `KPatchInit`, `ProbeMouseRaw`, `ProbeMouseNorm`, `ProbeCameraFrame`.
   - `strings` must show `mouse-probe`, `kpatch.log`.
6. Commit `mouse_probe_src` (local-setup). In the fork, the MouseProbe dir is untracked and sibling-adjacent, so ask before tracking it in git; just stage the build.

**Accept:** build OK, exports OK, doc 18 + mouse-probe doc updated, and a ready-to-run install checklist for the user test (step U).

## Chunk P: `scripts/kplog_parse.py` + tests (Sonnet, local-setup)
- `parse_line(s)`:
  - `^T=(\d+)\.(\d{3}) f=(\d+|-) ([A-Za-z0-9_.-]+)/([A-Za-z0-9_.-]+) (?:(WARN|ERR) )?(.*)$` → dict with t_ms, frame|None, patch, channel, level, msg.
  - Also: `# ` header lines, `MARK` lines (`T=… f=… MARK <label>`), and the `kplog` DROPPED line `(ring a, rate b)`.
- `strip_prefix(s)` returns the bare message, for legacy parsers.
- CLI:
  - `kplog_parse.py FILE... [--patch P] [--channel C] [--level L] [--since MS] [--merge]`
  - `--merge` gives one timeline across per-patch files (sorted by T; the same session is detected by the header wall clock).
  - `--stats` gives per-patch/channel counts and drops.
- `scripts/test_kplog_parse.py` (unittest): format variants, f=-, WARN/ERR, truncation `[...]`, header/MARK/DROPPED lines, merge ordering, malformed lines ignored.
  - Use real sample files from the Phase 1 smoke output; regenerate them via the H fixtures if needed.
- **Accept:** `python3 -m unittest scripts/test_kplog_parse.py` green, and `analyze_play_logs` tests still green (`mods678_research/test_analyze_play_logs.py`).

## Chunk R: upstream PR draft (Opus, with Sonnet drafting allowed)
- **`patch_manager_mods/19g_upstream_pr_draft.md`:** title, summary, motivation (19a "why"), what is included (the registry, `kpatch.log`, `KpLog.h`, the opt-in `debugLog` hook, tests, docs).
  - Stack: `feat/patch-registry-logging` + `feat/kplog-header`, rebased onto current upstream/master at PR time.
  - The first mod that ships with the logger is a placeholder, filled in when it happens.
  - Testing notes: host tests; msbuild/mac only covered by CI; multilib status.
  - Known pre-existing issue: dlclose at exit in a dlopen'd harness.
  - The MinGW auto-export note.
  - End with `🤖 Generated with [Claude Code](https://claude.com/claude-code)`.
- **Refresh 19a:** `KpLog.h` exists now. The "two migrated patches" become the first shipped mod; there is no pre-init buffer (fallback rule instead); PR # stays blank. **HOLD**: post nothing.
- **Doc 19 §8/§8a:** record the new rollout rule (PR goes up with the first mod that ships with the logger). Mark Phase 2 done and Phase 3 = mouse-probe migrated, awaiting the user test.

## Chunk A: adoption memory for all local Claude sessions (Opus)
- Write the memory file `~/.claude/projects/-home-ethansm-orca-projects-local-setup/memory/kotor2-kplog-adoption.md`, type `project`.
- **Content:**
  - "The new logger (`KpLog.h` on `kpatch.log`) is merged into fork `ethansm/local-mods` (commit …)."
  - "Every session that owns a mod: convert your mod's logging to KpLog.h in our local build, following `patch_manager_mods/19f_kplog_migration_guide.md`. Mouse-probe is the reference."
  - The rules:
    - keep tag text;
    - legacy flag files map to the fallback only;
    - no FrameTick unless the hook is frame-global;
    - update any `^`-anchored parser;
    - do not break the live install: the live core may be old, and fallback must keep your current file logging working.
  - "**Whichever mod ships first with the logger is when the logger goes up for its upstream PR.** That session tells the user and points to `19g_upstream_pr_draft.md`; never post without the user's go."
  - "Record your migration in your mod doc's 'Planned shared state and shared code' section, and in 19e's adoption table."
  - **Why:** the user wants one framework standard (10-02/10-03).
  - **How to apply:** before touching your mod's logging, read 19f.
- Add a `MEMORY.md` index line: `- [KOTOR2 kplog adoption](kotor2-kplog-adoption.md) — all sessions: convert your mod to KpLog.h (guide 19f); first mod shipping with it triggers the logger's upstream PR`.
- Update `kotor2-shared-logging.md`: Phase 2 status, the branch, the PR trigger rule.
- Add an adoption table to 19e (mod → owner session → status), initially mouse-probe = migrated/staged and the rest = pending.
- Commit the 19c/19e updates.

## Step U: user test handoff (prepare only; no install without the go)
Write the install/rollback checklist into 19e:
1. Back up the live `KotorPatcher.dll` and `MouseProbe` artifacts.
2. Install the new core DLL from local-mods (`make -C src/KotorPatcher dll`) and the 0.2.0 mouse-probe through the usual KPM install path (see doc 10 install records / `kpm_install_state.json`).
3. Optional `kplog.ini` enabling `[mouse-probe] channels=*`.
4. The user plays briefly.
5. Expected files: `mouse-probe_log.txt` with header + channels. With no ini, only WARN lines.
6. Rollback commands.

Per memory: offer "deploy now with rollback" vs wait. Never launch the game.

## Order and gates
Step 0 → T (if multilib) → K → C → H → D → merge into local-mods (Opus) → M → P → R → A → U-prep → final gate + T re-check → update 19c "Next", memory, 19e log.

## Verification (end of phase)
- `W=$W scripts/kpm_registry_gate.sh` PASS: x86_64 rows + `any_kplog_header`, plus full i386 if multilib.
- `make kplog-check` OK under MinGW win32 + posix. One upstream `debugLog` patch rebuilds unchanged.
- MouseProbe 0.2.0 built, exports verified, staged.
- `kplog_parse` tests and `analyze_play_logs` tests green.
- Docs are committed in KPM (`feat/kplog-header`, merged to local-mods) and in local-setup (19e/19f/19g, 19/19a/19c updates).
- Adoption memory and MEMORY.md line exist.
- Nothing posted upstream, nothing installed live.

## Risks / watch items
- **MSVC (preprocessor, `/W4`, `fopen_s`) is untested locally.** The first Windows/CI build of mouse-probe or the PR CI covers it. Keep macros `__VA_ARGS__`-plain.
- **The `Common.h` edit touches every upstream patch.** It is behaviour-neutral (ODS when no hook) but every patch needs a rebuild to pick it up. `Common.lib` is unaffected (inline).
- **The mouse-probe log changes.** Its file name changes to `mouse-probe_log.txt` on the new core, and the timestamp becomes the core `T=` prefix. Doc 18 notes need updating.
- **Merging into `local-mods`, which siblings also commit to.** Use a merge commit (no rebase) and verify sibling commits are intact afterwards.
- **High-rate CAM/RAW lines** can approach the 2000 lines/s per-patch limit. Look for `DROPPED` lines in the user test.

## Execution log

- C180 (done): verified

- Step 0 (10-03): plan copied; doc 19 status/Phase-1 row corrected (merged fa67a3b, not live); gate gets `W=` override. Multilib present (`/usr/lib/gcc/x86_64-linux-gnu/13/32`), so chunk T runs first.
- Chunk T (10-03): gate already "GATE PASS (full, multilib)" at baseline (15 i386 + 14 x86_64 rows, ELF32 `KotorPatcher.so`). Real 32-bit smoke (scratchpad `t32smoke/`: `gcc -m32` driver dlopens the .so; constructor init reads `patch_config.toml`/`kplog.ini` from the .so dir; DLL_ONLY 32-bit provider+logger fixtures; `CleanupPatcher` then `_exit`) wrote `logger-fixture_log.txt` with header (interfaces kpatch.log + test.counter) + `main WARN hello from KPatchInit` + `main info line`. No core diff. Re-run by Opus: identical.
- Chunk K (10-03): KpLog.h (462 l) + KpDebugHook.h (63 l) committed on feat/kplog-header (003042b). Compile-clean g++ -m64/-m32, MinGW win32/posix, windows.h before/after; 0 UNIQUE syms, only KPatchInit exported; host fallback + fake-api smoke OK (scratchpad kcheck/). Opus change: MSVC fallback open uses _fsopen(_SH_DENYNO) not fopen_s (sharing). Known: a channel that gets -1 is dead for good; re-Init with a different api keeps cached ids. MSVC/clang uncompiled.
- Chunk C (10-03, 31ac422): Common.h debugLog/debugHexDump -> kp_debug_emit (+<cstring>, opt-in + concatenation caveat comment). tests/compile/kplog_common{,_rev}.cpp compile with MinGW win32 + posix (Common.h is MinGW-only; no host build). ModOptions via create-patch: builds, import table identical to HEAD-header build (166 entries), +512 bytes.
- Chunk H (10-03, 3ec8730): any_kplog_header 159 checks/width (12 scenarios), gate GATE PASS (full, multilib) 16 i386 + 15 x86_64 rows (Opus re-ran), 20x loop clean both widths, TSan 5/5 clean, kplog-check OK; mutation spot-checks (visibility pragma, EveryGate, struct_size, [...]) each fail the expected scenario. Finding: core truncates lines at 232 bytes (header cap 511) — fine, documented in test 11a.
- Chunk D (10-03, 1eeb133): PatchRegistry.md 'Logging with KpLog.h' (+92), core README pointer, tests README (+79); 19f migration guide (97 l) in local-setup. analyze_play_logs ^-anchored regexes actually at 125-151 (ai_trace), 160-168 (ranged), 171-180 (inventory), 1086+ (PR_*).
- Merge (10-03): feat/kplog-header -> ethansm/local-mods as **fc11fa3** (merge commit, no rebase; parent 63a9239 = sibling tip, intact). Resolved in a temp worktree, then --ff-only in the fork clone (its dirty sibling files do not overlap). Only conflict: docs/PatchRegistry.md (local-mods 'Library patches' + our section replacing 'Not yet available'; both kept). Gate on the merged tree (first attempt 17a4414, identical for our paths): GATE PASS (full, multilib) + kplog-check OK.
- Chunk M (10-03): mouse-probe 0.2.0 on KpLog.h (channels main/raw/polls/norm/drag/cam, tags unchanged, STUCKDRAG? WARN, flag file -> fallback level on old core only, fallback file mouseprobe_log.txt, no FrameTick). Opus review found the MinGW DLL at 181 KB with SetSystemTime/GetSystemTimeAdjustment imports from std::chrono -> KpLog.h NowMs uses GetTickCount on Windows (KPM 22add9a, gate PASS, merged local-mods 706b150). Rebuilt: 89 KB (0.1.0 was 80 KB), exports KPatchInit/ProbeCameraFrame/ProbeMouseNorm/ProbeMouseRaw, strings mouse-probe/kpatch.log/mouseprobe_log.txt. Staged ~/Games/KOTOR2_Mod_Workspace/stage_mouseprobe_020/MouseProbe.kpatch sha256 483eaec8...d102. Fork source committed ceeca81 (Patches/MouseProbe was already tracked since c4c5715). Not installed.
- Chunk R (10-03, 434a32b): `19g_upstream_pr_draft.md` (HOLD; stack e36afa4..22add9a, rebase onto upstream/master needed: 10 commits ahead, tip 3f8b858), 19a refreshed (KpLog.h exists, opt-in debugLog, fallback sink instead of pre-init buffer, ships with the first mod), doc 19 Phase 2 DONE / Phase 3 status / 8a PR trigger rule.
- Chunk A (10-03): memory `kotor2-kplog-adoption.md` + MEMORY.md line; `kotor2-shared-logging.md` Phase 2 status. **Correction to this plan's Context:** the live `KotorPatcher.dll` is **`36f10b7f` = the registry core** (installed 10-03 11:44 by the portrait-slots session, restored 16:44 per doc 17/21) and already provides `kpatch.log`; live `kplog.ini` has `[global] enabled=1 sink=perpatch` + `[portrait-slots]`. Phase 2 changed no core source, so no core reinstall is needed for mouse-probe.

## Adoption table (KpLog.h migration per mod)
| Mod | Owner session | Status |
|---|---|---|
| mouse-probe | shared-logging (this) | **migrated 0.2.0, LIVE 10-03 21:02, awaiting user test** |
| portrait-slots | buff/shield split | pending (already uses the registry for `k2.portraitslots`) |
| buff-duration-hud | buff HUD | pending (Phase 3 next per doc 19) |
| shield-bar | shield bar | pending |
| action-hotbar (renamed from force-power-hotbar 10-04) | hotbar | **migrated (7cdbe17), 1.2.0 LIVE 10-04 12:24 (d2f1a37e, id action-hotbar), level=debug, awaiting user test** |
| level-up-override | level-up | pending |
| perf-lab, robustness-fixes | engine research / engine-bug | pending |
| k2-dialogue-letterbox-fix (+ probe variant) | engine-bug owns behaviour; shared-logging does the KpLog migration (19h) | **migrated 1.0.2 LIVE 10-03 22:42 (7b3936bc, backup pre_lb102_1003_2242), NOT VALIDATED: 17-row manual checklist in doc 17 "Manual validation checklist (1.0.2)"**; 1.0.2-probe staged (local-setup 2823ff6/aea74cb, fork b4e1db6) |
| ranged-ai | C5b (doc 24) | **migrated 0.4.0 (local-setup 1ec207f, fork 98d53f6), NOT installed**; fallback = `ranged_ai_log.txt` at INFO; core `ranged-ai_log.txt`; analyze_play_logs reads legacy, fallback and core |
| companion-ai | doc 24 / fix pipeline | **migrated 0.2.1-probe (2026-10-15), NOT installed**; fallback = `ai_trace.txt` at INFO; core `companion-ai_log.txt`; channels main/trace/probe; analyze_play_logs reads legacy, fallback and core (long lines are split into `t=N +` pieces under the core 232-byte cut and re-joined by the parser); new `KPatchInit` export => install via `kpm_reinstall.py` |
| load-hang-probe | probe | **migrated 0.1.5 (2026-10-16), NOT installed (NOT-LIVE probe, inert unless `hangprobe_debug.txt`)**; fallback = `hangprobe_log.txt` at INFO; core `load-hang-probe_log.txt`; channel main; `hangprobe_report.txt` unchanged (dedicated dump); pre-`KpLog_Init` DllMain lines queued and flushed by `KPatchInit` (old core: watchdog after <= 1.5 s); long lines split into `+ ` pieces under the 232-byte cut; new `KPatchInit` export => install via `kpm_reinstall.py` |
| k1-spawn-probe | probe (doc 38) | **migrated 0.3.3-probe (2026-10-16), NOT installed over LIVE 0.3.2**; fallback = `k1-spawn-probe_log.txt` at INFO; core same name; channel main; ADDTOAREA lines split into `+ ` pieces; install = DLL hot-swap + kplog.ini section |
| k2-texture-memory-release, k2-activation-fix, k2-equip-icon-fix | C5a (doc 24) | **migrated 1.1.0 (local-setup ed52d2b, fork 34c0785), NOT installed**; fallback = old log names; core `<id>_log.txt` read by analyze_runs |
| char-effects-panel, equip/inv3d | their sessions | pending |
| inventory-sort-filter (C180) | WI-18 | **migrated, DONE 9026162**: `logf` forwards to KpLog fallback (`inventory_sort_filter_log.txt`, INFO); build_check + contract test green; NOT installed (lines gain `<ms> <id>/main ` prefix; analyze_play_logs reader fix = WI-72 first); 0.2.1 (2026-10-16) drops the attach truncate; sessions split on the loaded line |

## Step U: user test hand-off (prepared, NOT done; needs the user's go; game closed; never launch it)
G="$HOME/.local/share/Steam/steamapps/common/Knights of the Old Republic II"; S=~/Games/KOTOR2_Mod_Workspace/stage_mouseprobe_020; B=~/Games/KOTOR2_Mod_Workspace/backups/mouseprobe_020_$(date +%Y%m%d_%H%M)
1. Check: game not running (`pgrep -f swkotor2.exe` empty); live core md5 `36f10b7f…` (has kpatch.log); live `patches/mouse-probe.dll` md5 `548d4237…` (0.1.0).
2. Backup: `mkdir -p $B && cp "$G/patches/mouse-probe.dll" "$G/kplog.ini" "$G/patch_config.toml" "$G/KotorPatcher.dll" $B/`.
3. Install (hot-swap, hooks unchanged from 0.1.0, same as the L1 1.0.1 precedent in doc 17): `unzip -p $S/MouseProbe.kpatch binaries/windows_x86.dll > "$G/patches/mouse-probe.dll"`; verify md5 = the staged DLL. (Alternative: full KPM reinstall per the CLAUDE.md "KPM Install Rule" with all live ids; source is committed at fork ceeca81.)
4. Optional channels: append to `$G/kplog.ini`: `[mouse-probe]` / `enabled=1` / `level=info` / `channels=*`, and under `[global]` consider `max_lines_per_sec=0` (RAW/CAM can exceed 2000/s; leave it and look for DROPPED first).
5. User plays a few minutes in-world with right-drag camera moves.
6. Expect: `$G/mouse-probe_log.txt` with the `# kplog session` header; with the ini: `mouse-probe/main START mouseprobe 0.2.0`, `raw`/`polls`/`norm`/`drag`/`cam` lines with the old tags; without the ini: only `STUCKDRAG?` WARN lines (if any). `mouseprobe_log.txt` must NOT grow (proves the new-core path). Read with `python3 scripts/kplog_parse.py --stats "$G/mouse-probe_log.txt"`; check for `DROPPED`.
7. Rollback: `cp $B/mouse-probe.dll "$G/patches/" && cp $B/kplog.ini "$G/"`.
- Chunk P (10-03, 1ec9833): scripts/kplog_parse.py (+40 tests, testdata/kplog real + hand-written samples); analyze_play_logs tests 53/53 still green. Final: gate GATE PASS (full, multilib) after the last KpLog.h change (22add9a); kplog-check OK; chunk T needed no re-check (multilib present all phase). **Phase 2 DONE 10-03.** Next = step U (user go).
- **Step U INSTALLED 10-03 21:02 (user go: "install")**: game closed; live core 36f10b7f unchanged; `patches/mouse-probe.dll` 548d4237 (0.1.0) -> **c4fa4f8f** (0.2.0, = staged kpatch DLL); `kplog.ini` += `[mouse-probe] enabled=1 level=info channels=*`; config hooks ProbeMouseRaw/Norm/CameraFrame all exported (+KPatchInit). Backup `/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/mouseprobe_020_20261003_2102` (dll, kplog.ini, config, core). Rollback: `cp /home/ethansm/Games/KOTOR2_Mod_Workspace/backups/mouseprobe_020_20261003_2102/mouse-probe.dll "$G/patches/" && cp /home/ethansm/Games/KOTOR2_Mod_Workspace/backups/mouseprobe_020_20261003_2102/kplog.ini "$G/"`. Awaiting user play test -> expect `mouse-probe_log.txt`; `mouseprobe_log.txt` must not grow (last write 10-03 01:41).

## RESUME HERE (10-03 22:20)
- Next phase (user-routed via the vanilla-exe session): fold the dialogue-letterbox probe into KpLog.h per `19h_letterbox_probe_kplog_handoff.md` (5da68a7): fix 1.0.2 + probe variant 1.0.2-probe from one source, analyzer update, T1 path kept installable. Plan is made in plan mode from `.claude/session_handoff.md`.
- Still open: mouse-probe 0.2.0 user play test (read `mouse-probe_log.txt` with `kplog_parse.py --stats`).

## Phase 3 (letterbox): k2-dialogue-letterbox-fix 1.0.2 + 1.0.2-probe on KpLog.h (19h), started 10-03

Plan (user-approved): one source `dialogue_letterbox_fix_src/K2DialogueLetterboxFix.cpp`. The probe code sits under `#ifdef DLB_PROBE`, switched on by an optional `dlb_variant.h` (`__has_include`), so create-patch stays unedited. Probe variant files live in `dialogue_letterbox_fix_src/probe/` (variant header, manifest 1.0.2-probe, 3+5 hooks toml, exports.def, `build_probe.sh`). `build_probe.sh` builds in a temporary `Patches/` dir, removed on exit, into `KPM/letterbox_probe_stage/1.0.2-probe/`.
- Channels: `main` (load, APPLY, LATE), `set` (SETTOP/SETBOTTOM), `probe` (W1/W2/W3/W3b/W5), `probe_w4` (W4). Problem lines go out as WARN/ERR.
- Switches: `log=` stays the content gate (1.0.1 semantics), `probe=` stays the first-instruction gate, `enabled=`/`mode=` unchanged.
- Old core: fallback file `dialogue_letterbox_fix_log.txt` at INFO.
- The load line is deferred out of DllMain (emitted by KPatchInit, or by the first hook on an old core).
- Dropped: own fopen, `[GetTickCount]` prefix, the 5000 cap. Kept: the SET cap of 400, and the probe limiter (`n=`/`skip=`).
- PROBE W3 is split into W3 + W3b (same `n=`) to stay under 232 bytes.
- The analyzer accepts the legacy, core and fallback prefixes and joins W3b into W3.
- Tasks: 0 baseline (fork `3a86d89` = 1.0.1 committed), 1 source, 2 smoke (+ core stub mode), 3 builds + DLL checks, 4 analyzer + fixtures, 5 review/commits/docs, 6 install checklist (prepare only).

### Phase 3 result (10-03)
- Built: shipping `stage_lb_102/K2DialogueLetterboxFix.kpatch` (DLL `7b3936bc`, 55296 B, KPatchInit + 3) and probe `KPM/letterbox_probe_stage/1.0.2-probe/K2DialogueLetterboxFixProbe.kpatch` (DLL `969fe458`, 67072 B, KPatchInit + 8). 1.0.1 was 47616 B. Imports are KERNEL32 + msvcrt only.
- Gates: Wine smoke passes (shipping 11 modes, probe 14), longest message 182 B, check_engfix_windows ALL OK, validate-patches 0 errors, analyzer 88 tests, kplog_parse and analyze_play_logs green, kpm_registry_gate PASS, raw-address FAIL=0.

### Task 6: install checklist (prepare only; needs the user's go; game closed; never launch the game)
1. Re-md5 the core: `md5sum "$G/KotorPatcher.dll"` must be `36f10b7f…`. Re-md5 `$G/patches/k2-dialogue-letterbox-fix.dll` (expect `cf45a03a…`). Check `$G/dialogue_letterbox_fix.ini`.
2. Backup: `B=~/Games/KOTOR2_Mod_Workspace/backups/pre_lb102_$(date +%m%d_%H%M); mkdir -p $B; cp -a "$G"/{patch_config.toml,kpm_install_state.json,kplog.ini,KotorPatcher.dll,dialogue_letterbox_fix.ini} "$G/patches" $B/`.
3. Fork is committed (`b4e1db6`). Run `stage_and_verify_install.py stage <dir>`, check the `<-` path picks `stage_lb_102` (or the probe kpatch for T1), and record the STAGE_INFO `head=` in doc 17. Then install with `KPATCH_NO_LAUNCH=1 …/bin/NoLaunch/KPatchLauncher` plus all live ids. The shipping hooks are unchanged, so a DLL hot-swap (as on 10-03 16:44) is the lighter alternative.
4. Append to `$G/kplog.ini`: `[k2-dialogue-letterbox-fix]` / `enabled=1` / `level=info` / `channels=main,set` (probe variant: add `,probe,probe_w4`, and set `probe=1` in `dialogue_letterbox_fix.ini`).
5. After play: expect `$G/k2-dialogue-letterbox-fix_log.txt`. The old `dialogue_letterbox_fix_log.txt` must not grow. Grep for `DROPPED`, then run `analyze_engfix_logs.py` on the new file.
6. Rollback: `cp $B/patches/k2-dialogue-letterbox-fix.dll "$G/patches/"` (plus `patch_config.toml` if the probe was installed) and remove the kplog.ini section.

- **Installed 10-03 22:42** (user "install"): the shipping DLL was hot-swapped and the kplog.ini section added (see the doc 17 top line). Next: the user plays one conversation, then check `k2-dialogue-letterbox-fix_log.txt` and run the analyzer.
