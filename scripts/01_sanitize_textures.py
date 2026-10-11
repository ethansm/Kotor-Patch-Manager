#!/usr/bin/env python3
"""Sanitize Echoes of the Republic textures in staging before merging into live override.

Resolves:
1. Internal TGA/TPC duplicates inside staging_echoes/Override/ (59 identified).
   Standard practice: delete duplicate .tpc so matching .tga/lightmap stays.
2. Cross-directory collisions between staging_echoes/Override/ and live override/ (35 identified):
   - 30 collisions where Live has high-res .tpc and Echoes has .tga: delete the low-res .tga from staging.
   - 5 collisions where Live has .tga and Echoes has .tpc: delete the .tpc from staging.
"""
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from _common import GAME_DIR, STAGING_DIR  # noqa: E402

STAGING_OVERRIDE = STAGING_DIR / "Override"
LIVE_OVERRIDE = GAME_DIR / "override"


def main():
    print("=== Step 1: Cleaning Internal Staging TGA/TPC Duplicates ===")
    staging_files = os.listdir(STAGING_OVERRIDE)
    staging_lower_map = {f.lower(): f for f in staging_files}

    staging_tga = {os.path.splitext(f)[0] for f in staging_lower_map if f.endswith(".tga")}
    staging_tpc = {os.path.splitext(f)[0] for f in staging_lower_map if f.endswith(".tpc")}

    internal_dupes = staging_tga & staging_tpc
    print(f"Found {len(internal_dupes)} internal duplicate basename(s) in staging.")
    deleted_internal = 0
    for base in sorted(internal_dupes):
        # Delete the .tpc side
        tpc_name = staging_lower_map[base + ".tpc"]
        target = os.path.join(STAGING_OVERRIDE, tpc_name)
        os.remove(target)
        deleted_internal += 1
    print(f"Removed {deleted_internal} internal duplicate .tpc file(s) from staging.")

    print("\n=== Step 2: Resolving Cross-Directory Collisions with Live Override ===")
    live_files = os.listdir(LIVE_OVERRIDE)
    live_lower_map = {f.lower(): f for f in live_files}
    live_tga = {os.path.splitext(f)[0] for f in live_lower_map if f.endswith(".tga")}
    live_tpc = {os.path.splitext(f)[0] for f in live_lower_map if f.endswith(".tpc")}

    # Refresh staging file list after step 1
    staging_files = os.listdir(STAGING_OVERRIDE)
    staging_lower_map = {f.lower(): f for f in staging_files}
    staging_tga = {os.path.splitext(f)[0] for f in staging_lower_map if f.endswith(".tga")}
    staging_tpc = {os.path.splitext(f)[0] for f in staging_lower_map if f.endswith(".tpc")}

    # Case 1: Live has .tpc, Staging has .tga
    c1 = live_tpc & staging_tga
    print(f"Live has .tpc, Staging has .tga: {len(c1)} collision(s). Removing .tga from staging...")
    for base in sorted(c1):
        target = os.path.join(STAGING_OVERRIDE, staging_lower_map[base + ".tga"])
        os.remove(target)
        print(f"   Deleted staging: {staging_lower_map[base + '.tga']}")

    # Case 2: Live has .tga, Staging has .tpc
    # Refresh staging
    staging_files = os.listdir(STAGING_OVERRIDE)
    staging_lower_map = {f.lower(): f for f in staging_files}
    staging_tpc = {os.path.splitext(f)[0] for f in staging_lower_map if f.endswith(".tpc")}
    c2 = live_tga & staging_tpc
    print(f"Live has .tga, Staging has .tpc: {len(c2)} collision(s). Removing .tpc from staging...")
    for base in sorted(c2):
        target = os.path.join(STAGING_OVERRIDE, staging_lower_map[base + ".tpc"])
        os.remove(target)
        print(f"   Deleted staging: {staging_lower_map[base + '.tpc']}")

    print("\n=== Verification ===")
    staging_files = os.listdir(STAGING_OVERRIDE)
    staging_lower = set(f.lower() for f in staging_files)
    st_tga = {os.path.splitext(f)[0] for f in staging_lower if f.endswith(".tga")}
    st_tpc = {os.path.splitext(f)[0] for f in staging_lower if f.endswith(".tpc")}

    # Verify no internal collisions remain
    remaining_internal = st_tga & st_tpc
    print(f"Remaining internal collisions in staging: {len(remaining_internal)}")

    # Verify no cross-directory collisions remain
    remaining_cross = (live_tpc & st_tga) | (live_tga & st_tpc)
    print(f"Remaining cross-directory collisions: {len(remaining_cross)}")
    assert len(remaining_internal) == 0, "Internal collisions still exist!"
    assert len(remaining_cross) == 0, "Cross-directory collisions still exist!"
    print("Texture sanitization 100% COMPLETE and VERIFIED!")


if __name__ == "__main__":
    main()
