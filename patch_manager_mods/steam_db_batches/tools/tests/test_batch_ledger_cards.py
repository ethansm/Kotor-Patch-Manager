"""Gap tests for batch.py: validated ledger (check-validated / persist / finalize), gate_tier, candidates
--name-regex / --extra-candidates, source_strength, and the card leak audit (DESIGN.md 4.1, 4.3, 4.6, 4.7).

Fixtures come from tests/test_batch.py (World, full_world, Base, run, write_candidates, write_reviews_and_overrides).
Run from tools/:  python3 -m unittest discover -s tests
"""
import contextlib
import io
import re
import sqlite3
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_batch import (B1_SPEC, GOG_BASE, STEAM_BASE, Base, batch, bd, h, run, sha,  # noqa: E402
                        write_candidates, write_reviews_and_overrides)


def run_exit(*argv):
    """Run batch.main expecting die(): -> (exit code, stdout, stderr)."""
    out, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        try:
            rc = batch.main(list(argv))
        except SystemExit as e:
            rc = e.code
    return rc, out.getvalue(), err.getvalue()


def tables(path):
    con = sqlite3.connect(str(path))
    try:
        return {r[0] for r in con.execute("select name from sqlite_master where type='table'")}
    finally:
        con.close()


class FinalizedB1(Base):
    """B1 candidates -> gates -> reviews/overrides -> finalize (the FinalizePersistTests recipe)."""

    def setUp(self):
        super().setUp()
        write_candidates(self.root, "B1", B1_SPEC)
        run("gates", "B1")
        write_reviews_and_overrides(self.root)
        rc, out = run("finalize", "B1", "--date", "2026-10-03")
        self.assertEqual(rc, 0, out)
        self.bdir = self.root / "batches" / "B1"

    def set_branch(self, path, rows):
        con = sqlite3.connect(str(path))
        con.execute("CREATE TABLE IF NOT EXISTS functions(id INTEGER PRIMARY KEY AUTOINCREMENT,class_name TEXT NOT NULL,"
                    "function_name TEXT NOT NULL,address INTEGER NOT NULL,notes TEXT,calling_convention TEXT,"
                    "param_size_bytes INTEGER,UNIQUE(class_name,function_name))")
        con.execute("delete from functions")
        for cls, fn, addr in rows:
            con.execute("insert into functions(class_name,function_name,address) values(?,?,?)", (cls, fn, addr))
        con.commit()
        con.close()

    def insert_mark(self, addr, name, fp_sha, batch_name="B1", verdict="verified", tier="auto:c2"):
        con = sqlite3.connect(str(self.world.kdb))
        con.execute(batch.VALIDATED_DDL)
        con.execute("insert into steam_validated values(?,?,?,?,?,?,?,?,?)",
                    (addr, name, verdict, tier, "", batch_name, fp_sha, "ref", "2026-10-03"))
        con.commit()
        con.close()


# ------------------------------------------------------------------------------------------------ 1. check-validated
class CheckValidatedTests(FinalizedB1):
    def test_explicit_branch_db_overrides_env_branchdb(self):
        other = self.root / "other_branch.db"
        self.set_branch(other, [("CFoo", "Alpha", 0x510000), ("CFoo", "OnlyHere", 0x510F00)])
        self.set_branch(self.world.branchdb, [("CFoo", "FromEnv", 0x510100)])        # BD_BRANCHDB: must be ignored
        out_tsv = self.root / "explicit.tsv"
        rc, out = run("check-validated", "--branch-db", str(other), "--out", str(out_tsv))
        self.assertEqual(rc, 0, out)
        names = [r["name"] for r in bd.iter_tsv(out_tsv)]
        self.assertEqual(names, ["CFoo::Alpha", "CFoo::OnlyHere"])
        self.assertIn("branch rows 2", out)

    def test_branch_db_is_opened_read_only(self):
        other = self.root / "other_branch.db"
        self.set_branch(other, [("CFoo", "Alpha", 0x510000)])
        before_b, before_k = sha(other), sha(self.world.kdb)
        rc, _out = run("check-validated", "--branch-db", str(other))
        self.assertEqual(rc, 0)
        self.assertEqual((before_b, before_k), (sha(other), sha(self.world.kdb)))

    def test_kdb_without_steam_validated_table_marks_everything_unvalidated(self):
        self.assertNotIn("steam_validated", tables(self.world.kdb))              # persist has not run
        self.set_branch(self.world.branchdb, [("CFoo", "Alpha", 0x510000), ("CFoo", "Theta", 0x5102C0),
                                              ("CFoo", "Never", 0x510500)])
        out_tsv = self.root / "none.tsv"
        rc, out = run("check-validated", "--out", str(out_tsv))
        self.assertEqual(rc, 0, out)
        rows = list(bd.iter_tsv(out_tsv))
        self.assertEqual(len(rows), 3)
        self.assertEqual({r["status"] for r in rows}, {"unvalidated"})
        self.assertEqual({(r["verdict"], r["tier"], r["batch"], r["note"]) for r in rows}, {("", "", "", "")})
        self.assertIn("validated 0 (skip), stale 0, unvalidated 3", out)
        self.assertNotIn("steam_validated", tables(self.world.kdb))              # the read never creates the table

    def test_empty_branch_db_reports_zero_rows(self):
        self.set_branch(self.world.branchdb, [])
        rc, out = run("check-validated")
        self.assertEqual(rc, 0)
        self.assertIn("branch rows 0: validated 0 (skip), stale 0, unvalidated 0", out)

    def test_mark_with_empty_fp_hash_is_validated_and_never_stale(self):
        # Behaviour pinned from the code: `if m[fp_sha] and shas.get(addr) != m[fp_sha]` -> an empty hash skips the
        # staleness test, so the mark is `validated` even though the Steam fp.tsv has a body hash for that address.
        # write_validated stores '' when the address is missing from fp.tsv, so such a mark can never go stale
        # (REPORTED: looks weak; a body that later appears/changes at that address is not detected).
        self.assertIn(0x510000, batch.steam_fp_shas(bd.get_paths()))             # a real hash exists for Alpha
        self.insert_mark("0x00510000", "CFoo::Alpha", "")
        self.set_branch(self.world.branchdb, [("CFoo", "Alpha", 0x510000)])
        out_tsv = self.root / "empty_fp.tsv"
        rc, out = run("check-validated", "--out", str(out_tsv))
        self.assertEqual(rc, 0, out)
        (row,) = list(bd.iter_tsv(out_tsv))
        self.assertEqual((row["status"], row["note"]), ("validated", ""))
        self.assertEqual((row["verdict"], row["tier"], row["batch"]), ("verified", "auto:c2", "B1"))

    def test_mark_with_null_fp_hash_behaves_like_empty(self):
        con = sqlite3.connect(str(self.world.kdb))
        con.execute(batch.VALIDATED_DDL)
        con.execute("insert into steam_validated(steam_addr,name,verdict,batch) values('0x00510000','CFoo::Alpha','verified','B1')")
        con.commit()
        con.close()
        self.set_branch(self.world.branchdb, [("CFoo", "Alpha", 0x510000)])
        out_tsv = self.root / "null_fp.tsv"
        rc, _out = run("check-validated", "--out", str(out_tsv))
        self.assertEqual(rc, 0)
        self.assertEqual([r["status"] for r in bd.iter_tsv(out_tsv)], ["validated"])

    def test_mark_whose_hash_matches_is_validated_and_a_wrong_hash_is_stale(self):
        good = batch.steam_fp_shas(bd.get_paths())[0x510000]
        self.insert_mark("0x00510000", "CFoo::Alpha", good)
        self.insert_mark("0x00510080", "CFoo::Beta", "deadbeefdeadbeef")
        self.set_branch(self.world.branchdb, [("CFoo", "Alpha", 0x510000), ("CFoo", "Beta", 0x510080)])
        out_tsv = self.root / "hash.tsv"
        run("check-validated", "--out", str(out_tsv))
        st = {r["name"]: r for r in bd.iter_tsv(out_tsv)}
        self.assertEqual(st["CFoo::Alpha"]["status"], "validated")
        self.assertEqual(st["CFoo::Beta"]["status"], "stale")
        self.assertEqual(st["CFoo::Beta"]["note"], "Steam body hash changed since B1")

    def test_out_file_columns_order_and_row_content(self):
        good = batch.steam_fp_shas(bd.get_paths())[0x510000]
        self.insert_mark("0x00510000", "CFoo::Alpha", good, tier="opus")
        self.insert_mark("0x00510240", "CFoo::Eta", "", batch_name="B3", verdict="likely", tier="reviewer")
        # branch rows deliberately inserted out of address order: output is `order by address`
        self.set_branch(self.world.branchdb, [("CFoo", "Eta", 0x510240), ("CFoo", "Never", 0x510500),
                                              ("CFoo", "Alpha", 0x510000), ("CFoo", "Theta", 0x510FF0)])
        self.insert_mark("0x005102c0", "CFoo::Theta", "")                         # same name, other address
        out_tsv = self.root / "out.tsv"
        rc, out = run("check-validated", "--out", str(out_tsv))
        self.assertEqual(rc, 0, out)
        self.assertEqual(out_tsv.read_text().split("\n")[0], "\t".join(batch.CHECK_COLS))
        rows = list(bd.iter_tsv(out_tsv))
        self.assertEqual([r["steam_addr"] for r in rows], ["0x00510000", "0x00510240", "0x00510500", "0x00510ff0"])
        by = {r["name"]: r for r in rows}
        self.assertEqual((by["CFoo::Alpha"]["status"], by["CFoo::Alpha"]["tier"], by["CFoo::Alpha"]["verdict"]),
                         ("validated", "opus", "verified"))
        self.assertEqual((by["CFoo::Eta"]["status"], by["CFoo::Eta"]["batch"], by["CFoo::Eta"]["verdict"]),
                         ("validated", "B3", "likely"))
        self.assertEqual(by["CFoo::Never"]["status"], "unvalidated")
        self.assertEqual(by["CFoo::Theta"]["status"], "stale")
        self.assertEqual(by["CFoo::Theta"]["note"], "validated at 0x005102c0, branch has 0x00510ff0")
        self.assertIn("validated 2 (skip), stale 1, unvalidated 1", out)

    def test_show_limits_listed_rows_but_not_the_out_file(self):
        self.set_branch(self.world.branchdb, [("CFoo", "N%d" % i, 0x510000 + 0x10 * i) for i in range(5)])
        out_tsv = self.root / "show.tsv"
        rc, out = run("check-validated", "--show", "2", "--out", str(out_tsv))
        self.assertEqual(rc, 0)
        self.assertEqual(len([ln for ln in out.split("\n") if ln.startswith("  unvalidated")]), 2)
        self.assertEqual(len(list(bd.iter_tsv(out_tsv))), 5)

    def test_without_out_nothing_is_written(self):
        before = set(self.root.rglob("*"))
        rc, _out = run("check-validated")
        self.assertEqual(rc, 0)
        self.assertEqual(before, set(self.root.rglob("*")))


# ------------------------------------------------------------------------------------------------ 2. persist needs validated.tsv
class PersistNeedsLedgerTests(FinalizedB1):
    def setUp(self):
        super().setUp()
        (self.bdir / "validated.tsv").unlink()

    def test_dry_run_without_validated_tsv_exits_2_and_writes_nothing(self):
        before = sha(self.world.kdb)
        rc, out, err = run_exit("persist", "B1")
        self.assertEqual(rc, 2)
        self.assertIn("validated.tsv missing", err)
        self.assertIn("finalize", err)
        self.assertIn("branch_apply.py", err)
        self.assertNotIn("DRY-RUN", out)
        self.assertEqual(before, sha(self.world.kdb))
        self.assertFalse((self.bdir / "validated.tsv").exists())

    def test_apply_without_validated_tsv_exits_2_and_writes_nothing(self):
        before = sha(self.world.kdb)
        rc, out, err = run_exit("persist", "B1", "--apply")
        self.assertEqual(rc, 2)
        self.assertIn("validated.tsv missing", err)
        self.assertNotIn("applied", out)
        self.assertEqual(before, sha(self.world.kdb))                           # no name_map rows, no lessons, no DDL
        self.assertNotIn("steam_validated", tables(self.world.kdb))
        self.assertFalse((self.bdir / "validated.tsv").exists())

    def test_with_the_ledger_restored_persist_succeeds(self):
        rc, out = run("finalize", "B1", "--date", "2026-10-03")
        self.assertEqual(rc, 0, out)
        self.assertTrue((self.bdir / "validated.tsv").exists())
        rc, out, err = run_exit("persist", "B1")
        self.assertEqual(rc, 0, err)
        self.assertIn("DRY-RUN", out)


# ------------------------------------------------------------------------------------------------ 3. finalize / B0
class FinalizeLedgerTests(Base):
    SENTINEL = "steam_addr\tname\nSENTINEL\twritten by branch_apply\n"

    def finalize(self, name):
        write_candidates(self.root, name, B1_SPEC)
        run("gates", name)
        rc, out = run("finalize", name, "--date", "2026-10-03")
        self.assertEqual(rc, 0, out)
        return self.root / "batches" / name

    def test_b0_finalize_never_writes_validated_tsv(self):
        write_candidates(self.root, "B0", B1_SPEC)
        run("gates", "B0")
        rc, out = run("finalize", "B0", "--date", "2026-10-03")
        self.assertEqual(rc, 0, out)
        bdir = self.root / "batches" / "B0"
        self.assertFalse((bdir / "validated.tsv").exists())
        self.assertTrue((bdir / "branch_deletes.tsv").exists())                   # B0-only outputs are still written
        self.assertTrue((bdir / "branch_corrections.csv").exists())

    def test_b0_finalize_leaves_a_branch_apply_ledger_untouched(self):
        write_candidates(self.root, "B0", B1_SPEC)
        run("gates", "B0")
        bdir = self.root / "batches" / "B0"
        (bdir / "validated.tsv").write_text(self.SENTINEL)
        rc, out = run("finalize", "B0", "--date", "2026-10-03")
        self.assertEqual(rc, 0, out)
        self.assertEqual((bdir / "validated.tsv").read_text(), self.SENTINEL)

    def test_other_batches_write_validated_tsv(self):
        for name in ("B1", "B2"):
            bdir = self.finalize(name)
            led = list(bd.iter_tsv(bdir / "validated.tsv"))
            self.assertTrue(led, name)
            self.assertEqual({r["batch"] for r in led}, {name})
            self.assertEqual(list(led[0].keys()), batch.VALIDATED_COLS)
            self.assertFalse((bdir / "branch_deletes.tsv").exists())              # B0-only

    def test_other_batch_finalize_overwrites_a_stale_ledger(self):
        write_candidates(self.root, "B1", B1_SPEC)
        run("gates", "B1")
        bdir = self.root / "batches" / "B1"
        (bdir / "validated.tsv").write_text(self.SENTINEL)
        run("finalize", "B1", "--date", "2026-10-03")
        self.assertNotIn("SENTINEL", (bdir / "validated.tsv").read_text())

    def test_b0_persist_dry_run_requires_branch_apply_ledger(self):
        write_candidates(self.root, "B0", B1_SPEC)
        run("gates", "B0")
        run("finalize", "B0", "--date", "2026-10-03")
        before = sha(self.world.kdb)
        rc, _out, err = run_exit("persist", "B0")
        self.assertEqual(rc, 2)
        self.assertIn("branch_apply.py", err)
        self.assertEqual(before, sha(self.world.kdb))


# ------------------------------------------------------------------------------------------------ 4. gate_tier
class GateTierTests(unittest.TestCase):
    def test_override_note_is_opus(self):
        self.assertEqual(batch.gate_tier({"final_note": "override: gate was wrong", "gate_class": "REVIEW"}), "opus")

    def test_override_beats_auto(self):
        self.assertEqual(batch.gate_tier({"final_note": "override x", "gate_class": "AUTO",
                                          "flags": "auto_route=sig"}), "opus")

    def test_auto_c2_route(self):
        self.assertEqual(batch.gate_tier({"gate_class": "AUTO", "flags": "auto_route=c2", "final_note": ""}), "auto:c2")

    def test_auto_sig_route(self):
        self.assertEqual(batch.gate_tier({"gate_class": "AUTO", "flags": "g6=review;auto_route=sig", "final_note": ""}),
                         "auto:sig")

    def test_auto_without_route_flag_defaults_to_c2(self):
        for flags in ("", None, "other_flag"):
            self.assertEqual(batch.gate_tier({"gate_class": "AUTO", "flags": flags}), "auto:c2", flags)

    def test_review_row_is_reviewer(self):
        self.assertEqual(batch.gate_tier({"gate_class": "REVIEW", "final_note": "reviewer note", "flags": ""}), "reviewer")
        self.assertEqual(batch.gate_tier({"gate_class": "REVIEW", "flags": "auto_route=sig"}), "reviewer")  # not AUTO

    def test_missing_keys_default_to_reviewer(self):
        self.assertEqual(batch.gate_tier({}), "reviewer")
        self.assertEqual(batch.gate_tier({"final_note": None, "gate_class": None}), "reviewer")

    def test_non_prefix_override_text_is_not_an_override(self):
        self.assertEqual(batch.gate_tier({"final_note": "see override elsewhere", "gate_class": "REVIEW"}), "reviewer")


# ------------------------------------------------------------------------------------------------ 5. candidates
class NameRegexExtraCandidatesTests(Base):
    def candidates(self, *extra_args, batch_name="B7"):
        rc, out = run("candidates", batch_name, "--classes", "CFoo", "--no-reexport",
                      "--findings-dir", str(self.root / "findings"), *extra_args)
        self.assertEqual(rc, 0, out)
        rows = {r["name"]: r for r in bd.iter_tsv(self.root / "batches" / batch_name / "candidates.tsv")}
        return rows, out

    def extra_file(self, lines, name="extra_candidates.tsv"):
        p = self.root / name
        p.write_text("gog\tsteam\tsource\n" + "".join(l + "\n" for l in lines))
        return p

    def test_name_regex_keeps_only_matching_functions(self):
        rows, _out = self.candidates("--name-regex", r"^(Alpha|Beta|Gamma)$")
        self.assertEqual(set(rows), {"Alpha", "Beta", "Gamma"})

    def test_name_regex_is_a_search_not_a_full_match(self):
        rows, _out = self.candidates("--name-regex", "ta$")            # Beta, Zeta, Eta, Theta, Delta, Iota, ...
        self.assertTrue({"Beta", "Zeta", "Eta", "Theta", "Delta", "Iota"} <= set(rows))
        self.assertFalse({"Alpha", "Gamma", "Lambda"} & set(rows))
        self.assertTrue(all(n.endswith("ta") for n in rows))

    def test_name_regex_matching_nothing_warns_and_writes_header_only(self):
        rows, out = self.candidates("--name-regex", "^NoSuchFunction$")
        self.assertEqual(rows, {})
        self.assertIn("warning: class CFoo has no GOG functions", out)
        self.assertEqual((self.root / "batches" / "B7" / "candidates.tsv").read_text().strip(), "\t".join(batch.CANDIDATE_COLS))

    def test_without_name_regex_all_class_rows_are_kept(self):
        rows, _out = self.candidates()
        self.assertTrue({"Alpha", "Beta", "Lambda", "X1", "X2"} <= set(rows))

    def test_extra_candidate_is_tagged_with_the_file_source(self):
        extra = self.extra_file(["%s\t%s\tcmdtable" % (h(GOG_BASE + 0x380), h(STEAM_BASE + 0x2345))])
        rows, _out = self.candidates("--name-regex", "^(Lambda|Alpha)$", "--extra-candidates", str(extra))
        self.assertEqual(set(rows), {"Lambda", "Alpha"})
        lam = dict(batch.parse_candidates(rows["Lambda"]["candidates"]))
        self.assertEqual(lam[STEAM_BASE + 0x2345], ["cmdtable"])               # Lambda had no candidate before
        alpha = dict(batch.parse_candidates(rows["Alpha"]["candidates"]))
        self.assertNotIn(STEAM_BASE + 0x2345, alpha)                           # only the row named by `gog` gets it

    def test_extra_candidate_merges_tags_with_an_existing_candidate_extra_first(self):
        extra = self.extra_file(["%s\t%s\tcmdtable_overwritten" % (h(GOG_BASE), h(STEAM_BASE))])
        rows, _out = self.candidates("--name-regex", "^Alpha$", "--extra-candidates", str(extra))
        cands = batch.parse_candidates(rows["Alpha"]["candidates"])
        self.assertEqual(len([a for a, _t in cands if a == STEAM_BASE]), 1)    # one entry for the shared address
        tags = dict(cands)[STEAM_BASE]
        self.assertEqual(tags[0], "cmdtable_overwritten")
        self.assertIn("mapper:vote", tags)
        self.assertEqual(batch.source_strength(tags), 6)

    def test_extra_for_a_filtered_out_function_does_not_appear(self):
        extra = self.extra_file(["%s\t%s\tcmdtable" % (h(GOG_BASE + 0x80), h(STEAM_BASE + 0x7777))])
        rows, _out = self.candidates("--name-regex", "^Alpha$", "--extra-candidates", str(extra))
        self.assertEqual(set(rows), {"Alpha"})
        text = (self.root / "batches" / "B7" / "candidates.tsv").read_text()
        self.assertNotIn("00517777", text)

    def test_extra_source_whitespace_is_stripped_and_multiple_rows_per_gog_accumulate(self):
        extra = self.extra_file(["%s\t%s\t cmdtable " % (h(GOG_BASE + 0x380), h(STEAM_BASE + 0x2000)),
                                 "%s\t%s\tcmdtable_overwritten" % (h(GOG_BASE + 0x380), h(STEAM_BASE + 0x2040))])
        rows, _out = self.candidates("--name-regex", "^Lambda$", "--extra-candidates", str(extra))
        cands = dict(batch.parse_candidates(rows["Lambda"]["candidates"]))
        self.assertEqual(cands[STEAM_BASE + 0x2000], ["cmdtable"])
        self.assertEqual(cands[STEAM_BASE + 0x2040], ["cmdtable_overwritten"])

    def test_extra_candidates_then_gates_treat_the_cmdtable_candidate_as_proof_source(self):
        extra = self.extra_file(["%s\t%s\tcmdtable" % (h(GOG_BASE + 0x2C0), h(STEAM_BASE + 0x2C0))])
        self.candidates("--name-regex", "^Theta$", "--extra-candidates", str(extra), batch_name="B7")
        ev = {"cand_sources": "0x005102c0|cmdtable+near"}
        self.assertEqual(batch.proof_candidates(ev), {0x005102C0})


class SourceStrengthTests(unittest.TestCase):
    def test_cmdtable_sources_rank_six(self):
        self.assertEqual(batch.source_strength(["cmdtable"]), 6)
        self.assertEqual(batch.source_strength(["cmdtable_overwritten"]), 6)

    def test_cmdtable_outranks_vote_branch_seeded_near_but_not_server(self):
        for weaker in ("mapper:vote", "branch", "mapper:seeded", "near", "name_map:medium:x"):
            self.assertGreater(batch.source_strength(["cmdtable"]), batch.source_strength([weaker]), weaker)
        self.assertLess(batch.source_strength(["cmdtable"]), batch.source_strength(["server"]))

    def test_strength_is_the_max_over_all_tags(self):
        self.assertEqual(batch.source_strength(["near", "cmdtable", "branch"]), 6)
        self.assertEqual(batch.source_strength(["cmdtable", "server"]), 7)
        self.assertEqual(batch.source_strength([]), 0)


# ------------------------------------------------------------------------------------------------ 6. cards leak audit
WITHHELD = "Candidate sources: (withheld on every card; judge the body)"


def line_type(line):
    """Normalise a card line to its label: strip hex addresses and numbers, drop everything after the first ':'."""
    head = line.split(":", 1)[0]
    head = re.sub(r"0x[0-9a-fA-F]+|\b[0-9a-fA-F]{8}\b", "", head)
    head = re.sub(r"\d+", "", head)
    return re.sub(r"\s+", " ", head).strip()


def split_cards(chunk_text):
    """{card_id: [lines]}"""
    out = {}
    for c in chunk_text.split("## Card ")[1:]:
        lines = c.split("\n")
        out[lines[0].strip()] = lines[1:]
    return out


class CardLeakAuditTests(Base):
    def setUp(self):
        super().setUp()
        self.bdir = self.root / "batches" / "B1"
        # Give every Steam function a (content-free) fingerprint row: the card prints a different GOG-side line
        # ("Steam candidate fingerprint missing") for a candidate without one, which would otherwise occur only
        # on the fixture's fp-less filler decoys (a fixture artefact, not a leak; see the report).
        for a, f in self.world.steam.items():
            if a not in self.world.steam_fp:
                self.world.fp("steam", a, f["retn"], size=f["size"])
        self.world.build()
        write_candidates(self.root, "B1", B1_SPEC)
        run("gates", "B1")
        ev = list(bd.iter_tsv(self.bdir / "evidence.tsv"))
        self.review = [e for e in ev if e["gate_class"] == "REVIEW"]
        self.assertGreaterEqual(len(self.review), 4)
        # one GOG-side fact (two for the first row) per REVIEW function
        facts = [{"gog": e["gog"], "fact": "slot %d of the command table" % (10 + i)} for i, e in enumerate(self.review)]
        facts.append({"gog": self.review[0]["gog"], "fact": "second fact: sibling of ActionX"})
        batch.write_tsv(self.bdir / "card_facts.tsv", ["gog", "fact"], facts)
        self.facts = {}
        for f in facts:
            self.facts.setdefault(f["gog"], []).append(f["fact"])
        rc, out = run("cards", "B1")
        self.assertEqual(rc, 0, out)
        cards_dir = self.bdir / "cards"
        self.key = {k["card_id"]: k for k in bd.iter_tsv(cards_dir / "key.tsv")}
        self.cards = {}
        for p in sorted(cards_dir.glob("chunk_*.md")):
            self.cards.update(split_cards(p.read_text()))
        self.real = {c: l for c, l in self.cards.items() if self.key[c]["kind"] == "real"}
        self.decoy = {c: l for c, l in self.cards.items() if self.key[c]["kind"] != "real"}

    def test_fixture_has_both_real_and_decoy_cards_and_key_covers_every_card(self):
        self.assertEqual(set(self.cards), set(self.key))
        self.assertEqual(len(self.real), len(self.review))
        self.assertGreaterEqual(len(self.decoy), batch.MIN_DECOYS)
        self.assertTrue(all(k["kind"] in ("real", "decoy_prev", "decoy_next") for k in self.key.values()))

    def test_every_card_withholds_candidate_sources(self):
        for cid, lines in self.cards.items():
            self.assertEqual(lines.count(WITHHELD), 1, "%s (%s)" % (cid, self.key[cid]["kind"]))
        self.assertTrue(all(not any(l.startswith("Candidate sources:") and l != WITHHELD for l in ls)
                            for ls in self.cards.values()))

    def test_no_card_mentions_source_tags_or_answer_key_words(self):
        text = "\n".join("\n".join(ls) for ls in self.cards.values())
        for needle in ("mapper:", "name_map:", "cmdtable", "decoy_prev", "decoy_next", "mismatch\t"):
            self.assertNotIn(needle, text)

    def test_card_facts_appear_on_real_and_decoy_cards_of_that_gog_function(self):
        seen_real = seen_decoy = 0
        for cid, lines in self.cards.items():
            gog = self.key[cid]["gog"]
            got = [l[len("- GOG-side fact: "):] for l in lines if l.startswith("- GOG-side fact: ")]
            self.assertEqual(got, self.facts.get(gog, []), "%s (%s) gog %s" % (cid, self.key[cid]["kind"], gog))
            if self.facts.get(gog):
                if cid in self.real:
                    seen_real += 1
                else:
                    seen_decoy += 1
        self.assertGreater(seen_real, 0)
        self.assertGreater(seen_decoy, 0)

    def test_fact_lines_are_formatted_identically_on_real_and_decoy_cards(self):
        gog = self.review[0]["gog"]
        fact_lines = {cid: [l for l in lines if l.startswith("- GOG-side fact:")]
                      for cid, lines in self.cards.items() if self.key[cid]["gog"] == gog}
        kinds = {self.key[c]["kind"] == "real" for c in fact_lines}
        self.assertEqual(kinds, {True, False}, "first REVIEW row should have a real and a decoy card")
        for lines in fact_lines.values():
            self.assertEqual(lines, ["- GOG-side fact: " + f for f in self.facts[gog]])

    def types(self, cards):
        per_card = {cid: {line_type(l) for l in lines if l.strip()} for cid, lines in cards.items()}
        union = set().union(*per_card.values())
        inter = set.intersection(*per_card.values())
        return per_card, union, inter

    def test_no_line_type_is_unique_to_real_or_decoy_cards(self):
        _pr, real_union, real_all = self.types(self.real)
        _pd, decoy_union, decoy_all = self.types(self.decoy)
        # weak form (always required): nothing present on ALL real cards is absent from every decoy card, and v.v.
        self.assertEqual(real_all - decoy_union, set(), "types on every real card and on no decoy card")
        self.assertEqual(decoy_all - real_union, set(), "types on every decoy card and on no real card")
        # strong form: the structured part of the card (everything above the body sections) has the same types
        # on real and decoy cards, taken over all cards of each kind
        def structured(cards):
            out = set()
            for lines in cards.values():
                for l in lines:
                    if l.startswith("### abi"):
                        break
                    if l.strip():
                        out.add(line_type(l))
            return out
        self.assertEqual(structured(self.real), structured(self.decoy))

    def test_every_section_heading_occurs_on_real_and_decoy_cards(self):
        def headings(cards):
            return {line_type(l) for ls in cards.values() for l in ls if l.startswith("### ")}
        self.assertEqual(headings(self.real), headings(self.decoy))
        # the heading set contains every fixed section
        for sec in ("### Computed facts (script output, do not recompute)", "### abi", "### prologue", "### anchors"):
            self.assertIn(line_type(sec), headings(self.real))

    def test_computed_fact_lines_have_the_same_shape_on_every_card(self):
        shapes = {}
        for cid, lines in self.cards.items():
            kind = "real" if cid in self.real else "decoy"
            for l in lines:
                if l.startswith("- ") and not l.startswith("- GOG-side fact:"):
                    shapes.setdefault(line_type(l), set()).add(kind)
        # every `- label:` computed-facts / neighbour line exists for both kinds (server-name line may be body-dependent)
        for label, kinds in shapes.items():
            if label.startswith("- server Steam name"):
                continue
            self.assertEqual(kinds, {"real", "decoy"}, label)

    def test_decoy_cards_name_a_decoy_candidate_that_differs_from_the_hosts_row(self):
        host = {e["gog"]: e["steam_addr"] for e in self.review}
        for cid in self.decoy:
            k = self.key[cid]
            self.assertIn(k["gog"], host)
            self.assertNotEqual(k["steam_addr"], host[k["gog"]])
            self.assertTrue(any(l.startswith("Candidate Steam address: %s " % k["steam_addr"]) for l in self.decoy[cid]))

    def test_real_cards_show_the_reviewed_candidate(self):
        host = {e["gog"]: e["steam_addr"] for e in self.review}
        for cid in self.real:
            k = self.key[cid]
            self.assertEqual(k["steam_addr"], host[k["gog"]])
            self.assertTrue(any(l.startswith("Candidate Steam address: %s " % k["steam_addr"]) for l in self.real[cid]))


if __name__ == "__main__":
    unittest.main()
