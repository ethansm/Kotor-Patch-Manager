#!/usr/bin/env python3
"""Tests for the run_tests.py build cache, compile slots and per-run wine dirs. Host only: g++ and temp dirs."""

import json
import os
import shutil
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import run_tests as rt  # noqa: E402

DEP_TEXT = (
    "/tmp/x/out.tmp.1.2: /tmp/x/a_test.cpp /tmp/x/a.h \\\n"
    " /tmp/x/with\\ space/b.h \\\r\n"
    " ../rel/c.h /tmp/x/a.h\n"
    " /tmp/x/dollar$$name.h\n"
)


class ParseDepfile(unittest.TestCase):
    def test_continuations_escapes_and_dedupe(self):
        self.assertEqual(rt.parse_depfile(DEP_TEXT),
                         ["/tmp/x/a_test.cpp", "/tmp/x/a.h", "/tmp/x/with space/b.h", "../rel/c.h",
                          "/tmp/x/dollar$name.h"])

    def test_empty_and_target_only(self):
        self.assertEqual(rt.parse_depfile(""), [])
        self.assertEqual(rt.parse_depfile("out:\n"), [])


@unittest.skipUnless(shutil.which("g++"), "g++ not found")
class CacheBehaviour(unittest.TestCase):
    def setUp(self):
        self.td = tempfile.TemporaryDirectory()
        self.root = Path(self.td.name).resolve()
        self.src_dir = self.root / "mod_src"
        self.src_dir.mkdir()
        (self.src_dir / "val.h").write_text("#pragma once\ninline int val() { return 1; }\n")
        self.src = self.src_dir / "val_test.cpp"
        self.src.write_text('#include "val.h"\nint main() { return val() == 1 ? 0 : 1; }\n')
        self.exe_dir = self.root / "bin"
        self.patches = [mock.patch.object(rt, "TMP", self.root / "tmp"), mock.patch.object(rt, "USE_CACHE", True)]
        for p in self.patches:
            p.start()
        self.compiles = 0
        real = rt.run_cmd

        def counting(cmd, *a, **k):
            if str(cmd[0]) == "g++":
                self.compiles += 1
            return real(cmd, *a, **k)
        p = mock.patch.object(rt, "run_cmd", counting)
        p.start()
        self.patches.append(p)

    def tearDown(self):
        for p in self.patches:
            p.stop()
        self.td.cleanup()

    def item(self, **kw):
        kw.setdefault("sanitize", False)
        kw.setdefault("cfg", {})
        kw.setdefault("exe_dir", self.exe_dir)
        return rt.cpp_item(self.src, **kw)

    def test_miss_then_hit_skips_build(self):
        it = self.item()
        self.assertRegex(Path(it.cmd[0]).name, r"^mod_src__val_test-[0-9a-f]{12}$")
        r1 = rt.execute(it)
        self.assertEqual((r1.status, self.compiles), ("PASS", 1), r1.output)
        exe = Path(it.cmd[0])
        side = exe.with_name(exe.name + ".deps.json")
        data = json.loads(side.read_text())
        self.assertEqual(data["key"], it.cache.key)
        self.assertIn(str(self.src_dir / "val.h"), data["deps"])
        self.assertIn(str(self.src), data["deps"])
        self.assertEqual([p.name for p in self.exe_dir.iterdir() if ".tmp." in p.name], [])
        ino = exe.stat().st_ino
        os.utime(exe, (1000, 1000))
        r2 = rt.execute(self.item())
        self.assertEqual((r2.status, self.compiles), ("PASS", 1))
        self.assertEqual(exe.stat().st_ino, ino)
        self.assertGreater(exe.stat().st_mtime, 10000)   # hit refreshes mtime for the pruner

    def test_header_content_change_rebuilds(self):
        rt.execute(self.item())
        (self.src_dir / "val.h").write_text("#pragma once\ninline int val() { return 2; }\n")
        r = rt.execute(self.item())
        self.assertEqual((r.status, self.compiles), ("FAIL", 2))   # rebuilt: the test now fails
        (self.src_dir / "val.h").write_text("#pragma once\ninline int val() { return 1; }\n")
        r = rt.execute(self.item())
        self.assertEqual((r.status, self.compiles), ("PASS", 3))

    def test_same_content_new_mtime_still_hits(self):
        rt.execute(self.item())
        h = self.src_dir / "val.h"
        h.write_text(h.read_text())
        os.utime(h, (2000000000, 2000000000))
        rt.execute(self.item())
        self.assertEqual(self.compiles, 1)

    def test_flags_change_new_exe_name(self):
        a, b = self.item(), self.item(extra_flags=["-DFOO"])
        self.assertNotEqual(a.cmd[0], b.cmd[0])
        self.assertNotEqual(a.cache.key, b.cache.key)
        c = self.item(cfg={"cxxflags": ["-DBAR"]})
        self.assertNotEqual(a.cmd[0], c.cmd[0])
        self.assertNotEqual(a.cmd[0], self.item(sanitize=True).cmd[0])
        self.assertEqual(a.cmd[0], self.item().cmd[0])
        rt.execute(a)
        rt.execute(b)
        self.assertEqual(self.compiles, 2)
        self.assertTrue(Path(a.cmd[0]).exists() and Path(b.cmd[0]).exists())

    def test_no_cache_forces_rebuild_and_refreshes(self):
        rt.execute(self.item())
        with mock.patch.object(rt, "USE_CACHE", False):
            r = rt.execute(self.item())
        self.assertEqual((r.status, self.compiles), ("PASS", 2))
        rt.execute(self.item())                  # entry refreshed: hit again
        self.assertEqual(self.compiles, 2)

    def test_missing_sidecar_or_exe_rebuilds(self):
        it = self.item()
        rt.execute(it)
        exe = Path(it.cmd[0])
        exe.with_name(exe.name + ".deps.json").unlink()
        rt.execute(self.item())
        self.assertEqual(self.compiles, 2)
        exe.unlink()
        rt.execute(self.item())
        self.assertEqual(self.compiles, 3)

    def test_build_failure_reported_and_not_cached(self):
        self.src.write_text("int main() { return nope; }\n")
        r = rt.execute(self.item())
        self.assertEqual(r.status, "FAIL")
        self.assertTrue(r.reason.startswith("build failed"))
        self.assertEqual(list(self.exe_dir.glob("*")), [])

    def test_unparseable_depfile_builds_without_cache(self):
        with mock.patch.object(rt, "parse_depfile", return_value=[]):
            r = rt.execute(self.item())
        self.assertEqual(r.status, "PASS")
        exe = Path(self.item().cmd[0])
        self.assertTrue(exe.exists())
        self.assertFalse(exe.with_name(exe.name + ".deps.json").exists())
        rt.execute(self.item())
        self.assertEqual(self.compiles, 2)

    def test_coverage_builds_straight_to_final_name(self):
        it = self.item(extra_flags=rt.COV_FLAGS)
        self.assertIsNone(it.cache)
        r = rt.execute(it)
        self.assertEqual(r.status, "PASS", r.output)
        exe = Path(it.cmd[0])
        self.assertTrue(list(self.exe_dir.glob(f"{exe.name}-val_test.gcda")), list(self.exe_dir.iterdir()))

    def test_scratch_tree_mutant_rebuilds(self):
        """mutate_header style: same relative layout under another root, header rewritten between runs."""
        scratch = self.root / "scratch"
        shutil.copytree(self.src_dir, scratch / "mod_src")
        st = scratch / "mod_src" / "val_test.cpp"
        a = rt.cpp_item(st, False, tag="_hb_0", cfg={}, exe_dir=self.exe_dir)
        self.assertEqual(rt.execute(a).status, "PASS")
        (scratch / "mod_src" / "val.h").write_text("#pragma once\ninline int val() { return 3; }\n")
        b = rt.cpp_item(st, False, tag="_h1_0", cfg={}, exe_dir=self.exe_dir)
        self.assertEqual(rt.execute(b).status, "FAIL")
        # even the same tag must rebuild when the header content differs
        c = rt.cpp_item(st, False, tag="_hb_0", cfg={}, exe_dir=self.exe_dir)
        self.assertEqual(rt.execute(c).status, "FAIL")
        self.assertEqual(self.compiles, 3)


class SlotsAndWine(unittest.TestCase):
    def test_suite_run_holds_a_slot_and_py_does_not(self):
        calls = []

        class Slot:
            def __enter__(s):
                calls.append("in")

            def __exit__(s, *a):
                calls.append("out")
        with mock.patch.object(rt._load_limit, "heavy_slot", lambda: Slot()), \
                mock.patch.object(rt, "run_cmd", lambda *a, **k: ("OK", "", "")):
            rt.execute(rt.Item(id="suite:x", kind="suite", cmd=["true"]))
            self.assertEqual(calls, ["in", "out"])
            rt.execute(rt.Item(id="py:x", kind="py", cmd=["true"]))
            self.assertEqual(calls, ["in", "out"])

    def test_cpp_compile_holds_a_slot(self):
        calls = []

        class Slot:
            def __enter__(s):
                calls.append("in")

            def __exit__(s, *a):
                calls.append("out")
        it = rt.Item(id="x", kind="x", build_cmd=["g++", "-o", "/nonexistent/x"], cmd=["true"])
        with mock.patch.object(rt._load_limit, "heavy_slot", lambda: Slot()), \
                mock.patch.object(rt, "run_cmd", lambda *a, **k: ("OK", "", "")):
            rt.build_item(it)
        self.assertEqual(calls, ["in", "out"])

    def test_wine_stage_runs_inside_exclusive_lock(self):
        events = []

        class Lock:
            def __init__(s, name, label=""):
                s.name = name

            def __enter__(s):
                events.append(("lock", s.name))

            def __exit__(s, *a):
                events.append(("unlock", s.name))

        def fake_run(cmd, cwd, env=None, timeout=300):
            events.append(("run", Path(str(cmd[0])).name, str(cwd)))
            return "OK", "", ""
        smoke = {"name": "sm", "timeout_s": 5}
        builds = {"sm": rt.Result("wine:sm", "wine", "PASS")}
        emitted = []
        with tempfile.TemporaryDirectory() as td:
            prefix_tmp = Path(td)
            wine = prefix_tmp / "bin" / "wine"
            wine.parent.mkdir()
            wine.write_text("")
            wserver = wine.with_name("wineserver")
            wserver.write_text("")
            with mock.patch.object(rt._load_limit, "exclusive", Lock), \
                    mock.patch.object(rt, "run_cmd", fake_run), \
                    mock.patch.object(rt, "TMP", prefix_tmp / "tmp"), \
                    mock.patch.object(rt, "WINE_RUN_DIR", prefix_tmp / "tmp" / "wine" / "run-1-2"), \
                    mock.patch.dict(os.environ, {"PROTON_WINE": str(wine)}):
                rt.wine_stage([("wine:sm", smoke, "", "")], {"sm": smoke}, builds, "", emitted.append)
        kinds = [e[0] for e in events]
        self.assertEqual(kinds[0], "lock")
        self.assertEqual(kinds[-1], "unlock")
        self.assertEqual(events[0][1], "wineprefix")
        runs = [e for e in events if e[0] == "run"]
        self.assertTrue(any(e[1] == "wineserver" for e in runs), runs)          # final kill, inside the lock
        self.assertTrue(any(e[1] == "wine" and e[2].endswith("run-1-2/sm") for e in runs), runs)   # per-run cwd
        self.assertEqual([r.status for r in emitted], ["PASS"])

    def test_wine_stage_skips_lock_when_nothing_to_run(self):
        with mock.patch.object(rt._load_limit, "exclusive", side_effect=AssertionError("no lock needed")):
            emitted = []
            rt.wine_stage([("wine:sm", {"name": "sm"}, "", "missing x")], {}, {}, "", emitted.append)
        self.assertEqual([r.status for r in emitted], ["SKIP"])

    def test_run_dir_removed_unless_wine_failed(self):
        with tempfile.TemporaryDirectory() as td:
            d = Path(td) / "run-1-2"
            for results, kept in (([rt.Result("wine:a", "wine", "PASS")], False),
                                  ([rt.Result("wine:a", "wine", "FAIL")], True),
                                  ([rt.Result("wine:a", "wine", "ERROR")], True),
                                  ([rt.Result("cpp:a", "cpp", "FAIL")], False)):
                d.mkdir(exist_ok=True)
                with mock.patch.object(rt, "WINE_RUN_DIR", d):
                    rt.finish_wine_run_dir(results)
                self.assertEqual(d.exists(), kept, results)
                shutil.rmtree(d, ignore_errors=True)

    def test_wine_root_follows_run_dir(self):
        with mock.patch.object(rt, "WINE_RUN_DIR", None), mock.patch.object(rt, "TMP", Path("/t")):
            self.assertEqual(rt.wine_root(), Path("/t/wine"))
        with mock.patch.object(rt, "WINE_RUN_DIR", Path("/t/wine/run-1-2")):
            self.assertEqual(rt.wine_root(), Path("/t/wine/run-1-2"))


if __name__ == "__main__":
    unittest.main()
