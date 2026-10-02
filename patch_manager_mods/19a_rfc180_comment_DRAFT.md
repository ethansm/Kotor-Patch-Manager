# DRAFT comment for LaneDibello/Kotor-Patch-Manager#180 (NOT POSTED)

Status: draft, 2026-10-02. **Hold**: post only together with the PR that implements the #180 registry with
logging as its first service (user decision 10-02). Before posting: fill in the PR number, re-check #180 for
new comments or a competing implementation, and update the numbers below from Phase 3 perf-lab results.
Design background: `19_shared_logging_design.md` (rev 2).

---

Picking up open question 2: **yes, and we think logging is the best first service for this registry.** PR #___ implements the registry as proposed here, with a patcher-provided `"kpatch.log"` interface as its first consumer and two patches migrated onto it.

**Why logging first.** Across the 16 patch DLLs we've written that log, we found five different timestamp formats, about ten DLLs that open and close their file on every line (expensive under Wine), caps and rotation that differ per patch, and around twelve hand-copied log helpers. Every one of those problems comes from each patch owning its own logging, and a patcher-provided service fixes them in one place:
- One clock, so lines from different patches can be merged and compared.
- One flush thread and one set of file handles for the whole process. Patches only format into a buffer, so game threads never wait on file I/O.
- One `kplog.ini` next to `patch_config.toml`, hot-reloaded, with a per-patch switch and per-channel switches. Logging is off by default except for warnings and errors.
- `debugLog()` / `debugHexDump()` in `Common.h` keep their signatures and are routed through the service, so existing patches get the common prefix and ini control without source changes. By default they still go to `OutputDebugString`.

**What the PR contains**
- `KPatchApi` exactly as sketched above (`struct_size`, `abi_version`, `provide`, `require`), pushed through an optional `KPatchInit` export after `LoadModule`. Patches without `KPatchInit` are untouched.
- `"kpatch.log"` v1: a plain-C table (`struct_size`, `Register`, `Enabled`, `Submit`, `FrameTick`, `Mark`, ...), append-only.
- `Patches/Common/KpLog.h`: macros (`KP_LOG(channel, fmt, ...)`, `KP_LOG_EVERY`, `KP_WARN`, `KP_ENABLED`) that `require` the interface in `KPatchInit`, plus a fallback to `debugLog()` when the patcher doesn't provide it (older patcher, or a patch tested on its own).
- Line format: `T=<ms since patcher init> f=<frame#|-> <patch>/<channel> <message>`. The message is the patch's own text, unchanged.
- Tests for the registry (versioning, size growth, missing `KPatchInit`) and for the log service (formatting, byte caps and rotation, rate limits, ring overflow).
- Two migrated patches as worked examples, and `docs/Logging.md` for contributors.

**Our answers to the other open questions, from building this:**
1. *Naming/versioning:* string name + integer major version, with `struct_size` covering append-only growth within a major. `require(name, v)` returns the provider of major version `v`, or null; a breaking change is a new major, provided side by side.
3. *Lazy vs eager:* lazy `require` is enough. `KPatchInit` already serves as the eager point. One gap we hit: a patch's `DllMain` (and its static initialisers) run inside `LoadModule`, before `KPatchInit`. `KpLog.h` holds lines logged that early in a small static buffer and hands them over when `KPatchInit` arrives.
4. *Threading:* we'd rather not promise "everything on the game thread". Several patches log from their own threads, and the patcher already applies hooks on a worker thread when SteamStub defers it. We'd state instead that `provide`/`require` are thread-safe after init, and that each interface documents its own rules (`kpatch.log`'s `Submit` is callable from any thread and never blocks).
- *MSVC vs MinGW:* every function pointer in the tables is explicit `__cdecl`, there are no C++ types across the boundary, and the service copies strings it keeps. That keeps patches built with MSVC and MinGW compatible in the same process.
- *Linux:* the registry and the log service sit behind the existing `Platform::` seam (clock, files, thread), so they work in `KotorPatcher.so`/`.dylib` too. `KpLog.h`'s fast path uses no Windows APIs.

5 (Mod Options) is yours to shape. If it helps, the Mod Options handler can be the second interface on the same registry.
