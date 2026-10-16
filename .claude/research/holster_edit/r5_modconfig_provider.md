# r5 mod-config / menu UI provider recon (read-only)

## OUTPUT
- Recommend one DLL `mod-config`, two interfaces: `k2.menuui@1` (styled components and page enter/leave) and `k2.modconfig@1` (entry registry). Menus that are not mod config require `k2.menuui@1` alone.
- Options column reflow (vanilla IDs, d2c_options_gui.md:17-24): new `Mod Configuration` clone of BTN_GAMEPLAY at y 242; BTN_FEEDBACK 242->278, BTN_AUTOPAUSE 278->314, BTN_GRAPHICS 314->350, BTN_SOUND 350->386, BTN_QUIT (Exit Game, ID2, assumed) 386->422. Read rects with getExtent, never constants, because ultrawide scales x.
- Sketch (plain C, KPATCH_CALL, struct_size first):
```
K2ModConfigEntry { uint32 struct_size; const char* name; const char* desc;
                   void (*enter)(uint32 page); void (*leave)(void); void (*tick)(void); /* tick may be NULL */ }
K2ModConfigApi   { uint32 struct_size; int32 (*Register)(const K2ModConfigEntry* e); void (*Unregister)(const char* name); }
K2MenuUiApi      { uint32 struct_size; uint32 (*OpenPage)(void* panel, const char* title); void (*ClosePage)(uint32 page);
                   int32 (*AddButton)(uint32 page, const char* label, const char* desc, void (*onClick)(void*), void* ctx);
                   int32 (*AddSlider)(uint32 page, const char* label, float lo, float hi, float* v);
                   int32 (*AddToggle)(uint32 page, const char* label, int32* v);
                   int32 (*AddView)(uint32 page, uint32 p3token);   /* k2.preview3d token */
                   void  (*SetStatus)(uint32 page, const char* text); void (*Layout)(uint32 page); /* idempotent, every tick */ }
```
- Tests: `preview_3d_src/test.toml` pattern (`-I../_shared_src -Werror`), `common_contract_test.cpp` with FakeKPatchApi.h, and a pure header plus `*_test.cpp` (as `hse_layout_test.cpp`).

## EVIDENCE
- Column IDs and rects: `.claude/research/holster_edit/d2c_options_gui.md:17-24`, `:44`.
- Options hooks (ctor, tick, dtor): `patch_manager_mods/holster_editor_src/HolsterEditor.cpp:241` (OptBindView), `:290` (OptTick), `:328` (OptDestroy); addresses in `holster_editor_src/kotor2-steam-aspyr.hooks.toml:10,24,38`.
- Rects set once per instance: `HolsterEditor.cpp:146` firstTick, `:164` gpOk, `:312` `if (!e.appended) firstTick()`; state reset per ctor `:254`.
- Vanilla hide/restore is flag-only: `HolsterEditor.cpp:199` (enter), `:212` (leave, mask FlagVisible|FlagClickThrough).
- Reusable: `_shared_src/GuiKit.h:209` appendToPanel, `:224` getExtent, `:238` setRect, `:430` clickEdge; `_shared_src/EditKit.h:16` sliderToValue, `:25` valueToSlider, `:40` repeatTick; `holster_editor_src/HseLayout.h:31` modalLayout; `_shared_src/SelfHealUi.h:158,199` popover rows.
- HUD-only: `_shared_src/HudKit.h:35-50` (icons).
- Ultrawide: `scripts/generate_ultrawide_gui_overrides.py:167` (optionsingame_p in FRAME_GUIS only; no button-column spec).
- Provider manifest and hooks: `preview_3d_src/manifest.toml:6`; metadata-only hooks file `preview_3d_src/kotor2-steam-aspyr.hooks.toml:2-5` (`hooks = []`).
- Gates: `scripts/check_provider_manifests.py:369-375` (duplicate hook address, provider vs consumer); `scripts/kpm_status.py:151` (NOT-LIVE), `:158` (MIRROR); `scripts/kpm_hotswap.py:48` (HARD_REFUSE).

## FINDING
1. No collision today: only holster-editor references BTN_GAMEPLAY and the Options column indices.
2. Rects are per-instance: the ctor runs per open and firstTick sets them once. Nothing re-applies them within one panel, so `Layout` must re-apply every tick.
3. Hook ownership: mod-config must own 0x8A16FE, 0x40EC90 and 0x8A1930. Holster-editor already hooks all three; as a consumer it must drop them or the checker fails.
4. Lift as-is: GuiKit label clone, setText, hit, clickEdge, hover sound; EditKit slider and repeat maths; HseLayout modalLayout; the holster-editor flag save/restore.
5. New work: hover-description detection, toggle, slider draw, page stack, `AddView` wrapper over k2.preview3d. HudKit and SelfHealUi are HUD-specific, so only the popover row maths is reusable.
6. A new id is NOT-LIVE, so it needs a full `kpm_reinstall.py`, not hot-swap. Uncommitted sources give DIRTY (HARD_REFUSE).

## OPEN
- Does the engine re-run the Options layout on re-show or resize within one panel? No evidence found.
- Confirm BTN_QUIT is the "Exit Game" label.
- Until holster-editor drops its hooks, two detours would sit on 0x8A16FE.

## VERDICT
GAPS. The reflow and provider split are feasible. Still to build: hook ownership move, per-tick rect re-apply, hover detection, toggle and slider draw, page stack.
