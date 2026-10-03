#!/usr/bin/env python3
"""Tests for kotor_save_forge.py. Run: .venv/bin/python3 scripts/test_kotor_save_forge.py
Works on a TMPDIR copy of the user's Game2 + QUICKSAVE slots (read-only source); skipped when they are absent.
Never writes to the real saves dir."""
import io
import contextlib
import json
import os
import shutil
import sys
import tempfile
import tomllib
import unittest
from pathlib import Path
from unittest import mock

try:
    import pykotor  # noqa: F401
except ImportError:  # scripts/run_tests.py runs test_*.py with the system python: re-run under the repo venv (pykotor lives there)
    _venv = Path(__file__).resolve().parent.parent / ".venv" / "bin" / "python3"
    if _venv.exists() and Path(sys.executable).resolve() != _venv.resolve():
        os.execv(str(_venv), [str(_venv), __file__] + sys.argv[1:])
    print("SKIP: pykotor not importable and no repo .venv")
    sys.exit(0)

sys.path.insert(0, str(Path(__file__).resolve().parent))
import kotor_save_forge as F  # noqa: E402
from pykotor.resource.formats.erf import read_erf  # noqa: E402
from pykotor.resource.formats.gff import read_gff  # noqa: E402

REAL = F.DEFAULT_SAVES
G2, QS = REAL / "000003 - Game2", REAL / "000000 - QUICKSAVE"
SCRATCH = Path(os.environ.get("TMPDIR", "/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/"
                              "23720339-14ad-4835-8296-de6df180a8f1/scratchpad")) / "m1"
HAVE = G2.is_dir() and QS.is_dir()
SKIP = f"user saves absent ({G2} / {QS})"
CLASSES_2DA = Path("/home/ethansm/Games/KOTOR2_Mod_Workspace/replay/pre_checkpoint_2da/classes.2da")
SCEN = tomllib.loads(F.DEFAULT_SCENARIOS.read_text())["scenario"]


def run(*args):
    out, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        rc = F.main(list(args))
    return rc, out.getvalue(), err.getvalue()


def flat(slot_dir):
    """every resource (outer + nested) -> bytes, keyed by path"""
    d = {}
    for f in Path(slot_dir).iterdir():
        if f.name == F.MANIFEST:
            continue
        d[f.name] = f.read_bytes()
    outer = read_erf(d["SAVEGAME.sav"])
    for r in outer:
        key = f"SAVEGAME.sav::{r.resref}.{r.restype.extension}"
        if r.restype.extension == "sav":
            for n in read_erf(r.data):
                d[f"{key}::{n.resref}.{n.restype.extension}"] = n.data
        else:
            d[key] = r.data
    return {k.lower(): v for k, v in d.items()}


@unittest.skipUnless(HAVE, SKIP)
class ForgeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        shutil.rmtree(SCRATCH, ignore_errors=True)
        cls.saves = SCRATCH / "saves"
        cls.saves.mkdir(parents=True)
        shutil.copytree(G2, cls.saves / G2.name)
        shutil.copytree(QS, cls.saves / QS.name)
        assert not str(cls.saves).startswith(str(REAL))
        cls.src = flat(cls.saves / G2.name)
        cls.patch = mock.patch.object(F, "game_running", return_value=False)
        cls.patch.start()

    @classmethod
    def tearDownClass(cls):
        cls.patch.stop()

    def forge(self, sid, src="Game2", *extra):
        before = {p.name for p in self.saves.iterdir()}
        rc, out, err = run("--saves-dir", str(self.saves), "forge", src, sid, "--write", "--skip-refused", *extra)
        self.assertEqual(rc, 0, out + err)
        new = [p for p in self.saves.iterdir() if p.name not in before]
        self.assertEqual(len(new), 1, out)
        self.assertIn("Steam Cloud may sync this slot until `cleanup`", out)
        return new[0], out

    def creatures(self, slot):
        outer = read_erf((slot / "SAVEGAME.sav").read_bytes())
        res = {}
        for r in outer:
            if r.restype.extension == "utc" and str(r.resref).startswith("availnpc"):
                g = read_gff(r.data).root
                res[g.acquire("Tag", "")] = g
        return outer, res

    def test_classes_match_classes_2da(self):
        if not CLASSES_2DA.exists():
            self.skipTest("classes.2da not found")
        from pykotor.resource.formats.twoda import read_2da
        t = read_2da(CLASSES_2DA.read_bytes())
        for i in range(t.get_height()):
            lab = t.get_cell(i, "label").split("(")[0].lower()
            self.assertEqual(F.class_label(i).lower(), lab, i)

    def test_list_real_dir_qs_guid(self):
        rc, out, _ = run("--saves-dir", str(REAL), "list")
        self.assertEqual(rc, 0)
        line = next(l for l in out.splitlines() if l.startswith("000000 - QUICKSAVE"))
        self.assertIn("GUID gfb7bdbf7", line)

    def test_inspect_runs(self):
        rc, out, _ = run("--saves-dir", str(self.saves), "inspect", "Game2")
        self.assertEqual(rc, 0)
        self.assertIn("atton", out)
        self.assertIn("GUID: none", out)

    def test_dry_run_writes_nothing(self):
        before = sorted(p.name for p in self.saves.iterdir())
        rc, out, _ = run("--saves-dir", str(self.saves), "forge", "Game2", "S1")
        self.assertEqual(rc, 0)
        self.assertIn("DRY RUN", out)
        self.assertEqual(before, sorted(p.name for p in self.saves.iterdir()))

    def test_game_running_refuses(self):
        before = sorted(p.name for p in self.saves.iterdir())
        with mock.patch.object(F, "game_running", return_value=True):
            rc, _, err = run("--saves-dir", str(self.saves), "forge", "Game2", "S1", "--write")
        self.assertEqual(rc, 2)
        self.assertIn("running", err)
        self.assertEqual(before, sorted(p.name for p in self.saves.iterdir()))

    def test_unrefused_refusal_without_skip(self):
        rc, _, err = run("--saves-dir", str(self.saves), "forge", "Game2", "S9")  # g0t0 absent in Game2
        self.assertEqual(rc, 2)
        self.assertIn("g0t0", err)

    def test_every_save_scenario(self):
        for sid, sc in SCEN.items():
            if not sc.get("save"):
                continue
            with self.subTest(sid):
                slot, out = self.forge(sid)
                self.check_scenario(sid, sc, slot, out)

    def check_scenario(self, sid, sc, slot, out):
        save = sc["save"]
        man = json.loads((slot / F.MANIFEST).read_text())
        self.assertEqual(man["scenario"], sid)
        self.assertEqual(man["source_slot"], G2.name)
        nfo = read_gff((slot / "savenfo.res").read_bytes()).root
        self.assertEqual(nfo.acquire("SAVEGAMENAME", ""), f"LUO TEST {sid}")
        after = flat(slot)
        expect_changed = {"savenfo.res"}
        outer, cre = self.creatures(slot)
        pt = read_gff((slot / "PARTYTABLE.res").read_bytes()).root
        # nested git / ifo
        last = "203tel"
        nest = read_erf(next(r for r in outer if str(r.resref) == last and r.restype.extension == "sav").data)
        git = read_gff(next(r for r in nest if r.restype.extension == "git").data).root
        ifo = read_gff(next(r for r in nest if r.restype.extension == "ifo").data).root
        gitc = {c.acquire("Tag", ""): c for c in git.get_list("Creature List")}
        refused_tags = " ".join(man["refused"])
        for ed in save.get("creature", []):
            tag = ed["tag"]
            if tag != "pc" and tag not in cre:
                self.assertIn(tag, refused_tags)
                continue
            targets = [ifo.get_list("Mod_PlayerList").at(0)] if tag == "pc" else [cre[tag]] + ([gitc[tag]] if tag in gitc else [])
            for c in targets:
                if "xp" in ed:
                    self.assertEqual(c.acquire("Experience", 0), ed["xp"], (sid, tag))
                for i, (cn, lvl) in enumerate(ed.get("classes", [])):
                    e = c.get_list("ClassList").at(i)
                    self.assertEqual(e.acquire("Class", -1), F.class_id(cn))
                    self.assertEqual(e.acquire("ClassLevel", 0), lvl)
            if tag == "pc":
                expect_changed |= {"SAVEGAME.sav", f"SAVEGAME.sav::{last}.sav", f"SAVEGAME.sav::{last}.sav::module.ifo"}
            else:
                n = next(r for r in outer if str(r.resref).startswith("availnpc") and
                         read_gff(r.data).root.acquire("Tag", "") == tag).resref
                expect_changed |= {"SAVEGAME.sav", f"SAVEGAME.sav::{n}.utc"}
                if tag in gitc:
                    expect_changed |= {f"SAVEGAME.sav::{last}.sav", f"SAVEGAME.sav::{last}.sav::{last}.git"}
            if ed.get("in_party") and tag in cre:
                n = [r for r in outer if str(r.resref).startswith("availnpc") and read_gff(r.data).root.acquire("Tag", "") == tag][0]
                idx = int(str(n.resref)[8:])
                self.assertIn(idx, [m.acquire("PT_MEMBER_ID", -1) for m in pt.get_list("PT_MEMBERS")])
                self.assertEqual(pt.acquire("PT_NUM_MEMBERS", -1), len(pt.get_list("PT_MEMBERS")))
                expect_changed.add("PARTYTABLE.res")
        if save.get("guid", "keep") != "keep":
            expect_changed.add("GLOBALVARS.res")
            found, guid = F.luo_bits((slot / "GLOBALVARS.res").read_bytes())
            g = save["guid"]
            if g == "none":
                self.assertEqual((found, guid), (0, None))
            elif g.startswith("partial:"):
                self.assertEqual((found, guid), (int(g[8:]), None))
            else:
                self.assertEqual((found, guid), (32, int(g[1:], 16)))
            # other booleans unchanged (by name)
            self.assert_other_globals_equal(self.saves / G2.name, slot)
        expect_changed = {x.lower() for x in expect_changed}
        # untouched byte-identical
        for k, v in self.src.items():
            if k in expect_changed or k.endswith("::module.ifo") and k in expect_changed:
                continue
            self.assertEqual(after[k], v, f"{sid}: untouched resource changed: {k}")
        self.assertEqual(set(after) - set(self.src), set(), "unexpected new resources")
        # manifest md5s match
        for k, c in man["changed_resources"].items():
            if k.lower() in after:
                self.assertEqual(F.md5(after[k.lower()]), c["md5_after"], k)

    def assert_other_globals_equal(self, a, b):
        def d(p):
            r = read_gff((p / "GLOBALVARS.res").read_bytes()).root
            n = [x.acquire("Name", "") for x in r.get_list("CatBoolean")]
            vb = r.acquire("ValBoolean", b"")
            return {nm: (vb[i // 8] >> (7 - i % 8)) & 1 for i, nm in enumerate(n) if not nm.startswith("LUO_G")}, \
                r.acquire("ValNumber", b""), r.get_list("CatNumber").__len__()
        self.assertEqual(d(a), d(b))

    def test_guid_partial_from_qs_keeps_bits(self):
        slot, _ = self.forge("S5", "QUICKSAVE")
        found, guid = F.luo_bits((slot / "GLOBALVARS.res").read_bytes())
        self.assertEqual((found, guid), (16, None))
        r = read_gff((slot / "GLOBALVARS.res").read_bytes()).root
        names = [x.acquire("Name", "") for x in r.get_list("CatBoolean")]
        vb = r.acquire("ValBoolean", b"")
        v = sum(((vb[names.index("LUO_G%02d" % k) // 8] >> (7 - names.index("LUO_G%02d" % k) % 8)) & 1) << k for k in range(16))
        self.assertEqual(v, 0xfb7bdbf7 & 0xFFFF)

    def test_guid_none_from_qs_and_roundtrip_values(self):
        slot, _ = self.forge("S7", "QUICKSAVE")
        self.assertEqual(F.luo_bits((slot / "GLOBALVARS.res").read_bytes()), (0, None))
        self.assert_other_globals_equal(self.saves / QS.name, slot)

    def test_never_overwrites_and_numbers_increase(self):
        a, _ = self.forge("S6")
        b, _ = self.forge("S6")
        self.assertNotEqual(a, b)
        self.assertEqual(int(b.name[:6]), int(a.name[:6]) + 1)
        self.assertTrue((self.saves / G2.name).is_dir())

    def test_cleanup_only_forged(self):
        d = Path(tempfile.mkdtemp(dir=SCRATCH))
        shutil.copytree(G2, d / G2.name)
        shutil.copytree(QS, d / QS.name)
        # a user slot whose name starts LUO TEST but has no manifest must survive; forged slot with other name too
        fake = d / "000004 - fake"
        shutil.copytree(G2, fake)
        nfo = read_gff((fake / "savenfo.res").read_bytes())
        nfo.root.set_string("SAVEGAMENAME", "LUO TEST handmade")
        (fake / "savenfo.res").write_bytes(F.bytes_gff(nfo))
        with mock.patch.object(F, "game_running", return_value=False):
            rc, out, err = run("--saves-dir", str(d), "forge", "Game2", "S6", "--write")
            self.assertEqual(rc, 0, err)
            rc, out, err = run("--saves-dir", str(d), "forge", "Game2", "S7", "--write", "--name", "keepme")
            self.assertEqual(rc, 0, err)
        names = {p.name for p in d.iterdir()}
        # declined
        with mock.patch("builtins.input", return_value="no"):
            self.assertEqual(F.cmd_cleanup(d, False), 1)
        self.assertEqual(names, {p.name for p in d.iterdir()})
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(F.cmd_cleanup(d, True), 0)
        left = {p.name for p in d.iterdir()}
        self.assertEqual(left, names - {"000005 - LUO TEST S6_guid_orphan"})
        self.assertIn("000004 - fake", left)
        self.assertIn(G2.name, left)


if __name__ == "__main__":
    unittest.main(verbosity=2)
