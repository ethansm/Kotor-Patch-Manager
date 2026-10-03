# Retired scripts

`check_inv3d_kpatch.py`, `check_equip3d_kpatch.py`, `check_chareffects_kpatch.py`: per-mod `.kpatch` checkers with hardcoded paths, superseded on 2026-10-03 by `scripts/check_hook_bytes.py` (hook and code-patch bytes vs the vanilla exe, instruction boundaries, relative branches, `.kpatch` packaging, `--kpatch PATH` for staged builds) plus `scripts/check_exports.py` (export tables). Dropped without replacement: the "imports USER32" and per-API import checks (mod-specific; an import mistake fails at load time).
