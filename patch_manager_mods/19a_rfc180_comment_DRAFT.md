# DRAFT comment for LaneDibello/Kotor-Patch-Manager#180 (NOT POSTED)

Status: draft, 2026-10-02, refreshed 10-03 after Phase 2 (`KpLog.h` exists; PR draft `19g_upstream_pr_draft.md`). **Hold**: post only together with the PR that implements the #180 registry with
logging as its first service (user decision 10-02), which goes up with the first of our mods that ships with the logger (user rule 10-03). Before posting: fill in the PR number, re-check #180 for
new comments or a competing implementation, and update the numbers below from Phase 3 perf-lab results.
Design background: `19_shared_logging_design.md` (rev 2).

---

Picking up open question 2: **yes, and we think logging is the best first service for this registry.** PR #___ implements the registry as proposed here, with a patcher-provided `"kpatch.log"` interface as its first consumer, and it ships together with the first mod that logs through it (mod: ___).

**Why logging first.** Across the 16 patch DLLs we've written that log, we found five different timestamp formats, about ten DLLs that open and close their file on every line (expensive under Wine), caps and rotation that differ per patch, and around twelve hand-copied log helpers. Every one of those problems comes from each patch owning its own logging, and a patcher-provided service fixes them in one place:
- One clock, so lines from different patches can be merged and compared.
- One flush thread and one set of file handles for the whole process. Patches only format into a buffer, so game threads never wait on file I/O.
- One `kplog.ini` next to `patch_config.toml`, hot-reloaded, with a per-patch switch and per-channel switches. Logging is off by default except for warnings and errors.
- `debugLog()` / `debugHexDump()` in `Common.h` keep their signatures. A patch that opts in (includes `KpLog.h` and initialises it) gets them on a `debug` channel with the common prefix and ini control. Every other patch still goes to `OutputDebugString`, byte for byte as today.

**What the PR contains**
- `KPatchApi` exactly as sketched above (`struct_size`, `abi_version`, `provide`, `require`), pushed through an optional `KPatchInit` export after `LoadModule`. Patches without `KPatchInit` are untouched.
- The full patch-to-patch registry, not only patcher services: any patch can `provide` an interface and any other can `require` it. The patcher's own services go through the same `provide` path, so they get no special case. Rules the PR defines and tests:
  - `KPatchInit` runs once per module, at load: right after `LoadModule` and before the hook symbol lookup, and even if a later hook of that patch fails. Today `LoadModule` runs once per hook entry, so a 20-hook DLL is loaded 20 times with the same handle.
  - Interfaces stay valid until shutdown. Modules already stay loaded until `CleanupPatcher`.
  - A duplicate `provide` of the same name and major version is rejected and logged.
  - `require` returns null when no patch provides the interface, and callers must handle that.
  - The session log lists every interface and which patch provides it.
  - Tests use a provider and a consumer test patch.
- `"kpatch.log"` v1: a plain-C table, append-only: `struct_size`, `Register(patchId)`, `Channel(patch, name)`, `Enabled(channel, level)`, `Submit(channel, level, line, len)`, `Mark(label)`, `FrameTick(patch)` (the first patch to call it owns the frame counter), `Frame()`, `NowUs()`.
- `Patches/Common/KpLog.h`: macros (`KP_LOG_PATCH`, `KP_CHANNEL`, `KP_LOG(channel, fmt, ...)`, `KP_LOG_EVERY`, `KP_WARN`, `KP_ERR`, `KP_ENABLED`, `KP_MARK`) that `require` the interface in the patch's `KPatchInit`, plus a small fallback sink (a file, or `OutputDebugString`/stderr, WARN and up by default) when the patcher doesn't provide it (older patcher, or a patch tested on its own).
- Line format: `T=<ms since patcher init> f=<frame#|-> <patch>/<channel> <message>`. The message is the patch's own text, unchanged.
- Host tests for the registry (versioning, size growth, duplicates, re-entrant `provide` from `KPatchInit`, concurrency), for module init against real plain-C test modules (init once per handle, lazy `require` across load order, missing provider, abort midway), and for the log service (formatting, byte caps and rotation, rate limits, ring overflow, hot reload, the flush thread, bounded shutdown).
- The first mod that logs through it, as a worked example, and a "Logging with KpLog.h" section in `docs/PatchRegistry.md` for contributors.

**Our answers to the other open questions, from building this:**
1. *Naming/versioning:* string name + integer major version, with `struct_size` covering append-only growth within a major. `require(name, v)` returns the provider of major version `v`, or null; a breaking change is a new major, provided side by side.
3. *Lazy vs eager:* lazy `require` is enough. `KPatchInit` already serves as the eager point. One gap we hit: a patch's `DllMain` (and its static initialisers) run inside `LoadModule`, before `KPatchInit`. `KpLog.h` does not buffer them: until `KPatchInit` has run, a line goes to the patch's fallback sink (WARN and up). In practice only static-initialiser and `DllMain` lines take that path, because `KPatchInit` runs before any hook.
4. *Threading:* we'd rather not promise "everything on the game thread". Several patches log from their own threads, and the patcher already applies hooks on a worker thread when SteamStub defers it. We'd state instead that `provide`/`require` are thread-safe at any time, and that each interface documents its own rules (`kpatch.log`'s `Submit` is callable from any thread and never blocks). `KPatchInit` itself is DllMain-class: on Windows it runs under the loader lock, and it may run on the SteamStub worker. It may store the api pointer and call `provide`/`require` and cheap registration calls, but must not load libraries, wait on threads or do file I/O. The patcher never holds the registry lock while calling it, so `provide` from inside `KPatchInit` is fine.
- *MSVC vs MinGW:* every function pointer in the tables is explicit `__cdecl`, there are no C++ types across the boundary, and the service copies strings it keeps. That keeps patches built with MSVC and MinGW compatible in the same process.
- *Linux/macOS:* the registry and the log service use only C++17 std (clock, threads, `FILE*`) plus the existing `Platform::LoadModule`/`GetSymbol`, so they work unchanged in `KotorPatcher.so`/`.dylib`. The patcher adds no symbol API: patches reach the registry only through `KPatchInit`. `KpLog.h`'s fast path uses no Windows APIs.

5 (Mod Options) is yours to shape. If it helps, the Mod Options handler can be the second interface on the same registry.

### GOG support callout (required text, rule of 2026-10-03)
Every issue, PR or RFC #180 comment that mentions the shared libraries (portrait-slots, `k2.portraitslots`) or GOG support for portrait-slots / buff-duration-hud / shield-bar must say plainly:

> GOG support is **static-verified only** (no GOG install here: hook bytes and addresses checked against the GOG exe in Ghidra, see doc 21b). **GOG contributor testing is required** before it is considered supported.

What a GOG tester should check and send:
1. Install through KPM: portrait-slots shows under "Shared libraries" and is auto-added; `patch_config.toml` lists it first; the patcher log has every hook `Applied`.
2. HUD: buff/debuff icon columns with draining durations on the party portraits; shield bar beside the health bar when a shield is up.
3. In-game menu: companion portrait gauges/bars and buff icon columns on every menu tab; nothing drawn outside the panels.
4. Portrait hover tooltip shows `Shield: cur/max` while shielded.
5. Logs: `kpatch.log`, `buffhud_log.txt`, `shieldprobe_log.txt`, the patcher log, plus a screenshot of HUD and one menu tab.
