#!/usr/bin/env python3
"""Host-only tests for pre_install_gate.py (no compiler, no Wine; fake KPM tree under a temp dir)."""
import io
import json
import os
import sys
import tempfile
import unittest
import zipfile
from contextlib import redirect_stdout
from pathlib import Path
from unittest import mock

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

    def test_scratch_tree_carries_common_headers(self):   # smoke builds use -I$KPM_DIR/Patches/Common
        (self.kpm / "Patches/Common").mkdir(parents=True)
        (self.kpm / "Patches/Common/kpatch_api.h").write_text("/* api */\n")
        dll = self.root / "new.dll"
        dll.write_bytes(b"FRESH")
        scratch = self.root / "scratch"
        g.step_scratch({"perf-lab": dll}, scratch, self.smokes, self.kpm)
        self.assertEqual((scratch / "Patches/Common/kpatch_api.h").read_text(), "/* api */\n")

    def test_step_scratch_records_packed(self):
        dll = self.root / "new.dll"
        dll.write_bytes(b"FRESH")
        scratch = self.root / "scratch"
        packed = {}
        g.step_scratch({"perf-lab": dll}, scratch, self.smokes, self.kpm, packed)
        self.assertEqual(packed["perf-lab"], [scratch / "Patches/PerfLab/PerfLab.kpatch"])

    def test_step_scratch_packs_mod_without_smoke_row(self):   # exports are still checked for it
        dll = self.root / "new.dll"
        dll.write_bytes(b"FRESH-OTHER")
        make_kpatch(self.kpm / "Patches/NoRow/NoRow.kpatch", "no-row")
        scratch = self.root / "scratch"
        packed = {}
        rows = g.step_scratch({"no-row": dll}, scratch, self.smokes, self.kpm, packed)
        self.assertEqual(rows, {"no-row": []})
        dest = scratch / "Patches/NoRow/NoRow.kpatch"
        self.assertEqual(packed["no-row"], [dest])
        with zipfile.ZipFile(dest) as z:
            self.assertEqual(z.read(g.DLL_MEMBER), b"FRESH-OTHER")

    def test_step_scratch_new_mod_has_no_packed_entry(self):   # no live kpatch at all
        dll = self.root / "new.dll"
        dll.write_bytes(b"FRESH")
        packed = {}
        g.step_scratch({"brand-new": dll}, self.root / "scratch", self.smokes, self.kpm, packed)
        self.assertNotIn("brand-new", packed)

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


class Exports(unittest.TestCase):
    def test_argv_and_pass(self):
        calls = []

        def runner(cmd, env=None, quiet=False):
            calls.append(cmd)
            return 0, ""
        scratch = Path("/s")
        packed = {"a": [scratch / "Patches/A/A.kpatch"], "b": [scratch / "Patches/B/B.kpatch"]}
        g.step_exports(["a", "b"], packed, scratch, runner)
        cmd = calls[0]
        self.assertIn(str(g.CHECK_EXPORTS), cmd)
        self.assertEqual(cmd[cmd.index("--kpm-dir") + 1], str(scratch))
        self.assertEqual([cmd[i + 1] for i, x in enumerate(cmd) if x == "--mod"], ["a", "b"])
        self.assertEqual([cmd[i + 1] for i, x in enumerate(cmd) if x == "--kpatch"],
                         [str(scratch / "Patches/A/A.kpatch"), str(scratch / "Patches/B/B.kpatch")])

    def test_nothing_packed_is_note_not_call(self):
        calls = []
        buf = io.StringIO()
        with redirect_stdout(buf):
            g.step_exports(["new"], {}, Path("/s"), lambda cmd, env=None, quiet=False: calls.append(cmd) or (0, ""))
        self.assertEqual(calls, [])
        self.assertIn("NOTE no live kpatch for new", buf.getvalue())

    def test_failure_is_gate_fail_3(self):
        runner = lambda cmd, env=None, quiet=False: (1, "FAIL   mod-a  missing export Foo\n")
        with self.assertRaises(g.GateFail) as c:
            g.step_exports(["a"], {"a": [Path("/s/a.kpatch")]}, Path("/s"), runner)
        self.assertEqual((c.exception.n, c.exception.step), (3, "scratch"))
        self.assertIn("missing export Foo", c.exception.reason)


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
            rc = g.gate(["perf_lab_src/perflab"], scratch=g.TMP_BASE / "gate-unittest", runner=runner,
                        full_default=True)
        return rc, calls, buf.getvalue()

    def test_default_fail_stops(self):
        rc, calls, out = self.gate("default")
        self.assertEqual((rc, calls), (1, ["default"]))
        self.assertIn("GATE FAIL 1 default: ", out)

    def test_build_fail_stops(self):
        rc, calls, out = self.gate("build")
        self.assertEqual((rc, calls), (1, ["default", "build"]))
        self.assertIn("GATE FAIL 2 build: ", out)

    def test_check_exports_only_failure_deferred(self):
        buf = io.StringIO()
        with redirect_stdout(buf):
            g.step_default(lambda cmd, env=None, quiet=False: (1, "FAIL   suite:check-exports  0.1s\n"))
        self.assertIn("WARN  step 1: suite:check-exports failed against the stale KPM kpatch", buf.getvalue())
        with self.assertRaises(g.GateFail) as c:
            g.step_default(lambda cmd, env=None, quiet=False:
                           (1, "FAIL   suite:check-exports  0.1s\nFAIL   py:x  0.1s\n"))
        self.assertEqual((c.exception.n, c.exception.step), (1, "default"))
        with self.assertRaises(g.GateFail):   # rc != 0 with nothing parseable is still a failure
            g.step_default(lambda cmd, env=None, quiet=False: (1, ""))

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


def scoped_runner(compat_plan=None, listing="", compat_rc=0, list_rc=0, run_rc=0, calls=None):
    """Fake runner for scoped step 1: compat_check writes compat_plan to its --json path, --list prints `listing`."""
    calls = [] if calls is None else calls

    def runner(cmd, env=None, quiet=False):
        calls.append(cmd)
        if str(g.COMPAT_CHECK) in cmd:
            if compat_plan is not None:
                Path(cmd[cmd.index("--json") + 1]).write_text(json.dumps({"plan": compat_plan}))
            return compat_rc, ""
        if "--list" in cmd:
            return list_rc, listing
        return run_rc, ""
    return runner, calls


PLAN = [{"kind": "build", "id": "build_check:x_src"}, {"kind": "test", "id": "cpp:x_test"},
        {"kind": "static", "id": "suite:check-raw-addresses"}, {"kind": "test", "id": "cpp:gone_test"}]
LISTING = "\n".join(["cpp:x_test  [cpp]", "cpp:other_test  [cpp]", "suite:check-raw-addresses  [suite]",
                     "suite:check-hook-bytes  [suite]  SKIP (x)", "suite:check-exports  [suite]", "py:a.py  [py]"])


class ScopedStep1(unittest.TestCase):
    def setUp(self):
        self.td = tempfile.TemporaryDirectory()
        self.scratch = Path(self.td.name) / "s"
        p = mock.patch.object(g, "mod_source_files", return_value=["patch_manager_mods/x_src/A.h"])
        p.start()
        self.addCleanup(p.stop)
        self.addCleanup(self.td.cleanup)

    def run_step(self, runner, full=False):
        buf = io.StringIO()
        with redirect_stdout(buf):
            g.step_default(runner, ["x_src"], self.scratch, full)
        return buf.getvalue()

    def test_ids_file_from_compat_and_list(self):
        runner, calls = scoped_runner(PLAN, LISTING)
        out = self.run_step(runner)
        ids = (self.scratch / "step1_ids.txt").read_text().split()
        # non-build plan ids + GLOBAL_SUITES, only those run_tests --list knows, no dups, no build entry
        self.assertEqual(ids, ["cpp:x_test", "suite:check-raw-addresses", "suite:check-hook-bytes", "suite:check-exports"])
        self.assertIn("== step 1/4: run_tests.py (scoped: 4 items; --full-default for all)", out)
        compat = calls[0]
        self.assertEqual(compat[compat.index("--json") + 1], str(self.scratch / "compat.json"))
        self.assertIn("patch_manager_mods/x_src/A.h", compat)
        self.assertIn("--list", compat)
        final = calls[-1]
        self.assertEqual(final[-2:], ["--ids-file", str(self.scratch / "step1_ids.txt")])

    def test_global_suites_exist_in_suites_toml(self):
        text = (g.REPO / "tests" / "suites.toml").read_text()
        if not text:
            self.skipTest("no suites.toml")
        for sid in g.GLOBAL_SUITES:
            self.assertIn(f'"{sid.split(":", 1)[1]}"', text, sid)

    def test_compat_failure_falls_back_to_full(self):
        for kw in ({"compat_plan": None}, {"compat_plan": PLAN, "compat_rc": 2}):
            runner, calls = scoped_runner(listing=LISTING, **kw)
            out = self.run_step(runner)
            self.assertIn("WARN  step 1: compat_check.py", out)
            self.assertIn("falling back to the full default tier", out)
            self.assertNotIn("--ids-file", calls[-1])
            self.assertEqual(calls[-1][1:], [str(g.RUN_TESTS)])

    def test_empty_intersection_falls_back(self):
        runner, calls = scoped_runner(PLAN, "py:a.py  [py]\n")
        out = self.run_step(runner)
        self.assertIn("falling back to the full default tier", out)
        self.assertNotIn("--ids-file", calls[-1])

    def test_full_default_skips_compat(self):
        runner, calls = scoped_runner(PLAN, LISTING)
        out = self.run_step(runner, full=True)
        self.assertEqual(len(calls), 1)
        self.assertNotIn("--ids-file", calls[0])
        self.assertIn("full default tier", out)

    def test_scoped_run_keeps_deferred_tolerance(self):
        runner = scoped_runner(PLAN, LISTING)[0]

        def wrapped(cmd, env=None, quiet=False):
            rc, out = runner(cmd, env, quiet)
            return (1, "FAIL   suite:check-exports  0.1s\n") if "--ids-file" in cmd else (rc, out)
        self.assertIn("deferred to step 3", self.run_step(wrapped))


class ScratchAndRecords(unittest.TestCase):
    def test_default_scratch_unique_per_run_and_under_tmp_base(self):
        a = g.default_scratch()
        self.assertTrue(a.is_relative_to(g.TMP_BASE))
        self.assertTrue(a.name.startswith(f"gate-{os.getpid()}-"))
        self.assertNotEqual(a, g.TMP_BASE / "gate")
        with mock.patch.object(g.os, "getpid", return_value=424242):
            self.assertNotEqual(a, g.default_scratch())
        self.assertEqual(g.safe_scratch(a), a.resolve())

    def gate_with_mocks(self, records, fail=False):
        dll = Path(records) / "fresh.dll"
        dll.write_bytes(b"FRESH-DLL")
        scratch = g.TMP_BASE / "gate-unittest-records"
        step_build = mock.Mock(side_effect=g.GateFail(2, "build", "boom")) if fail \
            else mock.Mock(return_value={"perf_lab_src/perflab": dll})
        with mock.patch.object(g, "step_build", step_build), \
                mock.patch.object(g, "step_scratch", return_value={"perf-lab": []}), \
                mock.patch.object(g, "step_exports"), mock.patch.object(g, "step_wine", return_value=[]), \
                redirect_stdout(io.StringIO()):
            rc = g.gate(["perf_lab_src/perflab"], skip_default=True, scratch=scratch, records_dir=Path(records) / "rec")
        return rc, dll, scratch

    def test_record_written_on_pass(self):
        with tempfile.TemporaryDirectory() as td:
            rc, dll, scratch = self.gate_with_mocks(td)
            self.assertEqual(rc, 0)
            rec = json.loads((Path(td) / "rec" / "perf-lab.json").read_text())
            self.assertEqual(set(rec), {"mod_id", "md5", "dll", "head", "time", "scratch"})
            self.assertEqual(rec["mod_id"], "perf-lab")
            self.assertEqual(rec["md5"], g.md5(dll))
            self.assertEqual((rec["dll"], rec["scratch"]), (str(dll), str(scratch.resolve())))
            self.assertEqual(rec["head"], g.git_head())
            self.assertRegex(rec["time"], r"^\d{4}-\d\d-\d\dT\d\d:\d\d:\d\dZ$")
            self.assertEqual([p.name for p in (Path(td) / "rec").iterdir()], ["perf-lab.json"])   # no tmp left over

    def test_no_record_on_fail(self):
        with tempfile.TemporaryDirectory() as td:
            rc, _, _ = self.gate_with_mocks(td, fail=True)
            self.assertEqual(rc, 1)
            self.assertFalse((Path(td) / "rec").exists())


if __name__ == "__main__":
    unittest.main()
