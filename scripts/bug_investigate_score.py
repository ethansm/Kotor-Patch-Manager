#!/usr/bin/env python3
"""Deterministic scorer and aggregator for bug-investigate runs (stdlib only; no model call, no network).

  python3 scripts/bug_investigate_score.py score --run RUN --case CASE [--json]
  python3 scripts/bug_investigate_score.py aggregate --run RUN1 --run RUN2 ... [--json]

score reads RUN/report.json, CASE["score_rubric"] (the only code that reads it), and the unit cost files
RUN/{blind,null,verify}/out/runs/*.json; it writes RUN/score.json (sorted keys, indent 2, trailing newline).
aggregate reads each RUN/score.json, pools the hypotheses, and writes nothing.
Exit codes: 0 ok; 1 score.json could not be written; 2 usage or input error (nothing written).

Labels: HIT = every fact named in score_rubric.hit matched; PARTIAL = at least one hit fact matched; MISS = none.
Informational facts (not in hit) are recorded in `facts` but never change the label.

Matchers (score --matcher v2|v1, default v2; score.json records which one ran):
  v1: the original matcher. Fact regexes are case-insensitive and run on claim + "\\n" + mechanism only.
  v2: the same regexes, run after (a) an echo mask: every run of ECHO_RUN or more words that also occurs as a
      word window of the case's symptom_text is blanked to spaces; (b) a negation check per regex match: the
      WINDOW_WORDS words before it, back to the sentence start, containing not/no/never/without/cannot/nor/
      neither or an n't word discards that match; a fact matches when at least one match survives. (c) A surviving
      match with might/may/could/possibly/perhaps/maybe/seems/appears in that same window lists its fact in
      `hedged_facts` (informational; the label is unchanged).
"""
import argparse
import hashlib
import json
import math
import os
import re
import sys

REPORT_FORMAT = "bug-investigate-report/1"
SCORE_FORMAT = "bug-investigate-score/1"
AGG_FORMAT = "bug-investigate-score-aggregate/1"
CAVEAT = "hypotheses from the same run share evidence: intervals are optimistic"
LABELS = ("HIT", "PARTIAL", "MISS")
STAGES = ("blind", "null", "verify")
STATUSES = ("supported", "rejected")
# Same ordering as bug_investigate.py's rank key (copied, not imported, so this scorer stays independent).
CONFIDENCE = {"high": 3, "medium": 2, "low": 1}
Z = 1.96
MATCHERS = ("v2", "v1")
DEFAULT_MATCHER = "v2"
ECHO_RUN = 4        # words: a run this long that also occurs in the symptom is masked
WINDOW_WORDS = 6    # words before a match, back to the sentence start, checked for negators and hedges
SENTENCE_END = ".;!?\n"
NEGATORS = frozenset(("not", "no", "never", "without", "cannot", "nor", "neither"))
HEDGES = frozenset(("might", "may", "could", "possibly", "perhaps", "maybe", "seems", "appears"))
WORD_RE = re.compile(r"\w+")
WINDOW_WORD_RE = re.compile(r"[\w']+")   # keeps apostrophes, so "doesn't" stays one word


class InputError(Exception):
    """A usage or input problem: the message goes to stderr and the exit code is 2."""


class OutputError(Exception):
    """score.json could not be written: exit 1."""


# ---------------------------------------------------------------- small helpers
def read_json_file(path, what):
    try:
        with open(path, "rb") as f:
            raw = f.read()
    except OSError as e:
        raise InputError("cannot read %s %s: %s" % (what, path, e.strerror))
    try:
        return json.loads(raw.decode("utf-8"))
    except (UnicodeDecodeError, ValueError):
        raise InputError("%s is not valid JSON: %s" % (what, path))


def _text(v):
    return v if isinstance(v, str) else ""


def _round(x):
    return None if x is None else round(x, 6)


def _cost(v):
    """A cost or duration as a finite float, or None. Booleans and missing values are unknown, never 0."""
    if isinstance(v, bool) or not isinstance(v, (int, float)) or not math.isfinite(v):
        return None
    return round(float(v), 6)


def wilson(k, n, z=Z):
    """Wilson score interval (lo, hi) for k successes out of n, or None when n is 0."""
    if n <= 0:
        return None
    p = k / n
    z2 = z * z
    center = (p + z2 / (2 * n)) / (1 + z2 / n)
    half = z * math.sqrt(p * (1 - p) / n + z2 / (4 * n * n)) / (1 + z2 / n)
    return center - half, center + half


def proportion(k, n):
    if n == 0:
        return {"k": k, "n": 0, "p": None, "wilson95": None}
    lo, hi = wilson(k, n)
    return {"k": k, "n": n, "p": _round(k / n), "wilson95": [_round(lo), _round(hi)]}


# ---------------------------------------------------------------- rubric and labels
def load_rubric(case_path):
    case = read_json_file(case_path, "case")
    if not isinstance(case, dict) or "score_rubric" not in case:
        raise InputError("case has no score_rubric: %s" % case_path)
    rub = case["score_rubric"]
    if not isinstance(rub, dict):
        raise InputError("score_rubric must be an object: %s" % case_path)
    facts = rub.get("facts")
    if not isinstance(facts, dict) or not facts:
        raise InputError("score_rubric.facts must be a non-empty object")
    compiled = {}
    for name, pats in facts.items():
        if not isinstance(pats, list) or not all(isinstance(p, str) for p in pats):
            raise InputError("score_rubric.facts[%r] must be a list of regex strings" % name)
        if any(not p.strip() for p in pats):
            raise InputError("score_rubric.facts[%r] has an empty or whitespace-only pattern" % name)
        try:
            compiled[name] = [re.compile(p, re.IGNORECASE) for p in pats]
        except re.error as e:
            raise InputError("score_rubric.facts[%r] regex does not compile: %s" % (name, e))
    hit = rub.get("hit")
    if not isinstance(hit, list) or not hit or not all(isinstance(x, str) for x in hit):
        raise InputError("score_rubric.hit must be a non-empty list of fact names")
    unknown = sorted(set(hit) - set(compiled))
    if unknown:
        raise InputError("score_rubric.hit names facts that are not in facts: %s" % ", ".join(unknown))
    return compiled, frozenset(hit), case


def match_facts_v1(compiled, claim, mechanism):
    text = claim + "\n" + mechanism
    return sorted(name for name, pats in compiled.items() if any(p.search(text) for p in pats))


def _word_spans(text):
    """(lower-cased word, start, end) for every \\w+ run, with offsets into the original text."""
    return [(m.group(0).lower(), m.start(), m.end()) for m in WORD_RE.finditer(text)]


def echo_masked(text, symptom):
    """Blank to spaces (same length) every word run of ECHO_RUN+ words that also occurs in the symptom."""
    sym = [w for w, _, _ in _word_spans(symptom)]
    windows = {tuple(sym[i:i + ECHO_RUN]) for i in range(len(sym) - ECHO_RUN + 1)}
    if not windows:
        return text
    toks = _word_spans(text)
    covered = [False] * len(toks)
    for i in range(len(toks) - ECHO_RUN + 1):
        if tuple(w for w, _, _ in toks[i:i + ECHO_RUN]) in windows:
            covered[i:i + ECHO_RUN] = [True] * ECHO_RUN
    chars = list(text)
    for (_, start, end), hit in zip(toks, covered):
        if hit:
            chars[start:end] = " " * (end - start)
    return "".join(chars)


def _window_words(text, pos):
    """The last WINDOW_WORDS words between the sentence start and pos, lower-cased."""
    start = max(text.rfind(c, 0, pos) for c in SENTENCE_END) + 1
    words = WINDOW_WORD_RE.findall(text[start:pos].lower().replace(chr(0x2019), "'"))
    return words[-WINDOW_WORDS:]


def _negated(words):
    return any(w in NEGATORS or w.endswith("n't") for w in words)


def _hedged(words):
    return any(w in HEDGES for w in words)


def match_facts(compiled, claim, mechanism, symptom=""):
    """v2. Returns (matched, hedged): sorted fact names with at least one surviving match, and the subset whose
    surviving match has a hedge word in its window. Runs on the echo-masked claim + "\\n" + mechanism."""
    text = echo_masked(claim + "\n" + mechanism, symptom)
    matched, hedged = [], []
    for name, pats in compiled.items():
        survivors = []
        for p in pats:
            for m in p.finditer(text):
                words = _window_words(text, m.start())
                if not _negated(words):
                    survivors.append(_hedged(words))
        if survivors:
            matched.append(name)
            if any(survivors):
                hedged.append(name)
    return sorted(matched), sorted(hedged)


def label_for(matched, hit_names):
    n = len(hit_names & matched)
    if n == len(hit_names):
        return "HIT"
    if n > 0:
        return "PARTIAL"
    return "MISS"


# ---------------------------------------------------------------- report and ranking
def load_report(run):
    path = os.path.join(run, "report.json")
    rep = read_json_file(path, "report")
    if not isinstance(rep, dict) or rep.get("format") != REPORT_FORMAT:
        raise InputError("report format is not %s: %s" % (REPORT_FORMAT, path))
    hyps = rep.get("hypotheses")
    ranked = rep.get("ranked")
    if not isinstance(hyps, list) or not isinstance(ranked, list) or not all(isinstance(x, str) for x in ranked):
        raise InputError("report hypotheses/ranked are malformed: %s" % path)
    for h in hyps:
        if (not isinstance(h, dict) or not isinstance(h.get("id"), str) or h.get("status") not in STATUSES
                or not isinstance(h.get("quote_valid"), bool) or not isinstance(h.get("evidence_ids"), list)):
            raise InputError("report hypothesis is malformed: %s" % path)
        if h.get("verifier") is not None and not isinstance(h.get("verifier"), dict):
            raise InputError("report hypothesis verifier is malformed: %s" % path)
    return rep


def distinct_evidence(h):
    return len({str(x) for x in h["evidence_ids"]})


def rank_key(h):
    conf = h.get("confidence")
    return (-distinct_evidence(h), -(CONFIDENCE.get(conf, 0) if isinstance(conf, str) else 0),
            _text(h.get("unit_id")), _text(h.get("hypothesis_id")))


# ---------------------------------------------------------------- unit costs
def unit_costs(run):
    """Cost records for every RUN/<stage>/out/runs/*.json. An unreadable or cost-less file is a null cost."""
    units = []
    for stage in STAGES:
        d = os.path.join(run, stage, "out", "runs")
        if not os.path.isdir(d):
            continue
        try:
            names = sorted(os.listdir(d))
        except OSError as e:
            raise InputError("cannot list %s: %s" % (d, e.strerror))
        for fn in names:
            p = os.path.join(d, fn)
            if fn.startswith(".") or not fn.endswith(".json") or not os.path.isfile(p):
                continue
            try:
                body = read_json_file(p, "unit")
            except InputError:
                body = None
            if not isinstance(body, dict):
                body = {}
            units.append({"stage": stage, "file": fn, "cost_usd": _cost(body.get("cost_usd")),
                          "duration_s": _cost(body.get("duration_s"))})
    return units


def known_costs(units):
    return [c for c in (_cost(u.get("cost_usd")) for u in units) if c is not None]


def cost_summary(units):
    known = known_costs(units)
    return {"units": units, "total_cost_usd": _round(float(sum(known))),
            "cost_complete": len(known) == len(units)}


# ---------------------------------------------------------------- score one run
def score_run(run, case_path, matcher=DEFAULT_MATCHER):
    if matcher not in MATCHERS:
        raise InputError("unknown matcher %r (use one of %s)" % (matcher, ", ".join(MATCHERS)))
    compiled, hit_names, case = load_rubric(case_path)
    rubric = case["score_rubric"]
    symptom = _text(case.get("symptom_text"))
    cid = case.get("case_id")
    rep = load_report(run)
    rows = []
    for h in rep["hypotheses"]:
        claim, mech = _text(h.get("claim")), _text(h.get("mechanism"))
        if matcher == "v1":
            matched, hedged = match_facts_v1(compiled, claim, mech), None
        else:
            matched, hedged = match_facts(compiled, claim, mech, symptom)
        ver = h.get("verifier")
        verdict = _text(ver.get("verdict")) if isinstance(ver, dict) else ""
        row = {"id": h["id"], "label": label_for(set(matched), hit_names), "facts": matched,
               "quote_valid": h["quote_valid"],
               "status_A": "supported" if h["quote_valid"] else "rejected",
               "status_B": h["status"], "verifier_verdict": verdict or None,
               "evidence_ids": distinct_evidence(h)}
        if hedged is not None:
            row["hedged_facts"] = hedged
        rows.append(row)
    # Canonical order: the report's rank key over every hypothesis. A ranks quote-valid ones; B ranks
    # verifier-surviving ones, so a verifier-rejected top entry falls through to the next supported one.
    ordered = [row for _, row in sorted(zip(rep["hypotheses"], rows), key=lambda pair: rank_key(pair[0]))]
    top_a = next((r for r in ordered if r["status_A"] == "supported"), None)
    top_b = next((r for r in ordered if r["status_B"] == "supported"), None)

    def top(row):
        return None if row is None else {"id": row["id"], "label": row["label"]}

    def tally(sel):
        return {lab: sum(1 for r in sel if r["label"] == lab) for lab in LABELS}

    valid = sum(1 for r in rows if r["quote_valid"])
    rubric_sha = hashlib.sha256(json.dumps(rubric, sort_keys=True, separators=(",", ":")).encode("utf-8"))
    return {
        "format": SCORE_FORMAT, "case": os.path.basename(case_path),
        "case_id": cid if isinstance(cid, str) and cid else None,
        "matcher": matcher, "rubric_sha256": rubric_sha.hexdigest(),
        "run_id": _text(rep.get("run_id")) or os.path.basename(run),
        "decision": rep.get("decision"), "null_verdict": rep.get("null_verdict"),
        "null_valid": rep.get("null_valid"), "ranked": list(rep["ranked"]),
        "quote_valid": {"valid": valid, "total": len(rows), "rate": _round(valid / len(rows)) if rows else None},
        "counts": {"all": tally(rows), "quote_valid": tally([r for r in rows if r["quote_valid"]])},
        "top_ranked": {"A": top(top_a), "B": top(top_b)},
        "hypotheses": rows,
        "cost": cost_summary(unit_costs(run)),
    }


def dump_bytes(obj):
    return (json.dumps(obj, sort_keys=True, indent=2) + "\n").encode("utf-8")


def write_score(run, data):
    path = os.path.join(run, "score.json")
    tmp = os.path.join(run, ".score.json.tmp-%d" % os.getpid())
    try:
        with open(tmp, "wb") as f:
            f.write(data)
        os.replace(tmp, path)
    except OSError as e:
        try:
            os.unlink(tmp)
        except OSError:
            pass
        raise OutputError("cannot write %s: %s" % (path, e.strerror))


def score_summary(s, path):
    cost = s["cost"]
    unknown = sum(1 for u in cost["units"] if u["cost_usd"] is None)
    counts = s["counts"]["all"]
    qv = s["counts"]["quote_valid"]

    def t(key):
        top = s["top_ranked"][key]
        return "none" if top is None else "%s [%s]" % (top["id"], top["label"])

    lines = [
        "run %s: decision=%s null=%s" % (s["run_id"], s["decision"], s["null_verdict"] or "none"),
        "labels (all): HIT %d, PARTIAL %d, MISS %d; quote-valid %d/%d" % (
            counts["HIT"], counts["PARTIAL"], counts["MISS"], s["quote_valid"]["valid"], s["quote_valid"]["total"]),
        "labels (quote-valid): HIT %d, PARTIAL %d, MISS %d" % (qv["HIT"], qv["PARTIAL"], qv["MISS"]),
        "top-ranked A: %s | B: %s" % (t("A"), t("B")),
        "cost: $%s total%s" % (cost["total_cost_usd"],
                               "" if cost["cost_complete"] else " (INCOMPLETE: %d unit(s) unknown)" % unknown),
        "wrote %s" % path,
    ]
    return "\n".join(lines)


def cmd_score(args):
    run = args.run
    if not os.path.isdir(run):
        raise InputError("run dir not found: %s" % run)
    s = score_run(run, args.case, matcher=args.matcher)
    data = dump_bytes(s)
    write_score(run, data)
    if args.json:
        sys.stdout.write(data.decode("utf-8"))
    else:
        print(score_summary(s, os.path.join(run, "score.json")))
    return 0


# ---------------------------------------------------------------- aggregate
def load_score(run):
    path = os.path.join(run, "score.json")
    obj = read_json_file(path, "score")
    if not isinstance(obj, dict) or obj.get("format") != SCORE_FORMAT:
        raise InputError("not a %s file: %s" % (SCORE_FORMAT, path))
    hyps = obj.get("hypotheses")
    top = obj.get("top_ranked")
    cost = obj.get("cost")
    if (not isinstance(hyps, list) or not isinstance(top, dict)
            or not isinstance(cost, dict) or not isinstance(cost.get("units"), list)):
        raise InputError("score file is malformed: %s" % path)
    for h in hyps:
        if (not isinstance(h, dict) or h.get("label") not in LABELS or not isinstance(h.get("quote_valid"), bool)
                or h.get("status_A") not in STATUSES or h.get("status_B") not in STATUSES):
            raise InputError("score hypothesis is malformed: %s" % path)
    for u in cost["units"]:
        if not isinstance(u, dict):
            raise InputError("score cost unit is malformed: %s" % path)
    return obj


def label_counts(rows):
    return {lab: sum(1 for r in rows if r["label"] == lab) for lab in LABELS}


def config_summary(rows):
    c = label_counts(rows)
    n = len(rows)
    return {"n_supported": n, "HIT": c["HIT"], "PARTIAL": c["PARTIAL"], "MISS": c["MISS"],
            "precision_hit": proportion(c["HIT"], n),
            "hit_or_partial": proportion(c["HIT"] + c["PARTIAL"], n)}


def rank1_summary(labels):
    tops = [x for x in labels if x is not None]
    hit = sum(1 for x in tops if x == "HIT")
    return {"runs_with_top": len(tops), "HIT": hit, "p": _round(hit / len(tops)) if tops else None}


def aggregate(scored):
    """scored: list of (run_path, score dict), already in a fixed order."""
    rows = [h for _, s in scored for h in s["hypotheses"]]
    a_rows = [h for h in rows if h["status_A"] == "supported"]
    b_rows = [h for h in rows if h["status_B"] == "supported"]
    rejected = [h for h in a_rows if h["status_B"] == "rejected"]
    discordance = {}
    for lab in LABELS:
        sel = [h for h in rows if h["label"] == lab]
        discordance[lab] = {
            "A+B+": sum(1 for h in sel if h["status_A"] == "supported" and h["status_B"] == "supported"),
            "A+B-": sum(1 for h in sel if h["status_A"] == "supported" and h["status_B"] == "rejected"),
            "A-B+": sum(1 for h in sel if h["status_A"] == "rejected" and h["status_B"] == "supported"),
            "A-B-": sum(1 for h in sel if h["status_A"] == "rejected" and h["status_B"] == "rejected"),
        }
    null_verdicts = {}
    null_valid = {}
    for _, s in scored:
        key = s.get("null_verdict") or "none"
        null_verdicts[key] = null_verdicts.get(key, 0) + 1
        if isinstance(s.get("null_valid"), bool):
            flag = "true" if s["null_valid"] else "false"
            null_valid[flag] = null_valid.get(flag, 0) + 1
    units = [u for _, s in scored for u in s["cost"]["units"]]
    known = known_costs(units)
    return {
        "format": AGG_FORMAT, "caveat": CAVEAT,
        "runs": [{"run_id": s.get("run_id"), "case": s.get("case"), "decision": s.get("decision")}
                 for _, s in scored],
        "n_runs": len(scored), "n_hypotheses": len(rows),
        "config_A": config_summary(a_rows), "config_B": config_summary(b_rows),
        "verifier_effect": {
            "rejected_by_verifier": dict(label_counts(rejected), total=len(rejected)),
            "false_rejections": sum(1 for h in rejected if h["label"] == "HIT"),
            "discordance": discordance,
        },
        "rank1": {
            "A": rank1_summary([s["top_ranked"]["A"]["label"] if s["top_ranked"].get("A") else None
                                for _, s in scored]),
            "B": rank1_summary([s["top_ranked"]["B"]["label"] if s["top_ranked"].get("B") else None
                                for _, s in scored]),
            "per_run": [{"run_id": s.get("run_id"),
                         "A": (s["top_ranked"].get("A") or {}).get("label"),
                         "B": (s["top_ranked"].get("B") or {}).get("label")} for _, s in scored],
        },
        "null_verdicts": null_verdicts, "null_valid": null_valid,
        "cost": {"total_cost_usd": _round(float(sum(known))), "cost_complete": len(known) == len(units),
                 "units": len(units), "units_known": len(known),
                 "per_unit_mean_usd": _round(sum(known) / len(known)) if known else None},
    }


def _p(block):
    if block["p"] is None:
        return "n/a"
    return "%.3f [%.3f, %.3f]" % (block["p"], block["wilson95"][0], block["wilson95"][1])


def aggregate_summary(agg):
    a, b = agg["config_A"], agg["config_B"]
    lines = [
        "aggregate: %d run(s), %d hypothesis(es)" % (agg["n_runs"], agg["n_hypotheses"]),
        "A (quote-valid): n=%d HIT %d PARTIAL %d MISS %d precision_hit %s hit_or_partial %s" % (
            a["n_supported"], a["HIT"], a["PARTIAL"], a["MISS"], _p(a["precision_hit"]), _p(a["hit_or_partial"])),
        "B (verifier): n=%d HIT %d PARTIAL %d MISS %d precision_hit %s hit_or_partial %s" % (
            b["n_supported"], b["HIT"], b["PARTIAL"], b["MISS"], _p(b["precision_hit"]), _p(b["hit_or_partial"])),
        "verifier false rejections (HIT): %d" % agg["verifier_effect"]["false_rejections"],
        "cost: $%s total, per unit mean %s" % (agg["cost"]["total_cost_usd"],
                                             "n/a" if agg["cost"]["per_unit_mean_usd"] is None
                                             else "$%s" % agg["cost"]["per_unit_mean_usd"]),
        "caveat: " + CAVEAT,
    ]
    return "\n".join(lines)


def cmd_aggregate(args):
    runs = sorted({os.path.realpath(r) for r in args.runs})
    scored = [(r, load_score(r)) for r in runs]
    agg = aggregate(scored)
    data = dump_bytes(agg)
    if args.json:
        sys.stdout.write(data.decode("utf-8"))
    else:
        print(aggregate_summary(agg))
    return 0


# ---------------------------------------------------------------- CLI
def main(argv=None):
    parser = argparse.ArgumentParser(description="Score and aggregate bug-investigate runs (deterministic, no model).")
    sub = parser.add_subparsers(dest="cmd", required=True)
    ps = sub.add_parser("score", help="score one run and write RUN/score.json")
    ps.add_argument("--run", required=True, help="run dir holding report.json")
    ps.add_argument("--case", required=True, help="case JSON holding score_rubric")
    ps.add_argument("--matcher", choices=MATCHERS, default=DEFAULT_MATCHER,
                    help="fact matcher: v2 (echo mask, negation, hedge flag; default) or v1 (original)")
    ps.add_argument("--json", action="store_true", help="print the score JSON instead of a summary")
    pa = sub.add_parser("aggregate", help="pool several runs' score.json files (writes nothing)")
    pa.add_argument("--run", action="append", dest="runs", required=True, help="run dir (repeat per run)")
    pa.add_argument("--json", action="store_true", help="print the aggregate JSON instead of a summary")
    try:
        args = parser.parse_args(argv)
    except SystemExit as e:
        return e.code if isinstance(e.code, int) else 2
    try:
        return cmd_score(args) if args.cmd == "score" else cmd_aggregate(args)
    except InputError as e:
        print("error: %s" % e, file=sys.stderr)
        return 2
    except OutputError as e:
        print("error: %s" % e, file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
