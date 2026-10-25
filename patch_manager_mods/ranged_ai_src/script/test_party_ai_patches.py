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


class F5DcrStampTest(unittest.TestCase):
    def _f5(self):
        entries = [e for e in P.PATCHES if e[0] == "F5"]
        self.assertEqual(len(entries), 1, "F5")
        return entries[0]

    def test_f5a_entry_is_party_scope_on_generic_file(self):
        entry = self._f5()
        self.assertEqual(entry[1], "party")
        self.assertEqual(entry[2], P.G)

    def test_f5b_party_protos_define_dcr_stamp(self):
        self.assertIn("int SW_NUMBER_PARTY_DCR_STAMP = 22;", P.PARTY_PROTOS)

    def test_f5c_new_text_has_window_and_skip_limit(self):
        new = self._f5()[4]
        self.assertIn("DelayCommand(0.15", new)
        self.assertIn("nPartyStamp < 4", new)


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


# Trace coverage (combat AI logging): every PATCHES entry that changes a decision leaves a trace line when it acts.
# Existing action prints use "Party <ID>:"; the DEBUG-class prints added for the silent entries use the exact prefix
# below (companion-ai filters "Party D " lines when its debug_log=0, so the prefix is a contract).
PARTY_D_PREFIX = "GENERIC DEBUG *************** Party D "
PRINT_LITERAL = re.compile(r'"((?:[^"\\]|\\.)*)"')

# Entries with no print of their own, one reason each.
SILENT_BY_DESIGN = {
    "MOD8-proto": "prototypes and a constant, no runtime decision",
    "H2c": "ratio operand fix inside an else-if condition; the returned member prints vanilla 'Return oP1'",
    "H2d": "ratio operand fix inside an else-if condition; the returned member prints vanilla 'Return oP2'",
    "M2": "runs inside the per-object AoE scan loop (GN_FindAOETarget); a print would fire once per scanned object",
    "M4d": "counter decrement only (nRoll--), no pick; the pick itself prints in M4c",
    "L4": "else-if condition inside a buff-tree chain with no statement slot; PC-only trees, party members never reach it",
}
# Entries whose new text only calls a helper in party_ai_fixes.nss; the helper prints.
HELPER_PRINTS = {"E1": "Party E1:", "E3": "Party E3:"}


class TraceCoverageTest(unittest.TestCase):
    def _prints(self, new):
        return [m for m in PRINT_LITERAL.findall(new) if "GENERIC DEBUG" in m and "Party " in m]

    def test_g_every_patch_prints_or_is_silent_by_design(self):
        ids = [e[0] for e in P.PATCHES]
        self.assertEqual(len(ids), len(set(ids)))
        for pid, scope, fname, old, new in P.PATCHES:
            with self.subTest(patch=pid):
                prints = self._prints(new)
                if pid in SILENT_BY_DESIGN:
                    self.assertEqual(prints, [], pid + " is listed silent but prints")
                elif pid in HELPER_PRINTS:
                    self.assertEqual(prints, [], pid + " prints in the patch itself; drop it from HELPER_PRINTS")
                else:
                    self.assertTrue(prints, pid + " leaves no 'Party ' trace line")

    def test_g_silent_and_helper_sets_name_real_entries(self):
        ids = {e[0] for e in P.PATCHES}
        self.assertLessEqual(set(SILENT_BY_DESIGN), ids)
        self.assertLessEqual(set(HELPER_PRINTS), ids)
        self.assertFalse(set(SILENT_BY_DESIGN) & set(HELPER_PRINTS))
        for pid, text in HELPER_PRINTS.items():
            with open(FIXES, encoding="utf-8") as fh:
                self.assertIn(text, fh.read(), pid)

    def test_g_party_d_prints_use_exact_prefix_and_a_patch_id(self):
        ids = {e[0] for e in P.PATCHES}
        seen = set()
        for pid, scope, fname, old, new in P.PATCHES:
            for lit in PRINT_LITERAL.findall(new):
                if "Party D" not in lit:
                    continue
                with self.subTest(patch=pid, text=lit[:60]):
                    self.assertTrue(lit.startswith(PARTY_D_PREFIX), lit)
                    tag = lit[len(PARTY_D_PREFIX):].split(":", 1)[0]
                    self.assertIn(tag, ids, lit)
                    self.assertTrue(lit[len(PARTY_D_PREFIX) + len(tag):].startswith(": "), lit)
                    seen.add(tag)
        # every previously silent entry (audit gaps 1-8) now has a Party D print, except the by-design ones
        gaps = {"MEDPAC-a", "M5", "M5b", "ANTIDOTE", "H1a", "H2a", "H2b", "H2e", "M3", "M9", "M4b", "M4c", "M7a",
                "M7b", "E6", "E13", "E10", "E11", "M1", "M10b", "MOD8-dispatch"}
        self.assertEqual(gaps - seen, set())

    def test_g_medpac_a_print_sits_in_the_party_closing_branch(self):
        new = [e for e in P.PATCHES if e[0] == "MEDPAC-a"][0][4]
        i = new.index("Party D MEDPAC-a: medpac/repair kit branch closed for party member")
        head = new[:new.rfind("GN_MyPrintString", 0, i)]
        self.assertTrue(head.rstrip().endswith("{"), "print must be the first statement of a branch")
        self.assertIn("else if(IsObjectPartyMember(OBJECT_SELF) &&", head)

    def test_g_baseline_mod8_dispatch_stays_untraced(self):
        # MOD8_ONLY must rebuild the live k_ai_master.ncs byte for byte, so it carries no trace line.
        for e in P.MOD8_ONLY:
            self.assertNotIn("Party D", e[4], e[0])


if __name__ == "__main__":
    unittest.main()
