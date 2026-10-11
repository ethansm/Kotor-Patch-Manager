#!/usr/bin/env python3
"""Kept, hardlinked snapshots of KOTOR II's override/ and the ultrawide state.

Take one before every deploy stage, so any texture or GUI regression can be undone
file by file. Each run creates <dest>/<YYYYmmdd_HHMMSS>_<label>/ holding:
  override/        rsync -a of $GAME/override, with --link-dest=<previous snapshot>/override:
                   files unchanged since the previous snapshot are hardlinks *between
                   snapshots*. Nothing is ever linked to the live tree (the ultrawide tools
                   rewrite files in place, which would change a linked snapshot); a guard
                   checks that no snapshot file shares an inode with a live file.
  ultrawide_gui/   the state directory ($WORKSPACE/ultrawide_gui) without upscaled/ and
                   research/ (large and regenerable), linked the same way.
  swkotor2.ini
  SHA256SUMS       sha256 of every file above, sorted by path (`sha256sum -c` format).
  snapshot.json    label, time, git HEAD of this repo, manifest counts, previous snapshot.

The game must be closed: checked before the copy, and again after it; if the game
started meanwhile, snapshot.json gets "incomplete": true and the exit code is 1.
Nothing is ever deleted. `--verify SNAPSHOT` re-hashes a snapshot against its SHA256SUMS
(it reads only the snapshot, so it runs with the game open).

Restore one file:  cp <snap>/override/<file> "$GAME/override/"
Restore all (by hand, never automatic):
  rsync -a --delete <snap>/override/ "$GAME/override/"
  rsync -a --delete --exclude=/upscaled/ --exclude=/research/ <snap>/ultrawide_gui/ "$WORKSPACE/ultrawide_gui/"

Stdlib only, so it works even when the pykotor environment doesn't.

Usage:
  snapshot_live.py --label p3_S2_pre_pazaak [--game-dir G] [--state-dir U] [--dest D]
  snapshot_live.py --verify D/20260924_233000_p3_S2_pre_pazaak
"""

import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys

DEFAULT_GAME_DIR = os.environ.get(
    "KOTOR2_ROOT", os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II"))
DEFAULT_WORKSPACE_DIR = os.environ.get("KOTOR2_WORKSPACE", os.path.expanduser("~/Games/KOTOR2_Mod_Workspace"))
STATE_EXCLUDES = ("upscaled", "research")
META_FILES = ("SHA256SUMS", "snapshot.json")
LABEL_RE = re.compile(r"^[A-Za-z0-9_.-]+$")


def is_game_running() -> bool:
    """The same [s]wkotor2.exe match as generate_ultrawide_gui_overrides.is_game_running."""
    try:
        out = subprocess.check_output(["ps", "-eo", "pid,args"], text=True, stderr=subprocess.DEVNULL)
    except Exception:
        return False
    return any(re.search(r"[s]wkotor2\.exe", line, re.IGNORECASE) for line in out.splitlines())


def file_sha256(path: Path) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def snapshot_files(snap: Path) -> list[str]:
    """Every regular file in the snapshot (relative, sorted), without the meta files."""
    out = []
    for root, _dirs, files in os.walk(snap):
        for f in files:
            p = Path(root) / f
            rel = p.relative_to(snap).as_posix()
            if rel in META_FILES or p.is_symlink():
                continue
            out.append(rel)
    return sorted(out)


def read_sums(snap: Path) -> dict[str, str]:
    sums = {}
    path = snap / "SHA256SUMS"
    if path.exists():
        for line in path.read_text(encoding="utf-8").splitlines():
            digest, rel = line.split("  ", 1)
            sums[rel] = digest
    return sums


def previous_snapshot(dest: Path, name: str) -> Path | None:
    """The latest earlier snapshot in dest, by name (names start with the timestamp)."""
    if not dest.is_dir():
        return None
    earlier = sorted(p for p in dest.iterdir() if p.is_dir() and p.name < name and (p / "override").is_dir())
    return earlier[-1] if earlier else None


def rsync(src: Path, dst: Path, link_dest: Path | None, excludes=()) -> None:
    cmd = ["rsync", "-a"]
    cmd += [f"--exclude=/{e}/" for e in excludes]
    if link_dest is not None and link_dest.is_dir():
        cmd.append(f"--link-dest={link_dest.resolve()}")
    cmd += [f"{src}/", f"{dst}/"]
    subprocess.run(cmd, check=True)


def live_inodes(*trees: Path) -> set[tuple[int, int]]:
    inodes = set()
    for tree in trees:
        if not tree.is_dir():
            continue
        for root, _dirs, files in os.walk(tree):
            for f in files:
                st = os.lstat(os.path.join(root, f))
                inodes.add((st.st_dev, st.st_ino))
    return inodes


def check_no_live_links(snap: Path, live: set[tuple[int, int]]) -> None:
    """Refuse a snapshot any of whose files is the same inode as a live file."""
    for rel in snapshot_files(snap):
        st = os.lstat(snap / rel)
        if (st.st_dev, st.st_ino) in live:
            raise RuntimeError(f"{rel} is hardlinked to the live tree; an in-place write would change the snapshot")


def write_sums(snap: Path, prev: Path | None) -> int:
    """Hash every file; a file hardlinked to the previous snapshot reuses its recorded hash."""
    prev_sums = read_sums(prev) if prev else {}
    lines = []
    for rel in snapshot_files(snap):
        p = snap / rel
        digest = None
        if rel in prev_sums:
            q = prev / rel
            try:
                if os.path.samefile(p, q):
                    digest = prev_sums[rel]
            except OSError:
                pass
        lines.append(f"{digest or file_sha256(p)}  {rel}")
    (snap / "SHA256SUMS").write_text("\n".join(lines) + ("\n" if lines else ""), encoding="utf-8")
    return len(lines)


def manifest_count(path: Path) -> int | None:
    try:
        return len(json.loads(path.read_text(encoding="utf-8")))
    except (OSError, ValueError):
        return None


def git_head() -> str | None:
    try:
        return subprocess.check_output(["git", "-C", str(Path(__file__).resolve().parent), "rev-parse", "HEAD"],
                                       text=True, stderr=subprocess.DEVNULL).strip()
    except Exception:
        return None


def take_snapshot(label: str, game_dir: Path, state_dir: Path, dest: Path, now: datetime.datetime | None = None) -> tuple[Path, bool]:
    """Create one snapshot. Returns (path, complete). Raises if the game runs at the start."""
    if not LABEL_RE.match(label):
        raise ValueError(f"label {label!r}: use letters, digits, '_', '.', '-'")
    if is_game_running():
        raise RuntimeError("swkotor2.exe is running; close the game first")
    override = game_dir / "override"
    if not override.is_dir():
        raise FileNotFoundError(f"{override} not found")
    now = now or datetime.datetime.now()
    name = f"{now:%Y%m%d_%H%M%S}_{label}"
    snap = dest / name
    if snap.exists():
        raise FileExistsError(f"{snap} exists")
    prev = previous_snapshot(dest, name)
    snap.mkdir(parents=True)
    rsync(override, snap / "override", prev / "override" if prev else None)
    if state_dir.is_dir():
        rsync(state_dir, snap / "ultrawide_gui", prev / "ultrawide_gui" if prev else None, STATE_EXCLUDES)
    ini = game_dir / "swkotor2.ini"
    if ini.is_file():
        (snap / "swkotor2.ini").write_bytes(ini.read_bytes())
    complete = not is_game_running()
    check_no_live_links(snap, live_inodes(override, state_dir))
    count = write_sums(snap, prev)
    meta = {
        "label": label,
        "time": now.isoformat(timespec="seconds"),
        "git_head": git_head(),
        "game_dir": str(game_dir),
        "state_dir": str(state_dir),
        "manifest_gui_entries": manifest_count(state_dir / "manifest.json"),
        "manifest_tex_entries": manifest_count(state_dir / "manifest_tex.json"),
        "files": count,
        "previous": prev.name if prev else None,
    }
    if not complete:
        meta["incomplete"] = True
    (snap / "snapshot.json").write_text(json.dumps(meta, indent=2) + "\n", encoding="utf-8")
    return snap, complete


def verify_snapshot(snap: Path) -> list[str]:
    """Problems found re-hashing a snapshot against its SHA256SUMS (empty = clean)."""
    if not (snap / "SHA256SUMS").exists():
        return [f"{snap}: no SHA256SUMS"]
    sums = read_sums(snap)
    files = set(snapshot_files(snap))
    problems = [f"missing {rel}" for rel in sorted(set(sums) - files)]
    problems += [f"extra {rel}" for rel in sorted(files - set(sums))]
    for rel in sorted(set(sums) & files):
        if file_sha256(snap / rel) != sums[rel]:
            problems.append(f"changed {rel}")
    meta = snap / "snapshot.json"
    if meta.exists() and json.loads(meta.read_text(encoding="utf-8")).get("incomplete"):
        problems.append("snapshot.json marks it incomplete (the game started during the copy)")
    return problems


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="Kept, hardlinked snapshots of KOTOR II override/ and ultrawide state.")
    ap.add_argument("--label", help="Stage label, e.g. p3_S2_pre_pazaak.")
    ap.add_argument("--game-dir", default=DEFAULT_GAME_DIR)
    ap.add_argument("--state-dir", default=None, help="Default: $WORKSPACE/ultrawide_gui.")
    ap.add_argument("--dest", default=None, help="Default: $WORKSPACE/backups/ultrawide_snapshots.")
    ap.add_argument("--verify", metavar="SNAPSHOT", help="Re-hash a snapshot against its SHA256SUMS.")
    args = ap.parse_args(argv)
    if args.verify:
        problems = verify_snapshot(Path(args.verify))
        for p in problems:
            print(f"  {p}")
        print(f"{args.verify}: {'FAILED, ' + str(len(problems)) + ' problem(s)' if problems else 'OK'}")
        return 1 if problems else 0
    if not args.label:
        ap.error("--label or --verify is required")
    state_dir = Path(args.state_dir) if args.state_dir else Path(DEFAULT_WORKSPACE_DIR) / "ultrawide_gui"
    dest = Path(args.dest) if args.dest else Path(DEFAULT_WORKSPACE_DIR) / "backups" / "ultrawide_snapshots"
    try:
        snap, complete = take_snapshot(args.label, Path(args.game_dir), state_dir, dest)
    except (RuntimeError, ValueError, FileNotFoundError, FileExistsError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    meta = json.loads((snap / "snapshot.json").read_text(encoding="utf-8"))
    print(f"{snap}: {meta['files']} files, previous {meta['previous']}")
    if not complete:
        print("ERROR: the game started during the copy; snapshot marked incomplete", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
