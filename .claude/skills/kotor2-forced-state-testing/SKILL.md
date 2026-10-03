---
name: kotor2-forced-state-testing
description: >-
  Force a KOTOR II mod into an edge-case state and check the result. Use when
  the user or a plan says "test edge case", "force a state", "make a test
  save", "forge a save", "check the play log", "did the level-up test pass",
  or when a level-up override (or other Patch Manager mod) behaviour must be
  reproduced. Runs the offline fake-engine scenario, forges a NEW removable
  save slot, switches the game-dir ini, and checks the log afterwards.
---

# KOTOR II forced-state testing

Details, safety rules and the save layout are in `patch_manager_mods/21_FORCED_STATE_TESTING.md`; read it first if anything here is
unclear. Scenarios live in `tests/luo_scenarios/saves.toml` (one id = save edits + config + expectations).

## Step 1: pick or define the scenario
- List ids: `python3 -c "import tomllib;print(list(tomllib.load(open('tests/luo_scenarios/saves.toml','rb'))['scenario']))"`.
- New edge case: add a `[scenario.<id>]` block (format in the toml header). Anchor regexes with `^`; APPLIED logs the preset id; the PC logs `tag=?`.

## Step 2: prove it offline
```
cd research/companion_levelup && python3 tools/parity/scenarios.py && python3 tools/parity/scenarios.py --m32
```
A failing offline scenario is a bug to fix before any live test.

## Step 3: forge a save (consent required)
```
.venv/bin/python3 scripts/kotor_save_forge.py list
.venv/bin/python3 scripts/kotor_save_forge.py forge <source-slot> <id>          # dry run: show the user
.venv/bin/python3 scripts/kotor_save_forge.py forge <source-slot> <id> --write  # only after the user says yes, game closed
```
Never write an existing slot, never launch the game. Tell the user Steam Cloud may sync the slot until cleanup. `unproven` edits must be named.

## Step 4: switch the config (consent required)
```
python3 scripts/luo_scenario.py plan <id> --guid g<hex>      # show the diff
python3 scripts/luo_scenario.py apply <id> --guid g<hex> --yes
```

## Step 5: the user plays
Record the current log length first: `wc -l "$G/levelup_override_log.txt"`. Give the user a short card: which slot to load, what to click.

## Step 6: check the log
```
python3 scripts/luo_log_check.py --from <old length + 1> --scenario <id>
```
Report each V item and scenario check with line numbers. Any FAIL: reproduce with the same id offline (Step 2), add exhaustive logging if
offline passes.

## Step 7: clean up
```
python3 scripts/luo_scenario.py restore --yes
.venv/bin/python3 scripts/kotor_save_forge.py cleanup
```
Persist the result in the mod's doc (doc 08 for the level-up override) and commit.
