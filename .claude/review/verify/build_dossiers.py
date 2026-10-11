#!/usr/bin/env python3
"""Verify stage, step 3: build the adjudication dossiers (deterministic, no LLM).

Usage:
  build_dossiers.py [--max-items 8] [--max-batches 30] [--hard-max-items 12] [--allow-partial] [--force]
  build_dossiers.py --self-test

Reads work/verify_queue.json, results/<id>.json (+ .check.json; recomputed in memory when absent) and the
worker tree. For every finding that has a valid Haiku result it writes, per batch, to work/adjudicate/:
  batch_NN.md    the dossier the Sonnet adjudicator receives (claim, Reduce record, Haiku verification,
                 quote_check result, +-15 line source excerpt). Decoys and controls look like any item.
  batch_NN.json  manifest {"batch", "items": [ids]} used to validate the adjudicator's answer
  index.json     batch -> ids, parameters, findings still missing a result
Batching: group by (specialty, file); pack groups in that order up to --max-items per batch; if that needs
more than --max-batches batches the item cap is raised one at a time up to --hard-max-items. Findings that
share identical claim text never share a batch (a decoy is built from a real claim; two identical claims
side by side would expose it). The answer key (work/verify_key.json) is never read here.
Refuses to overwrite when adjudications already exist (batch numbering would shift) unless --force.
"""
from __future__ import annotations

import argparse
import sys
import tempfile
from pathlib import Path

sys.path.append(str(Path(__file__).resolve().parent))   # append: this dir holds select.py
import vcommon as vc  # noqa: E402
import quote_check as qc  # noqa: E402

CONTEXT = 15
MAX_EXCERPT = 90


def excerpt(file: str, lines_spec: str, tree: Path) -> str:
    ls = vc.read_tree_lines(file, tree)
    if ls is None:
        return "[file not found in the worker tree, or path not readable]"
    s, e = vc.parse_lines(lines_spec)
    if s == 0:
        s = e = 1
    lo = max(1, s - CONTEXT)
    hi = min(len(ls), e + CONTEXT)
    note = ""
    if hi - lo + 1 > MAX_EXCERPT:
        hi = min(len(ls), lo + MAX_EXCERPT - 1)
        note = f"\n[excerpt truncated at {MAX_EXCERPT} lines; cited range is {lines_spec}]"
    w = len(str(hi))
    body = [f"{'>' if s <= n <= e else ' '} {str(n).rjust(w)} | {ls[n - 1]}" for n in range(lo, hi + 1)]
    if s > len(ls):
        note += f"\n[cited line {s} is beyond the end of the file ({len(ls)} lines)]"
    return f"lines {lo}-{hi} of {len(ls)} ('>' marks cited lines)\n" + "\n".join(body) + note


def fence(text: str) -> str:
    return "~~~~~~" if "~~~~~" in text else "~~~~~"


def q_lines(qs) -> str:
    if not qs:
        return "  (none)"
    return "\n".join(f"  - {q.get('file')}:{q.get('line')}  `{' '.join(str(q.get('text', '')).split())[:300]}`" for q in qs)


def render_item(n: int, total: int, item: dict, reduce: dict, res: dict, chk: dict, tree: Path) -> str:
    o = res["output"]
    rr = reduce or {}
    rline = (f"verification={rr.get('verification', 'unverified')}, severity={rr.get('severity')}, "
             f"confidence={rr.get('confidence')}")
    if rr.get("reason"):
        rline += f"\n  Reduce reason: {rr['reason']}"
    if rr.get("cited_lines_checked"):
        rline += f"\n  Reduce read: {rr['cited_lines_checked']}"
    searches = o.get("counter_evidence_searched") or []
    srch = "\n".join(f"  - `{s.get('pattern')}` in {s.get('scope')}: {s.get('hits')} hit(s). {s.get('note', '')}"
                     for s in searches) or "  (none recorded)"
    fab = chk.get("fabricated") or []
    fab_txt = "none" if not fab else "; ".join(
        f"{f.get('group')} {f.get('file')}:{f.get('line')} ({f.get('reason')})" for f in fab[:6])
    ex = excerpt(item["file"], item["lines"], tree)
    fnc = fence(ex)
    return "\n".join([
        f"## Item {n} of {total}: {item['id']}",
        f"Location: {item['file']}  lines {item['lines']}   category: {item['category']}   specialty: {item['specialty']}",
        "",
        "### Claim (from the earlier review)",
        item["claim"],
        "",
        "### Reduce record (earlier stage; informational, do not defer to it)",
        f"  {rline}",
        "",
        "### Haiku verification (a small model; may be wrong in either direction)",
        f"  verdict: {o.get('verdict')}   confidence: {o.get('confidence')}",
        f"  restated claim: {o.get('claim_restated')}",
        "  supporting quotes:", q_lines(o.get("supporting_quotes")),
        "  refutation quotes:", q_lines(o.get("refutation_quotes")),
        "  counter-evidence searches:", srch,
        f"  impact if real: {o.get('impact_if_real')}",
        f"  not verifiable without running: {o.get('not_verifiable_without_running')}",
        "  residual questions: " + ("; ".join(o.get("residual_questions") or []) or "(none)"),
        "",
        "### Quote check (script: quotes compared with the file within +-3 lines)",
        f"  quotes ok {chk.get('quotes_ok')}/{chk.get('quotes_total')}  quote_ok_ratio={chk.get('quote_ok_ratio')}  "
        f"verdict_supported={chk.get('verdict_supported')}  fabricated: {fab_txt}",
        "",
        "### Source excerpt (worker tree)",
        fnc, ex, fnc, "",
    ])


def pack(units: list[dict], cap: int) -> list[list[dict]]:
    """units sorted by (specialty, file, queue index) -> batches of <= cap; same-file items stay together."""
    groups: dict[tuple, list[dict]] = {}
    order = []
    for u in units:
        k = (u["item"]["specialty"], u["item"]["file"])
        if k not in groups:
            groups[k] = []
            order.append(k)
        groups[k].append(u)
    batches, cur = [], []
    for k in order:
        g = groups[k]
        if len(cur) + len(g) <= cap:
            cur += g
            continue
        if cur:
            batches.append(cur)
            cur = []
        while len(g) > cap:
            batches.append(g[:cap])
            g = g[cap:]
        cur = g
    if cur:
        batches.append(cur)
    return batches


def split_duplicates(units: list[dict]):
    seen, main, dups = set(), [], []
    for u in units:
        h = vc.sha_hex(vc.norm_ws(u["item"]["claim"]))
        (dups if h in seen else main).append(u)
        seen.add(h)
    return main, dups


def build(queue: dict, results_dir: Path, tree: Path, out_dir: Path, max_items=8, max_batches=30,
          hard_max=12, adj_dir: Path | None = None, force=False, allow_partial=True) -> dict:
    items = queue["items"]
    red = queue.get("reduce_records") or {}
    units, missing = [], []
    for idx, it in enumerate(items):
        res = vc.rj(results_dir / f"{it['id']}.json")
        if not (isinstance(res, dict) and res.get("status") == "ok" and res.get("schema_valid") is True
                and isinstance(res.get("output"), dict) and res["output"].get("finding_id") == it["id"]):
            missing.append(it["id"])
            continue
        chk = vc.rj(results_dir / f"{it['id']}.check.json")
        if not isinstance(chk, dict):
            chk = qc.check_result(res, tree)
        units.append({"idx": idx, "item": it, "res": res, "chk": chk, "reduce": red.get(it["id"], {})})
    if missing and not allow_partial:
        raise SystemExit(f"build_dossiers: {len(missing)} finding(s) lack a valid verifier result: {missing[:10]}...")
    if adj_dir is not None and adj_dir.is_dir() and any(adj_dir.glob("batch_*.json")) and not force:
        raise SystemExit("build_dossiers: adjudications already exist; rebuilding would renumber batches (use --force)")
    units.sort(key=lambda u: (u["item"]["specialty"], u["item"]["file"], u["idx"]))
    main, dups = split_duplicates(units)
    cap = max_items
    while True:
        batches = pack(main, cap) + pack(dups, cap)
        if len(batches) <= max_batches or cap >= hard_max:
            break
        cap += 1
    out_dir.mkdir(parents=True, exist_ok=True)
    for old in list(out_dir.glob("batch_*.md")) + list(out_dir.glob("batch_*.json")):
        old.unlink()
    index = {"params": {"max_items": max_items, "item_cap_used": cap, "max_batches": max_batches,
                        "hard_max_items": hard_max}, "batches": {}, "missing_results": missing,
             "items_total": len(units)}
    for n, b in enumerate(batches, 1):
        b.sort(key=lambda u: u["idx"])
        name = f"batch_{n:02d}"
        head = [f"# Adjudication batch {name}", "",
                f"Batch id: {name}. Items: {len(b)}. For each item decide the final verdict from the source; "
                "the Haiku verification is a lead, not an authority. Answer every item exactly once.", "",
                "Specialties in this batch: " + ", ".join(sorted({u['item']['specialty'] for u in b})), "", "---", ""]
        parts = [render_item(i, len(b), u["item"], u["reduce"], u["res"], u["chk"], tree) for i, u in enumerate(b, 1)]
        (out_dir / f"{name}.md").write_text("\n".join(head) + "\n---\n".join(parts), encoding="utf-8")
        ids = [u["item"]["id"] for u in b]
        (out_dir / f"{name}.json").write_text(vc.dumps({"batch": name, "items": ids}), encoding="utf-8")
        index["batches"][name] = ids
    (out_dir / "index.json").write_text(vc.dumps(index), encoding="utf-8")
    return index


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--max-items", type=int, default=8)
    ap.add_argument("--max-batches", type=int, default=30)
    ap.add_argument("--hard-max-items", type=int, default=12)
    ap.add_argument("--allow-partial", action="store_true", help="build from the findings that have results; list the rest")
    ap.add_argument("--force", action="store_true")
    ap.add_argument("--self-test", action="store_true")
    a = ap.parse_args(argv)
    if a.self_test:
        return self_test()
    q = vc.load_queue()
    idx = build(q, vc.RESULTS, vc.TREE, vc.BATCHES, a.max_items, a.max_batches, a.hard_max_items, vc.ADJ,
                a.force, a.allow_partial)
    sizes = [len(v) for v in idx["batches"].values()]
    print(f"build_dossiers: {len(sizes)} batch(es), {idx['items_total']} item(s), item cap used "
          f"{idx['params']['item_cap_used']}, sizes {sizes}")
    if idx["missing_results"]:
        print(f"build_dossiers: {len(idx['missing_results'])} queue item(s) have no valid verifier result "
              f"(not in any batch): {','.join(idx['missing_results'][:20])}", file=sys.stderr)
        return 0 if a.allow_partial else 3
    return 0


def self_test() -> int:
    ok = True

    def check(cond, msg):
        nonlocal ok
        if not cond:
            ok = False
            print("FAIL:", msg)

    with tempfile.TemporaryDirectory() as td:
        td = Path(td)
        tree, rd, out = td / "tree", td / "results", td / "out"
        (tree / "src").mkdir(parents=True)
        rd.mkdir()
        for name, n in (("a.cpp", 120), ("b.cpp", 30), ("c.py", 40)):
            (tree / "src" / name).write_text("\n".join(f"{name} line {i} ~~~~~ tilde" for i in range(1, n + 1)) + "\n")
        items, red = [], {}
        k = 0

        def add(file, lines, claim, spec):
            nonlocal k
            k += 1
            fid = f"id{k:02d}"
            items.append({"id": fid, "file": file, "lines": lines, "category": "logic", "claim": claim, "specialty": spec})
            red[fid] = {"verification": "unverified", "severity": "medium", "confidence": "low"}
            o = {"finding_id": fid, "verdict": "reproduced" if k % 2 else "refuted", "claim_restated": "restated claim here",
                 "tool_trace": [], "supporting_quotes": [{"file": file, "line": 3, "text": f"{file.split('/')[-1]} line 3"}],
                 "counter_evidence_searched": [{"pattern": "x", "scope": "src", "hits": 2, "note": "n"}],
                 "refutation_quotes": [], "impact_if_real": "i", "confidence": "high",
                 "not_verifiable_without_running": "nv", "residual_questions": ["q1"]}
            (rd / f"{fid}.json").write_text(vc.dumps({"status": "ok", "schema_valid": True, "output": o}))
            return fid

        for i in range(11):
            add("src/a.cpp", str(10 + i), f"claim number {i}", "cpp-logic")
        add("src/a.cpp", "100", "claim number 3", "cpp-logic")            # duplicate claim text (decoy-like)
        add("src/b.cpp", "5-7", "claim b", "cpp-logic")
        add("src/c.py", "39-40", "claim c", "python-tooling")
        miss = {"id": "idMISSING", "file": "src/c.py", "lines": "1", "category": "logic", "claim": "m", "specialty": "python-tooling"}
        queue = {"items": items + [miss], "reduce_records": red}
        idx = build(queue, rd, tree, out, max_items=8, max_batches=30)
        check(idx["missing_results"] == ["idMISSING"], "missing listed")
        flat = [i for v in idx["batches"].values() for i in v]
        check(sorted(flat) == sorted(i["id"] for i in items), "every verified item in exactly one batch")
        check(all(len(v) <= 8 for v in idx["batches"].values()), "cap respected")
        dup_id = "id12"
        twin = "id04"                                                       # claim number 3
        bt = {i: b for b, v in idx["batches"].items() for i in v}
        check(bt[dup_id] != bt[twin], "identical claims never share a batch")
        md = (out / "batch_01.md").read_text()
        check("## Item 1 of" in md and "~~~~~~" in md, "dossier rendered with safe fence")
        check("verify_key" not in md and "decoy" not in md.lower(), "no key leakage")
        a_lines = excerpt("src/a.cpp", "10-12", tree)
        check(a_lines.splitlines()[0].startswith("lines 1-27 of 120") and "> " in a_lines, "excerpt window")
        check("[file not found" in excerpt("src/nope.cpp", "1", tree), "missing file excerpt")
        check(excerpt("src/a.cpp", "1-100", tree).count("\n") <= MAX_EXCERPT + 3, "excerpt truncation")
        idx2 = build(queue, rd, tree, td / "out2", max_items=2, max_batches=3, hard_max=5)
        check(idx2["params"]["item_cap_used"] > 2 and idx2["params"]["item_cap_used"] <= 5, "cap raised when too many batches")
        (td / "adj").mkdir()
        (td / "adj" / "batch_01.json").write_text("{}")
        try:
            build(queue, rd, tree, out, adj_dir=td / "adj")
            check(False, "refuses to renumber over existing adjudications")
        except SystemExit:
            pass
        a = build(queue, rd, tree, td / "o3")
        b = build(queue, rd, tree, td / "o4")
        check(vc.dumps(a) == vc.dumps(b) and (td / "o3" / "batch_01.md").read_text() == (td / "o4" / "batch_01.md").read_text(),
              "deterministic")
    print("build_dossiers self-test:", "OK" if ok else "FAILED")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
