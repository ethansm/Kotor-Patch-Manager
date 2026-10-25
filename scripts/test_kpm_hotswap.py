#!/usr/bin/env python3
"""Host tests for kpm_hotswap.py (run: python3 test_kpm_hotswap.py, cwd=scripts). Tmp dirs only; never touches a real game dir."""
import contextlib
import hashlib
import io
import json
import os
import pathlib
import struct
import subprocess
import sys
import tempfile
import time
import unittest
import zipfile
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import kpm_hotswap as kh  # noqa: E402
REAL_PENDING, REAL_HEAD = kh.git_pending, kh.git_head
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
                  mock.patch.object(kh, "GATE_RECORDS", pathlib.Path(self.tmp.name) / "gate-records"),
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


class NewModMixin:
    def new_mod(self, pid="save-events", src="save_events_src"):
        """A repo mod with no KPM folder, live in the config (no live DLL, so it ends as SWAP)."""
        d = self.e.src.parent / src
        d.mkdir()
        (d / "manifest.toml").write_text(f'[patch]\nid = "{pid}"\nversion = "1.0.0"\n')
        (d / "SaveEvents.cpp").write_text("int z;\n")
        self.e.live[pid] = {"id": pid, "dll": f"patches/{pid}.dll", "hooks": []}
        self.e.write_config()

    def new_build(self, calls):
        def fake_build(kd, kpm):
            calls.append(kd)
            with zipfile.ZipFile(kd / "SaveEvents.kpatch", "w") as z:
                z.writestr("manifest.toml", (kd / "manifest.toml").read_bytes())
                z.writestr("binaries/windows_x86.dll", b"DLL2")
            t = time.time() + 5
            os.utime(kd / "SaveEvents.kpatch", (t, t))
            return 0, ""
        return fake_build


class PrepareTest(NewModMixin, Base):
    # ---- H4: canonical KPM folder creation (helpers in NewModMixin)
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
        self.assertEqual((self.e.kd / "demo.ini").read_text(), "i\n")
        for n in ("demo_test.cpp", "smoke_demo.cpp", "test.toml", "README.md"):
            self.assertFalse((self.e.kd / n).exists(), n)
        self.assertEqual((common.read_bytes(), common.stat().st_mtime), before)
        self.assertTrue(list(self.e.kd.glob("Demo.kpatch.*_pre.bak")))
        self.assertEqual(self.live_dll, b"DLL1")          # --prepare alone never touches the game dir
        self.assertIn("after prepare: demo=SWAP", out)
        self.assertIn("PREPARE SUCCESS: 1 mod(s)", out)
        self.assertIn("Ready for --apply (hot-swap): demo", out)

    def test_pascal_dir(self):
        self.assertEqual(kh.pascal_dir("save-events"), "SaveEvents")
        self.assertEqual(kh.pascal_dir("gui_input"), "GuiInput")
        self.assertEqual(kh.pascal_dir("demo"), "Demo")

    def test_prepare_creates_new_pascal_dir(self):
        self.new_mod()
        calls = []
        with mock.patch.object(kh, "run_create_patch", self.new_build(calls)):
            rc, out = self.run_cli("--mod", "save-events", "--prepare")
        kd = self.e.root / "kpm/Patches/SaveEvents"
        self.assertEqual(rc, 0, out)
        self.assertEqual(calls, [kd])
        self.assertTrue((kd / "manifest.toml").is_file())
        self.assertEqual((kd / "SaveEvents.cpp").read_text(), "int z;\n")
        self.assertIn("created ", out)
        self.assertIn("after prepare: save-events=SWAP", out)

    def test_prepare_reuses_alt_case_dir(self):
        self.new_mod()
        alt = self.e.root / "kpm/Patches/save-events"
        alt.mkdir()
        calls = []
        with mock.patch.object(kh, "run_create_patch", self.new_build(calls)):
            rc, out = self.run_cli("--mod", "save-events", "--prepare")
        self.assertEqual(rc, 0, out)
        self.assertEqual(calls, [alt])
        self.assertTrue((alt / "manifest.toml").is_file())
        self.assertFalse((self.e.root / "kpm/Patches/SaveEvents").exists())

    def test_prepare_refuses_foreign_manifest_dir(self):
        self.new_mod()
        foreign = self.e.root / "kpm/Patches/SaveEvents"
        foreign.mkdir()
        mf = foreign / "manifest.toml"
        mf.write_text('[patch]\nid = "other"\nversion = "1.0.0"\n')
        with mock.patch.object(kh, "run_create_patch", self.new_build([])):
            rc, out = self.run_cli("--mod", "save-events", "--prepare")
        self.assertEqual(rc, 1, out)
        self.assertIn("FAILED save-events:", out)
        self.assertIn("holds patch id 'other'", out)
        self.assertEqual(mf.read_text(), '[patch]\nid = "other"\nversion = "1.0.0"\n')

    def test_prepare_hotswap_duplicate_rc2(self):
        for n in ("SaveEvents", "save-events"):
            d = self.e.root / "kpm/Patches" / n
            d.mkdir()
            (d / "manifest.toml").write_text('[patch]\nid = "save-events"\nversion = "1.0.0"\n')
        rc, out = self.run_cli("--mod", "demo", "--prepare")
        self.assertEqual(rc, 2, out)
        self.assertIn("FATAL: Duplicate", out)

    def _reinstall_build(self):
        def fake_build(kd, kpm):
            self.e.kpatch(b"DLL2", hooks=HOOKS.replace("0x401000", "0x402000"))
            return 0, ""
        return fake_build

    def test_prepare_reinstall_exits_zero_with_summary(self):
        (self.e.src / "Demo.cpp").write_text("int y;\n")
        with mock.patch.object(kh, "run_create_patch", self._reinstall_build()):
            rc, out = self.run_cli("--mod", "demo", "--prepare")
        self.assertEqual(rc, 0, out)
        self.assertIn("PREPARE SUCCESS: 1 mod(s)", out)
        self.assertIn("Requires KPatchLauncher reinstall (hooks changed): demo", out)
        self.assertNotIn("Ready for --apply", out)
        self.assertNotIn("re-run with --apply", out)
        self.assertEqual(self.live_dll, b"DLL1")

    def test_prepare_apply_reinstall_still_refused(self):
        (self.e.src / "Demo.cpp").write_text("int y;\n")
        with mock.patch.object(kh, "run_create_patch", self._reinstall_build()):
            rc, out = self.run_cli("--mod", "demo", "--prepare", "--apply")
        self.assertEqual(rc, 1, out)
        self.assertIn("REINSTALL", out)
        self.assertEqual(self.live_dll, b"DLL1")
        self.assertEqual(self.backups(), [])

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


class CommitKpmTest(NewModMixin, Base):
    """--commit-kpm against a throwaway git repo as the KPM checkout (real git_pending/git_head)."""

    def setUp(self):
        super().setUp()
        self.kpm = self.e.root / "kpm"
        self.git("init", "-q")
        self.git("config", "user.email", "t@example.invalid")
        self.git("config", "user.name", "t")
        self.git("add", "-A")
        self.git("commit", "-q", "-m", "init")
        for name, real in (("git_pending", REAL_PENDING), ("git_head", REAL_HEAD)):
            p = mock.patch.object(kh, name, real)
            p.start()
            self.addCleanup(p.stop)
        self.new_mod()

    def git(self, *args):
        p = subprocess.run(["git", "-C", str(self.kpm), *args], capture_output=True, text=True)
        self.assertEqual(p.returncode, 0, p.stderr)
        return p.stdout

    def commits(self):
        return int(self.git("rev-list", "--count", "HEAD"))

    def prepare(self, *flags):
        with mock.patch.object(kh, "run_create_patch", self.new_build([])):
            return self.run_cli("--mod", "save-events", "--prepare", *flags)

    def test_commit_kpm_commits_new_dir(self):
        rc, out = self.prepare("--commit-kpm")
        self.assertEqual(rc, 0, out)
        self.assertIn("committed", out)
        self.assertTrue(self.git("log", "-1", "--format=%s").startswith("Patches/SaveEvents: save-events 1.0.0 (repo "))
        self.assertNotIn("SaveEvents.cpp", self.git("status", "--porcelain"))
        self.assertNotIn("manifest.toml", self.git("status", "--porcelain"))

    def test_commit_kpm_untracked_files_staged(self):
        rc, out = self.prepare("--commit-kpm")
        self.assertEqual(rc, 0, out)
        stat = self.git("show", "--stat", "--format=", "HEAD")
        self.assertIn("SaveEvents.cpp", stat)
        self.assertIn("manifest.toml", stat)
        self.assertNotIn(".kpatch", stat)

    def test_commit_kpm_clean_is_noop(self):
        self.assertEqual(self.prepare("--commit-kpm")[0], 0)
        n = self.commits()
        rc, out = self.prepare("--commit-kpm")
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.commits(), n)

    def test_commit_kpm_failure_aborts_before_apply(self):
        self.running = False
        real_run = subprocess.run

        def fake_run(cmd, *a, **kw):
            if isinstance(cmd, list) and cmd[:1] == ["git"] and "commit" in cmd:
                return mock.Mock(returncode=1, stdout="", stderr="hook rejected")
            return real_run(cmd, *a, **kw)
        with mock.patch.object(kh, "run_create_patch", self.new_build([])), \
                mock.patch.object(kh.subprocess, "run", fake_run):
            rc, out = self.run_cli("--mod", "save-events", "--prepare", "--apply", "--commit-kpm")
        self.assertEqual(rc, 1, out)
        self.assertIn("FAILED kpm commit", out)
        self.assertEqual(self.backups(), [])
        self.assertFalse((self.e.game / "patches/save-events.dll").exists())

    def test_commit_kpm_requires_prepare(self):
        rc, out = self.run_cli("--mod", "demo", "--apply", "--commit-kpm")
        self.assertEqual(rc, 2, out)
        self.assertIn("--commit-kpm needs --prepare", out)

    def test_prepare_without_flag_never_commits(self):
        n = self.commits()
        rc, out = self.prepare()
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.commits(), n)


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


def pe_variant(base=0x10000000, stamp=0, checksum=0, tail=b"", tail_at=0x500, exp_stamp=0):
    """build_pe() with chosen header fields and bytes written into the (otherwise zero) section tail."""
    b = bytearray(build_pe(["kernel32.dll"], ["A", "B"]))
    e = struct.unpack_from("<I", b, 0x3C)[0]
    struct.pack_into("<I", b, e + 8, stamp)
    struct.pack_into("<I", b, e + 24 + 64, checksum)
    struct.pack_into("<I", b, e + 24 + 28, base)
    struct.pack_into("<I", b, 0x200 + 0x300 + 4, exp_stamp)   # export directory TimeDateStamp
    b[0x200 + tail_at:0x200 + tail_at + len(tail)] = tail
    return bytes(b)


class NormalizedHashTest(unittest.TestCase):
    def test_headers_only_differences_hash_equal(self):
        a = pe_variant(stamp=1000, checksum=0xAAAA, base=0x10000000, exp_stamp=5)
        b = pe_variant(stamp=2000, checksum=0xBBBB, base=0x6B580000, exp_stamp=9)
        self.assertNotEqual(md5(a), md5(b))
        self.assertEqual(kh.normalized_md5(a), kh.normalized_md5(b))

    def test_differences_elsewhere_hash_differently(self):
        a = pe_variant(tail=b"\x01\x02\x03\x04")
        b = pe_variant(tail=b"\x01\x02\x03\x05")
        self.assertNotEqual(kh.normalized_md5(a), kh.normalized_md5(b))
        c = pe_variant(tail=b"\x01\x02\x03\x04")
        self.assertEqual(kh.normalized_md5(a), kh.normalized_md5(c))

    def test_relocated_pointer_is_rebased(self):
        """A HIGHLOW relocation target holding base+X hashes the same under any base."""
        def with_reloc(base):
            b = bytearray(pe_variant(base=base, tail=struct.pack("<I", base + 0x1234)))
            e = struct.unpack_from("<I", b, 0x3C)[0]
            reloc = 0x200 + 0x580   # reloc block inside the section: page rva 0x1000, 1 entry (type 3, offset 0x500)
            struct.pack_into("<II", b, reloc, 0x1000, 12)
            struct.pack_into("<HH", b, reloc + 8, (3 << 12) | 0x500, 0)
            struct.pack_into("<II", b, e + 24 + 96 + 5 * 8, 0x1000 + 0x580, 12)
            return bytes(b)
        a, b = with_reloc(0x10000000), with_reloc(0x6B580000)
        self.assertNotEqual(a, b)
        self.assertEqual(kh.normalized_md5(a), kh.normalized_md5(b))

    def test_non_pe_is_hashed_raw(self):
        self.assertEqual(kh.normalized_md5(b"DLL1"), md5(b"DLL1"))
        self.assertNotEqual(kh.normalized_md5(b"DLL1"), kh.normalized_md5(b"DLL2"))

    def test_rel32_to_fixed_address_accepted_only_for_the_base_delta(self):
        """call rel32 to a fixed (game exe) address: rel differs by the ImageBase difference between two links."""
        a = pe_variant(base=0x10000000, tail=b"\xE8" + struct.pack("<I", 0xFFFF1000))
        delta = 0x20000000 - 0x10000000
        b = pe_variant(base=0x20000000, tail=b"\xE8" + struct.pack("<I", (0xFFFF1000 - delta) & 0xFFFFFFFF))
        self.assertFalse(kh.normalized_md5(a) == kh.normalized_md5(b))
        self.assertTrue(kh.dlls_equivalent(a, b))
        bad = pe_variant(base=0x20000000, tail=b"\xE8" + struct.pack("<I", (0xFFFF1000 - delta + 1) & 0xFFFFFFFF))
        self.assertFalse(kh.dlls_equivalent(a, bad))
        other = pe_variant(base=0x20000000, tail=b"\xE8" + struct.pack("<I", (0xFFFF1000 - delta) & 0xFFFFFFFF) + b"\x07")
        self.assertFalse(kh.dlls_equivalent(a, other))


class GateCrossCheckTest(Base):
    def record(self, **kw):
        rec = {"mod_id": "demo", "md5": md5(b"DLL2"), "dll": "", "head": "deadbee", "time": time.time() + 1000,
               "scratch": ""}
        rec.update(kw)
        d = kh.GATE_RECORDS
        d.mkdir(parents=True, exist_ok=True)
        (d / "demo.json").write_text(json.dumps(rec))

    def check(self):
        return kh.gate_cross_check("demo", self.e.kd, self.e.src)

    def test_missing_record(self):
        self.e.kpatch(b"DLL2")
        self.assertEqual(self.check(), ["NOTE  demo: no gate record"])

    def test_match(self):
        self.e.kpatch(b"DLL2")
        self.record()
        self.assertEqual(self.check(), ["GATE  demo: fresh DLL matches the gated build (gate head deadbee)"])

    def test_mismatch(self):
        self.e.kpatch(b"DLL2")
        self.record(md5=md5(b"OTHER"), dll=str(pathlib.Path(self.tmp.name) / "gone.dll"))
        out = self.check()
        self.assertEqual(len(out), 1)
        self.assertTrue(out[0].startswith("NOTE  demo: gate DLL"), out)   # gate DLL deleted: inconclusive

        g = pathlib.Path(self.tmp.name) / "gate.dll"
        g.write_bytes(b"OTHER")
        self.record(md5=md5(b"OTHER"), dll=str(g))
        self.assertEqual(self.check(), [f"WARN  demo: rebuilt DLL differs from the gated build (gate {md5(b'OTHER')} "
                                        "at deadbee); re-run pre_install_gate"])

    def test_match_through_the_gate_dll_when_only_base_and_stamps_differ(self):
        delta = 0x20000000 - 0x10000000
        gate = pe_variant(base=0x10000000, stamp=1, tail=b"\xE8" + struct.pack("<I", 0xFFFF1000))
        new = pe_variant(base=0x20000000, stamp=2, tail=b"\xE8" + struct.pack("<I", (0xFFFF1000 - delta) & 0xFFFFFFFF))
        g = pathlib.Path(self.tmp.name) / "gate.dll"
        g.write_bytes(gate)
        self.e.kpatch(new)
        self.record(md5=md5(gate), dll=str(g))
        self.assertEqual(self.check(), ["GATE  demo: fresh DLL matches the gated build (gate head deadbee)"])

    def test_stale_record(self):
        self.e.kpatch(b"DLL2")
        self.record(time=time.time() - 3600)
        (self.e.src / "Demo.cpp").write_text("int y;\n")   # newer than the record
        out = self.check()
        self.assertEqual(out[0], "GATE  demo: fresh DLL matches the gated build (gate head deadbee)")
        self.assertEqual(out[1], "WARN  demo: gate record is older than the source (Demo.cpp); stale gate, "
                                 "re-run pre_install_gate")

    def test_iso_time_and_broken_record_never_raise(self):
        self.e.kpatch(b"DLL2")
        self.record(time="2020-01-01T00:00:00")
        (self.e.src / "Demo.cpp").write_text("int y;\n")
        self.assertIn("stale gate", self.check()[1])
        (kh.GATE_RECORDS / "demo.json").write_text("{not json")
        out = self.check()
        self.assertEqual(len(out), 1)
        self.assertTrue(out[0].startswith("NOTE  demo: gate cross-check skipped"), out)

    def test_prepare_prints_the_cross_check_and_stays_exit_0(self):
        (self.e.src / "Demo.cpp").write_text("int y;\n")

        def fake_build(kd, kpm):
            self.e.kpatch(b"DLL2")
            return 0, ""
        self.record(md5=md5(b"OTHER"), time=time.time() + 1000)
        with mock.patch.object(kh, "run_create_patch", fake_build):
            rc, out = self.run_cli("--mod", "demo", "--prepare")
        self.assertEqual(rc, 0, out)
        self.assertIn("NOTE  demo: gate DLL (none recorded) is gone", out)
        (kh.GATE_RECORDS / "demo.json").unlink()
        (self.e.src / "Demo.cpp").write_text("int z;\n")
        with mock.patch.object(kh, "run_create_patch", fake_build):
            rc, out = self.run_cli("--mod", "demo", "--prepare")
        self.assertEqual(rc, 0, out)
        self.assertIn("NOTE  demo: no gate record", out)


class LoadLimitTest(unittest.TestCase):
    def test_create_patch_runs_under_the_exclusive_lock(self):
        events = []

        @contextlib.contextmanager
        def fake_exclusive(name, label=""):
            events.append(("enter", name))
            yield
            events.append(("exit", name))

        def fake_run(cmd, **kw):
            events.append(("run", pathlib.Path(cmd[1]).name))
            return subprocess.CompletedProcess(cmd, 0, "ok\n", "")
        with tempfile.TemporaryDirectory() as t, mock.patch.object(kh.ll, "exclusive", fake_exclusive), \
                mock.patch.object(kh.subprocess, "run", fake_run):
            rc, tail = kh.run_create_patch(pathlib.Path(t), pathlib.Path(t) / "kpm")
        self.assertEqual(rc, 0)
        self.assertEqual(events, [("enter", "create-patch"), ("run", "create-patch.py"), ("exit", "create-patch")])

    def test_main_lowers_priority_once_at_start(self):
        with mock.patch.object(kh.ll, "lower_priority") as lp, contextlib.redirect_stdout(io.StringIO()):
            kh.main(["--list-backups", "--game-dir", tempfile.gettempdir() + "/no_such_game_dir_kh"])
        lp.assert_called_once_with()


if __name__ == "__main__":
    unittest.main()
