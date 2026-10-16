# R2 recon: runtime labels in the equip panel (holster_edit)

Sources: live `override/equip_p.gui` (79 controls, read with pykotor), Steam exe static dumps (`ghidra_dumps/steam_6a522e71`), PE image bytes, `_shared_src/GuiKit.h`, `level_up_override_src/LevelUpUi.h`.
Nothing was run live. Short results: (a) PASS, (b) PASS, (c) PASS, (d) PASS with one OPEN item.

## (a) Template choice and level-up clone recipe

- Live equip_p.gui: 79 controls, ids 0..78. Types: 61 labels (CONTROLTYPE 4), 16 buttons (6), 2 list boxes (11: LB_DESC, LB_ITEMS).
- Level-up tags `LBL_EXPERIENCE_STAT`, `LBL_XP_BACK`, `LBL_EFX_BACK` are NOT in equip_p.gui (checked by tag). Copying LevelUpUi.h:72-74 verbatim makes createLabelFromTemplate return 0 (CF_Template).
- Candidate templates (index, tag, extent, border, fill, font, align):
  - 11 `LBL_INV_HEAD` (30x54): no CORNER/EDGE/FILL, dialogfont16x16, align 9. Best neutral border-less blank base. Change the font at runtime if 10x10 is needed.
  - 57 `LBL_E3D_M00` (60x22): no CORNER/EDGE, FILL e3d_btn, dialogfont10x10, align 18. The only dialogfont10x10 template. Already bound by name at EquipCharPreview.cpp:900. e3d_btn is shipped in override/ (e3d_btn.tga), so this mod owns the look.
  - 78 `LBL_E3D_TIP` (100x40): no CORNER/EDGE/FILL, dialogfont16x16, align 18. Plain text template for a tooltip-like label.
- Level-up recipe, LevelUpUi.h:142-166 (ctor): 27 slots (enum UiCtl, C_N = 27). Per slot, one `createLabelFromTemplate` with kTplFramed or kTplPlain chosen by `kUiSpec[i].framed`. Slot C_BG0 tries kTplWindow first. A null result triggers one call with the other template. Stores `r->c[i]`. Takes the ctor-time extent of the first label for the fallback scale.
- Append happens later, in uiAppend (first tick): one `appendToPanel` per non-null slot, in index order, backgrounds first. Per label: 1 create, 1 append (+1 create on a miss), then setters.
- Equip needs about 45 clones: 45 creates in the ctor, then 45 appends. The create must run before StopLoadFromLayout (0x0040F5A0, GameAddr.h:24). Host point: BindEquipView (EquipCharPreview.cpp:845), H-E1 at 0x008AA16A in ctor 0x008A92D0 (EquipCharPreview.cpp:8).
- Avoid template tags containing "LBL_BAR" (LBL_BAR1..5): InitControl (0x0040F620) has a strstr(...,"LBL_BAR") branch that calls vt 0x50 on a child. Source of that string not traced.

## (b) Capacity: PASS (no engine cap)

- appendToPanel calls listAdd = 0x0083EA60 (CExoArrayList<void*>::Add). If count == cap: cap = 16 when 0, else 2*cap, then realloc via 0x0084A680. No fixed maximum. The only failure is an allocation returning NULL, which is not checked (0x00919723 path).
- Panel list: ptr at panel+0x24, count at +0x28, cap at +0x2C (K2Addr.h:289-290; GuiKit.h:49). 79 + 45 = 124 entries.
- Ctl_Id (+0x54) is an int equal to the list index. No range check found. appendToPanel verifies count+1 and slot == ctl (GuiKit.h:209-218).
- InitControl's addToList path pads NULLs and overwrites when id < count (0x0040F620 tail). GuiKit passes addToList = 0 and re-stamps the id from the count, so this path is not hit.

## (c) Draw order: PASS

- Panel draw 0x0040EC90: draws the background (+0x60, vt slot 0xC), then loops i = 0 .. count-1 ascending. For each child with flags bit1 set, it calls vt slot 0x38 (indirect site 0x0040EE2C).
- A label appended after the 79 existing controls has the highest index, so it draws last, above 3D_MODEL (index 49, x 536..752).
- Not checked: the scene view's internal draw order inside its own slot-0x38 call.

## (d) HARD GATE: PASS (for labels and buttons). One OPEN item.

- Walk 0x0040EAE0 (panel mouse hit, callers 0x0074B3A0, 0x007BCCB0, 0x00894DC0): point outside the panel rect returns 0. Otherwise `local_18` runs from count-1 DOWN to 0, skipping NULL children. Each child's vt slot 0x44 is called with panel-local (x, y). The first non-zero result ends the loop and is returned. So the LAST-APPENDED control is tested first, and the first hit wins. Siblings with lower index get nothing in that rect.
- Base hit 0x00418D20 (asm 0x418D20..0x418D97): needs flags bit1 = 1 and bit5 = 0, then an inclusive rect test on +4..+0x10. It returns the control pointer on a hit, else 0.
- So a shown label (setVisible(true): bit1 set, bit5 clear) swallows clicks in its rect. A hidden label (bit1 clear) returns 0 and lets clicks through.
- Slot 0x44 is 0x418D20 for label vtable 0x009878BC and for CSWGuiButton vtable 0x00987A1C (read from the PE image). Type-6 buttons are assumed to be CSWGuiButton (doc 31a note); not confirmed from the factory.
- Consequence: a shown clone shadows every lower-index control inside its rect: all centre buttons, and LB_DESC (index 0). Clones outside the centre pane do not shadow them.
- Hiding by tag: setVisible(false) on a label or button makes the base hit return 0. Hidden controls are also skipped by the draw loop.
- LB_ITEMS (48,134,236,364) does NOT overlap the centre pane (x 294..526). No shadowing risk there.
- LB_DESC (type 11, index 0) vtable 0x00987D40: slot 0x44 = 0x0041FFD0, which decompiles as a GFF Load ("BORDER", "SCROLLBAR", ...), not a hit test. UNSURE how the list box is hit-tested. It is index 0, so any shown clone above it wins first regardless.
- Not on the equip path: 0x00902B60 hit (vtable 0x009A9F54, ctor 0x009017B0 = CSWGuiKeyMapButton). It self-tests, then delegates to child +0x1D4 even when hidden.

### Centre-pane controls (x overlaps 294..526; IN = fully inside, PARTIAL = crosses an edge)

IN (37):
- List box: LB_DESC (0)
- Labels: LBL_INV_WEAP_R2 (2), LBL_INV_WEAP_L2 (3), LBL_INV_WEAP_R (4), LBL_INV_WEAP_L (5), LBL_INV_BELT (6), LBL_INV_ARM_R (7), LBL_INV_BODY (8), LBL_INV_ARM_L (9), LBL_INV_HANDS (10), LBL_INV_HEAD (11), LBL_INV_IMPLANT (17)
- Labels: LBL_DEF_BACK (12), LBL_DEF (13), LBL_DEF_TEXT (47), LBL_TOHITR (15), LBL_TOHITL (31), LBL_TOHIT (32), LBL_ATKL (27), LBL_ATKR (28), LBL_DAMAGE (30), LBL_ATTACKMOD (38), LBL_DAMTEXT (39), LBL_SLOTNAME (33)
- Buttons: BTN_INV_HEAD (16), BTN_INV_BODY (18), BTN_INV_ARM_L (19), BTN_INV_WEAP_L (20), BTN_INV_WEAP_L2 (21), BTN_INV_WEAP_R2 (22), BTN_INV_BELT (23), BTN_INV_WEAP_R (24), BTN_INV_ARM_R (25), BTN_INV_HANDS (26), BTN_INV_IMPLANT (48)
- Buttons: BTN_SWAPWEAPONS (43), BTN_PREVNPC (45)

PARTIAL (6):
- LBL_BACK1 (1), LBL_BAR1 (14), LBL_TITLE (29), LBL_BAR2 (34), LBL_BAR3 (35), LBL_BAR5 (37)

Not in the centre pane: LBL_BAR4 (575), LB_ITEMS (48..284), BTN_BACK, BTN_EQUIP, BTN_NEXTNPC, 3D_MODEL (536), LBL_BEVEL/LBL_BEVEL2 (532), LBL_E3D_* (684+).

## OPEN

- Which caller runs the equip panel's mouse walk, and does a consumed hit also stop world click-through? Candidates: 0x0074B3A0 (calls 0x007433B0 first, then 0x0040EAE0; no static callers found), 0x007BCCB0 (no callers found), 0x00894DC0 (CSWGuiInGameMap::HitCheckMouse, not the equip panel). Not traced.
