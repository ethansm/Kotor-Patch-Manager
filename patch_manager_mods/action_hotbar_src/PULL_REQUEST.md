# Pull Request: Action Hotbar

### PR Title
`feat(hud): Action Hotbar, ten keybound slots for powers, feats, items and stances`

### Summary
Adds ten on-screen slots above the action bar for keys `1` to `0`. A slot holds a Force power, a feat, an item or a
stance/form, and uses it through the HUD's own quick-action path.

- In-game editor behind a wrench button. One hotbar per character.
- One hotbar per save: copied into each save folder on save, restored on load. A save with no copy loads empty.
- Item slots can target the lowest-HP party member (`@lowhp`) or the controlled character (`@self`). Rules come from the
  columns of `override/spells.2da`.
- The vanilla action bar is refreshed after an AI stance or form change.
- Self-contained: the DLL creates its controls, art and INI. No `.gui` or texture files.
- `force_power_hotbar.ini` from the earlier name (Force Power Hotbar) is copied to `action_hotbar.ini` on first run.
- Logging through the shared KpLog logger.

See `README.md` for keys, the INI reference and troubleshooting, and `CHANGELOG.md` for history.

### Files
- `manifest.toml`: id `action-hotbar`, version 1.2.1, Steam Aspyr only.
- `kotor2-steam-aspyr.hooks.toml`: 8 detours (below).
- `ActionHotbar.cpp`, `HotbarConfig.h`, `HotbarLogic.h`, `Tlk.h`, `TwoDA.h`, `HotbarTextures.inc`: the patch.
- `*_test.cpp`, `test.toml`, `testdata/`: host tests.

### Hooks
| Address | Function | Purpose |
|---|---|---|
| `0x0072CB63` | `CaptureHotbarKey` | DirectInput key event capture. |
| (hud-host) | `HotbarHudTick` | Per-frame bar, editor and tooltip update via `k2.hud@1`. |
| (hud-host) | `HotbarBuildButtons` | Build controls inside the HUD constructor via `k2.hud@1`. |
| (save-events) | `onSaveEvent` | Per-save hotbar snapshots and restores via `k2.saveevents@1`. |

### Test evidence
Host tests, run with `python3 scripts/run_tests.py -k <name>` (each passes, 2026-10-05):
- `config_roundtrip_test`: INI parse, format and round trip, target suffix grammar, bounds.
- `hotbar_logic_test`: pure logic (keys, target choice, snapshot pick and merge, layout, TargetProbe).
- `hotbar_log_test`: log formatters and delivery through KpLog.
- `textures_test`, `tlk_test`, `twoda_test`: embedded art, `dialog.tlk` and `spells.2da` readers.
- `common_contract_test`: pins the shared Common APIs the patch uses.

Live play checks: target-mode research and the action-bar refresh finding are in
`patch_manager_mods/07_force_power_hotbar_evidence/` (`target_modes/`, `action_bar_refresh/`). Per-save behaviour was seen
live on build 25 (snapshots written and restored, Steam Cloud lists them). 1.2.0 and 1.2.1 play checks are pending.

Screenshots: TODO (live)

Relates to #104, #72, #180
