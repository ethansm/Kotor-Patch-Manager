# R4: UTC, dialogue and Wookiee voice for a new alien template (recon, read-only, 2026-10-11)

Prior recon 00, 01, 02, 04 (K2v check 5), 05 cited, not repeated. Game dir: `ls` only. pykotor: repo `.venv`, source reads only.
Corrections to prior: 01 L66 lists soundapptype 1 only on rows 275-278; rows 271-274 are also 1. 02 "Wookiee 15 x3" = 15 cells missing on each of rows 28, 29, 32, 33 (scratchpad rec02/scan.out L107).

## 1. Wookiee appearance rows (Q1)
Source: `patch_manager_mods/npc_diversity_src/override/appearance.2da` (repo copy; live `Override/appearance.2da` not read).
| row | label | modela/texa | walkdist/rundist | soundapptype | vanilla placements |
|---|---|---|---|---|---|
| 50 | Alien_Wookie_Female_01 | N_WookieF | 1.7/5.4 | 0 | 1 |
| 275-278 | Alien_Wookie_Female_02..05 | N_WookieF | 1.7/5.4 | 1 | 0 |
| 271 | Alien_Wookie_Male_02 | N_WookieM | 1.9/5.4 | 1 | 7 |
| 272 / 273 / 274 | Alien_Wookie_Male_03..05 | N_WookieM | 1.9/5.4 | 1 | 3 / 2 / 2 |
| 51 | Alien_Wookie_Male_01 | N_WookieM | 1.9/5.4 | 0 | 0 |
| 327 / 427 | Lite_Wookie_Male / Lite_Wookie_Female | N_WookieM (427 label F, modela M) | 1.9 / 1.7 | 1 | 0 |
All Wookiee rows: modeltype F, normalhead blank (head baked into mesh, no heads.2da gate), footsteptype 0, moverate NORM.
- Body N_WookieF: base-game KEY (01 row 50, VERIFIED). TPC wookief01-04 base game (01 L37).
- Body N_WookieM: K2 model `n_wookiem` (`.claude/review/k1_import/K2v_layout_evidence.md:55`), vanilla modules place it on rows 271-274 (registry). INFERRED present.
- Textures N_WookieM01-06.tpc: loose in live Override (`ls`). Base-game erf copy UNVERIFIED.
- Row 271 as commoner-slot body: loads (7 vanilla placements, 5 modules). Walk 1.9 vs commoner 1.813 (01 L63): near match. Run 5.4 vs 3.66: about 47% faster. Supermodel S_Female02, same as n_commm (K2v L55 table). Anim set INFERRED same as commoner.
- Verdict: 271 usable as-is for body/anims; run pace differs; no head to gate.

## 2. UTC fields (Q2)
Source: `.venv/lib/python3.12/site-packages/pykotor/resource/generics/utc.py` attribute docstring L59-189; `read_utc` L837, `write_utc` L847, `bytes_utc` L859. GIT: `generics/git.py` L1205-1216.
- Template-supplied (no module edit needed for the template itself): TemplateResRef L59, Tag L61, Conversation L65 (DLG resref), Race L75, Appearance_Type L77 (appearance.2da row), Gender L79, FactionID L81, WalkRate L83, SoundSetFile L85 (soundset.2da row), NoPermDeath L97, Min1HP L99, Plot L101, Interruptable L103, ScriptHeartbeat L151, ScriptDialogue L165, ScriptSpawn L167, Hologram L177 (K2 only; leave 0).
- Ignore: PaletteID L179 (toolset), BodyBag L181 and Phenotype L189 (not used by engine, per docstrings).
- Not in template: position, bearing, and the placement. GIT Creature List holds only TemplateResRef + X/Y/Z position + orientation (git.py L1207-1216). Placement = module `.mod` edit (GIT not overridable, 05 section 5).
- Override-only paths: (a) script spawn (CreateObject of a new template resref; needs a script, not a module edit); (b) replace an existing vanilla template in Override, which changes every GIT instance using that resref (Wookiee templates appear 1-2 times each).
- Vanilla Wookiee baseline (registry, 15 placements, `.claude/npc_div/registry.jsonl`, fields from `scripts/npc_div_registry.py` L172-175): race 6 (all 14 named rows), faction 5, plot 0, interruptable 1. ScriptSpawn: k_def_spawn01 x9, k_def_spellat01 x2, k_def_wookspn x2, blank x2. Reusable vanilla spawn scripts exist; no new script needed.

## 3. Dialogue and voice (Q3)
- Conversation to Override .dlg: a new `<resref>.dlg` in Override is what the template's Conversation loads (Override precedence). Live precedent: `Override/200ithr.dlg` (05 section 2). INFERRED, not tested in game.
- DLG tooling: pykotor `read_dlg` (`generics/dlg/io/gff.py` L565), `write_dlg` L592, `bytes_dlg` L620. Yes, a DLG GFF can be written. Repo: no DLG authoring script. `scripts/05_merge_dlgs_and_jrl.py` splices DLG GFFs with `read_gff`/`write_gff` (imports L16-17). Reader only: `scripts/npc_div_registry.py` L172-175.
- Voice: node `VO_ResRef` names a WAV. Text comes from the TLK strref. A node can carry voice alone (02 Layer C). Vanilla alien AVO nodes use strref -1: voice only, no subtitle (`302nar` `cordrgrecip.dlg`, 02 Layer C).
- Wookiee AVO on disk: `StreamVoice/AVO/_wookhap/` (`ls`) holds 4 clips: `avo_wookhapyl1`, `avo_wookhapym1`, `avo_wookhapym2`, `avo_wookhapys1`. Happy growls only. Usable base names: AVO_WookHapyL1, AVO_WookHapyM1/M2, AVO_WookHapyS1.
- Alienvo Wookie rows 28, 29, 32, 33 (02 alienvo.out L33-38) name cells such as `AVO_WookMHapyM` and `AVO_WookMAngM`. Scan: 0 of 60 resolve (02 scan.out L107). Cell names (WookM/WookF/WookMT/WookMW) do not match disk names (WookHapy). Alienvo Wookie rows are silent.
- Combat WAV: `StreamSounds/` (832 entries, `ls`): no Wookiee clip (only `amb_swoopidle_A` matches wook|woo). Sounds.bif not checked.
- Soundset: no Wookiee-labelled row in 02's soundset list (rows 25-44, 56-58). Candidate reuse: 25 Generic_Alien `a_aliengen` (4 events) or 11 Commoner_Male `n_commm` (6 events). Vanilla Wookiee SoundSetFile: UNKNOWN (Templates.bif not read).
- Text: append-only TLK precedent `scripts/append_ranged_kite_tlk.py` (02 section 4, 05 section 4). Never edit an existing strref (05 section 5).

## 4. Wookiee speech convention (Q4)
- Repo-wide rg for Shyriiwook/shyriiwook: 0 hits.
- 02 TLK keyword scan: no Shyriiwook translation rows (descriptive hits only).
- Vanilla alien subtitles exist: Ithorian `200ithr` lines are subtitle-only, VO files absent (05 section 2).
- Vanilla Wookiee conversations to check: genericwook1, plead_wook2, slavedial4, woorwill, holderoflaws, kas22_wookg1_01, kas22_wookg2_01 (registry). DLG text not read: game dir restricted.
- VERDICT for Q4: UNSURE.

## 5. Next single read (needs approval)
One read-only pykotor `read_dlg` pass over the 7 conversations in section 4, plus `read_utc` on the 271 template (e.g. n_slave10_wm). Would settle Wookiee SoundSetFile and subtitle convention.

## Open
1. Vanilla Wookiee SoundSetFile and DLG subtitle text (section 5).
2. AVO folder resolution for DLG `VO_ResRef` (02 open 2).
3. Whether base game ships N_WookieM01-06 TPCs or only the loose Override copies.
