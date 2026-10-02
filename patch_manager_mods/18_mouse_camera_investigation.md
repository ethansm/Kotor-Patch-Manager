# 18 - Mouse / camera control investigation (right-drag rotate, double-click move, jumpy camera, stuck pitch)

Status: **INVESTIGATION ONLY (2026-10-02, game closed, no live change, nothing installed, no commit).** Written for a new-session hand-off.
Legend: **[V]** verified on disk/log this session, **[H]** from repo docs/memory/DB (not re-verified now), **[S]** speculation.

## 1. Symptom (user report, 2026-10-02)

- Mouse camera control "broken for a little while": right-click-drag no longer rotates the camera like originally; double-click-to-move does not work like originally.
- Camera is very jumpy regardless, and gets stuck looking up or down (pitch pinned).
- User changed `Mouse Sensitivity` in `swkotor2.ini` 44 -> 17 while coping. Prior hypothesis (unverified): Wine viewport / mouse-capture bug.

## 2. What changed + timeline

| When | Event | Source |
|---|---|---|
| 2026-09-23 14:01-14:45 | Fresh Steam install, KPM framework installed (`CreatedAt 2026-09-28` in `kpm_install_state.json` = re-created later). `swkotor2.ini` vanilla: Sensitivity=44, Reverse Ingame YAxis=0, Mouse Look=0, 640 wide | [V] `backups/vanilla_clean/swkotor2.ini` |
| 09-23 14:44 | Steam maps app 208580 to **Proton 9.0** (`proton_9`) | [V] `~/.local/share/Steam/logs/compat_log.txt` |
| 09-24 | ReShade 6.8.0 (`opengl32.dll`, dated Aug 2; backup `backups/reshade_20260924`) installed; 3840x1600 set; ultrawide GUI work (Width=3840 from 09-24 23:41) | [V] file dates, snapshots |
| 09-24..26 | Ultrawide GUI/world texture rollout (assets only; exe md5 unchanged `4AB72FC1...`) | [H] memory kotor2-ultrawide-gui |
| **09-26 (evening)** | **Camera spin / mouse already reported** ("mouse camera spin under Wayland", open issue in ultrawide master plan). Tried `MouseWarpOverride=force` (cursor pinned to centre) and `+ EnableHardwareMouse=0` (still moved the mouse); **both reverted at user's request**. Next options noted: gamescope, Proton Experimental | [H] memory kotor2-ultrawide-gui lines 64-79 |
| **09-26 23:30** | Steam remaps 208580 to **Proton Experimental** (now `experimental-11.0-20260924`, wine prefix `version` = 11.0-100) | [V] compat_log, `steam-208580.log` header |
| 09-28 15:25..09-29 20:29 | `Mouse Sensitivity` found as **4** (was 44 at 09-26 19:56; changed between 09-26 19:56 and 09-28 15:25, i.e. while the camera was already misbehaving) | [V] snapshot inis |
| 09-28 15:43 | KotorPatcher.dll / binkw32 / sqlite3 reinstalled (framework rebuild) | [V] mtimes |
| 09-29 20:29 | Sensitivity restored 4 -> 44 on user OK (shield-bar session) | [H] doc 08 line 349; [V] `backups/ini_pre_sensitivity_restore_20260929_202924` |
| 09-29 23:xx..10-02 | Mod waves: hotbar, shield bar, perf-lab (09-30 01:15, 0.3.0 at 21:17), load-hang-probe, buff HUD, effects panel, activation fix (10-01 19:14), companion/ranged AI (10-01 23:36), level-up override (10-02 00:54), L1/L4 engine fixes (10-02 01:33). Last install 10-02 02:53 (28 ids) | [V] `kpm_install_state.json`, git log |
| 10-02 (before 02:39) | `swkotor2.ini` now Sensitivity=**17**, **Reverse Ingame YAxis=1** (vanilla 0), Action Menu=1 / New Target Selected=0 (user-changed in-game options) | [V] diff vs `swkotor2.ini.bak_20260926_hwmouse` |

**Timeline conclusion [V/H]:** the camera/mouse problem is **older than almost every injected patch** (documented 2026-09-26, with Proton 9, ReShade, ultrawide assets, and none of hotbar/perf-lab/probe/activation/engine-fix DLLs). It cannot be "caused by" the 28-id set alone; the current patch set could only add to or mask it. "A little while" most plausibly equals 09-26 onwards. The user never had a confirmed-good mouse session after the Wayland/ultrawide setup on this machine (not confirmed, ask: Q1).

## 3. Evidence gathered

### 3.1 Environment [V]
- Session: **Wayland (COSMIC), XWayland `DISPLAY=:1`**, game runs through `winex11.drv` (log: `Loaded winex11.drv`; no winewayland). `PROTON_ENABLE_WAYLAND` not set.
- Monitor: Acer XR382CQK, DP-3, 3840x1600 @ **74.48 Hz** (X reports), ini `RefreshRate=60`, `V-Sync=1`, MangoHud `fps_limit=60` and `vsync=0` (per `~/.config/MangoHud/wine-swkotor2.conf`), `taskset -c 0,6` pinning.
- Input hardware: Corsair M65 RGB Elite through **ckb-next** (`ckb-CORSAIR_M65_RGB_ELITE_Gaming_Mouse_vM_-event` userspace virtual mouse) plus Razer BlackWidow keyboard (its `if02` mouse interface also present).
- Steam launch options (localconfig.vdf, user 62703561): `PULSE_LATENCY_MSEC=60 PROTON_LOG=1 PROTON_LOG_DIR="/home/ethansm" MANGOHUD_CONFIGFILE=... gamemoderun taskset -c 0,6 mangohud %command%`. `gamemoderun` logs `libgamemode.so: cannot open shared object file` (harmless, but the wrapper chain is long). No `PROTON_NO_ESYNC`, no `SDL_*`/`WINE*` input vars.
- Proton: Experimental 11.0 (build 20260924), ntsync active. Prefix `user.reg` has **no** DirectInput / MouseWarpOverride / DXGrab / GrabFullscreen / DllOverrides for dinput8 under `Software\Wine\DirectInput`, `X11 Driver` or app defaults for swkotor2 (grep only found other games' AppDefaults). `Control Panel\Mouse` is stock (MouseSpeed=1, thresholds 6/10, Sensitivity 10). **There is no `dinput8.dll` in the game dir**; the only proxy DLLs are `binkw32.dll` (KotorPatcher loader), `opengl32.dll` (ReShade), `mss32.dll`, `Patchw32.dll`.
- Engine mouse options in `swkotor2.ini` [V]: `EnableHardwareMouse=1` (OS cursor, not software cursor), `Mouse Look=0`, `Mouse Sensitivity=17`, `Reverse Mouse Buttons=0`, `Reverse Ingame YAxis=1`, `Enable Mouse Teleporting To Buttons=1`, `Keyboard Camera DPS=200 / Accel=500 / Decel=2000`, FullScreen=1, 3840x1600, Frame Buffer=1, V-Sync=1, Anisotropy=16, AA=4.

### 3.2 Strongest single clue: Wine receives ABSOLUTE mouse events [V]
`~/steam-208580.log` (last run, ends 10-02 02:39): **6,941 lines** of
`fixme:dinput:dinput_mouse_rawinput_hook Unimplemented MOUSE_VIRTUAL_DESKTOP flag`
out of 35,420 lines, thread 0x228 (the game's DirectInput mouse poll). This fixme is hit when Wine's DInput mouse path (`dinput/mouse.c`) is fed raw-input mouse records flagged `MOUSE_MOVE_ABSOLUTE | MOUSE_VIRTUAL_DESKTOP` instead of relative ones. The engine reads the mouse via DirectInput8 (ReShade log shows `DirectInput8Create` + `IDirectInput8A::CreateDevice(GUID_SysMouse 6F1D2B60..)`, 4 times in the last run), and swkotor2 mouse-look/camera drag consumes **deltas**. If Wine converts absolute positions to deltas, camera rotation will (a) depend on desktop position, (b) saturate/stop when the cursor reaches a screen edge or top/bottom (matches "stuck looking up/down", especially with a 1600 px tall screen), (c) jump when the cursor is re-centred/warped (matches "jumpy"), (d) feel broken at any sensitivity. This is consistent with the 09-26 trial where `MouseWarpOverride=force` visibly pinned the cursor to the centre but the camera still misbehaved. Whether this is the root cause is **[S]**: the log is from one run only and there is no copy from the Proton 9 era to compare (the Proton 9 logs were overwritten; `PROTON_LOG` writes one `steam-208580.log`).

### 3.3 Our patches that sit on the input path [V static, from `patch_config.toml` + sources]
No active patch calls `ClipCursor`, `SetCursorPos`, `ShowCursor`, `SetCapture`, `ReleaseCapture`, `GetRawInput*`, `RegisterRawInputDevices`, `mouse_event` or `SendInput` (grep over all `patch_manager_mods/*_src`, excluding harness/tests: zero hits). No patch writes or reads the camera object or the mouse-sensitivity/look globals either. Input-adjacent code that does exist:

| Patch (id) | Address / mechanism | What it does to input | Mouse-look risk |
|---|---|---|---|
| force-power-hotbar | detour `0x0072CB63` (`CExoInputInternal::GetEvents+0x383`, the DInput record loop); `CaptureHotbarKey(rec, dev)` | returns immediately unless `dev == keyboard device (0)`; only keyboard digits/Esc/modifier records are zeroed. Uses `GetAsyncKeyState` and, for hover, `GetCursorPos`+`ScreenToClient` | **Low.** Mouse records (dev != 0) are never touched [V code]. Doc 07 line 429 records an earlier "stuck right mouse button" the user saw as the game's own mouse-look setting [H]. |
| force-power-hotbar, inventory-3d-viewport, equipment-3d-character, character-effects-panel | each does `SetWindowLongPtrA(GWLP_WNDPROC)` to chain a `wheelProc` onto the game window (old proc `0x0040A750`) | handles **only `WM_MOUSEWHEEL`**, and returns 0 only when the cursor is inside that mod's rect while its panel ticked < 300 ms ago; everything else `CallWindowProcA(old)` | **Low**, but 4 stacked subclasses on one window; unsubclass is "only if still on top". Not a mouse-move/button path. |
| inventory-3d-viewport / equipment-3d-character / effects panel | `GetCursorPos`+`ScreenToClient` polling; `GetAsyncKeyState` for LMB/RMB drag over their own 3D view (RMB drag = zoom in inventory view) | read-only | Low (read-only, only active over their own panels) |
| load-hang-probe | 13 hooks incl. `0x40A750` (WndProc), `0x40BC40/0x40BCF0` (app deactivate/activate), `0x40B4AD`, `0x40C6B3`, `0x40D637/0x40D692/0x40BF40` (movie window), `0x781BE0` MainLoop, `0x798xxx` | observation + logging of WM_ACTIVATEAPP / MainLoop gating | Low-medium: sits in WndProc/activation, where cursor grab is re-armed. Probe is slated for removal (doc 10 Phase 6). |
| k2-activation-fix | `0x0040D69E` (movie thread `CALL [PostMessageA]`) replaced | drops one synthetic `WM_ACTIVATEAPP(0,0)` while our process owns the foreground; installed 10-01 19:14 | Low; only affects the end of the intro movie and does not postdate the symptom. |
| perf-lab | 46 counter hooks (alloc/render/AI) | counters only; no input addresses (checked address list: no 0x40A750/0x72Cxxx/0x7B38xx) | Very low (perf side effect only) |
| k2-visible-console, post-combat-movement-fix-v1, k2-finesse-melee-fix | console / party movement after combat / feat fix | `post-combat-movement-fix-v1` touches movement-order logic | Low [S]: affects what happens after combat clicks, not the camera drag; worth one A/B because the user cites click-to-move |
| semi-transparent-letterbox, k2-dialogue-letterbox-fix (0x411AC1) | GUI geometry/letterbox | cinematic bars only | Very low |
| companion-ai, ranged-ai, level-up-override, k2-wrap-measure-fix, equip-icon-fix, inventory-sort-filter | AI/script/GUI | none | None identified |

Note [V]: the exe is unmodified (`CurrentHash == OriginalHash == 4AB72FC1...`); ultrawide = ini resolution + GUI/world textures and the sibling `k2-aspyr-map-aspect-fix`; **no ultrawide viewport/projection/glViewport patch exists in the exe or the DLL set**. The engine renders at 3840x1600 = the X11 desktop size, so there is no viewport/client mismatch for the mouse in fullscreen [S: unless COSMIC scales; xrandr says 3840x1600+0+0, scale 100%].

### 3.4 Other injectors [V]
- **ReShade 6.8.0** (`opengl32.dll`) hooks `user32.dll` (log: "Found 21 match(es)" = cursor/input functions, `GetCursorPos`, `SetCursorPos`, `ClipCursor`, `GetRawInputData`, `RegisterRawInputDevices` family are in ReShade's input hook list) and wraps `dinput8` (`DirectInput8Create`, `IDirectInput8A::CreateDevice` redirected). `ReShade.ini`: `[INPUT] InputProcessing=2` (meaning of 2 not re-checked, **[S]** blocks/filters input while the overlay is shown; the overlay hot-key is `KeyOverlay=36` = Home, KeyEffects=35 = End). ReShade installed 09-24, **before** the first documented spin (09-26), so it is an equal-standing candidate to Wine itself. `InputProcessing` and the dinput wrap were never A/B-tested.
- MangoHud (`mangohud` wrapper, Vulkan/GL layer, no input hooks but an overlay).
- `gamemoderun` (non-functional: lib missing).
- No Wine dinput8 override and no third-party input DLL.

### 3.5 Ghidra DB facts relevant to mouse/camera [H, query of `kotor2_ghidra_knowledge.db`]
- `CExoInputInternal::GetEvents` (contains the DInput capture record loop at `0x72CB63`, = function +0x383) (hotbar hook; keyboard device index 0, every keyboard record `dev=0`). Lesson 98.
- `WndProc 0x0040A750`, `AppDeactivateRequest 0x0040BDC0` (deliberate synthetic deactivate on Alt+Enter), `0xA1B4E0` app-active flag; lesson 146/180/235/238.
- `CClientExoAppInternal::HandleMouseOver 0x007B3A00`, `SetMouseCursor 0x007B3870`, `ActionIconToMouseCursor 0x007B3EE0`, `CSWGuiMainInterface::HitCheckMouse 0x0074B3A0` (cursor/UI side, not camera).
- INI getters for `"Mouse Look"` and `"Mouse Sensitivity"` are read by `FUN_00733570(name)` (research/companion_levelup/phase3_evidence/A2_tagxrefs_autolevel.txt lines 374/486); the consumers (camera yaw/pitch integration, double-click-to-move timer, RMB drag state) are **not identified in the DB**. No lesson exists about the camera mouse-delta handler. Lesson 144: when the window loses focus the loop runs ~900 fps and the hotbar log shows "cursor garbage" [H] (focus loss changes cursor values: a possible lead for "jumpy").
- Lessons 96/103: during an active **item shield** the 3D scene blacked out and "mouse camera affected" (09-29, before shield-bar fixes) [H]. Treat shield state as a confound when testing.
- Wine peculiarity (lesson 235/238): WM_ACTIVATEAPP is sent between threads of one process (movie thread); relevant because Wine's X11 pointer grab/ungrab follows activation [S].

### 3.6 Config facts that could amplify jumpiness [V]
- 74.48 Hz monitor vs engine `RefreshRate=60` + V-Sync=1: fixed 60 Hz frame cadence on a 74.48 Hz X mode can produce uneven present timing (a known jitter source), independent of the mouse. [S that it matters for camera feel]
- `taskset -c 0,6` pins the whole Wine process tree (including the DInput mouse thread, wineserver children inherit) to one physical core's two threads; the game is CPU-heavy (prior notes: audio crackle after the world rollout, perf-lab "menu stutter"). A starved input thread produces batched mouse deltas = "jumpy" [S].
- ckb-next at high polling (1000 Hz) feeds Wine many tiny events; under Proton's raw-input path the cost is per-event [S].

## 4. Hypotheses, ranked

| # | Hypothesis | For | Against / unknown | Confidence |
|---|---|---|---|---|
| H1 | **Wine/XWayland delivers absolute pointer data (no relative-motion grab) to dinput, so the engine's delta-based camera saturates at screen edges and jumps on warp.** Root = Wayland/XWayland + Wine cursor grab, not our patches | 6,941 `MOUSE_VIRTUAL_DESKTOP` fixmes in the latest log; symptom "stuck looking up/down" (hits 1600 px top/bottom) and "right-drag rotate wrong" are exactly absolute-vs-delta signatures; reported 09-26 before any current patch; `MouseWarpOverride=force` test changed cursor behaviour but not the camera; Proton 9 -> Experimental did not cure it | Only one log, no Proton-9 comparison, no log of actual delta values; fixme text may also appear for working relative paths in this Wine version | **High (~50%)** |
| H2 | **ReShade 6.8.0 input hooks (user32 + dinput8 wrap, InputProcessing=2) interfere** with clip/warp/raw input | Hooks user32 cursor/raw functions and dinput8 devices; installed 09-24 and the first spin report is 09-26; nobody A/B-tested it; a ReShade overlay (docked 822 px wide window) sometimes eats mouse input | Camera symptom could also be Wine-only; ReShade is widely used with DInput games under Proton | Medium (~20%) |
| H3 | **Fullscreen focus/grab state (Wine re-grabs on activate; engine's deliberate/synthetic WM_ACTIVATEAPP; Alt+Tab; COSMIC focus-follows)** leaves the cursor un-grabbed or warped | Lessons 144/146/180/235/238 show focus-state oddities (ungrab -> "cursor garbage", 900 fps); load-hang saga proves activation is unstable here; user report "double-click-to-move does not work" fits a cursor that is not where the engine thinks it is | The activation fix (10-01) addresses only the intro movie and post-dates the report; no direct log of grab state | Medium (~15%) |
| H4 | **Sensitivity/Y-axis/options interplay** (Sensitivity 4 -> 44 -> 17; Reverse Ingame YAxis flipped to 1; Mouse Look=0) makes camera feel inverted/slow/fast, plus keyboard-camera DPS | Ini diff shows 3 mouse-affecting changes by the user; Mouse Look=0 means free look is off (vanilla) | Cannot cause "stuck up/down" or broken double-click by itself; the spin pre-dates it. Probably a coping side effect, not the cause | Low as root (~5%), High as confounder |
| H5 | **Our own patches** (hotbar GetEvents detour, 4 stacked WndProc subclasses, probe WndProc hooks, post-combat-movement-fix) interfere | They are the only engine-side code on the input path; the 28-id set is new since 09-30 | Symptom older than most of them; mouse records skipped in hotbar hook (dev check); wheelProc consumes only WM_MOUSEWHEEL inside own rects; zero cursor/clip/rawinput API calls in all sources | Low (~7%), but cheap to exclude entirely with a stock-set run (see matrix row A5) |
| H6 | **CPU pinning/frame-pacing** (`taskset -c 0,6`, 74.48 Hz vs 60, V-Sync) makes mouse deltas bunch up (jumpy) | Pinning + audio crackle history; refresh mismatch | Does not explain stuck pitch or lost double-click | Low (~3%), contributes to "jumpy" |
| H7 | Wine regression in Proton Experimental 11.0 20260924 DInput/raw input (vs Proton 9.0-4, which is also installed) | Experimental is bleeding-edge; the fixme line is in the dinput hook of this Wine | Spin existed on Proton 9.0 too (09-26 pre-23:30), though the exact Proton-9 behaviour is not logged | Low-medium (~5%), covered by test B1 |

(Percentages are rough ordering aids, not measurements. **[S]**)

## 5. A/B test matrix (user runs later; game must be closed between runs; one change per run; use a fixed 30 s route: stand still, right-drag 360 deg yaw, drag up/down to extremes, release, double-click a floor point 10 m away, repeat once in menu->world)

Baseline first: set `Mouse Sensitivity=44`, `Reverse Ingame YAxis=0` (vanilla values) and keep them for every row. Backup `swkotor2.ini` before each edit. Do not use `PROTON_LOG` rows back to back without copying `~/steam-208580.log` (overwritten each launch; copy to `~/Games/KOTOR2_Mod_Workspace/playlogs_mouse_<tag>/`).

| ID | Change (one at a time) | How | Expected if the hypothesis is TRUE |
|---|---|---|---|
| A0 | Baseline: sens 44, Y reverse 0, current everything | edit ini only | Reproduce symptom; count `MOUSE_VIRTUAL_DESKTOP` lines (`grep -c`) in the log |
| A1 | **ReShade off** | rename `opengl32.dll` -> `opengl32.dll.off` (or launch option `WINEDLLOVERRIDES="opengl32=b"`), game dir only while closed | H2: camera normal. If unchanged, H2 ruled out |
| A2 | Windowed instead of fullscreen | ini `FullScreen=0` (both [Display Options] and [Graphics Options]) at 1920x1080 | H1/H3: camera differs (relative delta usually works when the window owns the pointer); if fixed, grab/focus issue |
| A3 | Resolution change | 2560x1440 or 1920x1080 fullscreen (note: GUI textures are ultrawide-tuned, only for mouse testing) | If symptom disappears at a lower size, viewport/desktop-geometry is involved; unchanged = not viewport-related (kills the original viewport hypothesis) |
| A4 | Proton version | Steam > Properties > Compatibility: `Proton 9.0-4` (installed, `proton_9`); also try `Proton Experimental` (current). Separate prefix per tool is NOT created automatically: Steam reuses compatdata/208580, so copy it first (`cp -a compatdata/208580 compatdata/208580.bak`) | H7: Proton 9 differs; if identical, it is not a Wine-version regression |
| A5 | **Stock patch set** (all non-vanilla DLLs out) | use the framework's stock restore (see doc 08/10 "rollback"; the 16-patch stock set `inv3d_live_patches` was used before) or kill switches below | H5: camera normal with 0 of our mods; if unchanged, our patches are ruled out entirely |
| A5a | Per-patch kill switches (sibling-owned, no code change) | `activationfix.txt` -> `enabled=0`; `perflab_debug.txt` -> rename `.off`; hotbar/inv3d/equip3d/effects: `enabled=0` in `inv3d.ini` / `equip3d.ini` / `chareffects.ini` (each has `enabled=`; `wheel_zoom=0` / `wheel_scroll=0` only drops the WndProc subclass), hotbar: see doc 07 for its off key; `load-hang-probe`: remove id via the framework; hotbar: remove id | Bisect H5 after A5 indicates ours; remove the 4 wheel subclasses together first |
| A6 | Wine mouse-warp knobs | registry `HKCU\Software\Wine\DirectInput` `MouseWarpOverride` = `disable` then `force` (value tested 09-26: `force`; also try `enable`), via `WINEPREFIX=... wine reg add`, backup `user.reg` first | If absolute-vs-delta (H1), `force` should stop edge saturation; the 09-26 trial says camera still bad, so a different result now strengthens H1 on Experimental |
| A7 | X11 grab | registry `HKCU\Software\Wine\X11 Driver` `GrabFullscreen`=`Y`, `UseXInput2`=`Y` | If H1/H3: relative motion appears, camera fixed; fixme count drops |
| A8 | Wayland native driver | launch option `PROTON_ENABLE_WAYLAND=1` (needs `DISPLAY` unset path; Proton Experimental only) | Native pointer-constraints/relative-motion => camera fixed (strong H1 confirmation) |
| A9 | Gamescope | launch `gamescope -W 3840 -H 1600 -f -- %command%` (own Xwayland, grabs pointer) | Camera fixed => compositor/XWayland grab issue |
| A10 | Decouple environment | remove `taskset -c 0,6`, `gamemoderun`, `mangohud` from launch options; `PROTON_NO_ESYNC=1` (+ `PROTON_NO_FSYNC=1` if needed; ntsync is active, so also `PROTON_USE_NTSYNC=0`) | H6: jumpiness disappears; others unchanged |
| A11 | Refresh/V-Sync | ini `V-Sync=0` and `RefreshRate=74` (or set compositor to 60 Hz) | Frame pacing changes the "jumpy" feel only |
| A12 | Mouse device | unplug/kill `ckb-next` daemon, use the Razer keyboard's mouse/another mouse; lower DPI/polling | If the Corsair/ckb-next virtual device matters (absolute-flavoured events), symptom disappears |
| A13 | Input env knobs | `SDL_VIDEODRIVER`/`SDL_VIDEO_X11_*` (not used by this game, skip), `WINE_*`: `WINE_CPU_TOPOLOGY` not relevant | Low value, listed only to say we considered them |
| A14 | Raw input registry | `HKCU\Software\Wine\DirectInput` `MouseWarpOverride`, `HKCU\Software\Wine\X11 Driver\UseXInput2` as above; there is no game-visible raw-input toggle | see A6/A7 |

Decision tree: A5 clean + A1 clean => our patches and ReShade ruled out => environment (H1/H3/H7): A2 then A6-A9. A5 clean but A1 differs => ReShade. A5 differs => bisect A5a.

## 6. Instrumentation ideas (nothing built; all additive, off by default, game dir changes only with the game closed)

1. **Mouse-record logger in the existing GetEvents hook** (`0x0072CB63`, hotbar DLL or a new tiny probe): for `dev != keyboard` log `dwOfs` (DIMOFS_X/Y/Z/BUTTON0..) and `dwData` with a timestamp, ring-buffered and flushed at 1 s, with counters: events/s, |dX| max, sign of Y accumulations. This directly shows whether DInput gives small relative deltas (healthy) or huge/absolute/repeating values (H1). Also log the device id used by the mouse (the `dev` index differs from 0) to verify there is a single mouse device. Needs one DB lesson (the mouse device array index) - first confirm from `CExoInputInternal` init.
2. **GetCursorPos / ClipCursor trace**: `GetClipCursor` + `GetCursorPos` + `GetForegroundWindow` once per second into the same log (read-only API calls, zero hooking), plus `GetWindowRect`/`IsIconic` of the game window: shows whether the pointer is clipped to the 3840x1600 rect (grab active) or free.
3. **WM_ACTIVATEAPP / WM_KILLFOCUS / WM_SETFOCUS / WM_MOUSEMOVE counters** via the existing `HangProbeWndProc` slot (load-hang-probe already hooks 0x40A750; add counters rather than a new subclass). Pair with the 900 fps focus-loss observation (lesson 144).
4. **Camera yaw/pitch sampler**: find the per-frame camera object (use `CSWGuiScene` camera vtable `0x0098c45c` as a lead for GUI cameras; the game camera class `CSWCCamera`-like/`SetViewAngle` slot 0x44 offset `+0x204` is known only for the GUI scene). Needs a **new Ghidra query first against existing dumps** (`ghidra_dumps/steam_6a522e71`): xref `FUN_00733570("Mouse Sensitivity")` and `"Mouse Look"` consumers (A2 evidence lines 374/486) to locate the mouse-delta handler and the RMB-drag/double-click state; no new headless run is allowed while the game is running (`pgrep -af "^S:.*swkotor2"`).
5. **Wine side**: `WINEDEBUG=+dinput,+rawinput,+cursor,+x11drv` for ONE 30 s run only (log size warning: the existing `PROTON_LOG` is already 3.5 MB per 70 s; `+rawinput` is huge). Cheaper: keep `PROTON_LOG=1` and add `WINEDEBUG=+cursor` only.
6. **Ini snapshot hook in the test script**: always `cp swkotor2.ini` before/after (the game rewrites it at exit; options changed in-game are persisted).
7. Record the Proton-9 vs Experimental comparison with the same route and count fixmes.

## 7. Open questions for the user

1. Was mouse look/double-click ever fine on this machine/session type (before 09-26), including on Proton 9 + Wayland? Since when exactly ("a little while" = since the ultrawide/ReShade work 09-24..26?).
2. Does it happen in menus/inventory 3D views, or only in the game world? Does the camera spin by itself with the mouse untouched (09-26 memory said "camera spin")? Does it correlate with Alt+Tab, a loading screen, or the shield effect (lesson 96/103)?
3. Which mouse is actually in use (Corsair M65 via ckb-next, or the Razer interface)? Polling rate/DPI? Any other pointer devices (touchpad)?
4. Is the ReShade overlay ever opened (Home key) in-game, and is ReShade needed? (A1 is the cheapest high-value test.)
5. Why `Reverse Ingame YAxis=1` and sensitivity 17: was that a deliberate preference? If not, the first run should use 44/0.
6. OK to use a throwaway copy of `compatdata/208580` for registry/Proton tests (A4-A8)? OK to remove `taskset`/`gamemoderun`/`mangohud` for one run?
7. Does the symptom also occur windowed, or on a second monitor/resolution?

## 8. Ground rules

- **Never touch the game dir or prefix while the game runs**: check `pgrep -af "^S:.*swkotor2"`; do not launch the game from an agent; no Ghidra headless while it runs (static background work only).
- All installs/ini/registry edits: back up first (`swkotor2.ini`, `user.reg`, `patch_config.toml`), md5 the DLLs before/after (sibling sessions reinstall patches), one change per run, record every run (date, change, result) in this doc's section 9 table.
- `swkotor2.ini` invariants noted by other sessions: `Mouse Sensitivity` (shield-bar doc 08 expects 44 as the baseline), `AR_ERROR.LOG` md5 `a2d887896b53eae113758dc8c3729db9` (doc 08).
- Do not re-add `MouseWarpOverride`/`EnableHardwareMouse=0` permanently (09-26: reverted at the user's request); test, log, and revert.
- Sibling-owned patches: use their documented kill switches only; do not edit their sources from this thread.
- Proton logs: copy `~/steam-208580.log` out before the next launch (overwritten); `~/.local/share/Steam/logs/compat_log.txt` holds the tool-mapping history.
- Prefer research/fixes outside the game dir first (this doc, DB queries, dumps).

## 9. Run log (to fill in)

| Date | Row | Change | Fixme count | Observation |
|---|---|---|---|---|
| - | A0 | - | - | - |
