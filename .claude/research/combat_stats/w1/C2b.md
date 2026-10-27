# C2b: Criticals and damage bonus (Steam swkotor2.exe)

Attack record = round+4+idx*0x17C, idx = round+0xABC (CSWSCombatAttackData, DB). +0x64 = special id (activated feat id, see Evidence). +0x5C = result (2 = crit). Owner creature = stats+0x24. Decomp: `ghidra_dumps/steam_6a522e71/decomp/<hh>/<addr>.c`.

| stat | what it means for the player | formula / parts | computed at | storage | shown in vanilla UI? | how a patch could read it | conf |
|---|---|---|---|---|---|---|---|
| Crit threat count | d20 faces that crit (19-20 = 2) | count = base + (weapon has item prop 0x1C ? base : 0) + tier(special id); base = baseitem+0x1E (1 if no weapon); tier: id 8 +1x, 31 +1x, 19 +2x, 20 +2x, 77 +3x, 81 +3x; returns 21-count | 0x006afa60 | baseitem row via 0x006d5200 (tbl+4+id*0xC8); attack+0x64 | no evidence | call 0x006afa60 (no RNG) | high (formula); low (prop 0x1C = Keen) |
| Precomputed crit min/mult | pre-rolled crit values per hand | stats+0x11C block: +0x0A main min roll (21-count), +0x0B main mult, +0x10 offhand min, +0x11 offhand mult | 0x006a7190 UpdateCombatInformation (:268-275) | stats+0x11C struct | no evidence | read stats+0x11C then offsets | high (write); med (consumer) |
| Lightsaber threshold shift | +/-1 on crit threshold by form | form 0x104 with saber: thr+1 if thr<20; 0x105: thr-1 if thr>1 | 0x006e0b10 (:85-90) | attack+0xF0 | no | read creature+0x1110 and attack+0xF0 | med |
| Crit multiplier | damage multiplier on crit (x2 default) | base = baseitem+0x1F (2 if no weapon; slot 0x8 not checked); +1 if special id in {17,18,28,29,82,83}; +1 if saber and form 0x106 | 0x006afcd0 (no RNG) | baseitem+0x1F; attack+0x64; creature+0x1110 | no evidence | call 0x006afcd0 | high |
| Crit confirmed | crit only if d20 >= thr AND 2nd d20 + attack bonus >= target AC | roll1 = RollDice(1,20) >= thr; roll2 = RollDice(1,20) (+4 if form 0x108) + attack bonus >= AC | 0x006e0b10 (:57-130) | attack+0xF4 flag, +0xF8 roll2, +0xFC crit, +0x101 mult, +0x104 AC | no | read attack record after resolve (RNG) | med |
| Flat damage bonus | added per damage roll, x mult on crit | sum of rows below; returned as sum&0xFF; caller keeps it as signed char | 0x006abf70 (GetDamageBonus) | copied to attack+0x111 | no evidence | call only with care (RollDice inside) or sum parts | med-high |
| Power Attack mode | +3 (state 2) / +7 (state 3) | CSWSCreature+0x512 byte | 0x006abf70 | creature+0x512 | no evidence | read byte +0x512 | med |
| Activated special damage | PA/PB +3, IPA/IPB +7, Master +12, Force Jump 101/102/103 +0/+2/+4 | switch(attack+0x64): 17,18->7; 28,29->3; 82,83->12; 101->0; 102->2; 103->4 | 0x006ad1a0 (label "Special Attack Damage Bonus") | attack+0x64 | no evidence | read special id, apply table | high (values); med (object read) |
| Favored-enemy damage | always 0 in this build | 0x006bada0 loop has no effect, returns 0 | 0x006bada0 | target+0x8D read only | no | ignore | high |
| Feat damage tiers | melee +2/+4/+6; ranged same plus Precise Shot | melee: 172/153 +2, 173/154 +4, 174/155 +6; ranged: 153-155 as melee; 240 +1, 241 +2, 242 +4, 243 +6, 244 +8 | 0x006abf70 | none (HasFeat 0x006b83f0) | no | HasFeat(id) | high |
| Makashi form damage | +3 with form 259 | IsWieldingLightsaber (0x005763e0) and creature+0x1110 == 0x103 | 0x006abf70 | creature+0x1110 | no | read +0x1110 | med |
| Unarmed tiers | +Nd4 when attack type 7 or 8 | Echani 209/210/211 -> 1/2/3 d4; Unarmed Specialist 212..219 -> 1..8 d4 (RollDice(n,4)) | 0x006abf70 | attack+0x3A (weapon attack type) | no | HasFeat chain; "type 7/8 = unarmed" is hypothesis | med |
| Strength part | STR: 1x main, ~1.5x two-handed, ~0.5x offhand, 0 ranged (main branch only) | stats+0xEE (C2a); two-handed = right item byte+8 in {2,3} and left empty; factors inferred, float not read | 0x006abf70 | attack+0x114 (NOT in flat sum; added in GetDamageRoll) | no | read attack+0x114 or stats+0xEE | low (factors) |
| Weapon specialization | +2 damage | attack+0x115 = 2 if spec test (0x006b8ee0) passes; NOT in flat sum; added in GetDamageRoll | 0x006abf70 | attack+0x115 | no | read attack+0x115 | med |
| Sneak eligibility | sneak applies to this hit | any HasFeat 60..69 (Sneak I-X); melee: target behind (dot<0) or other flags; ranged: dist <= DAT_00989EE0 (unread) | 0x006e1000 | attack+0x44 = 1 | no | read attack+0x44 | med |
| Sneak attack dice | Nd6 | tier = highest HasFeat 69..60 (10..1); RollDice(tier,6) only when GetDamageRoll p5 (= attack+0x44) != 0 | 0x006adec0 (:338-396) | attack+0x44 | no | HasFeat chain 60-69 | med-high |

## Stats-page guidance
- Pure (no RNG): 0x006afa60, 0x006afcd0. Not pure (RollDice): 0x006abf70, 0x006adec0.
- Breakdown labels ("+ %d (Weapon Specialization Bonus)" etc.) are built only when DAT_00a7e084 != 0 (debug). Do not parse them.

## Evidence
- 0x006afa60: `return 0x15 - local_8;`; `local_20 = *(byte*)(baseitem+0x1e)`; item-prop check `FUN_006042e0(&local_24,0x1c,0)`.
- 0x006afcd0: `local_c = (uint)*(byte*)(iVar1+0x1f)`; switch 0x11,0x12,0x1c,0x1d,0x52,0x53 adds 1.
- feat.tsv: 8 "doubles", 19 "triples", 20 "triples" (ranged), 31 "doubles" (ranged), 77/81 "quadruples"; 17/28 "+7"/"+3"; 82/83 "+12"; 28/29 "critical multiplier is increased by 1". Matches the switch tables above.
- 0x006bada0 (6b/006bada0.asm): loop with empty body; `XOR EAX,EAX` at return, so 0.
- 0x006e1000: gate HasFeat 0x3c..0x45; `attack+0x44 = 1` at :97, :103, :113.
- 0x006adec0:338-396: HasFeat 0x45..0x3c then `FUN_006a2000(local_a0,6)`; :431 and :449 add attack+0x114 and attack+0x115.
- 0x006e1540.asm 0x006e164f-0x006e1666: pushes attack+0x44 as GetDamageRoll p5, (attack+0x5c==2) as p4 (med).
- 0x006a7190:268-275: `*(char*)(stats+0x11c)+10 = 0x15 - FUN_006afa60(0)`; same for offhand (1).

## Open questions
1. Is item prop 0x1C on the weapon Keen? No itempropdef export in repo; unverified.
2. Droid/ion: no droid/ion branch in 0x006abf70 or 0x006adec0. Per-target roll 0x006a9100 (667 B) is unread; check it first.
3. Possible double count: creature+0x512 PA bonus and activated PA special id are both added in 0x006abf70. Need the setter of +0x512.
4. name_map conflicts: 0x006ad1a0 labelled GetFavoredEnemyBonus but computes special-attack bonus; 0x00591E10 labelled GetCurrentAttackWeapon but takes a weapon-type byte.
5. Unread: whether sneak dice are crit-multiplied (0x006adec0:396-420), flags 0x006bc570 / 0x006daa00, and DAT_00989EE0.
