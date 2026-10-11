#!/usr/bin/env python3
"""Tests for scripts/wave_budget.py (fixtures in temp dirs; one smoke test on the repo CLAUDE.md)."""
import os
import re
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import wave_budget as W  # noqa: E402

SCRIPT = HERE / "wave_budget.py"
REPO_CLAUDE = HERE.parent / "CLAUDE.md"
WINDOW_RE = re.compile(r"^(.*):(\d+)-(\d+)$")
FIVE_LINES = "".join(f"l{i}\n" for i in range(1, 6))  # 3 bytes per line


class Fixture(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.root = Path(tmp.name)

    def write(self, name, data):
        path = self.root / name
        path.write_bytes(data if isinstance(data, bytes) else data.encode())
        return str(path)

    def run_cli(self, *args, env=None):
        full_env = dict(os.environ)
        full_env.pop("WAVE_OVERHEAD_TOKENS", None)
        if env:
            full_env.update(env)
        return subprocess.run([sys.executable, "-I", str(SCRIPT), *args],
                              capture_output=True, text=True, env=full_env)


class EstimateEntryTests(Fixture):
    def test_prose_suffix_uses_4_bytes_per_token(self):
        for name in ("a.md", "a.TXT"):
            with self.subTest(name=name):
                self.assertEqual(W.estimate_entry(self.write(name, "x" * 10)), 3)  # ceil(10/4)

    def test_other_suffix_uses_3_5_bytes_per_token(self):
        self.assertEqual(W.estimate_entry(self.write("a.py", "x" * 8)), 3)  # ceil(8/3.5); prose would be 2
        self.assertEqual(W.estimate_entry(self.write("b.py", "x" * 7)), 2)  # exactly 2

    def test_empty_file_is_zero(self):
        p = self.write("e.py", b"")
        self.assertEqual(W.estimate_entry(p), 0)
        self.assertEqual(W.estimate_entry(f"{p}:1-3"), 0)

    def test_line_range_counts_only_selected_lines(self):
        p = self.write("l.txt", FIVE_LINES)
        self.assertEqual(W.estimate_entry(f"{p}:2-3"), 2)  # 6 bytes -> ceil(6/4)
        self.assertEqual(W.estimate_entry(f"{p}:1-1"), 1)  # 3 bytes -> 1

    def test_hi_clamps_to_eof(self):
        p = self.write("c.txt", FIVE_LINES)
        self.assertEqual(W.estimate_entry(f"{p}:3-100"), W.estimate_entry(f"{p}:3-5"))
        self.assertEqual(W.estimate_entry(f"{p}:3-100"), 3)  # 9 bytes -> ceil(9/4)

    def test_lo_past_eof_or_inverted_range_is_zero(self):
        p = self.write("d.txt", FIVE_LINES)
        self.assertEqual(W.estimate_entry(f"{p}:7-9"), 0)
        self.assertEqual(W.estimate_entry(f"{p}:4-2"), 0)

    def test_missing_file_raises(self):
        missing = str(self.root / "nope.md")
        with self.assertRaises(FileNotFoundError):
            W.estimate_entry(missing)
        with self.assertRaises(FileNotFoundError):
            W.estimate_entry(missing + ":1-2")

    def test_crlf_bytes_are_counted(self):
        p = self.write("crlf.md", b"abcd\r\n" * 4)  # 24 bytes
        self.assertEqual(W.estimate_entry(p), 6)
        self.assertEqual(W.estimate_entry(f"{p}:1-2"), 3)  # 12 bytes
        self.assertEqual(W.estimate_entry(self.write("crlf.py", b"abcd\r\n" * 4)), 7)  # ceil(24/3.5)

    def test_line_zero_rejected(self):
        p = self.write("z.md", "a\n")
        with self.assertRaises(ValueError):
            W.estimate_entry(f"{p}:0-1")


class CheckTests(Fixture):
    def test_small_set_is_ok(self):
        a = self.write("a.md", "x" * 40)  # 10 tokens
        b = self.write("b.py", "x" * 70)  # 20 tokens
        res = W.check([a, b], overhead=0)
        self.assertEqual(res["per_entry"], [10, 20])
        self.assertEqual(res["total"], 30)
        self.assertEqual(res["over_cap"], [])
        self.assertFalse(res["too_many"])
        self.assertTrue(res["ok"])

    def test_brief_bytes_round_up_by_4(self):
        self.assertEqual(W.check([], brief_bytes=5, overhead=0)["total"], 2)

    def test_four_entries_is_too_many(self):
        paths = [self.write(f"f{i}.md", "x") for i in range(4)]
        res = W.check(paths, overhead=0)
        self.assertTrue(res["too_many"])
        self.assertFalse(res["ok"])
        self.assertEqual(res["over_cap"], [])

    def test_three_entries_is_allowed(self):
        paths = [self.write(f"g{i}.md", "x") for i in range(3)]
        res = W.check(paths, overhead=0)
        self.assertFalse(res["too_many"])
        self.assertTrue(res["ok"])

    def test_entry_over_cap_is_listed(self):
        big = self.write("big.md", "x" * (4 * 12001))  # 12001 tokens
        edge = self.write("edge.md", "x" * (4 * 12000))  # exactly the cap is allowed
        res = W.check([edge, big], overhead=0)
        self.assertEqual(res["over_cap"], [big])
        self.assertFalse(res["ok"])
        res = W.check([edge], overhead=0)
        self.assertEqual(res["over_cap"], [])
        self.assertTrue(res["ok"])

    def test_brief_bytes_push_passing_set_over_total_cap(self):
        paths = [self.write(f"t{i}.md", "x" * (4 * 11000)) for i in range(3)]  # 11000 tokens each
        res = W.check(paths, overhead=10000)
        self.assertEqual(res["total"], 43000)
        self.assertTrue(res["ok"])
        # 68000 bytes = 17000 tokens -> total exactly 60000, still ok
        res = W.check(paths, brief_bytes=68000, overhead=10000)
        self.assertEqual(res["total"], 60000)
        self.assertTrue(res["ok"])
        # one more byte rounds up to 17001 tokens -> 60001, over
        res = W.check(paths, brief_bytes=68001, overhead=10000)
        self.assertEqual(res["total"], 60001)
        self.assertFalse(res["ok"])
        self.assertEqual(res["over_cap"], [])
        self.assertFalse(res["too_many"])

    def test_overhead_env_var_is_read_at_call_time(self):
        p = self.write("e.md", "x" * 40)  # 10 tokens
        with mock.patch.dict(os.environ, {"WAVE_OVERHEAD_TOKENS": "500"}):
            self.assertEqual(W.check([p])["total"], 510)
        with mock.patch.dict(os.environ, {"WAVE_OVERHEAD_TOKENS": "20000"}):
            self.assertEqual(W.check([p])["total"], 20010)
        with mock.patch.dict(os.environ):
            os.environ.pop("WAVE_OVERHEAD_TOKENS", None)
            self.assertEqual(W.check([p])["total"], W.DEFAULT_OVERHEAD + 10)

    def test_check_missing_file_raises(self):
        with self.assertRaises(FileNotFoundError):
            W.check([str(self.root / "gone.md")])


class WindowTests(Fixture):
    def spans(self, windows):
        out = []
        for w in windows:
            m = WINDOW_RE.match(w)
            self.assertIsNotNone(m, w)
            out.append((int(m.group(2)), int(m.group(3))))
        return out

    def assert_cover(self, windows, n):
        spans = self.spans(windows)
        self.assertEqual(spans[0][0], 1)
        for (_, hi), (lo_next, _) in zip(spans, spans[1:]):
            self.assertEqual(lo_next, hi + 1)  # no gap, no overlap
        self.assertEqual(spans[-1][1], n)
        for lo, hi in spans:
            self.assertLessEqual(lo, hi)

    def test_fits_in_one_window(self):
        p = self.write("small.md", "a\nb\nc\n")
        self.assertEqual(W.suggest_windows(p), [f"{p}:1-3"])

    def test_empty_file_has_no_windows(self):
        self.assertEqual(W.suggest_windows(self.write("empty.py", b"")), [])

    def test_windows_cover_file_without_gaps_or_overlap(self):
        # 500 lines x 100 bytes, .txt, cap 1000 -> floor(0.9*1000*4/100 * 500 / 500) = 36 lines per window
        p = self.write("long.txt", "".join("y" * 99 + "\n" for _ in range(500)))
        wins = W.suggest_windows(p, cap=1000)
        self.assertGreater(len(wins), 1)
        self.assertEqual(self.spans(wins)[0], (1, 36))
        self.assert_cover(wins, 500)
        for w in wins:
            self.assertLessEqual(W.estimate_entry(w), 1000)

    def test_long_lines_are_split_to_stay_under_cap(self):
        # formula gives 41 lines; the first 41 are long (3515 tokens), so the window shrinks to 11 lines
        lines = ["L" * 299 + "\n"] * 200 + ["s\n"] * 600
        p = self.write("mixed.py", "".join(lines))
        wins = W.suggest_windows(p, cap=1000)
        self.assert_cover(wins, 800)
        self.assertEqual(wins[0], f"{p}:1-11")
        for w in wins:
            self.assertLessEqual(W.estimate_entry(w), 1000)

    def test_window_length_follows_formula(self):
        # 1200 lines x 40 bytes = 48000 bytes (over cap, .py); floor(0.9*12000*3.5/40) = 945
        p = self.write("plan.py", "".join("x" * 39 + "\n" for _ in range(1200)))
        self.assertEqual(W.suggest_windows(p), [f"{p}:1-945", f"{p}:946-1200"])

    def test_single_overlong_line_gets_its_own_window(self):
        p = self.write("huge.py", "x" * 50000 + "\n" + "a\nb\n")
        self.assertEqual(W.suggest_windows(p), [f"{p}:1-1", f"{p}:2-3"])


class RepoSmokeTests(unittest.TestCase):
    def test_claude_md_estimate_and_single_window(self):
        size = REPO_CLAUDE.stat().st_size
        self.assertEqual(W.estimate_entry(str(REPO_CLAUDE)), -(-size // 4))
        with open(REPO_CLAUDE, "rb") as fh:
            n = len(fh.readlines())
        self.assertEqual(W.suggest_windows(str(REPO_CLAUDE)), [f"{REPO_CLAUDE}:1-{n}"])


class CliTests(Fixture):
    def test_check_ok_exits_0(self):
        r = self.run_cli("check", self.write("ok.md", "x" * 40))
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertIn("verdict=OK", r.stdout)

    def test_check_over_cap_exits_1(self):
        p = self.write("big.md", "x" * (4 * 12001))
        r = self.run_cli("check", p)
        self.assertEqual(r.returncode, 1)
        self.assertIn("verdict=OVER", r.stdout)
        self.assertIn(p, r.stdout)

    def test_check_brief_bytes_flag_can_push_over(self):
        p = self.write("ok.md", "x" * 40)
        self.assertEqual(self.run_cli("check", p).returncode, 0)
        r = self.run_cli("check", p, "--brief-bytes", "240000")  # 60000 tokens + 10000 overhead
        self.assertEqual(r.returncode, 1)

    def test_env_overhead_is_honoured_by_cli(self):
        p = self.write("ok.md", "x" * 40)
        self.assertEqual(self.run_cli("check", p, env={"WAVE_OVERHEAD_TOKENS": "60000"}).returncode, 1)
        self.assertEqual(self.run_cli("check", p).returncode, 0)  # default 10000

    def test_missing_file_exits_2(self):
        gone = str(self.root / "gone.md")
        r = self.run_cli("check", gone)
        self.assertEqual(r.returncode, 2)
        self.assertIn("missing file", r.stderr)
        self.assertEqual(self.run_cli("windows", gone).returncode, 2)

    def test_usage_errors_exit_2(self):
        self.assertEqual(self.run_cli().returncode, 2)
        self.assertEqual(self.run_cli("check").returncode, 2)
        p = self.write("ok.md", "x")
        self.assertEqual(self.run_cli("check", p, "--brief-bytes", "-1").returncode, 2)

    def test_windows_prints_one_per_line(self):
        p = self.write("plan.py", "".join("x" * 39 + "\n" for _ in range(1200)))
        r = self.run_cli("windows", p)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertEqual(r.stdout.splitlines(), [f"{p}:1-945", f"{p}:946-1200"])


if __name__ == "__main__":
    unittest.main()
