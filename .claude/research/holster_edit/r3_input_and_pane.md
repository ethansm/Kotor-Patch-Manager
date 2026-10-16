# R3 input and pane recon (D3c value pane, D3d viewport drag)

## OUTPUT
(a) Calls a D3c/D3d card uses (engine px, top-left origin):
- Cursor: `guikit::cursorEngine(&x,&y)` (GuiKit.h:412).
- LMB press edge: `guikit::clickEdge(ClickEdge&)` (GuiKit.h:430). RMB: `rclickEdge` (GuiKit.h:437). Both need game foreground.
- LMB held/up: no wrapper. Raw `guikit::g_gk.lbuttonDown()` and `gameForeground()` (GuiKit.h:92-93, 116, 691-693). Precedent: Inventory3DViewport.cpp:352 and CharEffectsPanel.cpp:725 poll GetAsyncKeyState per tick.
- Hit on engine control: `guikit::hit(ctl,x,y)` (GuiKit.h:423). Viewport control: `ViewControl(token)` (HolsterEditor.cpp:266), so `guikit::hit(g_ed.view,x,y)`.
- Hit on plain rect: none shared for ints. `guikit::rectContainsF` (GuiKit.h:511, floats). `inRect` is local to CharEffectsPanel.cpp:703.
- Wheel: `guikit::g_wheel` (`touch`, `setRect`, `take`, GuiKit.h:533-566), installed by `wheelInstall()` (GuiKit.h:716).
- Slider value: `editkit::sliderToValue` / `valueToSlider` (EditKit.h:16, 25). Row layout and thumb: `hse::modalLayout` (HseLayout.h:31, 58-72). Drag mode: `editkit::dragMode` (EditKit.h:59). Click vs drag: `midClick` (EditKit.h:72).
- 3D drag maths: `hs::screenDrag` (HolsterEdit.h:135), `worldDeltaToLocal` (:114), `worldMoveToLocal` (:119). Applied by `hs::overlayNudgeWorldRot` / `overlayNudgeWorldMove` / `overlayNudgeLocal` (HolsterPreview.h:168, 177, 187).

(b) gui-input delivers WM_MOUSEWHEEL only.
- `wheelProc` handles only `WM_MOUSEWHEEL` (GuiInputProvider.cpp:80-81). Other messages forward unchanged.
- No class filter. Window is picked by PID, visible, client > 320x240, largest area (GuiInputProvider.cpp:38-48). Class name is logged only (:124).
- Dispatch reaches all subscribers, no GUI check (GiDispatch.h:75-82).
- Consumer gate: `WheelHook::gate` = enabled && touched within 300 ms && a rect is set (GuiKit.h:525, 541, 543). In-rect events are consumed (`inside`, GuiKit.h:545; `guikitWheelCb` :623-640). Outside, forwarded.

## EVIDENCE
- EditKit.h:12 Box; :16 sliderToValue; :25 valueToSlider; :40 repeatTick; :59 dragMode; :72 midClick. No point-in-Box.
- HseLayout.h:58-64 row boxes; :66-72 thumb clamp; :89-91 legend text.
- hse_layout_test.cpp:91 centre = track.x + 0.5*w; :94 f=0 is flush left.
- GuiKit.h:92-93 lbuttonDown/rbuttonDown; :116 g_gk; :412-421 cursorEngine; :423-426 hit; :430-435 clickEdge; :437-442 rclickEdge; :511 rectContainsF; :520 WheelHook; :525 freshMs; :533-534 touch/setRect; :541 fresh; :543 gate; :545 inside; :566 take; :623-640 guikitWheelCb; :691-693 winLButton/InstallPlatform; :716 wheelInstall.
- GuiInputProvider.cpp:80-111 wheelProc (consume at :111); :117-130 ensureSubclass.
- GiDispatch.h:75-88 dispatch.
- Preview3dApi.h:81 SetView(token, yaw, zoom, panZ); :93 ViewControl. No pitch, no camera basis, no input call.
- HolsterEditor.cpp:51 kTickGapMs=500; :266 e.view = ViewControl; :290 OptTick; :311 leave on tick gap; :315 clickEdge; :318 cursorEngine; :319 hit; :322 INPUTCLASS log (cur, x, y, hitBtn, hitBack, editor; capped at 40 lines).
- CharEffectsPanel.cpp:703 inRect; :721-723 tickInput with wheel take; :725 LMB poll; :738 thumb drag start; :747-749 frac and scrollTo. This is a list scrollbar (int index), not a value slider.
- HolsterEdit.h:114, 119, 135. HolsterPreview.h:168, 177, 187 consume them.
- Inv3dLogic.h:155 camBasis; Inventory3DViewport.cpp:226 camera drag. Not shared.
- Layout math: real pane 232 wide, so track w = 88 px (HseLayout.h:58-64 arithmetic).
- No WM_LBUTTON*, WM_MOUSEMOVE, SetCapture in gui_input or _shared_src. The only mouse-message code is the mouse_probe logger.

## FINDING
1. gui-input gives only wheel events. Click, held, move and up must be polled per tick from the Options Draw hook (OptTick). The Options panel only ticks while open; kTickGapMs (500 ms) ends editor mode, so drag state must reset there.
2. Thumb mismatch (D3c blocker if ignored). modalLayout keeps the thumb centre inside track.x+5 .. track.x+w-5, but sliderToValue maps the full track. At the real pane (w = 88) the 5 px inset is about 5.7 percent of range at each end. A drag through sliderToValue jumps that much at the ends. Fix: map with `(px - track.x - 5) / (w - 10)` clamped, or drop the inset.
3. Missing helpers: a held-LMB wrapper, a point-in-Box for EditKit, and a value-thumb drag helper (CharEffectsPanel 738/747 is local to a scrollbar).
4. Camera basis for D3d: Preview3dApi exposes yaw only. camRight and camUp must be built by the card, as Inv3dLogic camBasis does for inv3d.
5. Wheel is not wired in holster-editor (no wheelInstall, touch or setRect in HolsterEditor.cpp).
6. The INPUTCLASS probe (HolsterEditor.cpp:315-323) logs LMB edges and control hits only. Its name suggests the engine input class, but it never reads GetInputClass (0x73ff00) or SetInputClass (0x73fee0). It proves clicks reach OptTick, nothing more.

## OPEN
- Does the engine input class route LMB drags to the world camera while Options is open? Not answered by repo evidence. Needs a log-only probe or a read of the input-class field.
- Does a held LMB stay reported as down across a window focus change under Wine/XWayland? Not verified.
- Is ViewControl visible (bit 1) at the moment the card calls guikit::hit on it? Not checked.
- Does preview-3d accept SetView while the editor is open? Not checked in this recon.

## VERDICT
GAPS. Primitives exist for click, cursor, control hit, wheel rect, slider maths and 3D drag maths. Gaps are small and host-testable: held-LMB wrapper, rect contains, thumb-to-value mapping fix (5 px inset), camera basis for D3d, and wheel wiring. None blocks D3c or D3d.
