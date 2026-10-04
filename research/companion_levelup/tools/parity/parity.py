#!/usr/bin/env python3
"""C++ vs Python parity for the level-up dry-run algorithm.

Builds parity_main.cpp (LevelUpOverride.cpp + stub/windows.h, native g++), generates cases from levelup_paths.ini (per record:
vanilla-like pend, replay of the result, class mismatch, negative unspent, random picks; plus missing records), runs both twins and
diffs the DRY lines (cases keyed by preset id, with a Class-Skill-feat csExtraMask); levelNow parity over every preset; then runs apply_test.cpp
(applyRewrite on fake engine structs) on the same cases gate_test.cpp (hold/instant/GUI-guard/Auto-OK handlers + M3 save-binding hooks/ptApply on fake engine structs) playthrough_test.cpp (Playthrough.h) and ui_test.cpp (LevelUpUi.h character-screen UI + R11 banked message). Usage: python3 tools/parity/parity.py   (from research/companion_levelup)
"""
import os, random, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, ROOT)
import simulate_rewrite as S

INI = os.path.join(ROOT, "runtime", "levelup_paths.ini")
EXE = os.path.join(HERE, "parity_bin")
APPLY_EXE = os.path.join(HERE, "apply_test_bin")
GATE_EXE = os.path.join(HERE, "gate_test_bin")
PT_EXE = os.path.join(HERE, "playthrough_test_bin")
UI_EXE = os.path.join(HERE, "ui_test_bin")
GAP_EXE = os.path.join(HERE, "unit_gap_test_bin")
R10_DIR = os.path.join(os.path.dirname(os.path.dirname(ROOT)), "patch_manager_mods", "level_up_override_src")
R10_EXE = os.path.join(HERE, "r10_filter_test_bin")
R10P_EXE = os.path.join(HERE, "r10_page_test_bin")


def case(tag, key, p, mask=0):
    j = lambda l: ",".join(map(str, l))
    return "%s %d %d %d %d %d|%s|%s|%s" % (tag, key, p["cls"], p["ability"], p["unspent"], mask, j(p["feats"]), j(p["spells"]), j(p["deltas"]))


def build(exe, src, m32=False):
    r = subprocess.run(["g++", "-std=c++17", "-O1", "-Wall", "-Wextra"] + (["-m32"] if m32 else []) + ["-I", os.path.join(HERE, "stub"), "-I", os.path.join(HERE, "stub", "inc"), "-include", os.path.join(HERE, "stub", "m3_win_shim.h"), "-o", exe,
                        os.path.join(HERE, src)], capture_output=True, text=True)
    if r.returncode:
        sys.exit(r.stderr)


def m32_ok():
    """True when `g++ -m32` can build and run a C++ program here (T3: the engine's real 32-bit pointer layout)."""
    import tempfile
    with tempfile.TemporaryDirectory() as d:
        c = os.path.join(d, "t.cpp")
        open(c, "w").write("#include <vector>\nint main(){std::vector<int> v(3);return v.size()==3?0:1;}\n")
        return subprocess.run(["g++", "-m32", "-o", os.path.join(d, "t"), c], capture_output=True).returncode == 0 and subprocess.run([os.path.join(d, "t")]).returncode == 0


def ln_cases(recs, presets):
    """levelNow cases per preset: (id, key, hold, classes). On-track keys, early/late conversion with hold on/off, below start, past end."""
    out = []
    for pid, lv in recs.items():
        if not lv:
            continue
        first = lv[min(lv)]
        base = first.cls
        hc = presets[pid].hold_class if pid in presets else 0
        blv = max([r.clvl for r in lv.values() if r.cls == base and r.clvl is not None] or [0])
        for cl, r in sorted(lv.items()):
            for hold in (True, False):
                if r.cls == base:
                    out.append((pid, cl, hold, [(base, r.clvl - 1)]))
                else:
                    out.append((pid, cl, hold, [(base, blv), (r.cls, r.clvl - 1)]))
        for k in list(range(0, blv + 3)) + [blv + 5]:
            for key in (k + 1, k + 3):
                for hold in (True, False):
                    out.append((pid, key, hold, [(base, k)]))
                    if hc:
                        out.append((pid, key + 1, hold, [(base, k), (hc, 0)]))
                        for j in (1, 3, 8):
                            out.append((pid, key + j, hold, [(base, k), (hc, j)]))
        out.append((pid, 4, True, [(99, 3)]))
        out.append((pid, 4, True, [(base, 1), (99, 2)]))
    return out


def ln_early_cases(recs, presets):
    """B2: explicit early-conversion cases. For every preset with hold_class != 0: the story converts the companion after L base-class levels, L from
    the utc start level (first record's class level - 1, min 1) up to convert_at-1; then EVERY Jedi class level j = 0..(30-L-1) up to character
    level 30 (state [(base, L), (hold_class, j)], key = L + j + 1), hold on and off. The B2 script (phase6_evidence/B2_rebase_check.py) proves the properties."""
    out = []
    for pid, lv in recs.items():
        hc = presets[pid].hold_class if pid in presets else 0
        if not lv or not hc:
            continue
        base = lv[min(lv)].cls
        conv = min([cl for cl, r in lv.items() if r.cls != base] or [0])
        if not conv:
            continue
        for L in range(max(1, lv[min(lv)].clvl - 1), conv):
            for j in range(0, 30 - L):
                for hold in (True, False):
                    out.append((pid, L + j + 1, hold, [(base, L), (hc, j)]))
    return out


def ln_pause_cases(recs, presets):
    """0.3.1 pause cases. For every preset with hold_class != 0 and convert_at > 0: every pause 1..convert_at (covers out of range, the default
    convert_at-1 and above) x hold on/off x creature states: base class at every level 0..blv+1 (key = total + 1), base at L + hold class at j in (0, 1, 5),
    a foreign (non-Jedi) class on top, and (A2 F2) a DIFFERENT Jedi class than the preset's hold class (every other Jedi class id, levels 0/1/5, also with the
    base class behind it). Cases are (id, key, hold, pause, classes)."""
    out = []
    for pid, lv in recs.items():
        pr = presets.get(pid)
        if not lv or not pr or not pr.hold_class or pr.convert_at <= 0:
            continue
        base = lv[min(lv)].cls
        blv = max([r.clvl for r in lv.values() if r.cls == base and r.clvl is not None] or [0])
        for pause in range(0, pr.convert_at + 1):
            for hold in (True, False):
                for L in range(0, blv + 2):
                    out.append((pid, L + 1, hold, pause, [(base, L)]))
                    for j in (0, 1, 5):
                        out.append((pid, L + j + 1, hold, pause, [(base, L), (pr.hold_class, j)]))
                    out.append((pid, L + 2, hold, pause, [(base, L), (99, 1)]))
                    for oj in (3, 4, 5, 11, 12, 13):
                        if oj == pr.hold_class:
                            continue
                        for j in (0, 1, 5):
                            out.append((pid, L + j + 1, hold, pause, [(base, L), (oj, j)]))
    return out


# ---------------------------------------------------------------------------------------------------------------------------------------
# Carry-over (0.4.0): C++ computeCarry vs simulate_rewrite.carry_rewrite through the `K` verb of parity_main.cpp.
# ---------------------------------------------------------------------------------------------------------------------------------------
def kline(pid, cl, key, p, mask, kf, kp):
    j = lambda l: ",".join(map(str, l))
    ks = lambda k: "-" if k is None else "k" + j(sorted(k))
    return "K %s %d %d %d %d %d %d %s %s|%s|%s|%s" % (pid, cl, key, p["cls"], p["ability"], p["unspent"], mask, ks(kf), ks(kp), j(p["feats"]), j(p["spells"]), j(p["deltas"]))


def kexpect(pid, cl, key, p, mask, kf, kp, recs, R, cs):
    res = S.carry_rewrite(recs[pid].get(cl), cl, key, recs[pid], p, kf, kp, R, cs, mask)
    if res["info"] == "carry":   # applyRewrite keeps out[0..nKept) in place: granted/engine-grant entries plus the engine-pick fallbacks
        nk = "%d/%d" % (len([f for f in p["feats"] if f in recs[pid][cl].granted]) + res["fb"][0],
                        len([x for x in p["spells"] if x in S.ENGINE_GRANT_POWERS]) + res["fb"][1])
    else:
        nk = None
    return "%s ## %s ## %s ## nk=%s" % (S.format_dry(pid, key, res), res["info"], S.format_carry(pid, key, res) if res["info"] == "carry" else "-", nk), res


def run_k(exe, ini, cases):
    text = "\n".join(cases) + "\n"
    r = subprocess.run([exe, ini], input=text, capture_output=True, text=True)
    if r.returncode:
        sys.exit("parity_bin K failed rc=%d: %s" % (r.returncode, r.stderr[:500]))
    return r.stdout.splitlines()


def pend_of(cls, feats=(), spells=(), unspent=60, ability=6, deltas=None):
    return dict(cls=cls, feats=list(feats), spells=list(spells), deltas=list(deltas or [0] * 8), unspent=unspent, ability=ability)


def carry_matrix(recs, presets, R, cs, rnd):
    """The B2 matrix (every preset with a hold class, every conversion level L from the utc start level to convert_at-1, hold on, engine loop to character
    level 30) driven through carry_rewrite. Yields case tuples (pid, cl, key, pend, mask, kf, kp) in call order; creature state evolves with the Python result.
    Even-numbered scenarios start from an empty creature, odd ones from a random known set; every other call carries random engine picks in the pending lists.
    Also returns the legality property violations (every non-fallback pick must be legal at placement under the parsed rules)."""
    cases, viol, nscen = [], [], 0
    for pid, lv in sorted(recs.items()):
        hc = presets[pid].hold_class if pid in presets else 0
        if not lv or not hc:
            continue
        first = lv[min(lv)]
        base = first.cls
        conv = min([cl for cl, r in lv.items() if r.cls != base] or [0])
        if not conv:
            continue
        plan_f = sorted({f for r in lv.values() for f in r.feats})
        plan_p = sorted({x for r in lv.values() for x in r.powers})
        for L in range(max(1, first.clvl - 1), conv):
            nscen += 1
            kf = set(rnd.sample(plan_f, min(4, len(plan_f))) + [rnd.randrange(1, 245) for _ in range(3)]) if nscen % 2 else set()
            kp = set(rnd.sample(plan_p, min(3, len(plan_p)))) if nscen % 2 else set()
            todo = [(cl, r) for cl, r in sorted(lv.items()) if r.cls == base and r.clvl <= L]    # the on-time base-class records come first
            seq = [(cl, cl, r) for cl, r in todo]
            for j, k2 in enumerate(range(L + 1, 31)):
                d, cl, r = S.level_now([(base, L), (hc, j)], lv, k2, True, hc)
                if d in ("PATH", "REBASE"):
                    seq.append((k2, cl, r))
            for n, (k2, cl, r) in enumerate(seq):
                eng_f = [rnd.randrange(1, 245) for _ in range(rnd.randrange(0, 3))] if n % 2 else []
                eng_s = [rnd.randrange(1, 285) for _ in range(rnd.randrange(0, 3))] if n % 2 else []
                pend = pend_of(r.cls, list(r.granted) + eng_f, [0x90] + eng_s, 60, 6)
                mask = 0
                cases.append((pid, cl, k2, pend, mask, set(kf), set(kp)))
                res = S.carry_rewrite(r, cl, k2, lv, pend, kf, kp, R, cs, mask)
                if res["info"] == "carry":
                    nk = len([f for f in pend["feats"] if f in r.granted]) + res["fb"][0]
                    knf = kf | set(f for f in pend["feats"] if f in r.granted)
                    for f in res["feats"][nk:]:
                        if S._feat_state(R, f, r.cls, k2, knf)[0] != "ok":
                            viol.append((pid, k2, "feat", f))
                        knf.add(f)
                    nks = len([x for x in pend["spells"] if x in S.ENGINE_GRANT_POWERS]) + res["fb"][1]
                    knp = kp | S.ENGINE_GRANT_POWERS
                    for x in res["spells"][nks:]:
                        if S._power_state(R, x, r.cls, k2, knp)[0] != "ok":
                            viol.append((pid, k2, "power", x))
                        knp.add(x)
                kf |= set(res["feats"]); kp |= set(res["spells"])
    return cases, viol, nscen


def ontime_walk(recs, R, cs, rnd):
    """Every record of every preset in design order with the creature state an on-time creature has (granted feats + everything applied before), clean + 6
    random pending lists (granted/engine picks/garbage). The carry result must be the plain rewrite byte for byte (info=ontime)."""
    cases = []
    for pid, lv in sorted(recs.items()):
        kf, kp = set(), set()
        for cl in sorted(lv):
            r = lv[cl]
            for t in range(7):
                if t == 0:
                    pend = pend_of(r.cls, r.granted)
                else:
                    pend = pend_of(r.cls, rnd.sample(list(r.granted) + [rnd.randrange(0, 250) for _ in range(6)], k=min(5, len(r.granted) + 3)),
                                   [rnd.choice(sorted(S.ENGINE_GRANT_POWERS)), rnd.randrange(0, 286)], rnd.randrange(0, 40), rnd.randrange(0, 7), [rnd.randrange(0, 3) for _ in range(8)])
                cases.append((pid, cl, cl, pend, 0, set(kf), set(kp)))
            base = S.compute_rewrite(r, pend_of(r.cls, r.granted), cs)
            kf |= set(base["feats"]); kp |= set(base["spells"])
    return cases


def _syn_rec(cl, cls, feats, powers, clvl, skills="", granted=""):
    return "L%d=%d|6|%s|%s|%s|%s|%d" % (cl, cls, ",".join(map(str, feats)), ",".join(map(str, powers)), skills, granted, clvl)


SYN = """[classskills]
cs2=11111111
cs5=11111111
[presets]
t__x=r|s|a|c|k|8|5|synthetic
[featrules]
f3=0|||2,5
f9=4|3||2,5
f20=0||31,32|5
f30=0|||5
f31=0|||5
f32=0|||5
f50=12|||5
f60=0|||2
f70=0|||5
f71=0|70||5
f72=0|71||5
f73=0|72||5
f74=0|73||5
f75=0|74||5
[powerrules]
p10=|5:6
p11=|5:0
p20=10|5:0
p21=|5:99
p22=10,11|5:8
p23=|
p144=|5:0
[t__x]
name=Synthetic
""" + "\n".join([_syn_rec(4, 2, [3], [], 4, "0:1"), _syn_rec(5, 2, [60], [], 5), _syn_rec(6, 2, [], [], 6),
                  _syn_rec(8, 5, [9], [10], 1), _syn_rec(9, 5, [30, 31], [11, 20], 2), _syn_rec(10, 5, [20], [21, 23], 3),
                  _syn_rec(11, 5, [50], [22], 4), _syn_rec(12, 5, [75], [], 5), _syn_rec(13, 5, [74], [], 6),
                  _syn_rec(14, 5, [60], [], 7), _syn_rec(15, 5, [30], [11, 10], 8, "0:5"), _syn_rec(16, 5, [], [144, 11], 9),
                  _syn_rec(17, 5, [30, 31], [11, 20], 9, "", "100")]) + "\n"


def synthetic_tests(fails):
    import tempfile
    tmp = os.environ.get("TMPDIR") or tempfile.gettempdir()
    with tempfile.NamedTemporaryFile("w", suffix=".ini", delete=False, dir=tmp) as f:
        f.write(SYN)
        path = f.name
    try:
        cs, presets, recs, R = S.parse_table_v3(SYN)
        pid = "t__x"
        T = []   # (name, cl, key, pend, kf, kp, expect dict: feats / spells / info / unfilled ...)

        def t(name, cl, key, kf, kp, pend=None, **exp):
            T.append((name, cl, key, pend or pend_of(recs[pid][cl].cls if cl in recs[pid] else 5), kf, kp, exp))
        t("jedi-first: planned picks keep their slots (owed 3 not taken)", 9, 5, set(), set(), feats=[30, 31], info="carry")
        t("prerequisite pull: planned 9 needs TWF(3): 3 first, 9 waits", 8, 5, set(), set(), feats=[3], info="carry")
        t("planned 9 legal once 3 is known", 8, 5, {3}, set(), feats=[9], info="carry")
        t("min-level deferral, no engine pick: slot stays empty", 8, 5, {3}, set(), spells=[], info="carry", unfilled=(0, 1))
        t("min-level deferral, engine pick kept for the slot", 8, 5, {3}, set(), pend_of(5, [], [0x90, 77]), spells=[0x90, 77], info="carry", unfilled=(0, 1), fb=(0, 1))
        t("engine pick fallback (feats): planned 9 blocked, no owed legal", 8, 4, set(), set(), pend_of(5, [], []), feats=[3], info="carry")
        t("min level reached: power 10 taken", 8, 6, {3}, set(), spells=[10], info="carry")
        t("power prerequisite pulled: 11 then 10 (20 waits)", 9, 6, set(), set(), spells=[11, 10], info="carry")
        t("power prerequisite not yet legal (10 min 6): fallback", 9, 5, set(), set(), pend_of(5, [], [0x91, 55]), spells=[0x91, 55, 11], info="carry", fb=(0, 1))
        t("OR-list: first legal alternative pulled", 10, 6, set(), set(), feats=[31], info="carry")
        t("OR-list satisfied by the second alternative", 10, 6, {32}, set(), feats=[20], info="carry")
        t("never-selectable power (21 = 99, 23 = no Jedi entry): both blocked", 10, 11, {3, 9, 60, 30, 31}, {10, 11, 20}, spells=[], info="carry", unfilled=(0, 2))
        t("depth: 75 needs a depth-5 chain -> unresolved", 12, 13, {3, 9, 60, 30, 31, 20, 50}, {10, 11, 20, 21, 22, 23}, feats=[], info="carry", unfilled=(1, 0))
        t("depth: 74 resolves through a depth-4 chain to 70", 13, 13, {3, 9, 60, 30, 31, 20, 50, 75}, {10, 11, 20, 22}, feats=[70], info="carry")
        t("level-gated feat 50 below its min level: fallback", 11, 5, {3, 9, 60, 30, 31, 20}, {10, 11, 20}, pend_of(5, [88], []), feats=[88], info="carry", fb=(1, 0))
        t("level-gated feat 50 at its min level", 11, 12, {3, 9, 60, 30, 31, 20}, {10, 11, 20}, feats=[50], info="carry")
        t("class-gated feat 60 (Scoundrel only) in a Jedi record: not taken", 14, 14, {3, 9, 30, 31, 20, 50, 70, 71, 72, 73, 74, 75}, {10, 11, 20, 22}, feats=[], info="carry", unfilled=(1, 0))
        t("on time with everything known: identical to the plain rewrite", 9, 9, {3, 9, 60}, {10}, info="ontime")
        t("on time, owed pick missing (player deviated): carry engages", 9, 9, {9, 60}, {10}, feats=[30, 31], info="carry")
        t("known powers 10, 11 are not re-picked: owed 20 and 22 fill the slots", 15, 15, {3, 9, 60, 30, 31, 20, 50, 75, 70}, {10, 11}, spells=[20, 22], info="carry")
        t("engine-granted power 144 planned in a record counts as known (kept, not re-picked)", 16, 17, {3, 9, 60, 30, 31, 20, 50, 75, 70}, {10}, spells=[11, 20], info="carry")
        t("engine-granted power 144 planned and in the pending list: kept once", 16, 17, {3, 9, 60, 30, 31, 20, 50, 75, 70}, {10}, pend_of(5, [], [144]), spells=[144, 11, 20], info="carry")
        t("feats unreadable: carry skipped", 9, 5, None, set(), info="skipped")
        t("powers unreadable: feats carried, powers plain", 9, 5, set(), None, feats=[30, 31], spells=[11, 20], info="carry", pk=0)
        t("no record: n/a", 3, 5, set(), set(), info="n/a")
        full = [100] * 63
        t("MaxPend: 63 kept feats + 63 kept powers, 2 owed picks each: never past 64, valid prefix", 17, 17, set(), set(),
          pend_of(5, full, [0x90] * 63), feats=full + [30], spells=[0x90] * 63 + [11], info="carry")
        t("MaxPend: 64 kept feats + 64 kept powers (array full): kept all, no pick appended", 17, 17, set(), set(),
          pend_of(5, [100] * 64, [0x90] * 64), feats=[100] * 64, spells=[0x90] * 64, info="carry")
        t("negative unspent in carry mode stays negative-unspent", 15, 6, {3}, set(), pend_of(5, [], [], 0, 6, [0] * 8), info="negative-unspent")
        t("class mismatch: n/a", 9, 5, set(), set(), pend_of(2), info="n/a")
        cases = [kline(pid, cl, key, pend, 0, kf, kp) for _n, cl, key, pend, kf, kp, _e in T]
        out = run_k(EXE, path, cases)
        for (name, cl, key, pend, kf, kp, e), line in zip(T, out):
            want, res = kexpect(pid, cl, key, pend, 0, kf, kp, recs, R, cs)
            ok = line == want
            if ok and "info" in e: ok = res["info"] == e["info"]
            if ok and "feats" in e: ok = res["feats"] == e["feats"]
            if ok and "spells" in e: ok = res["spells"] == e["spells"]
            if ok and "unfilled" in e: ok = res["unfilled"] == e["unfilled"]
            if ok and "fb" in e: ok = res["fb"] == e["fb"]
            if ok and "pk" in e: ok = int(res["pk"]) == e["pk"]
            if not ok:
                fails.append(name)
                print("FAIL synthetic:", name, "\n  C++:", line, "\n   Py:", want, "\n  exp:", e)
        if len(out) != len(T):
            fails.append("synthetic line count")
        # replay: the result fed back as the pending list is `already` (stateless), C++ and Python alike
        rep = []
        for name, cl, key, pend, kf, kp, e in T:
            if e.get("info") == "carry" and not e.get("pk") == 0:
                _w, res = kexpect(pid, cl, key, pend, 0, kf, kp, recs, R, cs)
                rep.append((name, cl, key, dict(pend, feats=res["feats"], spells=res["spells"], deltas=res["deltas"], unspent=res["unspent"], ability=res["ability"]), kf, kp))
        out = run_k(EXE, path, [kline(pid, cl, key, p, 0, kf, kp) for _n, cl, key, p, kf, kp in rep])
        for (name, cl, key, p, kf, kp), line in zip(rep, out):
            want, res = kexpect(pid, cl, key, p, 0, kf, kp, recs, R, cs)
            if line != want or res["status"] != "already":
                fails.append("replay " + name)
                print("FAIL synthetic replay:", name, "\n  C++:", line, "\n   Py:", want)
        print("synthetic carry cases %d, replay cases %d" % (len(T), len(rep)))
        # table parse: v3 / old table (no rule sections) / malformed rule lines, C++ parseTable vs parse_table_v3
        base_old = SYN[:SYN.index("[featrules]")] + SYN[SYN.index("[t__x]"):]
        bad = [("dup feat row", SYN.replace("f3=0|||2,5\n", "f3=0|||2,5\nf3=0|||2\n")),
               ("feat class id 16", SYN.replace("f3=0|||2,5", "f3=0|||2,16")),
               ("feat row 3 fields", SYN.replace("f3=0|||2,5", "f3=0||2,5")),
               ("3 req ids", SYN.replace("f3=0|||2,5", "f3=0|1,2,3||2")),
               ("6 or ids", SYN.replace("f3=0|||2,5", "f3=0||1,2,3,4,5,6|2")),
               ("feat id 256", SYN.replace("f3=0|||2,5", "f256=0|||2")),
               ("power id 320", SYN.replace("p10=|5:6", "p320=|5:6")),
               ("5 prereqs", SYN.replace("p10=|5:6", "p10=1,2,3,4,5|5:6")),
               ("min level 256", SYN.replace("p10=|5:6", "p10=|5:256")),
               ("power class 16", SYN.replace("p10=|5:6", "p10=|16:6")),
               ("power entry without colon", SYN.replace("p10=|5:6", "p10=|56")),
               ("non-numeric", SYN.replace("f9=4|3||2,5", "f9=4|x||2,5")),
               ("rule row without =", SYN.replace("f3=0|||2,5\n", "f3=0|||2,5\ngarbage\n")),
               ("dup power row", SYN.replace("p10=|5:6\n", "p10=|5:6\np10=|5:7\n"))]
        texts = [("v3", SYN), ("old (no rules)", base_old), ("only featrules", SYN[:SYN.index("[powerrules]")] + SYN[SYN.index("[t__x]"):]),
                 ("rules before presets", SYN[SYN.index("[featrules]"):SYN.index("[t__x]")] + SYN[:SYN.index("[featrules]")] + SYN[SYN.index("[t__x]"):])] + bad
        files = []
        for n, (name, txt) in enumerate(texts):
            fp = path + ".%d" % n
            open(fp, "w").write(txt)
            files.append(fp)
        out = run_k(EXE, path, ["R " + fp for fp in files])
        for (name, txt), line in zip(texts, out):
            try:
                _c, pr2, rc2, R2 = S.parse_table_v3(txt)
                want = "OK rules=%d feat=%d power=%d presets=%d" % (R2 is not None, len(R2.feat) if R2 else 0, len(R2.power) if R2 else 0, len(rc2))
            except ValueError:
                want = "ERR"
            # a bad rule row disables carry only: the table still loads (rules=0, no rule rows kept, the preset section still counted)
            is_bad = name in {n for n, _t in bad}
            if line != want or (is_bad and (want != "OK rules=0 feat=0 power=0 presets=1" or S.parse_table_v3.rule_bad is None)):
                fails.append("parse " + name)
                print("FAIL table parse:", name, "C++:", line, "Py:", want)
        for fp in files:
            os.unlink(fp)
        print("table parse cases", len(texts), "(v3, old, partial, reordered, %d malformed)" % len(bad))
    finally:
        os.unlink(path)


def carry_parity(fails):
    cs, presets, recs, R = S.parse_table_v3(open(INI).read())
    if R is None:
        fails.append("levelup_paths.ini has no rule sections")
        return
    rnd = random.Random(3)
    walk = ontime_walk(recs, R, cs, rnd)
    cases, viol, nscen = carry_matrix(recs, presets, R, cs, rnd)
    allc = walk + cases
    lines = run_k(EXE, INI, [kline(*c) for c in allc])
    bad, kinds = [], {}
    for n, (c, line) in enumerate(zip(allc, lines)):
        want, res = kexpect(c[0], c[1], c[2], c[3], c[4], c[5], c[6], recs, R, cs)
        kinds[res["info"]] = kinds.get(res["info"], 0) + 1
        if line != want:
            bad.append((line, want))
        if n < len(walk):
            plain = S.format_dry(c[0], c[2], S.compute_rewrite(recs[c[0]][c[1]], c[3], cs, c[4]))
            if res["info"] != "ontime" or S.format_dry(c[0], c[2], res) != plain:
                bad.append(("walk case not on-time/identical", want))
    print("carry parity cases %d (on-time walk %d, early-conversion matrix %d over %d scenarios) cpp lines %d mismatches %d kinds %s legality violations %d"
          % (len(allc), len(walk), len(cases), nscen, len(lines), len(bad), kinds, len(viol)))
    for a, b in bad[:5]:
        print(" C++:", a, "\n  Py:", b)
    for v in viol[:5]:
        print(" violation:", v)
    if bad or viol or len(lines) != len(allc):
        fails.append("carry parity")
    # replay (stateless: the applied result fed back is `already`) over every matrix carry case
    rep = []
    for c in cases:
        _w, res = kexpect(c[0], c[1], c[2], c[3], c[4], c[5], c[6], recs, R, cs)
        if res["info"] == "carry":
            rep.append((c[0], c[1], c[2], dict(c[3], feats=res["feats"], spells=res["spells"], deltas=res["deltas"], unspent=res["unspent"], ability=res["ability"]), c[4], c[5], c[6]))
    rl = run_k(EXE, INI, [kline(*c) for c in rep])
    rbad = 0
    for c, line in zip(rep, rl):
        want, res = kexpect(c[0], c[1], c[2], c[3], c[4], c[5], c[6], recs, R, cs)
        rbad += line != want or res["status"] != "already"
    print("carry replay cases %d mismatches/not-already %d" % (len(rep), rbad))
    if rbad:
        fails.append("carry replay")


def main():
    build(EXE, "parity_main.cpp")
    build(APPLY_EXE, "apply_test.cpp")
    build(GATE_EXE, "gate_test.cpp")
    build(PT_EXE, "playthrough_test.cpp")
    build(UI_EXE, "ui_test.cpp")
    build(GAP_EXE, "unit_gap_test.cpp")
    cs, presets, recs = S.parse_table_v2(open(INI).read())
    rnd = random.Random(1)
    cases = []
    for tag, lv in recs.items():
        for key, rec in lv.items():
            base = dict(cls=rec.cls, ability=rnd.choice([1, 3, 6]), unspent=rnd.randint(0, 12), feats=list(rec.granted) + [9999],
                        spells=[0x90, 8888, 0x109], deltas=[rnd.randint(0, 2) for _ in range(8)])
            mask = rnd.choice([0, 0, rnd.randint(0, 255), 1 << rnd.randint(0, 7)])
            out = S.compute_rewrite(rec, base, cs, mask)
            replay = dict(cls=rec.cls, **{k: out[k] for k in ("ability", "unspent", "feats", "spells", "deltas")})
            for p in (base, replay, dict(base, cls=rec.cls + 1), dict(base, unspent=0, deltas=[0] * 8),
                      dict(base, feats=[rnd.randint(0, 250) for _ in range(rnd.randint(0, 4))], spells=[rnd.randint(0, 300) for _ in range(3)])):
                cases.append((tag, key, p, mask))
            if rec.cls in cs:   # same pick under every extra-class-skill mask: the unspent recomputation must follow csExtraMask
                for m in (0, 0xFF, 0x0F, 0xF0):
                    cases.append((tag, key, base, m))
    cases += [("atton__live", 99, cases[0][2], 0), ("bao-dur", 5, cases[0][2], 0), ("KREIA__LIVE", 6, cases[-1][2], 0)]
    text = "\n".join(case(*c) for c in cases) + "\n"
    cpp = subprocess.run([EXE, INI], input=text, capture_output=True, text=True, check=True).stdout.splitlines()
    py = [S.format_dry(t, k, S.compute_rewrite(S.lookup_record(recs, t, k), p, cs, m)) for t, k, p, m in cases]
    bad = [(a, b) for a, b in zip(cpp, py) if a != b]
    statuses = {}
    for l in py:
        st = l.split("status=")[1].split()[0]
        statuses[st] = statuses.get(st, 0) + 1
    print("cases", len(cases), "cpp lines", len(cpp), "mismatches", len(bad), "statuses", statuses)
    for a, b in bad[:5]:
        print(" C++:", a, "\n  Py:", b)
    # levelNow parity: C++ vs S.level_now
    lcases = ln_cases(recs, presets)
    ltext = "\n".join("L %s %d %d %d %s" % (pid, key, int(hold), len(cl), ",".join("%d:%d" % c for c in cl)) for pid, key, hold, cl in lcases) + "\n"
    lcpp = subprocess.run([EXE, INI], input=ltext, capture_output=True, text=True, check=True).stdout.splitlines()
    lpy = []
    decs = {}
    for pid, key, hold, cl in lcases:
        d, c, _ = S.level_now(cl, recs[pid], key, hold, presets[pid].hold_class if pid in presets else 0)
        lpy.append("%s %s" % (d, "-" if c is None else c))
        decs[d] = decs.get(d, 0) + 1
    lbad = [(x, y, c) for x, y, c in zip(lcpp, lpy, lcases) if x != y]
    print("levelNow cases", len(lcases), "cpp lines", len(lcpp), "mismatches", len(lbad), "decisions", decs)
    for x, y, c in lbad[:5]:
        print(" C++:", x, " Py:", y, " case:", c)
    # B2: explicit early-conversion levelNow cases (every preset with a hold class, every conversion level, every Jedi level to char 30)
    ecases = ln_early_cases(recs, presets)
    etext = "\n".join("L %s %d %d %d %s" % (pid, key, int(hold), len(cl), ",".join("%d:%d" % c for c in cl)) for pid, key, hold, cl in ecases) + "\n"
    ecpp = subprocess.run([EXE, INI], input=etext, capture_output=True, text=True, check=True).stdout.splitlines()
    epy = []
    edecs = {}
    for pid, key, hold, cl in ecases:
        d, c, _ = S.level_now(cl, recs[pid], key, hold, presets[pid].hold_class)
        epy.append("%s %s" % (d, "-" if c is None else c))
        edecs[d] = edecs.get(d, 0) + 1
    ebad = [(x, y, c) for x, y, c in zip(ecpp, epy, ecases) if x != y]
    print("levelNow early-conversion cases", len(ecases), "cpp lines", len(ecpp), "mismatches", len(ebad), "decisions", edecs)
    for x, y, c in ebad[:5]:
        print(" C++:", x, " Py:", y, " case:", c)
    # 0.3.1: pauseAt levelNow cases (C++ levelNow(..., pause, ...) vs S.level_now(..., pause_at))
    pcases = ln_pause_cases(recs, presets)
    ptext = "\n".join("P %s %d %d %d %d %s" % (pid, key, int(hold), pause, len(cl), ",".join("%d:%d" % c for c in cl)) for pid, key, hold, pause, cl in pcases) + "\n"
    pcpp = subprocess.run([EXE, INI], input=ptext, capture_output=True, text=True, check=True).stdout.splitlines()
    ppy = []
    pdecs = {}
    for pid, key, hold, pause, cl in pcases:
        d, c, _ = S.level_now(cl, recs[pid], key, hold, presets[pid].hold_class, pause)
        ppy.append("%s %s" % (d, "-" if c is None else c))
        pdecs[d] = pdecs.get(d, 0) + 1
    pbad = [(x, y, c) for x, y, c in zip(pcpp, ppy, pcases) if x != y]
    print("levelNow pause cases", len(pcases), "cpp lines", len(pcpp), "mismatches", len(pbad), "decisions", pdecs)
    for x, y, c in pbad[:5]:
        print(" C++:", x, " Py:", y, " case:", c)
    # carry-over (computeCarry vs carry_rewrite): synthetic unit table + table parse + on-time walk + early-conversion matrix + replay
    cfails = []
    synthetic_tests(cfails)
    carry_parity(cfails)
    # applyRewrite on fake engine pending structs, same cases (apply_test.cpp)
    ap = subprocess.run([APPLY_EXE, INI], input=text, capture_output=True, text=True)
    print(ap.stdout.strip())
    if ap.stderr:
        print(ap.stderr[:4000])
    # gate handlers on fake engine structs (gate_test.cpp)
    gt = subprocess.run([GATE_EXE, INI], capture_output=True, text=True)
    print(gt.stdout.strip())
    if gt.stderr:
        print(gt.stderr[:4000])
    # playthrough model (Playthrough.h: GUID bit packing, levelup_playthroughs.ini parse/serialise/set/remove)
    pt = subprocess.run([PT_EXE], capture_output=True, text=True)
    print(pt.stdout.strip())
    if pt.stderr:
        print(pt.stderr[:4000])
    # R10 filter logic (R10Filter.h; colocated r10_filter_test.cpp, run with the cwd and ini path run_tests.py uses) - built here too so mutate.py kills R10Filter.h mutations
    rb = subprocess.run(["g++", "-std=c++17", "-O1", "-Wall", "-Wextra", "-I", R10_DIR, "-I", os.path.join(os.path.dirname(os.path.dirname(ROOT)), "tests", "include"),
                         os.path.join(R10_DIR, "r10_filter_test.cpp"), "-o", R10_EXE], capture_output=True, text=True)
    if rb.returncode:
        sys.exit("COMPILE FAILED r10_filter_test.cpp\n" + rb.stderr)
    r10 = subprocess.run([R10_EXE, "../../research/companion_levelup/runtime/r10_tags.ini"], cwd=R10_DIR, capture_output=True, text=True)
    print("r10_filter_test: " + (r10.stdout.strip() or ("rc=%d" % r10.returncode)))
    if r10.stderr:
        print(r10.stderr[:4000])
    # R10 PC filter page model (R10Page.h, batch 3a; colocated r10_page_test.cpp) - built here so mutate.py kills R10Page.h mutations
    rpb = subprocess.run(["g++", "-std=c++17", "-O1", "-Wall", "-Wextra", "-I", R10_DIR, "-I", os.path.join(os.path.dirname(os.path.dirname(ROOT)), "tests", "include"),
                          os.path.join(R10_DIR, "r10_page_test.cpp"), "-o", R10P_EXE], capture_output=True, text=True)
    if rpb.returncode:
        sys.exit("COMPILE FAILED r10_page_test.cpp\n" + rpb.stderr)
    r10p = subprocess.run([R10P_EXE], capture_output=True, text=True)
    print("r10_page_test: " + (r10p.stdout.strip() or ("rc=%d" % r10p.returncode)))
    if r10p.stderr:
        print(r10p.stderr[:4000])
    # M4 character-screen UI + R11 banked message on fake GuiKit/engine structs (ui_test.cpp)
    ui = subprocess.run([UI_EXE, INI], capture_output=True, text=True)
    print(ui.stdout.strip())
    if ui.stderr:
        print(ui.stderr[:4000])
    # A3: unit tests for the edge cases the mutation gate (mutate.py) found untested (unit_gap_test.cpp)
    gp = subprocess.run([GAP_EXE], capture_output=True, text=True)
    print(gp.stdout.strip())
    if gp.stderr:
        print(gp.stderr[:4000])
    # T3: the gate and UI tests again as 32-bit builds (F10: the stub VirtualQuery must not wrap; uintptr_t == the engine's 4-byte pointers)
    m32bad = 0
    if m32_ok():
        for exe, src, args in ((GATE_EXE + "32", "gate_test.cpp", [INI]), (UI_EXE + "32", "ui_test.cpp", [INI])):
            build(exe, src, m32=True)
            r32 = subprocess.run([exe] + args, capture_output=True, text=True, timeout=300)
            print("-m32 " + r32.stdout.strip())
            if r32.stderr:
                print(r32.stderr[:4000])
            m32bad += 1 if r32.returncode else 0
    else:
        print("-m32 gate/ui tests SKIPPED (g++ -m32 unavailable)")
    sys.exit(1 if cfails or m32bad or bad or lbad or ebad or pbad or len(pcpp) != len(ppy) or len(ecpp) != len(epy) or len(cpp) != len(py) or len(lcpp) != len(lpy) or ap.returncode or gt.returncode or pt.returncode or r10.returncode or r10p.returncode or ui.returncode or gp.returncode else 0)


if __name__ == "__main__":
    main()
