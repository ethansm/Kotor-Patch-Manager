"""Tests for router_eval.py: ranking, policies, gates, abort handling, rendering and cost checks.

Run: python3 scripts/run_tests.py router_eval   (or python3 scripts/test_router_eval.py)
All engines are faked; nothing here calls Kev or Claude.
"""
import json
import subprocess
import sys
import tempfile
import threading
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import router_eval as re_  # noqa: E402

CATALOG = re_.Catalog([
    {"id": "map.low", "stage": "map", "rank": 0, "model_effort": "", "description": "low d"},
    {"id": "map.medium", "stage": "map", "rank": 1, "model_effort": "", "description": "med d"},
    {"id": "map.high", "stage": "map", "rank": 2, "model_effort": "", "description": "high d"},
    {"id": "verify.medium", "stage": "verify", "rank": 0, "model_effort": "", "description": "vm"},
    {"id": "verify.high", "stage": "verify", "rank": 1, "model_effort": "", "description": "vh"},
    {"id": "adjudicate.ladder", "stage": "adjudicate", "rank": 0, "model_effort": "", "description": "al"},
    {"id": "escalate.sonnet", "stage": "adjudicate", "rank": 1, "model_effort": "", "description": "es"},
    {"id": "fix.haiku", "stage": "fix", "rank": 0, "model_effort": "", "description": "fh"},
    {"id": "fix.sonnet", "stage": "fix", "rank": 1, "model_effort": "", "description": "fs"},
])
GATES = {"top1_min": 0.80, "hard_underroute_max_rate": 0.10,
         "hard_underroute_max_count_verify_adjudicate": 0, "repeat_agreement_min": 0.90,
         "invalid_max_rate": 0.025, "label_agreement_min": 0.90}


def mk(i=0, **kw):
    case = {"id": f"c{i}", "stage": "map", "files": 2, "lines": 300, "paths": [f"src/a{i}.cpp"],
            "domain": "ui", "task": f"Review packet number {i}.", "gold": "map.medium",
            "hard": False, "why_hard": "", "source": "t", "gold_basis": "rubric", "prior_splits": 0}
    case.update(kw)
    return case


def haiku_reply(choice="map.medium", runner_up="map.high", cost=0.0003, **extra):
    d = {"type": "result", "is_error": False, "duration_ms": 900, "total_cost_usd": cost,
         "structured_output": {"choice": choice, "runner_up": runner_up, "confidence": 0.7,
                               "rationale": "r"},
         "usage": {"input_tokens": 100, "cache_creation_input_tokens": 0,
                   "cache_read_input_tokens": 1000, "output_tokens": 50,
                   "output_tokens_details": {"thinking_tokens": 10}}}
    d.update(extra)
    return json.dumps(d)


def metrics(**kw):
    m = {"top1": 0.9, "hard_under_rate": 0.0, "hard_under_va": 0, "repeat_agreement": 1.0,
         "invalid_rate": 0.0}
    m.update(kw)
    return m


class TestRankAndRoute(unittest.TestCase):
    def test_classify_under_over_correct(self):
        c = CATALOG
        self.assertEqual(re_.classify("map.low", "map.high", c, "map"), "under")
        self.assertEqual(re_.classify("map.high", "map.low", c, "map"), "over")
        self.assertEqual(re_.classify("map.medium", "map.medium", c, "map"), "correct")

    def test_invalid_choices(self):
        c = CATALOG
        self.assertEqual(re_.classify("verify.high", "map.high", c, "map"), "invalid")  # wrong stage
        self.assertEqual(re_.classify("nonsense", "map.high", c, "map"), "invalid")
        self.assertEqual(re_.classify(None, "map.high", c, "map"), "invalid")

    def test_rank_is_within_stage_order(self):
        self.assertEqual(CATALOG.allowed("adjudicate"), ["adjudicate.ladder", "escalate.sonnet"])
        self.assertLess(CATALOG.rank("fix.haiku"), CATALOG.rank("fix.sonnet"))

    def test_rules_route(self):
        r = re_.rules_route
        self.assertEqual(r(mk(paths=["x/_shared_src/G.h"])), "map.high")
        self.assertEqual(r(mk(paths=["x/MyHook.cpp"])), "map.high")
        self.assertEqual(r(mk(paths=["x/a.ASM"])), "map.high")
        self.assertEqual(r(mk(lines=601)), "map.high")
        self.assertEqual(r(mk(paths=["a.md", "b.toml"], lines=900 - 400)), "map.low")
        self.assertEqual(r(mk(lines=150)), "map.low")
        self.assertEqual(r(mk(lines=151)), "map.medium")
        self.assertEqual(r(mk(stage="verify", domain="asm")), "verify.high")
        self.assertEqual(r(mk(stage="verify", domain="ui")), "verify.medium")
        self.assertEqual(r(mk(stage="adjudicate", prior_splits=2)), "escalate.sonnet")
        self.assertEqual(r(mk(stage="adjudicate", prior_splits=1)), "adjudicate.ladder")
        self.assertEqual(r(mk(stage="fix", files=2)), "fix.sonnet")
        self.assertEqual(r(mk(stage="fix", files=1)), "fix.haiku")

    def test_missing_row_is_wrong_and_invalid(self):
        cases = [mk(0, gold="map.medium"), mk(1, gold="map.medium")]
        rows = {"c0": {"choice": "map.medium"}}  # c1 has no row
        m = re_.evaluate_policy("haiku", "P0", cases, rows, {}, CATALOG)
        self.assertEqual((m["correct"], m["invalid"], m["n"]), (1, 1, 2))


class TestPolicies(unittest.TestCase):
    def test_haiku_p1_takes_higher_rank_either_direction(self):
        p1 = re_.haiku_p1
        self.assertEqual(p1("map.low", "map.high", CATALOG, "map"), "map.high")
        self.assertEqual(p1("map.high", "map.low", CATALOG, "map"), "map.high")

    def test_haiku_p1_invalid_runner_up_keeps_choice(self):
        self.assertEqual(re_.haiku_p1("map.low", "verify.high", CATALOG, "map"), "map.low")
        self.assertEqual(re_.haiku_p1("map.low", None, CATALOG, "map"), "map.low")
        self.assertEqual(re_.haiku_p1("bogus", "map.high", CATALOG, "map"), "bogus")

    def _kev(self, p1, p2):
        probs = {"map.low": p1, "map.medium": p2, "map.high": 0.01}
        top2, a, b = re_.kev_top2(probs, CATALOG, "map")
        return re_.kev_p1("map.low", top2, a, b, CATALOG, "map")

    def test_kev_ratio_exactly_point_six_escalates(self):
        self.assertEqual(self._kev(0.5, 0.3), "map.medium")

    def test_kev_ratio_below_point_six_does_not(self):
        self.assertEqual(self._kev(1.0, 0.59), "map.low")

    def test_kev_p1_edge_cases(self):
        self.assertEqual(re_.kev_p1("map.low", [], None, None, CATALOG, "map"), "map.low")
        self.assertEqual(re_.kev_p1("map.low", ["map.low", "map.high"], 0, 0, CATALOG, "map"),
                         "map.low")
        top2, a, b = re_.kev_top2({"map.high": 0.4, "map.low": 0.4, "verify.high": 0.9},
                                  CATALOG, "map")  # other-stage ids ignored
        self.assertEqual(top2, ["map.low", "map.high"])

    def test_policy_choice_dispatch(self):
        row = {"choice": "map.low", "runner_up": "map.high"}
        self.assertEqual(re_.policy_choice("haiku", "P0", row, CATALOG, "map"), "map.low")
        self.assertEqual(re_.policy_choice("haiku", "P1", row, CATALOG, "map"), "map.high")
        self.assertIsNone(re_.policy_choice("haiku", "P1", None, CATALOG, "map"))

    def test_percentile_and_sign_test(self):
        self.assertEqual(re_.percentile([5, 1, 3, 2, 4], 0.5), 3)
        self.assertEqual(re_.percentile([1, 2, 3, 4, 5, 6, 7, 8, 9, 10], 0.9), 9)
        self.assertEqual(re_.exact_sign_p(0, 0), 1.0)
        self.assertAlmostEqual(re_.exact_sign_p(0, 5), 2 / 32)
        self.assertEqual(re_.exact_sign_p(3, 3), 1.0)


class TestGates(unittest.TestCase):
    def test_exact_thresholds_pass(self):
        ok, checks = re_.evaluate_gates(metrics(top1=32 / 40, hard_under_rate=1 / 10,
                                                invalid_rate=1 / 40, repeat_agreement=0.90), GATES)
        self.assertTrue(ok, checks)

    def test_just_past_thresholds_fail(self):
        self.assertFalse(re_.evaluate_gates(metrics(top1=0.79), GATES)[0])
        self.assertFalse(re_.evaluate_gates(metrics(hard_under_rate=0.11), GATES)[0])
        self.assertFalse(re_.evaluate_gates(metrics(invalid_rate=0.05), GATES)[0])
        self.assertFalse(re_.evaluate_gates(metrics(repeat_agreement=0.89), GATES)[0])
        self.assertFalse(re_.evaluate_gates(metrics(hard_under_va=1), GATES)[0])

    def test_missing_repeat_agreement_fails(self):
        self.assertFalse(re_.evaluate_gates(metrics(repeat_agreement=None), GATES)[0])

    def test_wilson_and_label_gate(self):
        lo, hi = re_.wilson(32, 40)
        self.assertTrue(0.6 < lo < 0.8 < hi < 0.95)
        self.assertEqual(re_.wilson(0, 0), (0.0, 0.0))
        cases = [mk(i, gold="map.medium") for i in range(10)]
        with tempfile.TemporaryDirectory() as d:
            labs = [{"id": f"c{i}", "label": "map.medium" if i else "map.low", "ambiguous": False,
                     "note": ""} for i in range(10)]
            Path(d, "map_labels.jsonl").write_text("\n".join(json.dumps(x) for x in labs))
            ok, _ = re_.compare_labels(cases, d, GATES)  # 9/10 == 0.90 passes
            self.assertTrue(ok)
            ok, _ = re_.compare_labels(cases + [mk(10)], d, GATES)  # unlabeled counts against: 9/11
            self.assertFalse(ok)


class _Seq:
    """Fake runner returning successive canned (rc, stdout, stderr), recording call count."""

    def __init__(self, replies):
        self.replies, self.calls, self.lock = replies, 0, threading.Lock()

    def __call__(self, argv, cwd, env, timeout):
        with self.lock:
            i = self.calls
            self.calls += 1
        r = self.replies[min(i, len(self.replies) - 1)] if isinstance(self.replies, list) \
            else self.replies
        if isinstance(r, Exception):
            raise r
        return r


class TestAborts(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.cases = [mk(i) for i in range(20)]

    def _run(self, engines, **kw):
        return re_.run_eval(self.cases, CATALOG, engines, kw.pop("passes", 1), kw.pop("budget", 1.0),
                            self.tmp.name, "r1", router_prompt="PROMPT", haiku_cwd=self.tmp.name,
                            version_fn=lambda: "fake", sleep=lambda s: None, **kw)

    def test_budget_abort_stops_new_launches(self):
        runner = _Seq([(0, haiku_reply(cost=1.0), "")])
        s = self._run(["haiku"], runner=runner, budget=1.0)
        self.assertEqual(s["aborted"], {"haiku": "budget"})
        self.assertEqual(runner.calls, 1)  # first call alone spends the whole budget
        self.assertAlmostEqual(s["haiku_spent_usd"], 1.0)

    def test_three_consecutive_failures_abort(self):
        runner = _Seq([(1, "", "boom")])
        s = self._run(["haiku"], runner=runner)
        self.assertEqual(s["aborted"], {"haiku": "consecutive_failures"})
        self.assertTrue(3 <= runner.calls < len(self.cases))
        self.assertEqual(s["failures"]["haiku"], runner.calls)

    def test_success_resets_failure_streak_and_retry_on_429(self):
        ok = (0, haiku_reply(), "")
        bad = (1, "", "x")
        rate = (0, json.dumps({"is_error": True, "api_error_status": 429}), "")
        runner = _Seq([bad, bad, ok, bad, bad, ok, rate, ok] + [ok])
        s = self._run(["haiku"], runner=runner, workers=1)
        self.assertIsNone(s["aborted"])
        self.assertEqual(s["failures"]["haiku"], 4)
        self.assertEqual(s["calls"]["haiku"], 20)
        self.assertEqual(runner.calls, 21)  # one 429 retry

    def test_timeout_is_failure(self):
        runner = _Seq([subprocess.TimeoutExpired("claude", 180)])
        s = self._run(["haiku"], runner=runner)
        self.assertEqual(s["aborted"], {"haiku": "consecutive_failures"})

    def test_missing_cost_is_none_not_zero(self):
        d = json.loads(haiku_reply())
        del d["total_cost_usd"]
        runner = _Seq([(0, json.dumps(d), "")])
        s = self._run(["haiku"], runner=runner)
        self.assertEqual(s["cost_missing"], 20)
        row = json.loads(Path(self.tmp.name, "r1", "haiku_pass1.jsonl").read_text().splitlines()[0])
        self.assertIsNone(row["cost_usd"])
        self.assertEqual(row["cost_check"], "n/a")

    def test_kev_three_none_aborts(self):
        calls = []

        def kev(payload, timeout):
            calls.append((payload, timeout))
            return None

        s = self._run(["kev"], kev_fn=kev)
        self.assertEqual(s["aborted"], {"kev": "consecutive_none"})
        self.assertEqual(len(calls), 3)
        self.assertEqual(s["kev_none"], 3)
        self.assertEqual(calls[0][1], 5.0)

    def test_kev_none_streak_resets_and_rows_scored(self):
        seq = iter([None, None, {"answers": {"profile": {
            "choice": "map.medium", "probabilities": {"map.medium": 0.5, "map.high": 0.3}}}}])

        def kev(payload, timeout):
            return next(seq, {"answers": {"profile": {"choice": "map.medium"}}})

        s = self._run(["kev"], kev_fn=kev)
        self.assertIsNone(s["aborted"])
        self.assertEqual(s["kev_none"], 2)
        rows = [json.loads(x) for x in
                Path(self.tmp.name, "r1", "kev_pass1.jsonl").read_text().splitlines()]
        self.assertEqual(len(rows), 20)
        self.assertEqual(sum(r["failure"] for r in rows), 2)
        self.assertTrue(all(r["invalid"] for r in rows if r["failure"]))

    def test_kev_missing_choice_is_invalid_not_failure(self):
        row = re_.kev_call(mk(), CATALOG, lambda p, t: {"answers": {}}, 1, 1)
        self.assertFalse(row["failure"])
        self.assertTrue(row["invalid"])

    def test_rules_written_once_and_score_roundtrip(self):
        cases = [mk(i, gold=re_.rules_route(mk(i))) for i in range(4)]
        re_.run_eval(cases, CATALOG, ["rules"], 2, 1.0, self.tmp.name, "r2", router_prompt="P",
                     version_fn=lambda: "fake")
        files = sorted(p.name for p in Path(self.tmp.name, "r2").iterdir())
        self.assertEqual(files, ["rules_pass1.jsonl", "summary.json"])
        res = re_.score_run(Path(self.tmp.name, "r2"), cases, CATALOG, GATES)
        self.assertEqual(res["engines"]["rules"]["policies"]["P0"]["top1"], 1.0)
        self.assertEqual(res["winner"], "rules")
        self.assertLessEqual(len(re_.build_report(res)), 80)

    def test_plan_run_estimate(self):
        calls, est = re_.plan_run(40, ["rules", "kev", "haiku"], 2)
        self.assertEqual(calls, {"rules": 40, "kev": 80, "haiku": 80})
        self.assertAlmostEqual(est, 0.001 + 0.0003 * 79)
        self.assertEqual(re_.plan_run(40, ["rules"], 2)[1], 0.0)


class TestRenderValidateCost(unittest.TestCase):
    def test_render_exact_and_deterministic(self):
        c = mk(files=1, lines=1, paths=["a/b.cpp", "c.h"], domain="hook", task="Do it.")
        want = ("Stage: map\nUnit: 1 file, 1 line\nPaths: a/b.cpp, c.h\nDomain: hook\n"
                "Task: Do it.\nAllowed profiles: map.low | map.medium | map.high")
        self.assertEqual(re_.render_prompt(c, CATALOG), want)
        self.assertEqual(re_.render_prompt(c, CATALOG), re_.render_prompt(dict(c), CATALOG))

    def test_render_adjudicate_has_prior_splits_and_plurals(self):
        c = mk(stage="adjudicate", files=3, lines=0, prior_splits=2, gold="escalate.sonnet")
        out = re_.render_prompt(c, CATALOG).split("\n")
        self.assertEqual(out[1], "Unit: 3 files, 0 lines")
        self.assertEqual(out[4], "Prior judge splits: 2")
        self.assertEqual(out[-1], "Allowed profiles: adjudicate.ladder | escalate.sonnet")
        self.assertNotIn("Prior judge splits", re_.render_prompt(mk(), CATALOG))

    def test_duplicate_prompt_rejected_after_normalization(self):
        a = mk(0, task="Same   task here")
        b = mk(1, id="other", paths=a["paths"], task="same task HERE")
        for i, x in enumerate((a, b), 1):
            x["_line"] = i
        errs = re_.validate_case_lines([a, b], CATALOG)
        self.assertTrue(any("duplicate normalized prompt" in e for e in errs), errs)
        self.assertEqual(re_.validate_case_lines([a, mk(2)], CATALOG), [])

    def test_validation_rules(self):
        bad = [mk(0, hard=True, why_hard=""), mk(1, task="w " * 41), mk(2, gold="verify.high"),
               mk(3, gold_basis="vibes"), mk(4, prior_splits=-1), mk(5, hard="yes")]
        for i, x in enumerate(bad):
            x["_line"] = i
        errs = re_.validate_case_lines(bad, CATALOG)
        self.assertEqual(len({e.split(":")[0] for e in errs}), 6, errs)

    def test_cost_self_check(self):
        calc = re_.calc_cost(100, 200, 1000, 50)
        self.assertAlmostEqual(calc, 100e-7 + 200 * 2e-7 + 1000e-8 + 50 * 5e-7)
        self.assertEqual(re_.cost_check(calc, 100, 200, 1000, 50), "ok")
        self.assertEqual(re_.cost_check(calc + 0.00002 - 1e-9, 100, 200, 1000, 50), "ok")
        self.assertEqual(re_.cost_check(calc + 0.001, 100, 200, 1000, 50), "mismatch")
        self.assertEqual(re_.cost_check(None, 100, 200, 1000, 50), "n/a")

    def test_haiku_argv_and_schema(self):
        schema = json.loads(re_.haiku_schema_json(CATALOG))
        self.assertEqual(schema["properties"]["choice"]["enum"], CATALOG.ids)
        argv = re_.haiku_argv("U", "SYS", "{}")
        self.assertEqual(argv[:3], ["claude", "-p", "U"])
        self.assertEqual(argv[argv.index("--system-prompt") + 1], "SYS")
        self.assertEqual(argv[argv.index("--max-budget-usd") + 1], "0.01")


if __name__ == "__main__":
    unittest.main()
