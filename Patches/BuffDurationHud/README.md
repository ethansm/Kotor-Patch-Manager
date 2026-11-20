# Buff Duration HUD

> **STATUS** Track: SHIPPABLE. Version 0.2.7 (matches manifest.toml).
> **What it does / explores:** Draws party buff/debuff icon columns with draining duration wipes on HUD portrait cards and menu companion portraits (hooks PortraitIconRowDraw).
> **Requires:** portrait-slots (provider `k2.portraitslots@1`, `Common/PortraitSlots.h`); KOTOR II Steam/Aspyr LAA (live-tested) and GOG/Aspyr (static-verified only); KPM.
> **Known limits:** Standalone: NO. The shield bar is a separate patch (shield-bar). GOG addresses are static-verified only and untested live. **Play-test status:** Steam live through BH-BG1 (LIVE 10-26..10-29, no pending BH rows in the project play-test queue, lines 233-235).
> **Install:** Install through KPM after portrait-slots.

## Features
- Buff/debuff icon columns on party HUD cards and menu companion portraits.
- Duration drain wipe on each icon; round backing disc for item abilities.
- Poison icon (embedded `bh_poison`) and item-ability badges (`bh_badge_*`), both generated `.inc` files.

## Logging
- Logs go through `KpLog.h` (`kplog.ini` section `[buff-duration-hud]`). The legacy flag file `buffhud_debug.txt` is only honoured on the old-core fallback.

## Hooks
- Steam (`kotor2-steam-aspyr.hooks.toml`): PortraitIconRowDraw at 0x00745A84, 0x00745A97, 0x00745AEC.
- GOG (`kotor2-gog-aspyr.hooks.toml`): PortraitIconRowDraw at 0x00517084, 0x00517097, 0x005170EC.
- These hook addresses appear only in this mod's hooks files (no overlap with other mods).

## Build/test
- Host tests: `python3 scripts/run_tests.py buff_hud_logic_test` and `python3 scripts/run_tests.py common_contract_test`.
- Lint: `python3 scripts/fork_lint.py buff_duration_hud`.

## Changelog pointer
- See project design doc 02 (research trail) and `PR_READINESS.md` (fork-prep status).
