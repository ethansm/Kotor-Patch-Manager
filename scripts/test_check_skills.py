#!/usr/bin/env python3
"""Tests for scripts/check_skills.py (fixtures in scripts/testdata/check_skills/: root/ = fake repo root,
cases/<skill>/SKILL.md = one skill per rule, user/ = fake ~/.claude/skills)."""
import io
import json
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import check_skills as C  # noqa: E402

FX = HERE / "testdata" / "check_skills"
ROOT = FX / "root"
REAL_ROOT = HERE.parent


def cli(*args):
    buf = io.StringIO()
    rc = C.main([str(a) for a in args], out=buf)
    return rc, buf.getvalue()


class CaseBase(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="check_skills_")
        self.addCleanup(shutil.rmtree, self.tmp, True)

    def lint(self, *cases, user=None, as_json=False):
        """Copy the named case skills into a tmp skills dir and lint them against the fixture root."""
        sk = Path(tempfile.mkdtemp(dir=self.tmp)) / "skills"
        sk.mkdir()
        for c in cases:
            shutil.copytree(FX / "cases" / c, sk / c)
        ud = Path(user) if user else Path(self.tmp) / "empty_user"
        ud.mkdir(exist_ok=True)
        args = ["--skills-dir", sk, "--user-dir", ud, "--repo-root", ROOT]
        if as_json:
            args.append("--json")
        return cli(*args)

    def assertLine(self, out, prefix, *needles):
        hits = [ln for ln in out.splitlines() if ln.startswith(prefix) and all(n in ln for n in needles)]
        self.assertTrue(hits, "no %r line containing %r in:\n%s" % (prefix, needles, out))

    def assertFailing(self, case, *needles, line=None):
        rc, out = self.lint(case)
        self.assertEqual(rc, 1, out)
        self.assertLine(out, "FAIL %s:" % case, *needles)
        if line is not None:
            self.assertLine(out, "FAIL %s:%d " % (case, line), *needles)
        self.assertTrue(out.splitlines()[-1].startswith("VERDICT: FAIL skills=1 fails="), out)
        return out


class RuleTests(CaseBase):
    def test_good_skill_passes(self):
        rc, out = self.lint("good")
        self.assertEqual(out, "VERDICT: PASS skills=1 fails=0 warns=0\n")
        self.assertEqual(rc, 0)

    def test_bad_name(self):
        self.assertFailing("bad-name", "BAD NAME", "wrong-name", line=2)

    def test_description_empty(self):
        self.assertFailing("desc-empty", "DESCRIPTION empty")

    def test_description_too_long(self):
        self.assertFailing("desc-long", "DESCRIPTION too long", "1149 > 1024")

    def test_no_frontmatter(self):
        self.assertFailing("no-frontmatter", "frontmatter missing")

    def test_quoted_name_and_folded_description_parse(self):
        rc, out = self.lint("quoted-fold")
        self.assertEqual(rc, 0, out)

    def test_missing_path(self):
        out = self.assertFailing("missing-path", "MISSING PATH research/nope.txt", line=5)
        self.assertLine(out, "FAIL missing-path:7 ", "MISSING PATH patch_manager_mods/gone/file.md")  # :12 stripped
        self.assertEqual(out.count("MISSING PATH"), 2, out)

    def test_script_missing(self):
        out = self.assertFailing("script-missing", "SCRIPT MISSING scripts/ghost.py")
        self.assertEqual(out.count("FAIL"), 2, out)  # 1 problem line + verdict, no duplicate MISSING PATH

    def test_unknown_subcommand(self):
        self.assertFailing("unknown-sub", "UNKNOWN SUBCOMMAND gamma", "scripts/fake.py")

    def test_unknown_flag(self):
        out = self.assertFailing("unknown-flag", "UNKNOWN FLAG --count", "scripts/fake.py alpha")
        self.assertLine(out, "FAIL unknown-flag:", "UNKNOWN FLAG --nope", "scripts/plain.py")

    def test_handrolled_subcommand_help(self):
        out = self.assertFailing("handrolled", "UNKNOWN FLAG --nope", "scripts/handrolled.py run")
        self.assertNotIn("--deep", out)

    def test_bad_shell_syntax(self):
        self.assertFailing("bad-sh", "BASH SYNTAX scripts/broken.sh")

    def test_placeholder_paths_ignored(self):
        rc, out = self.lint("placeholders")
        self.assertEqual(out, "VERDICT: PASS skills=1 fails=0 warns=0\n")

    def test_missing_dep_is_warn_not_fail(self):
        rc, out = self.lint("missing-dep")
        self.assertEqual(rc, 0, out)
        self.assertLine(out, "WARN missing-dep:", "skipped: missing dep nonexistent_fixture_module_xyz")
        self.assertTrue(out.endswith("VERDICT: PASS skills=1 fails=0 warns=1\n"), out)

    def test_script_without_help_parser_not_executed(self):
        rc, out = self.lint("no-help-parser")
        self.assertEqual(rc, 0, out)
        self.assertLine(out, "WARN no-help-parser:", "not executed")

    def test_diverged_user_copy_is_warn(self):
        rc, out = self.lint("diverged", "same", user=FX / "user")
        self.assertEqual(rc, 0, out)
        self.assertLine(out, "WARN diverged:0 DIVERGED diverged (user copy differs)")
        self.assertNotIn("same:", out)  # identical copy: silent
        self.assertTrue(out.endswith("VERDICT: PASS skills=2 fails=0 warns=1\n"), out)


class OutputTests(CaseBase):
    def test_sorted_and_json(self):
        rc, out = self.lint("unknown-flag", "bad-name", "good")
        lines = out.splitlines()
        body = lines[:-1]
        self.assertEqual(body, sorted(body, key=lambda s: (s.split(" ", 1)[1].split(":")[0],
                                                           int(s.split(":")[1].split(" ")[0]))))
        self.assertEqual(rc, 1)
        rc2, js = self.lint("unknown-flag", "bad-name", "good", as_json=True)
        d = json.loads(js)
        self.assertEqual(rc2, 1)
        self.assertEqual(d["verdict"], "FAIL")
        self.assertEqual((d["skills"], d["fails"], d["warns"]), (3, 3, 0))
        self.assertEqual(sorted(d["problems"][0]), ["level", "line", "message", "skill"])
        self.assertEqual(lines[-1], "VERDICT: FAIL skills=3 fails=3 warns=0")

    def test_read_only_no_bytecode_written(self):
        self.lint("good", "missing-dep")
        self.assertFalse(list(ROOT.rglob("__pycache__")), "linter wrote bytecode into the fixture root")

    def test_credential_named_files_not_opened(self):
        self.assertTrue(C.unsafe_name("scripts/my_credential_tool.py"))
        self.assertTrue(C.unsafe_name("x/.server_SECRET.sh"))
        self.assertFalse(C.unsafe_name("scripts/ok.sh"))
        st, info = C.HelpRunner(ROOT).run("scripts/credential_x.py", "")
        self.assertEqual(st, "skip")


class UnitTests(unittest.TestCase):
    def test_tokenize_stops_at_operators(self):
        self.assertEqual(C.tokenize_segment(" a --b=1 | c"), ["a", "--b=1"])
        self.assertEqual(C.tokenize_segment(' "x y" [--a|--b] && z'), ["x y", "[--a|--b]"])
        self.assertEqual(C.tokenize_segment(" s 2>&1 > out"), ["s"])
        self.assertEqual(C.tokenize_segment(" s # --c"), ["s"])
        self.assertEqual(C.tokenize_segment(" s <name> --d)"), ["s", "<name>", "--d"])

    def test_flag_names(self):
        self.assertEqual(C.flag_names("[--a|--b]"), ["--a", "--b"])
        self.assertEqual(C.flag_names("--x=val"), ["--x"])
        self.assertEqual(C.flag_names("--help"), [])
        self.assertEqual(C.flag_names("--<name>"), [])

    def test_help_subcommands_ignores_choice_flags(self):
        h = "usage: x.py [-h] [--mode {fast,slow}] {a,b,c} ...\n\npositional arguments:\n  {a,b,c}\n"
        self.assertEqual(C.help_subcommands(h), ["a", "b", "c"])
        self.assertEqual(C.help_subcommands("usage: x.py [-h] [--mode {fast,slow}] file\n"), [])

    def test_path_cleaning(self):
        self.assertEqual(C.extract_paths("see (scripts/a.py:12), `research/b.txt.` $G/scripts/x ../scripts/y"),
                         ["scripts/a.py", "research/b.txt"])


class RepoGate(unittest.TestCase):
    def test_repo_skills_pass(self):
        """STANDING GATE: every real .claude/skills/*/SKILL.md must lint clean (no FAIL). Never skipped."""
        with tempfile.TemporaryDirectory(prefix="check_skills_empty_user_") as empty:
            n, probs = C.run_lint(REAL_ROOT / ".claude" / "skills", empty, REAL_ROOT)
        fails = ["FAIL %s:%d %s" % (s, ln, m) for s, ln, lv, m in probs if lv == "FAIL"]
        self.assertGreater(n, 0, "no skills found under .claude/skills")
        self.assertEqual(fails, [], "REAL SKILLS HAVE %d FAIL(S) - fix the SKILL.md files:\n%s" % (len(fails), "\n".join(fails)))


if __name__ == "__main__":
    unittest.main()
