#!/usr/bin/env python3
"""Tests for k1_port_recipe.py (recipe + user-config loader). Tempdirs only; shipped recipes are read, never written.

Run: python3 -u test_k1_port_recipe.py   (exit 0 = all PASS)
"""
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import k1_port_recipe as R  # noqa: E402

GOOD = """\
[character]
id = "{id}"
display_name = "Test Guy"
tag = "k1p_{id}"
utc = "{utc}"
dlg = ""
soundset = 11
template_row = 17
gender = 0

[models]
converter = "pykotor"
body_a = {{ k1 = "p_{id}ba", k2 = "{id}_ba" }}
body_b = {{ k1 = "p_{id}bb", k2 = "{id}_bb" }}
head   = {{ k1 = "p_{id}h",  k2 = "{id}_h" }}

[[textures]]
k1 = "p_{id}ba01"
k2 = "{id}_ba01"
force_opaque = true

[[textures]]
k1 = "p_{id}h01"
k2 = "{id}_h01"
force_opaque = true
txi = "clamp 1"
"""


def good(rid="tst", utc=None):
    return GOOD.format(id=rid, utc=utc or f"k1p_{rid}")


class Base(unittest.TestCase):
    def setUp(self):
        self._td = tempfile.TemporaryDirectory(prefix="k1port_recipe_test_")
        self.addCleanup(self._td.cleanup)
        self.dir = Path(self._td.name)

    def write(self, name, text):
        p = self.dir / name
        p.write_text(text, encoding="utf-8")
        return p

    def recipe(self, text, rid="tst"):
        return self.write(f"{rid}.toml", text)

    def bad(self, text, *needles, rid="tst"):
        p = self.recipe(text, rid)
        with self.assertRaises(R.RecipeError) as cm:
            R.load_recipe(p)
        msg = str(cm.exception)
        self.assertIn(p.name, msg)
        for n in needles:
            self.assertIn(n, msg)
        return msg


class ShippedTests(unittest.TestCase):
    def test_shipped_recipes_load(self):
        rs = R.load_recipes(HERE / "recipes", ["carth", "juhani"])
        self.assertEqual([r.id for r in rs], ["carth", "juhani"])
        carth, juhani = rs
        self.assertEqual((carth.converter, carth.soundset, carth.gender), ("mdlops", 14, 0))
        self.assertEqual(carth.audio, R.AudioRef(k1_ssf="p_carth", k2_ssf="k1p_carthssf"))
        self.assertEqual((juhani.converter, juhani.dlg, juhani.gender), ("mdlops", "k1p_juhanidlg", 1))
        self.assertEqual(juhani.audio, R.AudioRef(k1_ssf="p_juhani", k2_ssf="k1p_juhanissf"))
        self.assertEqual((juhani.dialog.k1_dlg, juhani.dialog.k2_dlg), ("k_hjuh_dialog", "k1p_juhanidlg"))
        for r in rs:
            self.assertEqual(r.template_row, {"carth": 6, "juhani": 8}[r.id])  # vanilla K2 party rows
            self.assertEqual(len(r.textures), 3)
            self.assertTrue(all(t.force_opaque for t in r.textures))
            heads = [t for t in r.textures if t.k2.endswith("h01")]
            self.assertEqual([t.txi for t in heads], ["clamp 1"])
            self.assertEqual(r.warnings, ())
            self.assertIsNotNone(r.body_b)
            self.assertIsNotNone(r.head)

    def test_shipped_recipes_have_no_row_ids(self):
        for name in ("carth", "juhani"):
            text = (HERE / "recipes" / f"{name}.toml").read_text(encoding="utf-8")
            self.assertNotIn("appearance_row", text)
            self.assertNotIn("heads_row", text)
            self.assertNotIn("head_row", text)

    def test_example_config_loads(self):
        c = R.load_config(HERE / "k1_imports.example.toml")
        self.assertEqual(c.characters, ("carth", "juhani"))
        self.assertEqual(c.recipes_dir, R.DEFAULT_RECIPES_DIR)
        self.assertNotIn("~", str(c.k1_dir))
        self.assertNotIn("~", str(c.k2_dir))
        self.assertNotIn("~", str(c.out))
        self.assertTrue(str(c.k2_dir).endswith("Knights of the Old Republic II"))
        R.load_recipes(c.recipes_dir, c.characters)


class RecipeRuleTests(Base):
    def test_good_recipe(self):
        r = R.load_recipe(self.recipe(good()))
        self.assertEqual(r.id, "tst")
        self.assertEqual(r.textures[1].txi, "clamp 1")
        self.assertFalse(r.textures[0].txi)
        self.assertEqual(len(r.k2_names()), 6)

    def test_optional_parts(self):
        text = good().replace('body_b = { k1 = "p_tstbb", k2 = "tst_bb" }\n', "").replace(
            'head   = { k1 = "p_tsth",  k2 = "tst_h" }\n', "")
        r = R.load_recipe(self.recipe(text))
        self.assertIsNone(r.body_b)
        self.assertIsNone(r.head)

    def test_force_opaque_default_false(self):
        r = R.load_recipe(self.recipe(good().replace("force_opaque = true\n\n[[", "\n\n[[", 1)))
        self.assertFalse(r.textures[0].force_opaque)

    def test_keep_dangly_default_true_false(self):
        r = R.load_recipe(self.recipe(good()))
        self.assertIs(r.keep_dangly, False)
        self.assertEqual(r.warnings, ())
        r = R.load_recipe(self.recipe(good().replace('converter = "pykotor"', 'converter = "pykotor"\nkeep_dangly = true')))
        self.assertIs(r.keep_dangly, True)
        self.assertEqual(r.warnings, ())
        r = R.load_recipe(self.recipe(good().replace('converter = "pykotor"', 'converter = "pykotor"\nkeep_dangly = false')))
        self.assertIs(r.keep_dangly, False)

    def test_keep_dangly_non_bool_rejected(self):
        for v in ('"yes"', "1", "0", "[true]"):
            self.bad(good().replace('converter = "pykotor"', f'converter = "pykotor"\nkeep_dangly = {v}'),
                     "models.keep_dangly", "true or false")

    def test_keep_dangly_with_mdlops_warns_and_is_ignored(self):
        r = R.load_recipe(self.recipe(good().replace('converter = "pykotor"', 'converter = "mdlops"\nkeep_dangly = true')))
        self.assertIs(r.keep_dangly, False)
        self.assertEqual(len(r.warnings), 1)
        self.assertIn("keep_dangly", r.warnings[0])
        r = R.load_recipe(self.recipe(good().replace('converter = "pykotor"', 'converter = "mdlops"\nkeep_dangly = false')))
        self.assertEqual(r.warnings, ())

    def test_unknown_key_top_level(self):
        self.bad(good() + '\n[extras]\nx = 1\n', "extras", "unknown key")

    def test_unknown_key_nested(self):
        self.bad(good().replace('gender = 0', 'gender = 0\nfoo = 1'), "character.foo", "unknown key")
        self.bad(good().replace('converter = "pykotor"', 'converter = "pykotor"\nfoo = 1'), "models.foo")
        self.bad(good().replace('k2 = "tst_ba" }', 'k2 = "tst_ba", z = 1 }'), "models.body_a.z")
        self.bad(good().replace('txi = "clamp 1"', 'txi = "clamp 1"\nwat = 1'), "textures[1].wat")

    def test_bad_resref(self):
        self.bad(good().replace('k2 = "tst_ba" }', 'k2 = "Tst_BA" }'), "models.body_a.k2", "resref")
        self.bad(good().replace('k2 = "tst_ba" }', 'k2 = "a_name_far_too_long" }'), "models.body_a.k2")
        self.bad(good().replace('k1 = "p_tstba01"', 'k1 = "bad-name"'), "textures[0].k1")
        self.bad(good().replace('utc = "k1p_tst"', 'utc = ""'), "character.utc")
        self.bad(good().replace('dlg = ""', 'dlg = "BAD DLG"'), "character.dlg")

    def test_dlg_resref_accepted(self):
        r = R.load_recipe(self.recipe(good().replace('dlg = ""', 'dlg = "kvjdlg01"')))
        self.assertEqual(r.dlg, "kvjdlg01")

    def test_duplicate_k2(self):
        self.bad(good().replace('k2 = "tst_h" }', 'k2 = "tst_ba" }'), "duplicate k2 name", "tst_ba")
        self.bad(good().replace('k2 = "tst_ba01"', 'k2 = "tst_bb"'), "duplicate k2 name")
        self.bad(good().replace('utc = "k1p_tst"', 'utc = "tst_h"'), "duplicate k2 name")

    def test_duplicate_k1_texture(self):
        self.bad(good().replace('k1 = "p_tsth01"', 'k1 = "p_tstba01"'), "duplicate k1 texture")

    def test_cross_recipe_collision(self):
        self.recipe(good("one"), "one")
        # second recipe reuses one's body_a k2 name
        two = good("two").replace('k2 = "two_ba"', 'k2 = "one_ba"')
        self.recipe(two, "two")
        with self.assertRaises(R.RecipeError) as cm:
            R.load_recipes(self.dir, ["one", "two"])
        self.assertIn("one_ba", str(cm.exception))
        self.assertIn("collides", str(cm.exception))
        # without the collision the pair loads
        self.recipe(good("two"), "two")
        self.assertEqual([r.id for r in R.load_recipes(self.dir, ["one", "two"])], ["one", "two"])

    def test_load_recipes_id_errors(self):
        self.recipe(good("one"), "one")
        with self.assertRaises(R.RecipeError):
            R.load_recipes(self.dir, ["one", "one"])
        with self.assertRaises(R.RecipeError):
            R.load_recipes(self.dir, ["missing"])
        with self.assertRaises(R.RecipeError):
            R.load_recipes(self.dir, [])

    def test_bad_converter(self):
        self.bad(good().replace('"pykotor"', '"blender"'), "models.converter", "pykotor")

    def test_soundset_auto(self):
        self.bad(good().replace("soundset = 11", 'soundset = "auto"'), "character.soundset", "[audio] table")
        self.bad(good().replace("soundset = 11", 'soundset = "11"'), "character.soundset")
        self.bad(good().replace("soundset = 11", "soundset = -1"), "character.soundset")
        self.bad(good().replace("soundset = 11", "soundset = true"), "character.soundset")

    def test_row_id_keys_rejected(self):
        for key in ("appearance_row", "head_row", "heads_row"):
            self.bad(good().replace("gender = 0", f"gender = 0\n{key} = 818"), f"character.{key}", "assigned by the builder")
        self.bad(good().replace('converter = "pykotor"', 'converter = "pykotor"\nappearance_row = 818'),
                 "models.appearance_row", "assigned by the builder")

    def test_id_must_equal_stem(self):
        self.bad(good().replace('id = "tst"', 'id = "other"'), "character.id", "file stem")
        self.bad(good("Up"), "character.id", rid="Up")

    def test_audio_table_parsed(self):
        r = R.load_recipe(self.recipe(good() + '\n[audio]\nk1_ssf = "p_tst"\nk2_ssf = "k1p_tstssf"\n'))
        self.assertEqual(r.audio, R.AudioRef(k1_ssf="p_tst", k2_ssf="k1p_tstssf"))
        self.assertEqual(r.warnings, ())
        self.assertEqual(r.soundset, 11)  # the fallback stays required and parsed
        self.assertEqual(r.k2_names()[-1], "k1p_tstssf")
        self.assertIsNone(R.load_recipe(self.recipe(good())).audio)

    def test_audio_table_rejects(self):
        a = '\n[audio]\nk1_ssf = "p_tst"\nk2_ssf = "k1p_tstssf"\n'
        self.bad(good() + '\n[audio]\nk1_ssf = "p_tst"\n', "audio.k2_ssf", "missing required key")
        self.bad(good() + '\n[audio]\nk2_ssf = "k1p_tstssf"\n', "audio.k1_ssf", "missing required key")
        self.bad(good() + '\n[audio]\n', "audio.k1_ssf", "missing required key")
        self.bad(good() + a.replace("p_tst", "P_Tst", 1), "audio.k1_ssf", "bad resref")
        self.bad(good() + a.replace("k1p_tstssf", "k1p_far_too_long_ssf"), "audio.k2_ssf", "bad resref")
        self.bad(good() + a.replace('"k1p_tstssf"', "5"), "audio.k2_ssf", "must be a string")
        self.bad(good() + a + 'barks = ["x"]\n', "audio.barks", "unknown key")
        self.bad(good() + a.replace("k1p_tstssf", "tst_ba"), "audio.k2_ssf", "duplicate k2 name")
        self.bad(good() + a.replace("k1p_tstssf", "k1p_tst"), "audio.k2_ssf", "duplicate k2 name")  # clashes with the utc
        self.bad('audio = 3\n' + good(), "audio", "must be a table")

    def test_dialog_table_parsed(self):
        r = R.load_recipe(self.recipe(good() + '\n[dialog]\nk1_dlg = "k_htst_dialog"\nk2_dlg = "k1p_tstdlg"\nmax_voiced = 12\n'))
        self.assertEqual(r.dialog, R.DialogRef(k1_dlg="k_htst_dialog", k2_dlg="k1p_tstdlg", max_voiced=12))
        self.assertEqual(r.dlg, "k1p_tstdlg")  # the UTC conversation / default_dlg is the served dlg
        self.assertEqual(r.k2_names()[-1], "k1p_tstdlg")
        self.assertIsNone(R.load_recipe(self.recipe(good())).dialog)

    def test_dialog_defaults(self):
        r = R.load_recipe(self.recipe(good() + '\n[dialog]\nk2_dlg = "k1p_tstdlg"\n'))
        self.assertEqual(r.dialog, R.DialogRef(k1_dlg="", k2_dlg="k1p_tstdlg", max_voiced=30))
        r = R.load_recipe(self.recipe(good().replace('dlg = ""', 'dlg = "k1p_tstdlg"') + '\n[dialog]\nk2_dlg = "k1p_tstdlg"\n'))
        self.assertEqual(r.dlg, "k1p_tstdlg")  # character.dlg may repeat k2_dlg
        for n in (0, 80):
            r = R.load_recipe(self.recipe(good() + f'\n[dialog]\nk2_dlg = "k1p_tstdlg"\nmax_voiced = {n}\n'))
            self.assertEqual(r.dialog.max_voiced, n)

    def test_dialog_table_rejects(self):
        d = '\n[dialog]\nk1_dlg = "k_htst_dialog"\nk2_dlg = "k1p_tstdlg"\n'
        self.bad(good() + '\n[dialog]\nk1_dlg = "k_htst_dialog"\n', "dialog.k2_dlg", "missing required key")
        self.bad(good() + '\n[dialog]\n', "dialog.k2_dlg", "missing required key")
        self.bad(good() + d.replace("k_htst_dialog", "K_Htst"), "dialog.k1_dlg", "bad resref")
        self.bad(good() + d.replace("k1p_tstdlg", "k1p_tst_far_too_long_dlg"), "dialog.k2_dlg", "bad resref")
        self.bad(good() + d.replace("k1p_tstdlg", "Bad"), "dialog.k2_dlg", "bad resref")
        self.bad(good() + d + "max_voiced = 81\n", "dialog.max_voiced", "0..80")
        self.bad(good() + d + "max_voiced = -1\n", "dialog.max_voiced", "0..80")
        self.bad(good() + d + 'max_voiced = "30"\n', "dialog.max_voiced", "must be an integer")
        self.bad(good() + d + "max_voiced = true\n", "dialog.max_voiced", "must be an integer")
        self.bad(good() + d + "voices = 3\n", "dialog.voices", "unknown key")
        self.bad(good() + d.replace("k1p_tstdlg", "tst_ba"), "dialog.k2_dlg", "duplicate k2 name")
        self.bad(good() + d.replace("k1p_tstdlg", "k1p_tst"), "dialog.k2_dlg", "duplicate k2 name")  # clashes with the utc
        self.bad(good().replace('dlg = ""', 'dlg = "other_dlg"') + d, "character.dlg", "conflicts with [dialog] k2_dlg")
        self.bad("dialog = 3\n" + good(), "dialog", "must be a table")

    def test_dialog_k2_name_collides_across_recipes(self):
        self.recipe(good("one") + '\n[dialog]\nk2_dlg = "shared_dlg"\n', "one")
        self.recipe(good("two") + '\n[dialog]\nk2_dlg = "shared_dlg"\n', "two")
        with self.assertRaises(R.RecipeError) as cm:
            R.load_recipes(self.dir, ["one", "two"])
        self.assertIn("shared_dlg", str(cm.exception))

    def test_gender_and_ints(self):
        self.bad(good().replace("gender = 0", "gender = 2"), "character.gender")
        self.bad(good().replace("template_row = 17", "template_row = -3"), "character.template_row")
        self.bad(good().replace("template_row = 17", 'template_row = "17"'), "character.template_row")

    def test_names_and_txi(self):
        self.bad(good().replace('display_name = "Test Guy"', 'display_name = ""'), "character.display_name")
        self.bad(good().replace('display_name = "Test Guy"', 'display_name = "café"'), "character.display_name")
        self.bad(good().replace('tag = "k1p_tst"', 'tag = "' + "t" * 32 + '"'), "character.tag")
        self.bad(good().replace('txi = "clamp 1"', "txi = 5"), "textures[1].txi")
        self.bad(good().replace("force_opaque = true\n\n[[", 'force_opaque = "yes"\n\n[[', 1), "textures[0].force_opaque")

    def test_missing_pieces(self):
        self.bad(good().replace('body_a = { k1 = "p_tstba", k2 = "tst_ba" }\n', ""), "models.body_a", "missing")
        self.bad(good().replace('utc = "k1p_tst"\n', ""), "character.utc", "missing")
        self.bad("[models]\nconverter = 'pykotor'\n", "character", "missing")

    def test_invalid_toml(self):
        self.bad("[character\nid=", "invalid TOML")


class CollisionTests(Base):
    def test_check_collisions_across_recipes(self):
        a = R.load_recipe(self.write("a.toml", good("a")))
        b = R.load_recipe(self.write("b.toml", good("b")))
        R.check_collisions([a, b])
        c = R.load_recipe(self.write("c.toml", good("c").replace('"c_ba"', '"a_ba"')))
        with self.assertRaises(R.RecipeError) as cm:
            R.check_collisions([a, b, c])
        self.assertIn("c.toml", str(cm.exception))
        self.assertIn("'a_ba'", str(cm.exception))
        self.assertIn("recipe 'a'", str(cm.exception))
        R.check_collisions([a, a])  # the same recipe twice is not a collision with another recipe

    def test_load_recipes_still_rejects_collisions(self):
        self.write("a.toml", good("a"))
        self.write("c.toml", good("c").replace('"c_ba"', '"a_ba"'))
        with self.assertRaises(R.RecipeError) as cm:
            R.load_recipes(self.dir, ["a", "c"])
        self.assertIn("collides with recipe 'a'", str(cm.exception))

    def test_main_validates_a_config_with_auto_entries(self):
        import contextlib
        import io
        self.write("a.toml", good("a"))
        cfg = self.write("cfg.toml", '[k1]\ndir = "/k1"\n[k2]\ndir = "/k2"\n[build]\nout = "/o"\n'
                         f'characters = ["a", "utc:p_bastila"]\nrecipes_dir = "{self.dir}"\n')
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            rc = R.main(["k1_port_recipe.py", str(cfg)])
        self.assertEqual(rc, 0)
        self.assertIn("auto-derived (K1 UTC, checked at build)=['bastila']", out.getvalue())


class ConfigTests(Base):
    CFG = """\
[k1]
dir = "~/k1"
[k2]
dir = "/opt/k2"
[build]
out = "~/out"
characters = ["carth", "juhani"]
"""

    def cfg(self, text):
        return self.write("cfg.toml", text)

    def bad_cfg(self, text, *needles):
        p = self.cfg(text)
        with self.assertRaises(R.RecipeError) as cm:
            R.load_config(p)
        for n in needles:
            self.assertIn(n, str(cm.exception))

    def test_good_and_tilde(self):
        c = R.load_config(self.cfg(self.CFG))
        self.assertEqual(c.k1_dir, Path("~/k1").expanduser())
        self.assertEqual(c.k2_dir, Path("/opt/k2"))
        self.assertEqual(c.out, Path("~/out").expanduser())
        self.assertEqual(c.recipes_dir, R.DEFAULT_RECIPES_DIR)

    def test_recipes_dir_override(self):
        c = R.load_config(self.cfg(self.CFG + 'recipes_dir = "~/my_recipes"\n'))
        self.assertEqual(c.recipes_dir, Path("~/my_recipes").expanduser())

    def test_optional_build_keys(self):
        c = R.load_config(self.cfg(self.CFG))
        self.assertEqual((c.texture_source, c.first_appearance_row, c.first_head_row), ("vanilla", None, None))
        c = R.load_config(self.cfg(self.CFG + 'texture_source = "cr"\nfirst_appearance_row = 821\nfirst_head_row = 252\n'))
        self.assertEqual((c.texture_source, c.first_appearance_row, c.first_head_row), ("cr", 821, 252))
        c = R.load_config(self.cfg(self.CFG + 'first_soundset_row = 71\n'))
        self.assertEqual((c.first_soundset_row, R.load_config(self.cfg(self.CFG)).first_soundset_row), (71, None))
        self.bad_cfg(self.CFG + 'texture_source = "hd"\n', "build.texture_source", "one of")
        self.bad_cfg(self.CFG + "texture_source = 1\n", "build.texture_source", "string")
        self.bad_cfg(self.CFG + "first_appearance_row = -1\n", "build.first_appearance_row")
        self.bad_cfg(self.CFG + 'first_head_row = "9"\n', "build.first_head_row", "integer")

    def test_unknown_keys(self):
        self.bad_cfg(self.CFG + "extra = 1\n", "build.extra", "unknown key")
        self.bad_cfg(self.CFG.replace('dir = "/opt/k2"', 'dir = "/opt/k2"\nx = 1'), "k2.x")
        self.bad_cfg(self.CFG + "[other]\na = 1\n", "other")

    def test_bad_characters(self):
        self.bad_cfg(self.CFG.replace('["carth", "juhani"]', "[]"), "build.characters", "non-empty")
        self.bad_cfg(self.CFG.replace('["carth", "juhani"]', '"carth"'), "build.characters")
        self.bad_cfg(self.CFG.replace('["carth", "juhani"]', '["carth", "carth"]'), "unique")
        self.bad_cfg(self.CFG.replace('["carth", "juhani"]', '["Carth"]'), "build.characters[0]")
        self.bad_cfg(self.CFG.replace('["carth", "juhani"]', "[1]"), "build.characters[0]")

    def test_utc_entries(self):
        c = R.load_config(self.cfg(self.CFG.replace('["carth", "juhani"]', '["carth", "utc:p_bastila", "utc:dan13_juhani", "p_hk47"]')))
        self.assertEqual(c.characters, ("carth", "bastila", "dan13_juhani", "p_hk47"))  # ids; a plain id stays as written
        self.assertEqual(c.utc_entries, {"bastila": "p_bastila", "dan13_juhani": "dan13_juhani"})  # only the utc: entries
        self.assertEqual(R.load_config(self.cfg(self.CFG)).utc_entries, {})

    def test_bad_utc_entries(self):
        for bad in ('"utc:"', '"utc:P_Bastila"', '"utc:' + "a" * 17 + '"', '"utc:p bastila"', '"utc:p-bastila"'):
            self.bad_cfg(self.CFG.replace('["carth", "juhani"]', f"[{bad}]"), "build.characters[0]", "creature template")
        self.bad_cfg(self.CFG.replace('["carth", "juhani"]', '["carth", "utc:p_carth"]'), "unique", "carth")  # both derive id carth
        self.bad_cfg(self.CFG.replace('["carth", "juhani"]', '["utc:p_a", "utc:p_a"]'), "unique")

    def test_utc_entry_id_rules_apply_to_the_derived_id(self):
        self.assertEqual(R.utc_entry_id("p_carth"), "carth")
        self.assertEqual(R.utc_entry_id("p_"), "p_")
        self.assertEqual(R.utc_entry_id("n_comm_a"), "n_comm_a")
        self.assertTrue(R.ID_RE.fullmatch(R.utc_entry_id("p_" + "a" * 14)))

    def test_example_config_loads(self):
        c = R.load_config(HERE / "k1_imports.example.toml")
        self.assertEqual(c.characters, ("carth", "juhani"))
        text = (HERE / "k1_imports.example.toml").read_text()
        self.assertIn("utc:p_bastila", text)  # the pick-an-NPC form is documented
        c2 = R.load_config(self.cfg(text.replace('characters = ["carth", "juhani"]\n#', 'characters = ["carth", "utc:p_bastila"]\n#', 1)))
        self.assertEqual(c2.utc_entries, {"bastila": "p_bastila"})

    def test_missing_keys(self):
        self.bad_cfg(self.CFG.replace('dir = "~/k1"', ""), "k1.dir", "missing")
        self.bad_cfg(self.CFG.replace('out = "~/out"\n', ""), "build.out", "missing")
        self.bad_cfg(self.CFG.replace('dir = "~/k1"', 'dir = ""'), "k1.dir", "empty")


if __name__ == "__main__":
    unittest.main(verbosity=1)
