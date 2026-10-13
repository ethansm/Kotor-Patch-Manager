"""Tests for kpm_status.py (run: python3 test_kpm_status.py, cwd=scripts)."""
import contextlib
import io
import os
import pathlib
import sys
import tempfile
import time
import unittest
import zipfile
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import kpm_status as ks  # noqa: E402

MANIFEST = '[patch]\nid = "demo"\nversion = "1.0.0"\n'
HOOKS = ('[[hooks]]\naddress = 0x401000\ntype = "detour"\nfunction = "Fn"\noriginal_bytes = [0x8B, 0xEC]\n'
         'exclude_from_restore = []\n\n[[hooks.parameters]]\nsource = "ecx"\ntype = "int"\n')
LIVE_HOOK = {"address": 0x401000, "original_bytes": [0x8B, 0xEC], "type": "detour", "function": "Fn",
             "parameters": [{"source": "ecx", "type": "int"}]}


class Env:
    """repo src dir + KPM Patches/Demo + game dir with one live DLL, all under a temp dir."""

    def __init__(self, root):
        self.root = pathlib.Path(root)
        self.src = self.root / "repo/patch_manager_mods/demo_src"
        self.kd = self.root / "kpm/Patches/Demo"
        self.game = self.root / "game"
        for d in (self.src, self.kd, self.game / "patches"):
            d.mkdir(parents=True)
        for d in (self.src, self.kd):
            (d / "manifest.toml").write_text(MANIFEST)
            (d / "demo.hooks.toml").write_text(HOOKS)
            (d / "Demo.cpp").write_text("int x;\n")
        self.kpatch(b"DLL1")
        (self.game / "patches/demo.dll").write_bytes(b"DLL1")
        self.live = {"demo": {"id": "demo", "dll": "patches/demo.dll", "hooks": [dict(LIVE_HOOK)]}}

    def kpatch(self, dll, manifest=MANIFEST, hooks=HOOKS):
        with zipfile.ZipFile(self.kd / "Demo.kpatch", "w") as z:
            z.writestr("manifest.toml", manifest)
            z.writestr("demo.hooks.toml", hooks)
            z.writestr("binaries/windows_x86.dll", dll)
        t = time.time() + 5   # kpatch newer than the sources
        os.utime(self.kd / "Demo.kpatch", (t, t))

    def run(self):
        return ks.status(self.src / "manifest.toml", ks.kpm_dirs(self.root / "kpm"), self.live, self.game)


class DuplicateGuardTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.e = Env(self.tmp.name)
        self.addCleanup(self.tmp.cleanup)

    def twin(self, name, pid="save-events"):
        d = self.e.root / "kpm/Patches" / name
        d.mkdir()
        (d / "manifest.toml").write_text(f'[patch]\nid = "{pid}"\nversion = "1.0.0"\n')

    def test_duplicate_id_dirs_fatal(self):
        self.twin("SaveEvents")
        self.twin("save-events")
        with self.assertRaises(ks.DuplicateKpmId) as cm:
            ks.kpm_dirs(self.e.root / "kpm")
        self.assertEqual(str(cm.exception), "FATAL: Duplicate KPM patch ID 'save-events' found in directories 'SaveEvents' and 'save-events'")

    def test_same_dir_not_duplicate(self):
        self.assertEqual(ks.kpm_dirs(self.e.root / "kpm"), {"demo": self.e.kd})
        self.twin("Other", pid="other")
        self.assertEqual(set(ks.kpm_dirs(self.e.root / "kpm")), {"demo", "other"})

    def test_main_reports_duplicate_rc2(self):
        self.twin("SaveEvents")
        self.twin("save-events")
        (self.e.game / "patch_config.toml").write_text("")
        err = io.StringIO()
        with contextlib.redirect_stderr(err), contextlib.redirect_stdout(io.StringIO()):
            rc = ks.main(["--game", str(self.e.game), "--kpm", str(self.e.root / "kpm")])
        self.assertEqual(rc, 2)
        self.assertIn("FATAL: Duplicate KPM patch ID", err.getvalue())


class StatusTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.e = Env(self.tmp.name)

    def tearDown(self):
        self.tmp.cleanup()

    def test_live_when_md5_equal(self):
        self.assertEqual(self.e.run()["verdict"], "LIVE")

    def test_swap_when_kpatch_fresh_and_hooks_match(self):
        self.e.kpatch(b"DLL2")
        old = time.time() - 100
        os.utime(self.e.game / "patches/demo.dll", (old, old))
        r = self.e.run()
        self.assertEqual(r["verdict"], "SWAP", r)

    def test_live_other_when_live_dll_newer(self):
        self.e.kpatch(b"DLL2")
        t = time.time() + 50
        os.utime(self.e.game / "patches/demo.dll", (t, t))
        self.assertEqual(self.e.run()["verdict"], "LIVE-OTHER")

    def test_reinstall_when_hooks_differ(self):
        self.e.kpatch(b"DLL2")
        self.e.live["demo"]["hooks"][0]["address"] = 0x402000
        r = self.e.run()
        self.assertEqual(r["verdict"], "REINSTALL", r)
        self.assertIn("1 differ", r["detail"])

    def test_not_live(self):
        self.e.live = {}
        self.assertEqual(self.e.run()["verdict"], "NOT-LIVE")

    def test_mirror_when_repo_differs_from_kpm(self):
        (self.e.src / "Demo.cpp").write_text("int y;\n")
        r = self.e.run()
        self.assertEqual(r["verdict"], "MIRROR")
        self.assertIn("Demo.cpp", r["detail"])

    def test_tests_and_smokes_are_not_mirror_drift(self):
        (self.e.src / "demo_test.cpp").write_text("t\n")
        (self.e.src / "smoke_demo.cpp").write_text("s\n")
        (self.e.src / "test.toml").write_text("x\n")
        self.assertEqual(self.e.run()["verdict"], "LIVE")

    def test_rebuild_when_code_newer_than_kpatch(self):
        t = time.time() + 100
        os.utime(self.e.kd / "Demo.cpp", (t, t))
        self.assertEqual(self.e.run()["verdict"], "REBUILD")

    def test_rebuild_when_kpatch_manifest_content_differs(self):
        self.e.kpatch(b"DLL1", manifest=MANIFEST.replace("1.0.0", "0.9.0"))
        r = self.e.run()
        self.assertEqual(r["verdict"], "REBUILD")
        self.assertIn("manifest.toml(content)", r["detail"])


class HelperTest(unittest.TestCase):
    def test_norm_hooks_drops_installer_defaults(self):
        a = [{"address": 1, "function": "F", "original_bytes": [1], "exclude_from_restore": [], "parameters": [{"source": "ecx", "type": "int"}]}]
        b = [{"address": 1, "function": "F", "original_bytes": [1], "type": "detour", "parameters": [{"source": "ecx"}]}]
        self.assertEqual(ks.norm_hooks(a), ks.norm_hooks(b))

    def test_norm_hooks_order_free_but_field_sensitive(self):
        h1 = {"address": 1, "function": "F", "original_bytes": [1]}
        h2 = {"address": 2, "function": "G", "original_bytes": [2]}
        self.assertEqual(ks.norm_hooks([h1, h2]), ks.norm_hooks([h2, h1]))
        self.assertNotEqual(ks.norm_hooks([h1]), ks.norm_hooks([dict(h1, consumed_exit_address=5)]))

    def test_pick_sources_prefers_dir_matching_kpm_manifest(self):
        with tempfile.TemporaryDirectory() as t:
            root = pathlib.Path(t)
            old, new, kd = root / "old_src", root / "new_src", root / "Patches/X"
            for d in (old, new, kd):
                d.mkdir(parents=True)
            (old / "manifest.toml").write_text('[patch]\nid = "x"\nversion = "1.0.1-probe"\n')
            (new / "manifest.toml").write_text('[patch]\nid = "x"\nversion = "1.0.2"\n')
            (kd / "manifest.toml").write_text('[patch]\nid = "x"\nversion = "1.0.2"\n')
            t2 = time.time() + 50   # the stale dir is newer: the manifest match must still win
            os.utime(old / "manifest.toml", (t2, t2))
            got = ks.pick_sources([old / "manifest.toml", new / "manifest.toml"], {"x": kd})
            self.assertEqual(got, [new / "manifest.toml"])

    def test_pick_sources_prefers_release_dir_over_newer_probe_dir(self):
        # no dir matches the KPM copy: the newer -probe dir must not win over the release dir
        with tempfile.TemporaryDirectory() as t:
            root = pathlib.Path(t)
            probe, release, kd = root / "probe_src", root / "release_src", root / "Patches/X"
            for d in (probe, release, kd):
                d.mkdir(parents=True)
            (probe / "manifest.toml").write_text('[patch]\nid = "x"\nversion = "0.1.0-probe2"\n')
            (release / "manifest.toml").write_text('[patch]\nid = "x"\nversion = "0.1.0"\n')
            (kd / "manifest.toml").write_text('[patch]\nid = "x"\nversion = "1.0.2"\n')
            t2 = time.time() + 50
            os.utime(probe / "manifest.toml", (t2, t2))
            got = ks.pick_sources([probe / "manifest.toml", release / "manifest.toml"], {"x": kd})
            self.assertEqual(got, [release / "manifest.toml"])

    def test_pick_sources_falls_back_to_probe_when_only_probe_dirs(self):
        with tempfile.TemporaryDirectory() as t:
            root = pathlib.Path(t)
            a, b = root / "a_src", root / "b_src"
            for d in (a, b):
                d.mkdir(parents=True)
            (a / "manifest.toml").write_text('[patch]\nid = "x"\nversion = "0.1.0-probe"\n')
            (b / "manifest.toml").write_text('[patch]\nid = "x"\nversion = "0.1.0-probe2"\n')
            t2 = time.time() + 50
            os.utime(b / "manifest.toml", (t2, t2))
            got = ks.pick_sources([a / "manifest.toml", b / "manifest.toml"], {})
            self.assertEqual(got, [b / "manifest.toml"])

    def test_repo_manifests_finds_nested_manifests_at_any_depth(self):
        with tempfile.TemporaryDirectory() as t, mock.patch.object(ks, "MODS", pathlib.Path(t)):
            rels = ["a_src/manifest.toml", "b_src/perflab/manifest.toml", "c_src/probe/manifest.toml", "d_src/x/y/manifest.toml"]
            for rel in rels:
                (ks.MODS / rel).parent.mkdir(parents=True, exist_ok=True)
                (ks.MODS / rel).write_text('[patch]\nid = "x"\n')
            got = [m.relative_to(ks.MODS).as_posix() for m in ks.repo_manifests()]
            self.assertEqual(sorted(got), sorted(rels))

    def test_steam_hooks_name_skips_gog_and_ports(self):
        names = ["manifest.toml", "kotor2-gog-aspyr.hooks.toml", "kotor2-steam-aspyr.hooks.toml", "kotor2-steam-aspyr-linux.hooks.toml"]
        self.assertEqual(ks.steam_hooks_name(names), "kotor2-steam-aspyr.hooks.toml")
        self.assertEqual(ks.steam_hooks_name(["kotor2_steam_aspyr.hooks.toml"]), "kotor2_steam_aspyr.hooks.toml")
        self.assertIsNone(ks.steam_hooks_name(["manifest.toml"]))

    def test_is_production(self):
        for n in ("A.cpp", "A.h", "T.inc", "exports.def", "manifest.toml", "x.hooks.toml"):
            self.assertTrue(ks.is_production(n), n)
        for n in ("a_test.cpp", "test.toml", "smoke_a.cpp", "README.md", "code_patches.toml"):
            self.assertFalse(ks.is_production(n), n)


if __name__ == "__main__":
    unittest.main()
