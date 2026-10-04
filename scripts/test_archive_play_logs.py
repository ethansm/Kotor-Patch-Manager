#!/usr/bin/env python3
"""Tests for archive_play_logs.py (temp dirs only; never touches the real game/archive dirs)."""
import contextlib
import io
import os
import re
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import archive_play_logs as apl  # noqa: E402

MATCHING = {
    "perflab_log.txt": b"a" * 100,
    "kpatch.log": b"kpatch",
    "frame_metrics.txt": b"m" * 50,
    "heap_alloc.txt": b"z" * 10,
    "hotbar_log_2.txt": b"h",
}
NON_MATCHING = ["notes.txt", "swkotor2.exe", "kpatch.log.1", "readme.md", "log.dat", "Shield_LOG_x.txt"]  # last: glob is case-sensitive


def run_main(argv):
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        apl.main(argv)
    return buf.getvalue()


class ArchivePlayLogsTest(unittest.TestCase):
    def setUp(self):
        self._td = tempfile.TemporaryDirectory()
        self.addCleanup(self._td.cleanup)
        root = Path(self._td.name)
        self.game = root / "game"
        self.dest = root / "archive"
        self.game.mkdir()
        self.expected = dict(MATCHING)
        for name, data in self.expected.items():
            (self.game / name).write_bytes(data)
        for name in NON_MATCHING:
            (self.game / name).write_bytes(b"skip me")
        (self.game / "subdir_log.txt").mkdir()  # directory matching a glob must be skipped
        old = 1_000_000_000
        for p in self.game.iterdir():
            os.utime(p, (old, old))

    def snapshot(self):
        return {p.name: (p.read_bytes(), p.stat().st_mtime) for p in self.game.iterdir() if p.is_file()}

    def only_archive(self):
        subs = list(self.dest.iterdir())
        self.assertEqual(len(subs), 1)
        return subs[0]

    def test_copies_only_matching_files(self):
        run_main(["--game-dir", str(self.game), "--dest", str(self.dest)])
        copied = {p.name: p.read_bytes() for p in self.only_archive().iterdir()}
        self.assertEqual(copied, self.expected)

    def test_default_tag_and_folder_name(self):
        run_main(["--game-dir", str(self.game), "--dest", str(self.dest)])
        self.assertRegex(self.only_archive().name, r"^\d{8}_\d{6}_session$")

    def test_custom_tag_in_folder_name(self):
        run_main(["--game-dir", str(self.game), "--dest", str(self.dest), "--tag", "letterbox"])
        self.assertRegex(self.only_archive().name, r"^\d{8}_\d{6}_letterbox$")

    def test_empty_tag_gives_timestamp_only(self):
        with contextlib.redirect_stdout(io.StringIO()):
            dest = apl.archive_logs(str(self.game), str(self.dest), "")
        self.assertRegex(os.path.basename(dest), r"^\d{8}_\d{6}$")

    def test_dest_created_if_missing(self):
        nested = Path(self._td.name) / "a" / "b" / "c"
        run_main(["--game-dir", str(self.game), "--dest", str(nested)])
        self.assertTrue(nested.is_dir())
        self.assertEqual(len(list(nested.iterdir())), 1)

    def test_prints_count_mb_and_path(self):
        out = run_main(["--game-dir", str(self.game), "--dest", str(self.dest), "--tag", "t"])
        total = sum(len(v) for v in self.expected.values())
        self.assertIn(f"Archived {len(self.expected)} log files ({total / (1024 * 1024):.2f} MB) to:", out)
        self.assertIn(str(self.only_archive()), out)

    def test_return_value_is_dest_path(self):
        with contextlib.redirect_stdout(io.StringIO()):
            path = apl.archive_logs(str(self.game), str(self.dest), "x")
        self.assertEqual(Path(path), self.only_archive())

    def test_source_untouched(self):
        before = self.snapshot()
        run_main(["--game-dir", str(self.game), "--dest", str(self.dest)])
        self.assertEqual(self.snapshot(), before)

    def test_mtime_preserved_on_copy(self):
        run_main(["--game-dir", str(self.game), "--dest", str(self.dest)])
        for p in self.only_archive().iterdir():
            self.assertEqual(int(p.stat().st_mtime), 1_000_000_000)

    def test_no_matching_files_reports_zero(self):
        empty = Path(self._td.name) / "empty"
        empty.mkdir()
        (empty / "notes.txt").write_text("x")
        out = run_main(["--game-dir", str(empty), "--dest", str(self.dest)])
        self.assertIn("Archived 0 log files (0.00 MB)", out)
        self.assertEqual(list(self.only_archive().iterdir()), [])

    def test_missing_game_dir_archives_zero(self):
        out = run_main(["--game-dir", str(Path(self._td.name) / "nope"), "--dest", str(self.dest)])
        self.assertIn("Archived 0 log files", out)

    def test_help_exits_zero(self):
        with contextlib.redirect_stdout(io.StringIO()) as buf, self.assertRaises(SystemExit) as cm:
            apl.main(["--help"])
        self.assertEqual(cm.exception.code, 0)
        for flag in ("--tag", "--game-dir", "--dest"):
            self.assertIn(flag, buf.getvalue())

    def test_unknown_arg_exits_nonzero(self):
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as cm:
            apl.main(["--bogus"])
        self.assertEqual(cm.exception.code, 2)


if __name__ == "__main__":
    unittest.main()
