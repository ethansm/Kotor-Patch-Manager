# 19g. Upstream PR draft: patch registry + kpatch.log + KpLog.h (NOT POSTED)

Status: draft, 2026-10-03. **HOLD: post nothing without the user's explicit go.**

**Trigger (user rule 10-03):** this PR goes up together with the **first of our mods that ships with the logger**. Whichever session ships first tells the user and points here. Post this PR and the RFC #180 comment (`19a_rfc180_comment_DRAFT.md`) together.

**Before posting**
1. Rebase the stack onto current `upstream/master` (base was 55725f7; upstream was 10 commits ahead on 10-03, tip 3f8b858). Stack, in order:
   - `feat/patch-registry-logging`: e36afa4..4fd1d12 (registry, `kpatch.log`, wiring, docs)
   - `feat/kplog-header`: 003042b..22add9a (`KpLog.h`, `debugLog` hook, tests, docs, Windows clock)
2. After the rebase, re-run `W=<rebased checkout>/src/KotorPatcher scripts/kpm_registry_gate.sh` (needs "GATE PASS (full, multilib)") and `make -C src/KotorPatcher kplog-check`.
3. Re-check #180 for new comments or a competing implementation (`python3 scripts/check_kpm_updates.py`).
4. Fill in the first-shipped-mod section below and the PR number in 19a.
5. Get the user's go, in chat, on the final text.

---

## Title

Patch registry (RFC #180) with a patcher-provided logging service and `KpLog.h`

## Body

### Summary

This implements the registry proposed in #180 and uses it for its first service, a shared logger.

- **Registry.** A patch may export `KPatchInit(const KPatchApi*)`. The patcher calls it once per module at load time. Through the api a patch can `provide` a named, versioned interface (a plain-C function table) and `require` one that another patch or the patcher provided. Patches without `KPatchInit` are untouched.
- **`kpatch.log` v1.** The patcher provides a logging service through the registry: one clock, one flush thread, per-patch files, `kplog.ini` switches per patch and channel (hot-reloaded), rate limits, and a session header listing every registered interface. WARN and ERR are recorded without any ini; everything else is off by default.
- **`Patches/Common/KpLog.h`.** A header-only layer patches include: `KP_LOG_PATCH`, `KP_CHANNEL`, `KP_LOG`, `KP_WARN`, `KP_ERR`, `KP_LOG_EVERY`, `KP_ENABLED`, `KP_MARK`. It works on an older patcher too, through a small fallback sink (a file, or `OutputDebugStringA`/stderr) gated at WARN.
- **`debugLog()` opt-in.** `Common.h`'s `debugLog`/`debugHexDump` now write through a hook. With no hook set they call `OutputDebugStringA` exactly as before. A patch that includes `KpLog.h` and initialises it gets its `debugLog` lines on a `debug` channel instead.

### Why

Across the 16 patch DLLs we have written that log, we found five timestamp formats, about ten DLLs that open and close their log file on every line (slow under Wine), caps and rotation that differ per patch, and about twelve hand-copied log helpers. All of that comes from every patch owning its logging. A patcher-provided service fixes it in one place: one clock so logs from different patches can be merged, file I/O off the game threads, and one ini to switch logging on and off. Logging is also a good first registry service, because every patch needs it and it has no patch-to-patch ordering problems.

### What is included

- `Patches/Common/kpatch_api.h`: the plain-C ABI (`KPatchApi`, `KPatchLogApi`, levels). Explicit `__cdecl`, no C++ types across the boundary, layout `static_assert`s, so MSVC and MinGW patches share one process safely.
- `src/KotorPatcher`: the registry, `KPatchInit` dispatch (once per module handle), the log service and its flush thread, a bounded flush at shutdown, and abort/parse-failure WARN lines in `patcher_log.txt`.
- `Patches/Common/KpLog.h`, `KpDebugHook.h`, and the `Common.h` hook.
- Host tests (Linux, i386 and x86_64): `any_registry`, `any_module_init`, `any_log_service`, `any_kplog_header` (12 scenarios over two C++ fixture patches, including two patches with the same channel names and an 8-thread first-use race), plus a `kplog-check` target that compiles the headers with MinGW and the host compiler.
- Docs: `docs/PatchRegistry.md` (registry, `KPatchInit` rules, the log service, "Logging with KpLog.h"), core and test READMEs.

### First mod shipping with the logger

_Placeholder: filled in when it happens._ Mod name, version, PR link, and what its log looks like on a new and an old patcher. The reference migration is our `mouse-probe` (local diagnostics, not proposed upstream).

### Testing

- `make -C src/KotorPatcher all test`: all rows green for i386 and x86_64 (multilib host). `any_kplog_header`: 159 checks per width; a 20x loop is clean, and ThreadSanitizer (64-bit) is clean.
- 32-bit host smoke: a `gcc -m32` driver `dlopen`s `KotorPatcher.so` with a provider and a logger test patch; the logger's file has the session header and its lines.
- `make dll` (MinGW) builds; DLL imports are unchanged (KERNEL32 + msvcrt).
- An existing patch that uses `debugLog` (ModOptions) rebuilds unchanged with the new `Common.h`: identical import table.
- Not covered locally: MSVC builds (CI covers them), macOS (CI), and Windows at run time beyond our own play testing.

### Known issues and notes

- Pre-existing, not from this PR: a harness that `dlopen`s `KotorPatcher.so` and lets `dlclose` run at process exit can crash in teardown. Calling `KotorPatcher::CleanupPatcher()` before exit avoids it; our test driver does that.
- MinGW auto-exports every non-hidden symbol of `KotorPatcher.dll` when there is no `.def`, as it did before this PR. The new internal `Registry::`/`LogService::` symbols therefore show up in the export table (742 vs 706). Nothing depends on them; patches reach the registry only through `KPatchInit`.
- The core cuts a line at 232 bytes and ends it with `[...]`. The header documents this.
- `KPatchInit` must be written literally in a `.cpp` and listed in `exports.def`: `create-patch` finds exports by scanning the source, and keeps a committed `exports.def` as is.

Closes nothing yet; implements #180 (registry) with logging as its first service.

🤖 Generated with [Claude Code](https://claude.com/claude-code)
