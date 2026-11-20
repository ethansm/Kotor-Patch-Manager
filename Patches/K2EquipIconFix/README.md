# Equipment Screen Icon Reload Fix (`k2-equip-icon-fix`)

> **STATUS** Track: SHIPPABLE. Version 0.1.1 (matches manifest.toml).
> **What it does / explores:** Stops the equipment screen from destroying and re-reading the equipped weapon's icon texture every frame. The engine's per-frame refresh (0x008AD930) resets weapon slot images to placeholders and then sets the item icon on the same control; this patch defers the placeholder resets and drops any that are superseded in the same pass. Each slot still ends every frame showing what the engine would show. About 2.6 ms/frame saved with loose override icons.
> **Requires:** Steam/Aspyr LAA build, SHA `6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF` (manifest.toml `[patch.supported_versions]`). Patch Manager (KPM). No other patch is required and no hooks are used (DLL-only).
> **Known limits:** Only the Steam/Aspyr LAA build is listed; GOG is not covered. The host test covers shared helpers only; the defer/set/pass logic (`EquipIconFix.cpp`, EquipFixDefer/Set/Pass) has no host seam yet. The DllMain refuse branches (-1/-2/-3) are untested. Adjacent to equipment_3d_character's detour at 0x008ABA50 (0x3C away): no shared bytes, but co-install is not tested here. **Play-test status:** no row in `patch_manager_mods/PLAY_TEST_QUEUE.md` yet; the equipment-screen check is pending until one is added.
> **Install:** Install through KPM like any DLL patch. To turn it off without uninstalling, put `enabled=0` in `equipiconfix.txt` in the game folder (the 8 call sites are then left untouched).

## Features
- Re-targets 8 existing `CALL rel32` sites in the equipment panel update: one pass entry (0x008ABA8C), three placeholder-set sites that defer (0x008ADA08, 0x008ADA81, 0x008AE793), and four item/greyed-icon sites that cancel a deferred set (0x008ADFA2, 0x008AE02D, 0x008AE19A, 0x008AE1CD).
- Verifies all 8 expected CALL byte sequences before writing any of them. On a mismatch it refuses and logs `REFUSED(...)`, leaving the game unchanged.

## Configuration
- `equipiconfix.txt` in the game folder:
  - `enabled=0` disables the patch (default: enabled).

## Logging
- Uses the shared KpLog helper, channel `main`. The load line and `tick=` come from `KPatchInit`; a stats thread appends `STATS` lines and the first 16 `CANCEL` events.
- `equipiconfix_log.txt` is only the fallback file for the old core (no `KPatchInit`).

## Hooks
- Byte patches only (no KPM hooks). Sites are listed in `code_patches.toml` and `kotor2-steam-aspyr.hooks.toml`, with original bytes and the source lines that verify them.

## Build/test
- `python3 scripts/run_tests.py equip_icon_fix` (common contract test, 37 checks).

## What was learned
- The engine's image setter (0x00414840) only skips work when the resref is unchanged. The placeholder-then-item alternation in 0x008AD930 changes the name every pass, so the icon texture is destroyed and re-created each frame.

## Changelog
- No CHANGELOG.md yet. Version history is in `git log -- patch_manager_mods/equip_icon_fix_src`.
