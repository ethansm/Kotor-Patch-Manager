# isTargetable(entry): which hotbar actions can be cast on another party member

Static research, 2026-10-03. Source: live `override/spells.2da` (287 rows, 57 columns) + `dialog.tlk` read with
pykotor; UTI templates read from the game BIFs. Raw dump: `twoda_spells_targeting.tsv` (one line per row, TLK names resolved).

## 1. Key finding: there is NO `targettype` column in K2's spells.2da
Header (57): label name spelldesc forcepoints goodevil usertype prerequisites masterspell guardian consular sentinel weapmstr
jedimaster watchman marauder sithlord assassin inate maxcr category range iconresref impactscript conjtime conjanim
conjheadvisual conjhandvisual conjgrndvisual conjsoundvfx conjsoundmale conjsoundfemale castanim casttime castheadvisual
casthandvisual castgrndvisual castsound catchtime catchanim proj projmodel projtype projspwnpoint projsound projorientation
immunitytype itemimmunity forcehostile forcefriendly forcepassive forcepriority dark_recom light_recom exclusion
requireitemmask forbiditemmask pips itemtargeting hostilesetting formmask.
So the "creature/area/item/door bitmask" convention does not exist here; targeting is encoded by `range`, `hostilesetting`,
`forcehostile`/`forcefriendly`/`forcepassive`, `itemtargeting` and (AI) `category`.

## 2. Column semantics, with evidence
Value histograms over all 287 rows (see TSV):

| column | values (count) | meaning (evidence) |
|---|---|---|
| usertype | 1 (105), 4 (106), 2 (16), 6 (11), -2 (49) | 1 = Force power (rows 4-50, 133+ named powers); 4 = item ability (medpacs, grenades, shields, droid devices: rows 64-129, 257, ...); 2 = special/monster ability (Rage, Body Fuel, Slam Attack, Fire Breath); 6 = lightsaber/force forms (258-268); -2 = dead XXX placeholder rows (49, all unnamed). |
| range | P 96, T 63, M 58, L 38, S 21, W 2, blank 9 | P = personal, no target picked (every self buff: Burst of Speed r8, Valor r22, Armor r17, Shield r24, Energy Resist r42/40/133, Force Barrier 135-137, Fury, forms). T = touch (single creature picked by the player: Heal r10/28/134, medpacs r64-66, antidote r67, stims r69-77, repair kits r84/127/128, but ALSO energy shields, disguises, Battle Meditation). S/M/L = short/medium/long ranged (hostile powers: Push r23 M, Choke r9 M, Stun r46 M, Lightning r35 M, Storm r25 S, Wave r26 S, Death Field r11 S; grenades/rockets/darts L). W = weapon-range saber throw (r4, r49). |
| hostilesetting | 0 (166), 1 (108), blank 13 | 1 = hostile action (breaks stealth, wants an enemy): all of Push/Choke/Lightning/Storm/Stun/Wound, grenades, droid weapon items, rockets, darts, monster abilities, Shield Disruptor, Oil Slick. 0 = friendly/neutral: all buffs, heals, medpacs, stims, shields. Exceptions with hostilesetting 0 but hostile-ish: Mind Trick/Confusion/Beast Trick (control powers, 181-184, 200, 201, 269), Enemy Battle Meditation 141-143. |
| forcehostile | blank 218, else a group id 0..27 | Non-blank on every hostile power (Push 6, Choke 2, Lightning 5, Stun 0, Crush Opposition 20/21, Enemy BM 25, droid items 10-31, Mind Trick 27). It is the "hostile exclusion group" id (same-group powers share a number, e.g. Push/Wave/Whirlwind = 6). Non-blank => offensive. |
| forcefriendly | blank 226, else group id 0..34 | Non-blank on friendly effects: Heal rows 10/28/134 = 0, Armor/Aura/Shield = 1, Speed 2, Valor 3, Force Immunity/Resist Force 4, Energy Res 5, Force Barrier 12, Battle Meditation 13, Revitalize 18, Inspire Followers 16/17. Friendly stacking group, not a target filter by itself. |
| forcepassive | rows 6, 14, 156-163, 178-179 | passive/always-on powers (Force Camouflage, Deflection, Precognition). Never castable. |
| itemtargeting | blank 237, 1 (25), 2 (3), 3 (22) | Only non-blank on item rows. 1 = "use on self/equipped only": ALL energy shields (99-107, 257), droid deflector/hazard shields (110-115), disguises (85, 86), space suits (68, 109), Cold Serum 126, Heat Shield 132, PARTYSWAP. 3 = launcher ammo (rockets, darts, launched grenades), 2 = plot grenades. Inference from the row population (no engine decompile): medpacs/stims/repair kits (the "use on another" items) all have it blank. |
| category | hex string, 126 blank | NWN/K2 AI talent category, not a target filter, but it separates heals from buffs: 0x2441 = healing power (10, 28, 134, 173-175), 0x1408 = healing item (medpacs, Recovery Stim), 0x1404 = repair item, 0x1048 = cure item (antidote), 0x1808 = buff (all self buffs, stims, shields), 0x1201/0x1301/0x1101/0x2201/0x8000 etc = offensive. Confirmed by doc 07/R4: AI uses `GetCreatureTalentBest(0x1408,...)` for medpacs. |

Item mapping (verified from the BIFs): UTI `PropertiesList` entry PropertyName 10 (CastSpell), Subtype = **spells.2da row**.
g_i_medeqpmnt01/02/03 -> 64/65/66; g_i_drdrepeqp001 -> 84; g_i_adrnaline001/002 -> 69/70; g_i_cmbtshot001 -> 75 (BaseItems
53-56 are stim/medpac/repair base types). Matches doc 02's note that iprp_spells.2da is NOT the mapping. So an item's
targeting == its spells.2da row; medpacs are range T, category 0x1408, hostilesetting 0, no itemtargeting.

## 3. Proposed rule
```
// entry = a spells.2da row (power: the row of the power; item: the row of its CastSpell subtype)
isTargetable(r):                 // can sensibly be cast on a chosen friendly party member
    r.name != ""                        // drops the unnamed XXX/placeholder rows
 && r.usertype in {1, 4}                // force power or item ability
 && r.range == 'T'                      // touch: the player picks a creature (P = no target, S/M/L/W = ranged/hostile)
 && r.hostilesetting == 0
 && r.forcehostile == blank             // not offensive
 && r.itemtargeting == blank            // not a self-only gear/disguise item
 && !(r.usertype == 1 && r.category == 0x1808)   // T-range power buffs = Battle Meditation (party-wide, cast on self)
 && r.row != 108                        // Cloaking Device: T, 0x1808, no flag, but self-only (single manual deny)

isHealing(r) = isTargetable(r) && r.category in {0x2441, 0x1408, 0x1404, 0x1048}   // heal / medpac / repair / cure
```
Recommendation: offer "lowest HP%" mode when `isHealing` (11 rows); `isTargetable` minus `isHealing` (9 stims/adrenals,
0x1808) can be targeted at a member but "lowest HP%" is not meaningful for them (offer "leader"/default only, or let the
user decide). Medpac vs repair kit affinity (organic vs droid) is NOT in spells.2da; pick the lowest-HP% member among
the valid kind or accept the engine's refusal (see open questions).

## 4. Reference table (rule verdict vs expectation)
cols: row | name | label | usertype | range | category | hostilesetting | forcehostile | forcefriendly | itemtargeting | verdict | expected?

| row | name | usertype | range | cat | host | fh | ff | itgt | isTargetable | expected |
|---|---|---|---|---|---|---|---|---|---|---|
| 10 | Heal (label CURE, strref 287) | 1 | T | 0x2441 | 0 | - | 0 | - | YES (healing) | YES: single touch heal, ok |
| 28 | Improved Heal (label HEAL) | 1 | T | 0x2441 | 0 | - | 0 | - | YES (healing) | YES |
| 134 | Master Heal | 1 | T | 0x2441 | 0 | - | 0 | - | YES (healing) | YES |
| 173-175 | Revitalize / Imp / Master | 1 | P | 0x2441 | 0 | - | 18 | - | NO | YES, it is the party-wide heal+revive (range P), so a target is meaningless |
| 138-140 | Battle Meditation (3 tiers) | 1 | T | 0x1808 | 0 | - | 13 | - | NO | YES (party-wide, cast on self); note range T is nominal |
| 141-143 | Enemy Battle Meditation | 1 | T | 0x1808 | 0 | 25 | - | - | NO | YES (enemy-only, AI) |
| 17 / 22 / 24 | Force Armor / Force Valor / Force Shield | 1 | P | 0x1808 | 0 | - | 1/3/1 | - | NO | YES (self buffs; in K2 they affect self only per data) |
| 42 / 40 / 133 | Energy Resistance family | 1 | P | 0x1808 | 0 | - | 5 | - | NO | YES |
| 8 | Burst of Speed | 1 | P | 0x1808 | 0 | - | 2 | - | NO | YES |
| 23 | Force Push | 1 | M | 0x1301 | 1 | 6 | - | - | NO | YES (hostile) |
| 9 | Choke | 1 | M | 0x1301 | 1 | 2 | - | - | NO | YES |
| 46 | Stun | 1 | M | 0x1201 | 1 | 0 | - | - | NO | YES |
| 35 / 25 | Force Lightning / Force Storm | 1 | M / S | 0x8101 / 0x2111 | 1 | 5 | - | - | NO | YES (hostile) |
| 64 / 65 / 66 | Medpac I / II / III (ITEM_ABILITY_MED_PACK ...) | 4 | T | 0x1408 | 0 | - | - | - | YES (healing) | YES |
| 67 | Antidote Kit (sic "Antidode") | 4 | T | 0x1048 | 0 | - | - | - | YES (healing/cure) | YES |
| 69-74 | Adrenaline / Hyper Adrenaline: Str/Ala/Sta | 4 | T | 0x1808 | 0 | - | - | - | YES (not healing) | plausible YES |
| 75-77 | Combat Shot: Battle / Hyper Battle / Speed Stim | 4 | T | 0x1808 | 0 | - | - | - | YES (not healing) | plausible YES |
| 129 | Recovery Stim | 4 | T | 0x1408 | 0 | - | - | - | YES (healing) | YES |
| 84 / 127 / 128 | Repair Kit / Advanced / Construction Kit | 4 | T | 0x1404 | 0 | - | - | - | YES (healing) | YES (droid targets only) |
| 99-107, 257 | Energy shields (all) | 4 | T | 0x1808 | 0 | - | - | 1 | NO | YES (self-equip gear) |
| 110-115 | Droid deflector / hazard shields | 4 | T | 0x1808 | 0 | - | 6-11 | 1 | NO | YES |
| 108 | Cloaking Device | 4 | T | 0x1808 | 0 | - | - | - | NO (deny list) | YES (self) |
| 87-95 | Grenades | 4 | L | - | 1 | - | - | - | NO | YES (hostile area) |

Note: the brief expected "Force Heal = party-area heal". The data says otherwise: Heal/Improved/Master Heal are T (single
creature); the party-wide one is Revitalize (P). "Cure" = row 10 whose label is FORCE_POWER_CURE but display name "Heal" (strref 287).

## 5. Rule output: 20 targetable rows (11 healing)
Healing (isHealing, 11): 10 Heal, 28 Improved Heal, 134 Master Heal, 64 Medpac I, 65 Medpac II, 66 Medpac III, 67 Antidote Kit,
84 Repair Kit, 127 Advanced Repair Kit, 128 Construction Kit, 129 Recovery Stim.
Targetable but not healing (9, all usertype 4, category 0x1808, range T): 69 Adrenaline Shot: Strength, 70 Adrenaline Shot: Alacrity,
71 Adrenaline Shot: Stamina, 72 Hyper Adrenaline Shot: Strength, 73 Hyper Adrenaline Shot: Alacrity, 74 Hyper Adrenaline Shot: Stamina,
75 Combat Shot: Battle Stimulant, 76 Combat Shot: Hyper Battle Stimulant, 77 Combat Shot: Speed Stimulant.

T-range named rows REJECTED and why (so the rule's cuts are auditable): usertype 2 (52 Body Fuel, 53 Combat Regeneration,
57 Psychic Static, 62 Whirling Dervish, 63 Rage, 83 Slam Attack); hostile (78/79 Shield Disruptor, 80 Oil Slick);
itemtargeting=1 (68, 85, 86, 99-107, 109-115, 126, 132, 257); BM 138-143; 108 Cloaking Device; 283 "Meditated" (OTGM marker).

## 6. Edge cases / open questions (not verifiable statically)
1. Who may be the target of a T-range power/item from the hotbar? The engine's "target menu" path (hud+0xCC, doc 07) picks any creature; whether
   the engine accepts a friendly party member when invoked programmatically (and the action the hotbar invokes passes the target) is the RE
   task for the other agent. The data only says range T / hostilesetting 0 permit it. Medpac use via the vanilla inventory UI applies to
   the owner of that inventory; in the HUD quick-slot it is single-target touch (doc R4: AI medpacs are cast on OBJECT_SELF only).
2. Organic vs droid: medpacs (0x1408) vs repair kits (0x1404) have no species flag in spells.2da; vanilla scripts/engine (HK-47, G0-T0)
   decide. "Lowest HP% member" for a medpac should skip droids and for a repair kit skip organics, else the cast may fail or do nothing.
   Recommend filtering by creature type in the mod, or falling back to the default target when the lowest-HP member is the wrong kind.
3. Dead members: Revitalize (P) is the only revive-class row (cat 0x2441, ff 18); not targetable by the rule. Heal on a dead member
   is not expected to work, so the lowest-HP% pick should exclude dead/unconscious members.
4. Battle Meditation is range T (nominal); the rule excludes it by the usertype-1 + 0x1808 clause. If a modded row has another
   T-range 0x1808 power the clause also drops it (conservative). Cloaking Device needs the row-108 deny (no data column separates it from stims).
5. `itemtargeting` meaning (1/2/3) is inferred from which rows carry it, not from decompiled code. Medpacs/stims/kits have it blank;
   if a future item row has itemtargeting blank but is self-only (like Cloaking Device), add it to the deny list.
6. Modded spells.2da (Echoes etc.) may add rows: the rule is column-driven, so a new medpac row (T, 0x1408, hostilesetting 0)
   qualifies automatically. Blank `hostilesetting` (13 rows, all unnamed placeholders) is treated as "not 0" => not targetable.
7. Self vs other: a "lowest HP%" mode can resolve to the caster; that is fine for T-range (touch on self works).

## Opus gate correction (2026-10-03)
The agent's "Heal rows 10/28/134 are single-target touch" is WRONG for behaviour: their dialog.tlk descriptions (strrefs 136478/136479/136480)
say "heals all party members within a 15 meter radius" (range T in the 2DA, but the impact script is party-wide). Battle Meditation 138-140
is also "all party members". So in vanilla K2 **no Force power is a single-target friendly power**: the rule for powers is "never targetable"
(the mode button stays disabled for every power). Target modes apply to ITEMS only: medpacs 64-66, recovery stim 129, antidote 67,
repair kits 84/127/128 (droids only -> filter members by creature type), adrenaline/combat shots 69-77; excluded: shields 99-107/110-115/257
(itemtargeting=1), cloaking device 108. Item rule = usertype 4 && range T && hostilesetting 0 && forcehostile blank && itemtargeting blank && row != 108.
