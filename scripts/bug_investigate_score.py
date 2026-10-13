#!/usr/bin/env python3
"""Deterministic scorer and aggregator for bug-investigate runs (stdlib only; no model call, no network).

  python3 scripts/bug_investigate_score.py score --run RUN --case CASE [--json]
  python3 scripts/bug_investigate_score.py aggregate --run RUN1 --run RUN2 ... [--allow-mixed] [--json]
  python3 scripts/bug_investigate_score.py sample --run RUN [--run RUN ...] --n N --seed S [--exclude FILE.json]
  python3 scripts/bug_investigate_score.py agree --labels FILE.json --run RUN [--run RUN ...] [--case CASE] [--json]

score reads RUN/report.json, CASE["score_rubric"] (the only code that reads it), and the unit cost files
RUN/{blind,null,verify}/out/runs/*.json; it writes RUN/score.json (sorted keys, indent 2, trailing newline).
aggregate reads each RUN/score.json, pools the hypotheses, and writes nothing. It refuses score files that
disagree on case_id, rubric_sha256 or matcher unless --allow-mixed (a score file without the key counts as one
unnamed value, "<missing>").
sample prints one JSON list of N [run basename, hypothesis id] pairs: a seeded draw (random.Random(S).sample) from the
quote-valid hypotheses of the runs' report.json files, after the pool is sorted by (run basename, id) and the pairs in
--exclude (a JSON list of [run basename, id]) are removed. N above the pool size takes the whole pool (noted on stderr).
agree compares human labels with the labels already in each RUN/score.json (nothing is rescored). Labels come from
--labels: {"<run basename>/<id>": "HIT|PARTIAL|MISS", ...} or [[run basename, id, label], ...]. It prints the raw
agreement, Cohen's kappa (1.0 when every pair agrees, n/a when undefined) and a 3x3 table (rows = given label,
columns = scorer label). With --case, every score.json must carry that case's score_rubric hash.
Both sample and agree write nothing.
Exit codes: 0 ok; 1 score.json could not be written; 2 usage or input error, or refused mixed inputs (nothing written).

Rank-1 and ties: "top" means config B (verifier-surviving), the same hypothesis report.json names in ranked[0].
top_matches_report is false when the scorer's B top differs from ranked[0]. rank1_alt_label is the B top's label
when ties on distinct evidence and confidence are broken by unit_id descending instead of ascending; ties_decide_runs
counts runs where that label differs from the normal one.
Intervals: the pooled Wilson interval sits beside a cluster bootstrap over runs (B=2000, seed 0, percentile 2.5/97.5).
cost_complete needs at least 3 blind and 1 null cost record in the run, and every record's cost known.

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
import random
import re
import sys
from fractions import Fraction

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
BOOT_B = 2000                 # cluster bootstrap resamples
BOOT_SEED = 0
MIN_BLIND_COST_RECORDS = 3    # cost_complete needs at least this many blind cost records in a run
MIN_NULL_COST_RECORDS = 1
STYLE_RE = re.compile(r"^blind-([^:]+):")   # hypothesis id "blind-<style>:hN"; style is advocate/evidence/skeptic
PROVENANCE_KEYS = ("case_id", "rubric_sha256", "matcher")
MISSING_VALUE = "<missing>"   # a score.json without the key: one unnamed value
AGREE_FORMAT = "bug-investigate-agree/1"


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


def pooled_ratio(pairs):
    """pairs: (k, n) per run. Pooled k / n over the runs, or None when no n is positive."""
    n = sum(n for _, n in pairs)
    return sum(k for k, _ in pairs) / n if n else None


def cluster_bootstrap_ci(per_run_counts, stat=pooled_ratio, B=BOOT_B, seed=BOOT_SEED):
    """Percentile bootstrap with the run as the cluster: resample the runs with replacement B times, apply stat to
    each resample, and take the 2.5th and 97.5th percentiles. None when there are no runs or no defined resample."""
    runs = list(per_run_counts)
    if not runs:
        return None
    rng = random.Random(seed)
    vals = []
    for _ in range(B):
        v = stat([runs[rng.randrange(len(runs))] for _ in runs])
        if v is not None:
            vals.append(v)
    if not vals:
        return None
    vals.sort()
    lo = vals[int(0.025 * len(vals))]
    hi = vals[min(len(vals) - 1, int(0.975 * len(vals)))]
    return {"lo": _round(lo), "hi": _round(hi), "B": B, "seed": seed, "n_runs": len(runs), "n_defined": len(vals)}


# ---------------------------------------------------------------- rubric and labels
def rubric_hash(rubric):
    """sha256 of the rubric's canonical JSON (sorted keys, no spaces): the rubric_sha256 a score.json records."""
    return hashlib.sha256(json.dumps(rubric, sort_keys=True, separators=(",", ":")).encode("utf-8")).hexdigest()


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
    """Distinct excerpt texts behind the hypothesis when the report carries distinct_texts, else distinct ids."""
    dt = h.get("distinct_texts")
    if isinstance(dt, int) and not isinstance(dt, bool) and dt >= 0:
        return dt
    return len({str(x) for x in h["evidence_ids"]})


def rank_key(h):
    conf = h.get("confidence")
    return (-distinct_evidence(h), -(CONFIDENCE.get(conf, 0) if isinstance(conf, str) else 0),
            _text(h.get("unit_id")), _text(h.get("hypothesis_id")))


class _Reversed:
    """Sort-key wrapper that orders a string descending. Used only by the alternative tie-break."""
    __slots__ = ("s",)

    def __init__(self, s):
        self.s = s

    def __eq__(self, other):
        return self.s == other.s

    def __lt__(self, other):
        return self.s > other.s


def alt_rank_key(h):
    """rank_key with unit_id descending instead of ascending (hypothesis_id stays ascending)."""
    conf = h.get("confidence")
    return (-distinct_evidence(h), -(CONFIDENCE.get(conf, 0) if isinstance(conf, str) else 0),
            _Reversed(_text(h.get("unit_id"))), _text(h.get("hypothesis_id")))


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


def cost_gaps(units):
    """Why a run's cost is incomplete, as short reasons. Empty list means complete."""
    known = known_costs(units)
    blind = sum(1 for u in units if u.get("stage") == "blind")
    null = sum(1 for u in units if u.get("stage") == "null")
    gaps = []
    if len(known) != len(units):
        gaps.append("%d unit(s) unknown" % (len(units) - len(known)))
    if blind < MIN_BLIND_COST_RECORDS:
        gaps.append("only %d blind cost record(s), need %d" % (blind, MIN_BLIND_COST_RECORDS))
    if null < MIN_NULL_COST_RECORDS:
        gaps.append("only %d null cost record(s), need %d" % (null, MIN_NULL_COST_RECORDS))
    return gaps


def cost_summary(units):
    known = known_costs(units)
    return {"units": units, "total_cost_usd": _round(float(sum(known))),
            "cost_complete": not cost_gaps(units)}


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
    # report.ranked is the B list (verifier-surviving, rank_key order), so the B top is the one to compare.
    ranked = rep["ranked"]
    top_matches_report = (not ranked) or (top_b is not None and top_b["id"] == ranked[0])
    b_pairs = [(h, r) for h, r in zip(rep["hypotheses"], rows) if r["status_B"] == "supported"]
    rank1_alt = min(b_pairs, key=lambda pair: alt_rank_key(pair[0]))[1]["label"] if b_pairs else None

    def top(row):
        return None if row is None else {"id": row["id"], "label": row["label"]}

    def tally(sel):
        return {lab: sum(1 for r in sel if r["label"] == lab) for lab in LABELS}

    valid = sum(1 for r in rows if r["quote_valid"])
    return {
        "format": SCORE_FORMAT, "case": os.path.basename(case_path),
        "case_id": cid if isinstance(cid, str) and cid else None,
        "matcher": matcher, "rubric_sha256": rubric_hash(rubric),
        "run_id": _text(rep.get("run_id")) or os.path.basename(run),
        "decision": rep.get("decision"), "null_verdict": rep.get("null_verdict"),
        "null_valid": rep.get("null_valid"), "ranked": list(ranked),
        "top_matches_report": top_matches_report, "rank1_alt_label": rank1_alt,
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
                               "" if cost["cost_complete"] else " (INCOMPLETE: %s)" % "; ".join(
                                   cost_gaps(cost["units"]))),
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


def config_summary(groups, run_ids):
    """groups: one list of supported rows per run, in run_ids order. Pooled Wilson intervals sit beside a cluster
    bootstrap over runs and the per-run proportions."""
    rows = [h for g in groups for h in g]
    c = label_counts(rows)
    n = len(rows)
    per_run, counts = [], []
    for rid, g in zip(run_ids, groups):
        k = sum(1 for h in g if h["label"] == "HIT")
        counts.append((k, len(g)))
        per_run.append({"run_id": rid, "k": k, "n": len(g), "p": _round(k / len(g)) if g else None})
    return {"n_supported": n, "HIT": c["HIT"], "PARTIAL": c["PARTIAL"], "MISS": c["MISS"],
            "precision_hit": proportion(c["HIT"], n),
            "hit_or_partial": proportion(c["HIT"] + c["PARTIAL"], n),
            "precision_hit_cluster_ci": cluster_bootstrap_ci(counts),
            "per_run_precision_hit": per_run}


def rank1_summary(labels):
    """labels: one per run, the top's label, or None when the run has no top. A run with no top is a miss
    over all runs; p_among_top is the old statistic over runs that have a top. "p" is kept as p_among_top."""
    n = len(labels)
    tops = [x for x in labels if x is not None]
    hit = sum(1 for x in tops if x == "HIT")
    return {"runs": n, "runs_with_top": len(tops), "HIT": hit,
            "p_all_runs": _round(hit / n) if n else None,
            "p_among_top": _round(hit / len(tops)) if tops else None,
            "p": _round(hit / len(tops)) if tops else None}


def style_of(row):
    m = STYLE_RE.match(row["id"])
    return m.group(1) if m else "other"


def per_style(rows):
    """Per blind style: n, quote-valid, label counts over all its rows, and verify coverage (complete = quote-valid
    rows with a verifier verdict, queued = quote-valid rows)."""
    out = {}
    for st in sorted({style_of(h) for h in rows}):
        sel = [h for h in rows if style_of(h) == st]
        out[st] = {"n": len(sel), "quote_valid": sum(1 for h in sel if h["quote_valid"]),
                   "HIT": sum(1 for h in sel if h["label"] == "HIT"),
                   "PARTIAL": sum(1 for h in sel if h["label"] == "PARTIAL"),
                   "MISS": sum(1 for h in sel if h["label"] == "MISS"),
                   "verify": {"complete": sum(1 for h in sel if h["quote_valid"] and h["verifier_verdict"]),
                              "queued": sum(1 for h in sel if h["quote_valid"])}}
    return out


_MISSING = object()   # stands for "score.json has no such key"


def _show(v):
    return MISSING_VALUE if v is _MISSING else json.dumps(v, sort_keys=True)


def provenance(scored):
    """{key: {shown value: [run paths]}} for PROVENANCE_KEYS. A score file without the key is one unnamed value."""
    out = {}
    for key in PROVENANCE_KEYS:
        seen = {}
        for run, s in scored:
            seen.setdefault(_show(s[key] if key in s else _MISSING), []).append(run)
        out[key] = seen
    return out


def refuse_mixed(scored):
    """Raise InputError naming the differing values when the score files do not share case_id, rubric_sha256
    and matcher."""
    problems = []
    for key, seen in provenance(scored).items():
        if len(seen) > 1:
            problems.append("%s differs: %s" % (key, ", ".join(sorted(seen))))
    if problems:
        raise InputError("score files disagree (%s); pass --allow-mixed to pool them anyway" % "; ".join(problems))


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
    run_ids = [s.get("run_id") for _, s in scored]
    cfg_a = config_summary([[h for h in s["hypotheses"] if h["status_A"] == "supported"] for _, s in scored], run_ids)
    # HIT over all hypotheses, quote-invalid ones counted as non-HIT (their label is text-only, not evidence-backed)
    cfg_a["precision_hit_all"] = proportion(sum(1 for h in rows if h["label"] == "HIT" and h["quote_valid"]),
                                            len(rows))
    cfg_b = config_summary([[h for h in s["hypotheses"] if h["status_B"] == "supported"] for _, s in scored], run_ids)
    return {
        "format": AGG_FORMAT, "caveat": CAVEAT,
        "runs": [{"run_id": s.get("run_id"), "case": s.get("case"), "decision": s.get("decision")}
                 for _, s in scored],
        "n_runs": len(scored), "n_hypotheses": len(rows),
        "provenance": {key: sorted(seen) for key, seen in provenance(scored).items()},
        "config_A": cfg_a, "config_B": cfg_b,
        "per_style": per_style(rows),
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
            # rank_mismatch_runs: the scorer's B top is not report ranked[0]; ties_decide_runs: the unit_id
            # tie-break changes the B top's label
            "rank_mismatch_runs": sum(1 for _, s in scored if s.get("top_matches_report") is False),
            "ties_decide_runs": sum(1 for _, s in scored if tie_decides(s)),
        },
        "null_verdicts": null_verdicts, "null_valid": null_valid,
        "cost": {"total_cost_usd": _round(float(sum(known))),
                 "cost_complete": len(known) == len(units) and all(s["cost"].get("cost_complete") is True
                                                                   for _, s in scored),
                 "units": len(units), "units_known": len(known),
                 "per_unit_mean_usd": _round(sum(known) / len(known)) if known else None},
    }


def tie_decides(s):
    """True when the B rank-1 label changes under the unit_id-descending tie-break. False for a score.json that
    predates rank1_alt_label."""
    if "rank1_alt_label" not in s:
        return False
    normal = (s["top_ranked"].get("B") or {}).get("label")
    return s["rank1_alt_label"] != normal


def _p(block):
    if block["p"] is None:
        return "n/a"
    return "%.3f [%.3f, %.3f]" % (block["p"], block["wilson95"][0], block["wilson95"][1])


def _ci(block):
    ci = block.get("precision_hit_cluster_ci")
    return "n/a" if ci is None else "[%.3f, %.3f]" % (ci["lo"], ci["hi"])


def aggregate_summary(agg):
    a, b = agg["config_A"], agg["config_B"]
    mixed = [key for key, vals in agg["provenance"].items() if len(vals) > 1]
    lines = [
        "aggregate: %d run(s), %d hypothesis(es)" % (agg["n_runs"], agg["n_hypotheses"]),
        "A (quote-valid): n=%d HIT %d PARTIAL %d MISS %d precision_hit %s cluster95 %s hit_or_partial %s" % (
            a["n_supported"], a["HIT"], a["PARTIAL"], a["MISS"], _p(a["precision_hit"]), _ci(a),
            _p(a["hit_or_partial"])),
        "B (verifier): n=%d HIT %d PARTIAL %d MISS %d precision_hit %s cluster95 %s hit_or_partial %s" % (
            b["n_supported"], b["HIT"], b["PARTIAL"], b["MISS"], _p(b["precision_hit"]), _ci(b),
            _p(b["hit_or_partial"])),
        "rank-1: report mismatch runs %d, tie-break decides %d run(s)" % (
            agg["rank1"]["rank_mismatch_runs"], agg["rank1"]["ties_decide_runs"]),
        "verifier false rejections (HIT): %d" % agg["verifier_effect"]["false_rejections"],
        "cost: $%s total, per unit mean %s" % (agg["cost"]["total_cost_usd"],
                                             "n/a" if agg["cost"]["per_unit_mean_usd"] is None
                                             else "$%s" % agg["cost"]["per_unit_mean_usd"]),
        "caveat: " + CAVEAT,
    ]
    if mixed:
        lines.append("MIXED: pooled score files differ in %s (allowed by --allow-mixed)" % ", ".join(mixed))
    return "\n".join(lines)


def cmd_aggregate(args):
    runs = sorted({os.path.realpath(r) for r in args.runs})
    scored = [(r, load_score(r)) for r in runs]
    if not args.allow_mixed:
        refuse_mixed(scored)
    agg = aggregate(scored)
    data = dump_bytes(agg)
    if args.json:
        sys.stdout.write(data.decode("utf-8"))
    else:
        print(aggregate_summary(agg))
    return 0


# ---------------------------------------------------------------- held-out sample and agreement
def unique_ids(hyps, path):
    """{hypothesis id: hypothesis}. A repeated id is refused: a (run, id) key would then name two hypotheses."""
    out = {}
    for h in hyps:
        if h["id"] in out:
            raise InputError("duplicate hypothesis id %s: %s" % (h["id"], path))
        out[h["id"]] = h
    return out


def run_names(runs):
    """{run basename: run dir}. The basename is the run's identity in sample and agree, so two dirs sharing one are
    refused."""
    named = {}
    for run in runs:
        name = os.path.basename(os.path.normpath(run))
        if name in named:
            raise InputError("two --run dirs share the basename %s: %s and %s" % (name, named[name], run))
        named[name] = run
    return named


def sample_pool(runs):
    """Sorted (run basename, hypothesis id) for every quote-valid hypothesis in the runs' report.json files."""
    pool = []
    for name, run in run_names(runs).items():
        rep = load_report(run)
        for hid, h in unique_ids(rep["hypotheses"], os.path.join(run, "report.json")).items():
            if h["quote_valid"]:
                pool.append((name, hid))
    return sorted(pool)


def load_exclude(path):
    obj = read_json_file(path, "exclude file")
    if not isinstance(obj, list) or not all(
            isinstance(p, list) and len(p) == 2 and all(isinstance(x, str) for x in p) for p in obj):
        raise InputError("exclude file must be a JSON list of [run basename, id] pairs: %s" % path)
    return {tuple(p) for p in obj}


def cmd_sample(args):
    if args.n < 0:
        raise InputError("--n must be 0 or more, got %d" % args.n)
    pool = sample_pool(args.runs)
    if args.exclude:
        skip = load_exclude(args.exclude)
        pool = [p for p in pool if p not in skip]
    if args.n > len(pool):
        print("note: --n %d is larger than the %d quote-valid hypotheses left in the pool; taking all %d"
              % (args.n, len(pool), len(pool)), file=sys.stderr)
        chosen = list(pool)
    else:
        chosen = random.Random(args.seed).sample(pool, args.n)
    sys.stdout.write(json.dumps([list(p) for p in chosen]) + "\n")
    return 0


def load_labels(path):
    """[(run basename, hypothesis id, given label)] from {"<run>/<id>": label} or [[run, id, label], ...]."""
    obj = read_json_file(path, "labels")
    if isinstance(obj, dict):
        items = []
        for key, label in obj.items():
            run, sep, hid = key.partition("/")
            if not sep or not run or not hid:
                raise InputError("label key must be <run basename>/<id>, got %r: %s" % (key, path))
            items.append((run, hid, label))
    elif isinstance(obj, list) and all(
            isinstance(e, list) and len(e) == 3 and all(isinstance(x, str) for x in e) for e in obj):
        items = [tuple(e) for e in obj]
    else:
        raise InputError("labels must be a JSON object {run/id: label} or a list of [run, id, label]: %s" % path)
    seen = set()
    for run, hid, label in items:
        if label not in LABELS:
            raise InputError("label %s/%s is %r, not HIT, PARTIAL or MISS" % (run, hid, label))
        if (run, hid) in seen:
            raise InputError("label %s/%s is given twice" % (run, hid))
        seen.add((run, hid))
    return items


def agreement(pairs):
    """Raw agreement and Cohen's kappa over (given, scorer) label pairs, computed exactly with Fraction.
    kappa is 1.0 when every pair agrees (this includes one label only, where expected agreement is 1). It is None
    (n/a) only when expected agreement is 1 without full agreement, which exact labels cannot produce."""
    n = len(pairs)
    given = {lab: 0 for lab in LABELS}
    scorer = {lab: 0 for lab in LABELS}
    conf = {g: {s: 0 for s in LABELS} for g in LABELS}
    for g, s in pairs:
        given[g] += 1
        scorer[s] += 1
        conf[g][s] += 1
    agree = sum(conf[lab][lab] for lab in LABELS)
    p_o = Fraction(agree, n)
    p_e = sum((Fraction(given[lab], n) * Fraction(scorer[lab], n) for lab in LABELS), Fraction(0))
    if agree == n:
        kappa = 1.0
    elif p_e == 1:
        kappa = None
    else:
        kappa = float((p_o - p_e) / (1 - p_e))
    return {"format": AGREE_FORMAT, "n": n, "agree": agree, "raw_agreement": _round(float(p_o)),
            "expected_agreement": _round(float(p_e)), "kappa": _round(kappa), "given_counts": given,
            "scorer_counts": scorer, "confusion": conf}


def agree_summary(a):
    kappa = "n/a" if a["kappa"] is None else "%.3f" % a["kappa"]
    lines = ["agree: %d labelled ids; raw agreement %.3f (%d/%d); Cohen's kappa %s" % (
        a["n"], a["raw_agreement"], a["agree"], a["n"], kappa),
        "confusion (rows = given label, columns = scorer label):",
        "%-8s%s" % ("", "".join("%9s" % s for s in LABELS))]
    for g in LABELS:
        lines.append("%-8s%s" % (g, "".join("%9d" % a["confusion"][g][s] for s in LABELS)))
    return "\n".join(lines)


def cmd_agree(args):
    named = run_names(args.runs)
    items = load_labels(args.labels)
    if not items:
        raise InputError("no labels in %s" % args.labels)
    want_sha = rubric_hash(load_rubric(args.case)[2]["score_rubric"]) if args.case else None
    loaded, by_id = {}, {}   # run basename -> score dict, and -> {hypothesis id: hypothesis row}
    for name, hid, _ in items:
        if name not in named:
            raise InputError("label %s/%s: run %s is not one of the --run dirs" % (name, hid, name))
        if name in loaded:
            continue
        if not os.path.isfile(os.path.join(named[name], "score.json")):
            raise InputError("label %s/%s: no score.json in %s (score that run first)" % (name, hid, named[name]))
        loaded[name] = load_score(named[name])
        by_id[name] = unique_ids(loaded[name]["hypotheses"], os.path.join(named[name], "score.json"))
        if want_sha is not None and loaded[name].get("rubric_sha256") != want_sha:
            raise InputError("label %s/%s: score.json rubric_sha256 in %s is not the rubric of --case %s"
                             % (name, hid, named[name], args.case))
    problems = ["%s differs: %s" % (key, ", ".join(sorted(seen)))
                for key, seen in provenance(sorted(loaded.items())).items() if len(seen) > 1]
    if problems:
        raise InputError("score files disagree (%s); agree needs labels scored under one case and matcher"
                         % "; ".join(problems))
    pairs = []
    for name, hid, given in items:
        row = by_id[name].get(hid)
        if row is None:
            raise InputError("label %s/%s: no hypothesis %s in %s" % (name, hid, hid,
                                                                      os.path.join(named[name], "score.json")))
        pairs.append((given, row["label"]))
    a = agreement(pairs)
    if args.json:
        sys.stdout.write(json.dumps(a, sort_keys=True, indent=2) + "\n")
    else:
        print(agree_summary(a))
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
    pa.add_argument("--allow-mixed", action="store_true",
                    help="pool score files that differ in case_id, rubric_sha256 or matcher (default: refuse, exit 2)")
    pa.add_argument("--json", action="store_true", help="print the aggregate JSON instead of a summary")
    pz = sub.add_parser("sample", help="draw N quote-valid hypotheses from report.json files (writes nothing)")
    pz.add_argument("--run", action="append", dest="runs", required=True, help="run dir holding report.json (repeat)")
    pz.add_argument("--n", type=int, required=True, help="how many to draw (the whole pool when N is larger)")
    pz.add_argument("--seed", type=int, required=True, help="seed for random.Random")
    pz.add_argument("--exclude", help="JSON list of [run basename, id] pairs to leave out of the pool")
    pg = sub.add_parser("agree", help="raw agreement and Cohen's kappa of labels vs score.json (writes nothing)")
    pg.add_argument("--labels", required=True, help="JSON {run/id: HIT|PARTIAL|MISS} or [[run, id, label], ...]")
    pg.add_argument("--run", action="append", dest="runs", required=True, help="run dir holding score.json (repeat)")
    pg.add_argument("--case", help="case JSON; each score.json must carry this case's score_rubric hash")
    pg.add_argument("--json", action="store_true", help="print the agreement JSON instead of a summary")
    try:
        args = parser.parse_args(argv)
    except SystemExit as e:
        return e.code if isinstance(e.code, int) else 2
    handlers = {"score": cmd_score, "aggregate": cmd_aggregate, "sample": cmd_sample, "agree": cmd_agree}
    try:
        return handlers[args.cmd](args)
    except InputError as e:
        print("error: %s" % e, file=sys.stderr)
        return 2
    except OutputError as e:
        print("error: %s" % e, file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
