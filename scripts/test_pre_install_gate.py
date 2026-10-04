#!/usr/bin/env python3
"""Host-only tests for pre_install_gate.py (no compiler, no Wine; fake KPM tree under a temp dir)."""
import io
import sys
import tempfile
import unittest
import zipfile
from contextlib import redirect_stdout
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import pre_install_gate as g  # noqa: E402

SMOKES = """
[[smoke]]
name = "probe11"
kpatch = "LoadHangProbe/LoadHangProbe.kpatch"
[[smoke]]
name = "probe14"
kpatch = "LoadHangProbe/LoadHangProbe.kpatch"
[[smoke]]
name = "perflab"
kpatch = "PerfLab/PerfLab.kpatch"
[[smoke]]
name = "hwmip"
kpatch = "PerfLab/PerfLab.kpatch"
[[smoke]]
name = "combo"
kpatch = "Other/Other.kpatch"
extra_dlls = [{ kpatch = "PerfLab/PerfLab.kpatch", dll_name = "perflab.dll" }]
"""


def make_kpatch(path: Path, pid: str, dll=b"OLD-DLL"):
    path.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(path, "w") as z:
        z.writestr("manifest.toml", f'[patch]\nid = "{pid}"\nname = "x"\n')
        z.writestr("kotor2-steam-aspyr.hooks.toml", "# hooks\n")
        z.writestr(zipfile.ZipInfo(g.DLL_MEMBER), dll, compress_type=zipfile.ZIP_DEFLATED)


class Fixture(unittest.TestCase):
    def setUp(self):
        self.td = tempfile.TemporaryDirectory()
        self.root = Path(self.td.name)
        self.kpm = self.root / "kpm"
        make_kpatch(self.kpm / "Patches/LoadHangProbe/LoadHangProbe.kpatch", "load-hang-probe")
        make_kpatch(self.kpm / "Patches/PerfLab/PerfLab.kpatch", "perf-lab")
        make_kpatch(self.kpm / "Patches/Other/Other.kpatch", "other")
        self.smokes = self.root / "smokes.toml"
        self.smokes.write_text(SMOKES)

    def tearDown(self):
        self.td.cleanup()


class Mapping(Fixture):
    def test_rows_by_manifest_id(self):
        self.assertEqual(g.smoke_rows("perf-lab", self.smokes, self.kpm), ["perflab", "hwmip", "combo"])
        self.assertEqual(g.smoke_rows("load-hang-probe", self.smokes, self.kpm), ["probe11", "probe14"])
        self.assertEqual(g.smoke_rows("no-smoke-mod", self.smokes, self.kpm), [])
        self.assertEqual(g.smoke_rows("", self.smokes, self.kpm), [])

    def test_live_tree_if_present(self):
        live = g.KPM_DIR / "Patches" / "PerfLab" / "PerfLab.kpatch"
        if not live.is_file() or not g.SMOKES_TOML.is_file():
            self.skipTest("no live KPM tree")
        self.assertTrue({"perflab", "perflab2", "perflab3", "hwmip", "perflab5"}
                        <= set(g.smoke_rows("perf-lab", g.SMOKES_TOML, g.KPM_DIR)))
        self.assertEqual(g.smoke_rows("load-hang-probe", g.SMOKES_TOML, g.KPM_DIR), ["probe11", "probe12", "probe14"])

    def test_mod_key_matches_build_check(self):
        self.assertEqual(g.mod_key("perf_lab_src/perflab"), "perf_lab_src__perflab")
        self.assertEqual(g.mod_key("load_hang_probe_src"), "load_hang_probe_src")


class Swap(Fixture):
    def test_swap_keeps_other_members(self):
        src = self.kpm / "Patches/PerfLab/PerfLab.kpatch"
        dll = self.root / "new.dll"
        dll.write_bytes(b"NEW-DLL" * 100)
        dest = self.root / "scratch/Patches/PerfLab/PerfLab.kpatch"
        g.swap_dll(src, dest, dll)
        with zipfile.ZipFile(src) as a, zipfile.ZipFile(dest) as b:
            self.assertEqual(a.namelist(), b.namelist())
            for n in a.namelist():
                if n == g.DLL_MEMBER:
                    self.assertEqual(b.read(n), dll.read_bytes())
                else:
                    self.assertEqual(a.read(n), b.read(n), n)
        with zipfile.ZipFile(src) as a:   # source untouched
            self.assertEqual(a.read(g.DLL_MEMBER), b"OLD-DLL")

    def test_scratch_tree_swaps_only_our_mod(self):
        dll = self.root / "new.dll"
        dll.write_bytes(b"FRESH")
        scratch = self.root / "scratch"
        rows = g.step_scratch({"perf-lab": dll}, scratch, self.smokes, self.kpm)
        self.assertEqual(rows, {"perf-lab": ["perflab", "hwmip", "combo"]})
        with zipfile.ZipFile(scratch / "Patches/PerfLab/PerfLab.kpatch") as z:
            self.assertEqual(z.read(g.DLL_MEMBER), b"FRESH")
        with zipfile.ZipFile(scratch / "Patches/Other/Other.kpatch") as z:   # co-loaded: as installed
            self.assertEqual(z.read(g.DLL_MEMBER), b"OLD-DLL")
        self.assertFalse((scratch / "Patches/LoadHangProbe").exists())

    def test_scratch_guard(self):
        with self.assertRaises(SystemExit):
            g.safe_scratch(self.root / "elsewhere")
        with self.assertRaises(SystemExit):
            g.safe_scratch(g.KPM_DIR / "Patches")
        self.assertTrue(g.safe_scratch(g.TMP_BASE / "gate").is_relative_to(g.TMP_BASE.resolve()))


class Wine(unittest.TestCase):
    OUT = "\n".join([
        "PASS   wine:perflab  2.0s",
        "PASS   wine:perflab[m1]  1.0s",
        "SKIP   wine:probe14  0.0s  missing DISPLAY",
        "SKIP   wine:texmem  0.0s  missing /tmp/x/Patches/TextureMemoryRelease/TextureMemoryRelease.kpatch",
        "FAIL   wine:hwmip[hw]  3.1s  exit 3",
    ])

    def test_parse_and_judge(self):
        res = g.parse_wine(self.OUT)
        self.assertEqual(res["perflab"], [("PASS", "", "2.0s"), ("PASS", "[m1]", "1.0s")])
        self.assertEqual(g.judge_wine(["perflab"], res), [])
        self.assertEqual(g.judge_wine(["probe14"], res), ["probe14"])
        with self.assertRaises(g.GateFail) as c:
            g.judge_wine(["perflab", "hwmip"], res)
        self.assertEqual((c.exception.n, c.exception.step), (4, "wine"))
        self.assertIn("exit 3", c.exception.reason)
        with self.assertRaises(g.GateFail):   # a matched row skipped for a missing kpatch is a gate bug, not a pass
            g.judge_wine(["texmem"], res)
        with self.assertRaises(g.GateFail):   # no line at all
            g.judge_wine(["absent"], res)


class Ordering(unittest.TestCase):
    """Fake runner: records calls, fails a chosen step; later steps must not run."""

    def fake(self, fail_on):
        calls = []

        def runner(cmd, env=None, quiet=False):
            tag = "wine" if "--wine" in cmd else "build" if str(g.BUILD_CHECK) in cmd else "default"
            calls.append(tag)
            if tag == fail_on:
                return 1, "FAIL   py:x  0.1s\n"
            return 0, "PASS   wine:perflab  1.0s\n" if tag == "wine" else ""
        return runner, calls

    def gate(self, fail_on):
        runner, calls = self.fake(fail_on)
        buf = io.StringIO()
        with redirect_stdout(buf):
            rc = g.gate(["perf_lab_src/perflab"], scratch=g.TMP_BASE / "gate-unittest", runner=runner)
        return rc, calls, buf.getvalue()

    def test_default_fail_stops(self):
        rc, calls, out = self.gate("default")
        self.assertEqual((rc, calls), (1, ["default"]))
        self.assertIn("GATE FAIL 1 default: ", out)

    def test_build_fail_stops(self):
        rc, calls, out = self.gate("build")
        self.assertEqual((rc, calls), (1, ["default", "build"]))
        self.assertIn("GATE FAIL 2 build: ", out)

    def test_skip_default(self):
        runner, calls = self.fake("build")
        buf = io.StringIO()
        with redirect_stdout(buf):
            rc = g.gate(["perf_lab_src/perflab"], skip_default=True, scratch=g.TMP_BASE / "gate-unittest", runner=runner)
        self.assertEqual((rc, calls), (1, ["build"]))
        self.assertIn("WARN  step 1 default tier skipped", buf.getvalue())

    def test_build_without_dll_fails(self):   # build_check rc 0 (e.g. SKIP: no compiler) but no DLL produced
        rc, calls, out = self.gate(None)
        self.assertEqual((rc, calls), (1, ["default", "build"]))
        self.assertIn("GATE FAIL 2 build: perf_lab_src/perflab: expected one built windows_x86.dll", out)


if __name__ == "__main__":
    unittest.main()
