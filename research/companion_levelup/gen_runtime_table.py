#!/usr/bin/env python
"""Emit the runtime table the level-up override DLL reads (levelup_paths.ini) from paths/*.toml.

Usage: ../../.venv/bin/python gen_runtime_table.py [out.ini]   (2>/dev/null to hide pykotor noise)
Run check_paths.py first; this refuses to emit a path that has errors.

Format v2 (sections = lowercase <tag>__<variant>; dev paths use <tag>__live; '|' separated, all ids decimal):
  [presets]  <id>=<role>|<style>|<alignment>|<convert>|<skills>|<convert_at>|<hold_class>|<short>
  [atton__live]
  L<char_level>=<class>|<ability idx or 6>|<feat ids ,>|<power ids ,>|<skill:ranks ,>|<granted feat ids ,>|<class_level after this record>
  hold_class = class id of the first record whose class differs from the utc's newest class (0 = none)
  ability idx: 0 STR 1 DEX 2 CON 3 INT 4 WIS 5 CHA 6 none; skill = skills.2da row (0-7); ranks are DELTAS for that level
  (engine applies SetSkillRank(cur+delta)). granted feat ids = class-granted at that class level (kept from the engine's list).
  [recommended]  <tag>=<preset id>  the canon-style default the options page opens on (preset [meta] recommended = true, one per tag).
  [classskills]  cs<class>=<8 chars 0/1>  class-skill flag per skill (cost 1 vs 2) so the DLL can recompute unspent points.
"""
import os, sys, tomllib
from levelup_rules import Rules, SKILLS, CLASS_ID
from check_paths import check
from pykotor.resource.generics.utc import read_utc

HERE = os.path.dirname(os.path.abspath(__file__))
ATTR = ["STR", "DEX", "CON", "INT", "WIS", "CHA"]


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "runtime", "levelup_paths.ini")
    R = Rules()
    files = [os.path.join(HERE, "paths", f) for f in sorted(os.listdir(os.path.join(HERE, "paths"))) if f.endswith(".toml")]
    pdir = os.path.join(HERE, "paths", "presets")
    files += [os.path.join(pdir, f) for f in sorted(os.listdir(pdir)) if f.endswith(".toml")]
    used, rows, body, recommended = set(), [], [], {}
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
    lines = ["; generated by research/companion_levelup/gen_runtime_table.py - do not edit", "; format v2", "[classskills]"]
    for c in sorted(used):
        lines.append(f"cs{c}=" + "".join("1" if R.skill_is_class(i, c) else "0" for i in range(len(SKILLS))))
    lines += ["", "[presets]"] + rows
    lines += ["", "[recommended]"] + [f"{t}={p}" for t, p in sorted(recommended.items())]
    os.makedirs(os.path.dirname(out), exist_ok=True)
    text = "\n".join(lines + body) + "\n"
    open(out, "w").write(text)
    print("wrote", out)
    print(f"presets={len(rows)} records={st['records']} max_records_per_preset={st['max_rec']} max_char_level={st['max_lvl']} "
          f"max_feats={st['feats']} max_powers={st['powers']} max_granted={st['granted']} max_skill_entries={st['skills']} "
          f"max_line_len={max(map(len, text.split(chr(10))))} bytes={len(text.encode())}")


if __name__ == "__main__":
    main()
