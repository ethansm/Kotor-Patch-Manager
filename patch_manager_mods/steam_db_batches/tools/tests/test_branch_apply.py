"""Unit tests for steam_db_batches/tools/branch_apply.py on a synthetic branch DB built in a temp dir.

Contract: DESIGN.md 4.7 (validated ledger) and the README B0 section.  Nothing here touches the real B0/ B15/ directories
or the real branch DB: `branch_apply.ROOT` is pinned to a temp dir (None = bdlib paths / BD_ROOT, see EnvOverrideTests) and every
database is built under that temp dir.
Run:  python3 -m unittest discover -s tests   (from the tools directory)
"""
import contextlib
import csv
import hashlib
import io
import os
import sqlite3
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))
import batch  # noqa: E402
import bdlib as bd  # noqa: E402
import branch_apply  # noqa: E402

FINAL_COLS = batch.EVIDENCE_COLS + ["final_verdict", "final_note"]
PARITY_COLS = ["class", "function", "steam_addr", "calling_convention", "param_size_bytes", "note"]
FP_COLS = ["addr", "size", "retn", "n_insn", "callconv", "thunk_target", "callees", "str_refs", "disp", "imm"]


def h8(a):
    return "0x%08x" % a


class Fx:
    """A tiny B0 world: branch DB rows, B0/final_rows.tsv, B0/parity.tsv and the Steam server fp.tsv."""

    def __init__(self, root):
        self.root = Path(root)
        self.bdir = self.root / "B0"
        self.server = self.root / "server"
        self.db = self.root / "branch.db"
        self.branch, self.final, self.parity, self.fp_addrs = [], [], [], set()

    # -- row builders
    def add_branch(self, cls, fn, addr, cc=None, param=None, notes=None):
        self.branch.append((cls, fn, addr, notes, cc, param))

    def add_final(self, cls, name, cand_orig, verdict, steam=None, **kw):
        row = {"kind": "F", "class": cls, "name": name, "gog": kw.pop("gog", "0x00400000"),
               "steam_addr": h8(steam) if steam is not None else "", "cand_orig": h8(cand_orig),
               "final_verdict": verdict, "final_note": "", "evidence": "", "calling_convention": "",
               "param_size_bytes": "", "flags": "", "gate_class": ""}
        row.update(kw)
        self.final.append(row)
        if steam is not None:
            self.fp_addrs.add(steam)

    def add_parity(self, cls, fn, steam, cc="thiscall", param="4", note="gog label wrong"):
        self.parity.append({"class": cls, "function": fn, "steam_addr": h8(steam), "calling_convention": cc,
                            "param_size_bytes": param, "note": note})
        self.fp_addrs.add(steam)

    def row(self, cls, fn, addr, verdict="verified", steam=None, **kw):
        """branch row + a final row with the same class/name at that address (shipped at `steam` or in place)."""
        self.add_branch(cls, fn, addr)
        self.add_final(cls, fn, addr, verdict, steam=steam if steam is not None else addr, **kw)

    # -- writers
    def build(self):
        self.bdir.mkdir(parents=True, exist_ok=True)
        make_db(self.db, self.branch)
        self._tsv(self.bdir / "final_rows.tsv", FINAL_COLS, self.final)
        if self.parity:
            self._tsv(self.bdir / "parity.tsv", PARITY_COLS, self.parity)
        sdir = self.server / bd.STEAM_PROGRAM_DEFAULT
        sdir.mkdir(parents=True, exist_ok=True)
        rows = [{"addr": h8(a), "size": "48", "retn": "8", "n_insn": "%d" % (a % 7 + 4), "callconv": "unknown",
                 "callees": h8(a + 0x1000), "str_refs": "", "disp": h8(a & 0xFFF), "imm": ""}
                for a in sorted(self.fp_addrs)]
        self._tsv(sdir / "fp.tsv", FP_COLS, rows)
        return self

    @staticmethod
    def _tsv(path, cols, rows):
        with open(path, "w", newline="", encoding="utf-8") as fh:
            w = csv.DictWriter(fh, fieldnames=cols, delimiter="\t", restval="", extrasaction="ignore")
            w.writeheader()
            w.writerows(rows)

    # -- readers
    def functions_csv(self):
        with open(self.bdir / "branch_functions.csv", newline="") as fh:
            return list(csv.DictReader(fh))

    def validated(self):
        return list(bd.iter_tsv(self.bdir / "validated.tsv"))

    def report(self):
        return (self.bdir / "branch_apply_report.md").read_text()

    def expected_sha(self, addr):
        for r in bd.iter_tsv(self.server / bd.STEAM_PROGRAM_DEFAULT / "fp.tsv"):
            if bd.parse_addr(r["addr"]) == addr:
                cells = "|".join(r.get(k, "") for k in batch.FP_SHA_FIELDS)
                return hashlib.sha1(cells.encode()).hexdigest()[:16]
        return ""


def make_db(path, rows):
    """rows: (class, fn, addr, notes, cc, param) -> a branch DB with the real `functions` column set."""
    Path(path).unlink(missing_ok=True)
    con = sqlite3.connect(path)
    con.execute("CREATE TABLE functions(id INTEGER PRIMARY KEY, class_name TEXT, function_name TEXT, address INTEGER, "
                "notes TEXT, calling_convention TEXT, param_size_bytes INTEGER)")
    for cls, fn, addr, notes, cc, param in rows:
        con.execute("INSERT INTO functions(class_name,function_name,address,notes,calling_convention,param_size_bytes) "
                    "VALUES(?,?,?,?,?,?)", (cls, fn, addr, notes, cc, param))
    con.commit()
    con.close()


class Base(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.fx = Fx(self.tmp.name)
        env = mock.patch.dict(os.environ, {"BD_SERVER": str(self.fx.server),
                                           "BD_STEAM_PROGRAM": bd.STEAM_PROGRAM_DEFAULT})
        env.start()
        self.addCleanup(env.stop)
        root = mock.patch.object(branch_apply, "ROOT", Path(self.tmp.name))
        root.start()
        self.addCleanup(root.stop)

    def run_apply(self, db=None, batch_name="B0"):
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = branch_apply.main([batch_name, "--branch-db", str(db or self.fx.db)])
        return rc, buf.getvalue()


LONG_NOTE = 'has "quotes"\nand a newline ' + "x" * 200


class MainScenarioTests(Base):
    """Eight branch rows covering kept / moved / renamed / removed (three verdicts) / parity."""

    def setUp(self):
        super().setUp()
        fx = self.fx
        # 1 kept: cc gets the __ prefix, param from the final row, tier from the auto_route flag
        fx.add_branch("KeepC", "Keep", 0x510000)
        fx.add_final("KeepC", "Keep", 0x510000, "verified", steam=0x510000, calling_convention="thiscall",
                     param_size_bytes="8", gate_class="AUTO", flags="x;auto_route=sig", final_note="match by anchors",
                     gog="0x00410000")
        # 2 moved: no cc / param in the final row -> branch values; evidence used when final_note is empty
        fx.add_branch("MoveC", "Move", 0x510100, cc="cdecl", param=12, notes="old note")
        fx.add_final("MoveC", "Move", 0x510100, "likely", steam=0x510140, gate_class="REVIEW",
                     evidence="evidence text here", gog="0x00410140")
        # 3 renamed in final_rows (pass-2 match), keeps KPM name Old; already-prefixed cc; param 0 stays 0; override tier
        fx.add_branch("RenC", "Old", 0x510200)
        fx.add_final("RenC", "New", 0x510200, "verified", steam=0x510200, calling_convention="__stdcall",
                     param_size_bytes="0", final_note="override: opus fixed it", gog="0x00410200")
        # 4-6 removed
        fx.add_branch("SusC", "Sus", 0x510300)
        fx.add_final("SusC", "Sus", 0x510300, "label_suspect", final_note='body says "other"')
        fx.add_branch("RejC", "Rej", 0x510400)
        fx.add_final("RejC", "Rej", 0x510400, "rejected", steam=0x510440)
        fx.add_branch("UnrC", "Unr", 0x510500)
        fx.add_final("UnrC", "Unr", 0x510500, "unresolved")
        # 7 parity: verdict label_suspect in final_rows, shipped at the parity address
        fx.add_branch("ParC", "Par", 0x510600)
        fx.add_final("ParC", "Par", 0x510600, "label_suspect", gog="0x00410600")
        fx.add_parity("ParC", "Par", 0x510680, cc="thiscall", param="4", note='GameAPI "needs" it\nsecond line')
        # 8 long note with quote and newline
        fx.add_branch("QuoteC", "Q", 0x510700)
        fx.add_final("QuoteC", "Q", 0x510700, "verified", steam=0x510700, gate_class="AUTO", final_note=LONG_NOTE,
                     gog="0x00410700")
        fx.final.reverse()               # matching is by address, never by file order
        fx.build()
        self.rc, self.out = self.run_apply()
        self.csv = {(r["class_name"], r["function_name"]): r for r in fx.functions_csv()}
        self.marks = {m["name"]: m for m in fx.validated()}

    def test_exit_zero_and_counts_line(self):
        self.assertEqual(self.rc, 0, self.out)
        line = "branch rows 8 -> shipped 5 (same address 3, moved 2, parity 1), removed 3"
        self.assertIn(line, self.out)
        self.assertIn(line, self.fx.report())
        self.assertIn("duplicates: 0", self.out)

    def test_only_shipped_rows_are_in_the_functions_csv(self):
        self.assertEqual(sorted(self.csv), [("KeepC", "Keep"), ("MoveC", "Move"), ("ParC", "Par"), ("QuoteC", "Q"),
                                            ("RenC", "Old")])
        header = (self.fx.bdir / "branch_functions.csv").read_text().splitlines()[0].split(",")
        self.assertEqual(header, batch.FUNCTIONS_CSV_HEADER)

    def test_kept_row_stays_at_its_address(self):
        r = self.csv[("KeepC", "Keep")]
        self.assertEqual(r["address"], "0x00510000")
        self.assertEqual(r["calling_convention"], "__thiscall")
        self.assertEqual(r["param_size_bytes"], "8")
        self.assertIn("Steam B0 verified: match by anchors", r["notes"])
        self.assertIn("steam_db_batches/B0/final_rows.tsv", r["notes"])

    def test_moved_row_goes_to_the_final_address_and_falls_back_to_branch_cc_param(self):
        r = self.csv[("MoveC", "Move")]
        self.assertEqual(r["address"], "0x00510140")
        self.assertEqual(r["calling_convention"], "__cdecl")          # branch value, prefixed
        self.assertEqual(r["param_size_bytes"], "12")                 # branch value
        self.assertIn("Steam B0 likely: evidence text here", r["notes"])   # evidence when final_note is empty

    def test_branch_names_are_kept_when_final_name_differs(self):
        self.assertIn(("RenC", "Old"), self.csv)
        self.assertNotIn(("RenC", "New"), self.csv)
        self.assertIn("RenC::Old", self.marks)
        self.assertNotIn("RenC::New", self.marks)

    def test_calling_convention_prefix_is_not_doubled_and_zero_param_survives(self):
        r = self.csv[("RenC", "Old")]
        self.assertEqual(r["calling_convention"], "__stdcall")
        self.assertEqual(r["param_size_bytes"], "0")

    def test_removed_rows_listed_in_report_not_shipped(self):
        rep = self.fx.report()
        removed = rep.split("## Removed")[1]
        self.assertIn("SusC::Sus 0x00510300 (label_suspect)", removed)
        self.assertIn("RejC::Rej 0x00510400 (rejected)", removed)
        self.assertIn("UnrC::Unr 0x00510500 (unresolved)", removed)
        for name in ("SusC::Sus", "RejC::Rej", "UnrC::Unr"):
            self.assertNotIn(name, self.marks)
        moved = rep.split("## Moved")[1].split("## Removed")[0]
        self.assertIn("MoveC::Move 0x00510100 -> 0x00510140", moved)
        self.assertIn("ParC::Par 0x00510600 -> 0x00510680", moved)
        self.assertNotIn("KeepC", moved)

    def test_parity_row_uses_parity_address_cc_param_and_note(self):
        r = self.csv[("ParC", "Par")]
        self.assertEqual(r["address"], "0x00510680")
        self.assertEqual(r["calling_convention"], "__thiscall")
        self.assertEqual(r["param_size_bytes"], "4")
        self.assertIn("Steam B0 GOG parity: GameAPI 'needs' it second line", r["notes"])
        self.assertIn("steam_db_batches/B0/parity.tsv", r["notes"])

    def test_csv_notes_are_quote_free_single_line_and_truncated(self):
        raw = (self.fx.bdir / "branch_functions.csv").read_text()
        self.assertEqual(len(raw.splitlines()), 1 + 5, "one physical line per row (no embedded newline)")
        for r in self.csv.values():
            self.assertNotIn('"', r["notes"])
            self.assertNotIn("\n", r["notes"])
            self.assertNotIn("\r", r["notes"])
        q = self.csv[("QuoteC", "Q")]["notes"]
        self.assertIn("has 'quotes' and a newline ", q)
        # the final_note is cut to 160 chars before the note is built: 200 x's cannot all survive
        self.assertLess(q.count("x"), 200)
        self.assertEqual(q.count("x"), 160 - len('has "quotes"\nand a newline '))

    def test_validated_ledger_rows(self):
        self.assertEqual(sorted(self.marks), ["KeepC::Keep", "MoveC::Move", "ParC::Par", "QuoteC::Q", "RenC::Old"])
        cols = (self.fx.bdir / "validated.tsv").read_text().splitlines()[0].split("\t")
        self.assertEqual(cols, batch.VALIDATED_COLS)
        want = {  # name -> (steam_addr, verdict, tier, gog_addr, evidence_ref)
            "KeepC::Keep": ("0x00510000", "verified", "auto:sig", "0x00410000", "steam_db_batches/B0/final_rows.tsv"),
            "MoveC::Move": ("0x00510140", "likely", "reviewer", "0x00410140", "steam_db_batches/B0/final_rows.tsv"),
            "RenC::Old": ("0x00510200", "verified", "opus", "0x00410200", "steam_db_batches/B0/final_rows.tsv"),
            "ParC::Par": ("0x00510680", "parity", "opus", "0x00410600", "steam_db_batches/B0/parity.tsv"),
            "QuoteC::Q": ("0x00510700", "verified", "auto:c2", "0x00410700", "steam_db_batches/B0/final_rows.tsv"),
        }
        for name, (addr, verdict, tier, gog, ref) in want.items():
            m = self.marks[name]
            self.assertEqual((m["steam_addr"], m["verdict"], m["tier"], m["gog_addr"], m["evidence_ref"]),
                             (addr, verdict, tier, gog, ref), name)
            self.assertEqual(m["batch"], "B0")

    def test_tier_agrees_with_gate_tier(self):
        final = {(r["class"], r["name"]): r for r in self.fx.final}
        for key, name in ((("KeepC", "Keep"), "KeepC::Keep"), (("MoveC", "Move"), "MoveC::Move"),
                          (("RenC", "New"), "RenC::Old"), (("QuoteC", "Q"), "QuoteC::Q")):
            self.assertEqual(self.marks[name]["tier"], batch.gate_tier(final[key]), name)

    def test_fp_sha_is_16_hex_and_matches_the_server_fingerprint(self):
        for name, m in self.marks.items():
            self.assertRegex(m["fp_sha"], r"^[0-9a-f]{16}$", name)
            self.assertEqual(m["fp_sha"], self.fx.expected_sha(bd.parse_addr(m["steam_addr"])), name)
        self.assertEqual(len({m["fp_sha"] for m in self.marks.values()}), 5, "distinct bodies -> distinct hashes")

    def test_report_sections(self):
        rep = self.fx.report()
        self.assertTrue(rep.startswith("# B0 branch apply plan"))
        for sec in ("## Duplicates", "## Moved", "## Removed"):
            self.assertIn(sec, rep)
        self.assertIn("\nnone\n", rep)         # no duplicates

    def test_never_writes_the_branch_db(self):
        before = self.fx.db.read_bytes()
        self.run_apply()
        self.assertEqual(self.fx.db.read_bytes(), before)


class MissingFpTests(Base):
    def test_address_without_a_server_fingerprint_gets_an_empty_fp_sha(self):
        self.fx.row("AC", "A", 0x510000)
        self.fx.row("AC", "B", 0x510040)
        self.fx.build()
        # drop B's fingerprint row
        fp = self.fx.server / bd.STEAM_PROGRAM_DEFAULT / "fp.tsv"
        lines = fp.read_text().splitlines()
        fp.write_text("\n".join(l for l in lines if "0x00510040" not in l) + "\n")
        rc, out = self.run_apply()
        self.assertEqual(rc, 0, out)
        m = {r["name"]: r for r in self.fx.validated()}
        self.assertRegex(m["AC::A"]["fp_sha"], r"^[0-9a-f]{16}$")
        self.assertEqual(m["AC::B"]["fp_sha"], "")


class DuplicateTests(Base):
    def test_duplicate_name_exit_1_no_validated(self):
        self.fx.row("DupC", "Same", 0x510800)
        self.fx.row("DupC", "Same", 0x510840)       # no UNIQUE(class, function) in this test DB
        self.fx.row("OkC", "Fine", 0x510880)
        self.fx.build()
        rc, out = self.run_apply()
        self.assertEqual(rc, 1, out)
        self.assertIn("duplicates: 1", out)
        self.assertFalse((self.fx.bdir / "validated.tsv").exists())
        dup = self.fx.report().split("## Duplicates")[1].split("## Moved")[0]
        self.assertIn("'name'", dup)
        self.assertIn("DupC", dup)
        self.assertNotIn("none", dup)

    def test_duplicate_address_exit_1_no_validated(self):
        self.fx.row("AddrC", "One", 0x510900)
        self.fx.add_branch("AddrC", "Two", 0x510940)
        self.fx.add_final("AddrC", "Two", 0x510940, "likely", steam=0x510900)   # moved onto One's address
        self.fx.build()
        rc, out = self.run_apply()
        self.assertEqual(rc, 1, out)
        self.assertFalse((self.fx.bdir / "validated.tsv").exists())
        dup = self.fx.report().split("## Duplicates")[1].split("## Moved")[0]
        self.assertIn("'addr'", dup)
        self.assertIn("0x00510900", dup)

    def test_icf_noted_parity_rows_may_share_an_address(self):
        for fn in ("IcfA", "IcfB"):
            self.fx.add_branch("IcfC", fn, 0x510A00 if fn == "IcfA" else 0x510A40)
            self.fx.add_final("IcfC", fn, 0x510A00 if fn == "IcfA" else 0x510A40, "label_suspect")
            self.fx.add_parity("IcfC", fn, 0x510B00, note="ICF fold of two identical bodies")
        self.fx.build()
        rc, out = self.run_apply()
        self.assertEqual(rc, 0, out)
        marks = self.fx.validated()
        self.assertEqual(sorted((m["steam_addr"], m["name"]) for m in marks),
                         [("0x00510b00", "IcfC::IcfA"), ("0x00510b00", "IcfC::IcfB")])
        self.assertEqual({m["verdict"] for m in marks}, {"parity"})

    def test_same_parity_address_without_icf_note_is_a_duplicate(self):
        for i, fn in enumerate(("PlainA", "PlainB")):
            self.fx.add_branch("PlnC", fn, 0x510C00 + 0x40 * i)
            self.fx.add_final("PlnC", fn, 0x510C00 + 0x40 * i, "label_suspect")
            self.fx.add_parity("PlnC", fn, 0x510D00, note="same body, no marker")
        self.fx.build()
        rc, out = self.run_apply()
        self.assertEqual(rc, 1, out)
        self.assertFalse((self.fx.bdir / "validated.tsv").exists())

    def test_removed_rows_never_collide(self):
        """A rejected row whose candidate address equals a shipped row's address is not a duplicate."""
        self.fx.row("KeepC", "Keep", 0x510E00)
        self.fx.add_branch("GoneC", "Gone", 0x510E40)
        self.fx.add_final("GoneC", "Gone", 0x510E40, "rejected", steam=0x510E00)
        self.fx.build()
        rc, out = self.run_apply()
        self.assertEqual(rc, 0, out)
        self.assertEqual([m["name"] for m in self.fx.validated()], ["KeepC::Keep"])


class UnmatchedTests(Base):
    def assert_unmatched(self, expect_in_out=()):
        rc, out = self.run_apply()
        self.assertEqual(rc, 2, out)
        self.assertIn("UNMATCHED", out)
        for s in expect_in_out:
            self.assertIn(s, out)
        for f in ("branch_functions.csv", "validated.tsv", "branch_apply_report.md"):
            self.assertFalse((self.fx.bdir / f).exists(), f)

    def test_branch_row_without_a_final_row_exits_2_and_writes_nothing(self):
        self.fx.row("OkC", "Fine", 0x510000)
        self.fx.add_branch("LostC", "Lost", 0x510040)
        self.fx.build()
        self.assert_unmatched(["LostC", "Lost"])

    def test_two_leftover_final_rows_at_one_address_are_ambiguous(self):
        self.fx.add_branch("AmbC", "Old", 0x510100)
        self.fx.add_final("AmbC", "NewA", 0x510100, "verified", steam=0x510100)
        self.fx.add_final("AmbC", "NewB", 0x510100, "verified", steam=0x510100)
        self.fx.build()
        self.assert_unmatched(["AmbC", "Old"])

    def test_a_final_row_serves_only_one_branch_row(self):
        self.fx.add_branch("OneC", "A", 0x510200)
        self.fx.add_branch("OneC", "B", 0x510200)
        self.fx.add_final("OneC", "A", 0x510200, "verified", steam=0x510200)
        self.fx.build()
        self.assert_unmatched(["OneC", "B"])

    def test_same_address_branch_rows_match_by_name_whatever_the_file_order(self):
        self.fx.add_branch("TwinC", "First", 0x510300)
        self.fx.add_branch("TwinC", "Second", 0x510300)
        self.fx.add_final("TwinC", "Second", 0x510300, "verified", steam=0x510380, final_note="second")
        self.fx.add_final("TwinC", "First", 0x510300, "likely", steam=0x510300, final_note="first")
        self.fx.build()
        rc, out = self.run_apply()
        self.assertEqual(rc, 0, out)
        m = {r["name"]: r for r in self.fx.validated()}
        self.assertEqual((m["TwinC::First"]["steam_addr"], m["TwinC::First"]["verdict"]), ("0x00510300", "likely"))
        self.assertEqual((m["TwinC::Second"]["steam_addr"], m["TwinC::Second"]["verdict"]), ("0x00510380", "verified"))

    def test_class_must_match_in_pass_one_but_a_lone_leftover_still_matches_in_pass_two(self):
        self.fx.add_branch("RealC", "Fn", 0x510400)
        self.fx.add_final("OtherC", "Fn", 0x510400, "verified", steam=0x510400)
        self.fx.build()
        rc, out = self.run_apply()
        self.assertEqual(rc, 0, out)
        self.assertEqual([r["name"] for r in self.fx.validated()], ["RealC::Fn"])


class PostImportDbTests(Base):
    """Backup-only rule: branch_apply must be run against the PRE-import branch DB.  Against a DB that already holds
    the moved rows at their new addresses it must fail loudly instead of silently re-planning."""

    def test_running_against_the_post_import_db_exits_2_unmatched(self):
        fx = self.fx
        fx.row("KeepC", "Keep", 0x510000)
        fx.add_branch("MoveC", "Move", 0x510100)
        fx.add_final("MoveC", "Move", 0x510100, "likely", steam=0x510140)
        fx.add_branch("GoneC", "Gone", 0x510200)
        fx.add_final("GoneC", "Gone", 0x510200, "rejected")
        fx.build()
        rc, out = self.run_apply()
        self.assertEqual(rc, 0, out)
        csv_before = (fx.bdir / "branch_functions.csv").read_bytes()
        marks_before = (fx.bdir / "validated.tsv").read_bytes()
        # the imported DB = exactly the produced branch_functions.csv (mode replace)
        post = self.fx.root / "post_import.db"
        make_db(post, [(r["class_name"], r["function_name"], int(r["address"], 16), r["notes"],
                        r["calling_convention"] or None, int(r["param_size_bytes"]) if r["param_size_bytes"] else None)
                       for r in fx.functions_csv()])
        rc2, out2 = self.run_apply(db=post)
        self.assertEqual(rc2, 2, out2)
        self.assertIn("UNMATCHED", out2)
        self.assertIn("MoveC", out2)
        self.assertNotIn("KeepC", out2)                                   # in-place rows still match
        # and the loud failure leaves the earlier plan untouched
        self.assertEqual((fx.bdir / "branch_functions.csv").read_bytes(), csv_before)
        self.assertEqual((fx.bdir / "validated.tsv").read_bytes(), marks_before)


class ReportCountTests(Base):
    # Candidate production bug: the report's `parity N` is len(parity.tsv rows), not the number of parity rows that
    # were actually shipped.  A parity.tsv entry that matches no branch row is counted although nothing ships for it.
    def test_parity_count_counts_shipped_parity_rows_only(self):
        self.fx.row("KeepC", "Keep", 0x510000)
        self.fx.add_parity("GhostC", "NotInBranch", 0x510F00)
        self.fx.build()
        rc, out = self.run_apply()
        self.assertEqual(rc, 0, out)
        self.assertIn("parity 0)", out)


class EnvOverrideTests(unittest.TestCase):
    """DESIGN 0: every path is overridable by a BD_* variable (BD_ROOT, BD_BRANCHDB).  Until 2026-10-03 branch_apply
    computed both once at import time and ignored the variables."""

    def test_root_follows_bd_root(self):
        # fixed 2026-10-03: ROOT was Path(__file__).parents[1] and ignored BD_ROOT
        with tempfile.TemporaryDirectory() as t, mock.patch.dict(os.environ, {"BD_ROOT": t}), \
                mock.patch.object(branch_apply, "ROOT", None):
            self.assertEqual(branch_apply.batches_root(), bd.get_paths().batches)
            self.assertEqual(branch_apply.batches_root(), Path(t))

    def test_default_branch_db_follows_bd_branchdb(self):
        # fixed 2026-10-03: the --branch-db default was a hard-coded constant; now bdlib paths (BD_BRANCHDB)
        with tempfile.TemporaryDirectory() as t, mock.patch.dict(os.environ, {"BD_BRANCHDB": t + "/x.db",
                                                                              "BD_ROOT": t}), \
                mock.patch.object(branch_apply, "ROOT", None), mock.patch.object(branch_apply, "load_branch") as lb:
            lb.side_effect = SystemExit(9)
            with self.assertRaises(SystemExit):
                branch_apply.main(["B0"])
            self.assertEqual(lb.call_args[0][0], t + "/x.db")


class HelperTests(unittest.TestCase):
    def test_hx_accepts_hex_strings_and_ints(self):
        self.assertEqual(branch_apply.hx("0x00510000"), 0x510000)
        self.assertEqual(branch_apply.hx("510000"), 0x510000)
        self.assertEqual(branch_apply.hx(0x510000), 0x510000)

    def test_load_branch_is_read_only_and_ordered_by_id(self):
        with tempfile.TemporaryDirectory() as t:
            db = Path(t) / "b.db"
            make_db(db, [("B", "b", 2, "n", "thiscall", 4), ("A", "a", 1, None, None, None)])
            rows = branch_apply.load_branch(db)
            self.assertEqual([r["id"] for r in rows], [1, 2])
            self.assertEqual(rows[0]["cls"], "B")
            self.assertEqual((rows[1]["cc"], rows[1]["param"], rows[1]["notes"]), (None, None, None))
            con = sqlite3.connect("file:%s?mode=ro" % db, uri=True)
            with self.assertRaises(sqlite3.OperationalError):
                con.execute("DELETE FROM functions")
            con.close()


if __name__ == "__main__":
    unittest.main()
