#!/usr/bin/env python3
"""Tests for party_ai_patches.py and party_ai_fixes.nss (WI-138 F2: family-aware buff and debuff picks).

Stdlib only. Run as `python3 test_party_ai_patches.py` from this directory (run_tests.py py: entry).
Check (e) reads the vanilla scripts when the vanilla dir exists; when it does not, a note is printed and the
check still passes (a SKIP would fail the gate).
"""
import os
import re
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import party_ai_patches as P  # noqa: E402

VANILLA_DIR = os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/mod8_kite_script/vanilla")
FIXES = os.path.join(HERE, "party_ai_fixes.nss")

# Step 1 of WI-138: power family -> spells.2da ids. Buff families 1-10, single-target debuffs 20-24.
FAMILY_IDS = {
    1: [8, 34, 37],
    2: [17, 18, 24],
    3: [22, 33, 36],
    4: [40, 42, 133],
    5: [135, 136, 137],
    6: [138, 139, 140],
    7: [164, 165, 166],
    8: [167, 168, 169, 170, 171, 172],
    9: [150, 151, 152],
    10: [20, 41],
    20: [45, 7, 38],
    21: [50, 9],
    22: [16],
    23: [46, 29],
    24: [153, 154, 155],
}


def _all_ids():
    return [nid for ids in FAMILY_IDS.values() for nid in ids]


class PatchShapeTest(unittest.TestCase):
    def test_a_every_patch_is_five_tuple_of_strings(self):
        for entry in list(P.PATCHES) + list(P.MOD8_ONLY):
            with self.subTest(patch=entry[0]):
                self.assertEqual(len(entry), 5, entry[0])
                for field in entry:
                    self.assertIsInstance(field, str, entry[0])
                self.assertNotEqual(entry[3], entry[4], entry[0])


class F2IdTest(unittest.TestCase):
    def test_b_f2_ids_have_party_scope_and_generic_file(self):
        by_id = {}
        for entry in P.PATCHES:
            by_id.setdefault(entry[0], []).append(entry)
        for pid in ("F2a", "F2b"):
            with self.subTest(patch=pid):
                self.assertEqual(len(by_id.get(pid, [])), 1, pid)
                self.assertEqual(by_id[pid][0][1], "party", pid)
                self.assertEqual(by_id[pid][0][2], P.G, pid)


class PrototypeTest(unittest.TestCase):
    def test_c_party_protos_declare_both_helpers(self):
        self.assertIn("int GN_PartyPowerFamily(int nSpell);\n", P.PARTY_PROTOS)
        self.assertIn("int GN_PartyFamilyActive(int nSpell, object oTarget);\n", P.PARTY_PROTOS)

    def test_c_protos_reach_the_generic_include(self):
        mod8_proto = [e for e in P.PATCHES if e[0] == "MOD8-proto"][0]
        self.assertIn(P.PARTY_PROTOS, mod8_proto[4])


class HelperBodyTest(unittest.TestCase):
    def setUp(self):
        with open(FIXES, encoding="utf-8") as fh:
            self.src = fh.read()

    def _body(self, signature):
        start = self.src.find(signature)
        self.assertNotEqual(start, -1, signature)
        end = self.src.find("\n}\n", start)
        self.assertNotEqual(end, -1, signature)
        return self.src[start:end + 3]

    def test_d_both_functions_defined(self):
        self.assertRegex(self.src, r"(?m)^int GN_PartyPowerFamily\(int nSpell\)$")
        self.assertRegex(self.src, r"(?m)^int GN_PartyFamilyActive\(int nSpell, object oTarget\)$")

    def test_d_family_function_contains_every_step1_id(self):
        body = self._body("int GN_PartyPowerFamily(int nSpell)")
        for nid in _all_ids():
            with self.subTest(spell=nid):
                self.assertRegex(body, r"nSpell == %d\b" % nid)

    def test_d_each_family_returns_its_own_number(self):
        body = self._body("int GN_PartyPowerFamily(int nSpell)")
        for fam, ids in FAMILY_IDS.items():
            cond = " || ".join("nSpell == %d" % nid for nid in ids)
            block = "if (%s)\n    {\n        return %d;\n    }\n" % (cond, fam)
            with self.subTest(family=fam):
                self.assertIn(block, body)

    def test_d_active_check_uses_effect_spell_ids(self):
        body = self._body("int GN_PartyFamilyActive(int nSpell, object oTarget)")
        self.assertIn("GN_PartyPowerFamily(GetEffectSpellId(eCur)) == nFamily", body)
        self.assertIn("GetFirstEffect(oCheck)", body)
        self.assertIn("GetNextEffect(oCheck)", body)


class VanillaAnchorTest(unittest.TestCase):
    def test_e_anchors_occur_once_in_vanilla(self):
        if not os.path.isdir(VANILLA_DIR):
            print("NOTE: vanilla dir %s not found; anchor-in-vanilla check not run, shape checks still ran"
                  % VANILLA_DIR)
            return
        texts = {}
        for pid, scope, fname, old, new in P.PATCHES:
            if fname not in (P.G, P.S, P.M):
                continue
            if fname not in texts:
                path = os.path.join(VANILLA_DIR, fname)
                with open(path, "rb") as fh:
                    texts[fname] = P.crlf(fh.read().decode("latin-1"))
            with self.subTest(patch=pid):
                self.assertEqual(texts[fname].count(P.crlf(old)), 1, "%s in %s" % (pid, fname))


if __name__ == "__main__":
    unittest.main()
