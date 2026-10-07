# PR Draft: tools/affected-patches.py - Dependency & Affected Patch Selector

**Target Repo:** `LaneDibello/Kotor-Patch-Manager`  
**Branch:** `feat/affected-patches` (cut from `master`)  
**Status:** DRAFT (Do not open or post without explicit conversational chat approval)

---

## Summary

When touching shared code (`Patches/Common/*`), library patches (`kind = "library"`), hooks, or core `.NET` runtime code, contributors currently have to guess which downstream patches are affected, or wait for CI to build every patch directory sequentially.

This PR introduces `tools/affected-patches.py`: a zero-dependency, stdlib-only Python 3.9+ tool that statically analyzes the dependency graph and determines the exact blast radius of a change.

## Key Features

1. **Include Graph Analysis (`#include` reachability):**
   - Traces `#include "..."` and `#include <...>` across `Patches/*/` source files (`.cpp`, `.h`, `.hpp`, `.inc`).
   - Resolves includes against patch-local folders, `Patches/Common`, `Patches/Common/GameAPI`, and `Patches/_shared`.
   - Handles transitive include chains (e.g. `Platform.h` -> `Common.h` -> `Snake.cpp`).
   - Connects `Common.cpp` changes to `Common.h` and modules linking GameAPI.

2. **Library & Interface Fan-out:**
   - Detects patches marked `kind = "library"` and cascades changes to consumers via manifest `requires` and `requires_interfaces` (e.g. `k2.portraitslots@1`).

3. **Core & Hook Scope Routing:**
   - Changes to `*hooks.toml` or `manifest.toml` route to `python3 tools/validate-patches.py` and `ShippedHooksTests`.
   - Changes to `src/KPatchCore/` route to `dotnet test tests/KPatchCore.Tests`.
   - Changes to `kpatch_api.h` or `src/KotorPatcher/` route to native test targets.

4. **Change Classes Scans Cannot Resolve:**
   - Explicitly flags `AddressDatabases/*.db` modifications (consumers refer to rows dynamically at runtime; recommends `SqliteTools validate` and flags the 13 GameAPI-linked patches).
   - Flags `Patches/Common/GameAPI/*.cpp` modifications (linked into Windows modules).

5. **Fast Local Execution (`--run`, `--list`, `--json`):**
   - `--list`: prints affected patch names (ideal for scripting).
   - `--why`: displays concise edge-chain reasons explaining why each patch was selected.
   - `--json`: machine-readable execution plan for CI integration.
   - `--run`: packages only affected patches to a temporary folder via `python3 Patches/create-patch.py -o <tmp>` without installing, and runs required validators.

## Compatibility

- **Python 3.9+ compatible:** Uses standard library only (`pathlib`, `re`, `argparse`, `dataclasses`, `json`). Uses `tomllib` when available (Python 3.11+), with a lightweight regex fallback for Python 3.9 macOS system environments.
- **Standalone Test Suite:** Accompanied by `tools/test_affected_patches.py` (xUnit-free, runs directly with `python3`).
- **Non-destructive:** Does not modify any existing patch source or manifest. `python3 tools/validate-patches.py` remains 100% green.

## Example CLI Usage

```bash
# Check affected patches for a shared header
python3 tools/affected-patches.py Patches/Common/Common.h --list

# Inspect reasons why patches are affected
python3 tools/affected-patches.py Patches/Common/Common.h --why

# Check all files touched relative to upstream/master
python3 tools/affected-patches.py --since upstream/master

# Build only affected patches and run validators
python3 tools/affected-patches.py --since upstream/master --run
```
