"""Pre-registered router evaluation harness: rules vs Kev vs Haiku on labelled swarm work units.

  python3 scripts/router_eval.py validate-cases
  python3 scripts/router_eval.py export-blind --stage map
  python3 scripts/router_eval.py compare-labels
  python3 scripts/router_eval.py run --engines rules,kev,haiku --passes 2 --budget-usd 0.05 --dry-run
  python3 scripts/router_eval.py score --run <id>
  python3 scripts/router_eval.py selftest

Inputs live in .claude/router_eval/ (catalog.json, gates.json, router_prompt.txt, cases.jsonl) and are
read-only. Runs land in .claude/router_eval/runs/<run-id>/. Stdlib only. Tests: scripts/test_router_eval.py.
Never imports route_task (it writes the ledger); Kev is reached through clef_matcher.query_clef_engine only.
"""
import argparse
import hashlib
import json
import math
import os
import random
import re
import subprocess
import sys
import threading
import time
import unittest
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
EVAL_DIR = REPO / ".claude" / "router_eval"
DEFAULT_CASES = EVAL_DIR / "cases.jsonl"
DEFAULT_RUNS = EVAL_DIR / "runs"
LABEL_DIR = EVAL_DIR / "labelcheck"

STAGES = ("map", "verify", "adjudicate", "fix")
ENGINES = ("rules", "kev", "haiku")
EXPECTED_COUNTS = {"map": 14, "verify": 10, "adjudicate": 8, "fix": 8}
KEV_TIMEOUT = 5.0
HAIKU_TIMEOUT = 180
HAIKU_WORKERS = 4
EPS = 1e-9

OUTCOME_ACTIONS = {
    "rules": "ship rules-only",
    "kev": "wire Kev via questions.profile, keep rules for floors",
    "haiku": "wire a wave-level Haiku router at the measured $/call",
    "none": ("rules plus human review on hard cases, revisit catalog and rubric, "
             "consider direct API once a key exists"),
}


# ---------------------------------------------------------------- catalog and cases

class Catalog:
    """Profile catalog: ids, stages and within-stage ranks."""

    def __init__(self, items):
        self.items = list(items)
        self.by_id = {i["id"]: i for i in self.items}
        self.ids = [i["id"] for i in self.items]

    def allowed(self, stage):
        return [i["id"] for i in sorted((x for x in self.items if x["stage"] == stage),
                                        key=lambda x: x["rank"])]

    def rank(self, pid):
        return self.by_id[pid]["rank"]

    def description(self, pid):
        return self.by_id[pid]["description"]


def load_catalog(path=None):
    with open(path or EVAL_DIR / "catalog.json", encoding="utf-8") as f:
        return Catalog(json.load(f))


def load_gates(path=None):
    with open(path or EVAL_DIR / "gates.json", encoding="utf-8") as f:
        return json.load(f)


def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


def sha256_file(path):
    return sha256_bytes(Path(path).read_bytes())


def load_cases_raw(path):
    """Return (cases, errors); one JSON object per line, blank lines skipped."""
    cases, errors = [], []
    with open(path, encoding="utf-8") as f:
        for n, line in enumerate(f, 1):
            if not line.strip():
                continue
            try:
                obj = json.loads(line)
            except ValueError as e:
                errors.append(f"line {n}: invalid JSON ({e})")
                continue
            if not isinstance(obj, dict):
                errors.append(f"line {n}: not a JSON object")
                continue
            obj["_line"] = n
            cases.append(obj)
    return cases, errors


def _plural(n, word):
    return f"{n} {word}" if n == 1 else f"{n} {word}s"


def render_prompt(case, cat):
    """Exact router user message for a case (deterministic)."""
    stage = case["stage"]
    lines = [
        f"Stage: {stage}",
        f"Unit: {_plural(case['files'], 'file')}, {_plural(case['lines'], 'line')}",
        f"Paths: {', '.join(case['paths'])}",
        f"Domain: {case['domain']}",
    ]
    if stage == "adjudicate":
        lines.append(f"Prior judge splits: {case.get('prior_splits', 0)}")
    lines.append(f"Task: {case['task']}")
    lines.append(f"Allowed profiles: {' | '.join(cat.allowed(stage))}")
    return "\n".join(lines)


def prompt_hash(prompt):
    return sha256_bytes(" ".join(prompt.split()).lower().encode("utf-8"))


def _is_int(v):
    return isinstance(v, int) and not isinstance(v, bool)


def _words(s):
    return len(s.split())


def validate_case_lines(cases, cat):
    """Per-line schema checks plus duplicate id/prompt detection. Returns error strings."""
    errors = []
    seen_ids, seen_hash = {}, {}
    for c in cases:
        ln = c.get("_line", "?")
        tag = f"line {ln} ({c.get('id', '?')})"
        bad = []
        for key, typ in (("id", str), ("domain", str), ("task", str), ("source", str)):
            if not isinstance(c.get(key), typ) or (key in ("id", "task") and not c.get(key)):
                bad.append(f"{key} must be a non-empty string" if key in ("id", "task")
                           else f"{key} must be a string")
        for key in ("files", "lines"):
            if not _is_int(c.get(key)) or c[key] < 0:
                bad.append(f"{key} must be an int >= 0")
        if not (isinstance(c.get("paths"), list) and all(isinstance(p, str) for p in c["paths"])):
            bad.append("paths must be a list of strings")
        stage = c.get("stage")
        if stage not in STAGES:
            bad.append(f"stage must be one of {STAGES}")
        gold = c.get("gold")
        if gold not in cat.by_id:
            bad.append("gold not in catalog")
        elif stage in STAGES and cat.by_id[gold]["stage"] != stage:
            bad.append(f"gold {gold} not in stage {stage}")
        if c.get("gold_basis") not in ("rubric", "outcome"):
            bad.append("gold_basis must be rubric|outcome")
        if not isinstance(c.get("hard"), bool):
            bad.append("hard must be a bool")
        why = c.get("why_hard")
        if c.get("hard") is True and not (isinstance(why, str) and why.strip()):
            bad.append("hard requires a non-empty why_hard")
        if isinstance(why, str) and _words(why) > 20:
            bad.append("why_hard exceeds 20 words")
        if isinstance(c.get("task"), str) and _words(c["task"]) > 40:
            bad.append("task exceeds 40 words")
        if "prior_splits" in c and (not _is_int(c["prior_splits"]) or c["prior_splits"] < 0):
            bad.append("prior_splits must be an int >= 0")
        errors.extend(f"{tag}: {b}" for b in bad)
        if bad:
            continue
        if c["id"] in seen_ids:
            errors.append(f"{tag}: duplicate id (also line {seen_ids[c['id']]})")
        seen_ids.setdefault(c["id"], ln)
        h = prompt_hash(render_prompt(c, cat))
        if h in seen_hash:
            errors.append(f"{tag}: duplicate normalized prompt (same as {seen_hash[h]})")
        else:
            seen_hash[h] = c["id"]
    return errors


def validate_cases(cases, cat):
    """Return (errors, counts). Counts include the pre-registered distribution checks."""
    errors = validate_case_lines(cases, cat)
    lines_ok = not errors
    counts = {"total": len(cases)}
    for s in STAGES:
        counts[s] = sum(1 for c in cases if c.get("stage") == s)
    hard = [c for c in cases if c.get("hard") is True]
    counts["hard"] = len(hard)
    for s in STAGES:
        counts[f"hard_{s}"] = sum(1 for c in hard if c.get("stage") == s)
    if counts["total"] != 40:
        errors.append(f"total cases {counts['total']} != 40")
    for s, want in EXPECTED_COUNTS.items():
        if counts[s] != want:
            errors.append(f"stage {s}: {counts[s]} cases != {want}")
    if counts["hard"] < 12:
        errors.append(f"hard cases {counts['hard']} < 12")
    for s in STAGES:
        if counts[f"hard_{s}"] < 3:
            errors.append(f"hard cases in stage {s}: {counts[f'hard_{s}']} < 3")
    if lines_ok:
        under = sum(1 for c in hard if classify(rules_route(c), c["gold"], cat, c["stage"]) == "under")
        counts["rules_hard_under"] = under
        if hard and under * 2 < len(hard):
            errors.append(f"rules under-route {under}/{len(hard)} of hard cases < 50%")
    return errors, counts


# ---------------------------------------------------------------- rules baseline and policies

def rules_route(case):
    """Structured-fields-only baseline router."""
    stage, paths = case["stage"], case["paths"]
    suffixes = [os.path.splitext(p)[1].lower() for p in paths]
    if stage == "map":
        if (any("_shared_src/" in p for p in paths)
                or any("hook" in os.path.basename(p).lower() for p in paths)
                or any(s in (".asm", ".s") for s in suffixes)
                or case["lines"] > 600):
            return "map.high"
        if all(s in (".md", ".toml", ".json", ".txt") for s in suffixes) or case["lines"] <= 150:
            return "map.low"
        return "map.medium"
    if stage == "verify":
        return "verify.high" if case["domain"] in ("memory", "hook", "asm") else "verify.medium"
    if stage == "adjudicate":
        return "escalate.sonnet" if case.get("prior_splits", 0) >= 2 else "adjudicate.ladder"
    return "fix.sonnet" if case["files"] > 1 else "fix.haiku"


def classify(pred, gold, cat, stage):
    """correct | under | over | invalid (pred must be an allowed id of the stage)."""
    if pred not in cat.allowed(stage):
        return "invalid"
    rp, rg = cat.rank(pred), cat.rank(gold)
    if rp == rg:
        return "correct"
    return "under" if rp < rg else "over"


def _higher(a, b, cat):
    return a if cat.rank(a) >= cat.rank(b) else b


def haiku_p1(choice, runner_up, cat, stage):
    """Escalate on doubt: higher rank of choice/runner_up when runner_up is valid for the stage."""
    allowed = cat.allowed(stage)
    if choice in allowed and runner_up in allowed:
        return _higher(choice, runner_up, cat)
    return choice


def kev_top2(probs, cat, stage):
    """Top two allowed probabilities as ([id1, id2], p1, p2); catalog order breaks ties."""
    if not isinstance(probs, dict):
        return [], None, None
    cand = [(pid, float(probs[pid])) for pid in cat.allowed(stage)
            if isinstance(probs.get(pid), (int, float)) and not isinstance(probs.get(pid), bool)]
    cand.sort(key=lambda t: -t[1])
    top = cand[:2]
    return ([t[0] for t in top], top[0][1] if top else None,
            top[1][1] if len(top) > 1 else None)


def kev_p1(choice, top2, p1, p2, cat, stage):
    """Escalate when p2/p1 >= 0.6 (exactly 0.6 escalates), else keep the top choice."""
    allowed = cat.allowed(stage)
    if choice not in allowed or len(top2 or []) < 2 or not p1 or p1 <= 0 or p2 is None:
        return choice
    if p2 / p1 >= 0.6 - EPS and all(t in allowed for t in top2):
        return _higher(top2[0], top2[1], cat)
    return choice


def policy_choice(engine, policy, row, cat, stage):
    if row is None:
        return None
    choice = row.get("choice")
    if policy == "P0" or engine == "rules":
        return choice
    if engine == "haiku":
        return haiku_p1(choice, row.get("runner_up"), cat, stage)
    return kev_p1(choice, row.get("top2"), row.get("p1"), row.get("p2"), cat, stage)


# ---------------------------------------------------------------- engines

def kev_payload(case, cat):
    stage = case["stage"]
    return {
        "model": "kev-latest",
        "state": {"prompt": render_prompt(case, cat), "active_mod": "none", "has_diff": False,
                  "diff_lines": 0, "historical_prior": {}},
        "questions": {"profile": {
            "type": "choice",
            "instructions": "Which swarm profile fits this unit?",
            "criteria": {pid: cat.description(pid) for pid in cat.allowed(stage)},
        }},
    }


def default_kev_fn(payload, timeout_sec):
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    from clef_matcher import query_clef_engine
    return query_clef_engine(payload, timeout_sec)


def kev_call(case, cat, kev_fn, pass_n, seed, clock=time.perf_counter):
    """One Kev query -> row. failure=True means the daemon returned None."""
    stage = case["stage"]
    t0 = clock()
    try:
        resp = kev_fn(kev_payload(case, cat), KEV_TIMEOUT)
    except Exception:
        resp = None
    wall_ms = round((clock() - t0) * 1000.0, 1)
    row = {"id": case["id"], "stage": stage, "engine": "kev", "pass": pass_n, "seed": seed,
           "choice": None, "p1": None, "p2": None, "top2": [], "invalid": True,
           "failure": resp is None, "wall_ms": wall_ms, "latency_ms": None}
    if resp is None or not isinstance(resp, dict):
        row["failure"] = True
        return row
    prof = (resp.get("answers") or {}).get("profile")
    prof = prof if isinstance(prof, dict) else {}
    lat = resp.get("latency_ms", prof.get("latency_ms"))
    row["latency_ms"] = lat if isinstance(lat, (int, float)) else None
    choice = prof.get("choice")
    row["choice"] = choice if isinstance(choice, str) else None
    row["top2"], row["p1"], row["p2"] = kev_top2(prof.get("probabilities"), cat, stage)
    row["invalid"] = row["choice"] not in cat.allowed(stage)
    return row


def haiku_schema_json(cat):
    ids = cat.ids
    return json.dumps({"type": "object", "properties": {
        "choice": {"type": "string", "enum": ids},
        "runner_up": {"type": "string", "enum": ids},
        "confidence": {"type": "number"},
        "rationale": {"type": "string", "maxLength": 120},
    }, "required": ["choice", "runner_up", "confidence", "rationale"]})


def haiku_argv(rendered, router_prompt, schema_json):
    return ["claude", "-p", rendered, "--model", "claude-haiku-5-5", "--effort", "low",
            "--tools", "", "--no-session-persistence", "--strict-mcp-config",
            "--disable-slash-commands", "--permission-mode", "dontAsk",
            "--system-prompt", router_prompt, "--output-format", "json",
            "--json-schema", schema_json, "--max-budget-usd", "0.01"]


def default_runner(argv, cwd, env, timeout):
    p = subprocess.run(argv, cwd=cwd, env=env, capture_output=True, text=True, timeout=timeout)
    return p.returncode, p.stdout, p.stderr


def haiku_env():
    env = dict(os.environ)
    env.pop("CLAUDE_EFFORT", None)
    env.pop("CLAUDE_CODE_EFFORT_LEVEL", None)
    return env


def calc_cost(inp, cache_create, cache_read, out):
    """Expected USD from token counts (Haiku rates); None counts as 0."""
    return ((inp or 0) * 1e-7 + (cache_create or 0) * 2e-7
            + (cache_read or 0) * 1e-8 + (out or 0) * 5e-7)


def cost_check(reported, inp, cache_create, cache_read, out):
    """n/a when no reported cost, ok within 2e-5 USD of the token-derived cost, else mismatch."""
    if reported is None:
        return "n/a"
    return "ok" if abs(calc_cost(inp, cache_create, cache_read, out) - reported) <= 0.00002 \
        else "mismatch"


def _num(v):
    return v if isinstance(v, (int, float)) and not isinstance(v, bool) else None


def _parse_json(text):
    try:
        return json.loads(text)
    except (TypeError, ValueError):
        return None


def _extract_choice(data):
    """(choice, runner_up) from structured_output, else from JSON in result; None when absent."""
    so = data.get("structured_output")
    if isinstance(so, dict) and isinstance(so.get("choice"), str):
        return so["choice"], so.get("runner_up")
    res = _parse_json(data.get("result")) if isinstance(data.get("result"), str) else None
    if isinstance(res, dict) and isinstance(res.get("choice"), str):
        return res["choice"], res.get("runner_up")
    return None, None


def haiku_call(case, cat, router_prompt, schema_json, runner, cwd, env, pass_n, seed,
               sleep=time.sleep, clock=time.perf_counter):
    """One headless Haiku routing call -> row. Retries once on API 429/529."""
    stage = case["stage"]
    argv = haiku_argv(render_prompt(case, cat), router_prompt, schema_json)
    t0 = clock()
    attempt = 0
    while True:
        attempt += 1
        data, err = None, None
        try:
            rc, out, _ = runner(argv, str(cwd), env, HAIKU_TIMEOUT)
        except subprocess.TimeoutExpired:
            rc, out, err = None, "", "timeout"
        except Exception as e:
            rc, out, err = None, "", f"runner error: {type(e).__name__}"
        if err is None:
            data = _parse_json(out)
            if rc != 0:
                err = f"returncode {rc}"
            elif not isinstance(data, dict):
                err = "stdout not JSON"
            elif data.get("is_error"):
                err = "is_error"
        if (attempt == 1 and isinstance(data, dict)
                and data.get("api_error_status") in (429, 529)):
            sleep(2)
            continue
        break
    wall_ms = round((clock() - t0) * 1000.0, 1)
    row = {"id": case["id"], "stage": stage, "engine": "haiku", "pass": pass_n, "seed": seed,
           "choice": None, "runner_up": None, "invalid": True, "failure": err is not None,
           "cost_usd": None, "input_tokens": None, "cache_creation_input_tokens": None,
           "cache_read_input_tokens": None, "output_tokens": None, "thinking_tokens": None,
           "duration_ms": None, "wall_ms": wall_ms, "cold_write": False, "cost_check": "n/a"}
    if err is not None:
        row["error"] = err
        if isinstance(data, dict):  # a failed call can still have spent money
            row["cost_usd"] = _num(data.get("total_cost_usd"))
        return row
    choice, runner_up = _extract_choice(data)
    row["choice"], row["runner_up"] = choice, runner_up if isinstance(runner_up, str) else None
    row["invalid"] = choice not in cat.allowed(stage)
    usage = data.get("usage") if isinstance(data.get("usage"), dict) else {}
    row["cost_usd"] = _num(data.get("total_cost_usd"))
    row["input_tokens"] = _num(usage.get("input_tokens"))
    row["cache_creation_input_tokens"] = _num(usage.get("cache_creation_input_tokens"))
    row["cache_read_input_tokens"] = _num(usage.get("cache_read_input_tokens"))
    row["output_tokens"] = _num(usage.get("output_tokens"))
    det = usage.get("output_tokens_details")
    row["thinking_tokens"] = _num(det.get("thinking_tokens")) if isinstance(det, dict) else None
    row["duration_ms"] = _num(data.get("duration_ms"))
    row["cold_write"] = (row["cache_creation_input_tokens"] or 0) > 0
    row["cost_check"] = cost_check(row["cost_usd"], row["input_tokens"],
                                   row["cache_creation_input_tokens"],
                                   row["cache_read_input_tokens"], row["output_tokens"])
    return row


# ---------------------------------------------------------------- run orchestration

def plan_run(n_cases, engines, passes):
    """Call counts and estimated Haiku cost for a run (dry-run and budget refusal)."""
    calls = {}
    if "rules" in engines:
        calls["rules"] = n_cases
    if "kev" in engines:
        calls["kev"] = n_cases * passes
    if "haiku" in engines:
        calls["haiku"] = n_cases * passes
    h = calls.get("haiku", 0)
    est = 0.001 + 0.0003 * (h - 1) if h > 0 else 0.0
    return calls, est


def pass_order(cases, seed):
    order = list(cases)
    random.Random(seed).shuffle(order)
    return order


class RowWriter:
    """Thread-safe per-(engine, pass) JSONL writer that flushes every row."""

    def __init__(self, run_dir):
        self.run_dir = Path(run_dir)
        self.run_dir.mkdir(parents=True, exist_ok=True)
        self.handles = {}
        self.lock = threading.Lock()
        self.rows = []

    def write(self, row):
        key = (row["engine"], row["pass"])
        with self.lock:
            if key not in self.handles:
                self.handles[key] = open(self.run_dir / f"{key[0]}_pass{key[1]}.jsonl", "a",
                                         encoding="utf-8")
            fh = self.handles[key]
            fh.write(json.dumps(row) + "\n")
            fh.flush()
            self.rows.append(row)

    def close(self):
        for fh in self.handles.values():
            fh.close()


def _claude_version():
    try:
        p = subprocess.run(["claude", "--version"], capture_output=True, text=True, timeout=20)
        return (p.stdout or p.stderr).strip()
    except Exception:
        return "unavailable"


def run_eval(cases, cat, engines, passes, budget, runs_dir, run_id, *, router_prompt,
             kev_fn=None, runner=None, sleep=time.sleep, version_fn=None, haiku_cwd=None,
             hashes=None, workers=HAIKU_WORKERS):
    """Execute the engines over cases; write rows and summary.json; return the summary dict."""
    started = datetime.now().astimezone()
    run_dir = Path(runs_dir) / run_id
    writer = RowWriter(run_dir)
    schema_json = haiku_schema_json(cat)
    seeds = {n: n for n in range(1, passes + 1)}
    aborted = {}
    state = {"spent": 0.0, "streak": 0}
    lock = threading.Lock()

    try:
        if "rules" in engines:
            for c in cases:
                choice = rules_route(c)
                writer.write({"id": c["id"], "stage": c["stage"], "engine": "rules", "pass": 1,
                              "seed": 1, "choice": choice, "invalid": False, "failure": False})

        if "kev" in engines:
            fn = kev_fn or default_kev_fn
            streak = 0
            for n in range(1, passes + 1):
                for c in pass_order(cases, seeds[n]):
                    row = kev_call(c, cat, fn, n, seeds[n])
                    writer.write(row)
                    streak = streak + 1 if row["failure"] else 0
                    if streak >= 3:
                        aborted["kev"] = "consecutive_none"
                        break
                if "kev" in aborted:
                    break

        if "haiku" in engines:
            runner = runner or default_runner
            cwd = Path(haiku_cwd) if haiku_cwd else Path.home() / ".cache" / "router_eval" / "cwd"
            cwd.mkdir(parents=True, exist_ok=True)
            env = haiku_env()
            tasks = [(n, seeds[n], c) for n in range(1, passes + 1)
                     for c in pass_order(cases, seeds[n])]

            def do(task):
                n, seed, c = task
                with lock:
                    if "haiku" in aborted:
                        return
                    if state["spent"] >= budget:
                        aborted["haiku"] = "budget"
                        return
                row = haiku_call(c, cat, router_prompt, schema_json, runner, cwd, env, n, seed,
                                 sleep=sleep)
                with lock:
                    cost = row["cost_usd"]
                    if cost is None and not row["failure"]:
                        cost = calc_cost(row["input_tokens"], row["cache_creation_input_tokens"],
                                         row["cache_read_input_tokens"], row["output_tokens"])
                    state["spent"] += cost or 0.0
                    state["streak"] = state["streak"] + 1 if row["failure"] else 0
                    if state["streak"] >= 3 and "haiku" not in aborted:
                        aborted["haiku"] = "consecutive_failures"
                    writer.write(row)

            if tasks:
                do(tasks[0])  # first call alone to warm the prompt cache
                with ThreadPoolExecutor(max_workers=workers) as ex:
                    list(ex.map(do, tasks[1:]))
    finally:
        writer.close()

    rows = writer.rows
    by = lambda e: [r for r in rows if r["engine"] == e]  # noqa: E731
    hrows = by("haiku")
    summary = {
        "run_id": run_id,
        "started": started.isoformat(timespec="seconds"),
        "ended": datetime.now().astimezone().isoformat(timespec="seconds"),
        "engines": list(engines), "passes": passes, "budget_usd": budget,
        "seeds": seeds,
        "sha256": dict(hashes or {}, schema_json=sha256_bytes(schema_json.encode("utf-8"))),
        "claude_version": (version_fn or _claude_version)(),
        "python_version": sys.version.split()[0],
        "cases": len(cases),
        "calls": {e: len(by(e)) for e in engines},
        "haiku_spent_usd": round(state["spent"], 6),
        "cost_missing": sum(1 for r in hrows if r["cost_usd"] is None and not r["failure"]),
        "cost_check_mismatch": sum(1 for r in hrows if r["cost_check"] == "mismatch"),
        "cold_writes": sum(1 for r in hrows if r["cold_write"]),
        "kev_none": sum(1 for r in by("kev") if r["failure"]),
        "failures": {e: sum(1 for r in by(e) if r["failure"]) for e in engines},
        "invalid": {e: sum(1 for r in by(e) if r["invalid"] and not r["failure"]) for e in engines},
        "aborted": aborted or None,
    }
    (run_dir / "summary.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    return summary


# ---------------------------------------------------------------- scoring

def wilson(k, n, z=1.96):
    if n == 0:
        return 0.0, 0.0
    p = k / n
    d = 1 + z * z / n
    centre = (p + z * z / (2 * n)) / d
    half = z * math.sqrt(p * (1 - p) / n + z * z / (4 * n * n)) / d
    return max(0.0, centre - half), min(1.0, centre + half)


def percentile(values, p):
    """Nearest-rank percentile; None for an empty list."""
    if not values:
        return None
    s = sorted(values)
    return s[max(0, math.ceil(p * len(s)) - 1)]


def exact_sign_p(b, c):
    """Two-sided exact binomial (sign) test on discordant pairs."""
    n = b + c
    if n == 0:
        return 1.0
    return min(1.0, 2 * sum(math.comb(n, k) for k in range(min(b, c) + 1)) / 2 ** n)


def evaluate_policy(engine, policy, cases, rows1, rows2, cat):
    """Metrics for one engine/policy. A case with no pass-1 row is wrong AND invalid."""
    n = len(cases)
    correct = under = over = invalid = 0
    hard_n = hard_under = hard_under_va = 0
    stage = {s: [0, 0] for s in STAGES}  # [n, correct]
    for c in cases:
        pred = policy_choice(engine, policy, rows1.get(c["id"]), cat, c["stage"])
        kind = classify(pred, c["gold"], cat, c["stage"])
        stage[c["stage"]][0] += 1
        correct += kind == "correct"
        stage[c["stage"]][1] += kind == "correct"
        under += kind == "under"
        over += kind == "over"
        invalid += kind == "invalid"
        if c["hard"]:
            hard_n += 1
            if kind == "under":
                hard_under += 1
                hard_under_va += c["stage"] in ("verify", "adjudicate")
    lo, hi = wilson(correct, n)
    if engine == "rules":
        repeat, repeat_n = 1.0, n  # deterministic by construction
    else:
        both = [c for c in cases if c["id"] in rows1 and c["id"] in rows2]
        agree = 0
        for c in both:
            a = policy_choice(engine, policy, rows1[c["id"]], cat, c["stage"])
            b = policy_choice(engine, policy, rows2[c["id"]], cat, c["stage"])
            agree += a in cat.allowed(c["stage"]) and a == b
        repeat, repeat_n = (agree / len(both) if both else None), len(both)
    return {
        "n": n, "correct": correct, "top1": correct / n if n else 0.0,
        "wilson_lo": lo, "wilson_hi": hi,
        "per_stage": {s: {"n": v[0], "correct": v[1], "top1": v[1] / v[0] if v[0] else None}
                      for s, v in stage.items()},
        "under": under, "under_rate": under / n if n else 0.0,
        "over": over, "over_rate": over / n if n else 0.0,
        "hard_n": hard_n, "hard_under": hard_under,
        "hard_under_rate": hard_under / hard_n if hard_n else 0.0,
        "hard_under_va": hard_under_va,
        "invalid": invalid, "invalid_rate": invalid / n if n else 0.0,
        "repeat_agreement": repeat, "repeat_n": repeat_n,
        "repeat_note": "deterministic by construction" if engine == "rules" else None,
    }


def _ge(a, b):
    return a >= b - EPS


def _le(a, b):
    return a <= b + EPS


def evaluate_gates(m, gates):
    """Apply pre-registered gates (boundary values pass). Returns (passes, checks)."""
    rep = m["repeat_agreement"]
    checks = {
        "top1": (m["top1"], gates["top1_min"], ">="),
        "hard_underroute_rate": (m["hard_under_rate"], gates["hard_underroute_max_rate"], "<="),
        "hard_underroute_count_verify_adjudicate":
            (m["hard_under_va"], gates["hard_underroute_max_count_verify_adjudicate"], "<="),
        "repeat_agreement": (rep, gates["repeat_agreement_min"], ">="),
        "invalid_rate": (m["invalid_rate"], gates["invalid_max_rate"], "<="),
    }
    out = {}
    for name, (val, thr, op) in checks.items():
        if val is None:
            ok = False
        else:
            ok = _ge(val, thr) if op == ">=" else _le(val, thr)
        out[name] = {"value": val, "threshold": thr, "op": op, "pass": ok}
    return all(v["pass"] for v in out.values()), out


def load_run_rows(run_dir):
    """{engine: {pass: {id: row}}} from <engine>_pass<N>.jsonl files."""
    rows = {}
    for p in sorted(Path(run_dir).glob("*.jsonl")):
        m = re.fullmatch(r"(\w+)_pass(\d+)\.jsonl", p.name)
        if not m:
            continue
        d = rows.setdefault(m.group(1), {}).setdefault(int(m.group(2)), {})
        for line in p.read_text(encoding="utf-8").splitlines():
            if line.strip():
                r = _parse_json(line)
                if isinstance(r, dict) and "id" in r:
                    d[r["id"]] = r
    return rows


def _p1_correct(engine, policy, rows1, cases, cat):
    return {c["id"]: classify(policy_choice(engine, policy, rows1.get(c["id"]), cat, c["stage"]),
                              c["gold"], cat, c["stage"]) == "correct" for c in cases}


def score_run(run_dir, cases, cat, gates):
    run_dir = Path(run_dir)
    rows = load_run_rows(run_dir)
    summary = _parse_json((run_dir / "summary.json").read_text(encoding="utf-8")) \
        if (run_dir / "summary.json").is_file() else None
    ran = (summary or {}).get("engines") or [e for e in ENGINES if e in rows]
    engines = [e for e in ENGINES if e in ran]
    result = {"run_id": run_dir.name, "n_cases": len(cases), "engines": {}, "paired": None}
    for e in engines:
        r1, r2 = rows.get(e, {}).get(1, {}), rows.get(e, {}).get(2, {})
        pols = ["P0"] if e == "rules" else ["P0", "P1"]
        metrics = {p: evaluate_policy(e, p, cases, r1, r2, cat) for p in pols}
        chosen = "P1" if "P1" in metrics and metrics["P1"]["top1"] > metrics["P0"]["top1"] + EPS \
            else "P0"
        ok, checks = evaluate_gates(metrics[chosen], gates)
        info = {"policies": metrics, "chosen": chosen, "gate_checks": checks, "passes_gates": ok}
        allrows = [r for p in rows.get(e, {}).values() for r in p.values()]
        if e in ("kev", "haiku"):
            wall = [r["wall_ms"] for r in allrows if _num(r.get("wall_ms")) is not None]
            info["latency"] = {"p50_ms": percentile(wall, 0.5), "p90_ms": percentile(wall, 0.9),
                               "n": len(wall)}
        if e == "haiku":
            costs = [r["cost_usd"] for r in allrows if _num(r.get("cost_usd")) is not None]
            think = [r["thinking_tokens"] for r in allrows if _num(r.get("thinking_tokens")) is not None]
            info["cost"] = {"per_call_p50": percentile(costs, 0.5), "total_usd": sum(costs),
                            "calls": len(allrows), "cost_missing": len(allrows) - len(costs),
                            "thinking_p50": percentile(think, 0.5) if think else "n/a",
                            "cold_writes": sum(1 for r in allrows if r.get("cold_write")),
                            "cost_check_mismatch": sum(1 for r in allrows
                                                       if r.get("cost_check") == "mismatch")}
        result["engines"][e] = info
    if "kev" in engines and "haiku" in engines:
        k = _p1_correct("kev", "P0", rows.get("kev", {}).get(1, {}), cases, cat)
        h = _p1_correct("haiku", "P0", rows.get("haiku", {}).get(1, {}), cases, cat)
        b = sum(1 for i in k if k[i] and not h[i])
        c = sum(1 for i in k if h[i] and not k[i])
        result["paired"] = {"kev_only_right": b, "haiku_only_right": c, "p": exact_sign_p(b, c)}
    winner = next((e for e in engines if result["engines"][e]["passes_gates"]), "none")
    result["winner"] = winner
    result["action"] = OUTCOME_ACTIONS[winner]
    return result


def _f(v, nd=3):
    return "n/a" if v is None else (f"{v:.{nd}f}" if isinstance(v, float) else str(v))


def build_report(res):
    """Markdown report, at most 80 lines."""
    L = [f"# Router eval {res['run_id']}", "",
         f"Cases: {res['n_cases']}. Pass 1 is the accuracy pass; a case without a row counts wrong and invalid.",
         "", "## Results (top-1 with 95% Wilson; under/over vs gold; hard = flagged hard cases)"]
    for e, info in res["engines"].items():
        for p, m in info["policies"].items():
            mark = "*" if p == info["chosen"] else " "
            L.append(f"- {e} {p}{mark} top1 {_f(m['top1'])} [{_f(m['wilson_lo'])}-{_f(m['wilson_hi'])}] "
                     f"under {m['under']} ({_f(m['under_rate'])}) over {m['over']} ({_f(m['over_rate'])}) "
                     f"hard-under {m['hard_under']}/{m['hard_n']} ({_f(m['hard_under_rate'])}), "
                     f"verify+adjudicate {m['hard_under_va']}; invalid {m['invalid']} ({_f(m['invalid_rate'])}); "
                     f"repeat {_f(m['repeat_agreement'])}" + (" (deterministic)" if e == "rules" else ""))
            L.append("  per-stage top1: " + ", ".join(
                f"{s} {_f(v['top1'], 2)} ({v['correct']}/{v['n']})" for s, v in m["per_stage"].items()))
    L += ["", "(* = chosen policy: P1 only if strictly better top-1 than P0)", "", "## Latency and cost"]
    for e, info in res["engines"].items():
        if "latency" in info:
            lat = info["latency"]
            L.append(f"- {e} wall_ms p50 {_f(lat['p50_ms'], 1)} p90 {_f(lat['p90_ms'], 1)} (n={lat['n']})")
        if "cost" in info:
            c = info["cost"]
            L.append(f"- haiku $/call p50 {_f(c['per_call_p50'], 6)} total ${_f(c['total_usd'], 6)} "
                     f"thinking p50 {c['thinking_p50']} cold writes {c['cold_writes']} "
                     f"cost missing {c['cost_missing']} mismatches {c['cost_check_mismatch']}")
    if res["paired"]:
        p = res["paired"]
        L += ["", f"## Paired Kev vs Haiku (pass-1 P0): Kev-only right {p['kev_only_right']}, "
                  f"Haiku-only right {p['haiku_only_right']}, exact two-sided p = {p['p']:.4f}"]
    L += ["", "## Gates (chosen policy)"]
    for e, info in res["engines"].items():
        bad = [f"{k} {_f(v['value'])} {'<' if v['op'] == '>=' else '>'} {v['threshold']}"
               for k, v in info["gate_checks"].items() if not v["pass"]]
        L.append(f"- {e} ({info['chosen']}): {'PASS' if info['passes_gates'] else 'FAIL'}"
                 + (f" ({'; '.join(bad)})" if bad else ""))
    L += ["", f"## Decision: {res['winner']}", f"Action: {res['action']}", "",
          "## Framing",
          "- n=40 cannot certify an engine; it can reject bad ones.",
          "- Two policies per engine is a small multiplicity; treat a narrow P1 win with care.",
          "- Label agreement is evaluated by compare-labels, not here."]
    return L[:80]


# ---------------------------------------------------------------- labels

def export_blind(cases, cat, stage, out_dir):
    out_dir = Path(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    path = out_dir / f"{stage}_blind.jsonl"
    n = 0
    with open(path, "w", encoding="utf-8") as f:
        for c in cases:
            if c["stage"] != stage:
                continue
            f.write(json.dumps({
                "id": c["id"], "stage": stage, "files": c["files"], "lines": c["lines"],
                "paths": c["paths"], "domain": c["domain"], "task": c["task"],
                "prior_splits": c.get("prior_splits", 0), "allowed": cat.allowed(stage)}) + "\n")
            n += 1
    return path, n


def compare_labels(cases, label_dir, gates):
    """Return (passed, report_lines). Missing labels count as disagreement."""
    labels = {}
    for p in sorted(Path(label_dir).glob("*_labels.jsonl")):
        for line in p.read_text(encoding="utf-8").splitlines():
            r = _parse_json(line) if line.strip() else None
            if isinstance(r, dict) and "id" in r:
                labels[r["id"]] = r
    out, diffs = [], []
    stats = {b: {s: [0, 0] for s in STAGES} for b in ("rubric", "outcome")}  # [agree, n]
    for c in cases:
        lab = labels.get(c["id"])
        s = stats[c["gold_basis"]][c["stage"]]
        s[1] += 1
        if lab is not None and lab.get("label") == c["gold"]:
            s[0] += 1
        else:
            diffs.append((c["id"], c["stage"], c["gold"], lab.get("label") if lab else "MISSING",
                          lab.get("ambiguous") if lab else None, lab.get("note") if lab else None))
    for basis in ("rubric", "outcome"):
        out.append(f"{basis}-basis agreement:")
        for s in STAGES:
            a, n = stats[basis][s]
            out.append(f"  {s}: {a}/{n}" + (f" = {a / n:.3f}" if n else ""))
        a = sum(v[0] for v in stats[basis].values())
        n = sum(v[1] for v in stats[basis].values())
        out.append(f"  overall: {a}/{n}" + (f" = {a / n:.3f}" if n else ""))
        stats[basis]["_all"] = (a, n)
    out.append("disagreements (id, stage, gold, label, ambiguous, note):")
    out.extend(f"  {d}" for d in diffs)
    if not diffs:
        out.append("  none")
    a, n = stats["rubric"]["_all"]
    thr = gates["label_agreement_min"]
    passed = n > 0 and _ge(a / n, thr)
    out.append(f"label_agreement (rubric basis) {a}/{n} vs min {thr}: {'PASS' if passed else 'FAIL'}")
    return passed, out


# ---------------------------------------------------------------- CLI

def _resolve(path, default):
    return Path(path) if path else default


def _load_cases_or_exit(path, cat):
    """Return (cases, None) or (None, exit_code) after printing a clear message."""
    if not Path(path).is_file():
        print(f"cases file missing: {path}")
        return None, 1
    cases, errs = load_cases_raw(path)
    errs += validate_case_lines(cases, cat)
    if errs:
        print("case errors:")
        for e in errs:
            print(f"  {e}")
        return None, 1
    return cases, None


def cmd_validate(args, cat):
    path = _resolve(args.cases, DEFAULT_CASES)
    if not path.is_file():
        print(f"cases file missing: {path}")
        return 1
    cases, errs = load_cases_raw(path)
    verrs, counts = validate_cases(cases, cat)
    errs += verrs
    print("counts: " + ", ".join(f"{k}={v}" for k, v in counts.items()))
    for e in errs:
        print(f"ERROR {e}")
    print("PASS" if not errs else f"FAIL ({len(errs)} errors)")
    return 0 if not errs else 1


def cmd_export(args, cat):
    cases, code = _load_cases_or_exit(_resolve(args.cases, DEFAULT_CASES), cat)
    if cases is None:
        return code
    path, n = export_blind(cases, cat, args.stage, LABEL_DIR)
    print(f"wrote {n} rows -> {path}")
    return 0


def cmd_compare(args, cat):
    cases, code = _load_cases_or_exit(_resolve(args.cases, DEFAULT_CASES), cat)
    if cases is None:
        return code
    passed, lines = compare_labels(cases, LABEL_DIR, load_gates())
    print("\n".join(lines))
    return 0 if passed else 1


def cmd_run(args, cat):
    engines = [e.strip() for e in args.engines.split(",") if e.strip()]
    if not engines or any(e not in ENGINES for e in engines):
        print(f"--engines must be a comma list from {ENGINES}")
        return 1
    if args.passes < 1:
        print("--passes must be >= 1")
        return 1
    cases, code = _load_cases_or_exit(_resolve(args.cases, DEFAULT_CASES), cat)
    if cases is None:
        return code
    if args.limit:
        cases = cases[:args.limit]
    calls, est = plan_run(len(cases), engines, args.passes)
    print("planned calls: " + ", ".join(f"{k}={v}" for k, v in calls.items()))
    print(f"estimated haiku cost: ${est:.4f} (budget ${args.budget_usd:.4f})")
    if est > args.budget_usd:
        print("refusing: estimated Haiku cost exceeds --budget-usd")
        return 2
    if args.dry_run:
        print("dry run: nothing written, no calls made")
        return 0
    run_id = args.run_id or datetime.now().strftime("%Y%m%dT%H%M%S")
    runs_dir = _resolve(args.runs_dir, DEFAULT_RUNS)
    prompt_path = EVAL_DIR / "router_prompt.txt"
    summary = run_eval(
        cases, cat, engines, args.passes, args.budget_usd, runs_dir, run_id,
        router_prompt=prompt_path.read_text(encoding="utf-8"),
        hashes={"catalog.json": sha256_file(EVAL_DIR / "catalog.json"),
                "gates.json": sha256_file(EVAL_DIR / "gates.json"),
                "router_prompt.txt": sha256_file(prompt_path),
                "cases.jsonl": sha256_file(_resolve(args.cases, DEFAULT_CASES))})
    print(json.dumps(summary, indent=2))
    print(f"run dir: {Path(runs_dir) / run_id}")
    return 0


def cmd_score(args, cat):
    cases, code = _load_cases_or_exit(_resolve(args.cases, DEFAULT_CASES), cat)
    if cases is None:
        return code
    run_dir = _resolve(args.runs_dir, DEFAULT_RUNS) / args.run
    if not run_dir.is_dir():
        print(f"run directory missing: {run_dir}")
        return 1
    res = score_run(run_dir, cases, cat, load_gates())
    (run_dir / "scores.json").write_text(json.dumps(res, indent=2) + "\n", encoding="utf-8")
    report = build_report(res)
    (run_dir / "report.md").write_text("\n".join(report) + "\n", encoding="utf-8")
    print("\n".join(report))
    return 0


def cmd_selftest(args, cat):
    import importlib.util
    path = Path(__file__).resolve().parent / "test_router_eval.py"
    spec = importlib.util.spec_from_file_location("test_router_eval", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    suite = unittest.defaultTestLoader.loadTestsFromModule(mod)
    return 0 if unittest.TextTestRunner(verbosity=1).run(suite).wasSuccessful() else 1


def build_parser():
    common = argparse.ArgumentParser(add_help=False)
    common.add_argument("--cases", default=argparse.SUPPRESS, help="cases.jsonl path")
    common.add_argument("--runs-dir", default=argparse.SUPPRESS, help="runs directory")
    ap = argparse.ArgumentParser(description="Router evaluation harness.")
    ap.add_argument("--cases", default=None, help="cases.jsonl (default .claude/router_eval/cases.jsonl)")
    ap.add_argument("--runs-dir", default=None, help="runs dir (default .claude/router_eval/runs)")
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("validate-cases", parents=[common])
    p = sub.add_parser("export-blind", parents=[common])
    p.add_argument("--stage", required=True, choices=STAGES)
    sub.add_parser("compare-labels", parents=[common])
    p = sub.add_parser("run", parents=[common])
    p.add_argument("--engines", default="rules,kev,haiku")
    p.add_argument("--passes", type=int, default=2)
    p.add_argument("--budget-usd", type=float, default=0.05)
    p.add_argument("--dry-run", action="store_true")
    p.add_argument("--limit", type=int, default=0)
    p.add_argument("--run-id", default=None)
    p = sub.add_parser("score", parents=[common])
    p.add_argument("--run", required=True)
    sub.add_parser("selftest", parents=[common])
    return ap


def main(argv=None):
    args = build_parser().parse_args(argv)
    if args.cmd == "selftest":
        return cmd_selftest(args, None)
    cat = load_catalog()
    fn = {"validate-cases": cmd_validate, "export-blind": cmd_export,
          "compare-labels": cmd_compare, "run": cmd_run, "score": cmd_score}[args.cmd]
    return fn(args, cat)


if __name__ == "__main__":
    sys.exit(main())
