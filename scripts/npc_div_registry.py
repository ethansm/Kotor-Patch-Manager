#!/usr/bin/env python3
"""NPC registry for npc-diversity role-aware keep-default (Track A, offline; read-only on game files).

Scans every module's GIT creature list + UTC templates (modules deduped by lowercase stem) and writes one JSON line per
creature instance: module, tag, template, name text, appearance row, conversation, plot, interruptable, faction,
scriptspawn, position, generic-table family, and a rule-based first-pass role.

Usage (needs pykotor: use .venv/bin/python):
  npc_div_registry.py build [--out FILE] [--modules DIR] [--only STEM[,STEM]]
  npc_div_registry.py pairs [--registry FILE] [--out DIR] [--batch 40]   # distinct (tag,name) pairs for Haiku batches
  npc_div_registry.py report [--registry FILE]

Roles (rule pass): plot (Plot=1 or Interruptable=0), named (a conversation used by <4 instances, or a non-templated name on
<=2 instances), else generic. Haiku later refines generic|named|merchant|uniformed|plot for the generic-table creatures.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
TABLES_H = REPO / "patch_manager_mods/npc_diversity_src/DiversityTables.h"
DEFAULT_REG = REPO / ".claude/npc_div/registry.jsonl"
DEFAULT_ROLES = REPO / ".claude/npc_div/roles"
POS_TOL = 0.25          # m, position-fallback match tolerance
POS_ROUND = 0.1


def parse_tables(path=TABLES_H):
    """{'M': {rows}, 'F': ..., 'TM': ..., 'TF': ...} from the IsGeneric* switch bodies (never transcribed)."""
    src = Path(path).read_text()
    out = {}
    for fn, fam in (("IsGenericMaleCommoner", "M"), ("IsGenericFemaleCommoner", "F"),
                    ("IsGenericTwilekMale", "TM"), ("IsGenericTwilekFemale", "TF")):
        m = re.search(r"inline bool %s\(.*?\n\}" % fn, src, re.S)
        if not m:
            raise SystemExit("table %s not found in %s" % (fn, path))
        out[fam] = {int(x) for x in re.findall(r"case\s+(\d+)\s*:", m.group(0))}
    return out


def family_of(app, tables):
    for fam in ("M", "F", "TM", "TF"):
        if app in tables[fam]:
            return fam
    return ""


def module_files(mod_dir):
    """{stem_lower: {'mod': Path|None, 'rim': Path|None, 's_rim': Path|None}} deduped by lowercase name."""
    out = defaultdict(dict)
    for p in sorted(Path(mod_dir).iterdir()):
        n = p.name.lower()
        if n.endswith("_s.rim"):
            out[n[:-6]].setdefault("s_rim", p)
        elif n.endswith(".rim"):
            out[n[:-4]].setdefault("rim", p)
        elif n.endswith(".mod"):
            out[n[:-4]].setdefault("mod", p)
    return out


def _load_container(path):
    from pykotor.resource.formats.erf import read_erf
    from pykotor.resource.formats.rim import read_rim
    from pykotor.resource.type import ResourceType
    c = read_rim(str(path)) if path.suffix.lower() == ".rim" else read_erf(str(path))
    res = {}
    for r in c:
        res[(str(r.resref).lower(), r.restype)] = r.data
    return res, ResourceType


def module_resources(files):
    """(git_bytes_by_stem, utc_bytes_by_resref) for one module; .mod wins over .rim, _s.rim adds templates."""
    git, utc = {}, {}
    order = [files.get("mod")] if files.get("mod") else [files.get("rim"), files.get("s_rim")]
    for p in order:
        if not p:
            continue
        res, RT = _load_container(p)
        for (name, rt), data in res.items():
            if rt == RT.GIT:
                git.setdefault(name, data)
            elif rt == RT.UTC:
                utc.setdefault(name, data)
    return git, utc


class Tlk:
    def __init__(self, path):
        from pykotor.resource.formats.tlk import read_tlk
        self.t = read_tlk(str(path))
        self.cache = {}

    def text(self, ref):
        if ref is None or ref < 0:
            return ""
        if ref not in self.cache:
            try:
                self.cache[ref] = self.t[ref].text.strip()
            except Exception:
                self.cache[ref] = ""
        return self.cache[ref]


def locstring_text(ls, tlk):
    """Text of a GFF locstring: TLK strref when set, else the first embedded substring."""
    if ls is None:
        return ""
    ref = getattr(ls, "stringref", -1)
    if ref is not None and ref >= 0:
        return tlk.text(ref)
    try:
        for _lang, _gender, text in ls:
            if text:
                return text.strip()
    except Exception:
        pass
    return ""


def _get(gff_root, key, default=None):
    try:
        if gff_root.exists(key):
            return gff_root.acquire(key, default)
    except Exception:
        pass
    return default


def build(mod_dir, out, only=None):
    from pykotor.resource.formats.gff import read_gff
    tables = parse_tables()
    tlk = Tlk(GAME / "dialog.tlk")
    files = module_files(mod_dir)
    rows = []
    stems = sorted(files)
    if only:
        stems = [s for s in stems if s in only]
    for stem in stems:
        try:
            git_by, utc_by = module_resources(files[stem])
        except Exception as e:
            print("WARN module %s unreadable: %s" % (stem, e), file=sys.stderr)
            continue
        git = git_by.get(stem) or (next(iter(git_by.values())) if git_by else None)
        if git is None:
            continue
        root = read_gff(git).root
        creatures = _get(root, "Creature List") or []
        utc_cache = {}
        for c in creatures:
            tpl = str(_get(c, "TemplateResRef", "") or "").lower()
            if tpl not in utc_cache:
                utc_cache[tpl] = read_gff(utc_by[tpl]).root if tpl in utc_by else None
            u = utc_cache[tpl]
            if u is None:
                rows.append(dict(module=stem, template=tpl, unresolved=True))
                continue
            tag = str(_get(c, "Tag", "") or "") or str(_get(u, "Tag", "") or "")
            app = int(_get(u, "Appearance_Type", -1))
            rows.append(dict(
                module=stem, template=tpl, tag=tag,
                name=locstring_text(_get(u, "FirstName"), tlk),
                app=app, fam=family_of(app, tables),
                conv=str(_get(u, "Conversation", "") or ""),
                plot=int(_get(u, "Plot", 0) or 0), interruptable=int(_get(u, "Interruptable", 1) if _get(u, "Interruptable", None) is not None else 1),
                faction=int(_get(u, "FactionID", 0) or 0), spawn=str(_get(u, "ScriptSpawn", "") or ""),
                race=int(_get(u, "Race", 0) or 0),
                x=round(float(_get(c, "XPosition", 0.0)), 2), y=round(float(_get(c, "YPosition", 0.0)), 2),
                z=round(float(_get(c, "ZPosition", 0.0)), 2)))
    names = Counter(r["name"] for r in rows if r.get("name"))
    convs = Counter(r["conv"] for r in rows if r.get("conv"))
    for r in rows:
        if r.get("unresolved"):
            continue
        r["name_n"] = names[r["name"]] if r.get("name") else 0
        r["conv_n"] = convs[r["conv"]] if r.get("conv") else 0
        r["role"] = rule_role(r)
    out = Path(out)
    out.parent.mkdir(parents=True, exist_ok=True)
    with out.open("w") as f:
        for r in rows:
            f.write(json.dumps(r, sort_keys=True) + "\n")
    print("modules=%d creatures=%d -> %s" % (len(stems), len(rows), out))
    report(out)


GENERIC_CONV_MIN = 4    # a conversation shared by >= this many instances is a generic bark dialog, not a named NPC


def rule_role(r):
    """Hint only (Haiku decides). Generic commoners carry shared bark conversations ('200comm') and templated names
    ('Traveler{M1}'), so those are not named signals."""
    if r.get("plot") or not r.get("interruptable", 1):
        return "plot"
    if r.get("conv") and r.get("conv_n", 0) < GENERIC_CONV_MIN:
        return "named"
    if r.get("name") and "{" not in r["name"] and r.get("name_n", 0) <= 2:
        return "named"
    return "generic"


def load(reg):
    return [json.loads(line) for line in Path(reg).read_text().splitlines() if line.strip()]


def pairs(reg, out, batch):
    rows = [r for r in load(reg) if r.get("fam")]
    seen = {}
    for r in rows:
        k = (r["tag"], r["name"])
        d = seen.setdefault(k, dict(tag=r["tag"], name=r["name"], n=0, rule=Counter(), mods=set(), conv=0, plot=0, apps=set(), conv_n=0, conv_ids=set()))
        d["n"] += 1
        d["rule"][r["role"]] += 1
        d["mods"].add(r["module"])
        d["conv"] += 1 if r["conv"] else 0
        d["plot"] += r["plot"]
        d["apps"].add(r["app"])
        d["conv_n"] = max(d["conv_n"], r.get("conv_n", 0))
        if r["conv"]:
            d["conv_ids"].add(r["conv"])
    items = []
    for d in sorted(seen.values(), key=lambda d: (d["tag"].lower(), d["name"])):
        items.append(dict(tag=d["tag"], name=d["name"], n=d["n"], rule=d["rule"].most_common(1)[0][0],
                          mods=sorted(d["mods"])[:3], conv=sorted(d["conv_ids"])[:2], conv_shared=d["conv_n"], plot=d["plot"],
                          apps=sorted(d["apps"])))
    out = Path(out)
    out.mkdir(parents=True, exist_ok=True)
    files = []
    for i in range(0, len(items), batch):
        p = out / ("pairs_%03d.json" % (i // batch))
        p.write_text(json.dumps(items[i:i + batch], indent=0))
        files.append(p.name)
    print("distinct (tag,name) pairs=%d batches=%d (batch=%d) -> %s" % (len(items), len(files), batch, out))


def rounded(v):
    return round(round(v / POS_ROUND) * POS_ROUND, 1)


def ambiguity(rows, keep_pred):
    """Position-fallback ambiguity per module: share of keep-default instances that have a NON-keep instance within
    POS_TOL (so a position-keyed keep entry would also keep that generic NPC)."""
    by_mod = defaultdict(list)
    for r in rows:
        if r.get("fam"):
            by_mod[r["module"]].append(r)
    keep = amb = 0
    for rs in by_mod.values():
        for a in rs:
            if not keep_pred(a):
                continue
            keep += 1
            if any(b is not a and not keep_pred(b) and abs(a["x"] - b["x"]) <= POS_TOL and abs(a["y"] - b["y"]) <= POS_TOL
                   for b in rs):
                amb += 1
    return keep, amb


def report(reg):
    rows = [r for r in load(reg) if not r.get("unresolved")]
    tabled = [r for r in rows if r.get("fam")]
    print("instances=%d unresolved=%d in-generic-tables=%d" % (len(rows), len(load(reg)) - len(rows), len(tabled)))
    print("rule roles (tabled): " + ", ".join("%s=%d" % kv for kv in Counter(r["role"] for r in tabled).most_common()))
    print("families: " + ", ".join("%s=%d" % kv for kv in Counter(r["fam"] for r in tabled).most_common()))
    k, a = ambiguity(rows, lambda r: r["role"] != "generic")
    print("position fallback (rule keep-default): keep=%d ambiguous=%d (%.1f%%, target <2%%)" % (k, a, 100.0 * a / k if k else 0))
    tags = Counter(r["tag"] for r in tabled)
    print("tabled distinct tags=%d, top: %s" % (len(tags), ", ".join("%s=%d" % kv for kv in tags.most_common(5))))


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    b = sub.add_parser("build")
    b.add_argument("--out", default=str(DEFAULT_REG))
    b.add_argument("--modules", default=str(GAME / "modules"))
    b.add_argument("--only", default="")
    p = sub.add_parser("pairs")
    p.add_argument("--registry", default=str(DEFAULT_REG))
    p.add_argument("--out", default=str(DEFAULT_ROLES))
    p.add_argument("--batch", type=int, default=40)
    r = sub.add_parser("report")
    r.add_argument("--registry", default=str(DEFAULT_REG))
    a = ap.parse_args(argv)
    if a.cmd == "build":
        build(a.modules, a.out, {s.lower() for s in a.only.split(",") if s} or None)
    elif a.cmd == "pairs":
        pairs(a.registry, a.out, a.batch)
    else:
        report(a.registry)
    return 0


if __name__ == "__main__":
    sys.exit(main())
