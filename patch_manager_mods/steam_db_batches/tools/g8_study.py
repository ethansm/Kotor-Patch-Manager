#!/usr/bin/env python3
"""g8_study.py - report-only calibration of invariant G8 (mac_rotation).  Writes only under --out (a scratch dir).

  1. calibration gold: run calibrate.py with and without G8 (BD_DISABLE_G8=1) in a scratch BD_ROOT that holds only
     calibration/gold_annotations.tsv (no B*/final_rows.tsv, so earlier batches never leak into the gold);
  2. known GOG label errors (lead list, gold_annotations label_suspect rows, B0/opus/overrides_*.tsv label_suspect):
     how many does G8 (and G7) flag, which are missed;
  3. B0: copy B0/candidates.tsv into the scratch root and run `gates` with and without G8; list the AUTO rows that flip.
Usage: g8_study.py --repo-root STEAM_DB_BATCHES --out SCRATCH_DIR
"""
from __future__ import annotations

import argparse
import csv
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import bdlib as bd  # noqa: E402
from bdlib import hx8, iter_tsv, parse_addr  # noqa: E402

HERE = Path(__file__).resolve().parent
LEAD_ADDRS = [0x70CCB0, 0x70CD60, 0x70CD90, 0x70CEB0, 0x667600, 0x667710, 0x667790]
LEAD_NAMES = ["CGuiInGame::ShowDialogMessage", "CGuiInGame::SetCombatMessage", "CGuiInGame::GetMiniMapVisible",
              "CGuiInGame::RemoveGUIPopUps", "CGuiInGame::ResetInterfaceFont", "CSWCObject::AnimationPause",
              "CSWCObject::AnimationPlayOutOfPlace"]


def run(args, env_extra, cwd=None) -> str:
    env = dict(os.environ, **env_extra)
    res = subprocess.run([sys.executable] + args, env=env, cwd=cwd, capture_output=True, text=True)
    if res.returncode:
        raise SystemExit("failed: %s\n%s" % (args, res.stderr[-800:]))
    return res.stdout


def calibration(root: Path, out: Path, repo: Path) -> dict:
    res = {}
    (root / "calibration").mkdir(parents=True, exist_ok=True)
    shutil.copy(repo / "calibration" / "gold_annotations.tsv", root / "calibration" / "gold_annotations.tsv")
    for tag, env in (("base", {"BD_DISABLE_G8": "1"}), ("g8", {})):
        for block in (1, 20):
            d = out / ("cal_%s_b%d" % (tag, block))
            run([str(HERE / "calibrate.py"), "--no-cards", "--no-reexport", "--block", str(block), "--out-dir", str(d)],
                dict(env, BD_ROOT=str(root)))
            res[(tag, block)] = json.loads((d / "metrics.json").read_text())
    return res


def known_errors(corpus: bd.Corpus, repo: Path) -> dict[int, dict]:
    known: dict[int, dict] = {}

    def add(ga, src, steam=None):
        if ga in corpus.gog_by_addr:
            known.setdefault(ga, {"sources": [], "steam": steam})["sources"].append(src)
            if steam and not known[ga]["steam"]:
                known[ga]["steam"] = steam
        else:
            known.setdefault(("absent", ga), {"sources": [src + " (not a GOG function address)"], "steam": None})

    for ga in LEAD_ADDRS:
        add(ga, "lead list")
    for n in LEAD_NAMES:
        g = corpus.gog_by_name.get(n)
        if g:
            add(g.addr, "lead list")
    for r in iter_tsv(repo / "calibration" / "gold_annotations.tsv"):
        if r.get("annotation") == "label_suspect":
            add(parse_addr(r["gog_addr"]), "gold_annotations", parse_addr(r["steam_addr"]) if r.get("steam_addr") else None)
    for f in ("a", "b", "c"):
        for r in iter_tsv(repo / "B0" / "opus" / ("overrides_%s.tsv" % f)):
            if r.get("final_verdict") == "label_suspect":
                add(parse_addr(r["gog_addr"]), "B0 opus %s" % f, parse_addr(r["steam_addr"]) if r.get("steam_addr") else None)
    return known


def b0_eval(root: Path, repo: Path) -> tuple[list[dict], list[dict]]:
    (root / "B0").mkdir(parents=True, exist_ok=True)
    shutil.copy(repo / "B0" / "candidates.tsv", root / "B0" / "candidates.tsv")
    run([str(HERE / "batch.py"), "gates", "B0"], {"BD_ROOT": str(root), "BD_DISABLE_G8": "1"})
    base = list(iter_tsv(root / "B0" / "evidence.tsv"))
    shutil.copy(root / "B0" / "evidence.tsv", root / "B0" / "evidence_base.tsv")
    run([str(HERE / "batch.py"), "gates", "B0"], {"BD_ROOT": str(root)})
    new = list(iter_tsv(root / "B0" / "evidence.tsv"))
    return base, new


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--repo-root", required=True, help="steam_db_batches (read only)")
    ap.add_argument("--out", required=True, help="scratch dir (written)")
    a = ap.parse_args(argv)
    repo, out = Path(a.repo_root).resolve(), Path(a.out).resolve()
    root = out / "root"
    out.mkdir(parents=True, exist_ok=True)
    L = ["# G8 mac_rotation study", ""]
    cal = calibration(root, out, repo)
    L += ["## Calibration gold (positives AUTO with / without G8)", "", "| split | variant | AUTO without G8 | AUTO with G8 | lost | % of all positives | decoys+negatives AUTO | stress AUTO |",
          "|---|---|---|---|---|---|---|---|"]
    worst = 0.0
    for block in (1, 20):
        for v in ("novote", "vote"):
            b, n = cal[("base", block)][v], cal[("g8", block)][v]
            lost = b["positive_auto"] - n["positive_auto"]
            worst = max(worst, lost / b["positive_auto"])
            L.append("| block-%d | %s | %d | %d | %d (%.1f%% of those) | %.1f%% | %d | %d |" % (
                block, v, b["positive_auto"], n["positive_auto"], lost, 100 * lost / b["positive_auto"],
                100 * lost / b["positives"], n["negatives_auto"], n["stress_auto"]))
    L += ["", "Largest AUTO loss: %.1f%% of the positives that were AUTO (budget 3%%)." % (100 * worst), ""]
    corpus = bd.Corpus(bd.get_paths())
    known = known_errors(corpus, repo)
    L += ["## Known GOG label errors", "", "| GOG | name | G8 | G7 (with the override's Steam address) | sources |", "|---|---|---|---|---|"]
    c8 = c7 = cu = total = 0
    misses = []
    for key, info in sorted(known.items(), key=lambda kv: str(kv[0])):
        if isinstance(key, tuple):
            L.append("| %s | - | - | - | %s |" % (hx8(key[1]), "; ".join(info["sources"])))
            continue
        g = corpus.gog_by_addr[key]
        state, detail = corpus.mac_rotation(g)
        g7 = "-"
        if info["steam"]:
            s7, ov = corpus.mac_check(g, info["steam"])
            g7 = s7 if ov is None else "%s(%.2f)" % (s7, ov)
        total += 1
        hit8, hit7 = state.startswith("rotation"), g7.startswith("mac_contra")
        c8 += hit8
        c7 += hit7
        cu += hit8 or hit7
        if not (hit8 or hit7):
            misses.append(g)
        L.append("| %s | %s | %s | %s | %s |" % (hx8(key), g.name, state + (":" + detail if detail else ""), g7,
                                                 "; ".join(sorted(set(info["sources"])))))
    L += ["", "G8 catches %d of %d, G7 catches %d, union %d. Missed by both (%d): %s" % (
        c8, total, c7, cu, len(misses), ", ".join("%s %s" % (hx8(g.addr), g.name) for g in misses) or "none"), ""]
    base, new = b0_eval(root, repo)
    nb = {r["gog"]: r for r in base}
    flips = []
    for r in new:
        o = nb.get(r["gog"])
        if o and o["gate_class"] == "AUTO" and r["gate_class"] != "AUTO" and r["g8"].startswith("rotation"):
            flips.append(r)
    names = {hx8(g.addr): g.name for g in corpus.gog}
    L += ["## B0: AUTO rows flipped to REVIEW by G8 (%d of %d AUTO)" % (
        len(flips), sum(1 for r in base if r["gate_class"] == "AUTO")), "",
        "| class::name | GOG | Steam | better G' | G' name | fits (G' > G) | was |", "|---|---|---|---|---|---|---|"]
    for r in flips:
        kind, gp, fits = (r["g8"].split(":", 2) + ["", ""])[:3]
        L.append("| %s::%s | %s | %s | %s | %s | %s | %s |" % (r["class"], r["name"], r["gog"], r["steam_addr"], gp,
                                                              names.get(gp, "?"), fits + " (" + kind + ")",
                                                              nb[r["gog"]]["flags"].split("auto_route=")[-1][:3]))
    L += ["", "B0 rows with any G8 flag (any class): %d" % sum(1 for r in new if r["g8"].startswith("rotation"))]
    (out / "G8_STUDY.md").write_text("\n".join(L) + "\n", encoding="utf-8")
    print("\n".join(L))
    return 0


if __name__ == "__main__":
    sys.exit(main())
