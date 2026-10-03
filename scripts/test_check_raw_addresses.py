#!/usr/bin/env python3
"""Tests for scripts/check_raw_addresses.py (doc 20 Phase 1). Pure stdlib; fixtures are tiny repo roots under
scripts/testdata/static/raw/<case>/ (copied to a temp dir whenever a test mutates or writes to them)."""
import io
import shutil
import subprocess
import sys
import tempfile
import tomllib
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import _static_common as sc  # noqa: E402
import check_raw_addresses as cra  # noqa: E402

FX = HERE / "testdata" / "static" / "raw"
A = "patch_manager_mods/a_src/A.cpp"


def sevs(rep, sev):
    return [f for f in rep.findings if f.sev == sev]


def check(case_or_root):
    root = case_or_root if isinstance(case_or_root, Path) else FX / case_or_root
    return cra.run(root, reporter=sc.Reporter(out=None))


class RawAddressTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)

    def copy(self, case):
        dst = Path(self._tmp.name) / case
        shutil.copytree(FX / case, dst)
        return dst

    def test_base_passes_and_exclusions_are_ignored(self):
        # fixtures hold unlisted addresses in A_test.cpp, testdata/, retired/, a GENERATED .inc, a .txt, a non-_src dir
        # and GameAddr.h itself; out-of-range and non-8-digit literals sit in A.cpp
        rep, st = check("base")
        self.assertEqual(sevs(rep, "FAIL"), [])
        self.assertEqual(sevs(rep, "WARN"), [])
        self.assertEqual(sevs(rep, "PASS")[0].check, "raw-address")
        self.assertEqual(st["files"], 2)               # A.cpp and sub/deeper/Deep.h (nested dir scanned)
        self.assertEqual(st["occurrences"], 2)
        self.assertEqual(st["distinct"], {"0x00500000", "0x00500010"})

    def test_non_generated_inc_is_scanned(self):
        root = self.copy("base")
        (root / "patch_manager_mods/a_src/Hand.inc").write_text("// hand written table\nint x = 0x00520000;\n")
        rep, _ = check(root)
        self.assertEqual(len(sevs(rep, "FAIL")), 1)
        self.assertIn("a_src/Hand.inc:2 0x00520000", sevs(rep, "FAIL")[0].msg)

    def test_new_address_fails_with_file_and_line(self):
        rep, _ = check("new_address")
        fails = sevs(rep, "FAIL")
        self.assertEqual(len(fails), 1)
        self.assertIn(f"{A}:2 0x00500FF0", fails[0].msg)
        self.assertEqual(rep.exit_code(), 1)

    def test_every_occurrence_of_a_new_address_fails(self):
        root = self.copy("new_address")
        (root / A).write_text("int a = 0x00500FF0;\nint b = 0x00500ff0;\n")
        rep, _ = check(root)
        self.assertEqual([f.msg.split(" ")[0] for f in sevs(rep, "FAIL")], [f"{A}:1", f"{A}:2"])

    def test_stale_allowlist_entries_warn(self):
        rep, _ = check("stale")
        msgs = [f.msg for f in sevs(rep, "WARN")]
        self.assertEqual(sevs(rep, "FAIL"), [])
        self.assertEqual(len(msgs), 3, msgs)
        self.assertTrue(any("0x00500020" in m and "no longer contains" in m for m in msgs))
        self.assertTrue(any("0x004122A0" in m and "gameaddr::PlayGuiSound" in m for m in msgs))
        self.assertTrue(any("Gone.cpp" in m for m in msgs))
        self.assertEqual(rep.exit_code(), 0)

    def test_gameaddr_match_is_info_not_fail(self):
        rep, st = check("gameaddr_match")
        self.assertEqual(sevs(rep, "FAIL"), [])
        infos = {f.msg.split(":")[0]: f.msg for f in sevs(rep, "INFO")}
        self.assertEqual(len(infos), 2)                # one INFO line per file
        self.assertIn("3 raw constant(s)", infos["patch_manager_mods/a_src/A.cpp"])   # case-insensitive value compare
        self.assertIn("gameaddr::Name", infos["patch_manager_mods/a_src/A.cpp"])
        self.assertIn("PlayGuiSound", infos["patch_manager_mods/a_src/A.cpp"])
        self.assertIn("1 raw constant(s)", infos["patch_manager_mods/a_src/B.cpp"])
        self.assertEqual(st["distinct"], set())

    def test_missing_gameaddr_fails(self):
        rep, _ = cra.run(FX / "base", gameaddr=FX / "nope.h", reporter=sc.Reporter(out=None))
        self.assertEqual(len(sevs(rep, "FAIL")), 1)

    def test_overrides_for_gameaddr_and_allowlist(self):
        # an empty allowlist makes the base fixture's 2 addresses fail; --allowlist points at the real one again
        empty = Path(self._tmp.name) / "empty.toml"
        empty.write_text("")
        rep, _ = cra.run(FX / "base", allowlist=empty, reporter=sc.Reporter(out=None))
        self.assertEqual(len(sevs(rep, "FAIL")), 2)
        rep, _ = cra.run(FX / "base", allowlist=FX / "base/tests/static/raw_addresses.toml", reporter=sc.Reporter(out=None))
        self.assertEqual(sevs(rep, "FAIL"), [])

    def run_main(self, *args):
        return subprocess.run([sys.executable, str(HERE / "check_raw_addresses.py"), *args], capture_output=True, text=True)

    def test_main_quiet_and_exit_code(self):
        ok = self.run_main("--root", str(FX / "base"), "--quiet")
        bad = self.run_main("--root", str(FX / "new_address"))
        self.assertEqual((ok.returncode, bad.returncode), (0, 1))
        self.assertNotIn("PASS raw-address", ok.stdout)
        self.assertIn("check_raw_addresses: FAIL=0", ok.stdout)
        self.assertIn("FAIL raw-address", bad.stdout)

    def test_write_baseline_output_and_roundtrip(self):
        root = self.copy("baseline")
        buf = io.StringIO()
        added, removed = cra.write_baseline(root, None, None, out=buf)
        text = (root / "tests/static/raw_addresses.toml").read_text()
        self.assertIn("never by automation", text)
        data = tomllib.loads(text)["files"]
        # GameAddr values (0x004122A0, C.cpp's 0x00A1B4A4) are not baselined; duplicates collapse; addresses sorted
        self.assertEqual(data, {"patch_manager_mods/a_src/A.cpp": {"addresses": ["0x00500000", "0x005000B0"]},
                                "patch_manager_mods/a_src/B.h": {"addresses": ["0x00A00010"]}})
        self.assertEqual(added, {"patch_manager_mods/a_src/A.cpp": ["0x00500000", "0x005000B0"],
                                 "patch_manager_mods/a_src/B.h": ["0x00A00010"]})
        self.assertEqual(removed, {})
        self.assertIn("A.cpp: +2 -0", buf.getvalue())
        rep, _ = check(root)                           # the regenerated baseline passes
        self.assertEqual(sevs(rep, "FAIL"), [])
        # change the sources: one address added, one removed, B.h gone -> diff summary per file
        (root / "patch_manager_mods/a_src/A.cpp").write_text("int a = 0x00500000; int z = 0x00500AA0;\n")
        (root / "patch_manager_mods/a_src/B.h").unlink()
        buf = io.StringIO()
        added, removed = cra.write_baseline(root, None, None, out=buf)
        self.assertEqual(added, {"patch_manager_mods/a_src/A.cpp": ["0x00500AA0"]})
        self.assertEqual(removed, {"patch_manager_mods/a_src/A.cpp": ["0x005000B0"], "patch_manager_mods/a_src/B.h": ["0x00A00010"]})
        self.assertIn("A.cpp: +1 -1", buf.getvalue())
        self.assertIn("B.h: +0 -1", buf.getvalue())
        buf = io.StringIO()
        self.assertEqual(cra.write_baseline(root, None, None, out=buf), ({}, {}))
        self.assertIn("no changes", buf.getvalue())

    def test_write_baseline_via_main_returns_zero_and_writes_override_path(self):
        root = self.copy("baseline")
        target = Path(self._tmp.name) / "out" / "al.toml"
        rc = self.run_main("--root", str(root), "--allowlist", str(target), "--write-baseline").returncode
        self.assertEqual(rc, 0)
        self.assertTrue(target.is_file())
        self.assertFalse((root / "tests/static/raw_addresses.toml").exists())

    def test_real_repo_generated_inc_list_is_current(self):
        # every GENERATED_INC entry exists and every .inc in the repo is either listed or header-detected
        for rel in cra.GENERATED_INC:
            self.assertTrue((sc.REPO / rel).is_file(), rel)
        for p in (sc.REPO / "patch_manager_mods").rglob("*.inc"):
            self.assertTrue(p.relative_to(sc.REPO).as_posix() in cra.GENERATED_INC or cra.is_generated_inc(p), p)


if __name__ == "__main__":
    unittest.main()
