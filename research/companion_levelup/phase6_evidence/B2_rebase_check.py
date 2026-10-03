#!/usr/bin/env python3
"""B2: early-conversion re-base proof (Phase 6). Run from research/companion_levelup:  python3 phase6_evidence/B2_rebase_check.py

For EVERY preset with hold_class != 0, EVERY conversion level L (L base-class levels when the story converts; L from the utc start level =
first record's class level - 1 (min 1) up to convert_at-1 = the on-time conversion, as control), hold ON and OFF:
the engine loop is driven level by level to character level 30, each decision taken from the REAL C++ levelNow (tools/parity parity_bin, all
scenarios of one step batched per subprocess call; the applied record's class level advances the newest class like the engine does).
Checks: (1) Jedi class levels 1..30-L each get a PATH/REBASE decision on the preset's Jedi record (class level j) - no HOLD, no VANILLA
while a record exists; (2) the Jedi record set applied is identical to on-time, base-class records beyond L are the only ones skipped
(reported as inherent: both classes cannot be levelled in the same slot) and any feat/power in a skipped record that no applied record
also grants is listed; feat/power prerequisites (feat.2da / spells.2da snapshot, advisory) are compared against the on-time baseline;
Jedi picks applied at an EARLIER character level than designed are checked against the min character level (feat mincharlevel, spells.2da
class column); (3) no HOLD once the creature has the hold class. The Python twin level_now is compared with C++ at every step.
Exit 1 on any failure of (1)/(3)/(2: Jedi record lost) or twin mismatch.
"""
import os, re, subprocess, sys, tomllib
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, ROOT)
sys.path.insert(0, os.path.join(ROOT, "tools", "parity"))
import simulate_rewrite as S
import parity as P

TWODA = "/home/ethansm/Games/KOTOR2_Mod_Workspace/replay/scratch/override"   # offline snapshot (NOT the Steam install)
PWR_COL = {3: "guardian", 4: "consular", 5: "sentinel"}
OUT = os.path.join(HERE, "B2_rebase_results.txt")


def load_2da():
    try:
        from pykotor.resource.formats.twoda import read_2da
        import logging
        logging.disable(logging.CRITICAL)
        return {n: read_2da(open(os.path.join(TWODA, n + ".2da"), "rb").read()) for n in ("feat", "spells")}
    except Exception as e:   # advisory only
        return None


def cell(t, row, col):
    try:
        return t.get_row(row).get_string(col)
    except Exception:
        return ""


def num(s, d=0):
    try:
        return int(s)
    except (ValueError, TypeError):
        return d


def feat_prereq_ok(T, f, known):
    g = lambda c: num(cell(T["feat"], f, c))
    req = [x for x in (g("prereqfeat1"), g("prereqfeat2")) if x > 0]
    ors = [x for x in (num(cell(T["feat"], f, "orreqfeat%d" % k), -1) for k in range(5)) if x > 0]
    return all(x in known for x in req) and (not ors or any(x in known for x in ors))


def power_prereq_ok(T, p, known):
    v = cell(T["spells"], p, "prerequisites")
    return all(int(x) in known for x in v.split("_") if x.isdigit())


_SRC = None


def src_line(pid):
    """preset id (tag__variant, see gen_runtime_table.py) -> repo-relative <toml>:<line of convert_at>."""
    global _SRC
    if _SRC is None:
        import glob
        _SRC = {}
        for f in glob.glob(os.path.join(ROOT, "paths", "presets", "*.toml")) + glob.glob(os.path.join(ROOT, "paths", "*.toml")):
            try:
                m = tomllib.load(open(f, "rb"))["meta"]
                _SRC[(m["tag"].lower() + "__" + str(m.get("variant", "live")).lower())] = f
            except Exception:
                pass
    path = _SRC.get(pid, "?")
    rel = os.path.relpath(path, os.path.dirname(os.path.dirname(ROOT))) if path != "?" else "?"
    try:
        for n, l in enumerate(open(path), 1):
            if re.match(r"\s*convert_at\s*=", l):
                return "%s:%d" % (rel, n)
    except OSError:
        pass
    return rel


def run_cpp(states, hold):
    text = "".join("L %s %d %d %d %s\n" % (pid, key, int(hold), len(cl), ",".join("%d:%d" % c for c in cl)) for pid, key, cl in states)
    r = subprocess.run([P.EXE, P.INI], input=text, capture_output=True, text=True, check=True)
    return r.stdout.splitlines()


def main():
    P.build(P.EXE, "parity_main.cpp")
    cs, presets, recs = S.parse_table_v2(open(P.INI).read())
    T = load_2da()
    fails, notes, rows = [], [], []
    total_cases = 0
    decs = {}
    scen = []   # one per (preset, L, hold)
    meta = {}
    for pid, lv in sorted(recs.items()):
        hc = presets[pid].hold_class if pid in presets else 0
        if not lv or not hc:
            continue
        base = lv[min(lv)].cls
        jed = {cl: r for cl, r in lv.items() if r.cls != base}
        dummy = base == hc   # path starts in the Jedi class (companion already a Jedi-to-be at the first record): pre-conversion class unknown, use 99
        if dummy:
            base, jed = 99, dict(lv)
        conv = min(jed)
        if any(r.cls != hc for r in jed.values()):
            fails.append("%s: Jedi records class != hold_class %d" % (pid, hc))
        if presets[pid].convert_at not in (0, conv):
            fails.append("%s: convert_at %d != first Jedi record char level %d" % (pid, presets[pid].convert_at, conv))
        if any(r.clvl != cl for cl, r in lv.items() if r.cls == base):
            fails.append("%s: base class level != char level" % pid)
        if sorted(r.clvl for r in jed.values()) != list(range(1, len(jed) + 1)):
            fails.append("%s: Jedi class levels not 1..n contiguous" % pid)
        lo = max(1, (conv if dummy else lv[min(lv)].clvl) - 1)
        meta[pid] = dict(dummy=dummy, hc=hc, base=base, conv=conv, jed=jed, lo=lo, maxcl=max(lv))
        for L in range(lo, conv):
            for hold in (True, False):
                scen.append(dict(pid=pid, L=L, hold=hold, classes=[(base, L), (hc, 0)], applied=[], hold_hit=False, done=False))
    # lockstep engine loop
    for step in range(0, 30):
        for hold in (True, False):
            live = [s for s in scen if s["hold"] == hold and not s["done"] and sum(c[1] for c in s["classes"]) < 30]
            if not live:
                continue
            states = [(s["pid"], sum(c[1] for c in s["classes"]) + 1, s["classes"]) for s in live]
            out = run_cpp(states, hold)
            assert len(out) == len(live)
            for s, (pid, key, cl), line in zip(live, states, out):
                d, c = line.split()
                total_cases += 1
                pd, pc, prec = S.level_now(cl, recs[pid], key, hold, meta[pid]["hc"])
                if (pd, "-" if pc is None else str(pc)) != (d, c):
                    fails.append("%s L%d: twin mismatch at key %d: C++ %s vs Py %s" % (pid, s["L"], key, line, pd))
                decs[d] = decs.get(d, 0) + 1
                j = s["classes"][-1][1] + 1
                s["classes"] = [list(x) for x in s["classes"]]
                s["trace"] = s.get("trace", []) + [(key, d, None if c == "-" else int(c), j)]
                if d == "HOLD":
                    fails.append("%s conv@%d hold=%s: HOLD at key %d with hold class present (3)" % (pid, s["L"], hold, key))
                    s["done"] = True
                    continue
                if d in ("PATH", "REBASE"):
                    s["applied"].append((key, int(c), lv_rec(recs, pid, int(c))))
                s["classes"][-1][1] += 1
                s["classes"] = [tuple(x) for x in s["classes"]]
    # analyse
    by = {}
    for s in scen:
        by.setdefault(s["pid"], []).append(s)
    lines = ["B2 early-conversion re-base check (C++ levelNow oracle; Python twin compared at every step)", ""]
    n_lost_jedi = 0
    for pid, m in meta.items():
        jed, lv, hc = m["jed"], recs[pid], m["hc"]
        maxj = len(jed)
        ontime_char = {cl for cl in lv}
        # on-time item sets from the preset's own records
        def items(rs):
            f, p = set(), set()
            for r in rs:
                f |= set(r.feats); p |= set(r.powers)
            return f, p
        ot_f, ot_p = items(lv.values())
        pre_fail = []
        rec_sc = by[pid]
        agg = dict(PATH=0, REBASE=0, VANILLA_tail=0)
        lostrows, lvlrows = [], []
        for s in rec_sc:
            L = s["L"]
            tr = s["trace"]
            jappl = [(key, cl, r) for key, cl, r in s["applied"] if r.cls == hc]
            got = [r.clvl for _, _, r in jappl]
            exp = list(range(1, min(maxj, 30 - L) + 1))
            for key, d, cl, j in tr:
                if d in ("PATH", "REBASE"):
                    agg[d] += 1
                elif j > maxj:
                    agg["VANILLA_tail"] += 1
                else:
                    fails.append("%s conv@%d hold=%s: %s at key %d with Jedi record clvl %d available (1)" % (pid, L, s["hold"], d, key, j))
            if got != exp:
                fails.append("%s conv@%d hold=%s: Jedi class levels applied %s != expected %s (2)" % (pid, L, s["hold"], got, exp))
                n_lost_jedi += 1
            if len(tr) != 30 - L:
                fails.append("%s conv@%d: trace length %d != %d" % (pid, L, len(tr), 30 - L))
            if not s["hold"]:
                continue
            # item sets
            appl_recs = [lv[cl] for cl in sorted(lv) if lv[cl].cls == m["base"] and lv[cl].clvl <= L] + [r for _, _, r in s["applied"]]
            ef, ep = items(appl_recs)
            lost_f, lost_p = ot_f - ef, ot_p - ep
            skipped = [cl for cl in sorted(lv) if lv[cl].cls == m["base"] and lv[cl].clvl > L]
            jf, jp = items(jed.values())
            if (jf & lost_f) or (jp & lost_p):
                fails.append("%s conv@%d: Jedi-record items lost" % (pid, L))
            if lost_f or lost_p or skipped:
                lostrows.append((L, skipped, sorted(lost_f), sorted(lost_p)))
            # advisory prereq / min char level
            if T:
                def prereq_fail(seq):
                    kf = set(); kp = set(S.ENGINE_GRANT_POWERS); bad = set()
                    for r in seq:
                        kf |= set(r.granted)
                        for f in r.feats:
                            if not feat_prereq_ok(T, f, kf):
                                bad.add(("feat", f))
                            kf.add(f)
                        for p in r.powers:
                            if not power_prereq_ok(T, p, kp):
                                bad.add(("power", p))
                            kp.add(p)
                    return bad
                base_bad = prereq_fail([lv[cl] for cl in sorted(lv)])
                new_bad = prereq_fail(appl_recs) - base_bad
                for kind, x in sorted(new_bad):
                    lvlrows.append((L, "prereq-lost", x, "(%s id)" % kind))
                for key, cl, r in jappl:
                    for f in r.feats:
                        mc = num(cell(T["feat"], f, "mincharlevel"))
                        if mc > key and mc <= cl:
                            lvlrows.append((L, "feat-mincharlevel", f, "needs char %d, applied at %d (designed %d)" % (mc, key, cl)))
                    for p in r.powers:
                        mc = num(cell(T["spells"], p, PWR_COL.get(hc, "sentinel")), -1)
                        if 0 < mc <= 60 and mc > key and mc <= cl:
                            lvlrows.append((L, "power-mincharlevel", p, "needs char %d, applied at %d (designed %d)" % (mc, key, cl)))
        Ls = sorted({s["L"] for s in rec_sc})
        lines.append("%s  [%s]  hold_class=%d convert_at(on-time)=%d  conversion levels %d..%d (%d, x2 hold on/off)  Jedi records=%d  PATH=%d REBASE=%d VANILLA(after last Jedi record)=%d"
                     % (pid, src_line(pid), hc, m["conv"], Ls[0], Ls[-1], len(Ls), maxj, agg["PATH"], agg["REBASE"], agg["VANILLA_tail"]))
        seen = {}
        for L, skipped, lf, lp in lostrows:
            if L == Ls[-1]:
                continue
            seen[L] = (skipped, lf, lp)
        if seen:
            # compress: report the worst case (earliest conversion) and the full per-L counts
            L0 = min(seen)
            lines.append("    base-class records skipped (inherent): conv@%d skips char levels %s; items granted ONLY by skipped base records: feats %s powers %s"
                         % (L0, seen[L0][0], seen[L0][1], seen[L0][2]))
            lines.append("    per-L lost feat/power counts: " + ", ".join("%d:%d/%d" % (L, len(v[1]), len(v[2])) for L, v in sorted(seen.items())))
        agg2 = {}
        for L, kind, x, why in lvlrows:
            agg2.setdefault((kind, x, (re.sub(r"needs char (\d+), applied at \d+ \(designed (\d+)\)", r"needs char \1, designed at \2", why))), []).append(L)
        for (kind, x, why), Lset in sorted(agg2.items(), key=lambda kv: (kv[0][0], kv[0][1])):
            lines.append("    ADVISORY %s %s %s: conv@%d..%d (%d levels)" % (kind, x, why, min(Lset), max(Lset), len(set(Lset))))
        rows.append((pid, len(Ls), agg))
    lines.append("")
    lines.append("presets covered: %d; (preset, conversion level, hold) scenarios: %d; C++ levelNow calls: %d" % (len(meta), len(scen), total_cases))
    lines.append("decisions: " + ", ".join("%s %d" % kv for kv in sorted(decs.items())))
    lines.append("2da snapshot for prereq/min-level advisories: " + ("yes (%s)" % TWODA if T else "unavailable (skipped)"))
    lines.append("FAILURES: %d" % len(fails))
    lines += ["  " + f for f in fails]
    text = "\n".join(lines) + "\n"
    open(OUT, "w").write(text)
    print(text[-1500:])
    sys.exit(1 if fails else 0)


def lv_rec(recs, pid, cl):
    return recs[pid][cl]


def jed_items(jed):
    out = set()
    for r in jed.values():
        out |= set(r.feats) | set(r.powers)
    return out


if __name__ == "__main__":
    main()
