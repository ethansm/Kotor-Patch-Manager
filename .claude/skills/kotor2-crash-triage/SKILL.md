---
name: kotor2-crash-triage
description: >-
  Triage a KOTOR II (Steam/Proton, appid 208580) crash, freeze, or hang on
  this machine. Use when the user reports the game crashed, froze, went to a
  black screen, won't respond, or asks to check "the proton logs" / "the
  crash logs" / "the steam log". Locates the right log files, filters huge
  Wine trace output down to the actual signal, cross-references the modding
  pipeline's own manifests/backups when a texture or resource is suspected,
  and kills a hung game process safely without touching the rest of Steam.
  Also use for a winedbg backtrace, "unhandled exception", a crash address or
  swkotor2 offset to resolve, or "kill the game".
---

# KOTOR II crash triage

Full narrative history of past investigations (2DA corruption crash, capsule
revert bug, this skill's own origin case) lives in `CRASH_INVESTIGATION.md`
at the repo root — read it if the signature you find here doesn't match
anything below, and append a new dated section to it (don't just fix and
forget) once you've root-caused or ruled something out. `[[kotor2-ultrawide-gui]]`
and `[[kotor2-capsule-replay]]` memories have the current state of the mod
build if a deployed asset is a suspect.

## Step 1: is the process actually still running?

Check process state and detect spinning hangs via the triage CLI:
```bash
python3 scripts/crash_triage.py proc
```
- Emits `VERDICT: STOPPED` (process exited/crashed -> proceed to Step 2/3 for log analysis).
- Emits `VERDICT: HANG` (`swkotor2.exe` spinning at high CPU / black-screen hang -> proceed to Step 4 to kill).
- Emits `VERDICT: RUNNING` (`swkotor2.exe` is healthy/sleeping).

## Step 2: find the log

The **only** reliable way to get real crash output for this game (confirmed
across multiple sessions — see `CRASH_INVESTIGATION.md` for what doesn't
work: shell redirection of `%command%`, running wine directly bypassing
Steam):

- Steam launch options must include `PROTON_LOG=1 PROTON_LOG_DIR="$HOME"`.
  Current full launch line in use (check Steam → KOTOR II → Properties →
  Launch Options if this drifts):
  ```
  PULSE_LATENCY_MSEC=60 PROTON_LOG=1 PROTON_LOG_DIR="/home/ethansm" MANGOHUD_CONFIGFILE=/home/ethansm/.config/MangoHud/wine-swkotor2.conf gamemoderun taskset -c 0,6 mangohud %command%
  ```
  (`gamemoderun` is currently a silent no-op on this machine — missing
  `libgamemode.so` — harmless, not a bug to chase.)
- This writes `~/steam-208580.log`. Proton overwrites it fresh per launch.
- Two more logs live in the game install dir (`.../Knights of the Old
  Republic II/`), both **wall-clock timestamped** (unlike the Proton log,
  whose timestamps are seconds-since-boot):
  - `AR_ERROR.LOG` — Aurora engine's error log. Overwritten fresh every session.
  - `ReShade.log` — present only if ReShade is installed. Wall-clock anchor.

## Step 3: filter the log — don't read it whole

These logs run 3-30MB+. Never `Read` the whole file. Run the automated signal extractor:
```bash
python3 scripts/crash_triage.py log [--limit 40]
```
This isolates exceptions, signal dispatches, and backtrace frames, returning candidate crash offsets and a `VERDICT: FAULT(addr) | EXCEPTION | CLEAN`.

For KPM detours and patcher status:
```bash
python3 scripts/crash_triage.py kpm
```
Returns hook counts and patcher init status (`VERDICT: PASS(detours=N) | ABORTED`).

**Resolving Crash Offsets:**
Once you have the crash offset or instruction address (e.g. `0x0053f000`), immediately resolve the function and source symbol using:
```bash
python3 scripts/query_ghidra.py address 0x0053f000
```
This maps the raw crash address to the verified function name, prototype, and GOG counterpart in < 40ms.

## Step 4: kill a hung process

Target the game process only — **not** `reaper`, `wineserver`, or the Steam client:
```bash
python3 scripts/crash_triage.py kill [--timeout 5.0]
```
Sends `SIGTERM` gracefully, waits up to timeout, escalates to `SIGKILL` only if still alive, and confirms teardown. Emits `VERDICT: KILLED | NOT_RUNNING`.

### Relaunch

Agents don't launch the game; tell the user to relaunch. A plain Steam launch
keeps the KPM install (the proxy `binkw32.dll` stays in place). Never run
`KPatchLauncher --patches ...` just to restart: it uninstalls **all** patches
first and needs the full live ID list (CLAUDE.md "KPM Install Rule").

### Load-time hang vs. in-game hang

A `R`/`RN` game process at ~90-120% CPU in the first ~30-60 s, before the
world renders, is the known unattributed `NtGetContextThread`/
`GetCurrentPackageFullName` spin. Log evidence for it is *absence*: no
`Unhandled exception`, no new `err:` lines. Don't attribute it to whichever mod
was last touched: use a patch's own log (`python3 scripts/kotor2_logs.py check
buffhud_log.txt`, never a raw read; see the kotor2-log-analysis skill) to show
whether *its* hooks had even run by then, and count how many hangs happen with
vs. without the suspect patch (same live patch set otherwise) before blaming it.

## Patched installs (KPM): extra evidence sources

- `grep -ac "Applied DETOUR" ~/steam-208580.log` (hook count; ~200 hooks, so
  count, don't list) and `grep -a "SUCCESS: Patcher initialized" ~/steam-208580.log | head -3`
  (skip the duplicate `seh:dispatch_exception` echoes). A missing `SUCCESS`
  line means the patcher aborted.
- Patches write their own log next to the exe: `python3 scripts/kotor2_logs.py status`
  lists them. Hook code runs on the game's threads, so a log that stops
  mid-load shows where the game got to.
- `ConsoleFunc ... Offset not found` lines come from `additional-console-commands`
  and appear on every launch; they aren't a fault signal.

## Step 5: if a specific asset is a suspect

If the crash backtrace or the in-game context (what room, what item, what
NPC) points at something the modding pipeline touched, don't guess — check
the actual deployed bytes:

- Load the suspect `.tpc` through pykotor (`.venv/bin/python3`,
  `from pykotor.resource.formats.tpc import read_tpc`) and diff its
  `layers[0].mipmaps` (width/height/format/byte-length per mip) against the
  untouched source mod's copy in `~/Games/KOTOR2_Mod_Workspace/01_extracted/`.
  A mismatch in dimensions or a mip missing entirely is a real red flag; a
  matching mip chain clears it.
- `~/Games/KOTOR2_Mod_Workspace/ultrawide_gui/out_world/world_all_flags.json`
  has a `textures` dict keyed by resref with `format`/`flags`/`dxt_error` per
  deployed world texture. Only `RGB`/`RGBA` (uncompressed) entries can ever
  reach a runtime mipmap-builder crash (`glu32`/`gluBuild2DMipmaps`) — a
  `DXT1`/`DXT5` texture ships pre-baked mips and structurally can't cause
  that class of fault. Filter on `format` before chasing a DXT file for this
  specific symptom.
- Quality flags (`clamp`, `grain-loss`, `alpha-lanczos`, `decal`, `blending`,
  `dxt-error`) in that file are expected pipeline annotations, not
  corruption markers — don't treat their presence alone as a finding.

## When you're done

Append what you found to `CRASH_INVESTIGATION.md` (new dated `##` section:
symptom, log evidence, what was ruled out, root cause or "not reproduced",
fix applied or not) even if the answer is "couldn't reproduce, no fix
applied" — that's still load-bearing for the next investigation.
