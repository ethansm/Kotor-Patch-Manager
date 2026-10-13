#!/usr/bin/env python3
"""Tests for build_head_allowlist (stdlib, game-free)."""
import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import build_head_allowlist as B  # noqa: E402


class FakeStruct:
    def __init__(self, **fields):
        self.fields = fields

    def acquire(self, name, default=None):
        return self.fields.get(name, default)


class FakeRoot(FakeStruct):
    def __init__(self, base, props):
        super().__init__(BaseItem=base)
        self.props = props

    def get_list(self, name):
        return self.props if name == "PropertiesList" else None


def utc(resref, appearance, race, gender, equip="", head_refs="", has_head="0"):
    return {"resref": resref, "appearance_type": appearance, "race": race, "gender": gender,
            "equip": equip, "head_item_resrefs": head_refs, "has_head_item": has_head}


class RestrictionTests(unittest.TestCase):
    def test_keeps_listed_types_sorted_and_deduplicated(self):
        got = B.restrictions([(0x3D, 1), (0x2B, 0), (0x01, 5), (0x2C, 3), (0x3D, 1), (0x45, 12)])
        self.assertEqual(got, [{"type": 0x2B, "subtype": 0}, {"type": 0x2C, "subtype": 3},
                               {"type": 0x3D, "subtype": 1}, {"type": 0x45, "subtype": 12}])

    def test_unlisted_only_and_empty(self):
        self.assertEqual(B.restrictions([(0x01, 0), (0x0A, 2)]), [])
        self.assertEqual(B.restrictions([]), [])

    def test_uti_props_reads_fake_root(self):
        root = FakeRoot(44, [FakeStruct(PropertyName=0x3D, Subtype=1), FakeStruct(PropertyName=0x39, Subtype=7)])
        self.assertEqual(B.uti_props(root), (44, [(0x3D, 1), (0x39, 7)]))
        self.assertEqual(B.uti_props(FakeRoot(72, [])), (72, []))


class CellTests(unittest.TestCase):
    def test_race_rule_blank_is_zero(self):
        self.assertEqual(B.race_rule("2"), 2)
        self.assertEqual(B.race_rule("1"), 1)
        self.assertEqual(B.race_rule("0x2"), 2)
        self.assertEqual(B.race_rule(""), 0)
        self.assertEqual(B.race_rule("  "), 0)

    def test_req_feats_non_blank_in_column_order(self):
        cells = {"reqfeat0": "", "reqfeat1": "7", "reqfeat2": "", "reqfeat3": "9", "reqfeat4": ""}
        self.assertEqual(B.req_feats(cells), [7, 9])
        self.assertEqual(B.req_feats({}), [])

    def test_req_feats_rejects_non_integer(self):
        with self.assertRaises(ValueError):
            B.req_feats({"reqfeat0": "****"})

    def test_gender_mask_is_zero(self):
        self.assertEqual(B.GENDER_MASK, 0)


class HeadBaseitemTests(unittest.TestCase):
    def test_selects_head_bit_and_derives_fields(self):
        rows = {
            44: {"label": "Mask", "equipableslots": "0x00001", "droidorhuman": "1", "genderspecific": "0",
                 "reqfeat0": ""},
            45: {"label": "Belt", "equipableslots": "0x00400", "droidorhuman": "", "reqfeat0": "3"},
            69: {"label": "Droid_X", "equipableslots": "0x00001", "droidorhuman": "2"},
            70: {"label": "Dual", "equipableslots": "0x00003", "droidorhuman": "", "reqfeat0": "5",
                 "reqfeat1": ""},
        }
        out = B.head_baseitems(rows)
        self.assertEqual(sorted(out), [44, 69, 70])
        self.assertEqual(out[44], {"label": "Mask", "equipableslots": "0x00001", "race_rule": 1,
                                   "gender_mask": 0, "req_feats": [], "droid": False})
        self.assertTrue(out[69]["droid"])
        self.assertEqual(out[69]["race_rule"], 2)
        self.assertEqual(out[70]["equipableslots"], "0x00003")
        self.assertEqual(out[70]["req_feats"], [5])


class AppearanceTests(unittest.TestCase):
    def test_aggregates_from_synthetic_rows(self):
        utc_rows = [
            utc("C_Head1", "5", "6", "0", equip="1:Hel_A;16:W_B", head_refs="", has_head="1"),
            utc("c_head2", "5", "5", "1"),
            utc("C_HEAD1", "5", "6", "1", equip="1:hel_a", head_refs="hel_a;HEL_B", has_head="1"),
            utc("x", "9", "6", "0", equip="16:w", has_head="0"),
        ]
        wear = [
            {"row": "5", "label": "Lbl5", "modeltype": "F", "has_goggle_hook": "yes",
             "n_utc": "2", "n_utc_with_head_item": "1"},
            {"row": "9", "label": "Lbl9", "modeltype": "B", "has_goggle_hook": "no",
             "n_utc": "1", "n_utc_with_head_item": "0"},
        ]
        result = B.aggregate_appearances(wear, utc_rows)
        self.assertEqual(list(result), ["5"])
        a = result["5"]
        self.assertEqual(a, {"label": "Lbl5", "modeltype": "F", "has_goggle_hook": "yes",
                             "races": [5, 6], "genders": [0, 1], "n_utc": 2,
                             "head_items": ["hel_a", "hel_b"]})

    def test_count_mismatch_raises(self):
        utc_rows = [utc("a", "5", "6", "0", has_head="1"), utc("b", "5", "6", "0")]
        wear = [{"row": "5", "label": "L", "modeltype": "F", "has_goggle_hook": "no",
                 "n_utc": "3", "n_utc_with_head_item": "1"}]
        with self.assertRaises(ValueError):
            B.aggregate_appearances(wear, utc_rows)


class ItemTests(unittest.TestCase):
    def test_items_from_rows_filters_and_lowercases(self):
        item_rows = [
            {"resref": "Hel_A", "baseitem_id": "44", "model_resref": "I_Helm_01"},
            {"resref": "belt", "baseitem_id": "45", "model_resref": "I_Belt_01"},
            {"resref": "droid_x", "baseitem_id": "72", "model_resref": ""},
        ]
        uti = {"hel_a": (44, [(0x3D, 1), (0x01, 0)]), "droid_x": (72, [])}
        items, problems = B.items_from_rows(item_rows, {44, 69, 70, 71, 72}, uti)
        self.assertEqual(problems, [])
        self.assertEqual(items, {
            "hel_a": {"baseitem": 44, "model": "i_helm_01", "restrictions": [{"type": 0x3D, "subtype": 1}]},
            "droid_x": {"baseitem": 72, "model": "", "restrictions": []},
        })

    def test_missing_and_mismatched_uti_are_problems(self):
        item_rows = [
            {"resref": "gone", "baseitem_id": "44", "model_resref": ""},
            {"resref": "moved", "baseitem_id": "44", "model_resref": ""},
        ]
        uti = {"moved": (45, [])}
        items, problems = B.items_from_rows(item_rows, {44}, uti)
        self.assertEqual(items, {})
        self.assertEqual(len(problems), 2)


class JsonTests(unittest.TestCase):
    def test_dump_is_identical_regardless_of_insertion_order(self):
        base = {"label": "Mask", "droid": False}
        doc1 = {"schema": 1, "baseitems": {"44": dict(base)}, "items": {}, "appearances": {}}
        doc2 = {"appearances": {}, "items": {}, "baseitems": {"44": dict(reversed(list(base.items())))},
                "schema": 1}
        self.assertEqual(B.dump_json(doc1), B.dump_json(doc2))
        self.assertEqual(B.dump_json(doc1), B.dump_json(doc1))

    def test_keys_sorted_and_roundtrip(self):
        doc = {"schema": 1, "items": {"b": {}, "a": {}}, "baseitems": {}, "appearances": {}}
        text = B.dump_json(doc)
        self.assertLess(text.index('"appearances"'), text.index('"baseitems"'))
        self.assertLess(text.index('"items"'), text.index('"schema"'))
        self.assertLess(text.index('"a"'), text.index('"b"'))
        self.assertEqual(json.loads(text), doc)
        self.assertTrue(text.endswith("\n"))


class TsvTests(unittest.TestCase):
    def test_read_tsv_and_strict_width(self):
        with tempfile.TemporaryDirectory() as td:
            good = Path(td) / "good.tsv"
            good.write_text("a\tb\n1\t\n2\t3\n", encoding="utf-8")
            self.assertEqual(B.read_tsv(good), [{"a": "1", "b": ""}, {"a": "2", "b": "3"}])
            bad = Path(td) / "bad.tsv"
            bad.write_text("a\tb\n1\n", encoding="utf-8")
            with self.assertRaises(ValueError):
                B.read_tsv(bad)

    def test_main_refuses_output_inside_game_dir(self):
        with tempfile.TemporaryDirectory() as td:
            k2 = Path(td) / "game"
            k2.mkdir()
            self.assertEqual(B.main(["--k2", str(k2), "--out", str(k2 / "x.json")]), 2)


if __name__ == "__main__":
    unittest.main()
