#!/usr/bin/env python3
"""
scripts/test_prune_test_cache.py - unit tests for scripts/prune_test_cache.py.

Usage:
  python3 scripts/test_prune_test_cache.py
"""

from __future__ import annotations

import io
import os
import sys
import tempfile
import time
import unittest
from contextlib import redirect_stdout
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import prune_test_cache as P  # noqa: E402

DAY = 86400


def age(path: Path, days: float) -> None:
    t = time.time() - days * DAY
    os.utime(path, (t, t), follow_symlinks=False)


def mk(path: Path, data: bytes = b"x" * 100, days: float = 0) -> Path:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)
    if days:
        age(path, days)
    return path


def dead_pid() -> int:
    pid = 4_000_000
    while P._pid_alive(pid):
        pid += 1
    return pid


class PruneTest(unittest.TestCase):
    def setUp(self):
        self._td = tempfile.TemporaryDirectory()
        self.addCleanup(self._td.cleanup)
        self.root = Path(self._td.name).resolve() / "kotor-tests"
        self.root.mkdir()

    def run_prune(self, **kw):
        with redirect_stdout(io.StringIO()) as out:
            res = P.prune(self.root, **kw)
        return res, out.getvalue()

    def test_old_exe_and_sidecar_deleted_fresh_kept(self):
        h = self.root / "host"
        old = mk(h / "a_src__t-aaaaaaaaaaaa", days=5)
        oldsc = mk(h / "a_src__t-aaaaaaaaaaaa.deps.json", days=5)
        tmpf = mk(h / "a_src__t-bbbbbbbbbbbb.tmp.123", days=5)
        legacy = mk(h / "action_hotbar_src__common_contract_test_h100_4", days=9)
        fresh = mk(h / "b_src__t-cccccccccccc")
        freshsc = mk(h / "b_src__t-cccccccccccc.deps.json")
        (n, size), _ = self.run_prune(apply=True)
        for p in (old, oldsc, tmpf, legacy):
            self.assertFalse(p.exists(), p)
        self.assertTrue(fresh.exists() and freshsc.exists())
        self.assertEqual(n, 4)
        self.assertEqual(size, 400)

    def test_orphan_sidecar_deleted(self):
        h = self.root / "host"
        orphan = mk(h / "gone__t-dddddddddddd.deps.json")
        keep = mk(h / "x__t-eeeeeeeeeeee")
        keepsc = mk(h / "x__t-eeeeeeeeeeee.deps.json")
        self.run_prune(apply=True)
        self.assertFalse(orphan.exists())
        self.assertTrue(keep.exists() and keepsc.exists())

    def test_sidecar_of_old_exe_deleted_even_if_fresh(self):
        h = self.root / "host"
        exe = mk(h / "x__t-ffffffffffff", days=5)
        sc = mk(h / "x__t-ffffffffffff.deps.json")
        self.run_prune(apply=True)
        self.assertFalse(exe.exists() or sc.exists())

    def test_run_dirs_dead_vs_live_pid(self):
        old_epoch = int(time.time()) - 3 * DAY
        dead = self.root / f"gate-{dead_pid()}-{old_epoch}"
        live = self.root / f"gate-{os.getpid()}-{old_epoch}"
        wdead = self.root / "wine" / f"run-{dead_pid()}-{old_epoch}"
        wlive = self.root / "wine" / f"run-{os.getpid()}-{old_epoch}"
        recent = self.root / f"gate-{dead_pid()}-{int(time.time())}"
        for d in (dead, live, wdead, wlive, recent):
            mk(d / "sub" / "f.txt")
        (n, _), _ = self.run_prune(apply=True)
        self.assertFalse(dead.exists())
        self.assertFalse(wdead.exists())
        self.assertTrue(live.exists() and wlive.exists() and recent.exists())
        self.assertEqual(n, 2)

    def test_protected_dirs_kept(self):
        for name in ("gate-records", "wineprefix", "common-cache", "locks", "build"):
            f = mk(self.root / name / "f.bin", days=30)
            age(f.parent, 30)
        # even an old-looking gate-records must survive
        self.run_prune(apply=True)
        for name in ("gate-records", "wineprefix", "common-cache", "locks", "build"):
            self.assertTrue((self.root / name / "f.bin").exists(), name)

    def test_legacy_fixed_dirs(self):
        oldd = self.root / "gate"
        mk(oldd / "a.txt", days=3)
        mk(oldd / "b.txt", days=2)
        age(oldd, 2)
        newd = self.root / "gate-selftest"
        mk(newd / "a.txt", days=3)
        mk(newd / "fresh.txt")
        age(newd, 3)
        self.run_prune(apply=True)
        self.assertFalse(oldd.exists())
        self.assertTrue(newd.exists())

    def test_dry_run_deletes_nothing(self):
        old = mk(self.root / "host" / "a-aaaaaaaaaaaa", days=5)
        rd = self.root / f"gate-{dead_pid()}-{int(time.time()) - 3 * DAY}"
        mk(rd / "f")
        (n, size), out = self.run_prune()
        self.assertTrue(old.exists() and rd.exists())
        self.assertEqual(n, 2)
        self.assertIn("WOULD DELETE", out)
        self.assertIn("2 files/dirs", out)

    def test_quiet_prints_summary_only(self):
        mk(self.root / "host" / "a-aaaaaaaaaaaa", days=5)
        _, out = self.run_prune(quiet=True)
        self.assertEqual(len(out.strip().splitlines()), 1)
        self.assertIn("1 files/dirs", out)

    def test_symlink_outside_root_not_followed(self):
        outside = Path(self._td.name).resolve() / "outside"
        victim = mk(outside / "keep.txt", days=30)
        age(outside, 30)
        h = self.root / "host"
        h.mkdir()
        link_file = h / "link_to_file-aaaaaaaaaaaa"
        link_file.symlink_to(victim)
        link_dir = h / "link_to_dir-bbbbbbbbbbbb"
        link_dir.symlink_to(outside, target_is_directory=True)
        rlink = self.root / f"gate-{dead_pid()}-{int(time.time()) - 3 * DAY}"
        rlink.symlink_to(outside, target_is_directory=True)
        for l in (link_file, link_dir):
            age(l, 5)
        self.run_prune(apply=True)
        self.assertFalse(os.path.lexists(link_file))
        self.assertFalse(os.path.lexists(link_dir))
        self.assertFalse(os.path.lexists(rlink))
        self.assertTrue(victim.exists())

    def test_symlink_inside_old_run_dir_not_followed(self):
        outside = Path(self._td.name).resolve() / "outside2"
        victim = mk(outside / "keep.txt")
        rd = self.root / f"gate-{dead_pid()}-{int(time.time()) - 3 * DAY}"
        rd.mkdir()
        (rd / "ln").symlink_to(outside, target_is_directory=True)
        self.run_prune(apply=True)
        self.assertFalse(rd.exists())
        self.assertTrue(victim.exists())

    def test_missing_root(self):
        self.assertEqual(P.prune(self.root / "nope", quiet=True), (0, 0))

    def test_inside_guard(self):
        self.assertFalse(P._inside(self.root, self.root))
        self.assertFalse(P._inside(self.root, self.root.parent / "x"))
        self.assertTrue(P._inside(self.root, self.root / "host" / "x"))


if __name__ == "__main__":
    unittest.main()
