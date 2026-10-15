"""Tests for rea_ledger.py (REA findings ledger).

Run from the tools dir: python3 -m unittest tests.test_rea_ledger -v
"""
import contextlib
import io
import os
import re
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import rea_ledger as rl  # noqa: E402

HDR = "\t".join(rl.COLS)
EV = "procedure_assembly 0x6f5b4f: MOV EAX,[ESI+0x4]"
FNAME = "20261015_t.tsv"
LINE_RE = re.compile(r"^\S+:\d+ L\d \S.*$")


def fields(**kw):
    base = {
        "addr": "0x006f5b40", "kind": "function", "name": "insert-if-absent wrapper",
        "confidence": "confirmed", "evidence": EV, "topic": "WI-1", "date": "2026-10-15",
    }
    base.update(kw)
    return base


def mkrow(**kw):
    f = fields(**kw)
    return "\t".join(f[c] for c in rl.COLS)


def write_ledger(d, name, *lines, header=HDR):
    path = os.path.join(d, name)
    with open(path, "w", encoding="utf-8", newline="") as fh:
        fh.write(header + "\n")
        for line in lines:
            fh.write(line + "\n")
    return path


def read_bytes(path):
    with open(path, "rb") as fh:
        return fh.read()


def issue_keys(issues):
    return [(i.file, i.line, i.rule) for i in issues]


def captured(fn, *args, **kw):
    out, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        rc = fn(*args, **kw)
    return rc, out.getvalue(), err.getvalue()


class TmpCase(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.d = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def lint_one(self, *lines, header=HDR):
        write_ledger(self.d, FNAME, *lines, header=header)
        return rl.lint(self.d)


class LintRuleTests(TmpCase):
    def test_clean_row_has_no_issues(self):
        self.assertEqual(self.lint_one(mkrow()), [])

    # L1
    def test_L1_pass_known_kind(self):
        self.assertEqual(self.lint_one(mkrow(kind="xref", evidence="procedure_callers 0x6f5b40: CALL")), [])

    def test_L1_fail_unknown_kind(self):
        self.assertEqual(issue_keys(self.lint_one(mkrow(kind="bogus"))), [(FNAME, 2, "L1")])

    # L2
    def test_L2_pass_anchor_present(self):
        self.assertEqual(self.lint_one(mkrow(kind="string", evidence='search_strings 0x00a1b2c3: "Foo"')), [])

    def test_L2_pass_non_anchor_kind_needs_no_anchor(self):
        self.assertEqual(self.lint_one(mkrow(kind="xref", evidence="tool: no anchor")), [])

    def test_L2_fail_anchor_missing(self):
        self.assertEqual(issue_keys(self.lint_one(mkrow(evidence="looked at it"))), [(FNAME, 2, "L2")])

    def test_L2_fail_anchor_too_short(self):
        self.assertEqual(issue_keys(self.lint_one(mkrow(evidence="procedure_assembly 0x6f5b4: MOV"))),
                         [(FNAME, 2, "L2")])

    # L4
    def test_L4_pass_seven_cells(self):
        self.assertEqual(self.lint_one(mkrow()), [])

    def test_L4_fail_extra_cell(self):
        self.assertEqual(issue_keys(self.lint_one(mkrow() + "\textra")), [(FNAME, 2, "L4")])

    def test_L4_fail_header_mismatch_skips_rows(self):
        issues = self.lint_one(mkrow(kind="bogus"), header="addr\tkind\tname")
        self.assertEqual(issue_keys(issues), [(FNAME, 1, "L4")])

    # L5
    def test_L5_pass_valid_fields(self):
        self.assertEqual(self.lint_one(mkrow()), [])

    def test_L5_fail_addr_uppercase(self):
        self.assertEqual(issue_keys(self.lint_one(mkrow(addr="0x006F5B40"))), [(FNAME, 2, "L5")])

    def test_L5_fail_date_format(self):
        self.assertEqual(issue_keys(self.lint_one(mkrow(date="10/15/2026"))), [(FNAME, 2, "L5")])

    def test_L5_fail_impossible_date(self):
        self.assertEqual(issue_keys(self.lint_one(mkrow(date="2026-02-30"))), [(FNAME, 2, "L5")])

    def test_L5_fail_confidence(self):
        self.assertEqual(issue_keys(self.lint_one(mkrow(confidence="maybe"))), [(FNAME, 2, "L5")])

    # L6
    def test_L6_pass_superseded_marker(self):
        issues = self.lint_one(
            mkrow(name="A"),
            mkrow(name="A", confidence="refuted", evidence=EV + " supersedes: " + FNAME + ":2"),
        )
        self.assertEqual(issues, [])

    def test_L6_pass_no_confirmed_row(self):
        self.assertEqual(self.lint_one(mkrow(confidence="refuted")), [])

    def test_L6_pass_different_addr(self):
        self.assertEqual(self.lint_one(mkrow(), mkrow(addr="0x00111111", confidence="refuted")), [])

    def test_L6_fail_conflict_same_file(self):
        issues = self.lint_one(mkrow(name="A"), mkrow(name="B", confidence="refuted"))
        self.assertEqual(issue_keys(issues), [(FNAME, 3, "L6")])
        self.assertIn(FNAME + ":2", issues[0].msg)

    def test_L6_fail_conflict_across_files(self):
        write_ledger(self.d, "20261014_x.tsv", mkrow())
        write_ledger(self.d, "20261015_y.tsv", mkrow(confidence="refuted"))
        issues = rl.lint(self.d)
        self.assertEqual(issue_keys(issues), [("20261015_y.tsv", 2, "L6")])
        self.assertIn("20261014_x.tsv:2", issues[0].msg)

    # L8
    def test_L8_pass_xref_same_file(self):
        issues = self.lint_one(
            mkrow(kind="function", topic="B13a-q1"),
            mkrow(kind="xref", topic="B13a-q1", name="callers 1: x; callees: none; size 10 B RET 0",
                  evidence="procedure_callers 0x006f5b40: CALL"),
        )
        self.assertEqual(issues, [])

    def test_L8_fail_missing_xref(self):
        self.assertEqual(issue_keys(self.lint_one(mkrow(topic="B13a-q1"))), [(FNAME, 2, "L8")])

    def test_L8_fail_xref_in_other_file(self):
        write_ledger(self.d, "20261014_x.tsv", mkrow(kind="xref", topic="B13a-q1", evidence="tool: x"))
        write_ledger(self.d, "20261015_y.tsv", mkrow(topic="B13a-q1"))
        self.assertEqual(issue_keys(rl.lint(self.d)), [("20261015_y.tsv", 2, "L8")])

    def test_L8_ignores_non_matching_topic(self):
        self.assertEqual(self.lint_one(mkrow(topic="WI-1")), [])


class EdgeTests(TmpCase):
    def test_L6_addr_compared_by_int_value(self):
        issues = self.lint_one(mkrow(addr="0x6f5b40", name="A"), mkrow(addr="0x006f5b40", name="B", confidence="refuted"))
        self.assertEqual(issue_keys(issues), [(FNAME, 3, "L6")])

    def test_L8_per_file_not_global(self):
        write_ledger(self.d, "20261014_x.tsv", mkrow(topic="B13a-q1"), mkrow(kind="xref", topic="B13a-q1", evidence="tool: x"))
        write_ledger(self.d, "20261015_y.tsv", mkrow(topic="B13a-q1"))
        self.assertEqual(issue_keys(rl.lint(self.d)), [("20261015_y.tsv", 2, "L8")])

    def test_crlf_file_lints_and_reads_clean(self):
        path = os.path.join(self.d, FNAME)
        with open(path, "wb") as fh:
            fh.write(("\r\n".join([HDR, mkrow(name="a"), mkrow(name="b")]) + "\r\n").encode())
        self.assertEqual(rl.lint(self.d), [])
        rows = rl.read_rows(self.d, strict=True)
        self.assertEqual([(r["name"], r["date"], r["line"]) for r in rows], [("a", "2026-10-15", 2), ("b", "2026-10-15", 3)])

    def test_line_is_physical_with_blank_lines(self):
        write_ledger(self.d, FNAME, mkrow(name="a"), "", "", mkrow(name="b", kind="bogus"))
        self.assertEqual([r["line"] for r in rl.read_rows(self.d)], [2, 5])
        self.assertEqual(issue_keys(rl.lint(self.d)), [(FNAME, 5, "L1")])

    def test_trailing_newline_in_cell_rejected_by_append(self):
        with self.assertRaises(rl.LedgerError) as cm:
            rl.append_rows(self.d, FNAME, [fields(name="a\n")])
        self.assertEqual(cm.exception.rule, "L4")
        self.assertEqual(os.listdir(self.d), [])

    def test_dedupe_keeps_all_earlier_nonrefuted_when_last_refuted(self):
        rows = [dict(fields(name="X", confidence=c), file=FNAME, line=n)
                for n, c in ((2, "probable"), (3, "confirmed"), (4, "refuted"), (5, "refuted"))]
        out = rl.dedupe(rows)
        self.assertEqual([(r["line"], r["superseded"]) for r in out], [(2, True), (3, True), (5, False)])


class ReadRowsTests(TmpCase):
    def test_read_rows_fields(self):
        write_ledger(self.d, FNAME, mkrow())
        rows = rl.read_rows(self.d)
        self.assertEqual(len(rows), 1)
        self.assertEqual((rows[0]["file"], rows[0]["line"], rows[0]["addr"]), (FNAME, 2, "0x006f5b40"))
        self.assertTrue(set(rl.COLS).issubset(rows[0]))

    def test_strict_extra_tab_raises_with_file_and_line(self):
        write_ledger(self.d, FNAME, mkrow(), mkrow(name="x") + "\textra")
        with self.assertRaises(rl.LedgerError) as cm:
            rl.read_rows(self.d, strict=True)
        self.assertEqual((cm.exception.file, cm.exception.line), (FNAME, 3))
        self.assertIn(FNAME + ":3:", str(cm.exception))

    def test_non_strict_skips_extra_tab(self):
        write_ledger(self.d, FNAME, mkrow(), mkrow(name="x") + "\textra")
        self.assertEqual([r["line"] for r in rl.read_rows(self.d)], [2])

    def test_header_mismatch_skipped_unless_strict(self):
        write_ledger(self.d, "20261015_bad.tsv", mkrow(), header="addr\tkind")
        write_ledger(self.d, FNAME, mkrow())
        self.assertEqual([r["file"] for r in rl.read_rows(self.d)], [FNAME])
        with self.assertRaises(rl.LedgerError) as cm:
            rl.read_rows(self.d, strict=True)
        self.assertEqual((cm.exception.file, cm.exception.line), ("20261015_bad.tsv", 1))


class AppendTests(TmpCase):
    def test_new_file_gets_header(self):
        self.assertEqual(rl.append_rows(self.d, FNAME, [fields(name="new one")]), 1)
        self.assertEqual(read_bytes(os.path.join(self.d, FNAME)).split(b"\n")[0], HDR.encode())
        self.assertEqual([r["name"] for r in rl.read_rows(self.d)], ["new one"])

    def test_append_keeps_old_bytes(self):
        path = write_ledger(self.d, FNAME, mkrow(name="a"), mkrow(name="b"))
        old = read_bytes(path)
        self.assertEqual(rl.append_rows(self.d, FNAME, [fields(name="c")]), 1)
        new = read_bytes(path)
        self.assertEqual(new[:len(old)], old)
        self.assertGreater(len(new), len(old))
        self.assertEqual([r["name"] for r in rl.read_rows(self.d)], ["a", "b", "c"])

    def test_missing_trailing_newline_gets_one(self):
        path = os.path.join(self.d, FNAME)
        with open(path, "wb") as fh:
            fh.write((HDR + "\n" + mkrow(name="a")).encode())
        old = read_bytes(path)
        rl.append_rows(self.d, FNAME, [fields(name="b")])
        new = read_bytes(path)
        self.assertEqual(new[:len(old) + 1], old + b"\n")
        self.assertEqual([r["name"] for r in rl.read_rows(self.d)], ["a", "b"])

    def test_bad_kind_raises_and_leaves_file(self):
        path = write_ledger(self.d, FNAME, mkrow())
        old = read_bytes(path)
        with self.assertRaises(rl.LedgerError) as cm:
            rl.append_rows(self.d, FNAME, [fields(kind="bogus")])
        self.assertEqual(cm.exception.rule, "L1")
        self.assertEqual(read_bytes(path), old)
        self.assertEqual(os.listdir(self.d), [FNAME])

    def test_tab_in_name_raises_and_leaves_file(self):
        path = write_ledger(self.d, FNAME, mkrow())
        old = read_bytes(path)
        with self.assertRaises(rl.LedgerError) as cm:
            rl.append_rows(self.d, FNAME, [fields(name="a\tb")])
        self.assertEqual(cm.exception.rule, "L4")
        self.assertEqual(read_bytes(path), old)
        self.assertEqual(os.listdir(self.d), [FNAME])

    def test_bad_addr_date_confidence_rejected(self):
        for bad, rule in (({"addr": "6f5b40"}, "L5"), ({"date": "15-10-2026"}, "L5"),
                          ({"confidence": "sure"}, "L5"), ({"evidence": "no anchor"}, "L2")):
            with self.subTest(bad=bad):
                with self.assertRaises(rl.LedgerError) as cm:
                    rl.append_rows(self.d, FNAME, [fields(**bad)])
                self.assertEqual(cm.exception.rule, rule)
        self.assertFalse(os.path.exists(os.path.join(self.d, FNAME)))

    def test_bad_file_names_rejected(self):
        for bad in ("../x.tsv", "sub/x.tsv", "..", "x.txt", ""):
            with self.subTest(file=bad):
                with self.assertRaises(ValueError):
                    rl.append_rows(self.d, bad, [fields()])

    def test_crash_during_replace_leaves_original_and_no_temp(self):
        path = write_ledger(self.d, FNAME, mkrow())
        old = read_bytes(path)
        with mock.patch("os.replace", side_effect=OSError("simulated crash")):
            with self.assertRaises(OSError):
                rl.append_rows(self.d, FNAME, [fields(name="new")])
        self.assertEqual(read_bytes(path), old)
        self.assertEqual(os.listdir(self.d), [FNAME])

    def test_crash_on_new_file_leaves_no_file_and_no_temp(self):
        with mock.patch("os.replace", side_effect=OSError("simulated crash")):
            with self.assertRaises(OSError):
                rl.append_rows(self.d, FNAME, [fields()])
        self.assertEqual(os.listdir(self.d), [])


class DedupeTests(TmpCase):
    def test_different_names_both_kept_unflagged(self):
        write_ledger(self.d, FNAME, mkrow(name="insert-if-absent wrapper"),
                     mkrow(name="CSWSCreatureStats::AddFeat", confidence="refuted"))
        out = rl.dedupe(rl.read_rows(self.d))
        self.assertEqual([r["name"] for r in out], ["insert-if-absent wrapper", "CSWSCreatureStats::AddFeat"])
        self.assertEqual([r["superseded"] for r in out], [False, False])

    def test_same_name_confirmed_then_refuted_flags_confirmed(self):
        write_ledger(self.d, FNAME, mkrow(name="X"), mkrow(name="X", confidence="refuted"))
        out = rl.dedupe(rl.read_rows(self.d))
        self.assertEqual([(r["confidence"], r["superseded"]) for r in out],
                         [("confirmed", True), ("refuted", False)])

    def test_probable_then_refuted_flags_probable(self):
        write_ledger(self.d, FNAME, mkrow(name="X", confidence="probable"), mkrow(name="X", confidence="refuted"))
        out = rl.dedupe(rl.read_rows(self.d))
        self.assertEqual([(r["confidence"], r["superseded"]) for r in out],
                         [("probable", True), ("refuted", False)])

    def test_exact_duplicates_collapse_to_last(self):
        write_ledger(self.d, FNAME, mkrow(name="X"), mkrow(name="X"))
        out = rl.dedupe(rl.read_rows(self.d))
        self.assertEqual(len(out), 1)
        self.assertEqual((out[0]["line"], out[0]["superseded"]), (3, False))

    def test_addr_compared_by_int_value(self):
        write_ledger(self.d, FNAME, mkrow(name="X"), mkrow(name="X", addr="0x6f5b40"))
        self.assertEqual(len(rl.dedupe(rl.read_rows(self.d))), 1)

    def test_supersedes_marker_flags_target(self):
        write_ledger(self.d, FNAME, mkrow(name="old"),
                     mkrow(name="new", evidence=EV + " supersedes: " + FNAME + ":2"))
        out = rl.dedupe(rl.read_rows(self.d))
        self.assertEqual({r["line"]: r["superseded"] for r in out}, {2: True, 3: False})

    def test_every_row_has_bool_superseded_in_order(self):
        write_ledger(self.d, FNAME, mkrow(name="b"), mkrow(name="a"), mkrow(name="b", confidence="refuted"))
        out = rl.dedupe(rl.read_rows(self.d))
        self.assertTrue(all(isinstance(r["superseded"], bool) for r in out))
        self.assertEqual([r["line"] for r in out], sorted(r["line"] for r in out))


class CliTests(TmpCase):
    def test_lint_clean_exit_0(self):
        write_ledger(self.d, FNAME, mkrow())
        rc, out, _ = captured(rl.main, ["lint", "--dir", self.d])
        self.assertEqual(rc, 0)
        self.assertEqual(out.strip(), "0 issues")

    def test_lint_dirty_exit_1_and_line_format(self):
        write_ledger(self.d, FNAME, mkrow(kind="bogus"))
        rc, out, _ = captured(rl.main, ["lint", "--dir", self.d])
        self.assertEqual(rc, 1)
        lines = out.strip().splitlines()
        self.assertEqual(lines[0], f"{FNAME}:2 L1 kind 'bogus' not in KINDS")
        self.assertTrue(all(LINE_RE.match(ln) for ln in lines[:-1]))
        self.assertEqual(lines[-1], "1 issues")

    def test_strict_error_exit_1(self):
        write_ledger(self.d, FNAME, mkrow() + "\textra")
        rc, out, _ = captured(rl.main, ["lint", "--dir", self.d, "--strict"])
        self.assertEqual(rc, 1)
        self.assertIn(f"strict: {FNAME}:2:", out)

    def test_missing_dir_exit_2(self):
        rc, _, err = captured(rl.main, ["lint", "--dir", os.path.join(self.d, "nope")])
        self.assertEqual(rc, 2)
        self.assertIn("no such ledger dir", err)

    def test_usage_error_exit_2(self):
        rc, _, _ = captured(rl.main, ["bogus"])
        self.assertEqual(rc, 2)


class RealLedgerTests(unittest.TestCase):
    def test_real_ledger_has_no_L4_or_L5(self):
        if not rl.DEFAULT_DIR.is_dir():
            self.skipTest(f"no ledger at {rl.DEFAULT_DIR}")
        bad = [i for i in rl.lint(str(rl.DEFAULT_DIR)) if i.rule in ("L4", "L5")]
        self.assertFalse(bad, "\n".join(f"{i.file}:{i.line} {i.rule} {i.msg}" for i in bad))


if __name__ == "__main__":
    unittest.main()
