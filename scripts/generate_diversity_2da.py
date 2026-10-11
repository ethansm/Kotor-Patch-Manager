#!/usr/bin/env python3
"""Generate and append static 2DA entries for Master NPC Diversity Dispatcher.

Expands appearance.2da and heads.2da with non-destructive, idempotent rows:
  - Commoner_Fat_Male (N_CommM_Fat mesh + N_CommM texture)
  - Commoner_Fat_Female (N_CommF_Fat mesh + N_CommF texture)
  - Czerka_Officer_Female (N_CzerkaOff mesh + N_CzerkaOff texture)
  - Ashton/Leilukin Twi'lek Male combinations (llk_ahrnell..luposhar heads)
  - Lily Twi'lek Female dancer variants (N_TwiDance_Boot, N_TwiDance, N_TwiUnd)
  - Restored / custom heads in heads.2da (up to 254 usable head ids; the engine stores the id in one byte)

Base tables: when <game-override>/_diversity_premerge_backup/*.2da.bak exists, that pre-merge snapshot is the
base (the live files are only verified against it). Otherwise the live files are the base.

Generates:
  - <out-dir>/appearance.2da
  - <out-dir>/heads.2da
  - <src-dir>/DiversityRows.h (C++ header with compiled row IDs for runtime detour)
  - <out-dir>/diversity_2da_mapping.json

Usage:
  .venv/bin/python scripts/generate_diversity_2da.py [--game-override DIR] [--out-dir DIR] [--dry-run]
  .venv/bin/python scripts/generate_diversity_2da.py --apply [--force-base]   (verifies the live base first)
"""

from __future__ import annotations

import argparse
import hashlib
import json
import logging
import os
import shutil
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

logging.disable(logging.CRITICAL)

try:
    from pykotor.resource.formats.twoda import TwoDA, read_2da, write_2da
except ImportError:
    print("Error: pykotor must be installed (run inside the repo .venv).", file=sys.stderr)
    sys.exit(1)

# Default paths
_KOTOR2_ROOT = os.environ.get(
    "KOTOR2_ROOT",
    os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II"),
)
DEFAULT_GAME_OVERRIDE = os.path.join(_KOTOR2_ROOT, "override")
REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_OUT_DIR = REPO_ROOT / "patch_manager_mods" / "npc_diversity_src" / "override"
DEFAULT_SRC_DIR = REPO_ROOT / "patch_manager_mods" / "npc_diversity_src"

HEADS_HEADERS = [
    "head",
    "alttexture",
    "headtexvvve",
    "headtexvve",
    "headtexve",
    "headtexe",
    "headtexg",
    "headtexvg",
]

NEW_HEADS = [
    {"head": "llk_Ahrnell", "alttexture": "llk_Ahrnell"},
    {"head": "llk_Harra", "alttexture": "llk_Harra"},
    {"head": "llk_KehVehn", "alttexture": "llk_KehVehn"},
    {"head": "llk_LupoShar", "alttexture": "llk_LupoShar"},
    {"head": "llk_DahnisH", "alttexture": "llk_DahnisH"},
]

# The engine stores the head id in one byte; 255 is not a valid id, so the table may hold at most 254 rows.
MAX_HEAD_ROWS = 254

# Pre-merge snapshot of the live tables, written by the first --apply. It pins the base the rows were built on.
PREMERGE_BACKUP_DIRNAME = "_diversity_premerge_backup"

# Base template models to inherit common movement, sound, and animation attributes
MALE_COMM_TEMPLATE = {
    "race": "N_CommM",
    "walkdist": "1.813",
    "rundist": "3.66",
    "driveanimwalk": "1.813",
    "driveanimrun_pc": "3.66",
    "driveanimrun_xbox": "3.96",
    "modeltype": "B",
    "normalhead": "84",
    "backuphead": "",
    "modela": "N_CommM",
    "texa": "N_CommM",
    "modelb": "N_CommM",
    "texb": "N_CommM",
    "modelc": "PMBCM",
    "texc": "PMBC",
    "modeld": "PMBDM",
    "texd": "PMBD",
    "modele": "PMBEM",
    "texe": "PMBE",
    "modelf": "PMBFM",
    "texf": "PMBF",
    "modelg": "PMBGM",
    "texg": "PMBG",
    "modelh": "PMBHM",
    "texh": "PMBH",
    "modeli": "PMBIM",
    "texi": "PMBI",
    "modelk": "PMBKM",
    "texk": "PMBK",
    "modelm": "PMBMM",
    "texm": "PMBM",
    "modeln": "PMBNM",
    "texn": "PFBN",
    "envmap": "DEFAULT",
    "bloodcolr": "R",
    "wing_tail_scale": "1",
    "moverate": "NORM",
    "driveaccl": "50",
    "drivemaxspeed": "5.4",
    "hitradius": "0.25",
    "perspace": "0.35",
    "creperspace": "0.4",
    "height": "1",
    "targetheight": "l",
    "abortonparry": "0",
    "racialtype": "20",
    "haslegs": "1",
    "hasarms": "1",
    "footstepvolume": "1",
    "sizecategory": "3",
    "perceptiondist": "9",
    "footsteptype": "0",
    "soundapptype": "0",
    "headtrack": "0",
    "head_arc_h": "30",
    "head_arc_v": "15",
    "hitdist": "1",
    "prefatckdist": "1.4",
    "groundtilt": "0",
    "body_bag": "3",
}

FEMALE_COMM_TEMPLATE = {
    "race": "N_CommF",
    "walkdist": "1.6",
    "rundist": "3.66",
    "driveanimwalk": "1.6",
    "driveanimrun_pc": "3.66",
    "driveanimrun_xbox": "3.96",
    "modeltype": "B",
    "normalhead": "85",
    "backuphead": "",
    "modela": "N_CommF",
    "texa": "N_CommF",
    "modelb": "N_CommF",
    "texb": "N_CommF",
    "modelc": "PFBCM",
    "texc": "PFBC",
    "modeld": "PFBDM",
    "texd": "PFBD",
    "modele": "PFBEM",
    "texe": "PFBE",
    "modelf": "PFBFM",
    "texf": "PFBF",
    "modelg": "PFBGM",
    "texg": "PFBG",
    "modelh": "PFBHM",
    "texh": "PFBH",
    "modeli": "PFBIM",
    "texi": "PFBI",
    "modelk": "PFBKM",
    "texk": "PFBK",
    "modelm": "PFBMM",
    "texm": "PFBM",
    "modeln": "PFBNM",
    "texn": "PFBN",
    "envmap": "DEFAULT",
    "bloodcolr": "R",
    "wing_tail_scale": "1",
    "moverate": "NORM",
    "driveaccl": "50",
    "drivemaxspeed": "5.4",
    "hitradius": "0.25",
    "perspace": "0.35",
    "creperspace": "0.4",
    "height": "1",
    "targetheight": "l",
    "abortonparry": "0",
    "racialtype": "20",
    "haslegs": "1",
    "hasarms": "1",
    "footstepvolume": "1",
    "sizecategory": "3",
    "perceptiondist": "9",
    "footsteptype": "0",
    "soundapptype": "0",
    "headtrack": "0",
    "head_arc_h": "30",
    "head_arc_v": "15",
    "hitdist": "1",
    "prefatckdist": "1.4",
    "groundtilt": "0",
    "body_bag": "3",
}

CZERKA_OFF_F_TEMPLATE = {
    "race": "N_CzerkaOff",
    "walkdist": "1.6",
    "rundist": "3.66",
    "driveanimwalk": "1.6",
    "driveanimrun_pc": "3.66",
    "driveanimrun_xbox": "3.96",
    "modeltype": "B",
    "normalhead": "83",
    "backuphead": "",
    "modela": "N_CzerkaOff",
    "texa": "N_CzerkaOff",
    "modelb": "N_CzerkaOff",
    "texb": "N_CzerkaOff",
    "modelc": "N_CzerkaOff",
    "texc": "N_CzerkaOff",
    "modeld": "N_CzerkaOff",
    "texd": "N_CzerkaOff",
    "modele": "N_CzerkaOff",
    "texe": "N_CzerkaOff",
    "modelf": "N_CzerkaOff",
    "texf": "N_CzerkaOff",
    "modelg": "N_CzerkaOff",
    "texg": "N_CzerkaOff",
    "modelh": "N_CzerkaOff",
    "texh": "N_CzerkaOff",
    "modeli": "N_CzerkaOff",
    "texi": "N_CzerkaOff",
    "modelk": "N_CzerkaOff",
    "texk": "N_CzerkaOff",
    "modelm": "N_CzerkaOff",
    "texm": "N_CzerkaOff",
    "modeln": "N_CzerkaOff",
    "texn": "N_CzerkaOff",
    "envmap": "DEFAULT",
    "bloodcolr": "R",
    "wing_tail_scale": "1",
    "moverate": "NORM",
    "driveaccl": "50",
    "drivemaxspeed": "5.4",
    "hitradius": "0.25",
    "perspace": "0.35",
    "creperspace": "0.4",
    "height": "1",
    "targetheight": "l",
    "abortonparry": "0",
    "racialtype": "20",
    "haslegs": "1",
    "hasarms": "1",
    "footstepvolume": "1",
    "sizecategory": "3",
    "perceptiondist": "9",
    "footsteptype": "0",
    "soundapptype": "0",
    "headtrack": "0",
    "head_arc_h": "30",
    "head_arc_v": "15",
    "hitdist": "1",
    "prefatckdist": "1.4",
    "groundtilt": "0",
    "body_bag": "1",
}

# Specification of rows to append to appearance.2da
DIVERSITY_APPEARANCES: List[Dict[str, Any]] = [
    {
        "label": "Commoner_Fat_Male",
        "base": MALE_COMM_TEMPLATE,
        "overrides": {
            "modela": "N_CommM_Fat",
            "modelb": "N_CommM_Fat",
            "texa": "N_CommM",
            "texb": "N_CommM",
            "race": "N_CommM_Fat",
        },
    },
    {
        "label": "Commoner_Fat_Female",
        "base": FEMALE_COMM_TEMPLATE,
        "overrides": {
            "modela": "N_CommF_Fat",
            "modelb": "N_CommF_Fat",
            "texa": "N_CommF",
            "texb": "N_CommF",
            "race": "N_CommF_Fat",
        },
    },
    {
        "label": "Czerka_Officer_Female",
        "base": CZERKA_OFF_F_TEMPLATE,
        "overrides": {
            "modela": "N_CzerkaOff",
            "texa": "N_CzerkaOff",
            "race": "N_CzerkaOff",
        },
    },
    {
        "label": "Twilek_Commoner_M_02_A",
        "base": MALE_COMM_TEMPLATE,
        "overrides": {
            "normalhead": "llk_Ahrnell",
            "race": "N_CommM",
        },
    },
    {
        "label": "Twilek_Commoner_M_02_B",
        "base": MALE_COMM_TEMPLATE,
        "overrides": {
            "normalhead": "llk_Harra",
            "race": "N_CommM",
        },
    },
    {
        "label": "Twilek_Commoner_M_02_C",
        "base": MALE_COMM_TEMPLATE,
        "overrides": {
            "normalhead": "llk_KehVehn",
            "race": "N_CommM",
        },
    },
    {
        "label": "Twilek_Commoner_M_02_D",
        "base": MALE_COMM_TEMPLATE,
        "overrides": {
            "normalhead": "llk_LupoShar",
            "race": "N_CommM",
        },
    },
    # Dancer texa/texb follow the vanilla Twi'lek dancer rows (texb == texa there). N_TwiDance_Boot and N_TwiDance
    # both carry the N_TwiDanceA01 placeholder; vanilla rows 47/325 use A and 270 uses B. N_TwiUnd has no vanilla
    # row, so the underwear row keeps the human N_CommF texture set.
    {
        "label": "Twilek_Female_Dancer_Boot",
        "base": FEMALE_COMM_TEMPLATE,
        "overrides": {
            "modela": "N_TwiDance_Boot",
            "modelb": "N_TwiDance_Boot",
            "texa": "N_TwiDanceA",
            "texb": "N_TwiDanceA",
            "normalhead": "198",
        },
    },
    {
        "label": "Twilek_Female_Dancer",
        "base": FEMALE_COMM_TEMPLATE,
        "overrides": {
            "modela": "N_TwiDance",
            "modelb": "N_TwiDance",
            "texa": "N_TwiDanceB",
            "texb": "N_TwiDanceB",
            "normalhead": "200",
        },
    },
    {
        "label": "Twilek_Female_Underwear",
        "base": FEMALE_COMM_TEMPLATE,
        "overrides": {
            "modela": "N_TwiUnd",
            "modelb": "N_TwiUnd",
            "texa": "N_CommF",
            "normalhead": "204",
        },
    },
]


def load_or_create_2da(path: Path, default_headers: List[str]) -> TwoDA:
    if path.is_file():
        return read_2da(path)
    twoda = TwoDA()
    for h in default_headers:
        twoda.add_column(h)
    return twoda


def find_head_row(twoda: TwoDA, head_name: str) -> Optional[int]:
    head_lower = head_name.lower()
    for i in range(twoda.get_height()):
        row = twoda.get_row(i)
        if row.get_string("head").lower() == head_lower:
            return i
    return None


def find_appearance_row(twoda: TwoDA, label: str) -> Optional[int]:
    label_lower = label.lower()
    for i in range(twoda.get_height()):
        row = twoda.get_row(i)
        if row.get_string("label").lower() == label_lower:
            return i
    return None


def merge_heads_table(heads_2da: TwoDA) -> Tuple[Dict[str, int], List[str]]:
    mapping: Dict[str, int] = {}
    actions: List[str] = []

    for head_spec in NEW_HEADS:
        name = head_spec["head"]
        existing = find_head_row(heads_2da, name)
        if existing is not None:
            mapping[name] = existing
            actions.append(f"Head '{name}' already present at row {existing}")
        else:
            if heads_2da.get_height() + 1 > MAX_HEAD_ROWS:
                raise RuntimeError(
                    f"heads.2da would exceed the engine head-id limit ({MAX_HEAD_ROWS} rows; ids are one byte). "
                    f"Cannot add '{name}' at height {heads_2da.get_height()}."
                )
            idx = heads_2da.add_row()
            heads_2da.set_label(idx, str(idx))
            for h in heads_2da.get_headers():
                heads_2da.set_cell(idx, h, head_spec.get(h, ""))
            mapping[name] = idx
            actions.append(f"Appended head '{name}' at row {idx}")

    return mapping, actions


def merge_appearance_table(app_2da: TwoDA, head_mapping: Dict[str, int]) -> Tuple[Dict[str, int], List[str]]:
    mapping: Dict[str, int] = {}
    actions: List[str] = []
    headers = app_2da.get_headers()

    for spec in DIVERSITY_APPEARANCES:
        label = spec["label"]
        existing = find_appearance_row(app_2da, label)
        if existing is not None:
            mapping[label] = existing
            actions.append(f"Appearance '{label}' already present at row {existing}")
            continue

        if app_2da.get_height() >= 65535:
            raise RuntimeError(f"appearance.2da exceeds short limit (65536 rows)! Cannot add '{label}'.")

        idx = app_2da.add_row()
        app_2da.set_label(idx, str(idx))

        # Build full row values
        row_vals: Dict[str, str] = {}
        for h in headers:
            row_vals[h] = ""  # vanilla blank cells read back as '' (no literal "****")
        row_vals["label"] = label

        # Copy from base template
        for k, v in spec["base"].items():
            if k in headers:
                row_vals[k] = v

        # Apply specific overrides
        for k, v in spec["overrides"].items():
            if k in headers:
                row_vals[k] = v

        # Remap head if named in head_mapping
        normalhead_val = row_vals.get("normalhead", "")
        if normalhead_val in head_mapping:
            row_vals["normalhead"] = str(head_mapping[normalhead_val])

        # Write cells to table
        for h in headers:
            app_2da.set_cell(idx, h, row_vals[h])

        mapping[label] = idx
        actions.append(f"Appended appearance '{label}' at row {idx}")

    return mapping, actions


def _row_digest(twoda: TwoDA, labels: List[str], i: int) -> str:
    row = twoda.get_row(i)
    cells = [labels[i]] + [row.get_string(h) for h in twoda.get_headers()]
    return hashlib.sha256("\x1f".join(cells).encode("utf-8")).hexdigest()


def _index_of(twoda: TwoDA, column: str, value: str) -> Optional[int]:
    wanted = value.lower()
    for i in range(twoda.get_height()):
        if twoda.get_row(i).get_string(column).lower() == wanted:
            return i
    return None


def verify_base_unchanged(live_override: Path, backup_dir: Path) -> List[str]:
    """Check that the live tables still hold the pre-merge base. Returns problems; [] means safe to write.

    Rows 0..N-1 (N = rows in the backup) must match the backup cell by cell, row label included. Past N the live
    table may only hold this generator's own rows, so its first extra row must be the first row the generator
    would append: appearance Commoner_Fat_Male at N; heads the first NEW_HEADS entry missing from the backup.
    """
    problems: List[str] = []
    tables = (
        ("appearance.2da", [spec["label"] for spec in DIVERSITY_APPEARANCES], "label"),
        ("heads.2da", [spec["head"] for spec in NEW_HEADS], "head"),
    )
    for fname, wanted_names, key_col in tables:
        live_path = live_override / fname
        bak_path = backup_dir / (fname + ".bak")
        if not live_path.is_file() or not bak_path.is_file():
            problems.append(f"{fname}: live or backup file missing")
            continue
        live = read_2da(live_path)
        bak = read_2da(bak_path)
        if live.get_headers() != bak.get_headers():
            problems.append(f"{fname}: column headers differ from the pre-merge backup")
            continue
        n = bak.get_height()
        live_labels = live.get_labels()
        bak_labels = bak.get_labels()
        diff = [
            i for i in range(n)
            if i >= live.get_height() or _row_digest(live, live_labels, i) != _row_digest(bak, bak_labels, i)
        ]
        if diff:
            problems.append(
                f"{fname}: {len(diff)} of {n} base rows differ from the pre-merge backup; first 5 rows: {diff[:5]}"
            )
        first_new = next((name for name in wanted_names if _index_of(bak, key_col, name) is None), None)
        if live.get_height() > n:
            if first_new is None:
                problems.append(f"{fname}: {live.get_height() - n} row(s) past the backup's {n}, none expected")
            else:
                got = live.get_row(n).get_string(key_col)
                if got.lower() != first_new.lower():
                    problems.append(f"{fname}: row {n} is {got!r}, expected the first appended row {first_new!r}")
    return problems


def backup_once(src: Path, dst: Path) -> bool:
    """Copy src to dst only when dst does not exist yet, so the first pre-merge snapshot is never overwritten."""
    if dst.exists():
        return False
    shutil.copy2(src, dst)
    return True


def generate_cpp_header(
    app_mapping: Dict[str, int],
    head_mapping: Dict[str, int],
    out_path: Path,
    *,
    app_first: int,
    app_count: int,
    head_first: int,
    head_count: int,
) -> None:
    lines = [
        "// Auto-generated by scripts/generate_diversity_2da.py - DO NOT EDIT MANUALLY",
        "#pragma once",
        "",
        "#include <cstdint>",
        "",
        "namespace NpcDiversity {",
        "",
        "// Appended appearance.2da row indices",
    ]
    for label, row_idx in sorted(app_mapping.items()):
        const_name = "APP_" + label.upper().replace("-", "_")
        lines.append(f"constexpr uint16_t {const_name} = {row_idx};")

    lines.extend([
        "",
        "// Appended heads.2da row indices",
    ])
    for head_name, row_idx in sorted(head_mapping.items()):
        const_name = "HEAD_" + head_name.upper().replace("-", "_")
        lines.append(f"constexpr uint8_t {const_name} = {row_idx};")

    lines.extend([
        "",
        "// Table bounds: first row past the pre-merge base, and total row count after this generator run.",
        f"constexpr uint16_t APP_FIRST_APPENDED = {app_first};",
        f"constexpr uint16_t APP_COUNT = {app_count};",
        f"constexpr uint16_t HEAD_FIRST_APPENDED = {head_first};",
        f"constexpr uint16_t HEAD_COUNT = {head_count};",
        "static_assert(APP_COMMONER_FAT_MALE == APP_FIRST_APPENDED, "
        "\"Fat male commoner must be the first appended appearance row\");",
        "static_assert(APP_TWILEK_FEMALE_UNDERWEAR < APP_COUNT, "
        "\"Appended appearance rows must lie below APP_COUNT\");",
        "",
        "} // namespace NpcDiversity",
        "",
    ])

    out_path.write_text("\n".join(lines))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument(
        "--game-override",
        type=Path,
        default=Path(DEFAULT_GAME_OVERRIDE),
        help=f"Path to live game override directory (default: {DEFAULT_GAME_OVERRIDE})",
    )
    parser.add_argument(
        "--out-dir",
        type=Path,
        default=DEFAULT_OUT_DIR,
        help=f"Path to write staged 2DA files (default: {DEFAULT_OUT_DIR})",
    )
    parser.add_argument(
        "--src-dir",
        type=Path,
        default=DEFAULT_SRC_DIR,
        help=f"Path to mod source directory for C++ header (default: {DEFAULT_SRC_DIR})",
    )
    parser.add_argument(
        "--apply",
        action="store_true",
        help="Also write merged 2DAs directly into the live game override directory",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Simulate the merge without writing any files",
    )
    parser.add_argument(
        "--force-base",
        action="store_true",
        help="With --apply: write even when the live base differs from the pre-merge backup",
    )
    args = parser.parse_args()

    live_override = args.game_override
    out_dir = args.out_dir
    src_dir = args.src_dir

    backup_dir = live_override / PREMERGE_BACKUP_DIRNAME
    have_backup = (backup_dir / "heads.2da.bak").is_file() and (backup_dir / "appearance.2da.bak").is_file()

    if args.apply and have_backup:
        problems = verify_base_unchanged(live_override, backup_dir)
        for problem in problems:
            print(f"BASE CHECK: {problem}", file=sys.stderr)
        if problems and not args.force_base:
            print("Refusing to --apply: nothing written. Re-run with --force-base to override.", file=sys.stderr)
            sys.exit(3)
        if problems:
            print("WARNING: --force-base given; continuing despite the base mismatch.")
        else:
            print(f"Base check OK: live tables match the pre-merge backup in {backup_dir}")

    if have_backup:
        heads_src = backup_dir / "heads.2da.bak"
        app_src = backup_dir / "appearance.2da.bak"
        print(f"Base: pre-merge backup {backup_dir}")
    else:
        heads_src = live_override / "heads.2da"
        app_src = live_override / "appearance.2da"
        print(f"Base: live files in {live_override} (no pre-merge backup yet)")

    if not heads_src.is_file() or not app_src.is_file():
        print(f"Error: base heads.2da or appearance.2da not found ({heads_src.parent})", file=sys.stderr)
        sys.exit(1)

    heads_2da = read_2da(heads_src)
    app_2da = read_2da(app_src)

    orig_heads_count = heads_2da.get_height()
    orig_app_count = app_2da.get_height()
    print(f"Base heads.2da: {orig_heads_count} rows, {len(heads_2da.get_headers())} columns")
    print(f"Base appearance.2da: {orig_app_count} rows, {len(app_2da.get_headers())} columns")

    head_mapping, head_actions = merge_heads_table(heads_2da)
    app_mapping, app_actions = merge_appearance_table(app_2da, head_mapping)

    # Appended rows must start exactly at the base height (add_row appends, so this guards a changed base).
    new_app_rows = sorted(i for i in app_mapping.values() if i >= orig_app_count)
    if new_app_rows and new_app_rows[0] != orig_app_count:
        raise RuntimeError(f"appearance.2da: first appended row {new_app_rows[0]} != base height {orig_app_count}")
    new_head_rows = sorted(i for i in head_mapping.values() if i >= orig_heads_count)
    if new_head_rows and new_head_rows[0] != orig_heads_count:
        raise RuntimeError(f"heads.2da: first appended row {new_head_rows[0]} != base height {orig_heads_count}")

    print("\n--- Heads Summary ---")
    for act in head_actions:
        print("  " + act)

    print("\n--- Appearances Summary ---")
    for act in app_actions:
        print("  " + act)

    print(f"\nResulting heads.2da height: {heads_2da.get_height()} (was {orig_heads_count})")
    print(f"Resulting appearance.2da height: {app_2da.get_height()} (was {orig_app_count})")

    if args.dry_run:
        print("\n[Dry Run] No files written.")
        return

    # Create directories
    out_dir.mkdir(parents=True, exist_ok=True)
    src_dir.mkdir(parents=True, exist_ok=True)

    # Write staged files
    staged_heads = out_dir / "heads.2da"
    staged_app = out_dir / "appearance.2da"
    write_2da(heads_2da, staged_heads)
    write_2da(app_2da, staged_app)
    print(f"\nWrote staged 2DAs to {out_dir}")

    # Generate C++ header
    cpp_header = src_dir / "DiversityRows.h"
    generate_cpp_header(
        app_mapping,
        head_mapping,
        cpp_header,
        app_first=orig_app_count,
        app_count=app_2da.get_height(),
        head_first=orig_heads_count,
        head_count=heads_2da.get_height(),
    )
    print(f"Wrote C++ constants header to {cpp_header}")

    # Generate JSON mapping
    json_mapping = out_dir / "diversity_2da_mapping.json"
    mapping_data = {
        "appearances": app_mapping,
        "heads": head_mapping,
        "heads_count": heads_2da.get_height(),
        "appearance_count": app_2da.get_height(),
    }
    json_mapping.write_text(json.dumps(mapping_data, indent=2))
    print(f"Wrote mapping JSON to {json_mapping}")

    if args.apply:
        live_heads = live_override / "heads.2da"
        live_app = live_override / "appearance.2da"
        backup_dir.mkdir(parents=True, exist_ok=True)
        backed_up = [
            backup_once(live_heads, backup_dir / "heads.2da.bak"),
            backup_once(live_app, backup_dir / "appearance.2da.bak"),
        ]
        if any(backed_up):
            print(f"Backed up live 2DAs to {backup_dir}")
        else:
            print(f"Kept existing pre-merge backup in {backup_dir} (never overwritten)")

        shutil.copy2(staged_heads, live_heads)
        shutil.copy2(staged_app, live_app)
        print(f"Applied merged 2DAs to {live_override}")

    print("\nPhase 1 2DA generation completed successfully!")


if __name__ == "__main__":
    main()
