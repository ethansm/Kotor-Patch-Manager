#!/usr/bin/env python3
"""Host tests for kpm_hotswap.py (run: python3 test_kpm_hotswap.py, cwd=scripts). Tmp dirs only; never touches a real game dir."""
import contextlib
import hashlib
import io
import json
import os
import pathlib
import struct
import sys
import tempfile
import time
import unittest
import zipfile
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import kpm_hotswap as kh  # noqa: E402
import kpm_status as ks  # noqa: E402
from test_kpm_status import Env, MANIFEST, HOOKS  # noqa: E402



def md5(b):
    return hashlib.md5(b).hexdigest()


class HEnv(Env):
    """test_kpm_status.Env plus: a live config + state file, optional extra kpatch members, a Common dir, a PE table for the readers."""

    def __init__(self, root):
        super().__init__(root)
        self.write_config()
        (self.game / "kpm_install_state.json").write_text('{"v": 1}')
        (self.root / "kpm/Patches/Common").mkdir()
        (self.root / "kpm/Patches/Common/Shared.h").write_text("// shared\n")

    def write_config(self):
        """patch_config.toml on disk from self.live (the dict kpm_status reads)."""
        t = ""
        for p in self.live.values():
            t += f"[[patches]]\nid = '{p['id']}'\ndll = '{p['dll']}'\n"
            for h in p["hooks"]:
                t += (f"[[patches.hooks]]\naddress = {h['address']}\ntype = 'detour'\nfunction = '{h['function']}'\n"
                      f"original_bytes = {list(h['original_bytes'])}\nexclude_from_restore = []\n"
                      "[[patches.hooks.parameters]]\nsource = 'ecx'\ntype = 'int'\n")
        self.config = t
        (self.game / "patch_config.toml").write_text(t)

    def kpatch(self, dll, manifest=MANIFEST, hooks=HOOKS, extra=None):
        with zipfile.ZipFile(self.kd / "Demo.kpatch", "w") as z:
            z.writestr("manifest.toml", manifest)
            z.writestr("kotor2-steam-aspyr.hooks.toml", hooks)
            z.writestr("binaries/windows_x86.dll", dll)
            for n, d in (extra or {}).items():
                z.writestr(n, d)
        t = time.time() + 5
        os.utime(self.kd / "Demo.kpatch", (t, t))

    def swappable(self, dll=b"DLL2", **kw):
        self.kpatch(dll, **kw)
        old = time.time() - 100
        os.utime(self.game / "patches/demo.dll", (old, old))


# imports, exports per fake DLL
PE = {b"DLL1": (["kernel32.dll"], ["A", "B"]), b"DLL2": (["kernel32.dll"], ["A", "B"]),
      b"DLLIMP": (["kernel32.dll", "ws2_32.dll"], ["A", "B"]), b"DLLEXP": (["kernel32.dll"], ["A"]),
      b"DLLEXP2": (["kernel32.dll"], ["A", "B", "C"])}


class Base(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.e = HEnv(self.tmp.name)
        self.running = False
        for p in (mock.patch.object(kh, "MODS", self.e.src.parent),
                  mock.patch.object(ks, "MODS", self.e.src.parent),
                  mock.patch.object(ks, "git_dirty", lambda src: []),
                  mock.patch.object(kh, "is_game_running", lambda: self.running),
                  mock.patch.object(kh, "pe_imports", lambda b: PE[b][0]),
                  mock.patch.object(kh, "pe_exports", lambda b: PE[b][1]),
                  mock.patch.object(kh, "git_head", lambda d: "abc1234"),
                  mock.patch.object(kh, "git_pending", lambda d, p: ([], []))):
            p.start()
            self.addCleanup(p.stop)
        self.addCleanup(self.tmp.cleanup)

    def run_cli(self, *args):
        out = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(out):
            rc = kh.main(["--game-dir", str(self.e.game), "--kpm-dir", str(self.e.root / "kpm"), *args])
        return rc, out.getvalue()

    @property
    def live_dll(self):
        return (self.e.game / "patches/demo.dll").read_bytes()

    def backups(self):
        return sorted((self.e.game / "backups").glob("*_swap_*"))


class RefusalTest(Base):
    def test_refuses_dirty(self):
        self.e.swappable()
        with mock.patch.object(ks, "git_dirty", lambda src: ["Demo.cpp"]):
            rc, out = self.run_cli("--mod", "demo", "--apply")
        self.assertEqual(rc, 1, out)
        self.assertIn("DIRTY", out)
        self.assertIn("commit", out)
        self.assertEqual(self.live_dll, b"DLL1")

    def test_refuses_reinstall(self):
        self.e.swappable()
        self.e.live["demo"]["hooks"][0]["address"] = 0x402000
        self.e.write_config()
        rc, out = self.run_cli("--mod", "demo", "--apply")
        self.assertEqual(rc, 1, out)
        self.assertIn("REINSTALL", out)
        self.assertIn("BUILD_AND_INSTALL.md", out)
        self.assertEqual(self.live_dll, b"DLL1")

    def test_refuses_live_other(self):
        self.e.kpatch(b"DLL2")
        t = time.time() + 50
        os.utime(self.e.game / "patches/demo.dll", (t, t))
        rc, out = self.run_cli("--mod", "demo", "--apply")
        self.assertEqual(rc, 1, out)
        self.assertIn("LIVE-OTHER", out)
        self.assertEqual(self.live_dll, b"DLL1")

    def test_refuses_not_live(self):
        self.e.swappable()
        (self.e.game / "patch_config.toml").write_text("[[patches]]\nid = 'other'\ndll = 'patches/other.dll'\n")
        rc, out = self.run_cli("--mod", "demo", "--apply")
        self.assertEqual(rc, 1, out)
        self.assertIn("NOT-LIVE", out)

    def test_one_bad_mod_refuses_whole_batch_before_any_write(self):
        # second mod would be SWAP; the first is NOT-LIVE: nothing may be swapped
        self.e.swappable()
        (self.e.game / "patch_config.toml").write_text("[[patches]]\nid = 'x'\n")
        rc, _ = self.run_cli("--mod", "demo", "--apply")
        self.assertEqual(rc, 1)
        self.assertEqual(self.live_dll, b"DLL1")
        self.assertEqual(self.backups(), [])

    def test_live_is_nothing_to_do_and_unknown_mod_is_usage_error(self):
        rc, out = self.run_cli("--mod", "demo")
        self.assertEqual(rc, 0)
        self.assertIn("nothing to do", out)
        rc, _ = self.run_cli("--mod", "nope-mod")
        self.assertEqual(rc, 2)
        rc, _ = self.run_cli()
        self.assertEqual(rc, 2)

    def test_dry_run_writes_nothing(self):
        self.e.swappable()
        rc, out = self.run_cli("--mod", "demo")
        self.assertEqual(rc, 0, out)
        self.assertIn("SWAP", out)
        self.assertEqual(self.live_dll, b"DLL1")
        self.assertFalse((self.e.game / "backups").exists())

    def test_game_running_guard(self):
        self.e.swappable()
        self.running = True
        rc, out = self.run_cli("--mod", "demo", "--apply")
        self.assertEqual(rc, 1, out)
        self.assertIn("running", out)
        self.assertEqual(self.live_dll, b"DLL1")
        self.assertEqual(self.backups(), [])
        rc, out = self.run_cli("--rollback", "latest")
        self.assertEqual(rc, 1)


class NestedResolveTest(Base):
    def add_mod(self, rel, pid):
        d = self.e.src.parent / rel
        d.mkdir(parents=True, exist_ok=True)
        (d / "manifest.toml").write_text(f'[patch]\nid = "{pid}"\nversion = "1.0.0"\n')
        return d / "manifest.toml"

    def test_nested_mod_resolves_by_id_reldir_and_parent_name(self):
        want = self.add_mod("perf_lab_src/perflab", "perf-lab")
        for q in ("perf-lab", "perf_lab_src/perflab", "perflab", "PerfLab"):
            self.assertEqual(kh.resolve([q], {}), {"perf-lab": want}, q)

    def test_depth_three_manifest_resolves_by_id(self):
        want = self.add_mod("x_src/a/b", "deep-mod")
        self.assertEqual(kh.resolve(["deep-mod"], {}), {"deep-mod": want})

    def test_probe_dir_resolves_by_relative_dir_among_shared_parent_names(self):
        self.add_mod("other_letterbox_src/probe", "other-probe")
        want = self.add_mod("dialogue_letterbox_fix_src/probe", "letterbox-probe")
        self.assertEqual(kh.resolve(["dialogue_letterbox_fix_src/probe"], {}), {"letterbox-probe": want})

    def test_typo_gets_did_you_mean(self):
        rc, out = self.run_cli("--mod", "demoo")
        self.assertEqual(rc, 2, out)
        self.assertIn("unknown mod 'demoo'", out)
        self.assertIn("Did you mean: 'demo'?", out)
        self.assertIn("Available mod IDs: demo", out)

    def test_nonsense_name_has_no_suggestion(self):
        rc, out = self.run_cli("--mod", "nope-mod")
        self.assertEqual(rc, 2, out)
        self.assertNotIn("Did you mean", out)
        self.assertIn("Available mod IDs", out)

    def test_hyphen_underscore_typo_suggests_real_id(self):
        self.add_mod("action_hotbar_src", "action-hotbar")
        rc, out = self.run_cli("--mod", "action_hotbr")
        self.assertEqual(rc, 2, out)
        self.assertIn("Did you mean: 'action-hotbar'?", out)


class PrepareTest(Base):
    def test_prepare_mirrors_only_production_files_and_never_touches_common(self):
        (self.e.src / "Demo.cpp").write_text("int y;\n")
        (self.e.src / "demo_test.cpp").write_text("t\n")
        (self.e.src / "smoke_demo.cpp").write_text("s\n")
        (self.e.src / "test.toml").write_text("x\n")
        (self.e.src / "README.md").write_text("doc\n")
        (self.e.src / "demo.ini").write_text("i\n")
        common = self.e.root / "kpm/Patches/Common/Shared.h"
        before = (common.read_bytes(), common.stat().st_mtime)
        calls = []

        def fake_build(kd, kpm):
            calls.append(kd)
            self.e.kpatch(b"DLL2")
            return 0, ""
        with mock.patch.object(kh, "run_create_patch", fake_build):
            rc, out = self.run_cli("--mod", "demo", "--prepare")
        self.assertEqual(rc, 0, out)
        self.assertEqual(calls, [self.e.kd])
        self.assertEqual((self.e.kd / "Demo.cpp").read_text(), "int y;\n")
        for n in ("demo_test.cpp", "smoke_demo.cpp", "test.toml", "README.md", "demo.ini"):
            self.assertFalse((self.e.kd / n).exists(), n)
        self.assertEqual((common.read_bytes(), common.stat().st_mtime), before)
        self.assertTrue(list(self.e.kd.glob("Demo.kpatch.*_pre.bak")))
        self.assertEqual(self.live_dll, b"DLL1")          # --prepare alone never touches the game dir
        self.assertIn("after prepare: demo=SWAP", out)

    def test_apply_after_mirror_needs_prepare(self):
        (self.e.src / "Demo.cpp").write_text("int y;\n")
        rc, out = self.run_cli("--mod", "demo", "--apply")
        self.assertEqual(rc, 1, out)
        self.assertIn("--prepare", out)
        self.assertEqual((self.e.kd / "Demo.cpp").read_text(), "int x;\n")

    def test_prepare_then_apply_in_one_run(self):
        (self.e.src / "Demo.cpp").write_text("int y;\n")

        def fake_build(kd, kpm):
            self.e.kpatch(b"DLL2")
            old = time.time() - 100
            os.utime(self.e.game / "patches/demo.dll", (old, old))
            return 0, ""
        with mock.patch.object(kh, "run_create_patch", fake_build):
            rc, out = self.run_cli("--mod", "demo", "--prepare", "--apply")
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.live_dll, b"DLL2")

    def test_build_failure_keeps_old_kpatch_and_fails(self):
        (self.e.src / "Demo.cpp").write_text("int y;\n")
        with mock.patch.object(kh, "run_create_patch", lambda kd, kpm: (1, "boom")):
            rc, out = self.run_cli("--mod", "demo", "--prepare")
        self.assertEqual(rc, 1)
        self.assertIn("boom", out)

    def test_create_patch_invocation_uses_nonempty_db_symlinks(self):
        db = self.e.root / "kpm/AddressDatabases"
        db.mkdir()
        (db / "good.db").write_bytes(b"x")
        (db / "empty.db").write_bytes(b"")
        seen = {}

        def fake_run(cmd, cwd=None, env=None, **kw):
            d = pathlib.Path(env["KPATCH_ADDRESS_DATABASES"])
            seen.update(cmd=cmd, cwd=cwd, dbs=sorted(p.name for p in d.iterdir()), link=(d / "good.db").is_symlink())
            return mock.Mock(returncode=0, stdout="ok", stderr="")
        with mock.patch.object(kh.subprocess, "run", fake_run):
            rc, _ = kh.run_create_patch(self.e.kd, self.e.root / "kpm")
        self.assertEqual(rc, 0)
        self.assertEqual(seen["dbs"], ["good.db"])
        self.assertTrue(seen["link"])
        self.assertEqual(seen["cwd"], self.e.kd)
        self.assertTrue(seen["cmd"][1].endswith("Patches/create-patch.py"))
        self.assertNotIn("--sync", " ".join(seen["cmd"]))


class SwapTest(Base):
    def test_swap_result_md5_equals_kpatch_dll_md5(self):
        self.e.swappable()
        cfg = md5((self.e.game / "patch_config.toml").read_bytes())
        rc, out = self.run_cli("--mod", "demo", "--apply")
        self.assertEqual(rc, 0, out)
        self.assertEqual(md5(self.live_dll), md5(b"DLL2"))
        self.assertEqual(md5((self.e.game / "patch_config.toml").read_bytes()), cfg)
        (bk,) = self.backups()
        self.assertEqual((bk / "demo.dll").read_bytes(), b"DLL1")
        self.assertTrue((bk / "patch_config.toml").is_file() and (bk / "kpm_install_state.json").is_file())
        meta = json.loads((bk / "swap_manifest.json").read_text())
        self.assertEqual(meta["mods"]["demo"]["new_md5"], md5(b"DLL2"))
        self.assertEqual(meta["mods"]["demo"]["old_md5"], md5(b"DLL1"))
        self.assertEqual(meta["config_md5"]["patch_config.toml"], cfg)
        self.assertEqual((meta["repo_head"], meta["kpm_head"]), ("abc1234", "abc1234"))
        self.assertIn("--rollback " + bk.name, out)
        self.assertIn("Commit commands", out)
        self.assertEqual(ks.status(self.e.src / "manifest.toml", ks.kpm_dirs(self.e.root / "kpm"), self.e.live, self.e.game)["verdict"], "LIVE")

    def test_batch_is_atomic_mid_batch_failure_restores_first_mod(self):
        self.e.swappable()
        # second mod "demo2": clone of the first under another id
        src2 = self.e.src.parent / "demo2_src"
        kd2 = self.e.root / "kpm/Patches/Demo2"
        for d in (src2, kd2):
            d.mkdir()
            (d / "manifest.toml").write_text(MANIFEST.replace('"demo"', '"demo2"'))
            (d / "demo2.hooks.toml").write_text(HOOKS)
            (d / "Demo.cpp").write_text("int x;\n")
        with zipfile.ZipFile(kd2 / "Demo2.kpatch", "w") as z:
            z.writestr("manifest.toml", MANIFEST.replace('"demo"', '"demo2"'))
            z.writestr("demo2.hooks.toml", HOOKS)
            z.writestr("binaries/windows_x86.dll", b"DLL2")
        t = time.time() + 5
        os.utime(kd2 / "Demo2.kpatch", (t, t))
        (self.e.game / "patches/demo2.dll").write_bytes(b"DLL1")
        old = time.time() - 100
        os.utime(self.e.game / "patches/demo2.dll", (old, old))
        self.e.live["demo2"] = {"id": "demo2", "dll": "patches/demo2.dll", "hooks": [dict(self.e.live["demo"]["hooks"][0])]}
        self.e.write_config()
        real_write = kh.write_dll
        n = []

        def flaky(dst, data):
            n.append(dst.name)
            if dst.name == "demo2.dll" and data == b"DLL2":
                raise OSError("disk full")
            real_write(dst, data)

        with mock.patch.object(kh, "write_dll", flaky):
            rc, out = self.run_cli("--mod", "demo", "demo2", "--apply")
        self.assertEqual(rc, 1, out)
        self.assertIn("FAILED", out)
        self.assertEqual(n[:2], ["demo.dll", "demo2.dll"])
        self.assertEqual(self.live_dll, b"DLL1")                       # first mod restored
        self.assertEqual((self.e.game / "patches/demo2.dll").read_bytes(), b"DLL1")

    def test_config_md5_verified_after_swap(self):
        self.e.swappable()
        real_write = kh.write_dll

        def tamper(dst, data):
            real_write(dst, data)
            if dst.name == "demo.dll" and data == b"DLL2":
                (self.e.game / "patch_config.toml").write_text(self.e.config + "# touched\n")
        with mock.patch.object(kh, "write_dll", tamper):
            rc, out = self.run_cli("--mod", "demo", "--apply")
        self.assertEqual(rc, 1, out)
        self.assertIn("patch_config.toml changed during the swap", out)
        self.assertEqual(self.live_dll, b"DLL1")

    def test_multi_target_kpatch_is_not_repacked(self):
        self.e.swappable(extra={"kotor2-gog-aspyr.hooks.toml": HOOKS, "binaries/linux_x86.so": b"ELF"})
        kp = self.e.kd / "Demo.kpatch"
        before = kp.read_bytes()
        rc, out = self.run_cli("--mod", "demo", "--apply")
        self.assertEqual(rc, 0, out)
        self.assertEqual(kp.read_bytes(), before)
        with zipfile.ZipFile(kp) as z:
            self.assertIn("kotor2-gog-aspyr.hooks.toml", z.namelist())
            self.assertIn("binaries/linux_x86.so", z.namelist())
        self.assertEqual(self.live_dll, b"DLL2")

    def test_new_import_dll_refuses(self):
        self.e.swappable(b"DLLIMP")
        rc, out = self.run_cli("--mod", "demo", "--apply")
        self.assertEqual(rc, 1, out)
        self.assertIn("ws2_32.dll", out)
        self.assertEqual(self.live_dll, b"DLL1")
        self.assertEqual(self.backups(), [])

    def test_exports_must_equal_live_exports(self):
        for dll in (b"DLLEXP", b"DLLEXP2"):                 # one fewer, one more
            self.e.swappable(dll)
            rc, out = self.run_cli("--mod", "demo", "--apply")
            self.assertEqual(rc, 1, out)
            self.assertIn("exports differ", out)
            self.assertEqual(self.live_dll, b"DLL1")


class RollbackTest(Base):
    def swapped(self):
        self.e.swappable()
        rc, out = self.run_cli("--mod", "demo", "--apply")
        self.assertEqual(rc, 0, out)
        (bk,) = self.backups()
        return bk

    def test_rollback_restores_dlls_only(self):
        bk = self.swapped()
        kp = (self.e.kd / "Demo.kpatch").read_bytes()
        src = (self.e.src / "Demo.cpp").read_bytes()
        rc, out = self.run_cli("--rollback", bk.name)
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.live_dll, b"DLL1")
        self.assertEqual((self.e.kd / "Demo.kpatch").read_bytes(), kp)     # kpatch/KPM/repo untouched
        self.assertEqual((self.e.src / "Demo.cpp").read_bytes(), src)
        self.assertEqual((self.e.game / "patch_config.toml").read_text(), self.e.config)

    def test_rollback_with_changed_config_refuses_config_restore(self):
        bk = self.swapped()
        changed = self.e.config + "[[patches]]\nid = 'new'\n"
        (self.e.game / "patch_config.toml").write_text(changed)
        rc, out = self.run_cli("--rollback", str(bk))
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.live_dll, b"DLL1")                           # DLL part still done
        self.assertEqual((self.e.game / "patch_config.toml").read_text(), changed)
        self.assertIn("config NOT restored", out)

    def test_latest_picks_newest_swap_dir_by_mtime(self):
        b = self.e.game / "backups"
        names = ["a_swap_20260101_000000", "z_swap_20250101_000000", "m_swap_20260202_000000", "other_install_1"]
        for i, n in enumerate(names):
            d = b / n
            d.mkdir(parents=True)
            t = 1_000_000 + [30, 10, 20, 99][i]
            os.utime(d, (t, t))
        self.assertEqual(kh.swap_dirs(self.e.game)[0].name, "a_swap_20260101_000000")
        self.assertNotIn("other_install_1", [d.name for d in kh.swap_dirs(self.e.game)])
        # and rollback latest uses it: a dir without manifest is refused by name
        rc, out = self.run_cli("--rollback", "latest")
        self.assertEqual(rc, 1)
        self.assertIn("a_swap_20260101_000000", out)

    def test_rollback_latest_end_to_end_and_list(self):
        bk = self.swapped()
        rc, out = self.run_cli("--list-backups")
        self.assertIn(bk.name, out)
        rc, out = self.run_cli("--rollback", "latest")
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.live_dll, b"DLL1")


def build_pe(imports, exports):
    """Minimal PE32 with one section holding an import table and an export name table."""
    base, rva, raw = 0x10000000, 0x1000, 0x200
    sec = bytearray(0x600)
    off = 0
    imp_at = 0x000
    names_at = 0x100
    for i, n in enumerate(imports):
        o = imp_at + 20 * i
        struct.pack_into("<I", sec, o + 12, rva + names_at + off)
        sec[names_at + off:names_at + off + len(n) + 1] = n.encode() + b"\0"
        off += len(n) + 1
    exp_at, nametab_at, strs_at = 0x300, 0x340, 0x380
    struct.pack_into("<I", sec, exp_at + 24, len(exports))
    struct.pack_into("<I", sec, exp_at + 32, rva + nametab_at)
    so = 0
    for i, n in enumerate(exports):
        struct.pack_into("<I", sec, nametab_at + 4 * i, rva + strs_at + so)
        sec[strs_at + so:strs_at + so + len(n) + 1] = n.encode() + b"\0"
        so += len(n) + 1
    pe_off = 0x40
    hdr = bytearray(raw)
    hdr[0:2] = b"MZ"
    struct.pack_into("<I", hdr, 0x3C, pe_off)
    hdr[pe_off:pe_off + 4] = b"PE\0\0"
    opt = pe_off + 24
    struct.pack_into("<HHIIIHH", hdr, pe_off + 4, 0x14C, 1, 0, 0, 0, 224, 0x2102)
    struct.pack_into("<H", hdr, opt, 0x10B)
    struct.pack_into("<I", hdr, opt + 28, base)
    struct.pack_into("<I", hdr, opt + 92, 16)
    if exports:
        struct.pack_into("<II", hdr, opt + 96, rva + exp_at, 40)
    if imports:
        struct.pack_into("<II", hdr, opt + 96 + 8, rva + imp_at, 20 * (len(imports) + 1))
    so_ = opt + 224
    hdr[so_:so_ + 8] = b".rdata\0\0"
    struct.pack_into("<IIII", hdr, so_ + 8, len(sec), rva, len(sec), raw)
    return bytes(hdr) + bytes(sec)


class PeReaderTest(unittest.TestCase):
    def test_real_readers_on_synthetic_pe(self):
        dll = build_pe(["KERNEL32.dll", "user32.dll"], ["GetFoo", "Bar"])
        self.assertEqual(kh.pe_imports(dll), ["kernel32.dll", "user32.dll"])
        self.assertEqual(kh.pe_exports(dll), ["Bar", "GetFoo"])
        self.assertEqual(kh.pe_imports(build_pe([], [])), [])
        self.assertEqual(kh.check_new_dll("x", dll, dll), "")
        self.assertIn("new DLL dependency", kh.check_new_dll("x", build_pe(["kernel32.dll", "user32.dll", "ws2_32.dll"], ["GetFoo", "Bar"]), dll))
        self.assertIn("exports differ", kh.check_new_dll("x", build_pe(["kernel32.dll"], ["GetFoo"]), dll))
        self.assertIn("cannot parse", kh.check_new_dll("x", b"not a pe", dll))


if __name__ == "__main__":
    unittest.main()
