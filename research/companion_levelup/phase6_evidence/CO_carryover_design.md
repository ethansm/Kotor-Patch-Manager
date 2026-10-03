# CO - "carry-over" rule for early conversion (design + Python prototype)

Files: `CO_carryover_proto.py` (rule + generator/parser twins of the new rule data), `CO_carryover_check.py` (B2 matrix re-run, writes `CO_carryover_results.txt`).
Run from `research/companion_levelup`: `../../.venv/bin/python phase6_evidence/CO_carryover_check.py`. Nothing outside these files was edited.

## Rule (as implemented; deviations from the brief marked)

Inputs at the moment `computeRewrite` runs for a PATH/REBASE decision: `rec` (the record found by `levelNow`), `cl` (its character level = `recLevel`), `key` (character level being reached),
the preset's full record list, the creature's known feats and known powers, the pending list, `rec.cls`.

1. **On-time gate.** `key == cl` and nothing owed from records `< cl` -> the existing `computeRewrite` result, unchanged (this is what makes on-time byte-identical, see below).
   Anything else (REBASE, i.e. `key < cl`, or a PATH with something missing) -> carry mode.
2. **owed** (separately for feats and powers) = ids of `rec.feats` / `rec.powers` of every record with char level `< cl` (all classes, so the base-class records that early conversion skipped
   are included), in record order then in-record order, minus ids the creature already has, then this record's own ids appended last (also minus known). Granted feats are not in `rec.feats`, so they are never owed.
   Duplicates collapse to the first occurrence.
3. **Slots.** Feat slots = `len(rec.feats)`, power slots = `len(rec.powers)` (unchanged from what the record gives). Kept engine-granted feats/powers (`rec.granted`, `ENGINE_GRANT_POWERS`) stay first, exactly as today.
   Skills, ability, unspent, `negative-unspent` handling are untouched (taken from the plain `compute_rewrite` result).
4. **Per slot**, scan owed in plan order; for candidate `e` (not known, `known` = creature + kept granted + picks already made in this list):
   - legal -> take it. Legal = a rule row exists, `rec.cls` may pick it (feat.2da `<cls>_list` in {0,1}), `mincharlevel <= key` (powers: spells.2da class column `<= key`, 0 = no minimum, 99/-1 = never), all `prereqfeat1/2` known, and (if any) one of `orreqfeat0..4` known (powers: every `prerequisites` id known).
   - illegal **only** because a prerequisite is missing -> resolve the prerequisite chain (depth <= 4, children via the same test, OR-lists: first legal alternative) and take the first legal prerequisite instead. Refinement vs the brief: the prerequisite does **not** have to be in the plan (a plan item can depend on something class-granted in a skipped base level, e.g. Toughness); it only has to be legal and choosable. The data never needed this: 0 off-plan pulls. "Prereq-first" picks do occur (199 with an empty start state, 12 with the utc state) but always for a prerequisite that was itself in the plan, later than the dependent.
   - illegal for level / class -> skip, try the next owed id ("wait until allowed").
   - nothing legal -> the slot is left **empty** (shorter pending list). Alternative (not chosen): fall back to the original, illegal pick. See "Open points".
5. Stateless: everything is recomputed from the creature's feats/powers and the table each level-up; nothing is saved.

## Results (B2 matrix: 34 presets, 192 (preset, conversion level) scenarios x hold on/off, 3774 applied records per mode)

| metric | B2 baseline | carry-over |
|---|---|---|
| feat/power rows that lose a prerequisite (B2 metric: new vs design) | 94 | **0** |
| feat/power rows below min character level (B2 metric) | 250 (235 powers + 15 feats) | **0** |
| absolute prereq / min-level / not-choosable at placement | 681 / 250 / 48 (empty start) | 0 / 0 / 0 |
| on-time byte identity | - | identical (28658 comparisons + full on-time engine loops) |

Same with the real utc start state (130 prereq rows before, 0 after). Residual violations: none. Residual *losses* (the price, inherent in one-slot-per-level): items never obtained by character level 30 rise, because skipped base-class picks now
take slots in plan order and the Jedi records (which end at character level 30) push their last items off the end: all of them are "legal by level 30, no slot left" (feats 606, powers 134 summed over scenarios; empty start), none is blocked by a rule.
Unfilled slots: 134 power slots (a level whose owed powers are all below their minimum character level), 0 feat slots. Policy question for the user: plan order = base records first (user's rule, implemented);
a "Jedi-record first" tie-break would keep every Jedi item and lose the skipped base picks (today's behaviour) - one comparator change.

## Data needed in the runtime table

No new column on records/presets: the plan is the record list already in memory. New **rule data**, generated from feat.2da / spells.2da, two new sections (rows only for ids that occur in any record plus the transitive prerequisite closure; 70 feat rows + 69 power rows, 3.2 KB; table is 72 KB, `FileBufSize` 256 KB is fine):

```
[featrules]    f<id>=<mincharlevel>|<req feat ids ,>|<any-of feat ids ,>|<class ids that may pick it ,>
[powerrules]   p<id>=<prereq power ids ,>|<class>:<min character level> ,...        (99 = never selectable, 0 = no minimum)
```
Sources: `mincharlevel`, `prereqfeat1/2` (>0), `orreqfeat0..4` (>0), `<abbr>_list in {0,1}` for each class id used by any record (abbr map incl. 11/12/13 = jwm/jma/jwa), spells.2da `prerequisites` split on `_`, class columns guardian/consular/sentinel/weapmstr/jedimaster/watchman.
The prototype's `build_rules_text` is the exact emitter; `parse_rules_text` is the twin of the C++ parser and the check only ever runs the parsed text.

### Generator change (`gen_runtime_table.py`, not made)
After the preset bodies are built, collect `used feats/powers` (+ closure) and append the two sections; header "format v3". The pykotor `Rules` class needs `feat_list_cols` for classes 11-13 and `power_min_char_level` for 0 (levelup_rules.py only knows 3/4/5; add maps, do not guess columns: the snapshot header has `jwm_list/jma_list/jwa_list`, `weapmstr/jedimaster/watchman`).
`simulate_rewrite.parse_table_v2` and C++ `parseTable` must learn both section names (today an unknown `[section]` becomes a preset: `f19=...` fails with "unknown key" / the C++ `presetSlot` + `goto bad`, which rejects the whole table). Consequence: an old DLL rejects a v3 table (fail safe, vanilla); ship DLL and table together. `check_paths.py` should warn on a record feat/power with no rule row.

## C++ porting notes

- Where: inside `computeRewrite`, after the `r->cls != in.cls` check and before the feats loop; signature gains `const Preset* p, int recLevel, int key, const Known& k` (Known = creature feat ids + known power ids as small arrays). `levelNow` already returns `outRec`/`outCharLevel`, call site ~line 1346 has `key`, `recLevel`, `stats`.
  Skills/ability/unspent code below stays shared. Result keeps `nKeptFeats/nKeptSpells` = granted/engine-grant kept, then chosen ids (`applyRewrite` takes only the Pend + Result, no record).
- Tables: `struct FeatRule { uint8_t minChar, nReq, nOr; uint16_t req[2], orr[5]; uint16_t clsMask; }` indexed by feat id (`g_fr[256]`, max id 244, `present` flag; max `req` seen 2, `or` 0 but feat.2da allows 5).
  `struct PowerRule { uint8_t nPre; uint16_t pre[4]; uint8_t minLvl[16]; }` indexed by spell id (`g_pr[320]`, max id 284; class ids < 16, class mask via `cls < 16`). 
- Working arrays: owed/plan feats and powers `uint16_t/uint32_t[MaxPend=64]` (largest unique plan per preset: 18 feats, 36 powers), known-set arrays 64 + creature feat list up to the existing 200-entry read cap (copy of `classSkillMask`'s two-list read, St_FeatList 0x00 / St_FeatList2 0x18). Overflow of any cap -> log `CARRY skipped-cap` and fall back to the plain result.
- **Known powers are not read anywhere today.** The prototype needs the creature's known power ids (prereq + de-dup). The feat part can ship without them; the power part needs an RE of the class-entry/known-spell list (stride 0x28 class entries at stats+0x8D..; `HasSpell`-style engine call) - do it with the ghidra knowledge DB first, not by guessing offsets. Interim: power carry-over only uses ids obtained this session (not safe) -> do not ship partial.
- Logging: keep the `DRY` line format; add one `CARRY tag= key= cl= owed=F/P picks=... unfilled=F/P prereq_first=n` line only for carry-mode records, so on-time logs are byte-identical.
- Shorter-than-slot pending list: whether `applyRewrite` + the engine's pending "feats/powers remaining" counters accept fewer entries is untested (R10/M0 forged-save scenario needed before enabling); `already` status logic is unchanged (`same` compares the lists).
- Cap check on `rec.nFeats` slots: use the record's own count, not the engine's pending count.

## Tests to add

parity (`tools/parity/parity.py`, C++ vs `carry_rewrite` through a new `parity_main` verb or the scenario driver; compare `format_dry` + CARRY line):
1. All 4094 on-time records x clean + 6 random pending lists: output identical to plain (`info=ontime`).
2. The 192 matrix scenarios x both initial states: C++ and Python agree on every applied feat/power list.
3. Property test over the matrix: after carry, 0 prereq / min-level / class violations (independent 2da audit as in `CO_carryover_check.audit`).
scenarios (`scenarios.py`, fake engine + forged save):
4. Atton `melee_dual_dark_early` converted at level 3 (skips Two Weapon Fighting 3): Sentinel L8 pick list = [3,...], Two Weapon Advanced (9) / Mastery (85) only after 3.
5. Power below min level (Cure(10), min 6, designed at char 8, conversion at 3): key 4 gives other legal power or empty slot; Cure appears at key >= 6.
6. Empty-slot case (all owed powers blocked): list shorter than slots, log `unfilled`, game state consistent after Apply (live-verifiable).
7. Prereq chain: plan has 124 Master Toughness first, creature lacks 84/123 -> picks 84, then 123, then 124.
8. Creature already has the planned feat (manual pick): not re-listed, slot goes to the next owed item.
9. Player took a different feat on schedule (key == cl, owed non-empty): carry mode engages, stateless replay (`already` on second call).
10. VANILLA tail / HOLD / class-mismatch / no-record statuses unchanged; `negative-unspent` unchanged.
11. Table: v3 parse, unknown id row missing -> plain fallback, old-format table (no rule sections) -> carry disabled, plain behaviour.
12. Mutation list (`tools/parity/mutate.py`): flip `mc > key` to `>=`, drop the OR-list test, skip the on-time gate, drop the prerequisite pull - each must be caught.

## Open points / risks
- Slot-order policy (base first vs Jedi first) as above; user decision.
- Empty-slot vs fall back to the illegal planned pick when nothing owed is legal (134 power slots in the matrix).
- Only prerequisite feats/powers, min character level and class lists are modelled; `minattackbonus`, `minstr/dex/int/wis`, `reqskill`, `minspelllvl` are not (B2 did not either). Attribute minimums could still reject a "legal" pick in game.
- Class choosability uses `rec.cls` (the class gaining the level), which is an assumption about the engine's feat picker; verify live on one carry-over pick.
- Hold-class scenarios where records end before character level 30 leave later levels to the engine (VANILLA); extending carry-over into that tail (owed items into the engine's own pending slots) would recover the 134 empty power slots' items but needs the engine's pending counts.
