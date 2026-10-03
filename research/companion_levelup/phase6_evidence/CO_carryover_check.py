#!/usr/bin/env python3
"""CO check: re-runs the B2 early-conversion matrix (34 presets x every conversion level, hold on/off) with BASELINE compute_rewrite and
with the CARRY-OVER prototype, audits both with B2's independent 2da checks, proves on-time byte identity.
Run from research/companion_levelup:  ../../.venv/bin/python phase6_evidence/CO_carryover_check.py   (writes CO_carryover_results.txt)

Decisions come from simulate_rewrite.level_now (Python twin; proven equal to the C++ levelNow in B2/parity).
The rule under test sees only the PARSED generated rule text (parse_rules_text(build_rules_text(..))), the audit reads feat.2da/spells.2da
directly through B2's feat_prereq_ok / power_prereq_ok, so a table-generation bug cannot hide behind itself.
"""
import os, random, sys, tomllib, glob
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, ROOT)
sys.path.insert(0, HERE)
import simulate_rewrite as S
import CO_carryover_proto as C
import B2_rebase_check as B2
OUT = os.path.join(HERE, "CO_carryover_results.txt")
PWR_COL = C.POWER_COL
Z = [0] * 8


def utc_known():
    """preset id -> (feat set, power set) of the preset's starting utc (live_utc/*.utc)."""
    import logging
    logging.disable(logging.CRITICAL)
    from pykotor.resource.generics.utc import read_utc
    out = {}
    for f in glob.glob(os.path.join(ROOT, "paths", "presets", "*.toml")) + glob.glob(os.path.join(ROOT, "paths", "*.toml")):
        m = tomllib.load(open(f, "rb"))["meta"]
        pid = m["tag"].lower() + "__" + str(m.get("variant", "live")).lower()
        u = read_utc(open(os.path.join(ROOT, "live_utc", m["utc"]), "rb").read())
        fs = {x for x in getattr(u, "feats", [])}
        ps = {sp for c in u.classes for sp in getattr(c, "powers", [])}
        out[pid] = (fs, ps)
    return out


def apply_rec(mode, rec, cl, key, recs, known, R, cs):
    pend = dict(cls=rec.cls, feats=list(rec.granted), spells=[], deltas=list(Z), unspent=60, ability=6)
    kf, kp = known
    if mode == "base":
        res = S.compute_rewrite(rec, pend, cs)
    else:
        res = C.carry_rewrite(rec, cl, key, recs, pend, kf, kp, R, cs)
    kf.update(res["feats"]); kp.update(res["spells"])
    return pend, res


def run(mode, pid, recs, hc, base, L, hold, init, R, cs):
    """One scenario: base-class records up to class level L (on time), then the engine loop to character level 30. -> events, final known."""
    lv = recs[pid]
    kf, kp = (set(init[0]), set(init[1])) if init else (set(), set())
    kp |= set(S.ENGINE_GRANT_POWERS)
    ev = []
    for cl in sorted(lv):
        r = lv[cl]
        if r.cls == base and r.clvl <= L:
            pend, res = apply_rec(mode, r, cl, cl, lv, (kf, kp), R, cs)
            ev.append((cl, cl, r, pend, res, set(kf), set(kp)))
    classes = [(base, L), (hc, 0)]
    for key in range(L + 1, 31):
        d, cl, r = S.level_now(classes, lv, key, hold, hc)
        if d in ("PATH", "REBASE"):
            before = (set(kf), set(kp))
            pend, res = apply_rec(mode, r, cl, key, lv, (kf, kp), R, cs)
            ev.append((key, cl, r, pend, res, before[0], before[1]))
        classes = [tuple(c) for c in classes[:-1]] + [(classes[-1][0], classes[-1][1] + 1)]
    return ev, (kf, kp)


def design_bad(pid, recs, init, T):
    """(feat prereq-bad ids, power prereq-bad ids) of the on-time design sequence (B2 'baseline'), same initial known as the run."""
    kf = set(init[0]) if init else set()
    kp = set(S.ENGINE_GRANT_POWERS) | (set(init[1]) if init else set())
    bf, bp = set(), set()
    for cl in sorted(recs[pid]):
        r = recs[pid][cl]
        kf |= set(r.granted)
        for f in r.feats:
            if not B2.feat_prereq_ok(T, f, kf):
                bf.add(f)
            kf.add(f)
        for p in r.powers:
            if not B2.power_prereq_ok(T, p, kp):
                bp.add(p)
            kp.add(p)
    return bf, bp


def audit(ev, T, bf, bp):
    """-> dict of violation rows: 'prereq' / 'minlevel' = B2 metric (new vs design / design-legal); abs_* = absolute (illegal at placement)."""
    rows = dict(prereq=[], minlevel=[], abs_prereq=[], abs_minlevel=[], abs_class=[], plain_abs=0)
    for key, cl, r, pend, res, kf0, kp0 in ev:
        n0 = {k: len(rows[k]) for k in ('abs_prereq', 'abs_minlevel', 'abs_class')}
        plain = res.get('info') in (None, 'ontime', 'n/a')
        kf = set(kf0) | set(res["feats"][:len(r.granted)]) | set(r.granted)
        kp = set(kp0)
        picks_f = res["feats"][len([f for f in pend["feats"] if f in r.granted]):]
        for f in picks_f:
            mc = B2.num(B2.cell(T["feat"], f, "mincharlevel"))
            if not B2.feat_prereq_ok(T, f, kf):
                rows["abs_prereq"].append(("feat", f, key))
                if f not in bf:
                    rows["prereq"].append(("feat", f, key))
            if mc > key:
                rows["abs_minlevel"].append(("feat", f, key, mc))
                if mc <= cl:
                    rows["minlevel"].append(("feat", f, key, mc))
            if C.FEAT_ABBR.get(r.cls) and B2.cell(T["feat"], f, C.FEAT_ABBR[r.cls] + "_list") not in ("0", "1"):
                rows["abs_class"].append(("feat", f, key))
            kf.add(f)
        for p in res["spells"]:
            if p in S.ENGINE_GRANT_POWERS or p in kp0:
                kp.add(p)
                continue
            mc = B2.num(B2.cell(T["spells"], p, PWR_COL.get(r.cls, "sentinel")), -1)
            mc = mc if 0 <= mc <= 60 else 99
            if not B2.power_prereq_ok(T, p, kp):
                rows["abs_prereq"].append(("power", p, key))
                if p not in bp:
                    rows["prereq"].append(("power", p, key))
            if mc > key:
                rows["abs_minlevel"].append(("power", p, key, mc))
                if mc <= cl:
                    rows["minlevel"].append(("power", p, key, mc))
            kp.add(p)
        if plain and res.get("info") is not None:   # carry run, record emitted as designed: its own design illegality is inherent, not a carry-over effect
            for k in n0:
                rows["plain_abs"] += len(rows[k]) - n0[k]
                del rows[k][n0[k]:]
    return rows


def main():
    T = B2.load_2da()
    cs, presets, recs = S.parse_table_v2(open(S.INI).read())
    rules_text = C.build_rules_text({p: lv for p, lv in recs.items()}, T)
    R = C.parse_rules_text(rules_text)
    utc = utc_known()
    lines, fails = ["CO carry-over check (B2 matrix, Python twin decisions)", ""], []
    lines.append("rule text: %d bytes, %d feat rows, %d power rows" % (len(rules_text), len(R.feat), len(R.power)))
    scen = []
    for pid, lv in sorted(recs.items()):
        hc = presets[pid].hold_class if pid in presets else 0
        if not lv or not hc:
            continue
        base = lv[min(lv)].cls
        jed = {c: r for c, r in lv.items() if r.cls != base}
        dummy = base == hc
        if dummy:
            base, jed = 99, dict(lv)
        conv = min(jed)
        lo = max(1, (conv if dummy else lv[min(lv)].clvl) - 1)
        for L in range(lo, conv):
            scen.append((pid, hc, base, L, conv))
    lines.append("scenarios (preset, conversion level): %d (x hold on/off)" % len(scen))

    summary = {}
    held_sig = {}
    for init_name in ("records", "utc"):
        tot = {m: dict(plain_abs=0, prereq=set(), minlevel=set(), abs_prereq=0, abs_minlevel=0, abs_class=0, ev=0, unfilled=0, deferred=0,
                       lost_jedi=0, lost_base=0, carry_ev=0, blocked=0, extra=0, why={}) for m in ("base", "carry")}
        resid = {}
        for pid, hc, base, L, conv in scen:
            init = utc[pid] if init_name == "utc" else None
            bf, bp = design_bad(pid, recs, init, T)
            for mode in ("base", "carry"):
                for hold in (True, False):
                    ev, (kf, kp) = run(mode, pid, recs, hc, base, L, hold, init, R, cs)
                    if not hold:
                        if [(e[0], e[4]["feats"], e[4]["spells"]) for e in ev] != held_sig[(mode, pid, L)]:
                            fails.append("hold on/off differ %s %s L%d" % (mode, pid, L))
                        continue
                    held_sig[(mode, pid, L)] = [(e[0], e[4]["feats"], e[4]["spells"]) for e in ev]
                    a = audit(ev, T, bf, bp)
                    t = tot[mode]
                    t["prereq"] |= {(pid, L, k, i) for k, i, _ in a["prereq"]}
                    t["minlevel"] |= {(pid, L, k, i) for k, i, *_ in a["minlevel"]}
                    t["abs_prereq"] += len(a["abs_prereq"]); t["abs_minlevel"] += len(a["abs_minlevel"]); t["abs_class"] += len(a["abs_class"])
                    t["ev"] += len(ev)
                    t["plain_abs"] += a["plain_abs"]
                    for e in ev:
                        res = e[4]
                        if res.get("info") == "carry":
                            t["carry_ev"] += 1
                            t["unfilled"] += sum(res["unfilled"]); t["deferred"] += sum(res["deferred"]); t["blocked"] += sum(res["blocked"]); t["extra"] += sum(res["extra"])
                    # items lost at the end: planned (any record) but never obtained
                    plan_j = {x for c, r in recs[pid].items() if r.cls == hc for x in r.feats + r.powers}
                    plan_b = {x for c, r in recs[pid].items() if r.cls != hc for x in r.feats + r.powers}
                    end = kf | kp
                    t["lost_jedi"] += len(plan_j - end)
                    t["lost_base"] += len(plan_b - plan_j - end)
                    if mode == "carry":
                        for kind, plan, known, fn in (("feat", {x for r in recs[pid].values() for x in r.feats}, kf, C._feat_state),
                                                      ("power", {x for r in recs[pid].values() for x in r.powers}, kp, C._power_state)):
                            fn = C._feat_state if kind == "feat" else C._power_state

                            def reach(y, d=0):   # obtainable at char level 30 by itself or through a chain of legal prerequisites
                                st, miss = fn(R, y, hc, 30, known)
                                return st in ("ok", "known") or (st == "prereq" and d < 4 and all(reach(m, d + 1) for m in miss))
                            for x in plan - known:
                                st, miss = fn(R, x, hc, 30, known)
                                if st == "prereq" and reach(x):
                                    st = "ok"
                                k2 = "%s: %s" % (kind, {"ok": "legal by level 30 but no slot left (records exhausted)", "class": "not choosable by the Jedi class",
                                                        "level": "min level > 30", "prereq": "prerequisite not obtainable (not choosable)", "norule": "no rule row"}[st])
                                t["why"][k2] = t["why"].get(k2, 0) + 1
                        for k, i, *rest in a["abs_prereq"] + a["abs_minlevel"] + a["abs_class"]:
                            resid.setdefault((pid, L, k, i), []).append(rest)
        summary[init_name] = (tot, resid)
        lines.append("")
        lines.append("== initial creature state: %s ==" % ("starting utc feats/powers" if init_name == "utc" else "empty (B2 convention)"))
        for m in ("base", "carry"):
            t = tot[m]
            lines.append("%-6s prereq-lost rows %d (%d distinct preset/feat-or-power) | min-level rows %d | absolute: prereq %d min-level %d not-choosable %d (+%d inherent in as-designed records) | "
                         "events %d (carry-mode %d, unfilled slots %d of which blocked-with-owed-items %d, prereq-first picks %d, of which not in the plan %d) | items never obtained: Jedi-record %d, base-only %d"
                         % ("BEFORE" if m == "base" else "AFTER", len(t["prereq"]), len({(a, c, d) for a, b, c, d in t["prereq"]}), len(t["minlevel"]),
                            t["abs_prereq"], t["abs_minlevel"], t["abs_class"], t["plain_abs"], t["ev"], t["carry_ev"], t["unfilled"], t["blocked"], t["deferred"], t["extra"],
                            t["lost_jedi"], t["lost_base"]))
        lines.append("  items never obtained after carry-over, by reason (summed over scenarios): " + "; ".join("%s=%d" % kv for kv in sorted(tot["carry"]["why"].items())))
        if tot["carry"]["prereq"] or tot["carry"]["minlevel"]:
            fails.append("%s: carry-over leaves B2-metric violations" % init_name)
        if tot["carry"]["abs_prereq"] or tot["carry"]["abs_minlevel"] or tot["carry"]["abs_class"]:
            lines.append("  RESIDUAL (absolute) violations after carry-over, by (preset, conversion level, kind, id):")
            for k, v in sorted(resid.items())[:60]:
                lines.append("    %s L%d %s %d: %s" % (k + (v[0],)))
        else:
            lines.append("  residual violations after carry-over: none")
    lines.append("")

    # ---- on-time byte identity
    n_walk = n_fuzz = 0
    rnd = random.Random(7)
    for pid, lv in sorted(recs.items()):
        for init_name in ("records", "utc"):
            init = utc.get(pid) if init_name == "utc" else None
            kf, kp = (set(init[0]), set(init[1])) if init else (set(), set())
            kp |= set(S.ENGINE_GRANT_POWERS)
            for cl in sorted(lv):
                r = lv[cl]
                # the exact state the creature has when it reaches this record on time: sets BEFORE applying
                for trial in range(1 + 6):
                    if trial == 0:
                        pend = dict(cls=r.cls, feats=list(r.granted), spells=[], deltas=list(Z), unspent=60, ability=6)
                    else:
                        pend = dict(cls=r.cls, feats=rnd.sample(list(r.granted) + [rnd.randrange(0, 250) for _ in range(6)], k=min(5, len(r.granted) + 3)),
                                    spells=[rnd.choice(sorted(S.ENGINE_GRANT_POWERS)), rnd.randrange(0, 286)],
                                    deltas=[rnd.randrange(0, 3) for _ in range(8)], unspent=rnd.randrange(0, 40), ability=rnd.randrange(0, 7))
                    a = S.compute_rewrite(r, pend, cs)
                    b = C.carry_rewrite(r, cl, cl, lv, pend, set(kf), set(kp), R, cs)
                    n_fuzz += 1
                    if S.format_dry(pid, cl, a) != S.format_dry(pid, cl, b) or b.get("info") not in ("ontime", "n/a"):
                        fails.append("on-time mismatch %s cl%d trial %d (%s)" % (pid, cl, trial, init_name))
                n_walk += 1
                pend = dict(cls=r.cls, feats=list(r.granted), spells=[], deltas=list(Z), unspent=60, ability=6)
                res = S.compute_rewrite(r, pend, cs)
                kf.update(res["feats"]); kp.update(res["spells"])
    lines.append("ON-TIME byte identity: %d records walked in design order x2 initial states, %d format_dry comparisons (1 clean + 6 random pending lists each), "
                 "all with info=ontime: %s" % (n_walk, n_fuzz, "IDENTICAL" if not [f for f in fails if f.startswith("on-time")] else "MISMATCH"))
    # control scenarios (on-time conversion, L = convert_at-1) in the full matrix
    ctrl_bad = 0
    for pid, hc, base, L, conv in scen:
        if L != conv - 1:
            continue
        for init_name in ("records", "utc"):
            init = utc[pid] if init_name == "utc" else None
            e1, _ = run("base", pid, recs, hc, base, L, True, init, R, cs)
            e2, _ = run("carry", pid, recs, hc, base, L, True, init, R, cs)
            sig = lambda ev: [S.format_dry(pid, e[0], e[4]) for e in ev]
            ctrl_bad += sig(e1) != sig(e2) or any(e[4].get("info") == "carry" for e in e2 if e[0] == e[1])
    lines.append("on-time conversion scenarios (L = convert_at-1, full engine loop to 30, both initial states): %s" % ("IDENTICAL" if not ctrl_bad else "%d DIFFER" % ctrl_bad))
    if ctrl_bad:
        fails.append("control scenarios differ")
    lines.append("")
    lines.append("FAILURES: %d" % len(fails))
    lines += ["  " + f for f in fails]
    text = "\n".join(lines) + "\n"
    open(OUT, "w").write(text)
    print(text)
    sys.exit(1 if fails else 0)


if __name__ == "__main__":
    main()
