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

Note [V] (as of 09-26): the exe was unmodified by us (`CurrentHash == OriginalHash == 4AB72FC1...`); since 10-02 the live exe is vanilla `6A522E71` + `4gb-patch` (`7751F7D8`, only the PE LAA flag differs); ultrawide = ini resolution + GUI/world textures and the sibling `k2-aspyr-map-aspect-fix`; **no ultrawide viewport/projection/glViewport patch exists in the exe or the DLL set**. The engine renders at 3840x1600 = the X11 desktop size, so there is no viewport/client mismatch for the mouse in fullscreen [S: unless COSMIC scales; xrandr says 3840x1600+0+0, scale 100%].

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

## Addendum 2026-10-02: user history (verified by user, answers "did it ever work?")
- It WAS working at one point BEFORE our patching. Sequence: broken initially -> it **fixed itself at random when the user took a screenshot with PrtSc in Pop!_OS (COSMIC)** -> at some point it **reappeared**.
- Interpretation (hypothesis, unverified): the screenshot tool/compositor grabs and then releases the pointer/keyboard focus, resetting a stuck XWayland pointer grab / focus / relative-motion state (Wine DirectInput exclusive-mouse acquire, `MOUSE_VIRTUAL_DESKTOP` path). Raises hypothesis 1 (XWayland/dinput state) and hypothesis 3 (grab/focus) above ReShade and above our patches. Also means the bug is state-dependent (what fixed/re-broke it may be a compositor/COSMIC update, Proton change, or window focus event) not a fixed config.
- Cheap tests to add to the A/B matrix: (1) when broken in-game, press PrtSc (or open/close the COSMIC screenshot UI) and see if it recovers instantly; (2) alt-tab out and back; (3) Super key/workspace switch; (4) note exactly which COSMIC/Proton/Steam update dates correlate with the reappearance (check apt history `/var/log/apt/history.log`, Proton version change 09-26 23:30); (5) F12 Steam screenshot vs PrtSc: does only the compositor one fix it?
- Workaround idea if confirmed: a launch wrapper/gamescope with its own pointer grab, or a script that triggers the focus reset (e.g. `xdotool`/wlr tooling) after load. Not tried.
- Open: when exactly did it reappear (date / what changed: system update, new patch, ReShade)?

## Addendum 2 (2026-10-02): focus/grab reset tests, NEGATIVE
- User ran the quick reset tests (PrtSc / screenshot UI, alt-tab and the other focus resets listed above) while the camera was broken: **none of them fixed it** (user's words: "none of them seem to fix it").
- Consequence: the "stuck pointer grab that a compositor screenshot resets" idea is weakened (the original one-time fix may have been coincidence, or a different state). Remaining leads: Wine/XWayland DirectInput absolute-vs-relative motion (`MOUSE_VIRTUAL_DESKTOP` fixme), ReShade 6.8.0 input hooks, Proton version, mouse driver (ckb-next/M65), refresh-rate/vsync/taskset interplay.
- Next cheapest tests (not run yet): ReShade disabled (rename its dxgi/d3d9/opengl32 + ini); windowed mode; another Proton version; stock patch set (all our DLLs off) to prove it's not ours; different mouse / ckb-next stopped.

## Engine-side findings (Ghidra, 10-02)

Static analysis of the Steam/Aspyr exe (hash 4AB72FC1...). Game was not running. All facts are in `ghidra_knowledge` (lessons scope `steam / mouse-camera`, functions rows, struct_fields). New helper scripts: `ListImportXrefs`, `DecompileRange`, `HexDump`, `FindBytesRefs`, `ScanFieldRefs` (in kotor-patch-manager/tools/ghidra_scripts). **[V]** = read from decompile/disasm this session; **[S]** = inference.

### E1. How the engine reads the mouse (corrects section 3.2/3.4 assumptions)
- **Split design [V]:** DirectInput supplies ONLY the per-frame relative motion; buttons, wheel, cursor position and double-click come from Win32 window messages.
  - `DirectInput8Create` (IAT 0x986000, thunk 0x93B3B0) has one caller, `FUN_00730750` (CDirectInput ctor). Keyboard: `GUID_SysKeyboard` @0x99CB88, `c_dfDIKeyboard` @0x9B932C, coop level 6 (FOREGROUND|NONEXCLUSIVE), buffered (256). Joysticks via EnumDevices, coop 10.
  - Mouse: `FUN_00731070` (lazy, called from the poll when `this+0x28==0`): `GUID_SysMouse` @0x99CB78, `c_dfDIMouse` @0x9B9534 (size 0x18, DIDF_RELAXIS, 0x10-byte DIMOUSESTATE, 7 objs), **coop level 10 = BACKGROUND|NONEXCLUSIVE**, buffer 256 set but unused, `Acquire` once at creation.
  - Per-frame poll: `FUN_0072F110` (wrapper 0x72FD90, only caller 0x7AC60B inside `FUN_007AC470` = ProcessInput(dt)) -> `FUN_00732E00` = **`GetDeviceState`** (vtable +0x24, immediate, not GetDeviceData), retry loop: INPUTLOST/NOTACQUIRED -> `Acquire` (+0x1C). Struct: lX @+0, lY @+4, lZ @+8, rgbButtons[4] @+0xC.
  - Acquire/Unacquire: `FUN_00733090(on)` (via 0x72DF60 <- 0x72FB20, called on app activate/deactivate paths) acquires/unacquires keyboard+mouse+joysticks and flushes buffers (0x7311C0/0x7312A0). Keyboard exclusive toggles 0x732F00/0x732F40.
  - Win32 side: WndProc `0x40A750` handles WM_L/R/MBUTTON DOWN/UP/DBLCLK (0x201-0x208) and WM_MOUSEWHEEL (0x20A); builds {msg, wParam, lParam with y flipped `H-1-y`} and calls `0x72FBD0` -> `CExoInputInternal::OnMouseMessage 0x72E030`, which stores WM_MOUSEMOVE position at `this+0x3E4/+0x3E8` and pushes button events (device `DAT_0099C888`=1; axes LMB `DAT_0099C89C`, RMB `C8A0`, MMB `C8A4`, wheel `C8B0`; value 1 down / 0 up / **-1 double click**) through `0x72E730` into the queue drained at the end of `GetEvents 0x72C7E0` (the hotbar hook sees only keyboard/joystick DI records, dev 0 and >=2; the mouse has no DI record list there).
  - Win32 cursor APIs [V]: `GetCursorPos` (0x72E850 in-client test, 0x72E8D0 autohide timer), `SetCursorPos` only in `0x72EB20` (wrapper 0x72FC80; callers 0x78A23B, 0x7AFBFE, 0x7B01E9, 0x4141CC, 0x414261 = free-look/GUI "teleport to button"; **no per-frame recentering in the normal chase camera**), `ShowCursor` in 0x72E8D0 (hide when idle > `DAT_00996630`), `SetCursor` 0x72EA70, `SetCapture` on every button DOWN (0x40A750), **no `ReleaseCapture`, no `ClipCursor`, no `GetDoubleClickTime`, no `GetRawInput*`, no `mouse_event` imports**. Window class style 0xB = CS_HREDRAW|CS_VREDRAW|**CS_DBLCLKS** (0x4087A0).
  - Consequence for H1/H2: the engine requests a RELATIVE-axis DInput mouse device in background, non-exclusive mode, so under Wine the delta stream is whatever Wine's dinput derives from X11/XInput2/raw input (the `MOUSE_VIRTUAL_DESKTOP` fixme path). ReShade's dinput8 wrapper sits between the two. Nothing in the engine can compensate for absolute-derived deltas.

### E2. Ini keys and the sensitivity path
- `swkotor2.ini` getters in `FUN_007B88B0` (load) / `FUN_007B5F50` (save): `"Mouse Look"` 0x9A03A0 (read at 0x7B8A61) -> options flags bit1; `"Reverse Ingame YAxis"` 0x9A03D8 (0x7B8D27) -> flags bit3; **`"Mouse Sensitivity"` 0x9A0364 (0x7B8EEB) -> options BYTE +0x58 (atoi, stored as a byte, no clamp)**; `"Reverse Mouse Buttons"` 0x9A03C0 (+0x5C; also 0x7DCF90); `"Enable Mouse Teleporting To Buttons"` 0x9A0340; `"Keyboard Camera DPS"` 0x9A0108; `"EnableHardwareMouse"` 0x99F810 (0x796FE1/0x7970E3). No "Allow Mouse Look" string exists.
- Delta normalisation `FUN_0072F110` [V]: `this+0x408 = clamp(-lX / 100.0f, -1, 1)`, `this+0x40C = clamp(-lY / 100.0f, -1, 1)` (constant `DAT_00A1023C` = 100.0f, read-only). **No accumulation, no smoothing, no history; a frame with |delta| >= 100 counts as 1.0.** On GetDeviceState failure both are 0.
- Sensitivity `FUN_007BBFF0` = `(byte/100.0)*45.0 + 10.0` (ini 44 -> 29.8, 17 -> 17.65, 4 -> 11.8). Used in the camera director `FUN_0078C350` (this+0x18 = camera controller): mouse yaw rate = `scale * dxNorm * (+-1 by DAT_00A7EBBC)` -> `FUN_007F3EA0(v, dt)`; pitch rate = `scale * dyNorm / 10 * (+-1)` -> `FUN_007F3C70(v)`. Options flag bit4 (0x10) negates dy (this is the in-camera "invert Y"; Reverse Ingame YAxis ties to the options flags). Mouse-look-active = `DAT_00A7FCBC XOR options bit1 (Mouse Look)`. With it false, a cursor at the screen edge rotates the camera (`FUN_007F4220`/`FUN_007F3DE0`; edge flags from the cursor position vs `DAT_00A1B49C`).

### E3. Right-drag camera state, yaw/pitch, what can stick
- Hold flag `DAT_00A7FCBC`: written by `HandleInputEvent FUN_007B12C0` cases **0xDB/0xDC** (`= value`, 1 pressed / 0 released; which mouse button is bound is data-driven, not traced). Also cleared by `0x40A460` (app deactivate), `0x530170`, `0x7B3830`, event 0x107, internal 0x7B220F/0x7B25EA; set by 0x7B3810. **A lost release event (WM_RBUTTONUP never delivered, cursor outside window, capture games) leaves it 1 => camera keeps following dx and the cursor stays hidden** [S, mechanism V].
- Angle storage: controller `ctl = *(director+0x18)`, mode `ctl+0xC` (3/5/7 special cameras, else default chase). `AddYawInput 0x7F3EA0` adds into the camera data struct `(*(ctl+0x40))->vt[0x80](-1)->vt[0x1C]()` field **+0x40** and mirrors it to `ctl+0x98`; `AddPitchInput 0x7F3C70` adds into field **+0x48**, mirror `ctl+0x94`; `AddKeyboardYaw 0x7F44B0` adds field +0x34, mirror `ctl+0x9C`. The chase update `FUN_007F7A30` (via 0x7F4C30) computes `yaw = camobj.vt[0x64]() + ctl[0x98]*dt`, `pitch = camobj.vt[0x68]() + ctl[0x94]*dt` and **clamps pitch to [`DAT_00A10B24`=1.0, `DAT_00A10B28`=50.0]** (distance `vt[0x6C]` to [5.0?, 20.0] via `A10B1C/A10B20`). The mouse term is therefore a RATE multiplied by dt, not a direct angle.
- Stuck pitch candidates: (a) pitch pinned at the 1.0/50.0 clamp while a drag direction keeps pushing (absolute-derived dx/dy that never returns to 0 does exactly this); (b) NaN: the clamp is a compare chain (`max <= v ? max : v; min <= x ? x : min`) that maps NaN to the MIN bound; (c) hold flag stuck (above). **Open:** the reset site of camdata+0x40/+0x48 (the `+=` in default mode with `+= 0` on idle frames) was NOT found; if they are never reset the camera would drift, so a reset must exist elsewhere; the probe's per-frame log of `ctl+0x94/+0x98` answers it.

### E4. Double-click-to-move
- Detection is **OS-side**: class has CS_DBLCLKS, so Wine/Windows emits WM_LBUTTONDBLCLK (0x203) from the system double-click time/size; the engine has no timer or `GetDoubleClickTime`. `0x40A750` -> `0x72E030` converts it into LMB-axis event value **-1** (RMB dblclk 0x206 likewise on the RMB axis) and `SetCapture`. The -1 event is mapped by the action table inside GetEvents and ends up in `HandleInputEvent 0x7B12C0` / ProcessInput; the concrete move-to-point action was not traced. Any condition that makes Wine not synthesize 0x203 (pointer moved > SM_CXDOUBLECLK between clicks because of delta/absolute confusion, or button messages split across threads) kills double-click move. The `SetCursorPos` guard `this+0x3EC=1` makes the NEXT WM_MOUSEMOVE position be ignored once (button events still pass).

### E5. Frame timing
- Per-frame dt for ProcessInput/camera comes from the main loop (`FUN_00407920`: `FUN_00736220` time counter -> `DAT_00A1B728/72C` seconds; frame limiter busy-waits on `DAT_00A1B710`; `timeGetTime` only for the FPS counter). The raw mouse delta is NOT dt-normalised (GetDeviceState returns motion since the previous poll, clamped to +-100 counts), but the camera applies it as `rate*dt`, so uneven frame pacing AND batched deltas both show up as angular-velocity jitter. Mouse look in modes 3/5 additionally multiplies by dt inside `AddYawInput`.

### E6. Recommended probe hook points (read-only; format = existing `*.hooks.toml`, all sites are whole-instruction, no relative operands)
```toml
# raw DIMOUSESTATE, only on GetDeviceState success. [ebp-0x14]=lX [ebp-0x10]=lY [ebp-0xC]=lZ [ebp-8]=buttons[4], this=[ebp-0x28]
[[hooks]]
address = 0x0072F14E
type = "detour"
function = "ProbeMouseRaw"
original_bytes = [0x8B, 0x45, 0xEC, 0xF7, 0xD8]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "pointer"

# end of PollMouseDelta (success + failure paths merge): this+0x408/+0x40C = final normalised floats, this=[ebp-0x28]
[[hooks]]
address = 0x0072F2EC
type = "detour"
function = "ProbeMouseNorm"
original_bytes = [0x8B, 0x4D, 0xFC, 0x33, 0xCD]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "pointer"

# camera director per-frame, right after GetMouseDelta: [ebp-0x30]=dx float [ebp-0x50]=dy float [ebp-0x84]=sens scale float [ebp-0x130]=this
[[hooks]]
address = 0x0078C452
type = "detour"
function = "ProbeCameraFrame"
original_bytes = [0x8B, 0x0D, 0xA4, 0xB4, 0xA1, 0x00]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "pointer"

# optional angle-input sinks (entries): ecx=ctl, [esp+4]=float (yaw 0x7F3EA0: [esp+8]=dt)
# 0x007F3C70 pitch   55 8B EC 83 EC 1C            (6)
# 0x007F3EA0 yaw     55 8B EC 81 EC 9C 00 00 00   (9)
# 0x007F4220 edge    55 8B EC 83 EC 48            (6)
# 0x007F44B0 kbd yaw 55 8B EC 83 EC 0C            (6)
```
Per-frame sampler contents (ProbeCameraFrame, via `ebp`): dx/dy/scale floats; `this=*(void**)(ebp-0x130)`, `ctl=*(void**)(this+0x18)`, `camobj=*(void**)(ctl+0x40)`; log `ctl+0xC` (mode), `ctl+0x94/0x98/0x9C`, `DAT_00A7FCBC` (hold), `DAT_00A7FCC0`, `DAT_00A7EBBC`, options flags (`*(CExoInput...)` via `FUN_0072FB00()` +0x8) and call the float getters `camobj->vt[0x64]/[0x68]/[0x6C]` (thiscall, float in ST0; they are the same getters 0x78C350 itself calls at its end). Use QueryPerformanceCounter timestamps; also log the DI raw buttons byte (RMB = rgbButtons[1] bit 7) next to the hold flag to catch release desyncs. The dx/dy hooks run on the game main thread (same as GetEvents).
Doubts: the camera-object vtable getter slots and `ctl+0x40` layout are inferred from the call pattern in 0x78C350/0x7F7A30 (not RTTI-named); the bound mouse button for events 0xDB/0xDC and the move-to-point action are untraced; `A10B1C..28` pitch/distance globals' writers not traced.


## Planned shared state and shared code (doc 19 / 19b, added 2026-10-02)
**Nothing here is built yet.** When a piece lands, use it instead of extending this patch's local copy. The ids (A1, B2, ...) refer to [`19b_cross_patch_sharing_survey.md`](19b_cross_patch_sharing_survey.md), which has the evidence. Line numbers there are from 10-02.
- **Registry interfaces** (A-ids) use the RFC #180 registry from [`19_shared_logging_design.md`](19_shared_logging_design.md). Call `require` lazily at first use, and keep the current local code as the fallback when the provider is not installed.
- **Common helpers** (B-ids) live in `Patches/Common` on the KPM fork. Edit them in `patch_manager_mods/_shared_src/` and sync with `scripts/check_shared_headers.py --sync`. New code includes `"SafeRead.h"`, not `"../_shared/SafeRead.h"`.
- **Rollout:** shared pieces are consolidated on our fork first. Each one goes upstream inside the first mod PR that uses it, so order this mod's PR after the PR that introduces anything it reuses. Migrating this patch needs its owner's go.
- **Logging:** move to `KpLog.h` channels and `kplog.ini` (doc 19 Phase 6). Keep the tag text exactly as it is, because the parsers depend on it.
- **Mouse probe:** the doc 19 Phase 2 reference migration to `KpLog.h`. If it ever needs window or cursor state, use A1 `k2.guiinput` rather than another window subclass.


## Probe run 1 results (10-02)
Log: `backups/shield_bar23_install_20261002_151504/mouseprobe_log.txt.run22` (mouse-probe 0.1.0, 3 START sessions, 21k lines, ~15.8 h wall span but only 370 POLLS seconds).
- **POLLS:** 370 s logged; polls/s min 11 / median 60 / max 61 (display-rate poll, healthy). Max gap per second: p25 18.2 ms, median ~22 ms, 26 s with gap >50 ms, 43 s >30 ms. Worst gaps (21.97 s, 77.3 s, 44.6 s, plus 5.8 s/5.6 s/0.76 s) are loading screens or pauses, not a polling failure. Zero-poll share 61.9% (idle mouse).
- **RAW:** 8163 non-zero-or-button-change samples. 7544 (92%) have |lx| or |ly| >= 100; |lx| median in the thousands (4577 samples 5000+, 2554 in 1000-5000), max 528820 (ly 299228). Signs: lx negative 7543/8163, positive 3; ly negative 7546, positive 1. Net sum lx -1.02e8, ly -5.6e7. This is not plausible relative motion. Values come in quantized steps (-1375, -2750, -4125, -5496; -4652/-2326/-9295) and ly/lx is a near-constant ~0.36-0.43 ratio, so it looks like a fixed-direction vector (cursor offset from a warp/recenter point, or Wine absolute-to-relative with ~1160-1375 units per pixel) rather than hand motion. Sign flips are almost absent (5 in 8163), so it is a drifting ramp, not alternating jitter. 6928 of 7551 non-zero samples have no button down, so non-zero lx/ly without any button is the normal state. Bursts right after a gap: -279277/-61357 right after the 21.97 s gap, -528820/-299228 and -475546/-255284 are the largest.
- **Buttons:** mask 0x80 (rgbButtons[0], LMB): 117 downs / 117 ups, median hold 84 ms, no lost release. 0x8000 (rgbButtons[1], RMB): exactly one press, 7 samples (~130 ms), release seen. Final mask 0. No lost release found.
- **NORM:** 7551 non-zero samples, 99.9% SAT (nx=ny=1.0000 in 7461); only a handful of unsaturated values (0.43, 0.86, 0.40). The engine normaliser is pinned at the clamp.
- **CAM:** 5236 lines, 7 with drag=1 (single RMB drag at ts 2842726423.7, 130 ms, DRAGSTATE 0->1 rmbDown=1 then 1->0 rmbDown=0, matches raw). During it dx=dy=1.0 (saturated) for 5 frames then 0.0. Yaw and pitch (ctl+0x98/+0x94) read 0.0000 in all 5236 lines, never move: pitch never hits 1.0/50.0 and never NaN/inf. So the offsets read by the probe are probably not the live angles (or mode=3 ctl is not the camera controller). No STUCKDRAG? lines; the stuck-up/down symptom did not occur in the logged windows (or is not visible in these fields). scale constant 17.650. 258 lines with ctl unreadable, mode=-1 (startup).
- **Top windows:** (1) 2842726400-2842726553, the RMB drag: lx -68796..-48482 with NORM saturated, camera never rotated. (2) 2841882923 burst -279277/-61357 after 21.97 s gap. (3) 2842025887 -528820/-299228 after the 77 s gap. (4) 2842753554 -475546/-255284. (5) 2842220304 44.6 s gap with 14 of 15 polls zero.
- **Verdict:** the input already arrives broken at the raw DirectInput level (huge, one-signed, quantized, absolute-or-offset-like deltas), well before the engine. NORM/CAM saturation is a consequence. The pitch/yaw fields need re-verifying before blaming engine camera code.
- **Next A/B:** (a) run with game window focus/cursor grab variations (windowed vs fullscreen, Proton `PROTON_USE_WINED3D`, `winex11` raw-input / DInput relative-mode on and off, `SDL_VIDEO_MINIMIZE...` not needed) and watch whether lx/ly sign mix becomes normal. (b) Log SetCursorPos/ClipCursor/ShowCursor calls and GetCursorPos next to RAW to test the warp-offset theory; check whether deltas equal (cursor - window centre) x constant. (c) Log pitch via the 0x7F3C70 sink arg instead of ctl+0x94, and a RAW-sampled run with no loading screens, doing a deliberate slow right-drag look. (d) Mark the moment of the user-visible bug (hotkey writing a marker line) so windows can be correlated.


## 10-03 mouse-probe 0.2.0 (KpLog)
Reference migration to `KpLog.h` (19e chunk M, guide 19f). **INSTALLED 10-03 21:02** (live DLL md5 c4fa4f8f, `[mouse-probe]` in kplog.ini, backup `backups/mouseprobe_020_20261003_2102`) from `~/Games/KOTOR2_Mod_Workspace/stage_mouseprobe_020/MouseProbe.kpatch` (sha256 `483eaec862318af96bf403b99da01ce162049c20c90e11b67ea4343e3acdd102`). Source: `mouse_probe_src/` (synced to the fork's `Patches/MouseProbe/`).
- **Log file:** new core (registry + `kpatch.log`) writes `mouse-probe_log.txt`. Old core (live Sep-28 build, no service) writes `mouseprobe_log.txt` through the header's fallback sink, same name and 60000-line cap as 0.1.0.
- **Line prefix:** new core `T=<ms> f=- mouse-probe/<ch> <text>` (`f=` is `-` because the probe never calls `KP_FRAME_TICK`). Old core `<ms32> mouse-probe/<ch> <text>`, where `<ms32>` is `GetTickCount()` ms, not the old raw-QPC `%.3f`. A parser that anchored on the QPC float needs `strip_prefix` (or a regex that allows both shapes). Tag words and field layouts are unchanged.
- **Channels:** `main` START (new core: once, on the first hook call, no STOP; old core: START/STOP when `mouseprobe.txt` appears/disappears), `raw` RAW, `polls` POLLS, `norm` NORM, `drag` DRAGSTATE + STUCKDRAG?, `cam` CAM.
- **STUCKDRAG? is now WARN** (on `drag`), so it is written even with no `kplog.ini`. Drag state and the RMB bit are always tracked, even with every channel off. On the old core with no `mouseprobe.txt` it therefore also lands in `mouseprobe_log.txt` (fallback level WARN).
- **Old core flag file:** `mouseprobe.txt` only runs when there is no core; it sets the fallback level to INFO (present) or WARN (absent). On a new core it is ignored.
- **kplog.ini** (game dir, re-read about once a second):
```ini
[global]
enabled=1
sink=perpatch
; RAW + CAM can pass 2000/s; 0 = unlimited (still look for DROPPED)
max_lines_per_sec=0

[mouse-probe]
enabled=1
level=info
; or *,-polls to drop a channel
channels=*
```
- **Build notes:** DLL 80 KB -> 89 KB (KpLog.h uses `GetTickCount` on Windows; a first build with `std::chrono` was 181 KB and imported `SetSystemTime` etc., fixed in KPM 22add9a). Exports: `KPatchInit`, `ProbeCameraFrame`, `ProbeMouseNorm`, `ProbeMouseRaw`. `-DMOUSEPROBE_FRAMETICK` (off) adds `KP_FRAME_TICK()` to the camera hook, for tests only. Fork build source committed on `ethansm/local-mods` (see 19e chunk M).

- Note (log analysis 2026-10-04, session 10-03 evening, mouse-probe c4fa4f8f): over the full session NORM is 23773/23820 SAT (99.8%), raw deltas up to -2244/-1120. This confirms the line-260 finding at scale. Report `play_session_reports/2026-10-03_evening.md`.

## RESUME HERE (2026-10-04): double-click-to-move logging
Direction: doc 28's point-and-click mod is **dropped**, because vanilla already has double-click-to-move (doc 28 §6, c0b486d). The new goal is to find out with logs why vanilla double-click move no longer works. mouse-probe 0.2.0 only hooks DirectInput (0x72F14E/0x72F2EC/0x78C452), so it sees LMB down/up but not the Win32 WM_LBUTTONDBLCLK path (WndProc 0x40A750 -> OnMouseMessage 0x72E030 value -1 -> GetEvents 0x72C7E0 -> HandleInputEvent 0x7B12C0 -> walk dispatch, not traced). Plan: mouse-probe 0.3.0 adds a `click` channel along that chain. Handoff: `.claude/handoffs/18_mouse_camera_investigation.handoff.md`.

## Double-click chain (traced)
Static trace 2026-10-04 from the Steam dump (`ghidra_dumps/steam_6a522e71/decomp`). [V] = asm read, [S] = inferred. Names come from the Steam DB (B11a). Nothing was run.

### Chain
1. **Win32 -> axis event [V].** `OnMouseMessage 0x72E030` maps 0x201/0x202/0x203 to LMB axis `DAT_0099C89C` with value 1/0/**-1** and queues it through `0x72E730` (device `DAT_0099C888`).
2. **Axis -> action id [V].** `0x785DF0` (Steam `SetEventDescriptions`, 12175 B) registers the bindings with `0x72F8C0(action, type=1, DAT_0099C888, axis, 0x86)`:
   - LMB `DAT_0099C89C` -> action **0x19** (GUI input class) and action **0x43** (in-game input class).
   - RMB `DAT_0099C8A0` -> 0x1A / **0x44**. Middle `DAT_0099C8A4` -> 0x45. Wheel -> 0x46. Mouse X/Y `DAT_0099C8A8/AC` -> 0x17/0x18 (GUI) and **0x41/0x42** (game).
3. **GetEvents `0x72C7E0` [V].** It drains the OS queue, matches the binding by axis (`binding+0x1C`) and posts `0x72F540(value, a, b, bindingObj)`. The value stays -1 for a double click.
4. **ProcessInput `0x7AC470` [V].** In the event loop (asm `007ACE0A`) it reads `value = event[0]` and `action = *(bindingObj+0x10)` and calls `HandleInputEvent 0x7B12C0(action, value)`. This happens only when `action` is outside 0x27..0x40. Actions 0xB4..0xBB, 0xCE, 0xDF, 0xF3, 0xF4 and 0xFE..0x106 go to the GUI path `0x410AA0` when `this+0x40 -> +0x34 != 1`.
5. **HandleInputEvent `0x7B12C0` (thiscall, RET 8) [V].** `case 0x43` (asm `007B13B4`..`007B13DC`):
   - value != 0 -> `PerformLButtonDownAction 0x7B3560(isDbl)` with `isDbl = (value == -1)`.
   - value == 0 -> `PerformLButtonUpAction 0x7B3680()`.
6. **The double click differs from a single click in exactly one place [V].** `0x7B3560` does `if (isDbl == 0) this+0x2B8 = this+0x2B4`, then `GuiMgr(this+0x274)->0x4131E0(isDbl)` (sets bit0 of `gui+0x1C`, used only by GUI controls). A double-click press does **not** refresh `this+0x2B8` ("last clicked").
7. **Release `0x7B3680` [V]** clears `DAT_00A7FCC0` (LMB held) and, if the GUI did not consume it (`0x4132E0 == 0`) and `DAT_00A1B4A4+8 != 0`, calls `HandleMouseClickInWorld 0x7B3CA0` (fastcall, ECX = this).
8. **`0x7B3CA0` decision [V]** (asm, `H` = `this+0x4B0` hovered, `S` = `this+0x2B4` selected, `L` = `this+0x2B8` last clicked):
   - **Branch A** (`L == 0x7F000000 || S != H || L != S`, asm `007B3E50`): if `H != 0x7F000000` -> `SetLastTarget 0x7A0530(H, 0)`, then `0x74CFE0(1)` (HUD action panel refresh). No move, no message.
   - **Branch B** (`L != invalid && S == H && L == S`, asm `007B3D71`): sets `L = 0x7F000000`, takes `entry = *(this+0x4CC)` (first default action, only if `*(this+0x4D0) > 0`), calls `0x4122A0(6)`, then `entry->fn(entry->arg, actor)` at asm **`007B3E42 CALL EAX`**. The entry stride is 0x3C: +8 arg, +0xC fn, +0x10 this-adjust, +0x1C object-type id.
   - So the "second click on the selected object" runs the default action. Because of step 6, a true WM_LBUTTONDBLCLK second press (value -1) leaves `L` at the pre-click-1 selection, so it falls into Branch A unless the object was already selected. Two plain 0x201 presses reach Branch B. **This is the only double-click-specific logic in the chain; the spec of "double click = move" is not implemented here.**
9. **Default-action callbacks [V]** (set in `GetDefaultActions 0x7B40F0`): `0x77CF70` (dialog -> `0x879E80`), `0x77CE00` (attack -> `0x8799A0`, P2S minor 2), `0x841CE0/0x841D40` (use object -> `0x879CC0`, minor 0xB), `0x87BB20/0x87BD10` (P2S minor 3), `0x87F2D0/0x87F330` (use skill/item -> `0x879A20`). These send server-side actions (walk-to-and-do). None is a plain "walk to point".

### Findings that change the working theory
- **0xDB/0xDC are not walk.** `case 0xDB/0xDC: DAT_00A7FCBC = value` only (mouse-steer modifier; RMB press `0x44 -> 0x7B3810` sets the same flag). 0xDD/0xDE set `DAT_00A7FCC4`. Doc 18's suspicion of 0xDB/0xDC as the double-click action is refuted.
- **No ground click-to-move in this chain [V].**
  - `this+0x4B4..0x4BC` (hit vector) is written only by `HandleMouseOver 0x7B3A00` and never read: `rg '\+ 0x4b[4-c]\]'` over the whole dump finds only that store (plus the ctor zero and InitCommands). At the six `0x7B3A00` call sites in ProcessInput the vector is a stack local zeroed with `FLDZ` or not populated, so treat it as unreliable.
  - On a ground hover `H = 0x7F000000`, so Branch A skips SetLastTarget and Branch B is unreachable (`L == invalid` or `S != H`). The ground click ends at `0x74CFE0(1)`.
- **No walk sender exists in the Steam image [V].** All P2S messages go through `0x87A350` (the only writer of the 'p' = 0x70 header byte). `callgraph.tsv` shows 27 caller sites, all enumerated: major 6 minors sent are 2,3,7,8,9,0xB,0xC,0x12,0x21,0x24,0x25,0x26. Server `0x65BE70` switch: minor 1 -> `0x65B4F0` WalkToWaypoint, minor 0x1D -> `0x65B270` DriveControl, 0x21 -> `0x65B3D0`. **Nothing sends (6,1) or (6,0x1D).** Linux has the Drive and Walk senders (`0x08257080`, `0x082571E8`); the Windows builds do not, so `SendPlayerToServerInput_WalkToWayPoint` has no Steam address, and the only server walk path is dead from the client side. How the party leader actually moves on a ground click in Steam is **not** found in the Win32/ProcessInput chain.
- `AddMoveToPointAction 0x564770` callers are server-side only (`0x65B4F0`, script commands, spell actions). `0x5644B0` is the ToFront variant.

### Hook sites
Both are on instruction boundaries with no rel32 and no jump target inside the patched bytes (checked: no `0x007b13ba`, `0x007b3e3c`, `0x007b3e53` reference in the asm).

**(b) double-click press.**

| id | addr | patched bytes (asm) | tag | registers / stack at the hook | what to log |
|---|---|---|---|---|---|
| **H2** | `0x007B3560` `PerformLButtonDownAction` entry | `55 8B EC 83 EC 18` (PUSH EBP; MOV EBP,ESP; SUB ESP,0x18; patch 6) | [V] | ECX = this (CClientExoAppInternal). `[ESP]` = ret, `[ESP+4]` = isDbl (1 if value was -1). Function ends `RET 4`. Hovered id `[ECX+0x4B0]`; vector floats `[ECX+0x4B4]`, `[ECX+0x4B8]`, `[ECX+0x4BC]` (expect 0); selected `[ECX+0x2B4]`; last-clicked `[ECX+0x2B8]`; GUI mgr `[ECX+0x274]` and hovered GUI control `[[ECX+0x274]+8]`; input class `[ECX+0x9C]`; game-state `[[ECX+0x40]+0x34]`. | `isDbl`, hovered id, selected, last, GUI control ptr, `DAT_00A7FCC0`, tick. |
| H2b (alt) | `0x007B13BA` inside `0x7B12C0` case 0x43 | `33 C0 83 7D 0C FF` (XOR EAX,EAX; CMP [EBP+0xC],-1; patch 6) | [V] | EBP frame of HandleInputEvent: action `[EBP+8]` (= 0x43), value `[EBP+0xC]` (-1 for dbl, 1 for press), this `[EBP-0x304]`; ECX is not this here. Reached only for value != 0 (the `JZ` at `007B13B8` skips it). | action, value, then same this-fields via `[EBP-0x304]`. |

**(c) the action issue (walk not found, see above).**

| id | addr | patched bytes (asm) | tag | registers / stack at the hook | what to log |
|---|---|---|---|---|---|
| **H3** | `0x007B3CA0` `HandleMouseClickInWorld` entry (every LMB release that reaches the world) | `55 8B EC 83 EC 74` (patch 6) | [V] | ECX = this (fastcall, no stack args, plain `RET`). Same field offsets as H2: hovered `[ECX+0x4B0]`, vector `[ECX+0x4B4..0x4BC]`, selected `[ECX+0x2B4]`, last `[ECX+0x2B8]`, default-action list `[ECX+0x4CC]` (count `[ECX+0x4D0]`), `[ECX+0x40]`, `[ECX+0x4]->+0x75` (input mode byte). | hovered, selected, last, list count, and the derived branch (A if `last==0x7F000000 || sel!=hov || last!=sel`, else B). |
| H3b | `0x007B3E3C` Branch B, just before the indirect default-action call | `8B 55 FC 8B 42 0C` (MOV EDX,[EBP-4]; MOV EAX,[EDX+0xC]; the original `FF D0` CALL EAX follows at `007B3E42`; patch 6) | [V] | ECX = callee object (already `obj + entry+0x10`), EBP frame of 0x7B3CA0: this `[EBP-0x70]`, entry `[EBP-4]` (+8 arg, +0xC fn, +0x10 adj, +0x1C type), actor `[EBP-8]`, target obj ptr `[EBP-0xC]`. Pushed args: `[ESP]` = entry+8, `[ESP+4]` = actor. Hovered via `[[EBP-0x70]+0x4B0]`. | `entry->fn` (identifies Talk/Attack/Use), arg, actor, hovered id. This is the one real "action issue" call reachable from the double click. |
| H3c (alt) | `0x007B3E53` Branch A body | `8B 91 B0 04 00 00` (MOV EDX,[ECX+0x4B0]; patch 6) | [V] | ECX = this (just loaded from `[EBP-0x70]`). Branch A only (select / ground, no action). | confirm that ground clicks land in A. |

Negative-evidence probes for "does any walk reach the server" (both [V]): `0x00564770` `AddMoveToPointAction`, patched bytes `55 8B EC 83 EC 0C` (ECX = CSWSCreature, RET 0x44), and `0x0065B4F0` `HandlePlayerToServerInputWalkToWaypoint`, patched bytes `55 8B EC 81 EC BC 01 00 00` (patch 9; message bits are read inside, no register args). Log the return address `[ESP]` to see the caller. If H2/H3 fire on a double click but neither of these does, the ground "walk" is not part of the double click at all in this build.

### Next probe step
mouse-probe `click` channel: H2 (isDbl, hovered, sel, last) + H3 (branch) + H3b (fn) + the two negative probes; one session with (1) double click on a door/NPC that is not yet selected, (2) double click on a selected object, (3) double click on bare ground. Expected from the static trace: (1) A,A (no activation) for a real 0x203 press2, B only if both presses are plain 0x201; (2) B once, then A; (3) A only, no walk anywhere.
