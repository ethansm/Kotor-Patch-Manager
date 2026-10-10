# Session Handoff & Plan State: patch_manager_mods/18_mouse_camera_investigation.md

## 0. Next-Phase Direction
Execute 18 — Ranks 8, 9 & 10: Mouse Camera Fix & Shared Input Architecture (18_mouse_camera_investigation.md, 24_master_plan_status.md:68, 16_candidate_register.md).

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/18_mouse_camera_investigation.md`
- **Branch:** `KOTOR-II` (94c9c55 chore: sync session handoffs, skills, and agent guidelines)
- **Live Patch Status:** `mouse-probe` 0.3.0 INSTALLED (kpatch md5 `77a3b41e`, KOTOR-II `cb7e361`), 33 ids / 217 hooks active in `kpm_install_state.json`.

## 2. Technical Analysis: Mouse Camera Pitch Lock & MOUSE_VIRTUAL_DESKTOP
- **Proton/Wayland Root Cause:** Under COSMIC/Wayland with XWayland, Wine's DirectInput raw-input path emits over 6,900 `fixme:dinput:dinput_mouse_rawinput_hook Unimplemented MOUSE_VIRTUAL_DESKTOP flag` messages. Wine maps desktop-absolute pointer records into DirectInput relative motion.
- **Delta Saturation:** Raw `lX` and `lY` arrive as quantized, single-signed spikes (e.g. -1375, -2750, up to -528820). The engine normalizer `FUN_0072F110` executes `clamp(-lX / 100.0f, -1.0f, 1.0f)`, pinning `nx` and `ny` at 1.0 (99.9% SAT in probe logs).
- **Pitch Lock Mechanism:** Camera director `FUN_0078C350` integrates normalized delta rate multiplied by dt via `FUN_007F3C70`. The chase camera clamps pitch to `[DAT_00A10B24=1.0, DAT_00A10B28=50.0]`. Persistent non-zero deltas shove pitch directly into the clamp bounds where it locks.
- **Double-Click Failure:** Win32 `WM_LBUTTONDBLCLK` (0x203) requires spatial proximity within `SM_CXDOUBLECLK`. Spurious cursor warp jumps cause clicks to register outside the spatial tolerance, breaking double-click-to-move.

## 3. Architectural Design: Unified `k2.guiinput` Subclassing (Master Plan L4)
- **Hazard to Eliminate:** 4 separate mods (`force-power-hotbar`, `inventory-3d-viewport`, `equipment-3d-character`, `character-effects-panel`) currently invoke `SetWindowLongPtrA(GWLP_WNDPROC)` to chain `wheelProc`. Unsubclassing is fragile ("only if still on top" guard), leaving dangling pointers if unloaded out of sequence.
- **Single Owner Interface (`k2.guiinput`):**
  - Consolidates `EnumWindows` game window discovery into `HWND game_hwnd()`.
  - Replaces stacked `wheelProc` hooks with a single subscriber dispatcher: `wheel_subscribe(cb, user_data)`, `wheel_unsubscribe(cb)`.
  - Unifies cursor transformation: `cursor_engine(float* x, float* y)`.
  - Tracks window focus: `foreground()`.
  - Host for input filter: Provides interception point for `GetDeviceState` (`0x00732E00` / `0x0072F110`) to sanitize or reject corrupted `MOUSE_VIRTUAL_DESKTOP` deltas.

## 4. Candidate Register Context (Doc 16 & Doc 24:68)
- **Master Plan L4 (`24_master_plan_status.md:68`):** `k2.guiinput` shared input subsystem deferred until 2nd consumer consolidation.
- **Doc 16 Candidates:** L8 (Linux symbols, not needed), L9 (2DA token, killed), L10 (`heads.2da` cap, 249/254 rows).
- **Resolution Strategy:** Address mouse camera fix either via environment configuration (`PROTON_ENABLE_WAYLAND=1` / Gamescope) or via delta sanitization inside `k2.guiinput`.

## 5. Dead Ends & Anti-Patterns (Do NOT Attempt)
- ❌ **`MouseWarpOverride=force` or `EnableHardwareMouse=0` in INI:** Tested 09-26. Pinned cursor to center without fixing camera spin; reverted per user instruction.
- ❌ **Custom point-and-click movement mod:** Doc 28 cancelled. Native engine possesses double-click move; failure is purely input-stream corruption.
- ❌ **Unconditional `SetWindowLongPtrA` unhooking:** Mod-specific unhooking corrupts the subclass chain if another DLL was injected later.
- ❌ **Direct log scraping:** NEVER `cat`, `tail`, or `grep` raw logs. Use `python3 scripts/kotor2_logs.py check <log> --new`.

## 6. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.

## 7. Verification Commands
- Check mouse probe telemetry: `python3 scripts/kotor2_logs.py check mouse-probe_log.txt --new`
- Verify shared headers and subclass definitions: `python3 scripts/check_shared_headers.py`
- Run test suite for common components: `python3 scripts/run_tests.py -k "guiinput or common"`
