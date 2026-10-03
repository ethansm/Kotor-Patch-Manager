#!/usr/bin/env python3
"""Tests for scripts/check_exports.py (doc 20 Phase 1, step 3).

No real exe, KPM tree or game files: DLLs are tiny PE images with an export table built in memory (same helper shape
as scripts/test_static_checks.py), mods are _static_common.Mod objects, .kpatch zips are written to a temp dir.
"""
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import _static_common as sc  # noqa: E402
import check_exports as ce  # noqa: E402
from test_static_checks import build_pe, CODE_FLAGS  # noqa: E402


def dll(*names):
    return build_pe([(".text", 0x1000, b"\xc3" * 16, CODE_FLAGS)], exports=list(names))


def hook(function, type_="detour", addr=0x401000):
    return sc.Hook(address=addr, type=type_, orig=b"\x90" * 5, function=function, mod="m")


def mod(hooks, exports, mid="m", version="1.0.0", has_def=True, kpm_dir=None, mdir="/nonexistent"):
    return sc.Mod(id=mid, version=version, dir=Path(mdir), kpm_dir=kpm_dir, hooks=list(hooks), target_versions=[],
                  exports=list(exports), code_patches=[], has_def=has_def)


def run(mods, dlls=None, **kw):
    rep = sc.Reporter(out=None)
    ce.run(mods=mods, dlls=dlls if dlls is not None else {}, reporter=rep, **kw)
    return rep


def sevs(rep, check=None):
    return [f.sev for f in rep.findings if check is None or f.check == check]


class DefAndDllTests(unittest.TestCase):
    def test_all_exported_passes(self):
        m = mod([hook("A"), hook("B")], ["A", "B", "Seam"])
        rep = run([m], {"m": dll("A", "B", "Seam")})
        self.assertEqual(rep.exit_code(), 0)
        self.assertEqual(sevs(rep), ["PASS"])

    def test_name_missing_from_def_fails(self):
        m = mod([hook("A"), hook("B")], ["A"])
        rep = run([m], {"m": dll("A", "B")})
        self.assertEqual(rep.exit_code(), 1)
        fails = rep.of("FAIL")
        self.assertEqual([f.check for f in fails], ["def"])
        self.assertIn("B", fails[0].msg)
        self.assertNotIn("A,", fails[0].msg)

    def test_no_def_file_fails(self):
        rep = run([mod([hook("A")], [], has_def=False)], {"m": dll("A")})
        self.assertEqual(rep.exit_code(), 1)
        self.assertIn("no exports.def", rep.of("FAIL")[0].msg)

    def test_function_missing_from_dll_fails(self):
        m = mod([hook("A"), hook("B")], ["A", "B"])
        rep = run([m], {"m": dll("A")})
        self.assertEqual(rep.exit_code(), 1)
        fails = rep.of("FAIL")
        self.assertEqual([f.check for f in fails], ["dll"])
        self.assertIn("B", fails[0].msg)

    def test_extra_def_name_absent_from_dll_warns(self):
        m = mod([hook("A")], ["A", "TestSeam"])
        rep = run([m], {"m": dll("A")})
        self.assertEqual(rep.exit_code(), 0)
        self.assertEqual(sevs(rep, "stale-dll"), ["WARN"])
        self.assertIn("TestSeam", rep.of("WARN")[0].msg)

    def test_extra_dll_export_is_fine(self):
        rep = run([mod([hook("A")], ["A"])], {"m": dll("A", "Undeclared")})
        self.assertEqual(sevs(rep), ["PASS"])

    def test_repeated_function_names_are_fine(self):
        m = mod([hook("A", addr=0x401000), hook("A", addr=0x401100), hook("A", addr=0x401200)], ["A"])
        rep = run([m], {"m": dll("A")})
        self.assertEqual(sevs(rep), ["PASS"])
        self.assertIn("1 detour function", rep.of("PASS")[0].msg)

    def test_simple_hook_ignored(self):
        m = mod([hook("A"), hook(None, "simple")], ["A"])
        self.assertEqual(sevs(run([m], {"m": dll("A")})), ["PASS"])
        # a simple hook naming something unexported is still ignored
        m = mod([hook("A"), hook("NotExported", "simple")], ["A"])
        self.assertEqual(sevs(run([m], {"m": dll("A")})), ["PASS"])

    def test_mod_without_detours_skipped(self):
        for hooks in ([], [hook(None, "simple")]):
            rep = run([mod(hooks, [], has_def=False)], {})
            self.assertEqual(set(sevs(rep)), {"INFO"})
            self.assertIn("no detour hooks", rep.findings[0].msg)
            self.assertEqual(rep.exit_code(), 0)

    def test_detour_without_function_fails(self):
        rep = run([mod([hook(None)], ["A"])], {"m": dll("A")})
        self.assertEqual(rep.exit_code(), 1)

    def test_missing_or_bad_dll_fails(self):
        m = mod([hook("A")], ["A"])
        self.assertEqual(run([m], {"m": [("kp", None)]}).exit_code(), 1)
        self.assertEqual(run([m], {"m": b"not a dll" * 20}).exit_code(), 1)

    def test_dll_without_export_table_fails(self):
        plain = build_pe([(".text", 0x1000, b"\xc3" * 16, CODE_FLAGS)])
        self.assertEqual(run([mod([hook("A")], ["A"])], {"m": plain}).exit_code(), 1)

    def test_mod_filter(self):
        a, b = mod([hook("A")], ["A"], mid="a"), mod([hook("B")], [], mid="b")
        rep = run([a, b], {"a": dll("A"), "b": dll()}, mod_ids={"a"})
        self.assertEqual(rep.exit_code(), 0)
        self.assertEqual(len(rep.findings), 1)


class KpatchTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)

    def tearDown(self):
        self.tmp.cleanup()

    def kpatch(self, path, pid, version, dll_bytes, hooks_toml="[metadata]\n"):
        path.parent.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(path, "w") as z:
            z.writestr("manifest.toml", f'[patch]\nid = "{pid}"\nversion = "{version}"\n')
            z.writestr("kotor2-steam-aspyr.hooks.toml", hooks_toml)
            if dll_bytes is not None:
                z.writestr("binaries/windows_x86.dll", dll_bytes)
        return path

    def test_staged_kpatch_matched_by_id(self):
        m = mod([hook("A"), hook("B")], ["A", "B"], mid="x")
        good = self.kpatch(self.root / "good.kpatch", "x", "1.0.0", dll("A", "B"))
        bad = self.kpatch(self.root / "bad.kpatch", "x", "1.0.0", dll("A"))
        rep = sc.Reporter(out=None)
        ce.run(mods=[m], dlls={"x": dll("A", "B")}, kpatches=[good], reporter=rep)
        self.assertEqual(rep.exit_code(), 0)
        self.assertEqual(len(rep.of("PASS")), 2)
        rep = sc.Reporter(out=None)
        ce.run(mods=[m], dlls={"x": dll("A", "B")}, kpatches=[bad], reporter=rep)
        self.assertEqual(rep.exit_code(), 1)
        self.assertIn("bad.kpatch", rep.of("FAIL")[0].msg)

    def test_staged_kpatch_picks_variant_by_version(self):
        prod = mod([hook("A")], ["A"], mid="x", version="1.0.0")
        probe = mod([hook("A"), hook("P")], ["A", "P"], mid="x", version="1.0.1-probe")
        kp = self.kpatch(self.root / "p.kpatch", "x", "1.0.1-probe", dll("A", "P"))
        rep = sc.Reporter(out=None)
        ce.run(mods=[prod, probe], dlls={}, kpatches=[kp], mod_ids={"x"}, reporter=rep)
        self.assertEqual(rep.exit_code(), 0, [str(f) for f in rep.findings])
        # same staged build against the production source alone: P is not a hook there, still fine; the reverse fails
        kp2 = self.kpatch(self.root / "q.kpatch", "x", "1.0.1-probe", dll("A"))
        rep = sc.Reporter(out=None)
        ce.run(mods=[prod, probe], dlls={}, kpatches=[kp2], mod_ids={"x"}, reporter=rep)
        self.assertEqual(rep.exit_code(), 1)

    def test_staged_kpatch_unknown_id_warns_and_missing_dll_fails(self):
        m = mod([hook("A")], ["A"], mid="x")
        kp = self.kpatch(self.root / "u.kpatch", "unknown", "1", dll("A"))
        rep = sc.Reporter(out=None)
        ce.run(mods=[m], dlls={"x": dll("A")}, kpatches=[kp], reporter=rep)
        self.assertEqual(sevs(rep, "staged"), ["WARN"])
        self.assertEqual(rep.exit_code(), 0)
        nodll = self.kpatch(self.root / "n.kpatch", "x", "1.0.0", None)
        rep = sc.Reporter(out=None)
        ce.run(mods=[m], dlls={"x": dll("A")}, kpatches=[nodll], reporter=rep)
        self.assertEqual(rep.exit_code(), 1)

    def test_embedded_hooks_functions_are_checked(self):
        m = mod([hook("A")], ["A"], mid="x")
        toml = '[metadata]\n[[hooks]]\naddress = 4198400\ntype = "detour"\nfunction = "Old"\noriginal_bytes = [144,144,144,144,144]\n'
        kp = self.kpatch(self.root / "e.kpatch", "x", "1.0.0", dll("A"), toml)
        rep = sc.Reporter(out=None)
        ce.run(mods=[m], dlls={}, kpatches=[kp], reporter=rep)
        self.assertEqual(rep.exit_code(), 1)
        self.assertIn("Old", rep.of("FAIL")[0].msg)

    def test_kpm_dir_selects_matching_version_not_stale_file(self):
        d = self.root / "Patches" / "Foo"
        self.kpatch(d / "Foo.kpatch", "x", "2.0.0", dll("A"))
        self.kpatch(d / "Foo-1.0.0-live.kpatch", "x", "1.0.0", dll())   # stale: exports nothing
        m = mod([hook("A")], ["A"], mid="x", version="2.0.0", kpm_dir=d)
        rep = sc.Reporter(out=None)
        ce.run(mods=[m], reporter=rep)
        self.assertEqual(rep.exit_code(), 0, [str(f) for f in rep.findings])
        self.assertEqual(sevs(rep), ["PASS"])

    def test_kpm_dir_falls_back_to_canonical_with_version_warn(self):
        d = self.root / "Patches" / "Foo"
        self.kpatch(d / "Foo.kpatch", "x", "1.0.0", dll("A"))
        self.kpatch(d / "Other.kpatch", "x", "0.9.0", dll())
        m = mod([hook("A")], ["A"], mid="x", version="2.0.0", kpm_dir=d)
        rep = sc.Reporter(out=None)
        ce.run(mods=[m], reporter=rep)
        self.assertEqual(rep.exit_code(), 0)
        self.assertEqual(sevs(rep, "stale-kpatch"), ["WARN"])

    def test_no_kpatch_is_skip(self):
        m = mod([hook("A")], ["A"], mid="x", kpm_dir=None)
        rep = sc.Reporter(out=None)
        ce.run(mods=[m], reporter=rep)
        self.assertEqual(rep.count("SKIP"), 1)
        self.assertEqual(rep.exit_code(), 0)


class LoaderIntegrationTests(unittest.TestCase):
    def test_run_on_a_temp_repo_layout(self):
        with tempfile.TemporaryDirectory() as t:
            t = Path(t)
            md = t / "repo" / "patch_manager_mods" / "x_src"
            md.mkdir(parents=True)
            (md / "manifest.toml").write_text('[patch]\nid = "x"\nversion = "1.0.0"\n')
            (md / sc.HOOKS_TOML).write_text('[metadata]\ntarget_versions = ["AB"]\n[[hooks]]\naddress = 4198400\n'
                                            'type = "detour"\nfunction = "A"\noriginal_bytes = [144,144,144,144,144]\n'
                                            '[[hooks]]\naddress = 4198500\ntype = "detour"\nfunction = "B"\n'
                                            'original_bytes = [144,144,144,144,144]\n')
            (md / "exports.def").write_text("LIBRARY X\nEXPORTS\n    A ; comment\n")
            kd = t / "kpm" / "Patches" / "X"
            kd.mkdir(parents=True)
            (kd / "manifest.toml").write_text('[patch]\nid = "x"\nversion = "1.0.0"\n')
            with zipfile.ZipFile(kd / "X.kpatch", "w") as z:
                z.writestr("manifest.toml", '[patch]\nid = "x"\nversion = "1.0.0"\n')
                z.writestr("binaries/windows_x86.dll", dll("A", "B"))
            rep = sc.Reporter(out=None)
            ce.run(repo=t / "repo", kpm_dir=t / "kpm", reporter=rep)
            # B is exported by the DLL but missing from exports.def
            self.assertEqual([f.check for f in rep.of("FAIL")], ["def"])
            self.assertIn("B", rep.of("FAIL")[0].msg)


if __name__ == "__main__":
    unittest.main()
