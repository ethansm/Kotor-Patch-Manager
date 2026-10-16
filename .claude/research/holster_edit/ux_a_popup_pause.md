# UX-A recon: opaque popup modal with auto-pause (holster editor)

Method: static dumps only (`~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/`: decomp, callgraph.tsv, imports.tsv), prior notes R1/R2 (`.claude/research/holster_edit/`), W9/PQ1/U6 engine notes, and repo sources. No live REA, no game run. Addresses are Steam sha 6a522e71. "dump" = `decomp/<xx>/<addr>.c`.

FEASIBILITY: MEDIUM. Pause and modal push are proven. Main blocker: no proven way to construct a NEW CSWGuiPanel at runtime (base ctor not identified, no panel vtable we own, layout must come from a shipped .gui). Second blocker: swallowing mouse messages to stop world clicks also stops the GUI's own clicks, so the popup must poll its buttons.

## 1. Opening our own GUI panel or popup

1a. No existing mod creates a whole new panel. All of them add controls to a panel the engine already builds.
- Action hotbar: 86 controls and 4 textures on the HUD panel (`patch_manager_mods/RuntimeGuiControls.md:3-4`).
- Level-up override: 27 slots created in the character-panel ctor hook, appended on the first tick (`patch_manager_mods/level_up_override_src/LevelUpUi.h:4-6`).
- Equipment 3D: a `3D_MODEL` control from a patched `equip_p.gui`; `createView(panel,"3D_MODEL")` at `patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:854`, id 49 check at `:874`. Needs an override `.gui` edit (`patch_manager_mods/10_equipment_3d_character.md:28`).
- SelfHealUi.h is pure logic (`patch_manager_mods/_shared_src/SelfHealUi.h:1`); its popover lives over the existing HUD.
- Label clones are valid only while the layout is open: inside the panel ctor, before `StopLoadFromLayout` 0x40F5A0 (`_shared_src/GuiKit.h:171-172`, `_shared_src/GameAddr.h:24`).
- `k2.guiinput` v1 is wheel-only: HWND, cursor, foreground, wheel subscribe (`_shared_src/GuiInput.h:43-55`). No click, key or modal API.
- HudHost events fire only inside the HUD layout (`_shared_src/HudHost.h:13-18`). Not a popup host.

1b. Engine popup precedents, all named in the dumps or docs:
- `StartAutoLevelup` 0x84FE20 builds a `CSWGuiMessageBox` from `confirm_p` (OK/Cancel only) (`patch_manager_mods/08_level_up_override.md:452`).
- `Global::ExecuteCommandDisplayMessageBox` 0x69BEA0 and `PopUpGUIPanel` 0x68ECE0 are named; auto-fit not examined (`patch_manager_mods/15a_research_session_context_2026-10-01.md:47`).
- Layout loader `CSWGuiPanel::StartLoadFromLayout` 0x40EE40, RET 12, `(this, resref, scaleA, scaleB)` (`patch_manager_mods/upstream_prs/gameapi_signatures/followup.md:80`). Callers include the equip ctor 0x8A92D0 (`dump 40/0040ee40.c` callers list).
- Base ctor UNKNOWN: R9 names 0x40E770 `CSWGuiPanel::Constructor` (`patch_manager_mods/engine_research_findings/R9_map_CSWGuiManager_CSWGuiPanel.tsv:1`), but `dump 40/0040e770.c` is a 13-byte `ret 4` no-op.

1c. CSWGuiManager panel API (Steam):
- Manager object: global `*(0xA1B49C)` (`patch_manager_mods/engine_research_findings/W9.md:169`).
- `AddPanel` 0x410530, thiscall RET 12, `(panel, arg2, flags)`. Bit 0 of flags also pushes the modal stack (`W9.md:171`).
- Game call: `AddPanel(menu,0,1)` in `OpenInGameMenu` (`dump 7c/007c9df0.c`).
- `PushModalPanel` 0x410900 RET 4 (`dump 41/00410900.c`). `PopModalPanel` 0x4109C0 RET 0 (`patch_manager_mods/steam_db_pr3/pr3/functions.csv:235`).
- `RemovePanel` 0x410670 RET 4: pops the modal stack if top, calls panel vtable `+0x4C` (OnPanelRemoved) (`dump 41/00410670.c`).
- Lists: panel list `mgr+0x88/+0x8C`; modal stack `mgr+0x94/+0x98`, top = last (`W9.md:171-172`, `action_hotbar_src/ActionHotbar.cpp:75`).
- Panel flags `+0x48`; `PanelIgnoredByInput` = 0x600 (`ActionHotbar.cpp:73-74`).
- Engine-owned pause panels: `CSWGuiInGamePause` vt 0x9A7BAC, `CSWGuiInGameAutoPause` vt 0x9A8FCC (`_shared_src/GameAddr.h:84-85`).
- No vtable we own exists. `_shared_src/vtable_override_test.cpp` exists; a cloned-vtable route is UNKNOWN (not read).

## 2. Pausing

2a. Server pause = bit 2 of byte `[CServerExoAppInternal+0x10078]` (`patch_manager_mods/29_paused_command_evidence/PQ1.md:4`). Read chain `[[[AppGlobal]+8]+4]+0x10078` (`_shared_src/ActQueue.h:23,68`). Pause freezes the world timer (dt=0) and does not stop UpdateState or AIUpdate.
- Get: 0x538CD0 / wrapper 0x51C720 (`PQ1.md:15`).
- Set: 0x538ED0 internal, ECX=CServerExoAppInternal, `[esp+4]`=mask, `[esp+8]`=value; wrapper 0x51C760 RET 8 (`PQ1.md:17,173`).
- Toggle: 0x51C6B0 / 0x538C20 flips the bit (`NEG EAX`, `PQ1.md:14`). NOT idempotent.

2b. Engine pause entry points:
- Client request facade `CClientExoApp::SetPausedByCombat` 0x740350 (value, reason, force) -> internal 0x79BF40 (`dump 74/00740350.c`, `dump 79/0079bf40.c`). It sets request flags `[+0x37C]|=4`, `[+0x380]=value`, only if the current server state differs.
- MainLoop 0x781BE0 applies a pending request by calling the toggle 0x87A730 when current != requested (`dump 78/00781be0.c:651-653`).
- `GetPausedByCombat` 0x740330 = server GetPauseState(2) 0x79BF00 (`dump 74/00740330.c`).
- Galaxy-map precedent (closest pattern for a full-screen GUI): open `if (GetPaused==0) SetPausedByCombat(1,0,0)` in `dump 7c/007ca730.c`; close `if (GetPaused!=0) SetPausedByCombat(0,0,0)` in `dump 7c/007ca920.c`.
- `0x87A730` = `if server: TogglePauseState(2)`, returns 1 (`dump 87/0087a730.c`). The in-game menu calls it on open (`dump 7c/007c9df0.c`). Avoid blind calls.
- AppDeactivated 0x40BC40 sets pause (2,1) (`patch_manager_mods/engine_research_findings/U6.md:16`). AppActivated 0x40BCF0 undoes it (`patch_manager_mods/09_load_hang_investigation.md:257`).
- No mod source in the tree calls SetPauseState or 0x51C760; mods only read the bit (`ActQueue.h:23`, `queue_hud_src/QueueHud.cpp:66`).

2c. Can a mod set on open and restore on close? Yes, with these rules:
- Read the prior bit at open (GetPauseState). Set via the galaxy-map pair (0x740350) or 0x51C760(2,1) only if not already set.
- Restore only if we set it. Never call 0x87A730 or 0x51C6B0.
- Re-assert every tick while open, because AppActivated clears the bit on alt-tab return. Whether it clears unconditionally is UNKNOWN.
- The 0x79BF40 guard depends on `[[client+0x40]+0xCC]`; its meaning is UNKNOWN, and it may refuse the request in some states.

## 3. Input while the popup is open

3a. Keyboard: WM path via WndProc 0x40A750 -> WM_KEYDOWN 0x40A160, WM_CHAR 0x40A460 (`.claude/research/holster_edit/r1_camera_keys.md` sections c and d). DirectInput buffered keyboard also exists (R1 section d). `kp::WindowHook` can swallow WM keys (`_shared_src/WindowHook.h:24-54`). The DI gameplay reader is UNKNOWN (R1 open item).

3b. Mouse: WM-driven (R1 section b, mouse recorder 0x72E030). Swallowing WM_LBUTTON*/WM_RBUTTON* in our hook stops world AND GUI clicks. So the popup must poll: GetAsyncKeyState/GetCursorPos with a foreground gate, as `EquipCharPreview.cpp:403,803` does (R1 section b). The game imports no GetAsyncKeyState (`imports.tsv`), so polling is safe.

3c. Engine GUI routing: `ProcessInput` 0x7AC470 calls `CSWGuiManager::HandleInputEvent` 0x410AA0 only when `CGuiInGame+0x34 == 1` or the client input class `[+0x9C]` is 2 or 3 (`dump 7a/007ac470.c:508-512`). `SetInputClass` 0x7B3050 stores `[+0x9C]` (`dump 7b/007b3050.c`). 0x410AA0 dispatches to the top modal if any, else the panel list (`dump 41/00410aa0.c:121-139`). Its return is void, so world code is not stopped by it. Whether the class suppresses world clicks (PerformLButtonDownAction 0x7B3560, `patch_manager_mods/18_mouse_camera_investigation.md:331`) is UNKNOWN.

3d. Hit order inside a panel: last-appended control first, shown label swallows clicks in its rect (`.claude/research/holster_edit/r2_labels.md` section d).

3e. Cursor: game calls GetCursorPos per frame (`dump 72/0072e8d0.c`), ShowCursor/SetCursor (`imports.tsv:6327-6333`). `CSWGuiManager::CaptureMouse` 0x413420 is GUI-control capture, not OS capture (`dump 41/00413420.c`). SetCapture is used only in WndProc and window code (`imports.tsv:6340-6344`). No ClipCursor import, so a modal does not grab the cursor by itself. Whether the software cursor is hidden in gameplay: UNKNOWN.

3f. RMB: `RmbHoldFlag` 0x00A7FCBC lets RMB drag move the camera; site 0x7B3817 is already swallowed by companion-selfheal (`_shared_src/GameAddr.h:93-94`). Our RMB drag needs the same swallow, or the world camera turns.

3g. Esc: WM_KEYDOWN VK 0x1B path is in R1 section c. Esc also reaches the in-game menu opener 0x7C9DF0 (callers include 0x7B12C0), which toggles pause again (see 2b). Double-toggle risk.

## 4. Risks

- Save, load, area transition: LoadScreen is a modal panel in the same stack (`W9.md:166-172`). A stale popup could survive a module change. `HideAllPanels` 0x411EE0 has one caller, 0x79C0E0, reached from 0x7403A0; name not traced (`callgraph.tsv:82048`). Whether the pause bit is saved: UNKNOWN.
- Resolution change: `ReloadPanels` 0x412400 is called from the display-mode handler 0x7D9280 (`callgraph.tsv:88473`). Our layout must survive a rebuild; UNKNOWN whether it rebuilds children.
- Party switch and creature lifetime: the equip preview uses PanelSlots and `destroyCreature` on evict (`EquipCharPreview.cpp:867-870`). Our popup needs the same guard; it has no NPC switch.
- Dialogue and cutscenes: cutscene timestop is bit 1 (`PQ1.md:65`, OnApplyTimestop 0x5A4DC2). Never clear bit 1. Whether a modal can be pushed over dialogue is UNKNOWN (dialogue panel stack not traced).
- Game's own pause requests (galaxy map, menus) share bit 2. Restore only our own set (see 2c).
- Hotbar gate: any non-ignored modal closes it (`ActionHotbar.cpp:318-319`). Desirable. The pause and auto-pause panels are whitelisted only in the panel-list branch (`ActionHotbar.cpp:69,330`); inferred non-modal, UNVERIFIED.

## Open
- Construct a valid CSWGuiPanel at runtime: base ctor, a vtable we own (or reuse one), and whether a shipped `.gui` can carry a `3D_MODEL` control into it.
