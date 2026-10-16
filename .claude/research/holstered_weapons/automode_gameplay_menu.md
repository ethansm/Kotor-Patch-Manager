# Auto-holster toggle: where it can go (recon, 2026-10-15)

Read-only. Sources: search_docs, query_ghidra symbol/address, game-dir override .gui read in memory (repo .venv pykotor), swkotor2.ini grep, B16 review rows. No headless Ghidra, no extraction.

## 1. The Gameplay options screen
- GUI: `override/optgameplay_p.gui` in the game dir (GFF). VERIFIED.
- Opened from `optionsmain_p.gui` BTN_GAMEPLAY [161,134] (main menu) and `optionsingame_p.gui` BTN_GAMEPLAY [161,206] (ESC menu). VERIFIED (layout dump).
- Panel class: `CSWGuiInGameGameplay`. GOG ctor 0x005E43C0, HandleInputEvent 0x005E5510, OnDefault 0x005E5A20, SetUpOptions GOG 0x005E60D0 (per B16 review). VERIFIED (knowledge DB + B16 rows).
- Tab opener `OnGameplayOpt` GOG 0x0058F900 (sibling `OnAutopauseOpt` 0x0058F9C0). Owner class of the opener: GUESS.
- Controls (ID TAG [left,top,w,h]): 1 CB_INVERTCAM [175,222]; 2 CB_LEVELUP [175,182]; 3 BTN_DIFFICULTY [175,142]; 8 CB_AUTOSAVE [175,262]; 9 CB_REVERSE [175,342]; 10 CB_DISABLEMOVE [175,382] ("Combat Movement"); 18 CB_REVERSE_INGAME [175,302]; 11 BTN_KEYMAP [161,470]; 12 BTN_MOUSE [161,435]; 16 BTN_BACK [159,516]; 17 BTN_DEFAULT [256,516]; 7 LB_DESC [407,134,232x364]. VERIFIED.
- No free row: checkboxes are 36 px tall on a 40 px pitch, last one ends at y=418, BTN_MOUSE starts at y=435. A 9th checkbox does not fit without moving BTN_MOUSE and BTN_KEYMAP. VERIFIED (arithmetic).
- Free room exists on `optautopause_p.gui` (below CB_TRIGGERS, y 406-470), but that is the Autopause page, so the wrong home. VERIFIED.

## 2. How existing toggles are stored and read
- Engine keys: `swkotor2.ini` section `[Game Options]` (lines 43-78): `Auto Level Up NPCs=0` (l.67), `AutoSave=1` (l.69), `Reverse Ingame YAxis=1` (l.73), `Combat Movement=1` (l.74). VERIFIED.
- Autopause has its own section `[Autopause Options]` (l.145). VERIFIED.
- Load/save: CClientOptions LoadOptions GOG 0x00465580, SaveOptions GOG 0x00462CC0, SetAutoLevelUpNPCs GOG 0x00462C60 (Steam ini reader 0x007B88B0 / writer 0x007B5F50 per doc 08). VERIFIED.
- Each checkbox handler flips one bit of the options object, then calls the setter. Example: OnAutoLevelUp GOG 0x005E5700 toggles bit0 and calls SetAutoLevelUpNPCs; OnAutoSave 0x005E5770 uses mask 0xFB (bit2). Source: `patch_manager_mods/steam_db_batches/B16/review/sonnet_c1.tsv:26,61`. VERIFIED.
- Mod-owned config precedent: holster.ini is read with fopen (`patch_manager_mods/holster_src/Holster.cpp:112`) and live-reloaded (l.272-274). VERIFIED.

## 3. Data-only or code?
- A `.gui` edit alone is inert. Extra controls do nothing unless the panel ctor binds the tag (`patch_manager_mods/RuntimeGuiControls.md:15`). VERIFIED.
- A real engine checkbox needs a ctor-bound tag, a handler and an option bit, so it is code (DLL detour). VERIFIED (follows from the above).
- `optgameplay_p` is in FRAME_GUIS of `scripts/generate_ultrawide_gui_overrides.py:167`, so a hand-edited .gui collides with the ultrawide generator. VERIFIED.
- A mod-drawn clickable control is the working path. No engine handlers exist for runtime labels; poll GetAsyncKeyState edges plus cursor extent (`RuntimeGuiControls.md:156-160`). VERIFIED.
- No tlk edit is needed for the label itself (label text can be drawn at runtime). GUESS.

## 4. Ranking, least effort and risk first
1. ini-only key: add `auto_holster=1` to `holster.ini`, read in the holster DLL with the existing live reload. Smallest change, no UI, no collision. Effort: about 1 Haiku wave (GUESS). Risk: no in-game toggle.
2. Separate mod settings page: a mod-drawn panel with runtime toggle, like the level-up options page. Needs an opener button and page state. hud-host (k2.hud, PopulateMenus hook) and gui-input (window and cursor provider) do not draw an options page, so they are dependencies only. Effort: several waves (GUESS). Risk: layout and click polling.
3. Extend the Gameplay menu: new engine checkbox with ctor binding, handler, a persisted [Game Options] key read by a hooked LoadOptions, a moved BTN_MOUSE/BTN_KEYMAP, and an ultrawide collision. Highest effort and risk. VERIFIED (structure), effort GUESS.

## Recommendation
Start with option 1 (ini key), then add an in-game toggle only if the user asks for one.
