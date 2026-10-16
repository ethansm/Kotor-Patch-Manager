# Holster editor UX options: dedicated patch vs equip-screen piggyback (2026-10-16)

Question (user, 10-16): instead of putting the holster placement editor inside the equipment screen's 3D preview (plan HW-EDIT,
phases HE3/HE4), can it be its own patch with its own UX?
- **O1** an opaque popup modal holding a 3D viewport, auto-pausing the game while open;
- **O2** a new settings screen off the Options menu that holds both the 3D viewport and the value pane.

Sources: four Haiku recon notes in this folder, static dumps only (Steam sha 6a522e71), no live checks:
[ux_a_popup_pause.md](ux_a_popup_pause.md) (popup + pause), [ux_b_viewport.md](ux_b_viewport.md) (3D viewport),
[ux_c_menu_screen.md](ux_c_menu_screen.md) (options/main menu), [ux_d_reuse_providers.md](ux_d_reuse_providers.md) (reuse,
providers, cost). Earlier: [r1_camera_keys.md](r1_camera_keys.md), [r2_labels.md](r2_labels.md). Anchors live in those notes.
Addresses below were not re-verified by Opus. Before an address goes into a hooks.toml, run `query_ghidra.py address` on it
and read its bytes.

## 1. Verdict

| option | feasibility | main blocker | real party bodies | pause | separate patch |
|---|---|---|---|---|---|
| O0 equip-screen piggyback (current HE3/HE4) | high | none (R2 gate passed) | yes | yes (menu) | no, lives in equip3d |
| **O2a in-game Options sub-screen, editor hosted in the Options panel** | **medium-high** | tick/destroy hook sites and Esc behaviour of the Options panel unknown | yes | yes (in-game menu pauses on open) | **yes** |
| O1 popup modal, our own new panel | medium-low | no known way to build a new CSWGuiPanel at runtime | yes | must set it (proven pair) | yes |
| O1b popup that reuses a vanilla panel class | unknown | needs RE: which vanilla panel can be built standalone | yes | must set it | yes |
| O2b main-menu Options sub-screen | low value | no save loaded, so no party bodies or equipped weapons | no (N_CommM/F only) | n/a | yes |

**Recommendation: O2a.** It is the only dedicated route that does not need a panel class we have never built:
- It uses the same proven recipe as equip3d: clone controls inside a vanilla panel's constructor, plus a `3D_MODEL` control
  copied into that panel's `.gui`.
- The hook site is free, and the screen is already paused.
- It keeps the editor a separate patch.

Do **not** build O2b: per-body tuning is the point, and the main menu has no party bodies.
Keep O1 as a later upgrade, only if R-A below finds a panel we can build at runtime.

**Work that is not wasted in any option:**
- HE0 (preview probe; still needs your GO to apply);
- HE1 (pure units, landed `03aa85d`);
- HE2 (holster provides `k2.holster`, preview fast path, overlay, Save).

Only HE3/HE4 (equip3d UI wiring) change.

## 2. Facts per option

### O1 popup modal (ux_a, ux_b)
**Modal push is proven:**
- `CSWGuiManager` is at global `*(0xA1B49C)`.
- `AddPanel` 0x410530 (thiscall, RET 12): flags bit 0 also pushes the panel on the modal stack. The game calls `AddPanel(menu,0,1)` when it opens the in-game menu.
- `RemovePanel` 0x410670 pops the modal stack and calls the panel's vt+0x4C.
- `PushModalPanel` 0x410900, `PopModalPanel` 0x4109C0.

**Pause is proven (galaxy-map pattern):**
- On open: `if (!GetPausedByCombat) SetPausedByCombat(1,0,0)`, via 0x740350 (setter) and 0x740330 (getter); see 0x7CA730.
- On close: the mirror call (0x7CA920).
- Rules:
  - Restore only a pause we set ourselves.
  - Never call the toggles 0x87A730 / 0x51C6B0.
  - Re-assert the pause each tick, because alt-tab back (AppActivated 0x40BCF0) clears it.
  - Never touch bit 1 (cutscene timestop).

**Blocker:** no mod has ever created a new `CSWGuiPanel`.
- The R9 "ctor" 0x40E770 is a 13-byte no-op.
- No panel vtable of our own exists.
- A `3D_MODEL` control can only be created while a panel's layout file is open, i.e. inside its constructor: `StopLoadFromLayout` 0x40F5A0 frees the layout handle at panel+0x30.

So a popup needs one of:
- a panel class we fully construct (unknown);
- a reused vanilla panel class (O1b; see R-A).

**Input:**
- Clicks: polling with `GetAsyncKeyState` is safe (the game never imports it). Swallowing mouse messages would also kill GUI clicks, so we poll instead.
- Right-drag: needs the RMB-camera swallow at 0x7B3817 (companion-selfheal precedent), or the world camera turns.
- Esc: also reaches the in-game menu opener, which toggles pause (risk of a double toggle).

### O2 Options sub-screen (ux_c, ux_b)
**Hook sites.** Clone a "Holster placement" button inside the Options constructor, after its last `InitControl` and before `StopLoadFromLayout`:

| screen | hook | ECX | free button slots |
|---|---|---|---|
| in-game Options (`optionsingame_p`) | 0x008A1704 | `[ebp-0xAC]` | (161,422,236,32) and (161,460,236,32) |
| main-menu Options (`optionsmain_p`) | 0x007D27E6 | — | — |

No mod hooks either Options screen today.

**Hosting.** A button cannot push a panel without a panel object (that is O1's blocker again). So O2a hosts the editor **inside the Options panel itself**:
- clicking the button hides the vanilla `BTN_*` controls and `LB_DESC`;
- it shows our cloned viewport and value pane;
- Back restores them.

This is the equip3d pattern (clone in the constructor, toggle visibility), applied to another panel.

**3D view in Options:**
- Add a `3D_MODEL` control to `optionsingame_p.gui` by copying the control out of `upgrade_p.gui`. Same recipe as `scripts/add_equip_3dview_control.py`; it needs a sibling generator script with its own id check.
- Then build `createView` → `initScene("gui3D_room")` → rig `charrec_light` → `cameraAttach("camerahook")`.
- Each view has its own `CSWGuiScene` and camera, so it cannot conflict with equip3d's scene.

**Body:**
- Build a fresh `CSWCCreature` the way equip3d's `buildPreview` does (ctor 0x7645F0, BuildApp 0x76D200).
- Copy the appearance from the real party member (`ClientPartyTable` 0x73FB90, `PartyGetAt` 0x7E5DA0).
- Attach weapons with `FnCreateModel` 0x462320 and Gob vt[0x50].
- Add a party-member picker (◀ name ▶) in the pane, since this screen has no party bar.

**Pause:** opening the in-game menu already calls the pause toggle (`OpenInGameMenu` 0x7C9DF0 → 0x87A730). Options is a tab of that menu, so the world is paused. Not verified live.

**Esc on the Options screens: unknown.**

### Equip-screen piggyback O0 (current plan)
- Already proven up to the HE4 gate (R2 click blocking).
- A dedicated patch **cannot** re-hook the equip panel: H-E1/H-E3/H-E4 (0x8AA16A, 0x8ABA50, 0x8AA6F0) belong to equip3d, and KPM rejects two hooks at one address.
- So an equip-screen editor is either in-pane (inside equip3d), or a separate patch driving equip3d through a new `k2.preview3d` provider (XL; ux_d P4b). The second only makes sense if the editor must open from several screens.

## 3. Reuse and shared code (ux_d)

**Unchanged in every option:**
- `HolsterApi.h` (k2.holster v1); only its header comment, which names equip3d as the host, needs rewording.
- `HolsterEdit.h`, `HolsterTune.h`, `HolsterIniWrite.h`, `HolsterQuat.h`.
- `GearIconTex.inc`. Holster owns all the data and maths; the UI only calls the API.

**Moves to `_shared_src` (new `EditKit.h`):**
- `Box` (also removes the duplicate in `hud_host_src/HhStrip.h:9`);
- `Repeat`/`repeatTick`, `sliderToValue`/`valueToSlider`, `dragMode`/`DragSel`, `midClick`, `fmtValue`.

`modalLayout` and `legendText` go to the editor patch, with the layout redone for its own pane. Neither `Equip3dEdit.h` nor `GearIconTex.inc` is wired into live code yet, so moving them costs nothing now.

**New shared header `_shared_src/Preview3d.h`** (header, not provider):
- equip3d's body build, weapon attach/detach and drag/zoom input: `EquipCharPreview.cpp` 465-482, 524-580, 637-751, 795-826, about 300 lines.
- These move behind a `Viewer` struct (block, creature, wpn[2], yaw, zoom, drag).
- equip3d and the editor each keep their own scene, so a header is enough. A provider is needed only if one DLL must drive the other's view.
- equip3d's tests must follow the move.

**Providers:**
- **Holster must stay `kind = "patch"`.** KPM skips a ticked provider that no installed patch requires (`DependencyValidator.cs:293`), and that would drop holster's behaviour. It provides `k2.holster@1` from a normal patch, like ranged-ai does.
- The editor patch lists `requires = ["holster"]` plus `requires_interfaces = ["k2.holster@1"]` (a hard dependency; fine for an editor).
- equip3d must never list holster in `requires`. That would be an install error without holster; its optional `require()` stays unlisted (WARN only).
- `gui-input` (`k2.guiinput`) already gives cursor, wheel and foreground; equip3d uses it.
- Clicks stay polled.
- Keys: no capture design is possible yet, because the game also reads the keyboard through DirectInput (r1). Typed values (HE5) stay dropped.

**Provider ideas, all later:**
- `k2.modsettings` / `k2.modpanel`: shared "mod options" pages. Worth it only with two or more consumers, e.g. holster editor plus per-mod KpLog debug toggles (memory idea `kotor2-ingame-debug-toggle-idea`; KPM RFC draft `19a_rfc180_comment_DRAFT.md:40` names "Mod Options" as interface 5). If O2a lands, its Options-panel host code is the natural seed for it.
- `k2.hud` PANEL_BUILD phase: hud-host already sees every panel's layout close (0x40F5A0), so it could give any panel a build callback without a new hook site (ux_d P5).
- `k2.guiinput` key/capture growth: blocked on the DirectInput question.

## 4. Cost (SIZING_RUBRIC points; ux_d section 4, adapted to O2a)

| step | work | size | needs |
|---|---|---|---|
| HE0 | preview probe hot-swap + play row HW-C6 | done, awaiting GO | user GO |
| HE2 | holster provides k2.holster: fast path, overlay, Save | L (+ preview path XL, split) | HE1 (landed) |
| D1 | `EditKit.h` + `Preview3d.h` extraction; equip3d uses it with no behaviour change | M + L | compat_check, Common sync, equip3d gate + play recheck |
| D2 | RE: Options panel tick/destroy hooks, Esc, pause live check, ultrawide conflict | M (Haiku recon + 1 play row) | — |
| D3 | `holster-editor` patch: `optionsingame_p.gui` generator, ctor hook 0x8A1704, button, in-panel editor (viewport + pane + picker) | XL, split into D3a viewport, D3b pane, D3c drag | D1, D2, HE2 |
| D4 | optional: `k2.modsettings` provider seeded from D3's host code | L | a second consumer |

**Compared with O0:** HE3 + HE4 ≈ L + L, against D1 + D2 + D3 ≈ M + L + M + XL.

The dedicated route costs roughly 2x more. What it buys:
- a separate patch;
- the editor reachable from Options even with equip3d uninstalled;
- equip3d staying small.

## 5. Open questions (next recon wave)

- **R-A (O1b):**
  - Can a vanilla panel class be built standalone and pushed with `AddPanel(p,0,1)`? Candidates: the upgrade panel (`upgrade_p` already has a `3D_MODEL`), `CSWGuiMessageBox` (`confirm_p`, built by `StartAutoLevelup` 0x84FE20), `PopUpGUIPanel` 0x68ECE0.
  - What is its constructor contract, and its vtable +0x4C?
  - A yes upgrades O1 to medium-high.
- **R-B (O2a):**
  - The Options panel's per-frame entry: a tick or HandleInputEvent hook site; GOG `CSWGuiOptionsMain::HandleInputEvent` 0x568550 has a Steam twin to find.
  - Its destroy path.
  - What Esc and Back do while our editor is shown.
  - Whether `DispatchOptionButton` ignores unknown tags.
- **R-C (O2a):** does the ultrawide GUI work (`kotor2-ultrawide-gui`) already override `optionsingame_p.gui`? The new generator must chain onto that file, not replace it. Same sha-tracking as `add_equip_3dview_control.py`.
- **R-D, live (play row):** is the world paused while in-game Options is open? Does `ClientPartyTable` return all party members there?
- **R-E:** the view block destructor address and the render-time state with two scenes (ux_b open items). This only matters if equip3d's view and ours can be alive at once; they cannot in O2a, since the two screens are tabs of one menu.

## 6. Decision needed (user)

1. Should the editor be reachable from somewhere other than the equipment screen? If no, O0 is cheapest; if yes, go O2a.
2. If O2a: keep the small O0 bits in equip3d anyway (the holster toggle, i.e. a holstered **view** with no editor)? It costs one HE3-lite step and shows the tuned values next to the gear.

Suggested order if O2a: HE0 apply + play → HE2 → D2 recon (Haiku, $0 static) → D1 → D3a/b/c.

## 7. Decision (user, 2026-10-16)
O2a chosen: a dedicated in-game Options screen, independent of equipment-3d-character, because users without an ultrawide
monitor may not run the ultrawide menus that equip3d's layout relies on. Plan: `.claude/plans/holster_editor_options.md`.
