# P4 UTC recipe: safe ambient Telosian commoner (recon, read-only, 2026-10-11)

Source: vanilla module UTCs via pykotor. Telos Citadel = "Citadel Station": 202tel Entertainment, 203tel Residential 082 East, 204tel Residential 082 West, 207tel Cantina.

## 1. Vanilla commoner fields (all 73 GFF fields dumped; differing + key fields shown)

| field | n_telf01 @202tel | n_telm01s @207tel | n_telf01s @203tel |
|---|---|---|---|
| TemplateResRef | n_telf01 | n_telm01s | n_telf01s |
| Tag | TelF01 | TelM01s | TelF01s |
| FirstName | strref 127356 "Telosian{F01}" | 127476 "Telosian{M01s}" | 127431 "Telosian{F01s}" |
| LastName / Description | strref -1, empty | strref -1, empty | strref -1, empty |
| Race / SubraceIndex | 6 / 0 | 6 / 0 | 6 / 0 |
| Gender | 1 (F) | 0 (M) | 1 (F) |
| Appearance_Type | 610 Commoner_Fem_Black_01_D | 17 Commoner_Mal_Asian_01_A | 590 Commoner_Fem_White_01_B |
| SoundSetFile | 35 (= Rodian_Male_Tough) | 35 | 35 |
| Conversation | 200comf | 200comm | 200comf |
| FactionID | 5 | 5 | 5 |
| Plot / Min1HP / NoPermDeath | 0 / 0 / 0 | 0 / 0 / 0 | 0 / 0 / 0 |
| Interruptable / Disarmable / IsPC | 1 / 0 / 0 | 1 / 0 / 0 | 1 / 0 / 0 |
| Hologram / WillNotRender | 0 / absent | 0 / absent | 0 / 0 |
| NotReorienting | 0 | 0 | 1 |
| PerceptionRange / BlindSpot | 11 / 120.0 | 11 / 120.0 | 11 / 120.0 |
| ChallengeRating / LawfulChaotic / GoodEvil | 1.0 / 0 / 60 | 1.0 / 0 / 60 | 1.0 / 0 / 50 |
| Str..Cha / WalkRate | 10 each / 7 | 10 each / 7 | 10 each / 7 |
| HitPoints / Current / Max / ForcePoints | 8 / 8 / 8 / 0 | same | same |
| BodyVariation / TextureVar / MultiplierSet / PaletteID | 1 / 1 / 3 / 3 | same | same |
| ClassList | Class 0 (Soldier), level 1 | same | same |
| FeatList | 93 (PROFICIENCY_ALL) | same | same |
| SkillList | 8 entries, only [3] Awareness = 2 | same | same |
| Equip_ItemList | g_a_clothes010 | g_a_clothes06 | g_a_clothes010 |
| ScriptSpawn | k_202_commoner | k_207_commoner | k_def_ambient |
| Heartbeat, OnNotice, SpellAt, Attacked, Damaged, Disturbed, EndRound | k_def_heartbt01, percept01, spellat01, attacked01, damage01, disturb01, combend01 | same | same |
| Dialogue, EndDialogue, Death, UserDefine, OnBlocked | k_def_dialogue01, k_def_endconv, k_def_death01, k_def_userdef01, k_def_blocked01 | same | same |
| ScriptRested | blank | same | same |

Fourth template n_telf03 @204tel (tag `commoner`, spawn k_defcom_spawn, GoodEvil 50) matches otherwise.

## 2. Uniqueness and names
- TemplateResRef: must be unique. n_telf01 is module-only (absent from chitin.key and Override). Candidate `ncr_telf01` is absent from chitin and Override. Max 16 chars.
- Tag: must be unique. Vanilla reuses tags (TelF01 x2 in 202tel; `commoner` across modules). Duplicates load, but WP_<tag>_nn walkpoints and GetObjectByTag become ambiguous.
- Inline FirstName works. Evidence: 2004 of 12755 vanilla UTC entries (incl. module copies) have FirstName strref -1 with inline English text (e.g. "Bao-Dur", "Canderous", "Darth Revan"). Round trip: `LocalizedString.from_english("Telos Resident")` -> `bytes_utc` -> `read_gff` gives strref -1 and text intact.
- Round trip `read_utc`/`bytes_utc` keeps all 73 vanilla fields. It adds defaults: ItemList [], Morale 0, MoraleBreakpoint 0, MoraleRecovery 0, Portrait blank, SaveFortitude 0, SaveWill 0, WillNotRender 0. All harmless.

## 3. FactionID
- repute.2da (vanilla data/2DA.bif, 24 rows). Row index = FactionID: 0 Player, 1 Hostile_1, 2 Friendly_1, 3 Hostile_2, 4 Friendly_2, 5 Neutral, 6 Insane, 19 Zone_Controller, 14 Endar_Spire, 18 Czerka (others: scripted or special).
- Use FactionID 5 (Neutral). Player row to Neutral = 50 (neutral band). Hostile_1 row to Neutral = 50, so hostiles do not auto-attack commoners.
- Hostile: 1, 3, 6 (Insane), 19 (Zone_Controller): Player row = 0. Party-aligned: 0 (Player). Do not use.
- Cross-check registry: faction 1 = Telos droids and Red Eclipse slavers (hostile), 2 = HK-50, 5 = commoners. .claude/npc_div/aliens/ has no faction IDs.

## 4. Recipe (clone vanilla n_telf01 @202tel, then override)
1. `utc = read_utc(bytes of n_telf01 from 202tel.mod)`
2. `utc.resref = "ncr_telf01"` (new, <=16 chars, verify absent from chitin, Override and all modules)
3. `utc.tag = "NcrTelF01"` (unique; WP_NcrTelF01_02.. only if walking)
4. `utc.first_name = LocalizedString.from_english("Telos Resident")`; keep `last_name` blank (strref -1)
5. `utc.race = 6`; `utc.gender = 1`; `utc.appearance_type = 610` (N_CommF; female model with gender 1)
6. `utc.soundset_id = 35` (keep vanilla; flagged in OPEN)
7. `utc.conversation = "200comf"` (vanilla generic, 16 placements, DLG in chitin); or `""` for no talk
8. `utc.faction_id = 5`; `utc.plot = False`; `utc.min1_hp = False`; `utc.no_perm_death = False`
9. `utc.interruptable = True`; `utc.is_pc = False`; `utc.will_not_render = False`; hologram 0
10. `utc.on_spawn = "k_def_spawn01"` (chitin; no WP_<tag>_02+ means it stands, INFERRED); keep all other k_def_* scripts as vanilla
11. Keep equipment (g_a_clothes010, chitin), ClassList, FeatList, SkillList, HP (8) unchanged
12. `bytes_utc(utc)` -> `Override/ncr_telf01.utc`; GIT placement (TemplateResRef + X/Y/Z only) is a module .mod edit

## 5. Immortal, hostile, or save-risk fields
- Plot=1, Min1HP=1, NoPermDeath=1: immortal or plot-locked. Keep 0.
- FactionID 0 (party), 1, 3, 6, 19: ally or hostile behaviour. Use 5.
- IsPC=1 or Interruptable=0: party or dialogue breakage. Keep 0 / 1.
- Reusing a vanilla TemplateResRef (n_telf01, n_telm01s, n_telf01s, n_telf03) in Override changes every placement of it in all modules and in existing saves.
- k_defcom_spawn, k_202_commoner, k_207_commoner are NOT in chitin.key. They exist only inside module archives (201-204tel, 207tel). Use chitin scripts only (k_def_*).
- Renaming or deleting a TemplateResRef after release breaks saved references (INFERRED).

## OPEN
1. In-game check: does k_def_spawn01 stand with no WP_<tag> waypoints (R2 says walkpoint call is unconditional; guard not read)?
2. Vanilla SoundSetFile 35 = Rodian_Male_Tough for all four Telosians (female models too). Keep for parity, or choose another row?
3. Added placements in areas already visited may not appear in existing saves (INFERRED, not tested).
4. Conversation 200comf content not read; registry says plot 0 across its 16 placements.

## VERDICT
Clone n_telf01 @202tel: FactionID 5, Plot/Min1HP/NoPermDeath 0, IsPC 0, Interruptable 1, chitin k_def scripts only, unique resref and tag, inline FirstName.
