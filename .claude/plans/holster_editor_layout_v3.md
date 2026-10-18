# HW-UX3: holster editor layout v3 (consolidated page)

Status: APPROVED 10-18 ("approve HW-UX3"). Follows HW-UX2 (`holster_editor_ux_v2.md`): U0 to U3 are LIVE, and U4 markers are blocked on the camera model (§5).
Input: the user's 10-18 play feedback, screenshots 36 and 37 (the current page) and Gemini mockups moc1 to moc4 plus the Option C sheet.

## 1. User feedback (10-18), verbatim intent
1. The default zoom should fit the whole character; it is zoomed on the hip now.
2. The viewport should be bigger.
3. The description box only holds short lines. It does not need to be big or full height.
4. Show the actual values for the items.
5. Each slider needs a thumb element showing where it is, with the text outside the slider.
6. Reset buttons have no icon. Add a reset arrow that matches our gear and wrench art and the vanilla style.
7. No clicking through options (`<` `>` cycling) for inputs.
8. The page is far too busy: consolidate, clean and polish. There is a lot more space available.

## 2. Mockups vs current page

| Mockup element | Current page (0.3.2-dev) | v3 decision |
|---|---|---|
| Right-hand "Transform Offset" inspector with Position and Rotation groups | The six value rows sit in the left column; the right pane is an almost empty description box | Move the inspector to the right pane with `SetDescSplit(page, 850)`. The top 85% holds the inspector and the bottom 15% keeps one or two hint lines (fb 3, 8) |
| Slider row: name, track with a thumb, value box outside the track | A scrub button with the value text inside; no thumb | Row = name label, track, thumb, value label outside, reset icon (fb 4, 5) |
| Group RESET (moc4) | Six empty reset squares | A reset arrow icon per row plus one per group (fb 6) |
| RMB mode bar across the top of the viewport | Four buttons in the left column | Move it to a strip above the view, with Reset-view and Flip icons on the same strip |
| Animation state bar under the viewport | Stance `< >` cycling plus emote `< >` and Play | A direct stance row under the view: Idle, Walk, Run, Alert, Combat. Emotes are dropped from the editor (fb 7, 8) |
| Mouse legend in the viewport corner (moc B) | A long help paragraph in the description box | One legend line under the view: `LMB orbit · RMB+LMB move weapon · wheel zoom · Shift fine · Ctrl snap` |
| Bone hierarchy tree / 6 bone nodes | A `Bone:` cycle button | Five direct bone buttons with English names (no tree, since only 5 bones exist). Nodes in the viewport stay DEFERRED (toggle mode) |
| Gizmo axes at the weapon | None | U4, once the camera model is solved (§5) |
| Timeline, contextual HUD dial, file menus | n/a | Skip: no use for a holster offset |
| Whole character in view | Hip close-up (zoom 2.2 default, min 0.8) | Default "fit" zoom about 0.6 and minimum zoom 0.4; Reset view returns to fit (fb 1) |

## 3. Proposed page (screen ~3440x1440; current view rect is ext=(1345,357,576,896))

```
LEFT (narrowed)          VIEW (widened)                               RIGHT: inspector (desc split 850)
◀  MEETRA SURIK  ▶      [Move][Depth][Tilt][Roll/Turn]      (⟲)(⇄)   POSITION                         (↺)
WEAPON                  ┌──────────────────────────────────────┐    Slide   ─────●──────────   1.1 cm  (↺)
[■ Right: Vibrosword]   │                                      │    Depth   ──●─────────────  -11.7 cm (↺)
[  Left: Vibroblade ]   │                                      │    Height  ────────●───────  22.0 cm  (↺)
BONE                    │         whole character (fit)        │    ROTATION                         (↺)
[Upper back] [Back]     │                                      │    Tilt    ●───────────────  -145.5°  (↺)
[Pelvis] [R hip] [L hip]│                                      │    Roll    ───────●────────    7.0°   (↺)
APPLIES TO              │                                      │    Turn    ───────────●────   94.0°   (↺)
[This weapon][Type][All]└──────────────────────────────────────┘    Left: mirrored from right (+X tilt)
                        [Idle][Walk][Run][Alert][Combat]           ─────────────────────────────────────
[Save] [Revert]         LMB orbit · RMB+LMB move · wheel zoom       hint: one short line for the hovered row
Unsaved changes *
```

- Selection buttons are segmented: the active one is lit (text colour plus border tint), the rest are dim. One click picks it, no cycling. The member `◀ ▶` arrows stay, since that list is long.
- Editing is always on: the page opens editing the right weapon, and clicking a weapon button switches hand. The separate nav mode and the Done button go away. Back leaves; unsaved edits use the existing two-click guard on hand, scope, bone or member changes.
- The slider track spans the scrub window (±30 cm / ±180°). Click on the track to jump, drag the thumb or the track to scrub, Shift is fine, Ctrl snaps. The value text sits to the right, outside the track.
- `*` marks a non-default value in the value label colour (amber), not inside the name.

## 4. Feasibility (checked 10-18)
- **Description split:** `K2MenuUiApi.SetDescSplit(page, permille)` exists. Permille 600 to 850 keeps 15 to 40% of the description pane at the bottom. `PaneRect(MUI_PANE_PREVIEW)` returns the area above (`_shared_src/ModConfigApi.h:14-24`). Not used by the editor yet.
- **Icons:** `guikit::setFill(ctl, image, tint)` (`GuiKit.h:345`) plus embedded textures (`EmbedTex.h`, `HudKitTextures.inc`, `GearIconTex.inc`). A reset arrow is made the same way as the gear and wrench (teal accent `HudKit kAccent {10,96,80}`).
- **Item budget:**
  - The estimate is about 70 items: left 19, view and strips 12, inspector 34, U4 markers 5.
  - The cap is `kMaxItems = 40` (`mod_config_src/MenuUiLogic.h:37`). Raise it to 80; `kMaxOwn` scales (8 × 80 static slots).
  - This is a mod-config DLL hot-swap.
- **Thumb:** a small label with a fill placed by SetRect each frame. Per-frame SetRect is latch-safe (U4 recon).
- **Bigger view:**
  - The editor already places the custom view item with SetRect (`placeItem(viewItem, MUI_PANE_VIEW)`, HolsterEditor.cpp:1762), so it can take a wider rect.
  - UNSURE: whether the 3D render follows a resized control rect (see §5). Play check L2.
- **Fit zoom:**
  - The log gives fovy = 22.73 / zoom degrees (10.33 at 2.2, 17.49 at 1.30, 19.44 at 1.17).
  - At zoom 1.17 the page shows about 1.0 m of body, so a full 2.0 m needs zoom ≈ 0.6.
  - Lower `kZoomMin` from 0.8 to 0.4 (HseView.h:17) and set the default zoom to 0.6. The ini keeps `view_zoom` to tune it.

## 4b. Research wave R1 (10-18; 8 Haiku + 1 Sonnet; files in `research/holstered_weapons/ux_v3/`)
- **Wider view: CONFIRMED (A1 and A2 agree).**
  - View SetRect copies the rect into the scene sub-object (0x420F50).
  - On every draw, 0x4184F0 → 0x479460 sets glViewport from that rect.
  - Camera 0x47F320 takes its aspect from the rect while the camera's own viewport fields stay 0 (the CAMENG `vp=0,0,0,0` log line confirms this).
- **Reset icon: PARTIAL (B).**
  - `guikit::setFill` gates on the LABEL border at +0x60, so it silently fails on mod-config BUTTONS (border at +0x70).
  - L0 adds a button-border fill helper, after checking that the CSWGuiButton +0x70 border shares the CSWGuiBorder layout (fill +0x74, tint +0x28, style +0x34).
  - Fallback: a separate label item carrying the icon, placed over the button.
- **Desc split and cap: PARTIAL (C).**
  - The mod-config hit test is rect-only, so items placed in MUI_PANE_PREVIEW are clickable.
  - kMaxItems 80 breaks no provider code and costs about 118 KB static.
  - Open: whether the engine control list (CExoArrayList Add 0x83EA60) grows. 24 → 40 already worked live; L2 play-checks 80.
- **Page inventory (D):**
  - 38 items today. HolsterApi has no absolute bone setter, so a direct bone button loops CycleBone until the index matches (no holster change).
  - The code has 7 stances. v3 shows 5 (Idle, Walk, Run, Alert, Combat) and drops the rifle walk and run.
- **Camera (F):**
  - The full-window viewport hypothesis is REFUTED: the render viewport is the control rect, the aspect is w/h, and no scissor is set.
  - The 1.0 m vs 1.8 m gap is still unexplained. Lead: the live render pose differs from the fields read in the tick; the camera is attached (CAMATT flag48=1, off z 1.593).
  - CAM2 must also log V+0x64..+0x70 (the pushed rect) and re-read the camera pose inside a draw-time read, or click-fit it.
- **Flourish (E1, E2; Sonnet E3 settled them):**
  - Cause: the holster mod repoints the hand nodes to the hip globally (Holster.cpp:1169-1180), so any hand animation plays with the weapons still at the hip.
  - Flourish sets only the server combat state (SetCombatState mode 2, 0x79F621), so the client flag +0x138 never changes.
  - Fix design for the backlog: un-repoint around the flourish. It needs a start/end edge.

## 5. CAM1 result (10-18 play) and the U4 blocker (window-viewport lead REFUTED by R1 F, see §4b)
- **CAMENG** read the engine camera at view+0x78:
  - pos = (4.8725, 0, 0.4358), q = (0.4786, 0.5210, 0.5210, 0.4775): looking along −X, up +Z;
  - fovy follows the zoom; the creature yaw does not move the camera (hook_dot changes, pos does not).
  - The viewport fields are 0,0,0,0, so the guarded MapToScreen call was skipped (WARN, by design). The engine fills the viewport from `glGetIntegerv` at render time.
- **The pose plus control-rect maths does not match the picture.** It predicts z 0 to 1.8 m all inside the 896 px view at zoom 1.17, but the screen shows about 1.0 m.
  - Best lead: the render uses the full-window GL viewport (H 1440), and the control rect only clips. 896/1440 × 1.87 m ≈ 1.16 m, which matches.
  - Not proven.
- **CAM2 probe (holster-editor, log only, before U4):**
  1. Log `glGetIntegerv(GL_VIEWPORT)` in the tick.
  2. Split CALIB lines so the node list is not truncated (the 4 clicks logged 10-18 lost every node but the head).
  3. Log predictions under both hypotheses (control rect vs window viewport) for every click.
  - Then U4 markers follow the plan in `holster_editor_ux_v2.md` §3 U4.

## 6. Waves
- **L0 prep** (host only, parallel, $0 install):
  - (a) mod-config `kMaxItems` 40 → 80, plus tests.
  - (b) A reset-arrow texture: generator next to the gear/wrench source, EmbedTex `.inc`, and an embtex test.
  - (c) Pure `hse::pageLayoutV3(column, view, preview)` in HseLayout.h returning every Box (segmented rows, strips, inspector rows with thumbs), with a host test across 3 screen sizes (1280x720, 1920x1080, 3440x1440) and no overlaps.
- **L1 page rebuild** (holster-editor 0.4.0-dev; one Sonnet worker is the sole owner of HolsterEditor.cpp):
  - always-on edit, segmented selectors, the inspector in the desc split, sliders with thumbs, reset icons, the stance row, the toolbar and the legend;
  - fit default zoom and kZoomMin 0.4;
  - CAM2 probe lines (§5).
  - Keep the existing chord, scrub maths, guard and log names (INSP, CHORD, EDIT).
- **L2 install and play row UX3:**
  - mod-config plus holster-editor DLL hot-swap after the user's GO;
  - row UX3: whole body visible on open, the view rect wider and still rendering, thumbs move with drags, reset icons visible, no cycling buttons left except the member arrows, the hint line fits.
- **U4 markers:** after CAM2 settles the projection.

## 6b. Backlog (nice to have, user 10-18; not in v3)
- **Flourish out of combat:** draw the weapons, play the flourish, then re-holster.
  - Today the weapons stay holstered, the hands spin around nothing, and the character is left in a combat stance.
  - Needs RE of the flourish action path (who triggers it, and whether the engine draws first in combat only). Needs its own small plan.

## 7. Risks
| Risk | Mitigation |
|---|---|
| A wider control rect does not widen the 3D render | L2 checks it; fallback keeps today's width and grows only the height and the inspector |
| 80 items slow the mod-config hit test | Linear over ≤ 80 per click: negligible |
| The fill texture on a mod-config button is not drawn (latched border style) | `setFillStyle` exists (`GuiKit.h:381`); fallback is an ASCII `<-` glyph |
| The slider window ±30 cm hides values beyond it | The thumb pins at the end with an arrow mark; the value text is always exact |
