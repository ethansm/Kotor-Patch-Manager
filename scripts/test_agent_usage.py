#!/usr/bin/env python3
"""Tests for scripts/agent_usage.py (synthetic transcripts in a temp projects dir; no real ~/.claude reads)."""
import contextlib
import datetime as dt
import io
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import agent_usage as A  # noqa: E402

SESSION = "11111111-2222-3333-4444-555555555555"
HAIKU = "claude-haiku-5-5"
SCRIPT = HERE / "agent_usage.py"
DEEP = "[" * 100000 + "]" * 100000


def rec(req, model, inp, cw5=0, cw1=0, cr=0, out=0, ts="2026-10-10T01:00:00.000Z", text=""):
    usage = {"input_tokens": inp, "cache_creation_input_tokens": cw5 + cw1, "cache_read_input_tokens": cr,
             "output_tokens": out,
             "cache_creation": {"ephemeral_5m_input_tokens": cw5, "ephemeral_1h_input_tokens": cw1}}
    msg = {"id": "msg_" + req, "model": model, "usage": usage, "content": [{"type": "text", "text": text}]}
    return {"requestId": req, "timestamp": ts, "type": "assistant", "message": msg}


class Base(unittest.TestCase):
    def setUp(self):
        self.root = Path(tempfile.mkdtemp(prefix="agent_usage_test_"))
        self.addCleanup(shutil.rmtree, self.root, ignore_errors=True)
        self.projects = self.root / "projects"
        self.sub = self.projects / "proj-a" / SESSION / "subagents"
        self.sub.mkdir(parents=True)
        self.saved_rates = A.RATES_PATH
        self.addCleanup(setattr, A, "RATES_PATH", self.saved_rates)

    def agent(self, agent_id, records, meta=None):
        path = self.sub / f"agent-{agent_id}.jsonl"
        with open(path, "w", encoding="utf-8") as fh:
            for r in records:
                fh.write(r if isinstance(r, str) else json.dumps(r))
                fh.write("\n")
        if meta is not None:
            (self.sub / f"agent-{agent_id}.meta.json").write_text(json.dumps(meta), encoding="utf-8")
        return path

    def cli(self, *extra):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            code = A.main(["--session", SESSION, "--projects-dir", str(self.projects), *extra])
        return code, out.getvalue(), err.getvalue()

    def rates(self):
        return A.load_rates(A.RATES_PATH)

    def run_cli(self, *extra, env=None):
        """Subprocess run: real stdout encoding plus a hang guard."""
        e = {k: v for k, v in os.environ.items() if k != "CLAUDE_CODE_SESSION_ID"}
        e.update(env or {})
        p = subprocess.run([sys.executable, str(SCRIPT), "--session", SESSION, "--projects-dir", str(self.projects), *extra],
                           capture_output=True, env=e, timeout=20)
        return p.returncode, p.stdout, p.stderr

    def data(self, *extra):
        code, out, err = self.cli("--json", *extra)
        self.assertEqual(code, 0, err)
        return json.loads(out)


def with_usage(r, **kv):
    r["message"]["usage"].update(kv)
    return r


class TokenMath(Base):
    def test_single_request_peak_and_processed(self):
        self.agent("aaaa1111", [rec("r1", "claude-sonnet-5-5", 10, cw5=20, cr=30, out=5)])
        rows = self.cli("--json")[1]
        agent = json.loads(rows)["agents"][0]
        self.assertEqual(agent["peak_ctx"], 60)
        self.assertEqual(agent["processed"], 65)

    def test_multi_request_peak_is_max_processed_is_sum(self):
        self.agent("aaaa2222", [rec("r1", "claude-sonnet-5-5", 100, out=1),
                                rec("r2", "claude-sonnet-5-5", 200, out=2)])
        agent = json.loads(self.cli("--json")[1])["agents"][0]
        self.assertEqual(agent["peak_ctx"], 200)
        self.assertEqual(agent["processed"], 100 + 1 + 200 + 2)
        self.assertEqual(agent["requests"], 2)

    def test_duplicate_request_id_counted_once_last_wins(self):
        self.agent("aaaa3333", [rec("r1", "claude-sonnet-5-5", 100, out=5),
                                rec("r1", "claude-sonnet-5-5", 100, out=9)])
        agent = json.loads(self.cli("--json")[1])["agents"][0]
        self.assertEqual(agent["requests"], 1)
        self.assertEqual(agent["processed"], 109)


class Pricing(Base):
    def test_tier_boundary_100000_low_100001_high(self):
        self.agent("bbbb1111", [rec("r1", HAIKU, 100000)])
        self.agent("bbbb2222", [rec("r2", HAIKU, 100001)])
        rates = self.rates()
        low = A.price_request(HAIKU, {"in": 100000, "out": 0, "cw5": 0, "cw1": 0, "cr": 0}, rates)
        high = A.price_request(HAIKU, {"in": 100001, "out": 0, "cw5": 0, "cw1": 0, "cr": 0}, rates)
        self.assertAlmostEqual(low, 100000 * 0.10 / 1e6)
        self.assertAlmostEqual(high, 100001 * 0.50 / 1e6)
        by_id = {a["id"]: a for a in json.loads(self.cli("--json")[1])["agents"]}
        self.assertFalse(by_id["bbbb1111"]["flagged"])
        self.assertTrue(by_id["bbbb2222"]["flagged"])

    def test_tier_uses_prompt_incl_cache_not_output(self):
        req = {"in": 1, "out": 500000, "cw5": 50000, "cw1": 0, "cr": 50000}
        # prompt = 100001 -> high tier; output priced at high output rate
        usd = A.price_request(HAIKU, req, self.rates())
        self.assertAlmostEqual(usd, (1 * 0.50 + 500000 * 2.50 + 50000 * 0.625 + 50000 * 0.05) / 1e6)

    def test_flag_only_for_haiku_over_100k(self):
        self.agent("cccc1111", [rec("r1", "claude-sonnet-5-5", 150000)])
        self.agent("cccc2222", [rec("r2", "claude-haiku-4-5-20251001", 150000)])
        by_id = {a["id"]: a for a in json.loads(self.cli("--json")[1])["agents"]}
        self.assertFalse(by_id["cccc1111"]["flagged"])
        self.assertTrue(by_id["cccc2222"]["flagged"])
        self.assertIsNotNone(by_id["cccc2222"]["cost_usd"])

    def test_alias_meta_ignored_usage_model_wins(self):
        self.agent("dddd1111", [rec("r1", "claude-haiku-4-5-20251001", 10)], meta={"agentType": "x", "model": "haiku"})
        agent = json.loads(self.cli("--json")[1])["agents"][0]
        self.assertEqual(agent["model"], "claude-haiku-4-5-20251001")

    def test_synthetic_ignored_for_model_choice(self):
        self.agent("eeee1111", [rec("r0", "<synthetic>", 0), rec("r1", HAIKU, 10)])
        agent = json.loads(self.cli("--json")[1])["agents"][0]
        self.assertEqual(agent["model"], HAIKU)
        self.assertIsNotNone(agent["cost_usd"])

    def test_unknown_model_cost_na_and_total_partial(self):
        self.agent("ffff1111", [rec("r1", "claude-haiku-5-6", 10)])
        self.agent("ffff2222", [rec("r2", HAIKU, 10)])
        data = json.loads(self.cli("--json")[1])
        by_id = {a["id"]: a for a in data["agents"]}
        self.assertIsNone(by_id["ffff1111"]["cost_usd"])
        self.assertTrue(data["total"]["partial"])
        self.assertAlmostEqual(data["total"]["cost_usd"], by_id["ffff2222"]["cost_usd"])

    def test_cache_write_split_5m_and_1h(self):
        self.agent("gggg1111", [rec("r1", HAIKU, 0, cw5=1000, cw1=500)])
        agent = json.loads(self.cli("--json")[1])["agents"][0]
        self.assertAlmostEqual(agent["cost_usd"], (1000 * 0.125 + 500 * 0.20) / 1e6)
        self.assertEqual(agent["cache_write_1h_tokens"], 500)

    def test_missing_split_treated_as_5m(self):
        line = {"requestId": "r1", "timestamp": "2026-10-10T01:00:00Z",
                "message": {"id": "m", "model": HAIKU, "usage": {"input_tokens": 0, "cache_creation_input_tokens": 1000,
                                                                  "cache_read_input_tokens": 0, "output_tokens": 0}}}
        self.agent("gggg2222", [line])
        agent = json.loads(self.cli("--json")[1])["agents"][0]
        self.assertAlmostEqual(agent["cost_usd"], 1000 * 0.125 / 1e6)

    def test_total_equals_sum_of_unrounded_rows(self):
        self.agent("hhhh1111", [rec("r1", HAIKU, 12345, out=678)])
        self.agent("hhhh2222", [rec("r2", HAIKU, 99999, out=1)])
        data = json.loads(self.cli("--json")[1])
        self.assertAlmostEqual(data["total"]["cost_usd"], sum(a["cost_usd"] for a in data["agents"]), places=10)

    def test_cost_format_four_dp_under_one_dollar(self):
        self.assertEqual(A.fmt_cost(0.002167), "$0.0022")
        self.assertEqual(A.fmt_cost(1.2345), "$1.23")
        self.assertEqual(A.fmt_cost(None), "n/a")


class Rates(Base):
    def test_stale_rates_warn_on_stderr(self):
        path = self.root / "rates.json"
        raw = json.loads(Path(A.RATES_PATH).read_text(encoding="utf-8"))
        raw["as_of"] = "2000-01-01"
        path.write_text(json.dumps(raw), encoding="utf-8")
        A.RATES_PATH = path
        self.agent("ssss1111", [rec("r1", HAIKU, 10)])
        code, _, err = self.cli()
        self.assertEqual(code, 0)
        self.assertIn("days old", err)

    def test_bad_rates_fail_closed_exit_2(self):
        path = self.root / "rates.json"
        path.write_text(json.dumps({"as_of": "2026-10-10", "tier_threshold_tokens": 100000,
                                    "models": [{"match": "^x$", "low": {"input": 1}, "high": {}}]}), encoding="utf-8")
        A.RATES_PATH = path
        self.agent("ssss2222", [rec("r1", HAIKU, 10)])
        code, out, err = self.cli()
        self.assertEqual(code, 2)
        self.assertEqual(out, "")
        self.assertIn("bad rates file", err)


class Failures(Base):
    def test_missing_subagents_dir_exit_2(self):
        err = io.StringIO()
        with contextlib.redirect_stderr(err):
            code = A.main(["--session", SESSION, "--projects-dir", str(self.root / "missing")])
        self.assertEqual(code, 2)
        self.assertIn("no subagent transcripts", err.getvalue())

    def test_empty_subagents_dir_exit_2(self):
        code, _, err = self.cli()
        self.assertEqual(code, 2)
        self.assertIn("no subagent transcripts", err)

    def test_zero_parseable_records_exit_1(self):
        self.agent("ffff9999", ["not json", json.dumps({"type": "user", "message": {"content": "hi"}})])
        code, out, err = self.cli()
        self.assertEqual(code, 1)
        self.assertIn("layout drift: 1 lines skipped", err)

    def test_malformed_line_skipped_and_counted(self):
        self.agent("ffff8888", ["{broken", json.dumps(rec("r1", HAIKU, 10))])
        code, out, err = self.cli("--json")
        self.assertEqual(code, 0)
        self.assertIn("layout drift: 1 lines skipped", err)
        self.assertEqual(json.loads(out)["drift_lines"], 1)

    def test_usage_without_model_is_drift(self):
        bad = {"requestId": "r1", "message": {"usage": {"input_tokens": 1, "cache_creation_input_tokens": 0,
                                                       "cache_read_input_tokens": 0, "output_tokens": 0}}}
        self.agent("ffff7777", [bad, json.dumps(rec("r2", HAIKU, 10))])
        self.assertIn("layout drift: 1", self.cli()[2])

    def test_bad_since_exit_2(self):
        self.agent("ffff6666", [rec("r1", HAIKU, 10)])
        self.assertEqual(self.cli("--since", "not-a-date")[0], 2)


class Filters(Base):
    def setUp(self):
        super().setUp()
        for i, day in enumerate(["01", "02", "03", "04", "05", "06", "07"]):
            self.agent(f"0{i}aa{i}", [rec(f"r{i}", HAIKU, 10 + i, ts=f"2026-10-{day}T01:00:00Z")], meta={"agentType": "general-purpose", "description": f"task {i}"})

    def test_row_cap_and_more_row_and_total_covers_all(self):
        out = self.cli()[1]
        self.assertIn("+2 more", out)
        self.assertIn("Total (7)", out)
        self.assertEqual(sum(1 for line in out.splitlines() if line.startswith("| 0")), 5)

    def test_all_prints_every_row(self):
        out = self.cli("--all")[1]
        self.assertEqual(sum(1 for line in out.splitlines() if line.startswith("| 0")), 7)
        self.assertNotIn("more", out)

    def test_json_uncapped(self):
        self.assertEqual(len(json.loads(self.cli("--json")[1])["agents"]), 7)

    def test_since_filters_by_first_usage(self):
        data = json.loads(self.cli("--json", "--since", "2026-10-05T00:00:00Z")[1])
        self.assertEqual(len(data["agents"]), 3)

    def test_ids_prefix_filter(self):
        data = json.loads(self.cli("--json", "--ids", "02")[1])
        self.assertEqual([a["id"] for a in data["agents"]], ["02aa2"])

    def test_no_desc_drops_column(self):
        out = self.cli("--no-desc")[1]
        self.assertNotIn("Description", out)
        self.assertNotIn("task", out)

    def test_description_sanitized_and_truncated(self):
        long = "pipe|here\nnewline " + "x" * 80
        self.agent("zzzz1111", [rec("r1", HAIKU, 10)], meta={"agentType": "general-purpose", "description": long})
        out = self.cli("--all")[1]
        line = [l for l in out.splitlines() if l.startswith("| zzzz1111")][0]
        self.assertIn("pipe\\|here newline", line)
        self.assertNotIn("\n", line)
        desc = json.loads(self.cli("--json", "--ids", "zzzz1111")[1])["agents"][0]["description"]
        self.assertLessEqual(len(desc), A.DESC_MAX)
        self.assertTrue(desc.endswith("..."))

    def test_legend_only_when_flag_or_na(self):
        self.assertNotIn("rates as of", self.cli()[1])
        self.agent("yyyy1111", [rec("r1", "claude-unknown-9-9", 10)])
        self.assertIn("rates as of", self.cli()[1])

    def test_no_message_text_leaks(self):
        secret = "SECRET_PROMPT_TEXT_9f3a"
        self.agent("xxxx1111", [rec("r1", HAIKU, 150000, text=secret)])
        code, out, err = self.cli()
        code2, jout, _ = self.cli("--json")
        self.assertNotIn(secret, out + err + jout)
        self.assertNotIn(SESSION, out + err + jout)

    def md_ids(self, out):
        return [line.split(" | ")[0][2:] for line in out.splitlines() if line.startswith("| 0")]

    def test_shown_rows_are_earliest_in_order(self):
        self.assertEqual(self.md_ids(self.cli()[1]), ["00aa0", "01aa1", "02aa2", "03aa3", "04aa4"])

    def test_since_keeps_agent_at_exact_cutoff(self):
        data = self.data("--since", "2026-10-05T01:00:00Z")
        self.assertEqual([a["id"] for a in data["agents"]], ["04aa4", "05aa5", "06aa6"])

    def test_hidden_flag_counted_in_more_row(self):
        self.agent("08late", [rec("r8", HAIKU, 150000, ts="2026-10-08T01:00:00Z")])
        out = self.cli()[1]
        self.assertIn(f"+3 more ({A.FLAG} 1)", out)
        self.assertIn("a request exceeded 100K prompt", out)


class SurvivorGaps(Base):
    def test_low_tier_cache_read_priced_at_low_rate(self):
        usd = A.price_request(HAIKU, {"in": 0, "out": 0, "cw5": 0, "cw1": 0, "cr": 1000}, self.rates())
        self.assertAlmostEqual(usd, 1000 * 0.01 / 1e6, places=12)

    def test_output_excluded_from_tier_decision(self):
        usd = A.price_request(HAIKU, {"in": 100000, "out": 500000, "cw5": 0, "cw1": 0, "cr": 0}, self.rates())
        self.assertAlmostEqual(usd, (100000 * 0.10 + 500000 * 0.50) / 1e6, places=12)

    def test_rows_sorted_by_first_ts_not_filename(self):
        self.agent("b0000001", [rec("r1", HAIKU, 10, ts="2026-10-02T00:00:00Z")])
        self.agent("a0000002", [rec("r2", HAIKU, 10, ts="2026-10-03T00:00:00Z")])
        self.agent("c0000003", [rec("r3", HAIKU, 10, ts="2026-10-01T00:00:00Z")])
        self.assertEqual([a["id"] for a in self.data()["agents"]], ["c0000003", "b0000001", "a0000002"])

    def test_markdown_flag_marker(self):
        self.agent("f1000000", [rec("r1", HAIKU, 150000)])
        self.assertIn(f"{A.FLAG} 150,000", self.cli()[1])

    def test_markdown_description_truncated(self):
        self.agent("d1000000", [rec("r1", HAIKU, 10)], meta={"agentType": "x", "description": "x" * 80})
        out = self.cli()[1]
        self.assertIn("x" * 37 + "...", out)
        self.assertNotIn("x" * 38, out)

    def test_invalid_session_message(self):
        err = io.StringIO()
        with contextlib.redirect_stderr(err):
            code = A.main(["--session", "../x", "--projects-dir", str(self.projects)])
        self.assertEqual(code, 2)
        self.assertIn("invalid session id", err.getvalue())
        self.assertNotIn("unset", err.getvalue())

    def test_unset_session_message(self):
        err = io.StringIO()
        with mock.patch.dict(os.environ), contextlib.redirect_stderr(err):
            os.environ.pop("CLAUDE_CODE_SESSION_ID", None)
            code = A.main(["--projects-dir", str(self.projects)])
        self.assertEqual(code, 2)
        self.assertIn("CLAUDE_CODE_SESSION_ID unset", err.getvalue())

    def test_session_env_fallback(self):
        self.agent("e1000000", [rec("r1", HAIKU, 10)])
        out = io.StringIO()
        with mock.patch.dict(os.environ, {"CLAUDE_CODE_SESSION_ID": SESSION}), contextlib.redirect_stdout(out):
            code = A.main(["--projects-dir", str(self.projects), "--json"])
        self.assertEqual(code, 0)
        self.assertEqual(len(json.loads(out.getvalue())["agents"]), 1)

    def test_projects_dir_env_fallback(self):
        self.agent("e2000000", [rec("r1", HAIKU, 10)])
        out = io.StringIO()
        with mock.patch.dict(os.environ, {"AGENT_USAGE_PROJECTS_DIR": str(self.projects)}), contextlib.redirect_stdout(out):
            code = A.main(["--session", SESSION, "--json"])
        self.assertEqual(code, 0)
        self.assertEqual(len(json.loads(out.getvalue())["agents"]), 1)

    def test_legend_flag_clause_only_when_flagged(self):
        self.agent("f2000000", [rec("r1", HAIKU, 150000)])
        out = self.cli()[1]
        self.assertIn("a request exceeded 100K prompt", out)
        self.assertNotIn("n/a =", out)

    def test_legend_na_only_has_no_flag_clause(self):
        self.agent("n1000000", [rec("r1", "claude-unknown-9-9", 10)])
        out = self.cli()[1]
        self.assertIn("n/a = no rate on file", out)
        self.assertNotIn("exceeded", out)

    def test_legend_quality_follows_verified_flags(self):
        self.agent("f3000000", [rec("r1", HAIKU, 150000)])
        raw = json.loads(Path(A.RATES_PATH).read_text(encoding="utf-8"))
        path = self.root / "rates.json"
        raw["models"][0]["verified"] = {k: False for k in A.RATE_KEYS}
        path.write_text(json.dumps(raw), encoding="utf-8")
        A.RATES_PATH = path
        self.assertIn("unverified estimate", self.cli()[1])
        raw["models"][0]["verified"] = {k: True for k in A.RATE_KEYS}
        path.write_text(json.dumps(raw), encoding="utf-8")
        out = self.cli()[1]
        self.assertIn(", estimate.", out)
        self.assertNotIn("unverified", out)

    def test_naive_timestamp_assumed_utc(self):
        self.agent("t1000000", [rec("r1", HAIKU, 10, ts="2026-10-10T01:00:00")])
        data = self.data("--since", "2026-10-10T00:00:00Z")
        self.assertEqual(data["agents"][0]["first_ts"], "2026-10-10T01:00:00+00:00")

    def test_cost_format_boundary_one_dollar(self):
        self.assertEqual(A.fmt_cost(1.0), "$1.00")
        self.assertEqual(A.fmt_cost(0.99994), "$0.9999")


class InputRobustness(Base):
    def test_counter_validator(self):
        lines = [rec("ok", HAIKU, 10), rec("max", HAIKU, 10 ** 12),
                 with_usage(rec("b", HAIKU, 0), input_tokens=True), with_usage(rec("n", HAIKU, 0), input_tokens=-5),
                 with_usage(rec("h", HAIKU, 0), input_tokens=10 ** 12 + 1), with_usage(rec("o", HAIKU, 0), input_tokens=10 ** 310)]
        self.agent("v1000000", lines)
        data = self.data()
        self.assertEqual(data["drift_lines"], 4)
        self.assertEqual(data["agents"][0]["requests"], 2)
        self.assertEqual(data["agents"][0]["processed"], 10 ** 12 + 10)

    def test_unhashable_ids_fall_back(self):
        a = rec("x", HAIKU, 10)
        a["requestId"] = [1]
        b = rec("y", HAIKU, 20)
        b["requestId"], b["message"]["id"] = {"a": 1}, [2]
        self.agent("u1000000", [a, b])
        data = self.data()
        self.assertEqual((data["drift_lines"], data["agents"][0]["requests"]), (0, 2))

    def test_request_id_cannot_collide_with_line_key(self):
        b = rec("z", HAIKU, 20)
        del b["requestId"], b["message"]["id"]
        self.agent("k1000000", [rec("line1", HAIKU, 10), b])
        self.assertEqual(self.data()["agents"][0]["requests"], 2)

    def test_deep_json_line_is_drift(self):
        self.agent("j1000000", [rec("r1", HAIKU, 10), DEEP])
        self.assertEqual(self.data()["drift_lines"], 1)

    def test_drift_rules(self):
        plain = [json.dumps({"type": "user", "message": {"content": "hi"}}), json.dumps({"type": "summary"})]
        bad = ["[1]", "42", "null", '"s"', json.dumps({"message": "text"}), json.dumps({"message": [1]})]
        self.agent("r1000000", plain + bad + [rec("r1", HAIKU, 10)])
        self.assertEqual(self.data()["drift_lines"], 6)

    def test_cache_1h_above_total_is_drift(self):
        bad = with_usage(rec("c", HAIKU, 0), cache_creation_input_tokens=100, cache_creation={"ephemeral_1h_input_tokens": 500})
        self.agent("c1000000", [bad, rec("r1", HAIKU, 10)])
        data = self.data()
        self.assertEqual((data["drift_lines"], data["agents"][0]["requests"]), (1, 1))

    def test_cache_split_total_wins_over_5m_key(self):
        r = with_usage(rec("c", HAIKU, 0), cache_creation_input_tokens=5000, cache_creation={"ephemeral_5m_input_tokens": 2000})
        self.agent("c2000000", [r])
        agent = self.data()["agents"][0]
        self.assertEqual((agent["peak_ctx"], agent["cache_write_1h_tokens"]), (5000, 0))
        self.assertAlmostEqual(agent["cost_usd"], 5000 * 0.125 / 1e6, places=12)

    def test_first_ts_is_earliest_partial(self):
        self.agent("p1000000", [rec("r1", HAIKU, 10, out=1, ts="2026-10-09T23:59:59Z"),
                                rec("r1", HAIKU, 10, out=50, ts="2026-10-10T00:00:05Z")])
        agent = self.data()["agents"][0]
        self.assertEqual((agent["first_ts"], agent["processed"]), ("2026-10-09T23:59:59+00:00", 60))
        self.assertEqual(self.data("--since", "2026-10-10T00:00:00Z")["agents"], [])

    def test_lowercase_z_timestamp(self):
        self.assertEqual(A.parse_ts("2026-10-10T00:00:00z"), dt.datetime(2026, 10, 10, tzinfo=dt.timezone.utc))
        self.agent("z1000000", [rec("r1", HAIKU, 10, ts="2026-10-10T01:00:00z")])
        self.assertEqual(len(self.data("--since", "2026-10-10T00:00:00z")["agents"]), 1)

    def test_bad_args_checked_before_parse(self):
        self.agent("q1000000", ["not json"])
        self.assertEqual(self.cli("--since", "garbage")[0], 2)
        code, _, err = self.cli("--ids", ",")
        self.assertEqual(code, 2)
        self.assertIn("--ids has no ids", err)

    def test_empty_ids_string_is_noop(self):
        self.agent("q2000000", [rec("r1", HAIKU, 10)])
        self.assertEqual(len(self.data("--ids", "")["agents"]), 1)


class FileGuards(Base):
    def test_nonregular_transcripts_skipped(self):
        self.agent("ok000000", [rec("r1", HAIKU, 10)])
        (self.sub / "agent-zzdir.jsonl").mkdir()
        os.symlink("missing.jsonl", self.sub / "agent-bad.jsonl")
        os.symlink("agent-loop.jsonl", self.sub / "agent-loop.jsonl")
        code, out, err = self.cli("--json")
        self.assertEqual((code, len(json.loads(out)["agents"])), (0, 1))
        self.assertEqual(err.count("skipped agent"), 3)
        self.assertNotIn(str(self.root), err)
        self.assertNotIn(SESSION, err)

    @unittest.skipIf(hasattr(os, "geteuid") and os.geteuid() == 0, "root reads mode-0 files")
    def test_unreadable_transcript_skipped(self):
        self.agent("ok000000", [rec("r1", HAIKU, 10)])
        os.chmod(self.agent("noread00", [rec("r2", HAIKU, 10)]), 0)
        code, _, err = self.cli()
        self.assertEqual(code, 0)
        self.assertIn("skipped agent noread00", err)
        self.assertNotIn("Traceback", err)

    @unittest.skipUnless(hasattr(os, "mkfifo"), "needs mkfifo")
    def test_fifo_transcript_and_meta_do_not_hang(self):
        self.agent("ok000000", [rec("r1", HAIKU, 10)])
        os.mkfifo(self.sub / "agent-fifo.jsonl")
        os.mkfifo(self.sub / "agent-ok000000.meta.json")
        self.assertEqual(self.run_cli("--json")[0], 0)

    def test_deep_meta_treated_as_empty(self):
        self.agent("m1000000", [rec("r1", HAIKU, 10)])
        (self.sub / "agent-m1000000.meta.json").write_text(DEEP, encoding="utf-8")
        self.assertEqual(self.data()["agents"][0]["agent_type"], "unknown")


class OutputSafety(Base):
    def test_sanitize_drops_category_c(self):
        self.assertEqual(A.sanitize("a‮b⁧c\x9bd\ud800e"), "abc de")

    def test_surrogates_do_not_break_output(self):
        self.agent("s1000000", [rec("r1", "claude-haiku-\ud800", 10)],
                   meta={"agentType": "\udc80", "description": "\ud800 bad"})
        for extra in ((), ("--json",)):
            code, out, err = self.run_cli(*extra)
            self.assertEqual(code, 0, err)
            out.decode("utf-8")

    def test_ascii_stdout_does_not_crash(self):
        self.agent("f4000000", [rec("r1", HAIKU, 150000)])
        code, out, err = self.run_cli(env={"PYTHONIOENCODING": "ascii"})
        self.assertEqual(code, 0, err)
        self.assertIn(b"? 150,000", out)

    @unittest.skipUnless(sys.platform.startswith("linux"), "needs bytes filenames")
    def test_nonutf8_filename_id(self):
        path = self.agent("tmp00000", [rec("r1", HAIKU, 10)])
        os.rename(path, self.sub / os.fsdecode(b"agent-\xff\xfeab.jsonl"))
        for extra in ((), ("--json",)):
            code, out, err = self.run_cli(*extra)
            self.assertEqual(code, 0, err)
            out.decode("utf-8")

    def test_id_cell_escaped(self):
        self.agent("pi|pe000", [rec("r1", HAIKU, 10)])
        line = [l for l in self.cli()[1].splitlines() if l.startswith("| pi")][0]
        self.assertEqual(len(re.findall(r"(?<!\\)\|", line)), 8)

    def test_agent_type_capped(self):
        self.agent("t2000000", [rec("r1", HAIKU, 10)], meta={"agentType": "t" * 10000, "description": "d"})
        self.assertLess(max(len(l) for l in self.cli()[1].splitlines()), 300)
        self.assertEqual(len(self.data()["agents"][0]["agent_type"]), A.DESC_MAX)

    def test_json_cost_rounded_to_10dp(self):
        self.agent("r2000000", [rec("r1", HAIKU, 1), rec("r2", HAIKU, 2)])
        self.assertEqual(self.data()["agents"][0]["cost_usd"], 3e-07)

    def test_json_ascii_only(self):
        self.agent("a1000000", [rec("r1", HAIKU, 10)], meta={"agentType": "x", "description": "café \U0001f53a"})
        out = self.cli("--json")[1]
        self.assertTrue(out.isascii())
        self.assertEqual(json.loads(out)["agents"][0]["description"], "café \U0001f53a")


class RatesValidation(Base):
    def test_bad_rate_values_rejected(self):
        base = Path(A.RATES_PATH).read_text(encoding="utf-8")
        raw = json.loads(base)
        variants = {"bool": json.dumps(dict(raw, models=[dict(raw["models"][0], low=dict(raw["models"][0]["low"], input=True))])),
                    "inf": base.replace('"output": 0.50', '"output": 1e999', 1),
                    "nan": base.replace('"output": 0.50', '"output": NaN', 1),
                    "empty": json.dumps(dict(raw, models=[])),
                    "verified_list": json.dumps(dict(raw, models=[dict(raw["models"][0], verified=[])])),
                    "deep": DEEP}
        for name, text in variants.items():
            with self.subTest(name):
                path = self.root / f"rates-{name}.json"
                path.write_text(text, encoding="utf-8")
                with self.assertRaises(A.RatesError):
                    A.load_rates(path)

    def test_empty_models_exit_2(self):
        raw = json.loads(Path(A.RATES_PATH).read_text(encoding="utf-8"))
        path = self.root / "rates.json"
        path.write_text(json.dumps(dict(raw, models=[])), encoding="utf-8")
        A.RATES_PATH = path
        self.agent("e3000000", [rec("r1", HAIKU, 10)])
        code, out, err = self.cli()
        self.assertEqual((code, out), (2, ""))
        self.assertIn("bad rates file", err)


if __name__ == "__main__":
    unittest.main()
