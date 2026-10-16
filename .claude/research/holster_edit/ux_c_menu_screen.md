# UX-C recon: where a holster editor can live (options / main menu), 2026-10-16

Read-only. Steam addresses (sha 6a522e71) from static dumps `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71`
(`decomp/<xx>/<addr>.c|.asm`, `callgraph.tsv`, `strings.tsv`). GOG names from `inputs/gog_functions.tsv`.
GUI tags and extents read from the live `override/*.gui` with repo `.venv` pykotor. No live REA, no edits.
Names marked GUESS are inferred from call shape or string, not from a Steam symbol.

## 1. Options menu structure

| screen (.gui) | opened from | Steam ctor | size (= GOG size) |
|---|---|---|---|
| `optionsmain_p` (main menu Options) | `mainmenu8x6_p` BTN_OPTIONS -> `CSWGuiMainMenu::OnOptionsPicked` (GOG 0x53D5B0; Steam 0x882B10 GUESS) | `CSWGuiOptionsMain` 0x007D23E0 | 1637 (= GOG 0x567D70) |
| `optionsingame_p` (ESC / top-bar Options) | `CSWGuiInGameMenu` tab, see chain below | `CSWGuiInGameOptions` 0x008A1170 | 1983 (= GOG 0x58EC80) |
| `optgameplay_p` | BTN_GAMEPLAY on either screen | `CSWGuiInGameGameplay` 0x008DF930 | 3659 (= GOG 0x5E43C0) |
| `optfeedback_p` | BTN_FEEDBACK | 0x007D31D0 (name by string) | 1021 |
| `optautopause_p` | BTN_AUTOPAUSE | `CSWGuiInGameAutoPause` 0x008E1810 | 1702 |
| `optgraphics_p` | BTN_GRAPHICS | `CSWGuiOptionsGraphics` 0x007D4620 | 3078 (= GOG 0x569EF0) |
| `optsound_p` | BTN_SOUND | `CSWGuiOptionsSound` 0x007D9800 | 2959 |
| `optsoundadv_p` | sound OnAdvanced | `CSWGuiOptionsSoundAdvanced` 0x007DACB0 | 2007 |
| `OPTKeyMapping_p` | gameplay BTN_KEYMAP | `CSWGuiInGameOptKeyMappings` 0x008FFF20 | 1664 (= GOG 0x5EECD0) |
| `optmouse_p` | gameplay BTN_MOUSE | `CSWGuiOptionsMouse`: Steam UNKNOWN (GOG 0x571D20) | - |

Anchors: `strings.tsv:4148` (optionsmain_p -> 0x7D23E0), `:4884` (optionsingame_p -> 0x8A1170), `:5110` (optgameplay_p),
`:5118` (optautopause_p), `:5196` (OPTKeyMapping_p). Sub-screen openers (178 B each, call the ctor above):
0x7D2D30 gameplay, 0x7D2DF0 feedback, 0x7D2EB0 autopause, 0x7D2F70 graphics (GOG names by structure: OnGameplayOpt
0x58F900, OnAutopauseOpt 0x58F9C0, OnFeedbackOpt 0x5686C0, OnGraphicsOpt 0x568780, OnSoundOpt 0x568840,
`gog_functions.tsv:3043-3045, 3312-3313`). In-game copies: 0x8A1DF0 graphics, 0x8A1EB0 sound, 0x8E0FC0 keymap (GUESS).
Click dispatch: GOG `CSWGuiOptionsMain::HandleInputEvent` 0x568550, `DispatchOptionButton` 0x568680 (Steam UNKNOWN).

Entry chain, in game (Steam): 0x810CF0 (caller, GUESS) -> 0x811450 (329 B, takes a byte) -> 0x73F870 -> 0x78C330 ->
0x7BE4C0 (5472 B; builds top-bar tabs, calls equip 0x8A92D0 and options 0x8A1170). Tab switch 0x7D0760 (865 B, ret 8)
builds a tab panel on first use. Top bar is one singleton `CSWGuiInGameMenu` (`08_shield_bar.md:512`; GOG ctor 0x5265D0).

Mods on vanilla menus:
- No mod touches `optionsmain_p`, `optionsingame_p`, `optgameplay_p` or `mainmenu` (rg of `*_src` finds none). Only
  engine-native `CB_LEVELUP` handler 0x8E0C50 is documented (`08_level_up_override.md:332`).
- Mods DO add controls to other vanilla panels with the clone-template recipe (`RuntimeGuiControls.md`): equip screen
  ctor 0x8A92D0, hook H-E1 0x8AA16A (`EquipCharPreview.cpp:8`); char-screen level-up block (`LevelUpUi.h:63`, hook
  0x84D82C); HUD pill (`CompanionSelfHeal.cpp:1433`).

How to add a "Holster placement" button and a sub-panel:
1. Create the label in the panel ctor window (clone a template, `GuiKit.h:175` createLabelFromTemplate, then
   `appendToPanel` `GuiKit.h:209`). Hook must sit after the last InitControl and before StopLoadFromLayout.
   - `optionsingame_p`: last InitControl call 0x8A16EA; StopLoadFromLayout call **0x008A1704**, ECX = `[ebp-0xAC]`
     (asm `decomp/8a/008a1170.asm` lines 347-348).
   - `optionsmain_p`: StopLoadFromLayout call **0x007D27E6**, ECX = `[ebp-0x84]` (asm `decomp/7d/007d23e0.asm` 253-254).
2. Free button slots from the .gui extents (LEFT,TOP,W,H):
   - optionsmain_p: BTN_* at x161 y134/170/206/242/278 (h32); BTN_BACK y516. Free: (161,314,236,32) and below to y~490.
   - optionsingame_p: BTN_* y134..386 (SAVEGAME, LOADGAME, GAMEPLAY, FEEDBACK, AUTOPAUSE, GRAPHICS, SOUND, QUIT ends 418),
     EXIT y516. Free: (161,422,236,32) and (161,460,236,32). LB_DESC and LBL_BAR5 start at x407, no overlap.
3. Click: no engine handler for a new tag (`RuntimeGuiControls.md` section 7). Poll `GetAsyncKeyState` edges with
   `GuiKit.h:430 clickEdge` and `:423 hit`. Whether `DispatchOptionButton` ignores unknown tags: GUESS, not traced.
4. Sub-panel: a button cannot push a panel without a panel object. Two routes:
   - (a) Host the editor inside the same panel: clone the viewport and labels in the ctor window, hide vanilla BTN_*
     with `setVisible(false)` (`GuiKit.h:257`). This is the equip3d pattern (`EquipCharPreview.cpp:883-891`).
   - (b) A new standalone CSWGui panel with its own layout: no precedent in the repo, untested.

Esc on the options screens: UNKNOWN. Game WndProc 0x40A750 handles VK 0x1B only for movies (`r1_camera_keys.md` section c).

## 2. Main menu viewport without a save

- `mainmenu8x6_p.gui` already holds `LBL_3DVIEW` (label, built by 0x881970; `strings.tsv:4720`). The main-menu ctor
  0x880740 uses strings `gui3D_room`, `mainmenu01..05`, `default`, `camerahook` (`decomp/88/00880740.c:164-193`). So a
  live SceneView with a camera hook already runs on the main menu. Rendering a body there is structurally the same path.
- No save is needed to build a body. Equip3d builds a fresh `CSWCCreature` (ctor 0x7645F0, size 0x470, vtable 0x99EE14,
  `EquipCharPreview.cpp:55-57`), then calls 0x76D200 (BuildApp) with a 0x3C-byte appearance blob (`:62`). The blob is
  normally copied from the real creature at `+0x224` (`:58`). Blob size 0x3C (`:66`), BuildApp 0x76D200 (`:61`).
  Appearance row is at blob `+0x18` (`:587`), head item at `+0x10` (`:721`). A main-menu version can write its own blob
  with a chosen row and call the same 0x76D200.
- Weapons: `0x462320` = cdecl(name, type string 0x9A4AB0, 0, 0) -> Gob* (`EquipCharPreview.cpp:40, 512`). Attach to the
  body with vt[0x50](body, "rhand"/"lhand", 0). Works with w_* names chosen directly, no creature needed.
- CORRECTION to the brief: `PMBxx` is not in this install. `42_holstered_weapons.md:63` lists PMBA, PFBA, P_BastilaBA,
  P_CandBA, P_CarthBA, P_JuhaniBA as NOT FOUND. Working bodies are `N_CommM` and `N_CommF`.
- Lost without a save:
  1. the real party member's appearance row, tint and head (blob from real `+0x224`);
  2. alignment idle: stats `+0x80`, else neutral 50 (`EquipCharPreview.cpp:560-566`);
  3. equipped right/left items (`+0x238`/`+0x23c`, `:41`) and the helmet item;
  4. camera-hook choice by tag (t3m4, hk47, 3cfd, zaalbar; `10_equipment_3d_character.md:65`);
  5. per-body holster tuning (`HolsterTune` keyed by body). Tuning for real party bodies needs the real bodies.

## 3. In-game Options vs main menu

| | in-game `optionsingame_p` | main menu `optionsmain_p` |
|---|---|---|
| real party bodies | yes: `ClientPartyTable` 0x73FB90, `PartyGetAt` 0x7E5DA0, `GetControlledCreature` 0x7E5D80 (`GameAddr.h:33-36`) | no module, so no party (GUESS, not checked live) |
| pause | user says yes; NOT verified statically. GOG `CGuiInGame::SetPaused` 0x4DC4B0 (`gog_functions.tsv:1641`), `CClientExoApp::SetPauseState` 0x40CB60 (`:107`). Steam mapping not in DB | n/a, no world |
| entry | ESC or top-bar tab (chain in section 1) | BTN_OPTIONS |
| Esc / close | UNKNOWN for the options screens. Vanilla Esc closes the character screen (`08_level_up_override.md:627`) | UNKNOWN |
| ctor hook | 0x008A1704 | 0x007D27E6 |

## 4. Settings precedent and a generic provider

- No mod has a settings page and no shared settings API exists (`automode_settings_prior_art.md` section 3).
  Precedents are in-panel toggles: Self-Heal pill and RMB popover (`CompanionSelfHeal.cpp:1191-1200, 1433`), Action Hotbar
  Edit toggle (`ActionHotbar.cpp:1382-1386`), level-up Instant/Bank rows (`LevelUpUi.h:63`).
- Each mod owns its INI and writes with `atomicWrite` (`_shared_src/FileWatch.h:113`; `holster_src/Holster.cpp:112`).
- Debug switches today: flag files (`npc-diversity_debug.txt`, `mouseprobe.txt`) or `kplog.ini` (WI-24 C146). The
  in-game toggle idea is only in user memory (`kotor2-ingame-debug-toggle-idea`).
- Cross-mod ABIs exist: `k2.hud` (`_shared_src/HudHost.h`), `k2.guiinput` (`GuiInput.h`), `k2.holster` (`HolsterApi.h`,
  optional for equip3d).
- KPM RFC draft `patch_manager_mods/19a_rfc180_comment_DRAFT.md:40` already names "Mod Options" as interface 5 on the
  same `provide`/`require` registry. A generic "k2.modsettings" provider fits that exact pattern, but only with two or
  more consumers (holster editor plus KpLog debug switches). For the holster editor alone, keep the page in the holster DLL.

## Feasibility

FEASIBILITY: medium. Main blocker: no standalone sub-panel host. The proven host is a ctor-window clone into an existing
vanilla panel (options main 0x7D27E6, in-game options 0x8A1704, or the equip panel where HW-EDIT is planned). Second
blocker: the pause and Esc behaviour of the options screens is not traced in the Steam dump.

## Open
- Steam addresses for `OnOptionsPicked`, `DispatchOptionButton`, `CSWGuiOptionsMouse` and `SetPaused` (not in DB).
- Does the in-game Options screen pause the world, and what does Esc do there? Needs a live check or a Steam decompile.
- Does the main-menu party table stay empty (no PC creature)? Needs a live check.
