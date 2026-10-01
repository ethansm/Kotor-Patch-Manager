#!/usr/bin/env python3
"""Offline Python twin of the C++ dry-run rewrite (KOTOR II level-up override).

C++ twins: parseTable / lookupRecord / computeRewrite / skillCost / formatDry.
format_dry() output must be byte-identical to the C++ DRY log line.
Run: python3 simulate_rewrite.py   (self-tests against runtime/levelup_paths.ini)
"""
import os
import sys
from collections import namedtuple

NSKILL = 8
Rec = namedtuple("Rec", "cls ability feats powers skills granted")
ENGINE_GRANT_POWERS = (set(range(0x90, 0x95)) | set(range(0x9C, 0x9F)) |
                       set(range(0xA4, 0xAC)) | set(range(0x102, 0x105)) | {0x109})
INI = os.path.join(os.path.dirname(os.path.abspath(__file__)), "runtime", "levelup_paths.ini")


def _ids(s, ln):
    s = s.strip()
    return [int(x) for x in s.split(",")] if s else []


def parse_table(text):
    cs, recs, sect = {}, {}, None
    for n, raw in enumerate(text.splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith(";"):
            continue
        try:
            if line.startswith("[") and line.endswith("]"):
                sect = line[1:-1].strip().lower()
                if sect != "classskills":
                    recs.setdefault(sect, {})
                continue
            if sect is None or "=" not in line:
                raise ValueError("no section/key")
            k, v = line.split("=", 1)
            k, v = k.strip().lower(), v.strip()
            if sect == "classskills":
                if not k.startswith("cs") or len(v) != NSKILL or any(c not in "01" for c in v):
                    raise ValueError("bad cs line")
                cs[int(k[2:])] = [int(c) for c in v]
            elif k == "name":
                pass
            elif k.startswith("l"):
                f = v.split("|")
                if len(f) != 6:
                    raise ValueError("need 6 fields")
                ab = int(f[1])
                if not 0 <= ab <= 6:
                    raise ValueError("ability range")
                sk = []
                for p in (f[4].split(",") if f[4].strip() else []):
                    i, r = p.split(":")
                    if not 0 <= int(i) < NSKILL:
                        raise ValueError("skill idx")
                    sk.append((int(i), int(r)))
                recs[sect][int(k[1:])] = Rec(int(f[0]), ab, _ids(f[2], n), _ids(f[3], n), sk, _ids(f[5], n))
            else:
                raise ValueError("unknown key")
        except ValueError as e:
            raise ValueError("line %d: %s: %r" % (n, e, raw))
    return cs, recs


def lookup_record(recs, tag, key_level):
    return recs.get(tag.lower(), {}).get(key_level)


def skill_cost(deltas, mask):
    mask = mask if mask is not None else [1] * NSKILL
    return sum(deltas[i] * (1 if mask[i] else 2) for i in range(NSKILL))


def compute_rewrite(rec, pend, cs):
    def res(status, p=pend):
        return dict(status=status, feats=list(p["feats"]), spells=list(p["spells"]),
                    deltas=list(p["deltas"]), unspent=p["unspent"], ability=p["ability"])
    if rec is None:
        return res("no-record")
    if rec.cls != pend["cls"]:
        return res("class-mismatch")
    feats = [f for f in pend["feats"] if f in rec.granted]
    feats += [f for f in rec.feats if f not in feats]
    spells = [s for s in pend["spells"] if s in ENGINE_GRANT_POWERS]
    spells += [s for s in rec.powers if s not in spells]
    deltas = [0] * NSKILL
    for i, r in rec.skills:
        deltas[i] = r
    mask = cs.get(pend["cls"])
    unspent = pend["unspent"] + skill_cost(pend["deltas"], mask) - skill_cost(deltas, mask)
    if unspent < 0:
        return res("negative-unspent")
    ability = rec.ability if rec.ability != 6 else pend["ability"]
    new = dict(feats=feats, spells=spells, deltas=deltas, unspent=unspent, ability=ability)
    same = all(new[k] == pend[k] for k in new)
    return dict(status="already" if same else "rewrite", **new)


def format_dry(tag, key_level, res):
    j = lambda l: ",".join(str(x) for x in l)
    return ("DRY tag=%s key=%d status=%s feats=[%s] spells=[%s] deltas=[%s] unspent=%d ability=%d"
            % (tag, key_level, res["status"], j(res["feats"]), j(res["spells"]),
               j(res["deltas"]), res["unspent"], res["ability"]))


def main():
    cs, recs = parse_table(open(INI).read())
    fails = []

    def check(name, cond):
        print(("PASS " if cond else "FAIL ") + name)
        if not cond:
            fails.append(name)

    counts = {}
    for tag, lv in recs.items():
        ok = True
        for L, rec in lv.items():
            pend = dict(cls=rec.cls, feats=rec.granted + [9999], spells=[0x90, 8888],
                        deltas=[1] + [0] * 7, unspent=10, ability=6)
            r = compute_rewrite(rec, pend, cs)
            mask = cs.get(rec.cls)
            exp = 10 + skill_cost(pend["deltas"], mask) - skill_cost(
                [dict(rec.skills).get(i, 0) for i in range(NSKILL)], mask)
            if exp < 0:
                good = r["status"] == "negative-unspent"
            else:
                good = (r["status"] == "rewrite" and 9999 not in r["feats"]
                        and all(g in r["feats"] for g in rec.granted + rec.feats)
                        and 8888 not in r["spells"] and 0x90 in r["spells"]
                        and all(p in r["spells"] for p in rec.powers)
                        and r["deltas"] == [dict(rec.skills).get(i, 0) for i in range(NSKILL)]
                        and r["unspent"] == exp and r["unspent"] >= 0
                        and r["ability"] == (rec.ability if rec.ability != 6 else 6))
                if good:
                    p2 = dict(cls=rec.cls, **{k: r[k] for k in ("feats", "spells", "deltas", "unspent", "ability")})
                    good = compute_rewrite(rec, p2, cs)["status"] == "already"
            if not good:
                ok = False
                print("  bad record %s L%d: %s" % (tag, L, format_dry(tag, L, r)))
        counts[tag] = len(lv)
        check("all records %s (%d)" % (tag, len(lv)), ok)

    base = dict(cls=2, feats=[11, 62], spells=[], deltas=[0, 0, 2, 2, 0, 0, 0, 0], unspent=0, ability=3)
    check("missing tag", compute_rewrite(lookup_record(recs, "bao-dur", 5), base, cs)["status"] == "no-record")
    check("missing level", compute_rewrite(lookup_record(recs, "atton", 99), base, cs)["status"] == "no-record")
    rec = lookup_record(recs, "Atton", 5)
    check("case-insensitive tag", rec is not None and lookup_record(recs, "ATTON", 5) == rec)
    r = compute_rewrite(rec, dict(base, cls=4), cs)
    check("class mismatch unchanged", r["status"] == "class-mismatch" and r["feats"] == [11, 62]
          and r["deltas"] == base["deltas"] and r["unspent"] == 0 and r["ability"] == 3)
    r = compute_rewrite(rec, dict(base, deltas=[0] * 8, unspent=0), cs)
    check("negative unspent", r["status"] == "negative-unspent" and r["unspent"] == 0)
    r = compute_rewrite(rec, base, cs)
    exp_u = 0 + skill_cost(base["deltas"], cs[2]) - skill_cost([0, 0, 2, 1, 0, 0, 1, 0], cs[2])
    print("  " + format_dry("Atton", 5, r))
    check("Atton L5 GUI example", r["status"] == "rewrite" and r["feats"] == [62, 20]
          and r["deltas"] == [0, 0, 2, 1, 0, 0, 1, 0] and r["unspent"] == exp_u and exp_u >= 0)
    check("ability 6 keeps pend", r["ability"] == 3)
    r = compute_rewrite(lookup_record(recs, "atton", 4), dict(base, cls=2, feats=[], deltas=[0] * 8, unspent=20), cs)
    check("ability 1 overrides", r["status"] == "rewrite" and r["ability"] == 1)
    try:
        parse_table("[atton]\nL5=2|6|x||\n")
        check("malformed line raises", False)
    except ValueError:
        check("malformed line raises", True)

    print("records checked: " + ", ".join("%s=%d" % kv for kv in counts.items()))
    print("SUMMARY: %d failure(s)" % len(fails))
    sys.exit(1 if fails else 0)


if __name__ == "__main__":
    main()
