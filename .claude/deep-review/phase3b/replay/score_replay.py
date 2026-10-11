#!/usr/bin/env python3
"""Score a Haiku-ladder replay run (read-only, $0).

  score_replay.py --run rl1 [--run2 rl2] [--home DIR]

Compares per-item final verdicts against: stored Sonnet verdicts (.claude/review/verify/adjudications/batch_NN.json
-> output.items), the old single-Haiku A/B outputs (phase3b/replay_src/ab/batch_NN.json -> output.items) and the 4
ground-truth ids. Also L2 style self-agreement, escalation rate (items reaching L4) and CLI-reported cost per batch.
Missing layers print n/a. Output: <= 40 lines of markdown.
"""
from __future__ import annotations

import argparse
import json
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
HOME = Path(os.environ.get("DR_HOME") or Path.home() / ".cache/deep_haiku_review") / "kotor2-pilot"
BATCHES = ["batch_01", "batch_03", "batch_07", "batch_08", "batch_09", "batch_10", "batch_14", "batch_26"]
STORED = ROOT / ".claude/review/verify/adjudications"
OLD_HAIKU = ROOT / ".claude/deep-review/phase3b/replay_src/ab"
GT = {"e0a8aa228b90": "decoy", "279efdfe2ece": "K2", "257c4b579141": "K3a", "7bc29377aa57": "rejected"}


def rj(p: Path):
    try:
        return json.loads(p.read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return None


def items_of(rec) -> dict:
    out = (rec or {}).get("output") if isinstance(rec, dict) else None
    its = (out or {}).get("items") if isinstance(out, dict) else None
    return {i["finding_id"]: i for i in its or [] if isinstance(i, dict) and "finding_id" in i}


def cost_of(rec) -> float:
    c = (rec or {}).get("cost_usd") if isinstance(rec, dict) else None
    return float(c) if isinstance(c, (int, float)) and not isinstance(c, bool) else 0.0


def pct(n, d) -> str:
    return f"{n}/{d} ({100 * n / d:.0f}%)" if d else "n/a"


def load_run(home: Path, name: str):
    run = home / name
    if not run.is_dir():
        return None
    v = run / "verify"
    man = {b: (rj(run / "work/adjudicate" / f"{b}.json") or {}).get("items", []) for b in BATCHES}
    bat = {f: b for b, ids in man.items() for f in ids}
    R = {"name": name, "man": man, "bat": bat, "final": {}, "resolved": {}, "l2": {}, "styles": [], "l3": {}, "l4": {},
         "cost": {b: {"l2": 0.0, "l3": 0.0, "l4": 0.0} for b in BATCHES}, "have": {"l2": False, "l3": False, "l4": False}}
    for b in BATCHES:                                           # final ladder output
        rec = rj(v / "adjudications" / f"{b}.json")
        for f, it in items_of(rec).items():
            R["final"][f] = it.get("final_verdict")
            R["resolved"][f] = (it.get("ladder") or {}).get("resolved_by")
    st = rj(run / "work/ladder/state.json") or {}
    for f, s in (st.get("items") or {}).items():
        if f in bat:
            R["final"].setdefault(f, s.get("verdict"))
            R["resolved"].setdefault(f, (s.get("ladder") or {}).get("resolved_by"))
            if s.get("stage") == "need_l4":
                R["l4"].setdefault(f, "pending")
    l2dir = v / "adjudications_l2"
    for sd in sorted(p for p in l2dir.iterdir() if p.is_dir()) if l2dir.is_dir() else []:
        R["styles"].append(sd.name)
        for b in BATCHES:
            rec = rj(sd / f"{b}.json")
            if rec and rec.get("status") == "ok":
                R["have"]["l2"] = True
                R["cost"][b]["l2"] += cost_of(rec)
                for f, it in items_of(rec).items():
                    R["l2"].setdefault(f, {})[sd.name] = it.get("final_verdict")
    for layer in ("l3", "l4"):                                   # NN is compare_NN / escalate_NN, items keyed by finding_id
        d = v / f"adjudications_{layer}"
        for p in sorted(d.glob("batch_*.json")) if d.is_dir() else []:
            rec = rj(p)
            if not rec or rec.get("status") != "ok":
                continue
            R["have"][layer] = True
            if layer == "l3":
                per = [i for j in rec.get("judges") or [] if isinstance(j, dict) for i in items_of(j).values()]
            else:
                per = list(items_of(rec).values())
            fids = [i["finding_id"] for i in per if i["finding_id"] in bat]
            for i in per:
                if i["finding_id"] not in bat:
                    continue
                if layer == "l3":
                    R[layer].setdefault(i["finding_id"], []).append(i.get("final_verdict"))
                else:
                    R[layer][i["finding_id"]] = i.get("final_verdict")
            uniq = set(fids)
            for f in uniq:                                       # split the record cost across the batches it covers
                R["cost"][bat[f]][layer] += cost_of(rec) / len(uniq)
    return R


def agree(a: dict, b: dict, ids):
    both = [f for f in ids if a.get(f) is not None and b.get(f) is not None]
    return sum(1 for f in both if a[f] == b[f]), len(both)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--run", required=True)
    ap.add_argument("--run2")
    ap.add_argument("--home", default=str(HOME))
    a = ap.parse_args()
    home = Path(a.home)
    runs = []
    for n in [a.run] + ([a.run2] if a.run2 else []):
        R = load_run(home, n)
        if R is None:
            print(f"score_replay: run dir not found: {home / n}", file=sys.stderr)
            return 2
        runs.append(R)
    ids = [f for b in BATCHES for f in runs[0]["man"][b]]
    if not ids:
        print(f"score_replay: no manifests under {home / runs[0]['name']}/work/adjudicate (run build_rl1.py first)", file=sys.stderr)
        return 2
    sonnet, old, oldcost, sncost = {}, {}, 0.0, 0.0
    for b in BATCHES:
        s, o = rj(STORED / f"{b}.json"), rj(OLD_HAIKU / f"{b}.json")
        sonnet.update({f: i.get("final_verdict") for f, i in items_of(s).items()})
        old.update({f: i.get("final_verdict") for f, i in items_of(o).items()})
        sncost += cost_of(s)
        oldcost += float((o or {}).get("cost") or 0)
    L = [f"## Replay score: {', '.join(r['name'] for r in runs)} ({len(ids)} items, {len(BATCHES)} batches)"]
    na = lambda R, k: "n/a" if not R["have"][k] else None  # noqa: E731
    L.append(f"- baseline old single-Haiku vs Sonnet: {pct(*agree(old, sonnet, ids))}; stored Sonnet cost ${sncost:.3f}; old Haiku ${oldcost:.3f}")
    for R in runs:
        fin = R["final"]
        n_fin = sum(1 for f in ids if fin.get(f))
        L.append(f"### {R['name']}: styles={R['styles'] or 'n/a'}; final verdicts {n_fin}/{len(ids)}")
        L.append(f"- final vs Sonnet: {pct(*agree(fin, sonnet, ids))}; final vs old Haiku: {pct(*agree(fin, old, ids))}")
        if len(R["styles"]) >= 2:
            s1, s2 = R["styles"][:2]
            x = {f: R["l2"].get(f, {}).get(s1) for f in ids}
            y = {f: R["l2"].get(f, {}).get(s2) for f in ids}
            L.append(f"- L2 self-agreement {s1} vs {s2}: {pct(*agree(x, y, ids))}")
        else:
            L.append("- L2 self-agreement: n/a (needs >= 2 styles)")
        for s in R["styles"]:
            x = {f: R["l2"].get(f, {}).get(s) for f in ids}
            L.append(f"- L2 {s} vs Sonnet: {pct(*agree(x, sonnet, ids))}; vs old Haiku: {pct(*agree(x, old, ids))}")
        rb = {k: sum(1 for f in ids if R["resolved"].get(f) == k) for k in ("l2", "l3", "l4", "human")}
        L.append("- resolved_by: " + (", ".join(f"{k}={v}" for k, v in rb.items()) if any(rb.values()) else "n/a"))
        reach = sum(1 for f in ids if f in R["l4"] or R["resolved"].get(f) == "l4")
        L.append("- escalation rate (reached L4): " + (pct(reach, len(ids)) if R["resolved"] or R["have"]["l3"] or R["have"]["l4"] else "n/a"))
        c = {k: sum(R["cost"][b][k] for b in BATCHES) for k in ("l2", "l3", "l4")}
        tot = sum(c.values())
        L.append("- cost: " + (f"${tot:.3f} (L2 ${c['l2']:.3f}, L3 ${c['l3']:.3f}, L4 ${c['l4']:.3f}); per batch "
                               + " ".join(f"{b[-2:]}=${sum(R['cost'][b].values()):.3f}" for b in BATCHES) if tot else "n/a"))
    L.append("### Ground truth (id: Sonnet | old Haiku | " + " | ".join(f"{r['name']} final(resolved_by)" for r in runs) + ")")
    for f, tag in GT.items():
        cells = [f"{r['final'].get(f) or 'n/a'}({r['resolved'].get(f) or '-'})" for r in runs]
        L.append(f"- {tag} {f} [{runs[0]['bat'].get(f, '?')[-2:]}]: {sonnet.get(f) or 'n/a'} | {old.get(f) or 'n/a'} | " + " | ".join(cells))
    R = runs[0]
    dis = [f for f in ids if R["final"].get(f) and sonnet.get(f) and R["final"][f] != sonnet[f]]
    L.append(f"- {R['name']} disagreements with Sonnet ({len(dis)}): " + (", ".join(f"{f}:{R['final'][f]}!={sonnet[f]}" for f in dis[:4]) + (" ..." if len(dis) > 4 else "") if dis else "none/n/a"))
    print("\n".join(L[:40]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
