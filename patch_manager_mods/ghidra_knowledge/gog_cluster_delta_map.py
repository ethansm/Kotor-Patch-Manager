#!/usr/bin/env python3
"""Map GOG function names to Steam addresses without Ghidra, by cluster delta + drift.

Inputs: the framework's kotor2_gog_aspyr.db (names, GOG addresses, param_size_bytes) and
steam_functions.tsv (every Steam function start with its `ret N`).

Phase 1 (vote): for each contiguous run of a class's GOG functions (gap > GAP), every
member votes for delta = (Steam start with ret N == GOG param size) - GOG address. The top
delta is accepted if it has >= MIN_VOTES votes and strictly more than the runner-up; its
members that land on a matching start are tagged `vote`. Repeat on the rest (peeling).

Phase 2 (drift): Aspyr edits shift the code after them by a small amount, so for each
still-unmapped member take the delta of the nearest mapped member of the same run and look
for a Steam start with the same ret N within +-DRIFT of that; take the closest. Tagged
`drift+0xNN`. Keep order: the result must lie between its mapped neighbours.

Calibrated 2026-09-30 against addresses already confirmed by decompile (see --selftest).
Still decompile-confirm one anchor per run before hooking anything; `drift` rows are
hypotheses, `vote` rows are strong. ret N 0 matches many functions, so drift rows with
ret N 0 are weakest.

Usage: gog_cluster_delta_map.py CLASS_REGEX [...] [--out file.tsv] | --selftest
"""
import argparse, bisect, collections, re, sqlite3, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
GOG_DB = Path.home() / "Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/AddressDatabases/kotor2_gog_aspyr.db"
GAP = 0x3000
MIN_VOTES = 3
DRIFT = 0x600

# (class::name, steam address) confirmed by decompile in earlier sessions
KNOWN = [
    ("CGameEffect::SetExpiryTime", 0x005e4f80), ("CGameEffect::GetExpiryTime", 0x005e4fa0),
    ("CExoResMan::Constructor", 0x00711360), ("CSWGuiMainInterface::HitCheckMouse", 0x0074b3a0),
    ("CSWGuiInGameInventory::Constructor", 0x008a6170), ("CSWGuiInGameInventory::PopulateItemListBox", 0x008a7ba0),
    ("CSWGuiInGameInventory::CreateItemEntry", 0x008a75f0), ("CSWGuiInGameInventory::OnPanelAdded", 0x008a74c0),
    ("CSWGuiInGameInventory::OnPanelRemoved", 0x008a7520), ("CSWGuiInGameCharacter::RotateCharacter", 0x008500f0),
]


def load_steam():
    starts = {}
    for line in (HERE / "steam_functions.tsv").read_text().splitlines()[1:]:
        f = line.split("\t")
        starts[int(f[0], 16)] = int(f[2])
    return starts


def runs_of(rows):
    cur = []
    for r in sorted(rows, key=lambda r: r[2]):
        if cur and r[2] - cur[-1][2] > GAP:
            yield cur
            cur = []
        cur.append(r)
    if cur:
        yield cur


def map_run(run, starts, by_retn, sorted_starts, log, seeds=()):
    mapped = {}  # gog addr -> (steam addr, tag)
    rest = run
    if seeds:
        # Phase 1b (seeded): deltas already proven elsewhere; keep the one that lands
        # the most members exactly (ret N match), if it lands >= 2 and >= 40% of the run.
        best = max(seeds, key=lambda d: sum(starts.get(g + d) == p for _, _, g, p in run))
        took = [r for r in run if starts.get(r[2] + best) == r[3]]
        ok = len(took) >= max(2, 0.4 * len(run))
        log(f"#   seeded delta={best:+#x} took={len(took)}/{len(run)} {'OK' if ok else 'STOP'}")
        if ok:
            for r in took:
                mapped[r[2]] = (r[2] + best, "seeded")
            rest = []
    while len(rest) >= 2:
        score = collections.Counter()
        for (_, _, g, p) in rest:
            for s in by_retn.get(p, ()):
                score[s - g] += 1
        if not score:
            break
        ranked = score.most_common(2)
        d, hits = ranked[0]
        second = ranked[1][1] if len(ranked) > 1 else 0
        took = [r for r in rest if starts.get(r[2] + d) == r[3]]
        ok = hits >= MIN_VOTES and hits > second
        log(f"#   vote delta={d:+#x} votes={hits} runner-up={second} took={len(took)} {'OK' if ok else 'STOP'}")
        if not ok:
            break
        for r in took:
            mapped[r[2]] = (r[2] + d, "vote")
        rest = [r for r in rest if r[2] not in mapped]
    # drift fill, nearest mapped neighbour first
    for _ in range(len(run)):
        progress = False
        for i, (c, n, g, p) in enumerate(run):
            if g in mapped:
                continue
            nb = [j for j in range(len(run)) if run[j][2] in mapped]
            if not nb:
                break
            j = min(nb, key=lambda j: abs(j - i))
            d0 = mapped[run[j][2]][0] - run[j][2]
            lo_bound = max([mapped[run[k][2]][0] for k in nb if k < i], default=0)
            hi_bound = min([mapped[run[k][2]][0] for k in nb if k > i], default=1 << 32)
            lo = bisect.bisect_left(sorted_starts, g + d0 - DRIFT)
            hi = bisect.bisect_right(sorted_starts, g + d0 + DRIFT)
            cands = [s for s in sorted_starts[lo:hi] if starts[s] == p and lo_bound < s < hi_bound]
            if cands:
                s = min(cands, key=lambda s: abs(s - (g + d0)))
                mapped[g] = (s, f"drift{s - (g + d0):+#x}")
                progress = True
        if not progress:
            break
    return mapped


def map_classes(patterns, log=print):
    starts = load_steam()
    by_retn = collections.defaultdict(list)
    for a, rn in starts.items():
        by_retn[rn].append(a)
    sorted_starts = sorted(starts)
    db = sqlite3.connect(GOG_DB)
    rows = [r for r in db.execute("select class_name, function_name, address, param_size_bytes from functions")
            if r[3] is not None]
    gog_addr = {f"{c}::{n}": g for c, n, g, p in rows}
    proven = {s - gog_addr[k] for k, s in KNOWN if k in gog_addr}
    out, pending = [], []
    for pat in patterns:
        rx = re.compile(pat)
        for run in runs_of([r for r in rows if rx.search(r[0])]):
            log(f"# {pat} gog {run[0][2]:#x}-{run[-1][2]:#x} n={len(run)}")
            mapped = map_run(run, starts, by_retn, sorted_starts, log)
            if not mapped:
                pending.append((pat, run))
                continue
            proven |= {s - g for g, (s, tag) in mapped.items() if tag == "vote"}
            out += emit(run, mapped, log)
    for pat, run in pending:  # second pass: seed with every delta proven so far
        log(f"# {pat} (seeded retry) gog {run[0][2]:#x}-{run[-1][2]:#x} n={len(run)}")
        out += emit(run, map_run(run, starts, by_retn, sorted_starts, log, seeds=proven), log)
    return sorted(out)


def emit(run, mapped, log):
    rows = []
    for c, n, g, p in run:
        if g in mapped:
            s, tag = mapped[g]
            rows.append((s, g, f"{c}::{n}", tag, p))
        else:
            log(f"#   unmapped {c}::{n} gog {g:#x} retn={p}")
    return rows


def selftest():
    classes = sorted({k.split("::")[0] for k, _ in KNOWN})
    got = {n: (s, tag) for s, g, n, tag, p in map_classes([f"^{c}$" for c in classes], log=lambda *_: None)}
    bad = 0
    for name, want in KNOWN:
        s, tag = got.get(name, (None, "-"))
        ok = s == want
        bad += not ok
        print(f"{'PASS' if ok else 'FAIL'} {name} want {want:#x} got {s and hex(s)} ({tag})")
    return bad


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("classes", nargs="*")
    ap.add_argument("--out")
    ap.add_argument("--selftest", action="store_true")
    a = ap.parse_args()
    if a.selftest:
        sys.exit(1 if selftest() else 0)
    out = map_classes(a.classes)
    lines = [f"{s:#010x}\t{g:#x}\t{n}\t{tag}\tretn={p}" for s, g, n, tag, p in out]
    print("\n".join(lines))
    if a.out:
        Path(a.out).write_text("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
