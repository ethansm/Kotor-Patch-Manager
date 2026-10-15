#!/usr/bin/env python3
"""Host-only tests for review_bundle.py (temp git repo fixture; no network)."""
import io
import os
import subprocess
import sys
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import review_bundle as rb  # noqa: E402
import wave_budget as W  # noqa: E402

MANIFEST = """
[defaults]
cap_tokens = 5000
max_file_tokens = 2000

[[bundle]]
id = "b1"
title = "Bundle One"
roots = ["patch_manager_mods/mod_src"]
docs = ["patch_manager_mods/10_mod.md"]
deps = true
focus = "look at X"

[[bundle]]
id = "tiny"
title = "Tiny"
roots = ["patch_manager_mods/mod_src/a.cpp"]
cap_tokens = 10
"""

FILES = {
    "patch_manager_mods/10_mod.md": "# Mod design\nprose\n",
    "patch_manager_mods/mod_src/a.cpp": '#include "../hud_src/Hh.h"\n#include "Shared.h"\n#include "KpLog.h"\n#include <windows.h>\nint a(){return 1;}\n',
    "patch_manager_mods/mod_src/z_test.cpp": 'int t(){return 2;}\nconst char*s="</document> bad";\n',
    "patch_manager_mods/mod_src/manifest.toml": 'name = "m"\n',
    "patch_manager_mods/mod_src/x_credential_probe.txt": "hunter2\n",
    "patch_manager_mods/mod_src/my_secret_dir/ok.cpp": "int s;\n",
    "patch_manager_mods/mod_src/Big.inc": "0x00,\n" * 3000,
    "patch_manager_mods/mod_src/blob.bin": "binary-ish",
    "patch_manager_mods/mod_src/__pycache__/x.pyc": "junk",
    "patch_manager_mods/mod_src/tool.py": "print('hi')\n",
    "patch_manager_mods/mod_src/test_tool.py": "def test(): pass\n",
    "patch_manager_mods/hud_src/Hh.h": '#include "Deep.h"\nint hh;\n',
    "patch_manager_mods/hud_src/Deep.h": "int deep;\n",
    "patch_manager_mods/_shared_src/Shared.h": "int shared;\n",
}


def run(cmd, cwd):
    subprocess.run(cmd, cwd=cwd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


class Fixture(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        run(["git", "init", "-q"], self.root)
        run(["git", "config", "user.email", "t@example.com"], self.root)
        run(["git", "config", "user.name", "t"], self.root)
        for rel, content in FILES.items():
            p = Path(self.root, rel)
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_text(content, encoding="utf-8")
        Path(self.root, "patch_manager_mods/review_bundles.toml").write_text(MANIFEST, encoding="utf-8")
        run(["git", "add", "-A", "-f"], self.root)
        run(["git", "commit", "-q", "-m", "init"], self.root)
        self.out = os.path.join(self.root, "out")

    def tearDown(self):
        self.tmp.cleanup()

    def cli(self, *args):
        o, e = io.StringIO(), io.StringIO()
        with redirect_stdout(o), redirect_stderr(e):
            rc = rb.main(["--repo-root", self.root, *args])
        return rc, o.getvalue(), e.getvalue()

    def plan(self, bid="b1", **kw):
        cfg, bundles = rb.load_manifest(os.path.join(self.root, rb.DEFAULT_MANIFEST))
        b = next(x for x in bundles if x["id"] == bid)
        return rb.make_plan(self.root, b, cfg, rb.tracked_files(self.root), rb.dirty_files(self.root), "abc", kw.get("ln", False))


class GlobTests(unittest.TestCase):
    def test_double_star(self):
        self.assertTrue(rb.matches("a/**/test_*.py", "a/b/c/test_x.py"))
        self.assertTrue(rb.matches("a/**/test_*.py", "a/test_x.py"))
        self.assertFalse(rb.matches("a/**/test_*.py", "a/b/x.py"))
        self.assertTrue(rb.matches("**/test_*.py", "test_x.py"))
        self.assertTrue(rb.matches("a/**", "a/b/c"))

    def test_single_star_stops_at_slash(self):
        self.assertTrue(rb.matches("a/*.cpp", "a/x.cpp"))
        self.assertFalse(rb.matches("a/*.cpp", "a/b/x.cpp"))

    def test_plain_prefix_is_directory_or_file(self):
        self.assertTrue(rb.matches("a/mod_src", "a/mod_src/x.cpp"))
        self.assertTrue(rb.matches("a/mod_src/x.cpp", "a/mod_src/x.cpp"))
        self.assertFalse(rb.matches("a/mod", "a/mod_src/x.cpp"))

    def test_token_ratio_pinned_to_wave_budget(self):
        self.assertEqual(W._tokens(7000, "x.cpp"), 2000)  # code: bytes/3.5
        self.assertEqual(W._tokens(4000, "x.md"), 1000)   # prose: bytes/4
        self.assertEqual(rb.est_tokens(7000, "x.cpp"), W._tokens(7000, "x.cpp"))


class SkipRuleTests(unittest.TestCase):
    def test_unsafe_name_probe_is_flagged(self):
        self.assertTrue(rb.unsafe_name("x_credential_probe.txt"))
        self.assertTrue(rb.unsafe_name("a/My_SECRET_dir/ok.cpp"))
        self.assertTrue(rb.unsafe_name("a/b/Secrets.json"))
        self.assertFalse(rb.unsafe_name("a/b/ok.cpp"))

    def test_noise(self):
        for p in ("a/__pycache__/x.py", "a/foo_evidence/n.txt", "a/baseline_1.txt", "a/PULL_REQUEST.md", "CHANGELOG.md"):
            self.assertTrue(rb.is_noise(p), p)
        self.assertFalse(rb.is_noise("a/evidence.py"))


class PlanTests(Fixture):
    def by_rel(self, plan):
        return {e.rel: e for e in plan.entries}

    def test_credential_and_secret_dir_skipped_and_never_opened(self):
        opened = []
        real = rb.read_file

        def spy(path):
            opened.append(path)
            return real(path)

        with mock.patch.object(rb, "read_file", spy):
            p = self.plan()
        e = self.by_rel(p)
        self.assertEqual(e["patch_manager_mods/mod_src/x_credential_probe.txt"].reason, "secret-name")
        self.assertEqual(e["patch_manager_mods/mod_src/my_secret_dir/ok.cpp"].reason, "secret-name")
        self.assertFalse([o for o in opened if "credential" in o or "secret" in o], opened)
        self.assertNotIn("hunter2", p.text)

    def test_binary_oversize_and_noise_stubs(self):
        e = self.by_rel(self.plan())
        self.assertEqual(e["patch_manager_mods/mod_src/blob.bin"].reason, "binary")
        self.assertEqual(e["patch_manager_mods/mod_src/Big.inc"].reason, "over max_file_tokens")
        self.assertEqual(e["patch_manager_mods/mod_src/__pycache__/x.pyc"].reason, "noise")
        tree = self.plan().text
        self.assertIn("[omitted: over max_file_tokens, 18000 bytes]", tree)

    def test_nul_byte_file_is_binary(self):
        Path(self.root, "patch_manager_mods/mod_src/n.dat").write_bytes(b"ab\x00cd")
        run(["git", "add", "-f", "."], self.root)
        self.assertEqual(self.by_rel(self.plan())["patch_manager_mods/mod_src/n.dat"].reason, "binary")

    def test_role_order(self):
        roles = [e.role for e in self.plan().entries if e.status == "included"]
        self.assertEqual(roles, sorted(roles, key=rb.ROLE_ORDER.index))
        self.assertEqual(roles[0], "docs")
        self.assertIn("deps", roles)
        self.assertEqual(roles[-1], "config")
        e = self.by_rel(self.plan())
        self.assertEqual(e["patch_manager_mods/mod_src/z_test.cpp"].role, "tests")
        self.assertEqual(e["patch_manager_mods/mod_src/test_tool.py"].role, "tests")
        self.assertEqual(e["patch_manager_mods/mod_src/tool.py"].role, "source")

    def test_include_closure_relative_shared_and_unresolved(self):
        p = self.plan()
        e = self.by_rel(p)
        self.assertEqual(e["patch_manager_mods/hud_src/Hh.h"].role, "deps")      # ../hud_src/ relative
        self.assertEqual(e["patch_manager_mods/hud_src/Deep.h"].role, "deps")    # recursion
        self.assertEqual(e["patch_manager_mods/_shared_src/Shared.h"].role, "deps")  # _shared_src fallback
        self.assertEqual(p.unresolved, [("patch_manager_mods/mod_src/a.cpp", "KpLog.h")])
        self.assertIn('a.cpp -> "KpLog.h"', p.text)

    def test_no_deps_when_disabled(self):
        cfg, bundles = rb.load_manifest(os.path.join(self.root, rb.DEFAULT_MANIFEST))
        b = dict(bundles[0], deps=False)
        entries, unresolved = rb.select(self.root, b, cfg, rb.tracked_files(self.root), set())
        self.assertFalse([x for x in entries if x.role == "deps"])
        self.assertEqual(unresolved, [])

    def test_document_end_tag_escaped(self):
        text = self.plan().text
        self.assertIn('const char*s="<\\/document> bad";', text)
        self.assertEqual(text.count("</document>"), len([e for e in self.plan().entries if e.status == "included"]))

    def test_dirty_marker(self):
        Path(self.root, "patch_manager_mods/mod_src/tool.py").write_text("print('changed')\n", encoding="utf-8")
        p = self.plan()
        self.assertTrue(self.by_rel(p)["patch_manager_mods/mod_src/tool.py"].dirty)
        self.assertFalse(self.by_rel(p)["patch_manager_mods/mod_src/a.cpp"].dirty)
        self.assertIn('path="patch_manager_mods/mod_src/tool.py" role="source"', p.text)
        self.assertRegex(p.text, r'tool\.py" role="source" est_tokens="\d+" dirty="1"')

    def test_layout_rules_first_question_last(self):
        t = self.plan().text
        self.assertTrue(t.startswith("<review_bundle id=\"b1\""))
        self.assertLess(t.index("<instructions>"), t.index("\n<directory_tree>\n"))
        self.assertLess(t.index("\n<directory_tree>\n"), t.index("\n<documents>\n"))
        self.assertLess(t.rindex("</documents>"), t.index("\n<question>\n"))
        self.assertIn("look at X", t)
        self.assertTrue(t.rstrip().endswith("</review_bundle>"))

    def test_content_secret_scan_blocks(self):
        Path(self.root, "patch_manager_mods/mod_src/key.cpp").write_text("// -----BEGIN RSA PRIVATE KEY-----\n", encoding="utf-8")
        run(["git", "add", "-f", "."], self.root)
        p = self.plan()
        self.assertEqual(len(p.findings), 1)
        self.assertNotIn("PRIVATE KEY", p.text)
        rc, _, err = self.cli("build", "b1", "--out", self.out)
        self.assertEqual(rc, 1)
        self.assertIn("finding", err)
        self.assertFalse(os.path.exists(os.path.join(self.out, "b1.txt")))

    def test_line_numbers_option(self):
        self.assertRegex(self.plan(ln=True).text, r"(?m)^    1\| int t\(\)")
        self.assertNotRegex(self.plan().text, r"(?m)^    1\| ")


class BuildTests(Fixture):
    def test_cap_breach_exit_1_and_nothing_written(self):
        rc, out, err = self.cli("build", "tiny", "--out", self.out)
        self.assertEqual(rc, 1)
        self.assertIn("OVER CAP", err)
        self.assertIn("NOT WRITTEN", err)
        self.assertFalse(os.path.exists(self.out))

    def test_byte_cap_breach(self):
        manifest = Path(self.root, rb.DEFAULT_MANIFEST)
        manifest.write_text(MANIFEST.replace("cap_tokens = 5000", "cap_tokens = 5000\ncap_bytes = 500"), encoding="utf-8")
        rc, _, err = self.cli("build", "b1", "--out", self.out)
        self.assertEqual(rc, 1)
        self.assertIn("OVER CAP", err)
        self.assertFalse(os.path.exists(self.out))

    def test_question_repeated_at_top(self):
        t = self.plan().text
        self.assertLess(t.index("List your findings"), t.index("\n<directory_tree>\n"))
        self.assertGreater(t.rindex("List your findings"), t.index("\n<question>\n"))

    def test_allow_over_writes(self):
        rc, _, _ = self.cli("build", "tiny", "--out", self.out, "--allow-over")
        self.assertEqual(rc, 0)
        self.assertTrue(os.path.exists(os.path.join(self.out, "tiny.txt")))

    def test_build_writes_three_outputs_and_is_byte_identical(self):
        self.assertEqual(self.cli("build", "b1", "--out", self.out)[0], 0)
        first = {n: Path(self.out, n).read_bytes() for n in os.listdir(self.out)}
        self.assertEqual(sorted(first), ["_index.tsv", "b1.manifest.tsv", "b1.txt"])
        self.assertEqual(self.cli("build", "b1", "--out", self.out)[0], 0)
        second = {n: Path(self.out, n).read_bytes() for n in os.listdir(self.out)}
        self.assertEqual(first, second)
        tsv = first["b1.manifest.tsv"].decode()
        self.assertRegex(tsv, r"x_credential_probe\.txt\t\w+\tomitted\tsecret-name")

    def test_est_tokens_header_matches_index(self):
        self.cli("build", "b1", "--out", self.out)
        head = Path(self.out, "b1.txt").read_text(encoding="utf-8").splitlines()[0]
        idx = Path(self.out, "_index.tsv").read_text(encoding="utf-8").splitlines()[1].split("\t")
        self.assertIn('est_tokens="%s"' % idx[1], head)

    def test_all_builds_what_fits_and_reports_the_rest(self):
        rc, out, err = self.cli("build", "--all", "--out", self.out)
        self.assertEqual(rc, 1)
        self.assertTrue(os.path.exists(os.path.join(self.out, "b1.txt")))
        self.assertFalse(os.path.exists(os.path.join(self.out, "tiny.txt")))

    def test_plan_output_and_exit(self):
        rc, out, _ = self.cli("plan", "b1")
        self.assertEqual(rc, 0)
        for needle in ("role totals:", "omitted:", "dirty (", "unresolved include:", "total est_tokens="):
            self.assertIn(needle, out)
        self.assertFalse(os.path.exists(self.out))
        self.assertEqual(self.cli("plan", "tiny")[0], 1)

    def test_usage_errors_exit_2(self):
        self.assertEqual(self.cli("plan")[0], 2)
        self.assertEqual(self.cli("plan", "nope")[0], 2)
        self.assertEqual(rb.main(["--repo-root", self.root, "--manifest", "/nonexistent.toml", "list"]), 2)
        self.assertEqual(rb.main(["--bogus"]), 2)

    def test_review_set_is_read_and_listed(self):
        manifest = Path(self.root, rb.DEFAULT_MANIFEST)
        manifest.write_text(MANIFEST.replace('id = "b1"', 'id = "b1"\nreview_set = "grp"'), encoding="utf-8")
        _, bundles = rb.load_manifest(str(manifest))
        self.assertEqual(next(b for b in bundles if b["id"] == "b1")["review_set"], "grp")
        self.assertEqual(next(b for b in bundles if b["id"] == "tiny")["review_set"], "")
        rc, out, _ = self.cli("list")
        self.assertEqual(rc, 0)
        self.assertIn("b1\tset=grp", out)
        self.assertIn("tiny\tset=-", out)

    def test_list(self):
        rc, out, _ = self.cli("list")
        self.assertEqual(rc, 0)
        self.assertIn("b1\tset=-\tcap=5000", out)


if __name__ == "__main__":
    unittest.main()
