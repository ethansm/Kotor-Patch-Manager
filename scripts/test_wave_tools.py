#!/usr/bin/env python3
"""Host-only tests for wave_tools.py. Every run uses a temp git repo as --repo-root; nothing touches the real .claude/waves/."""
import contextlib
import io
import json
import os
import re
import subprocess
import sys
import tempfile
import unittest
from datetime import datetime
from pathlib import Path
from unittest import mock

SCRIPTS = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPTS))
import wave_budget  # noqa: E402
import wave_tools as wt  # noqa: E402

CONTRACT = SCRIPTS / "agent_contract.py"
TOOL = SCRIPTS / "wave_tools.py"


def contract_stdout(kind):
    return subprocess.run([sys.executable, str(CONTRACT), "print", kind],
                          capture_output=True, text=True, check=True).stdout


def worker_reply(modified, status="PASS"):
    return f"CREATED: None\nMODIFIED: {modified}\nTESTS: None\nNOTES: None\nSTATUS: {status}\n"


class WaveToolsTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.root = Path(tmp.name).resolve()
        subprocess.run(["git", "init", "-q", str(self.root)], check=True)
        (self.root / "src").mkdir()
        (self.root / "src" / "a.py").write_text("".join(f"x{i} = {i}\n" for i in range(50)), encoding="utf-8")
        (self.root / "docs").mkdir()
        (self.root / "docs" / "notes.md").write_text("note\n", encoding="utf-8")
        # agent_contract check appends to the master ledger; keep test runs out of the real one
        ledger = tempfile.TemporaryDirectory()
        self.addCleanup(ledger.cleanup)
        patcher = mock.patch.dict(os.environ, {"MASTER_LEDGER_PATH": str(Path(ledger.name) / "ledger.jsonl")})
        patcher.start()
        self.addCleanup(patcher.stop)
        os.environ.pop("WAVE_OVERHEAD_TOKENS", None)

    def cli(self, *args):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            code = wt.main([*args, "--repo-root", str(self.root)])
        return code, out.getvalue(), err.getvalue()

    def wave(self, wave_id="w1"):
        return self.root / ".claude" / "waves" / wave_id

    def manifest(self, wave_id="w1"):
        return json.loads((self.wave(wave_id) / "manifest.json").read_text(encoding="utf-8"))

    def test_new_creates_wave_baseline_and_manifest(self):
        code, _, err = self.cli("new", "w1", "--kind", "worker")
        self.assertEqual(code, 0, err)
        m = self.manifest()
        self.assertEqual(m["wave"], "w1")
        self.assertEqual(m["kind"], "worker")
        self.assertEqual(m["baseline"], ".claude/waves/w1/baseline.tsv")
        self.assertEqual(m["units"], [])
        datetime.strptime(m["created"], "%Y-%m-%dT%H:%M:%SZ")
        self.assertIn("src/a.py", (self.wave() / "baseline.tsv").read_text(encoding="utf-8"))

    def test_new_twice_errors(self):
        self.assertEqual(self.cli("new", "w1", "--kind", "recon")[0], 0)
        before = self.manifest()
        code, _, err = self.cli("new", "w1", "--kind", "recon")
        self.assertEqual(code, 1)
        self.assertIn("already exists", err)
        self.assertEqual(self.manifest(), before)

    def test_new_outside_git_repo_rolls_back(self):
        plain = tempfile.TemporaryDirectory()
        self.addCleanup(plain.cleanup)
        err = io.StringIO()
        with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(err):
            code = wt.main(["new", "w1", "--kind", "recon", "--repo-root", plain.name])
        self.assertEqual(code, 1)
        self.assertIn("snapshot failed", err.getvalue())
        self.assertFalse((Path(plain.name) / ".claude" / "waves" / "w1").exists())

    def test_invalid_wave_id_rejected(self):
        for bad in ["../evil", "a/b", ".hidden", "..", "bad id", ""]:
            with self.subTest(wave_id=bad):
                self.assertEqual(self.cli("new", bad, "--kind", "recon")[0], 1)
                self.assertEqual(self.cli("add", bad, "--id", "u01", "--task", "x", "--entries", "src/a.py")[0], 1)
        self.assertFalse((self.root / ".claude").exists())

    def test_add_writes_brief_and_appends_unit(self):
        self.cli("new", "w1", "--kind", "worker")
        code, out, err = self.cli("add", "w1", "--id", "u01",
                                  "--task", "Review the agent contract module carefully now",
                                  "--entries", "src/a.py:1-20",
                                  "--scope", "scripts/wave_tools.py", "scripts/test_wave_tools.py")
        self.assertEqual(code, 0, err)
        brief_path = self.wave() / "u01.brief.md"
        brief = brief_path.read_text(encoding="utf-8")
        self.assertIn("Contract: worker\n", brief)
        self.assertIn("Write Scope: scripts/wave_tools.py, scripts/test_wave_tools.py\n", brief)
        self.assertIn("Files (read only these ranges):\n- src/a.py:1-20\n", brief)
        self.assertIn("stop and put it in OPEN", brief)
        self.assertIn("Also write your 5-line reply to .claude/waves/w1/u01.reply.txt", brief)
        self.assertIn(contract_stdout("worker"), brief)

        unit = self.manifest()["units"][0]
        self.assertEqual(unit["id"], "u01")
        self.assertEqual(unit["description"], "w1-u01 Review the agent contract module")
        self.assertEqual(unit["entries"], ["src/a.py:1-20"])
        self.assertEqual(unit["scope"], ["scripts/wave_tools.py", "scripts/test_wave_tools.py"])
        self.assertEqual(unit["out"], ".claude/waves/w1/u01.md")
        self.assertEqual(unit["reply"], ".claude/waves/w1/u01.reply.txt")
        self.assertEqual(unit["brief"], ".claude/waves/w1/u01.brief.md")
        expected = wave_budget.check([f"{self.root / 'src/a.py'}:1-20"],
                                     brief_bytes=len(brief.encode("utf-8")))["total"]
        self.assertEqual(unit["est_tokens"], expected)

        self.assertIn(f"description: {unit['description']}\n", out)
        self.assertIn(f"Follow the brief at {brief_path} exactly. Final message: the 5-line schema only.\n", out)
        self.assertIn("model: haiku, subagent_type: general-purpose\n", out)

    def test_recon_scope_defaults_to_out_path(self):
        self.cli("new", "r1", "--kind", "recon")
        code, _, err = self.cli("add", "r1", "--id", "u01", "--task", "Check the docs notes", "--entries", "docs/notes.md")
        self.assertEqual(code, 0, err)
        self.assertEqual(self.manifest("r1")["units"][0]["scope"], [".claude/waves/r1/u01.md"])
        brief = (self.wave("r1") / "u01.brief.md").read_text(encoding="utf-8")
        self.assertIn("Contract: recon\n", brief)
        self.assertIn("Write Scope: .claude/waves/r1/u01.md\n", brief)
        self.assertIn(contract_stdout("recon"), brief)

    def test_worker_requires_scope(self):
        self.cli("new", "w1", "--kind", "worker")
        code, _, err = self.cli("add", "w1", "--id", "u01", "--task", "Edit a file", "--entries", "src/a.py")
        self.assertEqual(code, 1)
        self.assertIn("--scope", err)
        self.assertEqual(self.manifest()["units"], [])

    def test_over_budget_add_exits_3_and_suggests_windows(self):
        self.cli("new", "w1", "--kind", "recon")
        # 600 lines x 100 bytes = 60000 bytes -> 15000 estimated tokens in a .txt (over the 12000 cap)
        (self.root / "big.txt").write_text("".join(f"{i:05d} " + "y" * 93 + "\n" for i in range(600)), encoding="utf-8")
        code, out, err = self.cli("add", "w1", "--id", "u01", "--task", "Read the big file", "--entries", "big.txt")
        self.assertEqual(code, 3, out + err)
        self.assertIn("big.txt:1-", out)
        windows = [m.group(1) for m in (re.match(r"^  (big\.txt:\d+-\d+)$", ln) for ln in out.splitlines()) if m]
        self.assertGreaterEqual(len(windows), 2)
        for w in windows:
            self.assertLessEqual(wave_budget.estimate_entry(str(self.root / w)), wave_budget.PER_ENTRY_CAP)
        self.assertEqual(self.manifest()["units"], [])
        self.assertFalse((self.wave() / "u01.brief.md").exists())

    def test_missing_entry_exits_2(self):
        self.cli("new", "w1", "--kind", "recon")
        code, _, err = self.cli("add", "w1", "--id", "u01", "--task", "Read it", "--entries", "nope/missing.py:1-5")
        self.assertEqual(code, 2)
        self.assertIn("not found", err)
        self.assertEqual(self.manifest()["units"], [])

    def test_duplicate_unit_id_rejected(self):
        self.cli("new", "r1", "--kind", "recon")
        self.assertEqual(self.cli("add", "r1", "--id", "u01", "--task", "First unit", "--entries", "src/a.py")[0], 0)
        code, _, err = self.cli("add", "r1", "--id", "u01", "--task", "Second unit", "--entries", "docs/notes.md")
        self.assertEqual(code, 1)
        self.assertIn("duplicate", err)
        self.assertEqual(len(self.manifest("r1")["units"]), 1)

    def test_overlapping_worker_scopes_rejected(self):
        self.cli("new", "w1", "--kind", "worker")
        self.assertEqual(self.cli("add", "w1", "--id", "u01", "--task", "Edit src", "--entries", "src/a.py",
                                  "--scope", "src/*.py")[0], 0)
        # glob matches literal path
        code, _, err = self.cli("add", "w1", "--id", "u02", "--task", "Edit a", "--entries", "src/a.py",
                                "--scope", "src/a.py")
        self.assertEqual(code, 1)
        self.assertIn("overlaps", err)
        # directory is a path prefix of the glob
        self.assertEqual(self.cli("add", "w1", "--id", "u02", "--task", "Edit src dir", "--entries", "src/a.py",
                                  "--scope", "src")[0], 1)
        # disjoint scope is accepted
        code, _, err = self.cli("add", "w1", "--id", "u03", "--task", "Edit docs", "--entries", "docs/notes.md",
                                "--scope", "docs/*.md")
        self.assertEqual(code, 0, err)
        self.assertEqual([u["id"] for u in self.manifest()["units"]], ["u01", "u03"])

    def test_list_prints_one_line_per_unit(self):
        self.cli("new", "r1", "--kind", "recon")
        self.cli("add", "r1", "--id", "u01", "--task", "Check the notes file", "--entries", "docs/notes.md")
        self.cli("add", "r1", "--id", "u02", "--task", "Check the source file", "--entries", "src/a.py:1-10")
        code, out, _ = self.cli("list", "r1")
        self.assertEqual(code, 0)
        lines = out.splitlines()
        units = self.manifest("r1")["units"]
        self.assertEqual(len(lines), len(units))
        for line, unit in zip(lines, units):
            uid, est, desc = line.split("\t")
            self.assertEqual(uid, unit["id"])
            self.assertEqual(int(est), unit["est_tokens"])
            self.assertEqual(desc, unit["description"])
        self.assertEqual(self.cli("list", "missing")[0], 1)

    def test_env_var_sets_default_root(self):
        with mock.patch.dict(os.environ, {"WAVE_REPO_ROOT": str(self.root)}), contextlib.redirect_stdout(io.StringIO()):
            code = wt.main(["new", "env1", "--kind", "recon"])
        self.assertEqual(code, 0)
        self.assertTrue((self.root / ".claude" / "waves" / "env1" / "manifest.json").is_file())

    def test_cli_entrypoint_runs(self):
        r = subprocess.run([sys.executable, str(TOOL), "new", "cli1", "--kind", "recon", "--repo-root", str(self.root)],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertTrue((self.root / ".claude" / "waves" / "cli1" / "manifest.json").is_file())

    # ------------------------------------------------------------ check / report helpers

    def start_worker_wave(self):
        """Worker wave w1: u01 reads src/a.py:1-10 and owns src/b.py; u02 reads docs/notes.md and owns src/c.py."""
        self.assertEqual(self.cli("new", "w1", "--kind", "worker")[0], 0)
        self.assertEqual(self.cli("add", "w1", "--id", "u01", "--task", "Edit the b module carefully",
                                  "--entries", "src/a.py:1-10", "--scope", "src/b.py")[0], 0)
        self.assertEqual(self.cli("add", "w1", "--id", "u02", "--task", "Edit the c module carefully",
                                  "--entries", "docs/notes.md", "--scope", "src/c.py")[0], 0)

    def write(self, rel_path, text="changed = True\n"):
        p = self.root / rel_path
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(text, encoding="utf-8")

    def reply(self, unit_id, body, wave_id="w1"):
        unit = next(u for u in self.manifest(wave_id)["units"] if u["id"] == unit_id)
        (self.root / unit["reply"]).write_text(body, encoding="utf-8")

    def check_json(self, wave_id="w1"):
        code, out, err = self.cli("check", wave_id, "--json")
        self.assertIn(code, (0, 1), err)
        data = json.loads(out)
        return code, {u["id"]: u for u in data["units"]}, data["rerun"]

    def usage_file(self, agents):
        d = tempfile.TemporaryDirectory()
        self.addCleanup(d.cleanup)
        path = Path(d.name) / "usage.json"
        path.write_text(json.dumps({"agents": agents}), encoding="utf-8")
        return str(path)

    # ------------------------------------------------------------ check

    def test_check_passes_clean_unit_and_lists_missing_for_rerun(self):
        self.start_worker_wave()
        self.write("src/b.py")
        self.reply("u01", worker_reply("src/b.py"))
        code, out, err = self.cli("check", "w1")
        self.assertEqual(code, 1, err)
        lines = out.splitlines()
        self.assertEqual(len(lines), 3, out)
        self.assertTrue(lines[0].startswith("u01 PASS PASS "), lines[0])
        self.assertEqual(lines[1].split()[:3], ["u02", "MISSING", "-"])
        self.assertEqual(lines[2], "RERUN: u02")

    def test_empty_reply_counts_as_missing(self):
        self.start_worker_wave()
        self.write("src/b.py")
        self.write("src/c.py")
        self.reply("u01", "\n  \n")
        self.reply("u02", worker_reply("src/c.py"))
        code, out, _ = self.cli("check", "w1")
        self.assertEqual(code, 1)
        self.assertIn("u01 MISSING - reply missing or empty: .claude/waves/w1/u01.reply.txt\n", out)
        self.assertTrue(out.endswith("RERUN: u01\n"), out)

    def test_out_of_scope_path_under_entry_dir_fails(self):
        self.start_worker_wave()
        self.write("src/b.py")
        self.write("src/c.py")
        self.write("src/a.py", "edited = True\n")  # u01 reads src/, so this is its out-of-scope write
        self.reply("u01", worker_reply("src/b.py"))
        self.reply("u02", worker_reply("src/c.py"))
        code, units, rerun = self.check_json()
        self.assertEqual(code, 1)
        self.assertEqual(rerun, ["u01"])
        self.assertEqual(units["u01"]["status"], "FAIL")
        self.assertIn({"level": "FAIL", "code": "OUT_OF_SCOPE", "detail": "src/a.py outside src/b.py"},
                      units["u01"]["problems"])
        self.assertEqual(units["u02"]["status"], "PASS")

    def test_unrelated_dirty_path_only_warns(self):
        self.start_worker_wave()
        self.write("src/b.py")
        self.write("src/c.py")
        self.write("scratch/other.txt")  # not under any unit's entry dir, owned by no unit
        self.reply("u01", worker_reply("src/b.py"))
        self.reply("u02", worker_reply("src/c.py"))
        code, units, rerun = self.check_json()
        self.assertEqual(code, 0)
        self.assertEqual(rerun, [])
        self.assertEqual(units["u01"]["status"], "PASS")
        self.assertIn({"level": "WARN", "code": "OUT_OF_SCOPE", "detail": "scratch/other.txt outside src/b.py"},
                      units["u01"]["problems"])

    def test_sibling_unit_edit_under_entry_dir_is_not_a_failure(self):
        # u01 reads src/, and src/c.py is u02's owned file, so the edit is u02's and only a WARN for u01
        self.start_worker_wave()
        self.write("src/b.py")
        self.write("src/c.py")
        self.reply("u01", worker_reply("src/b.py"))
        self.reply("u02", worker_reply("src/c.py"))
        code, units, rerun = self.check_json()
        self.assertEqual(code, 0)
        self.assertEqual(rerun, [])
        self.assertIn({"level": "WARN", "code": "OUT_OF_SCOPE", "detail": "src/c.py outside src/b.py"},
                      units["u01"]["problems"])

    def test_claimed_out_of_scope_path_fails_outside_entry_dir(self):
        self.start_worker_wave()
        self.write("src/b.py")
        self.write("scratch/other.txt")
        self.reply("u01", worker_reply("src/b.py, scratch/other.txt"))
        self.reply("u02", "CREATED: None\nMODIFIED: None\nTESTS: None\nNOTES: None\nSTATUS: PASS\n")
        code, units, rerun = self.check_json()
        self.assertEqual(code, 1)
        self.assertEqual(rerun, ["u01"])
        self.assertIn({"level": "FAIL", "code": "OUT_OF_SCOPE", "detail": "scratch/other.txt outside src/b.py"},
                      units["u01"]["problems"])

    def test_contract_crash_is_reported_as_fail(self):
        self.start_worker_wave()
        self.write("src/b.py")
        self.reply("u01", worker_reply("src/b.py"))
        self.reply("u02", worker_reply("src/c.py"))
        with mock.patch.object(wt, "contract_check", return_value=(1, "", "Traceback: boom\n")):
            code, out, _ = self.cli("check", "w1")
        self.assertEqual(code, 1)
        self.assertIn("u01 FAIL PASS CONTRACT_ERROR Traceback: boom\n", out)

    def test_check_empty_wave_prints_no_units(self):
        self.assertEqual(self.cli("new", "e1", "--kind", "recon")[0], 0)
        code, out, _ = self.cli("check", "e1")
        self.assertEqual(code, 0)
        self.assertEqual(out, "(no units)\nRERUN: none\n")

    # ------------------------------------------------------------ report

    def test_report_flags_peak_over_60k_and_suggests_overhead(self):
        self.start_worker_wave()
        usage = self.usage_file([
            {"description": "w1-u01 Edit the b module carefully", "peak_ctx": 70000, "agent_type": "general-purpose"},
            {"description": "w1-u02 Edit the c module carefully", "peak_ctx": 15000},
        ])
        est1 = self.manifest()["units"][0]["est_tokens"]
        code, out, err = self.cli("report", "w1", "--usage-json", usage)
        self.assertEqual(code, 1, err)
        self.assertIn(f"u01 est={est1} peak=70000 FAIL", out)
        self.assertIn(f"  suggest WAVE_OVERHEAD_TOKENS={70000 - (est1 - wave_budget.DEFAULT_OVERHEAD)}\n", out)
        self.assertIn("u02 est=", out)
        self.assertIn(" peak=15000 OK\n", out)
        self.assertTrue(out.endswith("VERDICT: FAIL units=2 flagged=1 no_data=0\n"), out)
        # the suggestion uses the overhead in effect now
        with mock.patch.dict(os.environ, {"WAVE_OVERHEAD_TOKENS": "5000"}):
            _, out, _ = self.cli("report", "w1", "--usage-json", usage)
        self.assertIn(f"  suggest WAVE_OVERHEAD_TOKENS={70000 - (est1 - 5000)}\n", out)

    def test_report_no_data_when_no_usage_row_matches(self):
        self.start_worker_wave()
        usage = self.usage_file([
            {"description": "w1-u02 Edit the c module carefully", "peak_ctx": 15000},
            {"description": "Unrelated agent", "peak_ctx": 90000},
        ])
        est1 = self.manifest()["units"][0]["est_tokens"]
        code, out, err = self.cli("report", "w1", "--usage-json", usage)
        self.assertEqual(code, 0, err)
        self.assertIn(f"u01 est={est1} NO_DATA\n", out)
        self.assertIn(" peak=15000 OK\n", out)
        self.assertTrue(out.endswith("VERDICT: PASS units=2 flagged=0 no_data=1\n"), out)

    def test_report_matches_description_cut_at_40_chars(self):
        self.start_worker_wave()
        self.assertEqual(self.cli("add", "w1", "--id", "u03", "--task",
                                  "Reconcile extraordinarily complicated cross-module naming",
                                  "--entries", "docs/notes.md", "--scope", "docs/x.md")[0], 0)
        desc = self.manifest()["units"][2]["description"]
        self.assertGreater(len(desc), 40)
        usage = self.usage_file([{"description": desc[:37] + "...", "peak_ctx": 65000}])
        code, out, err = self.cli("report", "w1", "--usage-json", usage)
        self.assertEqual(code, 1, err)
        self.assertIn(" peak=65000 FAIL (> 60000)\n", out)

    def test_report_without_usage_json_surfaces_agent_usage_failure(self):
        self.start_worker_wave()
        empty = tempfile.TemporaryDirectory()
        self.addCleanup(empty.cleanup)
        with mock.patch.dict(os.environ, {"AGENT_USAGE_PROJECTS_DIR": empty.name}):
            os.environ.pop("CLAUDE_CODE_SESSION_ID", None)
            code, out, err = self.cli("report", "w1")
        self.assertEqual(code, 1)
        self.assertIn("agent_usage.py failed", err)


if __name__ == "__main__":
    unittest.main()
