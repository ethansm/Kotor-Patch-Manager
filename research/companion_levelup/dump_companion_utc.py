#!/usr/bin/env python
"""Dump companion creature templates (p_*.utc) with feat/power/class names resolved via the game's 2DAs + dialog.tlk.
Usage: python dump_companion_utc.py <utc_dir> <game_dir> <out_json>"""
import sys, json, glob, os
from pykotor.resource.generics.utc import read_utc
from pykotor.extract.installation import Installation
from pykotor.resource.type import ResourceType

utc_dir, game_dir, out = sys.argv[1:4]
inst = Installation(game_dir)
tlk = inst.talktable()

def table(name):
    r = inst.resource(name, ResourceType.TwoDA)
    from pykotor.resource.formats.twoda import read_2da
    return read_2da(r.data)

def label(t, i, cols=("name", "label")):
    try:
        row = t.get_row(i)
    except Exception:
        return f"#{i}"
    for c in cols:
        try:
            v = row.get_string(c)
        except Exception:
            continue
        if c == "name" and v.isdigit():
            s = tlk.string(int(v))
            if s:
                return s
        elif v and v != "****":
            return v
    return f"#{i}"

feat, spells, classes = table("feat"), table("spells"), table("classes")
res = {}
for f in sorted(glob.glob(os.path.join(utc_dir, "p_*.utc"))):
    u = read_utc(open(f, "rb").read())
    d = {
        "attrs": dict(STR=u.strength, DEX=u.dexterity, CON=u.constitution, INT=u.intelligence, WIS=u.wisdom, CHA=u.charisma),
        "classes": [{"class": label(classes, c.class_id), "level": c.class_level,
                     "powers": [f"{p}:{label(spells, p)}" for p in c.powers]} for c in u.classes],
        "feats": [f"{x}:{label(feat, x)}" for x in sorted(u.feats)],
        "skills": {k: getattr(u, k) for k in ("computer_use", "demolitions", "stealth", "awareness", "persuade", "repair", "security", "treat_injury")},
        "hp_base": getattr(u, "hp", None), "fp_base": getattr(u, "fp", None),
    }
    res[os.path.basename(f)] = d
json.dump(res, open(out, "w"), indent=1)
for k, v in res.items():
    print(k, v["attrs"], [(c["class"], c["level"]) for c in v["classes"]], "feats:", len(v["feats"]), "powers:", sum(len(c["powers"]) for c in v["classes"]))
