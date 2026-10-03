import contextlib, io, os, shutil, sys, tempfile, unittest
from unittest import mock

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import luo_scenario as L

FIX = os.path.join(HERE, "testdata", "luo_scenario")
TOML = os.path.join(FIX, "scenarios.toml")
REAL_TOML = os.path.join(HERE, "..", "tests", "luo_scenarios", "saves.toml")


def rd(p):
    with open(p, "rb") as f:
        return f.read()


class T(unittest.TestCase):
    def setUp(self):
        self.d = tempfile.mkdtemp(dir=os.environ.get("TMPDIR"))
        for n in (L.OVR, L.PLY):
            shutil.copy(os.path.join(FIX, n), self.d)
        self.orig = {n: rd(os.path.join(self.d, n)) for n in (L.OVR, L.PLY)}
        p = mock.patch.object(L, "game_running", return_value=[])
        p.start(); self.addCleanup(p.stop)
        self.addCleanup(shutil.rmtree, self.d, True)

    def run_cli(self, *args, toml=TOML):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            rc = L.main(list(args) + ["--game-dir", self.d, "--toml", toml])
        return rc, out.getvalue(), err.getvalue()

    def cur(self, n):
        return rd(os.path.join(self.d, n))

    def test_plan_writes_nothing(self):
        rc, out, _ = self.run_cli("plan", "T_save", "--guid", "gaaaaaaaa")
        self.assertEqual(rc, 0)
        self.assertIn("+mode=log", out)
        self.assertIn("+dev=1", out)   # 1.0.0 reads dev keys only with dev=1
        self.assertIn("+[gaaaaaaaa]", out)
        self.assertEqual({n: self.cur(n) for n in self.orig}, self.orig)
        self.assertFalse(os.path.exists(os.path.join(self.d, L.BACKUP_ROOT)))

    def test_save_requires_guid(self):
        rc, _, err = self.run_cli("plan", "T_save")
        self.assertEqual(rc, 1)
        self.assertIn("--guid", err)
        rc, _, err = self.run_cli("apply", "T_save", "--yes")
        self.assertEqual(rc, 1)
        self.assertEqual(self.cur(L.OVR), self.orig[L.OVR])

    def test_apply_without_yes_does_nothing(self):
        rc, out, _ = self.run_cli("apply", "T_save", "--guid", "gaaaaaaaa")
        self.assertEqual(rc, 2)
        self.assertEqual({n: self.cur(n) for n in self.orig}, self.orig)
        self.assertFalse(os.path.exists(os.path.join(self.d, L.BACKUP_ROOT)))

    def test_apply_restore_roundtrip(self):
        rc, out, err = self.run_cli("apply", "T_save", "--guid", "gaaaaaaaa", "--yes")
        self.assertEqual(rc, 0, err)
        self.assertIn("unsupported dev key 'bogus.key'", err)
        ov = self.cur(L.OVR).decode()
        self.assertIn("mode=log\n", ov); self.assertIn("verbose=1\n", ov)
        self.assertIn("preset.atton=x__y\n", ov); self.assertNotIn("bogus", ov)
        self.assertIn("apply=1\n", ov)          # untouched key
        ply = self.cur(L.PLY).decode()
        self.assertIn("[gaaaaaaaa]\natton=atton__melee_dual_light_early|instant=0|hold=1|pause=3\n", ply)
        rc, out, _ = self.run_cli("status")
        self.assertIn("T_save", out); self.assertIn("matches", out)
        rc, out, _ = self.run_cli("restore", "--yes")
        self.assertEqual(rc, 0)
        self.assertEqual({n: self.cur(n) for n in self.orig}, self.orig)

    def test_unrelated_sections_untouched_and_replace_in_place(self):
        guid = "gfb7bdbf7"
        self.run_cli("apply", "T_save", "--guid", guid, "--yes")
        new = self.cur(L.PLY).decode()
        old = self.orig[L.PLY].decode()
        for keep in ("[g11b3b802]\natton=atton__melee_dual_light_early|instant=1|hold=1\n", "[g22222222]\nkreia=kreia__live|instant=0|hold=1\n", "; hand note\n"):
            self.assertIn(keep, new)
        self.assertEqual(new.count("[" + guid + "]"), 1)
        self.assertEqual(new.count("atton="), 2)
        self.assertEqual(len(new.splitlines()), len(old.splitlines()))

    def test_none_guid_leaves_playthroughs(self):
        self.run_cli("apply", "T_none", "--yes")
        self.assertEqual(self.cur(L.PLY), self.orig[L.PLY])
        self.assertIn(b"apply=0\n", self.cur(L.OVR))
        self.run_cli("restore", "--yes")
        self.assertEqual(self.cur(L.OVR), self.orig[L.OVR])

    def test_refuses_while_game_runs(self):
        with mock.patch.object(L, "game_running", return_value=["1 swkotor2.exe"]):
            rc, _, err = self.run_cli("apply", "T_none", "--yes")
            self.assertEqual(rc, 1); self.assertIn("running", err)
            self.assertEqual(self.cur(L.OVR), self.orig[L.OVR])
        self.run_cli("apply", "T_none", "--yes")
        with mock.patch.object(L, "game_running", return_value=["1 swkotor2.exe"]):
            rc, _, _ = self.run_cli("restore", "--yes")
            self.assertEqual(rc, 1)

    def test_status_detects_drift(self):
        self.run_cli("apply", "T_none", "--yes")
        with open(os.path.join(self.d, L.OVR), "a") as f:
            f.write("mode=off\n")
        _, out, _ = self.run_cli("status")
        self.assertIn("DIFFERS", out)

    def test_missing_files_roundtrip(self):
        os.remove(os.path.join(self.d, L.PLY))
        self.run_cli("apply", "T_save", "--guid", "gaaaaaaaa", "--yes")
        self.assertTrue(os.path.exists(os.path.join(self.d, L.PLY)))
        self.run_cli("restore", "--yes")
        self.assertFalse(os.path.exists(os.path.join(self.d, L.PLY)))

    def test_all_real_scenarios_plan(self):
        import tomllib
        with open(REAL_TOML, "rb") as f:
            ids = tomllib.load(f)["scenario"].keys()
        for sid in ids:
            rc, _, err = self.run_cli("plan", sid, "--guid", "gaaaaaaaa", toml=REAL_TOML)
            self.assertEqual(rc, 0, (sid, err))


if __name__ == "__main__":
    unittest.main()
