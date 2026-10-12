#!/usr/bin/env python3
"""Tests for scripts/evidence_quote_check.py (stdlib unittest, synthetic fixture bundle only).

Run: python3 -I scripts/test_evidence_quote_check.py
Prints a final `evidence_quote_check tests: N/N PASS` line and exits non-zero on any failure.
"""
import copy
import hashlib
import io
import json
import subprocess
import sys
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import evidence_quote_check as eqc  # noqa: E402

SCRIPT = HERE / "evidence_quote_check.py"

A_LINES = [(10, "Mod init ok"), (11, "Loaded 12 entries"), (12, "WARN slot  3 missing   texture"),
           (13, "ERROR  null pointer in  handler"), (14, "Shutdown complete")]
B_LINES = [(40, "Frame hook  attached"), (41, "Queue length 0"), (42, "Tick 99 done")]


def sha(s):
    return hashlib.sha256(s.encode("utf-8")).hexdigest()


def make_excerpt(log, launch, kind, anchor_line, pairs):
    lines = [{"n": n, "text": t} for n, t in pairs]
    tsha = sha("\n".join(t for _, t in pairs))
    a, b = pairs[0][0], pairs[-1][0]
    return {"id": f"ev:{log}:L{a}-{b}:{tsha[:8]}", "log": log, "launch": launch,
            "anchor": {"kind": kind, "line": anchor_line}, "start": a, "end": b,
            "lines": lines, "text_sha256": tsha, "line_cut": False}


def finalize(bundle):
    body = {k: v for k, v in bundle.items() if k != "bundle_sha256"}
    canon = json.dumps(body, sort_keys=True, separators=(",", ":"), ensure_ascii=False)
    bundle["bundle_sha256"] = sha(canon)
    return bundle


def make_bundle():
    ex_a = make_excerpt("playlog", 1, "alert", 13, A_LINES)
    ex_b = make_excerpt("playlog", 1, "banner", 41, B_LINES)
    return finalize({"format": "playtest-bundle/1",
                     "intake": {"symptom": "thing broke", "kind": "logic", "mods": ["m"], "save": "s"},
                     "logs": [], "evidence": [ex_a, ex_b], "caps": {}, "truncated": False,
                     "dropped": [], "scrub": {}, "gaps": []})


BUNDLE = make_bundle()
ID_A = BUNDLE["evidence"][0]["id"]
ID_B = BUNDLE["evidence"][1]["id"]


def hyp(hid, evidence, **kw):
    h = {"id": hid, "claim": "c", "mechanism": "m", "evidence": evidence, "confidence": "low", "falsifier": "f"}
    h.update(kw)
    return h


def cit(eid, quote):
    return {"evidence_id": eid, "quote": quote}


def blind(*hyps):
    return {"unit_id": "blind-skeptic", "hypotheses": list(hyps)}


def null(verdict, *reasons):
    return {"unit_id": "null-1", "verdict": verdict, "reasons": list(reasons)}


def reason(eid, quote, text="t"):
    return {"text": text, "evidence_id": eid, "quote": quote}


def run_cli(args):
    out, err = io.StringIO(), io.StringIO()
    with redirect_stdout(out), redirect_stderr(err):
        try:
            code = eqc.main(args)
        except SystemExit as e:                   # argparse errors
            code = e.code
    return code, out.getvalue(), err.getvalue()


class Files:
    """Temp dir with a bundle file and an output file."""

    def __init__(self, case, bundle, output):
        self.td = tempfile.TemporaryDirectory()
        case.addCleanup(self.td.cleanup)
        root = Path(self.td.name)
        self.bundle = root / "bundle.json"
        self.output = root / "out.json"
        self.bundle.write_text(json.dumps(bundle), encoding="utf-8")
        self.output.write_text(json.dumps(output), encoding="utf-8")

    def args(self, *extra):
        return ["--bundle", str(self.bundle), "--output", str(self.output), *extra]


class BlindQuotes(unittest.TestCase):
    def one(self, citations, **kw):
        v = eqc.check(BUNDLE, blind(hyp("h1", citations, **kw)))
        self.assertEqual(len(v), 1)
        return v[0]

    def test_verdict_shape(self):
        v = self.one([cit(ID_A, "Loaded 12 entries")])
        self.assertEqual(set(v) >= {"id", "valid", "reason", "valid_evidence_ids"}, True)
        self.assertIs(v["valid"], True)
        self.assertEqual(v["id"], "h1")
        self.assertIsInstance(v["reason"], str)
        self.assertEqual(v["valid_evidence_ids"], [ID_A])

    def test_plain_quote_passes(self):
        self.assertTrue(self.one([cit(ID_A, "ERROR  null pointer in  handler")])["valid"])

    def test_partial_line_quote_passes(self):
        self.assertTrue(self.one([cit(ID_A, "null pointer")])["valid"])

    def test_whitespace_varied_quote_passes(self):
        # excerpt has runs of spaces, the quote has single spaces / tabs / newlines
        self.assertTrue(self.one([cit(ID_A, "WARN slot 3 missing texture")])["valid"])
        self.assertTrue(self.one([cit(ID_A, "  WARN\tslot   3\nmissing texture  ")])["valid"])
        self.assertTrue(self.one([cit(ID_B, "Frame hook attached")])["valid"])

    def test_line_prefixed_quote_passes(self):
        self.assertTrue(self.one([cit(ID_A, "L12| WARN slot  3 missing   texture")])["valid"])
        self.assertTrue(self.one([cit(ID_A, "L12|WARN slot 3 missing texture")])["valid"])

    def test_multiline_prefixed_quote_passes(self):
        q = "L12| WARN slot  3 missing   texture\nL13| ERROR  null pointer in  handler"
        self.assertTrue(self.one([cit(ID_A, q)])["valid"])
        q2 = "L12| WARN slot 3 missing texture L13| ERROR null pointer in handler"
        self.assertTrue(self.one([cit(ID_A, q2)])["valid"])
        q3 = "WARN slot 3 missing texture\nERROR null pointer in handler"
        self.assertTrue(self.one([cit(ID_A, q3)])["valid"])

    def test_non_adjacent_lines_rejected(self):
        q = "L11| Loaded 12 entries\nL13| ERROR  null pointer in  handler"
        self.assertFalse(self.one([cit(ID_A, q)])["valid"])

    def test_fabricated_quote_rejected(self):
        v = self.one([cit(ID_A, "Segmentation fault in the renderer")])
        self.assertFalse(v["valid"])
        self.assertTrue(v["reason"])
        self.assertEqual(v["valid_evidence_ids"], [])

    def test_case_sensitive(self):
        self.assertFalse(self.one([cit(ID_A, "loaded 12 entries")])["valid"])

    def test_paraphrase_and_ellipsis_rejected(self):
        self.assertFalse(self.one([cit(ID_A, "Loaded entries 12")])["valid"])
        self.assertFalse(self.one([cit(ID_A, "Mod init ok ... Shutdown complete")])["valid"])

    def test_unknown_id_rejected(self):
        self.assertFalse(self.one([cit("ev:playlog:L10-14:deadbeef", "Loaded 12 entries")])["valid"])
        self.assertFalse(self.one([cit("", "Loaded 12 entries")])["valid"])
        self.assertFalse(self.one([cit("Loaded 12 entries", "Loaded 12 entries")])["valid"])

    def test_quote_from_other_excerpt_rejected(self):
        # "Tick 99 done" is in B; citing it under A's id is fabricated
        self.assertFalse(self.one([cit(ID_A, "Tick 99 done")])["valid"])
        self.assertTrue(self.one([cit(ID_B, "Tick 99 done")])["valid"])

    def test_trivial_quote_rejected(self):
        for q in ("", " ", "ok", "Mod", "Queue", "Queue ", "L11| ok", "L11|", "L11| ", "   \n  "):
            v = self.one([cit(ID_A if q.strip() != "Queue" else ID_B, q)])
            self.assertFalse(v["valid"], q)

    def test_min_chars_boundary(self):
        self.assertTrue(self.one([cit(ID_B, "Tick 9")])["valid"])         # 6 chars
        self.assertFalse(self.one([cit(ID_B, "Tick ")])["valid"])         # 4 after normalisation
        self.assertFalse(self.one([cit(ID_B, "Tick")])["valid"])
        self.assertEqual(eqc.MIN_CHARS, 6)

    def test_uncited_hypothesis_rejected(self):
        v = self.one([])
        self.assertFalse(v["valid"])
        self.assertEqual(v["valid_evidence_ids"], [])

    def test_missing_evidence_key_rejected(self):
        h = hyp("h1", [])
        del h["evidence"]
        v = eqc.check(BUNDLE, blind(h))
        self.assertEqual(len(v), 1)
        self.assertFalse(v[0]["valid"])

    def test_non_list_evidence_rejected(self):
        for bad in (None, "Loaded 12 entries", {"evidence_id": ID_A, "quote": "Loaded 12 entries"}, 5, True):
            v = eqc.check(BUNDLE, blind(hyp("h1", bad)))
            self.assertEqual(len(v), 1)
            self.assertFalse(v[0]["valid"], bad)

    def test_malformed_citation_rejected(self):
        for bad in ("Loaded 12 entries", None, 3, [ID_A, "Loaded 12 entries"],
                    {"quote": "Loaded 12 entries"}, {"evidence_id": ID_A},
                    {"evidence_id": ID_A, "quote": None}, {"evidence_id": ID_A, "quote": 12345678},
                    {"evidence_id": None, "quote": "Loaded 12 entries"}, {"evidence_id": ["x"], "quote": "Loaded 12 entries"}):
            v = eqc.check(BUNDLE, blind(hyp("h1", [bad])))
            self.assertFalse(v[0]["valid"], bad)

    def test_mixed_valid_and_fabricated_rejects_whole_hypothesis(self):
        v = self.one([cit(ID_A, "Loaded 12 entries"), cit(ID_A, "this line was never logged")])
        self.assertFalse(v["valid"])
        self.assertEqual(v["valid_evidence_ids"], [])
        v = self.one([cit(ID_A, "this line was never logged"), cit(ID_B, "Tick 99 done")])
        self.assertFalse(v["valid"])

    def test_mixed_valid_and_unknown_id_rejects_whole_hypothesis(self):
        v = self.one([cit(ID_A, "Loaded 12 entries"), cit("ev:nope:L1-2:00000000", "Loaded 12 entries")])
        self.assertFalse(v["valid"])

    def test_mixed_valid_and_trivial_rejects_whole_hypothesis(self):
        self.assertFalse(self.one([cit(ID_A, "Loaded 12 entries"), cit(ID_A, "Mod")])["valid"])

    def test_two_valid_citations_across_excerpts_pass(self):
        v = self.one([cit(ID_A, "Mod init ok"), cit(ID_B, "Queue length 0"), cit(ID_A, "Shutdown complete")])
        self.assertTrue(v["valid"])
        self.assertEqual(v["valid_evidence_ids"], [ID_A, ID_B])           # deduplicated, first-seen order

    def test_quote_spanning_two_excerpts_rejected(self):
        # "Shutdown complete" ends A, "Frame hook attached" starts B: adjacent in the bundle, not in one excerpt
        q = "Shutdown complete Frame hook attached"
        self.assertFalse(self.one([cit(ID_A, q)])["valid"])
        self.assertFalse(self.one([cit(ID_B, q)])["valid"])
        q2 = "L14| Shutdown complete\nL40| Frame hook  attached"
        self.assertFalse(self.one([cit(ID_A, q2)])["valid"])

    def test_each_hypothesis_judged_independently(self):
        v = eqc.check(BUNDLE, blind(hyp("h1", [cit(ID_A, "Loaded 12 entries")]),
                                    hyp("h2", [cit(ID_A, "invented text here")]),
                                    hyp("h3", []),
                                    hyp("h4", [cit(ID_B, "Queue length 0")])))
        self.assertEqual([x["id"] for x in v], ["h1", "h2", "h3", "h4"])
        self.assertEqual([x["valid"] for x in v], [True, False, False, True])

    def test_non_dict_hypothesis_rejected(self):
        out = {"unit_id": "u", "hypotheses": ["text", None, 3, hyp("h1", [cit(ID_A, "Mod init ok")])]}
        v = eqc.check(BUNDLE, out)
        self.assertEqual([x["valid"] for x in v], [False, False, False, True])

    def test_hypothesis_without_id_still_gets_a_verdict(self):
        h = hyp("h1", [cit(ID_A, "Mod init ok")])
        del h["id"]
        v = eqc.check(BUNDLE, blind(h))
        self.assertEqual(len(v), 1)
        self.assertIsInstance(v[0]["id"], str)
        self.assertTrue(v[0]["id"])

    def test_empty_hypotheses_gives_no_verdicts(self):
        self.assertEqual(eqc.check(BUNDLE, blind()), [])

    def test_hypotheses_not_a_list_rejected(self):
        for bad in (None, "x", {"id": "h1"}, 4):
            v = eqc.check(BUNDLE, {"unit_id": "u", "hypotheses": bad})
            self.assertEqual(len(v), 1, bad)
            self.assertFalse(v[0]["valid"], bad)

    def test_output_not_an_object_or_empty_rejected(self):
        for bad in (None, [], "x", 3, {}, {"unit_id": "u"}):
            v = eqc.check(BUNDLE, bad)
            self.assertEqual(len(v), 1, bad)
            self.assertFalse(v[0]["valid"], bad)

    def test_inputs_not_mutated(self):
        b, o = copy.deepcopy(BUNDLE), blind(hyp("h1", [cit(ID_A, "L12| WARN slot 3 missing texture")]))
        o0 = copy.deepcopy(o)
        eqc.check(b, o)
        self.assertEqual(b, BUNDLE)
        self.assertEqual(o, o0)

    def test_strip_prefix_helper_only_strips_prefixes(self):
        self.assertEqual(eqc.norm_quote("L12| a\nL13|  b"), "a b")
        self.assertEqual(eqc.norm_quote("not L12 | a"), "not L12 | a")
        self.assertEqual(eqc.norm_quote("tailL12| a"), "tailL12| a")


class NullQuotes(unittest.TestCase):
    def test_valid_reason_passes(self):
        v = eqc.check(BUNDLE, null("not_a_bug", reason(ID_B, "Queue length 0")))
        self.assertEqual(len(v), 1)
        self.assertTrue(v[0]["valid"])
        self.assertEqual(v[0]["valid_evidence_ids"], [ID_B])
        self.assertEqual(v[0]["id"], "r1")

    def test_prefixed_and_whitespace_reason_passes(self):
        v = eqc.check(BUNDLE, null("config", reason(ID_A, "L12| WARN slot 3   missing texture")))
        self.assertTrue(v[0]["valid"])

    def test_fabricated_reason_rejected(self):
        v = eqc.check(BUNDLE, null("bug", reason(ID_A, "no such line was logged")))
        self.assertFalse(v[0]["valid"])
        self.assertEqual(v[0]["valid_evidence_ids"], [])

    def test_unknown_id_and_trivial_reason_rejected(self):
        self.assertFalse(eqc.check(BUNDLE, null("bug", reason("ev:x:L1-2:abcd1234", "Mod init ok")))[0]["valid"])
        self.assertFalse(eqc.check(BUNDLE, null("bug", reason(ID_A, "Mod")))[0]["valid"])
        self.assertFalse(eqc.check(BUNDLE, null("bug", reason(ID_A, "")))[0]["valid"])

    def test_spanning_quote_reason_rejected(self):
        self.assertFalse(eqc.check(BUNDLE, null("bug", reason(ID_A, "Shutdown complete Frame hook attached")))[0]["valid"])

    def test_missing_keys_and_wrong_types_rejected(self):
        for bad in ({"text": "t", "quote": "Mod init ok"}, {"text": "t", "evidence_id": ID_A},
                    {}, "Mod init ok", None, 7, {"text": "t", "evidence_id": ID_A, "quote": 5}):
            v = eqc.check(BUNDLE, {"unit_id": "n", "verdict": "bug", "reasons": [bad]})
            self.assertEqual(len(v), 1, bad)
            self.assertFalse(v[0]["valid"], bad)

    def test_each_reason_judged_independently(self):
        v = eqc.check(BUNDLE, null("config", reason(ID_A, "Mod init ok"), reason(ID_A, "never logged anywhere"),
                                   reason(ID_B, "Tick 99 done")))
        self.assertEqual([x["id"] for x in v], ["r1", "r2", "r3"])
        self.assertEqual([x["valid"] for x in v], [True, False, True])

    def test_insufficient_may_have_no_reasons(self):
        self.assertEqual(eqc.check(BUNDLE, null("insufficient")), [])

    def test_decisive_verdict_without_reasons_rejected(self):
        for verdict in ("bug", "not_a_bug", "config", "engine_as_designed"):
            v = eqc.check(BUNDLE, null(verdict))
            self.assertEqual(len(v), 1, verdict)
            self.assertFalse(v[0]["valid"], verdict)

    def test_missing_verdict_without_reasons_rejected(self):
        v = eqc.check(BUNDLE, {"unit_id": "n", "reasons": []})
        self.assertEqual(len(v), 1)
        self.assertFalse(v[0]["valid"])

    def test_reasons_not_a_list_rejected(self):
        for bad in (None, "x", {"text": "t"}, 3):
            v = eqc.check(BUNDLE, {"unit_id": "n", "verdict": "insufficient", "reasons": bad})
            self.assertEqual(len(v), 1, bad)
            self.assertFalse(v[0]["valid"], bad)

    def test_verify_shaped_output_checked_the_same_way(self):
        out = {"unit_id": "verify-blind-skeptic-h1", "hypothesis_id": "h1", "verdict": "supported",
               "reasons": [reason(ID_A, "ERROR  null pointer in  handler"), reason(ID_A, "made up")]}
        self.assertEqual([x["valid"] for x in eqc.check(BUNDLE, out)], [True, False])
        out["verdict"] = "inconclusive"
        out["reasons"] = []
        self.assertEqual(eqc.check(BUNDLE, out), [])
        out["verdict"] = "supported"
        self.assertFalse(eqc.check(BUNDLE, out)[0]["valid"])


class ShortEvidenceIds(unittest.TestCase):
    """`ev:<sha8>` resolves to the one excerpt whose id ends in `:<sha8>`; ambiguous or unknown otherwise."""
    SHORT_A = "ev:" + ID_A.rsplit(":", 1)[-1]           # ev:a80f7613
    SHORT_B = "ev:" + ID_B.rsplit(":", 1)[-1]

    def twin(self):
        # excerpt B replaced by a second excerpt with A's text under another log name: same sha8, other id
        b = copy.deepcopy(BUNDLE)
        b["evidence"][1] = make_excerpt("banner", 1, "alert", 13, A_LINES)
        return finalize(b)

    def test_short_id_one_match_resolves_to_full_id(self):
        v = eqc.check(BUNDLE, blind(hyp("h1", [cit(self.SHORT_A, "Loaded 12 entries")])))
        self.assertTrue(v[0]["valid"], v[0]["reason"])
        self.assertEqual(v[0]["valid_evidence_ids"], [ID_A])
        v = eqc.check(BUNDLE, blind(hyp("h1", [cit(self.SHORT_B, "Tick 99 done")])))
        self.assertEqual(v[0]["valid_evidence_ids"], [ID_B])

    def test_short_id_two_matches_is_ambiguous(self):
        b = self.twin()
        v = eqc.check(b, blind(hyp("h1", [cit(self.SHORT_A, "Loaded 12 entries")])))
        self.assertFalse(v[0]["valid"])
        self.assertEqual(v[0]["reason"], f"ambiguous evidence id {self.SHORT_A} (2 excerpts)")
        self.assertEqual(v[0]["valid_evidence_ids"], [])
        # the full ids of the two twins still resolve exactly
        v = eqc.check(b, blind(hyp("h1", [cit(ID_A, "Loaded 12 entries")])))
        self.assertTrue(v[0]["valid"])

    def test_short_id_zero_matches_is_unknown(self):
        v = eqc.check(BUNDLE, blind(hyp("h1", [cit("ev:00000000", "Loaded 12 entries")])))
        self.assertFalse(v[0]["valid"])
        self.assertEqual(v[0]["reason"], "unknown evidence id ev:00000000")

    def test_malformed_short_ids_rejected_as_unknown(self):
        bad = ("ev:" + self.SHORT_A[3:10],                  # ev: + 7 hex
               "ev:" + self.SHORT_A[3:] + "a",              # ev: + 9 hex
               "ev:" + self.SHORT_A[3:].upper(),            # uppercase hex
               self.SHORT_A + ":extra")                     # trailing segment
        for eid in bad:
            v = eqc.check(BUNDLE, blind(hyp("h1", [cit(eid, "Loaded 12 entries")])))
            self.assertFalse(v[0]["valid"], eid)
            self.assertEqual(v[0]["reason"], f"unknown evidence id {eid}", eid)

    def test_short_id_quote_not_in_that_excerpt_rejected(self):
        # "Tick 99 done" is only in B; citing it under A's short id is fabricated
        v = eqc.check(BUNDLE, blind(hyp("h1", [cit(self.SHORT_A, "Tick 99 done")])))
        self.assertFalse(v[0]["valid"])
        self.assertIn("not found verbatim", v[0]["reason"])
        self.assertIn(ID_A, v[0]["reason"])                  # message names the resolved full id

    def test_exact_full_id_behaves_as_before(self):
        v = eqc.check(BUNDLE, blind(hyp("h1", [cit(ID_A, "Loaded 12 entries")])))
        self.assertEqual((v[0]["valid"], v[0]["reason"], v[0]["valid_evidence_ids"]), (True, "", [ID_A]))
        v = eqc.check(BUNDLE, blind(hyp("h1", [cit(ID_A, "Tick 99 done")])))
        self.assertEqual(v[0]["reason"], f"{ID_A}: quote not found verbatim in that excerpt")

    def test_blind_output_takes_resolver(self):
        v = eqc.check(BUNDLE, blind(hyp("h1", [cit(self.SHORT_B, "Queue length 0")])))
        self.assertEqual((v[0]["valid"], v[0]["valid_evidence_ids"]), (True, [ID_B]))

    def test_null_output_takes_resolver(self):
        v = eqc.check(BUNDLE, null("not_a_bug", reason(self.SHORT_B, "Queue length 0")))
        self.assertEqual((v[0]["valid"], v[0]["valid_evidence_ids"]), (True, [ID_B]))

    def test_verify_output_takes_resolver(self):
        out = {"unit_id": "verify-blind-skeptic-h1", "hypothesis_id": "h1", "verdict": "supported",
               "reasons": [reason(self.SHORT_A, "ERROR  null pointer in  handler")]}
        v = eqc.check(BUNDLE, out)
        self.assertEqual((v[0]["valid"], v[0]["valid_evidence_ids"]), (True, [ID_A]))

    def test_hypothesis_with_short_and_full_ids_is_valid(self):
        v = eqc.check(BUNDLE, blind(hyp("h1", [cit(self.SHORT_A, "Loaded 12 entries"),
                                               cit(ID_B, "Tick 99 done")])))
        self.assertTrue(v[0]["valid"], v[0]["reason"])
        self.assertEqual(v[0]["valid_evidence_ids"], [ID_A, ID_B])


class Tamper(unittest.TestCase):
    def good_output(self):
        return blind(hyp("h1", [cit(ID_A, "Loaded 12 entries")]))

    def test_untampered_bundle_checks(self):
        self.assertTrue(eqc.check(BUNDLE, self.good_output())[0]["valid"])

    def test_edited_excerpt_text_with_stale_sha_raises(self):
        b = copy.deepcopy(BUNDLE)
        b["evidence"][0]["lines"][1]["text"] = "Loaded 99 entries"          # text_sha256 left stale
        finalize(b)                                                          # bundle hash is consistent
        with self.assertRaises(eqc.BundleTampered):
            eqc.check(b, self.good_output())

    def test_stale_text_sha_alone_raises(self):
        # lines edited AND the id re-derived from the new text, so only the recorded text_sha256 is stale
        b = copy.deepcopy(BUNDLE)
        ex = b["evidence"][0]
        ex["lines"][1]["text"] = "Loaded 99 entries"
        new = sha("\n".join(l["text"] for l in ex["lines"]))
        ex["id"] = ex["id"].rsplit(":", 1)[0] + ":" + new[:8]
        finalize(b)
        with self.assertRaises(eqc.BundleTampered) as cm:
            eqc.check(b, blind(hyp("h1", [cit(ex["id"], "Loaded 99 entries")])))
        self.assertIn("text_sha256", str(cm.exception))

    def test_edit_that_would_make_a_fabricated_quote_valid_still_raises(self):
        b = copy.deepcopy(BUNDLE)
        b["evidence"][0]["lines"][1]["text"] = "Segmentation fault in the renderer"
        finalize(b)
        with self.assertRaises(eqc.BundleTampered):
            eqc.check(b, blind(hyp("h1", [cit(ID_A, "Segmentation fault in the renderer")])))

    def test_stale_bundle_sha_raises(self):
        b = copy.deepcopy(BUNDLE)
        b["bundle_sha256"] = "0" * 64
        with self.assertRaises(eqc.BundleTampered):
            eqc.check(b, self.good_output())

    def test_edit_outside_excerpts_with_stale_bundle_sha_raises(self):
        b = copy.deepcopy(BUNDLE)
        b["intake"]["symptom"] = "something else entirely"
        with self.assertRaises(eqc.BundleTampered):
            eqc.check(b, self.good_output())

    def test_excerpt_and_bundle_both_recomputed_but_id_stale_raises(self):
        b = copy.deepcopy(BUNDLE)
        ex = b["evidence"][0]
        ex["lines"][1]["text"] = "Loaded 99 entries"
        ex["text_sha256"] = sha("\n".join(l["text"] for l in ex["lines"]))     # id still carries the old sha8
        finalize(b)
        with self.assertRaises(eqc.BundleTampered):
            eqc.check(b, self.good_output())

    def test_missing_hashes_raise(self):
        b = copy.deepcopy(BUNDLE)
        del b["bundle_sha256"]
        with self.assertRaises(eqc.BundleTampered):
            eqc.check(b, self.good_output())
        b = copy.deepcopy(BUNDLE)
        del b["evidence"][0]["text_sha256"]
        finalize(b)
        with self.assertRaises(eqc.BundleTampered):
            eqc.check(b, self.good_output())

    def test_malformed_bundle_raises(self):
        for mutate in (lambda b: b.pop("evidence"),
                       lambda b: b.__setitem__("evidence", "x"),
                       lambda b: b.__setitem__("evidence", ["x"]),
                       lambda b: b["evidence"][0].__setitem__("lines", "x"),
                       lambda b: b["evidence"][0].__setitem__("lines", ["x"]),
                       lambda b: b["evidence"][0]["lines"][0].__setitem__("text", 5),
                       lambda b: b["evidence"][0].__setitem__("id", 5)):
            b = copy.deepcopy(BUNDLE)
            mutate(b)
            finalize(b)
            with self.assertRaises(eqc.BundleTampered):
                eqc.check(b, self.good_output())
        for bad in (None, [], "x", 3):
            with self.assertRaises(eqc.BundleTampered):
                eqc.check(bad, self.good_output())

    def test_duplicate_id_with_different_text_raises(self):
        b = copy.deepcopy(BUNDLE)
        dup = copy.deepcopy(b["evidence"][0])
        dup["lines"][0]["text"] = "Mod init FAILED"
        dup["text_sha256"] = sha("\n".join(l["text"] for l in dup["lines"]))
        dup["id"] = b["evidence"][0]["id"]
        b["evidence"].append(dup)
        finalize(b)
        with self.assertRaises(eqc.BundleTampered):        # also fails the id/sha8 consistency check
            eqc.check(b, self.good_output())

    def test_exact_duplicate_excerpt_is_fine(self):
        b = copy.deepcopy(BUNDLE)
        b["evidence"].append(copy.deepcopy(b["evidence"][0]))
        finalize(b)
        self.assertTrue(eqc.check(b, self.good_output())[0]["valid"])

    def test_bundle_with_no_evidence_rejects_every_citation(self):
        b = copy.deepcopy(BUNDLE)
        b["evidence"] = []
        finalize(b)
        self.assertFalse(eqc.check(b, self.good_output())[0]["valid"])

    def test_non_ascii_text_hashes_consistently(self):
        ex = make_excerpt("playlog", 1, "alert", 1, [(1, "café — résumé failed")])
        b = finalize({"format": "f", "evidence": [ex]})
        out = blind(hyp("h1", [cit(ex["id"], "L1| café — résumé")]))
        self.assertTrue(eqc.check(b, out)[0]["valid"])


class Cli(unittest.TestCase):
    def test_all_pass_exit_0(self):
        f = Files(self, BUNDLE, blind(hyp("h1", [cit(ID_A, "Loaded 12 entries")])))
        code, out, err = run_cli(f.args())
        self.assertEqual(code, 0, err)
        self.assertIn("h1", out)

    def test_json_output(self):
        f = Files(self, BUNDLE, blind(hyp("h1", [cit(ID_A, "Loaded 12 entries")]), hyp("h2", [])))
        code, out, _ = run_cli(f.args("--json"))
        self.assertEqual(code, 1)
        data = json.loads(out)
        self.assertEqual([d["id"] for d in data], ["h1", "h2"])
        self.assertEqual([d["valid"] for d in data], [True, False])

    def test_any_rejected_exit_1(self):
        f = Files(self, BUNDLE, blind(hyp("h1", [cit(ID_A, "Loaded 12 entries")]),
                                      hyp("h2", [cit(ID_A, "invented text here")])))
        code, out, _ = run_cli(f.args())
        self.assertEqual(code, 1)
        self.assertIn("h2", out)

    def test_null_output_exit_codes(self):
        f = Files(self, BUNDLE, null("not_a_bug", reason(ID_B, "Queue length 0")))
        self.assertEqual(run_cli(f.args())[0], 0)
        f = Files(self, BUNDLE, null("not_a_bug", reason(ID_B, "never logged anywhere")))
        self.assertEqual(run_cli(f.args())[0], 1)

    def test_empty_verdict_list_exit_0(self):
        f = Files(self, BUNDLE, null("insufficient"))
        code, out, _ = run_cli(f.args("--json"))
        self.assertEqual(code, 0)
        self.assertEqual(json.loads(out), [])

    def test_tampered_excerpt_exit_2(self):
        b = copy.deepcopy(BUNDLE)
        b["evidence"][0]["lines"][1]["text"] = "Segmentation fault in the renderer"
        finalize(b)
        f = Files(self, b, blind(hyp("h1", [cit(ID_A, "Segmentation fault in the renderer")])))
        code, out, err = run_cli(f.args("--json"))
        self.assertEqual(code, 2)
        self.assertEqual(out.strip(), "")
        self.assertTrue(err.strip())

    def test_tampered_bundle_sha_exit_2(self):
        b = copy.deepcopy(BUNDLE)
        b["bundle_sha256"] = "f" * 64
        f = Files(self, b, blind(hyp("h1", [cit(ID_A, "Loaded 12 entries")])))
        self.assertEqual(run_cli(f.args())[0], 2)

    def test_unreadable_input_exit_2(self):
        f = Files(self, BUNDLE, blind())
        f.bundle.write_text("{not json", encoding="utf-8")
        self.assertEqual(run_cli(f.args())[0], 2)
        f = Files(self, BUNDLE, blind())
        f.output.write_text("[1, 2", encoding="utf-8")
        self.assertEqual(run_cli(f.args())[0], 2)
        f = Files(self, BUNDLE, blind())
        f.output.unlink()
        self.assertEqual(run_cli(f.args())[0], 2)
        f = Files(self, BUNDLE, blind())
        f.bundle.write_bytes(b"\xff\xfe\x00bad")
        self.assertEqual(run_cli(f.args())[0], 2)

    def test_non_object_json_exit_2_or_1_never_crash(self):
        f = Files(self, BUNDLE, ["not", "an", "object"])
        self.assertEqual(run_cli(f.args())[0], 1)             # output is parseable but not a valid output => rejected
        f = Files(self, [1, 2], blind())
        self.assertEqual(run_cli(f.args())[0], 2)

    def test_missing_args_exit_2(self):
        self.assertEqual(run_cli([])[0], 2)
        self.assertEqual(run_cli(["--bundle", "x"])[0], 2)

    def run_script(self, *args, isolated=True):
        cmd = [sys.executable] + (["-I"] if isolated else []) + ["-B", str(SCRIPT), *args]
        return subprocess.run(cmd, capture_output=True, text=True, timeout=30, cwd=tempfile.gettempdir())

    def test_help_works_without_isolated_flag(self):
        p = self.run_script("--help", isolated=False)
        self.assertEqual(p.returncode, 0, p.stderr)
        self.assertIn("--bundle", p.stdout)
        self.assertIn("--output", p.stdout)
        self.assertIn("--json", p.stdout)

    def test_script_runs_isolated_and_exit_codes(self):
        f = Files(self, BUNDLE, blind(hyp("h1", [cit(ID_A, "Loaded 12 entries")])))
        self.assertEqual(self.run_script(*f.args("--json")).returncode, 0)
        f = Files(self, BUNDLE, blind(hyp("h1", [cit(ID_A, "invented text here")])))
        self.assertEqual(self.run_script(*f.args()).returncode, 1)
        b = copy.deepcopy(BUNDLE)
        b["evidence"][1]["lines"][0]["text"] = "edited"
        finalize(b)
        f = Files(self, b, blind())
        p = self.run_script(*f.args())
        self.assertEqual(p.returncode, 2)
        self.assertIn("tamper", p.stderr.lower())


def main():
    suite = unittest.defaultTestLoader.loadTestsFromModule(sys.modules[__name__])
    res = unittest.TextTestRunner(verbosity=1).run(suite)
    bad = len(res.failures) + len(res.errors)
    print(f"evidence_quote_check tests: {res.testsRun - bad}/{res.testsRun} {'PASS' if not bad else 'FAIL'}")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
