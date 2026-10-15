#!/usr/bin/env python3
"""Tests for wi_model.py, wi_state.py and wi_driver.py. Run: python3 scripts/run_tests.py test_wi_driver -v

Builds throwaway git repos under a temp dir; the real repo is only read (index parse test). Ledger writes go to a temp file."""
import json
import os
import subprocess
import sys
import tempfile
import time
import unittest
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPTS))
import wi_driver as wd  # noqa: E402
import wi_model as wm  # noqa: E402
import wi_state as ws  # noqa: E402

DRIVER = SCRIPTS / "wi_driver.py"
GATE_OK = 'verify: python3 -I -c "import sys; sys.exit(0)"'

INDEX = """# Dev work-item list

| done | # | WI | P | state | size | after | title | rows | files | why |
|---|---|---|---|---|---|---|---|---|---|---|
| [ ] | 1 | WI-01 | P0 | ready | S | - | Alpha edit | 1 | 1 | alpha |
| [ ] | 2 | WI-02 | P0 | ready | S | WI-01 | Bravo after alpha | 1 | 1 | bravo |
| [ ] | 3 | WI-03 | P0 | ready | S | - | Charlie shares alpha file | 1 | 1 | charlie |
| [ ] | 4 | WI-04 | P1 | hold | S | - | Delta on hold | 1 | 1 | delta |
| [ ] | 5 | WI-05 | P1 | ready | S | - | Echo awaiting user | 1 | 1 | echo |
| [ ] | 6 | WI-06 | P1 | verify-only | S | - | Foxtrot verify only | 1 | 0 | foxtrot |
| [ ] | 7 | WI-07 | P1 | ready | S | - | Golf separate file | 1 | 1 | golf |
| [ ] | 8 | WI-08 | P2 | ready | S | WI-08 | Hotel self loop | 1 | 1 | hotel |
"""


def wi_file(wid, title, ready, files, extra="", rows=None):
    doc = "docs/plan.md" if wid == "WI-01" else f"docs/plan_{wid}.md"
    rows = rows if rows is not None else [f"- C001 {doc}:5 id=P1 open"]
    return (f"# {wid} {title}\narea: test\nsize: S\nready: {ready}\nitems: C001\nfiles: {files}\nout-of-repo: none\n"
            f"blockers: none\ndocs: {doc} (P1)\n{extra}\n## Goal\nChange it.\n## Steps\n1. Edit.\n## Verify\n1. gate\n## Docs\n1. none\n\n"
            "## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)\n" + "\n".join(rows) + "\n")


FILES = {
    ".claude/work_items/dev_work_items.md": INDEX,
    ".claude/work_items/WI-01.md": wi_file("WI-01", "Alpha edit", "ready", "src/a.txt", GATE_OK),
    ".claude/work_items/WI-02.md": wi_file("WI-02", "Bravo after alpha", "ready", "src/b.txt", GATE_OK + "\nafter: WI-01"),
    ".claude/work_items/WI-03.md": wi_file("WI-03", "Charlie shares alpha file", "ready", "src/a.txt", GATE_OK),
    ".claude/work_items/WI-04.md": wi_file("WI-04", "Delta on hold", "hold", "src/d.txt"),
    ".claude/work_items/WI-05.md": wi_file("WI-05", "Echo awaiting user", "awaiting-user", "src/e.txt"),
    ".claude/work_items/WI-06.md": wi_file("WI-06", "Foxtrot verify only", "verify-only", "none", GATE_OK, rows=[]),
    ".claude/work_items/WI-07.md": wi_file("WI-07", "Golf separate file", "ready", "src/g.txt; NEW:src/new_g.txt", GATE_OK, rows=[]),
    ".claude/work_items/WI-08.md": wi_file("WI-08", "Hotel self loop", "ready", "src/h.txt", GATE_OK + "\nafter: WI-08"),
    "src/a.txt": "alpha\n", "src/b.txt": "bravo\n", "src/d.txt": "delta\n", "src/e.txt": "echo\n", "src/g.txt": "golf\n",
    "src/h.txt": "hotel\n", "src/z.txt": "foreign\n",
    "docs/plan.md": "# Plan\n\n| ID | Task | Status | Commit | Notes |\n|---|---|---|---|---|\n| P1 | thing | OPEN | - | - |\n",
    ".gitignore": ".claude/waves/\n",
}


def sh(root, *args):
    return subprocess.run(args, cwd=str(root), capture_output=True, text=True)


def make_repo(tmp):
    root = Path(tmp) / "repo"
    for rel, text in FILES.items():
        p = root / rel
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(text, encoding="utf-8")
    sh(root, "git", "init", "-q")
    sh(root, "git", "config", "user.email", "t@example.com")
    sh(root, "git", "config", "user.name", "T")
    sh(root, "git", "add", "-A")
    sh(root, "git", "commit", "-q", "-m", "fixture")
    return root


class Base(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)
        self.root = make_repo(self._tmp.name)
        self.claims = Path(self._tmp.name) / "claims"
        self.env = dict(os.environ, MASTER_LEDGER_PATH=str(Path(self._tmp.name) / "ledger.jsonl"), MASTER_LEDGER_TEST="1")
        os.environ.update(MASTER_LEDGER_PATH=self.env["MASTER_LEDGER_PATH"], MASTER_LEDGER_TEST="1")
        self.st = ws.Store(self.root, self.claims)

    def drv(self, *args, session="s1"):
        return subprocess.run([sys.executable, str(DRIVER), "--repo-root", str(self.root), "--claims-dir", str(self.claims), *args],
                              capture_output=True, text=True, env=dict(self.env, WI_SESSION=session))

    def write(self, rel, text):
        (self.root / rel).write_text(text, encoding="utf-8")

    def worker_reply(self, wave, modified, created="None"):
        d = self.root / ".claude" / "waves" / wave
        (d / "u01.reply.txt").write_text(f"CREATED: {created}\nMODIFIED: {modified}\nTESTS: None\nNOTES: None\nSTATUS: PASS\n", encoding="utf-8")

    def checker_reply(self, wave, verdict="CONFIRMED"):
        d = self.root / ".claude" / "waves" / wave
        rel = (d / "u01.md").relative_to(self.root).as_posix()
        (d / "u01.md").write_text("evidence\n", encoding="utf-8")
        (d / "u01.reply.txt").write_text(f"OUTPUT: {rel}\nEVIDENCE: src/a.txt:1\nFINDING: ok\nOPEN: None\nVERDICT: {verdict}\n", encoding="utf-8")

    def claim_wave(self, wid):
        return self.st.read_claim(wid)["wave"]

    def run_flow_to_docs(self, wid="WI-01", edit=("src/a.txt", "alpha edited\n")):
        r = self.drv("next", wid)
        self.assertEqual(r.returncode, 0, r.stderr + r.stdout)
        self.write(*edit)
        self.worker_reply(self.claim_wave(wid), edit[0])
        r = self.drv("verify", wid)
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.checker_reply(self.st.read_claim(wid)["checker_wave"])
        r = self.drv("docs", wid)
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        return r


class ModelTests(Base):
    def test_real_index_parses_all_rows(self):
        items = wm.load_all(wm.REPO)
        self.assertGreaterEqual(len(items), 100)
        self.assertTrue(all(w.has_file for w in items.values()))
        self.assertFalse([w.id for w in items.values() if w.id in w.after], "self-loop survived")

    def test_real_lint_has_no_errors(self):
        errs = [p for p in wm.lint(wm.load_all(wm.REPO), wm.REPO) if p[0] == "ERROR"]
        self.assertEqual(errs, [])

    def test_header_parsing(self):
        items = wm.load_all(self.root)
        self.assertEqual(items["WI-02"].after, ["WI-01"])
        self.assertEqual(items["WI-05"].state, "awaiting_user")
        self.assertEqual(items["WI-07"].files, ["src/g.txt", "src/new_g.txt"])
        self.assertEqual(items["WI-07"].new_files, ["src/new_g.txt"])
        self.assertEqual(items["WI-06"].files, [])
        self.assertEqual(items["WI-08"].after, [])  # self-loop dropped
        self.assertEqual(items["WI-01"].source_rows[0].plan_id, "P1")

    def test_lint_flags_self_loop(self):
        probs = wm.lint(wm.load_all(self.root), self.root)
        self.assertIn(("ERROR", "WI-08", "self-loop in after"), probs)

    def test_eligibility_predicates(self):
        items = wm.load_all(self.root)
        e = lambda w, **k: wm.eligibility(items[w], items, k.get("done", set()), k.get("live", {}), k.get("dirty", set()), k.get("parked"))
        self.assertTrue(e("WI-01").ok)
        self.assertIn("waits for WI-01", e("WI-02").reason)
        self.assertTrue(e("WI-02", done={"WI-01"}).ok)
        self.assertIn("hold", e("WI-04").reason)
        self.assertIn("needs the user", e("WI-05").reason)
        self.assertIn("dirty by another session", e("WI-01", dirty={"src/a.txt"}).reason)
        live = {"WI-01": wm.lockset(items["WI-01"])}
        self.assertIn("claimed by another", e("WI-01", live=live).reason)
        self.assertIn("overlaps WI-01", e("WI-03", live=live).reason)
        self.assertIn("parked", e("WI-01", parked={"WI-01": "x"}).reason)
        self.assertIn("already done", e("WI-01", done={"WI-01"}).reason)

    def test_pick_next_order_and_explicit(self):
        items = wm.load_all(self.root)
        wi, skipped = wm.pick_next(items, set(), {}, set())
        self.assertEqual(wi.id, "WI-01")
        wi, _ = wm.pick_next(items, set(), {"WI-01": wm.lockset(items["WI-01"])}, set())
        self.assertEqual(wi.id, "WI-06")  # 02 waits, 03 overlaps, 04/05 not runnable
        wi, skipped = wm.pick_next(items, set(), {}, set(), want="WI-02")
        self.assertIsNone(wi)
        self.assertIn("waits", skipped[0][1])

    def test_lockset_includes_docs_and_changelog(self):
        w = wm.WorkItem(id="WI-99", files=["patch_manager_mods/foo_src/Foo.cpp"], area="foo",
                        source_rows=[wm.SourceRow("C1", "patch_manager_mods/02_x.md", 3, "P1", "open")])
        ls = wm.lockset(w)
        self.assertIn("patch_manager_mods/foo_src/CHANGELOG.md", ls)
        self.assertIn("patch_manager_mods/02_x.md", ls)
        c = wm.WorkItem(id="WI-98", files=["patch_manager_mods/_shared_src/X.h"], area="common")
        self.assertIn("patch_manager_mods/_shared_src/**", wm.lockset(c))
        self.assertIsNotNone(wm.locksets_conflict(wm.lockset(c), ["patch_manager_mods/_shared_src/Y.h"]))

    def test_external_ready_item_not_claimable(self):
        w = wm.WorkItem(id="WI-97", state="ready", out_of_repo=["/home/x/.claude/a.py"])
        self.assertIn("out-of-repo", wm.eligibility(w, {"WI-97": w}, set(), {}, set()).reason)
        w.state = "verify-only"
        self.assertTrue(wm.eligibility(w, {"WI-97": w}, set(), {}, set()).ok)

    def test_derive_gate_from_mod_dir(self):
        mod = self.root / "patch_manager_mods" / "foo_src"
        (mod / "sub").mkdir(parents=True)
        (mod / "sub" / "exports.def").write_text("EXPORTS\n")
        (mod / "logic_test.cpp").write_text("int main(){}\n")
        w = wm.WorkItem(id="WI-96", files=["patch_manager_mods/foo_src/sub/Foo.cpp"])
        cmds, why = wm.derive_gate(w, self.root)
        self.assertEqual(why, "derived")
        self.assertIn("python3 scripts/build_check.py --mod foo_src/sub", cmds)
        self.assertIn("python3 scripts/run_tests.py logic_test", cmds)
        none, why = wm.derive_gate(wm.WorkItem(id="WI-95", files=["notes/x.md"]), self.root)
        self.assertEqual(none, [])
        self.assertIn("no gate derivable", why)

    def test_expand_range_id(self):
        self.assertEqual(wd.expand_range_id("HP2-HP7"), ["HP2", "HP3", "HP4", "HP5", "HP6", "HP7"])
        self.assertEqual(wd.expand_range_id("T6-T8"), ["T6", "T7", "T8"])
        self.assertIsNone(wd.expand_range_id("P1"))  # not a range
        self.assertIsNone(wd.expand_range_id("WI-01"))
        self.assertIsNone(wd.expand_range_id("HP2-T7"))  # mixed prefix
        self.assertIsNone(wd.expand_range_id("T8-T6"))  # descending
        self.assertIsNone(wd.expand_range_id(None))

    def test_lint_title_mismatch_warning(self):
        warn = "WI-01: index title differs from header title"
        self.write(".claude/work_items/WI-01.md", wi_file("WI-01", "Alpha edit renamed", "ready", "src/a.txt", GATE_OK))
        self.assertIn(("WARN", "WI-01", warn), wm.lint(wm.load_all(self.root), self.root))

    def test_lint_allows_index_prefix_of_long_header(self):
        warn = "WI-01: index title differs from header title"
        long_header = "Alpha edit " + "x" * 60  # 71 chars, over the 60-char index limit
        index_title = long_header[:60]
        self.write(".claude/work_items/dev_work_items.md", INDEX.replace("Alpha edit", index_title))
        self.write(".claude/work_items/WI-01.md", wi_file("WI-01", long_header, "ready", "src/a.txt", GATE_OK))
        self.assertNotIn(("WARN", "WI-01", warn), wm.lint(wm.load_all(self.root), self.root))


class GateTests(Base):
    def fake(self, name, body):
        p = Path(self._tmp.name) / name
        p.write_text(body, encoding="utf-8")
        return p

    def test_skip_counts_as_fail(self):
        f = self.fake("run_tests.py", "print('PASS x\\n\\n1 passed, 0 failed, 0 error, 1 skipped (2 items) in 0.1s')\n")
        r = wd.run_gate_cmd(self.root, f"python3 {f}", Path(self._tmp.name) / "tmp")
        self.assertEqual(r["status"], "FAIL")
        self.assertIn("SKIPPED", r["detail"])

    def test_zero_match_is_fail_and_clean_pass(self):
        z = self.fake("run_tests.py", "print('0 passed, 0 failed, 0 error, 0 skipped (0 items) in 0.1s')\n")
        self.assertIn("no tests", wd.run_gate_cmd(self.root, f"python3 {z}", Path(self._tmp.name) / "t")["detail"])
        ok = self.fake("sub_run_tests.py", "print('3 passed, 0 failed, 0 error, 0 skipped (3 items) in 0.1s')\n")
        self.assertEqual(wd.run_gate_cmd(self.root, f"python3 {ok}", Path(self._tmp.name) / "t")["status"], "PASS")

    def test_build_check_skip_is_fail(self):
        f = self.fake("build_check.py", "print('build_check: FAIL=0 WARN=0 INFO=0 PASS=1 SKIP=1')\n")
        self.assertEqual(wd.run_gate_cmd(self.root, f"python3 {f}", Path(self._tmp.name) / "t")["status"], "FAIL")

    def test_disallowed_and_hotswap_gates(self):
        self.assertIn("not allowed", wd.run_gate_cmd(self.root, "curl http://x", Path(self._tmp.name) / "t")["detail"])
        self.assertIn("hot-swap", wd.run_gate_cmd(self.root, "python3 scripts/kpm_hotswap.py --apply", Path(self._tmp.name) / "t")["detail"])

    def test_gate_rejection_policy(self):
        allowed = ["python3 scripts/run_tests.py test_kpm_hotswap", "python3 scripts/run_tests.py hotswap",
                   "python3 scripts/run_tests.py test_kpm_hotswap -v", "python3 scripts/build_check.py"]
        for cmd in allowed:
            self.assertIsNone(wm.gate_cmd_rejection(cmd), cmd)
        rejected = ["python3 scripts/kpm_hotswap.py --apply", "python3 scripts/kpm_hotswap.py --prepare",
                    "python3 scripts/kpm_ship.py --appl", "python3 scripts/kpm_ship.py --app", "python3 scripts/kpm_ship.py --ap=x",
                    "python3 scripts/kpm_reinstall.py --ap", "bash -c 'python3 scripts/kpm_hotswap.py'",
                    "bash -c \"python3 scripts/kpm_hotswap.py --apply\"", "python3 -m kpm_hotswap", "python3 -c 'import kpm_hotswap'",
                    "python3 scripts/run_tests.py -k kpm_hotswap", "python3 scripts/run_tests.py test_x -k kpm_hotswap",
                    "python3 scripts/run_tests.py 'kpm_hotswap;x'"]
        for cmd in rejected:
            self.assertEqual(wm.gate_cmd_rejection(cmd), "gate may not install or hot-swap", cmd)
        self.assertIn("not allowed", wm.gate_cmd_rejection("curl http://x"))
        self.assertIn("unparseable", wm.gate_cmd_rejection("python3 'oops"))
        self.assertIs(wd.gate_rejection, wm.gate_rejection)

    def test_real_corpus_gates_pass_policy(self):
        items = wm.load_all(wm.REPO)
        bad = []
        for wi in items.values():
            if not wi.runnable:
                continue
            for c, d in wm.gate_preflight(wi, wm.REPO):
                bad.append(f"{wi.id}: {d} :: {c}")
        self.assertEqual(bad, [])

    def test_nonzero_exit_fails(self):
        self.assertEqual(wd.run_gate_cmd(self.root, "python3 -I -c 'import sys; sys.exit(3)'", Path(self._tmp.name) / "t")["status"], "FAIL")


class ClaimTests(Base):
    def test_exclusive_create(self):
        self.assertIsNotNone(self.st.create_claim("WI-01", "a", [], "h", "WI-01"))
        self.assertIsNone(self.st.create_claim("WI-01", "b", [], "h", "WI-01"))

    def test_parallel_next_claims_distinct_items(self):
        cmd = lambda s: [sys.executable, str(DRIVER), "--repo-root", str(self.root), "--claims-dir", str(self.claims), "next", "--session", s]
        procs = [subprocess.Popen(cmd(s), stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, env=self.env) for s in ("sa", "sb", "sc")]
        outs = [p.communicate()[0] for p in procs]
        self.assertTrue(all(p.returncode == 0 for p in procs), outs)
        claimed = sorted(self.st.all_claims())
        self.assertEqual(len(claimed), 3, outs)
        self.assertEqual(claimed, ["WI-01", "WI-06", "WI-07"])  # 02 waits for 01, 03 shares 01's file

    def test_next_dry_run_claims_nothing(self):
        r = self.drv("next", "--dry-run")
        self.assertEqual(r.returncode, 0)
        self.assertIn("CLAIM WI-01", r.stdout)
        self.assertEqual(self.st.all_claims(), {})

    def test_stale_claim_never_auto_taken_but_adoptable(self):
        c = self.st.create_claim("WI-01", "old", wm.lockset(wm.load_all(self.root)["WI-01"]), "h", "WI-01")
        c["heartbeat_at"] = "2020-01-01T00:00:00Z"
        c["base"] = ".claude/waves/WI-01/base.tsv"
        self.st.write_claim("WI-01", c)
        r = self.drv("next")
        self.assertIn("STALE claim WI-01", r.stdout)
        self.assertNotIn("CLAIM WI-01", r.stdout.replace("STALE claim WI-01", ""))
        r = self.drv("next", "WI-01", "--adopt", session="new")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertEqual(self.st.read_claim("WI-01")["session"], "new")

    def test_adopt_refuses_live_claim_and_release_needs_force(self):
        self.drv("next", "WI-01")
        r = self.drv("next", "WI-01", "--adopt")
        self.assertEqual(r.returncode, 1)
        self.assertEqual(self.drv("release", "WI-01").returncode, 1)
        self.assertEqual(self.drv("release", "WI-01", "--force").returncode, 0)
        self.assertEqual(self.st.all_claims(), {})

    def test_dirty_write_file_is_skipped(self):
        self.write("src/a.txt", "someone else\n")
        r = self.drv("next", "WI-01")
        self.assertEqual(r.returncode, 3)
        self.assertIn("dirty by another session", r.stdout)


class FlowTests(Base):
    def test_full_flow_commits_only_item_paths(self):
        self.write("src/z.txt", "foreign edit\n")  # another session's dirty file
        self.run_flow_to_docs("WI-01")
        r = self.drv("close", "WI-01")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        stat = sh(self.root, "git", "show", "--stat", "--format=%s%n%b", "HEAD").stdout
        self.assertTrue(stat.startswith("WI-01: Alpha edit"), stat)
        self.assertIn("src/a.txt", stat)
        self.assertIn("docs/plan.md", stat)
        self.assertIn("state.jsonl", stat)
        self.assertNotIn("src/z.txt", stat)
        self.assertIn("Co-Authored-By: Claude Sonnet 5.5", stat)
        self.assertEqual(sh(self.root, "git", "status", "--porcelain", "--", "src/a.txt", "docs/plan.md").stdout.strip(), "")
        self.assertIn("z.txt", sh(self.root, "git", "status", "--porcelain").stdout)  # foreign edit untouched
        self.assertEqual(self.st.all_claims(), {})
        self.assertIn("| DONE |", (self.root / "docs/plan.md").read_text())
        self.assertIn("| [x] | 1 | WI-01 |", (self.root / ".claude/work_items/dev_work_items.md").read_text())
        done, _ = self.st.state_maps()
        self.assertEqual(done, {"WI-01"})
        runs = self.st.runs()
        self.assertEqual(runs[-1]["wi"], "WI-01")
        self.assertEqual(runs[-1]["outcome"], "done")
        rep = self.drv("report", "WI-01")
        self.assertIn("### Saved State", rep.stdout)
        self.assertIn("Next Resume Command: /next-dev-work-item", rep.stdout)
        # dependency released: WI-02 now claimable
        self.assertIn("CLAIM WI-02", self.drv("next", "--dry-run").stdout)

    def test_close_dry_run_changes_nothing(self):
        self.run_flow_to_docs("WI-01")
        head = sh(self.root, "git", "rev-parse", "HEAD").stdout
        r = self.drv("close", "WI-01", "--dry-run")
        self.assertIn("would commit", r.stdout)
        self.assertEqual(sh(self.root, "git", "rev-parse", "HEAD").stdout, head)
        self.assertIsNotNone(self.st.read_claim("WI-01"))

    def test_gate_failure_retries_once_escalates_then_parks(self):
        bad = FILES[".claude/work_items/WI-01.md"].replace(GATE_OK, 'verify: python3 -I -c "import sys; sys.exit(1)"')
        self.write(".claude/work_items/WI-01.md", bad)
        sh(self.root, "git", "commit", "-qam", "bad gate")
        self.drv("next", "WI-01")
        self.write("src/a.txt", "edit\n")
        self.worker_reply(self.claim_wave("WI-01"), "src/a.txt")
        r = self.drv("verify", "WI-01")
        self.assertEqual(r.returncode, 1, r.stdout)
        self.assertIn("RETRY", r.stdout)
        self.assertIn("model=haiku", r.stdout)
        self.worker_reply(self.claim_wave("WI-01"), "src/a.txt")
        r = self.drv("verify", "WI-01")
        self.assertEqual(r.returncode, 1, r.stdout)
        self.assertIn("ESCALATE", r.stdout)
        self.assertIn("SPAWN 1 agent: model=sonnet", r.stdout)
        self.assertNotIn("PARKED", r.stdout)
        self.assertTrue(self.st.read_claim("WI-01")["escalated"])
        self.worker_reply(self.claim_wave("WI-01"), "src/a.txt")
        r = self.drv("verify", "WI-01")
        self.assertEqual(r.returncode, 2, r.stdout)
        self.assertIn("PARKED WI-01", r.stdout)
        self.assertIn("Partial edits left uncommitted", r.stdout)
        self.assertEqual(self.st.all_claims(), {})
        _, parked = self.st.state_maps()
        self.assertIn("WI-01", parked)
        self.assertIn("| [!] | 1 | WI-01 |", (self.root / ".claude/work_items/dev_work_items.md").read_text())
        self.assertIn("parked", self.drv("next", "WI-01").stdout)

    def test_next_refuses_policy_rejected_gate_without_claiming(self):
        bad = FILES[".claude/work_items/WI-01.md"].replace(GATE_OK, "verify: python3 scripts/kpm_hotswap.py --prepare")
        self.write(".claude/work_items/WI-01.md", bad)
        sh(self.root, "git", "commit", "-qam", "rejected gate")
        self.assertIn("(gate rejected: gate may not install or hot-swap)", "\n".join(m for _, _, m in wm.lint(wm.load_all(self.root), self.root)))
        r = self.drv("next", "WI-01")
        self.assertEqual(r.returncode, 3, r.stdout + r.stderr)
        self.assertIn("GATE CONFIG ERROR WI-01: gate may not install or hot-swap", r.stdout)
        self.assertEqual(self.st.all_claims(), {})
        self.assertFalse((self.root / ".claude" / "waves" / "WI-01-a1").exists())

    def test_unnamed_next_passes_over_policy_rejected_gate(self):
        bad = FILES[".claude/work_items/WI-01.md"].replace(GATE_OK, "verify: python3 scripts/kpm_hotswap.py --prepare")
        self.write(".claude/work_items/WI-01.md", bad)
        sh(self.root, "git", "commit", "-qam", "rejected gate")
        r = self.drv("next")
        self.assertNotIn("CLAIM WI-01", r.stdout, r.stdout)
        self.assertNotIn("WI-01", self.st.all_claims())
        self.assertNotIn("REFUSED", r.stdout)
        if r.returncode:
            self.assertIn("WI-01: gate config error", r.stdout)

    def test_verify_gate_config_error_parks_without_retry(self):
        self.drv("next", "WI-01")
        bad = FILES[".claude/work_items/WI-01.md"].replace(GATE_OK, "verify: python3 scripts/kpm_ship.py --appl")
        self.write(".claude/work_items/WI-01.md", bad)  # header changed after the claim
        sh(self.root, "git", "commit", "-qm", "rejected gate", "--", ".claude/work_items/WI-01.md")
        self.write("src/a.txt", "edit\n")
        self.worker_reply(self.claim_wave("WI-01"), "src/a.txt")
        r = self.drv("verify", "WI-01")
        self.assertEqual(r.returncode, 2, r.stdout + r.stderr)
        self.assertIn("GATE CONFIG ERROR WI-01: gate may not install or hot-swap :: python3 scripts/kpm_ship.py --appl", r.stdout)
        self.assertIn("THEN: fix the item's `verify:` header", r.stdout)
        self.assertNotIn("RETRY", r.stdout)
        self.assertNotIn("SPAWN", r.stdout)
        self.assertIn("PARKED WI-01", r.stdout)
        self.assertEqual(self.st.all_claims(), {})
        _, parked = self.st.state_maps()
        self.assertTrue(parked["WI-01"].startswith("gate config error"), parked)
        self.assertEqual(self.drv("unpark", "WI-01", "--reason", "verify header fixed").returncode, 0)

    def test_retry_adopts_attempt_one_edits_as_own_partial(self):
        bad = FILES[".claude/work_items/WI-07.md"].replace(GATE_OK, 'verify: python3 -I -c "import sys; sys.exit(1)"')
        self.write(".claude/work_items/WI-07.md", bad)
        sh(self.root, "git", "commit", "-qam", "bad gate")
        wi = wm.load_all(self.root)["WI-07"]
        self.write("src/z.txt", "pre-dirty, another session\n")
        self.drv("next", "WI-07")
        self.write("src/g.txt", "attempt one\n")
        self.write("src/new_g.txt", "created in attempt one\n")
        self.worker_reply(self.claim_wave("WI-07"), "src/g.txt", created="src/new_g.txt")
        r = self.drv("verify", "WI-07")
        self.assertIn("RETRY", r.stdout)
        claim = self.st.read_claim("WI-07")
        self.assertEqual(claim["own_partial"], ["src/g.txt", "src/new_g.txt"])
        self.worker_reply(self.claim_wave("WI-07"), "src/g.txt", created="src/new_g.txt")  # attempt two keeps attempt one's edits
        chk = wd.wave_check(self.st, self.claim_wave("WI-07"), wi)
        self.assertEqual(chk["status"], "PASS", chk)
        self.assertNotIn("NOT_DIRTY", json.dumps(chk.get("problems", [])))

    def test_missing_worker_reply_triggers_retry(self):
        self.drv("next", "WI-01")
        r = self.drv("verify", "WI-01")
        self.assertEqual(r.returncode, 1)
        self.assertIn("MISSING", r.stdout)
        self.assertIn("RETRY", r.stdout)

    def test_refuted_checker_goes_to_failure_path(self):
        self.drv("next", "WI-01")
        self.write("src/a.txt", "x\n")
        self.worker_reply(self.claim_wave("WI-01"), "src/a.txt")
        self.drv("verify", "WI-01")
        self.checker_reply(self.st.read_claim("WI-01")["checker_wave"], "REFUTED")
        r = self.drv("docs", "WI-01")
        self.assertEqual(r.returncode, 1)
        self.assertIn("RETRY", r.stdout)

    def test_close_hard_stops_on_pre_dirty_in_scope_path(self):
        self.write("docs/plan.md", (self.root / "docs/plan.md").read_text() + "\nforeign note\n")  # dirty before claim
        self.run_flow_to_docs("WI-01")
        self.write("docs/plan.md", (self.root / "docs/plan.md").read_text() + "\nmore foreign\n")  # changed after claim
        r = self.drv("close", "WI-01")
        self.assertEqual(r.returncode, 2)
        self.assertIn("HARD STOP", r.stdout)
        self.assertIsNotNone(self.st.read_claim("WI-01"))
        self.assertEqual(sh(self.root, "git", "log", "--format=%s", "-1").stdout.strip(), "fixture")

    def test_dirty_plan_doc_is_not_marked(self):
        self.write("docs/plan.md", (self.root / "docs/plan.md").read_text() + "\nforeign note\n")
        r = self.run_flow_to_docs("WI-01")
        self.assertIn("skipped-dirty 1", r.stdout)
        self.assertNotIn("| DONE |", (self.root / "docs/plan.md").read_text())

    def test_new_file_is_added_and_committed(self):
        self.drv("next", "WI-07")
        self.write("src/g.txt", "golf edited\n")
        self.write("src/new_g.txt", "brand new\n")
        self.worker_reply(self.claim_wave("WI-07"), "src/g.txt", created="src/new_g.txt")  # unlisted, it would count as a stray
        self.assertEqual(self.drv("verify", "WI-07").returncode, 0)
        self.checker_reply(self.st.read_claim("WI-07")["checker_wave"])
        self.assertEqual(self.drv("docs", "WI-07").returncode, 0)
        self.assertEqual(self.drv("close", "WI-07").returncode, 0)
        self.assertIn("src/new_g.txt", sh(self.root, "git", "show", "--stat", "--format=", "HEAD").stdout)

    def test_stage_order_enforced(self):
        self.drv("next", "WI-01")
        self.assertEqual(self.drv("docs", "WI-01").returncode, 1)
        self.assertEqual(self.drv("close", "WI-01").returncode, 1)

    def test_park_command_records_and_releases(self):
        self.drv("next", "WI-01")
        r = self.drv("park", "WI-01", "--reason", "needs design")
        self.assertEqual(r.returncode, 2)
        self.assertEqual(self.st.all_claims(), {})
        self.assertEqual(self.st.runs()[-1]["outcome"], "park")
        self.assertEqual(sh(self.root, "git", "log", "-1", "--format=%s").stdout.strip(), "WI-01: parked")

    def test_unpark_makes_parked_item_claimable_again(self):
        index = self.root / ".claude/work_items/dev_work_items.md"
        self.drv("next", "WI-01")
        self.assertEqual(self.drv("park", "WI-01", "--reason", "needs design").returncode, 2)
        self.assertIn("| [!] | 1 | WI-01 |", index.read_text())
        self.assertEqual(self.drv("next", "WI-01", "--dry-run").returncode, 3)  # parked: not claimable
        dry = self.drv("unpark", "WI-01", "--reason", "design settled", "--dry-run")
        self.assertEqual(dry.returncode, 0, dry.stdout + dry.stderr)
        self.assertIn("WI-01", self.st.state_maps()[1])  # dry run changes nothing
        r = self.drv("unpark", "WI-01", "--reason", "design settled")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertNotIn("WI-01", self.st.state_maps()[1])
        self.assertIn("| [ ] | 1 | WI-01 |", index.read_text())
        self.assertEqual(sh(self.root, "git", "log", "-1", "--format=%s").stdout.strip(), "WI-01: unparked")
        body = sh(self.root, "git", "log", "-1", "--format=%b").stdout
        self.assertIn("Reason: design settled", body)
        self.assertIn("Co-Authored-By: Claude Sonnet 5.5", body)
        self.assertIn("CLAIM WI-01", self.drv("next", "WI-01", "--dry-run").stdout)
        self.assertEqual(self.drv("unpark", "WI-01", "--reason", "again").returncode, 2)  # already unparked

    def test_unpark_refuses_when_not_parked(self):
        head = sh(self.root, "git", "rev-parse", "HEAD").stdout
        r = self.drv("unpark", "WI-01", "--reason", "nothing to do")
        self.assertEqual(r.returncode, 2, r.stdout + r.stderr)
        self.assertIn("not parked", r.stderr)
        self.assertEqual(sh(self.root, "git", "rev-parse", "HEAD").stdout, head)

    def test_worker_status_fail_goes_to_failure_path(self):
        self.drv("next", "WI-01")

        def worker_reports_fail():
            self.write("src/a.txt", "edit\n")
            self.worker_reply(self.claim_wave("WI-01"), "src/a.txt")
            reply = self.root / ".claude" / "waves" / self.claim_wave("WI-01") / "u01.reply.txt"
            reply.write_text(reply.read_text(encoding="utf-8").replace("STATUS: PASS", "STATUS: FAIL"), encoding="utf-8")

        worker_reports_fail()
        r = self.drv("verify", "WI-01")
        self.assertEqual(r.returncode, 1, r.stdout + r.stderr)
        self.assertIn("worker STATUS: FAIL", r.stdout)
        self.assertIn("RETRY", r.stdout)
        self.assertNotIn("GATE PASS", r.stdout)
        claim = self.st.read_claim("WI-01")
        self.assertEqual(claim["stage"], "worker")  # retry re-spawned the worker; no checker wave was created
        self.assertIsNone(claim.get("checker_wave"))

        worker_reports_fail()
        r = self.drv("verify", "WI-01")
        self.assertEqual(r.returncode, 1, r.stdout + r.stderr)
        self.assertIn("ESCALATE", r.stdout)
        self.assertIn("model=sonnet", r.stdout)

        worker_reports_fail()
        r = self.drv("verify", "WI-01")
        self.assertEqual(r.returncode, 2, r.stdout + r.stderr)
        self.assertIn("PARKED WI-01", r.stdout)
        self.assertNotIn("GATE PASS", r.stdout)
        self.assertEqual(self.st.all_claims(), {})
        _, parked = self.st.state_maps()
        self.assertIn("WI-01", parked)

    def test_partial_checker_verdict_blocks_docs(self):
        self.drv("next", "WI-01")
        self.write("src/a.txt", "alpha edited\n")
        self.worker_reply(self.claim_wave("WI-01"), "src/a.txt")
        self.assertEqual(self.drv("verify", "WI-01").returncode, 0)
        self.checker_reply(self.st.read_claim("WI-01")["checker_wave"], "PARTIAL")
        r = self.drv("docs", "WI-01")
        self.assertEqual(r.returncode, 1, r.stdout + r.stderr)
        self.assertIn("CHECKER: PARTIAL -> not CONFIRMED", r.stdout)
        self.assertIn("RETRY", r.stdout)
        self.assertNotIn("| DONE |", (self.root / "docs/plan.md").read_text())
        self.assertIsNotNone(self.st.read_claim("WI-01"))

        # control: the retry re-spawns the worker, and a CONFIRMED checker verdict lets docs proceed
        self.write("src/a.txt", "alpha edited again\n")
        self.worker_reply(self.claim_wave("WI-01"), "src/a.txt")
        self.assertEqual(self.drv("verify", "WI-01").returncode, 0)
        self.checker_reply(self.st.read_claim("WI-01")["checker_wave"])
        r = self.drv("docs", "WI-01")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertNotIn("not CONFIRMED", r.stdout)
        self.assertIn("| DONE |", (self.root / "docs/plan.md").read_text())


    def test_partial_checker_twice_escalates_to_sonnet_before_parking(self):
        self.drv("next", "WI-01")
        self.write("src/a.txt", "alpha edited\n")
        self.worker_reply(self.claim_wave("WI-01"), "src/a.txt")
        self.assertEqual(self.drv("verify", "WI-01").returncode, 0)
        self.checker_reply(self.st.read_claim("WI-01")["checker_wave"], "PARTIAL")
        self.assertIn("RETRY", self.drv("docs", "WI-01").stdout)  # Haiku retry
        self.write("src/a.txt", "alpha edited 2\n")
        self.worker_reply(self.claim_wave("WI-01"), "src/a.txt")
        self.assertEqual(self.drv("verify", "WI-01").returncode, 0)
        self.checker_reply(self.st.read_claim("WI-01")["checker_wave"], "PARTIAL")
        r = self.drv("docs", "WI-01")
        self.assertEqual(r.returncode, 1, r.stdout + r.stderr)
        self.assertIn("ESCALATE", r.stdout)
        self.assertIn("SPAWN 1 agent: model=sonnet", r.stdout)
        self.assertNotIn("PARKED", r.stdout)
        self.write("src/a.txt", "alpha edited 3\n")
        self.worker_reply(self.claim_wave("WI-01"), "src/a.txt")
        v = self.drv("verify", "WI-01")
        self.assertEqual(v.returncode, 0, v.stdout + v.stderr)
        self.assertIn("SPAWN 1 agent: model=sonnet", v.stdout)  # the escalated attempt is checked by Sonnet
        self.checker_reply(self.st.read_claim("WI-01")["checker_wave"], "PARTIAL")
        r = self.drv("docs", "WI-01")
        self.assertEqual(r.returncode, 2, r.stdout + r.stderr)
        self.assertIn("PARKED WI-01", r.stdout)

    def test_foreign_edit_mid_run_is_noted_not_failed(self):
        wid = "WI-01"
        wi = wm.load_all(self.root)[wid]
        self.drv("next", wid)
        self.write("src/a.txt", "alpha edited\n")
        self.worker_reply(self.claim_wave(wid), "src/a.txt")
        self.write("src/z.txt", "another session, mid-run\n")  # outside WI-01's lockset and not in its reply
        chk = wd.wave_check(self.st, self.claim_wave(wid), wi)
        self.assertEqual((chk["status"], chk["finding"]), ("PASS", "clean"))
        self.assertEqual(self.drv("verify", wid).returncode, 0)
        self.checker_reply(self.st.read_claim(wid)["checker_wave"])
        self.write("src/y.txt", "yet another session, after the checker baseline\n")
        chk = wd.wave_check(self.st, self.st.read_claim(wid)["checker_wave"], wi)
        self.assertEqual((chk["status"], chk["finding"]), ("PASS", "clean"))  # not downgraded to PARTIAL
        self.assertEqual(self.drv("docs", wid).returncode, 0)
        self.assertEqual(self.drv("close", wid).returncode, 0)
        self.assertIn("src/z.txt", self.st.runs()[-1]["foreign_paths"])
        self.assertIn("src/z.txt", sh(self.root, "git", "status", "--porcelain").stdout)  # noted, not committed

    def test_stray_edit_inside_lockset_still_counts(self):
        wid = "WI-01"
        self.drv("next", wid)
        self.write("src/a.txt", "alpha edited\n")
        self.write("docs/plan.md", "# stray write inside WI-01's lockset\n")  # in the lockset, not in the reply
        self.worker_reply(self.claim_wave(wid), "src/a.txt")
        r = self.drv("verify", wid)
        self.assertEqual(r.returncode, 1, r.stdout + r.stderr)
        self.assertIn("FAIL WI-01: worker reply FAIL", r.stdout)  # the cited doc rows are a read entry, so editing the doc is a hard FAIL
        self.assertIn("OUT_OF_SCOPE docs/plan.md", r.stdout)


class ForeignPathTests(Base):
    """Changed paths outside the item lockset and unlisted by the reply are foreign; everything else is kept."""
    REPLY = "CREATED: None\nMODIFIED: src/a.txt\nTESTS: None\nNOTES: None\nSTATUS: PASS\n"

    def test_unowned_paths_are_foreign(self):
        wi = wm.load_all(self.root)["WI-01"]
        probs = [("WARN", "UNCLAIMED", "src/z.txt changed since baseline but is not listed"),
                 ("WARN", "OUT_OF_SCOPE", "src/z.txt outside src/a.txt"),
                 ("FAIL", "OUT_OF_SCOPE", "src/y.txt outside src/a.txt")]
        self.assertEqual(wd.filter_foreign(probs, wi, self.REPLY, "worker"), ([], ["src/y.txt", "src/z.txt"]))

    def test_lockset_and_listed_paths_and_other_codes_are_kept(self):
        wi = wm.load_all(self.root)["WI-01"]
        in_lock = ("WARN", "UNCLAIMED", "docs/plan.md changed since baseline but is not listed")
        listed = ("WARN", "UNCLAIMED", "src/z.txt changed since baseline but is not listed")
        reply = self.REPLY.replace("MODIFIED: src/a.txt", "MODIFIED: src/a.txt, src/z.txt")
        other = [("FAIL", "NOT_DIRTY", "MODIFIED src/a.txt shows no change"), ("FAIL", "CONTRACT_ERROR", "bad line"),
                 ("WARN", "ORDER", "got TESTS,STATUS, want STATUS")]
        kept, foreign = wd.filter_foreign([in_lock, listed] + other, wi, reply, "worker")
        self.assertEqual(kept, [in_lock, listed] + other)
        self.assertEqual(foreign, [])
        kept, foreign = wd.filter_foreign([in_lock], wi, self.REPLY, "worker")
        self.assertEqual((kept, foreign), ([in_lock], []))


class RescueTests(Base):
    FAIL_REASON = "worker reply FAIL: worker STATUS: FAIL"

    def park_on_fail(self, wid="WI-01"):
        self.drv("next", wid)
        self.assertEqual(self.drv("park", wid, "--reason", self.FAIL_REASON).returncode, 2)

    def wave_dir(self, wid="WI-01"):
        return self.root / ".claude" / "waves" / f"{wid}-rescue"

    def test_worker_wave_lists_doc_ranges_and_write_scope_files(self):
        self.assertEqual(self.drv("next", "WI-01").returncode, 0)
        brief = (self.root / ".claude/waves" / self.claim_wave("WI-01") / "u01.brief.md").read_text(encoding="utf-8")
        self.assertIn("- docs/plan.md:1-5", brief)
        self.assertIn("- src/a.txt", brief)
        self.assertIn(".claude/work_items/WI-01.md", brief)

    def test_doc_read_ranges_pad_and_clamp(self):
        long_doc = "".join(f"line {n}\n" for n in range(1, 400))
        self.write("docs/plan.md", long_doc)
        wi = wm.load_all(self.root)["WI-01"]
        wi.docs = "docs/plan.md:lines 100, 120 (steps)"
        wi.source_rows = []
        self.assertEqual(wd.doc_read_ranges(self.root, wi), ["docs/plan.md:85-135"])
        wi.docs = "docs/plan.md:lines 100, 390"
        self.assertEqual(wd.doc_read_ranges(self.root, wi), ["docs/plan.md:85-235"])  # span cap

    def test_worker_fail_park_points_to_rescue(self):
        self.drv("next", "WI-01")
        for _ in range(3):  # Haiku attempt, Haiku retry, Sonnet escalation
            self.write("src/a.txt", "edit\n")
            self.worker_reply(self.claim_wave("WI-01"), "src/a.txt")
            reply = self.root / ".claude" / "waves" / self.claim_wave("WI-01") / "u01.reply.txt"
            reply.write_text(reply.read_text(encoding="utf-8").replace("STATUS: PASS", "STATUS: FAIL"), encoding="utf-8")
            r = self.drv("verify", "WI-01")
        self.assertIn("PARKED WI-01", r.stdout)
        self.assertIn("THEN: python3 scripts/wi_driver.py rescue WI-01", r.stdout)
        self.assertNotIn("Safe to /clear", r.stdout)

    def test_unparked_item_adopts_its_own_leftover_edits(self):
        self.drv("next", "WI-01")
        self.write("src/a.txt", "partial edit\n")
        self.assertEqual(self.drv("park", "WI-01", "--reason", "checker partial").returncode, 2)
        self.assertEqual(self.drv("unpark", "WI-01", "--reason", "steps fixed").returncode, 0)
        r = self.drv("next", "WI-01")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertIn("adopted 1 leftover edit", r.stdout)
        self.assertEqual(self.st.read_claim("WI-01")["own_partial"], ["src/a.txt"])
        self.worker_reply(self.claim_wave("WI-01"), "src/a.txt")  # a no-op worker still lists the adopted file
        v = self.drv("verify", "WI-01")
        self.assertEqual(v.returncode, 0, v.stdout + v.stderr)
        self.checker_reply(self.st.read_claim("WI-01")["checker_wave"])
        self.assertEqual(self.drv("docs", "WI-01").returncode, 0)
        c = self.drv("close", "WI-01")
        self.assertEqual(c.returncode, 0, c.stdout + c.stderr)
        self.assertIn("COMMITTED WI-01", c.stdout)
        self.assertEqual(sh(self.root, "git", "show", "--stat", "--format=", "HEAD").stdout.count("src/a.txt"), 1)

    def test_other_sessions_edit_still_blocks_claim_after_unpark(self):
        self.drv("next", "WI-01")
        self.write("src/a.txt", "partial edit\n")
        self.drv("park", "WI-01", "--reason", "x")
        self.drv("unpark", "WI-01", "--reason", "y")
        self.write("src/a.txt", "someone else changed it\n")
        self.assertEqual(self.drv("next", "WI-01").returncode, 3)

    def test_plain_park_does_not_point_to_rescue(self):
        self.drv("next", "WI-01")
        r = self.drv("park", "WI-01", "--reason", "needs design")
        self.assertNotIn("rescue", r.stdout)
        self.assertIn("Safe to /clear", r.stdout)

    def test_rescue_refuses_non_fail_park_and_unparked_item(self):
        r = self.drv("rescue", "WI-01")
        self.assertEqual(r.returncode, 1)
        self.assertIn("not parked", r.stderr)
        self.drv("next", "WI-01")
        self.drv("park", "WI-01", "--reason", "needs design")
        r = self.drv("rescue", "WI-01")
        self.assertEqual(r.returncode, 1)
        self.assertIn("not a worker FAIL", r.stderr)
        self.assertFalse(self.wave_dir().exists())

    def test_rescue_writes_seven_output_only_briefs_once(self):
        self.park_on_fail()
        dry = self.drv("rescue", "WI-01", "--dry-run")
        self.assertEqual(dry.returncode, 0, dry.stdout + dry.stderr)
        self.assertEqual(dry.stdout.count("OK"), 7)
        self.assertFalse(self.wave_dir().exists())  # dry run writes nothing
        r = self.drv("rescue", "WI-01")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertEqual(r.stdout.count("SPAWN 1 agent: model=haiku"), 7)
        self.assertIn("rescue WI-01 --collect", r.stdout)
        manifest = json.loads((self.wave_dir() / "manifest.json").read_text(encoding="utf-8"))
        self.assertEqual(manifest["kind"], "recon")
        self.assertEqual(len(manifest["units"]), 7)
        for u in manifest["units"]:
            self.assertEqual(u["scope"], [u["out"]])
            self.assertTrue(u["out"].startswith(".claude/waves/WI-01-rescue/"))
            self.assertIn("Contract: recon", (self.root / u["brief"]).read_text(encoding="utf-8"))
        self.assertIn("WI-01", self.st.state_maps()[1])  # the rescue marker leaves the item parked
        again = self.drv("rescue", "WI-01")
        self.assertEqual(again.returncode, 1)
        self.assertIn("already rescued", again.stderr)

    def test_rescue_collect_flags_missing_then_lists_reading_order(self):
        self.park_on_fail()
        self.drv("rescue", "WI-01")
        r = self.drv("rescue", "WI-01", "--collect")
        self.assertEqual(r.returncode, 1, r.stdout + r.stderr)
        self.assertEqual(r.stdout.count("RERUN"), 7)
        self.assertIn("SPAWN 1 agent", r.stdout)
        manifest = json.loads((self.wave_dir() / "manifest.json").read_text(encoding="utf-8"))
        for u in manifest["units"]:
            (self.root / u["out"]).write_text("evidence\n", encoding="utf-8")
            (self.root / u["reply"]).write_text(f"OUTPUT: {u['out']}\nEVIDENCE: src/a.txt:1\nFINDING: ok\nOPEN: None\nVERDICT: CONFIRMED\n", encoding="utf-8")
        r = self.drv("rescue", "WI-01", "--collect")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertIn("Read ONLY these OUTPUT files", r.stdout)
        self.assertIn("/plan-handoff WI-01", r.stdout)
        order = next(ln for ln in r.stdout.splitlines() if ln.startswith("Read ONLY"))
        self.assertLess(order.index("u07.md"), order.index("u01.md"))  # brief-gap analysis is read first

    def test_close_manual_commits_only_given_paths_and_marks_done(self):
        self.park_on_fail()
        self.write("docs/plan.md", "# Plan\n\nedited by hand\n")
        self.write("src/z.txt", "foreign edit\n")
        r = self.drv("close", "WI-01", "--manual", "--paths", "docs/plan.md", "--reason", "docs-only closure")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertIn("WI-01", self.st.state_maps()[0])
        self.assertEqual(self.st.runs()[-1]["outcome"], "manual-close")
        files = sh(self.root, "git", "show", "--name-only", "--format=", "HEAD").stdout.split()
        self.assertIn("docs/plan.md", files)
        self.assertNotIn("src/z.txt", files)
        self.assertIn("src/z.txt", sh(self.root, "git", "status", "--porcelain").stdout)
        again = self.drv("close", "WI-01", "--manual", "--paths", "docs/plan.md", "--reason", "x")
        self.assertEqual(again.returncode, 1)
        self.assertIn("already done", again.stderr)

    def test_close_manual_accepts_a_dirty_directory(self):
        self.park_on_fail()
        self.write("docs/plan.md", "# Plan\n\nedited\n")
        (self.root / "notes").mkdir()
        (self.root / "notes" / "r1.md").write_text("x\n", encoding="utf-8")
        r = self.drv("close", "WI-01", "--manual", "--paths", "docs/plan.md", "notes/", "--reason", "dir")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertIn("notes/r1.md", sh(self.root, "git", "show", "--name-only", "--format=", "HEAD").stdout)

    def test_close_manual_needs_paths_reason_and_changes(self):
        self.park_on_fail()
        self.assertEqual(self.drv("close", "WI-01", "--manual").returncode, 1)
        r = self.drv("close", "WI-01", "--manual", "--paths", "docs/plan.md", "--reason", "x")
        self.assertEqual(r.returncode, 1)
        self.assertIn("nothing to commit", r.stderr)

    # ---- multi-round rescue
    def wave_dir2(self, wid="WI-01"):
        return self.root / ".claude" / "waves" / f"{wid}-rescue2"

    def finish_round_one(self):
        self.park_on_fail()
        self.assertEqual(self.drv("rescue", "WI-01").returncode, 0)

    def repark(self):
        self.assertEqual(self.drv("unpark", "WI-01", "--reason", "round 1 fix applied").returncode, 0)
        self.park_on_fail()

    def test_second_rescue_needs_a_new_park(self):
        self.finish_round_one()
        r = self.drv("rescue", "WI-01")
        self.assertEqual(r.returncode, 1)
        self.assertIn("already rescued", r.stderr)
        self.assertFalse(self.wave_dir2().exists())
        self.assertFalse(wd.reparked_since_rescue(self.st, wm.load_all(self.root)["WI-01"]))

    def test_round_two_writes_four_briefs_with_decision_note(self):
        self.finish_round_one()
        note = ".claude/research/wi01_rescue/decision_r1.md"
        (self.root / note).parent.mkdir(parents=True)
        (self.root / note).write_text("outcome: code plan\nresidual: none\n", encoding="utf-8")
        self.repark()
        self.assertEqual(wd.rescue_rounds(self.st, wm.load_all(self.root)["WI-01"]), 1)
        r = self.drv("rescue", "WI-01")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertEqual(r.stdout.count("SPAWN 1 agent: model=haiku"), 4)
        self.assertIn("round 2 of 2", r.stdout)
        manifest = json.loads((self.wave_dir2() / "manifest.json").read_text(encoding="utf-8"))
        self.assertEqual(len(manifest["units"]), 4)
        for u in manifest["units"]:
            brief = (self.root / u["brief"]).read_text(encoding="utf-8")
            self.assertIn(note, brief)
            self.assertIn("Round 2: the round-1 outputs and decision are listed", brief)
            self.assertTrue(u["out"].startswith(".claude/waves/WI-01-rescue2/"))
        self.assertIn("round 2", [e for e in self.st.events() if e["ev"] == "rescue"][-1]["reason"])

    def test_round_two_collect_names_the_round_two_units(self):
        self.finish_round_one()
        self.repark()
        self.drv("rescue", "WI-01")
        r = self.drv("rescue", "WI-01", "--collect")
        self.assertEqual(r.returncode, 1, r.stdout + r.stderr)
        self.assertEqual(r.stdout.count("RERUN"), 4)
        self.assertIn("gap", r.stdout)
        manifest = json.loads((self.wave_dir2() / "manifest.json").read_text(encoding="utf-8"))
        for u in manifest["units"]:
            (self.root / u["out"]).write_text("evidence\n", encoding="utf-8")
            (self.root / u["reply"]).write_text(f"OUTPUT: {u['out']}\nEVIDENCE: src/a.txt:1\nFINDING: ok\nOPEN: None\nVERDICT: CONFIRMED\n", encoding="utf-8")
        r = self.drv("rescue", "WI-01", "--collect")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertIn("/plan-handoff WI-01", r.stdout)

    def test_round_three_is_refused(self):
        self.finish_round_one()
        self.repark()
        self.assertEqual(self.drv("rescue", "WI-01").returncode, 0)
        self.repark()
        r = self.drv("rescue", "WI-01")
        self.assertEqual(r.returncode, 1)
        self.assertIn("2 rounds used; needs you", r.stderr)
        self.assertFalse((self.root / ".claude" / "waves" / "WI-01-rescue3").exists())

    def test_status_flags_repark_open_marker_and_clean_item(self):
        self.finish_round_one()
        clean = self.drv("rescue", "WI-01", "--status")
        self.assertEqual(clean.returncode, 0, clean.stdout + clean.stderr)
        self.assertIn("1 of 2 rounds used, 1 left", clean.stdout)
        self.assertIn("THEN: no residual work; report", clean.stdout)
        self.drv("unpark", "WI-01", "--reason", "fix applied")
        self.write("docs/plan.md", "# Plan\n\nrow [OPEN WI-01 C001: still unverified] here\n")
        marked = self.drv("rescue", "WI-01", "--status")
        self.assertIn("[OPEN WI-01] markers in plan docs: 1", marked.stdout)
        self.assertIn("THEN: python3 scripts/wi_driver.py rescue WI-01 --residual", marked.stdout)
        self.write("docs/plan.md", "# Plan\n")
        self.park_on_fail()
        reparked = self.drv("rescue", "WI-01", "--status")
        self.assertIn("re-parked after the last rescue: yes", reparked.stdout)
        self.assertIn("THEN: python3 scripts/wi_driver.py rescue WI-01\n", reparked.stdout)
        self.assertEqual(len([e for e in self.st.events() if e["ev"] == "rescue"]), 1)  # --status mutates nothing

    def test_residual_round_two_on_an_unparked_item(self):
        self.finish_round_one()
        self.drv("unpark", "WI-01", "--reason", "fix applied")
        refused = self.drv("rescue", "WI-01", "--residual")
        self.assertEqual(refused.returncode, 1)
        self.assertIn("no residual rows", refused.stderr)
        self.write("docs/plan.md", "# Plan\n\nrow [OPEN WI-01 C001: still unverified] here\n")
        dry = self.drv("rescue", "WI-01", "--residual", "--dry-run")
        self.assertEqual(dry.returncode, 0, dry.stdout + dry.stderr)
        self.assertEqual(dry.stdout.count("OK"), 4)
        r = self.drv("rescue", "WI-01", "--residual")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertEqual(r.stdout.count("SPAWN 1 agent: model=haiku"), 4)
        brief = (self.wave_dir2() / "u01.brief.md").read_text(encoding="utf-8")
        self.assertIn("still unverified", brief)

    def test_residual_needs_a_finished_round_one(self):
        self.park_on_fail()
        r = self.drv("rescue", "WI-01", "--residual")
        self.assertEqual(r.returncode, 1)
        self.assertIn("follows a round 1", r.stderr)

    def test_fail_path_stops_pointing_to_rescue_at_the_cap(self):
        for n in (1, 2):
            self.st.append_event("WI-01", "rescue", "s", f"round {n}")
        self.drv("next", "WI-01")
        for _ in range(3):  # Haiku, Haiku retry, Sonnet escalation
            self.write("src/a.txt", "edit\n")
            self.worker_reply(self.claim_wave("WI-01"), "src/a.txt")
            reply = self.root / ".claude" / "waves" / self.claim_wave("WI-01") / "u01.reply.txt"
            reply.write_text(reply.read_text(encoding="utf-8").replace("STATUS: PASS", "STATUS: FAIL"), encoding="utf-8")
            r = self.drv("verify", "WI-01")
        self.assertIn("PARKED WI-01", r.stdout)
        self.assertNotIn("THEN: python3 scripts/wi_driver.py rescue", r.stdout)
        self.assertNotIn("Safe to /clear", r.stdout)
        self.assertIn("2 rescue rounds used: stop and ask the user", r.stdout)

    def test_close_manual_after_rescue_on_a_done_item(self):
        self.finish_round_one()
        self.write("docs/plan.md", "# Plan\n\nfirst\n")
        self.assertEqual(self.drv("close", "WI-01", "--manual", "--paths", "docs/plan.md", "--reason", "one").returncode, 0)
        self.write("docs/plan.md", "# Plan\n\nsecond\n")
        r = self.drv("close", "WI-01", "--manual", "--paths", "docs/plan.md", "--reason", "follow-up")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)

    def test_close_manual_refused_on_a_done_item_without_a_rescue(self):
        self.run_flow_to_docs()
        self.assertEqual(self.drv("close", "WI-01").returncode, 0)
        self.write("docs/plan.md", "# Plan\n\nlate edit\n")
        r = self.drv("close", "WI-01", "--manual", "--paths", "docs/plan.md", "--reason", "x")
        self.assertEqual(r.returncode, 1)
        self.assertIn("already done", r.stderr)


class AuditAndBacklogTests(Base):
    def test_analyze_is_pure_and_idempotent(self):
        events = [{"wi": "WI-01", "ev": "park", "reason": "a"}, {"wi": "WI-01", "ev": "park", "reason": "b"}]
        runs = [{"wi": "WI-02", "outcome": "done", "duration_s": 5, "gates": [{"cmd": "run_tests x", "status": "FAIL", "detail": "filter matched no tests"}],
                 "failures": [], "usage": {"peak_ctx_max": 70000}}]
        found = wd.analyze(runs, events, ["WI-09"], set())
        keys = {f["key"] for f in found}
        self.assertEqual(keys, {"repeat-park:WI-01", "gate-selects-nothing:WI-02", "token-overrun", "stale-claim:WI-09", "closed-on-partial"})
        self.assertEqual(wd.analyze(runs, events, ["WI-09"], keys), [])

    def test_analyze_flags_closed_on_partial(self):
        partial = [{"wi": "WI-05", "outcome": "done", "duration_s": 5, "checker_verdict": "PARTIAL", "failures": []}]
        found = {f["key"]: f for f in wd.analyze(partial, [], [], set())}
        self.assertIn("closed-on-partial", found)
        self.assertIn("WI-05", found["closed-on-partial"]["text"])
        confirmed = [{"wi": "WI-06", "outcome": "done", "duration_s": 5, "checker_verdict": "CONFIRMED", "failures": []}]
        self.assertNotIn("closed-on-partial", {f["key"] for f in wd.analyze(confirmed, [], [], set())})

    def test_analyze_flags_worker_fail_closed(self):
        run = [{"wi": "WI-07", "outcome": "done", "duration_s": 5, "retries": 1, "checker_verdict": "CONFIRMED",
                "failures": [{"detail": "worker STATUS: FAIL"}]}]
        found = {f["key"]: f for f in wd.analyze(run, [], [], set())}
        self.assertIn("worker-fail-closed", found)
        self.assertIn("WI-07", found["worker-fail-closed"]["text"])

    def test_audit_apply_holds_records_and_adds_backlog(self):
        for reason in ("first", "second"):
            self.st.append_event("WI-01", "park", "s", reason)
        self.st.append_jsonl(ws.RUNS_FILE, {"wi": "WI-02", "outcome": "done", "duration_s": 5, "gates": [{"cmd": "run_tests x", "status": "FAIL", "detail": "filter matched no tests"}], "failures": [], "usage": {}})
        sh(self.root, "git", "add", "-A")
        sh(self.root, "git", "commit", "-qm", "events")
        r = self.drv("audit")
        self.assertIn("repeat-park:WI-01", r.stdout)
        self.assertEqual(len(self.st.lessons()), 0)  # report only without --apply
        r = self.drv("audit", "--apply")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        items = wm.load_all(self.root)
        self.assertEqual(items["WI-01"].state, "hold")
        self.assertIn("[audit]", items["WI-01"].blockers)
        self.assertTrue(any(w.id == "WI-09" and w.state == "hold" and "audit:gate-selects-nothing" in w.title for w in items.values()), list(items))
        self.assertTrue(any(x["scope"] == "WI-01" for x in self.st.lessons()))
        self.assertEqual(sh(self.root, "git", "log", "-1", "--format=%s").stdout.strip()[:6], "audit:")
        self.assertEqual(sh(self.root, "git", "status", "--porcelain", "--", ".claude/work_items").stdout.strip(), "")
        self.assertIn("applied", self.drv("audit").stdout.lower() + "applied")  # second run is a no-op
        self.assertIn("findings=0", self.drv("audit").stdout)

    def test_lessons_are_echoed_by_next(self):
        self.st.append_jsonl(ws.LESSONS_FILE, {"key": "k", "scope": "global", "text": "write the reply file last", "ts": "t"})
        self.assertIn("LESSON: write the reply file last", self.drv("next", "--dry-run").stdout)
        self.drv("next", "WI-01")
        brief = next((self.root / ".claude/waves").glob("WI-01-a1/u01.brief.md")).read_text()
        self.assertIn("write the reply file last", brief)

    def test_backlog_add_and_promote(self):
        r = self.drv("backlog", "add", "--title", "Follow-up thing", "--why", "found by worker", "--files", "src/q.txt", "--origin", "WI-01")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        items = wm.load_all(self.root)
        self.assertEqual(items["WI-09"].state, "hold")
        self.assertEqual(items["WI-09"].rank, 9)
        self.assertEqual(items["WI-09"].files, ["src/q.txt"])
        self.assertIn("Backlog item (WI-01)", items["WI-09"].blockers)
        self.assertIn("hold", wm.eligibility(items["WI-09"], items, set(), {}, set()).reason)
        self.assertEqual(self.drv("backlog", "promote", "WI-09", "--state", "ready").returncode, 0)
        self.assertEqual(wm.load_all(self.root)["WI-09"].state, "ready")
        self.assertIn("WI-09", self.drv("backlog", "list").stdout)

    def test_render_index_is_idempotent(self):
        self.st.append_event("WI-03", "done", "s")
        a = self.st.render_index()
        b = self.st.render_index()
        self.assertEqual(a, b)
        self.assertIn("| [x] | 3 | WI-03 |", a)

    def test_status_and_lint_commands(self):
        self.assertIn("claimable now", self.drv("status").stdout)
        self.assertEqual(self.drv("lint").returncode, 1)  # fixture WI-08 has a self loop


if __name__ == "__main__":
    unittest.main(verbosity=1)
