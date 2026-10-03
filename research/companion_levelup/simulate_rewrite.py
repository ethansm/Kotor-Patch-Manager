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
Rec = namedtuple("Rec", "cls ability feats powers skills granted clvl", defaults=(None,))
Preset = namedtuple("Preset", "role style alignment convert skills convert_at hold_class short")
ENGINE_GRANT_POWERS = (set(range(0x90, 0x95)) | set(range(0x9C, 0x9F)) |
                       set(range(0xA4, 0xAC)) | set(range(0x102, 0x105)) | {0x109})
INI = os.path.join(os.path.dirname(os.path.abspath(__file__)), "runtime", "levelup_paths.ini")


def _ids(s, ln):
    s = s.strip()
    return [int(x) for x in s.split(",")] if s else []


def parse_table(text):
    cs, _presets, recs = parse_table_v2(text)
    return cs, recs


def parse_table_v2(text):
    cs, recs, presets, sect = {}, {}, {}, None
    for n, raw in enumerate(text.splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith(";"):
            continue
        try:
            if line.startswith("[") and line.endswith("]"):
                sect = line[1:-1].strip().lower()
                if sect not in ("classskills", "presets", "recommended"):
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
            elif sect == "presets":
                f = v.split("|")
                if len(f) != 8:
                    raise ValueError("need 8 preset fields")
                presets[k] = Preset(f[0], f[1], f[2], f[3], f[4], int(f[5]), int(f[6]), f[7])
            elif sect == "recommended":
                pass   # <tag>=<preset id>: UI default only
            elif k == "name":
                pass
            elif k.startswith("l"):
                f = v.split("|")
                if len(f) not in (6, 7):
                    raise ValueError("need 6 or 7 fields")
                ab = int(f[1])
                if not 0 <= ab <= 6:
                    raise ValueError("ability range")
                sk = []
                for p in (f[4].split(",") if f[4].strip() else []):
                    i, r = p.split(":")
                    if not 0 <= int(i) < NSKILL:
                        raise ValueError("skill idx")
                    sk.append((int(i), int(r)))
                recs[sect][int(k[1:])] = Rec(int(f[0]), ab, _ids(f[2], n), _ids(f[3], n), sk, _ids(f[5], n),
                                            int(f[6]) if len(f) == 7 else None)
            else:
                raise ValueError("unknown key")
        except ValueError as e:
            raise ValueError("line %d: %s: %r" % (n, e, raw))
    return cs, presets, recs


def lookup_record(recs, tag, key_level):
    return recs.get(tag.lower(), {}).get(key_level)


def skill_cost(deltas, mask):
    mask = mask if mask is not None else [1] * NSKILL
    return sum(deltas[i] * (1 if mask[i] else 2) for i in range(NSKILL))


def compute_rewrite(rec, pend, cs, cs_extra=0):
    """cs_extra: bit i = skill i is a class skill through a Class Skill feat the creature already has (C++ csExtraMask)."""
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
    if mask is not None and cs_extra:
        mask = [1 if (m or (cs_extra >> i) & 1) else 0 for i, m in enumerate(mask)]
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


JEDI_CLASSES = (3, 4, 5, 11, 12, 13)   # Guardian, Consular, Sentinel, Weapon Master, Master, Watchman (classes.2da); twin of isJediClass in LevelUpOverride.cpp


def level_now(classes, preset, key, hold, hold_class, pause_at=0):
    """Hold rule v2 (+ 0.3.1 pause_at: last char level taken before banking, 0 = off). Returns (decision, char_level_or_None, rec)."""
    if preset is None or not classes:
        return ("VANILLA", None, None)
    cls, lvl = classes[-1]
    # A2 F2: a creature that converted to a DIFFERENT Jedi class is not held (the non-pause rule gives it VANILLA too)
    if hold and pause_at > 0 and hold_class and key > pause_at and not any(c == hold_class or c in JEDI_CLASSES for c, _ in classes):
        return ("HOLD", None, None)
    for cl in sorted(preset):
        r = preset[cl]
        if (r.cls, r.clvl) == (cls, lvl + 1):
            return ("PATH" if cl == key else "REBASE", cl, r)
    # HOLD only once the preset's records for the current (base) class are used up; below the path start -> vanilla
    top = max((r.clvl for r in preset.values() if r.cls == cls and r.clvl is not None), default=None)
    if hold and hold_class and hold_class not in [c for c, _ in classes] and top is not None and lvl >= top:
        return ("HOLD", None, None)
    return ("VANILLA", None, None)


def simulate_auto_levelup(classes, xp, xp_table, preset, hold, hold_class):
    """Engine loop twin. xp_table[total_level] = xp needed to leave that level."""
    classes = [list(c) for c in classes]
    out, first = [], True
    while sum(l for _, l in classes) < 50 and xp >= xp_table[sum(l for _, l in classes)]:
        key = sum(l for _, l in classes) + 1
        d, cl, rec = level_now([tuple(c) for c in classes], preset, key, hold, hold_class)
        if d == "HOLD":
            if first:
                return []
            d = "HOLD_MIDLOOP"   # the LevelUp hook can never veto (AutoLevelup would spin): engine picks apply
        first = False
        out.append((key, d, cl))
        classes[-1][1] += 1
    return out


def _syn_preset():
    R = lambda c, cl: Rec(c, 6, [], [], [], [], cl)
    p = {L: R(2, L) for L in range(4, 8)}
    p.update({L: R(5, L - 7) for L in range(8, 16)})
    return p


def level_now_tests(check):
    p = _syn_preset()
    ln = lambda c, k, hold=True, hc=5, pr=p: level_now(c, pr, k, hold, hc)[:2]
    check("ln on track", ln([(2, 5)], 6) == ("PATH", 6))
    check("ln early conv", ln([(2, 6), (5, 0)], 7) == ("REBASE", 8))
    check("ln early conv 2 (no re-apply)", ln([(2, 6), (5, 1)], 8) == ("REBASE", 9))
    check("ln late hold on", ln([(2, 7)], 8) == ("HOLD", None))
    check("ln late hold off", ln([(2, 7)], 8, hold=False) == ("VANILLA", None))
    droid = {L: Rec(6, 6, [], [], [], [], L) for L in range(1, 5)}
    check("ln droid on track", ln([(6, 2)], 3, hc=0, pr=droid) == ("PATH", 3))
    check("ln droid past end", ln([(6, 4)], 5, hc=0, pr=droid) == ("VANILLA", None))
    check("ln unconfigured", ln([(2, 5)], 6, pr=None) == ("VANILLA", None))
    xt = [0] * 50
    for i in range(50):
        xt[i] = 1000 * i
    xt[14], xt[15] = 105000, 120000
    check("atton banked vetoed", simulate_auto_levelup([(2, 7)], 110000, xt, p, True, 5) == [])
    sim = simulate_auto_levelup([(2, 7), (5, 0)], 110000, xt, p, True, 5)
    check("atton banked after Sentinel", [k for k, _, _ in sim] == list(range(8, 16))
          and all(d == "PATH" for _, d, _ in sim) and [c for _, _, c in sim] == list(range(8, 16)))
    mid = simulate_auto_levelup([(2, 6)], 10 ** 6, xt, p, True, 5)
    check("mid-loop HOLD -> vanilla levels", mid[0][1] == "PATH" and all(d == "HOLD_MIDLOOP" for _, d, _ in mid[1:]))
    check("ln below path start -> vanilla", ln([(2, 2)], 3) == ("VANILLA", None))
    lp = lambda c, k, pa, hold=True, hc=5, pr=p: level_now(c, pr, k, hold, hc, pa)[:2]
    check("ln pause: key == pauseAt not held", lp([(2, 5)], 6, 6) == ("PATH", 6))
    check("ln pause: key == pauseAt+1 held (PATH record exists)", lp([(2, 6)], 7, 6) == ("HOLD", None))
    check("ln pause: held far above", lp([(2, 4)], 8, 6) == ("HOLD", None))
    check("ln pause 0 = default", lp([(2, 6)], 7, 0) == ("PATH", 7))
    check("ln pause: hold off no effect", lp([(2, 6)], 7, 6, hold=False) == ("PATH", 7))
    check("ln pause: no hold class no effect", lp([(2, 6)], 7, 6, hc=0) == ("PATH", 7))
    check("ln pause: creature has class -> REBASE", lp([(2, 6), (5, 0)], 7, 6) == ("REBASE", 8))
    check("ln pause: unconfigured", lp([(2, 5)], 6, 3, pr=None) == ("VANILLA", None))
    check("ln pause: OTHER Jedi class (Guardian 3 vs hold Sentinel 5) -> VANILLA like the non-pause rule", lp([(2, 5), (3, 2)], 8, 3) == ln([(2, 5), (3, 2)], 8) == ("VANILLA", None))
    check("ln pause: other Jedi class at level 0 -> VANILLA", lp([(2, 5), (4, 0)], 6, 3) == ("VANILLA", None))
    check("ln pause: foreign non-Jedi class still held", lp([(2, 5), (99, 1)], 7, 3) == ("HOLD", None))
    check("ln pause >= default end = default", lp([(2, 7)], 8, 7) == ("HOLD", None) and lp([(2, 6)], 7, 7) == ("PATH", 7))
    v1 = "[classskills]\ncs2=11111111\n[t]\nL1=2|1|3||0:1|\n"
    cs, rc = parse_table(v1)
    check("v1 parses clvl None", rc["t"][1].clvl is None and cs[2] == [1] * 8)
    v2 = ("[classskills]\ncs2=11111111\n[presets]\na__b=r|s|al|c|sk|8|5|sh\n"
          "[a__b]\nname=X\nL4=2|1|3||0:1||4\n")
    cs, pi, rc = parse_table_v2(v2)
    check("v2 parses", rc["a__b"][4].clvl == 4 and pi["a__b"].convert_at == 8
          and pi["a__b"].hold_class == 5 and pi["a__b"].short == "sh" and parse_table(v2)[1] == rc)


def main():
    text = open(INI).read()
    cs, recs = parse_table(text)
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

    level_now_tests(check)
    if "atton" not in recs:
        print("records checked: " + ", ".join("%s=%d" % kv for kv in counts.items()))
        print("SUMMARY: %d failure(s)" % len(fails))
        sys.exit(1 if fails else 0)
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
