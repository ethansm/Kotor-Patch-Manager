<!-- Copy of the approved plan-mode plan (2026-10-02). Status/results are appended in the "Execution log" section at the bottom. -->

# Phase 1: RFC #180 patch registry + `kpatch.log` service in the KotorPatcher core

## Context
The user's goal is a framework-level logging standard for Kotor Patch Manager, so mods log compatibly and cheaply. We also want #180's patch-to-patch communication as a first-class feature.

Decisions so far (persisted in the local-setup repo, `patch_manager_mods/`):
- **19c**: session context. Read it first.
- **19**: design rev 2.
- **19a**: held RFC comment.
- **19b**: cross-patch survey.

Phase 0 is done: the shared headers live in the fork's `Patches/Common` (KPM aad0a14).

Phase 1 implements, in the core:
- the upstream RFC LaneDibello/Kotor-Patch-Manager#180 registry (`KPatchApi`, optional per-patch `KPatchInit`);
- the logging service, provided as interface `"kpatch.log"` v1;
- host tests.

Out of scope for Phase 1:
- the patch-side `KpLog.h`;
- the `debugLog()` reroute;
- migrations (Phase 2+);
- posting anything upstream. The #180 comment is held until the PR exists, and posting needs the user's go.

## Step 0: persist the plan and set up the branch (Opus)
1. Copy this plan to `/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/19d_phase1_plan.md` and commit it in local-setup (Co-Authored-By trailer).
2. **The PR branch must start from upstream, not from `ethansm/local-mods`.** That branch carries all our local patches.
   - KPM fork: `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager`.
   - Run `git fetch upstream`, then `git worktree add ../kpm-registry -b feat/patch-registry-logging upstream/master`.
   - All Phase 1 work happens in `~/Games/KOTOR2_Mod_Workspace/tools/kpm-registry` (call it `$W`). Never touch the untracked `Patches/MouseProbe/` in the main clone.
3. Baseline check in `$W`: `make -C src/KotorPatcher all && make -C src/KotorPatcher test` must pass before any change. Record the result.
4. Check the toolchain: `i686-w64-mingw32-g++-posix` must exist and gcc must support `-m32` (the tests already use it).

## Decisions adopted from the design review (defaults; the user may override at approval)
- **ABI header**:
  - Canonical copy at `Patches/Common/kpatch_api.h`. It is pure C: `extern "C"`, `<stdint.h>`, no `windows.h`.
  - A one-line forwarder `src/KotorPatcher/include/kpatch_api.h` (`#include "../../../Patches/Common/kpatch_api.h"`).
  - Do **not** add `Patches/Common` to the core's `-I`: its `Platform.h` collides case-insensitively with the core's `platform.h`.
- **ABI hygiene**:
  - Only `uint32_t`/`int32_t`/`uint64_t`/`const char*`/function pointers. No bool, enum or double. Levels are `#define`s (`KPLOG_ERR 0, WARN 1, INFO 2, DEBUG 3, TRACE 4`).
  - `uint32_t struct_size` is the first field of every interface.
  - Wrap the structs in `#pragma pack(push, 8)`, because Common.h opens `pack(push,4)`.
  - Add `static_assert` on sizes and offsets (the C11 `_Static_assert` form inside `#ifdef __cplusplus`, or guarded).
  - `KPATCH_CALL` is `__cdecl` for MSVC and MinGW, `__attribute__((cdecl))` for `__i386__` GCC, and empty otherwise.
- **`KPatchApi`** follows #180 exactly: `struct_size, abi_version, provide(name, version, iface)`, which returns 0 on success, and `require(name, version)`.
  - The version is the major version. Growth within a major goes through `struct_size`.
  - A duplicate name+major is rejected and logged; the first provider wins.
- **`KPatchLogApi` v1**:
  - `struct_size`
  - `int32_t Register(const char* patchId)`: idempotent, returns -1 on bad input
  - `int32_t Channel(int32_t patch, const char* name)`: max 256 channels, -1 beyond that
  - `int32_t Enabled(int32_t channel, int32_t level)`
  - `void Submit(int32_t channel, int32_t level, const char* line, uint32_t len)`: non-blocking, copies the line
  - `void Mark(const char* label)`
  - `void FrameTick(int32_t patch)`: the first patch to call it owns the counter; calls from other patches are ignored
  - `uint32_t Frame(void)`
  - `uint64_t NowUs(void)`
  - These replace the doc's `NowMs()` and `FrameTick()` and must be updated in docs 19/19a (step F).
- **`KPatchInit`**:
  - Called at load, once per distinct module handle, after `LoadModule` and before `GetSymbol(functionName)`.
  - **Never hold the registry mutex while calling it** (patches re-enter `provide`).
  - Provider attribution uses a `thread_local` "current patch id" set around the call.
  - It is a DllMain-class contract: it runs under the loader lock on Windows and may run on the SteamStub worker thread. It may store the api pointer and call `provide`/`Register`/`Channel`; it must not load libraries, wait on threads or do I/O.
- **Lifetime**:
  - Interfaces are valid until `CleanupPatcher` begins.
  - Cleanup order: `Registry::Close()` (`require` then returns null), unload modules, final log flush, `Registry::Reset()` (clears the init-once set, since a handle can be reused).
  - Patches must not call other patches' interfaces from DETACH.
- **Singletons**: the registry and log service are heap-allocated and intentionally never deleted, with an atomic `stopping` flag. This is safe against Linux `exit()` destructor order and against ExitProcess killing threads that hold locks. No `#ifdef`.
- **Flush thread**:
  - Started unconditionally right after `SelfModuleDir()` succeeds (patcher.cpp ~:96), before `ParseConfig`.
  - Sleeps on a condition variable with `wait_for` 500 ms, is notified on WARN/ERR, and polls the config every 1 s.
  - No start handshake. Never joined.
  - The final flush uses `try_lock` with about 200 ms bounded retry, then gives up.
- **Ring**:
  - `std::mutex` with a memcpy-only critical section, 4096 fixed records of about 256 bytes, heap-allocated at init.
  - The timestamp and frame are stamped at `Submit`; the flush thread does the formatting.
  - Drop-newest with per-patch drop counters, reported as `DROPPED n`.
  - The last 25% of slots are reserved for WARN/ERR. WARN/ERR are exempt from the rate limit.
  - `Enabled` reads `std::atomic<uint8_t> level[256]`. Effective level is the ini level if enabled, otherwise WARN.
- **Files**:
  - `<SelfModuleDir>/<patchId>_log.txt` (perpatch, the default) and/or `kp_log.txt` (merged), using `FILE*`.
  - At a session's first open, the previous file is rotated to `.1`, so the last session survives a crash.
  - Byte cap: on overflow, remove `.1`, rename to `.1` (Windows rename does not overwrite), reopen, and write a fresh header.
  - If a file can't be opened, log one `Platform::Log` line and drop.
- **Config**:
  - `kplog.ini` beside `patch_config.toml` (schema in doc 19 section 6, minus `debuglog=`, which is Phase 2).
  - Hot reload compares the file **content** every 1 s; mtime is unreliable under Wine and on 1 s filesystems.
  - A missing file means defaults (logging off except WARN/ERR). The installer needs no changes (FrameLimiter.ini precedent).
- **Line format**: `T=<ms since init, 3dp> f=<frame|-> <patch>/<channel> <message>`.
  - Session header: wall clock, `KOTOR_VERSION_SHA`, patcher version, and the provided interfaces with their providers.
  - On an `ApplyPatches` abort, log one summary line: "aborted: N modules initialised".
- **No new `Platform::` functions**: use `std::chrono::steady_clock`/`system_clock`, `std::thread`/`mutex`/`condition_variable` (MinGW posix model), and `FILE*`.
- The core's own `Platform::Log`/`KPATCH_LOG` behaviour is **unchanged**. Mirroring core logs into the service is deferred.
- `PatchInfo` gains `std::string patchId`, set at config_reader.cpp:157 (DLL_ONLY) and :183 (hook). If the id is empty, fall back to the dll basename.

## Delegated chunks (one Sonnet `general-purpose` agent at a time, `model: "sonnet"`)
Brief each agent with:
- the path to `19d_phase1_plan.md` and its chunk;
- `$W`;
- the allowed files;
- the acceptance commands;
- "do not commit; report a diff summary and the test output";
- core conventions: `namespace KotorPatcher`, `"[KotorPatcher] ..."` messages via snprintf + `Platform::Log`, `#pragma once`, 4-space indent, verbose why-comments, no `#ifdef`, test output must never start with `[`.

After each chunk, Opus re-runs the acceptance checks, reviews the diff and commits on `feat/patch-registry-logging`. Commit style: `patcher: ...` subject, a prose paragraph, then `- path: change` bullets, with the Co-Authored-By trailer. Before running tests, check `ps` for other build or test processes.

**A. API header + registry (no wiring).**
- Files:
  - `Patches/Common/kpatch_api.h`
  - `src/KotorPatcher/include/kpatch_api.h` (forwarder)
  - `include/registry.h`
  - `src/core/registry.cpp`
  - `tests/any_registry.cpp`
  - Makefile: add `registry.cpp` to `TEST_ENGINE` (:150/166), add `-pthread` to both test compile loops (:177/:183)
- `namespace KotorPatcher::Registry`: `Init()`, `Close()`, `Reset()`, `Provide`, `Require`, `OnModuleLoaded(void* handle, const char* patchId)` (uses `Platform::GetSymbol(h,"KPatchInit")`), `GetApi()`, `DumpProvided(std::string&)`.
- Accept:
  - `make -C src/KotorPatcher test` passes, and the any_registry rows of the matrix below are all present.
  - `printf '#include "kpatch_api.h"\nint main(void){return 0;}' | gcc -std=c11 -m32 -fsyntax-only -IPatches/Common -x c -` succeeds.
  - The same check with `i686-w64-mingw32-gcc-posix` succeeds.

**B. Module fixtures + init-once tests.**
- `tests/fixtures/provider.c`, `consumer.c`, `noinit.c`: plain C, which proves the header is C. They are built into `$(BUILD)/tests/fixtures/{32,64}/*.so` with `gcc [-m32] -shared -fPIC -std=c11`, with a Makefile step inside `test`.
- `tests/any_module_init.cpp` takes `-DFIXTURE_DIR`. Load modules through `Platform::LoadModule`.
- Accept: `make test` passes `any_module_init` at both widths, and `git status` shows no artifacts outside `build/`.

**C. Log service core (no thread, no integration).**
- Files: `include/log_service.h`, `src/core/log_service.cpp`, `tests/any_log_service.cpp`.
- Test seams: `Init(dir, Options{startThread=false})`, `FlushNow()`, `PollConfigNow()`, `SetClockForTest()`, `Shutdown()`.
- Contents: the ring, the patch and channel tables, the level table, the formatter, the sinks, cap and rotation, the rate limit, the drop counters, the ini parser with content-compare reload, the session header, and the `KPatchLogApi` filler.
- Accept: `make test` passes all the deterministic any_log_service rows.

**D. Flush thread + shutdown semantics.**
- Add the thread and condition variable, notify on WARN/ERR, the 1 s config poll, and the `try_lock`-bounded final flush.
- Add the threaded rows of the matrix.
- Accept: `make test`. If TSan is available, also run a local `-fsanitize=thread` build of any_log_service and any_registry, 64-bit only.

**E. Integration.**
- `patcher.cpp`:
  - Start the log service after `SelfModuleDir`.
  - `Registry::Init()` + `Provide("kpatch.log", 1, LogService::Api())` before `ApplyPatches`.
  - Factor out `LoadPatchModule(const PatchInfo&)`, used at :226 and :252. It does `LoadModule`, `push_back`, then `Registry::OnModuleLoaded(h, patchId)`.
  - Add the abort summary line, the session header after apply (including the deferred path), and the new cleanup order.
- `patcher.h`: `PatchInfo::patchId`.
- `config_reader.cpp`: set `patchId` at :157 and :183.
- `Makefile`: add to `CORE_SRCS`.
- `KotorPatcher.vcxproj` and `.vcxproj.filters`: ClCompile and ClInclude entries, including the forwarder.
- Accept:
  - `make -C src/KotorPatcher all` (MinGW DLL + Linux .so).
  - `make test`.
  - `nm -D --defined-only build/*.so` diffed against the baseline shows no intended new exports.
  - A host smoke run: a tiny 32-bit driver `dlopen`s `KotorPatcher.so` with a temp `patch_config.toml` listing a fixture DLL_ONLY patch, and `<id>_log.txt` / the session header appear when `kplog.ini enabled=1`. Put the driver under `tests/` only if cheap; otherwise use a scratchpad script.
  - msbuild/macOS can't be built here; CI covers them. Note this in the PR.

**F. Docs (Sonnet drafts, Opus edits).**
- `src/KotorPatcher/README.md`: a registry/`KPatchInit` contract section covering loader lock, threading, lifetime, ABI append-only rules, `KPATCH_CALL`, the RTLD_LOCAL/DT_NEEDED `dlsym` caveat, and that init happens at load even if hooks later fail.
- `tests/README.md`: the new test kinds and fixtures.
- `docs/PatchRegistry.md`: author-facing. It covers `extern "C" void __cdecl KPatchInit(const KPatchApi* api)`; note that the exports grep needs a one-token return type.
- `README.md` "Patches" section: a short pointer.
- In local-setup:
  - doc 19 section 10: correct "flush thread only when needed" to always-on.
  - docs 19/19a: `NowUs`, `FrameTick(patch)`, init-at-load, and the RFC Q4 answer.
  - Status log, doc 19c "Next", and memory `kotor2-shared-logging`.
  - Commit.

## Test matrix (all host tests, `make -C src/KotorPatcher test`, i386 + x86_64)

**any_registry**
- provide then require returns the same pointer.
- require of a missing name returns null.
- version mismatch returns null; a second major is provided side by side.
- a duplicate is rejected, the first pointer is still served, and a log line is emitted.
- null name or iface is rejected without a crash.
- struct_size growth (consumer checks the size before reading new fields).
- `KPatchApi` fields are populated.
- re-entrant provide inside init does not deadlock.
- 8 threads of concurrent provide/require.
- require after close returns null.
- the dump lists each provider.

**any_module_init**
- the same .so loaded 20 times gets init once.
- lazy require: the consumer is loaded before the provider and succeeds at first call.
- the consumer without a provider takes its fallback.
- a module with no KPatchInit returns false and logs no error.
- provider attribution is recorded.
- after reset, a reload re-inits.
- an abort midway leaves module A's interface valid until close and module B's null.

**any_log_service**
- line format, including `f=-` before any tick.
- frame owner.
- default levels.
- ini enabled/channels/`-x`/`*`/level.
- hot reload within the same second (content compare), and file removal returns to defaults.
- malformed ini lines are ignored.
- a channel registered after a reload gets the current level.
- Register is idempotent.
- the 257th channel returns -1.
- truncation marker, and null/0-length lines.
- bad ids are ignored.
- perpatch/merged/both sinks.
- byte cap rotation, including replacing an existing `.1` and writing a new header.
- launch rotation to `.1`.
- rate limit with `DROPPED n`, and WARN/ERR exemption.
- a 100k-submit ring overflow stays non-blocking and keeps the WARN/ERR reserve.
- 8×10k multithread submit: flushed + dropped = submitted, per-thread order kept.
- Mark appears in every file.
- the session header contents.
- an unwritable dir produces one Platform::Log line and no crash.
- shutdown flush.
- shutdown with the lock held returns within the bound.
- submit after shutdown is a no-op.
- a real-thread WARN reaches disk within about 1 s.

## Verification (end of Phase 1)
- `make -C src/KotorPatcher all test` is green in `$W`. The commit series on `feat/patch-registry-logging` is reviewed.
- The baseline run and the final run are both recorded in doc 19 status.
- **Optional live regression smoke, gated by the user.** No patch uses `KPatchInit` yet, so the behaviour must be identical.
  - Back up the live `KotorPatcher.dll` (its location is recorded in the install state, `kpm_install_state.json` / doc 10 install records).
  - Install the new DLL. Never launch the game ourselves; the user launches and plays briefly.
  - Expect no new files unless a WARN/ERR occurs or `kplog.ini` is created. Then roll back, or keep it at the user's choice.
- Then merge or cherry-pick the branch into `ethansm/local-mods` for local use. The upstream PR waits for Phase 2 (header + first migrated examples) and Phase 3 (perf numbers), per doc 19 section 8.

## Risks / watch items
- The MSVC/vcxproj path can't be built locally. Keep core code to C++17 std only and rely on CI.
- `-m32` fixtures need 32-bit glibc dev files. The existing tests already prove this; if missing, run fixtures 64-bit only and note it.
- Sibling sessions edit the main KPM clone and the local-setup repo. Work only in the `$W` worktree and in this plan's docs. Commit only our own paths in local-setup.


## Execution log

### Step 0 (10-02 13:56)
- Worktree `~/Games/KOTOR2_Mod_Workspace/tools/kpm-registry`, branch `feat/patch-registry-logging` from upstream/master 55725f7.
- Toolchain: `i686-w64-mingw32-g{cc,++}-posix` present. **`g++-multilib` is NOT installed** (no `/usr/lib/gcc/x86_64-linux-gnu/13/32`, sudo needs a password), so `-m32` fails on `bits/c++config.h`: the Linux `.so` and every i386 test row fail to compile **at baseline**. This is the environment, not the code.
- Baseline: `make dll` OK. `make test`: all 11 x86_64 rows pass; the i386 rows fail to compile (multilib).
- Gate until multilib is installed (`scripts/kpm_registry_gate.sh`): `make dll` green, every x86_64 row "all pass", and the only compile errors allowed are the multilib header ones. With multilib present the gate is full `make all test`. The header C check uses 64-bit gcc + i686 MinGW instead of `gcc -m32`.

### Chunks A + B (10-02 ~14:05)
- A committed KPM e36afa4: `kpatch_api.h` (canonical + forwarder), `Registry`, `any_registry` (56 checks, x86_64 PASS, 20x loop clean, TSan clean under `setarch -R`). Review fix: `Provide`'s closed check moved under the mutex, so a Provide racing Close() can't insert afterwards.
- B committed KPM b27b93d: C11 fixtures (provider/consumer/noinit) built per width into `build/tests/fixtures/{32,64}`, `any_module_init` (26 checks, all 7 plan rows, x86_64 PASS). Real dlclose verified by init counts.
- Gate widened: tolerates any missing `bits/*.h` or `gnu/*.h` under -m32 (C fixtures hit `gnu/stubs-32.h`).

### Chunk C decisions (brief, 10-02)
- Level marker: WARN/ERR lines carry `WARN ` / `ERR ` before the message; INFO and below carry none.
- Session header lines start with `# `. `SetSessionInfo` writes a `# session info` block to already-open files.
- Id rules: patch ids 1-63 chars `[A-Za-z0-9_.-]`, max 64 patches. Channel names 1-31 chars, max 256 channels total.
- `level=` takes a number 0-4 or a name; the default when enabled is info. `channels=` absent means all.
- Truncation marker: `[...]`.

### Chunks C, D, E (10-02 14:15-14:40)
- **C, committed f766bca:** the log service with 150 deterministic checks.
  - Review fix: `channels=` with only exclusions now means "all but these". The draft had made it select nothing.
  - Unspecified choices made by the agent:
    - An unconditional `_CRT_SECURE_NO_WARNINGS`.
    - `level=0` is clamped to WARN.
    - Ini keys are case-insensitive; ids and channel names are case-sensitive.
    - `Submit` checks `Enabled` itself and strips trailing CR/LF.
    - The ring is double-buffered and swapped at drain.
    - A byte-cap rotation needs body > 0.
- **D, committed 7e4b25f:** the flush thread.
  - It is detached and never joined, with a session generation guard. Lock order: poll -> flush -> ring -> table, with wake as a leaf lock.
  - Measured: WARN wake reaches disk in 10 ms, periodic INFO in 503 ms, hot reload in about 1005 ms. Shutdown with the ring lock held returns in about 202 ms.
  - The stale-thread churn row was mutation-checked: with the generation check removed, 92-99 of 100 cycles leaked. 172 checks; TSan clean, 3 runs.
- **E, committed 700e6c7, reworded to 7138548:** patcher wiring.
  - Review addition: a config parse failure now also leaves a WARN in `patcher_log.txt` and publishes session info.
  - DLL imports are unchanged (KERNEL32 + msvcrt). **Correction:** my first export check was void (the `sed` range matched nothing on either side). A real diff against a baseline DLL built from 55725f7: MinGW auto-exports every symbol when there is no `.def`/`dllexport`. Upstream already exports 706; the branch exports 742. The additions are the internal `Registry::`/`LogService::` functions plus a few std template instances. There is no intended ABI export; patches use `KPatchInit` only. Commit E's message was reworded to say this.
  - The 64-bit host smoke ran from a scratchpad driver. The `logger-fixture` file gets the header, the interface list (kpatch.log by patcher, test.counter by provider-fixture) and the WARN and INFO lines. Without the ini, only WARN is written and the previous session is kept in `.1`. A broken DLL gives "Apply aborted: 1 module(s) initialised" in `patcher_log.txt`. A garbage config gives the parse WARN.
  - Pre-existing, not a regression: `dlclose` of patch modules from the `.so` destructor during `exit()` segfaults in a dlopen'd harness. It reproduces at upstream HEAD too. In the game the `.so` is DT_NEEDED, so this is untested there. Also, `dlclose` of the patcher does not unmap it; static libstdc++ unique symbols are a likely cause.

### Chunk F + final (10-02 14:45)
- **F, committed 4fd1d12:** `docs/PatchRegistry.md` (author guide), `src/KotorPatcher/README.md` (registry/KPatchInit contract + kpatch.log sections) and `tests/README.md`, plus a root README pointer. Local-setup docs 19/19a/19c updated to as-built.
- **create-patch export scan:**
  - The scan uses regex `extern.*__cdecl` and takes the 5th token split on space and `(`. So `KPatchInit` must be written as `extern "C" void __cdecl KPatchInit(const KPatchApi* api)` on one line, with a literal `__cdecl` (not `KPATCH_CALL`), no `__declspec`, and a one-token return type.
  - A committed `exports.def` skips the scan entirely.
  - Linux and macOS patch builds get no `-I Patches/Common`, so they use `#include "../Common/kpatch_api.h"` and `#define __cdecl` off Windows.
- **Final gate (14:44):** GATE PASS. `make dll` builds and 14/14 x86_64 rows pass. The i386 rows and the Linux `.so` are still unbuildable here (no multilib).
- **Branch:** `feat/patch-registry-logging` = e36afa4, b27b93d, f766bca, 7e4b25f, 7138548, 4fd1d12, on top of upstream 55725f7. It is not pushed, not merged into `ethansm/local-mods`, and not installed live.
