#!/usr/bin/env python3
"""Normalize NPC diversity commoner textures and models into sequential slots.

Maps verified raw mod textures into standard KotOR engine texture slots:
  - N_CommM01 through N_CommM35 (.tpc/.tga)
  - N_CommF01 through N_CommF35 (.tpc/.tga)

Also ingests supporting model assets:
  - DarthParametric Fat Commoner Body Models v1.1.0 (N_FatComM / N_FatComF / N_CommM_Fat / N_CommF_Fat)
  - Ashton Scorpius fixed mouth-cube TSL heads (pfhc01, pfhc03, pmhc04, pmhh01, n_brnmodulh, n_daraalah)
  - Leilukin / Ashton Scorpius Twi'lek male heads (llk_ahrnell, llk_harra, llk_kehvehn, llk_luposhar)
  - Lily female Twi'lek models (N_TwiDance_Boot, N_TwiDance, N_TwiUnd, etc.)
  - Female Czerka Officer body model (N_CzerkaOff)

Usage:
  python3 scripts/normalize_diversity_textures.py [--out-dir DIR] [--dry-run]
"""

from __future__ import annotations

import argparse
import os
import shutil
import sys
from typing import Dict, List, Tuple

# Default search directories for raw assets
DEFAULT_SEARCH_DIRS = [
    os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/01_extracted/nexus_mod144_uco_redux"),
    os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/01_extracted/mod149_character_textures_2x"),
    os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/01_extracted/extra_fashion_line_b"),
    os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/01_extracted/mod41_twilek_diversity"),
    os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/01_extracted/extra_hd_female_twileks"),
    os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/01_extracted/mod37_head_fixes"),
]

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_OUT_DIR = os.path.join(REPO_ROOT, "patch_manager_mods", "npc_diversity_src", "override")

MALE_SLOTS: List[str] = [
    "n_commm01", "n_commm02", "n_commm03", "n_commm04", "n_commm05",
    "n_commm06", "n_commm07", "n_commm08", "n_commm09", "n_commm10", "n_commm11",
    "n_commmb01", "n_commmb02", "n_commmb03", "n_commmb04", "n_commmd01",
    "n_comm_a_m", "n_comm_b_m", "n_comm_b_m02", "n_comm_b_m_a", "n_comm_b_m_m", "n_comm_b_m_w",
    "n_comm_w_m", "n_comm_w_m02", "n_comm_w_m2",
    "n_tatcoma_m", "n_tatcomw_m",
    "n_ondcomm01", "n_ondcomm02", "n_ondcomm03",
    "n_sithcom01", "n_sithcom02", "n_sithcom03",
    "n_fatcomm01", "n_sithcomm01"
]

FEMALE_SLOTS: List[str] = [
    "n_commf01", "n_commf02", "n_commf03", "n_commf04", "n_commf05",
    "n_commf06", "n_commf07", "n_commf08", "n_commf09", "n_commf10", "n_commf11",
    "n_commfb01", "n_commfb02", "n_commfb03", "n_commfb04",
    "n_commfc01", "n_commfd01",
    "comm_a_f", "comm_a_f02", "comm_a_f_b", "comm_a_f_m", "comm_a_f_w", "n_comm_a_f",
    "comm_b_f01", "comm_b_f02",
    "comm_w_f01", "comm_w_f02", "comm_w_f_a", "comm_w_f_b", "comm_w_f_m",
    "n_tatcoma_f", "n_tatcomw_f",
    "n_ondcomf01", "n_sithcomf01", "n_fatcomf01"
]

# Additional models to ingest for complete Phase 0 verification
MODEL_INGEST_MAP: Dict[str, List[str]] = {
    # Fat Commoner Body Models v1.1.0 (DarthParametric)
    "n_fatcomm.mdl": ["N_FatComM.mdl", "N_CommM_Fat.mdl"],
    "n_fatcomm.mdx": ["N_FatComM.mdx", "N_CommM_Fat.mdx"],
    "n_fatcomf.mdl": ["N_FatComF.mdl", "N_CommF_Fat.mdl"],
    "n_fatcomf.mdx": ["N_FatComF.mdx", "N_CommF_Fat.mdx"],
    # Czerka Officer Female
    "n_czerkaoff.mdl": ["N_CzerkaOff.mdl"],
    "n_czerkaoff.mdx": ["N_CzerkaOff.mdx"],
    "n_czerkaoff01.tpc": ["N_CzerkaOff01.tpc"],
    "n_czerkaoffc01.tpc": ["N_CzerkaOffC01.tpc"],
    # Ashton Scorpius fixed mouth-cube TSL heads
    "pfhc01.mdl": ["pfhc01.mdl"],
    "pfhc01.mdx": ["pfhc01.mdx"],
    "pfhc03.mdx": ["pfhc03.mdx"],
    "pmhc04.mdx": ["pmhc04.mdx"],
    "pmhh01.mdx": ["pmhh01.mdx"],
    "n_brnmodulh.mdx": ["n_brnmodulh.mdx"],
    "n_daraalah.mdx": ["n_daraalah.mdx"],
    # Leilukin / Ashton Scorpius Twi'lek male heads
    "llk_ahrnell.mdl": ["llk_ahrnell.mdl"],
    "llk_ahrnell.tpc": ["llk_ahrnell.tpc"],
    "llk_harra.mdl": ["llk_harra.mdl"],
    "llk_harra.tpc": ["llk_harra.tpc"],
    "llk_kehvehn.mdl": ["llk_kehvehn.mdl"],
    "llk_kehvehn.tpc": ["llk_kehvehn.tpc"],
    "llk_luposhar.mdl": ["llk_luposhar.mdl"],
    "llk_luposhar.tpc": ["llk_luposhar.tpc"],
    # Lily female Twi'lek models
    "n_twidance_boot.mdl": ["N_TwiDance_Boot.mdl"],
    "n_twidance_boot.mdx": ["N_TwiDance_Boot.mdx"],
    "n_twidance.mdl": ["N_TwiDance.mdl"],
    "n_twidance.mdx": ["N_TwiDance.mdx"],
    "n_twiund.mdl": ["N_TwiUnd.mdl"],
    "n_twiund.mdx": ["N_TwiUnd.mdx"],
}


def build_catalog(search_dirs: List[str]) -> Dict[str, str]:
    """Scan search directories and map lowercased basename to file path.
    Prefers .tpc over .tga when both exist for textures.
    """
    catalog: Dict[str, str] = {}
    for directory in search_dirs:
        if not os.path.isdir(directory):
            continue
        for root, dirs, files in os.walk(directory):
            dirs.sort()
            for f in sorted(files):
                lkey = f.lower()
                base, ext = os.path.splitext(lkey)
                # Store full key (e.g. n_fatcomm.mdl)
                if lkey not in catalog:
                    catalog[lkey] = os.path.join(root, f)

                # Store texture base key (e.g. n_commm01) preferring .tpc
                if ext in (".tpc", ".tga"):
                    if base not in catalog or ext == ".tpc":
                        catalog[base] = os.path.join(root, f)
    return catalog


def normalize_assets(
    out_dir: str,
    search_dirs: List[str],
    dry_run: bool = False
) -> Tuple[int, int, int]:
    """Normalize textures and copy supporting models into out_dir."""
    catalog = build_catalog(search_dirs)
    os.makedirs(out_dir, exist_ok=True)

    copied_males = 0
    copied_females = 0
    copied_models = 0

    # 1. Male textures (N_CommM01..35)
    for idx, key in enumerate(MALE_SLOTS, start=1):
        if key not in catalog:
            raise FileNotFoundError(f"Missing source texture for male slot {idx:02d} ({key})")
        src_path = catalog[key]
        _, ext = os.path.splitext(src_path)
        dest_name = f"N_CommM{idx:02d}{ext.lower()}"
        dest_path = os.path.join(out_dir, dest_name)
        if not dry_run:
            shutil.copy2(src_path, dest_path)
        copied_males += 1

    # 2. Female textures (N_CommF01..35)
    for idx, key in enumerate(FEMALE_SLOTS, start=1):
        if key not in catalog:
            raise FileNotFoundError(f"Missing source texture for female slot {idx:02d} ({key})")
        src_path = catalog[key]
        _, ext = os.path.splitext(src_path)
        dest_name = f"N_CommF{idx:02d}{ext.lower()}"
        dest_path = os.path.join(out_dir, dest_name)
        if not dry_run:
            shutil.copy2(src_path, dest_path)
        copied_females += 1

    # 3. Supporting models and heads
    for src_key, dest_names in MODEL_INGEST_MAP.items():
        if src_key not in catalog:
            print(f"Warning: optional asset {src_key} not found in catalog, skipping.")
            continue
        src_path = catalog[src_key]
        for dest_name in dest_names:
            dest_path = os.path.join(out_dir, dest_name)
            if not dry_run:
                shutil.copy2(src_path, dest_path)
            copied_models += 1

    return copied_males, copied_females, copied_models


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--out-dir",
        default=DEFAULT_OUT_DIR,
        help=f"Destination directory (default: {DEFAULT_OUT_DIR})"
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Simulate file copies without writing to disk"
    )
    args = parser.parse_args()

    print(f"=== NPC Diversity Asset Normalizer ===")
    print(f"Target Directory: {args.out_dir}")
    print(f"Search Roots: {len(DEFAULT_SEARCH_DIRS)} directories")

    try:
        males, females, models = normalize_assets(
            out_dir=args.out_dir,
            search_dirs=DEFAULT_SEARCH_DIRS,
            dry_run=args.dry_run
        )
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 1

    print("\n--- Summary ---")
    print(f"  Male Commoner Textures:   {males:02d}/35 (N_CommM01..35)")
    print(f"  Female Commoner Textures: {females:02d}/35 (N_CommF01..35)")
    print(f"  Supporting Models/Heads:  {models}")
    print(f"  Total Assets Ingested:    {males + females + models}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
