#!/usr/bin/env python3
"""Tests for k1_port_scan.py (recipe auto-derivation + NPC discovery). Plain python3, stdlib only: no pykotor, no game
install. A fake K1 dir holds JSON fixtures (UTC, 2DA) and tiny binary stand-ins (MDL, SSF, textures); FixtureReader is the
test double for PykotorReader. test_k1_port_import.py reuses K1Data + FixtureReader for its fake interpreter.

Run: python3 -u test_k1_port_scan.py   (exit 0 = all PASS)
"""
from __future__ import annotations

import contextlib
import io
import json
import re
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import k1_port_recipe as R  # noqa: E402
import k1_port_scan as S  # noqa: E402

APP_HEADERS = ["label", "race", "modeltype", "normalhead", "modela", "texa", "modelb", "texb"]
HEAD_HEADERS = ["head", "headtexe"]
SND_HEADERS = ["label", "resref", "strref", "gender", "type"]


class FixtureReader:
    """S.PykotorReader's contract over a fake K1 dir: <k1>/Override, <k1>/_bif (chitin), <k1>/modules, each searched in
    that order. UTC = <name>.utc.json, 2DA = <name>.2da (JSON {"headers","rows"}), MDL = binary with NUL-terminated
    texture names after the first token, SSF / textures = bare files."""

    DIRS = ("Override", "_bif", "modules")

    def __init__(self, k1):
        self.k1 = Path(k1)

    def _find(self, name):
        for d in self.DIRS:
            f = self.k1 / d / name
            if f.is_file():
                return f, d
        return None, None

    def find_utc(self, resref):
        f, d = self._find(f"{resref.lower()}.utc.json")
        if f is None:
            return None
        return {"utc": resref.lower(), "source": f"{d}:{resref.lower()}", **json.loads(f.read_text())}

    def iter_utcs(self):
        seen = set()
        for d in self.DIRS:
            for f in sorted((self.k1 / d).glob("*.utc.json")) if (self.k1 / d).is_dir() else []:
                key = f.name[:-len(".utc.json")]
                if key not in seen:
                    seen.add(key)
                    yield {"utc": key, "source": f"{d}:{key}", **json.loads(f.read_text())}

    def twoda_row(self, name, idx):
        f, _ = self._find(f"{name}.2da")
        if f is None:
            return None
        t = json.loads(f.read_text())
        if idx < 0 or idx >= len(t["rows"]):
            return None
        return dict(zip(t["headers"], t["rows"][idx]))

    def mdl_textures(self, resref):
        f, _ = self._find(f"{resref}.mdl")
        if f is None:
            return None
        out = []
        for tok in f.read_bytes().split(b"\0")[1:]:
            t = tok.decode("latin1").lower()
            if re.fullmatch(r"[a-z0-9_]{1,16}", t) and t != "null" and t not in out:
                out.append(t)
        return out

    def texture_exists(self, name):
        return any(self._find(f"{name}.{ext}")[0] is not None for ext in ("tpc", "tga"))

    def ssf_exists(self, name):
        return self._find(f"{name}.ssf")[0] is not None


class K1Data:
    """Accumulates fake K1 creatures + the 2DA rows they need, then writes the fixture files."""

    def __init__(self):
        self.app = [["filler0", "", "S", "", "", "", "", ""], ["filler1", "", "S", "", "", "", "", ""]]
        self.heads = [["fh0", ""], ["fh1", ""]]
        self.snd = [["s0", "", "-1", "0", ""], ["s1", "", "-1", "1", ""]]
        self.files: dict[tuple[str, str], bytes] = {}

    def creature(self, stem, *, where="_bif", resref=None, gender=0, modeltype="B", head=True, bodies=("ba", "bb"),
                 ssf=True, soundset=True, conv="k_h%s_dialog", first="", last="", tag=None, textures=None,
                 missing_models=(), no_tex_files=()):
        """Create creature template p_<stem> with models p_<stem>ba/bb/h. Returns the UTC resref."""
        resref = resref or f"p_{stem}"
        names = {b: f"p_{stem}{b}" for b in bodies}
        hname = f"p_{stem}h"
        app_id = len(self.app)
        head_id = len(self.heads)
        self.heads.append([hname, ""])
        self.app.append([f"Party_{stem}", names.get("bb", ""), modeltype, str(head_id) if head else "",
                         names.get("ba", ""), names.get("ba", ""), names.get("bb", ""), names.get("bb", "")])
        ss_id = 65535
        if soundset:
            ss_id = len(self.snd)
            self.snd.append([f"Snd_{stem}", f"p_{stem}" if ssf else "", "-1", str(gender), ""])
            if ssf:
                self.files[("_bif", f"p_{stem}.ssf")] = b"SSF V1.1" + b"\0" * 164
        tex = {"ba": [f"p_{stem}ba01", f"p_{stem}h01"], "bb": [f"p_{stem}bb01", f"p_{stem}h01"], "h": [f"p_{stem}h01"]}
        tex.update(textures or {})
        for key, model in (*[(b, names[b]) for b in bodies], ("h", hname)):
            if model in missing_models:
                continue
            body = f"K1:{model}.mdl".encode() + b"".join(b"\0" + t.upper().encode() for t in tex.get(key, [])) + b"\0"
            self.files[("_bif", f"{model}.mdl")] = body
            self.files[("_bif", f"{model}.mdx")] = b"MDX"
            for t in tex.get(key, []):
                if t not in no_tex_files:
                    self.files[("_bif", f"{t}.tpc")] = f"K1TEX:{t}".encode()
        utc = {"tag": tag if tag is not None else stem.capitalize(), "first_name": first or stem.capitalize(), "last_name": last,
               "appearance_id": app_id, "soundset_id": ss_id, "gender_id": gender,
               "conversation": conv % stem if "%s" in conv else conv}
        self.files[(where, f"{resref}.utc.json")] = json.dumps(utc).encode()
        return resref

    def write(self, k1):
        k1 = Path(k1)
        for (where, name), data in self.files.items():
            (k1 / where).mkdir(parents=True, exist_ok=True)
            (k1 / where / name).write_bytes(data)
        (k1 / "_bif").mkdir(parents=True, exist_ok=True)
        for name, headers, rows in (("appearance", APP_HEADERS, self.app), ("heads", HEAD_HEADERS, self.heads),
                                    ("soundset", SND_HEADERS, self.snd)):
            (k1 / "_bif" / f"{name}.2da").write_text(json.dumps({"headers": headers, "rows": rows}))


class Base(unittest.TestCase):
    def setUp(self):
        self._td = tempfile.TemporaryDirectory(prefix="k1port_scan_test_")
        self.addCleanup(self._td.cleanup)
        self.k1 = Path(self._td.name) / "k1"
        self.k1.mkdir()
        self.data = K1Data()

    def reader(self):
        self.data.write(self.k1)
        return FixtureReader(self.k1)

    def facts(self, resref):
        return S.gather_facts(self.reader(), resref)

    def derive(self, resref, rid=None):
        return S.derive_text(self.facts(resref), rid)


class NamingTests(unittest.TestCase):
    def test_auto_id_strips_a_leading_p_only(self):
        self.assertEqual(S.auto_id("p_carth"), "carth")
        self.assertEqual(S.auto_id("P_Carth"), "carth")
        self.assertEqual(S.auto_id("n_commoner01"), "n_commoner01")
        self.assertEqual(S.auto_id("p_"), "p_")
        self.assertEqual(S.auto_id("p_carth"), R.utc_entry_id("p_carth"))

    def test_k2_stem(self):
        self.assertEqual(S.k2_stem("p_carthba"), "k1p_carthba")
        self.assertEqual(S.k2_stem("p_carthba01"), "k1p_carthba01")
        self.assertEqual(S.k2_stem("P_CarthH"), "k1p_carthh")
        self.assertEqual(S.k2_stem("n_commoner_a_m01"), "k1p_n_commoner_a")  # cut to 16
        self.assertEqual(len(S.k2_stem("x" * 40)), 16)
        self.assertEqual(S.k2_stem("dan13_juhani"), "k1p_dan13_juhani")

    def test_utc_and_ssf_names(self):
        self.assertEqual(S.utc_name("carth"), "k1p_carth")
        self.assertEqual(S.ssf_name("k1p_carth"), "k1p_carthssf")
        self.assertEqual(len(S.utc_name("a" * 31)), 16)
        self.assertEqual(S.ssf_name("k1p_" + "a" * 12), "k1p_" + "a" * 9 + "ssf")  # the utc part is cut, the suffix stays
        self.assertEqual(len(S.ssf_name("k1p_" + "a" * 12)), 16)

    def test_collisions_fail_with_a_hint(self):
        self.assertEqual(S.claim_names([("a", "k1p_a"), ("a", "k1p_a"), ("b", "k1p_b")]), {"k1p_a": "a", "k1p_b": "b"})
        with self.assertRaises(S.ScanError) as cm:
            S.claim_names([("texture n_commoner_a_m01", "k1p_n_commoner_a"), ("texture n_commoner_a_f01", "k1p_n_commoner_a")])
        msg = str(cm.exception)
        self.assertIn("k1p_n_commoner_a", msg)
        self.assertIn("two K1 names", msg)
        self.assertIn("recipes_dir", msg)

    def test_ascii_name(self):
        self.assertEqual(S.ascii_name("Bastila", "x"), "Bastila")
        self.assertEqual(S.ascii_name("Café", "x"), "Cafe")
        self.assertEqual(S.ascii_name("日本", "fallback"), "fallback")
        self.assertEqual(S.ascii_name("", "fallback"), "fallback")
        self.assertEqual(len(S.ascii_name("a" * 100, "x")), 63)


class DeriveTests(Base):
    def test_basic_derivation(self):
        self.data.creature("aut", first="Autman")
        d, text = self.derive("p_aut")
        self.assertEqual((d.id, d.tag, d.utc, d.display_name), ("aut", "k1p_aut", "k1p_aut", "Autman"))
        self.assertEqual(d.body_a, ("p_autba", "k1p_autba"))
        self.assertEqual(d.body_b, ("p_autbb", "k1p_autbb"))
        self.assertEqual(d.head, ("p_auth", "k1p_auth"))
        self.assertEqual((d.template_row, d.gender, d.soundset), (17, 0, 11))
        self.assertEqual(d.audio, ("p_aut", "k1p_autssf"))
        self.assertEqual(d.dlg, "k_haut_dialog")
        # body textures first, then the head's own (the head texture is also used by both body meshes)
        self.assertEqual(d.textures, [("p_autba01", "k1p_autba01", ""), ("p_autbb01", "k1p_autbb01", ""),
                                      ("p_auth01", "k1p_auth01", "clamp 1")])

    def test_rendered_recipe_loads_and_matches(self):
        self.data.creature("aut")
        d, text = self.derive("p_aut")
        p = Path(self._td.name) / "aut.toml"
        p.write_text(text)
        r = R.load_recipe(p)
        self.assertEqual((r.id, r.converter, r.gender, r.template_row, r.soundset, r.dlg), ("aut", "mdlops", 0, 17, 11, ""))
        self.assertEqual([(t.k1, t.k2, t.force_opaque, t.txi) for t in r.textures],
                         [("p_autba01", "k1p_autba01", True, ""), ("p_autbb01", "k1p_autbb01", True, ""),
                          ("p_auth01", "k1p_auth01", True, "clamp 1")])
        self.assertEqual((r.audio.k1_ssf, r.audio.k2_ssf), ("p_aut", "k1p_autssf"))
        self.assertIn("# dialog: k_haut_dialog", text)
        self.assertNotRegex(text, r"(?m)^\[dialog\]")

    def test_deterministic(self):
        self.data.creature("aut")
        self.assertEqual(self.derive("p_aut")[1], self.derive("p_aut")[1])
        self.assertNotRegex(self.derive("p_aut")[1], r"20\d\d-\d\d")  # no dates in the file

    def test_female_gets_the_female_fallback_soundset(self):
        self.data.creature("fem", gender=1)
        d, _ = self.derive("p_fem")
        self.assertEqual((d.gender, d.soundset), (1, 57))

    def test_other_gender_ids_map_to_male(self):
        self.data.creature("oth", gender=2)
        self.assertEqual(self.derive("p_oth")[0].gender, 0)

    def test_no_soundset_means_no_audio(self):
        self.data.creature("mut", soundset=False)
        d, text = self.derive("p_mut")
        self.assertIsNone(d.audio)
        self.assertNotRegex(text, r"(?m)^\[audio\]")
        self.assertIn("soundset_id 65535 is not a K1 soundset.2da row", text)
        R.parse_recipe(__import__("tomllib").loads(text), Path("mut.toml"))

    def test_ssf_missing_in_the_install_means_no_audio(self):
        self.data.creature("nos", ssf=False)
        d, text = self.derive("p_nos")
        self.assertIsNone(d.audio)
        self.assertIn("names no SSF", text)
        self.data.creature("gone")
        self.data.files.pop(("_bif", "p_gone.ssf"))
        d2, text2 = self.derive("p_gone")
        self.assertIsNone(d2.audio)
        self.assertIn("p_gone.ssf is not in the K1 install", text2)

    def test_id_override_and_long_ids(self):
        self.data.creature("aut")
        d, _ = self.derive("p_aut", "mine")
        self.assertEqual((d.id, d.tag, d.utc, d.audio[1]), ("mine", "k1p_mine", "k1p_mine", "k1p_minessf"))
        long_id = "a" * 31
        d, text = self.derive("p_aut", long_id)
        self.assertEqual((len(d.utc), len(d.audio[1])), (16, 16))
        R.parse_recipe(__import__("tomllib").loads(text), Path(f"{long_id}.toml"))
        with self.assertRaises(S.ScanError) as cm:
            self.derive("p_aut", "Bad-Id")
        self.assertIn("bad id", str(cm.exception))

    def test_non_humanoid_is_refused(self):
        self.data.creature("drd", modeltype="S")
        with self.assertRaises(S.ScanError) as cm:
            self.derive("p_drd")
        msg = str(cm.exception)
        self.assertIn("model type 'S'", msg)
        self.assertIn("Fix:", msg)

    def test_no_head_row_is_refused(self):
        self.data.creature("nohead", head=False)
        with self.assertRaises(S.ScanError) as cm:
            self.derive("p_nohead")
        self.assertIn("no usable heads.2da row", str(cm.exception))

    def test_bad_appearance_row(self):
        self.data.creature("aut")
        f = self.facts("p_aut")
        f["appearance"] = None
        with self.assertRaises(S.ScanError) as cm:
            S.derive(f)
        self.assertIn("not a row of the K1 appearance.2da", str(cm.exception))

    def test_missing_model_is_reported(self):
        self.data.creature("aut", missing_models=("p_autbb",))
        with self.assertRaises(S.ScanError) as cm:
            self.derive("p_aut")
        self.assertIn("p_autbb.mdl is not in the K1 install", str(cm.exception))

    def test_single_body_model(self):
        self.data.creature("one", bodies=("ba",))
        d, text = self.derive("p_one")
        self.assertIsNone(d.body_b)
        self.assertNotIn("body_b", text)
        self.assertEqual([t[0] for t in d.textures], ["p_oneba01", "p_oneh01"])

    def test_textures_missing_from_k1_are_skipped_and_noted(self):
        self.data.creature("aut", textures={"ba": ["p_autba01", "p_auth01", "ghosttex"]}, no_tex_files=("ghosttex",))
        d, text = self.derive("p_aut")
        self.assertNotIn("ghosttex", [t[0] for t in d.textures])
        self.assertIn("ghosttex", text)
        self.assertIn("skipped", text)

    def test_mesh_without_any_texture_is_refused(self):
        self.data.creature("aut", textures={"ba": [], "bb": [], "h": []})
        with self.assertRaises(S.ScanError) as cm:
            self.derive("p_aut")
        self.assertIn("reference no texture", str(cm.exception))

    def test_truncation_collision_fails_not_silently(self):
        self.data.creature("aut", textures={"ba": ["n_commoner_a_m01", "n_commoner_a_f01", "p_auth01"]})
        with self.assertRaises(S.ScanError) as cm:
            self.derive("p_aut")
        self.assertIn("k1p_n_commoner_a", str(cm.exception))

    def test_display_name_fallbacks(self):
        self.data.creature("aut", first="", last="Last", tag="TAG")
        self.data.files[("_bif", "p_aut.utc.json")] = json.dumps({**json.loads(self.data.files[("_bif", "p_aut.utc.json")]),
                                                                  "first_name": ""}).encode()
        self.assertEqual(self.derive("p_aut")[0].display_name, "Last")
        self.data.files[("_bif", "p_aut.utc.json")] = json.dumps({**json.loads(self.data.files[("_bif", "p_aut.utc.json")]),
                                                                  "last_name": ""}).encode()
        self.assertEqual(self.derive("p_aut")[0].display_name, "TAG")

    def test_unknown_creature_has_no_facts(self):
        self.assertIsNone(self.facts("nothing_here"))

    def test_override_beats_chitin(self):
        self.data.creature("aut", where="_bif")
        self.data.files[("Override", "p_aut.utc.json")] = json.dumps(
            {**json.loads(self.data.files[("_bif", "p_aut.utc.json")]), "first_name": "Modded"}).encode()
        f = self.facts("p_aut")
        self.assertEqual((f["first_name"], f["source"]), ("Modded", "Override:p_aut"))

    def test_module_utc_is_found(self):
        self.data.creature("aut", where="modules", resref="p_aut001")
        f = self.facts("p_aut001")
        self.assertEqual(f["source"], "modules:p_aut001")
        self.assertEqual(self.derive("p_aut001")[0].id, "aut001")


class GroundTruthTests(Base):
    """A fixture shaped like the real K1 Carth / Juhani must derive what the committed hand recipes say."""

    def real_shape(self, stem, gender, conv):
        self.data.creature(stem, gender=gender, conv=conv, first=stem.capitalize(), tag=stem.capitalize())
        return self.derive(f"p_{stem}")[0]

    def check(self, stem, gender, conv):
        d = self.real_shape(stem, gender, conv)
        hand = R.load_recipe(HERE / "recipes" / f"{stem}.toml")
        self.assertEqual(d.id, hand.id)
        self.assertEqual((d.body_a[0], d.body_a[1]), (hand.body_a.k1, hand.body_a.k2))
        self.assertEqual((d.body_b[0], d.body_b[1]), (hand.body_b.k1, hand.body_b.k2))
        self.assertEqual((d.head[0], d.head[1]), (hand.head.k1, hand.head.k2))
        self.assertEqual(d.textures, [(t.k1, t.k2, t.txi) for t in hand.textures])
        self.assertEqual(d.gender, hand.gender)
        self.assertEqual((d.tag, d.utc, d.template_row), (hand.tag, hand.utc, hand.template_row))
        if hand.audio is not None:  # recipes/juhani.toml has no [audio] (the hand path uses its own soundset row)
            self.assertEqual(d.audio, (hand.audio.k1_ssf, hand.audio.k2_ssf))
        else:
            self.assertEqual(d.audio, (f"p_{stem}", f"k1p_{stem}ssf"))
        self.assertEqual(d.display_name, hand.display_name)

    def test_carth(self):
        self.check("carth", 0, "k_hcar_dialog")

    def test_juhani(self):
        self.check("juhani", 1, "k_hjuh_dialog")


class NpcTests(Base):
    def setUp(self):
        super().setUp()
        self.data.creature("carth", first="Carth", tag="Carth")
        self.data.creature("bastila", first="Bastila", tag="Bastila", gender=1)
        self.data.creature("carth001", where="modules", resref="p_carth001", soundset=False, first="Carth")
        self.data.creature("drd", modeltype="S", first="Droid", tag="HK47")

    def rows(self):
        return S.list_npcs(self.reader())

    def test_rows_and_columns(self):
        rows = {r["utc"]: r for r in self.rows()}
        self.assertEqual(sorted(rows), ["p_bastila", "p_carth", "p_carth001", "p_drd"])
        r = rows["p_carth"]
        self.assertEqual((r["tag"], r["name"], r["type"], r["models"]), ("Carth", "Carth", "B", "p_carthba+p_carthbb/p_carthh"))
        self.assertTrue(r["soundset"].endswith(":p_carth"))
        self.assertEqual((r["conversation"], r["source"]), ("k_hcarth_dialog", "_bif:p_carth"))
        self.assertEqual(rows["p_carth001"]["soundset"], "")
        self.assertEqual((rows["p_drd"]["type"], rows["p_drd"]["models"]), ("S", "p_drdba+p_drdbb"))  # no head shown for non-B

    def test_grep_matches_resref_tag_and_name_case_insensitively(self):
        rows = self.rows()
        self.assertEqual([r["utc"] for r in rows if S.match_npc(r, "CARTH")], ["p_carth", "p_carth001"])
        self.assertEqual([r["utc"] for r in rows if S.match_npc(r, "hk47")], ["p_drd"])  # tag
        self.assertEqual([r["utc"] for r in rows if S.match_npc(r, "^bast")], ["p_bastila"])  # regex
        self.assertEqual([r["utc"] for r in rows if S.match_npc(r, "(")], [])  # invalid regex falls back to plain text

    def test_format_table(self):
        lines = S.format_npcs(self.rows())
        self.assertEqual(len(lines), 5)
        self.assertTrue(lines[0].startswith("utc"))
        self.assertIn("p_carthba+p_carthbb/p_carthh", lines[2])
        self.assertEqual(len({line.index("p_") if "p_" in line else 0 for line in lines[1:]}), 1)  # aligned first column

    def test_override_listed_once_with_override_source(self):
        self.data.files[("Override", "p_carth.utc.json")] = self.data.files[("_bif", "p_carth.utc.json")]
        rows = {r["utc"]: r for r in self.rows()}
        self.assertEqual(rows["p_carth"]["source"], "Override:p_carth")
        self.assertEqual(len([r for r in self.rows() if r["utc"] == "p_carth"]), 1)


class HelperTests(Base):
    def run_helper(self, *argv):
        self.data.write(self.k1)
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            rc = S.helper_main(list(argv), reader_factory=FixtureReader)
        return rc, out.getvalue()

    def test_derive_op_prints_one_json_line_for_all_resrefs(self):
        self.data.creature("aut")
        rc, out = self.run_helper("derive", str(self.k1), "p_aut", "nope")
        self.assertEqual(rc, 0)
        got = json.loads(out.strip().splitlines()[-1])
        self.assertEqual(sorted(got["facts"]), ["nope", "p_aut"])
        self.assertIsNone(got["facts"]["nope"])
        self.assertEqual(got["facts"]["p_aut"]["models"]["p_autba"]["textures"], ["p_autba01", "p_auth01"])

    def test_npcs_op(self):
        self.data.creature("aut")
        rc, out = self.run_helper("npcs", str(self.k1))
        self.assertEqual(rc, 0)
        self.assertEqual([r["utc"] for r in json.loads(out)["rows"]], ["p_aut"])

    def test_unknown_op_and_errors(self):
        rc, _ = self.run_helper("bogus")
        self.assertEqual(rc, 2)
        err = io.StringIO()
        with contextlib.redirect_stderr(err):
            rc = S.helper_main(["derive", str(self.k1), "p_x"], reader_factory=lambda k: 1 / 0)
        self.assertEqual(rc, 1)
        self.assertIn("error: _scan derive", err.getvalue())


if __name__ == "__main__":
    unittest.main(verbosity=1)
