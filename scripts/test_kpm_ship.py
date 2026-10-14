#!/usr/bin/env python3
"""Tests for kpm_ship.py: status rows, mod resolution, game check and run_step are all mocked (no git, no subprocess)."""
import contextlib
import io
import pathlib
import sys
import unittest
from unittest import mock

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import kpm_ship as ksh  # noqa: E402


def row(pid, verdict, src=None):
    return {"id": pid, "version": "0.1.0", "src": src or f"patch_manager_mods/{pid}_src", "verdict": verdict,
            "live": "", "kpatch": "", "detail": ""}


class Base(unittest.TestCase):
    def setUp(self):
        self.rows = [row("a", "DIRTY"), row("b", "SWAP"), row("c", "LIVE"),
                     row("perflab", "MIRROR", "patch_manager_mods/perf_lab_src/perflab")]
        self.after = None  # rows returned by the status read after apply (None = same as self.rows)
        self.calls = []
        self.rcs = {}
        self.running = False
        self.loads = 0
        for target, new in (
            (ksh, ("load_rows", self.load_rows)),
            (ksh.kc, ("resolve_mods", self.resolve)),
            (ksh.hs, ("is_game_running", lambda: self.running)),
            (ksh, ("run_step", self.run_step)),
        ):
            p = mock.patch.object(target, new[0], new[1])
            p.start()
            self.addCleanup(p.stop)

    def load_rows(self):
        self.loads += 1
        return self.after if (self.after is not None and self.loads > 1) else self.rows

    def resolve(self, names):
        dirs = ["a_src", "b_src", "c_src", "perf_lab_src"]
        out, bad = [], []
        for n in names:
            m = n if n in dirs else n + "_src" if n + "_src" in dirs else "perf_lab_src" if n == "perflab" else None
            (out if m else bad).append(m or n)
        return out, bad, dirs

    def run_step(self, label, cmd):
        self.calls.append((label, cmd))
        return self.rcs.get(label, 0)

    def labels(self):
        return [c[0] for c in self.calls]

    def ship(self, *argv):
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            rc = ksh.main(list(argv))
        return rc, out.getvalue()


class ShipTests(Base):
    def test_dry_run_runs_nothing(self):
        rc, out = self.ship("--mod", "a", "--dry-run")
        self.assertEqual(rc, 0)
        self.assertEqual(self.calls, [])
        for label in ("commit", "gate", "prepare"):
            self.assertIn(f"{label}:", out)

    def test_no_mod_without_all_is_usage_error(self):
        rc, _ = self.ship()
        self.assertEqual(rc, 2)
        self.assertEqual(self.calls, [])

    def test_no_mod_dry_run_lists_pending(self):
        rc, out = self.ship("--dry-run")
        self.assertEqual(rc, 0)
        self.assertEqual(self.calls, [])
        self.assertNotIn("\nLIVE ", out)

    def test_unknown_mod_is_usage_error(self):
        rc, out = self.ship("--mod", "nope")
        self.assertEqual(rc, 2)
        self.assertIn("nope", out)
        self.assertEqual(self.calls, [])

    def test_gate_failure_halts_before_prepare(self):
        self.rcs["gate"] = 1
        rc, out = self.ship("--mod", "a", "--apply")
        self.assertEqual(rc, 1)
        self.assertEqual(self.labels(), ["commit", "gate"])
        self.assertIn("GATE FAILED", out)

    def test_game_running_refuses_apply_early(self):
        self.running = True
        rc, out = self.ship("--mod", "a", "--apply")
        self.assertEqual(rc, 1)
        self.assertEqual(self.calls, [])
        self.assertIn("REFUSED", out)

    def test_happy_path_order(self):
        self.after = [row("a", "LIVE"), row("perflab", "LIVE", "patch_manager_mods/perf_lab_src/perflab")]
        rc, _ = self.ship("--mod", "a", "perflab", "--apply")
        self.assertEqual(rc, 0)
        self.assertEqual(self.labels(), ["commit", "gate", "prepare", "apply"])
        cmds = dict(self.calls)
        self.assertEqual(cmds["commit"][-2:], ["--mod", "a_src"])  # only the DIRTY mod; perflab is MIRROR
        gate = cmds["gate"]
        self.assertEqual([gate[i + 1] for i, x in enumerate(gate) if x == "--mod"], ["a_src", "perf_lab_src/perflab"])
        self.assertIn("--commit-kpm", cmds["prepare"])
        self.assertIn("--apply", cmds["apply"])
        self.assertEqual(cmds["prepare"][cmds["prepare"].index("--mod") + 1:cmds["prepare"].index("--prepare")], ["a_src", "perf_lab_src"])

    def test_skip_gate_omits_gate(self):
        rc, out = self.ship("--mod", "a", "--skip-gate")
        self.assertEqual(rc, 0)
        self.assertEqual(self.labels(), ["commit", "prepare"])
        self.assertIn("WARNING: gate skipped", out)

    def test_refuses_not_live_and_reinstall(self):
        for verdict in ("NOT-LIVE", "REINSTALL"):
            with self.subTest(verdict=verdict):
                self.rows = [row("a", verdict)]
                rc, out = self.ship("--mod", "a", "--apply")
                self.assertEqual(rc, 1)
                self.assertEqual(self.calls, [])
                self.assertIn("REFUSED", out)

    def test_apply_not_live_afterwards_fails(self):
        self.after = [row("b", "SWAP")]
        rc, out = self.ship("--mod", "b", "--apply")
        self.assertEqual(rc, 1)
        self.assertEqual(self.labels(), ["gate", "prepare", "apply"])  # SWAP target: no commit step
        self.assertIn("NOT LIVE", out)

    def test_prepare_only_without_apply(self):
        rc, out = self.ship("--mod", "b")
        self.assertEqual(rc, 0)
        self.assertEqual(self.labels(), ["gate", "prepare"])
        self.assertIn("run again with --apply", out)

    def test_all_live_is_nothing_to_ship(self):
        rc, out = self.ship("--mod", "c", "--apply")
        self.assertEqual(rc, 0)
        self.assertEqual(self.calls, [])
        self.assertIn("nothing to ship", out)


if __name__ == "__main__":
    unittest.main()
