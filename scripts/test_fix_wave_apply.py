#!/usr/bin/env python3
"""Host tests for fix_wave_apply.py. Each test uses a temp git repo and a temp waves root; gate commands are stubbed."""
import contextlib
import io
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPTS))
import fix_wave_apply as fwa  # noqa: E402

ITEM_COLS = ["id", "kind", "size", "group", "after", "fids", "write", "tests", "mods", "entries", "anchors",
             "constraint", "context", "doc"]
BASE = "alpha\nbravo\ncharlie\ndelta\necho\nfoxtrot\n"
DEMO = "patch_manager_mods/demo_src/demo.txt"
SHARED = "patch_manager_mods/_shared_src/Shared.h"


def git(repo, *args):
    return subprocess.run(["git", "-c", "user.name=t", "-c", "user.email=t@example.invalid", "-c", "commit.gpgsign=false",
                           *args], cwd=repo, check=True, capture_output=True, text=True).stdout


class Stub:
    """Gate runner: git passes through to the real runner; gate commands are logged and return PASS unless scripted to fail."""

    def __init__(self, fail=None):
        self.fail = dict(fail or {})
        self.calls = []

    def __call__(self, cmd, cwd):
        if cmd[0] == "git":
            return fwa.run(cmd, cwd)
        self.calls.append(list(cmd))
        name = Path(cmd[1]).name
        if name in self.fail:
            return self.fail[name], f"scripted failure: {name}"
        return 0, "PASS"

    def gate_names(self):
        return [Path(c[1]).name for c in self.calls]

    def calls_for(self, name):
        return [c for c in self.calls if Path(c[1]).name == name]


class ApplyCase(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = Path(self._tmp.name)
        self.repo = self.tmp / "repo"
        self.waves = self.tmp / "waves"
        self.batch = self.waves / "fx4-A"
        self.repo.mkdir()
        self.batch.mkdir(parents=True)
        git(self.repo, "init", "-q")
        self.write_repo(DEMO, BASE)
        self.write_repo(SHARED, "int shared = 1;\n")
        git(self.repo, "add", "-A")
        git(self.repo, "commit", "-q", "-m", "base")

    def tearDown(self):
        self._tmp.cleanup()

    def write_repo(self, rel, text):
        path = self.repo / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")

    def diff_for(self, rel, new_text):
        """git diff of rel against HEAD for new_text, leaving the working tree as it was."""
        path = self.repo / rel
        old = path.read_text(encoding="utf-8")
        path.write_text(new_text, encoding="utf-8")
        diff = git(self.repo, "diff", "--no-color", "--no-ext-diff", "--src-prefix=a/", "--dst-prefix=b/", "--", rel)
        path.write_text(old, encoding="utf-8")
        return diff

    def add_item(self, item_id, write, diff_text, tests="py:demo", mods="demo_src",
                 constraint="Uppercase bravo.", review="DECISION: APPROVE\nBLOCKING: 0\n"):
        row = {"id": item_id, "kind": "bug", "size": "S", "group": "-", "after": "", "fids": "", "write": write,
               "tests": tests, "mods": mods, "entries": "", "anchors": "", "constraint": constraint,
               "context": "", "doc": ""}
        items = self.batch / "items.tsv"
        lines = items.read_text(encoding="utf-8").splitlines() if items.is_file() else ["\t".join(ITEM_COLS)]
        lines.append("\t".join(row[c] for c in ITEM_COLS))
        items.write_text("\n".join(lines) + "\n", encoding="utf-8")
        item_dir = self.batch / item_id
        item_dir.mkdir(parents=True, exist_ok=True)
        (item_dir / "final.diff").write_text(diff_text, encoding="utf-8")
        if review is not None:
            (item_dir / "review.md").write_text(review, encoding="utf-8")

    def apply(self, item_id, runner=None, dry_run=False):
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = fwa.apply_item(self.waves, self.repo, "fx4-A", item_id, dry_run=dry_run, runner=runner or Stub())
        return rc, buf.getvalue()

    def state(self):
        return fwa.read_state(self.batch)


class ApplyTests(ApplyCase):
    def test_clean_apply_gates_and_prints_wi_close_command(self):
        self.add_item("WI-12", "demo_src/demo.txt", self.diff_for(DEMO, BASE.replace("bravo", "BRAVO")))
        stub = Stub()
        rc, out = self.apply("WI-12", stub)
        self.assertEqual(rc, 0, out)
        self.assertEqual((self.repo / DEMO).read_text(encoding="utf-8"), BASE.replace("bravo", "BRAVO"))
        self.assertEqual(self.state()["WI-12"]["stage"], "gated")
        self.assertEqual(stub.gate_names(), ["run_tests.py", "build_check.py"])
        self.assertIn("python3 scripts/wi_driver.py release WI-12 && "
                      "python3 scripts/wi_driver.py close WI-12 --manual --paths " + DEMO +
                      ' --reason "WI-12 fix wave fx4-A"', out)
        self.assertEqual(out.count("close: "), 1)

    def test_non_wi_id_prints_git_commit_form(self):
        self.add_item("K1-5a-01", "demo_src/demo.txt", self.diff_for(DEMO, BASE.replace("bravo", "BRAVO")),
                      constraint="Keep bravo upper.")
        rc, out = self.apply("K1-5a-01")
        self.assertEqual(rc, 0, out)
        self.assertIn(f'git commit -m "K1-5a-01: Keep bravo upper." -- {DEMO}', out)
        self.assertNotIn("wi_driver", out)
        self.assertEqual(self.state()["K1-5a-01"]["stage"], "gated")

    def test_conflict_restores_tree_and_requests_rebase(self):
        self.add_item("WI-12", "demo_src/demo.txt", self.diff_for(DEMO, BASE.replace("bravo", "BRAVO")))
        self.write_repo(DEMO, BASE.replace("bravo", "BRAVO-OURS"))
        git(self.repo, "commit", "-q", "-am", "ours")
        before_bytes = (self.repo / DEMO).read_bytes()
        before_status = git(self.repo, "status", "--porcelain")
        stub = Stub()
        rc, out = self.apply("WI-12", stub)
        self.assertEqual(rc, 4, out)
        self.assertIn("REBASE WI-12: send consolidator a rebase request (1 allowed, then park)", out)
        self.assertEqual((self.repo / DEMO).read_bytes(), before_bytes)
        self.assertNotIn(b"<<<<<<<", (self.repo / DEMO).read_bytes())
        self.assertEqual(git(self.repo, "status", "--porcelain"), before_status)
        self.assertEqual(stub.calls, [])
        self.assertNotIn("WI-12", self.state())

    def test_three_way_clean_merge_leaves_changes_unstaged(self):
        # The committed change at line 4 sits inside the diff's context, so --check fails and -3 merges cleanly.
        self.add_item("WI-12", "demo_src/demo.txt", self.diff_for(DEMO, BASE.replace("bravo", "BRAVO")))
        self.write_repo(DEMO, BASE.replace("delta", "DELTA"))
        git(self.repo, "commit", "-q", "-am", "ours")
        rc, out = self.apply("WI-12")
        self.assertEqual(rc, 0, out)
        text = (self.repo / DEMO).read_text(encoding="utf-8")
        self.assertIn("BRAVO", text)
        self.assertIn("DELTA", text)
        self.assertEqual(git(self.repo, "diff", "--cached", "--name-only"), "")
        self.assertTrue(git(self.repo, "status", "--porcelain").startswith(" M"))

    def test_request_changes_is_refused_and_nothing_applies(self):
        self.add_item("WI-12", "demo_src/demo.txt", self.diff_for(DEMO, BASE.replace("bravo", "BRAVO")),
                      review="DECISION: REQUEST_CHANGES\nBLOCKING: 2\n")
        stub = Stub()
        rc, out = self.apply("WI-12", stub)
        self.assertEqual(rc, 2, out)
        self.assertIn("REFUSED WI-12", out)
        self.assertEqual((self.repo / DEMO).read_text(encoding="utf-8"), BASE)
        self.assertEqual(stub.calls, [])
        self.assertNotIn("WI-12", self.state())

    def test_dry_run_changes_nothing(self):
        self.add_item("WI-12", "demo_src/demo.txt", self.diff_for(DEMO, BASE.replace("bravo", "BRAVO")))
        stub = Stub()
        rc, out = self.apply("WI-12", stub, dry_run=True)
        self.assertEqual(rc, 0, out)
        self.assertIn("DRY-RUN WI-12", out)
        self.assertEqual((self.repo / DEMO).read_text(encoding="utf-8"), BASE)
        self.assertEqual(stub.calls, [])
        self.assertFalse((self.batch / "state.tsv").exists())

    def test_failing_run_tests_exits_5_and_leaves_diff_applied(self):
        self.add_item("WI-12", "demo_src/demo.txt", self.diff_for(DEMO, BASE.replace("bravo", "BRAVO")))
        stub = Stub(fail={"run_tests.py": 1})
        rc, out = self.apply("WI-12", stub)
        self.assertEqual(rc, 5, out)
        self.assertIn("GATE FAIL run_tests", out)
        self.assertIn("git apply -R", out)
        self.assertIn("BRAVO", (self.repo / DEMO).read_text(encoding="utf-8"))
        self.assertEqual(stub.gate_names(), ["run_tests.py"])
        self.assertNotIn("WI-12", self.state())

    def test_shared_src_runs_compat_check_and_shared_headers_sync(self):
        self.add_item("WI-40", "_shared_src/Shared.h", self.diff_for(SHARED, "int shared = 2;\n"),
                      tests="cpp:guikit_test", mods="demo_src")
        stub = Stub()
        rc, out = self.apply("WI-40", stub)
        self.assertEqual(rc, 0, out)
        compat = stub.calls_for("compat_check.py")
        self.assertEqual(len(compat), 1)
        self.assertEqual(compat[0][2:], [SHARED])
        headers = stub.calls_for("check_shared_headers.py")
        self.assertEqual(len(headers), 1)
        self.assertEqual(headers[0][2:], ["--sync"])

    def test_diff_outside_write_column_is_refused(self):
        combined = self.diff_for(DEMO, BASE.replace("bravo", "BRAVO")) + self.diff_for(SHARED, "int shared = 2;\n")
        self.add_item("WI-12", "demo_src/demo.txt", combined)
        stub = Stub()
        rc, out = self.apply("WI-12", stub)
        self.assertEqual(rc, 2, out)
        self.assertIn("outside the write column", out)
        self.assertEqual(stub.calls, [])
        self.assertEqual((self.repo / SHARED).read_text(encoding="utf-8"), "int shared = 1;\n")

    def test_second_apply_of_gated_item_is_refused(self):
        self.add_item("WI-12", "demo_src/demo.txt", self.diff_for(DEMO, BASE.replace("bravo", "BRAVO")))
        self.assertEqual(self.apply("WI-12")[0], 0)
        rc, out = self.apply("WI-12")
        self.assertEqual(rc, 2, out)
        self.assertIn("already gated", out)


class CliAndHelperTests(ApplyCase):
    def test_commit_flag_writes_applied_row(self):
        self.add_item("WI-12", "demo_src/demo.txt", self.diff_for(DEMO, BASE.replace("bravo", "BRAVO")))
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = fwa.main(["apply", "fx4-A", "WI-12", "--commit", "abc123",
                           "--waves-root", str(self.waves), "--repo-root", str(self.repo)])
        self.assertEqual(rc, 0, buf.getvalue())
        self.assertEqual(self.state()["WI-12"], {"stage": "applied", "commit": "abc123"})
        self.assertEqual((self.repo / DEMO).read_text(encoding="utf-8"), BASE)

    def test_main_dry_run_uses_real_git_check(self):
        self.add_item("WI-12", "demo_src/demo.txt", self.diff_for(DEMO, BASE.replace("bravo", "BRAVO")))
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = fwa.main(["apply", "fx4-A", "WI-12", "--dry-run",
                           "--waves-root", str(self.waves), "--repo-root", str(self.repo)])
        self.assertEqual(rc, 0, buf.getvalue())
        self.assertEqual((self.repo / DEMO).read_text(encoding="utf-8"), BASE)

    def test_ready_rules(self):
        diff = self.diff_for(DEMO, BASE.replace("bravo", "BRAVO"))
        cases = [
            ("DECISION: APPROVE\nBLOCKING: 1\n", False),
            ("**DECISION:** APPROVE_MINOR\n**BLOCKING:** 0\n", True),
            ("DECISION: APPROVE\nBLOCKING: x\n", False),
            ("DECISION: APPROVE\n", True),
            ("no verdict here\n", False),
        ]
        for text, want in cases:
            with self.subTest(review=text):
                self.add_item("WI-12", "demo_src/demo.txt", diff, review=text)
                self.assertEqual(fwa.ready(self.batch, "WI-12")[0], want)
        self.add_item("WI-13", "demo_src/demo.txt", diff, review=None)
        self.assertEqual(fwa.ready(self.batch, "WI-13"), (False, "no review.md"))

    def test_close_command_forms(self):
        wi = {"id": "WI-12", "write": "demo_src/demo.txt", "constraint": "x"}
        self.assertEqual(fwa.close_command(wi, "fx4-A"),
                         'python3 scripts/wi_driver.py release WI-12 && '
                         'python3 scripts/wi_driver.py close WI-12 --manual --paths ' + DEMO +
                         ' --reason "WI-12 fix wave fx4-A"')
        other = {"id": "K1-5a-01", "write": "demo_src/demo.txt;_shared_src/Shared.h",
                 "constraint": "A" * 80}
        self.assertEqual(fwa.close_command(other, "fx4-A"),
                         'git commit -m "K1-5a-01: ' + "A" * 60 + '" -- ' + DEMO + " " + SHARED)


if __name__ == "__main__":
    unittest.main()
