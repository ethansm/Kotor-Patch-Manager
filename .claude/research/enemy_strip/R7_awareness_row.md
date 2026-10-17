# R7: Awareness row in skills.2da and skill count (static)

Base paths: WS = `/home/ethansm/Games/KOTOR2_Mod_Workspace`; DEC = `WS/ghidra_dumps/steam_6a522e71/decomp`.

## Verdict

- Awareness = **row 3** (0-based), constant `SKILL_AWARENESS`.
- Skill count = **8** (vanilla K2 `skills.2da` has 8 data rows). The engine does not hardcode 8; it reads the count at runtime.
- Character sheet skill list is built in row order, so display order = 2DA order.

## Q1: Which row is Awareness (CONFIRMED, hard evidence)

- Source: `WS/backups/vanilla_clean/data/2DA.bif`, resource `skills` (2DA). It is not extracted as a loose file anywhere in the workspace, which is why earlier `find -iname skills.2da` runs came back empty.
- Read with pykotor `Installation.resource("skills", ResourceType.TwoDA)` (read-only; script at scratchpad `r7_skills.py`). Height = 8.
- Rows (0-based), label / constant / name StrRef / icon:
  - 0 ComputerUse / SKILL_COMPUTER_USE / 323 / isk_compuse
  - 1 Demolitions / SKILL_DEMOLITIONS / 324 / isk_demolitions
  - 2 Stealth / SKILL_STEALTH / 325 / isk_stealth
  - 3 **Awareness** / SKILL_AWARENESS / 326 / isk_awareness (keyability WIS)
  - 4 Persuade / SKILL_PERSUADE / 327 / isk_persuade
  - 5 Repair / SKILL_REPAIR / 328 / isk_repair
  - 6 Security / SKILL_SECURITY / 329 / isk_security
  - 7 TreatInjury / SKILL_TREAT_INJURY / 330 / isk_treatinj
- Script constants agree: `WS/mod8_kite_script/vanilla/nwscript.nss:1046` (`SKILL_AWARENESS = 3`), lines 1043-1051 for the full set and `SKILL_MAX_SKILLS = 8` at `:1051`. Same in `WS/01_extracted/mod127_repair_hk47/tslpatchdata/nwscript.nss:1051`.
- pykotor: `WS/venvs/k1port/lib/python3.12/site-packages/pykotor/common/scriptdefs.py:781` (AWARENESS 3), `:786` (MAX_SKILLS 8).

## Q2: Skill count in the engine (CONFIRMED that it is data-driven)

- Loader: `DEC/6a/006a17a0.c:27` loads the `"Skills"` 2DA. `:32` takes the 2DA field `local_120[9]` (byte count). `:33` stores it to `rules+0x123` (`DAT_00a1b4d0 + 0x123`). `:47` loops `0..count-1` over rows, 0x20 bytes each (`rules+0x12C` table).
- Caller: `DEC/69/0069dea0.c:152` (`FUN_006a17a0()`).
- Value 8 comes from the 2DA height (Q1). Field `[9]` = row count is inferred from the loader's use, not proven separately.
- Note: `DEC/6f/006f1770.c:57` also loads `"skills"` but inside a class-table routine (`CSWClass::LoadSkillsTable`, string at `strings.tsv:2868`). It is not the rules count source.

## Q3: GetSkillRank bound and array read (CONFIRMED)

- `DEC/6b/006b7bb0.c:17`: `if (param_2 < *(byte *)(DAT_00a1b4d0 + 0x123))` is the skill-index bound (count from Q2).
- `DEC/6b/006b7bb0.c:18`: `local_8 = (int)*(char *)(*(int *)(param_1 + 0x174) + (uint)param_2);`. The int8 array is read through a pointer at stats+0x174, index = skill row.
- `DEC/6b/006b7bb0.c:244`: `local_64 = 0;` is the out-of-range else. Out-of-range skill returns 0.
- Skill-index switch: `DEC/6b/006b7bb0.c:130` `case 3:` is the Awareness branch (AI3 mapping at `WS/ghidra_dumps/steam_6a522e71/findings/AI3.md:55`, perception use at AI3 line 115).

## Q4: Character sheet order (MATCHES, by loop)

- `DEC/8a/008a48a0.c:15-21`: `for (local_18 = 0; local_18 < count; ...)` where count = `rules+0x123`. Each row calls `FUN_006b7bb0(row,0,1)` (base) and `FUN_006b7bb0(row,0,0)` (total), then `FUN_008a5fb0` (row values) and `FUN_008a5dd0(row,0)`. No reorder inside the loop.
- Caller: `DEC/8a/008a3a80.c:13`.
- So sheet order = 2DA row order = ComputerUse, Demolitions, Stealth, Awareness, Persuade, Repair, Security, TreatInjury.
- Not checked: the GUI template slot layout (`skillinfo_p`, `DEC/89/0089c5f0.c:36`, `LB_SKILLS` at `strings.tsv:4865`). If the template pins slots by name, the visual order could differ from the loop order.

## Stale notes to update

- `patch_manager_mods/39_enemy_effects_strip.md:53` (repo root) says "Awareness row 3 and skill count 8 still unverified (skills.2da not extracted)". Now verified (Q1, Q2).
- `.claude/research/enemy_strip/R4_awareness_rank.md:42`, `:48`, `:69-70` mark row 3 and count 8 as unproven. Now CONFIRMED by the vanilla 2DA.

## Open

1. `DEC/6b/006b7bb0.c:131` (case 3) gates feats via `FUN_006b83f0(0x7a)`, `0x79`, `10`; `0x7a` also appears at `:149` and `:207` in the same function (other branches, not checked). nwscript `FEAT_SKILL_FOCUS_AWARENESS = 72` (`nwscript.nss:1130`). The feat IDs do not match; mapping not explained. Row mapping is unaffected.
2. Allocation size of the stats+0x174 array vs count not inspected (only the read bound at `:17`).
3. Field `[9]` = row count (Q2) is inferred from loader use, not proven from the 2DA class.
4. GUI slot layout for the char sheet not inspected (Q4).

## Method

- Search order: `search_docs.py "skills.2da"` (no hit on the 2DA; hits were unrelated V6/AI3 text), `rg` on patch_manager_mods (only 39_enemy_effects_strip.md:12/:53 and R3 notes), workspace `rg -l Awareness` (only AI3.md and a readme), then 2DA read from the backup BIF (hard evidence), then decomp.
- No live game files were written. Read-only access to `backups/vanilla_clean`.
