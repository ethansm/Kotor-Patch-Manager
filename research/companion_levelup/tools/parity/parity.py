#!/usr/bin/env python3
"""C++ vs Python parity for the level-up dry-run algorithm.

Builds parity_main.cpp (LevelUpOverride.cpp + stub/windows.h, native g++), generates cases from levelup_paths.ini (per record:
vanilla-like pend, replay of the result, class mismatch, negative unspent, random picks; plus missing records), runs both twins and
diffs the DRY lines (cases keyed by preset id, with a Class-Skill-feat csExtraMask); levelNow parity over every preset; then runs apply_test.cpp
(applyRewrite on fake engine structs) on the same cases gate_test.cpp (hold/instant/GUI-guard/Auto-OK handlers + M3 save-binding hooks/ptApply on fake engine structs) and playthrough_test.cpp (Playthrough.h). Usage: python3 tools/parity/parity.py   (from research/companion_levelup)
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


def case(tag, key, p, mask=0):
    j = lambda l: ",".join(map(str, l))
    return "%s %d %d %d %d %d|%s|%s|%s" % (tag, key, p["cls"], p["ability"], p["unspent"], mask, j(p["feats"]), j(p["spells"]), j(p["deltas"]))


def build(exe, src):
    r = subprocess.run(["g++", "-std=c++17", "-O1", "-Wall", "-Wextra", "-I", os.path.join(HERE, "stub"), "-I", os.path.join(HERE, "stub", "inc"), "-include", os.path.join(HERE, "stub", "m3_win_shim.h"), "-o", exe,
                        os.path.join(HERE, src)], capture_output=True, text=True)
    if r.returncode:
        sys.exit(r.stderr)


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


def main():
    build(EXE, "parity_main.cpp")
    build(APPLY_EXE, "apply_test.cpp")
    build(GATE_EXE, "gate_test.cpp")
    build(PT_EXE, "playthrough_test.cpp")
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
    sys.exit(1 if bad or lbad or len(cpp) != len(py) or len(lcpp) != len(lpy) or ap.returncode or gt.returncode or pt.returncode else 0)


if __name__ == "__main__":
    main()
