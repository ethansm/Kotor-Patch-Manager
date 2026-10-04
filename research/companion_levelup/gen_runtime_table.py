#!/usr/bin/env python
"""Emit the runtime table the level-up override DLL reads (levelup_paths.ini) from paths/*.toml.

Usage: ../../.venv/bin/python gen_runtime_table.py [out.ini]   (2>/dev/null to hide pykotor noise)
Run check_paths.py first; this refuses to emit a path that has errors.

Format v3 = v2 + the two rule sections below (carry-over, LevelUpOverride 0.4.0; an old DLL that sees them rejects the table, an old table without them
makes the new DLL run with carry-over off). Format v2 (sections = lowercase <tag>__<variant>; dev paths use <tag>__live; '|' separated, all ids decimal):
  [presets]  <id>=<role>|<style>|<alignment>|<convert>|<skills>|<convert_at>|<hold_class>|<short>
  [atton__live]
  L<char_level>=<class>|<ability idx or 6>|<feat ids ,>|<power ids ,>|<skill:ranks ,>|<granted feat ids ,>|<class_level after this record>
  hold_class = class id of the first record whose class differs from the utc's newest class (0 = none)
  ability idx: 0 STR 1 DEX 2 CON 3 INT 4 WIS 5 CHA 6 none; skill = skills.2da row (0-7); ranks are DELTAS for that level
  (engine applies SetSkillRank(cur+delta)). granted feat ids = class-granted at that class level (kept from the engine's list).
  [recommended]  <tag>=<preset id>  the canon-style default the options page opens on (preset [meta] recommended = true, one per tag).
  [classskills]  cs<class>=<8 chars 0/1>  class-skill flag per skill (cost 1 vs 2) so the DLL can recompute unspent points.
  [featrules]   f<id>=<mincharlevel>|<req feat ids ,>|<any-of feat ids ,>|<class ids that may pick it ,>
  [powerrules]  p<id>=<prereq power ids ,>|<class>:<min character level ,> ...   (99 = never selectable, 0 = no minimum)
  Rows only for ids that occur in a record plus the transitive closure of their prerequisites. Sources: feat.2da mincharlevel, prereqfeat1/2 (>0),
  orreqfeat0..4 (>0), <abbr>_list in {0,1} per class id used by a record; spells.2da prerequisites ('_' separated), guardian/consular/sentinel/
  weapmstr/jedimaster/watchman. Class ids are < 16.
"""
import os, sys, tomllib
from levelup_rules import Rules, SKILLS, CLASS_ID
from check_paths import check
from pykotor.resource.generics.utc import read_utc

HERE = os.path.dirname(os.path.abspath(__file__))
ATTR = ["STR", "DEX", "CON", "INT", "WIS", "CHA"]
# feat.2da <abbr>_list column per class id (classes.2da ids 11-13 = Weapon Master / Master / Watchman -> jwm / jma / jwa)
FEAT_ABBR = {0: "sol", 1: "sct", 2: "scd", 3: "jgd", 4: "jcn", 5: "jsn", 6: "drc", 7: "drx", 9: "tec", 11: "jwm", 12: "jma", 13: "jwa"}
# spells.2da per-class minimum CHARACTER level columns (Jedi classes only)
POWER_COL = {3: "guardian", 4: "consular", 5: "sentinel", 11: "weapmstr", 12: "jedimaster", 13: "watchman"}
NEVER = 99   # runtime min level for "not selectable" (2da 99 / -1 / blank on a Jedi class)


def build_rules(R, classes, feats, powers):
    """[featrules] + [powerrules] lines (rows for the used ids plus the transitive closure of their prerequisites)."""
    def num(t, row, col, d=0):
        try:
            return int(R.cell(t, row, col))
        except ValueError:
            return d

    def feat_row(f):
        req = [x for x in (num(R.feat, f, "prereqfeat1"), num(R.feat, f, "prereqfeat2")) if x > 0]
        ors = [x for x in (num(R.feat, f, "orreqfeat%d" % k, -1) for k in range(5)) if x > 0]
        sel = [c for c in classes if c in FEAT_ABBR and R.cell(R.feat, f, FEAT_ABBR[c] + "_list") in ("0", "1")]
        return num(R.feat, f, "mincharlevel"), req, ors, sel

    def power_row(p):
        pre = [int(x) for x in R.cell(R.spells, p, "prerequisites").split("_") if x.isdigit()]
        mins = []
        for c in classes:
            if c in POWER_COL:
                v = num(R.spells, p, POWER_COL[c], -1)
                mins.append((c, v if 0 <= v <= 60 else NEVER))   # 0 = no minimum
        return pre, mins

    def closure(seed, row, deps):
        out, todo = {}, list(seed)
        while todo:
            x = todo.pop()
            if x not in out:
                out[x] = row(x)
                todo += deps(out[x])
        return out
    frows = closure(feats, feat_row, lambda r: r[1] + r[2])
    prows = closure(powers, power_row, lambda r: r[0])
    out = ["[featrules]", "; f<id>=<mincharlevel>|<req feat ids>|<any-of feat ids>|<class ids that may pick it>"]
    for f in sorted(frows):
        mc, req, ors, sel = frows[f]
        out.append("f%d=%d|%s|%s|%s" % (f, mc, ",".join(map(str, req)), ",".join(map(str, ors)), ",".join(map(str, sel))))
    out += ["", "[powerrules]", "; p<id>=<prereq power ids>|<class>:<min character level> (99 = never selectable, 0 = no minimum)"]
    for p in sorted(prows):
        pre, mins = prows[p]
        out.append("p%d=%s|%s" % (p, ",".join(map(str, pre)), ",".join("%d:%d" % m for m in mins)))
    return out


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "runtime", "levelup_paths.ini")
    R = Rules()
    files = [os.path.join(HERE, "paths", f) for f in sorted(os.listdir(os.path.join(HERE, "paths"))) if f.endswith(".toml")]
    pdir = os.path.join(HERE, "paths", "presets")
    files += [os.path.join(pdir, f) for f in sorted(os.listdir(pdir)) if f.endswith(".toml")]
    used, rows, body, recommended = set(), [], [], {}
    usedf, usedp = set(), set()
    st = dict(records=0, max_rec=0, max_lvl=0, feats=0, powers=0, granted=0, skills=0)
    for p in files:
        rel = os.path.relpath(p, HERE)
        tag, errs, warns, end = check(p, R)
        if errs:
            sys.exit(f"{rel}: path has {len(errs)} error(s); run check_paths.py: {errs[0]}")
        spec = tomllib.load(open(p, "rb"))
        m = spec["meta"]
        dev = os.path.dirname(p) == os.path.join(HERE, "paths")
        if dev:
            pid = f"{m['tag'].lower()}__live"
            wiz, short = "-|-|-|-|-", f"dev path ({rel.replace(os.sep, '/')})"
        else:
            pid = f"{m['tag'].lower()}__{m['variant'].lower()}"
            w = m["wizard"]
            wiz = "|".join(str(w[k]) for k in ("role", "style", "alignment", "convert", "skills"))
            short = m.get("short")
            if not short:
                sys.exit(f"{rel}: missing [meta] short")
            if len(short) > 60 or "|" in short or not short.isascii():
                sys.exit(f"{rel}: bad short {short!r} (<=60 chars, ASCII, no '|')")
        u = read_utc(open(os.path.join(HERE, 'live_utc', m['utc']), 'rb').read())
        clv = {c.class_id: c.class_level for c in u.classes}
        newest = u.classes[-1].class_id
        recs = []
        hold = 0
        for lv in spec["level"]:
            cls = lv["class"] if isinstance(lv["class"], int) else CLASS_ID[lv["class"]]
            if not hold and cls != newest:
                hold = cls
            used.add(cls)
            clv[cls] = clv.get(cls, 0) + 1
            gr = [x for x in range(R.feat.get_height()) if R.feat_granted_level(x, cls) == clv[cls]]
            attrs = lv.get("attrs", {})
            ab = ATTR.index(next(iter(attrs))) if attrs else 6
            sk = ",".join(f"{SKILLS.index(k)}:{v}" for k, v in lv.get("skills", {}).items())
            fe, po = lv.get("feats", []), lv.get("powers", [])
            usedf.update(fe); usedp.update(po)
            recs.append(f"L{lv['char_level']}={cls}|{ab}|{','.join(map(str, fe))}|"
                        f"{','.join(map(str, po))}|{sk}|{','.join(map(str, gr))}|{clv[cls]}")
            st["max_lvl"] = max(st["max_lvl"], lv["char_level"])
            st["feats"] = max(st["feats"], len(fe)); st["powers"] = max(st["powers"], len(po))
            st["granted"] = max(st["granted"], len(gr)); st["skills"] = max(st["skills"], len(lv.get("skills", {})))
        if "hold_until_class" in m and m["hold_until_class"] != hold:
            print(f"WARNING: {rel}: meta.hold_until_class={m['hold_until_class']} but computed hold_class={hold}")
        st["records"] += len(recs); st["max_rec"] = max(st["max_rec"], len(recs))
        rows.append(f"{pid}={wiz}|{m.get('convert_at', 0)}|{hold}|{short}")
        if not dev and m.get("recommended"):
            if m["tag"].lower() in recommended:
                sys.exit(f"{rel}: second recommended preset for {m['tag']} ({recommended[m['tag'].lower()]})")
            recommended[m["tag"].lower()] = pid
        body += ["", f"[{pid}]", f"name={m['name']}"] + recs
    lines = ["; generated by research/companion_levelup/gen_runtime_table.py - do not edit", "; format v3 (v2 + [featrules]/[powerrules])", "[classskills]"]
    for c in sorted(used):
        lines.append(f"cs{c}=" + "".join("1" if R.skill_is_class(i, c) else "0" for i in range(len(SKILLS))))
    lines += ["", "[presets]"] + rows
    lines += ["", "[recommended]"] + [f"{t}={p}" for t, p in sorted(recommended.items())]
    lines += [""] + build_rules(R, sorted(used), usedf, usedp)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    text = "\n".join(lines + body) + "\n"
    open(out, "w").write(text)
    print("wrote", out)
    print(f"presets={len(rows)} records={st['records']} max_records_per_preset={st['max_rec']} max_char_level={st['max_lvl']} "
          f"max_feats={st['feats']} max_powers={st['powers']} max_granted={st['granted']} max_skill_entries={st['skills']} "
          f"max_line_len={max(map(len, text.split(chr(10))))} bytes={len(text.encode())}")


if __name__ == "__main__":
    main()
