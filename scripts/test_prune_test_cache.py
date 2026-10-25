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
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import prune_test_cache as P  # noqa: E402

DAY = 86400


def age(path: Path, days: float) -> None:
    t = time.time() - days * DAY
    os.utime(path, (t, t), follow_symlinks=False)


def age_tree(path: Path, days: float) -> None:
    """Age a real dir tree bottom-up (files first, then dirs)."""
    for dp, dns, fns in os.walk(path, topdown=False):
        for n in fns + dns:
            age(Path(dp) / n, days)
    age(path, days)


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


def old_run_dir(path: Path, days: float = 3) -> Path:
    mk(path / "sub" / "f.txt")
    age_tree(path, days)
    return path


class PruneTest(unittest.TestCase):
    def setUp(self):
        self._td = tempfile.TemporaryDirectory()
        self.addCleanup(self._td.cleanup)
        self.base = Path(self._td.name).resolve()
        self.root = self.base / "kotor-tests"
        self.root.mkdir()

    def run_prune(self, **kw):
        with redirect_stdout(io.StringIO()) as out, redirect_stderr(io.StringIO()) as err:
            res = P.prune(self.root, **kw)
        self.err = err.getvalue()
        return res, out.getvalue()

    def run_main(self, *args):
        with redirect_stdout(io.StringIO()) as out, redirect_stderr(io.StringIO()) as err:
            rc = P.main(list(args))
        return rc, out.getvalue(), err.getvalue()

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

    def test_orphan_sidecar_deleted_when_aged(self):
        h = self.root / "host"
        orphan = mk(h / "gone__t-dddddddddddd.deps.json", days=0.5)
        keep = mk(h / "x__t-eeeeeeeeeeee")
        keepsc = mk(h / "x__t-eeeeeeeeeeee.deps.json", days=0.5)
        self.run_prune(apply=True)
        self.assertFalse(orphan.exists())
        self.assertTrue(keep.exists() and keepsc.exists())

    def test_fresh_orphan_sidecar_kept(self):
        orphan = mk(self.root / "host" / "gone__t-dddddddddddd.deps.json")
        (n, _), _ = self.run_prune(apply=True)
        self.assertTrue(orphan.exists())
        self.assertEqual(n, 0)

    def test_sidecar_follows_its_exe(self):
        h = self.root / "host"
        exe = mk(h / "x__t-ffffffffffff", days=5)
        sc = mk(h / "x__t-ffffffffffff.deps.json")       # fresh sidecar, old exe: both go
        hit_exe = mk(h / "y__t-111111111111")            # cache hit refreshed only the exe
        hit_sc = mk(h / "y__t-111111111111.deps.json", days=5)
        self.run_prune(apply=True)
        self.assertFalse(exe.exists() or sc.exists())
        self.assertTrue(hit_exe.exists() and hit_sc.exists())

    def test_old_checkout_build_root_deleted_fresh_kept_build_untouched(self):
        old = mk(self.root / "build-0123abcd" / "k" / "x.dll", days=5).parent.parent
        fresh = mk(self.root / "build-89abcdef" / "k" / "x.dll").parent.parent
        shared = mk(self.root / "build" / "k" / "x.dll", days=30).parent.parent
        os.utime(old, (time.time() - 5 * 86400,) * 2)
        os.utime(old / "k", (time.time() - 5 * 86400,) * 2)
        self.run_prune(apply=True)
        self.assertFalse(old.exists())
        self.assertTrue(fresh.exists() and shared.exists())

    def test_fresh_tmp_file_kept(self):
        h = self.root / "host"
        fresh = mk(h / "a_src__t-aaaaaaaaaaaa.tmp.999")
        old = mk(h / "a_src__t-aaaaaaaaaaaa.tmp.1", days=5)
        self.run_prune(apply=True)
        self.assertTrue(fresh.exists())
        self.assertFalse(old.exists())

    def test_host_dirs_and_unrelated_names_skipped(self):
        h = self.root / "host"
        d = mk(h / "somedir__x" / "f", days=9).parent
        age(d, 9)
        other = mk(h / "notes.txt", days=9)
        (n, _), _ = self.run_prune(apply=True)
        self.assertTrue(d.exists() and other.exists())
        self.assertEqual(n, 0)

    def test_run_dirs_dead_vs_live_pid(self):
        old_epoch = int(time.time()) - 3 * DAY
        dead = old_run_dir(self.root / f"gate-{dead_pid()}-{old_epoch}")
        live = old_run_dir(self.root / f"gate-{os.getpid()}-{old_epoch}")
        wdead = old_run_dir(self.root / "wine" / f"run-{dead_pid()}-{old_epoch}")
        wlive = old_run_dir(self.root / "wine" / f"run-{os.getpid()}-{old_epoch}")
        recent = self.root / f"gate-{dead_pid()}-{int(time.time())}"
        old_run_dir(recent)
        (n, _), _ = self.run_prune(apply=True)
        self.assertFalse(dead.exists())
        self.assertFalse(wdead.exists())
        self.assertTrue(live.exists() and wlive.exists() and recent.exists())
        self.assertEqual(n, 2)

    def test_dead_old_run_dir_with_fresh_content_kept(self):
        old_epoch = int(time.time()) - 3 * DAY
        rd = old_run_dir(self.root / f"gate-{dead_pid()}-{old_epoch}")
        fresh = mk(rd / "sub" / "new.log")
        (n, _), _ = self.run_prune(apply=True)
        self.assertTrue(rd.exists() and fresh.exists())
        self.assertEqual(n, 0)

    def test_protected_dirs_kept(self):
        names = ("gate-records", "wineprefix", "common-cache", "locks", "build")
        for name in names:
            mk(self.root / name / "f.bin")
            age_tree(self.root / name, 30)
        self.run_prune(apply=True)
        for name in names:
            self.assertTrue((self.root / name / "f.bin").exists(), name)

    def test_legacy_fixed_dirs(self):
        oldd = self.root / "gate"
        mk(oldd / "a.txt")
        mk(oldd / "b.txt")
        age_tree(oldd, 2)
        newd = self.root / "gate-selftest"
        mk(newd / "a.txt", days=3)
        mk(newd / "fresh.txt")
        age(newd, 3)
        self.run_prune(apply=True)
        self.assertFalse(oldd.exists())
        self.assertTrue(newd.exists())

    def test_dry_run_deletes_nothing(self):
        old = mk(self.root / "host" / "a__b-aaaaaaaaaaaa", days=5)
        rd = old_run_dir(self.root / f"gate-{dead_pid()}-{int(time.time()) - 3 * DAY}")
        (n, size), out = self.run_prune()
        self.assertTrue(old.exists() and rd.exists())
        self.assertEqual(n, 2)
        self.assertIn("WOULD DELETE", out)
        self.assertIn("2 files/dirs", out)

    def test_quiet_prints_summary_only(self):
        mk(self.root / "host" / "a__b-aaaaaaaaaaaa", days=5)
        _, out = self.run_prune(quiet=True)
        self.assertEqual(len(out.strip().splitlines()), 1)
        self.assertIn("1 files/dirs", out)

    def test_symlink_outside_root_not_followed(self):
        outside = self.base / "outside"
        victim = mk(outside / "keep.txt", days=30)
        age(outside, 30)
        h = self.root / "host"
        h.mkdir()
        link_file = h / "link_to_file__a-aaaaaaaaaaaa"
        link_file.symlink_to(victim)
        link_dir = h / "link_to_dir__b-bbbbbbbbbbbb"
        link_dir.symlink_to(outside, target_is_directory=True)
        rlink = self.root / f"gate-{dead_pid()}-{int(time.time()) - 3 * DAY}"
        rlink.symlink_to(outside, target_is_directory=True)
        for l in (link_file, link_dir, rlink):
            age(l, 5)
        self.run_prune(apply=True)
        self.assertFalse(os.path.lexists(link_file))
        self.assertFalse(os.path.lexists(link_dir))
        self.assertFalse(os.path.lexists(rlink))
        self.assertTrue(victim.exists())

    def test_symlink_inside_old_run_dir_not_followed(self):
        outside = self.base / "outside2"
        victim = mk(outside / "keep.txt")
        rd = self.root / f"gate-{dead_pid()}-{int(time.time()) - 3 * DAY}"
        rd.mkdir()
        (rd / "ln").symlink_to(outside, target_is_directory=True)
        age_tree(rd, 3)
        self.run_prune(apply=True)
        self.assertFalse(rd.exists())
        self.assertTrue(victim.exists())

    def test_symlinked_host_skipped(self):
        real = self.base / "realhost"
        victim = mk(real / "a__t-aaaaaaaaaaaa", days=9)
        (self.root / "host").symlink_to(real, target_is_directory=True)
        (n, _), _ = self.run_prune(apply=True)
        self.assertTrue(victim.exists())
        self.assertEqual(n, 0)

    def test_symlinked_wine_skipped(self):
        real = self.base / "realwine"
        rd = old_run_dir(real / f"run-{dead_pid()}-{int(time.time()) - 3 * DAY}")
        (self.root / "wine").symlink_to(real, target_is_directory=True)
        (n, _), _ = self.run_prune(apply=True)
        self.assertTrue(rd.exists())
        self.assertEqual(n, 0)

    def test_missing_root_prints_summary_even_quiet(self):
        with redirect_stdout(io.StringIO()) as out:
            res = P.prune(self.base / "kotor-tests-nope" / "kotor-tests", quiet=True)
        self.assertEqual(res, (0, 0))
        self.assertEqual(out.getvalue().strip(), "0 files/dirs, 0.0 GB")

    def test_inside_guard(self):
        self.assertFalse(P._inside(self.root, self.root))
        self.assertFalse(P._inside(self.root, self.root.parent / "x"))
        self.assertTrue(P._inside(self.root, self.root / "host" / "x"))
        link = self.root / "lnk"
        link.symlink_to(self.base, target_is_directory=True)
        self.assertFalse(P._inside(self.root, link / "x"))

    # --- error handling ---

    @unittest.skipIf(os.getuid() == 0, "chmod 000 does not block root")
    def test_unreadable_scan_dir_does_not_abort(self):
        h = self.root / "host"
        mk(h / "a__t-aaaaaaaaaaaa", days=9)
        old_run_dir(self.root / "wine" / f"run-{dead_pid()}-{int(time.time()) - 3 * DAY}")
        h.chmod(0)
        self.addCleanup(h.chmod, 0o755)
        (n, _), _ = self.run_prune(apply=True)
        self.assertEqual(n, 1)  # wine run dir still pruned
        self.assertIn("WARN cannot scan", self.err)

    def test_removed_scan_dir_does_not_abort(self):
        (self.root / "host").mkdir()
        (self.root / "wine").mkdir()
        real = os.scandir

        def boom(path):
            if str(path).endswith(("host", "wine")):
                raise FileNotFoundError(path)
            return real(path)

        with mock.patch.object(P.os, "scandir", side_effect=boom):
            res, _ = self.run_prune(apply=True)
        self.assertEqual(res, (0, 0))
        self.assertIn("WARN cannot scan", self.err)

    def test_file_not_found_mid_delete_skipped(self):
        gone = mk(self.root / "host" / "a__t-aaaaaaaaaaaa", days=9)
        other = mk(self.root / "host" / "b__t-bbbbbbbbbbbb", days=9)
        real = P._delete

        def flaky(path):
            if path.name.startswith("a__"):
                raise FileNotFoundError(path)
            real(path)

        with mock.patch.object(P, "_delete", side_effect=flaky):
            (n, _), _ = self.run_prune(apply=True)
        self.assertEqual(n, 1)
        self.assertFalse(other.exists())
        self.assertTrue(gone.exists())
        self.assertIn("vanished", self.err)

    def test_delete_failure_returns_exit_1(self):
        mk(self.root / "host" / "a__t-aaaaaaaaaaaa", days=9)
        with mock.patch.object(P, "_delete", side_effect=PermissionError("nope")):
            rc, out, err = self.run_main("--root", str(self.root), "--apply", "--quiet")
        self.assertEqual(rc, 1)
        self.assertIn("FAILED", err)
        self.assertIn("0 files/dirs", out)

    def test_refused_target_returns_exit_1(self):
        mk(self.root / "host" / "a__t-aaaaaaaaaaaa", days=9)
        with mock.patch.object(P, "_inside", return_value=False):
            rc, out, err = self.run_main("--root", str(self.root), "--apply", "--quiet")
        self.assertEqual(rc, 1)
        self.assertIn("REFUSED", err)
        self.assertTrue((self.root / "host" / "a__t-aaaaaaaaaaaa").exists())

    # --- root and argument validation ---

    def test_root_must_be_named_kotor_tests(self):
        other = self.base / "something"
        other.mkdir()
        victim = mk(other / "host" / "a__t-aaaaaaaaaaaa", days=9)
        with self.assertRaises(ValueError):
            P.prune(other, apply=True)
        self.assertTrue(victim.exists())
        rc, _, err = self.run_main("--root", str(other), "--apply")
        self.assertEqual(rc, 2)
        self.assertTrue(victim.exists())
        rc, _, _ = self.run_main("--root", str(other), "--force-root", "--apply", "--quiet")
        self.assertEqual(rc, 0)
        self.assertFalse(victim.exists())

    def test_root_validation_on_resolved_path(self):
        other = self.base / "something2"
        other.mkdir()
        link = self.base / "kotor-tests-link"
        link.symlink_to(other, target_is_directory=True)
        with self.assertRaises(ValueError):
            P.prune(link)
        alias = self.base / "kotor-tests-alias" / "kotor-tests"
        alias.parent.mkdir()
        alias.symlink_to(self.root, target_is_directory=True)
        with redirect_stdout(io.StringIO()):
            self.assertEqual(P.prune(alias, quiet=True)[0], 0)

    def test_empty_relative_and_fs_roots_rejected(self):
        for bad in ("", "kotor-tests", "../kotor-tests", "/"):
            with self.assertRaises(ValueError, msg=bad):
                P.prune(bad, force_root=(bad == "/"))
            rc, _, _ = self.run_main("--root", bad)
            self.assertEqual(rc, 2, bad)

    def test_days_minimum(self):
        for flag in ("--days", "--run-days"):
            for val in ("0", "-1", "0.05"):
                with redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
                    P.main(["--root", str(self.root), flag, val])
        with self.assertRaises(ValueError):
            P.prune(self.root, days=0)
        with self.assertRaises(ValueError):
            P.prune(self.root, run_days=-1)
        rc, _, _ = self.run_main("--root", str(self.root), "--days", "0.1", "--quiet")
        self.assertEqual(rc, 0)


if __name__ == "__main__":
    unittest.main()
