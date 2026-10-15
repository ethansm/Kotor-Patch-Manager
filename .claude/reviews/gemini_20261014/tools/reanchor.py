"""G2.1: re-anchor every verdicts.tsv row to the current HEAD + worktree. Run from repo root.

drift: none (anchor hits the G1 line), moved (hits elsewhere), gone (G1 hit, now no hit),
       noanchor (G1 anchor was NOT-FOUND/no-file; evidence lines decide).
ev_changed: a cited evidence line sits in a hunk changed since the bundle head (worktree diff, conservative).
"""
import csv, re, subprocess
from collections import Counter
from pathlib import Path

D = Path(".claude/reviews/gemini_20261014")
BUNDLE_HEAD = "02b5be44bb54"
rd = lambda p: list(csv.DictReader(open(p, newline=""), delimiter="\t", quoting=csv.QUOTE_NONE))
V, F = rd(D / "verdicts.tsv"), {r["fid"]: r for r in rd(D / "findings.tsv")}
A = {r["fid"]: r for r in rd(D / "anchors.tsv")}
sh = lambda *a: subprocess.run(a, capture_output=True, text=True).stdout
head = sh("git", "rev-parse", "--short=12", "HEAD").strip()
dirty = {l[3:].strip() for l in sh("git", "status", "--short").splitlines()}

TOK = re.compile(r"([\w./-]+\.(?:h|cpp|c|py|toml|md|txt|tsv|sh|json|ini|nss|2da|java))?:(\d+)(?:-(\d+))?((?:,\s*\d+(?:-\d+)?)*)")


def ev_lines(ev):
    """[(path, lo, hi)] from 'a.h:1-2, :5; b.cpp:7,9'; a bare ':N' reuses the last path."""
    out, last = [], None
    for m in TOK.finditer(ev):
        last = m.group(1) or last
        if not last:
            continue
        spans = [(int(m.group(2)), int(m.group(3) or m.group(2)))]
        for extra in re.findall(r"\d+(?:-\d+)?", m.group(4) or ""):
            lo, _, hi = extra.partition("-")
            spans.append((int(lo), int(hi or lo)))
        out += [(last, lo, hi) for lo, hi in spans]
    return out


_hunks = {}


def changed(path):
    if path not in _hunks:
        diff = sh("git", "diff", "-U0", BUNDLE_HEAD, "--", path)
        _hunks[path] = [(int(a), int(a) + max(int(b or 1), 1) - 1) for a, b in re.findall(r"^@@ -\S+ \+(\d+)(?:,(\d+))? @@", diff, re.M)]
    return _hunks[path]


def find(anchor, files):
    cands = [anchor] + [p.strip() for p in anchor.split(";") if len(p.strip()) >= 12] + [anchor[:60], anchor[:40]]
    for c in cands:
        if c:
            out = sh("rg", "-F", "-n", "--no-heading", "--with-filename", "--", c, *files)
            if out:
                return [":".join(l.split(":", 2)[:2]) for l in out.splitlines()[:3]]
    for t in [t for t in anchor.replace("(", " ").replace(")", " ").split() if len(t) >= 6][:3]:
        out = sh("rg", "-F", "-n", "--no-heading", "--with-filename", "--", t, *files)
        if out:
            return [":".join(l.split(":", 2)[:2]) for l in out.splitlines()[:3]]
    return []


rows = []
for v in V:
    fid, f = v["fid"], F[v["fid"]]
    evs, bad, chg = ev_lines(v["evidence"]), [], False
    for p, lo, hi in evs:
        fp = Path(p)
        if not fp.is_file():  # evidence sometimes uses a short path: try the finding's dirs, then a unique repo match
            near = [Path(x.strip()).parent / p for x in f["file"].split(",") if x.strip()]
            near += [Path(x.strip()) for x in f["file"].split(",") if x.strip().endswith("/" + p)]
            hit = [h for h in sh("git", "ls-files", f"*/{p}").split() if "/_ref/" not in h]
            fp = next((n for n in near if n.is_file()), Path(hit[0]) if len(hit) == 1 else fp)
        if not fp.is_file():
            bad.append(f"{p}:nofile")
            continue
        n = sum(1 for _ in open(fp, errors="replace"))
        if hi > n:
            bad.append(f"{p}:{hi}>{n}")
        if any(a <= hi and lo <= b for a, b in changed(str(fp))):
            chg = True
    files = [x.strip() for x in f["file"].split(",") if x.strip() and Path(x.strip()).is_file()]
    g1 = A.get(fid, {})
    anchor = (f.get("anchor") or "").strip().replace("\\\"", "\"")
    if g1.get("found") in ("NOT-FOUND", "no-file", "no-anchor", None) or not files or anchor in ("", "-"):
        now, drift = [], "noanchor"
    else:
        now = find(anchor, files)
        g1w = set(g1["where"].split(","))
        drift = "gone" if not now else ("none" if g1w & set(now) else "moved")
    rows.append({"fid": fid, "head_sha": head, "verdict": v["verdict"],
                 "evidence_ok": "yes" if evs and not bad else ("noref" if not evs else "no:" + ";".join(bad)),
                 "ev_changed": "yes" if chg else "no", "anchor_now": ",".join(now) or "-", "drift": drift,
                 "dirty": "yes" if any(x in dirty for x in [f["file"]] + [p for p, _, _ in evs]) else "no"})

cols = ["fid", "head_sha", "verdict", "evidence_ok", "ev_changed", "anchor_now", "drift", "dirty"]
with open(D / "reanchor.tsv", "w", newline="") as fh:
    w = csv.DictWriter(fh, cols, delimiter="\t", quoting=csv.QUOTE_NONE, escapechar="\\", lineterminator="\n")
    w.writeheader()
    w.writerows(rows)
print("rows", len(rows), "head", head)
for k in ("drift", "dirty", "ev_changed"):
    print(k, dict(Counter(r[k] for r in rows)))
print("evidence_ok", dict(Counter(r["evidence_ok"].split(":")[0] for r in rows)))
for r in rows:
    if r["drift"] in ("gone", "moved") or r["evidence_ok"].startswith("no") or r["ev_changed"] == "yes":
        print(" ", r["fid"], r["verdict"], r["drift"], r["evidence_ok"], "chg=" + r["ev_changed"], r["anchor_now"])
