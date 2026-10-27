# C3a: lightsaber forms, Force forms, guard stances

Data: `spells.tsv` usertype 6 = 11 rows (258-268), all form rows. `feat.tsv` has no lightsaber or Force form rows. Guard stances are feats. Effects are "per description" unless marked. Confidence = row/label / effect.

| form/stance | 2da (table:row label) | claimed effects (per description) | conditions | shown in vanilla UI? | confidence |
|---|---|---|---|---|---|
| Shii-Cho I "Determination" | spells:258 FORM_SABER_I_SHII_CHO | Attack +1; Defense +3; Defense vs target -3 (net 0 vs target, +3 vs others) | none stated | Unverified (desc only) | High / Low |
| Makashi II "Contention" | spells:259 FORM_SABER_II_MAKASHI | Attack +3; Damage +3; blaster deflection -5; Force saves +2 | attack bonus only vs lightsaber-wielding target | Unverified (desc only) | High / Low |
| Soresu III "Resilience" | spells:260 FORM_SABER_III_SORESU | Defense vs target +2; blaster deflection +4; threat range -1 | anti-blaster design | Unverified (desc only) | High / Low |
| Ataru IV "Aggression" | spells:261 FORM_SABER_IV_ATARU | Defense -2 (net +3 vs target, -2 vs others); Defense vs target +5; blaster deflection -4; threat range +1 | best vs single foe; weak vs blaster | Unverified (desc only) | High / Low |
| Shien V "Perseverance" | spells:262 FORM_SABER_V_SHIEN | Attack +2; Defense vs target -5; blaster deflection +2 (net +4 incl. attack); crit multiplier +1 | best vs blasters or outnumbered; weakest vs single foe | Unverified (desc only) | High / Low |
| Niman VI "Moderation" | spells:263 FORM_SABER_VI_NIMAN | Attack +1; Defense +1; blaster deflection +1 (net +2); Force saves +1 | none (all-round) | Unverified (desc only) | High / Low |
| Juyo VII "Ferocity" | spells:264 FORM_SABER_VII_JUYO | Defense -4; Defense vs target +2 (net -2 vs target); Force saves -4; attacks/round +1; crit attack mod +4 | best vs single powerful foe | Unverified (desc only) | High / Low |
| Force Channel I | spells:265 FORM_FORCE_I_FOCUS | FP regen +50% (non-combat); Force power dmg +3; Force saves +2 | non-combat regen | Unverified (desc only) | High / Low |
| Force Potency II | spells:266 FORM_FORCE_II_POTENCY | Force power dmg +30%; FP cost +20% | none | Unverified (desc only) | High / Low |
| Force Affinity III | spells:267 FORM_FORCE_III_AFFINITY | FP regen during combat (reduced rate) | none | Unverified (desc only) | High / Low |
| Force Mastery IV | spells:268 FORM_FORCE_IV_MASTERY | Force power duration +50%; opponents' Force saves -2; own Force saves -4; FP cost +20% | none | Unverified (desc only) | High / Low |
| Guard Stance | feat:54 XXXX_GUARD_STANCE | Defense +2; VP regen x2; FP regen 5%/combat; -6 to hit; blocks Critical Strike, Power Attack, Flurry | no prereq; activated | Unverified (desc only) | High / Low |
| Advanced Guard | feat:2 XXXX_ADVANCED_GUARD_STANCE | Defense +2; same regen; -4 to hit; same blocks | base attack +3 | Unverified (desc only) | High / Low |
| Master Guard | feat:25 XXXX_MASTER_GUARD_STANCE | Defense +3; same regen; -3 to hit; same blocks | base attack +6 | Unverified (desc only) | High / Low |

## Usertype map (spells.tsv col 7, counted)
- 1 = 105 Force powers; 2 = 16 special (incl. spells:54/55 SPECIAL_ABILITY_*_STANCE_XXXX, blank name/desc); 4 = 106 items; 6 = 11 forms (258-268); -2 = 49 unused.
- The -2 rows include XXX_FORM_LIGHTSABER_* (205-222) and XXX_FORM_CONSULAR_* (223-243): blank name/desc, old form IDs.

## How active form is detectable
1. Read: `int IsFormActive(object oCreature, int nFormID)` at nwscript.nss:5854 ("trumping and lightsaber-wielding checks"). Engine: ExecuteCommandIsFormActive, Steam 0x0068BE80 (B15/functions.csv:370).
2. Write: `void SetCurrentForm(object, int)` at nwscript.nss:6178 (no-op if form unknown). Engine: ExecuteCommandSetCurrentForm, Steam 0x00691020, calls HasSpell 0x006BD9C0 (functions.csv:422). So a form is a known spell first. Caller: a_set_form.nss:130-131 (GrantSpell, then SetCurrentForm).
3. Masks: GetSpellFormMask at nwscript.nss:5861 (engine 0x00688590, functions.csv:320). FORM_MASK_* sit inside /* */ at nwscript.nss:750-762, so not live.
4. Live constants: nwscript.nss:937-947 (FORM_SABER_* 258-264, FORM_FORCE_* 265-268). The 39 FORM_CONSULAR_*/FORM_LIGHTSABER_* defines (nwscript.nss:896-934) are inside /* */ (894-935).
5. Per-creature storage: string "CurrentForm" in creature-stats field table at 0x00999A4C (patch_manager_mods/ghidra_knowledge/steam_docs/CSWSCreatureStats.md:2597). Likely the save field; not confirmed.
6. Separate stance: `IsInTotalDefense(object)` at nwscript.nss:5826 (engine CSWSCreature::IsInTotalDefense, CSWSCreatureStats.md:445).

## Evidence
- spells.tsv rows 258-268 (usertype 6); feat.tsv rows 2, 25, 54.
- k_inc_force.nss:311-315 (live: Force I +3 dmg, Force II dmg); :448 and :527 (live: Mastery duration/DC); :537-549 (live: target save DC for Makashi 259, Force I 265, Niman 263, Juyo 264, Mastery 268). Magnitudes match the descriptions.
- a_set_form.nss:22-69 grants only 261-264 and 266-268; no 258-260 literal in file.
- No script checks for Shii-Cho 258, Soresu 260, Ataru 261, Shien 262 effects. Their attack/defense/deflection effects are engine-side (unverified).
- k_inc_force.nss:552-610 is a comment block holding Total Defense (578) and consular/Ancient checks (585-606). Dead in this vanilla copy. The awk scan also flags 2033/2156/2756 as commented (not individually verified).
- Guard-stance labels: no live script references (rg -w hits are comments only).

## Open questions
1. Which script or 2da grants Shii-Cho/Makashi/Soresu (258-260)? a_set_form.nss never grants them.
2. Are the XXXX_ labels (feat 2/25/54, spells 54/55) unused placeholders? Confirm against engine feat use.
3. Which engine function reads the active form for attack/defense/deflection? Not found in scripts.
4. Description values are not verified in-game, and UI display is unverified.
