# 02 Alien NPC audio: soundsets, VO, TLK (static recon, 2026-10-11)

Read-only. Game dir untouched. Parsed with repo `.venv` pykotor (Installation, read_2da, read_ssf, read_tlk, read_erf, read_dlg). Scripts and raw outputs: `scratchpad/rec02/` (scan.out, tlk.out, dlg.out, utc.out, alienvo.out).
Game dir: `/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II`

## 1. How a creature gets its voice

Chain: UTC `SoundSetFile` (uint16 row index) -> `soundset.2da` row -> `<resref>.ssf` (28 event slots, each a dialog.tlk StrRef) -> TLK entry (text + voiceover ResRef) -> WAV.

- UTC field: `SoundSetFile` (pykotor `resource/generics/utc.py` L85, L391).
- `soundset.2da` (CHITIN, 70 rows, 0-based). Columns: `label resref strref gender type`. `type` is empty on every row. `gender` is 1 on female rows (row 37 Rodian_Female, row 31 Twilek_Female_K2, row 57 Commoner_Female).
- SSF files for vanilla rows are in `data/Templates.bif`. The only loose SSF in the game is `Override/c_hutt.ssf`.
- SSF event order (pykotor `ssf_data.py` L138+): BATTLE_CRY_1-6, SELECT_1-3, ATTACK_GRUNT_1-3, PAIN_GRUNT_1-2, LOW_HEALTH, DEAD, CRITICAL_HIT, ... (28 total).

Alien rows (soundset row: label, SSF resref, SSF strrefs):
- 25 Generic_Alien `a_aliengen`; 26 Aqualish `a_aqua`; 27 Chandra_Fan `a_chandra`; 28 Weequay `a_weequay`; 32 Trandoshan `a_trando`; 33 Sullustan `a_sullust`; 34 Sith_Assassin `a_sithass`; 38 Nikto `a_nikto`; 40 Gran `a_gran`; 41 Gand `a_gand`; 42 Gammorean_K2 `a_gamor`; 44 Devaronian `a_devar`: 4 events each.
- 29 Twilek_Male_Tough `a_twitough` (4 events); 30 Twilek_Male_K2 `a_twimale` (8 events, BATTLE_CRY_2 = 132406); 31 Twilek_Female_K2 `a_twifem` (gender 1, 8 events).
- 35 Rodian_Male_Tough `a_rodtough`: strrefs 130684 (bat), 130685 (atk), 130686 (dam), 130687 (die).
- 36 Rodian_Male `a_rodmale`: strrefs 130679, 130680, 130681, 130682.
- 37 Rodian_Female `a_rodfem` (gender 1): strrefs 130674-130677.
- 39 Ithorian_K2 `a_ithor`: strrefs 130660-130663.
- 43 Duro `a_duro` (4 events). 58 Bith_K2 `a_bith`: strrefs 130619-130622.
- Commoners: 11 Commoner_Male `n_commm` (6 events); 56 Commoner_Male_Kid `n_commonkm` (132418-132421); 57 Commoner_Female `n_commonf` (132413-132416).

UTC examples (Templates.bif): `n_rodian` -> row 36; `n_bith` -> 58; `n_duros` -> 43; `n_twilekm` -> 30; `n_twilekf` -> 31; `n_commm` -> 11. No UTC exists under `n_commonf`, `n_rodtough`, `n_ithorian`, `n_rodmale`.
- `c_hutt` UTC -> row 15 (Republic_Soldier), not row 69. Row 69 (`c_hutt`, strref -1) is linked to `Override/c_hutt.ssf` (strrefs 38231-38238). Vanilla UTC does not use row 69.

## 2. Alien voice layers

Layer A, SSF combat voice (WAV in `StreamSounds/`): e.g. `A_RODMALE_BAT.wav`, `a_ithor_bat.wav`, `A_BITH_DIE.wav`, `A_DURO_ATK.wav`. Gap: no `N_COMMONF_BAT.wav` in StreamSounds (`n_commonf_bat` is 132413; `N_COMMONF_die.wav` exists).

Layer B, `alienvo.2da` (vocal emotes; `Override/alienvo.2da`, binary 2DA V2.b, 43 rows x 36 cols; same dims as CHITIN copy; row 14 Ithorian `angry_long` = `AVO_IthoAngL` in both).
- Columns are emotion/ambient categories: angry_long..laughter_short, comment_generic_*, greeting_*, silence, wounded_*, screaming_*.
- Rows are voice types: 2 Bith, 7 Duro, 13 Hutt, 14 Ithorian, 17 Rodian_Female_Generic, 18 Rodian_Male_Generic, 19 Rodian_Male_Tough, 23 Twilek_Female_Generic, 24 Twilek_Male_Generic, 25 Twilek_Male_Tough, 40 Generic_Alien.
- Cells are base names like `AVO_RodiFAngL`. Files are `StreamVoice/AVO/<folder>/AVO_RodiFAngL1.wav` (suffix 1..3).
- `StreamVoice/AVO/`: 1428 wav + 8 mp3 in real folders. Lowercase names are symlinks (e.g. `_rodifan -> _RodiFAn`). Mp3 seen in `_selkgre/`, `_jawhap/`.
- Wav-only coverage check: 0 missing cells for Rodian, Twi'lek, Ithorian, Duro, Bith, Hutt, Gamorrean, Gand, Gran, Sullustan, Devaronian rows. Missing: Chandra-Fan 18, Rakata 24+24, Selkath 23+23 (some mp3), Wookiee 15 x3, Tusken 10, Jawa 6, Nikto 1, Quarren 1, Weequay 1.

Layer C, DLG nodes carry AVO names with no TLK text:
- `302nar.mod` `cordrgrecip.dlg`: entry nodes vo_resref `AVO_DuroGretS1`, `AVO_DuroCmntM3`; strref -1 (no text).
- `972cor.mod` `corzerkdaesh.dlg`: `AVO_TwilFCmntM2`, `AVO_TwilFLaffS1`, `AVO_TwilFMockS1`, `AVO_TwilFCmntS1`; strref -1.
- 27 `Modules/*.mod` files contain `AVO_` bytes (rg, case-sensitive).
- TLK: only 2 entries carry AVO voiceover: 133976 (`AVO_GranCmntS1`, text "You can go in.") and 133977 (`AVO_granangs3`, "Keep quiet about this place.").
- Counter-example: DLG node strref 109813 has DLG `vo_resref` `gblmandalore101` while its TLK voiceover is empty. The DLG field can carry audio alone.

Alien-language text: none found. TLK keyword scan (Huttese, Ryl, Rodese, Ithorese, Shyriiwook) hit only descriptive text, e.g. 84411 "...in Huttese". "Ryl" hits are Ryloth/Rylith. Alien speech is English text or no text over vocal clips.

## 3. Barks

- No barks table. `barkstrings.2da` is absent from the CHITIN key. Key names with bark/ambient/voice: `barkbubble_*` (textures), `avo_*`, `alienvo`, `aliensound`, `ambientsound`, `soundsettype`.
- Barks are SSF battle-cry slots pointing at TLK entries whose voiceover is the bark WAV (e.g. 130684 -> `a_rodtough_bat`).
- TLK text for these entries is a developer placeholder: 305 TLK entries contain "SHOULD NOT BE TRANSLATED" or "Do not translate". Examples: 130684 "SHOULD NOT BE TRANSLATED - Rodian Male Tough battlecry", 38231 "Battlecry 1 - Do not translate this text."
- Whether the engine displays bark text (floating text): UNSURE (static evidence only).
- `swkotor2.exe` strings: `VoiceChat` and `Voiceover` present; `VO_ResRef` and `SoundResRef` 1 hit each. ASCII `alienvo`, `soundset.2da`, `StreamVoice`, `dialog.tlk` are absent, so the loader path is not confirmed.

## 4. Swapping text without touching audio

- `dialog.tlk` lives only in the game root (no Override path). A strref edit is global: every user of that strref changes.
- pykotor `TalkTable.replace(stringref, text, sound_resref="")` (`tlk_data.py` L163-179): empty `sound_resref` keeps the old voiceover, empty `text` keeps old text. A text-only swap keeps audio.
- Repo precedent: `scripts/append_ranged_kite_tlk.py` (idempotent append, backs up dialog.tlk; `patch_manager_mods/13_ranged_ai_spacing.md` L77, L149; new strrefs 136538/136539).
- Per-NPC option: append a new strref and repoint the DLG node's text strref. DLG `vo_resref` stays untouched, so audio stays. Depends on OPEN item 1.
- KPM TLK patch support: no hits in `BUILD_AND_INSTALL.md` or `MASTER_KEY.md` for `tlk`.

## 5. Repo docs checked

- `34_npc_diversity_dispatcher.md`: no VO, TLK or soundset content. Only diversity-mod table rows (L109-112) and story tags (L266).
- `38_k1_to_k2_asset_import.md` row 11: K1 voice is MP3-in-WAV in `streamwaves`; TSL needs a DLG subfolder (REFUTED as stated). Not re-verified here.
- `37_bug_board.md` L71: soundset.2da staged-vs-live diff listed as an investigate item.

## Open

1. Engine precedence when DLG `vo_resref` and TLK voiceover both differ (needs rea-recon on `swkotor2.exe`).
2. Loader for `alienvo.2da` and the cell-to-file rule (`AVO_X` -> `X1..X3`, folder mapping).
3. Whether bark/battle-cry text is ever drawn on screen.
4. `n_commonf_bat.wav` not in StreamSounds; check `Sounds.bif`.
5. Which Hutt SoundSet the game uses: `c_hutt` UTC (row 15) vs row 69.

## Verdict

PARTIAL. Audio and text are separate fields (TLK strref -> voiceover; DLG vo_resref). A text-only swap keeps audio. Bark text is a placeholder, and AVO voice nodes have no text to change.
