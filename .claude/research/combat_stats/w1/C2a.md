# C2a: Attack bonus parts (Steam swkotor2.exe) - GetAttackModifierVersus 0x006a93a0

Result = sum of parts (return local_38, 006a93a0:916). Parts are per attack record (attack index at combat round +0xabc). Feat/spell ids are decimal in feat.tsv/spells.tsv.

| stat | what it means for the player | formula / parts | computed at | storage (attack record base = round+4+idx*0x17c) | shown in vanilla UI? | how a patch could read it | conf |
|---|---|---|---|---|---|---|---|
| Base attack bonus (BAB) | Same BAB for every attack in a round | GetBaseAttackBonus: sum over classes of per-level progression (class count stats+0x8d, rows stats+0xab stride 0x28); +0x106 override if nonzero; +0x250/0x251 scaling for non-party | 006a6f80; call 006a93a0:144 | not stored (local only) | no evidence | call 006a6f80 (side-effect free, reads stats) | med (class table FUN_006f08f0 not read) |
| Main-hand penalty (type 1) | Penalty on right-hand attack while off-hand weapon present | base -6 (+0xcc); +2 if off-hand light (rel. size -1 via 0057e350, or base type 3); + TWF feats 0x55 +4, 0x09 +2 | 006a93a0:177-222 | +0xcc base, +0xcd light flag, +0xce feat sum, +0xcf feat-id tag | no evidence | read +0xcc..+0xcf after calc | high (values), med (light rule) |
| Off-hand penalty (type 2) | Penalty on left-hand attack | base -10 (+0xcc); feat 0x03 +4 (TWF), 0x09 +6, 0x55 +8; SWF 0xa1 +1 | 006a93a0:223-265 | same fields as above | no evidence | read +0xcc..+0xcf | high (feat 3 text: "-6/-10 reduced by 0/4") |
| Superior TWF (feats 159-161) | Extra reduction of two-weapon penalty | type 1: 0xa1/0xa0 +2, 0x9f +1 (max one); type 2: only 0xa1 +1 | 006a93a0:201-219, 246-258 | in +0xce | no | as above | med (type-2 asymmetry looks odd) |
| Special attack to-hit | Force/special-attack hit modifier | ResolveSpecialAttackAttackBonus(target) | 006a93a0:288 via 006acec0 | +0xc6 | no | read +0xc6 | med |
| Dueling (feats 113-115) | +1/+2/+3 with one-hand weapon or empty hands | 0x71 +1, 0x72 +2, 0x73 +3; gate: both hands empty, or right-hand only with base type 2 or 4 | 006a93a0:319-347 | +0xd0 value, +0xd1 feat id | label only; label gated on wrong var (line 350 uses local_58) | read +0xd0 | med |
| Good Aim (feats 139/140) | Ranged attacks within range | +1 per 0x8b/0x8c, ranged attack and distance^2 <= range^2 (_DAT_00986b68) | 006a93a0:369-383 | +0xd2 | no | read +0xd2 | med |
| Offensive training vs race | Melee, base-item flag; value 6 minus 2 per 0x8b and 0x8c | 6 - 2*HasFeat(139) - 2*HasFeat(140); gate: right-hand base flag +0x1a | 006a93a0:403-419 | +0xd3 | no | read +0xd3 | low (sign looks inverted, OPEN) |
| Dual Strike (feats 190-192) | Teamwork +2 per feat when party member's target matches | +2 per HasFeat(190/191/192), gated by party target id (creature+0x550) | 006a93a0:437-468 | +0xe0 | no | read +0xe0 | med (190 text confirms +2) |
| STR or DEX mod | Melee (non-finesse) STR; finesse melee or ranged DEX | STR: stats+0xee byte; DEX: GetDEXMod(0) (006b5db0) | 006a93a0:471-491 | +0xc3 (STR), +0xc4 (DEX) | label only | read +0xc3/+0xc4 | high |
| Weapon focus | Feat matching current weapon, plus lightsaber tiers | +1 if HasFeat(base item +0xb2 id) (006b8e80); +1/+2/+3 for 156/157/158 if lightsaber | 006a93a0:527-553 | +0xd4 (WF + lightsaber tier) | label only | read +0xd4 | high |
| Targeting (ranged only) | Ranged accuracy | feats 126..135 -> +1..+10 (non-stacking chain) | 006a93a0:557-626 | +0xe1 id, +0xe2 bonus | label only | read +0xe2 | high |
| Crush Opposition penalty | Negative to-hit vs target | spell 149 -> -2, spell 146 -> -1; gated on target stats+0x18a <= 40 | 006a93a0:665-716 | +0xe3 id, +0xe4 value | label only | read +0xe4 | med (+0x18a identity open) |
| Inspire Followers | Positive to-hit | spell 172 -> +2, spell 169 -> +1; gated on self stats+0x18a >= 60 (or > 59) | 006a93a0:737-788 | +0xe5 id, +0xe6 value | label only | read +0xe6 | med |
| Lightsaber form | Form bonus while wielding lightsaber | owner +0x1110: 258 +1, 259 +3 (needs target), 262 +2, 263 +1 | 006a93a0:809-834 | +0xe8 id, +0xec value | no (unlabelled "+ %d") | read +0xec | high (ids: Shii-Cho, Makashi, Shien, Niman) |
| Effect attack bonus | From active effects | CSWSCreature::GetTotalEffectBonus(1, ...) | 006a93a0:849-863 via 00582010 | +0xd5 | label only | read +0xd5 | med |
| PC Charisma attack | Party-member CHA-based bonus | stats+0xf8 byte of last party member with flag +0x6c | 006a93a0:883-895 | +0xe7 | label only | read +0xe7 | low |

## Attacks per round
- Steam GetAttacksPerRound 0x006a7170 is a constant `return 1` (23 bytes). It is written to combat info byte [0] at 006a7190:214-215.
- 005905f0 sets round +0xae0 = 1 + min(+0xaf4, 2) (005922c0) + 1 for anim ids {0x0b,0x5b,0x35} or {0x1e,0x5c,0x1a} + 1 if lightsaber and owner form 0x108 (Juyo). Confidence: med. The setter of +0xaf4 (00592280) has no identified caller.
- Per-hand counters: round +0xae8 (type 2, incremented each call) and +0xaec (types 6/8). BAB is not per-attack in this function.

## Evidence
- 006a93a0:144 `local_38 = local_38 + local_ac;` (BAB added to total)
- 006a93a0:916 `return local_38;`
- 006a7170 body `return 1;` (23 bytes); caller 006a7190:214 `uVar3 = FUN_006a7170();`
- 006a93a0:129-139 type 2 increments +0xae8 per call (side effect)
- 00591e10:  attack type 1/6 -> slot 0x10 (right hand); type 2 -> 0x20 (left) unless right-hand base type 3 (double-bladed)
- 005908c0: `return param_1 + 4 + param_2 * 0x17c;` (attack record array)
- feat 3 text: "normal penalties of -6 (main hand)/-10 (off hand) are reduced by 0/4"

## Open questions
- Is the debug-breakdown gate DAT_00a7e084 ever set in vanilla play? Labels appear only when it is nonzero.
- Who calls 00592280 (sets round +0xaf4), and what does 006f08f0 (class BAB row) return?
- Is Offensive Training sign inverted (6 minus feats), and is stats+0x18a a level?
- Does ResolveAttackRoll (006e0b10) apply the -5 per extra attack, or is it in the attack list build?
- Side effects: calling 006a93a0 writes attack-record fields and increments +0xae8, so a stats page must not call it.
