# Disciple (Mical) presets

Files: `disciple__caster.toml`, `disciple__melee.toml`, `disciple__support.toml` (all 0 errors / 0 warnings in `check_paths.py`).
Live start: Soldier 6, char 6 (`live_companions.json` p_disciple.utc). All variants run char 7-30. Skill points are a PROPOSAL, not user-approved.

## Conversion facts
- Disciple is recruited on Dantooine (Jedi Archive, Enclave Sublevel), female Exile only: text/strategywiki_Disciple.txt:199; text/elkripper_disciple.txt:17-18.
- Becomes a Jedi Consular by the "Training" dialogue ("I could train you to feel the Force again", influence-gated, shown as `< 10 Influence > 90`, strategywiki_Disciple.txt:710-713). Wiki text says use it "if you don't want him to gain any more levels as a Soldier". Party page: "can be trained in the ways of the Jedi Consular" (strategywiki_Party.txt:511).
- No character-level gate. Elkripper: "enough influence can be gained via dialog immediately upon meeting him" (elkripper_disciple.txt:22, :26; also :11). Earliest = right after the Dantooine intro conversations, so the "caster" variant (convert at the first level-up, char 7) is legal.
- Alignment only changes the flavour of the dialogue (Dark Jedi Consular line, strategywiki_Disciple.txt:747 vs light 748); class is the same. Class is added by script (a_makejedi, AddMultiClass), never by the level-up GUI.
- Soldier class skills: Demolitions, Awareness, Treat Injury; Consular: Awareness, Repair, Treat Injury (elkripper_disciple.txt:12-13; matches skills.2da).
- TSLRCM / PartySwap / Extended Enclave: none of the fetched sources describe a changed Disciple conversion gate. Unverified in play (README "still unresolved").
- Engine rules obeyed (Ghidra, from coordinator): auto level-up always levels the newest class, so after conversion every level is Consular (no return to Soldier); max 2 classes (Soldier + Consular only). AddMultiClass grants XP for one extra level if below the next threshold.

## Planned conversion level per variant (`convert_at` = first Consular char level)
- caster: 7 (Consular 1-24)
- support: 13 (Soldier 7-12, Consular 1-18)
- melee: 16 (Soldier 7-15, Consular 1-15). The mod must hold/bank Soldier levels until char `convert_at-1` and then the Jedi class must exist before char `convert_at`.

## Variants and rationale
- **caster** - elkripper "Caster Disciple" (text/elkripper_disciple-caster.txt). Feat plan close to elkripper (Regenerate FP, Master TWF, Master Toughness/Conditioning, Adv./Master Jedi Defense) but Improved Toughness (123) added at char 12 because live Disciple lacks it. Powers re-prioritised for healer/buffer, light side: Heal line, Force/Knight/Master Valor, Force Barrier line, Battle Meditation line, Revitalize line, Force Body line, then Stasis/Wave and Energy Resistance fillers. Elkripper's own list is offence-heavy and leaves late levels "?"; all `?` slots filled here. No dark-side powers (finding 4).
- **melee** - elkripper melee (text/elkripper_disciple-melee.txt): Soldier to 15 for the Flurry/TWF/Conditioning chain and HP (10 vs 6/level, :228-229), DEX-based (DEX > STR, :227). Deviations: Improved Toughness (123) inserted before Master Toughness (live lacks it); Dual Strike (190) at Soldier 15 then Improved/Master Dual Strike at Consular 6/9; Master Toughness moved to char 27; elkripper's repeated "Dual Strike (s)" at char 21 treated as a typo. Extra Force Valor at Consular 15 (elkripper left a slot spare).
- **support** - Soldier 7-12 for 6 durability feats (Improved/Master Toughness, Improved/Master Conditioning, Master TWF, Regenerate Vitality), then the caster power list (truncated to 18 Consular levels). Caveat: Consular attack bonus is 0.5/level under Classic Class Attack Bonus (README finding 3), so a Consular Disciple is a weak melee fighter regardless of feats.
- Skills (all): Treat Injury and Awareness alternate (Heal potency scales with Treat Injury, finding 4), then Demolitions (Soldier) / Repair (Consular), then Computer Use/Stealth cross-class (cost 2) so no points are left unspent. Max rank char+3, cross-class 2 assumed (PROJECT_CONTEXT section 9). INT 15 -> skill points Soldier 3, Consular 4 per level.
- `attrs` rows (WIS 16..21 for caster/support, DEX 15..20 for melee, every 4 levels from 8) follow elkripper's pattern shifted to the live base (WIS15, DEX14). Companion attribute gains are auto-set (README finding 6), so treat these as descriptive; the validator only uses them for feat prereqs and INT.

## Close Combat / Finesse discrepancy
Thematic Companions details.txt:51-52 says Disciple loses Improved Toughness and Close Combat and gains Empathy and Finesse: Melee (194). Live p_disciple.utc: has Empathy (10), no Improved Toughness (matches), but STILL has Close Combat (139) and NO Finesse: Melee; also has Superior Weapon Focus: Lightsaber II (157) and no Rapid Shot. The shipped mod template (`installed_mod_thematic_companions/utc/p_disciple.utc`) differs again (feats incl. 123, INT/WIS 10). Presets treat live data as truth: they never take 139 (already known) and the melee preset TAKES Finesse (194, Soldier 11); if in-game he actually has Finesse the validator would flag a duplicate, so verify. `k2-finesse-melee-fix` interaction not checked.

## Open user questions
1. Which variant (caster / support / melee) per playthrough? Melee only works if the mod can hold Disciple at Soldier until char 16.
2. Is Finesse: Melee (194) wanted in the melee build if he really has Close Combat instead? Verify in game.
3. Is a spare Consular slot better used for Force Valor/Heal earlier in melee (elkripper delays Valor line)?
4. Skill priorities (Treat Injury + Awareness first) acceptable, or Computer Use/Repair for crafting?
5. Confirm influence threshold for the training dialogue in play (wiki `< 10 Influence > 90` is ambiguous) and that TSLRCM/PartySwap do not alter it.
