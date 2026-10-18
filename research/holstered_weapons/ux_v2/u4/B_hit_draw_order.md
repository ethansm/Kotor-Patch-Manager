# U4 Question B: draw order and input (recon, read-only)

Paths are relative to the repo root. Line numbers are current (the older H2/S1 cites have drifted).

## 1. Drawn above the 3D view?

**Answer: YES for any control appended after the view. Premise correction: the LIVE mode bar is NOT over the view.**

- Z-order = list index. The panel draws ascending; appended controls draw above all native ones.
  `patch_manager_mods/RuntimeGuiControls.md:106`
- Mod-config appends page items in item-id order on first tick: `patch_manager_mods/mod_config_src/ModConfig.cpp:529`.
  Creation order is draw order. The view is `AddCustom` at `patch_manager_mods/holster_editor_src/HolsterEditor.cpp:429`,
  before the buttons (438-466, 477) and labels (430, 437, 446).
- Precedent for an overlay drawn over a 3D view: `patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp:400-401`
  and `:473` (LBL_BEVEL covers, built right after the view "so they draw on top of it").
  `patch_manager_mods/01_inventory_3d_viewport.md:44`. The doc does not record a live check of the bevels on top.
- 3D drawn inside the view control's draw call: **PROBABLE, not proven.**
  - Camera pass `0x0047F320` has 0 direct callers (`static_facts.py addr`). It is reached only through the camera
    object's vtable slot 2 (`research/holstered_weapons/ux_v2/H1.md` C5). It brackets its own GL state (push/pop,
    viewport restore).
  - View slot 14 `0x0084de40` (37 B) only forwards to the scene sub-object's slot 3:
    static dump `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/84/0084de40.c:5-6`. The doc calls slot 14 "probably Draw"
    (`patch_manager_mods/01_inventory_3d_viewport.md:255`). Unverified.
  - No code found that renders the 3D after the panel loop.
- Mode bar location: NOT over the view. It sits in the left strip, `leftStrip = [col.x, view.x - gap)`
  (`patch_manager_mods/holster_editor_src/HseView.h:96-100`, applied at `HolsterEditor.cpp:1494`). Mode buttons use
  `compactSlot(e.strip)` (`HolsterEditor.cpp:1162`). The only item placed on `MUI_PANE_VIEW` is the view (`:1482`).
  So there is no in-repo button-over-view precedent in the holster editor.

## 2. Input today

- **Clicked** fires once per click, on the release edge only (`patch_manager_mods/_shared_src/ModConfigApi.h:92`).
  No hover API. H2 item 13 is confirmed refuted at the API level.
- **Weapon mode (chord).** `ci.inView = insideView && !overModeBar && !overInspector`
  (`HolsterEditor.cpp:1602`). A press edge with `!inView` sets `CS_DEAD` / `CW_PRESS_OUTSIDE`, a drop
  (`HseView.h:444-448`). DEAD clears only when all buttons are up (`HseView.h:404`). `inView` is read only on press
  edges, so a drag that moves over a button mid-gesture continues.
- **Nav mode.** No button exclusion at all. `insideView` (`HolsterEditor.cpp:1932`, a raw `guikit::hit` on the view
  control) gates MMB reset (`:1958`), drag begin (`:1967`) and `calibClick` (`:1971`). A press on any control that
  overlaps the view starts an orbit drag. Latent today (nothing overlaps); breaks with markers.
- **Hit test.** `guikit::hit` is a raw cursor rect test that requires `FlagVisible` (`_shared_src/GuiKit.h:267-270`,
  `:426-429`). Hidden or parked items never hit.
- **Wheel.** Gated by the view rect (`HolsterEditor.cpp:1500`). Markers inside the view keep wheel zoom; no change needed.
- **Clone defaults.** Label clones start hidden and click-through (`ModConfig.cpp:595`;
  click-through bit 0x20 per `RuntimeGuiControls.md:100-102`). Engine clicks pass through labels.

## 3. Minimal change (hit-test protocol)

1. **Build.** Markers are fixed-pool `AddLabel` items, created in build() right after `AddCustom(view)` at `:429`.
   Not `AddButton`: markers need no `Clicked`, and `AddX` is build-only (`ModConfigApi.h:86-91`).
2. **Per tick.** In `cbTick` at `:1932`, one line:
   `insideView = ctl && guikit::hit(ctl, curX, curY) && !overMarker;`
   This covers both paths. Weapon: `ci.inView` goes false, so the press is a dead press (`:1602`, `HseView.h:444-448`).
   Nav: MMB reset, drag begin and calibClick are all blocked (`:1958`, `:1967`, `:1971`).
3. **`overMarker`.** Loop the marker ids with the existing `overItem` pattern (`HolsterEditor.cpp:1275-1279`), the same
   pattern as `overModeBar` (`:1553`). Hover highlight = the same raw `hit` per tick (no API needed).
4. **Parking.** Parked markers use `offscreen = {-9999,-9999,0,0}` (`:1103`) and hide, so `hit` fails and they never
   block the view.
5. **Warn.** One `KP_LOGV(..., KPLOG_WARN, ...)` if any marker `AddLabel` returns < 0, same as `MODE WARN` (`:480`).

Mid-drag entry into a marker is unaffected: the chord reads edges, not `inView`, after the gesture has begun.

## 4. Unknowns

- `Clicked` for press-on-view / release-on-button: not read, UNSURE. Moot if markers are labels.
- Live check that the inventory bevels draw on top of the view: not recorded in the doc.
- View slot 14 = Draw: unverified (forwarder only).
- Panel draw loop calls each control's Draw in list order: inferred from `RuntimeGuiControls.md:106`, not read in the dump.
