#!/usr/bin/env python3
"""Verify stage: deterministic quote checker (no LLM). Compares every quote a verifier (or adjudicator)
gave against the real file in the sanitized worker tree.

Usage:
  quote_check.py <finding_id> [...]        check results/<id>.json -> results/<id>.check.json
  quote_check.py --all                      every results/*.json that has no up-to-date check
  quote_check.py --adjudication batch_NN    check adjudications/batch_NN.json -> adjudications/batch_NN.check.json
  quote_check.py --self-test
Env: VERIFY_DIR (data dir), KOTOR2_REVIEW_HOME (tree = $KOTOR2_REVIEW_HOME/worktree), --tree DIR.

A quote {file, line, text} is OK when: the path is safe (relative, no '..', not excluded, inside the
tree), the file exists, and the whitespace-normalized text occurs in the whitespace-normalized text of
lines [line-3, line+3 (+ extra lines for a multi-line quote)]. Quotes under 6 normalized characters are
'trivial' and never count as OK. Output per result:
  {finding_id, verdict, quotes_total, quotes_ok, quote_ok_ratio, fabricated: [...], trivial: [...],
   supporting: {total, ok}, refutation: {total, ok}, verdict_supported}
verdict_supported: reproduced needs >=1 supporting quote OK; refuted needs >=1 refutation quote OK;
intended-behaviour needs >=1 OK quote of either kind; inconclusive is always supported.
"""
from __future__ import annotations

import json
import sys
import tempfile
from pathlib import Path

sys.path.append(str(Path(__file__).resolve().parent))   # append: this dir holds select.py
import vcommon as vc  # noqa: E402

WINDOW = 3
MIN_CHARS = 6


def check_quote(q: dict, tree: Path, cache: dict) -> tuple[str, str]:
    """-> (status, reason) with status in ok|fabricated|trivial."""
    if not isinstance(q, dict):
        return "fabricated", "not an object"
    f, line, text = q.get("file"), q.get("line"), q.get("text")
    if not isinstance(f, str) or not isinstance(text, str) or isinstance(line, bool) or not isinstance(line, int):
        return "fabricated", "malformed quote"
    rel = vc.safe_rel(f)
    if rel is None:
        return "fabricated", "unsafe or excluded path"
    if rel not in cache:
        cache[rel] = vc.read_tree_lines(rel, tree)
    lines = cache[rel]
    if lines is None:
        return "fabricated", "file not found in tree"
    want = vc.norm_ws(text)
    if len(want) < MIN_CHARS:
        return "trivial", "quote shorter than 6 characters"
    if line < 1 or line > len(lines):
        return "fabricated", f"line {line} outside file (1-{len(lines)})"
    extra = max(0, len(text.strip().splitlines()) - 1)
    lo = max(1, line - WINDOW)
    hi = min(len(lines), line + WINDOW + extra)
    window = vc.norm_ws("\n".join(lines[lo - 1:hi]))
    if want in window:
        return "ok", ""
    return "fabricated", f"text not found within lines {lo}-{hi}"


def check_quotes(groups: dict[str, list], tree: Path) -> dict:
    cache: dict = {}
    out = {"quotes_total": 0, "quotes_ok": 0, "fabricated": [], "trivial": []}
    per = {}
    for name, qs in groups.items():
        t = ok = 0
        for q in qs or []:
            t += 1
            st, why = check_quote(q, tree, cache)
            if st == "ok":
                ok += 1
            else:
                rec = {"group": name, "file": q.get("file") if isinstance(q, dict) else None,
                       "line": q.get("line") if isinstance(q, dict) else None,
                       "text": (str(q.get("text"))[:160] if isinstance(q, dict) else None), "reason": why}
                out["trivial" if st == "trivial" else "fabricated"].append(rec)
        per[name] = {"total": t, "ok": ok}
        out["quotes_total"] += t
        out["quotes_ok"] += ok
    out["quote_ok_ratio"] = round(out["quotes_ok"] / out["quotes_total"], 4) if out["quotes_total"] else 0.0
    return out | {"by_group": per}


def check_result(res: dict, tree: Path) -> dict:
    o = res.get("output") or {}
    c = check_quotes({"supporting": o.get("supporting_quotes") or [],
                      "refutation": o.get("refutation_quotes") or []}, tree)
    per = c.pop("by_group")
    verdict = o.get("verdict")
    if verdict == "reproduced":
        sup = per["supporting"]["ok"] >= 1
    elif verdict == "refuted":
        sup = per["refutation"]["ok"] >= 1
    elif verdict == "intended-behaviour":
        sup = per["supporting"]["ok"] + per["refutation"]["ok"] >= 1
    else:
        sup = verdict == "inconclusive"
    return {"finding_id": o.get("finding_id") or res.get("finding_id"), "verdict": verdict, **c,
            "supporting": per["supporting"], "refutation": per["refutation"], "verdict_supported": bool(sup)}


def check_adjudication(adj: dict, tree: Path) -> dict:
    items = {}
    tot = ok = 0
    fab = []
    for it in (adj.get("output") or {}).get("items") or []:
        c = check_quotes({"evidence": it.get("evidence_quotes") or []}, tree)
        c.pop("by_group")
        items[it.get("finding_id")] = c
        tot += c["quotes_total"]
        ok += c["quotes_ok"]
        fab += [dict(x, finding_id=it.get("finding_id")) for x in c["fabricated"]]
    return {"batch": adj.get("batch"), "quotes_total": tot, "quotes_ok": ok,
            "quote_ok_ratio": round(ok / tot, 4) if tot else 0.0, "fabricated": fab, "items": items}


def run_ids(ids: list[str], tree: Path) -> int:
    vc.RESULTS.mkdir(parents=True, exist_ok=True)
    for fid in ids:
        res = vc.rj(vc.RESULTS / f"{fid}.json")
        if not isinstance(res, dict) or not isinstance(res.get("output"), dict):
            print(f"{fid}: no valid result to check", file=sys.stderr)
            continue
        chk = check_result(res, tree)
        (vc.RESULTS / f"{fid}.check.json").write_text(vc.dumps(chk), encoding="utf-8")
        print(f"{fid}: quotes {chk['quotes_ok']}/{chk['quotes_total']} supported={chk['verdict_supported']} "
              f"fabricated={len(chk['fabricated'])}")
    return 0


def main(argv: list[str]) -> int:
    tree = vc.TREE
    args = list(argv)
    if "--tree" in args:
        i = args.index("--tree")
        tree = Path(args[i + 1])
        del args[i:i + 2]
    if not args:
        print(__doc__)
        return 2
    if args[0] == "--self-test":
        return self_test()
    if args[0] == "--all":
        ids = sorted(p.stem for p in vc.RESULTS.glob("*.json") if not p.name.endswith(".check.json"))
        return run_ids(ids, tree)
    if args[0] == "--adjudication":
        for b in args[1:]:
            adj = vc.rj(vc.ADJ / f"{b}.json")
            if not isinstance(adj, dict) or not isinstance(adj.get("output"), dict):
                print(f"{b}: no valid adjudication", file=sys.stderr)
                continue
            chk = check_adjudication(adj, tree)
            (vc.ADJ / f"{b}.check.json").write_text(vc.dumps(chk), encoding="utf-8")
            print(f"{b}: quotes {chk['quotes_ok']}/{chk['quotes_total']} fabricated={len(chk['fabricated'])}")
        return 0
    return run_ids(args, tree)


def self_test() -> int:
    ok = True

    def check(cond, msg):
        nonlocal ok
        if not cond:
            ok = False
            print("FAIL:", msg)

    with tempfile.TemporaryDirectory() as td:
        tree = Path(td)
        (tree / "src").mkdir()
        body = ["// header", "int a = 1;", "if (x   >  3) {", "    return foo(bar);", "}", "int b = 2;", "", "",
                "void tail() {}", "// end of file here"]
        (tree / "src" / "a.cpp").write_text("\n".join(body) + "\n")
        (tree / "config").mkdir()
        (tree / "config" / "app_secret.txt").write_text("TOPSECRET value in here\n")
        cache: dict = {}

        def q(f, l, t):
            return check_quote({"file": f, "line": l, "text": t}, tree, cache)[0]

        check(q("src/a.cpp", 3, "if (x > 3) {") == "ok", "whitespace normalised")
        check(q("src/a.cpp", 4, "if (x > 3) { return foo(bar); }") == "ok", "multi-line span")
        check(q("src/a.cpp", 5, "return foo(bar);") == "ok", "within +-3 (1 line off)")
        check(q("src/a.cpp", 7, "return foo(bar);") == "ok", "within +-3 (3 lines off)")
        check(q("src/a.cpp", 8, "return foo(bar);") == "fabricated", "4 lines off fails")
        check(q("src/a.cpp", 3, "return bar(foo);") == "fabricated", "wrong text")
        check(q("src/a.cpp", 99, "return foo(bar);") == "fabricated", "line outside file")
        check(q("src/a.cpp", 0, "return foo(bar);") == "fabricated", "line 0")
        check(q("src/missing.cpp", 1, "return foo(bar);") == "fabricated", "missing file")
        check(q("/etc/passwd", 1, "root:x:0:0") == "fabricated", "absolute path rejected")
        check(q("../x/a.cpp", 1, "int a = 1;") == "fabricated", "dotdot rejected")
        check(q("src/../src/a.cpp", 2, "int a = 1;") == "fabricated", "embedded dotdot rejected")
        check(q("config/app_secret.txt", 1, "TOPSECRET value in here") == "fabricated", "excluded path rejected")
        check(q(".env", 1, "SOMEKEY=1") == "fabricated", ".env rejected")
        check(q("src/a.cpp", 9, "}") == "trivial", "trivial quote")
        check(check_quote({"file": "src/a.cpp", "line": "3", "text": "int a = 1;"}, tree, cache)[0] == "fabricated",
              "non-int line")
        (tree / "link.cpp").symlink_to("/etc/hostname")
        check(q("link.cpp", 1, "anything long enough") == "fabricated", "symlink escaping tree rejected")

        res = {"output": {"finding_id": "f1", "verdict": "reproduced",
                          "supporting_quotes": [{"file": "src/a.cpp", "line": 3, "text": "if (x > 3) {"},
                                                {"file": "src/a.cpp", "line": 3, "text": "made up line"}],
                          "refutation_quotes": []}}
        c = check_result(res, tree)
        check(c["quotes_total"] == 2 and c["quotes_ok"] == 1 and len(c["fabricated"]) == 1, "counts")
        check(c["quote_ok_ratio"] == 0.5 and c["verdict_supported"] is True, "ratio/supported")
        res["output"]["verdict"] = "refuted"
        check(check_result(res, tree)["verdict_supported"] is False, "refuted needs refutation quote")
        res["output"]["refutation_quotes"] = [{"file": "src/a.cpp", "line": 6, "text": "int b = 2;"}]
        check(check_result(res, tree)["verdict_supported"] is True, "refuted with quote")
        res["output"]["verdict"] = "inconclusive"
        check(check_result(res, tree)["verdict_supported"] is True, "inconclusive supported")
        res["output"] = {"finding_id": "f1", "verdict": "reproduced", "supporting_quotes": [], "refutation_quotes": []}
        c = check_result(res, tree)
        check(c["verdict_supported"] is False and c["quote_ok_ratio"] == 0.0, "reproduced with no quotes")
        adj = {"batch": "batch_01", "output": {"items": [{"finding_id": "f1", "evidence_quotes": [
            {"file": "src/a.cpp", "line": 2, "text": "int a = 1;"}, {"file": "src/a.cpp", "line": 2, "text": "nope nope nope"}]}]}}
        ca = check_adjudication(adj, tree)
        check(ca["quotes_total"] == 2 and ca["quotes_ok"] == 1 and len(ca["fabricated"]) == 1, "adjudication counts")
    print("quote_check self-test:", "OK" if ok else "FAILED")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
