#!/usr/bin/env python3
"""Tests for scripts/check_npc_add.py. Pure stdlib: every input is a synthetic fixture (plain dicts, bytes, name sets),
so no pykotor and no game install is needed."""
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import check_npc_add as cna  # noqa: E402

SHA_BUILT = "b" * 64
SHA_LIVE = "a" * 64


def utc(row=271, **over):
    """Plain UTC that passes check 7 and uses one k_def script."""
    u = {"Appearance_Type": row, "SoundSetFile": 35, "FactionID": 5, "Interruptable": 1,
         "Plot": 0, "Min1HP": 0, "NoPermDeath": 0, "IsPC": 0,
         "ScriptSpawn": "k_def_spawn01", "ScriptHeartbeat": "", "Conversation": ""}
    u.update(over)
    return u


def wp(tag, x, y):
    return {"Tag": tag, "XPosition": x, "YPosition": y}


def base_creatures():
    return [{"TemplateResRef": f"n_vanilla{i:02d}"} for i in range(cna.BASE_CREATURES)]


def base_waypoints():
    return [wp(f"WP_vanilla_{i:02d}", 100.0 + i, 100.0) for i in range(cna.BASE_WAYPOINTS)]


def base_git():
    return {"__struct_id": -1, "Creature List": base_creatures(), "WaypointList": base_waypoints(),
            "Door List": [{"X": 1.0}], "Placeable List": [{"X": 0.0, "Y": 0.0}]}


def base_resources():
    res = {(f"r{i:03d}", "UTC"): f"body{i}".encode() for i in range(cna.ORIGINAL_RESOURCES - 1)}
    res[cna.GIT_KEY] = b"original-git"
    return res


def built_git(k=1, routes=(("wookm", (-65.7, 20.05), (-70.45, 26.3), (-66.2, 32.05)),)):
    g = base_git()
    g["Creature List"] = g["Creature List"] + [{"TemplateResRef": f"npcadd_{s}"} for s, *_ in routes][:k]
    added = []
    for slug, *pts in routes:
        for i, (x, y) in enumerate(pts, 1):
            added.append(wp(f"WP_npcadd_{slug}_{i:02d}", x, y))
    g["WaypointList"] = g["WaypointList"] + added
    return g


def fixture(k=1, routes=(("wookm", (-65.7, 20.05), (-70.45, 26.3), (-66.2, 32.05)),), bad_utc=None):
    """A fully consistent synthetic build: one canary UTC, one route with three waypoints."""
    slugs = [s for s, *_ in routes][:k]
    utcs = {f"npcadd_{s}": utc() for s in slugs}
    if bad_utc:
        utcs[bad_utc[0]] = bad_utc[1]
    res = dict(base_resources())
    for r in utcs:
        res[(r, "UTC")] = f"utc-bytes-{r}".encode()
    tags = {f"r{i:03d}": f"tag{i}" for i in range(cna.ORIGINAL_RESOURCES - 1)}
    tags.update({r: r for r in utcs})
    mirror = {r: res[(r, "UTC")] for r in utcs}
    built = cna.BuiltInputs(SHA_BUILT, built_git(k, routes), res, list(res), utcs, tags, mirror)
    baseline = cna.BaselineInputs(SHA_LIVE, base_git(), base_resources())
    return built, baseline


SOURCES_OK = {"patch": frozenset({"n_wookiem.mdl", "n_wookiem.mdx"}),
              "live": frozenset(),
              "chitin": frozenset({"n_wookiem.mdl", "n_wookiem.mdx", "n_wookiem06.tpc", "k_def_spawn01.ncs",
                                   "twilek_m01.tpc"})}
APPEARANCE = {271: {"race": "N_WookieM", "racetex": "N_WookieM02", "normalhead": "", "modela": "N_WookieM",
                    "texa": "N_WookieM"}}
HEADS = {61: {"head": "P_TwiH1", "alttexture": "P_Twi1"}}


class WaypointPointsTests(unittest.TestCase):
    def test_groups_npcadd_tags_by_route_in_index_order(self):
        pts = cna.waypoint_points([wp("WP_npcadd_rodian_02", 2, 0), wp("WP_vanilla_01", 9, 9),
                                   wp("WP_npcadd_rodian_01", 1, 0), wp("WP_npcadd_bith_01", 3, 3)])
        self.assertEqual(pts, {"npcadd_rodian": [(1, 1.0, 0.0), (2, 2.0, 0.0)], "npcadd_bith": [(1, 3.0, 3.0)]})


class CheckGitTests(unittest.TestCase):
    def test_canary_counts_and_bit_exact_originals_pass(self):
        built, base = fixture()
        r = cna.check_git(built.git, base.git, 1, {"npcadd_wookm"}, built.resources, base.resources)
        self.assertEqual(r.status, cna.PASS, r.lines)
        self.assertIn("creatures 18=17+1", r.summary)
        self.assertIn("waypoints 51=48+3*1", r.summary)

    def test_wrong_creature_count_fails(self):
        built, base = fixture()
        built.git["Creature List"].append({"TemplateResRef": "npcadd_wookm"})
        r = cna.check_git(built.git, base.git, 1, {"npcadd_wookm"}, built.resources, base.resources)
        self.assertEqual(r.status, cna.FAIL)
        self.assertIn("Creature List has 19 entries, expected 17+1=18", r.lines[0])

    def test_wrong_waypoint_count_fails(self):
        built, base = fixture()
        built.git["WaypointList"].pop()
        r = cna.check_git(built.git, base.git, 1, {"npcadd_wookm"}, built.resources, base.resources)
        self.assertEqual(r.status, cna.FAIL)
        self.assertTrue(any("WaypointList has 50" in x for x in r.lines))

    def test_new_creature_without_embedded_utc_fails(self):
        built, base = fixture()
        r = cna.check_git(built.git, base.git, 1, set(), built.resources, base.resources)
        self.assertEqual(r.status, cna.FAIL)
        self.assertTrue(any("has no embedded UTC" in x for x in r.lines))

    def test_changed_original_creature_fails(self):
        built, base = fixture()
        built.git["Creature List"][3]["TemplateResRef"] = "changed"
        r = cna.check_git(built.git, base.git, 1, {"npcadd_wookm"}, built.resources, base.resources)
        self.assertEqual(r.status, cna.FAIL)
        self.assertIn("  - original Creature List entries changed", r.lines)

    def test_changed_other_root_list_fails(self):
        built, base = fixture()
        built.git["Door List"] = [{"X": 2.0}]
        r = cna.check_git(built.git, base.git, 1, {"npcadd_wookm"}, built.resources, base.resources)
        self.assertEqual(r.status, cna.FAIL)
        self.assertIn("  - root field 'Door List' differs from baseline", r.lines)

    def test_missing_original_resource_fails_and_changed_git_alone_does_not(self):
        built, base = fixture()
        del built.resources[("r000", "UTC")]
        r = cna.check_git(built.git, base.git, 1, {"npcadd_wookm"}, built.resources, base.resources)
        self.assertIn("  - original resource missing: r000.UTC", r.lines)
        built2, base2 = fixture()
        built2.resources[cna.GIT_KEY] = b"different-git-bytes"
        r2 = cna.check_git(built2.git, base2.git, 1, {"npcadd_wookm"}, built2.resources, base2.resources)
        self.assertEqual(r2.status, cna.PASS, r2.lines)

    def test_changed_original_resource_fails(self):
        built, base = fixture()
        built.resources[("r005", "UTC")] = b"edited"
        r = cna.check_git(built.git, base.git, 1, {"npcadd_wookm"}, built.resources, base.resources)
        self.assertEqual(r.status, cna.FAIL)
        self.assertIn("  - original resource changed: r005.UTC", r.lines)

    def test_no_baseline_passes_count_checks_and_says_compare_skipped(self):
        built, _ = fixture()
        r = cna.check_git(built.git, None, 1, {"npcadd_wookm"}, built.resources, None)
        self.assertEqual(r.status, cna.PASS)
        self.assertIn("SKIPPED", r.summary)


class MirrorTests(unittest.TestCase):
    def test_none_mirror_compares_nothing(self):
        self.assertEqual(cna.check_mirror({"npcadd_a": b"x"}, None), [])

    def test_missing_and_differing_mirror_files_are_reported(self):
        out = cna.check_mirror({"npcadd_a": b"1", "npcadd_b": b"2"}, {"npcadd_a": b"9", "npcadd_c": b"3"})
        self.assertIn("mirror missing npcadd_b.utc", out)
        self.assertIn("mirror npcadd_a.utc differs from embedded UTC", out)
        self.assertEqual(len(out), 2)


class UniqueTests(unittest.TestCase):
    def test_clean_build_passes_even_with_vanilla_duplicate_tags(self):
        tags = {"merc01": "commoner", "other": "commoner", "npcadd_a": "npcadd_a"}
        r = cna.check_unique([("a", "UTC"), ("b", "UTW")], tags, ["npcadd_a"], ["WP_npcadd_a_01"], ["npcadd_a"])
        self.assertEqual(r.status, cna.PASS, r.lines)

    def test_each_duplicate_kind_fails(self):
        tags = {"npcadd_a": "npcadd_a", "vanilla": "npcadd_a"}
        r = cna.check_unique([("a", "UTC"), ("a", "UTC")], tags, ["npcadd_a"],
                             ["WP_npcadd_a_01", "wp_npcadd_a_01"], ["npcadd_a", "NPCADD_A"])
        self.assertEqual(r.status, cna.FAIL)
        joined = "\n".join(r.lines)
        self.assertIn("duplicate resource a.UTC", joined)
        self.assertIn("new UTC npcadd_a tag 'npcadd_a' shared by 2 UTCs", joined)
        self.assertIn("waypoint tag 'wp_npcadd_a_01' x2", joined)
        self.assertIn("creature template 'npcadd_a' used by 2 GIT entries", joined)


class VanillaNameTests(unittest.TestCase):
    def test_unreadable_chitin_skips(self):
        self.assertEqual(cna.check_vanilla_names(["npcadd_a"], None).status, cna.SKIP)

    def test_reused_vanilla_template_fails_and_fresh_name_passes(self):
        chitin = frozenset({"n_telf01.utc"})
        self.assertEqual(cna.check_vanilla_names(["N_TelF01"], chitin).status, cna.FAIL)
        self.assertEqual(cna.check_vanilla_names(["npcadd_a"], chitin).status, cna.PASS)


class WaypointCheckTests(unittest.TestCase):
    PLACE = [(0.0, 0.0), (10.0, 0.0)]

    def test_three_points_clear_of_placeables_pass(self):
        r = cna.check_waypoints({"npcadd_a": [(1, 5.0, 5.0), (2, 5.0, 6.0), (3, 5.0, 7.0)]}, self.PLACE)
        self.assertEqual(r.status, cna.PASS, r.lines)

    def test_two_waypoints_fail(self):
        r = cna.check_waypoints({"npcadd_a": [(1, 5.0, 5.0), (2, 5.0, 6.0)]}, self.PLACE)
        self.assertEqual(r.status, cna.FAIL)
        self.assertIn("npcadd_a has 2 waypoints, need >= 3", r.lines[0])

    def test_non_contiguous_indices_fail(self):
        r = cna.check_waypoints({"npcadd_a": [(1, 5.0, 5.0), (2, 5.0, 6.0), (4, 5.0, 7.0)]}, self.PLACE)
        self.assertEqual(r.status, cna.FAIL)
        self.assertTrue(any("indices not 1..3" in x for x in r.lines))

    def test_distance_just_under_one_metre_fails_and_exactly_one_passes(self):
        under = cna.check_waypoints({"npcadd_a": [(1, 0.0, 0.99), (2, 5.0, 5.0), (3, 6.0, 5.0)]}, self.PLACE)
        self.assertEqual(under.status, cna.FAIL)
        self.assertIn("npcadd_a wp 01 is 0.99 m from a placeable (need >= 1.0)", under.lines[0])
        exact = cna.check_waypoints({"npcadd_a": [(1, 0.0, 1.0), (2, 5.0, 5.0), (3, 6.0, 5.0)]}, self.PLACE)
        self.assertEqual(exact.status, cna.PASS, exact.lines)

    def test_no_placeables_is_not_a_failure(self):
        r = cna.check_waypoints({"npcadd_a": [(1, 0.0, 0.0), (2, 1.0, 0.0), (3, 2.0, 0.0)]}, [])
        self.assertEqual(r.status, cna.PASS)


class InstalledTests(unittest.TestCase):
    def test_absent_build_or_game_file_skips(self):
        self.assertEqual(cna.check_installed(None, SHA_LIVE, None).status, cna.SKIP)
        self.assertEqual(cna.check_installed(SHA_BUILT, None, None).status, cna.SKIP)

    def test_equal_bytes_pass(self):
        self.assertEqual(cna.check_installed(SHA_BUILT, SHA_BUILT, SHA_LIVE).status, cna.PASS)

    def test_different_bytes_skip_as_not_installed(self):
        r = cna.check_installed(SHA_BUILT, SHA_LIVE, SHA_LIVE)
        self.assertEqual(r.status, cna.SKIP)
        self.assertIn("not installed", r.summary)
        self.assertIn("live is the unmodified baseline", r.summary)
        r2 = cna.check_installed(SHA_BUILT, SHA_LIVE, "c" * 64)
        self.assertNotIn("unmodified", r2.summary)


class ForbiddenFieldTests(unittest.TestCase):
    def test_canonical_utc_passes(self):
        self.assertEqual(cna.check_forbidden({"npcadd_a": utc()}).status, cna.PASS)

    def test_each_forbidden_value_fails(self):
        for field, value in (("Plot", 1), ("Min1HP", 1), ("NoPermDeath", 1), ("IsPC", 1), ("FactionID", 0),
                             ("FactionID", 1), ("Interruptable", 0)):
            with self.subTest(field=field, value=value):
                r = cna.check_forbidden({"npcadd_a": utc(**{field: value})})
                self.assertEqual(r.status, cna.FAIL)
                self.assertIn(field, "\n".join(r.lines))

    def test_absent_required_fields_fail_but_absent_zero_fields_pass(self):
        u = utc()
        del u["FactionID"]
        self.assertEqual(cna.check_forbidden({"npcadd_a": u}).status, cna.FAIL)
        u2 = utc()
        del u2["Interruptable"]
        self.assertEqual(cna.check_forbidden({"npcadd_a": u2}).status, cna.FAIL)
        u3 = utc()
        del u3["Plot"]
        self.assertEqual(cna.check_forbidden({"npcadd_a": u3}).status, cna.PASS)


class CollisionRowTests(unittest.TestCase):
    def test_forbidden_rows_cover_tables_snippet_b_and_app_remap(self):
        tables = {"IsGenericMaleCommoner": [17, 24], "IsGenericTwilekFemale": [280]}
        forb = cna.forbidden_rows(tables, {"APP_COMMONER_FAT_MALE": 802, "APP_ALIEN_KELDOR_01": 812})
        self.assertEqual(forb[24], "IsGenericMaleCommoner input list")
        self.assertEqual(forb[449], "Snippet B collision list")
        self.assertEqual(forb[802], "APP_COMMONER_FAT_MALE remap output")
        self.assertNotIn(812, forb)
        self.assertNotIn(47, forb)
        self.assertNotIn(464, forb)

    def test_check_rows_fails_on_a_colliding_row_and_passes_the_canary_rows(self):
        forb = cna.forbidden_rows({}, {})
        self.assertEqual(cna.check_rows({"npcadd_a": 271, "npcadd_b": 47}, forb).status, cna.PASS)
        r = cna.check_rows({"npcadd_a": 416}, forb)
        self.assertEqual(r.status, cna.FAIL)
        self.assertIn("npcadd_a uses appearance row 416: Snippet B collision list", r.lines[0])


class AssetTests(unittest.TestCase):
    def check(self, u, sources=SOURCES_OK, soundset=100, heads=HEADS, appearance=APPEARANCE):
        return cna.check_assets({"npcadd_a": u}, appearance, heads, soundset, sources)

    def test_resolved_assets_pass(self):
        r = self.check(utc())
        self.assertEqual(r.status, cna.PASS, r.lines)
        self.assertIn("1 UTCs, 3 refs OK, 0 live Override only", r.summary)

    def test_live_only_texture_passes_and_prints_source(self):
        app = {271: {"race": "", "racetex": "", "normalhead": "", "modela": "N_WookieM", "texa": "N_WookieM02"}}
        srcs = {"patch": frozenset(), "live": frozenset({"n_wookiem02.tpc"}),
                "chitin": frozenset({"n_wookiem.mdl", "n_wookiem.mdx", "k_def_spawn01.ncs"})}
        r = cna.check_assets({"npcadd_a": utc()}, app, HEADS, 100, srcs)
        self.assertEqual(r.status, cna.PASS, r.lines)
        self.assertTrue(any("texture N_WookieM02: live Override only" in x for x in r.lines))

    def test_head_and_alttexture_are_checked_through_heads_2da(self):
        app = {47: {"race": "N_TwiDance_Boot", "racetex": "", "normalhead": "61", "modela": "N_TwiDance_Boot",
                    "texa": ""}}
        srcs = {"patch": frozenset({"n_twidance_boot.mdl", "n_twidance_boot.mdx"}),
                "live": frozenset({"p_twih1.mdl", "p_twih1.mdx", "p_twi1.tpc"}), "chitin": frozenset({"k_def_spawn01.ncs"})}
        r = cna.check_assets({"npcadd_twilekf": utc(47)}, app, HEADS, 100, srcs)
        self.assertEqual(r.status, cna.PASS, r.lines)
        self.assertTrue(any("head 61 P_TwiH1: live Override only" in x for x in r.lines))
        self.assertTrue(any("head 61 alttexture P_Twi1: live Override only" in x for x in r.lines))

    def test_missing_model_fails(self):
        srcs = dict(SOURCES_OK, patch=frozenset(), chitin=frozenset({"k_def_spawn01.ncs"}))
        r = self.check(utc(), sources=srcs)
        self.assertEqual(r.status, cna.FAIL)
        self.assertTrue(any("model N_WookieM not found" in x for x in r.lines))

    def test_missing_script_fails(self):
        srcs = dict(SOURCES_OK, chitin=frozenset({"n_wookiem.mdl", "n_wookiem.mdx"}))
        r = self.check(utc())
        self.assertEqual(r.status, cna.PASS)
        r2 = self.check(utc(), sources=srcs)
        self.assertEqual(r2.status, cna.FAIL)
        self.assertTrue(any("ScriptSpawn k_def_spawn01 not found" in x for x in r2.lines))

    def test_unknown_source_skips_not_fails(self):
        srcs = dict(SOURCES_OK, chitin=None)
        r = self.check(utc(), sources=srcs)
        self.assertEqual(r.status, cna.SKIP, r.lines)

    def test_bad_row_soundset_and_heads_fail(self):
        self.assertEqual(self.check(utc(row=999)).status, cna.FAIL)
        self.assertEqual(self.check(utc(SoundSetFile=100), soundset=100).status, cna.FAIL)
        bad_heads = self.check(utc(), appearance={271: dict(APPEARANCE[271], normalhead="99")})
        self.assertEqual(bad_heads.status, cna.FAIL)
        self.assertIn("heads row 99 missing", "\n".join(bad_heads.lines))

    def test_race_fallback_when_modela_blank(self):
        app = {464: {"race": "Sch_Trando", "racetex": "p_trndoshan", "normalhead": "", "modela": "", "texa": ""}}
        srcs = {"patch": frozenset(), "live": frozenset({"sch_trando.mdl", "sch_trando.mdx", "p_trndoshan.tpc"}),
                "chitin": frozenset({"k_def_spawn01.ncs"})}
        r = cna.check_assets({"npcadd_t": utc(464)}, app, HEADS, 100, srcs)
        self.assertEqual(r.status, cna.PASS, r.lines)
        self.assertTrue(any("model Sch_Trando: live Override only" in x for x in r.lines))


class RunAllTests(unittest.TestCase):
    def ctx(self, built=None, live_sha=SHA_LIVE, baseline=None, sources=SOURCES_OK):
        return cna.Context(built=built, live_sha=live_sha, baseline=baseline, sources=sources,
                           appearance=APPEARANCE, heads=HEADS, soundset_rows=100,
                           tables={"IsGenericMaleCommoner": [17]}, app_consts={"APP_X": 802})

    def test_no_build_skips_every_built_check_and_exits_zero(self):
        code, lines = cna.run_all(self.ctx(built=None, live_sha=None))
        self.assertEqual(code, 0)
        skips = [x for x in lines if x.startswith("SKIP")]
        self.assertEqual(len(skips), 8)
        self.assertIn("RESULT: PASS (0 pass, 0 fail, 8 skip)", lines)

    def test_consistent_build_passes_and_reports_not_installed(self):
        built, base = fixture()
        code, lines = cna.run_all(self.ctx(built=built, baseline=base))
        self.assertEqual(code, 0, lines)
        self.assertTrue(any(x.startswith("SKIP  6") for x in lines))
        self.assertIn("RESULT: PASS (7 pass, 0 fail, 1 skip)", lines)

    def test_one_forbidden_field_makes_the_run_fail(self):
        built, base = fixture(bad_utc=("npcadd_bad", utc(Plot=1)))
        built.git["Creature List"].append({"TemplateResRef": "npcadd_bad"})
        code, lines = cna.run_all(self.ctx(built=built, baseline=base))
        self.assertEqual(code, 1)
        self.assertTrue(any(x.startswith("FAIL  7") for x in lines))
        self.assertTrue(any(x.startswith("PASS  1") for x in lines))

    def test_installed_build_is_pass_for_check_six(self):
        built, base = fixture()
        code, lines = cna.run_all(self.ctx(built=built, live_sha=SHA_BUILT, baseline=base))
        self.assertEqual(code, 0, lines)
        self.assertTrue(any(x.startswith("PASS  6") for x in lines))


if __name__ == "__main__":
    unittest.main()
