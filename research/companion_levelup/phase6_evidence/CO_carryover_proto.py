#!/usr/bin/env python3
"""CO: "carry-over" rule prototype for the level-up override (Python twin candidate of a future C++ computeRewriteCarry).

Imports simulate_rewrite read-only; nothing else is modified. The rule works on the runtime table's records (all records of the
preset are already in memory in C++) plus a small rule-data block ([featrules]/[powerrules]) emitted by the generator from feat.2da/spells.2da.
This module also contains the generator side (build_rules_text, from the 2da snapshot) and the runtime parser (parse_rules_text), and
carry_rewrite() only ever sees the PARSED text, so it proves the proposed table columns are sufficient.

Run:  ../../.venv/bin/python phase6_evidence/CO_carryover_proto.py   (self-tests; the full matrix is CO_carryover_check.py)
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, ROOT)
import simulate_rewrite as S   # read-only import

TWODA = "/home/ethansm/Games/KOTOR2_Mod_Workspace/replay/scratch/override"   # offline snapshot (same as B2)
# feat.2da <abbr>_list columns per class id (levelup_rules.CLASS_ABBR + classes.2da ids 11-13 = Weapon Master/Master/Watchman -> jwm/jma/jwa)
FEAT_ABBR = {0: "sol", 1: "sct", 2: "scd", 3: "jgd", 4: "jcn", 5: "jsn", 6: "drc", 7: "drx", 9: "tec", 11: "jwm", 12: "jma", 13: "jwa"}
# spells.2da per-class minimum CHARACTER level columns
POWER_COL = {3: "guardian", 4: "consular", 5: "sentinel", 11: "weapmstr", 12: "jedimaster", 13: "watchman"}
NEVER = 99   # runtime min level for "not selectable" (2da 99 / -1 / blank on a Jedi class)


def _num(s, d=0):
    try:
        return int(s)
    except (ValueError, TypeError):
        return d


def load_2da(path=TWODA):
    import logging
    logging.disable(logging.CRITICAL)
    from pykotor.resource.formats.twoda import read_2da
    return {n: read_2da(open(os.path.join(path, n + ".2da"), "rb").read()) for n in ("feat", "spells")}


def _cell(t, row, col):
    try:
        return t.get_row(row).get_string(col)
    except Exception:
        return ""


# ---------------------------------------------------------------- generator side (would live in gen_runtime_table.py)
def build_rules_text(recs_by_preset, T):
    """-> text of the two new sections. recs_by_preset = {preset id: {char level: Rec}} (the generator's own record lists).
    Rows are emitted for every feat/power that appears in any record plus the transitive closure of their prerequisites."""
    classes = sorted({r.cls for lv in recs_by_preset.values() for r in lv.values()})
    feats = {f for lv in recs_by_preset.values() for r in lv.values() for f in r.feats}
    pows = {p for lv in recs_by_preset.values() for r in lv.values() for p in r.powers}

    def feat_row(f):
        g = lambda c: _num(_cell(T["feat"], f, c))
        req = [x for x in (g("prereqfeat1"), g("prereqfeat2")) if x > 0]
        ors = [x for x in (_num(_cell(T["feat"], f, "orreqfeat%d" % k), -1) for k in range(5)) if x > 0]
        sel = [c for c in classes if c in FEAT_ABBR and _cell(T["feat"], f, FEAT_ABBR[c] + "_list") in ("0", "1")]
        return g("mincharlevel"), req, ors, sel

    def pow_row(p):
        pre = [int(x) for x in _cell(T["spells"], p, "prerequisites").split("_") if x.isdigit()]
        mins = []
        for c in classes:
            if c in POWER_COL:
                v = _num(_cell(T["spells"], p, POWER_COL[c]), -1)
                mins.append((c, v if 0 <= v <= 60 else NEVER))   # 0 = no minimum
        return pre, mins

    frows, prows = {}, {}
    todo = list(feats)
    while todo:
        f = todo.pop()
        if f in frows:
            continue
        frows[f] = feat_row(f)
        todo += frows[f][1] + frows[f][2]
    todo = list(pows)
    while todo:
        p = todo.pop()
        if p in prows:
            continue
        prows[p] = pow_row(p)
        todo += prows[p][0]
    out = ["[featrules]   ; f<id>=<mincharlevel>|<req feat ids ,>|<any-of feat ids ,>|<class ids that may pick it ,>"]
    for f in sorted(frows):
        mc, req, ors, sel = frows[f]
        out.append("f%d=%d|%s|%s|%s" % (f, mc, ",".join(map(str, req)), ",".join(map(str, ors)), ",".join(map(str, sel))))
    out.append("[powerrules]  ; p<id>=<prereq power ids ,>|<class:min character level ,>   (99 = never)")
    for p in sorted(prows):
        pre, mins = prows[p]
        out.append("p%d=%s|%s" % (p, ",".join(map(str, pre)), ",".join("%d:%d" % m for m in mins)))
    return "\n".join(out) + "\n"


# ---------------------------------------------------------------- runtime side (C++ parser twin)
class Rules:
    def __init__(self):
        self.feat = {}    # id -> (minchar, req, orreq, selectable class set)
        self.power = {}   # id -> (prereq, {cls: min})


def parse_rules_text(text):
    R, sect = Rules(), None
    for raw in text.splitlines():
        line = raw.split(";", 1)[0].strip()
        if not line:
            continue
        if line.startswith("["):
            sect = line.strip("[]").lower()
            continue
        k, v = line.split("=", 1)
        f = v.split("|")
        ids = lambda s: [int(x) for x in s.split(",")] if s.strip() else []
        if sect == "featrules":
            R.feat[int(k[1:])] = (int(f[0]), ids(f[1]), ids(f[2]), set(ids(f[3])))
        elif sect == "powerrules":
            R.power[int(k[1:])] = (ids(f[0]), {int(a): int(b) for a, b in (x.split(":") for x in f[1].split(",") if x)})
        else:
            raise ValueError("rules text outside a section: " + raw)
    return R


# ---------------------------------------------------------------- the rule
def _feat_state(R, f, cls, key, known):
    """-> ('ok'|'known'|'level'|'class'|'prereq'|'norule', missing prereq ids)."""
    if f in known:
        return "known", []
    row = R.feat.get(f)
    if row is None:
        return "norule", []
    mc, req, ors, sel = row
    if cls not in sel:
        return "class", []
    if mc > key:
        return "level", []
    miss = [x for x in req if x not in known]
    if ors and not any(x in known for x in ors):
        miss += ors
    return ("prereq", miss) if miss else ("ok", [])


def _power_state(R, p, cls, key, known):
    if p in known:
        return "known", []
    row = R.power.get(p)
    if row is None:
        return "norule", []
    pre, mins = row
    mc = mins.get(cls, 0 if cls not in POWER_COL else NEVER)   # non-Jedi class: no gate
    if mc > key:
        return "level", []
    miss = [x for x in pre if x not in known]
    return ("prereq", miss) if miss else ("ok", [])


def _pick(state_fn, owed, cls, key, known, depth=0, plan=()):
    """Earliest owed id that is legal now, or whose missing prerequisite chain ends in a legal owed id (which is returned instead).
    Returns (id_to_take, why) or (None, why-none)."""
    for e in owed:
        if e in known:
            continue
        st, miss = state_fn(e, cls, key, known)
        if st == "ok":
            return e, "plan" if depth == 0 else ("prereq" if e in plan else "prereq_extra")
        if st == "prereq" and depth < 4:
            # take the prerequisite first: it must itself be legal now (or have a legal prerequisite chain); it need NOT be in the plan
            # (e.g. Toughness granted by a skipped base-class level) - that costs one slot, which is the user's rule
            if miss:
                r, _ = _pick(state_fn, miss, cls, key, known, depth + 1, plan)
                if r is not None:
                    return r, "prereq"
    return None, "none"


def carry_rewrite(rec, cl, key, recs, pend, known_f, known_p, R, cs, cs_extra=0):
    """Carry-over version of compute_rewrite (same result dict). recs = {char level: Rec} of the preset, cl = char level of `rec`,
    key = character level being reached, known_f/known_p = ids the creature already has (stateless: read from the creature).
    On-time (key == cl and nothing owed from earlier records) -> exactly compute_rewrite. Extra key 'info' describes the decision."""
    base = S.compute_rewrite(rec, pend, cs, cs_extra)
    if rec is None or rec.cls != pend["cls"]:
        return dict(base, info="n/a")
    owed_f = _owed([r.feats for c, r in sorted(recs.items()) if c < cl], known_f)
    owed_p = _owed([r.powers for c, r in sorted(recs.items()) if c < cl], known_p)
    if key == cl and not owed_f and not owed_p:
        return dict(base, info="ontime")
    kept_f = [f for f in pend["feats"] if f in rec.granted]
    kept_p = [s for s in pend["spells"] if s in S.ENGINE_GRANT_POWERS]
    own_f = _owed([r.feats for c, r in sorted(recs.items()) if c == cl], known_f)   # this record's own items last in plan order
    own_p = _owed([r.powers for c, r in sorted(recs.items()) if c == cl], known_p)
    plan_f, plan_p = owed_f + [f for f in own_f if f not in owed_f], owed_p + [p for p in own_p if p not in owed_p]
    kf = set(known_f) | set(kept_f)
    kp = set(known_p) | set(kept_p) | set(S.ENGINE_GRANT_POWERS)
    feats, spells, why_f, why_p = list(kept_f), list(kept_p), [], []
    for _ in range(len(rec.feats)):
        f, w = _pick(lambda e, c, k, kn: _feat_state(R, e, c, k, kn), plan_f, rec.cls, key, kf, 0, plan_f)
        if f is None:
            break
        feats.append(f); kf.add(f); why_f.append(w)
    for _ in range(len(rec.powers)):
        p, w = _pick(lambda e, c, k, kn: _power_state(R, e, c, k, kn), plan_p, rec.cls, key, kp, 0, plan_p)
        if p is None:
            break
        spells.append(p); kp.add(p); why_p.append(w)
    new = dict(base, feats=feats, spells=spells)
    # base already holds skills/unspent/ability/negative-unspent handling; re-derive status for the changed lists
    if base["status"] == "negative-unspent":
        return dict(base, info="negative-unspent")
    same = all(new[k] == pend[k] for k in ("feats", "spells", "deltas", "unspent", "ability"))
    new["status"] = "already" if same else "rewrite"
    new["info"] = "carry"
    new["unfilled"] = (len(rec.feats) - len(why_f), len(rec.powers) - len(why_p))
    # of those, slots left empty although owed items remain (all blocked: min level / class / unresolved prerequisite) vs nothing owed
    new["blocked"] = (new["unfilled"][0] if any(x not in kf for x in plan_f) else 0, new["unfilled"][1] if any(x not in kp for x in plan_p) else 0)
    new["deferred"] = (sum(1 for w in why_f if w.startswith("prereq")), sum(1 for w in why_p if w.startswith("prereq")))
    new["extra"] = (sum(1 for w in why_f if w == "prereq_extra"), sum(1 for w in why_p if w == "prereq_extra"))
    return new


def _owed(rec_lists, known):
    out = []
    for lst in rec_lists:
        for x in lst:
            if x not in known and x not in out:
                out.append(x)
    return out


# ---------------------------------------------------------------- self-test
def selftest():
    T = load_2da()
    cs, presets, recs = S.parse_table_v2(open(S.INI).read())
    R = parse_rules_text(build_rules_text(recs, T))
    ok = True

    def chk(name, c):
        nonlocal ok
        print(("PASS " if c else "FAIL ") + name)
        ok &= bool(c)
    # synthetic: Two Weapon Advanced (9) needs Two Weapon Fighting (3); slot of a Sentinel record at char 6 wants 9 but creature lacks 3
    rr = S.Rec(5, 6, [9], [], [], [], 6)
    r0 = S.Rec(2, 6, [3], [], [], [], 2)
    pend = dict(cls=5, feats=[], spells=[], deltas=[0] * 8, unspent=0, ability=6)
    res = carry_rewrite(rr, 7, 6, {2: r0, 7: rr}, pend, set(), set(), R, cs)
    chk("prereq-first: slot takes feat 3, not 9", res["feats"] == [3] and res["info"] == "carry")
    res = carry_rewrite(rr, 7, 7, {2: r0, 7: rr}, pend, {3}, set(), R, cs)
    chk("on-time with prereq present: plain", res["info"] == "ontime" and res["feats"] == [9])
    print("selftest", "OK" if ok else "FAILED")
    return ok


if __name__ == "__main__":
    sys.exit(0 if selftest() else 1)
