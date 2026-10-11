#!/usr/bin/env python3
"""Tests for scripts/wave_cards.py (temp-dir repo fixtures; the CLI runs as a subprocess)."""
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import wave_cards as WC  # noqa: E402

SCRIPT = HERE / "wave_cards.py"
SRC_LINES = 20


def card_text(n=1, name="unit", contract="worker", scope=None, targets="`src/a.py:1-5`",
              objective="Implement the unit.", verification="`python3 scripts/tool.py`", drop=()):
    if scope is None:
        scope = f"`out/task{n}/*.py`"
    fields = [("Contract", contract), ("Write Scope", scope), ("Target Files", targets),
              ("Objective", objective), ("Verification", verification)]
    lines = [f"### Task {n}: {name}"]
    lines += [f"- **{key}**: {value}" for key, value in fields if key not in drop]
    return "\n".join(lines) + "\n"


class Fixture(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.root = Path(tmp.name)
        self.cards = self.root / "cards"
        self.cards.mkdir()
        self.write("scripts/tool.py", "print('x')\n")
        self.write("src/a.py", "".join(f"line {i}\n" for i in range(1, SRC_LINES + 1)))

    def write(self, rel, data):
        path = self.root / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data if isinstance(data, bytes) else data.encode())

    def run_verify(self, files):
        for name, text in files.items():
            (self.cards / name).write_text(text)
        return subprocess.run([sys.executable, str(SCRIPT), "verify", str(self.cards),
                               "--repo-root", str(self.root)], capture_output=True, text=True)

    def run_one(self, *cards):
        return self.run_verify({"wave.md": "\n".join(cards)})

    def card_line(self, proc, label):
        hits = [ln for ln in proc.stdout.splitlines() if ln.startswith(label + " ")]
        self.assertEqual(len(hits), 1, proc.stdout)
        return hits[0]


class VerifyTests(Fixture):
    def test_all_pass(self):
        proc = self.run_one(
            card_text(1, objective="Mirror `src/a.py:10` `line 14` behaviour."),
            card_text(2, targets="`src/a.py:1-3`, `src/a.py:4-6`"),
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
        self.assertIn("wave.md#Task 1 PASS", self.card_line(proc, "wave.md#Task 1"))
        self.assertIn("wave.md#Task 2 PASS", self.card_line(proc, "wave.md#Task 2"))
        self.assertEqual(proc.stdout.splitlines()[-1], "VERDICT: PASS cards=2 fails=0")

    def test_missing_field(self):
        proc = self.run_one(card_text(1, drop=("Objective",)))
        self.assertEqual(proc.returncode, 1)
        self.assertIn("missing Objective", self.card_line(proc, "wave.md#Task 1"))
        self.assertEqual(proc.stdout.splitlines()[-1], "VERDICT: FAIL cards=1 fails=1")

    def test_invalid_contract(self):
        proc = self.run_one(card_text(1, contract="maybe"))
        self.assertEqual(proc.returncode, 1)
        self.assertIn("Contract must be worker or recon", self.card_line(proc, "wave.md#Task 1"))

    def test_missing_target_file(self):
        proc = self.run_one(card_text(1, targets="`src/nope.py`"))
        self.assertEqual(proc.returncode, 1)
        self.assertIn("missing target file src/nope.py", self.card_line(proc, "wave.md#Task 1"))

    def test_too_many_targets(self):
        proc = self.run_one(card_text(1, targets="`src/a.py:1-2`, `src/a.py:3-4`, `src/a.py:5-6`, `src/a.py:7-8`"))
        self.assertEqual(proc.returncode, 1)
        self.assertIn("4 Target Files entries > 3", self.card_line(proc, "wave.md#Task 1"))

    def test_window_out_of_range(self):
        proc = self.run_one(card_text(1, targets="`src/a.py:1-500`"))
        self.assertEqual(proc.returncode, 1)
        self.assertIn("out of range", self.card_line(proc, "wave.md#Task 1"))

    def test_bad_anchor_quote(self):
        # Line 16 exists in src/a.py but lies outside +-5 lines of the anchor at line 10.
        proc = self.run_one(card_text(1, objective="See `src/a.py:10` `line 16` for the mirror."))
        self.assertEqual(proc.returncode, 1)
        self.assertIn("quote for src/a.py:10 not within 5 lines", self.card_line(proc, "wave.md#Task 1"))

    def test_anchor_beyond_eof(self):
        proc = self.run_one(card_text(1, objective="See `src/a.py:99`."))
        self.assertEqual(proc.returncode, 1)
        self.assertIn("outside file (20 lines)", self.card_line(proc, "wave.md#Task 1"))

    def test_unresolvable_verification_binary(self):
        proc = self.run_one(card_text(1, verification="`frobnicate --check`"))
        self.assertEqual(proc.returncode, 1)
        self.assertIn("unresolvable verification binary frobnicate", self.card_line(proc, "wave.md#Task 1"))

    def test_missing_verification_script(self):
        proc = self.run_one(card_text(1, verification="`python3 scripts/missing.py`"))
        self.assertEqual(proc.returncode, 1)
        self.assertIn("verification script not found scripts/missing.py", self.card_line(proc, "wave.md#Task 1"))

    def test_overlapping_write_scopes(self):
        proc = self.run_one(card_text(1, scope="`src/*.py`"), card_text(2, scope="`src/a.py`"))
        self.assertEqual(proc.returncode, 1)
        self.assertIn("write scope overlaps wave.md#Task 2", self.card_line(proc, "wave.md#Task 1"))
        self.assertIn("write scope overlaps wave.md#Task 1", self.card_line(proc, "wave.md#Task 2"))
        self.assertEqual(proc.stdout.splitlines()[-1], "VERDICT: FAIL cards=2 fails=2")

    def test_over_budget_target(self):
        # 50500 bytes of prose-free code is about 14429 tokens, over the 12000-token per-entry cap.
        self.write("src/big.py", "".join("x" * 99 + "\n" for _ in range(500)))
        proc = self.run_one(card_text(1, targets="`src/big.py`"))
        self.assertEqual(proc.returncode, 1)
        self.assertIn("src/big.py is ~", self.card_line(proc, "wave.md#Task 1"))
        self.assertIn("> 12000", self.card_line(proc, "wave.md#Task 1"))

    def test_sensitive_target_is_refused(self):
        proc = self.run_one(card_text(1, targets="`src/secret_keys.py`"))
        self.assertEqual(proc.returncode, 1)
        self.assertIn("refusing to read sensitive file src/secret_keys.py", self.card_line(proc, "wave.md#Task 1"))

    def test_empty_directory(self):
        proc = self.run_verify({})
        self.assertEqual(proc.returncode, 1)
        self.assertEqual(proc.stdout.strip(), "VERDICT: FAIL cards=0 fails=0")

    def test_missing_cards_dir_is_usage_error(self):
        proc = subprocess.run([sys.executable, str(SCRIPT), "verify", str(self.root / "absent"),
                               "--repo-root", str(self.root)], capture_output=True, text=True)
        self.assertEqual(proc.returncode, 2)
        self.assertIn("not a directory", proc.stderr)


class HelperTests(unittest.TestCase):
    def test_globs_overlap(self):
        self.assertTrue(WC._globs_overlap("src/*.py", "src/a.py"))
        self.assertTrue(WC._globs_overlap("scripts/*.py", "scripts/run_tests.py"))
        self.assertFalse(WC._globs_overlap("scripts/wave_*.py", "scripts/run_tests.py"))
        self.assertFalse(WC._globs_overlap("docs/*.md", "src/*.py"))


if __name__ == "__main__":
    unittest.main()
