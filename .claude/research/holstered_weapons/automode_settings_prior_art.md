# Auto-mode settings prior art (holster recon)

Read-only recon. Paths repo-relative. Result: toggles exist on the HUD and the character screen. No settings page and no shared settings API exist.

## 1. Existing in-game toggle / settings UI
1. Companion Self-Heal "Auto: ON/OFF" pill and RMB popover. Custom-drawn engine labels from `guikit::createLabelFromTemplate` (`_shared_src/GuiKit.h:175`), built in hud-host K2HUD_BUILD (`companion_selfheal_src/CompanionSelfHeal.cpp:1433`). Pill draw `:635`; row enum `_shared_src/SelfHealUi.h:148`; label `SelfHealUi.h:244`; click `CompanionSelfHeal.cpp:1191-1200` (plays vanilla checkbox sound).
2. Action Hotbar wrench "Edit" toggle, custom-drawn: `action_hotbar_src/ActionHotbar.cpp:1382-1386`, click `:1503`. A mode switch, not a setting.
3. Level-Up Override options page: On/Off rows "Instant" and "Bank for Jedi" as engine labels on the character screen (`level_up_override_src/LevelUpUi.h:63`). Ctor hook `0x84D82C` (`LevelUpUi.h:6`; `kotor2-steam-aspyr.hooks.toml:358`).
4. Engine-native, not mod-made: `CB_LEVELUP` "Auto Level Up NPCs" checkbox in optgameplay_p, handler `0x8E0C50` (`patch_manager_mods/08_level_up_override.md:332`). No `_src` file touches optgameplay, CSWGuiOptions or 0x8E0C50.
- combat_hotbar_src: INI-only `[Buttons]` settings, no in-game toggle (`combat_hotbar_src/CombatHotbar.cpp:17,29`). inventory_sort_filter_src: 0 toggle/button matches.

## 2. Persistence and reads
- Each mod owns one file in the game dir. No shared settings file.
- Writes: Self-Heal `saveIni` calls `filewatch::atomicWrite(IniPath, text, true)` (`CompanionSelfHeal.cpp:482-487`; `_shared_src/FileWatch.h:113`). It reloads before writing (`:1192`). `SelfHealUi.h:344` `serializeFile` keeps unknown lines verbatim.
- Action Hotbar `Config::Save` uses atomicWrite (`action_hotbar_src/HotbarConfig.h:450`). Level-up playthroughs use `.tmp` + MoveFileExA (`level_up_override_src/LevelUpOverride.cpp:69,1370`).
- Reads: Self-Heal `GetPrivateProfile*A` on `.\\companion_selfheal.ini` (`CompanionSelfHeal.cpp:101-105`). Holster uses `fopen("holster.ini")` with `filewatch::changed`/`accept` (`holster_src/Holster.cpp:112,272,407`).
- Bare names in GetPrivateProfile resolve to the Windows dir (comment at `CompanionSelfHeal.cpp:101`). Use `.\\` or an absolute path.
- No DLL reads another mod's ini. The only cross-mod ABIs are `k2.hud` (`_shared_src/HudHost.h:33`) and `k2.guiinput` (`_shared_src/GuiInput.h:20`).

## 3. Cross-mod settings API
- None exists. Only planned: `research/companion_levelup/MASTER_PLAN.md:5` ("configuration via an in-game GUI (settings page or new main-menu page)"). Not built.
- The per-mod debug-toggle idea lives only in user memory (`kotor2-ingame-debug-toggle-idea`). KpLog core is not in this repo.
- Reusable pieces: `_shared_src/IniKit.h:102,123` (enabledFlag / enabledFile); Self-Heal parseFile / serializeFile round-trip (`SelfHealUi.h:323,344`).

## 4. Recommendation (cheapest viable first)
1. Persist first: add `auto=0|1` to `holster.ini`, read in the existing `Holster.cpp:112` loader and hot-reloaded at `:272`. No UI, no new API. Hand edit only.
2. Toggle UI (target): copy the Self-Heal pill (k2.hud@1 K2HUD_BUILD, `guikit::createLabelFromTemplate`, LMB edge, `wPill`). Save with `atomicWrite` after a reload-before-write. Use a dedicated one-line `holster_auto.ini`, not `holster.ini`, because that file is comment-heavy and every write reloads its tuning keys.
3. Defer: engine options-page checkbox. It needs a new optgameplay panel ctor hook (only the 0x8E0C50 handler is documented; see `RuntimeGuiControls.md:28-34` for the InitControl window rule) and a home for the value. Highest cost.
- Do not build a settings API for one toggle.

## Verdict basis
- Toggles on HUD and character screen: yes. Settings page or shared settings API: no.
