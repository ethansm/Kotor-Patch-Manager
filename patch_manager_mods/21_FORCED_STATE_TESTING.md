# 21 - Forced-state and mock testing (level-up override first; reusable by any mod)

Built in level-up Phase 6b (`research/companion_levelup/PHASE6B_PLAN.md`, Track M), 2026-10-03.
Goal: every edge case can be (a) proven offline against the REAL mod C++ with fake engine memory, and (b) reproduced in the game in about a
minute from a forged save, with the log checked automatically. Skill: `.claude/skills/kotor2-forced-state-testing/SKILL.md`.

## 1. Three layers, one scenario id
| layer | tool | what it proves | tier (doc 20) |
|---|---|---|---|
| offline fake engine | `research/companion_levelup/tools/parity/scenarios.py` (+ `--m32`) -> `scenario_test.cpp` | the real `LevelUpOverride.cpp` makes the expected decisions and log lines for the scenario's creatures / globals / ini | T2 (host), T3 (`-m32`, real 4-byte pointers) |
| ini overrides | `scripts/luo_scenario.py plan/apply/restore/status` | the game-dir `levelup_override.ini` / `levelup_playthroughs.ini` hold the scenario config | T5 accelerator |
| forged saves | `scripts/kotor_save_forge.py list/inspect/forge/cleanup` | the game loads straight into the edge state | T5 accelerator |
| log check | `scripts/luo_log_check.py --from N [--scenario ID]` | V-checklist 1-7 + the scenario's expected/absent events, replay of DECIDE lines through `simulate_rewrite.level_now` | T5 verdict |

All four read `tests/luo_scenarios/saves.toml` (format in its header). A live failure maps 1:1 to the offline repro with the same id.

## 2. Safety rules (forge + switcher)
- **New slots only.** The forge never writes an existing slot; new slot = max + 1, savenfo name `LUO TEST <id>`, a `forge_manifest.json`
  (source, scenario, edits, md5 of each changed resource) inside. Dry-run by default; `--write` needed.
- **Consent per write**, game closed (`pgrep -fa '^S:.*swkotor2\.exe'` empty; both tools refuse otherwise). Never launch the game from a session.
- **Cleanup:** `kotor_save_forge.py cleanup` lists every `LUO TEST` slot that has a manifest and removes them after confirmation.
- **Steam Cloud** may sync a forged slot (and its deletion) until cleanup.
- **ini switcher:** backup to `<game>/luo_scenario_backup/<timestamp>/` + `state.json` before any write; `restore` puts the latest back.
- **Only edited resources are re-serialised** (pykotor drops duplicate GFF fields the engine writes); every other resource is copied
  byte-for-byte (the tests assert this).

## 3. Save layout facts (M0, `research/companion_levelup/phase6_evidence/M0_save_layout.md`)
- Active party members exist only in `availnpc<n>.utc` inside SAVEGAME.sav. Standing NPCs also appear in the module `.git` Creature List
  (identical but ObjectId): the forge edits both; which one the engine prefers is unproven (M0 Q1).
- The live PC = nested `module.ifo` `Mod_PlayerList[0]` of `savenfo.LASTMODULE`; `pc.utc` is a stale template.
- Globals: GLOBALVARS.res `CatBoolean` names + `ValBoolean` packed **MSB-first** (index i = byte i//8, bit 7-i%8). pykotor's `GlobalVars`
  packs LSB-first: never use it for writes.
- pykotor 2.3.12 in `.venv` has patch #6 (nested capsule edits survive). ERF read/write is byte-stable per resource.
- Engine evidence: `SaveMemberByObject 0x5F8130` writes AVAILNPC from the live creature on every save; load = `LoadTableInfo 0x5FBF60` ->
  `CreateParty 0x5F9220` -> `SpawnNPC 0x5FD480`.

## 4. Scenario format (short; full header in `tests/luo_scenarios/saves.toml`)
```toml
[scenario.S2_atton_pause3_many]
title = "..."; source = "Game2"; status = "ready|unproven|offline-only"; v_items = [3]
[scenario.S2_atton_pause3_many.save]          # guid = keep|none|partial:n|g<hex>
[[scenario.S2_atton_pause3_many.save.creature]]
tag = "atton"; classes = [["Scoundrel", 3]]; xp = 105000; in_party = true
[scenario.S2_atton_pause3_many.config]        # playthrough_guid = save|none|g<hex>; companions = {tag = "preset|instant|hold|pause"}
[scenario.S2_atton_pause3_many.expect]        # regexes after the [tick] prefix, ^-anchored; APPLIED logs the PRESET id; PC = tag=?
events = ['^HOLD tag=atton ', '^BANKED tag=atton ']
absent = ['^APPLIED tag=atton__']
```
Expected offline decisions live in `scenarios.py` `EXPECT_DECISIONS` for now (TODO: move to `[expect].decisions`).

## 5. Adding a scenario for another mod
- The forge is mod-neutral: party membership, XP, class levels and arbitrary boolean globals (`guid` handling is LUO-specific; add a generic
  `globals = {NAME = 0|1}` key when a second mod needs it). The hotbar / Buff HUD can reuse it for party/XP states.
- Offline: copy the `scenario_test.cpp` pattern (fake creature via `makeCreature`, fake party `fakeControlled/fakePartyGet`, fake globals
  table, `setIni`, `logHas`) and include the mod's `.cpp` with the parity stub (`tools/parity/stub/`).
- Log check: a checker per mod (events differ); `luo_log_check.py` is the template (typed events, rule table, scenario regexes).

## 6. Current scenarios (level-up)
S1 hold default, S2 pause 3 (banks N=12), S3 early conversion (unproven: added class has no feats/powers), S4 cap, S5 partial GUID, S6 orphan
GUID, S7 GUID removed (mod-removal safety), S8 Kreia row, S9 G0-T0 (offline-only: no save with G0-T0 yet), S10 PC leads, S11 three
companions (atton/kreia/t3m4). Offline extra: X1 UI apply writes the 32 globals.
Source save Game2 has nobody in the party, so every `in_party` edit is unproven until one forged save has been loaded; a dedicated base save
with Atton in the party would remove that caveat.

## 7. Typical live session (about 1 minute per scenario)
1. `python3 research/companion_levelup/tools/parity/scenarios.py` green.
2. `.venv/bin/python3 scripts/kotor_save_forge.py forge Game2 S2_atton_pause3_many` (dry run) -> consent -> `--write`; note the new slot's GUID.
3. `python3 scripts/luo_scenario.py plan S2_atton_pause3_many --guid g<hex>` -> consent -> `apply ... --yes`.
4. `wc -l` the game log, user loads `LUO TEST S2_...` and plays the steps.
5. `python3 scripts/luo_log_check.py --from <line+1> --scenario S2_atton_pause3_many`.
6. `luo_scenario.py restore --yes`, `kotor_save_forge.py cleanup`.
