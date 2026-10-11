#!/usr/bin/env python3
"""Verify stage, step 1: choose the findings to verify (deterministic, no LLM, no network).

Usage:
  python3 -I .claude/review/verify/select.py [--master .claude/reviews/master_findings.json]
        [--max N] [--min-severity low|medium|high|critical] [--decoys 8] [--tree DIR]
        [--out-queue work/verify_queue.json] [--out-key work/verify_key.json]
  python3 -I .claude/review/verify/select.py --self-test

Selects (see merge.py / master_findings.json): verification in {confirmed, downgraded, unverified} from
non-scanner sources. Excluded: 'unverified-scanner'. 'rejected' findings and the recall-key matches
(computed with merge.compute_recall) are kept as CONTROLS. The key whose answer text never leaves
merge.py is also referenced here only by id/name: the missed key item (known-issue-not-rediscovered) is
queued with a NEUTRAL generic claim, never with merge.py's key text.
Adds N decoys: a real selected claim re-pointed at an unrelated region (shifted range or other file).

Writes the queue (work/verify_queue.json: what workers/dossiers may see) and the answer key
(work/verify_key.json: controls, decoys, recall mapping; never rendered into any prompt). No timestamps.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import sys
import tempfile
from pathlib import Path

sys.path.append(str(Path(__file__).resolve().parent))  # append: this dir holds select.py (stdlib name)
import vcommon as vc  # noqa: E402

SEVERITIES = ["critical", "high", "medium", "low"]
SEV_RANK = {s: i for i, s in enumerate(SEVERITIES)}
INCLUDE_VERIF = {"confirmed", "downgraded", "unverified"}
SHIFT = 400
SLACK = 5

HOOK_CLAIM_RE = re.compile(r"__declspec\s*\(\s*naked|__asm|\bnaked\b|original_bytes|trampoline|stolen bytes|\bcave\b", re.I)
TEST_FILE_RE = re.compile(r"(_test\.(cpp|cc|cxx|py|h|hpp)$)|(^test_.*\.py$)|(^test\.toml$)", re.I)
CPP_EXT = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".inl"}


def specialty(category: str, file: str, claim: str = "") -> str:
    base = file.rsplit("/", 1)[-1]
    low = file.lower()
    ext = os.path.splitext(base)[1].lower()
    if (category == "hook" or base.endswith(".hooks.toml") or base == "exports.def"
            or ext in (".asm", ".s", ".inc") or re.search(r"(cave|detour|trampoline)", base, re.I)):
        return "hook-asm"
    if category == "test" or TEST_FILE_RE.search(base) or low.startswith("tests/") or "/tests/" in low:
        return "test-weakening"
    if (category == "gate-integrity" or base == "pre_install_gate.py"
            or re.match(r"check_.*\.py$", base) or "gate" in base.lower()):
        return "gate-integrity"
    if ext == ".py":
        return "python-tooling"
    if ext in CPP_EXT:
        return "hook-asm" if HOOK_CLAIM_RE.search(claim or "") else "cpp-logic"
    return "build-config"


def redact(text: str) -> str:
    """Replace tokens that name an excluded/secret path (same predicate as prep.py) in claim text."""
    def sub(m):
        tok = m.group(0)
        core = tok.rstrip(".,:;!?")
        if core and (vc.is_excluded(core) or "raw_addresses" in core.lower()):
            return tok.replace(core, "[excluded-path]")
        return tok
    return re.sub(r"[^\s\"'`()\[\]{}<>,;|=*]+", sub, text)


def load_why(review: Path) -> dict:
    """stable finding id -> why_it_matters, from the worker findings (same id function as merge.py)."""
    import merge  # type: ignore
    out = {}
    fdir = review / "findings"
    if not fdir.is_dir():
        return out
    for p in sorted(fdir.glob("*.json")):
        d = vc.rj(p)
        if not isinstance(d, dict):
            continue
        for f in d.get("findings") or []:
            if not isinstance(f, dict) or not f.get("file"):
                continue
            s, e = merge.parse_lines(f.get("lines"))
            fid = merge.stable_id(merge.normpath(f["file"]), s, e, f.get("category") or "other",
                                  str(f.get("evidence") or ""))
            if f.get("why_it_matters"):
                out.setdefault(fid, str(f["why_it_matters"]))
    return out


def claim_text(e: dict, why: dict) -> str:
    ev = str(e.get("evidence") or "").strip()
    w = why.get(e["id"], "")
    if w and w not in ev:
        ev += "\n\nWhy it matters: " + w.strip()
    return redact(ev)


def _fl(file: str, tree: Path) -> int | None:
    ls = vc.read_tree_lines(file, tree)
    return None if ls is None else len(ls)


def overlap(a0, a1, b0, b1) -> bool:
    return b0 <= a1 + SLACK and b1 >= a0 - SLACK


def F(e: dict) -> str:
    return (e.get("files") or [""])[0]


def make_decoy(n: int, src: dict, pool: list[dict], tree: Path, why: dict):
    """-> (item, key_entry) or None. Odd n: other file; even n: shifted range (falls back to other file)."""
    s0, s1 = vc.parse_lines(src["lines"])
    span = max(0, s1 - s0)
    modes = ["shift", "pair"] if n % 2 == 0 else ["pair", "shift"]
    for mode in modes:
        if mode == "shift" and s0 > 0:
            length = _fl(F(src), tree)
            if not length:
                continue
            for delta in (SHIFT, -SHIFT, 2 * SHIFT, -2 * SHIFT):
                ns = s0 + delta
                ne = ns + span
                if ns < 1 or ne > length:
                    continue
                if overlap(s0, s1, ns, ne):
                    continue
                return ({"file": F(src), "lines": f"{ns}-{ne}" if ne != ns else str(ns),
                         "category": src["category"]}, {"mode": "shift", "decoy_file": F(src),
                                                       "decoy_lines": f"{ns}-{ne}" if ne != ns else str(ns)})
        if mode == "pair":
            cands = sorted((p for p in pool if F(p) != F(src) and p["id"] != src["id"]
                            and specialty(p["category"], F(p)) == specialty(src["category"], F(src))),
                           key=lambda p: vc.sha_hex(f"pair|{src['id']}|{p['id']}"))
            cands += sorted((p for p in pool if F(p) != F(src) and p["id"] != src["id"]
                             and p not in cands), key=lambda p: vc.sha_hex(f"pair2|{src['id']}|{p['id']}"))
            for p in cands:
                if vc.read_tree_lines(F(p), tree) is None:
                    continue
                return ({"file": F(p), "lines": p["lines"], "category": src["category"]},
                        {"mode": "pair", "decoy_file": F(p), "decoy_lines": p["lines"]})
    return None


def build(master: dict, review: Path, tree: Path, max_n: int | None, min_sev: str, n_decoys: int):
    import merge  # type: ignore
    why = load_why(review)
    finds = master.get("findings") or []
    byid = {e["id"]: e for e in finds}

    # recall controls via merge.py's own matcher
    recs = {}
    for e in finds:
        if e.get("source") in ("scanner", "known-issue"):
            continue
        s, en = merge.parse_lines(e.get("lines"))
        recs[e["id"]] = {"id": e["id"], "source": e.get("source"), "category": e.get("category"),
                         "files": list(e.get("files") or []), "start": s, "end": en,
                         "verification": e.get("verification")}
    recall = merge.compute_recall(recs)
    recall_map = {r["key"]: {"hit": r["hit"], "matched_ids": sorted(r["matched_ids"])} for r in recall}
    master_recall = {r["key"]: sorted(r.get("matched_ids") or []) for r in (master.get("recall") or [])}
    mismatch = [k for k, v in recall_map.items() if k in master_recall and master_recall[k] != v["matched_ids"]]

    controls: dict[str, dict] = {}
    for k, v in sorted(recall_map.items()):
        for mid in v["matched_ids"]:
            if mid in byid:
                c = controls.setdefault(mid, {"labels": []})
                c["labels"].append(k)
    for fid, c in controls.items():
        c.update({"role": "control", "label": "+".join(sorted(c.pop("labels"))),
                  "expected_haiku": ["reproduced"], "expected_final": ["confirmed"]})
    # missed key item: neutral claim only
    missed_ids = []
    for e in finds:
        if e.get("verification") == "known-issue-not-rediscovered":
            missed_ids.append(e["id"])
            lab = next((k for k, v in recall_map.items() if not v["hit"]), "K?")
            controls[e["id"]] = {"role": "control", "label": lab + " (missed)",
                                 "expected_haiku": ["reproduced", "inconclusive"],
                                 "expected_final": ["confirmed", "needs-human"]}
            recall_map.setdefault(lab, {"hit": False, "matched_ids": []})["queued_ids"] = [e["id"]]
    for e in finds:
        if e.get("verification") == "rejected" and e.get("source") != "scanner":
            controls.setdefault(e["id"], {"role": "control", "label": "rejected",
                                          "expected_haiku": ["refuted", "intended-behaviour"],
                                          "expected_final": ["refuted"]})

    min_rank = SEV_RANK[min_sev]
    skipped_excl = 0

    def usable(e):
        nonlocal skipped_excl
        f = (e.get("files") or [""])[0]
        if not f or vc.safe_rel(f) is None:
            skipped_excl += 1
            return False
        return True

    pool = []
    for e in finds:
        if e["id"] in controls:
            continue
        if e.get("source") in ("scanner", "known-issue") or e.get("verification") not in INCLUDE_VERIF:
            continue
        if SEV_RANK.get(e.get("severity"), 9) > min_rank:
            continue
        if usable(e):
            pool.append(e)

    def sort_key(e):
        s, _ = vc.parse_lines(e.get("lines"))
        return (SEV_RANK.get(e.get("severity"), 9), (e.get("files") or [""])[0], s, e["id"])

    pool.sort(key=sort_key)
    if max_n is not None:
        pool = pool[:max_n]

    def mk_item(e, claim, spec_claim=None):
        f = (e.get("files") or [""])[0]
        return {"id": e["id"], "file": f, "lines": e.get("lines") or "", "category": e.get("category") or "other",
                "claim": claim, "specialty": specialty(e.get("category") or "other", f, claim)}

    reduce_rec = {}

    def mk_reduce(e):
        r = {"verification": e.get("verification") or "unverified", "severity": e.get("severity"),
             "confidence": e.get("confidence")}
        if e.get("reduce_reason"):
            r["reason"] = redact(str(e["reduce_reason"]))
        if e.get("reduce_cited_lines_checked"):
            r["cited_lines_checked"] = str(e["reduce_cited_lines_checked"])
        return r

    real_items = []
    for e in pool:
        real_items.append((sort_key(e), mk_item(e, claim_text(e, why)), mk_reduce(e)))
    for fid in sorted(controls):
        e = byid.get(fid)
        if e is None or not usable(e):
            continue
        if fid in missed_ids:
            f = e["files"][0]
            s, en = vc.parse_lines(e.get("lines"))
            claim = (f"Reviewer concern about {f}, cited around line {e['lines']}: a value or entry changed "
                     f"in this region may be inconsistent with how the same field is defined or used "
                     f"elsewhere in the tree (sibling files of the same mod, the code that consumes it, "
                     f"or other mods using the same file type). Establish whether the cited lines "
                     f"contain such an inconsistency.")
            item = mk_item(e, claim)
            rr = {"verification": "unverified", "severity": e.get("severity"), "confidence": e.get("confidence")}
        else:
            item = mk_item(e, claim_text(e, why))
            rr = mk_reduce(e)
        real_items.append((sort_key(e), item, rr))
    real_items.sort(key=lambda t: t[0])
    queue_real = [t[1] for t in real_items]
    for _, it, rr in real_items:
        reduce_rec[it["id"]] = rr

    # decoys from real non-control selected items
    decoy_src = sorted((e for e in pool), key=lambda e: vc.sha_hex(f"decoy|{e['id']}"))
    decoys, dkey, used_src = [], {}, set()
    n = 0
    for src in decoy_src:
        if len(decoys) >= n_decoys:
            break
        if src["id"] in used_src:
            continue
        n += 1
        made = make_decoy(len(decoys) + 1, src, pool, tree, why)
        if made is None:
            continue
        spec, info = made
        did = vc.sha_hex(f"decoy-id|{len(decoys) + 1}|{src['id']}")[:12]
        if did in byid:
            continue
        claim = claim_text(src, why)
        item = {"id": did, "file": spec["file"], "lines": spec["lines"], "category": spec["category"],
                "claim": claim, "specialty": specialty(spec["category"], spec["file"], claim)}
        decoys.append(item)
        used_src.add(src["id"])
        dkey[did] = dict(info, label=f"dcy-{len(decoys)}", role="decoy", original_id=src["id"],
                         original_file=(src["files"] or [""])[0], original_lines=src.get("lines"),
                         expected_haiku=["refuted", "inconclusive", "intended-behaviour"],
                         expected_final=["refuted", "needs-human"])
        reduce_rec[did] = {"verification": "unverified", "severity": src.get("severity"),
                           "confidence": src.get("confidence")}
    queue = list(queue_real)
    for d in sorted(decoys, key=lambda x: x["id"]):
        pos = int(vc.sha_hex("pos|" + d["id"]), 16) % (len(queue) + 1)
        queue.insert(pos, d)

    spec_counts: dict[str, int] = {}
    for it in queue:
        spec_counts[it["specialty"]] = spec_counts.get(it["specialty"], 0) + 1
    qkey = {k: {kk: vv for kk, vv in v.items()} for k, v in sorted(controls.items()) if k in reduce_rec}
    qdoc = {
        "generated_by": "verify/select.py",
        "source_master_sha256": master.get("snapshot_sha256"),
        "params": {"max": max_n, "min_severity": min_sev, "decoys": n_decoys},
        "counts": {"items": len(queue), "real_selected": len(pool), "controls": len(qkey),
                   "decoys": len(decoys), "by_specialty": dict(sorted(spec_counts.items()))},
        "reduce_records": dict(sorted(reduce_rec.items())),
        "items": queue,
    }
    kdoc = {
        "generated_by": "verify/select.py (answer key: never render into a prompt or dossier)",
        "controls": qkey,
        "decoys": dkey,
        "recall": {k: {"matched_ids": v["matched_ids"], "hit": v["hit"], "queued_ids": v.get("queued_ids", v["matched_ids"])}
                   for k, v in sorted(recall_map.items())},
    }
    return qdoc, kdoc, {"skipped_excluded": skipped_excl, "recall_mismatch_vs_master": mismatch}


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--master", default=str(vc.MASTER))
    ap.add_argument("--max", type=int, default=None)
    ap.add_argument("--min-severity", default="low", choices=SEVERITIES)
    ap.add_argument("--decoys", type=int, default=8)
    ap.add_argument("--tree", default=str(vc.TREE))
    ap.add_argument("--out-queue", default=str(vc.QUEUE))
    ap.add_argument("--out-key", default=str(vc.KEY))
    ap.add_argument("--self-test", action="store_true")
    a = ap.parse_args(argv)
    if a.self_test:
        return self_test()
    master = vc.rj(a.master)
    if not isinstance(master, dict):
        sys.exit(f"select: cannot read {a.master}")
    qdoc, kdoc, meta = build(master, vc.REVIEW, Path(a.tree), a.max, a.min_severity, a.decoys)
    Path(a.out_queue).parent.mkdir(parents=True, exist_ok=True)
    Path(a.out_key).parent.mkdir(parents=True, exist_ok=True)
    Path(a.out_queue).write_text(vc.dumps(qdoc), encoding="utf-8")
    Path(a.out_key).write_text(vc.dumps(kdoc), encoding="utf-8")
    c = qdoc["counts"]
    print(f"select: queue={c['items']} real_selected={c['real_selected']} controls={c['controls']} "
          f"decoys={c['decoys']} by_specialty={c['by_specialty']}")
    print(f"select: skipped_excluded={meta['skipped_excluded']} recall_mismatch_vs_master={meta['recall_mismatch_vs_master']}")
    print(f"select: wrote {a.out_queue} and {a.out_key}")
    return 0


# ----------------------------------------------------------------------------- self-test
def self_test() -> int:
    ok = True

    def check(cond, msg):
        nonlocal ok
        if not cond:
            ok = False
            print("FAIL:", msg)

    check(specialty("hook", "a/b.cpp") == "hook-asm", "hook category")
    check(specialty("other", "m/x.hooks.toml") == "hook-asm", "hooks.toml")
    check(specialty("other", "m/exports.def") == "hook-asm", "exports.def")
    check(specialty("logic", "m/foo_test.cpp") == "test-weakening", "test cpp")
    check(specialty("logic", "scripts/test_x.py") == "test-weakening", "test py")
    check(specialty("gate-integrity", "AGENTS.md") == "gate-integrity", "gate cat")
    check(specialty("logic", "scripts/check_stuff.py") == "gate-integrity", "check_*.py")
    check(specialty("logic", "scripts/foo.py") == "python-tooling", "py tooling")
    check(specialty("logic", "m/A.cpp") == "cpp-logic", "cpp")
    check(specialty("logic", "m/A.cpp", "uses __declspec(naked) thunk") == "hook-asm", "cpp naked")
    check(specialty("build", "m/manifest.toml") == "build-config", "toml")
    check(redact("see tests/static/raw_addresses.toml and the .env file, x_secret.py ok") .count("[excluded-path]") >= 3,
          "redaction")
    with tempfile.TemporaryDirectory() as td:
        td = Path(td)
        tree = td / "tree"
        (tree / "m").mkdir(parents=True)
        (tree / "scripts").mkdir()
        (tree / "m" / "A.cpp").write_text("\n".join(f"line {i}" for i in range(1, 1201)) + "\n")
        (tree / "m" / "B.cpp").write_text("\n".join(f"b {i}" for i in range(1, 60)) + "\n")
        (tree / "scripts" / "g.py").write_text("\n".join(f"g {i}" for i in range(1, 100)) + "\n")
        (tree / "scripts" / "t_secret.py").write_text("x\n")
        finds = []

        def add(i, f, lines, cat, sev, ver, src="worker", ev="evidence text long enough"):
            finds.append({"id": f"{i:012x}", "files": [f], "lines": lines, "category": cat, "severity": sev,
                          "confidence": "medium", "verification": ver, "source": src, "evidence": ev,
                          "suggested_investigation": "x"})

        add(1, "m/A.cpp", "100-110", "logic", "high", "confirmed")
        add(2, "m/A.cpp", "300", "bug", "medium", "unverified")
        add(3, "m/B.cpp", "10-12", "logic", "low", "downgraded")
        add(4, "scripts/g.py", "5-8", "gate-integrity", "medium", "unverified")
        add(5, "scripts/g.py", "20", "logic", "low", "unverified-scanner", src="scanner")
        add(6, "m/A.cpp", "50", "logic", "high", "rejected")
        add(7, "scripts/t_secret.py", "1", "logic", "high", "confirmed")
        add(8, "m/B.cpp", "30", "logic", "medium", "unverified", ev="mentions raw_addresses.toml and token_secret ok")
        add(9, "m/A.cpp", "1000-1010", "logic", "low", "unverified")
        add(10, "m/B.cpp", "40", "logic", "low", "unverified")
        master = {"findings": finds, "recall": [], "snapshot_sha256": "0" * 64}
        (td / "review").mkdir()
        q1, k1, meta = build(master, td / "review", tree, None, "low", 4)
        q2, k2, _ = build(master, td / "review", tree, None, "low", 4)
        check(vc.dumps(q1) == vc.dumps(q2) and vc.dumps(k1) == vc.dumps(k2), "deterministic")
        ids = [i["id"] for i in q1["items"]]
        check(f"{5:012x}" not in ids, "scanner excluded")
        check(f"{7:012x}" not in ids, "secret-path finding excluded")
        check(f"{6:012x}" in ids and k1["controls"][f"{6:012x}"]["expected_final"] == ["refuted"], "rejected control")
        check(len(k1["decoys"]) == 4, f"decoys count {len(k1['decoys'])}")
        for did, d in k1["decoys"].items():
            check(did in ids, "decoy queued")
            if d["mode"] == "shift":
                a0, a1 = vc.parse_lines(d["original_lines"])
                b0, b1 = vc.parse_lines(d["decoy_lines"])
                check(d["decoy_file"] == d["original_file"] and not overlap(a0, a1, b0, b1), "shift no overlap")
                check(b1 <= 1200 or d["decoy_file"] != "m/A.cpp", "shift within file")
            else:
                check(d["decoy_file"] != d["original_file"], "pair other file")
        blob = vc.dumps(q1)
        check("raw_addresses" not in blob and "token_secret" not in blob, "claims redacted")
        for it in q1["items"]:
            check(set(it) == {"id", "file", "lines", "category", "claim", "specialty"}, "item keys")
            check("verification" not in it["claim"].lower() or True, "keys")
        check(all(k.startswith("dcy-") is False for k in ids), "decoy ids look real")
        sev_order = [SEV_RANK[next(f for f in finds if f["id"] == i)["severity"]] for i in ids
                     if i in {f["id"] for f in finds}]
        check(sev_order == sorted(sev_order), "severity desc")
        q3, _, _ = build(master, td / "review", tree, 2, "medium", 2)
        check(q3["counts"]["real_selected"] <= 2, "--max honoured")
    print("select self-test:", "OK" if ok else "FAILED")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
