# R8: solid dark fill for the enemy-effects-strip label (static recon)

Question: a texture resref that renders as a solid dark rectangle through `guikit::setFill` / `setBorderTint` / `setAlpha` on a label cloned from `LBL_QUEUE1`.

## Answer
- Use the vanilla resref `uibit_fill_2bt`. No file to ship.
- Decoded pixels: every block is RGB 0,0,0. Alpha is uniform 199/255 (about 0.78), not 1.0 as the repo notes say.
- Tinting `buffhud_circle` black does NOT give a rectangle. It is a disc (corner alpha 0).

## 1. Resrefs passed to setFill / imageFor / imageForRetry (non-test, `patch_manager_mods/*_src`)

| Mod / anchor | Resref | Role | Tint / alpha | Shipped where |
|---|---|---|---|---|
| queue_hud QueueHud.cpp:426-427 | uibit_fill_2wt | cancel button fill | cancel style tint | vanilla (swpc_tex_gui.erf), not repo |
| queue_hud QueueHud.cpp:562, 569 | lbl_miqueue_teal | combat queue backdrop (drawImage, not setFill) | white, alpha 0.95 | repo `queue_hud_src/override/lbl_miqueue_teal.tga` + `.txi` (524332 B) |
| queue_hud QueueHud.cpp:433-440 | tex::Cancel / CancelHi | companion cancel | teal border 0.10,0.58,0.62; hover 0.15,0.75,0.80 | tex module (not traced) |
| combat_hotbar CombatHotbar.cpp:254-255, 350, 357-362, 713-715 | lbl_miscroll_f / lbl_miscroll_hi | pill frame | setBorderTint per group; hover 1,0.35,0.35 | game override has them; not in repo |
| shield_bar ShieldBar.cpp:180-181, 262, 323, 407 | uibit_bar_sh_p, uibit_bar_sc_p | shield bar, corner | none | repo `shield_bar_src/art/`; manifest.toml:12 says `override/`; game override has both (131116 B each) |
| buff_duration_hud BuffDurationHUD.cpp:52, 292-296 | buffhud_circle | item-ability disc | BgColor 0.035,0.415,0.34 @ alpha 0.80*0.35 | repo `buff_duration_hud_src/buffhud_circle.tga` (65554 B, 128x128 disc) |
| buff_duration_hud BuffDurationHUD.cpp:49 | (none) | drain "dim" | DimAlpha 0.35 on the icon draw, no texture | n/a |
| active_effects CharEffectsPanel.cpp:86, 341-342, 640-641, 693-694 | buffhud_circle, uparrowg, dnarrowg | disc (DiscTeal), scroll arrows (White/Dim) | as listed | disc as above; arrows vanilla |
| action_hotbar ActionHotbar.cpp:118-119, 847, 1042, 1136 | fph_slot, fph_slot_hi, fph_edit | slot frames | alpha only | embedded: action_hotbar_src/HotbarTextures.inc:13300-13301 |
| level_up LevelUpUi.h:78, 80, 228-242 | luo_fill (embedded), uibit_fill_2bt | window BG0 and tip fill | alpha 1.0 | luo_fill: level_up_override_src/LuoTextures.inc:1378; uibit_fill_2bt vanilla |
| companion_selfheal CompanionSelfHeal.cpp:606, 617, 628, 642, 677 | cross image, ring frame | button and ring | gold ring SelfHealUi.h:427 (1.00,0.80,0.25) | cross image source not traced |
| equipment_3d EquipCharPreview.cpp:313 | e3d_btn, e3d_btn_hi | button fill | none | game override only, not repo |

No holster_src setFill/imageFor call sites were found.

## 2. Existing solid TGA/TPC

- Repo: 81 TGA/TPC under `patch_manager_mods`. Smallest is `buff_duration_hud_src/buffhud_circle.tga` at 65554 B. None are tiny.
- Game override dir: 12851 TGA/TPC. Smallest are 34 B, 2x2 32bpp lightmaps (e.g. `961cor_lm_25.tga`) with varied grey pixels, not black. A scan of 296 TGAs at or under 200 B found no all-zero body.
- Vanilla `texturepacks/swpc_tex_gui.erf` holds `uibit_fill_2bt` (TPC, 1179 B), `uibit_fill_2wt` (TPC, 1181 B) and `lbl_miscroll_f` (name hit in the ERF key table).
- `uibit_fill_2bt` decoded (32x32, DXT5, 1 mip, 64 blocks): colour endpoints 0x0000/0x0001 with index 0, so RGB 0,0,0. Alpha endpoints 199/200 with index 0, so alpha 199/255 (about 0.78) everywhere.
- `uibit_fill_2wt`: colour index 1 is white, alpha 49/255 (about 0.19). Faint white, see-through, matching `14_character_active_effects_panel.md:76`.
- `luo_fill` (`level_up_override_src/LuoTextures.inc:1378`, `kLuoFill[300]`): 8x8 32bpp TGA. All 64 pixels are BGRA (6,8,2,255), so opaque RGB(2,8,6). Embedded, no file.
- `buffhud_circle.tga`: 128x128. Centre (255,255,255,255), corners alpha 0. Tinted black it is a black disc.

## 3. How an extra TGA ships

- Mod side: a `<mod>_src/override/` subtree. `scripts/kpm_hotswap.py:213` `mirror()` copies production files recursively into the KPM `Patches/<X>/` tree, skipping test dirs and nested manifests.
- `scripts/kpm_status.py:36` ASSET_EXT = {.ini, .toml, .tpc, .mdl, .mdx, .2da, .tga}. `is_production` at `:44-49`.
- `.txi` is NOT in ASSET_EXT, so `mirror()` would skip a shipped `.txi`. Yet `queue_hud_src/override/lbl_miqueue_teal.txi` exists.
- Precedents: `queue_hud_src/override/` (tga + txi), `npc_diversity_src/override/` (tpc), `BuffDurationHUD.cpp:52` comment "shipped in override/".
- No `override` key in any `manifest.toml`. `BUILD_AND_INSTALL.md` has no override section.
- The step that copies KPM `Patches/<X>/override/` into the game `override/` was not located in `scripts/kpm_*.py` or the KPM `create-patch.py`. OPEN.

## Recommendation (lowest risk first)

1. Vanilla `uibit_fill_2bt`, zero shipping:
```cpp
void* img = guikit::imageFor("uibit_fill_2bt");       // vanilla TPC, black
if (img) guikit::setFill(ctl, img);                    // LBL_QUEUE1 is FILLSTYLE 2 (stretch), 07_force_power_hotbar.md:645
guikit::setBorderTint(ctl, black);                     // force black regardless of the template tint
guikit::setAlpha(ctl, a);                              // effective alpha = a * 0.78
```
   - For an effective 0.65, call `setAlpha(ctl, 0.65f / 0.78f)`.
   - Already proven in this repo: `LevelUpUi.h:236-242`, and `14_character_active_effects_panel.md:81`.
   - Correct the "opaque black" comments (`LevelUpUi.h:80`, `14_character_active_effects_panel.md:81`) when touching this.
2. Embedded `luo_fill`: opaque RGB(2,8,6). Copy the 300-byte `kLuoFill` array and use `embtex::ensure` (Steam-only, `_shared_src/EmbedTex.h`). More code; only if option 1 fails.
3. Ship a 4x4 TGA in `enemy_effects_strip_src/override/`: highest risk. New deploy step, delivery path unverified, `.txi` gap.

Rejected:
- `buffhud_circle` tinted black: disc, not a rectangle (doc 39 plan at `39_enemy_effects_strip.md:55` is wrong here).
- `uibit_fill_2wt` tinted black: alpha about 0.19, too faint.

## Open
- In-game check of `uibit_fill_2bt` on LBL_QUEUE1: whether the engine honours the 0.78 TPC alpha and the border tint. Static cannot prove it. Add a `PLAY_TEST_QUEUE.md` row; do not block.
- Mod `override/` to game `override/` copy step not located. `.txi` missing from ASSET_EXT.
- `shield_bar_src/manifest.toml:12` says `override/` but the files sit in `art/`.
- The enemy-effects-strip manifest is probe-only ("Draws and writes nothing", `enemy_effects_strip_src/manifest.toml:4-9`) and requires hud-host. The UX also needs a manifest change.
