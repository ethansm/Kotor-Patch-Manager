#!/usr/bin/env python3
"""
scripts/prune_test_cache.py - prune the KOTOR-II test scratch tree (TMP = $TMPDIR or /tmp/claude-<uid> + /kotor-tests).

Deletes stale test binaries under host/, dead per-run dirs (gate-<pid>-<epoch>, wine/run-<pid>-<epoch>)
old legacy gate dirs, and per-checkout run_tests build roots build-<8 hex> whose newest file is older than --days.
Never touches gate-records/, wineprefix/, common-cache/, locks/, build/.

Usage:
  python3 scripts/prune_test_cache.py                  # dry run: WOULD DELETE <path> <size> + summary
  python3 scripts/prune_test_cache.py --apply          # delete, print DELETED ... freed X.Y GB
  python3 scripts/prune_test_cache.py --days 3 --run-days 1
  python3 scripts/prune_test_cache.py --quiet          # summary line only
  python3 scripts/prune_test_cache.py --root DIR       # override TMP (tests); basename must be kotor-tests
  python3 scripts/prune_test_cache.py --root DIR --force-root   # allow another basename (tests only)

Exit status: 0 ok, 1 when any target was REFUSED or FAILED, 2 on a bad --root / argument.

Library: prune(root, days=3, run_days=1, apply=False, quiet=False, force_root=False) -> (count, bytes)
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
BUILD_ROOT_RE = re.compile(r"^build-[0-9a-f]{8}$")   # run_tests.py per-checkout build_check --keep roots
SIDECAR_SUFFIX = ".deps.json"
ROOT_NAME = "kotor-tests"
MIN_DAYS = 0.1
ORPHAN_GRACE = 3600  # an orphan sidecar must be at least this old (seconds) before it is deleted


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


def _newest(path: Path) -> float:
    """Newest mtime of a path; a symlink counts only as itself (never followed)."""
    st = os.lstat(path)
    if stat.S_ISDIR(st.st_mode):
        return _tree_stats(path)[1]
    return st.st_mtime


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


def _host_candidate(name: str) -> bool:
    """host/ holds test exes (`<dir>__<stem>...`), `.deps.json` sidecars and `.tmp.` files only."""
    return "__" in name or name.endswith(SIDECAR_SUFFIX) or ".tmp." in name


def _scandir(path: Path, warn) -> list:
    try:
        with os.scandir(path) as it:
            return list(it)
    except OSError as exc:
        warn(f"WARN cannot scan {path}: {exc}")
        return []


def _scan(root: Path, days: float, run_days: float, now: float, warn=lambda m: None) -> list[Path]:
    targets: list[Path] = []
    age_cut = now - days * 86400
    run_cut = now - run_days * 86400

    host = root / "host"
    if host.is_dir() and not host.is_symlink():
        entries = {}
        for e in _scandir(host, warn):
            try:
                st = os.lstat(e.path)
            except OSError:
                continue
            if stat.S_ISDIR(st.st_mode) or not _host_candidate(e.name):
                continue
            entries[e.name] = st.st_mtime
        # A sidecar follows its exe (a cache hit refreshes only the exe's mtime); only an orphan ages on its own.
        doomed = {n for n, m in entries.items() if m < age_cut and not n.endswith(SIDECAR_SUFFIX)}
        for n, m in entries.items():
            if n.endswith(SIDECAR_SUFFIX):
                exe = n[: -len(SIDECAR_SUFFIX)]
                if exe in doomed or (exe not in entries and m < now - ORPHAN_GRACE):
                    doomed.add(n)
        targets += [host / n for n in sorted(doomed)]

    for parent in (root, root / "wine"):
        if not parent.is_dir() or parent.is_symlink():
            continue
        want_prefix = "gate-" if parent == root else "run-"
        for e in sorted(_scandir(parent, warn), key=lambda x: x.name):
            if not e.name.startswith(want_prefix):
                continue
            m = RUN_DIR_RE.match(e.name)
            if not m:
                continue
            pid, epoch = int(m.group(1)), int(m.group(2))
            if epoch >= run_cut or _pid_alive(pid):
                continue
            try:
                newest = _newest(Path(e.path))
            except OSError as exc:
                warn(f"WARN cannot stat {e.path}: {exc}")
                continue
            if newest < run_cut:
                targets.append(Path(e.path))

    for name in LEGACY_DIRS:
        d = root / name
        try:
            if _newest(d) < run_cut:
                targets.append(d)
        except OSError:
            continue

    for e in sorted(_scandir(root, warn), key=lambda x: x.name):
        if not BUILD_ROOT_RE.match(e.name) or e.is_symlink() or not e.is_dir(follow_symlinks=False):
            continue
        try:
            if _newest(Path(e.path)) < age_cut:
                targets.append(Path(e.path))
        except OSError:
            continue
    return targets


def _delete(path: Path) -> None:
    st = os.lstat(path)
    if stat.S_ISDIR(st.st_mode):
        shutil.rmtree(path)
    else:
        os.unlink(path)


def _validate_root(root, force_root: bool) -> Path:
    if root is None or str(root) == "" or not os.path.isabs(str(root)):
        raise ValueError(f"root must be a non-empty absolute path, got {str(root)!r}")
    real = Path(root).resolve()
    if not force_root and real.name != ROOT_NAME:
        raise ValueError(f"root {real} is not named {ROOT_NAME!r} (use --force-root to override)")
    if real == Path(real.anchor):
        raise ValueError("refusing to prune the filesystem root")
    return real


def _prune(root, days, run_days, apply, quiet, force_root) -> tuple[int, int, int]:
    """Returns (count, bytes, errors); errors counts REFUSED and FAILED targets."""
    if days < MIN_DAYS or run_days < MIN_DAYS:
        raise ValueError(f"days and run_days must be >= {MIN_DAYS}")
    real = _validate_root(root, force_root)

    def warn(msg: str) -> None:
        if not quiet:
            print(msg, file=sys.stderr)

    count = freed = errors = 0
    if real.exists():
        for target in _scan(real, days, run_days, time.time(), warn):
            if not _inside(real, target):
                print(f"REFUSED {target} (outside root)", file=sys.stderr)
                errors += 1
                continue
            try:
                size = _entry_size(target)
            except OSError as exc:
                warn(f"WARN cannot stat {target}: {exc}")
                continue
            if apply:
                try:
                    _delete(target)
                except FileNotFoundError:
                    warn(f"WARN {target} vanished before delete")
                    continue
                except OSError as exc:
                    print(f"FAILED {target}: {exc}", file=sys.stderr)
                    errors += 1
                    continue
            count += 1
            freed += size
            if not quiet:
                print(f"{'DELETED' if apply else 'WOULD DELETE'} {target} {_human(size)}")
    if apply:
        print(f"DELETED {count} files/dirs, freed {_gb(freed)}")
    else:
        print(f"{count} files/dirs, {_gb(freed)}")
    return count, freed, errors


def prune(root, days=3, run_days=1, apply=False, quiet=False, force_root=False) -> tuple[int, int]:
    """Prune the scratch tree under root (basename must be kotor-tests unless force_root)."""
    count, freed, _errors = _prune(root, days, run_days, apply, quiet, force_root)
    return count, freed


def _min_days(text: str) -> float:
    v = float(text)
    if v < MIN_DAYS:
        raise argparse.ArgumentTypeError(f"must be >= {MIN_DAYS}")
    return v


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="Prune the KOTOR-II test scratch tree.")
    ap.add_argument("--apply", action="store_true", help="delete (default is a dry run)")
    ap.add_argument("--days", type=_min_days, default=3, help="host/ file max age in days (default 3, min 0.1)")
    ap.add_argument("--run-days", type=_min_days, default=1, help="run/legacy dir max age in days (default 1, min 0.1)")
    ap.add_argument("--root", default=str(TMP), help="override the scratch root (tests)")
    ap.add_argument("--force-root", action="store_true", help=f"allow a root not named {ROOT_NAME}")
    ap.add_argument("--quiet", action="store_true", help="print only the summary line")
    a = ap.parse_args(argv)
    try:
        _c, _b, errors = _prune(a.root, a.days, a.run_days, a.apply, a.quiet, a.force_root)
    except ValueError as exc:
        print(f"ERROR {exc}", file=sys.stderr)
        return 2
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
