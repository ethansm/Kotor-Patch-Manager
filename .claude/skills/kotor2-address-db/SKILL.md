---
name: kotor2-address-db
description: >-
  Rules for editing the KOTOR II Steam address database (kotor2_steam_aspyr.db) in the kpm-pr-steam-db worktree:
  validation standard, SqliteTools integrity gate, commit granularity, and live-game sync. Use before adding,
  changing, validating, or committing any address-DB row, or when the user mentions Steam DB batches or PR 5.
---

# KOTOR II address database

- **Worktree:** `/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db`
- **Branch:** `addressdb/k2-steam-acc`
- **Target DB:** `AddressDatabases/kotor2_steam_aspyr.db`
- **Git Commits:** Keep commits granular by subsystem during active R&D. Do NOT squash until all batches are completed and the user explicitly directs opening PR 5.
- **Validation Standard (Zero Forced Guesses):**
  - 16-byte boundary alignment (`0x...0`).
  - Standard x86 prolog (`55 8b ec ...` / `56 8b f1 ...` / `e9 ...`).
  - x86-32 ABI stack cleanup normalization: `RET n == ceil(param_size_bytes / 4) * 4`.
  - Semantic callee topology and struct offset verification.
- **Batch Pipeline Tool:** For batch generation and persistence:
  ```bash
  python3 patch_manager_mods/steam_db_batches/tools/batch.py persist <batch> [--apply]
  ```
- **Integrity Gate:** Always validate with `SqliteTools` before committing:
  ```bash
  /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db/tools/SqliteTools/bin/Debug/net8.0/SqliteTools validate --database /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db/AddressDatabases/kotor2_steam_aspyr.db
  ```
- **Live Game Deployment:** Sync validated database to:
  `~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/addresses.db`
