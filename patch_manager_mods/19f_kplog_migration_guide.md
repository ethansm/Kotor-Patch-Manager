# 19f. KpLog.h migration guide for mod sessions (2026-10-03)

This is the checklist for converting one of our mods from its own log helper to `Patches/Common/KpLog.h`. It applies to every mod we build from the fork clone (`~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager`, branch `ethansm/local-mods`).

Reference implementation: **mouse-probe** (`patch_manager_mods/mouse_probe_src/`, chunk M of 19e). Copy its pattern. At the time of writing chunk M has not landed yet, so check that `mouse_probe_src/MouseProbe.cpp` contains `KpLog` before you rely on it.

Full API reference: `docs/PatchRegistry.md`, section "Logging with KpLog.h" (in the KPM repo, branch `feat/kplog-header`, merged into `ethansm/local-mods` after chunk D). Design: 19, 19e. Parser: chunk P (`scripts/kplog_parse.py`).

## Before you start

- **The live core may be old.** The live `KotorPatcher.dll` can still be the Sep-28 build, which has no `kpatch.log` and never calls `KPatchInit`. Then `KpLog_Init` never runs and every line goes to the header's fallback sink. A migrated mod must therefore still write the file it writes today. Point the fallback at your current log file name (point 4 below). Check which core is live before you test.
- **What changes in the log.** On a new core, lines go to `<patch-id>_log.txt` (for example `mouse-probe_log.txt`), and each line gets the core prefix `T=<ms> f=<frame> <patch>/<channel> `. Your own clock field is redundant. Keep it only if a parser reads it.

## Checklist (8 points)

1. **Include path.** In the fork clone build, use `#include "KpLog.h"` (the Windows build passes `-I Common`). For a native Linux/macOS build, use `#include "../Common/KpLog.h"`. Put `KP_LOG_PATCH("<manifest id>");` once, at global scope, in one `.cpp`. The id must match the manifest `id`, 1 to 63 characters from `[A-Za-z0-9_.-]`.

2. **`KPatchInit` in the `.cpp` and in a committed `exports.def`.** Write it out literally on one line. Do not use a macro, because `create-patch` finds exports by scanning the source text.

   ```cpp
   extern "C" void __cdecl KPatchInit(const KPatchApi* api) { KpLog_Init(api); }
   ```

   Add `KPatchInit` to the patch's `exports.def` and commit the file. `create-patch` writes an `exports.def` only when none exists, so an existing one will not get the new export on its own. If the patch already has a `KPatchInit`, add the `KpLog_Init(api)` call to it. `KpLog_Init` does no I/O, so it is safe there. Bump the manifest version.

3. **Map each helper's tags to channels. Keep the tag text.** Every tag your old helper wrote (`RAW`, `CAM`, `E1`, `FM4`) becomes a `KP_CHANNEL`, and the tag stays in the message text, so `grep` and the parsers still find it. Channel names: lowercase `[a-z0-9_]`, at most 12 characters, stable (users put them in `kplog.ini`). Move anything a person must see in a crash report to `KP_WARN` or `KP_ERR`: those need no ini. Keep the old field layout.

   Illustrative:

   ```cpp
   KP_CHANNEL(chRaw, "raw");
   KP_LOG(chRaw, "RAW dx=%d dy=%d", dx, dy);          // was: Log("RAW dx=%d dy=%d", ...)
   KP_WARN("STUCKDRAG? held=%d", ms);                 // visible without kplog.ini
   ```

   Guard expensive arguments and hot paths with `KP_ENABLED(ch, level)`, and use `KP_LOG_EVERY(ch, ms, ...)` for samples. The core drops anything above 2000 INFO-or-lower lines per second per patch.

4. **Legacy flag file: only on the old core, and it drives the fallback level.** If the mod used a flag file (for example `mouseprobe.txt`) to switch logging on, keep it, but run it only when `!KpLog_HasCore()`. On a new core `kplog.ini` is the switch. Also set the fallback file so an old core behaves like today.

   ```cpp
   KpLog_SetFallbackFile("mouseprobe_log.txt");        // today's file name (call early, e.g. in the first hook)
   // in the poll that used to read the flag file:
   if (!KpLog_HasCore()) {
       KpLog_SetFallbackLevel(flagFilePresent ? KPLOG_INFO : KPLOG_WARN);
   }
   ```

   Default fallback level is WARN, so without this step an old core would log nothing at INFO. The fallback file stops after 60000 lines (`--- log cap reached ---`). Static constructors and `DllMain` run before `KpLog_Init`, so they always use the fallback.

5. **Behavioural kill switches stay.** A flag that disables a behaviour (a fix, an override, a hook body) is not a logging flag. Leave it alone. Only the flags that existed to turn logging on or off are replaced.

6. **Parser impact.** The log lines now start with the core prefix. A parser that matches `^t=...` will not match. For example, the `ai_trace` and `ranged_ai` regexes in `patch_manager_mods/mods678_research/analyze_play_logs.py` (the `ai_trace` ones are `PA_RE` to `REPEAT_RE`, lines 125-151; the ranged-ai `R_*` ones are lines 160-168; the inventory ones are 171-180; the `PR_*`/`PRS_*` ones start at line 1086) are all `^`-anchored. Do one of these in the same change as the migration:
   - update the regexes to allow the prefix; or
   - strip it first with `strip_prefix` from `scripts/kplog_parse.py` (being written in chunk P), which returns the bare message.

   Run `mods678_research/test_analyze_play_logs.py` afterwards. Remember that on an old core the fallback line is `<ms> <patch-id>/<channel> text` with no `T=`/`f=`, so a parser must handle both shapes while both cores are in use.

7. **Never call `KP_FRAME_TICK()` from a hook that is not frame-global.** The first patch to tick owns the shared `f=` counter, and a second ticker would double it. A hook that runs only in some places (in-world only, one screen) would also take the counter away from a better ticker. If you are not sure, do not tick.

8. **Verify.**
   1. Build the patch (`create-patch.py`, MinGW). Check `objdump -p <dll>` lists `KPatchInit` among the exports.
   2. Run with no `kplog.ini`. Only WARN and ERR lines may appear, or no file at all.
   3. Run with a `kplog.ini` that enables the patch (below). The `<patch-id>_log.txt` file shows your channels, one per subsystem.
   4. Search for `DROPPED`. Any hit means the rate limit cut lines.
   5. Run your parser or analysis script on the new log.

## Sample kplog.ini

Put it in the game directory, next to the patcher DLL and `patch_config.toml`. It is optional, and the core re-reads it about once a second, so you can edit it while the game runs. Only these keys exist.

```ini
[global]
enabled=1              ; master switch for everything above WARN (default 0)
sink=perpatch          ; perpatch (default) | merged | both
flush_ms=500           ; default 500
max_bytes=8000000      ; per file, then rotated to .1
max_lines_per_sec=2000 ; per patch, INFO and below; 0 = unlimited

[mouse-probe]          ; the KP_LOG_PATCH id, case-sensitive
enabled=1
level=info             ; err | warn | info | debug | trace, or 0-4 (default info)
channels=*,-polls      ; every channel except "polls"; absent = all
```

- A patch logs above WARN only when both `[global] enabled` and its own `enabled` are on.
- `level` never goes below WARN.
- Files appear next to the patcher: `<patch-id>_log.txt` (perpatch), `kp_log.txt` (merged). The previous session's file is kept as `<name>.1`. Lines starting `# ` at the top are a session header.
- Where to find all the keys: `src/KotorPatcher/README.md`, "Configuration: kplog.ini".

## Pitfalls we already know

- A channel the core rejects (bad name, or the 256-channel limit across all patches) stays dead for the session. It is reported once in the fallback sink.
- On an old core, `KP_ENABLED` and `KP_LOG` follow the fallback level, not `kplog.ini`.
- The core cuts a line at 232 bytes and ends it with `[...]`. Put the fields a parser needs first.
- `debugLog()` is rerouted to the `debug` channel (DEBUG level) once `KpLog_Init` succeeds. Each call becomes its own line, so log one complete line per call. Text is also still sent to `OutputDebugStringA` on Windows.
- MSVC is untested for the header. The mod builds here use MinGW.
- Sibling reinstalls of the live core can swap it under you. Check the `KotorPatcher.dll` md5 before you read a test result.
