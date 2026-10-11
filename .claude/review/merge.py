#!/usr/bin/env python3
"""Deterministic merge stage for the find-only code audit pipeline.

Usage:
  python3 -I .claude/review/merge.py [--review-dir .claude/review] [--out-dir .claude/reviews]
  python3 -I .claude/review/merge.py --self-test
  python3 -I .claude/review/merge.py --emit-bundle wp1,wp2      (compact findings bundle for Reduce)
  python3 -I .claude/review/merge.py --render-domain <domain_x.json>   (markdown to stdout)

Reads (see INTERFACES.md): coverage_plan.json, packets/*.json, runs/*.json, findings/*.json,
work/classifier_verdicts.json, work/scanner_candidates.json, snapshot.json, optional
work/hunks.jsonl (commit lists only), optional <out-dir>/domain_*.json (Reduce output).
Writes: <out-dir>/master_findings.{json,md}, coverage_report.{json,md}.
Exit 1 when coverage fails (outputs are still written), 0 otherwise.
Stdlib only. No timestamps in outputs, so reruns are byte-identical.
"""
import argparse
import hashlib
import json
import posixpath
import re
import sys
import tempfile
from pathlib import Path

# --- Estimated API-equivalent rates, USD per million tokens (Haiku 5.5). All outputs are estimates.
RATES = {
    "haiku-5.5": {
        "le_100k": {"input": 0.10, "cache_write_5m": 0.125, "cache_read": 0.01, "output": 0.50},
        "gt_100k": {"input": 0.50, "cache_write_5m": 0.625, "cache_read": 0.05, "output": 2.50},
    }
}
LONG_PROMPT_TOKENS = 100_000
RATE_MODEL = "haiku-5.5"

SEVERITIES = ["critical", "high", "medium", "low"]
CONFIDENCES = ["high", "medium", "low"]
SEV_RANK = {s: i for i, s in enumerate(SEVERITIES)}
CONF_RANK = {c: i for i, c in enumerate(CONFIDENCES)}
VERIF_ORDER = {"confirmed": 0, "downgraded": 1, "unverified": 2, "unverified-scanner": 3,
               "known-issue-not-rediscovered": 4, "rejected": 5}
REDUCE_STATUSES = ("confirmed", "downgraded", "rejected")
SLACK = 5


# ----------------------------------------------------------------------------- answer key
def _known_issues():
    """Recall answer key. Lives ONLY here (and in the final ledger outputs)."""
    return [
        {"key": "K1",
         "targets": [("scripts/test_pre_install_gate.py", [(148, 148)]),
                     ("scripts/pre_install_gate.py", None)],
         "categories": {"gate-integrity", "test", "build", "other"},
         "text": "py:x fake-runner fixture string in test_pre_install_gate.py:148 reaches the "
                 "production ledger via pre_install_gate.py BUILD_GATE logging: test-fixture / "
                 "ledger pollution, not a build failure"},
        {"key": "K2",
         "targets": [("patch_manager_mods/action_hotbar_src/HotbarLogic.h", [(89, 90)])],
         "categories": {"logic", "bug", "other"},
         "text": "wide isExile predicate (sentinel range 0xFFFFFFF0..0xFFFFFFFE or partyCount==1 "
                 "or isMember0 or partySlot==0): boundary / mis-keying risk"},
        {"key": "K3a",
         "targets": [("patch_manager_mods/npc_diversity_src/manifest.toml", [(7, 7)])],
         "categories": {"gate-integrity", "build", "other"},
         "text": "kind changed mod->patch in 83354c0 'Fix gate failures': allowlist-style edit "
                 "to review"},
        {"key": "K3b",
         "targets": [("tests/static/subclassers.toml", None)],
         "categories": {"gate-integrity", "build", "other"},
         "text": "dirty allowlist edit (hint line numbers bumped, new GuiInputProvider entry): "
                 "allowlist-style edit to review"},
        {"key": "K4",
         "targets": [("patch_manager_mods/action_hotbar_src/hotbar_logic_test.cpp",
                      [(400, 400), (549, 549), (387, 408), (558, 563)])],
         "categories": {"test"},
         "text": "normalizeSaveFolder assertions removed; check replacement in "
                 "se_registry_test.cpp; not a defect if replaced"},
    ]


# ----------------------------------------------------------------------------- helpers
def normpath(p):
    p = str(p or "").strip().replace("\\", "/")
    p = posixpath.normpath(p) if p else ""
    while p.startswith("./"):
        p = p[2:]
    return "" if p == "." else p


def parse_lines(v):
    """'a-b' or 'a' (tolerant) -> (start, end); (0, 0) when unparseable."""
    if isinstance(v, bool):
        return (0, 0)
    if isinstance(v, int):
        return (v, v)
    nums = [int(x) for x in re.findall(r"\d+", str(v or ""))]
    if not nums:
        return (0, 0)
    return (min(nums), max(nums))


def fmt_lines(a, b):
    if a == 0 and b == 0:
        return ""
    return str(a) if a == b else f"{a}-{b}"


def norm_evidence(s):
    return " ".join(str(s or "").lower().split())


def sha_hex(s):
    return hashlib.sha256(s.encode("utf-8")).hexdigest()


def stable_id(file, start, end, category, evidence):
    return sha_hex(f"{normpath(file)}|{start}-{end}|{category}|{norm_evidence(evidence)[:80]}")[:12]


def cluster_tag(member_ids):
    return "C-" + sha_hex("|".join(sorted(member_ids)))[:8]


def rj(path, warn=None):
    """Read JSON; None when missing or invalid (invalid recorded in warn)."""
    try:
        with open(path, encoding="utf-8") as fh:
            return json.load(fh)
    except FileNotFoundError:
        return None
    except (OSError, ValueError) as exc:
        if warn is not None:
            warn.append(f"unreadable JSON {path}: {exc}")
        return None


def dumps(obj):
    return json.dumps(obj, sort_keys=True, indent=1, ensure_ascii=False) + "\n"


def esc(s):
    return str(s if s is not None else "").replace("|", "\\|").replace("\n", " ").replace("\r", " ")


def sev_of(v, default="low"):
    return v if v in SEV_RANK else default


def conf_of(v, default="low"):
    return v if v in CONF_RANK else default


def sha_file(path):
    try:
        return hashlib.sha256(Path(path).read_bytes()).hexdigest()
    except OSError:
        return None


# ----------------------------------------------------------------------------- coverage
def load_dir_json(d, warn, skip_prefix=None):
    out = {}
    if not d.is_dir():
        return out
    for p in sorted(d.glob("*.json")):
        if skip_prefix and p.stem.startswith(skip_prefix):
            continue
        data = rj(p, warn)
        if isinstance(data, dict):
            out[p.stem] = data
    return out


def run_problem(run):
    """None when the run is COMPLETE, else a reason string."""
    if run is None:
        return "missing run"
    if run.get("status") != "ok":
        return f"run status {run.get('status')!r}"
    if run.get("schema_valid") is not True:
        return "invalid (schema_valid not true)"
    if run.get("truncated") is not False:
        return "truncated"
    return None


def compute_coverage(rd, warn):
    errors = []
    plan = rj(rd / "coverage_plan.json", warn)
    units = []
    if isinstance(plan, dict) and isinstance(plan.get("units"), list):
        units = plan["units"]
    else:
        errors.append("coverage_plan.json missing or has no units list")
    packets = load_dir_json(rd / "packets", warn)
    runs = load_dir_json(rd / "runs", warn, skip_prefix="reduce_")
    findings_files = load_dir_json(rd / "findings", warn)
    cv_data = rj(rd / "work" / "classifier_verdicts.json", warn)
    passed_hunks = set()
    if isinstance(cv_data, dict):
        for v in cv_data.get("verdicts") or []:
            if isinstance(v, dict) and v.get("pass") is True and v.get("hunk_id"):
                passed_hunks.add(v["hunk_id"])
    elif cv_data is None:
        warn.append("work/classifier_verdicts.json missing: every mechanical unit becomes a gap")

    hunk_packets = {}
    for pid, pk in packets.items():
        for h in pk.get("hunks") or []:
            hunk_packets.setdefault(h, set()).add(pk.get("packet_id") or pid)

    def packet_run_key(pid):
        return pid

    verdicts = {}   # hunk_id -> list of (verdict, reason)
    unit_info = {}
    seen = set()
    dup_ids = []
    for u in units:
        hid = u.get("hunk_id") if isinstance(u, dict) else None
        if not hid:
            errors.append("plan unit without hunk_id")
            continue
        if hid in seen:
            dup_ids.append(hid)
            errors.append(f"duplicate hunk_id in plan: {hid}")
            continue
        seen.add(hid)
        unit_info[hid] = u
        planned = u.get("planned")
        reason = u.get("reason") or ""
        out = verdicts.setdefault(hid, [])
        if planned == "review":
            pids = sorted(set(u.get("packets") or []) | hunk_packets.get(hid, set()))
            if not pids:
                out.append(("gap", "no packet lists this unit"))
                continue
            probs = []
            for pid in pids:
                if pid not in packets:
                    probs.append(f"{pid}: packet file missing")
                    continue
                prob = run_problem(runs.get(packet_run_key(pid)))
                if prob:
                    probs.append(f"{pid}: {prob}")
            out.append(("gap", "; ".join(probs)) if probs else ("reviewed", ""))
        elif planned == "mechanical":
            if hid in passed_hunks:
                out.append(("mechanical", reason))
            else:
                out.append(("gap", "no passing classifier verdict"))
        elif planned in ("skipped", "excluded"):
            out.append((planned, reason))
        else:
            out.append(("gap", f"unknown planned value {planned!r}"))

    # integrity: exactly one verdict per unit
    for hid in seen:
        n = len(verdicts.get(hid, []))
        if n != 1:
            errors.append(f"unit {hid} has {n} verdicts (expected exactly 1)")

    counts = {k: 0 for k in ("reviewed", "mechanical", "skipped", "excluded", "gap")}
    per_file = {}
    unit_rows = []
    gaps = []
    for hid in sorted(seen):
        u = unit_info[hid]
        v, reason = verdicts[hid][0] if verdicts[hid] else ("gap", "no verdict")
        counts[v] += 1
        f = normpath(u.get("file"))
        pf = per_file.setdefault(f, {k: 0 for k in counts})
        pf[v] += 1
        row = {"hunk_id": hid, "file": f, "range": u.get("range"), "planned": u.get("planned"),
               "verdict": v, "reason": reason}
        unit_rows.append(row)
        if v == "gap":
            gaps.append({"hunk_id": hid, "file": f, "reason": reason})
    unit_rows.sort(key=lambda r: (r["file"], r["hunk_id"]))

    # packets to re-queue
    all_pids = set(packets) | {p for u in units if isinstance(u, dict) for p in (u.get("packets") or [])}
    requeue = []
    for pid in sorted(all_pids):
        prob = run_problem(runs.get(pid)) if pid in packets else "packet file missing"
        if prob:
            requeue.append({"packet_id": pid, "reason": prob})

    # cross-check only: packet files vs worker files_examined
    fe_warnings = []
    for pid in sorted(packets):
        if run_problem(runs.get(pid)):
            continue
        ff = findings_files.get(pid)
        if ff is None:
            fe_warnings.append({"packet_id": pid, "problem": "findings file missing for complete run",
                                "missing_files": []})
            continue
        fe = ff.get("files_examined")
        if not isinstance(fe, list):
            fe_warnings.append({"packet_id": pid, "problem": "files_examined absent", "missing_files": []})
            continue
        examined = set()
        for item in fe:
            if isinstance(item, dict):
                item = item.get("file") or item.get("path")
            if item:
                examined.add(normpath(item))
        missing = sorted({normpath(x) for x in packets[pid].get("files") or []} - examined)
        if missing:
            fe_warnings.append({"packet_id": pid, "problem": "files_examined misses packet files",
                                "missing_files": missing})

    snap_sha = sha_file(rd / "snapshot.json")
    plan_snap = plan.get("snapshot") if isinstance(plan, dict) else None
    if snap_sha and plan_snap and plan_snap != snap_sha:
        warn.append("coverage_plan.json snapshot differs from sha256(snapshot.json)")

    return {
        "passed": not errors and counts["gap"] == 0,
        "errors": sorted(set(errors)),
        "duplicate_hunk_ids": sorted(set(dup_ids)),
        "unit_count": len(seen),
        "verdict_counts": counts,
        "gaps": gaps,
        "per_file": {k: per_file[k] for k in sorted(per_file)},
        "requeue": requeue,
        "files_examined_warnings": fe_warnings,
        "snapshot_sha256": snap_sha,
        "units": unit_rows,
    }


# ----------------------------------------------------------------------------- findings
def load_worker_findings(rd, warn):
    """Return dict stable_id -> record (deduped across packets)."""
    recs = {}
    fdir = rd / "findings"
    if not fdir.is_dir():
        return recs
    packets = load_dir_json(rd / "packets", [])
    for p in sorted(fdir.glob("*.json")):
        data = rj(p, warn)
        if not isinstance(data, dict):
            continue
        pid = data.get("packet_id") or p.stem
        domain = (packets.get(pid) or {}).get("domain") or data.get("domain")
        for i, f in enumerate(data.get("findings") or []):
            if not isinstance(f, dict):
                continue
            files = f.get("files") if isinstance(f.get("files"), list) else [f.get("file")]
            files = [normpath(x) for x in files if x]
            if not files:
                warn.append(f"finding without file in {p.name} index {i}")
                continue
            start, end = parse_lines(f.get("lines"))
            category = f.get("category") or "other"
            evidence = str(f.get("evidence") or "")
            fid = stable_id(files[0], start, end, category, evidence)
            local_id = str(f.get("local_id") or f.get("id") or f"#{i}")
            sev = sev_of(f.get("severity"))
            conf = conf_of(f.get("confidence"))
            cand = f.get("candidate_id")
            commits = [str(c) for c in (f.get("commits") or [])]
            if fid in recs:
                r = recs[fid]
                if pid not in r["packet_ids"]:
                    r["packet_ids"].append(pid)
                r["refs"].append((pid, local_id))
                if cand and cand not in r["candidate_ids"]:
                    r["candidate_ids"].append(cand)
                for c in commits:
                    if c not in r["commits"]:
                        r["commits"].append(c)
                if SEV_RANK[sev] < SEV_RANK[r["severity"]]:
                    r["severity"] = sev
                continue
            recs[fid] = {
                "id": fid, "files": files, "file": files[0], "start": start, "end": end,
                "lines": fmt_lines(start, end), "category": category, "severity": sev,
                "confidence": conf, "verification": "unverified", "evidence": evidence,
                "suggested_investigation": str(f.get("suggested_investigation")
                                               or f.get("investigation") or ""),
                "cluster": "", "source": "worker", "packet_ids": [pid], "commits": commits,
                "refs": [(pid, local_id)], "candidate_ids": [cand] if cand else [],
                "domain": domain, "group": "worker",
            }
    return recs


def load_hunk_commits(rd):
    out = {}
    p = rd / "work" / "hunks.jsonl"
    if not p.is_file():
        return out
    try:
        with open(p, encoding="utf-8") as fh:
            for line in fh:
                line = line.strip()
                if not line:
                    continue
                try:
                    h = json.loads(line)
                except ValueError:
                    continue
                if not isinstance(h, dict) or h.get("class") == "excluded":
                    continue
                f = normpath(h.get("file"))
                s = out.setdefault(f, [])
                for c in h.get("commits") or []:
                    if c not in s:
                        s.append(c)
    except OSError:
        pass
    return {k: sorted(v) for k, v in out.items()}


def add_scanner_candidates(recs, rd, warn):
    data = rj(rd / "work" / "scanner_candidates.json", warn)
    if not isinstance(data, dict):
        return 0
    confirmed = {c for r in recs.values() for c in r["candidate_ids"]}
    added = 0
    for c in data.get("candidates") or []:
        if not isinstance(c, dict) or not c.get("candidate_id") or c["candidate_id"] in confirmed:
            continue
        f = normpath(c.get("file"))
        start, end = parse_lines(c.get("lines"))
        detail = str(c.get("detail") or "")
        evidence = f"[scanner {c.get('scanner')}] {detail}"
        fid = stable_id(f, start, end, "other", "scanner|" + c["candidate_id"] + "|" + evidence)
        if fid in recs:
            continue
        recs[fid] = {
            "id": fid, "files": [f], "file": f, "start": start, "end": end,
            "lines": fmt_lines(start, end), "category": "other", "severity": "low",
            "confidence": "low", "verification": "unverified-scanner", "evidence": evidence,
            "suggested_investigation": "Confirm or dismiss this scanner candidate by reading the "
                                       "cited lines; no worker reported it.",
            "cluster": "", "source": "scanner", "packet_ids": [], "commits": [],
            "refs": [], "candidate_ids": [c["candidate_id"]], "domain": None, "group": "scanner",
        }
        added += 1
    return added


def apply_reduce(recs, out_dir, warn):
    """Apply domain_*.json verdicts and cross-file findings. Returns list of files used."""
    used = []
    if not out_dir.is_dir():
        return used
    ref_map = {}
    for r in recs.values():
        for ref in r["refs"]:
            ref_map[ref] = r["id"]
    for path in sorted(out_dir.glob("domain_*.json")):
        d = rj(path, warn)
        if not isinstance(d, dict):
            continue
        used.append(path.name)
        for v in d.get("verdicts") or []:
            if not isinstance(v, dict):
                continue
            fid = v.get("finding_id") or ref_map.get((v.get("packet_id"), v.get("local_id")))
            r = recs.get(fid)
            if r is None:
                warn.append(f"{path.name}: verdict for unknown finding {fid or (v.get('packet_id'), v.get('local_id'))}")
                continue
            if v.get("status") not in REDUCE_STATUSES:
                continue
            r["verification"] = v["status"]
            if v.get("severity") in SEV_RANK:
                r["severity"] = v["severity"]
            if v.get("confidence") in CONF_RANK:
                r["confidence"] = v["confidence"]
            r["reduce_reason"] = str(v.get("reason") or "")
            r["reduce_cited_lines_checked"] = str(v.get("cited_lines_checked") or "")
        for c in d.get("cross_file") or []:
            if not isinstance(c, dict):
                continue
            files = [normpath(x) for x in (c.get("files") or []) if x]
            if not files:
                continue
            start, end = parse_lines(c.get("lines"))
            category = c.get("category") or "other"
            evidence = str(c.get("evidence") or "")
            fid = stable_id(files[0], start, end, category, evidence)
            if fid in recs:
                continue
            why = str(c.get("why_it_matters") or "")
            recs[fid] = {
                "id": fid, "files": files, "file": files[0], "start": start, "end": end,
                "lines": fmt_lines(start, end), "category": category,
                "severity": sev_of(c.get("severity")), "confidence": conf_of(c.get("confidence")),
                "verification": "unverified",
                "evidence": evidence + (f" Why it matters: {why}" if why else ""),
                "suggested_investigation": str(c.get("suggested_investigation") or ""),
                "cluster": "", "source": "reduce", "packet_ids": [], "commits": [],
                "refs": [], "candidate_ids": [], "domain": d.get("domain"), "group": "worker",
            }
        for cl in d.get("clusters") or []:
            if isinstance(cl, dict):
                for fid in cl.get("finding_ids") or []:
                    if fid in recs:
                        recs[fid]["root_cause"] = str(cl.get("root_cause") or "")
    return used


def assign_clusters(recs):
    """Same file + category (+group) with overlapping line ranges, transitively."""
    groups = {}
    for r in recs.values():
        groups.setdefault((r["file"], r["category"], r["group"]), []).append(r)
    sizes = {}
    for key in sorted(groups):
        items = sorted(groups[key], key=lambda r: (r["start"], r["end"], r["id"]))
        parent = {r["id"]: r["id"] for r in items}

        def find(x):
            while parent[x] != x:
                parent[x] = parent[parent[x]]
                x = parent[x]
            return x

        cur_end = None
        cur_root = None
        for r in items:
            if cur_end is not None and r["start"] <= cur_end:
                parent[find(r["id"])] = find(cur_root)
                cur_end = max(cur_end, r["end"])
            else:
                cur_root, cur_end = r["id"], r["end"]
        members = {}
        for r in items:
            members.setdefault(find(r["id"]), []).append(r["id"])
        for ids in members.values():
            tag = cluster_tag(ids)
            for i in ids:
                recs[i]["cluster"] = tag
                sizes[i] = len(ids)
    return sizes


# ----------------------------------------------------------------------------- recall
def _overlaps(ranges, start, end):
    if ranges is None:
        return True
    if start == 0 and end == 0:
        return False
    return any(start <= b + SLACK and end >= a - SLACK for a, b in ranges)


def compute_recall(recs):
    rows = []
    for ki in _known_issues():
        matched = []
        for r in sorted(recs.values(), key=lambda r: r["id"]):
            if r["source"] == "scanner" or r["category"] not in ki["categories"]:
                continue
            for kf, ranges in ki["targets"]:
                if kf in r["files"] and _overlaps(ranges, r["start"], r["end"]):
                    matched.append(r)
                    break
        rows.append({
            "key": ki["key"],
            "files": [t[0] for t in ki["targets"]],
            "text": ki["text"],
            "hit": bool(matched),
            "matched_ids": [m["id"] for m in matched],
            "matched_verification": sorted({m["verification"] for m in matched}),
        })
    return rows


def add_known_issue_misses(recs, recall):
    ki_by_key = {k["key"]: k for k in _known_issues()}
    for row in recall:
        ki = ki_by_key[row["key"]]
        if row["hit"]:
            for mid in row["matched_ids"]:
                recs[mid].setdefault("known_issues", []).append(row["key"])
            continue
        f = ki["targets"][0][0]
        ranges = ki["targets"][0][1]
        if ranges:
            start, end = min(a for a, _ in ranges), max(b for _, b in ranges)
        else:
            start, end = 0, 0
        fid = stable_id(f, start, end, "known-issue", ki["key"] + ki["text"])
        recs[fid] = {
            "id": fid, "files": [t[0] for t in ki["targets"]], "file": f, "start": start,
            "end": end, "lines": fmt_lines(start, end) or "whole-file", "category": "other",
            "severity": "medium", "confidence": "low",
            "verification": "known-issue-not-rediscovered",
            "evidence": f"[{ki['key']}] {ki['text']}",
            "suggested_investigation": "Recall gap: no worker finding covered this item. "
                                       "Inspect the cited location manually and review why the "
                                       "audit missed it.",
            "cluster": "", "source": "known-issue", "packet_ids": [], "commits": [],
            "refs": [], "candidate_ids": [], "domain": None, "group": "known-issue",
        }


# ----------------------------------------------------------------------------- cost
def _int(v):
    try:
        return int(v or 0)
    except (TypeError, ValueError):
        return 0


def estimate_call(usage):
    inp = _int(usage.get("input_tokens"))
    cw = _int(usage.get("cache_creation_input_tokens"))
    cr = _int(usage.get("cache_read_input_tokens"))
    out = _int(usage.get("output_tokens"))
    prompt = inp + cw + cr
    tier = "gt_100k" if prompt > LONG_PROMPT_TOKENS else "le_100k"
    r = RATES[RATE_MODEL][tier]
    usd = (inp * r["input"] + cw * r["cache_write_5m"] + cr * r["cache_read"]
           + out * r["output"]) / 1_000_000
    return prompt, usd, (inp, cw, cr, out)


def compute_cost(rd, warn):
    cost = {
        "label": "estimate", "rate_model": RATE_MODEL, "rates_usd_per_mtok": RATES[RATE_MODEL],
        "calls": 0, "calls_with_usage": 0, "attempts_total": 0,
        "input_tokens": 0, "cache_creation_input_tokens": 0, "cache_read_input_tokens": 0,
        "output_tokens": 0, "calls_prompt_over_100k_upper_bound": 0,
        "estimated_api_equivalent_usd": 0.0, "cli_reported_total_cost_usd": 0.0,
        "calls_with_cli_cost": 0,
        "reduce_calls": 0, "reduce_cli_reported_total_cost_usd": 0.0,
        "note": "Runs recorded from stream-json are priced per API request (exact >100K count in "
                "requests_over_100k_exact); runs with only aggregate usage fall back to an upper-bound proxy. Spend is an estimate at Haiku 5.5 list rates; Reduce "
                "(Sonnet) calls are not rate-estimated, only their CLI-reported cost is summed.",
    }
    rdir = rd / "runs"
    if not rdir.is_dir():
        return cost
    for p in sorted(rdir.glob("*.json")):
        run = rj(p, warn)
        if not isinstance(run, dict):
            continue
        if p.stem.startswith("reduce_"):
            cost["reduce_calls"] += 1
            c = run.get("total_cost_usd")
            if isinstance(c, (int, float)):
                cost["reduce_cli_reported_total_cost_usd"] += c
            continue
        cost["calls"] += 1
        cost["attempts_total"] += _int(run.get("attempts"))
        c = run.get("cost_usd")
        if isinstance(c, (int, float)):
            cost["cli_reported_total_cost_usd"] += c
            cost["calls_with_cli_cost"] += 1
        turns = run.get("turn_usage")
        if isinstance(turns, list) and turns:           # stream-json: exact per-request pricing
            cost["calls_with_usage"] += 1
            cost["calls_priced_per_request"] = cost.get("calls_priced_per_request", 0) + 1
            any_long = False
            for t in turns:
                prompt, usd, (inp, cw, cr, out) = estimate_call(t)
                cost["input_tokens"] += inp
                cost["cache_creation_input_tokens"] += cw
                cost["cache_read_input_tokens"] += cr
                cost["output_tokens"] += out
                cost["estimated_api_equivalent_usd"] += usd
                if prompt > LONG_PROMPT_TOKENS:
                    cost["requests_over_100k_exact"] = cost.get("requests_over_100k_exact", 0) + 1
                    any_long = True
            if any_long:
                cost["calls_prompt_over_100k_upper_bound"] += 1
            continue
        usage = run.get("usage")
        if not isinstance(usage, dict) or not usage:
            continue
        cost["calls_with_usage"] += 1
        prompt, usd, (inp, cw, cr, out) = estimate_call(usage)
        cost["input_tokens"] += inp
        cost["cache_creation_input_tokens"] += cw
        cost["cache_read_input_tokens"] += cr
        cost["output_tokens"] += out
        cost["estimated_api_equivalent_usd"] += usd
        if prompt > LONG_PROMPT_TOKENS:
            cost["calls_prompt_over_100k_upper_bound"] += 1
    for k in ("estimated_api_equivalent_usd", "cli_reported_total_cost_usd",
              "reduce_cli_reported_total_cost_usd"):
        cost[k] = round(cost[k], 6)
    return cost


# ----------------------------------------------------------------------------- ledger
def ledger_entry(r, cluster_size):
    e = {
        "id": r["id"], "files": r["files"], "lines": r["lines"], "category": r["category"],
        "severity": r["severity"], "confidence": r["confidence"],
        "verification": r["verification"], "evidence": r["evidence"],
        "suggested_investigation": r["suggested_investigation"], "cluster": r["cluster"],
        "cluster_size": cluster_size, "source": r["source"],
        "packet_ids": sorted(r["packet_ids"]), "commits": sorted(set(r["commits"])),
    }
    for k in ("reduce_reason", "reduce_cited_lines_checked", "root_cause"):
        if r.get(k):
            e[k] = r[k]
    if r.get("known_issues"):
        e["known_issues"] = sorted(set(r["known_issues"]))
    if r.get("candidate_ids"):
        e["candidate_ids"] = sorted(r["candidate_ids"])
    return e


def rank_key(e):
    return (SEV_RANK[e["severity"]], VERIF_ORDER.get(e["verification"], 9),
            CONF_RANK[e["confidence"]], -e["cluster_size"], e["id"])


def build_all(rd, out_dir):
    warn = []
    coverage = compute_coverage(rd, warn)
    recs = load_worker_findings(rd, warn)
    scanner_added = add_scanner_candidates(recs, rd, warn)
    reduce_files = apply_reduce(recs, out_dir, warn)
    recall = compute_recall(recs)
    add_known_issue_misses(recs, recall)
    commits = load_hunk_commits(rd)
    for r in recs.values():
        if not r["commits"]:
            cs = set()
            for f in r["files"]:
                cs.update(commits.get(f, []))
            r["commits"] = sorted(cs)
    sizes = assign_clusters(recs)
    cluster_sizes = {}
    for r in recs.values():
        cluster_sizes[r["cluster"]] = cluster_sizes.get(r["cluster"], 0) + 1
    entries = [ledger_entry(r, cluster_sizes[r["cluster"]]) for r in recs.values()]
    entries.sort(key=lambda e: (e["files"][0], parse_lines(e["lines"])[0], e["category"], e["id"]))
    cost = compute_cost(rd, warn)
    human_only = sorted({(u["file"], u["reason"]) for u in coverage["units"]
                         if u["planned"] == "excluded"})
    human_only = [{"file": f, "reason": r} for f, r in human_only]
    return {
        "coverage": coverage, "entries": entries, "recall": recall, "cost": cost,
        "human_only": human_only, "warnings": sorted(set(warn)), "reduce_files": reduce_files,
        "scanner_unconfirmed": scanner_added,
    }


# ----------------------------------------------------------------------------- rendering
def render_coverage_md(cov, warnings):
    c = cov["verdict_counts"]
    L = ["# Coverage report", "",
         f"Result: **{'PASS' if cov['passed'] else 'FAIL'}**  (units: {cov['unit_count']}, "
         f"gaps: {c['gap']}, errors: {len(cov['errors'])})", "",
         "| verdict | units |", "|---|---|"]
    for k in ("reviewed", "mechanical", "skipped", "excluded", "gap"):
        L.append(f"| {k} | {c[k]} |")
    if cov["errors"]:
        L += ["", "## Errors", ""] + [f"- {esc(e)}" for e in cov["errors"]]
    if cov["gaps"]:
        L += ["", "## Gaps", "", "| hunk | file | reason |", "|---|---|---|"]
        L += [f"| {g['hunk_id']} | {esc(g['file'])} | {esc(g['reason'])} |" for g in cov["gaps"]]
    if cov["requeue"]:
        L += ["", "## Packets to re-queue", "", "| packet | reason |", "|---|---|"]
        L += [f"| {r['packet_id']} | {esc(r['reason'])} |" for r in cov["requeue"]]
    if cov["files_examined_warnings"]:
        L += ["", "## Warnings: worker files_examined cross-check (not a failure)", "",
              "| packet | problem | missing files |", "|---|---|---|"]
        L += [f"| {w['packet_id']} | {esc(w['problem'])} | {esc(', '.join(w['missing_files']))} |"
              for w in cov["files_examined_warnings"]]
    if warnings:
        L += ["", "## Other warnings", ""] + [f"- {esc(w)}" for w in warnings]
    L += ["", "## Per-file roll-up", "",
          "| file | reviewed | mechanical | skipped | excluded | gap |", "|---|---|---|---|---|---|"]
    for f, pf in cov["per_file"].items():
        L.append(f"| {esc(f)} | {pf['reviewed']} | {pf['mechanical']} | {pf['skipped']} | "
                 f"{pf['excluded']} | {pf['gap']} |")
    return "\n".join(L) + "\n"


def render_cost_md(cost):
    return [
        "## Cost (estimate)", "",
        f"All figures are estimates. {cost['note']}", "",
        f"- Worker calls: {cost['calls']} (with usage: {cost['calls_with_usage']}, "
        f"attempts total: {cost['attempts_total']})",
        f"- Tokens (estimate): input {cost['input_tokens']:,}, cache creation "
        f"{cost['cache_creation_input_tokens']:,}, cache read {cost['cache_read_input_tokens']:,}, "
        f"output {cost['output_tokens']:,}",
        f"- Calls with prompt > 100K tokens (upper-bound proxy): "
        f"{cost['calls_prompt_over_100k_upper_bound']}",
        f"- Estimated API-equivalent spend ({cost['rate_model']} rates): "
        f"${cost['estimated_api_equivalent_usd']:.4f} (estimate)",
        f"- CLI-reported total_cost_usd, worker runs: ${cost['cli_reported_total_cost_usd']:.4f} "
        f"over {cost['calls_with_cli_cost']} calls (estimate)",
        f"- Reduce sessions: {cost['reduce_calls']}, CLI-reported "
        f"${cost['reduce_cli_reported_total_cost_usd']:.4f} (estimate)",
    ]


def render_master_md(res, out_name_cov="coverage_report.md"):
    cov, entries, recall, cost = res["coverage"], res["entries"], res["recall"], res["cost"]
    c = cov["verdict_counts"]
    L = ["# Master findings ledger", "",
         f"Coverage: **{'PASS' if cov['passed'] else 'FAIL'}**; findings in ledger: {len(entries)}; "
         f"reduce files applied: {len(res['reduce_files'])}.", ""]
    top = [e for e in entries if e["verification"] not in ("rejected", "known-issue-not-rediscovered")]
    top.sort(key=rank_key)
    L += ["## Top risks", ""]
    if top:
        L += ["| # | id | sev | verification | conf | cluster | file:lines | category | evidence |",
              "|---|---|---|---|---|---|---|---|---|"]
        for i, e in enumerate(top[:15], 1):
            L.append(f"| {i} | {e['id']} | {e['severity']} | {e['verification']} | {e['confidence']} | "
                     f"{e['cluster']} | {esc(e['files'][0])}:{esc(e['lines'])} | {e['category']} | "
                     f"{esc(e['evidence'][:160])} |")
    else:
        L.append("(none)")
    L += ["", "## Coverage summary", "",
          f"- units: {cov['unit_count']}; reviewed {c['reviewed']}, mechanical {c['mechanical']}, "
          f"skipped {c['skipped']}, excluded {c['excluded']}, gap {c['gap']}",
          f"- packets to re-queue: {len(cov['requeue'])}"
          + (": " + ", ".join(r["packet_id"] for r in cov["requeue"][:30]) if cov["requeue"] else ""),
          f"- errors: {len(cov['errors'])}; details in {out_name_cov}", "",
          "## Known-issue recall", "",
          "| key | result | matched ids | verification | files |", "|---|---|---|---|---|"]
    for r in recall:
        L.append(f"| {r['key']} | {'HIT' if r['hit'] else 'MISS (recall gap)'} | "
                 f"{esc(', '.join(r['matched_ids']))} | {esc(', '.join(r['matched_verification']))} | "
                 f"{esc(', '.join(r['files']))} |")
    hits = sum(1 for r in recall if r["hit"])
    L += ["", f"Recall: {hits}/{len(recall)}.", "", "## Human-only (excluded, not read)", ""]
    if res["human_only"]:
        L += ["| file | reason |", "|---|---|"]
        L += [f"| {esc(h['file'])} | {esc(h['reason'])} |" for h in res["human_only"]]
    else:
        L.append("(none)")
    L += [""] + render_cost_md(cost) + ["", "## All findings by cluster", ""]
    clusters = {}
    for e in entries:
        clusters.setdefault(e["cluster"], []).append(e)
    order = sorted(clusters, key=lambda t: (min(SEV_RANK[e["severity"]] for e in clusters[t]), t))
    for tag in order:
        es = sorted(clusters[tag], key=rank_key)
        L += [f"### {tag} ({len(es)} finding{'s' if len(es) != 1 else ''}, {esc(es[0]['files'][0])})", "",
              "| id | sev | verification | conf | lines | category | source | packets | commits | evidence | investigation |",
              "|---|---|---|---|---|---|---|---|---|---|---|"]
        for e in es:
            L.append(f"| {e['id']} | {e['severity']} | {e['verification']} | {e['confidence']} | "
                     f"{esc(e['lines'])} | {e['category']} | {e['source']} | "
                     f"{esc(', '.join(e['packet_ids']))} | {esc(', '.join(e['commits']))} | "
                     f"{esc(e['evidence'][:300])} | {esc(e['suggested_investigation'][:300])} |")
        rc = next((e.get("root_cause") for e in es if e.get("root_cause")), None)
        if rc:
            L += ["", f"Root cause (Reduce): {rc}"]
        L.append("")
    return "\n".join(L).rstrip() + "\n"


def run_merge(rd, out_dir):
    rd, out_dir = Path(rd), Path(out_dir)
    res = build_all(rd, out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    cov = res["coverage"]
    master = {
        "generated_by": "merge.py", "coverage_passed": cov["passed"],
        "snapshot_sha256": cov["snapshot_sha256"], "findings": res["entries"],
        "recall": res["recall"], "human_only": res["human_only"], "cost": res["cost"],
        "reduce_files": res["reduce_files"], "warnings": res["warnings"],
    }
    (out_dir / "master_findings.json").write_text(dumps(master), encoding="utf-8")
    (out_dir / "master_findings.md").write_text(render_master_md(res), encoding="utf-8")
    (out_dir / "coverage_report.json").write_text(dumps(cov), encoding="utf-8")
    (out_dir / "coverage_report.md").write_text(render_coverage_md(cov, res["warnings"]),
                                                encoding="utf-8")
    c = cov["verdict_counts"]
    hits = sum(1 for r in res["recall"] if r["hit"])
    print(f"coverage {'PASS' if cov['passed'] else 'FAIL'}: units={cov['unit_count']} "
          f"reviewed={c['reviewed']} mechanical={c['mechanical']} skipped={c['skipped']} "
          f"excluded={c['excluded']} gap={c['gap']} errors={len(cov['errors'])} "
          f"requeue={len(cov['requeue'])}")
    print(f"findings={len(res['entries'])} recall={hits}/{len(res['recall'])} -> {out_dir}")
    return 0 if cov["passed"] else 1


# ----------------------------------------------------------------------------- reduce bundle
def emit_bundle(rd, wps):
    warn = []
    wanted = {w.strip() for w in wps.split(",") if w.strip()}
    recs = load_worker_findings(Path(rd), warn)
    items = []
    for r in recs.values():
        if r["domain"] not in wanted:
            continue
        pid, lid = r["refs"][0]
        items.append({
            "id": r["id"], "file": r["file"], "lines": r["lines"], "category": r["category"],
            "severity": r["severity"], "confidence": r["confidence"],
            "evidence": r["evidence"][:400],
            "candidate_id": r["candidate_ids"][0] if r["candidate_ids"] else None,
            "packet_id": pid, "local_id": lid,
        })
    items.sort(key=lambda x: (x["file"], parse_lines(x["lines"])[0], x["category"], x["id"]))
    bundle = {"domains": sorted(wanted), "count": len(items), "findings": items}
    sys.stdout.write(json.dumps(bundle, sort_keys=True, separators=(",", ":"), ensure_ascii=False) + "\n")
    for w in sorted(set(warn)):
        print(f"warning: {w}", file=sys.stderr)
    return 0


def render_domain(path):
    d = rj(path)
    if not isinstance(d, dict):
        print(f"cannot read {path}", file=sys.stderr)
        return 1
    vs = d.get("verdicts") or []
    counts = {}
    for v in vs:
        counts[v.get("status")] = counts.get(v.get("status"), 0) + 1
    L = [f"# Reduce: {d.get('domain')}", "",
         "Verdicts: " + (", ".join(f"{k} {counts[k]}" for k in sorted(counts, key=str)) or "none"),
         f"Cross-file findings: {len(d.get('cross_file') or [])}", ""]
    if vs:
        L += ["## Verdicts", "", "| finding | status | sev | conf | reason |", "|---|---|---|---|---|"]
        for v in vs:
            ref = v.get("finding_id") or f"{v.get('packet_id')}/{v.get('local_id')}"
            L.append(f"| {esc(ref)} | {v.get('status')} | {v.get('severity')} | {v.get('confidence')} | "
                     f"{esc(str(v.get('reason'))[:300])} |")
    if d.get("cross_file"):
        L += ["", "## Cross-file", ""]
        for c in d["cross_file"]:
            L.append(f"- [{c.get('severity')}/{c.get('confidence')}] {esc(', '.join(c.get('files') or []))} "
                     f"{esc(c.get('lines'))} ({c.get('category')}): {esc(str(c.get('evidence'))[:300])}")
    if d.get("clusters"):
        L += ["", "## Clusters", ""]
        for c in d["clusters"]:
            L.append(f"- {esc(c.get('tag'))}: {esc(c.get('root_cause'))} ({len(c.get('finding_ids') or [])} findings)")
    if d.get("notes"):
        L += ["", "## Notes", "", str(d["notes"])]
    sys.stdout.write("\n".join(L) + "\n")
    return 0


# ----------------------------------------------------------------------------- self test
def _w(p, obj):
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(json.dumps(obj, indent=1), encoding="utf-8")


def self_test():
    K2F = "patch_manager_mods/action_hotbar_src/HotbarLogic.h"
    with tempfile.TemporaryDirectory(prefix="merge_selftest_") as td:
        rd = Path(td) / "review"
        out = Path(td) / "reviews"

        def unit(n, f, planned, packets, reason="r"):
            return {"hunk_id": f"h-{n:012x}", "file": f, "range": "-1,1 +1,1", "planned": planned,
                    "packets": packets, "reason": reason}

        units = [unit(1, "src/a.cpp", "review", ["wp1-001"]),
                 unit(2, K2F, "review", ["wp1-001"]),
                 unit(3, "src/b.cpp", "review", ["wp1-002"]),
                 unit(4, "src/c.h", "mechanical", [], "whitespace"),
                 unit(5, "docs/x.md", "skipped", [], "class b"),
                 unit(6, "cfg/.env", "excluded", [], "secret pattern")]
        _w(rd / "coverage_plan.json", {"snapshot": "x", "units": units})
        _w(rd / "snapshot.json", {"snapshot": "synthetic"})
        _w(rd / "packets" / "wp1-001.json", {"packet_id": "wp1-001", "domain": "wp1",
           "files": ["src/a.cpp", K2F], "hunks": [units[0]["hunk_id"], units[1]["hunk_id"]]})
        _w(rd / "packets" / "wp1-002.json", {"packet_id": "wp1-002", "domain": "wp1",
           "files": ["src/b.cpp"], "hunks": [units[2]["hunk_id"]]})
        ok_run = {"exit_code": 0, "attempts": 1, "schema_valid": True, "truncated": False,
                  "status": "ok", "cost_usd": 0.01,
                  "usage": {"input_tokens": 1000, "cache_creation_input_tokens": 2000,
                            "cache_read_input_tokens": 150000, "output_tokens": 500}}
        _w(rd / "runs" / "wp1-001.json", dict(ok_run, packet_id="wp1-001", files=["src/a.cpp"]))
        _w(rd / "runs" / "wp1-002.json", dict(ok_run, packet_id="wp1-002", truncated=True))
        _w(rd / "work" / "classifier_verdicts.json", {"verdicts": [
            {"hunk_id": units[3]["hunk_id"], "file": "src/c.h", "rule": "whitespace", "pass": True}]})
        _w(rd / "work" / "scanner_candidates.json", {"candidates": [
            {"candidate_id": "S-aaaaaaaaaa", "scanner": "sentinel-range", "file": K2F,
             "lines": "89-90", "hunk_id": None, "detail": "isExile"},
            {"candidate_id": "S-bbbbbbbbbb", "scanner": "pointer-math", "file": "src/b.cpp",
             "lines": "5-5", "hunk_id": None, "detail": "reinterpret_cast"}]})
        _w(rd / "findings" / "wp1-001.json", {"packet_id": "wp1-001", "files_examined": ["src/a.cpp"],
           "findings": [
               {"local_id": "F1", "file": "src/a.cpp", "lines": "10-20", "category": "logic",
                "severity": "high", "confidence": "medium", "evidence": "Off by one in  loop"},
               {"local_id": "F2", "file": "./src/a.cpp", "lines": "18-30", "category": "logic",
                "severity": "medium", "confidence": "low", "evidence": "same loop bound again"},
               {"local_id": "F3", "file": "src/a.cpp", "lines": "100", "category": "logic",
                "severity": "low", "confidence": "low", "evidence": "unrelated far away"},
               {"local_id": "F4", "file": K2F, "lines": "89-90", "category": "logic",
                "severity": "high", "confidence": "high", "evidence": "isExile too wide",
                "candidate_id": "S-aaaaaaaaaa"}]})

        # 1. truncated packet => coverage fails, h3 is a gap, wp1-002 requeued
        rc = run_merge(rd, out)
        assert rc == 1, "coverage must fail on truncated packet"
        cov = rj(out / "coverage_report.json")
        assert cov["verdict_counts"] == {"reviewed": 2, "mechanical": 1, "skipped": 1, "excluded": 1,
                                         "gap": 1}, cov["verdict_counts"]
        assert [g["hunk_id"] for g in cov["gaps"]] == [units[2]["hunk_id"]]
        assert "truncated" in cov["gaps"][0]["reason"]
        assert [r["packet_id"] for r in cov["requeue"]] == ["wp1-002"]
        assert any(w["packet_id"] == "wp1-001" and K2F in w["missing_files"]
                   for w in cov["files_examined_warnings"]), "files_examined cross-check"
        master = rj(out / "master_findings.json")
        by_ev = {e["evidence"]: e for e in master["findings"]}
        f1, f2, f3 = by_ev["Off by one in  loop"], by_ev["same loop bound again"], by_ev["unrelated far away"]
        assert f1["cluster"] == f2["cluster"] != f3["cluster"], "overlap clustering"
        assert f1["cluster"] == cluster_tag([f1["id"], f2["id"]])
        assert f1["id"] == stable_id("src/a.cpp", 10, 20, "logic", "off by one in loop")
        rec = {r["key"]: r for r in master["recall"]}
        assert rec["K2"]["hit"] and len(rec["K2"]["matched_ids"]) == 1
        assert not rec["K1"]["hit"] and not rec["K4"]["hit"]
        misses = [e for e in master["findings"] if e["verification"] == "known-issue-not-rediscovered"]
        assert len(misses) == 4 and all(e["source"] == "known-issue" for e in misses)
        scanners = [e for e in master["findings"] if e["source"] == "scanner"]
        assert len(scanners) == 1 and scanners[0]["verification"] == "unverified-scanner"
        assert scanners[0]["severity"] == "low" and scanners[0]["confidence"] == "low"
        assert master["cost"]["calls"] == 2 and master["cost"]["calls_prompt_over_100k_upper_bound"] == 2
        assert master["human_only"] == [{"file": "cfg/.env", "reason": "secret pattern"}]
        first = (out / "master_findings.json").read_bytes()

        # 2. fix the truncated packet => pass
        _w(rd / "runs" / "wp1-002.json", dict(ok_run, packet_id="wp1-002", files=["src/b.cpp"]))
        _w(rd / "findings" / "wp1-002.json", {"packet_id": "wp1-002", "files_examined": ["src/b.cpp"],
                                              "findings": []})
        assert run_merge(rd, out) == 0, "coverage must pass once packet is complete"
        assert rj(out / "coverage_report.json")["verdict_counts"]["gap"] == 0
        before = [(out / n).read_bytes() for n in ("master_findings.json", "master_findings.md",
                                                  "coverage_report.json", "coverage_report.md")]
        assert run_merge(rd, out) == 0
        after = [(out / n).read_bytes() for n in ("master_findings.json", "master_findings.md",
                                                 "coverage_report.json", "coverage_report.md")]
        assert before == after, "outputs must be byte-identical across reruns"

        # 3. Reduce rejects the K2 finding: still a recall hit, verification recorded as rejected
        k2id = rec["K2"]["matched_ids"][0]
        _w(out / "domain_wp1.json", {"domain": "wp1", "notes": "", "clusters": [], "cross_file": [
            {"files": ["src/a.cpp", "src/b.cpp"], "lines": "1-2", "category": "other",
             "severity": "medium", "confidence": "low", "evidence": "hook vs state",
             "why_it_matters": "w", "suggested_investigation": "look"}],
            "verdicts": [{"finding_id": None, "packet_id": "wp1-001", "local_id": "F4",
                          "status": "rejected", "severity": "low", "confidence": "high",
                          "reason": "guarded elsewhere", "cited_lines_checked": "89-90"},
                         {"finding_id": f1["id"], "packet_id": None, "local_id": None,
                          "status": "downgraded", "severity": "medium", "confidence": "medium",
                          "reason": "r", "cited_lines_checked": "10-20"}]})
        run_merge(rd, out)
        master = rj(out / "master_findings.json")
        rec = {r["key"]: r for r in master["recall"]}
        assert rec["K2"]["hit"] and rec["K2"]["matched_verification"] == ["rejected"]
        ent = {e["id"]: e for e in master["findings"]}
        assert ent[k2id]["verification"] == "rejected" and ent[f1["id"]]["verification"] == "downgraded"
        assert ent[f1["id"]]["severity"] == "medium"
        assert any(e["source"] == "reduce" for e in master["findings"])
        md = (out / "master_findings.md").read_text(encoding="utf-8")
        assert "Top risks" in md and k2id not in md.split("## Coverage summary")[0]

        # 4. duplicate hunk ids and failing classifier verdicts fail coverage
        _w(rd / "coverage_plan.json", {"snapshot": "x", "units": units + [units[0]]})
        assert run_merge(rd, out) == 1
        assert rj(out / "coverage_report.json")["duplicate_hunk_ids"] == [units[0]["hunk_id"]]
        _w(rd / "coverage_plan.json", {"snapshot": "x", "units": units})
        _w(rd / "work" / "classifier_verdicts.json", {"verdicts": [
            {"hunk_id": units[3]["hunk_id"], "pass": False}]})
        assert run_merge(rd, out) == 1
        assert "no passing classifier verdict" in rj(out / "coverage_report.json")["gaps"][0]["reason"]

        # 5. bundle emission
        import io
        buf = io.StringIO()
        old = sys.stdout
        sys.stdout = buf
        try:
            emit_bundle(rd, "wp1")
        finally:
            sys.stdout = old
        b = json.loads(buf.getvalue())
        assert b["count"] == 4 and all(len(x["evidence"]) <= 400 for x in b["findings"])
        assert {x["local_id"] for x in b["findings"]} == {"F1", "F2", "F3", "F4"}
    print("self-test: OK")
    return 0


# ----------------------------------------------------------------------------- main
def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--review-dir", default=str(Path(__file__).resolve().parent))
    ap.add_argument("--out-dir", default=str(Path(__file__).resolve().parent.parent / "reviews"))
    ap.add_argument("--self-test", action="store_true")
    ap.add_argument("--emit-bundle", metavar="WPS", help="print compact findings bundle for these domains (comma separated)")
    ap.add_argument("--render-domain", metavar="JSON", help="print markdown for a Reduce domain_*.json")
    a = ap.parse_args(argv)
    if a.self_test:
        return self_test()
    if a.emit_bundle:
        return emit_bundle(a.review_dir, a.emit_bundle)
    if a.render_domain:
        return render_domain(a.render_domain)
    return run_merge(a.review_dir, a.out_dir)


if __name__ == "__main__":
    sys.exit(main())
