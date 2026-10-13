#!/usr/bin/env python3
"""Deterministic quote checker for playtest evidence bundles (no LLM, stdlib only).

Usage:
  evidence_quote_check.py --bundle FILE --output FILE [--json]

  --bundle   playtest bundle JSON (scripts/playtest_bundle.py) with its evidence excerpts.
  --output   a blind model output ({unit_id, hypotheses[{id, evidence[{evidence_id, quote}], ...}]}),
             a null output ({unit_id, verdict, reasons[{text, evidence_id, quote}]}) or a verify
             output (same reasons shape). One verdict is printed per hypothesis / per reason.
  --json     print the verdict list as JSON instead of text lines.

Rule: a citation is valid iff its evidence_id names an excerpt in the bundle (the full id, or a short
`ev:<sha8>` that matches exactly one excerpt id; an ambiguous short id is rejected) AND its quote, after removing
`L<n>| ` line prefixes (also in multi-line quotes) and normalising whitespace, is a substring of the
whitespace-normalised text of that ONE excerpt AND the quote is at least 6 characters (MIN_CHARS; the
same rule as the deep-haiku-review bundle's quote_check.py) AND keeps at least 4 alphanumeric characters
once bracket tags, timestamps and punctuation are removed (MIN_ALNUM) AND every `L<n>|` marker names a
line of that excerpt. A quote spanning two excerpts is fabricated.
ANY invalid citation rejects the whole hypothesis; a hypothesis with no citations (missing or empty or
non-list `evidence`) is rejected. A null/verify output with a decisive verdict and no reasons is rejected
(verdict `insufficient` / `inconclusive` may have none). A rejected verdict lists no valid_evidence_ids.

Integrity: every excerpt's text_sha256 and the bundle_sha256 are recomputed first. A mismatch, a missing
hash or a malformed bundle raises BundleTampered (CLI exit 2).

Exit: 0 all verdicts valid (also when there are none), 1 any rejected, 2 tampered/unreadable input.
Library: check(bundle, output) -> list of {id, valid, reason, valid_evidence_ids, distinct_texts, ...};
distinct_texts counts the distinct excerpt texts (sha8) behind the valid ids, 0 when rejected.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys

MIN_CHARS = 6
MIN_ALNUM = 4
# `L<n>| ` as rendered by the prompt builder; only recognised at a token start so text such as
# "tailL12| x" or "L12 | x" is left alone.
_PREFIX = re.compile(r"(?<!\S)L\d+\|[ \t]?")
# the same token-start rule, capturing the line number of each marker in the raw quote
_MARKER = re.compile(r"(?<!\S)L(\d+)\|")
# content that carries no evidence on its own: a bracket tag such as [KP-hotbar] or a log timestamp
_TAG = re.compile(r"\[[^\]]*\]")
_TIMESTAMP = re.compile(r"\d{1,2}:\d{2}:\d{2}(?:\.\d+)?")
_UNKNOWN_HINT = " (cite the full id from the bracket header, or ev:<8 hex>)"
# short evidence id `ev:<sha8>` (first 8 lowercase hex chars of an excerpt's text_sha256); see _resolve
_SHORT_ID = re.compile(r"ev:[0-9a-f]{8}")
# verdicts of a null/verify output that may legitimately come with no reasons
_NO_REASON_OK = frozenset({"insufficient", "inconclusive"})
_MISSING = object()


class BundleTampered(ValueError):
    """The bundle is malformed or does not match its recorded hashes."""


def norm_ws(s) -> str:
    return " ".join(str(s or "").split())


def norm_quote(q: str) -> str:
    """Strip `L<n>| ` prefixes anywhere in the quote, then normalise whitespace."""
    return norm_ws(_PREFIX.sub("", q))


def _sha(s: str) -> str:
    try:
        return hashlib.sha256(s.encode("utf-8")).hexdigest()
    except UnicodeEncodeError as e:
        raise BundleTampered(f"text is not encodable as utf-8: {e}") from None


def index_bundle(bundle) -> dict:
    """Verify the bundle's hashes -> {evidence id: whitespace-normalised excerpt text}."""
    if not isinstance(bundle, dict):
        raise BundleTampered("bundle is not a JSON object")
    evidence = bundle.get("evidence")
    if not isinstance(evidence, list):
        raise BundleTampered("bundle has no evidence list")
    texts: dict = {}
    seen_sha: dict = {}
    for i, ex in enumerate(evidence):
        where = f"evidence[{i}]"
        if not isinstance(ex, dict):
            raise BundleTampered(f"{where} is not an object")
        eid, lines, recorded = ex.get("id"), ex.get("lines"), ex.get("text_sha256")
        if not isinstance(eid, str) or not eid:
            raise BundleTampered(f"{where} has no id")
        where = f"excerpt {eid}"
        if not isinstance(lines, list) or not all(isinstance(l, dict) and isinstance(l.get("text"), str)
                                                  for l in lines):
            raise BundleTampered(f"{where} has malformed lines")
        if not isinstance(recorded, str) or not recorded:
            raise BundleTampered(f"{where} has no text_sha256")
        raw = "\n".join(l["text"] for l in lines)
        actual = _sha(raw)
        if actual != recorded:
            raise BundleTampered(f"{where}: text_sha256 mismatch (excerpt text was edited)")
        if eid.rsplit(":", 1)[-1] != actual[:8]:
            raise BundleTampered(f"{where}: id does not end with the first 8 hex chars of text_sha256")
        if eid in seen_sha and seen_sha[eid] != actual:
            raise BundleTampered(f"{where}: duplicate id with different text")
        seen_sha[eid] = actual
        texts[eid] = norm_ws(raw)
    recorded_bundle = bundle.get("bundle_sha256")
    if not isinstance(recorded_bundle, str) or not recorded_bundle:
        raise BundleTampered("bundle has no bundle_sha256")
    body = {k: v for k, v in bundle.items() if k != "bundle_sha256"}
    try:
        canon = json.dumps(body, sort_keys=True, separators=(",", ":"), ensure_ascii=False)
    except (TypeError, ValueError) as e:
        raise BundleTampered(f"bundle is not canonicalisable: {e}") from None
    if _sha(canon) != recorded_bundle:
        raise BundleTampered("bundle_sha256 mismatch (bundle was edited)")
    return texts


def index_lines(bundle) -> dict:
    """{evidence id: set of line numbers n of that excerpt}. Call after index_bundle (structure is checked)."""
    out: dict = {}
    for ex in bundle["evidence"]:
        nums = {l["n"] for l in ex["lines"] if isinstance(l.get("n"), int) and not isinstance(l.get("n"), bool)}
        out.setdefault(ex["id"], set()).update(nums)
    return out


def _alnum_count(quote_norm: str) -> int:
    """Alphanumeric characters left once bracket tags, timestamps and punctuation are removed."""
    s = _TIMESTAMP.sub(" ", _TAG.sub(" ", quote_norm))
    return sum(ch.isalnum() for ch in s)


def _unknown_markers(quote: str, known: set) -> list:
    """Digits of each line marker in the RAW quote that names no line of the excerpt (leading zeros ignored)."""
    have = {str(n) for n in known}
    return [d for d in _MARKER.findall(quote) if (d.lstrip("0") or "0") not in have]


def _resolve(eid: str, texts: dict):
    """-> (full evidence id or None, reason). An exact key of texts wins. Otherwise a short `ev:<sha8>`
    (lowercase hex, whole string) resolves iff exactly one excerpt id ends in `:<sha8>`."""
    if eid in texts:
        return eid, ""
    if _SHORT_ID.fullmatch(eid):
        hits = [k for k in texts if k.endswith(":" + eid[3:])]
        if len(hits) == 1:
            return hits[0], ""
        if hits:
            return None, f"ambiguous evidence id {eid} ({len(hits)} excerpts)"
    return None, f"unknown evidence id {eid}"


def _citation(cit, texts: dict, lines: dict):
    """-> (ok, evidence_id or None, reason). The returned id is the resolved full id when ok."""
    if not isinstance(cit, dict):
        return False, None, "citation is not an object"
    eid, quote = cit.get("evidence_id", _MISSING), cit.get("quote", _MISSING)
    if not isinstance(eid, str):
        return False, None, "citation has no evidence_id"
    if not isinstance(quote, str):
        return False, eid, f"{eid}: citation has no quote"
    full, why = _resolve(eid, texts)
    if full is None:
        if why.startswith("unknown evidence id "):
            why += _UNKNOWN_HINT
        return False, eid, why
    eid = full
    want = norm_quote(quote)
    if len(want) < MIN_CHARS:
        return False, eid, f"{eid}: quote shorter than {MIN_CHARS} characters"
    if _alnum_count(want) < MIN_ALNUM:
        return False, eid, f"{eid}: quote has no substance (tag or timestamp only)"
    unknown = _unknown_markers(quote, lines.get(eid, set()))
    if unknown:
        return False, eid, f"{eid}: line marker L{unknown[0]} is not in that excerpt"
    if want not in texts[eid]:
        return False, eid, f"{eid}: quote not found verbatim in that excerpt"
    return True, eid, ""


def _verdict(index: int, vid: str, results: list, empty_reason: str) -> dict:
    """results = [(ok, evidence_id, reason)] for the item's citations."""
    if not results:
        return _rejected(vid, empty_reason, index)
    bad = [r for ok, _, r in results if not ok]
    if bad:
        return _rejected(vid, "; ".join(bad), index)
    ids: list = []
    for _, eid, _ in results:
        if eid not in ids:
            ids.append(eid)
    # the sha8 suffix identifies the excerpt text; two ids with the same suffix hold the same text
    return {"id": vid, "index": index, "valid": True, "reason": "", "valid_evidence_ids": ids,
            "distinct_texts": len({eid.rsplit(":", 1)[-1] for eid in ids})}


def _rejected(vid: str, why: str, index: int = 0) -> dict:
    return {"id": vid, "index": index, "valid": False, "reason": why, "valid_evidence_ids": [],
            "distinct_texts": 0}


def _blind(hypotheses, texts: dict, lines: dict) -> list:
    if not isinstance(hypotheses, list):
        return [_rejected("hypotheses", "hypotheses is not a list")]
    out = []
    for i, h in enumerate(hypotheses):
        if not isinstance(h, dict):
            out.append(_rejected(f"#{i + 1}", "hypothesis is not an object", i))
            continue
        hid = h.get("id")
        hid = hid if isinstance(hid, str) and hid else f"#{i + 1}"
        ev = h.get("evidence", _MISSING)
        if ev is _MISSING:
            out.append(_rejected(hid, "missing evidence key (uncited)", i))
        elif not isinstance(ev, list):
            out.append(_rejected(hid, "evidence is not a list", i))
        else:
            out.append(_verdict(i, hid, [_citation(c, texts, lines) for c in ev], "no citations (uncited)"))
    return out


def _reasons(output: dict, texts: dict, lines: dict) -> list:
    reasons = output.get("reasons")
    if not isinstance(reasons, list):
        return [_rejected("reasons", "reasons is not a list")]
    if not reasons:
        if output.get("verdict") in _NO_REASON_OK:
            return []
        return [_rejected("verdict", f"verdict {output.get('verdict')!r} has no cited reasons")]
    return [_verdict(i, f"r{i + 1}", [_citation(r, texts, lines)], "no citation") for i, r in enumerate(reasons)]


def check(bundle, output) -> list:
    """Quote-check a blind (hypotheses) or null/verify (reasons) model output against the bundle.

    Returns one verdict dict per hypothesis / reason: {id, index, valid, reason, valid_evidence_ids}.
    Raises BundleTampered when the bundle fails its integrity check. Pure: inputs are not modified.
    """
    texts = index_bundle(bundle)
    lines = index_lines(bundle)
    if not isinstance(output, dict):
        return [_rejected("output", "output is not a JSON object")]
    if "hypotheses" not in output and "reasons" not in output:
        return [_rejected("output", "output has neither hypotheses nor reasons")]
    verdicts: list = []
    if "hypotheses" in output:
        verdicts += _blind(output["hypotheses"], texts, lines)
    if "reasons" in output:
        verdicts += _reasons(output, texts, lines)
    return verdicts


def _load(path: str, what: str):
    try:
        with open(path, encoding="utf-8") as f:
            return json.load(f)
    except (OSError, ValueError) as e:                # ValueError covers JSONDecodeError and UnicodeDecodeError
        raise ValueError(f"cannot read {what} {path}: {e}") from None


def main(argv=None) -> int:
    # an id from a model output may hold a lone surrogate; print it escaped instead of crashing
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(errors="backslashreplace")
        except (AttributeError, ValueError):      # not a TextIOWrapper (e.g. redirected to StringIO) or closed
            pass
    ap = argparse.ArgumentParser(prog="evidence_quote_check.py", description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--bundle", required=True, help="playtest bundle JSON")
    ap.add_argument("--output", required=True, help="blind/null/verify model output JSON")
    ap.add_argument("--json", action="store_true", help="print verdicts as JSON")
    a = ap.parse_args(argv)
    try:
        bundle = _load(a.bundle, "bundle")
        output = _load(a.output, "output")
    except ValueError as e:
        print(f"error: {e}", file=sys.stderr)
        return 2
    try:
        verdicts = check(bundle, output)
    except BundleTampered as e:
        print(f"error: bundle tampered or malformed: {e}", file=sys.stderr)
        return 2
    if a.json:
        print(json.dumps(verdicts, indent=2, ensure_ascii=False))
    else:
        for v in verdicts:
            if v["valid"]:
                print(f"ok     {v['id']}: {', '.join(v['valid_evidence_ids'])}")
            else:
                print(f"REJECT {v['id']}: {v['reason']}")
        print(f"{sum(1 for v in verdicts if v['valid'])}/{len(verdicts)} valid")
    return 0 if all(v["valid"] for v in verdicts) else 1


if __name__ == "__main__":
    sys.exit(main())
