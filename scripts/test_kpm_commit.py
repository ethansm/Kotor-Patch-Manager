#!/usr/bin/env python3
"""Tests for kpm_commit.py: a tmp git repo stands in for the worktree (REPO/MODS patched), tests are stubbed."""
import contextlib
import io
import pathlib
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import kpm_commit as kc  # noqa: E402

MANIFEST = "[patch]\nid = '{id}'\nversion = '0.1.0'\n"


class Base(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.repo = pathlib.Path(self.tmp.name)
        self.mods = self.repo / "patch_manager_mods"
        self.git("init", "-q")
        self.git("config", "user.email", "t@example.com")
        self.git("config", "user.name", "t")
        self.make_mod("demo")
        self.git("add", "-A")
        self.git("commit", "-qm", "init")
        for attr, val in (("REPO", self.repo), ("MODS", self.mods)):
            p = mock.patch.object(kc, attr, val)
            p.start()
            self.addCleanup(p.stop)
        self.tests_rc = 0
        self.tests_calls = []
        p = mock.patch.object(kc, "run_mod_tests", self.fake_tests)
        p.start()
        self.addCleanup(p.stop)

    def fake_tests(self, stem):
        self.tests_calls.append(stem)
        return self.tests_rc, "boom" if self.tests_rc else "ok"

    def git(self, *args):
        return subprocess.run(["git", "-C", str(self.repo), *args], capture_output=True, text=True, check=True).stdout

    def make_mod(self, name):
        d = self.mods / f"{name}_src"
        d.mkdir(parents=True)
        (d / "manifest.toml").write_text(MANIFEST.format(id=name))
        (d / f"{name.capitalize()}.cpp").write_text("int a;\n")
        (d / f"{name.capitalize()}_test.cpp").write_text("int t;\n")
        (d / "README.md").write_text("doc\n")

    def src(self, name="demo"):
        return self.mods / f"{name}_src"

    def count(self):
        return int(self.git("rev-list", "--count", "HEAD"))

    def status(self):
        return self.git("status", "--porcelain", "-uall")

    def run_cli(self, *args):
        out = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(out):
            rc = kc.main(list(args))
        return rc, out.getvalue()


class CommitTest(Base):
    def test_clean_tree_prints_nothing_to_commit(self):
        rc, out = self.run_cli()
        self.assertEqual(rc, 0, out)
        self.assertIn("All mod directories clean; nothing to commit", out)

    def test_modified_production_file_committed(self):
        (self.src() / "Demo.cpp").write_text("int b;\n")
        rc, out = self.run_cli()
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.git("log", "-1", "--format=%s").strip(), "feat(demo): update production sources")
        self.assertNotIn("Demo.cpp", self.status())
        self.assertEqual(self.tests_calls, ["demo"])

    def test_untracked_new_cpp_staged_and_committed(self):
        (self.src() / "Extra.h").write_text("#pragma once\n")
        rc, out = self.run_cli()
        self.assertEqual(rc, 0, out)
        self.assertIn("Extra.h", self.git("show", "--stat", "HEAD"))

    def test_untracked_new_subdir_listed_per_file(self):
        (self.src() / "sub").mkdir()
        (self.src() / "sub" / "Deep.cpp").write_text("int d;\n")
        rc, out = self.run_cli()
        self.assertEqual(rc, 0, out)
        self.assertIn("sub/Deep.cpp", self.git("show", "--stat", "HEAD"))

    def test_non_production_files_ignored(self):
        (self.src() / "README.md").write_text("changed\n")
        (self.src() / "Demo_test.cpp").write_text("int t2;\n")
        (self.src() / "x.bak").write_text("old\n")
        rc, out = self.run_cli()
        self.assertEqual(rc, 0, out)
        self.assertIn("All mod directories clean; nothing to commit", out)
        st = self.status()
        for f in ("README.md", "Demo_test.cpp", "x.bak"):
            self.assertIn(f, st)
        self.assertEqual(self.tests_calls, [])

    def test_test_failure_aborts_without_commit(self):
        (self.src() / "Demo.cpp").write_text("int b;\n")
        before = self.count()
        self.tests_rc = 1
        rc, out = self.run_cli()
        self.assertEqual(rc, 1, out)
        self.assertIn("TEST FAIL for demo; commit aborted", out)
        self.assertEqual(self.count(), before)

    def test_dry_run_never_runs_tests_or_commits(self):
        (self.src() / "Demo.cpp").write_text("int b;\n")
        before = self.count()
        rc, out = self.run_cli("--dry-run")
        self.assertEqual(rc, 0, out)
        self.assertIn("DRY  demo", out)
        self.assertEqual(self.tests_calls, [])
        self.assertEqual(self.count(), before)

    def test_prefix_and_mod_filter(self):
        self.make_mod("other")
        self.git("add", "-A")
        self.git("commit", "-qm", "other")
        (self.src() / "Demo.cpp").write_text("int b;\n")
        (self.src("other") / "Other.cpp").write_text("int o;\n")
        rc, out = self.run_cli("--mod", "demo", "--prefix", "fix")
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.git("log", "-1", "--format=%s").strip(), "fix(demo): update production sources")
        st = self.status()
        self.assertIn("Other.cpp", st)
        self.assertNotIn("Demo.cpp", st)
        rc, out = self.run_cli("--mod", "nope")
        self.assertEqual(rc, 2, out)
        self.assertIn("other_src", out)

    def test_mod_accepts_manifest_id_and_src_name(self):
        (self.src() / "Demo.cpp").write_text("int b;\n")
        rc, out = self.run_cli("--mod", "demo_src", "--dry-run")
        self.assertEqual(rc, 0, out)
        self.assertIn("DRY  demo", out)


if __name__ == "__main__":
    unittest.main()
