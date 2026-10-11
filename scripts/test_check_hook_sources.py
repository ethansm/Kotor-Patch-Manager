#!/usr/bin/env python3
"""Tests for scripts/check_hook_sources.py (review F-2: unbracketed stack sources). Pure stdlib; every fixture is a
temp-dir hooks.toml, so no exe, KPM tree or game dir is needed."""
import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import _static_common as sc  # noqa: E402
import check_hook_sources as chs  # noqa: E402


def hooks_toml(*sources, address="0x0076D200"):
    """A hooks.toml with one detour whose parameters carry the given sources (same shape as npc_diversity_src)."""
    params = "".join(f'\n[[hooks.parameters]]\nsource = "{s}"\ntype = "pointer"\n' for s in sources)
    return (f'[metadata]\ntarget_versions = ["AA"]\n\n[[hooks]]\naddress = {address}\ntype = "detour"\n'
            f'function = "F"\noriginal_bytes = [0x55, 0x8B, 0xEC]\n{params}')


class BadSourceTests(unittest.TestCase):
    def test_good_sources_are_not_flagged(self):
        for s in ("ecx", "eax", "ebp", "esp", "[esp+4]", "[ebp+0x08]", "[esp+0]", "[ebp-0x20]", "[ebp]", "int", ""):
            self.assertFalse(chs.bad_source(s), s)

    def test_unbracketed_stack_offsets_are_flagged(self):
        for s in ("esp+4", "ebp + 8", "esp-0x10", "ebp+0X1C", "esp - 12", " esp+4 ", "ESP+4"):
            self.assertTrue(chs.bad_source(s), s)

    def test_non_string_source_is_not_flagged(self):
        self.assertFalse(chs.bad_source(None))
        self.assertFalse(chs.bad_source(4))

    def test_fix_is_the_bracketed_form_without_spaces(self):
        self.assertEqual(chs.fix_for("esp+4"), "[esp+4]")
        self.assertEqual(chs.fix_for("ebp + 8"), "[ebp+8]")
        self.assertEqual(chs.fix_for(" esp-0x10 "), "[esp-0x10]")


class FileTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.dir = Path(self._tmp.name)

    def tearDown(self):
        self._tmp.cleanup()

    def write(self, text, name="kotor2-steam-aspyr.hooks.toml"):
        p = self.dir / name
        p.write_text(text, encoding="utf-8")
        return p

    def run_main(self, *paths):
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = chs.main([str(p) for p in paths])
        return rc, buf.getvalue()

    def test_good_fixture_exits_0(self):
        p = self.write(hooks_toml("ecx", "[esp+4]", "[ebp+0x08]", "ebp"))
        rc, out = self.run_main(p)
        self.assertEqual(rc, 0, out)
        self.assertFalse([ln for ln in out.splitlines() if ln.startswith("FAIL ")], out)

    def test_bad_sources_exit_1_and_name_file_address_source_and_fix(self):
        p = self.write(hooks_toml("esp+4", "ebp + 8", "esp-0x10", address="0x0076D200"))
        rc, out = self.run_main(p)
        self.assertEqual(rc, 1)
        fail_lines = [ln for ln in out.splitlines() if ln.startswith("FAIL source")]
        self.assertEqual(len(fail_lines), 3, out)
        self.assertIn(str(p.name), fail_lines[0])
        self.assertIn("0x0076D200", fail_lines[0])
        self.assertIn('source "esp+4"', fail_lines[0])
        self.assertIn('fix: source = "[esp+4]"', fail_lines[0])
        self.assertIn('fix: source = "[ebp+8]"', fail_lines[1])
        self.assertIn("check_hook_sources: 1 file(s)", out)

    def test_deliberately_bad_toml_exits_1(self):
        # npc_diversity_src as it was at HEAD (review F-2): one bare `esp+4` among valid sources.
        p = self.write(hooks_toml("ebp", "ecx", "esp+4", "[esp+8]"), name="bad.hooks.toml")
        rc, out = self.run_main(p)
        self.assertEqual(rc, 1, out)
        self.assertEqual(sum(1 for ln in out.splitlines() if ln.startswith("FAIL")), 1, out)

    def test_unparseable_file_exits_1(self):
        p = self.write('[[hooks]]\naddress = 0x1\nsource = "esp+4"\n[[hooks.parameters]\n', name="broken.hooks.toml")
        rc, out = self.run_main(p)
        self.assertEqual(rc, 1)
        self.assertIn("FAIL parse", out)

    def test_missing_file_exits_1(self):
        rc, out = self.run_main(self.dir / "nope.hooks.toml")
        self.assertEqual(rc, 1)
        self.assertIn("FAIL parse", out)

    def test_hook_without_parameters_is_ignored(self):
        p = self.write('[[hooks]]\naddress = 0x1000\ntype = "simple"\n')
        rc, _ = self.run_main(p)
        self.assertEqual(rc, 0)

    def test_check_file_collects_into_reporter(self):
        p = self.write(hooks_toml("esp+4"))
        rep = sc.Reporter(out=None)
        chs.check_file(p, rep)
        self.assertEqual([f.sev for f in rep.findings], ["FAIL"])
        self.assertEqual(rep.exit_code(), 1)


class RealRepoTests(unittest.TestCase):
    def test_repo_hooks_files_do_not_crash_the_checker(self):
        files = chs.find_hooks_files()
        self.assertTrue(files, "expected hooks.toml files under patch_manager_mods/")
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = chs.main([])
        self.assertIn(rc, (0, 1), buf.getvalue())
        self.assertIn(f"check_hook_sources: {len(files)} file(s)", buf.getvalue())


if __name__ == "__main__":
    unittest.main()
