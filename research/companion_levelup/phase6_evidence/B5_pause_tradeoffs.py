#!/usr/bin/env python3
"""B5: what does "Pause at level minPause" gain/lose vs the preset default? (Phase 6B). Run from research/companion_levelup with the repo venv:
    ../../.venv/bin/python3 phase6_evidence/B5_pause_tradeoffs.py
Writes phase6_evidence/B5_pause_tradeoffs.md (tables + README paragraphs; the per-companion story notes are the STORY dict below).

Model (same engine loop as B2, decisions from simulate_rewrite.level_now, the Python twin proven == C++ in B2/parity.py):
  start = [(base class, minPause)] (char level minPause, first level-up key minPause+1), hold ON;
  default path : pause_at = 0       -> engine takes the preset's base-class records up to convert_at-1, then HOLD, story conversion,
                                       then REBASE onto the Jedi records to char level 30;
  pause path   : pause_at = minPause -> HOLD at once (frozen at minPause), story conversion, then every banked level is a Jedi level.
minPause = max(1, char level of the first record - 1) (A_pause_spec.md); valid pause range minPause..convertAt-2. Presets whose records
already start in the Jedi class (base == hold class: Mira early, Handmaiden/Disciple 'caster/saber'): minPause == convertAt-1 -> pause == default (n/a).
Items come from the applied records: feats (selected + class-granted), powers, skill ranks (sum of record deltas), ability bumps.
Reuses B2_rebase_check helpers (2da snapshot loader, prereq checks). BAB/forcedie/hitdie from the offline classes.2da snapshot.
"""
import os, sys, re
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, ROOT)
sys.path.insert(0, HERE)
import simulate_rewrite as S
import B2_rebase_check as B

OUT = os.path.join(HERE, "B5_pause_tradeoffs.md")
SKN = ["Computer Use", "Demolitions", "Stealth", "Awareness", "Persuade", "Repair", "Security", "Treat Injury"]
CLS = {0: "Soldier", 1: "Scout", 2: "Scoundrel", 3: "Jedi Guardian", 4: "Jedi Consular", 5: "Jedi Sentinel", 9: "Tech Specialist"}
# classes.2da snapshot (override/classes.2da): attackbonustable CLS_ATK_1 for ALL of these classes (=> identical BAB); hitdie / forcedie
HITDIE = {0: 10, 1: 8, 2: 6, 3: 10, 4: 6, 5: 8, 9: 6}
FORCEDIE = {0: 0, 1: 0, 2: 0, 3: 4, 4: 8, 5: 6, 9: 0}
SAVETBL = {0: "SOLDIER", 1: "SCOUT", 2: "SCNDRL", 3: "JEDI_G", 4: "JEDI_C", 5: "JEDI_S", 9: "TECHSPEC"}

# earliest realistic story gate per companion (from paths/presets/<tag>.md), char level of the EXILE/party when the Jedi class can arrive
STORY = {
    "atton": ("Nar Shaddaa refugee-sector secret + influence, then the 'train you' talk; Exile level about 7-9 (assumed 8, no source gives a level); "
              "the player decides when ('only say you will train him once you do not want more Scoundrel levels', atton.md). The late preset waits to 20 for Sneak Attack.", 8),
    "baodur": ("three chained Ebon Hawk talks (influence ~60/75/89, unverified), player-timed; estimate after Nar Shaddaa, char about 12 (ESTIMATE, not sourced); "
               "elkripper only models 30.", 12),
    "disciple": ("'Training' dialogue is influence-gated only, available right after the Dantooine intro talks (elkripper_disciple.txt:22): char 7 (his first level-up) is legal; "
                 "the support/melee presets simply convert later by choice.", 7),
    "handmaiden": ("after the three Ebon Hawk duels (second duel needs Exile able to reach 14, third 18; elkripper 'levels 10, 14, 18'), then the training talk; "
                   "realistic earliest about Exile 14-18. Sources say to keep her at Soldier 6 until then.", 18),
    "mira": ("Nar Shaddaa only (she is recruited there); 'You seem restless' talk + Kreia scene + influence, conversion on the spot; "
             "can be done before she gains XP, so conversion at char about 6-7.", 7),
}


def feat_name(T, f):
    return "%s(%d)" % (B.cell(T["feat"], f, "label").title().replace("_", " ") if T else "", f) if T else str(f)


def pow_name(T, p):
    return "%s(%d)" % (B.cell(T["spells"], p, "label").title().replace("_", " ") if T else "", p) if T else str(p)


def run_path(recs, pid, base, hc, lo, pause_at, total=30):
    """engine loop twin. returns dict(frozen=char level frozen at, applied=[(key,dec,rec)]) after story conversion at the freeze."""
    classes = [(base, lo)]
    applied = []
    while sum(l for _, l in classes) < total:
        key = sum(l for _, l in classes) + 1
        d, cl, rec = S.level_now(classes, recs[pid], key, True, hc, pause_at)
        if d == "HOLD":
            break
        if d == "VANILLA":   # shouldn't happen before the freeze for presets with records from lo+1
            break
        applied.append((key, d, rec))
        classes = [(c, l) for c, l in classes[:-1]] + [(classes[-1][0], classes[-1][1] + 1)]
    frozen = sum(l for _, l in classes)
    assert frozen < total
    # story conversion at the freeze (a_makejedi AddMultiClass), then banked levels are applied in order
    classes.append((hc, 0))
    n_base = len(applied)
    jed_tail = 0
    while sum(l for _, l in classes) < total:
        key = sum(l for _, l in classes) + 1
        d, cl, rec = S.level_now(classes, recs[pid], key, True, hc, pause_at)
        assert d != "HOLD", (pid, key)
        if d == "VANILLA":
            jed_tail += 1
        else:
            applied.append((key, d, rec))
        classes = classes[:-1] + [(classes[-1][0], classes[-1][1] + 1)]
    return dict(frozen=frozen, applied=applied, n_base=n_base, jed_tail=jed_tail)


def totals(path, hc, base):
    f, g, p = [], [], []
    sk = [0] * 8
    ab = []
    for key, d, r in path["applied"]:
        f += r.feats; g += r.granted; p += r.powers
        for i, n in r.skills:
            sk[i] += n
        if r.ability != 6:
            ab.append((key, r.ability))
    return dict(feats=f, granted=g, powers=p, skills=sk, abil=ab)


def prereq_issues(T, seq_recs, base_seq):
    def fail(seq):
        kf = set(); kp = set(S.ENGINE_GRANT_POWERS); bad = set()
        for r in seq:
            kf |= set(r.granted)
            for f in r.feats:
                if not B.feat_prereq_ok(T, f, kf):
                    bad.add(("feat", f))
                kf.add(f)
            for p in r.powers:
                if not B.power_prereq_ok(T, p, kp):
                    bad.add(("power", p))
                kp.add(p)
        return bad
    return sorted(fail(seq_recs) - fail(base_seq))


def early_apply(T, path, hc):
    """Jedi picks applied at a character level earlier than their min char level (feat.2da mincharlevel / spells.2da class column)."""
    out = []
    for key, d, r in path["applied"]:
        if r.cls != hc:
            continue
        for f in r.feats:
            mc = B.num(B.cell(T["feat"], f, "mincharlevel"))
            if mc > key:
                out.append(("feat", f, key, mc))
        for p in r.powers:
            mc = B.num(B.cell(T["spells"], p, B.PWR_COL.get(hc, "sentinel")), -1)
            if 0 < mc <= 60 and mc > key:
                out.append(("power", p, key, mc))
    return out


def cnt_diff(a, b):
    """multiset a - b"""
    from collections import Counter
    return Counter(a) - Counter(b)


def main():
    cs, presets, recs = S.parse_table_v2(open(S.INI).read())
    T = B.load_2da()
    assert T, "needs the repo venv (pykotor) + 2da snapshot"
    rows = {}   # tag -> list of result dicts
    na = []
    for pid, p in sorted(presets.items()):
        if not p.hold_class or p.convert_at <= 0 or pid not in recs:
            if p.hold_class:
                na.append((pid, "convert_at 0 (dev live path, pause not applicable)"))
            continue
        lv = recs[pid]
        hc = p.hold_class
        base = lv[min(lv)].cls
        tag = pid.split("__")[0]
        if base == hc:
            na.append((pid, "records start in the Jedi class (base == hold class): minPause = convertAt-1, pause == default"))
            continue
        lo = max(1, lv[min(lv)].clvl - 1)
        if lo > p.convert_at - 2:
            na.append((pid, "minPause %d > convertAt-2 = %d: no pause range" % (lo, p.convert_at - 2)))
            continue
        dflt = run_path(recs, pid, base, hc, lo, 0)
        paus = run_path(recs, pid, base, hc, lo, lo)
        assert dflt["frozen"] == p.convert_at - 1, (pid, dflt["frozen"], p.convert_at)
        assert paus["frozen"] == lo
        td, tp = totals(dflt, hc, base), totals(paus, hc, base)
        # lost: base records skipped (items only they grant), gained: items only the pause path applies
        lost_f = cnt_diff(td["feats"] + td["granted"], tp["feats"] + tp["granted"])
        lost_p = cnt_diff(td["powers"], tp["powers"])
        gain_f = cnt_diff(tp["feats"] + tp["granted"], td["feats"] + td["granted"])
        gain_p = cnt_diff(tp["powers"], td["powers"])
        seq_d = [r for _, _, r in dflt["applied"]]
        seq_p = [r for _, _, r in paus["applied"]]
        # what a Jedi record applied by REBASE earlier than designed does vs the default (prereqs/min level), new vs default
        newbad = prereq_issues(T, seq_p, seq_d)
        early_d = {(k, x) for k, x, _, _ in early_apply(T, dflt, hc)}
        early_p = [e for e in early_apply(T, paus, hc) if (e[0], e[1]) not in early_d]
        rows.setdefault(tag, []).append(dict(
            pid=pid, hc=hc, base=base, lo=lo, conv=p.convert_at, dflt=dflt, paus=paus, td=td, tp=tp,
            lost_f=lost_f, lost_p=lost_p, gain_f=gain_f, gain_p=gain_p, newbad=newbad, early=early_p,
            maxj=sum(1 for r in lv.values() if r.cls == hc), lv=lv))
    write(rows, na, T)


def fl(T, cnt, fn, limit=14):
    items = sorted(cnt.items())
    s = ", ".join(fn(T, k) + (" x%d" % n if n > 1 else "") for k, n in items[:limit])
    if len(items) > limit:
        s += ", ... (+%d more)" % (len(items) - limit)
    return s or "none"


def write(rows, na, T):
    L = []
    w = L.append
    w("# B5 - what \"Pause at level minPause\" gains and loses vs the preset default")
    w("")
    w("Generated by `phase6_evidence/B5_pause_tradeoffs.py` (re-run: `cd research/companion_levelup && ../../.venv/bin/python3 phase6_evidence/B5_pause_tradeoffs.py`)."
      " Presets are the CURRENT ones (B4 fixes of 2026-10-03). Decisions come from `simulate_rewrite.level_now` (Python twin of the C++ `levelNow`, proven equal in B2/parity),"
      " the same engine loop as `B2_rebase_check.py`.")
    w("")
    w("## How to read this")
    w("- **Default** = the preset's own rule: take the base-class records up to `convert_at - 1`, bank, the story script adds the Jedi class, every banked level is then a Jedi level (REBASE onto the preset's Jedi records).")
    w("- **Pause at minPause** = freeze at `minPause` (the preset's first level-up minus 1, i.e. the level the companion already has when you recruit him/her), bank every later level until the Jedi class exists, then the same REBASE. "
      "The Jedi class arrives at a STORY point, not at a level (see each companion), so the freeze lasts until that story point; the end state is char level 30 in both columns.")
    w("- Both paths end at character level 30 with exactly two classes. Pausing early swaps the base-class levels in `minPause+1 .. convert_at-1` for extra Jedi levels at the END of the Jedi record list "
      "(Jedi class level 1 is taken at char minPause+1 instead of at convert_at).")
    w("- Jedi records only exist up to class level 30-convert_at+1; when the pause path has more Jedi levels than records, the surplus levels are **unplanned** (`VANILLA`: the engine's own picks / the player chooses in the GUI).")
    w("- **BAB**: identical in every path. `classes.2da` (offline snapshot `replay/scratch/override/classes.2da`) points ALL of Soldier, Scout, Scoundrel, Tech Specialist, Guardian, Consular and Sentinel at `CLS_ATK_1`, so base attack bonus depends only on character level (the \"Sentinel 0.75/level\" note in atton.md is not what the 2DA says).")
    w("- **Saves**: classes use different tables (`CLS_ST_*`: Soldier/Scout/Scoundrel/TechSpec vs JEDI_G/C/S); the table contents live in BIFs and are not in the offline snapshot, so no save numbers are computed here (open item; check in game).")
    w("- **Force/VP**: nominal die per class level from `classes.2da` (forcedie Guardian 4, Consular 8, Sentinel 6, base classes 0; hitdie Soldier 10, Scout 8, Scoundrel 6, TechSpec 6, Guardian 10, Consular 6, Sentinel 8). Force points = die x extra Jedi levels (plus Wisdom bonus; how the engine rolls the die was not checked). "
      "Skill points per level are `skillpointbase` + INT mod (Soldier 1, Scout 3, Scoundrel 4, TechSpec 4, Guardian 1, Consular 2, Sentinel 3).")
    w("")
    for tag in ("atton", "baodur", "disciple", "handmaiden", "mira"):
        rs = rows.get(tag, [])
        w("## %s" % tag.capitalize())
        w("")
        story, sl = STORY[tag]
        w("Story gate for the Jedi class: %s" % story)
        w("")
        if not rs:
            w("No preset of this companion has a pause range (all Jedi presets start in the Jedi class: minPause = convertAt-1).")
            w("")
            continue
        w("| preset | default path to 30 | pause-at-min path to 30 | lose | gain |")
        w("|---|---|---|---|---|")
        for r in rs:
            d, p = r["dflt"], r["paus"]
            bn, hn = CLS[r["base"]], CLS[r["hc"]]
            nbd = d["frozen"] - r["lo"]          # base levels taken in default after minPause
            jd = 30 - d["frozen"]
            jp = 30 - p["frozen"]
            planned_d = jd - d["jed_tail"]
            planned_p = jp - p["jed_tail"]
            dflt_s = "%s %d-%d (%d levels taken from char %d), freeze at %d, then %s 1-%d (%d planned%s)" % (
                bn, r["lo"] + 1, d["frozen"], nbd, r["lo"] + 1, d["frozen"], hn, jd, planned_d,
                ", %d unplanned" % d["jed_tail"] if d["jed_tail"] else "")
            paus_s = "freeze at %d (0 %s levels), then %s 1-%d (%d planned%s)" % (
                p["frozen"], bn, hn, jp, planned_p, ", %d unplanned" % p["jed_tail"] if p["jed_tail"] else "")
            td, tp = r["td"], r["tp"]
            # lose
            lose = []
            lose.append("%d %s levels (char %d-%d)" % (nbd, bn, r["lo"] + 1, d["frozen"]))
            lose.append("%d selected feats / %d class-granted from those levels: %s" % (
                len(td["feats"]) - len(tp["feats"]) if False else sum(1 for k, x, rr in d["applied"] if rr.cls == r["base"] for _ in rr.feats),
                sum(1 for k, x, rr in d["applied"] if rr.cls == r["base"] for _ in rr.granted),
                fl(T, r["lost_f"], feat_name)))
            if r["lost_p"]:
                lose.append("powers: " + fl(T, r["lost_p"], pow_name))
            bsk = [0] * 8
            for k, x, rr in d["applied"]:
                if rr.cls == r["base"]:
                    for i, n in rr.skills:
                        bsk[i] += n
            lose.append("%d base-class skill ranks" % sum(bsk))
            dsk = sum(td["skills"]); psk = sum(tp["skills"])
            lose.append("base-class hit die d%d vs d%d on %d levels, skill points/level %s base vs %s Jedi" % (
                HITDIE[r["base"]], HITDIE[r["hc"]], nbd,
                {0: 1, 1: 3, 2: 4, 9: 4}[r["base"]], {3: 1, 4: 2, 5: 3}[r["hc"]]))
            # gain
            gain = []
            extra = jp - jd
            gain.append("+%d Jedi levels by 30 (%d vs %d), Force points +%d nominal (die %d)" % (
                extra, jp, jd, extra * FORCEDIE[r["hc"]], FORCEDIE[r["hc"]]))
            gain.append("powers %d vs %d (%+d)%s" % (len(tp["powers"]), len(td["powers"]), len(tp["powers"]) - len(td["powers"]),
                                                     (": " + fl(T, r["gain_p"], pow_name)) if r["gain_p"] else ""))
            gain.append("feats gained (selected + granted): %s" % fl(T, r["gain_f"], feat_name))
            gain.append("planned skill ranks total %d (pause) vs %d (default)" % (psk, dsk))
            if p["jed_tail"] > d["jed_tail"]:
                gain.append("(%d more levels with NO planned picks)" % (p["jed_tail"] - d["jed_tail"]))
            if r["newbad"]:
                gain.append("RISK new prereq gaps vs default: " + ", ".join(
                    (feat_name if k == "feat" else pow_name)(T, x) for k, x in r["newbad"]))
            if r["early"]:
                gain.append("RISK picks applied before their min char level (apply may be refused by the engine): " + "; ".join(
                    "%s %s at %d needs %d" % (k, (feat_name if k == "feat" else pow_name)(T, x), kk, mc) for k, x, kk, mc in r["early"][:8]) +
                    (" ..." if len(r["early"]) > 8 else ""))
            w("| `%s` (convert %d) | %s | %s | %s | %s |" % (r["pid"], r["conv"], dflt_s, paus_s,
                                                          "<br>".join(lose), "<br>".join(gain)))
        w("")
        # README paragraph
        w("README paragraph")
        w("")
        for r in rs:
            w(readme_line(T, r, tag))
        w("")
    w("## Not applicable")
    w("")
    for pid, why in na:
        w("- `%s`: %s" % (pid, why))
    w("")
    open(OUT, "w").write("\n".join(L) + "\n")
    print("wrote", OUT)


def readme_line(T, r, tag):
    d, p = r["dflt"], r["paus"]
    bn, hn = CLS[r["base"]], CLS[r["hc"]]
    nbd = d["frozen"] - r["lo"]
    extra = nbd
    jp = 30 - p["frozen"]
    unplanned = p["jed_tail"] - d["jed_tail"]
    lf = {k: n for k, n in r["lost_f"].items()}
    lp = len(r["lost_p"])
    s = ("- **%s, `%s`**: pausing at level %d (instead of %d) trades %d %s levels (%d feat picks, %d class-granted feats%s) for %d extra %s levels "
         "(Jedi class levels %d instead of %d by 30; %+d powers, ~+%d Force points nominal)" % (
             tag.capitalize(), r["pid"], p["frozen"], d["frozen"], nbd, bn,
             sum(1 for k, x, rr in d["applied"] if rr.cls == r["base"] for _ in rr.feats),
             sum(1 for k, x, rr in d["applied"] if rr.cls == r["base"] for _ in rr.granted),
             ": e.g. " + fl(T, r["lost_f"], feat_name, 4) if lf else "", extra, hn, jp, 30 - d["frozen"],
             len(r["tp"]["powers"]) - len(r["td"]["powers"]), extra * FORCEDIE[r["hc"]]))
    if unplanned > 0:
        s += "; %d of the extra levels are unplanned (no preset record, pick by hand)" % unplanned
    s += ". BAB is unchanged."
    return s


if __name__ == "__main__":
    main()
