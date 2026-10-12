#!/usr/bin/env python3
"""Tests for scripts/npc_add_install.py. Every test runs against a temp fake game dir and a temp source .mod.
The game-running probe is injected (running=...), so results never depend on whether the real game is open."""
import contextlib
import io
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import npc_add_install as nai  # noqa: E402

ORIGINAL = b"MOD V1.0" + b"original-204tel-bytes" * 64
INSTALLED = b"MOD V1.0" + b"npc-add-build-204tel-bytes" * 64


def not_running():
    return False


def running():
    return True


class Fixture(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        root = Path(self._tmp.name)
        self.game = root / "Knights of the Old Republic II"
        self.live = self.game / "Modules" / "204tel.mod"
        self.live.parent.mkdir(parents=True)
        self.live.write_bytes(ORIGINAL)
        self.source = root / "npc_add_src" / "out" / "modules" / "204tel.mod"
        self.source.parent.mkdir(parents=True)
        self.source.write_bytes(INSTALLED)

    def tearDown(self):
        self._tmp.cleanup()

    def run_main(self, *args, probe=not_running):
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            rc = nai.main(["--game-dir", str(self.game), "--source", str(self.source), *args], running=probe)
        return rc, out.getvalue()

    def tree(self):
        return {str(p.relative_to(self.game)): p.read_bytes() for p in sorted(self.game.rglob("*")) if p.is_file()}

    def backups(self):
        root = self.game / "backups"
        return sorted(p.name for p in root.iterdir()) if root.is_dir() else []

    def leftovers(self):
        return sorted(p.name for p in self.live.parent.iterdir() if p.name.endswith(".tmp"))


class DryRun(Fixture):
    def test_dry_run_writes_nothing(self):
        before = self.tree()
        rc, out = self.run_main()
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.tree(), before)
        self.assertFalse((self.game / "backups").exists())
        self.assertIn(nai.sha256_bytes(INSTALLED), out)
        self.assertIn(nai.sha256_bytes(ORIGINAL), out)
        self.assertIn("would apply", out)

    def test_dry_run_reports_running_game_and_exits_nonzero(self):
        before = self.tree()
        rc, out = self.run_main(probe=running)
        self.assertEqual(rc, 1)
        self.assertEqual(self.tree(), before)
        self.assertIn("game running  yes", out)
        self.assertIn("BLOCKED", out)

    def test_apply_and_rollback_are_exclusive(self):
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            self.run_main("--apply", "--rollback", "latest")


class Apply(Fixture):
    def test_apply_writes_backup_manifest_and_replacement(self):
        rc, out = self.run_main("--apply")
        self.assertEqual(rc, 0, out)
        self.assertEqual(len(self.backups()), 1)
        name = self.backups()[0]
        self.assertTrue(name.startswith("npc_add_"))
        bdir = self.game / "backups" / name
        self.assertEqual((bdir / "204tel.mod").read_bytes(), ORIGINAL)
        m = json.loads((bdir / "manifest.json").read_text())
        self.assertEqual(m["original_sha256"], nai.sha256_bytes(ORIGINAL))
        self.assertEqual(m["installed_sha256"], nai.sha256_bytes(INSTALLED))
        self.assertEqual(self.live.read_bytes(), INSTALLED)
        self.assertEqual(nai.sha256_file(self.live), m["installed_sha256"])
        self.assertEqual(self.leftovers(), [])
        self.assertEqual(self.source.read_bytes(), INSTALLED)

    def test_apply_refuses_while_game_runs(self):
        rc, out = self.run_main("--apply", probe=running)
        self.assertEqual(rc, 1)
        self.assertIn("running", out)
        self.assertEqual(self.live.read_bytes(), ORIGINAL)
        self.assertEqual(self.backups(), [])

    def test_apply_refuses_missing_source(self):
        self.source.unlink()
        rc, out = self.run_main("--apply")
        self.assertEqual(rc, 1)
        self.assertIn("missing source", out)
        self.assertEqual(self.live.read_bytes(), ORIGINAL)
        self.assertEqual(self.backups(), [])

    def test_apply_refuses_non_mod_source(self):
        self.source.write_bytes(b"JUNK" * 32)
        rc, out = self.run_main("--apply")
        self.assertEqual(rc, 1)
        self.assertIn("not an ERF/MOD", out)
        self.assertEqual(self.live.read_bytes(), ORIGINAL)

    def test_apply_refuses_missing_live_target(self):
        self.live.unlink()
        rc, out = self.run_main("--apply")
        self.assertEqual(rc, 1)
        self.assertIn("live target missing", out)
        self.assertEqual(self.backups(), [])

    def test_apply_is_noop_when_live_already_equals_source(self):
        self.live.write_bytes(INSTALLED)
        rc, out = self.run_main("--apply")
        self.assertEqual(rc, 0, out)
        self.assertIn("NO-OP", out)
        self.assertEqual(self.backups(), [])

    def test_second_apply_refused_until_rollback(self):
        self.assertEqual(self.run_main("--apply")[0], 0)
        rc, out = self.run_main("--apply")
        self.assertEqual(rc, 1)
        self.assertIn("already applied", out)
        self.assertEqual(self.live.read_bytes(), INSTALLED)
        self.assertEqual(len(self.backups()), 1)

    def test_apply_refuses_when_live_is_neither_original_nor_installed(self):
        self.assertEqual(self.run_main("--apply")[0], 0)
        self.live.write_bytes(b"edited by hand")
        rc, out = self.run_main("--apply")
        self.assertEqual(rc, 1)
        self.assertIn("neither the original nor the installed", out)
        self.assertEqual(self.live.read_bytes(), b"edited by hand")

    def test_failed_write_leaves_live_intact(self):
        with mock.patch.object(nai.os, "replace", side_effect=OSError("disk full")):
            rc, out = self.run_main("--apply")
        self.assertEqual(rc, 1)
        self.assertIn("apply failed", out)
        self.assertEqual(self.live.read_bytes(), ORIGINAL)
        self.assertEqual(self.backups(), [])
        self.assertEqual(self.leftovers(), [])

    def test_post_write_mismatch_is_undone(self):
        real_write = nai.atomic_write

        def corrupt_install_only(dst, data):
            real_write(dst, data + b"!" if data == INSTALLED else data)

        with mock.patch.object(nai, "atomic_write", side_effect=corrupt_install_only):
            rc, out = self.run_main("--apply")
        self.assertEqual(rc, 1)
        self.assertIn("restored to the original", out)
        self.assertEqual(self.live.read_bytes(), ORIGINAL)
        self.assertEqual(self.backups(), [])


class Rollback(Fixture):
    def test_rollback_latest_restores_original_sha(self):
        self.assertEqual(self.run_main("--apply")[0], 0)
        rc, out = self.run_main("--rollback", "latest")
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.live.read_bytes(), ORIGINAL)
        self.assertEqual(nai.sha256_file(self.live), nai.sha256_bytes(ORIGINAL))
        self.assertEqual(len(self.backups()), 1)

    def test_rollback_accepts_full_name_and_bare_stamp(self):
        self.assertEqual(self.run_main("--apply")[0], 0)
        full = self.backups()[0]
        self.assertEqual(self.run_main("--rollback", full)[0], 0)
        self.assertEqual(self.live.read_bytes(), ORIGINAL)
        self.live.write_bytes(INSTALLED)
        self.assertEqual(self.run_main("--rollback", full[len("npc_add_"):])[0], 0)
        self.assertEqual(self.live.read_bytes(), ORIGINAL)

    def test_rollback_refuses_on_hash_drift(self):
        self.assertEqual(self.run_main("--apply")[0], 0)
        self.live.write_bytes(b"edited by hand")
        rc, out = self.run_main("--rollback", "latest")
        self.assertEqual(rc, 1)
        self.assertIn("hash drift", out)
        self.assertEqual(self.live.read_bytes(), b"edited by hand")
        self.assertEqual(len(self.backups()), 1)

    def test_rollback_refuses_while_game_runs(self):
        self.assertEqual(self.run_main("--apply")[0], 0)
        rc, out = self.run_main("--rollback", "latest", probe=running)
        self.assertEqual(rc, 1)
        self.assertIn("running", out)
        self.assertEqual(self.live.read_bytes(), INSTALLED)

    def test_unknown_stamp_refused(self):
        self.assertEqual(self.run_main("--apply")[0], 0)
        installed = self.live.read_bytes()
        for bad in ("20990101_000000", "../../etc", "not-a-stamp"):
            rc, out = self.run_main("--rollback", bad)
            self.assertEqual(rc, 1, bad)
            self.assertIn("unknown stamp", out)
        self.assertEqual(self.live.read_bytes(), installed)

    def test_latest_without_any_backup_refused(self):
        rc, out = self.run_main("--rollback", "latest")
        self.assertEqual(rc, 1)
        self.assertIn("no complete", out)
        self.assertEqual(self.live.read_bytes(), ORIGINAL)

    def test_incomplete_backup_without_manifest_is_ignored(self):
        bdir = self.game / "backups" / "npc_add_20260101_000000"
        bdir.mkdir(parents=True)
        (bdir / "204tel.mod").write_bytes(ORIGINAL)
        rc, out = self.run_main("--rollback", "latest")
        self.assertEqual(rc, 1)
        self.assertIn("no complete", out)
        rc, out = self.run_main("--rollback", "20260101_000000")
        self.assertEqual(rc, 1)
        self.assertIn("unknown stamp", out)

    def test_corrupt_backup_refused(self):
        self.assertEqual(self.run_main("--apply")[0], 0)
        (self.game / "backups" / self.backups()[0] / "204tel.mod").write_bytes(b"rotten")
        rc, out = self.run_main("--rollback", "latest")
        self.assertEqual(rc, 1)
        self.assertIn("corrupt", out)
        self.assertEqual(self.live.read_bytes(), INSTALLED)

    def test_failed_restore_keeps_installed_build(self):
        self.assertEqual(self.run_main("--apply")[0], 0)
        with mock.patch.object(nai.os, "replace", side_effect=OSError("io error")):
            rc, out = self.run_main("--rollback", "latest")
        self.assertEqual(rc, 1)
        self.assertIn("rollback failed", out)
        self.assertEqual(self.live.read_bytes(), INSTALLED)
        self.assertEqual(self.leftovers(), [])


class Probe(unittest.TestCase):
    def test_missing_pgrep_fails_closed(self):
        with mock.patch.object(nai.subprocess, "run", side_effect=OSError("no pgrep")):
            self.assertTrue(nai.game_running())

    def test_pgrep_match_means_running(self):
        with mock.patch.object(nai.subprocess, "run", return_value=subprocess.CompletedProcess([], 0)):
            self.assertTrue(nai.game_running())
        with mock.patch.object(nai.subprocess, "run", return_value=subprocess.CompletedProcess([], 1)):
            self.assertFalse(nai.game_running())


if __name__ == "__main__":
    unittest.main()
