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

# Step 1 of WI-138: power family -> spells.2da ids. Buff families 1-11, single-target debuffs 20-24.
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
    11: [180],
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
        self.assertIn("GN_PartyFamilyCovers(GN_PartyPowerFamily(GetEffectSpellId(eCur)), nFamily)", body)
        self.assertIn("GetFirstEffect(oCheck)", body)
        self.assertIn("GetNextEffect(oCheck)", body)

    def test_d_family_covers_is_enlightenment_only(self):
        body = self._body("int GN_PartyFamilyCovers(int nActive, int nFamily)")
        self.assertIn("nActive == 11 && nFamily >= 1 && nFamily <= 3", body)


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
    "O3a": "roll-chain counter only (nRoll--), no pick; the Insanity pick itself is the vanilla block, which prints nothing",
    "L4": "else-if condition inside a buff-tree chain with no statement slot; PC-only trees, party members never reach it",
}
# Entries whose new text only calls a helper in party_ai_fixes.nss; the helper prints.
HELPER_PRINTS = {"E1": "Party E1:", "E3": "Party E3:", "O3": "Party D O3:"}


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


# WI-160 (O3 option B): Horror 30, Insanity 31 and Mass Stasis 44 need 2 unaffected enemies within 4 m of the AoE centre;
# the roll-chain typos at gensupport:3235 and :3310 are fixed for party members only.
# Vanilla select chain, gensupport:3218-3318: (power, flag its pick block tests, flag its decrement block tests).
CHAIN_VANILLA = [
    ("HORROR", "HORROR", "HORROR"),
    ("INSANITY", "INSANITY", "HORROR"),      # :3235 decrement tests nHorror
    ("STORM", "STORM", "STORM"),
    ("WAVE", "WAVE", "WAVE"),
    ("SABER", "SABER", "SABER"),
    ("LIGHTNING", "LIGHTNING", "LIGHTNING"),
    ("SLEEP", "SLEEP", "SLEEP"),
    ("STUNDROID", "STUNDROID", "STUNDROID"),
    ("KILLDROID", "KILLDROID", "KILLDROID"),
    ("HOWL", "KILLDROID", "HOWL"),           # :3310 pick tests nKillDroid
    ("SCREAM1", "SCREAM1", "SCREAM1"),
    ("SCREAM2", "SCREAM2", "SCREAM2"),
    ("SCREAM3", "SCREAM3", "SCREAM3"),
]
CHAIN_PARTY = [
    ("HORROR", "HORROR", "HORROR"),
    ("INSANITY", "INSANITY", "INSANITY"),
    ("STORM", "STORM", "STORM"),
    ("WAVE", "WAVE", "WAVE"),
    ("SABER", "SABER", "SABER"),
    ("LIGHTNING", "LIGHTNING", "LIGHTNING"),
    ("SLEEP", "SLEEP", "SLEEP"),
    ("STUNDROID", "STUNDROID", "STUNDROID"),
    ("KILLDROID", "KILLDROID", "KILLDROID"),
    ("HOWL", "HOWL", "HOWL"),
    ("SCREAM1", "SCREAM1", "SCREAM1"),
    ("SCREAM2", "SCREAM2", "SCREAM2"),
    ("SCREAM3", "SCREAM3", "SCREAM3"),
]


def _chain_pick(chain, flags, roll):
    """Replays the vanilla select chain: a pick block fires at roll 1, a decrement block lowers any roll above 1."""
    picked, nRoll = None, roll
    for name, pick_flag, dec_flag in chain:
        if flags[pick_flag] and nRoll == 1:
            picked = name
            nRoll -= 1
        if flags[dec_flag] and nRoll > 1:
            nRoll -= 1
    return picked


def _chain_mismatches(chain):
    names = [n for n, _, _ in CHAIN_VANILLA]
    bad = 0
    for mask in range(1 << len(names)):
        flags = {name: bool(mask >> i & 1) for i, name in enumerate(names)}
        applicable = [n for n in names if flags[n]]
        for roll in range(1, len(applicable) + 1):
            if _chain_pick(chain, flags, roll) != applicable[roll - 1]:
                bad += 1
    return bad


class O3Test(unittest.TestCase):
    def setUp(self):
        with open(FIXES, encoding="utf-8") as fh:
            self.src = fh.read()

    def _entry(self, pid):
        entries = [e for e in P.PATCHES if e[0] == pid]
        self.assertEqual(len(entries), 1, pid)
        return entries[0]

    def _body(self, signature):
        start = self.src.find(signature)
        self.assertNotEqual(start, -1, signature)
        end = self.src.find("\n}\n", start)
        self.assertNotEqual(end, -1, signature)
        return self.src[start:end + 3]

    def test_h_o3_entries_are_party_only_on_their_files(self):
        self.assertEqual(self._entry("O3")[1:3], ("party", P.G))
        self.assertEqual(self._entry("O3a")[1:3], ("party", P.S))
        self.assertEqual(self._entry("O3b")[1:3], ("party", P.S))

    def test_h_o3_caller_runs_after_both_centre_branches(self):
        old, new = self._entry("O3")[3], self._entry("O3")[4]
        self.assertTrue(old.endswith("        }\n    }\n    else\n"), "anchor must end at the centre branch")
        self.assertIn("        }\n        //Party AI fix O3", new)
        self.assertIn("        tUse = GN_PartyAOEUnaffectedCheck(tUse, oTarget);\n    }\n    else\n", new)

    def test_h_o3_proto_declared_for_the_generic_include(self):
        self.assertIn("talent GN_PartyAOEUnaffectedCheck(talent tUse, object oCentre);\n", P.PARTY_PROTOS)

    def test_h_o3_helper_matches_only_the_three_lasting_powers(self):
        body = self._body("talent GN_PartyAOEUnaffectedCheck(talent tUse, object oCentre)")
        self.assertIn("nSpell != 30 && nSpell != 31 && nSpell != 44", body)
        self.assertIn("IsObjectPartyMember(OBJECT_SELF)", body)
        self.assertIn("GetFirstObjectInShape(SHAPE_SPHERE, 4.0, lCentre)", body)
        self.assertIn("!GN_PartySpellEffectOn(nSpell, oNear)", body)
        self.assertIn("if (nFree >= 2)", body)
        self.assertIn("return tNone;", body)

    def test_h_o3_storm_and_scream_are_not_in_the_helper(self):
        body = self._body("talent GN_PartyAOEUnaffectedCheck(talent tUse, object oCentre)")
        for nid in (25, 159, 160, 161):
            with self.subTest(spell=nid):
                self.assertNotRegex(body, r"\b%d\b" % nid)

    def test_h_o3_effect_walk_is_an_exact_spell_id_match(self):
        body = self._body("int GN_PartySpellEffectOn(int nSpell, object oTarget)")
        self.assertIn("GetEffectSpellId(eCur) == nSpell", body)
        self.assertIn("GetFirstEffect(oTarget)", body)
        self.assertIn("GetNextEffect(oTarget)", body)

    def test_h_o3_drop_print_uses_the_party_d_prefix_and_tag(self):
        self.assertIn(PARTY_D_PREFIX + "O3: spell ", self.src)
        self.assertIn("dropped, ", self.src)

    def test_h_o3a_fixes_the_insanity_decrement_only_for_party(self):
        old, new = self._entry("O3a")[3], self._entry("O3a")[4]
        self.assertIn("//Force Storm\n", old)
        self.assertIn("nHorror == 1", old)
        self.assertIn("(IsObjectPartyMember(OBJECT_SELF) && nInsanity == 1)", new)
        self.assertIn("(!IsObjectPartyMember(OBJECT_SELF) && nHorror == 1)", new)

    def test_h_o3b_fixes_the_sonic_howl_pick_only_for_party(self):
        old, new = self._entry("O3b")[3], self._entry("O3b")[4]
        self.assertIn("nKillDroid == 1 && nRoll == 1", old)
        self.assertIn("(IsObjectPartyMember(OBJECT_SELF) && nHowl == 1)", new)
        self.assertIn("(!IsObjectPartyMember(OBJECT_SELF) && nKillDroid == 1)", new)
        self.assertIn("Party D O3b: Sonic Howl picked", new)

    def test_h_o3_party_roll_chain_picks_the_rolled_power(self):
        self.assertEqual(_chain_mismatches(CHAIN_PARTY), 0)

    def test_h_o3_simulator_detects_the_vanilla_typos(self):
        # Guards the simulator itself: the vanilla chain must fail it, or the party check above proves nothing.
        self.assertGreater(_chain_mismatches(CHAIN_VANILLA), 0)


if __name__ == "__main__":
    unittest.main()
