#!/usr/bin/env python3
"""C++ vs Python parity for the level-up dry-run algorithm.

Builds parity_main.cpp (LevelUpOverride.cpp + stub/windows.h, native g++), generates cases from levelup_paths.ini (per record:
vanilla-like pend, replay of the result, class mismatch, negative unspent, random picks; plus missing records), runs both twins and
diffs the DRY lines; then runs apply_test.cpp (applyRewrite on fake engine structs) on the same cases. Usage: python3 tools/parity/parity.py   (from research/companion_levelup)
"""
import os, random, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, ROOT)
import simulate_rewrite as S

INI = os.path.join(ROOT, "runtime", "levelup_paths.ini")
EXE = os.path.join(HERE, "parity_bin")
APPLY_EXE = os.path.join(HERE, "apply_test_bin")


def case(tag, key, p):
    j = lambda l: ",".join(map(str, l))
    return "%s %d %d %d %d|%s|%s|%s" % (tag, key, p["cls"], p["ability"], p["unspent"], j(p["feats"]), j(p["spells"]), j(p["deltas"]))


def build(exe, src):
    r = subprocess.run(["g++", "-std=c++17", "-O1", "-Wall", "-Wextra", "-I", os.path.join(HERE, "stub"), "-I", os.path.join(HERE, "stub", "inc"), "-o", exe,
                        os.path.join(HERE, src)], capture_output=True, text=True)
    if r.returncode:
        sys.exit(r.stderr)


def main():
    build(EXE, "parity_main.cpp")
    build(APPLY_EXE, "apply_test.cpp")
    cs, recs = S.parse_table(open(INI).read())
    rnd = random.Random(1)
    cases = []
    for tag, lv in recs.items():
        for key, rec in lv.items():
            base = dict(cls=rec.cls, ability=rnd.choice([1, 3, 6]), unspent=rnd.randint(0, 12), feats=list(rec.granted) + [9999],
                        spells=[0x90, 8888, 0x109], deltas=[rnd.randint(0, 2) for _ in range(8)])
            out = S.compute_rewrite(rec, base, cs)
            replay = dict(cls=rec.cls, **{k: out[k] for k in ("ability", "unspent", "feats", "spells", "deltas")})
            for p in (base, replay, dict(base, cls=rec.cls + 1), dict(base, unspent=0, deltas=[0] * 8),
                      dict(base, feats=[rnd.randint(0, 250) for _ in range(rnd.randint(0, 4))], spells=[rnd.randint(0, 300) for _ in range(3)])):
                cases.append((tag, key, p))
    cases += [("atton", 99, cases[0][2]), ("bao-dur", 5, cases[0][2]), ("Kreia", 6, cases[-1][2])]
    text = "\n".join(case(*c) for c in cases) + "\n"
    cpp = subprocess.run([EXE, INI], input=text, capture_output=True, text=True, check=True).stdout.splitlines()
    py = [S.format_dry(t, k, S.compute_rewrite(S.lookup_record(recs, t, k), p, cs)) for t, k, p in cases]
    bad = [(a, b) for a, b in zip(cpp, py) if a != b]
    statuses = {}
    for l in py:
        st = l.split("status=")[1].split()[0]
        statuses[st] = statuses.get(st, 0) + 1
    print("cases", len(cases), "cpp lines", len(cpp), "mismatches", len(bad), "statuses", statuses)
    for a, b in bad[:5]:
        print(" C++:", a, "\n  Py:", b)
    # applyRewrite on fake engine pending structs, same cases (apply_test.cpp)
    ap = subprocess.run([APPLY_EXE, INI], input=text, capture_output=True, text=True)
    print(ap.stdout.strip())
    if ap.stderr:
        print(ap.stderr[:4000])
    sys.exit(1 if bad or len(cpp) != len(py) or ap.returncode else 0)


if __name__ == "__main__":
    main()
