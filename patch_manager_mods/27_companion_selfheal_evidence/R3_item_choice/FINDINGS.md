# R3 item choice: smallest healing item that covers missing HP (static, 2026-10-04)

## Answer in one paragraph
Heal amount lives in the compiled script `k_sup_healing` (spells.2da `impactscript` for rows 64,65,66,67,84,127,128,129), NOT in a 2da/item property. It is SKILL-SCALED on the USER (OBJECT_SELF = the creature that uses the item), not the target. 8 vanilla items, 6 heal single targets (3 medpac tiers organic, 3 repair tiers droid). Antidote heals nothing; Squad Recovery Stim heals 10 to the whole party. Enumerate candidates from HUD group 1 (name carries stack count) and key the formula by the item's spells.2da row. `items.tsv` has all 8.

## Q1 Heal amounts and formula
Source: `evidence/k_sup_healing.nss` (vanilla Scripts.bif nss, extracted with pykotor; generator `evidence/enum_heal_items.py`). Final apply: `EffectHeal(nSkill)` to `GetSpellTarget()` (nss:72-76). Items -> spell: UTI property 0 = CastSpell (type 10), `subtype` = **spells.2da row directly** (not iprp_spells; matches doc 07 TARGETABLE.md). spells.2da: usertype 4, range T, hostilesetting 0.
- Medpac (row 64, cat 0x1408): `TI + 10 + WIS` (nss:19-22)
- Advanced Medpac (65): `20 + 2*TI + WIS` (:23-26)
- Life Support Pack (66): `30 + 3*TI + WIS` (:27-30)
- Repair Kit (84, cat 0x1404): `Repair + INT + 15` (:36-39)
- Advanced Repair Kit (127): `2*Repair + INT + 25` (:40-43)
- Construction Kit (128): `3*Repair + INT + 35` (:44-47)
- Antidote (67, cat 0x1048): no heal. Squad Recovery Stim (129, cat 0x1408): fixed 10 to each party member plus resurrection (:48-70).
TI = `GetSkillRank(SKILL_TREAT_INJURY)`, Repair = `GetSkillRank(SKILL_REPAIR)` of the USER; WIS/INT = `GetAbilityModifier` of the USER (standard (score-10)/2). No skill-ranks cap; heal is capped by EffectHeal at max HP. Not target-dependent, so a companion using its own item is scaled by its own skill; the existing `@lowhp` path (PerformUseItem from controlled char) uses the CONTROLLED character's skill, not the target's.
- Live check: `override/spells.2da` differs from vanilla only in force-power rows (impactscripts m_imp_fpaligned, k_fp_heal*ti); rows 64-67,84,127-129 and baseitems 55/56/94 are identical to vanilla; no `k_sup_healing`/`g_i_med*` in override. So the formulas above are live. Sweep of every UTI in the chitin key (incl. any module resources reachable through pykotor iter_all_resources) found exactly the 8 resrefs in items.tsv; baseitems 55/56/94 stacking=99, stims/adrenals are rows 69-77 (k_sup_comshots, buffs, not heals; exclude).
- Reading inputs at runtime (unverified): creature stats ptr = server creature+0x1198 (Creature_Stats, K2Addr.h:271); skill ranks `byte*` at stats+0x174, indexed Computer0 Demo1 Stealth2 Aware3 Persuade4 Repair5 Security6 TI7 (doc 08 L101; CharEffectsPanel.cpp:443 order); base ability bytes +0xED.. (STR DEX CON INT WIS CHA, doc 08 L100). Base only: gear/buff bonuses are NOT included, so the estimate is a lower bound. GetSkillRank returns -1 when the creature lacks the skill (droids/organics cross-use is already refused by CanUseItem). Calibrate once with a TargetProbe-style log of (estimate vs actual HP delta).

## Q2 Enumerating items with stack counts
Reuse the HUD group read; no server inventory walk needed. `readGroup(hud, 1, &es)` (ActionHotbar.cpp:196-228) yields `Entry{key,name}`; group 1 = medpacs/stims/antidote/repair kits (doc 07 L599: Exile group 1 = adrenals, Battle Stim, Antidote, Medpac, Repair Kit). Stack count: `hb::StackCount(key,name)` (HotbarConfig.h:85) parses "Name (N)"; the engine prints the suffix only when N>1, so no suffix = 1, and an item absent from the list = 0. For each entry: `clientItem = key & ~ItemKeyFlag`, `itemPtr = serverItemFor(clientItem)` (ActionHotbar.cpp:482), then `classifyItem(itemPtr)` (:505) gives spells row + `healing` (isHealingRow HotbarLogic.h:430) + usable flag (`entry+0xC`). Map row -> formula with a 6-entry table (formula is in the .ncs, so it cannot be read from 2da; the row key, not the display name, is the identity). Caveats: HUD lists are the shared party inventory but refresh only on inventory change (resolve at click time, as `resolve()` does); if the engine ever lists two stacks of one resref separately that is fine (separate candidates); whether it merges them is unverified (open, check on a live run with log=debug).

## Q3 Droid vs organic gating
Sufficient. `CanUseItem(serverCreature, itemPtr, 0, 1)` strict (0x5D3650 race gate via baseitems+0xB4; full-HP refusal 0x5D3023) is already used as `eligible` in targetedUse (ActionHotbar.cpp ~558) and documented in doc 07 L207-L209. For the self-heal: build candidates only from entries where CanUseItem(companion.server, item, 0, 1) != 0. Row category also tells the family (0x1408 organic medpac, 0x1404 droid kit) so the picker needs no isDroid flag. Strict refusal also covers "full HP" (missing=0), so it is a safe second guard but pickHealItem must still return NONE itself. Caveat: Recovery Stim row 129 and Antidote 67 pass `isHealingRow` and pass CanUseItem for hurt organics, so they must be excluded by row, not by gating.

## Q4 Rule (pure function)
```cpp
struct HealCand { std::string name; int key; int row; int heal; int count; int cost; bool usable; };
// heal: estimated HP restored (>0). cost: UTI cost/basecost for tie-break (0 if unknown).
// Returns index into c[], or -1 for "do nothing".
int pickHealItem(int missingHp, const HealCand* c, int n);
```
1. `missingHp <= 0` -> -1.
2. Drop candidates with `!usable || count <= 0 || heal <= 0` or `row` in {67 antidote, 129 stim} (exclude by row; no-heal and party-wide). Any non-heal ids (0x1808 buffs, rows 69-77) never enter (isHealingRow false).
3. If none left -> -1.
4. covering = candidates with `heal >= missingHp`. If non-empty, pick the smallest `heal`. Else pick the largest `heal` (best partial; next click re-runs).
5. Ties on heal: lower `cost`, then larger `count` (keep scarce items), then lower `key` (stable). Vanilla has no equal-heal pair in one family at the same skill, so ties arise only from stacks/mods.
6. Scaled heals use the USER's own skill: for companion self-heal that is the companion's own TI/Repair/WIS/INT. Never use the controlled char's skill for a companion's own item.
7. Organic vs droid never both appear after CanUseItem gating; the rule does not need to know race.

### Worked examples (companion TI=10, WIS +2 -> Medpac 22, Adv 42, LSP 62; droid Repair=8, INT +3 -> Kit 26, Adv 44, Constr 62)
| # | situation | candidates (heal x count) | missing | pick | why |
|---|---|---|---|---|---|
| 1 | organic, small wound | Med 22x5, Adv 42x2, LSP 62x1 | 15 | Medpac | 22>=15 smallest |
| 2 | organic, 30 missing | Med 22x5, Adv 42x2, LSP 62x1 | 30 | Advanced Medpac | Med 22 does not cover; 42 smallest covering |
| 3 | organic, 100 missing | Med 22x5, Adv 42x2, LSP 62x1 | 100 | Life Support Pack | none cover -> largest |
| 4 | exactly equal | Med 22x1, Adv 42x1 | 22 | Medpac | heal>=missing is inclusive |
| 5 | droid | Kit 26x3, Adv 44x1, Constr 62x0 | 40 | Advanced Repair Kit | Kit 26 short; Constr count 0 dropped; 44 covers |
| 6 | full HP | any | 0 | none (-1) | zero missing |
| 7 | only stim and antidote held | Antidote, Recovery Stim 10x4 | 50 | none (-1) | excluded rows 67, 129 |
| 8 | stack 0 / unusable | Med 22x0, Adv 42x2 | 10 | Advanced Medpac | Medpac dropped (count 0) -> next covering |
| 9 | tie | stack A Med 22x1 cost 40, stack B Med 22x4 cost 40 | 20 | stack B | same heal, same cost, larger stack |
| 10 | organic with skill 0, WIS 0 | Med 10x2, Adv 20x1 | 15 | Advanced Medpac | scaled: 10<15, 20 covers |
| 11 | high skill (TI 24, WIS +3) | Med 37, Adv 71, LSP 105 | 60 | Advanced Medpac | scaling shifts thresholds, same rule |

Note on estimate error: if the base-stat estimate understates (gear bonuses), a smaller item might overheal never harmful; if it overstates, the click leaves some HP missing and the next click uses the next item. Neither wastes more than one tier.

## Open items
- Verify the engine HUD does not merge/split stacks unexpectedly (Q2 caveat).
- Calibrate estimate (stats+0x174/+0xED offsets and GetSkillRank including item bonuses) against live HP deltas.
- Other mods changing `k_sup_healing.ncs` would invalidate the table (none live now).
