#!/usr/bin/env python3
"""Tests for scripts/build_check.py (doc 20 build gate).

Hermetic apart from the MinGW compiler: the "KPM tree" is a tiny fake (Patches/Common with two trivial sources,
lib/sqlite3.def), the repo is a temp tree with one fixture mod under patch_manager_mods/x_src, and the build and
Common-cache dirs are temp dirs. Tests that really compile are skipped when i686-w64-mingw32-g++ is absent.
"""
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import _static_common as sc  # noqa: E402
import build_check as bc  # noqa: E402

HAVE_CC = bc.find_compiler() is not None
needs_cc = unittest.skipUnless(HAVE_CC, "i686-w64-mingw32-g++ not installed")

GOOD_CPP = '''#include "../_shared/Shared.h"
extern "C" __declspec(dllexport) void __cdecl Detour(void) { (void)SHARED_VALUE; }
'''
HOOKS = ('[metadata]\ntarget_versions = ["AB"]\n[[hooks]]\naddress = 4198400\ntype = "detour"\nfunction = "{fn}"\n'
         'original_bytes = [144,144,144,144,144]\n')


def make_kpm(root: Path) -> Path:
    kpm = root / "kpm"
    (kpm / "Patches" / "Common" / "GameAPI").mkdir(parents=True)
    (kpm / "Patches" / "Common" / "Common.cpp").write_text("int common_value() { return 7; }\n")
    (kpm / "Patches" / "Common" / "GameAPI" / "Api.cpp").write_text("int api_value() { return 9; }\n")
    (kpm / "Patches" / "Common" / "Common.h").write_text("#pragma once\n")
    (kpm / "lib").mkdir()
    (kpm / "lib" / "sqlite3.def").write_text("LIBRARY sqlite3.dll\nEXPORTS\n    sqlite3_libversion\n")
    (kpm / "lib" / "sqlite3.h").write_text("")
    return kpm


def make_repo(root: Path, cpp=GOOD_CPP, hook_fn="Detour", def_names=("Detour",), kpm_name=None, kpm=None) -> Path:
    """repo/patch_manager_mods/x_src: one cpp, exports.def, manifest + hooks toml; plus _shared_src/Shared.h and
    host-test sources that must NOT be compiled (a syntax-error *_test.cpp and a t32_ file)."""
    repo = root / "repo"
    mods = repo / "patch_manager_mods"
    md = mods / "x_src"
    md.mkdir(parents=True)
    (md / "X.cpp").write_text(cpp)
    (md / "x_logic_test.cpp").write_text("this is not C++ and must not be compiled\n")
    (md / "t32_x.cpp").write_text("neither is this\n")
    (md / "manifest.toml").write_text('[patch]\nid = "x"\nversion = "1.0.0"\n')
    (md / sc.HOOKS_TOML).write_text(HOOKS.format(fn=hook_fn))
    (md / "exports.def").write_text("LIBRARY X\nEXPORTS\n" + "".join(f"    {n}\n" for n in def_names))
    shared = mods / "_shared_src"
    shared.mkdir()
    (shared / "Shared.h").write_text("#pragma once\n#define SHARED_VALUE 5\n")
    (shared / "shared_test.cpp").write_text("not C++\n")
    if kpm_name and kpm:
        kd = kpm / "Patches" / kpm_name
        kd.mkdir(parents=True, exist_ok=True)
        (kd / "manifest.toml").write_text('[patch]\nid = "x"\nversion = "1.0.0"\n')
    return repo


class Env:
    """Temp workspace shared by the compile tests (the Common library is built once per class)."""

    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.root = Path(cls.tmp.name)
        cls.kpm = make_kpm(cls.root)
        cls.cache = cls.root / "cache"

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def go(self, repo, only=None, keep=False, build=None, kpm=None, **kw):
        rep = sc.Reporter(out=None)
        build = build or self.root / f"build_{id(rep)}"
        bc.run(repo=repo, kpm_dir=kpm or self.kpm, only=only, keep=keep, reporter=rep, build_root=build,
               cache_root=self.cache, err_stream=None, jobs=2, **kw)
        return rep, build


@needs_cc
class CompileTests(Env, unittest.TestCase):
    def fresh_repo(self, kpm=None, **kw):
        d = Path(tempfile.mkdtemp(dir=self.root))
        return make_repo(d, kpm=kpm, **kw)

    def test_good_mod_builds_and_passes(self):
        rep, build = self.go(self.fresh_repo())
        self.assertEqual(rep.exit_code(), 0, [str(f) for f in rep.findings])
        self.assertEqual([f.check for f in rep.of("PASS")], ["build"])
        self.assertIn("x_src (x_src)", rep.of("PASS")[0].msg)
        self.assertFalse((build / "x_src").exists(), "build dir must be cleaned without --keep")

    def test_keep_leaves_a_staged_tree_without_test_sources(self):
        rep, build = self.go(self.fresh_repo(), keep=True)
        self.assertEqual(rep.exit_code(), 0)
        staged = build / "x_src"
        self.assertTrue((staged / "x_src" / "build" / "windows_x86.dll").is_file())
        self.assertTrue((staged / "x_src" / "X.cpp").is_file())
        self.assertFalse((staged / "x_src" / "x_logic_test.cpp").exists())
        self.assertFalse((staged / "x_src" / "t32_x.cpp").exists())
        self.assertTrue((staged / "_shared" / "Shared.h").is_file())
        self.assertFalse((staged / "_shared" / "shared_test.cpp").exists())

    def test_syntax_error_fails_with_compiler_output(self):
        repo = self.fresh_repo(cpp='extern "C" __declspec(dllexport) void __cdecl Detour(void) { int = ; }\n')
        rep = sc.Reporter(out=None)
        out = []

        class Sink:
            def write(self, s):
                out.append(s)
        bc.run(repo=repo, kpm_dir=self.kpm, reporter=rep, build_root=self.root / "b_syn", cache_root=self.cache,
               err_stream=Sink())
        self.assertEqual(rep.exit_code(), 1)
        self.assertEqual([f.check for f in rep.of("FAIL")], ["build"])
        text = "".join(out)
        self.assertIn("X.cpp", text)
        self.assertIn("error", text)

    def test_unexported_hook_function_fails(self):
        # the hooks toml names a function the DLL does not export (defined, but missing from exports.def)
        repo = self.fresh_repo(hook_fn="Other", def_names=("Detour",))
        rep, _ = self.go(repo)
        self.assertEqual(rep.exit_code(), 1)
        fails = rep.of("FAIL")
        self.assertEqual([f.check for f in fails], ["exports"])
        self.assertIn("Other", fails[0].msg)
        self.assertNotIn("Detour", fails[0].msg)

    def test_def_names_an_undefined_function_fails_to_link(self):
        repo = self.fresh_repo(def_names=("Detour", "Ghost"))
        rep, _ = self.go(repo)
        self.assertEqual(rep.exit_code(), 1)
        self.assertEqual([f.check for f in rep.of("FAIL")], ["build"])

    def test_missing_mod_dir_fails(self):
        rep, _ = self.go(self.fresh_repo(), only=["x_src", "nope_src"])
        self.assertEqual(rep.exit_code(), 1)
        self.assertIn("nope_src", rep.of("FAIL")[0].msg)

    def test_generated_inc_borrowed_from_kpm_warns(self):
        cpp = '#include "../_shared/Shared.h"\n#include "Gen.inc"\nextern "C" __declspec(dllexport) void __cdecl ' \
              'Detour(void) { (void)GEN_VALUE; }\n'
        kpm = Path(tempfile.mkdtemp(dir=self.root)) / "kpm"
        shutil.copytree(self.kpm, kpm)   # own copy: the KPM Patches/<Name> match must not leak into other tests
        repo = self.fresh_repo(cpp=cpp, kpm_name="XPatch", kpm=kpm)
        (kpm / "Patches" / "XPatch" / "Gen.inc").write_text("#define GEN_VALUE 3\n")
        rep, _ = self.go(repo, kpm=kpm)
        self.assertEqual(rep.exit_code(), 0, [str(f) for f in rep.findings])
        self.assertEqual([f.check for f in rep.of("WARN")], ["inc"])
        self.assertIn("Gen.inc", rep.of("WARN")[0].msg)
        self.assertIn("XPatch", rep.of("PASS")[0].msg)   # the KPM dir name is the staged <Name>

    def test_cli_exit_codes(self):
        env = dict(os.environ, TMPDIR=str(self.root / "cli_tmp"))
        (self.root / "cli_tmp").mkdir(exist_ok=True)

        def cli(repo):
            return subprocess.run([sys.executable, str(HERE / "build_check.py"), "--repo", str(repo), "--kpm-dir",
                                   str(self.kpm), "--quiet"], capture_output=True, text=True, env=env)
        good = cli(self.fresh_repo())
        self.assertEqual(good.returncode, 0, good.stdout + good.stderr)
        self.assertIn("FAIL=0", good.stdout)
        bad = cli(self.fresh_repo(cpp="int broken(\n"))
        self.assertEqual(bad.returncode, 1, bad.stdout)
        self.assertIn("FAIL build", bad.stdout)


class SkipAndUnitTests(unittest.TestCase):
    def test_absent_compiler_skips_with_exit_0(self):
        with tempfile.TemporaryDirectory() as t:
            t = Path(t)
            kpm = make_kpm(t)
            repo = make_repo(t)
            rep = sc.Reporter(out=None)
            env = {k: v for k, v in os.environ.items() if k != bc.CXX_ENV}
            with mock.patch.dict(os.environ, env, clear=True), mock.patch.object(shutil, "which", return_value=None):
                self.assertIsNone(bc.find_compiler())
                bc.run(repo=repo, kpm_dir=kpm, reporter=rep, build_root=t / "b", cache_root=t / "c")
            self.assertEqual([f.sev for f in rep.findings], ["SKIP"])
            self.assertEqual(rep.exit_code(), 0)
            self.assertIn("mingw", rep.findings[0].msg)
            self.assertFalse((t / "b").exists())

    @needs_cc
    def test_missing_kpm_tree_skips(self):
        with tempfile.TemporaryDirectory() as t:
            t = Path(t)
            rep = sc.Reporter(out=None)
            bc.run(repo=make_repo(t), kpm_dir=t / "no_kpm", reporter=rep, build_root=t / "b", cache_root=t / "c")
            self.assertEqual([f.sev for f in rep.findings], ["SKIP"])
            self.assertEqual(rep.exit_code(), 0)

    def test_is_test_source(self):
        self.assertTrue(bc.is_test_source("sort_logic_test.cpp"))
        self.assertTrue(bc.is_test_source("t32_vtable.cpp"))
        self.assertFalse(bc.is_test_source("Sorter.cpp"))
        self.assertFalse(bc.is_test_source("attest.cpp"))

    def test_pragma_libraries(self):
        with tempfile.TemporaryDirectory() as t:
            f = Path(t) / "a.cpp"
            f.write_text('#pragma comment(lib, "opengl32.lib")\n#pragma comment (lib,"user32")\n'
                         '#pragma comment(lib, "opengl32")\n')
            self.assertEqual(bc.pragma_libraries([f]), ["-lopengl32", "-luser32"])

    def test_stage_mod_filters_files_and_depth(self):
        with tempfile.TemporaryDirectory() as t:
            t = Path(t)
            src = t / "m"
            (src / "sub" / "deep").mkdir(parents=True)
            (src / "testdata").mkdir()
            for rel in ("A.cpp", "A.h", "T.inc", "exports.def", "a_test.cpp", "t32_a.cpp", "notes.md", "pic.tga",
                        "sub/B.cpp", "sub/B.h", "sub/deep/C.cpp", "sub/deep/C.h", "testdata/D.cpp"):
                (src / rel).write_text("x\n")
            shared = t / "sh"
            shared.mkdir()
            for n in ("S.h", "s_test.cpp", "test.toml"):
                (shared / n).write_text("x\n")
            dest = bc.stage_mod(src, "Name", t / "build" / "key", shared)
            got = sorted(str(p.relative_to(dest)) for p in dest.rglob("*") if p.is_file())
            self.assertEqual(got, ["A.cpp", "A.h", "T.inc", "exports.def", "sub/B.cpp", "sub/B.h", "sub/deep/C.h"])
            self.assertEqual(sorted(p.name for p in (dest.parent / "_shared").iterdir()), ["S.h"])
            self.assertEqual([p.name for p in bc.find_sources(dest)], ["A.cpp", "B.cpp"])

    def test_stage_mod_overlay_variant(self):
        """A variant dir without .cpp (dialogue_letterbox_fix_src/probe/) stages the parent's sources + its own files."""
        with tempfile.TemporaryDirectory() as t:
            t = Path(t)
            src = t / "m"
            (src / "probe").mkdir(parents=True)
            for rel, txt in (("A.cpp", "a"), ("A.h", "a"), ("exports.def", "ship"), ("manifest.toml", "ship"),
                             ("probe/exports.def", "probe"), ("probe/manifest.toml", "probe"), ("probe/v.h", "v"),
                             ("probe/notes.md", "n")):
                (src / rel).write_text(txt + "\n")
            shared = t / "sh"
            shared.mkdir()
            dest = bc.stage_mod(src / "probe", "Name", t / "build" / "key", shared)
            self.assertEqual([p.name for p in bc.find_sources(dest)], ["A.cpp"])
            self.assertEqual((dest / "exports.def").read_text(), "probe\n")
            self.assertEqual((dest / "manifest.toml").read_text(), "probe\n")
            self.assertTrue((dest / "v.h").is_file())
            self.assertFalse((dest / "notes.md").exists())
            ship = bc.stage_mod(src, "Name", t / "build" / "key2", shared)
            self.assertEqual((ship / "exports.def").read_text(), "ship\n")
            self.assertFalse((ship / "v.h").exists())

    def test_discover_default_and_explicit(self):
        with tempfile.TemporaryDirectory() as t:
            t = Path(t)
            repo = make_repo(t)
            mods = repo / "patch_manager_mods"
            (mods / "perf_src" / "nested").mkdir(parents=True)
            (mods / "perf_src" / "nested" / "exports.def").write_text("EXPORTS\n    F\n")
            (mods / "no_def_src").mkdir()
            (mods / "retired" / "old_src").mkdir(parents=True)
            (mods / "retired" / "old_src" / "exports.def").write_text("EXPORTS\n")
            (mods / "plain_dir").mkdir()
            (mods / "plain_dir" / "exports.def").write_text("EXPORTS\n")   # not under a *_src dir
            found = bc.discover(repo, t / "no_kpm")
            self.assertEqual([m.key for m in found], ["perf_src__nested", "x_src"])
            x = next(m for m in found if m.key == "x_src")
            self.assertEqual(x.funcs, {"Detour"})
            self.assertEqual(x.name, "x_src")
            one = bc.discover(repo, t / "no_kpm", only=["perf_src/nested"])
            self.assertEqual([m.key for m in one], ["perf_src__nested"])


if __name__ == "__main__":
    unittest.main()
