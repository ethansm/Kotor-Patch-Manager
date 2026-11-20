# Portrait Slots (shared party-portrait hooks)

> **STATUS** Track: SHIPPABLE. Version 0.1.0 (matches manifest.toml).
> **What it does / explores:** Owns the party-portrait hooks (UpdatePortraits and the per-slot draw gate) and provides the `k2.portraitslots@1` registry so several HUD mods share them. Draws nothing by itself.
> **Requires:** a KotorPatcher with the patch registry (KPatchInit). Steam/Aspyr LAA is live-tested. GOG is static-verified only (see the GOG callout below). Consumers: buff-duration-hud, shield-bar, queue-hud.
> **Known limits:** Infrastructure only, useless alone. GOG untested live. The menu clamp uses a literal `& 3` eviction (K2PS_MAX_SLOTS = 4). **Play-test status:** row PRV in `patch_manager_mods/PLAY_TEST_QUEUE.md`, PASS 10-20.
> **Install:** Install through KPM as a provider. It is auto-added and listed first in `patch_config.toml`. Install it together with the consumer HUD mods. Do not install it alone for normal play.

## Features
- Hooks UpdatePortraits and the per-slot portrait draw gate once, so consumers do not hook the same sites.
- Registry `k2.portraitslots@1`: consumers register a phase, priority and owner. The header is `_shared_src/PortraitSlots.h`.
- Snapshot of companion portraits per frame, with fresh/stale slot tracking and eviction when more than 4 distinct slots are seen.

## Consumers
- `patch_manager_mods/buff_duration_hud_src` (buff-duration-hud)
- `patch_manager_mods/shield_bar_src` (shield-bar)
- `patch_manager_mods/queue_hud_src` (queue-hud)

Each consumer must list portrait-slots in its `requires`.

## Configuration
None. The mod has no ini keys.

## Logging
- Channel `portrait-slots/core` in kpatch.log. WARN is always written. INFO is written when kplog.ini enables it.
- Fallback file `portrait-slots_log.txt` in the game dir, set at INFO level (`PortraitSlotsProvider.cpp`, `KpLog_SetFallbackFile`).

## Hooks
Hook sites are in `kotor2-steam-aspyr.hooks.toml` and `kotor2-gog-aspyr.hooks.toml`. Design and address history: `patch_manager_mods/21_portrait_slots_and_split.md`. GOG addresses: `patch_manager_mods/21b_gog_address_map.md`.

## Build and test
- Scoped host tests: `python3 scripts/run_tests.py ps_dispatch_test ps_registry_test portrait_slots`
- Fork readiness: `python3 scripts/fork_lint.py portrait_slots`
- Readiness audit: `PR_READINESS.md` in this directory.

## GOG callout
GOG support is **static-verified only** (no GOG install here: hook bytes and addresses checked against the GOG exe in Ghidra, see doc 21b). **GOG contributor testing is required** before it is considered supported.

## What was learned
See `patch_manager_mods/21_portrait_slots_and_split.md` (Buff HUD / Shield Bar split).

## Changelog
No CHANGELOG.md yet. History: `patch_manager_mods/21_portrait_slots_and_split.md` and `git log -- patch_manager_mods/portrait_slots_src`.
