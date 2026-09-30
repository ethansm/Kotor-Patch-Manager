# 09 -- Intermittent load-time hang (Proton / Patch Manager): investigation brief

**Status (2026-09-30 00:30): OPEN -- probe INSTALLED (section 11), awaiting the next hang. REFRAMED by section 9: the "hang" is a `CSWGuiLoadScreen` that is never dismissed while the main thread keeps rendering; the `IsBadReadPtr` burst is attributed (Force Power Hotbar first-tick scan, benign). Earlier status: OPEN, UNATTRIBUTED. Deliberately split out of the Buff Duration HUD work so it gets its own session.** This document is the hand-off: symptom, every incident so far, what the logs do and do not show, what has been ruled out, ranked hypotheses, and a concrete experiment plan with the capture procedure that was missing every time. Companion file: `CRASH_INVESTIGATION.md` (per-incident narratives, dated 2026-09-28 / 09-29). Skill: `kotor2-crash-triage`.

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
| 8 | 2026-09-29 20:25 | **16** (14 + `force-power-hotbar` + `inventory-3d-viewport`, installed by other sessions; Buff HUD v1.0.0, logging OFF) | load, killed at 50 s | 50 s, `SNsl` 91% | first hang WITH a saved Proton log (`~/hang_20260929_202500.log`); see section 3b |
| 9 | 2026-09-30 00:05 | 16 (newer Buff HUD/hotbar builds than #8; NO probe) | load, killed at ~45 s | ~45 s, `RNsl` 60% | captured `~/hang_20260930_000500.log` + hotbar log copy BEFORE killing. Same signature as #8: 8-fault hotbar scan burst at the first HUD tick, `modal[0]=CSWGuiLoadScreen`, hotbar HUD frames advancing ~60 fps (f660..f2460) under it. Buff HUD debug was off. |
| 7 | 2026-09-29 | 14 (redesign build with backing disc) | load after install | 2 m 23 s, `RNsl` 72% CPU | mod log: `snapshot` lines only, **zero `COLUMN#` lines** -> `DrawBuffColumn` never ran |

Counting: incidents 1-4 happened with **no Buff-HUD code loaded at all**. Incidents 5-7 (3 hangs in roughly 10 launches with the patch) happened with it, but in each, the mod's draw hook had not run yet; only the tiny `SnapshotPortraitSlot` store hooks had. **No launch with zero patches has been tested**, so nothing rules Patch Manager itself in or out.

## 3. What the logs show

- `~/steam-208580.log` is written with `PROTON_LOG=1` and Effective `WINEDEBUG=+timestamp,+pid,+tid,+seh,+unwind,+threadname,+debugstr,+loaddll,+mscoree`. `+seh`/`+unwind` tracing is what makes `NtGetContextThread` visible, and it also slows everything (log grows to several MB in 2 minutes).
- Hang signature: a tight repeating pair `trace:seh:NtGetContextThread 0x<h>: ...` / `fixme:ver:GetCurrentPackageFullName (...): stub` on one thread (`020c`/`0208`/`0218` -- thread ids differ per run), with the main thread `013c` separately looping on `nls:get_dummy_preferred_ui_language` / `RtlGetThreadPreferredUILanguages`. Counts of `NtGetContextThread` in a hung log: 13,713 and 18,510.
- **A healthy launch also has these calls, but only ~54** (checked on the launch that followed incident 7): 27 from the main thread `013c` (context of other threads, e.g. handle `0x1e8`, eip `7bf1e250` inside Wine's ntdll) and a handful of `0xfffffffe` (= current thread) self-queries per worker thread, plus 4 `GetCurrentPackageFullName` stubs. So the hang is a **runaway version of a normal startup behaviour**: something repeatedly captures thread contexts (often of its own thread) and asks for the package name, ~250x more than normal.
- Useless for attribution: none of the spin addresses point at a Patch Manager hook, and the enabled trace channels do not cover file I/O, so loading progress is invisible. Every hung Proton log was overwritten by the next launch (Proton rewrites `steam-208580.log` per launch) -- **no hung log from incidents 5-7 survives**; only the prose above and the mod log copy for #7 (`buffhud_log.hang4.txt`, scratchpad, likely gone).

## 3b. Incident 8 (2026-09-29 20:25): first fully captured hang -- CHANGES THE PICTURE

Captured before killing (`~/hang_20260929_202500.log`, 24,472 lines, killed at 50 s): 8,673 `NtGetContextThread`, 3,558 `GetCurrentPackageFullName`, no `Unhandled exception`, no new `err:`, `AR_ERROR.LOG` unchanged. Findings:
- **The "spin" thread is `wine_dinput_worker`, not a game/loader thread.** Thread `0138:0210` was renamed `wine_dinput_worker` at startup (`threadname` line) and every one of its 13,711 log lines is dinput mouse handling: `dinput_mouse_rawinput_hook` (1,599x, `Unimplemented MOUSE_VIRTUAL_DESKTOP flag`) interleaved with `GetCurrentPackageFullName` + `NtGetContextThread 0xfffffffe` (self) at `eip=7b582038` (inside `win32u.dll`, eax=0x14fa = a win32u syscall). It ran ~240 log-events/s, in proportion to raw mouse events: **the `NtGetContextThread` flood is most plausibly just the dinput worker servicing mouse movement (the user was moving the mouse at the frozen screen), i.e. a symptom-noise, not the hang itself.** Earlier incidents' spin threads (`020c`, `0208`, `0218`) are the same thread-id range and are probably the same dinput worker. `top -H` at the time: game main thread 50%, mangohud threads ~20% each, dinput worker ~10%.
- **The main game thread (`013c`) is the stuck one**: its last trace line is at t=2602456.747 -- a burst of 8 handled access violations with `IsBadReadPtr` on garbage pointers (`00000001`, `FFFFFFFD`, `7FFFFFFF`, `7F000000`, `0000FFFF`, `00000004`, all within the same millisecond, `c0000005` handled by SEH) -- and then NO further output from the main thread for the remaining 36 s while it stayed at ~50% CPU, i.e. it is inside code that calls no traced API (game code, a driver, or hook code) or is busy in a long legitimate load. `IsBadReadPtr` is used by the Buff HUD's `readAt()` helper (and possibly the game / other patches; not attributed) -- **an open lead worth checking first**: run with `buffhud_debug.txt` present so the mod log shows whether the hooks were mid-run, and compare the `IsBadReadPtr` burst against healthy launches (none captured yet with this trace set).
- Because the previous incidents' logs were overwritten, we cannot tell whether they also ended in this pattern.

**Follow-up (2026-09-29 22:00, verified on disk):**
- **The installed set was 16 patches, not 14.** `kpm_install_state.json` lists the 14 plus `force-power-hotbar` and `inventory-3d-viewport` (both DLLs are in the hang #8 `loaddll` lines and in the healthy 21:50 run). Incidents 5-7 may also have had one or both of those two; not recorded at the time.
- ~~Healthy-run control: 0 `IsBadReadPtr` faults in the 21:40-21:50 session -> burst is hang-specific.~~ **INVALID control (corrected ~22:10):** that session never reached gameplay (Buff HUD debug log has no `snapshot` line, hotbar log has no HUD tick, inv3d log only init), so no mod probe code ran. The burst is attributed by other evidence in section 9b.
- All 8 faults have the **same frame** (`esp=00dbf3fc ebp=00dbf468` inside `IsBadReadPtr`, `addr=7BEAD017`), i.e. one caller loop issued them. Pointer arguments: `FFFFFFFD`, `00000001` x3, `7FFFFFFF`, `7F000000`, `0000FFFF`, `00000004` -- small ints / sentinels, the shape of **freed or reused memory read as pointers**. Before the burst the main thread was quiet for 4 s (last line `RtlGetThreadPreferredUILanguages` at t=...452.672).
- Vectored exception handlers called per AV (all returned 0 = continue search): `7992E9A0` = `force-power-hotbar.dll` (base `79910000`), `79C5AF70` = `additional-console-commands.dll` (`79C10000`), `79E75C90` = `KotorPatcher.dll` (`79E50000`); then SEH `7BEB78B0` = kernel32 (IsBadReadPtr's own `__TRY`). The two mod VEHs come from the MinGW runtime (the DLLs import `AddVectoredExceptionHandler` but no mod source calls it); they are passive here.
- Mod code that probes pointers with `IsBadReadPtr`/`IsBadWritePtr` (framework clone `Patches/`): `BuffDurationHUD.cpp` `readAt` (every game read, incl. the effect-list walk and the Shield probe), `ForcePowerHotbar.cpp` `readAt`/string/entry/panel/hud checks, `Inventory3DViewport.cpp` `readAt`/gob/vtable/scene/item checks. KPM `src/` has none.
- **Leading hypothesis:** a mod reads a **stale cached game-object pointer after a load**. The Buff HUD caches the raw creature pointer per portrait slot (`g_slots`, filled by `SnapshotPortraitSlot`) and never invalidates it; `collectBuffs`/`collectPools` then walk `creature+0x148/+0x14c` (up to 512 entries) -> garbage effect pointers -> one handled fault each. This explains the burst, **not** the 36 s silent spin; the spin needs its own evidence (watchdog in section 10).

## 4. Environment facts (relevant to hypotheses)

- Proton `experimental-11.0-20260924`, `ntsync` active, steamrt4, kernel 7.0.11, Ryzen 5 3600 (6C/12T), RTX 5070 driver 595.84. Game is the Steam Aspyr build (LAA variant, hash `4AB72FC1...`, original `6A522E71...`) using OpenGL (`opengl32.dll` in the game dir is **ReShade 6.8.0.2157**, 32-bit, present in every run).
- Steam launch options (localconfig): `PULSE_LATENCY_MSEC=60 PROTON_LOG=1 PROTON_LOG_DIR="/home/ethansm" MANGOHUD_CONFIGFILE=... gamemoderun taskset -c 0,6 mangohud %command%`. **`taskset -c 0,6` pins the entire game process, every wine service thread and every driver thread to logical CPUs 0 and 6, which are the two SMT siblings of ONE physical core (`lscpu -e`: CPU 0 and 6 both CORE 0).** `gamemoderun` is a no-op (`libgamemode.so` missing).
- Injection chain: proxy `binkw32.dll` -> `KotorPatcher.dll` applies detours at init (`Applied DETOUR` lines); also present: ReShade (`OPENGL32.dll`), Miles (`Mss32.dll`, `mss32midi.dll`), Steam overlay + MangoHud shims (LD_PRELOAD).
- KPM source (`src/`, `Patches/`) has no `SuspendThread`/`GetThreadContext`/`GetCurrentPackageFullName` uses -- the context-capture loop is not in our patcher. Candidates that do such things: ReShade (MinHook-style hook install freezes threads and reads contexts), the Steam overlay renderer, MangoHud, Wine's own machinery.

## 5. Hypotheses (ranked)

1. **CPU affinity starvation (`taskset -c 0,6`)** -- the whole game on one physical core. A thread that waits on another with a spin/yield loop (thread freeze, `SuspendThread`+`GetThreadContext` handshake, ReShade/overlay hooking during GL context creation on a loading screen) can livelock when the thread it waits for cannot get scheduled promptly, or when SMT contention makes a 2-CPU set effectively 1. Fits: intermittent, timing-sensitive, "faster on 2nd repro", spin on a helper thread, main thread still pumps. Cheapest to test (drop the affinity).
2. **`PROTON_LOG=1` with `+seh,+unwind,+debugstr`** widens race windows (heavy synchronous logging) and even provides the visible symptom (`NtGetContextThread` is only logged because `+seh` is on). The hang may exist without logging but appear as a plain freeze; logging may raise its frequency.
3. **ReShade (OpenGL proxy) re-hooking/recreating its runtime on loading screens** ("Recreated runtime environment" appears in `ReShade.log`; loading screens tear down/recreate GL state). Its hook engine freezes threads and reads contexts.
4. **Our detour patches (KotorPatcher) -- low but nonzero (raised after hang #8, see 3b follow-up).** Rate looks slightly higher with the 14th patch installed (3/10) but incidents 1-4 predate it; patched code that runs during loading is only the UpdatePortraits snapshot hooks (two stores) plus other mods' hooks. A baseline of 13 patches also hangs.
5. Wine/Proton experimental regression or `ntsync` race (Proton-Experimental updates frequently; 2026-09-24 build).
6. Save/module content (specific save or area). Incidents span different areas/saves, so unlikely.

## 5b. Revised first steps after incident 8
1. Launch with `buffhud_debug.txt` created (Buff HUD logging on) and ALSO keep the Proton log; on the next hang copy both immediately. Look for: mod log lines right before the freeze (`snapshot`/`arrow`/`COLUMN`), and whether the same `IsBadReadPtr` burst precedes it in the Proton log.
2. In a healthy launch's Proton log, count `IsBadReadPtr ... caused page fault` (expected 0?). If healthy launches show none, the burst is tied to the hang; then find the caller (module of the `eip` at the faulting `dispatch_exception`, `addr=7BEAD021` is Wine ntdll's `IsBadReadPtr`; the caller frame is in the `backtrace:` lines of the `warn:seh:dispatch_exception backtrace` entries -- read those).
3. Only then run the A/B matrix below.

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

## 9. Engine bad-pointer map + what hang #8 really was (2026-09-29, Ghidra + mod logs)

**9a. The engine does not probe pointers.** `ImportCallers.java` over `swkotor2.exe`: no import of `IsBadReadPtr`, `IsBadCodePtr`, `IsBadStringPtr*`, `VirtualQuery` or `AddVectoredExceptionHandler`. `IsBadWritePtr` has one user (`FUN_00555d00`, re-allocates a 0x2000 scratch buffer). SEH frames exist only in the MSVC CRT (66 functions via `__SEH_prolog4`/`_except_handler4`); the `ExceptionList` pushes in game decompiles are C++ destructor unwinding. So the game tolerates no garbage pointers at all, and **every `IsBadReadPtr` fault in a Proton log comes from a loaded DLL**. In the live set only three import it (`objdump -p`): `buff-duration-hud`, `force-power-hotbar`, `inventory-3d-viewport`.

| Engine function | Address | Garbage tolerated | How |
|---|---|---|---|
| scratch-buffer reset | `0x00555D00` | stale/unwritable `this+0x274` buffer | `IsBadWritePtr(buf,0x2000)` -> allocate new |
| server `GetGameObject(id)` | `0x0051C0A0` -> `0x0053DFB0` | stale / invalid object ids (`0x7F000000`) | table lookup by id; returns 0, never dereferences a raw pointer |
| typed object lookup (`GetItemByObjectId` 0x51C0E0) | `0x00537B00` | wrong-type ids | checks the object's type tag before use |
| client object by id | `0x0073F550` -> `0x0078BDF0` | stale ids | same pattern, client side (used by `GetPlayerClientCreature` 0x78CEE0) |
| everything else | -- | nothing | raw derefs |

**Safe resolution API for mods:** cache the **object id** (`obj+4`), not the raw pointer, and re-resolve with `0x0051C0A0(id)` (server) when you need the object, or at least validate a cached pointer by (a) `probeRead` of the vtable slot, (b) vtable equals the expected class vtable (`CSWCCreature` `0x0099EE14`, `CSWSObject` family `0x0099450C`), (c) `*(ptr+4) == cachedId`.

**9b. The burst is attributed and benign.** Wine's `IsBadReadPtr` fault dump has `eax = ptr`, `edx = size` (proof: the `7FFFFFFF` probe faulted at `80000004` = `p[size-1]` with `edx=6`). The eight sizes (5,1,1,2,1,16,1,6) are CExoString lengths: `ForcePowerHotbar.cpp` `readCExoString` (`IsBadReadPtr(p, len)`) scanning the controlled creature for inline strings on the hotbar's **first HUD tick**; the hotbar log line `scan controlled obj=12a889c0 vt=0099ee14` has the same tick (`2602456747` vs Proton `2602456.747`). It runs once per new controlled character on the first HUD tick of every in-game session (healthy runs included; their Proton logs were overwritten, so their fault counts are unknown). **Not the hang.** The stale-pointer hypothesis of section 3b is withdrawn for the burst (the Buff HUD fix below is kept as hardening only).

**9c. What hang #8 was.** The same hotbar log shows `HotbarHudTick` (hook on `CSWGuiMainInterface::Draw` 0x74B3F0) running at ~60 fps for 32 s (frames 1..1860) after the burst, until the kill -- **the main thread was not stuck; it was rendering.** The first tick logged `gate: inactive (modal open)` with `modal[0]=02ca1f40 vt=009a5c84` = **`CSWGuiLoadScreen`** (RTTI). The only other run whose first HUD tick saw that modal is the 19:37 run (`force_power_hotbar_log.build2.txt`, ~60 fps until killed, likely incident 7). Every healthy run's first tick sees `hud on top`. => **The load finished far enough to render the world and HUD (the "ghost" under the load screen), but `HideLoadScreen` was never called.** 50% CPU = normal rendering. Esc works because the main loop is alive.

**9d. Where the load screen is dismissed** (`CClientExoAppInternal`, `internal = *(*(*(0x00A1B4A4)+4)+4)`; load screen at `internal+0x278`, GUI manager `+0x274`):
- `ShowLoadScreen` `0x00798190`, `HideLoadScreen` `0x007983E0`, `GetLoadScreenVisible` `0x00798540` (GOG->Steam delta +0x353A40).
- `CClientExoAppInternal::MainLoop` `0x00781BE0` area-finish block: `if (internal+0x288 && internal+0x18 && GetPlayerClientCreature()) { if (!flag(*(internal+0x48)+0x268) && internal+0x338 != 0x7F000000) ProcessPendingPartySwap() else { PlayMovies (busy-wait); Hide...; } }`. `+0x288` is set to 1 when the server module state `*(*(app+0x14)+4)` is 1 or 3. `GetPlayerClientCreature` `0x0078CEE0` resolves the player object id `internal+0x20` (cached at `+0x3BC/+0x3C0`).
- Other `Hide` callers go through the `CClientExoApp` wrapper `0x0073FF50` (called from `0x00534600`, `0x007D1530`, `0x0068E720`, `0x00857490`, `0x008FA550`).
- So the stuck condition is one of: `+0x288` never set (server never reports ready), `+0x18` zero, player creature id unresolvable on the client, or a party swap id in `+0x338` that never clears. The watchdog (section 10) logs exactly these fields while a load screen is up.

**9e. Engine-side candidate (write-up only, not implemented):** `PlayMovies` `0x00798820` busy-waits `while (IsMoviePlaying()==1)` with a message pump only every 500 iterations and no sleep -> 100% of a core while a Bink movie plays, and a permanent spin if a movie never reports done. Not matched to hang #8 (frames kept rendering), but a real hang shape; a safe patch would pump + `Sleep(1)` each iteration. Needs the user's go-ahead.

## 10. Diagnostics: `load-hang-probe` 1.1.0 + `SafeRead.h` (INSTALLED 2026-09-30 00:07; see section 11) 

**What it is.** A diagnostics-only patch (`Patches/LoadHangProbe`, sources of record `patch_manager_mods/load_hang_probe_src/`), inert unless `hangprobe_debug.txt` exists in the game dir. One hook: `0x00781BE0` = `CClientExoAppInternal::MainLoop` entry (`55 8B EC 6A FF`, verified against the live and vanilla exe; no other installed patch hooks near it), ECX = client internal -> frame heartbeat + main thread id + stack bounds. A watchdog thread **polls** every 250 ms (plain memory reads; it never pauses the game thread in a normal session). **Sampling is armed only when something already looks wrong**: load screen up > 5 s with > 120 frames rendered under it, no MainLoop call for 5 s, or a manual `hangprobe_dump.txt` (sampled 2.5 s, then reported). While armed (and only then) it samples the main thread every 250 ms (v1.0.0 sampled always):
- sampling =  `SuspendThread` / `GetThreadContext` / EBP-chain walk + raw return-address scan, reading **only `[ESP, StackBase)` and only when ESP lies in the stack's own reservation**, with no API call while the thread is suspended; `ResumeThread`;
- reads the load-state fields of section 9d (via `SafeRead`) and logs every change to `hangprobe_log.txt` (`STATE ...`, `load screen UP/DOWN after N ms, M frames`), plus a heartbeat line every 60 s;
- writes `hangprobe_report.txt` (rewritten every 5 s while the condition holds) on **`LOADSCREEN_STUCK`** (CSWGuiLoadScreen modal > 20 s while > 600 frames ran), **`MAIN_STALL`** (no MainLoop call for 15 s and one stack signature in >= 90% of the last 10 s), or **`MANUAL`** (`touch "$G/hangprobe_dump.txt"`). Report = reason, load state, main-thread EIP histogram + return-address frequencies with module+offset, last 3 stacks, the failed-probe ring, module map, and tails of `force_power_hotbar_log.txt`, `buffhud_log.txt`, `inv3d_log.txt`, `hangprobe_log.txt`.
- Side effect: under `PROTON_LOG=1` with `+seh`, each sample adds ~3 `NtGetContextThread` trace lines (4 samples/s, only while armed; log shows `sampling ARMED (why)` / `disarmed`) from the watchdog thread -- do not count those as the old "spin" signature.

**`Patches/_shared/SafeRead.h`** (copy: `patch_manager_mods/_shared_src/`): `saferead::probeRead(site, p, len)` / `readAt(site, base, off, &out)` = VirtualQuery with a per-thread 4-region cache cleared by `beginScope()` at every hook entry; never raises an exception, refuses guard pages; failed probes go to a 64-entry ring (`site, addr, len, caller, tick`) formatted by `saferead::dump`. Rule: never call it while you have another thread suspended.

**Smoke test of 1.1.0: 3 s of normal frames -> no sampling, no ARMED line; manual trigger -> ARMED, MANUAL report after 2.5 s, disarmed; 20 s spin -> ARMED (no MainLoop frame) at 5 s, MAIN_STALL report at 15 s. Earlier 1.0.0 smoke test (Proton-Experimental wine, throwaway prefix, harness `smoke.cpp`: 60 fps fake MainLoop, manual trigger, then a 21 s busy spin):** MANUAL report written; MAIN_STALL first written 15.8 s after the last frame and rewritten every 5 s; top EIP / return addresses resolve into the spinning function; no crash. It also caught a real bug (unsigned `now - lastFrameTick` underflow when the main thread writes a newer tick -> false MAIN_STALL while a vsync-like sleep dominates the stack), fixed with a signed difference before staging.

**Deliberately NOT done (scope decision, see section 9b):** the `IsBadReadPtr` -> `probeRead` swap and stale-pointer validation in `BuffDurationHUD`, `ForcePowerHotbar` and `Inventory3DViewport`. Their probes are not the hang (9b/9c), and all three sources were being edited by other sessions at the time (21:53-21:54). Ready to apply later: `#include "../_shared/SafeRead.h"`, replace each `IsBadReadPtr(p, n)` with `!saferead::probeRead(<site>, p, n)`, call `saferead::beginScope()` at each exported hook entry. Cheapest stale-pointer hardening for the Buff HUD: stamp `g_slots` entries with `GetTickCount()` in `SnapshotPortraitSlot` and ignore entries older than 500 ms in `creatureForSlot` (UpdatePortraits refreshes live slots every frame).

**Install (needs the game CLOSED and the user's OK; the launcher uninstalls everything and relaunches the game).** Stage `tools/kotor-patch-manager/hangprobe_stage/` = byte-identical copies of `inv3d_live_patches/` (the exact source of every live DLL, md5-matched 2026-09-29 22:20; note it carries the **19:08 Buff HUD build and the 20:15 hotbar build**, i.e. what is live now) + `LoadHangProbe.kpatch`. Then:
```
touch "$G/hangprobe_debug.txt" "$G/buffhud_debug.txt"
cd ~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/bin/Debug && ./KPatchLauncher "$G/swkotor2.exe" --patches ../../hangprobe_stage \
  k2-visible-console good-alignment-heads k2-aspyr-normal-map-fix k2-aspyr-saber-hilt-texture-fix k2-finesse-melee-fix k2_grass_fog_fix \
  max-lights semi-transparent-letterbox k2-mp3-playback-fix loading_screen_line_fix_k2 k2-aspyr-map-aspect-fix post-combat-movement-fix-v1 \
  additional-console-commands buff-duration-hud force-power-hotbar inventory-3d-viewport load-hang-probe
```
Verify: steam log `Applied DETOUR` count = previous count + 1 (`HangProbeMainLoop @ 0x00781BE0`), `SUCCESS: Patcher initialized`, `AR_ERROR.LOG` md5 `a2d887896b53eae113758dc8c3729db9`, `hangprobe_log.txt` shows `main thread ... opened` and `load screen UP` / `DOWN after N ms` around each load. **Calibrate** (section 5b rule): over a few normal sessions (save loads, area transitions, menus, alt-tab) no `REPORT` line may appear and `sampling ARMED` should be rare (a normal load screen under ~5 s must not arm it; a long legitimate load that arms it is fine but tells us to raise `ArmLoadMs`); record the longest `load screen DOWN after` value here and raise `LoadStuckMs` if a normal load comes near 20 s.

**On the next hang:** copy `~/steam-208580.log` first (section 6), then `hangprobe_report.txt` + `hangprobe_log.txt` (+ the three mod logs); if no report was written yet, `touch "$G/hangprobe_dump.txt"` and wait 1 s. Read the `STATE` line that stops changing: which of `areaReady(+288)`, `+18`, `playerId`/`cache`, `pendingSwap(+338)`, `sub48flag`, `movie`, `a1b4e0`, module state holds the area-finish block of `0x00781BE0` closed. Then map the top return addresses with Ghidra (`DecompileArgs`).

**Hang tally after this point** (success criterion: a report naming the stuck condition, or 0 hangs in 10+ launches):

| Date | Launches | Hangs | Notes |
|---|---|---|---|

## 11. Install history, incident 9, calibration, change log (2026-09-30 00:00-00:30)

**Incident 9 (00:05)** -- see the table. Evidence copied first: `~/hang_20260930_000500.log`, `~/hang_hotbar_*.txt`. Killed with `kill -TERM` (exited in 1 s).

**Install sequence (all times 2026-09-30):**
1. ~00:06 I installed 17 patches (16 ids + `load-hang-probe`) with `KPatchLauncher --patches hangprobe_stage/`, which was staged from `inv3d_live_patches/` (md5-matched to the live DLLs at ~22:20 on 09-29). **That was stale:** another session had reinstalled newer builds at 23:56 (`kpm_install_state.json` UpdatedAt 23:56:32) -- Buff HUD with the `ShieldProbeSlot` hook (`shield_probe_stage3/`, 23:47) and the hotbar with `HotbarBuildButtons` (0x00748502). My install silently dropped those two hooks (hook diff against the 00:05 log: 24 hooks before, 23 after adding the probe's). Detected by diffing the `Loaded DETOUR` lists of the two Proton logs.
2. Backup taken by me BEFORE that install: `~/Games/KOTOR2_Mod_Workspace/backups/pre_hangprobe_20260930_000546/` (`kpm_install_state.json`, `patch_config.toml`, `patches/` = the 23:56 live DLLs).
3. Recovery stage `tools/kotor-patch-manager/hangprobe_stage2/` = `hotbar_stage2/` kpatches + `LoadHangProbe.kpatch`, with `ForcePowerHotbar.kpatch` **repacked around the exact backed-up live hotbar DLL** (md5 3ede05bc, 3 hooks); every other DLL md5-verified equal to the backup. Not used in the end.
4. 00:07:50 an install by another session (or the user) landed: 17 ids incl. `load-hang-probe`, 25 hooks (= the 24 pre-existing + `HangProbeMainLoop @ 0x00781BE0`), hotbar = the 00:07 rebuild (`hotbar_stage2/ForcePowerHotbar.kpatch`, md5 05fbb24f, same 3 hooks), the other 7 DLLs byte-identical to the backup, `AR_ERROR.LOG` md5 `a2d887896b53eae113758dc8c3729db9`. **This is the current live state.**

**Lessons (also in the Ghidra DB `lessons`):** (a) a stage directory's match against the live DLLs is only valid at the moment it was checked -- re-hash `$G/patches/*.dll` against the stage and re-read `kpm_install_state.json` immediately before every install, and take a backup first; (b) after any install diff the `[Config] Loaded DETOUR` lines against the previous session's; (c) another session may install at any time -- check `pgrep` and `UpdatedAt` before assuming the game is closed.

**Calibration data (3 probe launches, 00:00-00:27, 0 hangs):** normal load screens `UP -> DOWN` in 4.77 s (x3, save loads), one 6.04 s load; `sampling ARMED` fired once (the 6.04 s load crossed `ArmLoadMs`=5 s, 1 sample, disarmed 254 ms later, no report); `failedProbes=0`; heartbeats every 60 s (frames up to 12,640) with `armed=0`. **Action:** raise `ArmLoadMs` from 5000 to 10000 in the next probe rebuild (normal loads reach 6 s; a real hang stays > 20 s anyway). Success criterion still open: a report naming the stuck condition, or 0 hangs in 10+ launches (3 so far with the probe).

| Date | Launches with probe | Hangs | Notes |
|---|---|---|---|
| 2026-09-30 00:07-00:27 | 3 | 0 | load screens 4.8/4.8/4.8/6.0 s; one benign arming |

**Change log (everything built or changed during the 09-29 evening / 09-30 early session; framework clone = `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager`, uncommitted):**
| Artifact | Location | Status |
|---|---|---|
| `LoadHangProbe` patch 1.1.0 (hook 0x00781BE0, poll + armed sampling, `hangprobe_log/report.txt`, `hangprobe_dump.txt` manual trigger) | framework `Patches/LoadHangProbe/`; sources of record `patch_manager_mods/load_hang_probe_src/` | installed |
| `SafeRead.h` (VirtualQuery-based `probeRead`, failed-read ring) | framework `Patches/_shared/`; copy `patch_manager_mods/_shared_src/` | built into the probe only; NOT yet used by the other mods |
| Ghidra scripts `ImportCallers.java`, `RangeDecompileArgs.java` | framework `tools/ghidra_scripts/`; copies in `load_hang_probe_src/` | reusable, registered in DB `ghidra_scripts` |
| Knowledge DB: 14 functions, 3 classes (CSWGuiLoadScreen 009a5c84, CSWGuiMessageBox 0099e6a4, CSWCCreature 0099ee14), 7 lessons | `patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db` | committed |
| Launcher copy with `KPATCH_NO_LAUNCH=1` (install without launching the game) | framework `bin/NoLaunch/` (source edit reverted; `bin/Debug` untouched) | usable: `KPATCH_NO_LAUNCH=1 bin/NoLaunch/KPatchLauncher "$G/swkotor2.exe" --patches <stage> <ids>` |
| Stages | `hangprobe_stage/` (STALE, 09-29 22:xx builds, do not use), `hangprobe_stage2/` (= 23:56 live builds + probe, verified) | stage2 is the safe reference |
| Doc corrections | this file sections 3b/9-11, `CRASH_INVESTIGATION.md` | committed |
| Deferred | `IsBadReadPtr -> probeRead` swaps + stale-pointer guard in Buff HUD / hotbar / inv3d; PlayMovies busy-wait engine patch (section 9e) | not done, need go-ahead / quiet sources |

**Next steps for the hang:** (1) keep `hangprobe_debug.txt` + `buffhud_debug.txt` in the game dir and play normally; (2) on the next hang copy `~/steam-208580.log`, then `hangprobe_report.txt`, `hangprobe_log.txt`, `force_power_hotbar_log.txt` BEFORE killing; the report's last `STATE` line says which area-finish gate stayed closed (section 9d); (3) rebuild the probe with `ArmLoadMs`=10000 when convenient (keep the stage rule above).

## 12. Incident 10 (2026-09-30 01:17) -- first hang captured by the probe (load-hang-probe 1.1.0 + perf-lab 0.1.0 installed, 19 ids)
Evidence dir: `~/Games/KOTOR2_Mod_Workspace/incidents_10_0118/` (probe log/report, perflab_*, `steam-208580.log`, AR_ERROR.LOG md5 `a2d887896b53eae113758dc8c3729db9` unchanged). Killed with TERM after ~95 s (`SNsl`, 95% CPU), teardown clean. Proton log: no `Unhandled exception`/`err:` lines (only the usual NAME_THREAD/PRINTEXCEPTION trace).
- **Probe fired as designed:** `load screen UP` (f30510) -> `sampling ARMED` -> `REPORT LOADSCREEN_STUCK` (dominance 22-35%, i.e. no single stuck function). State at the stuck report: `lsModal=1 modals=1`, `areaReady(+288)=1`, `pendingSwap(+338)=7F000000` (none), **`playerId=FFFFFFFD` and `cache(+3bc)=FFFFFFFD`** (the player-creature id never became a real object id; earlier healthy state showed `80000240`-style ids), `movie=1`, `a1b4e0=0`, module state [0]=[1]=0, 0 failed probes. Frames kept running (MainLoop calls every few ms).
- **Main thread samples alternate between two stacks:** (a) `MainLoop 0x781D7B -> 0x798882 (inside PlayMovies 0x798820) -> 0x797DAF -> 0x72FB36 -> 0x72DF91 -> ... -> DINPUT8 -> win32u (PeekMessage/pump)` in 18-32% of samples: **the area-finish block's PlayMovies wait loop is entered repeatedly** (09 section 9d/9e); (b) `MainLoop -> 0x78316F -> 0x7A24CA -> 0x7F3763 -> 0x480385 -> 0x46C4FF -> GUI draw -> opengl32`: normal rendering of the load screen. Top EIPs: win32u+11BE0 32%, opengl32+C6BA0 21%.
- Interpretation (hypothesis, not proven): the load never finishes because the client never obtains a valid player creature (`GetPlayerClientCreature` 0x78CEE0 keeps failing, id -3), so the load screen is never hidden; the loop keeps re-entering the PlayMovies branch. Next question: why the player id is `FFFFFFFD` (server-side create/transfer not done? save/area load error swallowed?). Not attributable to perf-lab (allocation hooks touch no game state; 9 earlier incidents happened without it, 4 without any mod code), but the with/without rule stays: tally below.
- PerfLab at the hang: live 257 MB in 69k blocks (steady 256.8-256.9 MB across three reports: no growth while stuck); 195 MB in 73 blocks live at one site (`FUN_0073DBA0+0x71 <- CExoResMan::Malloc+0x87`, 692 MB allocated in 283 calls) i.e. large whole-resource reads via the resource manager.

| Date | Launches with probe | Hangs | Notes |
|---|---|---|---|
| 2026-09-30 01:15 | 1 (with perf-lab 0.1.0) | 1 | incident 10, first full probe capture |

**Suggested next diagnostics (need a code change; ask first):** log `playerId`/`GetPlayerClientCreature` result and the server module state each 250 ms from LOAD_UP to stuck; hook or read the server-side creature-created state; capture `AR_ERROR.LOG`-style engine errors in the window. Also raise `ArmLoadMs` to 10 s (calibration, section 11).

**Incident 11 (01:19, relaunch of the same install, evidence `~/Games/KOTOR2_Mod_Workspace/incidents_11_0119/`):** identical to incident 10: hang at the first load ~59 s after launch, `LOADSCREEN_STUCK`, `playerId 7F000000 -> FFFFFFFD` ~4 s after `load screen UP`, same `internal=04C9E1F0`, same stacks (PlayMovies wait loop 0x798882 alternating with GUI render), live heap **byte-identical** to incident 10 (256,941,452 B / 69,403 blocks) => deterministic content. AR_ERROR.LOG md5 unchanged. Killed with TERM, clean teardown. Tally: with probe only 3 launches / 0 hangs (00:07-00:27); with probe + perf-lab 0.1.0 (alloc census ON) 2 launches / 2 hangs. **A/B in progress:** perf-lab census switched off (`perflab_debug.txt` renamed to `.off`: DLL loaded but inert, no IAT hooks), same load.

| Date | Launches with probe + perf-lab (census on) | Hangs |
|---|---|---|
| 2026-09-30 01:15-01:19 | 2 | 2 |

**A/B result #1 (01:24, census OFF = perf-lab loaded but inert, probe 1.1.0):** the first load after launch completed normally: `load screen UP -> DOWN after 4540 ms, 36 frames`, playerId became a real id (`80000249`), 4.5 min of healthy play at the time of writing. Tally: **census on 0.1.0: 2 launches / 2 hangs; census off: 1 launch / 0 hangs; probe only (00:07-00:27): 3 / 0.** Suggestive (p ~ 1/16 if hang rate were 50%) but n is small: the census stays the leading suspect. Bisect plan (each arm >= 3 launches, same save/first load): (1) census on, `scan=48` again (reproduce); (2) census on, `scan=4` (almost no stack scan); (3) census on with the HeapFree/ReAlloc hooks only; then read the code for a semantic difference (my hooks call the real functions with unchanged arguments; candidates: added latency on the game's own heap calls changing thread timing during load, the 32 MB + 2 MB VirtualAlloc tables perturbing the address-space layout of an already 3.8 GB-peak process, or a bug in ptrInsert/ptrRemove under contention).

## 13. load-hang-probe 1.2.0 (built 2026-09-30 ~01:35, NOT yet installed; stage `kotor-patch-manager/probe12_stage/` = perflab_stage + new LoadHangProbe.kpatch + PerfLab 0.1.1)
Additions (source `load_hang_probe_src/`, smoke-tested in Wine: `perf_lab_src/harness/smoke_probe12.cpp`):
1. **Hook on `SetPlayerCreatureId` 0x00798660** (thiscall(internal,newId); sole writer of the client player id at internal+0x20; callers MainLoop 0x782377 -> 0x7F000000, ResetGame 0x78544E, 0x79EE42, 0x7B3102, wrapper 0x73F921). Ring of (tick, frames, tid, caller, old, new), logged as `SETPLAYER` lines. The logged caller must be one of `78237C/785453/79EE47/7B3107/73F926`, otherwise the `[esp+0]`/`[esp+4]` parameter sources are wrong for a function-entry detour (`(?)` marks it). Identifies WHO passes `FFFFFFFD`.
2. **File I/O ring via IAT hooks** (CreateFileA/W, ReadFile, SetFilePointer, CloseHandle; `io=0` in `hangprobe_debug.txt` disables): last 2048 events with file name tail, sizes, errors; absent-file probes counted only; failed opens (other than not-found), read failures and short reads in a separate ring. The stuck report gains a `file I/O` section (last 150 events, totals, `last I/O N ms ago on <file>`) and a `SetPlayerId` section.
3. **Load timeline:** `LOADTL` line every 2 s while the load screen is up (io deltas since UP, last I/O age and file, `gstate`(+9C), playerId, +28C, areaReady); `load screen UP/DOWN` lines carry io counts; `STATE` gains `gstate(+9c) +28c +18 +14 +324`.
4. `ArmLoadMs` raised to 10 s.
Note: the probe's own IAT hooks are independent of the perf-lab census (different imports); if the census is the hang trigger, run the probe with the census off.

**Install of 1.2.0 (2026-09-30 01:33):** 19 ids via `probe12_stage/`; backup `~/Games/KOTOR2_Mod_Workspace/backups_probe12_0133`. Install-safety caught a hand-copied DLL: `buff-duration-hud.dll` had been replaced at 01:31:50 (newer than `kpm_install_state.json` UpdatedAt 01:15:34) by another session's build (`Patches/BuffDurationHUD/BuffDurationHUD.kpatch`, packaged 01:29, same version `1.1.0-probe4`, same 7 hooks), so the stage's Buff HUD kpatch was replaced with that one before installing (live md5 d65312df == new kpatch). Live: 19 ids, hooks 59 -> 60 (+SetPlayerId). `perflab_debug.txt` stays `.off` (census off); `hangprobe_debug.txt` present.

**Session hand-off pointer (2026-09-30 ~01:40):** the current state, evidence locations and ordered next steps for the hang AND the engine research live in `10_engine_optimization_research.md` ("HAND-OFF STATE" + "NEXT STEPS"). Hang status: root cause unknown; census (perf-lab) is the leading suspect (2/2 hangs on, 1/1 clean off); probe 1.2.0 installed to capture SetPlayerId callers / I/O tail / LOADTL; awaiting healthy-baseline launches.

## 14. Incident 12 (2026-09-30 02:26) -- HANG WITH THE CENSUS OFF (probe 1.2.0, perf-lab loaded but inert)
Evidence: `~/Games/KOTOR2_Mod_Workspace/incidents_13_0227/` (probe log/report, threads.txt, steam log, AR_ERROR.LOG md5 unchanged `a2d88789...`). First load after launch, 19 ids, `$G/perflab_debug.txt` absent (flag `.off`, `perflab_*` untouched since 01:19), killed with TERM after ~60 s, teardown clean.
- **The census is NOT required for the hang.** Tally now: census on 2 launches / 2 hangs; census off (probe-only 00:07-00:27 3/0, then 01:24 0/1, 01:44 0/1, ~02:00 0/1, ~02:20 0/1 first loads, plus repeat loads) **1 hang in ~8 first loads**. Fisher exact (2/2 vs 1/8) p ~ 0.07: the census may raise the rate, unproven; the bisect (doc 10 step 2) is now about rate, not cause.
- **Timeline (LOADTL):** UP f243; +2010 ms opens+442 reads+1706 240 MB; +4017 ms opens+681 reads+2555 598 MB; `SETPLAYER f278 ret=0073f926 7f000000 -> fffffffd` (same caller as healthy loads); by +6 s I/O is finished: **opens+727 reads+2794 647 MB fails+408, i.e. the same totals as a healthy load (695-741 / 2664-2816 / 644-648 MB)** and never changes again, last file `.\DIALOG.tlk`. Healthy loads print `load screen DOWN` within ~1 ms of SETPLAYER; here it never does (LOADTL kept printing to +30 s, `gstate=0`, `playerId=fffffffd`, `areaReady=1`, `movie=1 a1b4e0=0`; healthy: areaReady goes 0 at DOWN).
- **So the load's resource I/O completes; what is missing is the step after SETPLAYER that dismisses the load screen.** Main thread (R, 90% CPU, 58 fps) samples: PlayMovies wait loop `0x798882/0x798B52 -> 0x797DAF -> 0x72FB36 -> 0x72DF91 -> DINPUT8 -> win32u` (~40%) alternating with GUI render (opengl32). No other swkotor2 thread is running (all sleeping in ntsync/pipe/poll). No exception, no `err:` lines, no failed/short reads.
- Interpretation: not an I/O or allocator failure. The `PlayMovies` area-finish wait (candidate F1, doc 09 s9d/9e) is waiting for a condition (movie/`a1b4e0`/module state) that never becomes true. Next: Ghidra `PlayMovies` 0x798820 exit conditions and who sets them; add to the probe a per-250 ms log of those condition variables from UP on (ask first).

## 15. ROOT CAUSE FOUND (2026-09-30 ~03:00, Ghidra + probe data): the load screen is only dismissed while the window is ACTIVE
- `MainLoop 0x781BE0` area-finish block: after `PlayMovies(1)` it calls `HideLoadScreen 0x7983E0` **only if `IsMoviePlaying() != 0 || DAT_00A1B4E0 != 0`** (and again later if `a1b4e0 != 0 || FUN_007fae30() != 0`). `PlayMovies 0x798820` itself just loops while `IsMoviePlaying()==1` and is called every frame; its prologue `FUN_00797d90` polls DirectInput, which is why the hang samples show `0x798882 -> 0x797DAF -> ... DINPUT8 -> win32u`.
- **`0xA1B4E0` is the application-active flag.** Set to 1 by `0x40BCF0`, cleared by `0x40BC40`; both are driven by `WndProc 0x40A750` `case 0x1c` (**WM_ACTIVATEAPP**: wParam=0 -> `0x40BC40`; activate -> `0x40BCF0`), also `0x409750`, and `0x40BF40` (end of the area-finish block, gated by `DAT_A1B768==1 && DAT_A1B76C==0`).
- **Data (probe `a1b4e0` at the time of `load screen UP`)**: every load with `a1b4e0=1` finished (`DOWN` after 4.5-4.8 s, 15 loads, census on or off); **every hang (incidents 10, 11, 13 and the census-on ones) had `a1b4e0=0` at UP and it never changed.** I/O totals, the `SETPLAYER` call and `playerId=FFFFFFFD` are identical in healthy and hung loads, so they are not the gate (section 12's "player id" hypothesis is withdrawn; `GetPlayerClientCreature` returns non-zero for FFFFFFFD in healthy loads too).
- **Hypothesis (strongly supported, not yet confirmed by a recovery):** the hang is the game believing its window is deactivated (WM_ACTIVATEAPP(FALSE) received, or TRUE never delivered, e.g. focus on another window/launcher when the first load runs). It should clear as soon as the window is activated. Census on/off is then a coincidence of who had focus; this also explains "first load after launch" and why the game renders normally while stuck.
- **Confirm next hang (no code change):** with the game hung, click on the game window / alt-tab away and back; expect `load screen DOWN` within a second. Also ask: was the game window focused during each hang (KDE focus-stealing prevention, Steam/terminal in front)?
- **Possible fix (engine behaviour, needs go-ahead, candidate H1 in the DB):** hide the load screen regardless of `a1b4e0`, or force `0x40BCF0` when the area-finish block runs with the load screen up and the window is actually focused. Non-engine mitigation: keep the game window focused during the first load (launch, click into the window, then load).

## 16. Fix design + confirmation logging: load-hang-probe 1.3.0 (built 2026-09-30, NOT installed)
**Startup pattern behind the hang (probe `a1b4e0` from launch to `load screen UP`, 14 launches in the incident dirs):** healthy launches show `0,1,0,1` then UP; **hung launches (4 of 6 hung ones) show `0,1,0,1,0`**, i.e. one more deactivation after startup, never followed by an activation; one launch had `0,1,0,1,0,1@f2736` (re-activated before the load) and loaded fine. So the flag tracks real WM_ACTIVATEAPP traffic, and the hang is "the last activation event before the first load was a deactivation".
**Design (guarded, engine-native, no engine bytes patched):**
1. *Log first (always on when the probe is on):* hooks on `WndProc 0x40A750` (only activation-related messages: SIZE/ACTIVATE/SETFOCUS/KILLFOCUS/SHOWWINDOW/ACTIVATEAPP/MOUSEACTIVATE/WINDOWPOS*/NCACTIVATE/SYSCOMMAND/DISPLAYCHANGE/CLOSE, each with the OS foreground window and a1b4e0), `AppActivated 0x40BCF0` and `AppDeactivated 0x40BC40` (caller return address, a1b4e0 before). Watchdog logs `WINEV ...` per event and a `WIN ...` line on every change of {game hwnd, fgIsGame, iconic, visible, a1b4e0, activation-gate globals d4/bc/c0/768/76c/42a0/init/lastDeactTick, foreground pid/title}; `LOADTL` lines now carry `active(a1b4e0)` and `fgIsGame`; the hang report has a `window / activation` section with the last 80 events. **Confirmation questions the logs answer:** (a) did a `WM_ACTIVATEAPP wp=1` arrive after the last deactivation and get ignored by the handler (then a `WINEV WM_ACTIVATEAPP wp=1` with no `APP-ACTIVATED`, and the gate globals show why), or never arrive (then `fgIsGame` says whether the OS thought the game was focused, and `fg=` names who was)? (b) who called AppDeactivated (`ret=`)?
2. *Manual causality test (no behaviour change unless you ask for it):* `touch hangprobe_activate.txt` in the game dir during a hang: the main-thread hook calls the engine's own `AppActivated` once and logs `FIX MANUAL ...`; if `load screen DOWN` follows, activation is the cause.
3. *The fix (default OFF; `fix=auto` in `hangprobe_debug.txt`, optional `fixdelay=6000`):* in the MainLoop hook (main thread, frame boundary), only when `a1b4e0==0` AND the load screen has been up >= fixdelay (a healthy load is ~4.8 s) AND `internal+0x288` (area ready) is set AND `GetForegroundWindow()==game window` (so a real alt-tab is never overridden), call `AppActivated 0x40BCF0` (max 5 tries per load, >= 2 s apart, logged as `FIX AUTO`). If the window is not foreground it does nothing and logs why once. The engine hides the load screen itself on the next area-finish pass, since `a1b4e0` is now 1.
**Not done on purpose:** no patch of the hide condition (`IsMoviePlaying() || a1b4e0`), since that would also dismiss the screen while the user truly alt-tabbed.
**Verification done (Wine harness `perf_lab_src/harness/smoke_probe13.cpp`, fake engine globals at their real addresses):** `off` -> a1b4e0 stays 0 for 400 frames, only log lines; `auto` (fixdelay 1500) -> `FIX AUTO` after 1508 ms, a1b4e0=1; `manual` -> trigger file consumed, `FIX MANUAL`, a1b4e0=1; WM_MOUSEMOVE not logged; the report gets the `window / activation` section. **Bug found and fixed by the harness:** `GetWindowTextA` on a window of the same process sends WM_GETTEXT and blocked the watchdog forever (would have frozen the diagnostics exactly when the main thread stalls); titles are now read only for other processes.
**Install plan (needs the user's go):** stage from the CURRENT live stage (install-safety rules) with the new `LoadHangProbe.kpatch` (adds hooks 0x40A750, 0x40BCF0, 0x40BC40: 60 -> 63 hooks); `hangprobe_debug.txt` keeps `fix` OFF for the first observation runs; enable `fix=auto` only after logs confirm the mechanism or you say so.

