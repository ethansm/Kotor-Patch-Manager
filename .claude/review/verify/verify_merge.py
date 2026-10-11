#!/usr/bin/env python3
"""Verify stage, final step: merge Haiku verifications + Sonnet adjudications into tiers (deterministic).

Usage:
  verify_merge.py [--queue work/verify_queue.json] [--key work/verify_key.json] [--results DIR]
                  [--adjudications DIR] [--master .claude/reviews/master_findings.json] [--out-dir DIR]
  verify_merge.py --self-test
Writes <out-dir>/verified_findings.{json,md} (default .claude/reviews; KOTOR2_VERIFY_OUT overrides). Stdlib
only, no timestamps: identical inputs give byte-identical outputs. Exit 1 when any queue item (decoys and
controls included) lacks a verifier result or an adjudication; the gaps are listed for re-queue.

Tiers (real findings only; decoys and controls are stripped from them and reported in `calibration`):
  CONFIRMED                 Haiku reproduced AND Sonnet confirmed AND Haiku's quotes verified (no fabricated
                            quote, verdict supported) AND Sonnet gave at least one verified evidence quote
  CONFIRMED-BY-SONNET-ONLY  Sonnet confirmed; Haiku disagreed, was inconclusive, or its quotes did not verify
  CONTESTED-NEEDS-HUMAN     Sonnet needs-human, or Sonnet confirmed without a verified evidence quote
  REFUTED                   Sonnet refuted (flag haiku_disagrees when Haiku had reproduced)
The answer key lives in work/verify_key.json (written by select.py). The recall key texts live only in merge.py.
"""
from __future__ import annotations

import argparse
import json
import sys
import tempfile
from pathlib import Path

sys.path.append(str(Path(__file__).resolve().parent))   # append: this dir holds select.py
import vcommon as vc  # noqa: E402
import merge  # type: ignore  # noqa: E402  (Haiku per-request pricing; same rates as the Map stage)

SEV_RANK = {"critical": 0, "high": 1, "medium": 2, "low": 3, "none": 4, None: 5}
TIERS = ["CONFIRMED", "CONFIRMED-BY-SONNET-ONLY", "CONTESTED-NEEDS-HUMAN", "REFUTED"]
H2F = {"reproduced": "confirmed", "refuted": "refuted", "intended-behaviour": "refuted", "inconclusive": "needs-human"}
SONNET_RATES = {"input": 2.0, "cache_write_5m": 2.5, "cache_read": 0.10, "output": 10.0}   # USD / MTok; write = 1.25x input (assumed)


def _i(v):
    try:
        return int(v or 0)
    except (TypeError, ValueError):
        return 0


def sonnet_usd(t: dict) -> float:
    r = SONNET_RATES
    return (_i(t.get("input_tokens")) * r["input"] + _i(t.get("cache_creation_input_tokens")) * r["cache_write_5m"]
            + _i(t.get("cache_read_input_tokens")) * r["cache_read"] + _i(t.get("output_tokens")) * r["output"]) / 1e6


def price(rec: dict, model: str) -> tuple[float, dict]:
    """Per-request price of one recorded run (turn_usage), falling back to the aggregate usage."""
    turns = rec.get("turn_usage") if isinstance(rec.get("turn_usage"), list) and rec.get("turn_usage") else None
    if turns is None:
        u = rec.get("usage")
        turns = [u] if isinstance(u, dict) and u else []
    tot = {"input_tokens": 0, "cache_creation_input_tokens": 0, "cache_read_input_tokens": 0, "output_tokens": 0}
    usd = 0.0
    for t in turns:
        for k in tot:
            tot[k] += _i(t.get(k))
        usd += merge.estimate_call(t)[1] if model == "haiku" else sonnet_usd(t)
    return usd, tot


def load_results(results: Path) -> tuple[dict, dict]:
    res, chk = {}, {}
    if results.is_dir():
        for p in sorted(results.glob("*.json")):
            d = vc.rj(p)
            if not isinstance(d, dict):
                continue
            if p.name.endswith(".check.json"):
                chk[p.name[:-len(".check.json")]] = d
            elif d.get("status") == "ok" and d.get("schema_valid") is True and isinstance(d.get("output"), dict) \
                    and d["output"].get("finding_id") == p.stem:
                res[p.stem] = d
    return res, chk


def load_adjudications(adjd: Path) -> tuple[dict, dict, dict, list]:
    adj, achk, batches, runs = {}, {}, {}, []
    if adjd.is_dir():
        for p in sorted(adjd.glob("batch_*.json")):
            d = vc.rj(p)
            if not isinstance(d, dict):
                continue
            if p.name.endswith(".check.json"):
                achk[p.name[:-len(".check.json")]] = d
                continue
            runs.append((p.stem, d))
            if d.get("status") == "ok" and d.get("schema_valid") is True and isinstance(d.get("output"), dict):
                batches[p.stem] = d["output"]
                for it in d["output"].get("items") or []:
                    if isinstance(it, dict) and it.get("finding_id"):
                        adj.setdefault(it["finding_id"], dict(it, _batch=p.stem))
    return adj, achk, batches, runs


def merge_all(queue: dict, key: dict, results: Path, adjd: Path, master: dict | None) -> dict:
    items = {it["id"]: it for it in queue["items"]}
    order = [it["id"] for it in queue["items"]]
    red = queue.get("reduce_records") or {}
    controls, decoys = key.get("controls") or {}, key.get("decoys") or {}
    mrec = {e["id"]: e for e in (master or {}).get("findings", [])} if master else {}
    res, chk = load_results(results)
    adj, achk, batches, adj_runs = load_adjudications(adjd)

    tiers = {t: [] for t in TIERS}
    rows = {}          # id -> summary used by calibration
    for fid in order:
        it = items[fid]
        h = res.get(fid)
        a = adj.get(fid)
        c = chk.get(fid) or {}
        hv = h["output"]["verdict"] if h else None
        fv = a["final_verdict"] if a else None
        ac = (achk.get(a["_batch"]) or {}).get("items", {}).get(fid) if a else None
        rows[fid] = {"id": fid, "specialty": it["specialty"], "haiku": hv, "final": fv,
                     "agree": (H2F.get(hv) == fv) if (hv and fv) else None,
                     "sonnet_self_agree": a.get("agrees_with_haiku") if a else None,
                     "role": "decoy" if fid in decoys else "control" if fid in controls else "real"}
        if rows[fid]["role"] != "real" or not (h and a):
            continue
        haiku_clean = bool(c) and c.get("verdict_supported") is True and not c.get("fabricated")
        sonnet_has_quote = (ac is None) or _i(ac.get("quotes_ok")) >= 1
        if fv == "confirmed":
            if not sonnet_has_quote:
                tier = "CONTESTED-NEEDS-HUMAN"
            elif hv == "reproduced" and haiku_clean:
                tier = "CONFIRMED"
            else:
                tier = "CONFIRMED-BY-SONNET-ONLY"
        elif fv == "needs-human":
            tier = "CONTESTED-NEEDS-HUMAN"
        else:
            tier = "REFUTED"
        m = mrec.get(fid, {})
        rr = red.get(fid, {})
        tiers[tier].append({
            "id": fid, "file": it["file"], "lines": it["lines"], "category": it["category"], "specialty": it["specialty"],
            "tier": tier, "final_verdict": fv, "final_severity": a.get("final_severity"),
            "reduce_verification": rr.get("verification"), "reduce_severity": rr.get("severity"),
            "haiku_verdict": hv, "haiku_confidence": h["output"].get("confidence"),
            "haiku_quote_ok_ratio": c.get("quote_ok_ratio"), "haiku_fabricated_quotes": len(c.get("fabricated") or []),
            "agrees_with_haiku": a.get("agrees_with_haiku"), "haiku_disagrees": (hv == "reproduced" and fv == "refuted"),
            "sonnet_quotes_ok": None if ac is None else f"{_i(ac.get('quotes_ok'))}/{_i(ac.get('quotes_total'))}",
            "reasoning": a.get("reasoning"), "human_check_steps": a.get("human_check_steps") or [],
            "evidence_quotes": a.get("evidence_quotes") or [], "claim": it["claim"],
            "suggested_investigation": m.get("suggested_investigation", ""),
        })
    for t in TIERS:
        tiers[t].sort(key=lambda e: (SEV_RANK.get(e["final_severity"], 5), e["file"], vc.parse_lines(e["lines"])[0], e["id"]))

    # ---- calibration
    dec = []
    for did, d in sorted(decoys.items()):
        r = rows.get(did, {})
        dec.append({"id": did, "label": d.get("label"), "mode": d.get("mode"), "haiku": r.get("haiku"),
                    "final": r.get("final"),
                    "caught": None if r.get("final") is None else r["final"] in ("refuted", "needs-human"),
                    "haiku_fooled": None if r.get("haiku") is None else r["haiku"] == "reproduced"})
    judged = [d for d in dec if d["caught"] is not None]
    decoy_block = {"total": len(dec), "adjudicated": len(judged), "caught": sum(1 for d in judged if d["caught"]),
                   "confirmed_loud_failures": [d["id"] for d in judged if not d["caught"]],
                   "haiku_reproduced_decoys": [d["id"] for d in dec if d["haiku_fooled"]],
                   "catch_rate": round(sum(1 for d in judged if d["caught"]) / len(judged), 4) if judged else None,
                   "detail": dec}
    ctl = []
    for cid, c0 in sorted(controls.items()):
        r = rows.get(cid, {})
        ctl.append({"id": cid, "label": c0.get("label"), "expected_haiku": c0.get("expected_haiku"),
                    "expected_final": c0.get("expected_final"), "haiku": r.get("haiku"), "final": r.get("final"),
                    "haiku_match": None if r.get("haiku") is None else r["haiku"] in (c0.get("expected_haiku") or []),
                    "final_match": None if r.get("final") is None else r["final"] in (c0.get("expected_final") or [])})
    recall = {}
    byctl = {c["id"]: c for c in ctl}
    for k, v in sorted((key.get("recall") or {}).items()):
        qids = [i for i in v.get("queued_ids", []) if i in byctl]
        recall[k] = {"queued_ids": qids, "found_by_audit": bool(v.get("hit")),
                     "haiku_ok": any(byctl[i]["haiku_match"] for i in qids) if qids else None,
                     "final_ok": any(byctl[i]["final_match"] for i in qids) if qids else None}
    both = [r for r in rows.values() if r["agree"] is not None]
    real_both = [r for r in both if r["role"] == "real"]
    spec = {}
    for r in both:
        s = spec.setdefault(r["specialty"], {"n": 0, "agree": 0})
        s["n"] += 1
        s["agree"] += 1 if r["agree"] else 0
    for s in spec.values():
        s["rate"] = round(s["agree"] / s["n"], 4)
    def rate(rs):
        return round(sum(1 for r in rs if r["agree"]) / len(rs), 4) if rs else None
    fab_h = sum(len(c.get("fabricated") or []) for c in chk.values())
    triv_h = sum(len(c.get("trivial") or []) for c in chk.values())
    fab_s = sum(len(c.get("fabricated") or []) for c in achk.values())
    qual = {}
    for b, o in sorted(batches.items()):
        r = (o.get("haiku_quality") or {}).get("rating")
        qual[r] = qual.get(r, 0) + 1
    sonnet_self = [r for r in rows.values() if r["sonnet_self_agree"] is not None]
    cal = {
        "decoys": decoy_block, "controls": ctl, "key_recall": recall,
        "haiku_vs_sonnet": {"items_compared": len(both), "agreement_rate_all": rate(both),
                            "agreement_rate_real": rate(real_both),
                            "sonnet_self_reported_agree_rate": round(sum(1 for r in sonnet_self if r["sonnet_self_agree"]) / len(sonnet_self), 4) if sonnet_self else None,
                            "haiku_reproduced_sonnet_refuted": sum(1 for r in both if r["haiku"] == "reproduced" and r["final"] == "refuted"),
                            "haiku_refuted_sonnet_confirmed": sum(1 for r in both if H2F.get(r["haiku"]) == "refuted" and r["final"] == "confirmed")},
        "per_specialty_agreement": dict(sorted(spec.items())),
        "fabricated_quotes": {"haiku": fab_h, "haiku_trivial": triv_h, "sonnet": fab_s},
        "haiku_quality_by_batch": dict(sorted(qual.items(), key=lambda kv: str(kv[0]))),
    }

    # ---- cost
    hc = {"calls": 0, "usd_est": 0.0, "cli_usd": 0.0, "input_tokens": 0, "cache_creation_input_tokens": 0,
          "cache_read_input_tokens": 0, "output_tokens": 0}
    sc = dict(hc)
    allres = {}
    if results.is_dir():
        for p in sorted(results.glob("*.json")):
            if not p.name.endswith(".check.json"):
                d = vc.rj(p)
                if isinstance(d, dict):
                    allres[p.stem] = d
    for rec, acc, model in [(r, hc, "haiku") for r in allres.values()] + [(d, sc, "sonnet") for _, d in adj_runs]:
        usd, tot = price(rec, model)
        acc["calls"] += 1
        acc["usd_est"] += usd
        if isinstance(rec.get("cost_usd"), (int, float)):
            acc["cli_usd"] += rec["cost_usd"]
        for k in tot:
            acc[k] += tot[k]
    for acc in (hc, sc):
        acc["usd_est"] = round(acc["usd_est"], 6)
        acc["cli_usd"] = round(acc["cli_usd"], 6)
    cost = {"label": "estimate (API-equivalent list rates, priced per request)",
            "haiku_verifier": hc, "sonnet_adjudicator": sc,
            "total_usd_est": round(hc["usd_est"] + sc["usd_est"], 6),
            "total_cli_reported_usd": round(hc["cli_usd"] + sc["cli_usd"], 6),
            "rates_usd_per_mtok": {"haiku_le_100k": merge.RATES["haiku-5.5"]["le_100k"], "haiku_gt_100k": merge.RATES["haiku-5.5"]["gt_100k"],
                                   "sonnet": SONNET_RATES, "note": "Sonnet cache-write rate assumed 1.25x input"}}

    # ---- coverage
    miss_v = [i for i in order if i not in res]
    miss_a = [i for i in order if i not in adj]
    cov = {"complete": not miss_v and not miss_a, "queue_items": len(order),
           "missing_verifier_result": miss_v, "missing_adjudication": miss_a}
    counts = {t: len(tiers[t]) for t in TIERS}
    return {"generated_by": "verify/verify_merge.py", "queue_source_master_sha256": queue.get("source_master_sha256"),
            "counts": dict(counts, real_items=sum(counts.values())), "tiers": tiers, "calibration": cal,
            "cost": cost, "coverage": cov}


def esc(s) -> str:
    return " ".join(str(s if s is not None else "").split()).replace("|", "\\|")


def render_md(doc: dict) -> str:
    cal, cost, cov = doc["calibration"], doc["cost"], doc["coverage"]
    L = ["# Verified findings (Haiku verifier, then Sonnet adjudicator)", "",
         "Find-only. Tiers list real findings; decoys and controls are reported under Calibration only.", ""]
    L += ["## Summary", "", "| Tier | Count |", "|---|---|"]
    for t in TIERS:
        L.append(f"| {t} | {doc['counts'][t]} |")
    L.append("")
    d = cal["decoys"]
    L += ["## Calibration", ""]
    flag = "" if not d["confirmed_loud_failures"] else "  **LOUD FAILURE: decoy(s) confirmed: " + ", ".join(d["confirmed_loud_failures"]) + "**"
    L.append(f"- Decoys: {d['caught']}/{d['adjudicated']} caught (final refuted or needs-human) of {d['total']} queued; "
             f"catch rate {d['catch_rate']}; Haiku reproduced {len(d['haiku_reproduced_decoys'])}.{flag}")
    hv = cal["haiku_vs_sonnet"]
    L.append(f"- Haiku vs Sonnet agreement: all {hv['agreement_rate_all']} over {hv['items_compared']} items; real {hv['agreement_rate_real']}; "
             f"Sonnet self-reported {hv['sonnet_self_reported_agree_rate']}; Haiku reproduced but Sonnet refuted {hv['haiku_reproduced_sonnet_refuted']}; "
             f"Haiku refuted but Sonnet confirmed {hv['haiku_refuted_sonnet_confirmed']}.")
    fq = cal["fabricated_quotes"]
    L.append(f"- Fabricated quotes: Haiku {fq['haiku']} (trivial {fq['haiku_trivial']}), Sonnet {fq['sonnet']}. Haiku quality by batch: {cal['haiku_quality_by_batch']}.")
    for k, v in cal["key_recall"].items():
        L.append(f"- Recall key {k}: found by audit={v['found_by_audit']}; verifier agrees={v['haiku_ok']}; adjudicator agrees={v['final_ok']}; queued {', '.join(v['queued_ids']) or 'none'}.")
    L += ["", "Controls", "", "| id | label | expected (Haiku / final) | Haiku | final | ok |", "|---|---|---|---|---|---|"]
    for c in cal["controls"]:
        L.append(f"| {c['id']} | {esc(c['label'])} | {'/'.join(c['expected_haiku'] or [])} ; {'/'.join(c['expected_final'] or [])} | {c['haiku']} | {c['final']} | {c['haiku_match']}/{c['final_match']} |")
    L += ["", "Per-specialty Haiku/Sonnet agreement", "", "| specialty | n | agree | rate |", "|---|---|---|---|"]
    for s, v in cal["per_specialty_agreement"].items():
        L.append(f"| {s} | {v['n']} | {v['agree']} | {v['rate']} |")
    h, s_ = cost["haiku_verifier"], cost["sonnet_adjudicator"]
    L += ["", "## Cost (estimate)", "",
          f"- Haiku verifier: {h['calls']} call(s), est ${h['usd_est']} (CLI-reported ${h['cli_usd']})",
          f"- Sonnet adjudicator: {s_['calls']} session(s), est ${s_['usd_est']} (CLI-reported ${s_['cli_usd']})",
          f"- Total est ${cost['total_usd_est']} (CLI-reported ${cost['total_cli_reported_usd']}). {cost['rates_usd_per_mtok']['note']}.", ""]
    L += ["## Coverage", "",
          f"- Complete: {cov['complete']} ({cov['queue_items']} queue items, decoys and controls included)"]
    if cov["missing_verifier_result"]:
        L.append("- Missing verifier result (re-queue: `run_verify.sh --only " + ",".join(cov["missing_verifier_result"]) + "`)")
    if cov["missing_adjudication"]:
        L.append("- Missing adjudication: " + ", ".join(cov["missing_adjudication"]))
    L.append("")
    for t in TIERS:
        L += [f"## {t} ({doc['counts'][t]})", ""]
        for e in doc["tiers"][t]:
            L.append(f"### {e['id']}  {e['file']}:{e['lines']}  [{e['final_severity']}] {e['category']} ({e['specialty']})")
            L.append(f"- Haiku: {e['haiku_verdict']} ({e['haiku_confidence']}), quote ok ratio {e['haiku_quote_ok_ratio']}; "
                     f"Reduce: {e['reduce_verification']}/{e['reduce_severity']}; Sonnet quotes ok {e['sonnet_quotes_ok']}"
                     + ("; **Haiku had reproduced**" if e["haiku_disagrees"] else ""))
            L.append(f"- Reasoning: {esc(e['reasoning'])}")
            for st in e["human_check_steps"][:6]:
                L.append(f"  - check: {esc(st)}")
            for q in e["evidence_quotes"][:3]:
                L.append(f"  - quote {q.get('file')}:{q.get('line')}: `{esc(q.get('text'))[:240]}`")
            L.append("")
    return "\n".join(L) + "\n"


def run(queue_p: Path, key_p: Path, results: Path, adjd: Path, master_p: Path | None, out_dir: Path) -> int:
    queue, key = vc.rj(queue_p), vc.rj(key_p)
    if not isinstance(queue, dict) or not isinstance(key, dict):
        print(f"verify_merge: cannot read queue/key ({queue_p}, {key_p})", file=sys.stderr)
        return 2
    master = vc.rj(master_p) if master_p else None
    doc = merge_all(queue, key, results, adjd, master if isinstance(master, dict) else None)
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "verified_findings.json").write_text(vc.dumps(doc), encoding="utf-8")
    (out_dir / "verified_findings.md").write_text(render_md(doc), encoding="utf-8")
    cov = doc["coverage"]
    print("verify_merge: " + " ".join(f"{t}={doc['counts'][t]}" for t in TIERS) +
          f" decoys_caught={doc['calibration']['decoys']['caught']}/{doc['calibration']['decoys']['adjudicated']}"
          f" cost_est=${doc['cost']['total_usd_est']}")
    if not cov["complete"]:
        print(f"verify_merge: GAPS verifier={len(cov['missing_verifier_result'])} adjudication={len(cov['missing_adjudication'])}",
              file=sys.stderr)
        if cov["missing_verifier_result"]:
            print("  re-queue verifier: run_verify.sh --only " + ",".join(cov["missing_verifier_result"]), file=sys.stderr)
        if cov["missing_adjudication"]:
            print("  missing adjudication ids: " + ",".join(cov["missing_adjudication"]), file=sys.stderr)
        return 1
    return 0


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--queue", default=str(vc.QUEUE))
    ap.add_argument("--key", default=str(vc.KEY))
    ap.add_argument("--results", default=str(vc.RESULTS))
    ap.add_argument("--adjudications", default=str(vc.ADJ))
    ap.add_argument("--master", default=str(vc.MASTER))
    ap.add_argument("--out-dir", default=str(vc.OUT_DIR))
    ap.add_argument("--self-test", action="store_true")
    a = ap.parse_args(argv)
    if a.self_test:
        return self_test()
    return run(Path(a.queue), Path(a.key), Path(a.results), Path(a.adjudications),
               Path(a.master) if a.master and Path(a.master).exists() else None, Path(a.out_dir))


def self_test() -> int:
    ok = True

    def check(cond, msg):
        nonlocal ok
        if not cond:
            ok = False
            print("FAIL:", msg)

    with tempfile.TemporaryDirectory() as td:
        td = Path(td)
        rd, ad, out = td / "results", td / "adj", td / "out"
        rd.mkdir()
        ad.mkdir()
        spec = ["cpp-logic", "python-tooling"]
        ids = ["r1", "r2", "r3", "r4", "r5", "c1", "c2", "d1", "d2"]
        items = [{"id": i, "file": f"src/{i}.cpp", "lines": "10-12", "category": "logic", "claim": f"claim {i}",
                  "specialty": spec[n % 2]} for n, i in enumerate(ids)]
        queue = {"items": items, "reduce_records": {i: {"verification": "unverified", "severity": "medium"} for i in ids},
                 "source_master_sha256": "x"}
        key = {"controls": {"c1": {"label": "K1", "expected_haiku": ["reproduced"], "expected_final": ["confirmed"]},
                            "c2": {"label": "rejected", "expected_haiku": ["refuted"], "expected_final": ["refuted"]}},
               "decoys": {"d1": {"label": "dcy-1", "mode": "shift"}, "d2": {"label": "dcy-2", "mode": "pair"}},
               "recall": {"K1": {"hit": True, "matched_ids": ["c1"], "queued_ids": ["c1"]}}}
        hv = {"r1": "reproduced", "r2": "reproduced", "r3": "refuted", "r4": "inconclusive", "r5": "refuted",
              "c1": "reproduced", "c2": "refuted", "d1": "reproduced", "d2": "refuted"}
        fv = {"r1": "confirmed", "r2": "confirmed", "r3": "confirmed", "r4": "needs-human", "r5": "refuted",
              "c1": "confirmed", "c2": "refuted", "d1": "confirmed", "d2": "refuted"}
        usage = {"input_tokens": 10, "cache_creation_input_tokens": 1000, "cache_read_input_tokens": 5000, "output_tokens": 300}
        for i in ids:
            o = {"finding_id": i, "verdict": hv[i], "confidence": "high"}
            (rd / f"{i}.json").write_text(vc.dumps({"status": "ok", "schema_valid": True, "output": o, "cost_usd": 0.01,
                                                    "turn_usage": [usage, usage]}))
            fab = [{"x": 1}] if i == "r2" else []
            (rd / f"{i}.check.json").write_text(vc.dumps({"verdict_supported": True, "quote_ok_ratio": 1.0, "fabricated": fab,
                                                          "trivial": []}))
        items_out = [{"finding_id": i, "final_verdict": fv[i], "final_severity": "high" if fv[i] == "confirmed" else "none",
                      "agrees_with_haiku": H2F[hv[i]] == fv[i], "reasoning": f"why {i}", "human_check_steps": ["look"],
                      "evidence_quotes": []} for i in ids]
        (ad / "batch_01.json").write_text(vc.dumps({"status": "ok", "schema_valid": True, "cost_usd": 0.5,
            "turn_usage": [usage], "output": {"batch": "batch_01", "haiku_quality": {"rating": "weak", "reason": "r"},
                                              "items": items_out, "notes": ""}}))
        (ad / "batch_01.check.json").write_text(vc.dumps({"items": {i: {"quotes_ok": 1, "quotes_total": 1, "fabricated": []} for i in ids}}))
        doc = merge_all(queue, key, rd, ad, None)
        t = {k: [e["id"] for e in v] for k, v in doc["tiers"].items()}
        check(t["CONFIRMED"] == ["r1"], f"CONFIRMED {t['CONFIRMED']}")
        check(sorted(t["CONFIRMED-BY-SONNET-ONLY"]) == ["r2", "r3"], f"SONNET-ONLY {t['CONFIRMED-BY-SONNET-ONLY']} (r2 has a fabricated Haiku quote)")
        check(t["CONTESTED-NEEDS-HUMAN"] == ["r4"], "contested")
        check(t["REFUTED"] == ["r5"], "refuted")
        allids = [i for v in t.values() for i in v]
        check(not any(x in allids for x in ("c1", "c2", "d1", "d2")), "controls/decoys stripped from tiers")
        d = doc["calibration"]["decoys"]
        check(d["confirmed_loud_failures"] == ["d1"] and d["caught"] == 1 and d["catch_rate"] == 0.5, f"decoy block {d}")
        check(d["haiku_reproduced_decoys"] == ["d1"], "haiku fooled")
        c = {x["id"]: x for x in doc["calibration"]["controls"]}
        check(c["c1"]["final_match"] and c["c2"]["haiku_match"], "control matches")
        check(doc["calibration"]["key_recall"]["K1"]["final_ok"] is True, "recall")
        check(doc["calibration"]["fabricated_quotes"]["haiku"] == 1, "fab count")
        check(doc["calibration"]["haiku_vs_sonnet"]["haiku_refuted_sonnet_confirmed"] == 1, "refuted->confirmed count")
        check(doc["coverage"]["complete"], "coverage complete")
        hc = doc["cost"]["haiku_verifier"]
        exp_h = 9 * 2 * (10 * 0.10 + 1000 * 0.125 + 5000 * 0.01 + 300 * 0.50) / 1e6
        check(abs(hc["usd_est"] - round(exp_h, 6)) < 1e-6, f"haiku per-request price {hc['usd_est']} vs {exp_h}")
        exp_s = (10 * 2 + 1000 * 2.5 + 5000 * 0.10 + 300 * 10) / 1e6
        check(abs(doc["cost"]["sonnet_adjudicator"]["usd_est"] - round(exp_s, 6)) < 1e-6, "sonnet price")
        rc = run(td / "q.json", td / "k.json", rd, ad, None, out)
        check(rc == 2, "missing queue -> 2")
        (td / "q.json").write_text(vc.dumps(queue))
        (td / "k.json").write_text(vc.dumps(key))
        rc = run(td / "q.json", td / "k.json", rd, ad, None, out)
        check(rc == 0, "complete -> exit 0")
        a = (out / "verified_findings.json").read_bytes() + (out / "verified_findings.md").read_bytes()
        run(td / "q.json", td / "k.json", rd, ad, None, td / "out2")
        b = (td / "out2" / "verified_findings.json").read_bytes() + (td / "out2" / "verified_findings.md").read_bytes()
        check(a == b, "byte-reproducible")
        check("LOUD FAILURE" in (out / "verified_findings.md").read_text(), "loud failure in md")
        # gaps
        (rd / "r3.json").unlink()
        items_out2 = [x for x in items_out if x["finding_id"] != "r4"]
        (ad / "batch_01.json").write_text(vc.dumps({"status": "ok", "schema_valid": True, "output": {
            "batch": "batch_01", "haiku_quality": {"rating": "ok", "reason": "r"}, "items": items_out2, "notes": ""}}))
        rc = run(td / "q.json", td / "k.json", rd, ad, None, out)
        check(rc == 1, "gaps -> exit 1")
        cov = vc.rj(out / "verified_findings.json")["coverage"]
        check(cov["missing_verifier_result"] == ["r3"] and cov["missing_adjudication"] == ["r4"], f"gap lists {cov}")
    print("verify_merge self-test:", "OK" if ok else "FAILED")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
