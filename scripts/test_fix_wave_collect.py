#!/usr/bin/env python3
"""scripts/test_fix_wave_collect.py - tests for fix_wave_collect.py (status and cost).

Every test builds its own waves root and repo root under tempfile.TemporaryDirectory(); the real
R/.claude/waves tree is never read or written. cost() is driven by a mocked usage runner.
"""
import contextlib
import io
import json
import re
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))

import fix_wave_collect as fwc  # noqa: E402
import wi_state as ws  # noqa: E402

BATCH = "fx-A"
CREATED = "2026-10-15T20:00:00Z"
APPROVE = "DECISION: APPROVE\nBLOCKING: 0\n\nlooks good\n"
REQUEST = "DECISION: REQUEST_CHANGES\nBLOCKING: 1\n\nfix the index map\n"

# item -> (unit uid, label, has final.diff, review.md text or None, extra)
FIXTURE_UNITS = [
    ("u01", "P-HAIKU-A"), ("u02", "P-HAIKU-B"),
    ("u03", "P-CONS-A"), ("u04", "P-REV-A"), ("u05", "P-REVISE-A"),
    ("u06", "P-PARK-A"), ("u07", "P-APPLY-A"), ("u08", "P-DONE-A"),
]
FIXTURE_ITEMS = ["P-HAIKU", "P-CONS", "P-REV", "P-REVISE", "P-PARK", "P-APPLY", "P-DONE"]
EXPECTED_STAGES = [
    "P-HAIKU haiku-pending", "P-CONS cons-ready", "P-REV rev-ready", "P-REVISE revise",
    "P-PARK park", "P-APPLY apply-ready", "P-DONE applied",
]


def write(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def build_batch(waves_root):
    """One item per stage. u02 (P-HAIKU-B) has no reply, so inflight starts at 1."""
    batch = Path(waves_root) / BATCH
    write(batch / "manifest.json", json.dumps({"wave": "fx", "kind": "recon", "created": CREATED, "units": []}))
    write(batch / "units.tsv", "uid\tlabel\n" + "".join(f"{u}\t{lbl}\n" for u, lbl in FIXTURE_UNITS))
    write(batch / "items.tsv", "id\tkind\tsize\tconstraint\n"
          + "".join(f"{i}\tbug\tS\tfixture\n" for i in FIXTURE_ITEMS))
    for uid, _lbl in FIXTURE_UNITS:
        if uid != "u02":
            write(batch / f"{uid}.reply.txt", "OUTPUT: x\nVERDICT: CONFIRMED\n")
    write(batch / "P-REV" / "final.diff", "diff --git a/x b/x\n")
    write(batch / "P-REVISE" / "final.diff", "diff --git a/x b/x\n")
    write(batch / "P-REVISE" / "review.md", "DECISION: APPROVE\nBLOCKING: 2\n\nblocking issues\n")
    write(batch / "P-REVISE" / "verdict.md", "# verdict P-REVISE\n")
    write(batch / "P-PARK" / "final.diff", "diff --git a/x b/x\n")
    write(batch / "P-PARK" / "review.r1.md", REQUEST)
    write(batch / "P-PARK" / "review.md", REQUEST)
    write(batch / "P-APPLY" / "final.diff", "diff --git a/x b/x\n")
    write(batch / "P-APPLY" / "review.md", "DECISION: APPROVE_MINOR\nBLOCKING: 0\n\nnit only\n")
    write(batch / "P-DONE" / "final.diff", "diff --git a/x b/x\n")
    write(batch / "P-DONE" / "review.md", APPROVE)
    write(batch / "state.tsv", "id\tstage\tcommit\nP-DONE\tapplied\tabc1234\n")
    return batch


def stage_pairs(lines):
    return [" ".join(line.split()[:2]) for line in lines[1:]]


class FixtureTestCase(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)
        self.waves = self.root / "waves"
        self.repo = self.root / "repo"
        self.repo.mkdir()
        self.batch = build_batch(self.waves)

    def tearDown(self):
        self._tmp.cleanup()

    def status(self, **kw):
        return fwc.status(self.waves, self.repo, BATCH, 20, **kw)


class StatusStageTests(FixtureTestCase):
    def test_one_item_per_stage_in_order(self):
        lines = self.status()
        self.assertEqual(lines[0], "inflight=1 free=19")
        self.assertEqual(stage_pairs(lines), EXPECTED_STAGES)

    def test_items_derived_from_units_without_items_tsv(self):
        (self.batch / "items.tsv").unlink()
        lines = self.status()
        self.assertEqual(stage_pairs(lines), EXPECTED_STAGES)

    def test_block_with_approve_routes_to_revise(self):
        self.assertEqual(fwc.read_review(self.batch / "P-REVISE" / "review.md"), ("APPROVE", 2))
        stage, _nxt = fwc.stage_of(self.batch, "P-REVISE", self.repo)
        self.assertEqual(stage, "revise")

    def test_request_changes_after_one_archive_parks(self):
        self.assertEqual(fwc.read_review(self.batch / "P-PARK" / "review.md"), ("REQUEST_CHANGES", 1))
        stage, _nxt = fwc.stage_of(self.batch, "P-PARK", self.repo)
        self.assertEqual(stage, "park")

    def test_applied_item_names_commit(self):
        stage, nxt = fwc.stage_of(self.batch, "P-DONE", self.repo)
        self.assertEqual((stage, nxt), ("applied", "committed abc1234"))

    def test_gated_state_asks_for_close_command(self):
        write(self.batch / "state.tsv", "id\tstage\tcommit\nP-APPLY\tgated\t\n")
        stage, nxt = fwc.stage_of(self.batch, "P-APPLY", self.repo)
        self.assertEqual(stage, "apply-ready")
        self.assertIn("close command", nxt)


class ReadReviewTests(unittest.TestCase):
    def test_missing_blocking_counts_as_zero(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "review.md"
            write(path, "DECISION: APPROVE_MINOR\n\nno blocking line in this review\n")
            self.assertEqual(fwc.read_review(path), ("APPROVE_MINOR", 0))

    def test_missing_file_is_none_and_zero(self):
        with tempfile.TemporaryDirectory() as tmp:
            self.assertEqual(fwc.read_review(Path(tmp) / "review.md"), (None, 0))

    def test_unknown_decision_is_none(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "review.md"
            write(path, "DECISION: MAYBE\nBLOCKING: 0\n")
            self.assertEqual(fwc.read_review(path), (None, 0))


class CopyOutTests(FixtureTestCase):
    def worktree_review(self, agent, item, text):
        path = self.repo / ".claude" / "worktrees" / agent / ".claude" / "waves" / BATCH / item / "review.md"
        write(path, text)
        return path

    def test_copy_out_lands_in_batch_dir(self):
        self.worktree_review("agent-x", "P-REV", APPROVE)
        lines = self.status()
        target = self.batch / "P-REV" / "review.md"
        self.assertTrue(target.is_file())
        self.assertEqual(target.read_text(encoding="utf-8"), APPROVE)
        self.assertTrue(any(line.startswith("P-REV apply-ready") for line in lines))

    def test_stale_copy_identical_to_archive_is_not_recopied(self):
        self.status(revise="P-REVISE")
        self.assertFalse((self.batch / "P-REVISE" / "review.md").exists())
        archived = (self.batch / "P-REVISE" / "review.r1.md").read_text(encoding="utf-8")
        self.worktree_review("agent-old", "P-REVISE", archived)
        lines = self.status()
        self.assertFalse((self.batch / "P-REVISE" / "review.md").exists())
        self.assertTrue(any(line.startswith("P-REVISE cons-ready") for line in lines))


class ReviseTests(FixtureTestCase):
    def test_revise_archives_round_one_and_goes_cons_ready(self):
        lines = self.status(revise="P-REVISE")
        item = self.batch / "P-REVISE"
        self.assertTrue((item / "review.r1.md").is_file())
        self.assertTrue((item / "final.r1.diff").is_file())
        self.assertTrue((item / "verdict.r1.md").is_file())
        self.assertFalse((item / "review.md").exists())
        self.assertFalse((item / "final.diff").exists())
        line = next(line for line in lines if line.startswith("P-REVISE "))
        self.assertTrue(line.startswith("P-REVISE cons-ready"), line)
        self.assertIn("revision round 1", line)
        self.assertIn("P-REVISE/review.r1.md", line)

    def test_revise_without_review_is_an_error(self):
        with self.assertRaises(fwc.FixWaveError):
            self.status(revise="P-CONS")

    def test_revise_unknown_item_is_an_error(self):
        with self.assertRaises(fwc.FixWaveError):
            self.status(revise="NOPE")


class MarkAndInflightTests(FixtureTestCase):
    def test_mark_cons_then_rev_counts_markers_in_inflight(self):
        lines = self.status(mark=("P-CONS", "cons"))
        self.assertTrue((self.batch / "P-CONS" / "cons.launched").is_file())
        self.assertIn("P-CONS cons-ready await consolidator P-CONS/consolidate.md", lines)
        self.assertEqual(lines[0], "inflight=2 free=18")
        lines = self.status(mark=("P-REV", "rev"))
        self.assertTrue((self.batch / "P-REV" / "rev.launched").is_file())
        self.assertEqual(lines[0], "inflight=3 free=17")

    def test_inflight_counts_missing_replies_and_markers(self):
        write(self.batch / "P-CONS" / "cons.launched", "")
        write(self.batch / "P-REV" / "rev.launched", "")
        lines = fwc.status(self.waves, self.repo, BATCH, 5)
        self.assertEqual(lines[0], "inflight=3 free=2")

    def test_bad_mark_kind_and_unknown_item_are_errors(self):
        with self.assertRaises(fwc.FixWaveError):
            self.status(mark=("P-CONS", "xyz"))
        with self.assertRaises(fwc.FixWaveError):
            self.status(mark=("NOPE", "cons"))

    def test_zero_byte_reply_counts_as_missing(self):
        write(self.batch / "u03.reply.txt", "")
        lines = self.status()
        self.assertEqual(lines[0], "inflight=2 free=18")
        self.assertTrue(any(line.startswith("P-CONS haiku-pending") for line in lines))


class MaxInflightTests(unittest.TestCase):
    def test_default_when_limits_absent(self):
        with tempfile.TemporaryDirectory() as tmp:
            self.assertEqual(fwc.load_max_inflight(Path(tmp) / "limits.toml"), 20)

    def test_reads_max_inflight_from_toml(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "limits.toml"
            write(path, "max_units = 12\nmax_inflight = 7\n\n[rates]\nhaiku = 0.045\n")
            self.assertEqual(fwc.load_max_inflight(path), 7)


class CostTests(FixtureTestCase):
    AGENTS = [
        {"id": "a1", "model": "claude-haiku-5-5", "cost_usd": 0.05, "description": "VW haiku WI-73-A u01"},
        {"id": "a2", "model": "claude-haiku-5-5", "cost_usd": 0.07, "description": "VW haiku WI-73-B u02"},
        {"id": "a3", "model": "claude-sonnet-test", "cost_usd": 0.60, "description": "VW cons WI-73"},
        {"id": "a4", "model": "claude-sonnet-test", "cost_usd": 0.10, "description": "VW cons WI-87"},
        {"id": "a5", "model": "claude-opus-test", "cost_usd": 0.30, "description": "VW rev WI-73"},
        {"id": "a6", "model": "claude-haiku-5-5", "cost_usd": None, "description": "unrelated helper"},
    ]

    def fake_runner(self, data=None, calls=None):
        payload = {"rates_as_of": "2026-10-01", "drift_lines": 0, "agents": self.AGENTS if data is None else data,
                   "total": {}}

        def run(argv):
            if calls is not None:
                calls.append(list(argv))
            return payload
        return run

    @staticmethod
    def rows(lines):
        out = {}
        for line in lines:
            m = re.match(r"^(VW haiku|VW cons|VW rev|other)\s+(\d+)\s+([\d.]+)$", line)
            if m:
                out[m.group(1)] = (int(m.group(2)), float(m.group(3)))
        return out

    def test_cost_buckets_with_mock_runner(self):
        calls = []
        lines = fwc.cost(self.waves, BATCH, [], run_usage=self.fake_runner(calls=calls))
        self.assertEqual(calls, [["--since", CREATED, "--json"]])
        rows = self.rows(lines)
        self.assertEqual(rows["VW haiku"], (2, 0.12))
        self.assertEqual(rows["VW cons"], (2, 0.70))
        self.assertEqual(rows["VW rev"], (1, 0.30))
        self.assertEqual(rows["other"], (1, 0.0))
        total = next(line for line in lines if line.startswith("total"))
        self.assertIn("1.1200", total)
        self.assertTrue(any(line.startswith("FLAG: 1 agent(s)") for line in lines))
        self.assertTrue(any(line.lstrip().startswith("claude-haiku-5-5") and " 3 " in line for line in lines))

    def test_one_call_per_session(self):
        calls = []
        lines = fwc.cost(self.waves, BATCH, ["s1", "s2"], run_usage=self.fake_runner(calls=calls))
        self.assertEqual(calls, [["--since", CREATED, "--json", "--session", "s1"],
                                 ["--since", CREATED, "--json", "--session", "s2"]])
        self.assertEqual(self.rows(lines)["VW haiku"], (4, 0.24))
        self.assertTrue(any("sessions: s1, s2" in line for line in lines))

    def test_missing_manifest_is_an_error(self):
        (self.batch / "manifest.json").unlink()
        with self.assertRaises(fwc.FixWaveError):
            fwc.cost(self.waves, BATCH, [], run_usage=self.fake_runner())


class HeartbeatTests(FixtureTestCase):
    """status() refreshes heartbeat_at of claimed WI-NN items only; claims live in a temp claims dir."""
    OLD = "2020-01-01T00:00:00Z"

    def setUp(self):
        super().setUp()
        self.store = ws.Store(self.root, claims_dir=self.root / "c")
        write(self.batch / "items.tsv", "id\tkind\tsize\tconstraint\n"
              "WI-12\tbug\tS\tfixture\nF3\tbug\tS\tfixture\nWI-13\tbug\tS\tfixture\n")
        for wi in ("WI-12", "F3"):
            claim = self.store.create_claim(wi, "s1", [], "abc1234", "VW-fx-A")
            claim["heartbeat_at"] = self.OLD
            self.store.write_claim(wi, claim)

    def test_claimed_wi_refreshed_non_wi_and_unclaimed_untouched(self):
        lines = self.status(store=self.store)
        self.assertEqual(len(lines), 4)
        refreshed = self.store.read_claim("WI-12")["heartbeat_at"]
        self.assertGreater(ws.parse_iso(refreshed), ws.parse_iso(self.OLD))
        self.assertEqual(self.store.read_claim("F3")["heartbeat_at"], self.OLD)
        self.assertFalse(self.store.claim_path("WI-13").exists())

    def test_store_error_warns_and_status_still_prints(self):
        class BrokenStore:
            def read_claim(self, wi_id):
                raise OSError("claims unreadable")

        err = io.StringIO()
        with contextlib.redirect_stderr(err):
            lines = self.status(store=BrokenStore())
        self.assertEqual(len(lines), 4)
        self.assertIn("claim heartbeat skipped", err.getvalue())
        self.assertEqual(self.store.read_claim("WI-12")["heartbeat_at"], self.OLD)


class CliTests(FixtureTestCase):
    def run_main(self, argv):
        out = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(io.StringIO()):
            rc = fwc.main(argv)
        return rc, out.getvalue().splitlines()

    def test_status_cli_prints_inflight_and_stages(self):
        limits = self.root / "no-limits.toml"
        rc, lines = self.run_main(["status", BATCH, "--waves-root", str(self.waves),
                                   "--repo-root", str(self.repo), "--limits", str(limits)])
        self.assertEqual(rc, 0)
        self.assertEqual(lines[0], "inflight=1 free=19")
        self.assertEqual(stage_pairs(lines), EXPECTED_STAGES)

    def test_status_cli_unknown_batch_returns_2(self):
        rc, _lines = self.run_main(["status", "nope-Z", "--waves-root", str(self.waves),
                                    "--repo-root", str(self.repo)])
        self.assertEqual(rc, 2)

    def test_status_cli_mark_and_revise_flags(self):
        rc, lines = self.run_main(["status", BATCH, "--waves-root", str(self.waves), "--repo-root", str(self.repo),
                                   "--mark", "P-CONS", "cons", "--revise", "P-REVISE"])
        self.assertEqual(rc, 0)
        self.assertTrue((self.batch / "P-CONS" / "cons.launched").is_file())
        self.assertTrue((self.batch / "P-REVISE" / "review.r1.md").is_file())
        self.assertTrue(any(line.startswith("P-REVISE cons-ready") for line in lines))


if __name__ == "__main__":
    unittest.main()
