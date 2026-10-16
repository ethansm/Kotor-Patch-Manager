# Holster key in the KOTOR II key-mapping menu (recon, read-only)

Question: can a mod add "Holster/Draw weapon" to Options > Controls > Keyboard?
Scope: project docs, query_ghidra symbol/address, chitin.key index (names and types only), swkotor2.ini [Keymapping]. No extraction, no headless Ghidra.

## 1. What defines the bindable action list
- VERIFIED: keymap.2da exists in the archives. chitin.key index: resref `keymap`, type 2017. Type 2017 is 2DA (`appearance` and `heads` are also 2017). No keymap file in `override/`. Contents not read.
- VERIFIED: loader is `CTwoDimArrays::Load2DArrays_Keymap`. GOG 0x00483510 (176b); Steam 0x006ea570 (batch B0 verified, `patch_manager_mods/steam_db_batches/B0/validated.tsv:1113`). Steam exe string `'keymap'` at 0x0099ae40 (`B0/evidence.tsv:169`).
- VERIFIED: menu class `CSWGuiInGameOptKeyMappings` (GOG 0x005eecd0, 1664b). Linux symbols: FillListBox, CreateOption, Update, UpdateEvent, IsKeyUsedAlready, OnAcceptClick. Per-row button `CSWGuiKeyMapButton` (GOG 0x005f0560; Linux GetActionId, SetDescription, ResetToDefault, PostChangeToEvent).
- GUESS: rows come from keymap.2da (loop over 2DA rows). Basis: a dedicated 2DA loader plus per-row ActionId and Description. query_ghidra gives no pseudocode, so the loop is unconfirmed.
- VERIFIED: layout file `optkeymapping_p` is type 2047 (GUI; known `mipc28x6_p` is also 2047). Not extracted.
- Input layer symbols (VERIFIED names only, role GUESS): `CExoInputInternal` (GOG 0x00610e10; Linux EndAttemptedBind, SetCaptureKeyboardKeys), `CExoInputClassBinding` (GOG 0x00611c00).

## 2. How bindings are stored
- VERIFIED: `swkotor2.ini` in the game root, section `[Keymapping]` (lines 80-144, 64 entries).
- VERIFIED format: `Action<ID><A|B>=<key code>`. A = primary, B = secondary. IDs run 204-286 with gaps. Values 7-85. Duplicate values exist (280A and 282A both 73).
- GUESS: key codes are DirectInput scancodes (e.g. 57 = space, 54 = right shift).
- OPEN: whether the game rewrites this block on exit (not checked).

## 3. Adding a row: data or code?
- Label and row (GUESS, data-only): add a keymap.2da row through `override/` with a dialog.tlk string. The repo has `scripts/test_02_merge_2da_and_tlk.py` for 2DA and TLK merges.
- Behaviour (code needed): nothing in the data runs an action. A key hook must read the bound key for the new ID and call the holster routine. No verified evidence that the dispatcher reads 2DA rows.
- Precedent: ExpandedKeyboardControl hard-codes GOG addresses for keyboard menus (`patch_manager_mods/06_gog_to_steam_conversions.md` section 4); skipped 2026-09-29.
- Verdict for a menu row: PARTIAL (display may be data-only; behaviour is not).

## 4. Fallback if the menu cannot take the row
1. Own key in `holster.ini` as a decimal VK, edge-detected with GetAsyncKeyState, gated like combat-hotbar (`.claude/research/holstered_weapons/hotkey_prior_art.md` lines 21, 37; `CombatHotbar.cpp:238`).
2. Own settings page via hud-host / gui-input: not verified here; hud-host gives frame and menu callbacks, not a key API (`hotkey_prior_art.md` line 16).
3. Do not write `[Keymapping]` from a mod until the on-exit rewrite question (section 2) is answered.

## Open
- Does `CSWGuiInGameOptKeyMappings` loop over keymap.2da rows, or use a constant? Needs decompile of FillListBox or Update, which query_ghidra does not return.
