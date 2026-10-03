#!/usr/bin/env python3
"""Check the GUI control-id registry tests/static/gui_ids.toml against the .gui files (doc 20 section 6).

  .venv/bin/python scripts/check_gui_ids.py [--quiet] [--registry R] [--originals DIR] [--override DIR]

Static ranges in one .gui must not overlap or reach into the vanilla ids, the live override/<file> must hold exactly those
ids with the registered tags, nominal rects of different mods must not intersect, and each entry's source_regex must still
match its C++ source. Append entries (controls a mod creates at runtime) are not collision-checked; their runtime id range
is printed. Schema: header of the registry. Needs pykotor (the repo .venv) only to read the .gui files; everything else is
pure functions over (id, tag, rect) tuples. Read-only toward the game dir.

Output lines: FAIL|WARN|INFO|PASS|SKIP check-gui-ids: msg. --quiet hides PASS and INFO. Exit 1 on any FAIL.
"""
import argparse
import os
import logging
import re
import sys
import tomllib
from pathlib import Path
from typing import NamedTuple

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(Path(__file__).resolve().parent))
CHECK = "check-gui-ids"
DEFAULT_REGISTRY = REPO / "tests" / "static" / "gui_ids.toml"
DEFAULT_ORIGINALS = Path("~/Games/KOTOR2_Mod_Workspace/ultrawide_gui/originals").expanduser()


class Finding(NamedTuple):
    level: str   # FAIL | WARN | INFO | PASS | SKIP
    msg: str


class Control(NamedTuple):
    id: int
    tag: str
    rect: tuple   # (left, top, width, height) in 800x600 units


# ---- reading .gui files (pykotor) ---------------------------------------------------------------------------------------

def load_controls(path) -> list[Control]:
    """Top-level CONTROLS of a .gui as Control tuples (ImportError if pykotor is missing)."""
    logging.disable(logging.CRITICAL)   # pykotor logs at import and read
    from pykotor.resource.formats.gff import read_gff
    out = []
    for c in read_gff(Path(path)).root.get_list("CONTROLS"):
        e = c.get_struct("EXTENT")
        out.append(Control(c.get_int32("ID"), c.get_string("TAG"),
                           tuple(e.get_int32(k) for k in ("LEFT", "TOP", "WIDTH", "HEIGHT"))))
    return out


# ---- pure checks --------------------------------------------------------------------------------------------------------

def _label(gui, e):
    return f"{gui} {e['mod']}"


def rects_intersect(a, b) -> bool:
    """True if two [left, top, width, height] rects share area (touching edges do not count)."""
    return a[0] < b[0] + b[2] and b[0] < a[0] + a[2] and a[1] < b[1] + b[3] and b[1] < a[1] + a[3]


def check_static_overlap(gui, statics) -> list[Finding]:
    out = []
    for i, a in enumerate(statics):
        lo, hi = a["ids"]
        if lo > hi:
            out.append(Finding("FAIL", f"{_label(gui, a)}: ids [{lo}, {hi}] is empty or reversed"))
        for b in statics[i + 1:]:
            if lo <= b["ids"][1] and b["ids"][0] <= hi:
                out.append(Finding("FAIL", f"{gui}: static id ranges overlap: {a['mod']} {lo}..{hi} and "
                                           f"{b['mod']} {b['ids'][0]}..{b['ids'][1]}"))
    return out


def check_vs_vanilla(gui, statics, vanilla_max) -> list[Finding]:
    return [Finding("FAIL", f"{_label(gui, e)}: static range {e['ids'][0]}..{e['ids'][1]} starts at or below the vanilla "
                            f"max id {vanilla_max} of {gui}")
            for e in statics if e["ids"][0] <= vanilla_max]


def check_rect_collisions(gui, entries) -> list[Finding]:
    """Nominal rects (static and append alike) of different mods must not intersect."""
    out = []
    for i, a in enumerate(entries):
        for b in entries[i + 1:]:
            if a["mod"] == b["mod"]:
                continue
            for ra in a.get("rects", []):
                for rb in b.get("rects", []):
                    if rects_intersect(ra, rb):
                        out.append(Finding("FAIL", f"{gui}: nominal rects of {a['mod']} {list(ra)} and "
                                                   f"{b['mod']} {list(rb)} intersect"))
    return out


def _tag_ok(entry, ctl_id, tag) -> bool:
    if "tags" in entry:
        i = ctl_id - entry["ids"][0]
        return 0 <= i < len(entry["tags"]) and tag == entry["tags"][i]
    p = entry.get("tag_prefix")
    if p is None:
        return True
    return tag.startswith(tuple([p] if isinstance(p, str) else p))


def check_live(gui, statics, live: list[Control], vanilla_max) -> list[Finding]:
    """The live file must hold exactly the registered ids above the vanilla max, once each, with matching tags."""
    out = []
    by_id = {}
    for c in live:
        by_id.setdefault(c.id, []).append(c)
    for i, ctls in sorted(by_id.items()):
        if len(ctls) > 1:
            out.append(Finding("FAIL", f"{gui}: live file has {len(ctls)} controls with id {i} ({', '.join(c.tag for c in ctls)})"))
    registered = set()
    for e in statics:
        lo, hi = e["ids"]
        if "tags" in e and len(e["tags"]) != hi - lo + 1:
            out.append(Finding("FAIL", f"{_label(gui, e)}: {len(e['tags'])} tags listed for ids {lo}..{hi} ({hi - lo + 1})"))
        bad = []
        for i in range(lo, hi + 1):
            registered.add(i)
            if i not in by_id:
                bad.append(f"id {i} missing")
            elif not _tag_ok(e, i, by_id[i][0].tag):
                bad.append(f"id {i} has tag {by_id[i][0].tag!r}")
        if bad:
            out.append(Finding("FAIL", f"{_label(gui, e)}: live {gui}: " + "; ".join(bad[:6]) + (f"; +{len(bad) - 6} more" if len(bad) > 6 else "")))
        else:
            out.append(Finding("PASS", f"{_label(gui, e)}: live {gui} holds ids {lo}..{hi} with the registered tags"))
    stray = sorted(i for i in by_id if i > vanilla_max and i not in registered)
    if stray:
        out.append(Finding("FAIL", f"{gui}: live file has unregistered ids above the vanilla max {vanilla_max}: "
                                   f"{stray[:8]}{'...' if len(stray) > 8 else ''}"))
    return out


def append_bases(gui, statics, appends, vanilla_next) -> list[Finding]:
    """INFO line per append: runtime ids = vanilla next free id + static sizes of the mods in `after`."""
    out = []
    size = {e["mod"]: e["ids"][1] - e["ids"][0] + 1 for e in statics}
    for a in appends:
        missing = [m for m in a.get("after", []) if m not in size]
        if missing:
            out.append(Finding("FAIL", f"{_label(gui, a)}: `after` names {missing}, which have no static range in {gui}"))
            continue
        base = vanilla_next + sum(size[m] for m in a.get("after", []))
        out.append(Finding("INFO", f"{_label(gui, a)}: appends {a['count']} controls at runtime ids {base}..{base + a['count'] - 1} "
                                   f"(vanilla next free id {vanilla_next} + after {a.get('after', [])})"))
    return out


# ---- sources ------------------------------------------------------------------------------------------------------------

def find_mod_dirs(repo) -> dict:
    """{manifest [patch].id: dir} for patch_manager_mods/*/ and */*/ (empty if the tree is absent)."""
    out = {}
    root = Path(repo) / "patch_manager_mods"
    for m in sorted([*root.glob("*/manifest.toml"), *root.glob("*/*/manifest.toml")]):
        try:
            out.setdefault(tomllib.loads(m.read_text(encoding="utf-8"))["patch"]["id"], m.parent)
        except (OSError, KeyError, tomllib.TOMLDecodeError):
            continue
    return out


def check_source(gui, entry, mod_dirs) -> list[Finding]:
    """The mod must exist and, when source_regex is given, every pattern must match the cited source file."""
    pats = entry.get("source_regex", [])
    pats = [pats] if isinstance(pats, str) else pats
    d = mod_dirs.get(entry["mod"])
    if d is None:
        return [Finding("FAIL", f"{_label(gui, entry)}: no mod with manifest id {entry['mod']!r} under patch_manager_mods/")]
    src = entry.get("source", "").split(":")[0]
    f = d / src
    if not src or not f.is_file():
        hits = list(d.rglob(src)) if src else []
        f = hits[0] if hits else None
    if f is None:
        return [Finding("FAIL" if pats else "WARN", f"{_label(gui, entry)}: source file {src!r} not found in {d.name}/")]
    text = f.read_text(encoding="utf-8", errors="replace")
    out = []
    for p in pats:
        if not re.search(p, text, re.M):
            out.append(Finding("FAIL", f"{_label(gui, entry)}: source_regex {p!r} no longer matches {src} (constant changed? update the registry)"))
    if pats and not out:
        out.append(Finding("PASS", f"{_label(gui, entry)}: {len(pats)} source_regex pattern(s) match {src}"))
    return out


# ---- driver -------------------------------------------------------------------------------------------------------------

def run(registry_path, originals_dir, override_dir, repo, loader=load_controls) -> list[Finding]:
    """All findings for the registry. `loader(path) -> [Control]` is injectable for tests."""
    reg = tomllib.loads(Path(registry_path).read_text(encoding="utf-8"))
    originals, override = Path(originals_dir), Path(override_dir)
    mod_dirs = find_mod_dirs(repo)
    out = []
    if not mod_dirs:
        out.append(Finding("SKIP", f"no manifests under {Path(repo) / 'patch_manager_mods'}: source checks skipped"))
    for g in reg.get("gui", []):
        gui = g["file"]
        statics, appends = g.get("static", []), g.get("append", [])
        out += check_static_overlap(gui, statics)
        out += check_rect_collisions(gui, statics + appends)
        if mod_dirs:
            for e in statics + appends:
                out += check_source(gui, e, mod_dirs)
        vp = originals / gui
        if not vp.is_file():
            out.append(Finding("SKIP", f"{gui}: vanilla original {vp} absent: id checks skipped"))
            continue
        vanilla_max = max((c.id for c in loader(vp)), default=-1)
        out += check_vs_vanilla(gui, statics, vanilla_max)
        out += append_bases(gui, statics, appends, vanilla_max + 1)
        lp = override / gui
        if not statics:
            continue
        if not lp.is_file():
            out.append(Finding("SKIP", f"{gui}: live override {lp} absent: live check skipped"))
            continue
        out += check_live(gui, statics, loader(lp), vanilla_max)
    if not any(f.level == "FAIL" for f in out):
        out.append(Finding("PASS", f"registry {Path(registry_path).name}: no overlaps or collisions"))
    return out


def main(argv=None) -> int:
    try:
        import _common
        game = Path(_common.GAME_DIR)
    except ImportError:  # _common is optional
        game = Path(os.environ.get("KOTOR2_ROOT")
                    or "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II").expanduser()
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--registry", type=Path, default=DEFAULT_REGISTRY)
    ap.add_argument("--originals", type=Path, default=DEFAULT_ORIGINALS, help="vanilla .gui files (ultrawide_gui/originals)")
    ap.add_argument("--override", type=Path, default=game / "override", help="live override dir (read-only)")
    ap.add_argument("--quiet", action="store_true", help="hide PASS and INFO lines")
    a = ap.parse_args(argv)
    try:
        findings = run(a.registry, a.originals.expanduser(), a.override.expanduser(), REPO)
    except ImportError:
        print(f"FAIL {CHECK}: pykotor not importable: run with the repo .venv/bin/python")
        return 1
    for f in findings:
        if not (a.quiet and f.level in ("PASS", "INFO")):
            print(f"{f.level} {CHECK}: {f.msg}")
    return 1 if any(f.level == "FAIL" for f in findings) else 0


if __name__ == "__main__":
    sys.exit(main())
