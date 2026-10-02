import os
import shutil
import tempfile
import unittest

import helpers as h
import atlas_common as ac
import atlas_coverage as cov
import export_dump_inputs as ex


class ExportTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = os.path.join(self.tmp.name, "root")
        os.makedirs(os.path.join(self.root, "inputs"))
        self.db = os.path.join(self.tmp.name, "k.db")
        h.build_db(self.db)
        self.inp = os.path.join(self.root, "inputs")
        with open(os.path.join(self.inp, "lessons_recent.tsv"), "w") as f:
            f.write("id\tscope\ttopic\tnote\n150\ts\tt150\tn150\n")
        with open(os.path.join(self.inp, "candidates.tsv"), "w") as f:
            f.write("id\ttitle\tsubsystem\thypothesis\tevidence\tstatus\nZ9\tOLD\tsub\thyp\tevi\topen\n")
        self.args = ["--root", self.root, "--db", self.db]

    def tearDown(self):
        self.tmp.cleanup()

    @staticmethod
    def slurp(path):
        with open(path) as f:
            return f.read()

    def read(self, name):
        with open(os.path.join(self.inp, name)) as f:
            return f.read()

    def test_formats_and_ordering(self):
        rc, out, _ = h.run_main(ex, self.args)
        self.assertEqual(rc, 0)
        nm = self.read("name_map.tsv").split("\n")
        self.assertEqual(nm[0], "steam_addr\tname\tsource\tconfidence\tevidence")
        self.assertEqual(nm[1].split("\t")[0], "0x00401000")     # ordered by address
        self.assertEqual(nm[-1], "")                              # trailing newline
        fn = self.read("db_functions.tsv").split("\n")
        self.assertEqual(fn[0], "address\tsymbol\tsize\tpurpose\tnotes")
        self.assertEqual(len(fn), 4)                              # header + 2 steam rows + ''
        self.assertNotIn("OtherBinaryFn", "\n".join(fn))          # other binary excluded
        self.assertEqual(fn[1], "00401000\tSteamFn0\t\tmulti line text\t")  # NULL -> '', \n \t flattened
        cand = self.read("candidates.tsv").split("\n")
        self.assertEqual([c.split("\t")[0] for c in cand[1:3]], ["Z9", "A1"])  # rowid order, not id order
        les = self.read("lessons_recent.tsv").split("\n")
        self.assertEqual([l.split("\t")[0] for l in les[1:-1]], ["150", "200"])  # id >= 150

    def test_lessons_min_id_flag(self):
        h.run_main(ex, self.args + ["--lessons-min-id", "100"])
        self.assertEqual(len(self.read("lessons_recent.tsv").strip().split("\n")), 4)

    def test_backup_made_once_and_never_clobbered(self):
        h.run_main(ex, self.args)
        bak = os.path.join(self.inp, "candidates.tsv.0930")
        self.assertTrue(os.path.exists(bak))
        self.assertIn("OLD", self.slurp(bak))
        self.assertFalse(os.path.exists(os.path.join(self.inp, "name_map.tsv.0930")))  # nothing to back up
        h.run_main(ex, self.args)   # second run: backup must still be the original
        self.assertIn("OLD", self.slurp(bak))
        self.assertNotIn("late id first", self.slurp(bak))

    def test_dry_run_touches_nothing(self):
        before = self.read("candidates.tsv")
        rc, out, _ = h.run_main(ex, self.args + ["--dry-run"])
        self.assertEqual(rc, 0)
        self.assertIn("DRY RUN", out)
        self.assertEqual(self.read("candidates.tsv"), before)
        self.assertFalse(os.path.exists(os.path.join(self.inp, "name_map.tsv")))
        self.assertFalse(os.path.exists(os.path.join(self.inp, "candidates.tsv.0930")))
        self.assertIn("changed=1", out)      # Z9 differs from the fake old row
        self.assertIn("added=1", out)        # A1
        tmpdir = out.split("DRY RUN ->")[1].split("\n")[0].strip()
        shutil.rmtree(tmpdir, ignore_errors=True)

    def test_diff_summary(self):
        d = ex.diff_summary(["k\tv", "a\t1", "b\t2", "c\t3"], ["k\tv", "a\t1", "b\tX", "d\t4"])
        self.assertEqual((d["same"], d["changed"], d["added"], d["removed"]), (1, ["b"], ["d"], ["c"]))


class CoverageTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.db = os.path.join(self.tmp.name, "k.db")

    def tearDown(self):
        self.tmp.cleanup()

    def rows(self, **kw):
        db = ac.open_db_ro(self.db)
        rows, has_sf, nmap = cov.compute(db, h.FIXROOT, ["Foo", "Bar", "Missing"], **kw)
        return {r["class"]: r for r in rows}, has_sf, nmap

    def test_counts(self):
        h.build_db(self.db)
        r, has_sf, nmap = self.rows()
        self.assertFalse(has_sf)
        self.assertEqual(nmap, 4)
        self.assertEqual((r["Foo"]["mapped"], r["Foo"]["high"], r["Foo"]["verified_in_atlas"]), (3, 2, 1))
        # mapper rows: Foo::Gamma (0x401300) has no name_map row -> open; Foo::Alpha/Beta are mapped
        self.assertEqual(r["Foo"]["open"], 1)
        self.assertEqual(r["Bar"], {"class": "Bar", "mapped": 0, "high": 0, "verified_in_atlas": 0, "open": 1})
        self.assertEqual(r["Missing"]["mapped"], 0)
        self.assertNotIn("struct_fields", r["Foo"])

    def test_prefix_requires_double_colon(self):
        h.build_db(self.db)
        r, _, _ = self.rows()
        db = ac.open_db_ro(self.db)
        rows, _, _ = cov.compute(db, h.FIXROOT, ["NoNamespace"])
        self.assertEqual(rows[0]["mapped"], 0)

    def test_no_mapper_files_is_zero(self):
        h.build_db(self.db)
        r, _, nmap = self.rows(map_glob="does_not_exist_*.tsv")
        self.assertEqual(nmap, 0)
        self.assertEqual(r["Foo"]["open"], 0)

    def test_struct_fields_column_when_table_exists(self):
        h.build_db(self.db, with_struct_fields=True)
        r, has_sf, _ = self.rows()
        self.assertTrue(has_sf)
        self.assertEqual((r["Foo"]["struct_fields"], r["Bar"]["struct_fields"], r["Missing"]["struct_fields"]), (2, 1, 0))

    def test_render_md_and_tsv(self):
        h.build_db(self.db)
        rc, md, _ = h.run_main(cov, ["--root", h.FIXROOT, "--db", self.db, "--classes", "Foo,Bar", "--md"])
        self.assertEqual(rc, 0)
        self.assertTrue(md.startswith("| class | mapped | high | verified_in_atlas | open |"))
        self.assertIn("| Foo | 3 | 2 | 1 | 1 |", md)
        self.assertIn("| TOTAL |", md)
        rc, tsv, _ = h.run_main(cov, ["--root", h.FIXROOT, "--db", self.db, "--classes", "Foo"])
        self.assertEqual(tsv.split("\n")[1], "Foo\t3\t2\t1\t1")

    def test_db_opened_read_only(self):
        h.build_db(self.db)
        db = ac.open_db_ro(self.db)
        with self.assertRaises(Exception):
            db.execute("delete from name_map")


if __name__ == "__main__":
    unittest.main()
