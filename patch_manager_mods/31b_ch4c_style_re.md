# 31b - CH4c style RE (click sound, target-menu box art, HUD GUI extents)

Read-only research, 2026-10-05. Binary: Steam Aspyr build (decomp `ghidra_dumps/steam_6a522e71/decomp`).
Scratch (PNGs, tools): `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/cb26d3b4-8fcf-41e2-a81f-8aa49480a80b/scratchpad/`

## Q1. GUI click-sound index

ANSWER: click = index **0**, hover = index **1**. `PlayGuiSound(mgr=[0xA1B49C], idx)` is `FUN_004122a0(char idx)`
(thiscall; plays `mgr->+0xA0[idx]` if idx != -1, idx < `mgr->+0xA4`, entry non-null).

Index -> `guisounds.2da` row (vanilla `2da.bif` entry 76; no override copy in `override/`):
0 Clicked_Default `gui_click`, 1 Entered_Default `gui_scroll`, 2 Error_Default `gui_error`, 3 Checkbox_Check `gui_check`,
4 Fullscreen_Panel_Added `gui_open`, 5 Fullscreen_Panel_Removed `gui_close`, 6 Action_Menu_Use `gui_actuse`,
7 Action_Menu_Scroll `gui_actscroll`, 8 Action_Menu_Queue `gui_button`, 9 Inventory_Add, 10 Inventory_Select, 11 Inventory_Drop,
12 Level_Up_Notify, 13 Quest_New, 14 Quest_Complete, 15 Prompt, 16 Upgrade, 17 DarkSide, 18 LightSide.

Per-control sound fields (CSWGuiControl ctor `0x00418990`): `+0x58` = click sound, default 0 (`*(byte*)(p+0x16)=0`);
`+0x59` = hover/focus sound, default 1. `CSWGuiButtonToggle` ctor `0x0041a330` sets `+0x58 = 3` (checkbox sound).
Control event field `+0x4c` = last event code, set in `FUN_00418c50` (CSWGuiControl::HandleInputEvent, callback table at `+0x3c`).

Click evidence (CSWGuiButton vtable = `0x987a1c`, ctor `0x0041a510`):
- `0x0041a110` = CSWGuiButton vtbl slot +0x3c (HandleInputEvent). If event == `0x27` (select/accept) and down: `PlayGuiSound(*(byte*)(this+0x58))`, then base `0x004187b0`. So a button click plays idx `+0x58` (0 by default).
- Mouse-up path: `0x008868d0` -> `0x00419190`: sends event `0x1f9`, then `0x27` (if control flag bit3). Mouse-down variant `0x0041aac0` sends `0x27` then `0x1f9`. (Not traced which exact one is the pure click-release; both end in the `0x27` -> `0x0041a110` sound.)
- Panels register the same handler for both `0x27` and `0x2d` via `FUN_00418af0` (e.g. `0x008b2f70:82-83` -> `0x008b3500`). Handlers play `*(byte*)(ctrl+0x58)` only when `ctrl+0x4c == 0x2d` (so one sound per click: `0x27` is played by the button, `0x2d` by the panel handler). Callers using `+0x58`: `0x008b3500`, `0x008cfd10`, `0x008df5b0`, `0x0089eb50`, `0x008fa6a0`.
- Hardcoded `PlayGuiSound(0)` in panel input handlers (accept/click): `0x008ba700` (cases 0x27/0x2d), `0x008e0a80` (CSWGuiInGameGameplay::HandleInputEvent, cases 0x28/0x2e/0x2a), `0x0090e850` (CSWGuiSkillsCharGen::OnInputEvent), `0x00908920` (PowersLevelUp::OnInputEvent), `0x008d4f60`, `0x008d9d60`, `0x008df550`, `0x008ddf20`, `0x00901590`, `0x009007a0`, `0x0090b200`, `0x00903ed0`, `0x008bed80`, `0x008ba940`.

Hover evidence (idx 1):
- `0x00418750` = CSWGuiControl vtbl slot +0x0c (CSWGuiButton slot 3 = `0x418750`): on mouse-enter (`param_2 != 0`) and control enabled (`flags&1 == 0`): highlight then `PlayGuiSound(1)`; mouse-leave just un-highlights.
- `0x00418fe0` (`PlayGuiSound(1)` if control enabled, then parent callback), `0x0040ec00` (CSWGuiPanel::SetActiveControl: plays `+0x59` of the newly focused control, default 1), `0x0041e870` (listbox select, plays child `+0x59`), `0x008e1140`/`0x008e11c0` (OnDifficultyLeft/Right), `0x0090e850` lines 24/29, `0x007d7730`..`0x007dbb10` family.

Other fixed indices seen (for reference): 2 error (`0x00751750`, `0x007cd510`); 3 toggle/check (`0x008e0a80` case 0x2d, `0x007d3bb0`, `0x007dccd0`);
4/5 panel add/remove (`0x0079d070`, `0x0079d230`, `0x007c9df0`, `0x007ca060`, ...); 6 action-menu use in the world (`0x007b3ca0` HandleMouseClickInWorld, `0x007b12c0`);
9/0xb inventory add/drop (`0x008d5e20`/`0x008da9b0`, `0x008d6090`/`0x008dac20`); 0xc-0x12 notifications (`0x0075f630`).
Variable-index callers (not constants): `0x0041b220` (`+0x7c`), `0x0041d940` (`+0x2d1`), `0x00744260`..`0x007469f0`, `0x007b12c0` (`uVar4`).

UNVERIFIED: which code raises raw event `0x2d` (only its registrations/consumers were found); whether any GUI file field overrides `+0x58`/`+0x59` per control (no loader write found).

Recommendation for a custom hotbar-style button click: `PlayGuiSound(0)`; hover: `PlayGuiSound(1)`.

## Q2. Target-menu box art (`lbl_miscroll_f` / `lbl_miscroll_hi`)

CONFIRMED resrefs: the frame is `lbl_miscroll_f` on `BTN_TARGET0/1/2` BORDER.FILL; highlight is `lbl_miscroll_hi` on the same buttons' HILIGHT.FILL.
They are NOT square: the art is a tall rounded "pill" (1:2). The square element in the box is a different control, `LBL_TARGETn` (30x30, no fill; icon drawn at runtime) - see Q3. UNVERIFIED hypothesis: the "square frame, centred, arrows above/below" the user sees is that 30x30 LBL (or the icon in it), not the BTN fill.

Sources:
- VANILLA: `texturepacks/swpc_tex_gui.erf` (dated Sep 23, install time), TPC, DXT5, **64 x 128**, 1 mip. Also present in erf: `lbl_miscroll_f2`, `_h`, `_h2` (unused by this GUI).
  Previews: `lbl_miscroll_f_vanilla.png`, `lbl_miscroll_f_vanilla_x4.png`, `lbl_miscroll_hi_vanilla.png`, `lbl_miscroll_hi_vanilla_x4.png`.
- LIVE OVERRIDE (mod-touched, ultrawide ESRGAN x2 upscale, 09-26): `override/lbl_miscroll_f.tga`, `_hi.tga`, **128 x 256** 32bpp, `.txi` = `mipmap 0`. Previews: `lbl_miscroll_f_override.png`, `lbl_miscroll_hi_override.png`. Same shape, exactly 2x.

Measured (alpha threshold 128, colour distance from interior > 60):

| texture | size | visible extent | side border | top border | frame colour | interior |
|---|---|---|---|---|---|---|
| `_f` vanilla | 64x128 | x 1..62, y 1..126 (1 px transparent margin) | 4 px (x 1..4) | 5 px (y 1..5, curved cap) | teal (24,178,139) | black, alpha ~199 (78%) |
| `_hi` vanilla | 64x128 | same | 4 px | 5 px | cream (180,179,153) | same as `_f` |
| `_f` override | 128x256 | x 2..125, y 2..253 | 8 px | 10 px | teal | black alpha ~201 |
| `_hi` override | 128x256 | same | 8 px | 10 px | cream | same |

`_f` and `_hi` are pixel-identical in shape/alpha; only the border colour differs. Top/bottom caps are elliptical (corners rounded, ~5 px corner radius at 64x128). On screen the HUD is 1:1 uniform at 2.667 px/unit at 3840x1600 (see `scripts/generate_ultrawide_gui_overrides.py` docstring), so the 36x60-unit button draws at ~96x160 px: the 1:2 art is stretched slightly to 3:5, border ~ 4/64*96 = 6 px sides.

## Q3. `mipc28x6_p.gui` target-menu controls

Source: VANILLA copy read from `chitin.key` -> `data\gui.bif` entry 78 (84348 bytes), saved `scratchpad/mipc28x6_p.vanilla`.
`override/mipc28x6_p.gui` (live, Oct 2, mod-touched, same 111 controls) was only used for comparison: the target-group extents and fills are identical;
the 6 differing controls are LBL_ACTIONDESC(BG) and the TB_STEALTH/TB_SOLO/TB_PAUSE/BTN_SWAPWEAPONS shifts, unrelated.
Parse tool: `scratchpad/gff.py` (own GFF reader; pykotor not installed).

Only the BTNs use the textures (ids 94, 98, 102). All controls are CONTROLTYPE 6, parent = root panel (`Obj_ParentID -1`), ALPHA unset.
Extents are `LEFT, TOP, WIDTH, HEIGHT` in 800x600 virtual units:

| id | TAG | L | T | W | H | BORDER.FILL | HILIGHT.FILL |
|---|---|---|---|---|---|---|---|
| 94 | BTN_TARGET0 | 43 | 35 | 36 | 60 | `lbl_miscroll_f` | `lbl_miscroll_hi` |
| 95 | BTN_TARGETUP0 | 43 | 37 | 36 | 12 | `lbl_miarr_1` | `lbl_miarr_2` |
| 96 | BTN_TARGETDOWN0 | 43 | 81 | 36 | 12 | `lbl_miarr_1` | `lbl_miarr_2` |
| 97 | LBL_TARGET0 | 46 | 50 | 30 | 30 | (none) | (none) |

Groups 1 and 2 repeat at x = 83 (ids 98-101) and x = 123 (ids 102-105; `BTN_TARGETDOWN2` is L124 W35). 40-unit pitch.
So per group: the 36x60 box spans y 35..95; UP arrow is inside the top (y 37..49), DOWN inside the bottom (y 81..93), and the 30x30 SQUARE LBL is centred between them (y 50..80, x inset 3). The arrows overlap the box, they are not outside it.

BORDER block fields on all of these (BTN_TARGET0, same for UP/DOWN/LBL except FILL):
`CORNER ""`, `EDGE ""`, `FILL <resref>`, `FILLSTYLE 2`, `DIMENSION 0`, `INNEROFFSET 0`, `INNEROFFSETY 0`, `PULSING 0`, COLOR = engine struct id (not extracted).
HILIGHT block has the same field set (FILL = `lbl_miscroll_hi` / `lbl_miarr_2`). TEXT: `FONT dialogfont16x16`, `ALIGNMENT 9`, empty text, `STRREF 0xFFFFFFFF`. MOVETO all -1.
Since CORNER/EDGE are empty and DIMENSION is 0, the frame is drawn as a single stretched fill (FILLSTYLE 2), not a 9-slice, so the whole 1:2 texture is scaled to the 36x60 box.

Not found in the vanilla GUI: any other control referencing `lbl_miscroll_*` (only ids 94, 98, 102).
