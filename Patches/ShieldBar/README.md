# Shield Bar

> **STATUS** Track: SHIPPABLE. Version 0.1.3 (matches manifest.toml).
> **What it does / explores:** Draws an energy-shield absorb bar beside each party portrait's health bar, adds health, force and shield gauges to the companion portraits on the menu screens, and shows "Shield: cur/max" in the portrait hover tooltip.
> **Requires:** portrait-slots (`k2.portraitslots@1`, hard dependency), `Common/KpLog.h`, Steam/Aspyr LAA build (live-tested). GOG build is static-verified only.
> **Known limits:** GOG untested live. The corner texture `uibit_bar_sc_p` is not shipped (see Art). The `lfw == 0` fallback path (ShieldBar.cpp:418-443) is live-only. Draws beside buff-duration-hud. **Play-test status:** no shield-bar rows pending in the project play-test queue; the PRV row (10-20) covers the bar on load.
> **Install:** install through Patch Manager after portrait-slots. Not standalone (`requires = ["portrait-slots"]` in manifest.toml).

## Features
- Shield bar: one segment per shield, coloured like the shield's in-world VFX, draining as it absorbs damage.
- Companion gauges (health, force, shield) on the menu-screen portraits.
- Portrait tooltip line "Shield: cur/max".

## Configuration
- `shield_nodraw.txt` in the game dir: skips all shield bar draws (ShieldBar.cpp:343). Buff HUD's `buffhud_off.txt` is a separate switch in buff-duration-hud.
- `shieldprobe.txt` in the game dir: enables the extra probe work (effect dumps, GL queries, F9 mark). Diagnostics only.

## Logging
- Probe lines go to `shield-bar_log.txt`, channel `probe` at DEBUG (`kplog.ini` `[shield-bar] level=debug`).
- On an old core without `KpLog_Init` the fallback file is `shieldprobe_log.txt`.

## Hooks
- Steam: `0x00744EEC`. GOG: `0x005164EC`. Both unique across `*.hooks.toml`.
- Address rows: `kotor2-steam-aspyr.hooks.toml`, `kotor2-gog-aspyr.hooks.toml`.

## Art
- `uibit_bar_sh_p.tga/.txi` (shield bar) and `uibit_bar_vp_sq.tga/.txi` (squared health bar while a shield is up) are committed.
- `uibit_bar_sc_p` (corner filler) is not committed. Generate it with `scripts/gen_shield_bar_art.py` from the vanilla `uibit_bar_vp_p.tga`. Without it the corner notch is not filled.

## Build and test
- `python3 scripts/run_tests.py shield_logic`
- `python3 scripts/run_tests.py common_contract_test` (pins the portrait-slots Common API)

## Research
- `research/` holds the probe logs, screenshots, the Ghidra script and the blackout notes from the investigation. None of it is needed to build or install.

## Changelog
- No CHANGELOG.md. Readiness and open items: `PR_READINESS.md`.
