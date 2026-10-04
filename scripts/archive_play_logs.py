#!/usr/bin/env python3
"""Snapshot and archive all KOTOR II mod log files.

Creates a timestamped snapshot of all *.txt log files from the game folder
into ~/Games/KOTOR2_Mod_Workspace/logs_archive/<YYYYmmdd_HHMMSS>_<tag>/
Prevents logs from being overwritten or lost when line caps are reached.
"""

import argparse
import datetime
import glob
import os
import shutil
import sys

DEFAULT_GAME_DIR = os.path.expanduser(
    os.environ.get("KOTOR2_ROOT", "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II")
)
DEFAULT_ARCHIVE_DIR = os.path.expanduser(
    os.environ.get("KOTOR2_LOG_ARCHIVE", "~/Games/KOTOR2_Mod_Workspace/logs_archive")
)


def archive_logs(game_dir: str, archive_dir: str, tag: str) -> str:
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    dest_name = f"{timestamp}_{tag}" if tag else timestamp
    dest_path = os.path.join(archive_dir, dest_name)
    os.makedirs(dest_path, exist_ok=True)

    log_patterns = ["*log*.txt", "kpatch.log", "*metrics*.txt", "*alloc*.txt"]
    copied = 0
    total_bytes = 0

    found_files = set()
    for pattern in log_patterns:
        for f in glob.glob(os.path.join(game_dir, pattern)):
            found_files.add(f)

    for f in sorted(found_files):
        if os.path.isfile(f):
            dest_file = os.path.join(dest_path, os.path.basename(f))
            shutil.copy2(f, dest_file)
            size = os.path.getsize(f)
            total_bytes += size
            copied += 1

    print(f"Archived {copied} log files ({total_bytes / (1024 * 1024):.2f} MB) to:")
    print(f"  {dest_path}")
    return dest_path


def main(argv=None):
    parser = argparse.ArgumentParser(description="Archive KOTOR II mod log files")
    parser.add_argument("--tag", default="session", help="Tag label for the archive folder")
    parser.add_argument("--game-dir", default=DEFAULT_GAME_DIR, help="Path to game directory")
    parser.add_argument("--dest", default=DEFAULT_ARCHIVE_DIR, help="Path to archive destination")
    args = parser.parse_args(argv)

    archive_logs(args.game_dir, args.dest, args.tag)


if __name__ == "__main__":
    main()
