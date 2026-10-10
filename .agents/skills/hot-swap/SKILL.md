---
name: hot-swap
description: >-
  Safely hot-swap built KPM mod DLLs into KOTOR II without running KPatchLauncher.
  Use when testing, updating, or rolling back mod DLLs (combat-hotbar, companion-selfheal,
  action-hotbar, queue-hud, etc.) in the live game.
---

# KPM DLL Hot-Swap

Quickly and safely hot-swap mod DLLs directly into the live KOTOR II game directory (`~/.steam/steam/steamapps/common/Knights of the Old Republic II/patches/`), avoiding `KPatchLauncher` uninstalls and peer DLL reversions.

## Workflow

### 1. Pre-Swap Diff Gate Check (Dry Run <100ms)
Verify that hooks, exports, imports, and manifests match live game configuration:
```bash
python3 scripts/kpm_hotswap.py --mod combat-hotbar companion-selfheal action-hotbar queue-hud --check
```

### 2. Full Gate & Automated Hot-Swap
Rebuilds/smokes target DLLs, verifies compatibility, creates an atomic backup snapshot in `$GAME_DIR/backups/`, and swaps DLLs:
```bash
python3 scripts/kpm_hotswap.py --mod combat-hotbar companion-selfheal action-hotbar queue-hud --gate --apply
```

### 3. Fast Swap (Gated DLL Reuse)
If `scripts/pre_install_gate.py` already completed, swap instantly without double compilation:
```bash
python3 scripts/kpm_hotswap.py --mod combat-hotbar --apply
```

### 4. Rollback
Instantly restore previous DLLs and configuration files:
```bash
python3 scripts/kpm_hotswap.py --rollback latest
```
