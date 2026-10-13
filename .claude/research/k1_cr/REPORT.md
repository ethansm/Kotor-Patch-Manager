# Characters Revisited (CR, Laast, Nexus 1639): analysis for doc 34 / doc 38

Date 2026-10-12. Static, local-only. Nothing touched the game, repo assets or doc 34 code. Licence: unknown, every CR source is registered `requires_author_approval: true`; permission draft is unsent in `patch_manager_mods/k1_port/PERMISSION_REQUESTS.md`.
Extracted to `~/Games/KOTOR2_Mod_Workspace/k1_port_extract/characters_revisited/` (1,117 manifest rows, 10 shards, ~9 GB; portraits and items skipped). Data: `.claude/waves/w9-38-CR/` (`cr_vs_base_*.tsv`, `cr_models.tsv`, `cr_catalog.tsv`, `cr_disagree.tsv`, `cr_candidates.tsv`). Wave dirs are gitignored.

## Headline
CR is mostly **K1's original textures, upscaled and cleaned** (2048px), not new designs. K2 re-did many of those same-named textures (different face, outfit or UV layout), so "CR vs K2" looks like a redesign when it is really "K1 look vs K2 look". Only **13 CR TGA names exist in neither K1 nor K2** (n_commf33, n_smuggler A/B/C 01-04). Every species skin name (rodian, tusken, duros, wookiee, twi'lek ...) already exists in K2: CR is a **replacement** source there, not an extra-slot source (D3 Haiku guesses of "extra slot" for n_rodian05 etc. were wrong; checked against `cr_classified.tsv`).

## Counts (614 TGAs in the 9 texture shards)
| shard | palette | redesign | nobase | identical |
|---|---|---|---|---|
| head_player | 83 | 68 | 0 | 0 |
| body_player | 96 | 13 | 0 | 2 |
| species | 55 | 15 | 0 | 0 |
| faction_npc | 49 | 2 | 12 | 0 |
| commoner | 17 | 20 | 1 | 0 |
| creature | 54 | 18 | 0 | 0 |
| updates | 11 | 6 | 0 | 0 |
| party | 26 | 2 | 0 | 1 |
| other | 55 | 6 | 0 | 2 |
| **total** | 446 | 150 | 13 | 5 |

`redesign` = layout correlation vs the K2 (else K1) base below 0.9. Haiku visual pass (609 rows, 26 units): PALETTE 456, SAME 68, REDESIGN 69, UNSURE 16; 45 marked usable-as-extra. Blind re-rate (4 units, 155 rows = all 45 + 20% sample): 41 of 45 agreed; but rater 2 also said YES for 17 of 110 sampled "NO" rows, so rater 1 was strict and the YES set is a shortlist, not a verdict. 43 disagreements in `cr_disagree.tsv`; the Opus gate below used pixel correlation plus eye checks of ~15 rows.

## Models (21 MDL: 20 main + c_drdwar in updates)
All 21 **overwrite** K2 files of the same resref (K2 ships every one). Supermodels match; node counts differ by +-1 on pfhb04, pfhc01, pmhc01, comm_a_f2, and +3 on n_repsold_f. The "K1-unique" label for n_lashoweh, n_mekelh, n_shaleenah, n_repsold_f is wrong: K2 has all four (single unique-NPC rows 434/435/438, n_repsold_f used by rows 356-359,364,483,484). Reuse for generic spawns: only the head models, via the generic rows (pfhb03: 112-114,211; pfhb04: 115-117,212; pfhc01: 30,121-123; pmhc01: 26,31,166-168,316). K2 vertex counts in `cr_models.tsv` are unreliable (read as 0); ignore that column.

## Key risk (found at the gate)
CR heads and bodies are K1-layout. For heads pfhb03, pfhb04, pfhc01, pmhc01 CR ships a matching K1-layout MDL, so model+texture is self-consistent. For pfha01/04/05, pfhb01, pfhc05, pmhc03 there is **no CR MDL**: the K2 mesh uses the K2 layout (corr 0.15-0.66), so a same-name CR texture would mis-map. CR head textures also have **black eye sockets** (eye is a separate prop in K1 layout): needs the K1 eyeball mesh. K1-layout body atlases (pmbi*, pfbi*, pfbama01, pmbamb01) show a different outfit than K2 (e.g. K2 grey armour vs CR orange robe): replacement changes the whole look.

## Ranked shortlist (<=15; all licence-flagged: unknown, author approval required)
| # | stem | kind | K2 target | verdict |
|---|---|---|---|---|
| 1 | n_twilekfb01 | species body | Twi'lek F body, layout corr 0.84 (safe) | only true CR-original design (green quilted jacket); replacement |
| 2 | twilek_f01 | species | K2 pack name, corr 0.76 | new face-paint; replacement |
| 3 | n_smugglerA/B/C 01-04 (12) | faction body | model N_Smuggler: K2 rows 27, 385, 386, 444, 328-333 | NEW names; recolour variants of smuggler01-04 (C is near-dup of B); EXTRA slots |
| 4 | n_commf33 | commoner body | unknown model | NEW name, black tunic; EXTRA if a model uses it |
| 5 | n_comm_a_m | commoner | K2 comm_a_m rows 17,18,302,345,386,479,591,812; corr 0.86 | replacement; CR MDL also ships |
| 6 | n_utharh | head | other shard, K2 uthar head | new tattoo swirls; replacement |
| 7 | pfhb03 (+d, d1, d2) | head | row 112-114,211, head 33, CR MDL | self-consistent set; rename for EXTRA |
| 8 | pfhb04 (+d, d1, d2) | head | rows 115-117,212, head 34, CR MDL | as above |
| 9 | pfhc01 family | head (K1-only textures) | rows 30,121-123, head 36, CR MDL (K2 uses pfhc01a) | K2 lacks these textures; Track A |
| 10 | pmhc01 family | head (K1-only) | rows 26,31,166-168,316, head 51, CR MDL | K2 lacks these; Track A |
| 11 | pmhc03 / d1 / d2 | head | K2 mesh, corr 0.6, no CR MDL | layout risk |
| 12 | pfha04 family | head | corr 0.58, no CR MDL | layout risk |
| 13 | pfha05 family | head | corr 0.65, no CR MDL | layout risk |
| 14 | pfha01 family | head | corr 0.36, no CR MDL | layout risk |
| 15 | pfhb01 / pfhc05 families | head | corr 0.37 / 0.16, no CR MDL | layout risk |

## Ignore list
169 portraits (po_*, not game textures) and 90 item textures; 68 SAME rows; 456 PALETTE rows (sharper K1/K2 copies, replacement only); `c_holovandar` and other holo stems (CR is 2x2-tiled, broken); greyscale `*01b` maps (c_rakghoul01b, c_terantanak01b: height/normal maps, wrong format for a colour slot); creature palette swaps.
K1-only textures (K2 lacks them, CR ships hi-res): 95 head TGA rows (pfh*/pmh* d/d1-d3 variants; a few duplicated across packs), 10 body atlases (pfbal*, pfbas*, pmbam01), 16 party (p_bastilaba, p_candb*, p_carthba01, p_joleeb*, p_juhani*). These are the Track A (K1->K2 import) candidates and need K1 head/body MDLs.

## Open questions
1. TextureVar slots: can a renamed head texture (pfhb03 -> unused slot) be selected per spawn? Head selection code is absent in doc 34 (normalhead index only).
2. Species rows for Tier A (kAlienTable probe): not tested; n_rodian05, n_tusken04/05, n_duros01-03, n_wookief04/05 exist in K2, so no new species slot comes from CR.
3. Where does K2 bind the N_Smuggler variants (A/B/C): which model/head accepts them?
4. CR eye sockets: does the K1 eyeball prop render in K2 with CR head MDLs?
5. Wave budget: 32 of the 38 recon units peaked above the 60k context cap (range 42-85k; 6-sheet image units were the heaviest). For future image waves use <=4 sheets per unit. Whole run: 40 Haiku agents, $0.90.

## Next phase
`/plan-handoff 38:T-trackA`: head-model set (pfhb03, pfhb04, pfhc01, pmhc01) with the CR MDLs as a renamed extra set, plus smuggler A/B/C slots; or `34:Phase 3` after the play-test report.
