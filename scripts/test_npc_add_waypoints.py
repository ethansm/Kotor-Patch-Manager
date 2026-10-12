#!/usr/bin/env python3
"""Tests for scripts/npc_add_waypoints.py. Pure stdlib: no pykotor, no game dir, no file writes. The 48 existing
204TEL waypoint tags are a fixture read once from the live 204tel.mod GIT (WaypointList)."""
import math
import struct
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import npc_add_waypoints as naw  # noqa: E402

EXISTING_204TEL_TAGS = (
    "from_203TEL", "wp_moza_office_02", "wp_moza_office", "wp_moza_office_01", "wp_moza", "FROM_201TEL_ITHM1",
    "wp_mercl_01", "wp_mercbg1_01", "WP_DRDITH1", "wp_mercbg2_01", "WP_DRDITH", "wp_leavingithorian",
    "wp_moza_attack_01", "WP_com_ith_07", "WP_com_ith_10", "WP_com_ith_01", "WP_com_ith_05", "WP_com_ith_04",
    "WP_com_ith_03", "WP_com_ith_02", "WP_com_ith_08", "WP_com_ith_06", "WP_com_ith_09", "WP_com_ith_11",
    "WP_22_09", "WP_22_08", "WP_22_06", "WP_22_07", "WP_22_04", "WP_22_05", "WP_22_03", "WP_22_02", "WP_22_01",
    "WP_from_term", "WP_party_0", "WP_ith_comp", "WP_ith_facing", "WP_party_1", "WP_party_2", "WP_tsf_spawn",
    "WP_tsf_run", "mn_infoterm", "mn_to082", "mn_vivarium", "mn_ithorian", "mn_medical", "mn_labstation",
    "wp_transabort",
)


def read_utw(data):
    """Minimal stdlib GFF reader for a 'UTW ' root struct. Returns (fields dict, total size from header)."""
    assert data[:4] == b"UTW ", data[:4]
    assert data[4:8] == b"V3.2", data[4:8]
    (s_off, s_cnt, f_off, f_cnt, l_off, l_cnt, d_off, d_cnt,
     i_off, i_cnt, li_off, li_cnt) = struct.unpack_from("<12I", data, 8)
    assert (s_off, s_cnt) == (56, 1)
    _, dod, nfields = struct.unpack_from("<iII", data, s_off)
    assert nfields == 2, nfields
    fields = {}
    for k in range(nfields):
        (idx,) = struct.unpack_from("<I", data, i_off + dod + 4 * k)
        ftype, label_idx, fdod = struct.unpack_from("<III", data, f_off + 12 * idx)
        name = data[l_off + 16 * label_idx: l_off + 16 * label_idx + 16].rstrip(b"\0").decode("ascii")
        if ftype == 10:
            (n,) = struct.unpack_from("<I", data, d_off + fdod)
            fields[name] = data[d_off + fdod + 4: d_off + fdod + 4 + n].decode("ascii")
        elif ftype == 11:
            n = data[d_off + fdod]
            fields[name] = data[d_off + fdod + 1: d_off + fdod + 1 + n].decode("ascii")
        else:
            raise AssertionError(f"unexpected field type {ftype} for {name}")
    assert li_cnt == 0 and li_off + li_cnt == len(data), (li_off, li_cnt, len(data))
    return fields, len(data)


def unit(dx, dy):
    n = math.hypot(dx, dy)
    return dx / n, dy / n


class RouteConstantsTests(unittest.TestCase):
    def test_three_routes_with_three_points_each(self):
        self.assertEqual(sorted(naw.ROUTES), ["R1", "R2", "R3"])
        for name, pts in naw.ROUTES.items():
            self.assertEqual(len(pts), 3, name)

    def test_route_coordinates_match_walkmesh_recon(self):
        self.assertEqual(naw.ROUTES["R1"], ((-65.70, 20.05), (-70.45, 26.30), (-66.20, 32.05)))
        self.assertEqual(naw.ROUTES["R2"], ((-73.45, 20.30), (-65.70, 21.05), (-70.20, 27.55)))
        self.assertEqual(naw.ROUTES["R3"], ((-68.70, 19.30), (-75.45, 22.55), (-73.70, 30.30)))


class BuildRouteTests(unittest.TestCase):
    def setUp(self):
        self.utw, self.structs = naw.build_route("npcadd_rodian", naw.ROUTES["R1"])

    def test_canary_names_and_counts(self):
        self.assertEqual(sorted(self.utw), ["npcwp_rodian_01", "npcwp_rodian_02", "npcwp_rodian_03"])
        self.assertEqual([s["Tag"] for s in self.structs],
                         ["WP_npcadd_rodian_01", "WP_npcadd_rodian_02", "WP_npcadd_rodian_03"])
        for resref in self.utw:
            self.assertLessEqual(len(resref), naw.MAX_RESREF)
            self.assertEqual(resref, resref.lower())

    def test_struct_field_order_and_fixed_values(self):
        for s in self.structs:
            self.assertEqual(list(s), [name for name, _ in naw.WAYPOINT_FIELDS])
            self.assertEqual(s["Appearance"], 1)
            self.assertEqual(s["LinkedTo"], "")
            self.assertEqual(s["LocalizedName"], -1)
            self.assertEqual(s["Description"], -1)
            self.assertEqual(s["HasMapNote"], 0)
            self.assertEqual(s["MapNote"], -1)
            self.assertEqual(s["MapNoteEnabled"], 1)
            self.assertEqual(s["ZPosition"], 0.0)
            self.assertIn(s["TemplateResRef"], self.utw)

    def test_positions_are_route_points_stored_as_single(self):
        for s, (x, y) in zip(self.structs, naw.ROUTES["R1"]):
            self.assertAlmostEqual(s["XPosition"], x, places=4)
            self.assertAlmostEqual(s["YPosition"], y, places=4)
            self.assertEqual(s["XPosition"], struct.unpack("<f", struct.pack("<f", s["XPosition"]))[0])
            self.assertEqual(s["YPosition"], struct.unpack("<f", struct.pack("<f", s["YPosition"]))[0])

    def test_orientation_points_to_next_and_last_to_previous(self):
        pts = naw.ROUTES["R1"]
        expected = [
            unit(pts[1][0] - pts[0][0], pts[1][1] - pts[0][1]),
            unit(pts[2][0] - pts[1][0], pts[2][1] - pts[1][1]),
            unit(pts[1][0] - pts[2][0], pts[1][1] - pts[2][1]),
        ]
        for s, (ex, ey) in zip(self.structs, expected):
            self.assertAlmostEqual(s["XOrientation"], ex, places=6)
            self.assertAlmostEqual(s["YOrientation"], ey, places=6)
            self.assertAlmostEqual(math.hypot(s["XOrientation"], s["YOrientation"]), 1.0, places=6)

    def test_all_routes_build_with_contiguous_numbering(self):
        for name, pts in naw.ROUTES.items():
            utw, structs = naw.build_route("npcadd_wookiee", pts)
            self.assertEqual(sorted(utw), [f"npcwp_wookiee_0{i}" for i in (1, 2, 3)], name)
            self.assertEqual([s["Tag"] for s in structs],
                             [f"WP_npcadd_wookiee_0{i}" for i in (1, 2, 3)], name)

    def test_utw_holds_only_tag_and_template_resref(self):
        for resref, data in self.utw.items():
            fields, _ = read_utw(data)
            self.assertEqual(sorted(fields), ["Tag", "TemplateResRef"])
            self.assertEqual(fields["TemplateResRef"], resref)
            self.assertEqual(fields["Tag"], "WP_npcadd_rodian_" + resref[-2:])
        _, size = read_utw(self.utw["npcwp_rodian_01"])
        self.assertEqual(size, 171)

    def test_long_slug_gets_seven_char_key(self):
        utw, structs = naw.build_route("npcadd_abcdefgh", naw.ROUTES["R2"])
        for resref in utw:
            self.assertTrue(resref.startswith("npcwp_abcd"), resref)
            self.assertEqual(len(resref), naw.MAX_RESREF)
        self.assertEqual(structs[0]["Tag"], "WP_npcadd_abcdefgh_01")

    def test_seven_char_slug_fits_resref_limit(self):
        utw, _ = naw.build_route("npcadd_wookiee", naw.ROUTES["R1"])
        self.assertIn("npcwp_wookiee_01", utw)
        for resref in utw:
            self.assertLessEqual(len(resref), naw.MAX_RESREF)


class CollisionTests(unittest.TestCase):
    def test_existing_204tel_tags_do_not_collide(self):
        self.assertEqual(len(EXISTING_204TEL_TAGS), 48)
        utw, structs = naw.build_route("npcadd_rodian", naw.ROUTES["R1"],
                                       existing_tags=EXISTING_204TEL_TAGS)
        self.assertEqual(len(structs), 3)

    def test_tag_collision_is_case_insensitive(self):
        with self.assertRaises(ValueError):
            naw.build_route("npcadd_rodian", naw.ROUTES["R1"], existing_tags=["wp_npcadd_RODIAN_02"])

    def test_resref_collision_is_case_insensitive(self):
        with self.assertRaises(ValueError):
            naw.build_route("npcadd_rodian", naw.ROUTES["R1"], existing_resrefs=["NPCWP_RODIAN_01"])


class InputValidationTests(unittest.TestCase):
    def assertRejects(self, creature_tag, points):
        with self.assertRaises(ValueError):
            naw.build_route(creature_tag, points)

    def test_bad_creature_tags(self):
        for tag in ("rodian", "npcadd_", "npcadd_Rodian", "npcadd_abcdefghij", None):
            self.assertRejects(tag, naw.ROUTES["R1"])

    def test_point_count(self):
        self.assertRejects("npcadd_rodian", naw.ROUTES["R1"][:2])
        self.assertRejects("npcadd_rodian", [])

    def test_identical_consecutive_points(self):
        self.assertRejects("npcadd_rodian", ((1.0, 2.0), (1.0, 2.0), (3.0, 4.0)))

    def test_non_finite_point(self):
        self.assertRejects("npcadd_rodian", ((1.0, 2.0), (float("nan"), 3.0), (3.0, 4.0)))

    def test_bad_point_shape(self):
        self.assertRejects("npcadd_rodian", ((1.0, 2.0, 0.0), (3.0, 4.0), (5.0, 6.0)))


if __name__ == "__main__":
    unittest.main()
