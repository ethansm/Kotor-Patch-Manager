#!/usr/bin/env python3
"""
install_echoes_compatch.py - Turnkey Master Installer for Echoes of the Republic Compatch

Automates the entire 7-phase compatch pipeline between Echoes of the Republic
and the 154-mod Full Community Mod Build for Star Wars: Knights of the Old Republic II.

Features:
- Works on any install: --game-dir (default: Steam on Linux, or $KOTOR2_ROOT) is
  checked here and passed to every phase as KOTOR2_ROOT, with --staging-dir and
  --map-src as ECHOES_STAGING and ECHOES_MAP_SRC (see _common.py).
- Optional automatic extraction of the Echoes of the Republic .zip archive.
- Refuses to run on a stock (unpatched) pykotor, which would blank 2DA tables
  in phase 2 (../pykotor_patches/README.md, patch #5). --skip-pykotor-check overrides.
- Refuses to run without nwnnsscomp.exe and Wine for phase 3 (--nwnnsscomp,
  $KOTOR2_NWNNSSCOMP, or Repair HK-47's copy in the workspace; $KOTOR2_PROTON_WINE).
  pykotor's built-in compiler swaps engine-call arguments; --allow-builtin-nss
  uses it anyway.
- Creates a tar backup of override/, Modules/ and dialog.tlk before modifying files.
- Sequentially executes Phases 1 through 6.
- Executes Phase 7 automated regression and sanity testing.
- --verify-only runs just that read-only audit.

Phase 2 refuses to run twice (it appends rows and strings), so a second full run
stops there with the game unchanged by it; restore the backup first to redo it.
"""

import argparse
import os
import shutil
import subprocess
import sys
import tarfile
import time
import zipfile
from datetime import datetime
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))
from _common import (  # noqa: E402
    ALLOW_BUILTIN_NSS, GAME_DIR, MAP_SRC_DIR, NWNNSSCOMP, PROTON_WINE, STAGING_DIR, WORKSPACE)

DEFAULT_GAME_DIR = GAME_DIR
DEFAULT_STAGING = STAGING_DIR
DEFAULT_MAP_SRC = MAP_SRC_DIR
DEFAULT_BACKUP_DIR = WORKSPACE / "backups"
PYKOTOR_README = SCRIPT_DIR.parent / "pykotor_patches" / "README.md"
MAP_SOURCES = ["a_galaxymap.nss", "k_sup_galaxymap.nss"]  # what phase 3 compiles

def log(msg: str, header: bool = False):
    if header:
        print("\n" + "=" * 65)
        print(f"  {msg}")
        print("=" * 65)
    else:
        print(f"[*] {msg}")

def check_environment(game_dir: Path):
    if not game_dir.exists():
        sys.exit(f"[ERROR] Game directory does not exist: {game_dir}")
    if not (game_dir / "dialog.tlk").exists():
        sys.exit(f"[ERROR] Invalid game directory (missing dialog.tlk): {game_dir}")
    if not (game_dir / "override").exists():
        sys.exit(f"[ERROR] Missing override/ directory in: {game_dir}")
    if not (game_dir / "Modules").exists():
        sys.exit(f"[ERROR] Missing Modules/ directory in: {game_dir}")
    log(f"Game directory verified: {game_dir}")

def zero_size_2da_labels():
    """Read back a 2DA whose cell-data size field is 0, as some shipped 2DAs have.
    Patched pykotor returns the labels; stock 2.3.12 returns blanks (patch #5)."""
    from pykotor.resource.formats.twoda import bytes_2da, read_2da
    from pykotor.resource.formats.twoda.twoda_data import TwoDA
    from pykotor.resource.type import ResourceType

    labels = ["Party_NPC_T3M3", "Party_NPC_Atton"]
    t = TwoDA()
    t.add_column("label")
    t.add_column("walkdist")
    for i, (label, walk) in enumerate(zip(labels, ["1.5", "2.1"])):
        t.add_row(str(i), {"label": label, "walkdist": walk})
    data = bytearray(bytes_2da(t, ResourceType.TwoDA))
    # layout ends: <uint16 cell offsets> <uint16 data size> <string blob>
    blob = sum(len(v) + 1 for v in {*labels, "1.5", "2.1"})
    pos = len(data) - blob - 2
    data[pos:pos + 2] = b"\0\0"
    back = read_2da(bytes(data))
    return [back.get_cell(i, "label") for i in range(len(labels))]


def check_pykotor():
    try:
        ok = zero_size_2da_labels() == ["Party_NPC_T3M3", "Party_NPC_Atton"]
        why = "its binary 2DA reader returns blank cells"
    except ImportError as e:
        ok, why = False, f"it can't be imported ({e})"
    if not ok:
        sys.exit(f"[ERROR] The pykotor in {sys.executable} is not the patched one: {why}. "
                 f"Phase 2 would write blanked 2DA tables into the game. Set up the patched "
                 f"venv as described in {PYKOTOR_README}, or pass --skip-pykotor-check.")
    log("pykotor is patched (zero-size 2DA reads back).")

def check_compiler(nwnnsscomp, allow_builtin: bool):
    """Phase 3 compiles with nwnnsscomp under Wine; stop before any phase if it can't."""
    if allow_builtin:
        log("WARNING: phase 3 may use pykotor's built-in NSS compiler (--allow-builtin-nss), "
            "which swaps engine-call arguments.")
        return
    problems = []
    if not nwnnsscomp or not Path(nwnnsscomp).is_file():
        problems.append(f"no nwnnsscomp.exe ({nwnnsscomp or 'none found'}): pass --nwnnsscomp or set "
                        "KOTOR2_NWNNSSCOMP to a TSLPatcher nwnnsscomp.exe (most mods ship one in tslpatchdata/)")
    if not Path(PROTON_WINE).is_file():
        problems.append(f"no Wine at {PROTON_WINE}: set KOTOR2_PROTON_WINE")
    if problems:
        sys.exit("[ERROR] Phase 3 can't compile the galaxy map scripts: " + "; ".join(problems)
                 + ". pykotor's built-in compiler swaps engine-call arguments; --allow-builtin-nss uses it anyway.")
    log(f"Phase 3 compiler: {nwnnsscomp} under {PROTON_WINE}")


def extract_archive(archive_path: Path, staging_dir: Path):
    log(f"Extracting {archive_path.name} to {staging_dir}...", header=True)
    staging_dir.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(archive_path, 'r') as zip_ref:
        total_files = len(zip_ref.infolist())
        log(f"Archive contains {total_files} files. Extracting...")
        zip_ref.extractall(staging_dir)
    log(f"Extraction complete: {staging_dir}")

def create_backup(game_dir: Path, backup_dir: Path) -> Path:
    log("Creating pre-compatch safety backup...", header=True)
    backup_dir.mkdir(parents=True, exist_ok=True)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    backup_file = backup_dir / f"live_pre_echoes_{timestamp}.tar"
    
    log(f"Writing archive: {backup_file}")
    with tarfile.open(backup_file, "w") as tar:
        for folder in ["override", "Modules"]:
            p = game_dir / folder
            if p.exists():
                log(f"  Archiving {folder}/...")
                tar.add(p, arcname=folder)
        for f in ["dialog.tlk", "swkotor2.ini"]:
            p = game_dir / f
            if p.exists():
                log(f"  Archiving {f}...")
                tar.add(p, arcname=f)
                
    size_mb = backup_file.stat().st_size / (1024 * 1024)
    log(f"Backup created successfully: {backup_file.name} ({size_mb:.1f} MB)")
    return backup_file

def phase_env(game_dir: Path, staging_dir: Path, map_src: Path, nwnnsscomp=None,
              allow_builtin: bool = False) -> dict:
    """The environment for a phase script: _common.py reads its paths from these."""
    env = {
        **os.environ,
        "KOTOR2_ROOT": str(game_dir),
        "ECHOES_STAGING": str(staging_dir),
        "ECHOES_MAP_SRC": str(map_src),
    }
    if nwnnsscomp:
        env["KOTOR2_NWNNSSCOMP"] = str(nwnnsscomp)
    if allow_builtin:
        env["KOTOR2_ALLOW_BUILTIN_NSS"] = "1"
    return env

def run_phase_script(script_name: str, desc: str, env: dict):
    log(f"Running {script_name} ({desc})...", header=True)
    script_path = SCRIPT_DIR / script_name
    if not script_path.exists():
        sys.exit(f"[ERROR] Missing phase script: {script_path}")
        
    start_time = time.time()
    res = subprocess.run([sys.executable, str(script_path)], env=env)
    elapsed = time.time() - start_time
    
    if res.returncode != 0:
        sys.exit(f"\n[ERROR] {script_name} failed with exit code {res.returncode}. Aborting pipeline.")
    log(f"{script_name} finished successfully in {elapsed:.2f}s")

def main():
    parser = argparse.ArgumentParser(
        description="Turnkey Installer & Verifier for Echoes of the Republic Compatch (KOTOR II 154-Mod Build)"
    )
    parser.add_argument(
        "--game-dir",
        type=Path,
        default=DEFAULT_GAME_DIR,
        help=f"Path to KOTOR II installation (default: $KOTOR2_ROOT or {DEFAULT_GAME_DIR})"
    )
    parser.add_argument(
        "--archive",
        type=Path,
        default=None,
        help="Path to KOTOR 2 Echoes of the Republic.zip (optional; will extract to staging if provided)"
    )
    parser.add_argument(
        "--staging-dir",
        type=Path,
        default=DEFAULT_STAGING,
        help=f"Path to extracted Echoes files (default: $ECHOES_STAGING or {DEFAULT_STAGING})"
    )
    parser.add_argument(
        "--map-src",
        type=Path,
        default=DEFAULT_MAP_SRC,
        help=f"Echoes' galaxy map .nss sources, for phase 3 (default: $ECHOES_MAP_SRC or {DEFAULT_MAP_SRC})"
    )
    parser.add_argument(
        "--backup-dir",
        type=Path,
        default=DEFAULT_BACKUP_DIR,
        help=f"Directory to store safety backups (default: {DEFAULT_BACKUP_DIR})"
    )
    parser.add_argument(
        "--no-backup",
        action="store_true",
        help="Skip creating the pre-compatch tarball backup"
    )
    parser.add_argument(
        "--verify-only",
        action="store_true",
        help="Only run the Phase 7 audit without applying file modifications"
    )
    parser.add_argument(
        "--nwnnsscomp",
        type=Path,
        default=None,
        help="TSLPatcher nwnnsscomp.exe for phase 3 (default: $KOTOR2_NWNNSSCOMP, else Repair HK-47's "
             "copy in the workspace)"
    )
    parser.add_argument(
        "--allow-builtin-nss",
        action="store_true",
        help="Let phase 3 use pykotor's built-in NSS compiler when nwnnsscomp/Wine is missing "
             "(it swaps engine-call arguments)"
    )
    parser.add_argument(
        "--skip-pykotor-check",
        action="store_true",
        help="Don't refuse to run when pykotor fails the patched-2DA-reader check"
    )
    
    args = parser.parse_args()
    
    print("\n" + "=" * 65)
    print("   ECHOES OF THE REPUBLIC - FULL COMPATCH AUTOMATION PIPELINE")
    print("   Target: 154-Mod Full Community Build (Linux / Proton)")
    print("=" * 65)
    
    check_environment(args.game_dir)
    nwnnsscomp = args.nwnnsscomp or NWNNSSCOMP
    allow_builtin = args.allow_builtin_nss or ALLOW_BUILTIN_NSS
    env = phase_env(args.game_dir, args.staging_dir, args.map_src, nwnnsscomp, allow_builtin)
    
    if args.verify_only:
        run_phase_script("verify_all_compatch.py", "Phase 7: End-to-End Audit & Verification", env)
        print("\n[SUCCESS] Verification complete.")
        return
        
    if args.archive and args.archive.exists():
        if not args.staging_dir.exists() or len(list(args.staging_dir.iterdir())) == 0:
            extract_archive(args.archive, args.staging_dir)
            
    if not args.staging_dir.exists():
        sys.exit(f"[ERROR] Staging directory not found: {args.staging_dir}. Provide --archive or extract files first.")
    missing = [n for n in MAP_SOURCES if not (args.map_src / n).is_file()]
    if missing:
        sys.exit(f"[ERROR] Galaxy map sources not found in {args.map_src}: {', '.join(missing)}. Pass --map-src.")

    if not args.skip_pykotor_check:
        check_pykotor()
    check_compiler(nwnnsscomp, allow_builtin)
        
    if not args.no_backup:
        create_backup(args.game_dir, args.backup_dir)
        
    pipeline_start = time.time()
    
    # Phase 1: Texture sanitization
    run_phase_script("01_sanitize_textures.py", "Phase 1: Texture Sanitization & Collision Dedup", env)
    
    # Phase 2: 2DA & TLK dynamic merge
    run_phase_script("02_merge_2da_and_tlk.py", "Phase 2: Dynamic 2DA Table & TLK String Merge", env)
    
    # Phase 3: Core scripting & galaxy map compile
    run_phase_script("03_compile_galaxy_map.py", "Phase 3: Galaxy Map Script Compilation & M4-78 Decoupling", env)
    
    # Phase 4: Surgical module merges
    run_phase_script("04_merge_modules.py", "Phase 4: Base Game Module Surgical Merges (7 Capsules)", env)
    
    # Phase 5: Companion DLG & Journal splice
    run_phase_script("05_merge_dlgs_and_jrl.py", "Phase 5: Companion Dialogue Splice & Journal/GUI Merge", env)
    
    # Phase 6: Final deployment
    run_phase_script("06_deploy_assets.py", "Phase 6: Asset Deployment (128 Modules, 12.7K Files)", env)
    
    # Phase 7: End-to-End verification audit
    run_phase_script("verify_all_compatch.py", "Phase 7: End-to-End Regression & Sanity Audit", env)
    
    total_elapsed = time.time() - pipeline_start
    
    print("\n" + "=" * 65)
    print("   COMPATCH PIPELINE COMPLETED SUCCESSFULLY!")
    print(f"   Total Pipeline Duration: {total_elapsed:.2f} seconds")
    print("   Your installation is 100% verified and ready to play.")
    print("=" * 65 + "\n")

if __name__ == "__main__":
    main()
