#!/usr/bin/env python3
"""
scripts/prune_test_cache.py - prune the KOTOR-II test scratch tree (TMP = $TMPDIR or /tmp/claude-<uid> + /kotor-tests).

Deletes stale test binaries under host/, dead per-run dirs (gate-<pid>-<epoch>, wine/run-<pid>-<epoch>)
and old legacy gate dirs. Never touches gate-records/, wineprefix/, common-cache/, locks/, build/.

Usage:
  python3 scripts/prune_test_cache.py                  # dry run: WOULD DELETE <path> <size> + summary
  python3 scripts/prune_test_cache.py --apply          # delete, print DELETED ... freed X.Y GB
  python3 scripts/prune_test_cache.py --days 3 --run-days 1
  python3 scripts/prune_test_cache.py --quiet          # summary line only
  python3 scripts/prune_test_cache.py --root DIR       # override TMP (tests)

Library: prune(root, days=3, run_days=1, apply=False, quiet=False) -> (count, bytes)
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import stat
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from _load_limit import TMP  # noqa: E402

RUN_DIR_RE = re.compile(r"^(?:gate|run)-(\d+)-(\d+)$")
LEGACY_DIRS = ("gate", "gate-selftest")
SIDECAR_SUFFIX = ".deps.json"


def _human(n: int) -> str:
    if n < 1024:
        return f"{n}B"
    if n < 1024**2:
        return f"{n / 1024:.1f}KB"
    if n < 1024**3:
        return f"{n / 1024**2:.1f}MB"
    return f"{n / 1024**3:.1f}GB"


def _gb(n: int) -> str:
    return f"{n / 1024**3:.1f} GB"


def _pid_alive(pid: int) -> bool:
    try:
        os.kill(pid, 0)
    except ProcessLookupError:
        return False
    except PermissionError:
        return True
    except OverflowError:
        return False
    return True


def _tree_stats(path: Path) -> tuple[int, float]:
    """(total bytes, newest mtime) of a real dir tree, never following symlinks."""
    total = 0
    newest = 0.0
    try:
        newest = os.lstat(path).st_mtime
    except OSError:
        pass
    for dp, dns, fns in os.walk(path, followlinks=False):
        for name in dns + fns:
            try:
                st = os.lstat(os.path.join(dp, name))
            except OSError:
                continue
            total += st.st_size
            newest = max(newest, st.st_mtime)
    return total, newest


def _entry_size(path: Path) -> int:
    st = os.lstat(path)
    if stat.S_ISDIR(st.st_mode):
        return _tree_stats(path)[0]
    return st.st_size


def _inside(root: Path, target: Path) -> bool:
    """True when target (its parent resolved, the leaf NOT resolved) is strictly inside root."""
    try:
        real = target.parent.resolve() / target.name
        real.relative_to(root)
    except (ValueError, OSError):
        return False
    return real != root


def _scan(root: Path, days: float, run_days: float, now: float) -> list[Path]:
    targets: list[Path] = []
    age_cut = now - days * 86400
    run_cut = now - run_days * 86400

    host = root / "host"
    if host.is_dir() and not host.is_symlink():
        entries = {}
        for e in os.scandir(host):
            try:
                entries[e.name] = os.lstat(e.path).st_mtime
            except OSError:
                pass
        doomed = {n for n, m in entries.items() if m < age_cut}
        for n in entries:
            if n.endswith(SIDECAR_SUFFIX) and n not in doomed:
                exe = n[: -len(SIDECAR_SUFFIX)]
                if exe not in entries or exe in doomed:
                    doomed.add(n)
        targets += [host / n for n in sorted(doomed)]

    for parent in (root, root / "wine"):
        if not parent.is_dir() or parent.is_symlink():
            continue
        want_prefix = "gate-" if parent == root else "run-"
        for e in sorted(os.scandir(parent), key=lambda x: x.name):
            if not e.name.startswith(want_prefix):
                continue
            m = RUN_DIR_RE.match(e.name)
            if not m:
                continue
            pid, epoch = int(m.group(1)), int(m.group(2))
            if epoch < run_cut and not _pid_alive(pid):
                targets.append(Path(e.path))

    for name in LEGACY_DIRS:
        d = root / name
        try:
            st = os.lstat(d)
        except OSError:
            continue
        if stat.S_ISLNK(st.st_mode):
            if st.st_mtime < run_cut:
                targets.append(d)
        elif stat.S_ISDIR(st.st_mode):
            if _tree_stats(d)[1] < run_cut:
                targets.append(d)
    return targets


def _delete(path: Path) -> None:
    st = os.lstat(path)
    if stat.S_ISDIR(st.st_mode):
        shutil.rmtree(path)
    else:
        os.unlink(path)


def prune(root, days=3, run_days=1, apply=False, quiet=False) -> tuple[int, int]:
    root = Path(root)
    if not root.exists():
        if not quiet:
            print(f"root {root} does not exist")
        return 0, 0
    root = root.resolve()
    count = 0
    freed = 0
    for target in _scan(root, days, run_days, time.time()):
        if not _inside(root, target):
            print(f"REFUSED {target} (outside root)", file=sys.stderr)
            continue
        try:
            size = _entry_size(target)
        except OSError:
            continue
        verb = "DELETED" if apply else "WOULD DELETE"
        if apply:
            try:
                _delete(target)
            except OSError as exc:
                print(f"FAILED {target}: {exc}", file=sys.stderr)
                continue
        count += 1
        freed += size
        if not quiet:
            print(f"{verb} {target} {_human(size)}")
    if apply:
        print(f"DELETED {count} files/dirs, freed {_gb(freed)}")
    else:
        print(f"{count} files/dirs, {_gb(freed)}")
    return count, freed


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="Prune the KOTOR-II test scratch tree.")
    ap.add_argument("--apply", action="store_true", help="delete (default is a dry run)")
    ap.add_argument("--days", type=float, default=3, help="host/ file max age in days (default 3)")
    ap.add_argument("--run-days", type=float, default=1, help="run/legacy dir max age in days (default 1)")
    ap.add_argument("--root", default=str(TMP), help="override the scratch root (tests)")
    ap.add_argument("--quiet", action="store_true", help="print only the summary line")
    a = ap.parse_args(argv)
    prune(a.root, days=a.days, run_days=a.run_days, apply=a.apply, quiet=a.quiet)
    return 0


if __name__ == "__main__":
    sys.exit(main())
