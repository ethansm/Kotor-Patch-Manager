# 09 -- Intermittent load-time hang (Proton / Patch Manager): investigation brief

**Status (2026-09-29): OPEN, UNATTRIBUTED. Deliberately split out of the Buff Duration HUD work so it gets its own session.** This document is the hand-off: symptom, every incident so far, what the logs do and do not show, what has been ruled out, ranked hypotheses, and a concrete experiment plan with the capture procedure that was missing every time. Companion file: `CRASH_INVESTIGATION.md` (per-incident narratives, dated 2026-09-28 / 09-29). Skill: `kotor2-crash-triage`.

## 1. Symptom

During a loading screen (save load or area transition, i.e. before/while the world renders) the game stops making progress:
- `swkotor2.exe` is alive, state `RNsl`/`SNsl`, **70-120% CPU** (one thread pegged), never recovers on its own (one incident sat 12+ minutes).
- Visuals: black/frozen screen, or a half-rendered scene: partial 3D model, a cloaked-looking ghost of the character animating over the loading screen and a static loading bar (screenshots `Documents/Screenshot_2026-09-28_18-16-57.png`, `2026-09-29_18-31-41/52/00.png`). Pressing Esc hides the ghost and plays the menu-open sound, so **the UI/input thread still pumps messages**; the loading work is what is stuck. Keyboard input otherwise unresponsive.
- No crash: no `Unhandled exception`, no `Backtrace:`, no new `err:` lines; `AR_ERROR.LOG` md5 unchanged (`a2d887896b53eae113758dc8c3729db9`).
- Recovery: `kill -TERM $(pgrep -f '^S:.*swkotor2\.exe')`; clean exit in ~3-8 s, the Proton tree tears itself down. **Relaunching usually works** (never deterministic).

## 2. Incident table

| # | Date | Patch set at the time | Trigger | Elapsed at hang | Notes |
|---|---|---|---|---|---|
| 1 | 2026-09-28 | 9 (Phase 1b batch) | Peragus area-transition loading screen | -- | first sighting; retry fine |
| 2 | 2026-09-28 | 11 (+ map-aspect-fix, loading-screen-line-fix) | save load | 13 min | main thread `013c` busy in `RtlGetThreadPreferredUILanguages`, spin thread `020c`/`0208` |
| 3 | 2026-09-28 | same 11 | same save, retry #2 | 1 m 22 s | retry #3 loaded fine |
| 4 | 2026-09-29 | 13 baseline, with and without a throwaway probe hook | after Buff-HUD probe testing | -- | 13,713 `NtGetContextThread` lines, spin thread `0138:020c` |
| 5-6 | 2026-09-29 | 14 (13 + `buff-duration-hud` first diagnostic build) | load right after install, two launches in a row | ~90-116% CPU | mod log showed `buffhud loaded` + `snapshot` lines only |
| 7 | 2026-09-29 | 14 (redesign build with backing disc) | load after install | 2 m 23 s, `RNsl` 72% CPU | mod log: `snapshot` lines only, **zero `COLUMN#` lines** -> `DrawBuffColumn` never ran |

Counting: incidents 1-4 happened with **no Buff-HUD code loaded at all**. Incidents 5-7 (3 hangs in roughly 10 launches with the patch) happened with it, but in each, the mod's draw hook had not run yet; only the tiny `SnapshotPortraitSlot` store hooks had. **No launch with zero patches has been tested**, so nothing rules Patch Manager itself in or out.

## 3. What the logs show

- `~/steam-208580.log` is written with `PROTON_LOG=1` and Effective `WINEDEBUG=+timestamp,+pid,+tid,+seh,+unwind,+threadname,+debugstr,+loaddll,+mscoree`. `+seh`/`+unwind` tracing is what makes `NtGetContextThread` visible, and it also slows everything (log grows to several MB in 2 minutes).
- Hang signature: a tight repeating pair `trace:seh:NtGetContextThread 0x<h>: ...` / `fixme:ver:GetCurrentPackageFullName (...): stub` on one thread (`020c`/`0208`/`0218` -- thread ids differ per run), with the main thread `013c` separately looping on `nls:get_dummy_preferred_ui_language` / `RtlGetThreadPreferredUILanguages`. Counts of `NtGetContextThread` in a hung log: 13,713 and 18,510.
- **A healthy launch also has these calls, but only ~54** (checked on the launch that followed incident 7): 27 from the main thread `013c` (context of other threads, e.g. handle `0x1e8`, eip `7bf1e250` inside Wine's ntdll) and a handful of `0xfffffffe` (= current thread) self-queries per worker thread, plus 4 `GetCurrentPackageFullName` stubs. So the hang is a **runaway version of a normal startup behaviour**: something repeatedly captures thread contexts (often of its own thread) and asks for the package name, ~250x more than normal.
- Useless for attribution: none of the spin addresses point at a Patch Manager hook, and the enabled trace channels do not cover file I/O, so loading progress is invisible. Every hung Proton log was overwritten by the next launch (Proton rewrites `steam-208580.log` per launch) -- **no hung log from incidents 5-7 survives**; only the prose above and the mod log copy for #7 (`buffhud_log.hang4.txt`, scratchpad, likely gone).

## 4. Environment facts (relevant to hypotheses)

- Proton `experimental-11.0-20260924`, `ntsync` active, steamrt4, kernel 7.0.11, Ryzen 5 3600 (6C/12T), RTX 5070 driver 595.84. Game is the Steam Aspyr build (LAA variant, hash `4AB72FC1...`, original `6A522E71...`) using OpenGL (`opengl32.dll` in the game dir is **ReShade 6.8.0.2157**, 32-bit, present in every run).
- Steam launch options (localconfig): `PULSE_LATENCY_MSEC=60 PROTON_LOG=1 PROTON_LOG_DIR="/home/ethansm" MANGOHUD_CONFIGFILE=... gamemoderun taskset -c 0,6 mangohud %command%`. **`taskset -c 0,6` pins the entire game process, every wine service thread and every driver thread to logical CPUs 0 and 6, which are the two SMT siblings of ONE physical core (`lscpu -e`: CPU 0 and 6 both CORE 0).** `gamemoderun` is a no-op (`libgamemode.so` missing).
- Injection chain: proxy `binkw32.dll` -> `KotorPatcher.dll` applies detours at init (`Applied DETOUR` lines); also present: ReShade (`OPENGL32.dll`), Miles (`Mss32.dll`, `mss32midi.dll`), Steam overlay + MangoHud shims (LD_PRELOAD).
- KPM source (`src/`, `Patches/`) has no `SuspendThread`/`GetThreadContext`/`GetCurrentPackageFullName` uses -- the context-capture loop is not in our patcher. Candidates that do such things: ReShade (MinHook-style hook install freezes threads and reads contexts), the Steam overlay renderer, MangoHud, Wine's own machinery.

## 5. Hypotheses (ranked)

1. **CPU affinity starvation (`taskset -c 0,6`)** -- the whole game on one physical core. A thread that waits on another with a spin/yield loop (thread freeze, `SuspendThread`+`GetThreadContext` handshake, ReShade/overlay hooking during GL context creation on a loading screen) can livelock when the thread it waits for cannot get scheduled promptly, or when SMT contention makes a 2-CPU set effectively 1. Fits: intermittent, timing-sensitive, "faster on 2nd repro", spin on a helper thread, main thread still pumps. Cheapest to test (drop the affinity).
2. **`PROTON_LOG=1` with `+seh,+unwind,+debugstr`** widens race windows (heavy synchronous logging) and even provides the visible symptom (`NtGetContextThread` is only logged because `+seh` is on). The hang may exist without logging but appear as a plain freeze; logging may raise its frequency.
3. **ReShade (OpenGL proxy) re-hooking/recreating its runtime on loading screens** ("Recreated runtime environment" appears in `ReShade.log`; loading screens tear down/recreate GL state). Its hook engine freezes threads and reads contexts.
4. **Our detour patches (KotorPatcher) -- low but nonzero.** Rate looks slightly higher with the 14th patch installed (3/10) but incidents 1-4 predate it; patched code that runs during loading is only the UpdatePortraits snapshot hooks (two stores) plus other mods' hooks. A baseline of 13 patches also hangs.
5. Wine/Proton experimental regression or `ntsync` race (Proton-Experimental updates frequently; 2026-09-24 build).
6. Save/module content (specific save or area). Incidents span different areas/saves, so unlikely.

## 6. Experiment plan (do these in order; each is cheap; change ONE variable at a time)

**Capture procedure (do this at every hang -- it was the missing piece):**
```
# 1. BEFORE killing: keep the evidence
cp ~/steam-208580.log ~/hang_$(date +%Y%m%d_%H%M%S).log
PID=$(pgrep -f '^S:.*swkotor2\.exe'); echo $PID
top -H -b -n1 -p $PID | head -20                     # which threads (tids) are hot
grep -c NtGetContextThread ~/steam-208580.log
tail -400 ~/steam-208580.log | grep -a 'NtGetContextThread\|GetCurrentPackage' | head -6
ls /proc/$PID/task/*/comm | head; for t in /proc/$PID/task/*; do echo $(basename $t) $(cat $t/comm) $(cut -d' ' -f3,14,15 $t/stat); done   # per-thread state + utime/stime
# 2. optional, richer: gdb -p $PID -batch -ex 'thread apply all bt 12'  (Linux-side stacks; add wine symbols if available)
# 3. only then: kill -TERM $PID ; wait ~10 s ; verify with the ps line in the kotor2-crash-triage skill
```
Also copy `buffhud_log.txt` / `ReShade.log` and note which patches were installed (`kpm_install_state.json`).

**A/B matrix (target >= 10 cold launches, load one specific slow save each time; record hang/no-hang per cell in a table in this file):**
1. Baseline as-is (13 patches + buff-hud, PROTON_LOG=1, `taskset -c 0,6`) -- establishes today's rate.
2. Same, **drop `taskset`** (or pin to 0-5 / all 12) -- tests H1.
3. Same as 1, **`PROTON_LOG` off** (remove `PROTON_LOG=1 PROTON_LOG_DIR`) -- tests H2 (you lose logs; count hangs by CPU spin/freeze only, use the `top -H` checks above).
4. Same as 1, ReShade disabled (rename `OPENGL32.dll` in the game dir) -- tests H3.
5. 13-patch baseline without `buff-duration-hud` -- tests H4 (needs the 5+ vs 5+ comparison; also 0 patches via `KPatchLauncher` uninstall-all for a pure-vanilla-under-Proton cell).
6. Different Proton (Proton 10 / GE) -- tests H5.
Interpretation: if a single cell drops the rate to 0 over 10+ launches (vs a non-zero baseline), that variable is implicated; then bisect inside it (e.g. affinity to 0-5, or single patches).

**If it reproduces and a hang is live:** get the *Windows-side* stack of the spinning thread. Options: `winedbg --gdb`/`winedbg` attach (`winedbg --command "attach <winpid>" "bt all"`, needs the wine pid from `winedbg`'s `info process`), or a Wine `+relay` snapshot of a few seconds (large), or `WINEDEBUG=+seh,+module,+thread` on a repro. The goal is to name the caller of `NtGetContextThread` (module + address) -- that identifies ReShade vs overlay vs game vs patcher immediately.

## 7. Mitigations to try if H1/H2 hold

Remove/relax `taskset` (a 2-thread affinity on one core is a large performance and scheduling handicap for Wine anyway; if pinning is wanted for frame pacing use 4 distinct physical cores, e.g. `0-3`), leave `PROTON_LOG` off except when triaging. Record the before/after hang rate here.

## 8. Do NOT

- Blame the newest patch by default (see incidents 1-4).
- Kill anything but the `^S:.*swkotor2\.exe` PID (see the triage skill), and don't drop the capture step first.
- Reinstall patches just to restart (`KPatchLauncher` with `--patches` uninstalls all first; plain `KPatchLauncher <exe>` keeps the current install).
