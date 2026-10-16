"""Unit tests for k1_port_mdlops.rename_ascii and helpers (pure Python, no Proton, no pykotor)."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import k1_port_mdlops as M  # noqa: E402

FIXTURE = """# mdlops ver: 1.0.0 from KOTOR binary source\r
# model P_JuhaniBB\r
filedependancy p_juhanibb NULL.mlk\r
newmodel P_JuhaniBB\r
setsupermodel P_JuhaniBB S_Female03\r
beginmodelgeom P_JuhaniBB\r
node dummy P_JuhaniBB\r
  parent NULL\r
endnode\r
node dummy rootdummy\r
  parent P_JuhaniBB\r
endnode\r
node trimesh lfootT_g\r
  parent rootdummy\r
  bitmap P_JuhaniBB01\r
  bitmap2 NULL\r
  weights 2\r
    lfoot_g 0.5 lfootT_g 0.5\r
endnode\r
node danglymesh CLOTH\r
  parent lfootT_g\r
  bitmap P_JuhaniH01\r
endnode\r
donemodel P_JuhaniBB\r
"""
MAP = {"p_juhanibb": "k1p_juhanibb"}


class RenameTest(unittest.TestCase):
    def setUp(self):
        self.out = M.rename_ascii(FIXTURE, MAP)

    def test_model_names_renamed(self):
        for line in ("newmodel k1p_juhanibb", "beginmodelgeom k1p_juhanibb", "node dummy k1p_juhanibb",
                     "setsupermodel k1p_juhanibb S_Female03", "donemodel k1p_juhanibb",
                     "filedependancy k1p_juhanibb NULL.mlk", "# model k1p_juhanibb"):
            self.assertIn(line + "\n", self.out + "\n")
        self.assertIn("  parent k1p_juhanibb\n", self.out)

    def test_no_double_prefix(self):
        self.assertNotIn("k1k1", self.out)
        self.assertNotIn("k1p_k1p_", self.out)
        self.assertNotIn("P_JuhaniBB\n", self.out)

    def test_node_casing_preserved(self):
        for token in ("lfootT_g", "CLOTH", "rootdummy"):
            self.assertIn(token, self.out)
        self.assertIn("    lfoot_g 0.5 lfootT_g 0.5", self.out)

    def test_bitmap_mapping(self):
        self.assertIn("  bitmap k1p_juhanibb01\n", self.out)
        self.assertIn("  bitmap k1p_juhanih01\n", self.out)
        self.assertIn("  bitmap2 NULL\n", self.out)

    def test_explicit_texture_map_wins(self):
        out = M.rename_ascii(FIXTURE, MAP, {"P_JuhaniBB01": "k1p_custom01"})
        self.assertIn("  bitmap k1p_custom01\n", out)

    def test_supermodel_target_untouched_when_unmapped(self):
        self.assertIn("S_Female03", self.out)

    def test_idempotent(self):
        self.assertEqual(M.rename_ascii(self.out, MAP), self.out)

    def test_crlf_normalised(self):
        self.assertNotIn("\r", self.out)

    def test_word_boundary(self):
        out = M.rename_ascii("node dummy P_JuhaniBB2\nnode dummy xp_juhanibb\nnode dummy P_JuhaniBB\n", MAP)
        self.assertEqual(out, "node dummy P_JuhaniBB2\nnode dummy xp_juhanibb\nnode dummy k1p_juhanibb\n")

    def test_lowercase_nodes_flag(self):
        out = M.rename_ascii(FIXTURE, MAP, lowercase_nodes=True)
        self.assertIn("node trimesh lfoott_g\n", out)
        self.assertIn("node danglymesh cloth\n", out)
        self.assertIn("  parent lfoott_g\n", out)
        self.assertIn("  parent NULL\n", out)
        self.assertIn("lfootT_g 0.5", out)  # weight lines keep the K1 spelling, as in the 10-16 build


class HelperTest(unittest.TestCase):
    def test_node_names_keep_case(self):
        self.assertEqual(M.node_names(FIXTURE), ["P_JuhaniBB", "rootdummy", "lfootT_g", "CLOTH"])

    def test_default_texture(self):
        self.assertEqual(M.default_texture("P_JuhaniBA01"), "k1p_juhaniba01")
        self.assertEqual(M.default_texture("N_CommM04"), "k1p_n_commm04")
        self.assertEqual(M.default_texture("NULL"), "NULL")
        self.assertEqual(M.default_texture("k1p_x01"), "k1p_x01")

    def test_parse_map(self):
        self.assertEqual(M.parse_map(["P_A=k1p_a"]), {"p_a": "k1p_a"})
        with self.assertRaises(M.MdlopsError):
            M.parse_map(["nonsense"])

    def test_supermodel_of(self):
        blob = bytearray(300)
        blob[M.SUPER_OFF:M.SUPER_OFF + 10] = b"S_Female03"
        self.assertEqual(M.supermodel_of(bytes(blob)), "S_Female03")


if __name__ == "__main__":
    unittest.main()
