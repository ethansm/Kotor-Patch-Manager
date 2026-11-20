# Command Queue Probe (cmdq-probe)

> **STATUS** Track: DIAGNOSTIC. Version 0.2.0 (matches manifest.toml). Developer probe, silent by default.
> **What it does / explores:** Log-only probe of the command queue, pause, combat round, ClearAllActions, medpack/CanUseItem paths, and QS queue/round/effect snapshots of party casts. It changes no game behaviour.
> **Requires:** Steam/Aspyr build (target 6A522E71...BBFFEF), KpLog core (`Patches/Common` KpLog.h). No other mod required (`requires = []`, `conflicts = []`).
> **Known limits:** Log only. Shipped ini is silent (`probe=0`). Busy when on: raise `max_lines` (20000) for QS. Open "S1 stale build" notes are engine findings, not mod bugs (PLAY_TEST_QUEUE.md:199, :202). Play-test status: CQ-QS1 PASS (PLAY_TEST_QUEUE.md:199).
> **Install:** Developer use only. Do not install for normal play. It is useful only paired with `scripts/cmdq_probe_log_check.py` and the research notes below.

## Features
- Log channels: PAUSE (edge-only), CMD, FIRE, ROUND, CLR, REEQ, MED, QS.
- All channels share one line budget (`max_lines`).
- Handlers only read game memory through SafeRead. They make no writes, no engine calls, and return at once while their channel is off.

## Configuration (cmdq_probe.ini, game dir)
Read once at game start. A missing file or bad value keeps the silent default.
- `probe` master switch (0 = every hook returns at once).
- `ch_pause`, `ch_cmd`, `ch_fire`, `ch_round`, `ch_clr`, `ch_med`, `ch_qs`: a channel logs only when `probe=1` AND its key is `1`.
- `party_only` (default 1): CLR only for creatures with +0x11AC nonzero (meaning unverified). 0 drops that test.
- `max_lines` (default 2000, range 1 to 1000000): total log lines per game run.

## Logging (kplog.ini section, required)
Log lines are dropped unless the game-dir `kplog.ini` has a section for this patch with `level=info`:

```ini
[cmdq-probe]
level=info
```

The log file is `cmdq-probe_log.txt` in the game dir.

## Hooks
11 sites in `kotor2-steam-aspyr.hooks.toml`: SetPauseState 0x538ED0, AddCastSpellActions 0x565310, AddItemCastSpellActions 0x5648D0, cast fire 0x5BB07C, item fire 0x5B9414, StartCombatRound 0x58E730, EndCombatRound 0x58EA53, ClearAllActions 0x541080 (CSWSObject::ClearAllActions, DB-backed), ReequipResumeAttack 0x5DF920, PartyMenu item handler 0x77C780, CanUseItem exit 0x5D32AE. Several sites are mid-function detours, so they have no function-start DB row.

## Build/test
- Log checker (read-only, stdlib): `python3 scripts/cmdq_probe_log_check.py [LOG] [--json] [--all]`.
- Checker tests: `python3 scripts/test_cmdq_probe_log_check.py`.
- Fork lint: `python3 scripts/fork_lint.py cmdq_probe`.

## What was learned
- Research notes: the project design notes (not in this repo).
- QS (0.2.0, combat AI recast WI-134) captures a party member's action queue, round nodes, and active-effect spell ids at each cast queue, cast build, spell fire, and round end.

## Changelog pointer
Version history is in `manifest.toml` and the git log of this directory.
