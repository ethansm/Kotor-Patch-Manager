# Atton presets (tag `atton`, template `live_utc/p_atton.utc`)

Start state (live, `live_companions.json` p_atton.utc): Scoundrel 3, char level 3, STR12 DEX16 CON12 INT10 WIS10 CHA15 (Thematic Companions #8: CON14->12, CHA12->15, `installed_mod_thematic_companions/details.txt:2-9`; elkripper's CON14/CHA12 table is stale). First level-up = char 4. INT 10 -> 4 skill points per Scoundrel level, 3 per Sentinel level. Existing live path `paths/atton.toml` (convert at 20, ends 28) is untouched; `atton__ranged_pistols_light_late` is its 30-level extension.

All 7 files: `check_paths.py` = 0 errors, 0 warnings (every feat, power and skill point slot used). Level range 4-30. All carry `[meta.wizard]` (WIZARD_SCHEMA v1), `variant`, `summary`, `convert_at`, and `hold_until_class = 5` for converting presets.

## Conversion facts
- Class is added only by script (`a_makejedi`, AddMultiClass; phase3_evidence/C1); auto level-up then levels only the NEWEST class, so every post-conversion level is Sentinel (class 5); max 2 classes (Scoundrel + Sentinel). Never converting = Scoundrel only.
- Gate is dialogue + influence, not a level: `text/elkripper_atton.txt:16` (learn his secret from the NPCs in the Nar Shaddaa refugee sector, then talk to him); StrategyWiki `strategywiki_Atton.txt:361-426` (Nar Shaddaa confrontation), `:538-579` (Jedi/past story, influence-gated), `:614-650` (train-you lines), `:646` "only say you'll train him once you don't want him to gain any more levels as a Scoundrel". Influence sources: `elkripper_atton.txt:17-60`.
- Light vs dark: `strategywiki_Atton.txt:614-626` (dark options "follow me ... kill the Jedi", Dark Jedi Sentinel text :662) vs :649-663 (light "Then I will train you", Jedi Sentinel :663); outcome table :664-675 depends on the EXILE's alignment AND influence (table is ambiguous after flattening, so verify in game).
- Why late: elkripper `:6-8`: Atton gains Sneak Attack at every odd level through 19 (Sneak Attack III..X granted at Scoundrel 5,7,9,...,19), Scoundrel 20 would also give one more selected feat; level 20 is the latest sensible switch (19 Scoundrel, 10 Sentinel at max level 29).
- Why early: more Force power slots (Sentinel level 1 = 2 powers, then 1 per level: 24 powers by 30 when converting at 8, 12 when converting at 20) and the 0.75/level Sentinel attack bonus (mod #10) vs Scoundrel (not listed; check in game).
- Early = char 8 is an ASSUMPTION: Nar Shaddaa is reached around Exile level 7-9 (no source gives a level) and Atton may be converted any time after the Nar Shaddaa secret + influence. If conversion happens later, `hold_until_class = 5` banks the Sentinel levels; Scoundrel levels 4-7 are not held, so a later conversion needs a re-based variant (see questions).
- Slots (live 2DAs via check_paths): Scoundrel feats at class levels 1,2,5,8,11,14,17,20,23,26,29, no powers; Sentinel feats at class levels 1,3,6,7,9,12,13,15,18,19,21,24,25,27, powers 2 at class level 1 then 1 each. Power minimums in spells.2da are CHARACTER levels.
- Sentinel auto-grants: Weapon Proficiency Lightsaber, Jedi Defense, Jedi Sense, Force Immunity Fear at Sentinel 1; Unarmed Specialist I + Force Sensitive at 2; Force Immunity Stun + Unarmed II at 6 (`elkripper_atton.txt` table).
- Sources for feats: dual-blaster Sniper+TWF = elkripper build note `:299-304` (also "swap ranged feats for melee ones" for a melee build); rapid-shot alternative `:301`; Precise Shot I-III last `:303`. Feat/power semantics: `strategywiki_Feats.txt`, `strategywiki_Force_powers.txt`, `strategywiki_Skills.txt`, `strategywiki_Classes.txt:550-551` (Sentinel is balanced; Atton becomes Sentinel, `details.txt:279`).
- Mod #142: LS powers boosted, DS powers penalised; Heal scales with Treat Injury; light/neutral presets avoid E powers; the dark preset is flagged. CHA 15 (+2) offsets 10% of the dark penalty.
- DEX bumps (17/18/19/20/21/22/23 at 4,8,12,16,20,24,28) follow `paths/atton.toml`/elkripper; the engine applies companion attribute gains itself (Finding 6).

## Presets and wizard answers
| file (`atton__<role>_<style>_<align>_<convert>`) | role | style | alignment | convert | skills | one-line rationale |
|---|---|---|---|---|---|---|
| `ranged_pistols_light_late` | ranged | pistols | light | late (20) | stealth | The elkripper build: Scoundrel 4-19 for max Sneak Attack, Sniper Shot + TWF dual blasters, Sentinel 20-30 (Close Combat, Precise Shot, Speed/Heal/Deflection) |
| `ranged_pistols_light_early` | ranged | pistols | light | early (8) | combat | Same dual-blaster kit (TWF, Sniper, Precise Shot I-V, Critical Strike) with 23 Sentinel levels: Speed/Heal/Deflection/Stasis/Battle Meditation/Barrier |
| `melee_dual_light_early` | melee | dual | light | early (8) | combat | Dual lightsabers: TWF line, Dual Strike line, WF Lightsaber, Adv/Master Jedi Defense, Valor/Speed/Heal/Barrier kit |
| `melee_dual_dark_early` | melee | dual | dark | early (8) | stealth | Dark Jedi Sentinel (Atton's dark route), same saber feats; Speed line + Shock/Lightning/Wound/Choke/Kill/Drain Life/Storm/Death Field; mod #142 caveat |
| `support_pistols_light_early` | support | pistols | light | early (8) | combat | Force-support gunner: Heal line with Treat Injury 33 (mod #142), Valor/Barrier/Battle Meditation/Revitalize, Regenerate FP/VP, Precise Shot |
| `ranged_pistols_neutral_early` | ranged | pistols | neutral | early (8) | tech | Universal powers only (Speed, Deflection/Redirection, Resistances, Force Body, Battle Meditation, Push/Whirlwind/Wave/Breach), Rapid Shot gunner, Security/Computer Use |
| `ranged_rifle_none_never` | ranged | rifle | none | never (0) | stealth | Never a Jedi: Scoundrel 4-30 sniper (Sniper line, WF Rifle, Precise Shot I-V, Rapid Shot), no Force |

Not covered on purpose: `caster`, `tank`, `unarmed`, `double` (Atton has no caster stats and no tank kit; Unarmed Specialist is auto-granted but is not a build; double-blade has no source), `social` skills (Persuade is a Scoundrel class skill but no source recommends it; add a variant if wanted), a dark/late and a never+pistols/melee variant (never+rifle covers the "never" answer; the wizard falls back to nearest match).

Skill points are proposals (weighted greedy, `check_paths` rules: cap char_level+3, cross-class cost 2). Treat Injury is cross-class for Scoundrel (cost 2), so the combat/support presets only pick it up after conversion except where weights apply.

## Open questions for the user
1. Early conversion level: 8 assumed. When does the story really allow it (Exile level at the Nar Shaddaa secret + influence)? A re-base with another `convert_at` is cheap.
2. Does hold_until_class bank only Sentinel levels or all levels while Atton is still Scoundrel? If the player converts at 12, levels 8-11 would be Scoundrel picks not in the early files.
3. Dark preset: conversion needs the dark dialogue branch; does mod #142 read Atton's own alignment or the Exile's? Does Atton's alignment drift with influence (StrategyWiki :664)?
4. Is the Sentinel attack bonus (0.75/level, mod #10) vs Scoundrel actually better for a dual-blaster build? Pistols-early may be worse than late for Sneak Attack damage.
5. Should the support preset's Heal rely on Treat Injury ranks (Scoundrel cross-class cost 2 before conversion)? Skills are proposals.
6. Level cap with TSLRCM: files assume 30 (elkripper assumes 29).
7. Unverified attributes: live attribute gains for Atton (DEX per level) are assumed from elkripper.

## B4 preset fixes (2026-10-03, from phase6_evidence/B3_build_review.md)
- `melee_dual_light_early` and `melee_dual_dark_early`: Two-Weapon Fighting stays at L5 and Finesse: Melee Weapons (194) takes the next slot (L8); feats up to L26 move one slot later (WF Lightsaber L10, Improved TWF L13, Dual Strike L14, Master TWF L16, Advanced Jedi Defense L19, Toughness L20, Improved Dual Strike L22, Master Jedi Defense L25, Improved Toughness L26, Conditioning L28). Master Dual Strike is dropped; the ranged-only Close Combat (L19) / Improved Close Combat (L25) became Toughness / Improved Toughness (net result of the shift).
- `ranged_pistols_light_late` (+ dev `paths/atton.toml`): Heal line needs Treat Injury (#142 combined install is live). Sentinel levels (L20+, Treat Injury is a class skill) now put points into Treat Injury first (up to level+3), then Awareness/Stealth/Security. Scoundrel levels unchanged (Treat Injury is cross-class there). End ranks: Treat Injury 33 (atton.toml 27 at L28), Stealth/Awareness/Security 22.
