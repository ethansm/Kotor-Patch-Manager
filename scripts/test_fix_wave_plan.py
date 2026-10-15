#!/usr/bin/env python3
"""Host-only tests for fix_wave_plan.py. Every run uses temp dirs for items, waves root and repo root; nothing touches the real .claude/waves/."""
import contextlib
import io
import json
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPTS))
import fix_wave_plan as fwp  # noqa: E402

FX3 = SCRIPTS / "testdata" / "fix_wave" / "fx3_items.tsv"
REAL_LIMITS = SCRIPTS.parent / ".claude" / "skills" / "variant-fix-wave" / "limits.toml"
HEADER = "\t".join(fwp.COLUMNS)


def row(rid, kind="bug", size="M", group="", after=""):
    return {"id": rid, "kind": kind, "size": size, "group": group, "after": after}


def write_items(dirpath, rows, name="items.tsv"):
    path = Path(dirpath) / name
    lines = [HEADER] + ["\t".join(r.get(c, "") for c in fwp.COLUMNS) for r in rows]
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return path


class TempCase(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = Path(self._tmp.name)
        self.waves = self.tmp / "waves"
        self.repo = self.tmp / "repo"  # no limits.toml here, so defaults apply
        self.repo.mkdir()

    def tearDown(self):
        self._tmp.cleanup()

    def run_main(self, argv):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            rc = fwp.main(argv)
        return rc, out.getvalue(), err.getvalue()

    def plan_argv(self, items, wave="FX9", *extra):
        return [str(items), "--wave", wave, "--waves-root", str(self.waves),
                "--repo-root", str(self.repo), *extra]


class HelperTests(TempCase):
    def test_lst_splits_and_drops_placeholders(self):
        self.assertEqual(fwp.lst("WI-1; WI-2 ;-;"), ["WI-1", "WI-2"])
        self.assertEqual(fwp.lst(""), [])

    def test_ent_prefixes_only_bare_paths(self):
        self.assertEqual(fwp.ent("k1_port/x.py:1-5"), "patch_manager_mods/k1_port/x.py:1-5")
        self.assertEqual(fwp.ent("scripts/a.py"), "scripts/a.py")
        self.assertEqual(fwp.ent("patch_manager_mods/b.py"), "patch_manager_mods/b.py")

    def test_variants_by_kind_and_size(self):
        self.assertEqual(fwp.variants("bug", "S"), ["A"])
        self.assertEqual(fwp.variants("bug", "M"), ["A", "B", "C"])
        self.assertEqual(fwp.variants("bug", "L"), ["A", "B", "C"])
        self.assertEqual(fwp.variants("gap", "S"), ["T2"])
        self.assertEqual(fwp.variants("gap", "M"), ["T2"])
        self.assertEqual(fwp.variants("gap", "L"), ["T1", "T2"])
        with self.assertRaises(fwp.PlanError):
            fwp.variants("nope", "S")

    def test_unit_cost_formula_numbers(self):
        limits = fwp.load_limits(None)
        agents, usd = fwp.unit_cost(row("a", "bug", "S"), limits)
        self.assertEqual(agents, 3)
        self.assertAlmostEqual(usd, 1.3065, places=6)
        agents, usd = fwp.unit_cost(row("b", "bug", "M"), limits)
        self.assertEqual(agents, 5)
        self.assertAlmostEqual(usd, 1.4235, places=6)
        agents, usd = fwp.unit_cost(row("c", "gap", "L"), limits)
        self.assertEqual(agents, 4)
        self.assertAlmostEqual(usd, 1.365, places=6)
        agents, usd = fwp.unit_cost(row("d", "gap", "M"), limits)
        self.assertEqual(agents, 3)
        self.assertAlmostEqual(usd, 1.3065, places=6)

    def test_load_limits_defaults_when_file_absent(self):
        limits = fwp.load_limits(self.tmp / "absent.toml")
        self.assertEqual((limits["max_units"], limits["max_agents"], limits["max_cost_usd"],
                          limits["max_inflight"]), (12, 60, 15.0, 20))
        self.assertEqual(limits["rates"]["revision_factor"], 1.3)

    def test_load_limits_file_then_overrides(self):
        path = self.tmp / "limits.toml"
        path.write_text("max_units = 7\nmax_cost_usd = 9.5\n[rates]\nhaiku = 0.1\n", encoding="utf-8")
        limits = fwp.load_limits(path, {"max_units": 5, "max_agents": None})
        self.assertEqual(limits["max_units"], 5)  # override wins over file
        self.assertEqual(limits["max_cost_usd"], 9.5)  # from file
        self.assertEqual(limits["max_agents"], 60)  # None override ignored
        self.assertEqual(limits["rates"]["haiku"], 0.1)
        self.assertEqual(limits["rates"]["consolidator"], 0.62)

    def test_load_limits_rejects_unknown_key(self):
        path = self.tmp / "limits.toml"
        path.write_text("max_unit = 3\n", encoding="utf-8")
        with self.assertRaises(fwp.PlanError):
            fwp.load_limits(path)

    def test_repo_limits_toml_loads_when_present(self):
        if not REAL_LIMITS.is_file():
            self.skipTest("no variant-fix-wave limits.toml in this checkout")
        limits = fwp.load_limits(REAL_LIMITS)
        self.assertEqual(limits["max_units"], 10)
        self.assertEqual(limits["rates"]["haiku"], 0.045)

    def test_read_items_rejects_bad_kind_and_names_row(self):
        items = write_items(self.tmp, [row("WI-1", kind="feat")])
        with self.assertRaisesRegex(fwp.PlanError, "WI-1"):
            fwp.read_items(items)

    def test_read_items_rejects_bad_size_and_names_row(self):
        items = write_items(self.tmp, [row("WI-2", size="XL")])
        with self.assertRaisesRegex(fwp.PlanError, "WI-2"):
            fwp.read_items(items)

    def test_read_items_empty_size_is_M(self):
        items = write_items(self.tmp, [row("WI-3", size="")])
        self.assertEqual(fwp.read_items(items)[0]["size"], "M")

    def test_read_items_fx3_fixture(self):
        rows = fwp.read_items(FX3)
        self.assertEqual(len(rows), 23)
        self.assertEqual(rows[0]["id"], "WI-73")
        self.assertEqual(rows[-1]["id"], "WI-103")


class BlockTests(TempCase):
    @staticmethod
    def ids(blks):
        return [[r["id"] for r in b] for b in blks]

    def test_group_rows_form_one_block_and_free_rows_stand_alone(self):
        items = write_items(self.tmp, [row("K1", group="K"), row("X1"), row("K2", group="K")])
        self.assertEqual(self.ids(fwp.blocks(fwp.read_items(items))), [["K1", "K2"], ["X1"]])

    def test_after_puts_prerequisite_block_first(self):
        items = write_items(self.tmp, [row("A1", after="B1"), row("B1")])
        self.assertEqual(self.ids(fwp.blocks(fwp.read_items(items))), [["B1"], ["A1"]])

    def test_rows_inside_a_block_follow_after(self):
        items = write_items(self.tmp, [row("I2", group="I", after="I3"), row("I1", group="I"),
                                       row("I3", group="I", after="I1")])
        self.assertEqual(self.ids(fwp.blocks(fwp.read_items(items))), [["I1", "I3", "I2"]])

    def test_row_cycle_raises(self):
        items = write_items(self.tmp, [row("A1", after="B1"), row("B1", after="A1")])
        with self.assertRaisesRegex(fwp.PlanError, "cycle"):
            fwp.blocks(fwp.read_items(items))

    def test_cross_block_cycle_raises(self):
        # K1 must follow free X1, and X1 must follow K2: no row cycle, but a cycle between blocks.
        items = write_items(self.tmp, [row("K1", group="K", after="X1"), row("X1", after="K2"),
                                       row("K2", group="K")])
        with self.assertRaisesRegex(fwp.PlanError, "cycle"):
            fwp.blocks(fwp.read_items(items))

    def test_unknown_after_id_raises(self):
        items = write_items(self.tmp, [row("A1", after="WI-999")])
        with self.assertRaisesRegex(fwp.PlanError, "WI-999"):
            fwp.blocks(fwp.read_items(items))


class PackTests(TempCase):
    def test_under_caps_is_one_batch(self):
        items = write_items(self.tmp, [row(f"F{i}") for i in range(3)])
        batches = fwp.pack_batches(fwp.blocks(fwp.read_items(items)), fwp.load_limits(None))
        self.assertEqual(len(batches), 1)
        self.assertEqual([r["id"] for r in batches[0]], ["F0", "F1", "F2"])

    def test_fx3_fixture_packs_within_caps_and_order(self):
        limits = fwp.load_limits(None)
        rows = fwp.read_items(FX3)
        batches = fwp.pack_batches(fwp.blocks(rows), limits)
        self.assertGreaterEqual(len(batches), 2)
        for b in batches:
            units, agents, usd = fwp.batch_totals(b, limits)
            self.assertLessEqual(units, 12)
            self.assertLessEqual(agents, 60)
            self.assertLessEqual(usd, 15.0)
        flat = [r for b in batches for r in b]
        self.assertEqual(sorted(r["id"] for r in flat), sorted(r["id"] for r in rows))
        where = {r["id"]: i for i, b in enumerate(batches) for r in b}
        pos = {r["id"]: i for i, r in enumerate(flat)}
        groups = {}
        for r in rows:
            if r["group"] not in ("", "-"):
                groups.setdefault(r["group"], set()).add(where[r["id"]])
        for g, seen in groups.items():
            self.assertEqual(len(seen), 1, f"group {g} split across batches {sorted(seen)}")
        for r in rows:
            for a in fwp.lst(r["after"]):
                self.assertLessEqual(where[a], where[r["id"]], f"{r['id']} after {a}: batch order")
                self.assertLess(pos[a], pos[r["id"]], f"{r['id']} after {a}: row order")

    def test_block_over_unit_cap_exits_2_with_block_message(self):
        items = write_items(self.tmp, [row(f"Z{i}", size="S", group="Z") for i in range(13)])
        rc, _, err = self.run_main(self.plan_argv(items, "FX9", "--max-cost", "100"))
        self.assertEqual(rc, 2)
        self.assertIn("BLOCK Z over cap (units): split the group or raise the limit with --max-*", err)
        self.assertFalse((self.waves / "FX9").exists())

    def test_free_row_over_cost_cap_names_the_row(self):
        items = write_items(self.tmp, [row("FREE-1")])
        rc, _, err = self.run_main(self.plan_argv(items, "FX9", "--max-cost", "1.0"))
        self.assertEqual(rc, 2)
        self.assertIn("BLOCK FREE-1 over cap (cost)", err)

    def test_cycle_exits_2_and_writes_nothing(self):
        items = write_items(self.tmp, [row("A1", after="B1"), row("B1", after="A1")])
        rc, _, err = self.run_main(self.plan_argv(items))
        self.assertEqual(rc, 2)
        self.assertIn("cycle", err)
        self.assertFalse((self.waves / "FX9").exists())

    def test_unknown_after_exits_2_naming_the_id(self):
        items = write_items(self.tmp, [row("A1", after="WI-999")])
        rc, _, err = self.run_main(self.plan_argv(items))
        self.assertEqual(rc, 2)
        self.assertIn("WI-999", err)

    def test_max_units_override_splits_batches(self):
        items = write_items(self.tmp, [row(f"F{i}") for i in range(5)])
        rc, _, err = self.run_main(self.plan_argv(items, "FX9", "--max-units", "2"))
        self.assertEqual(rc, 0, err)
        lines = (self.waves / "FX9" / "batches.tsv").read_text(encoding="utf-8").splitlines()
        self.assertEqual([line.split("\t")[2] for line in lines[1:]], ["2", "2", "1"])


class PlanFileTests(TempCase):
    def test_writes_items_plan_json_and_batches_for_fx3(self):
        rc, out, err = self.run_main(self.plan_argv(FX3))
        self.assertEqual(rc, 0, err)
        plan = self.waves / "FX9"
        self.assertEqual((plan / "items.tsv").read_bytes(), FX3.read_bytes())
        meta = json.loads((plan / "plan.json").read_text(encoding="utf-8"))
        self.assertEqual(set(meta), {"items", "findings", "cols", "limits"})
        self.assertEqual(meta["items"], str(FX3.resolve()))
        self.assertIsNone(meta["findings"])
        self.assertIsNone(meta["cols"])
        self.assertEqual(meta["limits"]["max_units"], 12)
        lines = (plan / "batches.tsv").read_text(encoding="utf-8").splitlines()
        self.assertEqual(lines[0], "batch\twave_id\tunits\tagents\ttest_usd\tids")
        self.assertGreaterEqual(len(lines) - 1, 2)
        ids = []
        for i, line in enumerate(lines[1:]):
            letter, wave_id, units, agents, usd, members = line.split("\t")
            self.assertEqual(letter, chr(65 + i))
            self.assertEqual(wave_id, f"FX9-{letter}")
            self.assertEqual(int(units), len(members.split(";")))
            self.assertRegex(usd, r"^\d+\.\d\d$")
            ids.extend(members.split(";"))
        self.assertEqual(sorted(ids), sorted(r["id"] for r in fwp.read_items(FX3)))
        self.assertIn("total: 23 units", out)

    def test_findings_and_cols_recorded(self):
        findings = self.tmp / "findings.tsv"
        findings.write_text("key\tnote\nWI-1\tx\n", encoding="utf-8")
        items = write_items(self.tmp, [row("WI-1")])
        rc, _, err = self.run_main(self.plan_argv(items, "FX9", "--findings", str(findings), "--cols", "key,note"))
        self.assertEqual(rc, 0, err)
        meta = json.loads((self.waves / "FX9" / "plan.json").read_text(encoding="utf-8"))
        self.assertEqual(meta["findings"], str(findings.resolve()))
        self.assertEqual(meta["cols"], ["key", "note"])

    def test_findings_without_cols_exits_2(self):
        findings = self.tmp / "findings.tsv"
        findings.write_text("key\n", encoding="utf-8")
        items = write_items(self.tmp, [row("WI-1")])
        rc, _, _ = self.run_main(self.plan_argv(items, "FX9", "--findings", str(findings)))
        self.assertEqual(rc, 2)

    def test_leading_plan_token_is_accepted(self):
        items = write_items(self.tmp, [row("WI-1")])
        rc, _, err = self.run_main(["plan", *self.plan_argv(items)])
        self.assertEqual(rc, 0, err)
        self.assertTrue((self.waves / "FX9" / "batches.tsv").is_file())


class SelectSourceTests(TempCase):
    """Items written beside a candidates.tsv (the `select` layout): batch A keeps its claims, the rest are released."""

    def setUp(self):
        super().setUp()
        self.wave_dir = self.waves / "FX9"
        self.wave_dir.mkdir(parents=True)

    def plan_select(self, release, *extra, with_candidates=True):
        items = write_items(self.wave_dir, [row("WI-1"), row("WI-2"), row("WI-3")], name="items.input.tsv")
        if with_candidates:
            write_items(self.wave_dir, [row("WI-1"), row("WI-2"), row("WI-3")], name="candidates.tsv")
        args = fwp.build_parser().parse_args(self.plan_argv(items, "FX9", "--max-units", "2", *extra))
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            rc = fwp.run_plan(args, release=release)
        return rc, out.getvalue()

    def plan_meta(self):
        return json.loads((self.wave_dir / "plan.json").read_text(encoding="utf-8"))

    def test_select_releases_only_candidates_outside_batch_a(self):
        calls = []
        rc, out = self.plan_select(lambda ids: calls.append(list(ids)))
        self.assertEqual(rc, 0)
        batches = (self.wave_dir / "batches.tsv").read_text(encoding="utf-8").splitlines()
        self.assertEqual(batches[1].split("\t")[5], "WI-1;WI-2")  # batch A
        self.assertEqual(calls, [["WI-3"]])
        self.assertIn("released, not in batch A: WI-3", out)
        self.assertEqual(self.plan_meta()["source"], "select")

    def test_without_candidates_nothing_is_released_and_no_source(self):
        calls = []
        rc, out = self.plan_select(lambda ids: calls.append(list(ids)), with_candidates=False)
        self.assertEqual(rc, 0)
        self.assertEqual(calls, [])
        self.assertNotIn("released", out)
        self.assertNotIn("source", self.plan_meta())

    def test_default_release_drops_claims_in_claims_dir_only_outside_batch_a(self):
        import wi_state as ws  # imported here: only the default release path needs it
        claims = self.tmp / "claims"
        store = ws.Store(self.repo, claims_dir=claims)
        for wi in ("WI-1", "WI-3"):
            self.assertIsNotNone(store.create_claim(wi, "test", [], "abc1234", "VW-FX9"))
        rc, _ = self.plan_select(None, "--claims-dir", str(claims))
        self.assertEqual(rc, 0)
        self.assertTrue(store.claim_path("WI-1").is_file())  # batch A keeps its claim
        self.assertFalse(store.claim_path("WI-3").exists())  # released


if __name__ == "__main__":
    unittest.main()
