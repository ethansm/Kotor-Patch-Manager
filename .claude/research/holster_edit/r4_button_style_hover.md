# R4: vanilla option-button style, hover and description, plus shared menu-UI controls (static recon, 10-16)

Read-only. Static dump `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` (decomp `.c`/`.asm`, `strings.tsv`), vtables read from `swkotor2_vanilla_6a522e71.exe` (PE parse, `.rdata`). No live run, no repo edits except this file. Dump paths below are `decomp/<xx>/00<addr>.c`.

## OUTPUT

1. Vanilla option buttons (BTN_*) are CSWGuiButton objects, not labels. vtable 0x987A1C, ctor 0x41A510, 0x1D0 bytes, inline in the Options panel at `P+0x68 + k*0x1D0`. k = 0 LOADGAME, 1 SAVEGAME, 2 GAMEPLAY, 3 FEEDBACK, 4 AUTOPAUSE, 5 GRAPHICS, 6 SOUND, 7 QUIT. Those are not list IDs (list IDs differ, see d2c).
2. A button's look is two CSWGuiBorder objects (`+0x70` normal, `+0xE8` highlight, selected by `+0x48` bit 0) plus a CSWGuiText object (`+0x160`).
3. The description is not keyed on `mgr+0x08`. Each vanilla button gets event 0 bound to handler 0x8A2210. That handler switches on k to a TLK strref and calls 0x8A2320, which writes the text into an inline label at `P+0x1030`.
4. Per-panel control map (scope change):

| Need | Vanilla panel (tag) | Class, ctor, vtable | Size | Clone route |
|---|---|---|---|---|
| Header | LBL_TITLE in all option panels (optionsmain 0x7D23E0, gameplay 0x8DF930, sound 0x7D9800, graphics 0x7D4620, feedback 0x7D31D0, mouse 0x7DC410, graphicadv 0x7D6170, soundadv 0x7DACB0) | CSWGuiLabel 0x419740, vt 0x9878BC | 0x148 | `createLabelFromTemplate("LBL_TITLE")`, works now |
| List rows | LB_OPTIONS (feedback 0x7D31D0), LB_RESOLUTIONS (resolution 0x7D8540), LB_DESC (Options, list idx 15) | CSWGuiListBox 0x41CE90, vt 0x987D3C | about 0x438 (inferred) | Heavy. Prefer label rows. |
| Slider | SLI_MOUSESEN (gameplay 0x8DF930, mouse 0x7DC410), SLI_MUSIC/VO/FX/MOVIE (sound 0x7D9800), SLI_GAMMA (graphics 0x7D4620) | CSWGuiSlider 0x41AB60, vt 0x987AEC | 0x1C4 (inferred from stride) | Class clone via ctor. Value text is a LBL_* label. |
| Toggle | CB_* (gameplay 0x8DF930, mouse 0x7DC410, graphics 0x7D4620, graphicadv 0x7D6170, soundadv 0x7DACB0) | CSWGuiButtonToggle 0x41A330, vt 0x98796C; base ctor is 0x41A510 | at least 0x1D8 (writes `+0x1D4`) | Class clone via ctor |
| Back/Close | BTN_BACK (all 8 sub-panels above, plus optionsmain), BTN_OK/BTN_CANCEL (resolution 0x7D8540), BTN_EXIT (Options, idx 13) | CSWGuiButton 0x41A510 | 0x1D0 | Class clone via ctor |

## EVIDENCE

- Button ctor 0x41A510 (`41/0041a510.c`, asm `.asm` same dir): `*this = 0x987A1C`; base ctor 0x418570; border ctor 0x414A50 on `+0x70` and `+0xE8`; text ctor 0x4171A0 on `+0x160`. Dtor 0x41A3D0.
- Options ctor 0x8A1170 (`8a/008a1170.c`): `_eh_vector_constructor_iterator_(this+0x1a, 0x1d0, 8, 0x41A510, 0x41A3D0)`, tags in k order via 0x733570 (layout lookup) and 0x40F620 (InitControl, addToList 1).
- Event 0 binding, asm `8a1862-8a1877`: `PUSH 0; LEA ECX,[P+0x68+k*0x1D0]; CALL 0x418AF0`, eight times with handler 0x8A2210. Click handlers at `8a1754-8a17dc` push 0x27.
- 0x8A2210 (`8a/008a2210.c`): finds k by pointer compare, then switch: k0 0xA53C, k1 0xA53D, k2 0xA521, k3 0x2147A, k4 0xA523, k5 0xBE2F, k6 0xBE30, k7 0xA53E. Unknown control: `local_c = -1`, no call.
- 0x8A2320 (`8a/008a2320.c`): writes the description via label `P+0x1030` (vt[4]), TLK lookup via `P+0x1108` vt[8], empty string → 0x4178B0. `P+0x1030` is one of the inline 0x148 labels (r2 `d2b/d2c` dtor list, 0x8A1960) and not a list entry.
- Options vtable 0x9A701C (`PE .rdata`): slot 0x08 = 0x40EC00 (SetActiveControl), 0x34 = 0x40EC90 (Draw), 0x44 = 0x40E7B0 (returns focus `P+0x20`), 0x48 = 0x8A24F0, 0x58 = 0x8878D0.
- 0x8A24F0 (`8a/008a24f0.c`): `M = P+0x1C`, hover = `[M+0x08]`. Only if `hover->vt[0x58]() != 0` does it match `P+0x68+k*0x1D0` and call 0x8A1C10(k), which calls `P->vt[0x08](ctl, 0)`. Then if focus is 0, it calls 0x8A1C10(0). Button `vt[0x58]` = 0x4558A0, which returns 0 (r2), so the match block is skipped for buttons.
- SetActiveControl 0x40EC00 (`40/0040ec00.c`): old `vt[0x3C](1,1)`, new `vt[0x3C](0,1)`, sound 0x4122A0(`ctl+0x59`) when the flag is set.
- Shared enter/leave 0x418750 (`41/00418750.c`), vt[0x0C] of button, slider, toggle, listbox: arg 0 → `vt[0x3C](1,1)`; arg 1 (not `+0x48` bit 0) → `vt[0x3C](0,1)`, sound 0x4122A0(1), then 0x419060 → 0x413F90.
- Button vt[0x3C] = 0x41A110 (calls 0x4187B0). vt[0x40] = 0x41A290 sets `+0x48` bit 0 and `+0x1B0` bit 0x40. vt[0x38] = 0x41A0A0 applies its arg to the active border and to `+0x160`.
- Toggle ctor 0x41A330: base 0x41A510, `+0x1D0 = 0x27`, `+0x1D4` bit 0 cleared, `+0x58 = 3`. Slider ctor 0x41AB60: base 0x418570, sub-objects 0x414A50 and 0x416290, `+0x74 = 0`, `+0x78 = 0`, `+0x7C = 1`.
- Sub-panel ctors (tag order, `decomp/7d`, `decomp/8d`): 7D23E0 (optionsmain), 8DF930 (gameplay), 7D9800 (sound), 7D4620 (graphics), 7D31D0 (feedback), 7DC410 (mouse), 7D6170 (graphicadv), 7DACB0 (soundadv), 7D8540 (resolution).
- Style fields for a copied look (static only): border `+0x74` fill, `+0x28` tint RGB, `+0x24` alpha, `+0x34` fill style (`_shared_src/GuiKit.h:47-48, 341-383`). Text colour `text+0x2C` (`GuiKit.h:47, 292-303`).
- Mods: `GuiKit.h:175-208` `createLabelFromTemplate` allocates `LabelSize`, calls `LabelCtor`, checks `LabelVtable`, then InitControl(addToList 0). No button path. Label clones in CombatHotbar, QueueHud, HolsterEditor (`LBL_TITLE`), LevelUpUi, CompanionSelfHeal, ActionHotbar. No mod clones a real CSWGuiButton. `BTN_` appears only in HolsterEditor.cpp, as a comment and in the probe tag list.

## FINDING

1. Copying fields gives the static look only. Hover and selected visuals come from the class vtable (`vt[0x0C]`, `vt[0x3C]`, `vt[0x40]`) and the second border, so a cloned label cannot match.
2. The right route is a class clone inside the panel-ctor window, using the same vtable check as the label clone: alloc 0x1D0, ctor 0x41A510, check vt == 0x987A1C, InitControl with the BTN_* tag, then append.
3. Header: the existing LBL_TITLE label clone already works. Sliders, toggles and list rows are clonable through their ctors, but sizes for slider and listbox are inferred.
4. A mod button gets no description: it matches no k, and the switch does nothing. The mgr+0x08 poll cannot focus buttons (`vt[0x58]` = 0).
5. Mod hover text: from OptTick, when `M+0x08 == ours`, either set the mod's own label or call 0x8A2320 with a strref (thiscall, ECX = panel). The second path is unverified for custom strings.

## OPEN

1. Who raises event 0 on a control (likely the enter path, unverified). Dispatcher not read.
2. Caller of Options `vt[0x48]` (0x8A24F0). Not found. Per-frame or not is unknown.
3. Writers of `M+0x08` (hover), as in r2 OPEN 1.
4. Sizes: slider 0x1C4 and listbox about 0x438 are inferred from stride. Toggle size is not confirmed. Button 0x1D0 is confirmed.
5. 0x8A2320 string path: how `local_28` is filled, and whether a non-TLK string works.
6. Slider thumb sub-object (0x416290) and listbox PROTOITEM template were not read.

## VERDICT

CLONE-BUTTON. Add `ButtonCtor = 0x41A510`, `ButtonVtable = 0x987A1C`, `ButtonSize = 0x1D0` to K2Addr/GameAddr, then add `createButtonFromTemplate` beside the label clone. Gate: one live probe before ship. Log the vtable check, confirm the 0x1D0 size, and confirm event 0 fires on a mod button hover.
