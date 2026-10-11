#!/usr/bin/env python3
"""
kotor2_toolset.py - Unified KOTOR II Mod Management, Verification, & Compatch CLI

Provides an all-in-one command suite for managing, verifying, and patching
Star Wars: Knights of the Old Republic II on Linux / Proton.

Commands:
  status           Show installation health, module counts, and configuration summary
  verify           Run comprehensive 2DA table, texture collision, animation TXI, and compatch audits
  compatch-echoes  Run the turnkey automated Echoes of the Republic compatch pipeline
  dedup-textures   Scan and resolve conflicting .tga/.tpc texture pairs in override
  backup           Create an immutable timestamped backup of the current game state
  restore          Extract a backup tarball over the installation (doesn't delete files added since)
"""

import argparse
import os
import shutil
import subprocess
import sys
import tarfile
from datetime import datetime
from pathlib import Path

DEFAULT_GAME_DIR = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
DEFAULT_WORKSPACE = Path.home() / "Games/KOTOR2_Mod_Workspace"
DEFAULT_BACKUP_DIR = DEFAULT_WORKSPACE / "backups"

SCRIPT_DIR = Path(__file__).resolve().parent

def cmd_status(args):
    game_dir = args.game_dir
    print("\n" + "=" * 60)
    print("   KOTOR II INSTALLATION STATUS REPORT")
    print("=" * 60)
    print(f"Game Directory: {game_dir}")
    if not game_dir.exists():
        print("[!] Directory does not exist.")
        return
        
    ovr = game_dir / "override"
    mods = game_dir / "Modules"
    tlk = game_dir / "dialog.tlk"
    ini = game_dir / "swkotor2.ini"
    
    ovr_count = len(list(ovr.iterdir())) if ovr.exists() else 0
    # Real files only: Modules/ also holds case-alias symlinks (symlink_pass.py) to the same capsules
    capsules = [p for p in mods.iterdir() if p.is_file() and not p.is_symlink()] if mods.exists() else []
    mod_count = sum(p.suffix.lower() == ".mod" for p in capsules)
    rim_count = sum(p.suffix.lower() == ".rim" for p in capsules)
    tlk_size = tlk.stat().st_size if tlk.exists() else 0
    
    print(f"Override Files:    {ovr_count:,}")
    print(f"Module Capsules:   {mod_count} .mod files, {rim_count} .rim files")
    print(f"dialog.tlk Size:   {tlk_size:,} bytes")
    
    if ini.exists():
        with open(ini, 'r') as f:
            lines = [l.strip() for l in f if 'width=' in l.lower() or 'height=' in l.lower() or 'v-sync=' in l.lower()]
        print(f"Resolution Config: {', '.join(lines[:4])}")
        
    print("=" * 60 + "\n")

def cmd_verify(args):
    print("\n[*] Running comprehensive verification audit...")
    ovr = str(args.game_dir / "override")
    # each exits non-zero on a finding; none of them changes anything
    checks = [
        ("2DA blank-prefix corruption", [SCRIPT_DIR / "verify_2da.py", ovr]),
        ("case-variant duplicates / OS junk", [SCRIPT_DIR / "case_dedup.py", ovr, "--dry-run"]),
        ("scripts with swapped call arguments (pykotor built-in compiler)",
         [SCRIPT_DIR / "check_ncs_arg_order.py", args.game_dir]),
        ("animation TXIs cycling a texture that isn't a flipbook",
         [SCRIPT_DIR / "check_txi_cycles.py", ovr, "--texturepacks", args.game_dir / "TexturePacks"]),
    ]
    verify_script = SCRIPT_DIR / "echoes_compatch" / "verify_all_compatch.py"
    if not verify_script.exists():
        verify_script = SCRIPT_DIR / "verify_all_compatch.py"
    if verify_script.exists():
        checks.append(("Echoes compatch", [verify_script]))
    else:
        print("[!] verify_all_compatch.py not found in script directory.")
    failed = []
    env = {**os.environ, "KOTOR2_ROOT": str(args.game_dir)}  # the Echoes verifier reads the game dir from it
    for name, cmd in checks:
        print(f"\n[*] {name}")
        if subprocess.run([sys.executable, *map(str, cmd)], env=env).returncode != 0:
            failed.append(name)
    print(f"\n[*] .tga/.dds + .tpc pairs")
    sys.path.insert(0, str(SCRIPT_DIR))
    from dedup_tga_tpc import dedup_override
    pairs = dedup_override(ovr, dry_run=True)
    print(f"    {len(pairs)} .tpc file(s) would be deleted by dedup-textures --fix")
    if pairs:
        failed.append(".tga/.tpc pairs")
    print(f"\n[{'!' if failed else 'OK'}] {'FAILED: ' + ', '.join(failed) if failed else 'all checks passed'}")
    return 1 if failed else 0

def cmd_compatch_echoes(args):
    installer = SCRIPT_DIR / "echoes_compatch" / "install_echoes_compatch.py"
    if not installer.exists():
        installer = SCRIPT_DIR / "install_echoes_compatch.py"
    if not installer.exists():
        sys.exit("[!] install_echoes_compatch.py not found in script directory.")
    cmd = [sys.executable, str(installer), "--game-dir", str(args.game_dir)]
    if args.archive:
        cmd.extend(["--archive", str(args.archive)])
    if args.no_backup:
        cmd.append("--no-backup")
    if args.verify_only:
        cmd.append("--verify-only")
    return subprocess.run(cmd).returncode

def cmd_dedup_textures(args):
    # Same rule as dedup_tga_tpc.py (delete the .tpc of each .tga/.dds + .tpc pair), by
    # calling it: a separate implementation here once grouped by basename alone, so two
    # case spellings of one .tpc ("TEL_SFtr.tpc", "tel_sftr.tpc") counted as a pair and
    # --fix deleted both.
    sys.path.insert(0, str(SCRIPT_DIR))
    from dedup_tga_tpc import dedup_override
    ovr = args.game_dir / "override"
    if not ovr.exists():
        sys.exit("[!] override directory not found.")
    print(f"[*] Scanning {ovr} for .tpc files that share a basename with a .tga/.dds...")
    found = dedup_override(str(ovr), dry_run=not args.fix)
    verb = "Deleted" if args.fix else "Would delete (pass --fix to delete)"
    print(f"[*] {verb} {len(found)} .tpc file(s).")
    for name in found:
        print(f"  {name}")

def cmd_backup(args):
    game_dir = args.game_dir
    backup_dir = args.backup_dir
    backup_dir.mkdir(parents=True, exist_ok=True)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    tar_path = backup_dir / f"live_kotor2_backup_{timestamp}.tar"
    print(f"[*] Creating backup: {tar_path}...")
    with tarfile.open(tar_path, "w") as tar:
        for folder in ["override", "Modules", "StreamMusic", "StreamVoice", "Movies"]:
            p = game_dir / folder
            if p.exists():
                print(f"  Archiving {folder}...")
                tar.add(p, arcname=folder)
        for f in ["dialog.tlk", "swkotor2.ini"]:
            p = game_dir / f
            if p.exists():
                print(f"  Archiving {f}...")
                tar.add(p, arcname=f)
    print(f"[SUCCESS] Backup saved: {tar_path} ({tar_path.stat().st_size / (1024*1024):.1f} MB)\n")

def cmd_restore(args):
    tar_path = Path(args.tarball)
    if not tar_path.exists():
        sys.exit(f"[ERROR] Backup tarball not found: {tar_path}")
    game_dir = args.game_dir
    print(f"[!] Restoring {game_dir} from {tar_path}...")
    with tarfile.open(tar_path, "r") as tar:
        # "data" rejects absolute paths and links pointing outside game_dir
        tar.extractall(game_dir, filter="data")
    print(f"[SUCCESS] Restored {game_dir} from backup. Files added after the backup were not removed.")

def main():
    parser = argparse.ArgumentParser(
        description="Unified KOTOR II Mod Management, Verification, & Compatch Toolset"
    )
    parser.add_argument(
        "--game-dir",
        type=Path,
        default=DEFAULT_GAME_DIR,
        help=f"Path to KOTOR II installation (default: {DEFAULT_GAME_DIR})"
    )
    subparsers = parser.add_subparsers(dest="command", required=True)
    
    # status
    subparsers.add_parser("status", help="Show active installation metrics and config")
    
    # verify
    subparsers.add_parser("verify", help="Run full 2DA, texture, and compatch regression audit")
    
    # compatch-echoes
    p_comp = subparsers.add_parser("compatch-echoes", help="Run turnkey Echoes of the Republic compatch")
    p_comp.add_argument("--archive", type=Path, default=None, help="Path to Echoes .zip archive")
    p_comp.add_argument("--no-backup", action="store_true", help="Skip pre-flight backup")
    p_comp.add_argument("--verify-only", action="store_true", help="Only run audit checks")
    
    # dedup-textures
    p_dedup = subparsers.add_parser("dedup-textures", help="Scan/fix TGA/TPC duplicate collisions")
    p_dedup.add_argument("--fix", action="store_true", help="Automatically remove colliding low-priority textures")
    
    # backup
    p_bkp = subparsers.add_parser("backup", help="Create a timestamped tarball backup")
    p_bkp.add_argument("--backup-dir", type=Path, default=DEFAULT_BACKUP_DIR, help="Backup storage directory")
    
    # restore
    p_rst = subparsers.add_parser(
        "restore",
        help="Extract a backup tarball over the installation (overwrites; doesn't delete files added since)")
    p_rst.add_argument("tarball", type=str, help="Path to .tar backup file")
    
    args = parser.parse_args()
    
    dispatch = {
        "status": cmd_status,
        "verify": cmd_verify,
        "compatch-echoes": cmd_compatch_echoes,
        "dedup-textures": cmd_dedup_textures,
        "backup": cmd_backup,
        "restore": cmd_restore,
    }
    
    sys.exit(dispatch[args.command](args) or 0)

if __name__ == "__main__":
    main()
