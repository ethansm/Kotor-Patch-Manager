#!/usr/bin/env python3
"""Tests for scripts/npc_add_build_git.py. Run: python3 scripts/run_tests.py npc_add_build_git
Needs pykotor (re-runs under the repo .venv when the system python lacks it). Most tests use a synthetic 204tel-shaped
MOD (17 creatures, 48 waypoints, a few other lists) built in memory, so they run without the game. The real-archive
tests read the live 204tel.mod (read only) and skip when it or the out/override UTCs are absent. CLI tests write only
to a temp dir, never to the repo output or the game dir."""
import contextlib
import hashlib
import io
import os
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

try:
    import pykotor  # noqa: F401
except ImportError:  # scripts/run_tests.py runs test_*.py with the system python: re-run under the repo venv
    _venv = Path(__file__).resolve().parent.parent / ".venv" / "bin" / "python3"
    if _venv.exists() and Path(sys.prefix).resolve() != _venv.parent.parent.resolve():
        os.execv(str(_venv), [str(_venv), __file__] + sys.argv[1:])
    print("SKIP: pykotor not importable and no repo .venv")
    sys.exit(0)

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import _common  # noqa: E402
import npc_add_build_git as B  # noqa: E402
import npc_add_waypoints as wp  # noqa: E402
from pykotor.resource.formats.erf import ERF, ERFType, bytes_erf, read_erf  # noqa: E402
from pykotor.resource.formats.gff import bytes_gff, read_gff  # noqa: E402
from pykotor.resource.formats.gff.gff_data import GFF, GFFContent, GFFList  # noqa: E402
from pykotor.resource.type import ResourceType  # noqa: E402

UTC_STUB = b"UTC V3.2" + b"\0" * 48
CANARY_NAMES = [name for name, _ in B.CANARIES]


def make_git(n_creatures=17, n_waypoints=48, extra_wp_tag=None):
    """Synthetic GIT: baseline list sizes, vanilla-order creature fields, waypoints in the W1d field order."""
    g = GFF(GFFContent.GIT)
    g.root.set_uint8("UseTemplates", 0)
    creatures = GFFList()
    for i in range(n_creatures):
        s = creatures.add(4)
        B._put(s, "XPosition", "Single", -60.0 - i)
        B._put(s, "YPosition", "Single", 20.0 + i)
        B._put(s, "ZPosition", "Single", 0.0)
        B._put(s, "XOrientation", "Single", 1.0)
        B._put(s, "YOrientation", "Single", 0.0)
        B._put(s, "TemplateResRef", "ResRef", f"c_test{i:02d}")
    g.root.set_list("Creature List", creatures)
    waypoints = GFFList()
    for i in range(n_waypoints):
        s = waypoints.add(5)
        tag = extra_wp_tag if (extra_wp_tag and i == n_waypoints - 1) else f"WP_test_{i:02d}"
        values = {"Appearance": 1, "LinkedTo": "", "TemplateResRef": f"wp_test_{i:02d}", "Tag": tag,
                  "LocalizedName": -1, "Description": -1, "HasMapNote": 0, "MapNote": -1, "MapNoteEnabled": 1,
                  "XPosition": float(i), "YPosition": 0.0, "ZPosition": 0.0, "XOrientation": 1.0, "YOrientation": 0.0}
        for label, typ in wp.WAYPOINT_FIELDS:
            B._put(s, label, typ, values[label])
    g.root.set_list("WaypointList", waypoints)
    doors = GFFList()
    B._put(doors.add(8), "TemplateResRef", "ResRef", "door_test")
    g.root.set_list("Door List", doors)
    return g


def make_mod(git=None, extra_utc=()):
    erf = ERF(ERFType.MOD)
    erf.set_data("204tel", ResourceType.GIT, bytes_gff(git if git is not None else make_git()))
    erf.set_data("n_telf01", ResourceType.UTC, UTC_STUB)
    erf.set_data("203_merc003", ResourceType.UTC, UTC_STUB + b"x")
    erf.set_data("a_fixture", ResourceType.NCS, b"NCS V1.0" + b"\x42" * 24)
    for name in extra_utc:
        erf.set_data(name, ResourceType.UTC, UTC_STUB)
    return bytes_erf(erf, ResourceType.MOD)


def utc_blobs():
    return {name: UTC_STUB for name in CANARY_NAMES}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def git_of(data):
    return read_gff(read_erf(data).get("204tel", ResourceType.GIT))


class BuildLayoutTests(unittest.TestCase):
    def test_canary_layout(self):
        self.assertEqual(CANARY_NAMES, ["npcadd_wookm", "npcadd_twilekf", "npcadd_bith", "npcadd_rodian"])
        walker_xy = dict(B.CANARIES)["npcadd_rodian"]
        self.assertEqual(walker_xy, wp.ROUTES["R1"][0])
        for name, (x, y) in B.CANARIES:
            self.assertLessEqual(len(name), 16)
            if name != "npcadd_rodian":
                self.assertLessEqual(x, -66.45, name)

    def test_orientation_is_unit_vector_toward_target(self):
        for name, (x, y) in B.CANARIES:
            ux, uy = B.facing_toward_target(x, y)
            self.assertAlmostEqual(ux * ux + uy * uy, 1.0, places=12, msg=name)
            tx, ty = B.TARGET_XY[0] - x, B.TARGET_XY[1] - y
            self.assertAlmostEqual(ux * ty - uy * tx, 0.0, places=9, msg=name)  # collinear with the target
            self.assertGreater(ux * tx + uy * ty, 0.0, msg=name)               # and pointing at it

    def test_orientation_zero_length_fails_closed(self):
        with self.assertRaises(B.BuildError):
            B.facing_toward_target(*B.TARGET_XY)


class SyntheticBuildTests(unittest.TestCase):
    def setUp(self):
        self.src = make_mod()
        self.utc = utc_blobs()

    def test_appends_creatures_and_waypoints_and_keeps_originals(self):
        out = B.build_mod(self.src, self.utc)
        self.assertEqual(B.verify(self.src, out, self.utc), [])
        erf_out, erf_src = read_erf(out), read_erf(self.src)
        self.assertEqual(len(erf_out), len(erf_src) + 4 + 3)
        git = git_of(out)
        self.assertEqual(len(git.root.get_list("Creature List")), 21)
        self.assertEqual(len(git.root.get_list("WaypointList")), 51)
        for name in CANARY_NAMES:
            self.assertEqual(erf_out.get(name, ResourceType.UTC), UTC_STUB)
        for resref in ("npcwp_rodian_01", "npcwp_rodian_02", "npcwp_rodian_03"):
            self.assertIsNotNone(erf_out.get(resref, ResourceType.UTW))

    def test_other_lists_are_bit_exact(self):
        out = B.build_mod(self.src, self.utc)
        src_root, out_root = git_of(self.src).root, git_of(out).root
        for label in ("Door List", "UseTemplates"):
            self.assertEqual(B.list_signature(src_root.get(label)), B.list_signature(out_root.get(label)), label)

    def test_creature_fields_use_vanilla_order_and_types(self):
        out = B.build_mod(self.src, self.utc)
        new = git_of(out).root.get_list("Creature List").at(17)
        self.assertEqual(new.struct_id, 4)
        self.assertEqual([label for label, _, _ in new], [label for label, _ in B.CREATURE_FIELDS])
        self.assertEqual([ftype.name for _, ftype, _ in new], [typ for _, typ in B.CREATURE_FIELDS])
        self.assertEqual(str(new.get("TemplateResRef")), "npcadd_wookm")
        self.assertNotIn("Tag", [label for label, _, _ in new])
        self.assertNotIn("ObjectId", [label for label, _, _ in new])

    def test_build_is_deterministic(self):
        first = B.build_mod(self.src, self.utc)
        second = B.build_mod(self.src, self.utc)
        self.assertEqual(sha(first), sha(second))

    def test_tampered_creature_is_reported(self):
        out = B.build_mod(self.src, self.utc)
        erf = read_erf(out)
        g = git_of(out)
        lst = g.root.get_list("Creature List")
        lst.at(18).set_single("XPosition", 0.0)
        g.root.set_list("Creature List", lst)
        erf.set_data("204tel", ResourceType.GIT, bytes_gff(g))
        tampered = bytes_erf(erf, ResourceType.MOD)
        problems = B.verify(self.src, tampered, self.utc)
        self.assertTrue(any("npcadd_twilekf" in p for p in problems), problems)

    def test_tampered_original_resource_is_reported(self):
        out = B.build_mod(self.src, self.utc)
        erf = read_erf(out)
        erf.set_data("a_fixture", ResourceType.NCS, b"NCS V1.0" + b"\x00" * 24)
        problems = B.verify(self.src, bytes_erf(erf, ResourceType.MOD), self.utc)
        self.assertTrue(any("a_fixture" in p for p in problems), problems)

    def test_not_a_mod_is_rejected(self):
        with self.assertRaises(B.BuildError):
            B.build_mod(b"not an erf at all", self.utc)

    def test_non_git_archive_is_rejected(self):
        erf = ERF(ERFType.MOD)
        erf.set_data("other", ResourceType.UTC, UTC_STUB)
        with self.assertRaises(B.BuildError):
            B.build_mod(bytes_erf(erf, ResourceType.MOD), self.utc)


class FailClosedTests(unittest.TestCase):
    def test_missing_utc_blob_fails_closed(self):
        src = make_mod()
        blobs = utc_blobs()
        del blobs["npcadd_bith"]
        with self.assertRaises(B.BuildError) as ctx:
            B.build_mod(src, blobs)
        self.assertIn("npcadd_bith", str(ctx.exception))

    def test_load_utc_blobs_fails_on_missing_file(self):
        with tempfile.TemporaryDirectory() as tmp:
            with self.assertRaises(B.BuildError):
                B.load_utc_blobs(Path(tmp))

    def test_load_utc_blobs_rejects_non_utc(self):
        with tempfile.TemporaryDirectory() as tmp:
            for name in CANARY_NAMES:
                (Path(tmp) / f"{name}.utc").write_bytes(b"NOPE" + b"\0" * 60)
            with self.assertRaises(B.BuildError):
                B.load_utc_blobs(Path(tmp))

    def test_baseline_mismatch_fails(self):
        src = make_mod(make_git(n_creatures=16))
        with self.assertRaises(B.BuildError) as ctx:
            B.build_mod(src, utc_blobs())
        self.assertIn("baseline", str(ctx.exception))

    def test_existing_npcadd_utc_is_a_collision(self):
        src = make_mod(extra_utc=("npcadd_bith",))
        with self.assertRaises(B.BuildError):
            B.build_mod(src, utc_blobs())

    def test_waypoint_tag_collision_fails(self):
        src = make_mod(make_git(extra_wp_tag="WP_npcadd_rodian_02"))
        with self.assertRaises(B.BuildError) as ctx:
            B.build_mod(src, utc_blobs())
        self.assertIn("collision", str(ctx.exception))


class CliTests(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="npc_add_git_"))
        self.live = self.tmp / "204tel.mod"
        self.live.write_bytes(make_mod())
        self.override = self.tmp / "override"
        self.override.mkdir()
        for name in CANARY_NAMES:
            (self.override / f"{name}.utc").write_bytes(UTC_STUB)
        self.out = self.tmp / "out" / "204tel.mod"

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def run_cli(self, *args):
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf), contextlib.redirect_stderr(buf):
            code = B.main(["--live-mod", str(self.live), "--override-dir", str(self.override),
                           "--out", str(self.out), *args])
        return code, buf.getvalue()

    def test_write_then_check_round_trip(self):
        code, _ = self.run_cli()
        self.assertEqual(code, 0)
        self.assertTrue(self.out.is_file())
        self.assertEqual(self.run_cli("--check")[0], 0)

    def test_check_flags_drift(self):
        self.assertEqual(self.run_cli()[0], 0)
        self.out.write_bytes(self.out.read_bytes() + b"\0")
        code, text = self.run_cli("--check")
        self.assertEqual(code, 1)
        self.assertIn("DRIFT", text)

    def test_check_flags_missing_output(self):
        code, text = self.run_cli("--check")
        self.assertEqual(code, 1)
        self.assertIn("missing", text)

    def test_refuses_output_under_game_dir(self):
        self.out = _common.GAME_DIR / "Modules" / "npc_add_build_git_must_not_exist.mod"
        code, text = self.run_cli()
        self.assertEqual(code, 2)
        self.assertIn("game directory", text)
        self.assertFalse(self.out.exists())

    def test_missing_override_utc_exits_2(self):
        (self.override / "npcadd_bith.utc").unlink()
        code, text = self.run_cli()
        self.assertEqual(code, 2)
        self.assertIn("npcadd_bith", text)
        self.assertFalse(self.out.exists())


LIVE_MOD = B.LIVE_MOD


def _live_ready():
    return LIVE_MOD.is_file() and all((B.OVERRIDE_DIR / f"{n}.utc").is_file() for n in CANARY_NAMES)


@unittest.skipUnless(_live_ready(), "live 204tel.mod or out/override UTCs absent")
class LiveModTests(unittest.TestCase):
    def test_real_archive_counts_and_contract(self):
        src = LIVE_MOD.read_bytes()
        blobs = B.load_utc_blobs(B.OVERRIDE_DIR)
        out = B.build_mod(src, blobs)
        self.assertEqual(B.verify(src, out, blobs), [])
        erf = read_erf(out)
        self.assertEqual(len(erf), 193)
        git = _git_root(out)
        self.assertEqual(len(git.get_list("Creature List")), 21)
        self.assertEqual(len(git.get_list("WaypointList")), 51)

    def test_real_archive_build_is_deterministic(self):
        src = LIVE_MOD.read_bytes()
        blobs = B.load_utc_blobs(B.OVERRIDE_DIR)
        self.assertEqual(sha(B.build_mod(src, blobs)), sha(B.build_mod(src, blobs)))


def _git_root(data):
    return read_gff(read_erf(data).get("204tel", ResourceType.GIT)).root


if __name__ == "__main__":
    unittest.main(verbosity=1)
