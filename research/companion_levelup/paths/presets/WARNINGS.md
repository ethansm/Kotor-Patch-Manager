# check_paths.py warnings: triage (Phase 6 step B1, 2026-10-03)

Command: `cd research/companion_levelup && ../../.venv/bin/python3 check_paths.py paths/*.toml paths/presets/*.toml` (0 errors before and after).

Baseline: 199 warnings, ALL of one type ("N skill point(s) unspent"). The checker emits no unspent feat-slot, unspent power-slot or
base-attack warnings for any current path; no prerequisite-order, duplicate-feat or wasted-feat problems exist (those are errors, 0).
Root cause (doc 08 "0.3.0 follow-ups"): the checker now honours "Class Skill: X" feats (cost 1 instead of 2), so plans written with the old
cross-class cost (2 per rank) leave points over. Only Kreia, Bao-Dur and Disciple were affected.

## Fixed (177 warnings): extra rank(s) added to the `skills = {...}` line of each flagged level
Only `skills` lines changed (verified: every diff line is a `skills = {` line); no feats, powers, classes, attrs, `convert_at`, ids or names touched.
A rank was added to the first skill that still fits the point budget and the level+3 rank cap (skills already used by the preset first, then the
preset's `[meta.wizard] skills` focus list, then any skill). Added skill per file:
- Kreia (`paths/kreia.toml`, `kreia__support_none_dark/light`, `kreia__tank_single_light`: L10-30; `kreia__support_none_neutral`: L4-30): +1 Stealth/level
  (her utc has the Class Skill: Stealth feat, so it costs 1); neutral also +1 Awareness at L4-5. Her three combat skills hit the rank cap at L30.
- Bao-Dur `early_guardian` L26-30, `melee_dual_light_early` L8-11 and L24-30, `melee_unarmed_light_early`, `support_single_light_early` (same levels),
  `late_guardian` L30: +1 Persuade (Jedi Guardian class skill) after conversion; before conversion (L8-11) +1 Stealth.
  `melee_single_dark_early` L8-11: +1 Treat Injury. `tank_single_none_never`: Stealth (cross-class, 2 points/rank) at L8-30, with 2 ranks per level at L24-30 where 4 points were spare.
- Disciple `caster_none_dark_early` L23-30: +1 Persuade.
Judgement call for a human: these dump the spare point into a "reasonable" skill, which slightly changes the skill mix (see per-skill end totals in the checker output).

## Accepted (22 warnings): cap-bound unspendable single point
Tech Specialist (class 9) levels where the only skills costing 1 point per rank (computer_use, demolitions, awareness, repair, security: the five
class skills) are already at the max rank (char_level + 3) and the leftover is 1 point, while every other skill is cross-class (2 points per rank).
A rank cannot be bought with 1 point, and unspent points are harmless (doc 08 Q4; engine and rewrite just store the remaining count). Not a
checker false positive: the engine behaves identically. Carry-over of leftover points to the next level is NOT modelled by the checker or the
runtime table (the record stores an absolute per-level unspent count), so it was not used.

| type | reason | count |
|---|---|---|
| 1 skill point unspent, class skills capped, remaining skills cost 2 | engine-correct, unavoidable without changing earlier spending | 22 |

## Per-preset counts (before -> after; unlisted presets 0 -> 0)
| file | before | after |
|---|---|---|
| paths/kreia.toml | 21 | 0 |
| baodur__early_guardian | 5 | 0 |
| baodur__late_guardian | 7 | 6 (L16-21) |
| baodur__melee_dual_light_early | 11 | 0 |
| baodur__melee_single_dark_early | 4 | 0 |
| baodur__melee_unarmed_light_early | 11 | 0 |
| baodur__support_single_light_early | 11 | 0 |
| baodur__tank_single_none_never | 23 | 8 (L16-23) |
| baodur__tech | 8 | 8 (L16-23) |
| disciple__caster_none_dark_early | 8 | 0 |
| kreia__support_none_dark | 21 | 0 |
| kreia__support_none_light | 21 | 0 |
| kreia__support_none_neutral | 27 | 0 |
| kreia__tank_single_light | 21 | 0 |
| **total** | **199** | **22** |

`check_paths.py` was not modified (no false positive found).
