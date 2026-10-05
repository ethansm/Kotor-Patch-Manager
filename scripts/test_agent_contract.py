#!/usr/bin/env python3
"""test_agent_contract.py - unit tests for agent_contract.py (stdlib unittest, temp git repos)."""
import io
import json
import re
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "scripts"))
import agent_contract as ac  # noqa: E402

WORKER_OK = """CREATED: a/new.py
MODIFIED: a/old.py
TESTS: python3 -m unittest a/test.py -> 3/3 PASS
NOTES: None
STATUS: PASS
"""
RECON_OK = """OUTPUT: notes/find.md
EVIDENCE: a/old.py:1; 0x00712f30
FINDING: old.py has one line
OPEN: None
VERDICT: CONFIRMED
"""


def git(root, *args):
    subprocess.run(["git", *args], cwd=root, check=True, capture_output=True)


class Repo:
    def __init__(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        git(self.root, "init", "-q")
        git(self.root, "config", "user.email", "t@t")
        git(self.root, "config", "user.name", "t")
        self.write("a/old.py", "x = 1\n")
        self.write("b/other.py", "y = 1\n")
        git(self.root, "add", "-A")
        git(self.root, "commit", "-qm", "init")

    def write(self, rel, text):
        p = self.root / rel
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(text)

    def run(self, *args, stdin=""):
        out = io.StringIO()
        argv = list(args) if args[0] == "print" else [args[0], "--repo-root", str(self.root)] + list(args[1:])
        rc = ac.main(argv, out=out, stdin=io.StringIO(stdin))
        return rc, out.getvalue()


def codes(text):
    return re.findall(r"^(?:FAIL|WARN) (\w+)", text, re.M)


class TestParse(unittest.TestCase):
    def test_worker_ok(self):
        kind, _, probs = ac.parse(WORKER_OK)
        self.assertEqual((kind, probs), ("worker", []))

    def test_recon_ok(self):
        kind, _, probs = ac.parse(RECON_OK)
        self.assertEqual((kind, probs), ("recon", []))

    def test_fenced_and_blank_lines(self):
        _, _, probs = ac.parse("\n```text\n" + WORKER_OK + "```\n\n")
        self.assertEqual(probs, [])

    def test_prose_fails(self):
        _, _, probs = ac.parse("Done! All good.\n" + WORKER_OK)
        self.assertIn(("FAIL", "PROSE"), [(l, c) for l, c, _ in probs])

    def test_missing_key(self):
        _, _, probs = ac.parse(WORKER_OK.replace("NOTES: None\n", ""))
        self.assertIn("MISSING_KEY", [c for _, c, _ in probs])

    def test_order_is_warn_only(self):
        lines = WORKER_OK.strip().splitlines()
        _, _, probs = ac.parse("\n".join([lines[-1]] + lines[:-1]))
        self.assertEqual([(l, c) for l, c, _ in probs], [("WARN", "ORDER")])

    def test_bad_status(self):
        _, _, probs = ac.parse(WORKER_OK.replace("STATUS: PASS", "STATUS: MAYBE"))
        self.assertIn("BAD_VALUE", [c for _, c, _ in probs])

    def test_tests_form_warn(self):
        _, _, probs = ac.parse(WORKER_OK.replace("python3 -m unittest a/test.py -> 3/3 PASS", "all passed"))
        self.assertEqual([c for _, c, _ in probs], ["TESTS_FORM"])

    def test_recon_evidence_empty(self):
        _, _, probs = ac.parse(RECON_OK.replace("a/old.py:1; 0x00712f30", "None"))
        self.assertIn("EVIDENCE_EMPTY", [c for _, c, _ in probs])

    def test_template_round_trips_keys(self):
        for kind in ac.SPECS:
            body = ac.template(kind).splitlines()[1:]
            self.assertEqual([l.split(":")[0] for l in body], ac.keys(kind))


class TestGit(unittest.TestCase):
    def setUp(self):
        self.r = Repo()

    def tearDown(self):
        self.r.tmp.cleanup()

    def test_worker_pass_with_baseline(self):
        self.r.write("b/other.py", "y = 2\n")  # pre-existing dirt, not this agent's
        self.assertEqual(self.r.run("snapshot")[0], 0)
        self.r.write("a/new.py", "n = 1\n")
        self.r.write("a/old.py", "x = 2\n")
        rc, out = self.r.run("check", "-", stdin=WORKER_OK)
        self.assertEqual(rc, 0, out)
        self.assertEqual(codes(out), [])

    def test_no_path_and_not_dirty(self):
        rc, out = self.r.run("check", "-", stdin=WORKER_OK)
        self.assertEqual(rc, 1)
        self.assertIn("NO_PATH", codes(out))
        self.assertIn("NOT_DIRTY", codes(out))

    def test_already_dirty_file_unchanged_since_baseline(self):
        self.r.write("a/old.py", "x = 2\n")
        self.r.write("a/new.py", "n = 1\n")
        self.r.run("snapshot")
        rc, out = self.r.run("check", "-", stdin=WORKER_OK)
        self.assertEqual(rc, 1)
        self.assertEqual(codes(out).count("NOT_DIRTY"), 2)

    def test_unclaimed_warn(self):
        self.r.run("snapshot")
        self.r.write("a/new.py", "n = 1\n")
        self.r.write("a/old.py", "x = 2\n")
        self.r.write("b/other.py", "y = 3\n")
        rc, out = self.r.run("check", "-", stdin=WORKER_OK)
        self.assertEqual(rc, 0, out)
        self.assertEqual(codes(out), ["UNCLAIMED"])

    def test_out_of_scope(self):
        self.r.run("snapshot")
        self.r.write("a/new.py", "n = 1\n")
        self.r.write("a/old.py", "x = 2\n")
        self.r.write("b/other.py", "y = 3\n")
        rc, out = self.r.run("check", "--scope", "a/*", "--", "-", stdin=WORKER_OK)
        self.assertEqual(rc, 1)
        self.assertIn("OUT_OF_SCOPE", codes(out))
        self.assertIn("b/other.py", out)

    def test_recon_output_must_exist(self):
        rc, out = self.r.run("check", "-", stdin=RECON_OK)
        self.assertEqual(rc, 1)
        self.assertIn("NO_PATH", codes(out))
        self.r.write("notes/find.md", "# f\n")
        rc, out = self.r.run("check", "--scope", "notes/*", "--", "-", stdin=RECON_OK)
        self.assertEqual(rc, 0, out)

    def test_json_shape(self):
        rc, out = self.r.run("check", "--json", "-", stdin="hello\n")
        data = json.loads(out)
        self.assertEqual(rc, 1)
        self.assertEqual(data["verdict"], "FAIL")
        self.assertTrue({"type", "fails", "warns", "problems", "baseline"} <= set(data))


class TestDocsDrift(unittest.TestCase):
    """AGENTS.md and CLAUDE.md carry a short pointer; its key order must match the templates."""

    def section(self, name):
        text = (REPO_ROOT / name).read_text(encoding="utf-8")
        m = re.search(r"^## Subagent Contracts.*?(?=^## |\Z)", text, re.M | re.S)
        self.assertIsNotNone(m, "%s has no '## Subagent Contracts' section" % name)
        return m.group(0).strip()

    def test_order_line_matches_templates(self):
        for name in ("AGENTS.md", "CLAUDE.md"):
            sec = self.section(name)
            for kind in ac.SPECS:
                m = re.search(kind + r" ([A-Z,]+)", sec)
                self.assertIsNotNone(m, "%s: no '%s KEY,KEY' order list" % (name, kind))
                self.assertEqual(m.group(1).split(","), ac.keys(kind), name)

    def test_files_identical(self):
        self.assertEqual(self.section("AGENTS.md"), self.section("CLAUDE.md"))


if __name__ == "__main__":
    unittest.main()
