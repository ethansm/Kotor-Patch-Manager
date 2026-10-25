#!/usr/bin/env python3
"""Tests for the run_tests.py build cache, compile slots and per-run wine dirs. Host only: g++ and temp dirs."""

import hashlib
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
            if str(cmd[0]) == "g++" and "-M" not in cmd:   # dependency scans are not builds
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

    def test_isystem_header_content_change_rebuilds(self):
        sysdir = self.root / "sys"
        sysdir.mkdir()
        (sysdir / "sysval.h").write_text("#pragma once\ninline int sysval() { return 1; }\n")
        self.src.write_text('#include <sysval.h>\nint main() { return sysval() == 1 ? 0 : 1; }\n')
        flags = ["-isystem", str(sysdir)]
        r = rt.execute(self.item(extra_flags=flags))
        self.assertEqual((r.status, self.compiles), ("PASS", 1), r.output)
        side = Path(self.item(extra_flags=flags).cmd[0] + ".deps.json")
        self.assertIn(str(sysdir / "sysval.h"), json.loads(side.read_text())["deps"])
        rt.execute(self.item(extra_flags=flags))
        self.assertEqual(self.compiles, 1)
        (sysdir / "sysval.h").write_text("#pragma once\ninline int sysval() { return 2; }\n")
        r = rt.execute(self.item(extra_flags=flags))
        self.assertEqual((r.status, self.compiles), ("FAIL", 2))   # -M hashes -isystem headers (-MM omitted them)

    def test_sidecar_from_older_mm_scan_rebuilds(self):
        it = self.item()
        rt.execute(it)
        side = Path(it.cmd[0] + ".deps.json")
        data = json.loads(side.read_text())
        self.assertEqual(data["scan"], rt.DEPS_SCAN)
        del data["scan"]                       # what a -MM era sidecar looks like
        side.write_text(json.dumps(data))
        rt.execute(self.item())
        self.assertEqual(self.compiles, 2)
        rt.execute(self.item())
        self.assertEqual(self.compiles, 2)

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

    def test_multi_source_compile_validates_every_translation_unit(self):
        """-MMD of a multi-source compile lists only the last TU: edits to the main TU's headers must still rebuild."""
        (self.src_dir / "m.h").write_text("#define M 0\n")
        (self.src_dir / "m.cpp").write_text('#include "m.h"\nint other();\nint main() { return other() + M; }\n')
        (self.src_dir / "o.h").write_text("#define O 0\n")
        (self.src_dir / "o.cpp").write_text('#include "o.h"\nint other() { return O; }\n')
        flags = ["-I", str(self.src_dir)]
        srcs = [str(self.src_dir / "m.cpp"), str(self.src_dir / "o.cpp")]
        exe = self.exe_dir / "exe-multi"
        self.exe_dir.mkdir(exist_ok=True)
        key = rt.cache_key(self.src_dir / "m.cpp", ["g++"] + flags + srcs, self.src_dir)
        it = rt.Item(id="x", kind="cpp", cwd=self.src_dir, build_cwd=self.src_dir, src=self.src_dir / "m.cpp",
                     build_cmd=["g++"] + flags + srcs + ["-o", str(exe)], cmd=[str(exe)], cache=rt.CacheInfo(exe, key))
        self.assertEqual(rt.execute(it).status, "PASS")
        deps = json.loads(exe.with_name(exe.name + ".deps.json").read_text())["deps"]
        for n in ("m.cpp", "m.h", "o.cpp", "o.h"):
            self.assertIn(str(self.src_dir / n), deps)
        (self.src_dir / "m.h").write_text("#define M 7\n")
        r = rt.execute(it)
        self.assertEqual((r.status, self.compiles), ("FAIL", 2))
        (self.src_dir / "o.h").write_text("#define O 3\n")
        self.assertEqual(rt.execute(it).status, "FAIL")
        self.assertEqual(self.compiles, 3)

    def test_new_shadowing_header_rebuilds(self):
        i1, i2 = self.root / "i1", self.root / "i2"
        i1.mkdir()
        i2.mkdir()
        (i2 / "s.h").write_text("#define S 0\n")
        (self.src_dir / "sh_test.cpp").write_text('#include <s.h>\nint main() { return S; }\n')
        exe = self.exe_dir / "exe-shadow"
        self.exe_dir.mkdir(exist_ok=True)
        fl = ["-I", str(i1), "-I", str(i2)]
        sp = str(self.src_dir / "sh_test.cpp")
        key = rt.cache_key(sp, ["g++"] + fl + [sp], self.src_dir)
        it = rt.Item(id="y", kind="cpp", cwd=self.src_dir, build_cwd=self.src_dir,
                     build_cmd=["g++"] + fl + [sp, "-o", str(exe)], cmd=[str(exe)], cache=rt.CacheInfo(exe, key))
        self.assertEqual(rt.execute(it).status, "PASS")
        self.assertEqual(rt.execute(it).status, "PASS")
        self.assertEqual(self.compiles, 1)
        (i1 / "s.h").write_text("#define S 5\n")     # now wins the search order
        self.assertEqual(rt.execute(it).status, "FAIL")
        self.assertEqual(self.compiles, 2)

    def _shadow_case(self, source, flags, make_shadow, name="sh"):
        """Build+run once (PASS), run again (still PASS from cache), add a shadowing file, run again (must FAIL)."""
        src = self.src_dir / f"{name}_test.cpp"
        src.write_text(source)
        exe = self.exe_dir / f"exe-{name}"
        self.exe_dir.mkdir(exist_ok=True)
        argv = ["g++"] + flags + [str(src)]
        key = rt.cache_key(src, argv, self.src_dir)
        it = rt.Item(id=name, kind="cpp", cwd=self.src_dir, build_cwd=self.src_dir,
                     build_cmd=argv + ["-o", str(exe)], cmd=[str(exe)], cache=rt.CacheInfo(exe, key))
        self.assertEqual(rt.execute(it).status, "PASS")
        self.assertEqual(rt.execute(it).status, "PASS")
        self.assertEqual(self.compiles, 1)
        make_shadow()
        self.assertEqual(rt.execute(it).status, "FAIL")
        self.assertEqual(self.compiles, 2)

    def test_shadow_with_cpp_suffix_rebuilds(self):       # F1a: the repo #includes .cpp files
        i1, i2 = self.root / "i1", self.root / "i2"
        i1.mkdir()
        i2.mkdir()
        (i2 / "s.cpp").write_text("#define S 0\n")
        self._shadow_case("#include <s.cpp>\nint main() { return S; }\n", ["-I", str(i1), "-I", str(i2)],
                          lambda: (i1 / "s.cpp").write_text("#define S 5\n"))

    def test_shadow_inside_pruned_dir_name_rebuilds(self):  # F1b: <fixtures/s.h> is a valid include
        i1, i2 = self.root / "i1", self.root / "i2"
        (i1 / "fixtures").mkdir(parents=True)
        (i2 / "fixtures").mkdir(parents=True)
        (i2 / "fixtures" / "s.h").write_text("#define S 0\n")
        self._shadow_case("#include <fixtures/s.h>\nint main() { return S; }\n", ["-I", str(i1), "-I", str(i2)],
                          lambda: (i1 / "fixtures" / "s.h").write_text("#define S 5\n"))

    def test_shadow_via_symlinked_subdir_rebuilds(self):    # F1c
        i1, i2, outside = self.root / "i1", self.root / "i2", self.root / "outside"
        i1.mkdir()
        outside.mkdir()
        (i2 / "link").mkdir(parents=True)
        (i2 / "link" / "s.h").write_text("#define S 0\n")
        (i1 / "link").symlink_to(outside)
        self._shadow_case("#include <link/s.h>\nint main() { return S; }\n", ["-I", str(i1), "-I", str(i2)],
                          lambda: (outside / "s.h").write_text("#define S 5\n"))

    def test_symlink_loop_does_not_hang(self):
        i1 = self.root / "i1"
        (i1 / "sub").mkdir(parents=True)
        (i1 / "sub" / "back").symlink_to(i1)
        self.assertEqual(len(rt.include_fingerprint([str(i1)], [])), 64)

    def test_shadow_in_earlier_cpath_dir_rebuilds(self):    # F2
        c1, c2 = self.root / "c1", self.root / "c2"
        c1.mkdir()
        c2.mkdir()
        (c2 / "s.h").write_text("#define S 0\n")
        with mock.patch.dict(os.environ, {"CPATH": f"{c1}{os.pathsep}{c2}"}):
            self._shadow_case("#include <s.h>\nint main() { return S; }\n", [],
                              lambda: (c1 / "s.h").write_text("#define S 5\n"))

    def test_deleted_dependency_rebuilds(self):
        rt.execute(self.item())
        (self.src_dir / "val.h").unlink()
        r = rt.execute(self.item())
        self.assertEqual((r.status, self.compiles), ("FAIL", 2))
        self.assertTrue(r.reason.startswith("build failed"))

    def test_compiler_path_change_changes_key(self):
        a = self.item().cache.key
        try:
            rt.compiler_path.cache_clear()
            with mock.patch.object(rt.shutil, "which", return_value="/opt/other-gcc/bin/g++"):
                b = self.item().cache.key
        finally:
            rt.compiler_path.cache_clear()
        self.assertNotEqual(a, b)
        self.assertEqual(a, self.item().cache.key)

    def test_own_source_absent_from_deps_writes_no_sidecar(self):
        dep = str(self.src_dir / "val.h")
        with mock.patch.object(rt, "parse_depfile", return_value=[dep]), mock.patch.object(rt, "_note") as note:
            self.assertEqual(rt.execute(self.item()).status, "PASS")
        self.assertTrue(note.called)
        exe = Path(self.item().cmd[0])
        self.assertFalse(exe.with_name(exe.name + ".deps.json").exists())

    def test_shadowing_header_beside_the_test_rebuilds(self):
        inc = self.root / "inc"
        inc.mkdir()
        (inc / "q.h").write_text("#define Q 0\n")
        self.src.write_text('#include "q.h"\nint main() { return Q; }\n')
        self.assertEqual(rt.execute(self.item(include_dirs=[inc])).status, "PASS")
        (self.src_dir / "q.h").write_text("#define Q 4\n")   # a quoted include finds the test's own dir first
        self.assertEqual(rt.execute(self.item(include_dirs=[inc])).status, "FAIL")

    def test_editing_the_test_source_rebuilds(self):
        rt.execute(self.item())
        self.src.write_text('#include "val.h"\nint main() { return 9; }\n')
        r = rt.execute(self.item())
        self.assertEqual((r.status, self.compiles), ("FAIL", 2))

    def test_exe_hash_mismatch_rebuilds(self):
        it = self.item()
        rt.execute(it)
        exe = Path(it.cmd[0])
        exe.write_bytes(exe.read_bytes() + b"\0")        # not the exe the sidecar vouches for
        r = rt.execute(self.item())
        self.assertEqual((r.status, self.compiles), ("PASS", 2))
        self.assertIn("exe_sha256", json.loads(exe.with_name(exe.name + ".deps.json").read_text()))

    def test_env_include_path_changes_key(self):
        a = self.item().cache.key
        with mock.patch.dict(os.environ, {"CPATH": str(self.root)}):
            b = self.item().cache.key
        self.assertNotEqual(a, b)

    def test_refused_sidecar_prints_note(self):
        os.utime(self.src_dir / "val.h", (4102444800, 4102444800))   # year 2100: newer than any build start
        with mock.patch.object(rt, "_note") as note:
            self.assertEqual(rt.execute(self.item()).status, "PASS")
        self.assertTrue(note.called)
        exe = Path(self.item().cmd[0])
        self.assertFalse(exe.with_name(exe.name + ".deps.json").exists())

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

    def test_suite_runs_under_its_own_lock_before_the_slot(self):
        calls = []

        class Ctx:
            def __init__(s, tag):
                s.tag = tag

            def __enter__(s):
                calls.append(s.tag)

            def __exit__(s, *a):
                calls.append("/" + s.tag)
        with mock.patch.object(rt._load_limit, "heavy_slot", lambda: Ctx("slot")), \
                mock.patch.object(rt._load_limit, "exclusive", lambda name, label="": Ctx(name)), \
                mock.patch.object(rt, "run_cmd", lambda *a, **k: ("OK", "", "")):
            rt.execute(rt.Item(id="suite:levelup-scenarios", kind="suite", cmd=["true"]))
        lock = rt.suite_lock_name(rt.Item(id="suite:levelup-scenarios", kind="suite"))
        self.assertEqual(calls, [lock, "slot", "/slot", "/" + lock])
        self.assertTrue(lock.endswith("-levelup-scenarios"))
        self.assertEqual(rt.suite_lock_name(rt.Item(id="suite:a/b c", kind="suite")).rsplit("-", 1)[1], "a_b_c")

    def test_suite_env_carries_slot_held(self):
        seen = {}

        def fake(cmd, cwd, env=None, timeout=300):
            seen.update(env or {})
            return "OK", "", ""
        with mock.patch.object(rt, "run_cmd", fake):
            rt.execute(rt.Item(id="suite:x", kind="suite", cmd=["true"], env={"A": "1"}))
        self.assertEqual(seen.get("KOTOR_SLOT_HELD"), "1")
        self.assertEqual(seen.get("A"), "1")

    def test_build_root_is_per_checkout_and_lock_name_matches_build_check(self):
        self.assertEqual(rt.build_root().name, "build-" + hashlib.sha1(str(rt.REPO).encode()).hexdigest()[:8])
        kd = rt.build_root() / "mod_src"
        self.assertEqual(rt._build_lock_name(kd), "build-" + hashlib.sha1(str(kd.resolve()).encode()).hexdigest()[:12])

    def test_ensure_built_dll_copies_under_lock_and_never_builds_under_it(self):
        events = []

        class Lock:
            def __init__(s, name, label=""):
                s.name = name

            def __enter__(s):
                events.append(("lock", s.name))

            def __exit__(s, *a):
                events.append(("unlock", s.name))
        with tempfile.TemporaryDirectory() as td:
            td = Path(td)
            mod = rt.REPO / "patch_manager_mods" / "zz_nonexistent_src"   # not on disk: nothing to be newer than the dll
            tmp = td / "tmp"
            bdir = tmp / ("build-" + hashlib.sha1(str(rt.REPO).encode()).hexdigest()[:8])
            dll = bdir / "zz_nonexistent_src" / "M" / "build" / "m.dll"
            dll.parent.mkdir(parents=True)
            dll.write_bytes(b"dll")
            dest = td / "out.dll"
            with mock.patch.object(rt._load_limit, "exclusive", Lock), mock.patch.object(rt, "TMP", tmp), \
                    mock.patch.object(rt, "KPM_DIR", td / "nokpm"), \
                    mock.patch.object(rt.shutil, "copy2", lambda a, b: events.append(("copy", str(b)))), \
                    mock.patch.object(rt.subprocess, "run", side_effect=AssertionError("must not rebuild")):
                os.utime(dll, (4102444800, 4102444800))
                rt._ensure_built_dll(mod, dest)
        self.assertEqual([e[0] for e in events], ["lock", "copy", "unlock"])

    def test_ensure_built_dll_uses_the_dll_it_just_built_even_if_sources_look_newer(self):
        with tempfile.TemporaryDirectory() as td:
            td = Path(td)
            tmp = td / "tmp"
            key_dir = tmp / ("build-" + hashlib.sha1(str(rt.REPO).encode()).hexdigest()[:8]) / "zz_nonexistent_src"
            dll = key_dir / "M" / "build" / "m.dll"
            calls = []

            def fake_build(cmd, **kw):
                calls.append(cmd)
                dll.parent.mkdir(parents=True, exist_ok=True)
                dll.write_bytes(b"dll")
                return mock.Mock(returncode=0, stdout="")
            dest = td / "out.dll"
            with mock.patch.object(rt, "TMP", tmp), mock.patch.object(rt, "_dll_is_fresh", return_value=False), \
                    mock.patch.object(rt.subprocess, "run", fake_build):
                rt._ensure_built_dll(rt.REPO / "patch_manager_mods" / "zz_nonexistent_src", dest)
            self.assertEqual(len(calls), 1)
            self.assertEqual(dest.read_bytes(), b"dll")
            self.assertIn("--build-root", calls[0])

    def test_ensure_built_dll_fails_when_build_produces_no_dll(self):
        with tempfile.TemporaryDirectory() as td:
            with mock.patch.object(rt, "TMP", Path(td)), \
                    mock.patch.object(rt.subprocess, "run", return_value=mock.Mock(returncode=0, stdout="")):
                with self.assertRaises(RuntimeError):
                    rt._ensure_built_dll(rt.REPO / "patch_manager_mods" / "zz_nonexistent_src", Path(td) / "o.dll")

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
