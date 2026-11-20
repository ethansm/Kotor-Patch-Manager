# Companion AI

> **STATUS** Track: EXPERIMENTAL. Version 0.3.4-probe (matches manifest.toml).
> **What it does / explores:** Writes a party-AI trace log and applies INI-gated engine AI fixes for companion attack loops, follow-queue bumps, a NULL spell-row crash, and cast-queue handling.
> **Requires:** kotor2_steam_aspyr (Steam/Aspyr build, hash pinned in manifest.toml); Patch Manager with mod-config; `Common` headers (SafeRead.h, Effects.h, GameAddr.h). No other mod required or conflicting.
> **Known limits:** Perception throttle is off by default. Flurry bonus attacks are not addressed. Log-only probes add log volume. Unit tests and the upstream validator have not been run as part of this patch's PR cut.
> **Play-test status:** pending rows CAI-PT1/PT2, WI-120, WI-125, CA-FG1/FG2/FG5, CA-RS1, CA-MC1 in `patch_manager_mods/PLAY_TEST_QUEUE.md`.
> **Install:** Experimental. Do not install for normal play. Install only to test the INI flags, because the fixes are not yet verified in game.

## Features
- Attack-spin guard (`a1`): ranged and stationary party companions with an out-of-reach target retarget to an enemy in weapon range, or end the stuck attack.
- Follow-queue bump guard (`fm1`): keeps the push-aside move when a party member's queue holds only follow actions.
- NULL spell-row guard (`a4`): skips a NULL spell row in AIActionCombat.
- Cast-family guard (`cast_family_guard`) and cast-queue newest rule (`cast_queue_newest`).
- Opt-in perception throttle (`p1`, `p1_interval_ms`).
- Mod Configuration page: toggles the main fix flags; it rewrites the keys in `companion_ai.ini`.

## Configuration
`companion_ai.ini` (section `[CompanionAI]`). All flags 0 = vanilla behaviour. Main keys:
- Trace: `trace`, `scripts`, `engine_points`, `dedupe_ms`, `max_lines_per_sec`, `max_file_mb`.
- Fixes: `cast_family_guard`, `cast_queue_newest`, `p1`, `p1_interval_ms`.
- Logging: `log`, `debug_log`, `engine_print`, `engine_verbose`, `summary_sec`.
- Probes (log-only): `probe_rs`, `probe_pf`, `probe_rar`, `probe_shx`, `probe_gca`, `probe_party`, `probe_lines`.

The INI is re-read at most once a second, when the file changes. No restart needed.

## Logging
One log: `companion-ai_log.txt` in the game folder, through the shared KpLog core. Enable it with a `[companion-ai]` section in `kplog.ini`. On an old core the trace goes to `ai_trace.txt`.

## Hooks
33 hook entries in `kotor2-steam-aspyr.hooks.toml`. Hook addresses are not in `raw_addresses.toml`. Addresses are checked against the Steam address DB and the windows validation in the hooks header. No hook address is shared with another mod's hooks file.

## Build and test
```
python3 scripts/run_tests.py companion_ai
```
`test.toml` takes the KPM Common headers from `${KPM_COMMON_DIR}` (default `<KPM_DIR>/Patches/Common`).

## What was learned (EXPERIMENTAL)
- Out-of-reach attack loops come from the companion re-queuing its attack after the target leaves reach.
- The leader bump's `ClearAllActions` drops a push-aside move queued in front of a follow queue.
- Modded item spells can produce a NULL spell row in AIActionCombat.

Research notes and the full issue breakdown are in the project research folder. The changelog is in the git history of this directory.
