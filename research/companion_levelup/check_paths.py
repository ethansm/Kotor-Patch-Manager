#!/usr/bin/env python
"""Validate paths/<char>.toml against the LIVE rule tables so a path can never ask for an unavailable pick.

Usage: ../../.venv/bin/python check_paths.py [paths/atton.toml ...]   (2>/dev/null to hide pykotor debug noise)
Starting state comes from live_utc/<meta.utc> (the companion's template in the live Override).
Checks per level: sequential char_level, class change legality, feat selectable for the class + slot count
(featgain.2da), feat prereqs (feats/or-feats/min char level/attrs), duplicate feats, power min class level
(spells.2da), power prereq spell known, power slot count (classpowergain.2da), skill points (base + INT mod;
cross-class 2 per rank) and max rank (char_level + 3). Attack-bonus prereqs are NOT checked (needs class tables + mods).
Exit status 1 if any path has errors."""
import sys, glob, os, tomllib
from levelup_rules import Rules, SKILLS, JEDI, CLASS_ID
from pykotor.resource.generics.utc import read_utc

HERE = os.path.dirname(os.path.abspath(__file__))
UTC_SKILL_ATTR = {s: s for s in SKILLS}


def check(path, R):
    spec = tomllib.load(open(path, "rb"))
    meta = spec["meta"]
    errs, warns = [], []
    u = read_utc(open(os.path.join(HERE, "live_utc", meta["utc"]), "rb").read())
    feats = set(u.feats)
    powers = {p for c in u.classes for p in c.powers}
    clv = {c.class_id: c.class_level for c in u.classes}
    skills = {s: getattr(u, s) for s in SKILLS}
    attrs = dict(STR=u.strength, DEX=u.dexterity, CON=u.constitution, INT=u.intelligence, WIS=u.wisdom, CHA=u.charisma)
    cl = sum(clv.values())
    for lv in spec["level"]:
        tag = f"{meta['tag']} L{lv['char_level']}"
        e = lambda m: errs.append(f"{tag}: {m}")
        cl += 1
        if lv["char_level"] != cl:
            e(f"char_level {lv['char_level']} not sequential (expected {cl})"); cl = lv["char_level"]
        cls = lv["class"]
        if isinstance(cls, str):
            cls = CLASS_ID[cls]
        if cls not in (0, 1, 2, 3, 4, 5, 9):
            e(f"class {cls} unsupported")
            continue
        attrs.update(lv.get("attrs", {}))
        clv[cls] = clv.get(cls, 0) + 1
        k = clv[cls]
        if cls in (3, 4, 5) and not any(c in clv and c != cls and c in (3, 4, 5) for c in ()):  # single-Jedi-class rule
            other = [c for c in clv if c in (3, 4, 5) and c != cls]
            if other:
                e(f"mixes Jedi classes {other} + {cls}")
        # --- feats ---
        pf = lv.get("feats", [])
        slots = R.feat_slots(cls, k)
        if len(pf) > slots:
            e(f"{len(pf)} feat picks but class level {k} only grants {slots} slot(s)")
        elif len(pf) < slots:
            warns.append(f"{tag}: {len(pf)}/{slots} feat slot(s) used (unspent = engine default may fill)")
        # class-granted feats arrive at this level too (make them known so prereqs can chain)
        granted = [f for f in range(R.feat.get_height()) if R.feat_granted_level(f, cls) == k]
        newly = set(granted)
        for f in pf:
            lab = R.feat_label(f)
            if not R.feat_selectable(f, cls):
                e(f"feat {lab} not selectable for class {cls}")
            if f in feats or f in newly:
                e(f"feat {lab} already known/granted")
            pr = R.feat_prereqs(f)
            miss = [R.feat_label(x) for x in pr["req"] if x not in feats and x not in newly]
            if miss:
                e(f"feat {lab} missing prereq feat(s) {miss}")
            if pr["orreq"] and not any(x in feats or x in newly for x in pr["orreq"]):
                e(f"feat {lab} needs one of {[R.feat_label(x) for x in pr['orreq']]}")
            if pr["minchar"] > lv["char_level"]:
                e(f"feat {lab} needs character level {pr['minchar']}")
            for a in ("str", "dex", "int", "wis"):
                if pr[a] > attrs[a.upper()]:
                    e(f"feat {lab} needs {a.upper()} {pr[a]} (have {attrs[a.upper()]})")
            if pr["minattack"]:
                warns.append(f"{tag}: feat {lab} needs base attack {pr['minattack']} (not verified)")
            feats.add(f)
        feats |= newly
        # --- powers ---
        pw = lv.get("powers", [])
        if cls in JEDI:
            pslots = R.power_slots(cls, k)
            if len(pw) > pslots:
                e(f"{len(pw)} power picks but Jedi level {k} grants {pslots}")
            elif len(pw) < pslots:
                warns.append(f"{tag}: {len(pw)}/{pslots} power slot(s) used")
            for p in pw:
                lab = R.spell_label(p)
                ml = R.power_min_class_level(p, cls)
                if ml < 0 or ml > 60:
                    e(f"power {lab} not selectable for class {cls} (min level {ml})")
                elif ml > lv["char_level"]:
                    e(f"power {lab} needs character level {ml}")
                miss = [R.spell_label(x) for x in R.power_prereqs(p) if x not in powers and x not in pw]
                if miss:
                    e(f"power {lab} missing prereq power(s) {miss}")
                if p in powers:
                    e(f"power {lab} already known")
                powers.add(p)
        elif pw:
            e("powers listed for a non-Jedi class")
        # --- skills ---
        sk = lv.get("skills", {})
        cost = 0
        for name, n in sk.items():
            if name not in SKILLS:
                e(f"unknown skill {name}"); continue
            idx = SKILLS.index(name)
            cost += n * (1 if R.skill_is_class(idx, cls) else 2)
            skills[name] += n
            if skills[name] > lv["char_level"] + 3:
                e(f"skill {name} rank {skills[name]} exceeds max {lv['char_level'] + 3}")
        avail = R.skill_points(cls, attrs["INT"])
        if cost > avail:
            e(f"skill cost {cost} > {avail} points")
        elif cost < avail:
            warns.append(f"{tag}: {avail - cost} skill point(s) unspent")
    return meta["tag"], errs, warns, dict(level=cl, classes=clv, feats=len(feats), powers=len(powers), skills=skills)


if __name__ == "__main__":
    files = sys.argv[1:] or sorted(glob.glob(os.path.join(HERE, "paths", "*.toml")))
    R = Rules()
    bad = 0
    for f in files:
        tag, errs, warns, end = check(f, R)
        print(f"== {tag}: {len(errs)} error(s), {len(warns)} warning(s)  end={end}")
        for m in errs:
            print("  ERROR", m)
        for m in warns:
            print("  warn ", m)
        bad += bool(errs)
    sys.exit(1 if bad else 0)
