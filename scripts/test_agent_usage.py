#!/usr/bin/env python3
"""Tests for scripts/agent_usage.py (synthetic transcripts in a temp projects dir; no real ~/.claude reads)."""
import contextlib
import io
import json
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import agent_usage as A  # noqa: E402

SESSION = "11111111-2222-3333-4444-555555555555"
HAIKU = "claude-haiku-5-5"


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
        self.assertIsNone(by_id["cccc2222"]["cost_usd"])

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
        self.assertEqual(data["total"]["cost_usd"], sum(a["cost_usd"] for a in data["agents"]))

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
        code = A.main(["--session", SESSION, "--projects-dir", str(self.root / "missing")])
        self.assertEqual(code, 2)

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
        self.agent("yyyy1111", [rec("r1", "claude-sonnet-5-5", 10)])
        self.assertIn("rates as of", self.cli()[1])

    def test_no_message_text_leaks(self):
        secret = "SECRET_PROMPT_TEXT_9f3a"
        self.agent("xxxx1111", [rec("r1", HAIKU, 150000, text=secret)])
        code, out, err = self.cli()
        code2, jout, _ = self.cli("--json")
        self.assertNotIn(secret, out + err + jout)
        self.assertNotIn(SESSION, out + err + jout)


if __name__ == "__main__":
    unittest.main()
