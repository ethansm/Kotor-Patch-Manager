#!/usr/bin/env python3
"""Tests for scripts/bug_investigate_score.py (stdlib unittest; synthetic reports and cases only).

Run: python3 -I scripts/test_bug_investigate_score.py
Prints a final `bug_investigate_score tests: N/N PASS` line and exits non-zero on any failure.

Every run dir and case lives in a temp dir. No model call, no network, and bug_investigate.py is never imported.
"""
import builtins
import contextlib
import hashlib
import io
import json
import math
import os
import random
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import bug_investigate_score as bis  # noqa: E402

SCRIPT = HERE / "bug_investigate_score.py"
CAVEAT = "hypotheses from the same run share evidence: intervals are optimistic"
RUBRIC = {"facts": {"stale": [r"stale\s+slot"], "queue": [r"queue\s+length"], "info": [r"retry"]},
          "hit": ["stale", "queue"], "note": "synthetic rubric"}


# ---------------------------------------------------------------- helpers
def read_json(path):
    with open(path, "rb") as f:
        return json.loads(f.read().decode("utf-8"))


def write_json(path, obj):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(obj, f, sort_keys=True)


def write_text(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


def hyp(hid, claim="", mechanism="", falsifier="", quote_valid=True, status=None, verifier=None,
        evidence=("ev:a",), unit="blind-skeptic", confidence="high", **extra):
    """A report.json hypothesis entry in the bug-investigate-report/1 shape (synthetic)."""
    d = {"id": "%s:%s" % (unit, hid), "unit_id": unit, "hypothesis_id": hid, "claim": claim,
         "mechanism": mechanism, "falsifier": falsifier, "confidence": confidence, "quote_valid": quote_valid,
         "evidence_ids": list(evidence) if quote_valid else [],
         "status": status or ("supported" if quote_valid else "rejected"), "reason": "", "verifier": verifier}
    d.update(extra)
    return d


def verdict(v, applied):
    return {"complete": True, "verdict": v, "quote_valid": True, "applied": applied, "reason": ""}


def closed_wilson(k, n, z=1.96):
    """Wilson 95% interval, straight from the closed form in the spec."""
    p = k / n
    z2 = z * z
    c = (p + z2 / (2 * n)) / (1 + z2 / n)
    h = z * math.sqrt(p * (1 - p) / n + z2 / (4 * n * n)) / (1 + z2 / n)
    return c - h, c + h


def listing(root):
    out = set()
    for dp, _, fns in os.walk(root):
        for fn in fns:
            out.add(os.path.relpath(os.path.join(dp, fn), root))
    return out


class Base(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="bis_test_")
        self.addCleanup(shutil.rmtree, self.tmp, True)
        self.case = os.path.join(self.tmp, "cases", "case.json")
        self.set_rubric(RUBRIC)

    def set_rubric(self, rubric, has_key=True):
        body = {"id": "synthetic_case"}
        if has_key:
            body["score_rubric"] = rubric
        write_json(self.case, body)

    def new_run(self, name="run1"):
        d = os.path.join(self.tmp, name)
        os.makedirs(d, exist_ok=True)
        return d

    def report(self, run, hyps, ranked=None, decision="ranked", null_verdict="not_a_bug", null_valid=True,
               fmt="bug-investigate-report/1"):
        if ranked is None:
            ranked = [h["id"] for h in hyps if h["status"] == "supported"]
        write_json(os.path.join(run, "report.json"), {
            "format": fmt, "run_id": os.path.basename(run), "kind": "crash", "decision": decision,
            "human_gate": "synthetic", "ranked": ranked, "hypotheses": hyps, "null_complete": True,
            "null_verdict": null_verdict, "null_valid": null_valid, "incomplete": [], "verify_incomplete": []})
        return run

    def unit(self, run, stage, name, body):
        write_json(os.path.join(run, stage, "out", "runs", name + ".json"), body)

    def call(self, *argv):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            code = bis.main(list(argv))
        return code, out.getvalue(), err.getvalue()

    def score(self, run):
        code, _, err = self.call("score", "--run", run, "--case", self.case, "--json")
        self.assertEqual(code, 0, err)
        return read_json(os.path.join(run, "score.json"))

    def aggregate(self, *runs):
        """Score every run first (aggregate reads score.json), then aggregate them."""
        for r in runs:
            self.score(r)
        argv = ["aggregate"]
        for r in runs:
            argv += ["--run", r]
        code, out, err = self.call(*argv, "--json")
        self.assertEqual(code, 0, err)
        return json.loads(out)

    def cli(self, *args):
        return subprocess.run([sys.executable, "-I", str(SCRIPT)] + list(args), capture_output=True, text=True,
                              timeout=120)

    def recorded(self, fn):
        """Run fn with builtins.open / io.open wrapped; return (fn result, list of realpaths opened)."""
        real_open = builtins.open
        opened = []

        def rec(file, *a, **k):
            if isinstance(file, (str, bytes, os.PathLike)):
                opened.append(os.path.realpath(os.fsdecode(file)))
            return real_open(file, *a, **k)

        with mock.patch("builtins.open", rec), mock.patch("io.open", rec):
            result = fn()
        return result, opened


# ---------------------------------------------------------------- label table and fact matching
class TestLabels(Base):
    def labelled(self, hyps, rubric=None):
        if rubric is not None:
            self.set_rubric(rubric)
        run = self.report(self.new_run("lab"), hyps)
        return {h["id"]: h for h in self.score(run)["hypotheses"]}

    def test_hit_when_every_hit_fact_matches(self):
        got = self.labelled([hyp("h1", claim="stale slot 3 and queue length 0")])
        self.assertEqual(got["blind-skeptic:h1"]["label"], "HIT")
        self.assertEqual(got["blind-skeptic:h1"]["facts"], ["queue", "stale"])

    def test_partial_when_some_but_not_all_hit_facts_match(self):
        got = self.labelled([hyp("h1", claim="stale slot 3 only")])
        self.assertEqual(got["blind-skeptic:h1"]["label"], "PARTIAL")
        self.assertEqual(got["blind-skeptic:h1"]["facts"], ["stale"])

    def test_miss_when_no_hit_fact_matches(self):
        got = self.labelled([hyp("h1", claim="nothing relevant here")])
        self.assertEqual(got["blind-skeptic:h1"]["label"], "MISS")
        self.assertEqual(got["blind-skeptic:h1"]["facts"], [])

    def test_mechanism_text_is_searched_too(self):
        got = self.labelled([hyp("h1", claim="stale slot", mechanism="queue length 0")])
        self.assertEqual(got["blind-skeptic:h1"]["label"], "HIT")

    def test_matching_is_case_insensitive(self):
        got = self.labelled([hyp("h1", claim="STALE   SLOT 3 and Queue Length")])
        self.assertEqual(got["blind-skeptic:h1"]["label"], "HIT")

    def test_falsifier_and_quote_text_are_ignored(self):
        got = self.labelled([hyp("h1", claim="nothing here", mechanism="none",
                                 falsifier="stale slot and queue length", quote="stale slot and queue length")])
        self.assertEqual(got["blind-skeptic:h1"]["label"], "MISS")
        self.assertEqual(got["blind-skeptic:h1"]["facts"], [])

    def test_informational_fact_outside_hit_does_not_change_label(self):
        got = self.labelled([
            hyp("p1", claim="stale slot"),
            hyp("p2", claim="stale slot", mechanism="retry"),
            hyp("m1", claim="retry"),
            hyp("h1", claim="stale slot queue length", mechanism="retry"),
        ])
        self.assertEqual(got["blind-skeptic:p1"]["label"], "PARTIAL")
        self.assertEqual(got["blind-skeptic:p2"]["label"], "PARTIAL")
        self.assertEqual(got["blind-skeptic:p2"]["facts"], ["info", "stale"])
        self.assertEqual(got["blind-skeptic:m1"]["label"], "MISS")
        self.assertEqual(got["blind-skeptic:m1"]["facts"], ["info"])
        self.assertEqual(got["blind-skeptic:h1"]["label"], "HIT")
        self.assertEqual(got["blind-skeptic:h1"]["facts"], ["info", "queue", "stale"])

    def test_counts_over_all_and_over_quote_valid(self):
        run = self.report(self.new_run("cnt"), [
            hyp("h1", claim="stale slot queue length"),
            hyp("p1", claim="stale slot"),
            hyp("m1", claim="nothing", quote_valid=False),
        ])
        s = self.score(run)
        self.assertEqual(s["counts"]["all"], {"HIT": 1, "PARTIAL": 1, "MISS": 1})
        self.assertEqual(s["counts"]["quote_valid"], {"HIT": 1, "PARTIAL": 1, "MISS": 0})
        self.assertEqual(s["quote_valid"]["valid"], 2)
        self.assertEqual(s["quote_valid"]["total"], 3)
        self.assertAlmostEqual(s["quote_valid"]["rate"], 2 / 3, places=6)

    def test_score_fields_per_hypothesis(self):
        run = self.report(self.new_run("fld"), [
            hyp("h2", claim="stale slot queue length", evidence=("e1", "e1", "e2"), status="rejected",
                verifier=verdict("rejected", True)),
        ])
        h = self.score(run)["hypotheses"][0]
        self.assertEqual(h["status_A"], "supported")
        self.assertEqual(h["status_B"], "rejected")
        self.assertEqual(h["verifier_verdict"], "rejected")
        self.assertEqual(h["evidence_ids"], 2)
        self.assertTrue(h["quote_valid"])
        self.assertEqual(h["label"], "HIT")

    def test_status_A_is_rejected_when_quote_invalid(self):
        run = self.report(self.new_run("qv"), [hyp("m1", claim="stale slot queue length", quote_valid=False)])
        h = self.score(run)["hypotheses"][0]
        self.assertEqual(h["status_A"], "rejected")
        self.assertIsNone(h["verifier_verdict"])


# ---------------------------------------------------------------- Wilson interval
class TestWilson(unittest.TestCase):
    def test_wilson_n10_k5_interval(self):
        lo, hi = bis.wilson(5, 10)
        self.assertEqual(round(lo, 3), 0.237)
        self.assertEqual(round(hi, 3), 0.763)

    def test_wilson_n5_k2_matches_closed_form(self):
        lo, hi = bis.wilson(2, 5)
        elo, ehi = closed_wilson(2, 5)
        self.assertAlmostEqual(lo, elo, places=5)
        self.assertAlmostEqual(hi, ehi, places=5)

    def test_wilson_empty_is_none(self):
        self.assertIsNone(bis.wilson(0, 0))


# ---------------------------------------------------------------- verifier effect and aggregate
class TestVerifierAndAggregate(Base):
    def test_verifier_rejected_hit_is_a_false_rejection(self):
        run = self.report(self.new_run("vf"), [
            hyp("h1", claim="stale slot queue length"),
            hyp("h2", claim="stale slot queue length", status="rejected", verifier=verdict("rejected", True)),
            hyp("h3", claim="nothing", quote_valid=False),
        ])
        agg = self.aggregate(run)
        self.assertEqual(agg["config_A"]["n_supported"], 2)
        self.assertEqual(agg["config_B"]["n_supported"], 1)
        eff = agg["verifier_effect"]
        self.assertEqual(eff["rejected_by_verifier"]["HIT"], 1)
        self.assertEqual(eff["rejected_by_verifier"]["total"], 1)
        self.assertEqual(eff["false_rejections"], 1)
        self.assertEqual(eff["discordance"]["HIT"], {"A+B+": 1, "A+B-": 1, "A-B+": 0, "A-B-": 0})
        self.assertEqual(eff["discordance"]["MISS"]["A-B-"], 1)

    def test_verifier_rejected_miss_is_not_a_false_rejection(self):
        run = self.report(self.new_run("vm"), [
            hyp("m1", claim="nothing", status="rejected", verifier=verdict("rejected", True)),
        ])
        eff = self.aggregate(run)["verifier_effect"]
        self.assertEqual(eff["rejected_by_verifier"]["MISS"], 1)
        self.assertEqual(eff["false_rejections"], 0)

    def test_pooled_counts_precision_and_rank1(self):
        r1 = self.report(self.new_run("r1"), [
            hyp("a", claim="stale slot queue length", evidence=("e1", "e2")),
            hyp("b", claim="stale slot", evidence=("e1",)),
            hyp("c", claim="nothing", quote_valid=False),
        ])
        r2 = self.report(self.new_run("r2"), [
            hyp("d", claim="stale slot queue length", evidence=("e1", "e2", "e3"), status="rejected",
                verifier=verdict("rejected", True)),
            hyp("e", claim="queue length", evidence=("e4",)),
        ], ranked=["blind-skeptic:e"])
        self.score(r1)
        self.score(r2)
        before = listing(r1)
        agg = self.aggregate(r2, r1)  # order of --run must not matter
        self.assertEqual(agg["caveat"], CAVEAT)
        self.assertEqual(agg["n_runs"], 2)
        a, b = agg["config_A"], agg["config_B"]
        self.assertEqual((a["n_supported"], a["HIT"], a["PARTIAL"], a["MISS"]), (4, 2, 2, 0))
        self.assertEqual((b["n_supported"], b["HIT"], b["PARTIAL"], b["MISS"]), (3, 1, 2, 0))
        self.assertEqual((a["precision_hit"]["k"], a["precision_hit"]["n"]), (2, 4))
        self.assertAlmostEqual(a["precision_hit"]["p"], 0.5, places=6)
        for got, exp in zip(a["precision_hit"]["wilson95"], closed_wilson(2, 4)):
            self.assertAlmostEqual(got, exp, places=5)
        self.assertAlmostEqual(a["hit_or_partial"]["p"], 1.0, places=6)
        for got, exp in zip(a["hit_or_partial"]["wilson95"], closed_wilson(4, 4)):
            self.assertAlmostEqual(got, exp, places=5)
        for got, exp in zip(b["precision_hit"]["wilson95"], closed_wilson(1, 3)):
            self.assertAlmostEqual(got, exp, places=5)
        self.assertEqual(agg["rank1"]["A"]["runs_with_top"], 2)
        self.assertEqual(agg["rank1"]["A"]["HIT"], 2)
        # run r2: A top is d (HIT); d is verifier-rejected, so B top is e (PARTIAL)
        self.assertEqual(agg["rank1"]["B"]["runs_with_top"], 2)
        self.assertEqual(agg["rank1"]["B"]["HIT"], 1)
        self.assertEqual(agg["rank1"]["B"]["p"], 0.5)
        self.assertEqual(agg["verifier_effect"]["false_rejections"], 1)
        self.assertEqual(listing(r1), before)  # aggregate writes nothing

    def test_zero_hypotheses_does_not_crash(self):
        run = self.report(self.new_run("zero"), [], ranked=[])
        agg = self.aggregate(run)
        self.assertEqual(agg["config_A"]["n_supported"], 0)
        self.assertIsNone(agg["config_A"]["precision_hit"]["p"])
        self.assertIsNone(agg["config_A"]["precision_hit"]["wilson95"])
        self.assertIsNone(agg["config_B"]["hit_or_partial"]["wilson95"])
        self.assertEqual(agg["rank1"]["A"]["runs_with_top"], 0)
        self.assertIsNone(agg["rank1"]["A"]["p"])
        self.assertIsNone(agg["cost"]["per_unit_mean_usd"])
        self.assertEqual(agg["caveat"], CAVEAT)

    def test_null_verdict_counts(self):
        r1 = self.report(self.new_run("n1"), [hyp("h1", claim="stale slot queue length")], null_verdict="not_a_bug")
        r2 = self.report(self.new_run("n2"), [], ranked=[], null_verdict=None, null_valid=False)
        agg = self.aggregate(r1, r2)
        self.assertEqual(agg["null_verdicts"], {"not_a_bug": 1, "none": 1})
        self.assertEqual(agg["null_valid"], {"true": 1, "false": 1})

    def test_aggregate_rejects_a_non_score_file(self):
        run = self.new_run("bad")
        write_json(os.path.join(run, "score.json"), {"format": "something-else/1"})
        code, _, err = self.call("aggregate", "--run", run, "--json")
        self.assertEqual(code, 2, err)


# ---------------------------------------------------------------- cost
class TestCost(Base):
    def test_null_cost_is_never_summed_as_zero(self):
        run = self.report(self.new_run("c1"), [hyp("h1", claim="stale slot queue length")])
        self.unit(run, "blind", "a", {"cost_usd": 0.5, "duration_s": 4})
        self.unit(run, "blind", "b", {"cost_usd": None, "duration_s": 2})
        cost = self.score(run)["cost"]
        self.assertEqual(cost["total_cost_usd"], 0.5)
        self.assertFalse(cost["cost_complete"])
        by_file = {u["file"]: u for u in cost["units"]}
        self.assertIsNone(by_file["b.json"]["cost_usd"])
        self.assertEqual(by_file["b.json"]["duration_s"], 2)
        self.assertEqual(by_file["a.json"]["cost_usd"], 0.5)

    def test_missing_cost_key_is_null(self):
        run = self.report(self.new_run("c2"), [hyp("h1", claim="stale slot queue length")])
        self.unit(run, "null", "n1", {"duration_s": 2.5})
        cost = self.score(run)["cost"]
        self.assertIsNone(cost["units"][0]["cost_usd"])
        self.assertEqual(cost["units"][0]["duration_s"], 2.5)
        self.assertFalse(cost["cost_complete"])

    def test_boolean_cost_is_null(self):
        run = self.report(self.new_run("c3"), [hyp("h1", claim="stale slot queue length")])
        self.unit(run, "verify", "v1", {"cost_usd": True, "duration_s": 1})
        self.assertIsNone(self.score(run)["cost"]["units"][0]["cost_usd"])

    def test_known_costs_sum_and_complete(self):
        # The completeness gate now needs 3 blind and 1 null cost record (see TestScorerStatistics), so the
        # fixture carries them; the old fixture (1 blind + 1 verify) is no longer complete by design.
        run = self.report(self.new_run("c4"), [hyp("h1", claim="stale slot queue length")])
        for name in ("a", "b", "c"):
            self.unit(run, "blind", name, {"cost_usd": 0.25, "duration_s": 1})
        self.unit(run, "null", "n", {"cost_usd": 0.25, "duration_s": 1})
        self.unit(run, "verify", "v", {"cost_usd": 0.5, "duration_s": 1})
        cost = self.score(run)["cost"]
        self.assertAlmostEqual(cost["total_cost_usd"], 1.5, places=9)   # 3 x 0.25 blind + 0.25 null + 0.5 verify
        self.assertTrue(cost["cost_complete"])

    def test_missing_unit_dirs_are_fine(self):
        run = self.report(self.new_run("c5"), [hyp("h1", claim="stale slot queue length")])
        cost = self.score(run)["cost"]
        self.assertEqual(cost["units"], [])
        self.assertEqual(cost["total_cost_usd"], 0.0)

    def test_aggregate_per_unit_mean_is_over_known_costs_only(self):
        r1 = self.report(self.new_run("ca"), [hyp("h1", claim="stale slot queue length")])
        self.unit(r1, "blind", "a", {"cost_usd": 1.0, "duration_s": 1})
        self.unit(r1, "null", "n", {"cost_usd": None, "duration_s": 1})
        r2 = self.report(self.new_run("cb"), [hyp("h1", claim="stale slot queue length")])
        self.unit(r2, "verify", "v", {"cost_usd": 3.0, "duration_s": 1})
        cost = self.aggregate(r1, r2)["cost"]
        self.assertEqual(cost["units"], 3)
        self.assertEqual(cost["units_known"], 2)
        self.assertAlmostEqual(cost["total_cost_usd"], 4.0, places=9)
        self.assertFalse(cost["cost_complete"])
        self.assertAlmostEqual(cost["per_unit_mean_usd"], 2.0, places=9)


# ---------------------------------------------------------------- exit codes and input errors
class TestExitCodes(Base):
    def test_missing_score_rubric_exits_2_and_writes_nothing(self):
        self.set_rubric(None, has_key=False)
        run = self.report(self.new_run("nr"), [hyp("h1", claim="stale slot queue length")])
        before = listing(run)
        p = self.cli("score", "--run", run, "--case", self.case)
        self.assertEqual(p.returncode, 2, p.stderr)
        self.assertIn("score_rubric", p.stderr)
        self.assertFalse(os.path.exists(os.path.join(run, "score.json")))
        self.assertEqual(listing(run), before)

    def test_bad_rubric_exits_2_and_writes_nothing(self):
        cases = [
            {"facts": {"a": ["x"]}, "hit": ["b"]},      # hit names an unknown fact
            {"facts": {"a": ["x"]}, "hit": []},         # empty hit list
            {"facts": {"a": ["("]}, "hit": ["a"]},      # regex does not compile
            {"facts": {"a": "x"}, "hit": ["a"]},        # fact value is not a list
        ]
        run = self.report(self.new_run("br"), [hyp("h1", claim="x")])
        for rubric in cases:
            with self.subTest(rubric=rubric):
                self.set_rubric(rubric)
                p = self.cli("score", "--run", run, "--case", self.case)
                self.assertEqual(p.returncode, 2, p.stderr)
                self.assertFalse(os.path.exists(os.path.join(run, "score.json")))

    def test_other_report_format_exits_2_and_writes_nothing(self):
        run = self.report(self.new_run("fm"), [hyp("h1", claim="x")], fmt="bug-investigate-report/0")
        p = self.cli("score", "--run", run, "--case", self.case)
        self.assertEqual(p.returncode, 2, p.stderr)
        self.assertFalse(os.path.exists(os.path.join(run, "score.json")))

    def test_unreadable_report_exits_2(self):
        run = self.new_run("ur")
        self.assertEqual(self.cli("score", "--run", run, "--case", self.case).returncode, 2)  # no report.json
        write_text(os.path.join(run, "report.json"), "{not json")
        p = self.cli("score", "--run", run, "--case", self.case)
        self.assertEqual(p.returncode, 2, p.stderr)
        self.assertFalse(os.path.exists(os.path.join(run, "score.json")))

    def test_unreadable_case_exits_2(self):
        run = self.report(self.new_run("uc"), [hyp("h1", claim="x")])
        write_text(self.case, "[not an object")
        self.assertEqual(self.cli("score", "--run", run, "--case", self.case).returncode, 2)
        self.assertEqual(self.cli("score", "--run", run, "--case", os.path.join(self.tmp, "absent.json")).returncode,
                         2)

    def test_aggregate_missing_score_exits_2(self):
        run = self.new_run("ms")
        p = self.cli("aggregate", "--run", run, "--json")
        self.assertEqual(p.returncode, 2, p.stderr)

    def test_usage_error_exits_2(self):
        run = self.new_run("ue")
        self.assertEqual(self.cli("score", "--run", run).returncode, 2)


# ---------------------------------------------------------------- isolation: only --run dirs and --case
class TestIsolation(Base):
    def test_score_never_opens_outside_run_and_case(self):
        run = self.report(self.new_run("run1"), [hyp("h1", claim="stale slot queue length")])
        self.unit(run, "blind", "u1", {"cost_usd": 0.1, "duration_s": 1})
        sib = self.new_run("run1_sibling")
        write_text(os.path.join(sib, "report.json"), "garbage that must never be read")
        write_text(os.path.join(sib, "score.json"), "garbage that must never be read")
        write_text(os.path.join(self.tmp, "report.json"), "garbage that must never be read")
        write_text(os.path.join(self.tmp, "cases", "case_other.json"), "garbage that must never be read")
        result, opened = self.recorded(
            lambda: self.call("score", "--run", run, "--case", self.case, "--json"))
        self.assertEqual(result[0], 0, result[2])
        allowed = os.path.realpath(run) + os.sep
        case_real = os.path.realpath(self.case)
        self.assertTrue(opened)
        for p in opened:
            self.assertTrue(p == case_real or p.startswith(allowed), "opened outside allowed set: %s" % p)
        self.assertIn(os.path.realpath(os.path.join(run, "report.json")), opened)

    def test_aggregate_opens_only_score_files_under_run_dirs(self):
        r1 = self.report(self.new_run("ag1"), [hyp("h1", claim="stale slot queue length")])
        r2 = self.report(self.new_run("ag2"), [hyp("h2", claim="stale slot")])
        self.score(r1)
        self.score(r2)
        sib = self.new_run("ag1_sibling")
        write_text(os.path.join(sib, "score.json"), "garbage")
        result, opened = self.recorded(lambda: self.call("aggregate", "--run", r1, "--run", r2, "--json"))
        self.assertEqual(result[0], 0, result[2])
        expected = {os.path.realpath(os.path.join(r1, "score.json")), os.path.realpath(os.path.join(r2, "score.json"))}
        self.assertTrue(set(opened) <= expected, set(opened) - expected)


# ---------------------------------------------------------------- determinism and layout
class TestDeterminism(Base):
    def test_two_score_calls_are_byte_identical(self):
        run = self.report(self.new_run("det"), [
            hyp("h1", claim="stale slot queue length", evidence=("e2", "e1")),
            hyp("p1", claim="stale slot"),
        ])
        self.unit(run, "blind", "u", {"cost_usd": 0.2, "duration_s": 3})
        first = self.cli("score", "--run", run, "--case", self.case, "--json")
        self.assertEqual(first.returncode, 0, first.stderr)
        b1 = open(os.path.join(run, "score.json"), "rb").read()
        second = self.cli("score", "--run", run, "--case", self.case, "--json")
        self.assertEqual(second.returncode, 0, second.stderr)
        b2 = open(os.path.join(run, "score.json"), "rb").read()
        self.assertEqual(b1, b2)
        self.assertEqual(first.stdout, second.stdout)

    def test_score_json_layout(self):
        run = self.report(self.new_run("lay"), [hyp("h1", claim="stale slot queue length")])
        self.score(run)
        raw = open(os.path.join(run, "score.json"), "rb").read().decode("utf-8")
        obj = json.loads(raw)
        self.assertEqual(raw, json.dumps(obj, sort_keys=True, indent=2) + "\n")

    def test_aggregate_output_is_order_independent(self):
        r1 = self.report(self.new_run("o1"), [hyp("h1", claim="stale slot queue length")])
        r2 = self.report(self.new_run("o2"), [hyp("h2", claim="stale slot")])
        self.score(r1)
        self.score(r2)
        a = self.call("aggregate", "--run", r1, "--run", r2, "--json")
        b = self.call("aggregate", "--run", r2, "--run", r1, "--json")
        self.assertEqual(a[1], b[1])


# ---------------------------------------------------------------- CLI smoke under python3 -I
class TestCli(Base):
    def test_cli_json_and_summary(self):
        run = self.report(self.new_run("cli"), [hyp("h1", claim="stale slot queue length")])
        p = self.cli("score", "--run", run, "--case", self.case, "--json")
        self.assertEqual(p.returncode, 0, p.stderr)
        self.assertEqual(json.loads(p.stdout)["format"], "bug-investigate-score/1")
        p2 = self.cli("score", "--run", run, "--case", self.case)
        self.assertEqual(p2.returncode, 0, p2.stderr)
        self.assertNotIn("{", p2.stdout.strip()[:1])
        self.assertIn(os.path.basename(run), p2.stdout)


# ---------------------------------------------------------------- matcher v2: echo mask, negation, hedge, guards
GOLDEN_RUBRIC = {"facts": {"invalid_id": [r"invalid\s+id", r"ffffffff"],
                           "follower_queue_entry": ["companion", "queue"],
                           "recast": [r"already\s+cast"], "info": ["retry"]},
                 "hit": ["invalid_id", "recast"], "note": "synthetic"}
GOLDEN_SYMPTOM = "When a companion is in my party and I queue a spell"
NEW_KEYS = ("matcher", "rubric_sha256", "case_id", "top_matches_report", "rank1_alt_label")
# score.json written by the pre-change (v1-only) scorer for golden_hyps() + GOLDEN_RUBRIC, compact form.
# Two deliberate differences from the pre-change file, both from the statistics change: cost_complete is false
# (zero units: the gate needs 3 blind and 1 null record), and the new top-level keys in NEW_KEYS are stripped.
# The v1 matcher output itself (hypotheses, labels, facts, counts) is unchanged.
GOLDEN_V1 = (
    '{"case":"case.json","cost":{"cost_complete":false,"total_cost_usd":0.0,"units":[]},'
    '"counts":{"all":{"HIT":0,"MISS":0,"PARTIAL":4},"quote_valid":{"HIT":0,"MISS":0,"PARTIAL":3}},'
    '"decision":"ranked","format":"bug-investigate-score/1","hypotheses":['
    '{"evidence_ids":1,"facts":["invalid_id"],"id":"blind-skeptic:h1","label":"PARTIAL","quote_valid":true,'
    '"status_A":"supported","status_B":"supported","verifier_verdict":null},'
    '{"evidence_ids":2,"facts":["follower_queue_entry","info","invalid_id"],"id":"blind-skeptic:h2",'
    '"label":"PARTIAL","quote_valid":true,"status_A":"supported","status_B":"supported","verifier_verdict":null},'
    '{"evidence_ids":1,"facts":["follower_queue_entry","recast"],"id":"blind-skeptic:h3","label":"PARTIAL",'
    '"quote_valid":true,"status_A":"supported","status_B":"rejected","verifier_verdict":"rejected"},'
    '{"evidence_ids":0,"facts":["invalid_id"],"id":"blind-null:n1","label":"PARTIAL","quote_valid":false,'
    '"status_A":"rejected","status_B":"rejected","verifier_verdict":null}],'
    '"null_valid":true,"null_verdict":"not_a_bug","quote_valid":{"rate":0.75,"total":4,"valid":3},'
    '"ranked":["blind-skeptic:h1","blind-skeptic:h2"],"run_id":"golden_run",'
    '"top_ranked":{"A":{"id":"blind-skeptic:h2","label":"PARTIAL"},"B":{"id":"blind-skeptic:h2","label":"PARTIAL"}}}'
)


def canon(obj):
    return json.dumps(obj, sort_keys=True, indent=2) + "\n"


def by_id(obj):
    return {h["id"]: h for h in obj["hypotheses"]}


def golden_hyps():
    return [
        hyp("h1", claim="is not an invalid id for the slot", confidence="medium", evidence=("ev:a",)),
        hyp("h2", claim="the loader returns an invalid id on retry", mechanism="the queue entry keeps the stale value",
            confidence="high", evidence=("ev:b", "ev:a")),
        hyp("h3", claim="the companion queue entry might have already cast", confidence="low", evidence=("ev:c",),
            status="rejected", verifier=verdict("rejected", True)),
        hyp("n1", claim="this was never ffffffff", unit="blind-null", confidence="low", quote_valid=False),
    ]


class TestMatcherV2(Base):
    def set_case(self, rubric, symptom=None, case_id=None):
        body = {"score_rubric": rubric}
        if symptom is not None:
            body["symptom_text"] = symptom
        if case_id is not None:
            body["case_id"] = case_id
        write_json(self.case, body)

    def scored(self, hyps, rubric, symptom=None, matcher=None, case_id=None, name="run"):
        self.set_case(rubric, symptom=symptom, case_id=case_id)
        run = self.report(self.new_run(name), hyps)
        argv = ["score", "--run", run, "--case", self.case, "--json"]
        if matcher is not None:
            argv += ["--matcher", matcher]
        code, out, err = self.call(*argv)
        self.assertEqual(code, 0, err)
        return json.loads(out)

    def test_negated_match_is_discarded(self):
        rubric = {"facts": {"invalid_id": [r"invalid\s+id", r"ffffffff"]}, "hit": ["invalid_id"]}
        hyps = [
            hyp("n1", claim="is not an invalid id"),
            hyp("n2", claim="this was never ffffffff"),
            hyp("p1", claim="returns an invalid id"),
        ]
        obj = self.scored(hyps, rubric)
        rows = by_id(obj)
        self.assertEqual(rows["blind-skeptic:n1"]["facts"], [])
        self.assertEqual(rows["blind-skeptic:n1"]["label"], "MISS")
        self.assertEqual(rows["blind-skeptic:n2"]["facts"], [])
        self.assertEqual(rows["blind-skeptic:p1"]["facts"], ["invalid_id"])
        self.assertEqual(rows["blind-skeptic:p1"]["label"], "HIT")
        self.assertEqual(obj["matcher"], "v2")
        v1 = by_id(self.scored(hyps, rubric, matcher="v1", name="neg_v1"))
        self.assertEqual(v1["blind-skeptic:n1"]["facts"], ["invalid_id"])

    def test_negation_window_is_one_sentence_and_six_words(self):
        rubric = {"facts": {"invalid_id": [r"invalid\s+id"]}, "hit": ["invalid_id"]}
        obj = self.scored([
            hyp("s1", claim="No retry here. The loader returns an invalid id"),
            hyp("s2", claim="not a single retry happened before the loader returned an invalid id"),
            hyp("s3", claim="it doesn't return an invalid id"),
            hyp("s4", claim="not an invalid id. An invalid id was returned"),
        ], rubric)
        rows = by_id(obj)
        self.assertEqual(rows["blind-skeptic:s1"]["label"], "HIT")
        self.assertEqual(rows["blind-skeptic:s2"]["label"], "HIT")
        self.assertEqual(rows["blind-skeptic:s3"]["facts"], [])
        self.assertEqual(rows["blind-skeptic:s3"]["label"], "MISS")
        self.assertEqual(rows["blind-skeptic:s4"]["facts"], ["invalid_id"])
        self.assertEqual(obj["matcher"], "v2")

    def test_echo_of_the_symptom_is_masked_before_matching(self):
        rubric = {"facts": {"follower_queue_entry": ["companion", "queue"]}, "hit": ["follower_queue_entry"]}
        symptom = "When a companion is in my party and I queue a spell"
        hyps = [
            hyp("e1", claim="when a companion is in my party and I queue a spell it recasts"),
            hyp("e2", claim="the companion queue entry"),
        ]
        obj = self.scored(hyps, rubric, symptom=symptom)
        rows = by_id(obj)
        self.assertEqual(rows["blind-skeptic:e1"]["facts"], [])
        self.assertEqual(rows["blind-skeptic:e1"]["label"], "MISS")
        self.assertEqual(rows["blind-skeptic:e2"]["facts"], ["follower_queue_entry"])
        self.assertEqual(rows["blind-skeptic:e2"]["label"], "HIT")
        self.assertEqual(obj["matcher"], "v2")
        v1 = by_id(self.scored(hyps, rubric, symptom=symptom, matcher="v1", name="echo_v1"))
        self.assertEqual(v1["blind-skeptic:e1"]["facts"], ["follower_queue_entry"])

    def test_echo_mask_needs_a_four_word_run(self):
        rubric = {"facts": {"queue": [r"queue"]}, "hit": ["queue"]}
        rows = by_id(self.scored([
            hyp("t1", claim="queue a spell"),
            hyp("t2", claim="queue a spell now"),
        ], rubric, symptom="queue a spell now"))
        self.assertEqual(rows["blind-skeptic:t1"]["facts"], ["queue"])
        self.assertEqual(rows["blind-skeptic:t2"]["facts"], [])

    def test_hedge_flag_is_informational_and_listed(self):
        rubric = {"facts": {"recast": [r"already\s+cast"]}, "hit": ["recast"]}
        obj = self.scored([
            hyp("g1", claim="the spell might have already cast"),
            hyp("g2", claim="the spell has already cast"),
            hyp("g3", claim="the spell may be unrelated. The spell already cast"),
            hyp("g4", claim="it might not have already cast"),
        ], rubric)
        rows = by_id(obj)
        self.assertEqual(rows["blind-skeptic:g1"]["label"], "HIT")
        self.assertEqual(rows["blind-skeptic:g1"]["facts"], ["recast"])
        self.assertEqual(rows["blind-skeptic:g1"].get("hedged_facts"), ["recast"])
        self.assertEqual(rows["blind-skeptic:g2"]["label"], "HIT")
        self.assertEqual(rows["blind-skeptic:g2"].get("hedged_facts"), [])
        self.assertEqual(rows["blind-skeptic:g3"]["label"], "HIT")
        self.assertEqual(rows["blind-skeptic:g3"].get("hedged_facts"), [])
        self.assertEqual(rows["blind-skeptic:g4"]["facts"], [])
        self.assertEqual(rows["blind-skeptic:g4"]["label"], "MISS")
        self.assertEqual(obj["matcher"], "v2")

    def test_empty_or_blank_pattern_exits_2_and_writes_nothing(self):
        run = self.report(self.new_run("blank"), [hyp("h1", claim="stale slot queue length")])
        for facts in ({"a": [""]}, {"a": ["   "]}, {"a": ["x", "\t"]}):
            with self.subTest(facts=facts):
                self.set_case({"facts": facts, "hit": ["a"]})
                before = listing(run)
                p = self.cli("score", "--run", run, "--case", self.case)
                self.assertEqual(p.returncode, 2, p.stderr)
                self.assertIn("pattern", p.stderr)
                self.assertFalse(os.path.exists(os.path.join(run, "score.json")))
                self.assertEqual(listing(run), before)

    def test_matcher_flag_defaults_to_v2_and_rejects_other_names(self):
        run = self.report(self.new_run("mflag"), [hyp("h1", claim="stale slot queue length")])
        self.set_rubric(RUBRIC)
        s = self.score(run)
        self.assertEqual(s.get("matcher"), "v2")
        p = self.cli("score", "--run", run, "--case", self.case, "--matcher", "v3")
        self.assertEqual(p.returncode, 2, p.stderr)

    def test_v1_output_matches_pre_change_golden(self):
        self.set_case(GOLDEN_RUBRIC, symptom=GOLDEN_SYMPTOM, case_id="golden_synthetic")
        run = self.report(self.new_run("golden_run"), golden_hyps())
        code, out, err = self.call("score", "--run", run, "--case", self.case, "--matcher", "v1", "--json")
        self.assertEqual(code, 0, err)
        obj = json.loads(out)
        self.assertEqual(obj["matcher"], "v1")
        for h in obj["hypotheses"]:
            self.assertNotIn("hedged_facts", h)
        stripped = {k: v for k, v in obj.items() if k not in NEW_KEYS}
        self.assertEqual(canon(stripped), canon(json.loads(GOLDEN_V1)))

    def test_v2_is_byte_identical_across_calls_and_default(self):
        self.set_case(GOLDEN_RUBRIC, symptom=GOLDEN_SYMPTOM)
        hyps = golden_hyps() + [hyp("e1", claim="when a companion is in my party and I queue a spell it recasts")]
        run = self.report(self.new_run("v2det"), hyps)
        first = self.cli("score", "--run", run, "--case", self.case, "--matcher", "v2", "--json")
        self.assertEqual(first.returncode, 0, first.stderr)
        b1 = open(os.path.join(run, "score.json"), "rb").read()
        second = self.cli("score", "--run", run, "--case", self.case, "--matcher", "v2", "--json")
        self.assertEqual(second.returncode, 0, second.stderr)
        b2 = open(os.path.join(run, "score.json"), "rb").read()
        self.assertEqual(b1, b2)
        self.assertEqual(first.stdout, second.stdout)
        default = self.cli("score", "--run", run, "--case", self.case, "--json")
        self.assertEqual(default.returncode, 0, default.stderr)
        self.assertEqual(open(os.path.join(run, "score.json"), "rb").read(), b1)

    def test_rubric_sha256_is_the_canonical_json_hash(self):
        rubric_a = {"facts": {"stale": [r"stale\s+slot"], "queue": [r"queue"]}, "hit": ["stale", "queue"]}
        rubric_b = {"hit": ["stale", "queue"], "facts": {"queue": [r"queue"], "stale": [r"stale\s+slot"]}}
        expected = hashlib.sha256(
            json.dumps(rubric_a, sort_keys=True, separators=(",", ":")).encode("utf-8")).hexdigest()
        obj = self.scored([hyp("h1", claim="stale slot")], rubric_a)
        self.assertEqual(obj.get("rubric_sha256"), expected)
        obj_b = self.scored([hyp("h1", claim="stale slot")], rubric_b, name="sha_b")
        self.assertEqual(obj_b.get("rubric_sha256"), expected)

    def test_case_id_comes_from_the_case_file_or_is_null(self):
        obj = self.scored([hyp("h1", claim="stale slot")], RUBRIC, case_id="hotbar_case_1")
        self.assertEqual(obj.get("case_id"), "hotbar_case_1")
        bare = self.scored([hyp("h1", claim="stale slot")], RUBRIC, name="no_case_id")
        self.assertIn("case_id", bare)
        self.assertIsNone(bare["case_id"])


# ---------------------------------------------------------------- scorer statistics: rank-1, bootstrap, mixing, cost
COUNTS_5 = [(3, 9), (2, 9), (4, 10), (1, 8), (3, 11)]   # (HIT, n) per run, synthetic
PINNED_LO = 0.19    # seed-0 percentile 2.5 of COUNTS_5 (0.190476), cross-checked by an independent resample
PINNED_HI = 0.347   # seed-0 percentile 97.5 of COUNTS_5 (0.346939)


class TestScorerStatistics(Base):
    def write_case(self, name, rubric=RUBRIC, **extra):
        body = {"score_rubric": rubric}
        body.update(extra)
        path = os.path.join(self.tmp, "cases", name)
        write_json(path, body)
        return path

    def score_with(self, run, case, matcher=None):
        argv = ["score", "--run", run, "--case", case, "--json"]
        if matcher is not None:
            argv += ["--matcher", matcher]
        code, _, err = self.call(*argv)
        self.assertEqual(code, 0, err)

    def run_with_hits(self, name, k, n):
        """A synthetic run with k HIT hypotheses out of n quote-valid ones."""
        hyps = [hyp("h%d" % i, claim="stale slot queue length" if i < k else "nothing here", evidence=("e%d" % i,))
                for i in range(n)]
        return self.report(self.new_run(name), hyps)

    # rank-1: a run with no top counts as a miss over all runs
    def test_run_without_a_top_lowers_p_all_runs(self):
        r1 = self.report(self.new_run("top1"), [hyp("h1", claim="stale slot queue length")])
        r2 = self.report(self.new_run("top0"), [hyp("m1", claim="stale slot queue length", quote_valid=False)])
        a = self.aggregate(r1, r2)["rank1"]["A"]
        self.assertEqual(a.get("runs"), 2)
        self.assertEqual(a.get("runs_with_top"), 1)
        self.assertEqual(a.get("HIT"), 1)
        self.assertEqual(a.get("p_all_runs"), 0.5)
        self.assertEqual(a.get("p_among_top"), 1.0)
        self.assertEqual(a.get("p"), 1.0)   # kept: old key, equal to p_among_top

    # cluster bootstrap over runs
    def test_cluster_bootstrap_ci_is_seeded_and_pinned(self):
        fn = getattr(bis, "cluster_bootstrap_ci", None)
        self.assertIsNotNone(fn, "bis.cluster_bootstrap_ci is missing")
        first, second = fn(COUNTS_5), fn(COUNTS_5)
        self.assertEqual(first, second)
        self.assertEqual((first["B"], first["seed"], first["n_runs"]), (2000, 0, 5))
        self.assertEqual(round(first["lo"], 3), PINNED_LO)
        self.assertEqual(round(first["hi"], 3), PINNED_HI)
        self.assertTrue(first["lo"] < 13 / 47 < first["hi"])   # pooled 13/47 sits inside the cluster CI

    def test_cluster_ci_and_per_run_proportions_sit_beside_wilson(self):
        runs = [self.run_with_hits("c1", 2, 4), self.run_with_hits("c2", 1, 3), self.run_with_hits("c3", 0, 2)]
        agg = self.aggregate(*runs)
        for cfg in ("config_A", "config_B"):
            with self.subTest(config=cfg):
                block = agg[cfg]
                self.assertIsNotNone(block.get("precision_hit_cluster_ci"))
                self.assertIsNotNone(block["precision_hit"]["wilson95"])
                per_run = block.get("per_run_precision_hit")
                self.assertEqual([(r["k"], r["n"]) for r in per_run], [(2, 4), (1, 3), (0, 2)])

    # tie-break and report agreement
    def test_tie_break_by_unit_id_decides_rank1_label(self):
        run = self.report(self.new_run("tie"), [
            hyp("h1", claim="nothing here", unit="blind-advocate"),
            hyp("h1", claim="stale slot queue length", unit="blind-skeptic"),
        ], ranked=["blind-advocate:h1", "blind-skeptic:h1"])
        s = self.score(run)
        self.assertEqual(s["top_ranked"]["B"]["id"], "blind-advocate:h1")
        self.assertEqual(s.get("rank1_alt_label"), "HIT")
        self.assertIs(s.get("top_matches_report"), True)
        self.assertEqual(self.aggregate(run)["rank1"].get("ties_decide_runs"), 1)

    def test_report_ranked_top_that_differs_from_scorer_is_counted(self):
        run = self.report(self.new_run("mis"), [
            hyp("h1", claim="stale slot queue length", evidence=("e1", "e2", "e3")),
            hyp("h2", claim="stale slot", evidence=("e1",)),
        ], ranked=["blind-skeptic:h2", "blind-skeptic:h1"])
        self.assertIs(self.score(run).get("top_matches_report"), False)
        self.assertEqual(self.aggregate(run)["rank1"].get("rank_mismatch_runs"), 1)

    def test_empty_ranked_counts_as_matching_the_report(self):
        run = self.report(self.new_run("emp"), [hyp("h1", claim="stale slot")], ranked=[])
        self.assertIs(self.score(run).get("top_matches_report"), True)

    def test_distinct_texts_is_preferred_over_raw_evidence_count(self):
        run = self.report(self.new_run("dt"), [
            hyp("h1", claim="stale slot queue length", evidence=("e1", "e2", "e3"), distinct_texts=1),
            hyp("h2", claim="stale slot queue length", evidence=("e1", "e2")),
        ])
        s = self.score(run)
        rows = by_id(s)
        self.assertEqual(rows["blind-skeptic:h1"]["evidence_ids"], 1)
        self.assertEqual(rows["blind-skeptic:h2"]["evidence_ids"], 2)
        self.assertEqual(s["top_ranked"]["B"]["id"], "blind-skeptic:h2")

    # precision over all hypotheses and the per-style table
    def test_precision_hit_all_counts_quote_invalid_as_non_hit(self):
        run = self.report(self.new_run("pha"), [
            hyp("h1", claim="stale slot queue length"),
            hyp("m1", claim="stale slot queue length", quote_valid=False),
            hyp("m2", claim="nothing", quote_valid=False),
        ])
        a = self.aggregate(run)["config_A"]
        self.assertEqual((a["precision_hit"]["k"], a["precision_hit"]["n"]), (1, 1))
        pha = a.get("precision_hit_all")
        self.assertIsNotNone(pha)
        self.assertEqual((pha["k"], pha["n"]), (1, 3))
        self.assertAlmostEqual(pha["p"], 1 / 3, places=6)

    def test_per_style_table_counts_labels_quote_and_verify_coverage(self):
        run = self.report(self.new_run("ps"), [
            hyp("h1", claim="stale slot queue length", unit="blind-advocate", verifier=verdict("supported", True)),
            hyp("h2", claim="stale slot", unit="blind-advocate"),
            hyp("h3", claim="nothing", unit="blind-advocate", quote_valid=False),
            hyp("e1", claim="queue length", unit="blind-evidence"),
            hyp("s1", claim="stale slot queue length", unit="blind-skeptic", status="rejected",
                verifier=verdict("rejected", True)),
            hyp("s2", claim="nothing", unit="blind-skeptic"),
        ])
        ps = self.aggregate(run).get("per_style")
        self.assertIsNotNone(ps)
        self.assertEqual(ps.get("advocate"), {"n": 3, "quote_valid": 2, "HIT": 1, "PARTIAL": 1, "MISS": 1,
                                              "verify": {"complete": 1, "queued": 2}})
        self.assertEqual(ps.get("evidence"), {"n": 1, "quote_valid": 1, "HIT": 0, "PARTIAL": 1, "MISS": 0,
                                              "verify": {"complete": 0, "queued": 1}})
        self.assertEqual(ps.get("skeptic"), {"n": 2, "quote_valid": 2, "HIT": 1, "PARTIAL": 0, "MISS": 1,
                                             "verify": {"complete": 1, "queued": 2}})

    # mixing guard
    def test_aggregate_refuses_mixed_case_id_unless_allow_mixed(self):
        case_a = self.write_case("case_a.json", case_id="hotbar_case_1")
        case_b = self.write_case("case_b.json", case_id="queue_case_2")
        r1 = self.report(self.new_run("ci1"), [hyp("h1", claim="stale slot queue length")])
        r2 = self.report(self.new_run("ci2"), [hyp("h2", claim="stale slot queue length")])
        self.score_with(r1, case_a)
        self.score_with(r2, case_b)
        code, out, err = self.call("aggregate", "--run", r1, "--run", r2, "--json")
        self.assertEqual(code, 2, err)
        self.assertIn('"hotbar_case_1"', err)
        self.assertIn('"queue_case_2"', err)
        self.assertEqual(out, "")
        code, out, err = self.call("aggregate", "--run", r1, "--run", r2, "--json", "--allow-mixed")
        self.assertEqual(code, 0, err)
        self.assertEqual(json.loads(out)["provenance"]["case_id"], ['"hotbar_case_1"', '"queue_case_2"'])

    def test_aggregate_refuses_mixed_rubric_and_matcher(self):
        other = self.write_case("rubric_other.json", rubric={"facts": {"stale": [r"stale\s+slot"]}, "hit": ["stale"]})
        r1 = self.report(self.new_run("rm1"), [hyp("h1", claim="stale slot queue length")])
        r2 = self.report(self.new_run("rm2"), [hyp("h2", claim="stale slot queue length")])
        r3 = self.report(self.new_run("rm3"), [hyp("h3", claim="stale slot queue length")])
        self.score_with(r1, self.case)
        self.score_with(r2, other)
        self.score_with(r3, self.case, matcher="v1")
        code, _, err = self.call("aggregate", "--run", r1, "--run", r2, "--json")
        self.assertEqual(code, 2, err)
        self.assertIn("rubric_sha256", err)
        code, _, err = self.call("aggregate", "--run", r1, "--run", r3, "--json")
        self.assertEqual(code, 2, err)
        self.assertIn("matcher", err)
        self.assertIn('"v1"', err)

    def test_missing_key_is_one_unnamed_value(self):
        old1 = self.report(self.new_run("old1"), [hyp("h1", claim="stale slot queue length")])
        old2 = self.report(self.new_run("old2"), [hyp("h2", claim="stale slot queue length")])
        new = self.report(self.new_run("new1"), [hyp("h3", claim="stale slot queue length")])
        for r in (old1, old2):
            self.score(r)
            obj = read_json(os.path.join(r, "score.json"))
            for key in ("case_id", "matcher", "rubric_sha256", "top_matches_report", "rank1_alt_label"):
                obj.pop(key, None)   # a pre-change score.json
            write_json(os.path.join(r, "score.json"), obj)
        code, _, err = self.call("aggregate", "--run", old1, "--run", old2, "--json")
        self.assertEqual(code, 0, err)   # two old files agree: one unnamed value per key
        self.score(new)
        code, _, err = self.call("aggregate", "--run", old1, "--run", new, "--json")
        self.assertEqual(code, 2, err)
        self.assertIn("<missing>", err)

    # cost completeness gate
    def test_cost_complete_needs_three_blind_and_one_null_record(self):
        run = self.report(self.new_run("cg"), [hyp("h1", claim="stale slot queue length")])
        for name in ("a", "b"):
            self.unit(run, "blind", name, {"cost_usd": 0.1, "duration_s": 1})
        self.assertIs(self.score(run)["cost"]["cost_complete"], False)   # 2 blind: short
        self.unit(run, "blind", "c", {"cost_usd": 0.1, "duration_s": 1})
        self.assertIs(self.score(run)["cost"]["cost_complete"], False)   # 3 blind, no null: short
        self.unit(run, "null", "n1", {"cost_usd": 0.05, "duration_s": 1})
        self.assertIs(self.score(run)["cost"]["cost_complete"], True)

    def test_aggregate_cost_is_incomplete_when_any_run_is(self):
        full = self.report(self.new_run("cf"), [hyp("h1", claim="stale slot queue length")])
        for name in ("a", "b", "c"):
            self.unit(full, "blind", name, {"cost_usd": 0.1, "duration_s": 1})
        self.unit(full, "null", "n1", {"cost_usd": 0.05, "duration_s": 1})
        short = self.report(self.new_run("cs"), [hyp("h2", claim="stale slot queue length")])
        self.unit(short, "blind", "a", {"cost_usd": 0.1, "duration_s": 1})
        self.assertIs(self.aggregate(full)["cost"]["cost_complete"], True)
        self.assertIs(self.aggregate(full, short)["cost"]["cost_complete"], False)


# ---------------------------------------------------------------- held-out sample (synthetic reports only)
class TestSample(Base):
    def valid_runs(self, n_runs=3, per_run=10):
        """n_runs synthetic runs: per_run quote-valid hypotheses each, plus one quote-invalid one per run."""
        runs = []
        for r in range(n_runs):
            hyps = [hyp("h%d" % i) for i in range(1, per_run + 1)] + [hyp("bad", quote_valid=False)]
            runs.append(self.report(self.new_run("run%d" % r), hyps))
        return runs

    def expected_pool(self, n_runs, per_run):
        return sorted(("run%d" % r, "blind-skeptic:h%d" % i) for r in range(n_runs) for i in range(1, per_run + 1))

    def sample(self, runs, n, seed, exclude=None):
        argv = ["sample"]
        for r in runs:
            argv += ["--run", r]
        argv += ["--n", str(n), "--seed", str(seed)]
        if exclude is not None:
            argv += ["--exclude", exclude]
        return self.call(*argv)

    def test_sample_is_identical_for_fixed_inputs_and_seed(self):
        runs = self.valid_runs()
        code1, out1, err1 = self.sample(runs, 5, 11)
        code2, out2, _ = self.sample(runs, 5, 11)
        self.assertEqual((code1, err1), (0, ""))
        self.assertEqual(out1, out2)
        drawn = json.loads(out1)
        self.assertEqual(len(drawn), 5)
        self.assertEqual(len({tuple(p) for p in drawn}), 5)

    def test_sample_different_seed_gives_a_different_draw(self):
        runs = self.valid_runs()
        code1, out1, err1 = self.sample(runs, 5, 1)
        code2, out2, err2 = self.sample(runs, 5, 2)
        self.assertEqual((code1, code2), (0, 0), err1 + err2)
        self.assertNotEqual(out1, out2)

    def test_sample_draws_rng_sample_over_the_sorted_pool(self):
        """Pinned: the pool is sorted by (run basename, hypothesis id) first, then random.Random(seed).sample draws."""
        runs = self.valid_runs(n_runs=3, per_run=10)
        code, out, err = self.sample(runs, 6, 7)
        self.assertEqual(code, 0, err)
        self.assertEqual(json.loads(out), [list(p) for p in random.Random(7).sample(self.expected_pool(3, 10), 6)])

    def test_sample_never_draws_a_quote_invalid_hypothesis(self):
        runs = self.valid_runs(n_runs=2, per_run=4)
        code, out, err = self.sample(runs, 100, 3)
        self.assertEqual(code, 0, err)
        drawn = json.loads(out)
        self.assertNotIn(["run0", "blind-skeptic:bad"], drawn)
        self.assertEqual(sorted(tuple(p) for p in drawn), self.expected_pool(2, 4))

    def test_sample_exclusion_matches_run_basename_and_id(self):
        runs = self.valid_runs(n_runs=2, per_run=4)
        excl = os.path.join(self.tmp, "exclude.json")
        write_json(excl, [["run0", "blind-skeptic:h1"], ["run1", "blind-skeptic:h2"]])
        code, out, err = self.sample(runs, 100, 3, exclude=excl)
        self.assertEqual(code, 0, err)
        gone = {("run0", "blind-skeptic:h1"), ("run1", "blind-skeptic:h2")}
        self.assertEqual([tuple(p) for p in json.loads(out)],
                         [p for p in self.expected_pool(2, 4) if p not in gone])

    def test_sample_n_above_the_pool_takes_all_and_says_so(self):
        runs = self.valid_runs(n_runs=2, per_run=4)
        code, out, err = self.sample(runs, 50, 3)
        self.assertEqual(code, 0, err)
        self.assertEqual([tuple(p) for p in json.loads(out)], self.expected_pool(2, 4))
        self.assertIn("taking all 8", err)

    def test_sample_writes_nothing(self):
        runs = self.valid_runs(n_runs=2, per_run=4)
        before = listing(self.tmp)
        code, _, err = self.sample(runs, 3, 1)
        self.assertEqual(code, 0, err)
        self.assertEqual(listing(self.tmp), before)

    def test_sample_negative_n_exits_2_and_prints_nothing(self):
        runs = self.valid_runs(n_runs=1, per_run=4)
        code, out, err = self.sample(runs, -1, 1)
        self.assertEqual(code, 2)
        self.assertEqual(out, "")
        self.assertIn("--n", err)

    def test_sample_malformed_exclude_file_exits_2(self):
        runs = self.valid_runs(n_runs=1, per_run=4)
        excl = os.path.join(self.tmp, "exclude.json")
        write_json(excl, [["run0", "blind-skeptic:h1", "HIT"]])
        code, out, err = self.sample(runs, 2, 1, exclude=excl)
        self.assertEqual(code, 2)
        self.assertEqual(out, "")
        self.assertIn("exclude", err)

    def test_sample_refuses_two_run_dirs_with_one_basename(self):
        a = self.report(self.new_run("alpha"), [hyp("h1")])
        b = self.report(os.path.join(self.tmp, "other", "alpha"), [hyp("h1")])
        code, out, err = self.sample([a, b], 1, 1)
        self.assertEqual(code, 2)
        self.assertIn("alpha", err)


# ---------------------------------------------------------------- agreement with score.json (synthetic)
# (given label, scorer label) per item. Run kappa_a holds items 1-5, kappa_b items 6-10.
KAPPA_PAIRS = [
    ("HIT", "HIT"), ("HIT", "HIT"), ("HIT", "HIT"), ("HIT", "PARTIAL"), ("HIT", "MISS"),
    ("PARTIAL", "PARTIAL"), ("PARTIAL", "PARTIAL"), ("PARTIAL", "MISS"),
    ("MISS", "MISS"), ("MISS", "MISS"),
]
KAPPA_PLAN = [("kappa_a", KAPPA_PAIRS[:5]), ("kappa_b", KAPPA_PAIRS[5:])]
# claims that the real scorer (RUBRIC above) labels HIT, PARTIAL and MISS
CLAIM_FOR = {"HIT": "stale slot with queue length", "PARTIAL": "a stale slot only", "MISS": "unrelated noise"}


class TestAgree(Base):
    def scored(self, plan):
        """plan: [(run name, [(given, scorer), ...]), ...]. One synthetic report per run, each scored by the real
        scorer (the claim is written to score as the scorer label). Returns (run dirs, {run/id: given label})."""
        runs, labels, k = [], {}, 0
        for name, items in plan:
            hyps = []
            for given, scorer in items:
                k += 1
                hyps.append(hyp("h%d" % k, claim=CLAIM_FOR[scorer]))
                labels["%s/blind-skeptic:h%d" % (name, k)] = given
            run = self.report(self.new_run(name), hyps)
            self.score(run)
            runs.append(run)
        return runs, labels

    def labels_file(self, obj):
        path = os.path.join(self.tmp, "labels.json")
        write_json(path, obj)
        return path

    def agree(self, labels_path, runs, *extra):
        argv = ["agree", "--labels", labels_path]
        for r in runs:
            argv += ["--run", r]
        return self.call(*(argv + list(extra)))

    def test_kappa_hand_computed_on_ten_items(self):
        # Confusion, rows = given, columns = scorer (HIT, PARTIAL, MISS):
        #   given HIT     : 3 1 1   (5 items)
        #   given PARTIAL : 0 2 1   (3 items)
        #   given MISS    : 0 0 2   (2 items)
        # Given marginals 5, 3, 2; scorer marginals 3, 3, 4; n = 10.
        # p_o = (3 + 2 + 2) / 10 = 0.7
        # p_e = (5*3 + 3*3 + 2*4) / 10^2 = 32 / 100 = 0.32
        # kappa = (p_o - p_e) / (1 - p_e) = 0.38 / 0.68 = 19/34 = 0.558823... -> 0.559
        runs, labels = self.scored(KAPPA_PLAN)
        code, out, err = self.agree(self.labels_file(labels), runs, "--case", self.case, "--json")
        self.assertEqual(code, 0, err)
        a = json.loads(out)
        self.assertEqual((a["n"], a["agree"]), (10, 7))
        self.assertAlmostEqual(a["raw_agreement"], 0.7, places=9)
        self.assertEqual(round(a["kappa"], 3), 0.559)
        self.assertAlmostEqual(a["kappa"], 19 / 34, places=5)
        self.assertEqual(a["given_counts"], {"HIT": 5, "PARTIAL": 3, "MISS": 2})
        self.assertEqual(a["scorer_counts"], {"HIT": 3, "PARTIAL": 3, "MISS": 4})
        self.assertEqual(a["confusion"], {"HIT": {"HIT": 3, "PARTIAL": 1, "MISS": 1},
                                          "PARTIAL": {"HIT": 0, "PARTIAL": 2, "MISS": 1},
                                          "MISS": {"HIT": 0, "PARTIAL": 0, "MISS": 2}})

    def test_identical_lists_have_kappa_one(self):
        plan = [(name, [(s, s) for _, s in items]) for name, items in KAPPA_PLAN]
        runs, labels = self.scored(plan)
        code, out, err = self.agree(self.labels_file(labels), runs, "--json")
        self.assertEqual(code, 0, err)
        a = json.loads(out)
        self.assertEqual(a["kappa"], 1.0)
        self.assertEqual(a["raw_agreement"], 1.0)
        self.assertTrue(all(a["confusion"][g][s] == 0 for g in bis.LABELS for s in bis.LABELS if g != s))

    def test_identical_single_label_lists_have_kappa_one_not_na(self):
        # Expected agreement is 1 here (every label is HIT); the identical-lists rule still gives 1.0, not n/a.
        runs, labels = self.scored([("solo", [("HIT", "HIT")] * 3)])
        code, out, err = self.agree(self.labels_file(labels), runs, "--json")
        self.assertEqual(code, 0, err)
        self.assertEqual(json.loads(out)["kappa"], 1.0)

    def test_list_form_of_labels_gives_the_same_result(self):
        runs, labels = self.scored(KAPPA_PLAN)
        as_dict = self.agree(self.labels_file(labels), runs, "--json")
        as_list = [[k.split("/", 1)[0], k.split("/", 1)[1], v] for k, v in sorted(labels.items())]
        as_list_run = self.agree(self.labels_file(as_list), runs, "--json")
        self.assertEqual(as_dict[0], 0, as_dict[2])
        self.assertEqual(as_list_run[0], 0, as_list_run[2])
        self.assertEqual(json.loads(as_dict[1]), json.loads(as_list_run[1]))

    def test_label_for_an_unscored_id_exits_2_and_names_it(self):
        runs, labels = self.scored(KAPPA_PLAN)
        labels["kappa_a/blind-skeptic:h99"] = "HIT"
        code, out, err = self.agree(self.labels_file(labels), runs, "--json")
        self.assertEqual(code, 2)
        self.assertEqual(out, "")
        self.assertIn("blind-skeptic:h99", err)

    def test_missing_score_json_exits_2_and_names_a_labelled_id(self):
        runs, labels = self.scored(KAPPA_PLAN)
        os.remove(os.path.join(runs[1], "score.json"))
        code, out, err = self.agree(self.labels_file(labels), runs, "--json")
        self.assertEqual(code, 2)
        self.assertTrue(any("kappa_b/blind-skeptic:h%d" % k in err for k in range(6, 11)), err)
        self.assertFalse(os.path.exists(os.path.join(runs[1], "score.json")))

    def test_runs_scored_with_different_matchers_are_refused(self):
        runs, labels = self.scored(KAPPA_PLAN)
        code, _, err = self.call("score", "--run", runs[1], "--case", self.case, "--matcher", "v1", "--json")
        self.assertEqual(code, 0, err)
        code, out, err = self.agree(self.labels_file(labels), runs, "--json")
        self.assertEqual(code, 2)
        self.assertIn("matcher", err)

    def test_case_with_another_rubric_is_refused(self):
        runs, labels = self.scored(KAPPA_PLAN)
        other = os.path.join(self.tmp, "other_case.json")
        write_json(other, {"id": "other", "score_rubric": {"facts": {"stale": [r"stale"]}, "hit": ["stale"]}})
        code, out, err = self.agree(self.labels_file(labels), runs, "--case", other, "--json")
        self.assertEqual(code, 2)
        self.assertIn("rubric", err)

    def test_label_outside_hit_partial_miss_exits_2_and_names_the_id(self):
        runs, labels = self.scored(KAPPA_PLAN)
        labels["kappa_a/blind-skeptic:h1"] = "hit"
        code, out, err = self.agree(self.labels_file(labels), runs, "--json")
        self.assertEqual(code, 2)
        self.assertIn("kappa_a/blind-skeptic:h1", err)

    def test_label_whose_run_is_not_given_exits_2_and_names_the_id(self):
        runs, labels = self.scored(KAPPA_PLAN)
        code, out, err = self.agree(self.labels_file(labels), runs[:1], "--json")
        self.assertEqual(code, 2)
        self.assertTrue(any("kappa_b/blind-skeptic:h%d" % k in err for k in range(6, 11)), err)

    def test_agree_writes_nothing(self):
        runs, labels = self.scored(KAPPA_PLAN)
        path = self.labels_file(labels)
        before = listing(self.tmp)
        code, _, err = self.agree(path, runs, "--json")
        self.assertEqual(code, 0, err)
        self.assertEqual(listing(self.tmp), before)

    def test_summary_prints_kappa_and_the_confusion_table(self):
        runs, labels = self.scored(KAPPA_PLAN)
        code, out, err = self.agree(self.labels_file(labels), runs)
        self.assertEqual(code, 0, err)
        self.assertIn("raw agreement 0.700 (7/10)", out)
        self.assertIn("Cohen's kappa 0.559", out)
        rows = [line.split() for line in out.splitlines()]
        for expected in (["HIT", "3", "1", "1"], ["PARTIAL", "0", "2", "1"], ["MISS", "0", "0", "2"]):
            self.assertIn(expected, rows)


def main():
    suite = unittest.defaultTestLoader.loadTestsFromModule(sys.modules[__name__])
    res = unittest.TextTestRunner(verbosity=1).run(suite)
    bad = len(res.failures) + len(res.errors)
    print("bug_investigate_score tests: %d/%d %s" % (res.testsRun - bad, res.testsRun, "PASS" if not bad else "FAIL"))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
