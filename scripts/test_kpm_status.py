"""Tests for kpm_status.py (run: python3 test_kpm_status.py, cwd=scripts)."""
import os
import pathlib
import sys
import tempfile
import time
import unittest
import zipfile

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
