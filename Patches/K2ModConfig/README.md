# Mod Configuration

> **STATUS** Track: SHIPPABLE. Version 0.4.9-dev (matches manifest.toml).
> **What it does / explores:** Adds a "Mod Configuration" button under Gameplay in the Options screen. Other mods register config pages into it through `k2.modconfig@1` and build them with `k2.menuui@1`, so every mod's settings look like vanilla menus.
> **Requires:** Steam/Aspyr KOTOR II (target hash in manifest.toml) and KPM. No other patch is required (`requires = []`). Optional `k2.guiinput` provider for mouse wheel scrolling.
> **Known limits:** `arrow_icons=1` is reserved (logged only, text arrows are used). The ESC hook is not unhooked on DLL detach yet (open safety item in PR_READINESS.md). Latched-click and fail-fast root clone optimisations are not done. **Play-test status:** rows MC1 and ESC2 in `patch_manager_mods/PLAY_TEST_QUEUE.md` are pending (live-only, not blocking).
> **Install:** KPM install of `mod-config` (provider, install before its consumers: holster-editor, enemy-effects-strip, inventory-sort-filter).

## Features
- One "Mod Configuration" button after GAMEPLAY (or below QUIT when `reflow=0`).
- List page: one button per registered entry, hover shows the entry description.
- Entry page: the entry's own controls (toggles, steppers, sliders, labels, custom panes), built by its `build()` callback.
- Esc closes the menu from any page; reopening starts at the Options root.

## Configuration (`mod_config.ini`, section `[mod-config]`)

| Key | Default | Meaning |
|---|---|---|
| `enabled` | 1 | Master switch. 0 makes every hook return at once. |
| `debug` | 0 | Extra log lines (hook entry, button rects, saved control flags). |
| `button_clone` | 1 | 1 = real button clones (BTN_GAMEPLAY, BTN_EXIT). 0 = label clones. A failed clone falls back to a label. |
| `reflow` | 1 | 1 = Mod Configuration sits after GAMEPLAY and later rows move down one pitch. 0 = below QUIT. |
| `desc` | 1 | 1 = hovering a button writes a description into the description pane. |
| `free_labels` | 1 | 1 = free our buttons and labels when the Options panel is destroyed. 0 = leak them. |
| `arrow_icons` | 0 | Reserved for chevron icons. Not built. |
| `selftest` | 0 | 1 or 2 registers a built-in test entry for play checks. Never leave on in a shipped config. |

## Logging
Add a `[mod-config]` section to the game-dir `kplog.ini`. Lines go to `mod-config_log.txt` (fallback level INFO). Check a session with `python3 scripts/kotor2_logs.py check mod-config_log.txt --new`.

## Interface summary for mod authors
Declared in `patch_manager_mods/_shared_src/ModConfigApi.h`. Provided by this mod; consumers use the struct sizes from that header.
- `k2.modconfig@1` (`K2ModConfigApi`): `Register(entry)` returns a token (0 = refused, all slots used); `Unregister(token)`. An entry carries `id`, `name`, `desc` and the callbacks `build`, `enter`, `leave`, `tick`, `destroy`.
- `k2.menuui@1` (`K2MenuUiApi`): `NewPage`, `AddButton`, `AddLabel`, `AddCustom`, `Clicked`, `SetText`, `SetRect`, `SetDesc`, `SetDescSplit`, `PaneRect`, `Control`. Only valid inside the entry's own `build()`, for that entry's page.
- Page changes fire on the left mouse button release edge.

## Hooks
Three Options-menu sites in `kotor2-steam-aspyr.hooks.toml` (plain detours, whole stolen instructions). Addresses are raw; lookup evidence for the PR text is still to be added (see PR_READINESS.md).

## Build and test
- Scoped tests: `python3 scripts/run_tests.py mod_config` (three suites: common contract, menu UI logic, slider row logic).
- Fork lint: `python3 scripts/fork_lint.py mod_config`.

## Known gaps
- Host-glue tests for `ModConfig.cpp` (readConfig, pane status floor, slider give-up paths) are not written. The logic headers are covered.
- Declarative layer (ModConfigDecl, PreviewCanvas, doc 46) lives outside this directory.
