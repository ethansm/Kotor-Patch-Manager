"""Tests for tools/calibrate.py on a synthetic world (gold sets, hold-out untrust, decoy-reaches-AUTO listing)."""
import contextlib
import csv
import io
import json
import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent))
import calibrate  # noqa: E402
import score_review  # noqa: E402,F401
import bdlib as bd  # noqa: E402
from test_batch import GOG_BASE, STEAM_BASE, World, h  # noqa: E402

TRUSTED = "engine-ai-atlas wave2"


def g(off):
    return GOG_BASE + off


def s(off):
    return STEAM_BASE + off


def calib_world(root) -> World:
    """Class CCal: four positives (all retn 8, spaced 0x40) with STR+SIG anchors, plus gold-set edge cases.

    Good0/1/2: the trusted Steam address carries the anchors.  Swap: the trusted address S (0x180) has none, but its
    same-ret next neighbour D (0x1c0) carries all of Swap's GOG anchors -> D is a decoy that reaches AUTO.
    """
    w = World(root)
    for i, off in enumerate((0x000, 0x080, 0x100)):
        w.pair("CCal", "Good%d" % i, g(off), s(off), 8, 8, anchors=("STR", "SIG"))
        w.name_map.append((h(s(off)), "CCal::Good%d" % i, TRUSTED, "high", "audited"))
    for off in (0x040, 0x0C0, 0x140, 0x200, 0x240):
        w.steam_fn(s(off), "FUN_%08x" % s(off), retn=8)
    w.pair("CCal", "Swap", g(0x180), s(0x1C0), 8, 8, anchors=("STR", "SIG"))
    w.steam_fn(s(0x180), "FUN_%08x" % s(0x180), retn=8)
    w.name_map.append((h(s(0x180)), "CCal::Swap", TRUSTED, "high", "audited"))
    # not positives: untrusted source, and a name with two trusted addresses
    w.filler("CCal", "Untrusted", g(0x280), s(0x280))
    w.name_map.append((h(s(0x280)), "CCal::Untrusted", "batch5_reverse_engineering", "high", "projection"))
    w.filler("CCal", "Amb", g(0x2C0), s(0x2C0))
    w.b1.append(("CCal", "Amb", h(s(0x2C0))))
    w.steam_fn(s(0x300), "FUN_%08x" % s(0x300), retn=8)
    w.name_map.append((h(s(0x300)), "CCal::Amb", TRUSTED, "high", "audited"))
    # negatives: b1 rejected candidate, name_map wrong row, a known-false mapper address
    w.filler("CCal", "Rej", g(0x340), s(0x340))
    w.steam_fn(s(0x380), "FUN_%08x" % s(0x380), retn=8)
    w.b1_rejected.append(("CCal", "Rej", s(0x380)))
    w.filler("CCal", "Wr", g(0x3C0), s(0x3C0))
    w.steam_fn(s(0x400), "FUN_%08x" % s(0x400), retn=8)
    w.name_map.append((h(s(0x400)), "CCal::Wr", "gog_cluster_delta_map.py", "wrong", "known false"))
    w.gog_fn(g(0x440), "CSWSCreature", "FooBar", param=8)
    w.steam_fn(s(0x440), "FUN_%08x" % s(0x440), retn=8)
    return w.build()


def write_known_false(w: World):
    fd = w.dump / "findings"
    fd.mkdir(exist_ok=True)
    (fd / "AI0_mapper_summary.md").write_text(
        "## 3. Known false mappings (watch list)\nRe-mapped by this run (all `vote`): 0x%06x (FooBar, size=30), "
        "0x%06x (Nothing).\nNot mapped: 0x681880.\n"
        "Full outlier cluster in `AI0_map_CSWSCreature_all.tsv` (1 row):\n  %08x:FooBar\n\n## 4. next\n" % (
            s(0x440), 0x7FFFFF, s(0x440)))
    (fd / "AI0_map_CSWSCreature_all.tsv").write_text("0x%08x\t0x%x\tCSWSCreature::FooBar\tvote\tretn=8\n" % (s(0x440), g(0x440)))


class CalibrateBase(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.world = calib_world(self.root)
        write_known_false(self.world)
        p = mock.patch.dict(os.environ, self.world.env())
        p.start()
        self.addCleanup(p.stop)
        self.corpus = bd.Corpus(bd.get_paths())

    def run_cal(self, *argv):
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = calibrate.main(list(argv) + ["--out-dir", str(self.root / "cal")])
        self.assertEqual(rc, 0)
        return buf.getvalue()


class GoldSetTests(CalibrateBase):
    def test_positives_exclude_untrusted_and_ambiguous(self):
        pos, origins, amb = calibrate.build_positives(self.corpus)
        names = {self.corpus.gog_by_addr[a].name for a in pos}
        self.assertEqual(names, {"CCal::Good0", "CCal::Good1", "CCal::Good2", "CCal::Swap"})
        self.assertEqual(pos[g(0)], s(0))
        self.assertTrue(origins[g(0)].startswith("name_map:" + TRUSTED))
        self.assertEqual([a for a, _ in amb], [g(0x2C0)])                  # Amb: b1 and name_map disagree
        self.assertEqual(amb[0][1], [s(0x2C0), s(0x300)])

    def test_negatives_and_known_false(self):
        pos, _o, _a = calibrate.build_positives(self.corpus)
        halves = calibrate.assign_halves(pos, 1)
        neg, known = calibrate.build_negatives(self.corpus, pos, halves, 1)
        by = {c.kind: c for c in neg}
        self.assertEqual(by["neg_b1rej"].steam, s(0x380))
        self.assertEqual(self.corpus.gog_by_addr[by["neg_b1rej"].gog].name, "CCal::Rej")
        self.assertEqual(by["neg_wrong"].steam, s(0x400))
        self.assertEqual(by["neg_known"].steam, s(0x440))
        self.assertEqual(known, [(g(0x440), s(0x440), "CSWSCreature::FooBar")])   # the unmappable 0x7fffff is dropped
        self.assertTrue(all(c.expected == "mismatch" for c in neg))

    def test_decoys_prev_next_random_are_same_retn_and_deterministic(self):
        pos, _o, _a = calibrate.build_positives(self.corpus)
        halves = calibrate.assign_halves(pos, 1)
        d1 = calibrate.build_decoys(self.corpus, pos, halves, calibrate.SEED)
        d2 = calibrate.build_decoys(self.corpus, pos, halves, calibrate.SEED)
        self.assertEqual([(c.kind, c.gog, c.steam) for c in d1], [(c.kind, c.gog, c.steam) for c in d2])
        good1 = {c.kind: c.steam for c in d1 if c.gog == g(0x080)}
        self.assertEqual(good1["decoy_prev"], s(0x040))
        self.assertEqual(good1["decoy_next"], s(0x0C0))
        pretn, psize = 8, self.corpus.steam[s(0x080)].size
        r = self.corpus.steam[good1["decoy_rand"]]
        self.assertEqual(r.retn, pretn)
        self.assertLessEqual(abs(r.size - psize), 0.1 * psize)
        self.assertNotIn(good1["decoy_rand"], (s(0x080), good1["decoy_prev"], good1["decoy_next"]))
        # a different seed picks (very likely) a different random decoy but never changes prev/next
        d3 = calibrate.build_decoys(self.corpus, pos, halves, 1)
        self.assertEqual([c.steam for c in d1 if c.kind == "decoy_next"], [c.steam for c in d3 if c.kind == "decoy_next"])

    def test_halves_split_by_rank_parity_or_block(self):
        pos, _o, _a = calibrate.build_positives(self.corpus)
        self.assertEqual(calibrate.assign_halves(pos, 1), {g(0): "A", g(0x80): "B", g(0x100): "A", g(0x180): "B"})
        self.assertEqual(calibrate.assign_halves(pos, 2), {g(0): "A", g(0x80): "A", g(0x100): "B", g(0x180): "B"})


class ShiftTests(CalibrateBase):
    def test_shift_candidates_follow_steam_function_starts(self):
        pos, _o, _a = calibrate.build_positives(self.corpus)
        halves = calibrate.assign_halves(pos, calibrate.SHIFT_BLOCK)
        cases = calibrate.build_shifts(self.corpus, pos, halves)
        by = {(c.kind, c.gog): c.steam for c in cases}
        self.assertEqual(by[("shift_p1", g(0))], s(0x40))
        self.assertEqual(by[("shift_p2", g(0))], s(0x80))                # Good1's true address
        self.assertNotIn(("shift_m1", g(0)), by)                          # no function start before the first
        self.assertNotIn(("shift_m2", g(0)), by)
        self.assertEqual(by[("shift_m1", g(0x80))], s(0x40))
        self.assertEqual(by[("shift_m2", g(0x100))], s(0x80))
        self.assertEqual(by[("shift_m1", g(0x100))], s(0xC0))
        self.assertTrue(all(c.steam in self.corpus.steam for c in cases))
        self.assertTrue(all(halves[c.gog] == c.half == "A" for c in cases))   # one block-20 half holds all 4 positives
        self.assertEqual(calibrate.contiguous_runs(pos, self.corpus, halves), [[g(0), g(0x80), g(0x100), g(0x180)]])

    def test_runs_are_one_gates_run_tagged_vote_with_the_whole_half_untrusted(self):
        out = self.run_cal("--quick")
        cal = self.root / "cal"
        rows = list(bd.iter_tsv(cal / "runs" / "novote_A_shift_p1" / "candidates.tsv"))
        self.assertEqual(len(rows), 4)                                    # all rows of the segment in ONE run
        self.assertTrue(all("mapper:vote" in r["candidates"] for r in rows))   # even in the novote variant
        self.assertTrue((cal / "runs" / "novote_A_shift_m2" / "evidence.tsv").exists())
        ev = {r["name"]: r for r in bd.iter_tsv(cal / "runs" / "novote_A_shift_p1" / "evidence.tsv")}
        # Swap's shift_p1 lands on D, which carries Swap's anchors: it reaches AUTO and must be listed
        self.assertEqual(ev["Swap"]["gate_class"], "AUTO")
        auto = [r for r in bd.iter_tsv(cal / "auto_negatives.tsv") if r["kind"] == "shift_p1"]
        self.assertEqual([r["name"] for r in auto], ["Swap"])
        m = json.loads((cal / "metrics.json").read_text())["novote"]
        self.assertEqual(m["stress_auto"], 1)
        self.assertEqual(m["negatives_auto"], 0)                          # ordinary decoys carry no vote in novote; stress counted apart
        self.assertIn("shift_p1", (cal / "CALIBRATION_draft.md").read_text())

    def test_whole_half_untrusted_in_shift_runs(self):
        gold = calibrate.build_gold(self.corpus, 1, calibrate.SEED)        # main split is rank parity, shifts use block 20
        plan = {(h, k): (cases, base) for h, k, cases, base in gold["plan"]}
        cases, base = plan[("A", "shift_p1")]
        self.assertEqual(base, {s(0), s(0x80), s(0x100), s(0x180)})        # every positive of the block-20 half
        self.assertEqual(len(cases), 4)
        self.assertNotIn(("B", "shift_p1"), plan)

    def test_outlier_cluster_is_one_run(self):
        out = self.run_cal("--quick")
        rows = list(bd.iter_tsv(self.root / "cal" / "runs" / "novote_C_neg_cluster" / "candidates.tsv"))
        self.assertEqual([r["name"] for r in rows], ["FooBar"])
        self.assertIn("mapper:vote", rows[0]["candidates"])
        self.assertIn("neg_cluster=1", out)


class HoldOutTests(CalibrateBase):
    def test_untrust_covers_the_half_and_the_candidate_only(self):
        gold = calibrate.build_gold(self.corpus, 1, calibrate.SEED)
        scorer = bd.Scorer(self.corpus)
        pos_b = {c.steam for c in gold["cases"] if c.kind == "pos" and c.half == "B"}
        pos_a = {c.steam for c in gold["cases"] if c.kind == "pos" and c.half == "A"}
        self.assertEqual(pos_b, {s(0x080), s(0x180)})
        cases = [c for c in gold["cases"] if c.kind == "decoy_next" and c.half == "B"]
        evs, untrust = calibrate.run_cases(self.corpus, scorer, self.root / "cal", "novote", "B", "decoy_next",
                                           cases, pos_b)
        self.assertTrue(pos_b <= untrust)
        self.assertFalse(pos_a & untrust)
        self.assertTrue({c.steam for c in cases} <= untrust)       # the candidate itself never helps (G6 cannot mask)
        for a in pos_b:
            self.assertFalse(self.corpus.trust.is_trusted(a))
            self.assertNotIn(a, self.corpus.pairs.values())
        for a in pos_a:
            self.assertTrue(self.corpus.trust.is_trusted(a))
            self.assertIn(a, self.corpus.pairs.values())
        # one candidate per GOG function, written as a synthetic candidates.tsv
        rows = list(bd.iter_tsv(self.root / "cal" / "runs" / "novote_B_decoy_next" / "candidates.tsv"))
        self.assertEqual(len(rows), len(cases))
        self.assertTrue(all(";" not in r["candidates"] for r in rows))
        self.assertEqual(len(evs), len(cases))

    def test_own_trusted_entry_is_hidden_from_its_half(self):
        """A half-B positive is gated with its own address untrusted: the library-name exemption must not apply."""
        self.world.steam_fn(s(0x080), "FID_conflict:something", retn=8, source="ANALYSIS")
        self.world.build()
        corpus = bd.Corpus(bd.get_paths())
        scorer = bd.Scorer(corpus)
        gold = calibrate.build_gold(corpus, 1, calibrate.SEED)
        cases = [c for c in gold["cases"] if c.kind == "pos" and c.half == "B" and c.gog == g(0x080)]
        pos_b = {c.steam for c in gold["cases"] if c.kind == "pos" and c.half == "B"}
        evs, _u = calibrate.run_cases(corpus, scorer, self.root / "cal", "vote", "B", "pos", cases, pos_b)
        self.assertEqual(evs[0]["g1"], "warn:autolabel")           # trusted would have skipped the library test
        self.assertEqual(evs[0]["gate_class"], "REVIEW")


class EndToEndTests(CalibrateBase):
    def test_decoy_reaching_auto_is_listed(self):
        out = self.run_cal("--quick", "--variants", "vote", "--cards-pos", "3", "--cards-neg", "3")
        cal = self.root / "cal"
        auto = list(bd.iter_tsv(cal / "auto_negatives.tsv"))
        swap = [r for r in auto if r["name"] == "Swap" and r["kind"] == "decoy_next"]
        self.assertEqual(len(swap), 1, [r["name"] for r in auto])
        self.assertEqual((swap[0]["kind"], swap[0]["steam_addr"]), ("decoy_next", h(s(0x1C0))))
        self.assertEqual(swap[0]["gate_class"], "AUTO")
        self.assertIn("AUTO NEGATIVE decoy_next", out)
        draft = (cal / "CALIBRATION_draft.md").read_text()
        self.assertIn("## Decoys / negatives that reached AUTO (vote)", draft)
        self.assertIn("CCal::Swap", draft)
        self.assertIn("0x%08x" % s(0x1C0), draft)
        m = json.loads((cal / "metrics.json").read_text())["vote"]
        self.assertEqual(m["negatives_auto"], 1)
        self.assertGreaterEqual(m["positive_auto"], 3)             # Good0..2 reach AUTO with their STR+SIG anchors
        self.assertAlmostEqual(m["positive_auto_recall"], m["positive_auto"] / m["positives"])
        self.assertEqual(m["per_kind"]["pos"]["REVIEW"], 1)        # Swap's trusted address has no anchors
        self.assertEqual(m["g3_na_all"], 0)

    def test_novote_variant_reports_and_cards(self):
        self.run_cal("--variants", "novote", "--cards-pos", "2", "--cards-neg", "4")
        cal = self.root / "cal"
        for f in ("CALIBRATION_draft.md", "metrics.json", "auto_negatives.tsv", "positive_rejects.tsv", "gold_sets.tsv"):
            self.assertTrue((cal / f).exists(), f)
        runs = {p.name for p in (cal / "runs").iterdir()}
        self.assertIn("novote_A_pos", runs)
        self.assertIn("novote_B_decoy_prev", runs)
        key = list(bd.iter_tsv(cal / "cards" / "key.tsv"))
        kinds = {k["kind"] for k in key}
        self.assertIn("pos", kinds)
        self.assertTrue(kinds - {"pos"})
        self.assertTrue(all((k["expected"] == "match") == (k["kind"] == "pos") for k in key))
        chunk = (cal / "cards" / "chunk_1.md").read_text()
        self.assertEqual(chunk.count("## Card "), len(key))
        self.assertNotIn("\tmismatch", chunk)
        self.assertNotIn("decoy_prev", chunk)                      # kinds live only in key.tsv

    def test_sets_only_runs_nothing(self):
        out = self.run_cal("--sets-only")
        self.assertIn("gold sets: positives=4", out)
        self.assertFalse((self.root / "cal" / "runs").exists())
        self.assertTrue((self.root / "cal" / "gold_sets.tsv").exists())


class AnnotationAndChunkTests(CalibrateBase):
    def write_ann(self, rows):
        p = self.root / "ann.tsv"
        p.write_text("gog_addr\tsteam_addr\tannotation\tnote\n" + "".join("%s\t%s\t%s\tnote\n" % r for r in rows))
        return p

    def test_label_suspect_pairs_leave_the_gold_sets(self):
        ann = calibrate.load_annotations(self.write_ann([(h(g(0x180)), h(s(0x180)), "label_suspect"),     # a positive
                                                         (h(g(0x340)), h(s(0x380)), "label_suspect"),     # neg_b1rej
                                                         (h(g(0x100)), h(s(0x100)), "other")]))           # not excluded
        gold = calibrate.build_gold(self.corpus, 1, calibrate.SEED, ann)
        gogs = {c.gog for c in gold["cases"]}
        self.assertNotIn(g(0x180), gogs)                           # positive AND all its decoys / shifts are gone
        self.assertIn(g(0x100), gogs)
        self.assertEqual(gold["counts"]["positives"], 3)
        self.assertEqual(gold["counts"]["neg_b1rej"], 0)
        self.assertTrue(any(c.kind == "pos" and c.gog == g(0x180) for c in gold["excluded"]))
        # the suspect positive's address stays untrusted in its half's base set
        bases = [base for h_, k, cases, base in gold["plan"] if k == "pos"]
        self.assertTrue(any(s(0x180) in b for b in bases))

    def test_blank_steam_label_suspect_and_positive_corrections(self):
        p = self.write_ann([(h(g(0x100)), "", "label_suspect"),                                   # whole GOG function
                            (h(g(0x340)), h(s(0x340)), "positive")])                              # gold correction (Rej -> s(0x340))
        ann = calibrate.load_annotations(p)
        self.assertEqual(ann[(g(0x100), None)][0], "label_suspect")
        gold = calibrate.build_gold(self.corpus, 1, calibrate.SEED, ann)
        gogs = {c.gog for c in gold["cases"]}
        self.assertNotIn(g(0x100), gogs)                                    # positive, decoys and shifts all gone
        pos = {c.gog: c for c in gold["cases"] if c.kind == "pos"}
        self.assertEqual(pos[g(0x340)].steam, s(0x340))
        self.assertEqual(pos[g(0x340)].note, "annotation")
        self.assertEqual(gold["corrected"], [(g(0x340), s(0x340))])
        # the correction replaces any other gold positive that used the same Steam address
        p2 = self.write_ann([(h(g(0x340)), h(s(0x000)), "positive")])
        gold2 = calibrate.build_gold(self.corpus, 1, calibrate.SEED, calibrate.load_annotations(p2))
        self.assertEqual([c.gog for c in gold2["cases"] if c.kind == "pos" and c.steam == s(0)], [g(0x340)])

    def test_body_conflicting_trusted_row_is_no_gold_positive(self):
        self.world.name_map.append((h(s(0x500)), "CCal::Wr2", TRUSTED, "high", "trusted"))
        self.world.filler("CCal", "Wr2", g(0x500), s(0x500), retn=4, param=4)                  # GOG retn 4 ...
        self.world.fp("gog", g(0x500), 4)
        self.world.fp("steam", s(0x500), 8)                                                    # ... Steam fingerprint says 8
        self.world.build()
        corpus = bd.Corpus(bd.get_paths())
        pos, _o, _a = calibrate.build_positives(corpus)
        self.assertNotIn(g(0x500), pos)
        self.assertIn("CCal::Wr2", [n for _g, _s, n, _w in corpus.pair_conflicts])

    def test_annotations_flow_through_the_metrics(self):
        p = self.write_ann([(h(g(0x180)), h(s(0x180)), "label_suspect")])
        self.run_cal("--quick", "--variants", "vote", "--annotations", str(p))
        m = json.loads((self.root / "cal" / "metrics.json").read_text())["vote"]
        self.assertEqual(m["positives"], 3)
        self.assertEqual(m["stress_auto"], 0)                      # Swap's shift_p1 -> D is gone with its function
        self.assertEqual(m["negatives_auto"], 0)                   # ... and so is its decoy_next -> D
        self.assertIn("Gold annotations", (self.root / "cal" / "CALIBRATION_draft.md").read_text())

    def test_second_chunk_excludes_earlier_rows_and_keeps_chunk_one(self):
        self.run_cal("--quick", "--cards-pos", "2", "--cards-neg", "3")
        cards = self.root / "cal" / "cards"
        before = (cards / "chunk_1.md").read_bytes(), (cards / "key.tsv").read_bytes()
        self.run_cal("--quick", "--chunk-id", "2", "--cards-seed", "20261004", "--exclude-cards", str(cards / "key.tsv"),
                     "--cards-pos", "5", "--cards-neg", "30")
        self.assertEqual(before, ((cards / "chunk_1.md").read_bytes(), (cards / "key.tsv").read_bytes()))
        k1 = {r["gog"] for r in bd.iter_tsv(cards / "key.tsv")}
        k2 = list(bd.iter_tsv(cards / "key_2.tsv"))
        self.assertTrue(k2)
        self.assertFalse(k1 & {r["gog"] for r in k2})
        self.assertTrue(all(r["card_id"].startswith("CAL-2-") and r["chunk"] == "2" for r in k2))
        self.assertIn("## Card CAL-2-01", (cards / "chunk_2.md").read_text())


class ScoreDecoyDecisionTests(unittest.TestCase):
    def run_case(self, n_decoys, accepted_idx):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        d = Path(tmp.name)
        key = ["card_id\tchunk\tgog\tsteam_addr\tkind\texpected"]
        rev = ["gog_addr\tsteam_addr\tverdict\tcorrected_addr\tcontent_anchor\tconfidence\treason"]
        for i in range(n_decoys):
            g_, s_ = 0x400000 + 0x10 * i, 0x500000 + 0x10 * i
            key.append("C%d\t1\t0x%08x\t0x%08x\tdecoy_next\tmismatch" % (i, g_, s_))
            rev.append("0x%08x\t0x%08x\t%s\t\tx\t3\tr" % (g_, s_, "match" if i in accepted_idx else "mismatch"))
        (d / "k.tsv").write_text("\n".join(key) + "\n")
        (d / "r.tsv").write_text("\n".join(rev) + "\n")
        res = score_review.score(d / "k.tsv", d / "r.tsv")
        return score_review.escalation(res), score_review.render(res, "k", "r")

    def test_zero_tolerance_and_minimum(self):
        (decision, accepted), text = self.run_case(10, set())
        self.assertTrue(decision.startswith("OK"))
        self.assertIn("chunk 1: 10 decoys, 0 accepted", text)
        (decision, accepted), text = self.run_case(10, {4})
        self.assertTrue(decision.startswith("ESCALATE"))
        self.assertEqual(len(accepted), 1)
        self.assertIn("100%", decision)
        (decision, _a), _t = self.run_case(9, set())
        self.assertTrue(decision.startswith("INSUFFICIENT"))


class ScoreReviewTests(unittest.TestCase):
    def test_confusion_precision_and_annotated_rows(self):
        import score_review
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        d = Path(tmp.name)
        (d / "key.tsv").write_text(
            "card_id\tchunk\tgog\tsteam_addr\tkind\texpected\n"
            "C1\t1\t0x00400010\t0x00500010\tpos\tmatch\n"
            "C2\t1\t0x00400020\t0x00500020\tpos\tmatch\n"
            "C3\t1\t0x00400030\t0x00500030\tdecoy_prev\tmismatch\n"
            "C4\t1\t0x00400040\t0x00500040\tdecoy_next\tmismatch\n"
            "C5\t1\t0x00400050\t0x00500050\tneg_b1rej\tmismatch\n"       # annotated label_suspect
            "C6\t1\t0x00400060\t0x00500060\tpos\tmatch\n")                 # no review row
        head = "gog_addr\tsteam_addr\tverdict\tcorrected_addr\tcontent_anchor\tconfidence\treason\n"
        (d / "rev.tsv").write_text(head +
                                   "0x00400010\t0x00500010\tmatch\t\tx\t4\tok\n"
                                   "0x00400020\t0x00500020\tunsure\t\t\t2\tno anchor\n"
                                   "0x00400030\t0x00500030\tmatch\t\tx\t3\tfalse accept\n"
                                   "0x00400040\t0x00500040\tmismatch\t\tx\t4\tfine\n"
                                   "0x00400050\t0x00500050\tmatch\t\tx\t4\tlabel looks swapped\n")
        (d / "ann.tsv").write_text("gog_addr\tsteam_addr\tannotation\tnote\n0x00400050\t0x00500050\tlabel_suspect\tn\n")
        res = score_review.score(d / "key.tsv", d / "rev.tsv", d / "ann.tsv")
        c = res["confusion"]
        self.assertEqual(dict(c["expected match"]), {"match": 1, "unsure": 1, "missing": 1})
        self.assertEqual(dict(c["expected mismatch"]), {"match": 1, "mismatch": 1})
        self.assertEqual(dict(c["annotated label_suspect"]), {"match": 1})
        self.assertEqual((res["tp"], res["matches"]), (1, 2))              # the annotated `match` is not counted
        self.assertAlmostEqual(res["precision"], 0.5)
        self.assertEqual([r["card_id"] for r in res["false_accepts"]], ["C3"])
        self.assertEqual([r["card_id"] for r in res["annotated"]], ["C5"])
        text = score_review.render(res, "k", "r")
        self.assertIn("| annotated label_suspect |", text)
        self.assertIn("1 / 2 = 50.0%", text)


if __name__ == "__main__":
    unittest.main()
