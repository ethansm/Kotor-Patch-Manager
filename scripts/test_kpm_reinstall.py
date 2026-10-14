#!/usr/bin/env python3
"""Tests for kpm_reinstall.py: temp game dir + fake KPM tree; status rows, mod resolution, game check and the launcher are mocked."""
import contextlib
import io
import json
import pathlib
import shutil
import sys
import tempfile
import unittest
import zipfile
from unittest import mock

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import kpm_reinstall as kr  # noqa: E402

HOOK = "[[hooks]]\naddress = {addr}\nfunction = \"f\"\noriginal_bytes = [1, 2]\n"
IDS = ["t", "p1", "p2", "s"]
KP_DLL = {"t": b"t-new", "p1": b"p1-kp", "p2": b"p2-kp"}
LIVE_DLL = {"t": b"t-old", "p1": b"p1-swapped", "p2": b"p2-kp"}  # p1 was hot-swapped away from its kpatch: a peer
LIVE_HOOK = {"t": 100, "p1": 300, "p2": 400}
KP_HOOK = {"t": 200, "p1": 300, "p2": 400}  # t's hooks changed: REINSTALL


def config_text(extra=""):
    out = extra
    for pid in ("t", "p1", "p2"):
        out += f"\n[[patches]]\nid = \"{pid}\"\ndll = \"patches/{pid}.dll\"\n" + HOOK.format(addr=LIVE_HOOK[pid]).replace("[[hooks]]", "[[patches.hooks]]")
    out += "\n[[patches]]\nid = \"s\"\ndll = \"\"\nreplacement_bytes = [144]\n"
    return out


def row(pid, verdict):
    return {"id": pid, "version": "0.1.0", "src": f"patch_manager_mods/{pid}_src", "verdict": verdict,
            "live": "", "kpatch": "", "detail": ""}


class Base(unittest.TestCase):
    def setUp(self):
        tmp = pathlib.Path(tempfile.mkdtemp())
        self.addCleanup(shutil.rmtree, tmp, True)
        self.game, self.kpm = tmp / "game", tmp / "kpm"
        (self.game / "patches").mkdir(parents=True)
        (self.game / "patch_config.toml").write_text(config_text('version = 1\n'))
        (self.game / "kpm_install_state.json").write_text('{"state": "old"}')
        (self.game / "swkotor2.exe").write_bytes(b"exe")
        for pid, data in LIVE_DLL.items():
            (self.game / "patches" / f"{pid}.dll").write_bytes(data)
        (self.kpm / "bin/NoLaunch").mkdir(parents=True)
        (self.kpm / "bin/NoLaunch/KPatchLauncher").write_bytes(b"#!launcher")
        for pid in IDS:
            d = self.kpm / "Patches" / pid.upper()
            d.mkdir(parents=True)
            (d / "manifest.toml").write_text(f"[patch]\nid = \"{pid}\"\n")
            with zipfile.ZipFile(d / f"{pid}.kpatch", "w") as z:
                z.writestr("manifest.toml", f"[patch]\nid = \"{pid}\"\n")
                z.writestr("binaries/windows_x86.dll", KP_DLL.get(pid, b""))
                z.writestr("kotor2-steam-aspyr.hooks.toml", HOOK.format(addr=KP_HOOK[pid]) if pid in KP_HOOK else "")
        self.rows = [row("t", "REINSTALL")]
        self.after = [row("t", "LIVE")]
        self.loads = 0
        self.running = False
        self.launch_rc = 0
        self.drop_key = False
        self.launches = []
        for target, name, new in (
            (kr.hs, "GAME", self.game),
            (kr.hs, "KPM", self.kpm),
            (kr.hs, "is_game_running", lambda: self.running),
            (kr.kpm_ship, "load_rows", self.load_rows),
            (kr.kc, "resolve_mods", self.resolve),
            (kr, "run_launcher", self.fake_launcher),
        ):
            p = mock.patch.object(target, name, new)
            p.start()
            self.addCleanup(p.stop)

    def load_rows(self):
        self.loads += 1
        return self.rows if self.loads == 1 else self.after

    def resolve(self, names):
        dirs = ["t_src", "p1_src", "p2_src", "n_src"]
        out, bad = [], []
        for n in names:
            m = n if n in dirs else n + "_src" if n + "_src" in dirs else None
            (out if m else bad).append(m or n)
        return out, bad, dirs

    def fake_launcher(self, cmd, env):
        """Like the real one: rewrites the config and reverts every DLL to its kpatch build; scripted rc."""
        self.launches.append((cmd, env))
        stage = pathlib.Path(cmd[cmd.index("--patches") + 1])
        for pid in KP_DLL:
            (self.game / "patches" / f"{pid}.dll").write_bytes(zipfile.ZipFile(stage / f"{pid}.kpatch").read("binaries/windows_x86.dll"))
        text = config_text("" if self.drop_key else "version = 1\n").replace("address = 100", "address = 200")
        (self.game / "patch_config.toml").write_text(text)
        (self.game / "kpm_install_state.json").write_text('{"state": "new"}')
        return self.launch_rc

    def run_main(self, *argv):
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            rc = kr.main(list(argv))
        return rc, out.getvalue()

    def snapshot(self):
        files = {str(p.relative_to(self.game)): p.read_bytes() for p in (self.game / "patches").rglob("*") if p.is_file()}
        for n in ("patch_config.toml", "kpm_install_state.json"):
            files[n] = (self.game / n).read_bytes()
        return files

    def backups(self):
        return sorted((self.game / "backups").glob("*_reinstall_*")) if (self.game / "backups").exists() else []


class ReinstallTests(Base):
    def test_dry_run_touches_nothing(self):
        before = self.snapshot()
        rc, out = self.run_main("--mod", "t", "--dry-run")
        self.assertEqual(rc, 0)
        self.assertFalse((self.game / "backups").exists())
        self.assertEqual(self.launches, [])
        self.assertEqual(self.snapshot(), before)
        self.assertIn("KPatchLauncher", out)
        self.assertIn("--patches", out)
        self.assertIn("p1", out)  # the peer that would be restored

    def test_no_args_is_usage_error(self):
        self.assertEqual(self.run_main()[0], 2)
        rc, out = self.run_main("--mod", "nope")
        self.assertEqual(rc, 2)
        self.assertIn("nope", out)
        self.assertEqual(self.launches, [])

    def test_refuses_non_reinstall_target(self):
        for verdict in ("SWAP", "DIRTY"):
            with self.subTest(verdict=verdict):
                self.rows, self.loads = [row("t", verdict)], 0
                rc, out = self.run_main("--mod", "t")
                self.assertEqual(rc, 1)
                self.assertIn(f"REFUSED t: {verdict}", out)
                self.assertEqual(self.launches, [])
                self.assertEqual(self.backups(), [])

    def add_new_id_kpatch(self):
        d = self.kpm / "Patches" / "N"
        d.mkdir(parents=True)
        (d / "manifest.toml").write_text('[patch]\nid = "n"\n')
        with zipfile.ZipFile(d / "n.kpatch", "w") as z:
            z.writestr("manifest.toml", '[patch]\nid = "n"\n')
            z.writestr("binaries/windows_x86.dll", b"n-kp")
            z.writestr("kotor2-steam-aspyr.hooks.toml", "")

    def test_new_id_without_kpatch_refused(self):
        self.rows, self.loads = [row("n", "NOT-LIVE")], 0
        rc, out = self.run_main("--mod", "n")
        self.assertEqual(rc, 1)
        self.assertIn("new id(s) n have no built kpatch", out)
        self.assertEqual(self.launches, [])
        self.assertFalse((self.game / "backups").exists())

    def test_new_id_added_to_launcher_ids(self):
        self.add_new_id_kpatch()
        self.rows, self.after, self.loads = [row("t", "REINSTALL"), row("n", "NOT-LIVE")], [row("t", "LIVE"), row("n", "LIVE")], 0
        rc, out = self.run_main("--mod", "t", "n")
        self.assertEqual(rc, 0, out)
        (cmd, _env), = self.launches
        self.assertEqual(cmd[cmd.index("--patches") + 2:], [*IDS, "n"])  # live ids first, the new id last
        stage = pathlib.Path(cmd[cmd.index("--patches") + 1])
        self.assertIn("n.kpatch", {p.name for p in stage.iterdir()})

    def test_refuses_game_running(self):
        self.running = True
        rc, out = self.run_main("--mod", "t")
        self.assertEqual(rc, 1)
        self.assertIn("running", out)
        self.assertEqual(self.launches, [])
        self.assertFalse((self.game / "backups").exists())

    def test_refuses_missing_kpatch(self):
        (self.kpm / "Patches/P2/p2.kpatch").unlink()
        rc, out = self.run_main("--mod", "t")
        self.assertEqual(rc, 1)
        self.assertIn("p2", out)
        self.assertEqual(self.launches, [])
        self.assertFalse((self.game / "backups").exists())

    def test_refuses_peer_hook_mismatch(self):
        cfg = self.game / "patch_config.toml"
        cfg.write_text(cfg.read_text().replace("address = 400", "address = 999"))
        rc, out = self.run_main("--mod", "t")
        self.assertEqual(rc, 1)
        self.assertIn("peer p2 also has changed hooks", out)
        self.assertEqual(self.launches, [])
        self.assertFalse((self.game / "backups").exists())

    def test_happy_path(self):
        before = self.snapshot()
        rc, out = self.run_main("--mod", "t")
        self.assertEqual(rc, 0, out)
        (backup,) = self.backups()
        self.assertEqual({str(p.relative_to(backup)): p.read_bytes() for p in (backup / "patches").rglob("*") if p.is_file()},
                         {k: v for k, v in before.items() if k.startswith("patches/")})
        self.assertEqual((backup / "patch_config.toml").read_bytes(), before["patch_config.toml"])
        self.assertEqual((backup / "kpm_install_state.json").read_bytes(), before["kpm_install_state.json"])
        meta = json.loads((backup / "reinstall_manifest.json").read_text())
        self.assertEqual(meta["state"], "done")
        self.assertEqual([p["id"] for p in meta["peers"]], ["p1"])
        (cmd, env), = self.launches
        self.assertEqual(env["KPATCH_NO_LAUNCH"], "1")
        self.assertEqual(cmd[-4:], IDS)  # every live id, including the simple byte patch
        stage = pathlib.Path(cmd[cmd.index("--patches") + 1])
        self.assertEqual(sorted(p.name for p in stage.iterdir()), sorted(f"{i}.kpatch" for i in IDS))  # flat
        self.assertEqual((self.game / "patches/p1.dll").read_bytes(), LIVE_DLL["p1"])  # peer restored
        self.assertEqual((self.game / "patches/t.dll").read_bytes(), KP_DLL["t"])  # target keeps the kpatch build
        self.assertIn("rollback: python3 scripts/kpm_reinstall.py --rollback", out)

    def test_launcher_failure_rolls_back(self):
        before = self.snapshot()
        self.launch_rc = 3
        rc, out = self.run_main("--mod", "t")
        self.assertEqual(rc, 1)
        self.assertIn("LAUNCHER FAILED (rc=3)", out)
        self.assertEqual(self.snapshot(), before)
        self.assertEqual(json.loads((self.backups()[0] / "reinstall_manifest.json").read_text())["state"], "rolled_back")

    def test_verify_failure_rolls_back(self):
        before = self.snapshot()
        self.after = [row("t", "REINSTALL")]
        rc, out = self.run_main("--mod", "t")
        self.assertEqual(rc, 1)
        self.assertIn("VERIFY FAILED for t", out)
        self.assertEqual(self.snapshot(), before)

    def test_config_drop_warns(self):
        self.drop_key = True
        rc, out = self.run_main("--mod", "t")
        self.assertEqual(rc, 0, out)
        self.assertIn("WARNING: launcher dropped top-level key 'version'", out)
        self.assertNotIn("version", (self.game / "patch_config.toml").read_text())  # the tool never edits the config


class RollbackTests(Base):
    def corrupt(self):
        (self.game / "patches/p1.dll").write_bytes(b"junk")
        (self.game / "patches/t.dll").write_bytes(b"junk")
        (self.game / "patch_config.toml").write_text("junk = 1")
        (self.game / "kpm_install_state.json").write_text("junk")

    def test_rollback_latest_and_named(self):
        before = self.snapshot()
        self.assertEqual(self.run_main("--mod", "t")[0], 0)
        (backup,) = self.backups()
        for target in ("latest", backup.name, str(backup)):
            with self.subTest(target=target):
                self.corrupt()
                rc, out = self.run_main("--rollback", target)
                self.assertEqual(rc, 0, out)
                self.assertEqual(self.snapshot(), before)

    def test_rollback_accepts_started_state(self):
        before = self.snapshot()
        self.assertEqual(self.run_main("--mod", "t")[0], 0)
        (backup,) = self.backups()
        mf = backup / "reinstall_manifest.json"
        meta = json.loads(mf.read_text())
        meta["state"] = "started"
        mf.write_text(json.dumps(meta))
        self.corrupt()
        self.assertEqual(self.run_main("--rollback", "latest")[0], 0)
        self.assertEqual(self.snapshot(), before)

    def test_rollback_refuses_without_manifest_or_running(self):
        self.assertEqual(self.run_main("--mod", "t")[0], 0)
        (backup,) = self.backups()
        bare = self.game / "backups" / "x_reinstall_1"
        shutil.copytree(backup, bare)
        (bare / "reinstall_manifest.json").unlink()
        self.corrupt()
        rc, out = self.run_main("--rollback", str(bare))
        self.assertEqual(rc, 1)
        self.assertIn("no reinstall_manifest.json", out)
        self.assertEqual((self.game / "patches/t.dll").read_bytes(), b"junk")
        self.running = True
        rc, out = self.run_main("--rollback", str(backup))
        self.assertEqual(rc, 1)
        self.assertIn("running", out)
        self.assertEqual((self.game / "patches/t.dll").read_bytes(), b"junk")

    def test_rollback_missing_dir_refuses(self):
        rc, out = self.run_main("--rollback", "latest")
        self.assertEqual(rc, 1)
        self.assertIn("REFUSED", out)


if __name__ == "__main__":
    unittest.main()
