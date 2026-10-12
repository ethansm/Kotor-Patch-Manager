#!/usr/bin/env python3
"""Offline checks for the NPC-add PoC: the built 204tel.mod, its embedded UTCs, and the live install.

Built inputs: patch_manager_mods/npc_add_src/out/modules/204tel.mod (UTCs embedded) and the out/override/npcadd_*.utc
mirror. Baseline (the unmodified module) is, in order: --baseline; the live Modules/204tel.mod while it differs from the
build; the newest <game>/backups/npc_add_*/204tel.mod. Read-only: no game launch, no writes anywhere.

Checks (each prints PASS, FAIL or SKIP; SKIP is never a failure):
  1 GIT        built .mod read as raw GFF: Creature List 17+K, WaypointList 48+3R, other root lists equal to the
               baseline, all baseline resources kept (byte-equal except GIT), mirror .utc files equal the embedded UTCs.
  2 assets     model, texture, head, soundset row, scripts (.ncs) and conversation of each embedded npcadd UTC resolve in
               the mod's override, chitin (texture packs included) or the live Override. Live-only names print
               "live Override only" and pass.
  3 unique     no duplicate resource keys; new UTC tags, waypoint tags and creature templates are unique.
  4 vanilla    no new TemplateResRef exists in chitin.
  5 waypoints  every route has >= 3 waypoints numbered 1..N and each is >= 1.0 m (XY) from every GIT placeable.
  6 installed  live Modules/204tel.mod sha256 == built. Any other state prints SKIP "not installed".
  7 fields     Plot, Min1HP, NoPermDeath, IsPC == 0; FactionID == 5; Interruptable == 1.
  8 rows       no UTC Appearance_Type is in the IsGeneric* lists, APP_* 802-811 or the fixed collision list.

Exit 0 when no check FAILs, 1 otherwise.

  .venv/bin/python scripts/check_npc_add.py
  .venv/bin/python scripts/check_npc_add.py --out <dir> --game <dir> --baseline <204tel.mod>

Pure checks take parsed data (plain dicts, bytes, sets), so the tests need no pykotor and no game install. pykotor is
needed only to read the files; main() re-execs itself with the repo .venv when pykotor is not importable.
"""

from __future__ import annotations

import argparse
import hashlib
import logging
import math
import os
import re
import sys
from collections import Counter
from dataclasses import dataclass, field
from pathlib import Path
from typing import AbstractSet, Any, Dict, FrozenSet, List, Mapping, Optional, Sequence, Tuple

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import check_diversity_assets as cda  # noqa: E402  (parsers and resolvers are reused, not copied)

REPO = cda.REPO
ADD_SRC = REPO / "patch_manager_mods" / "npc_add_src"
DEFAULT_OUT = ADD_SRC / "out"
DEFAULT_GAME_DIR = cda.DEFAULT_GAME_DIR
MODULE = "204tel"
MOD_REL = Path("modules") / "204tel.mod"
BASE_CREATURES = 17
BASE_WAYPOINTS = 48
ORIGINAL_RESOURCES = 186
WAYPOINTS_PER_ROUTE = 3
MIN_ROUTE_WAYPOINTS = 3
PLACEABLE_KEEP_M = 1.0
NEW_PREFIX = "npcadd_"
GIT_KEY = (MODULE, "GIT")
GIT_LISTS = ("Creature List", "WaypointList")
ZERO_FIELDS = ("Plot", "Min1HP", "NoPermDeath", "IsPC")
FACTION_ID = 5
INTERRUPTABLE = 1
COLLISION_ROWS = frozenset({416, 449, 450, 606, 618, 805, 806, 280, 446, 620})
SNIPPET_B_ROWS = frozenset({416, 417, 445, 449, 450, 606, 618, 619, 280, 446, 447, 448, 620, 621, 622})
REMAP_OUTPUTS = frozenset(range(802, 812))
TEX_VARIANTS = tuple(f"{n:02d}" for n in range(1, 12))
WP_RE = re.compile(r"^WP_(npcadd_.+)_(\d{2})$", re.IGNORECASE)
TABLE_NAMES = tuple(cda.TABLES)

PASS, FAIL, SKIP = "PASS", "FAIL", "SKIP"
MAX_LINES = 12

Key = Tuple[str, str]  # (lowercase resref, resource type name such as "UTC")
Sources = Mapping[str, Optional[AbstractSet[str]]]


@dataclass
class Result:
    name: str
    status: str
    summary: str = ""
    lines: List[str] = field(default_factory=list)


@dataclass
class BuiltInputs:
    sha: str
    git: Mapping[str, Any]                       # plain root of the built GIT
    resources: Mapping[Key, bytes]               # every resource of the built .mod (last copy wins)
    res_keys: Sequence[Key]                      # every resource key in file order, duplicates included
    utcs: Mapping[str, Mapping[str, Any]]        # embedded npcadd UTCs (plain), by lowercase resref
    utc_tags: Mapping[str, str]                  # every UTC in the module: lowercase resref -> lowercase Tag
    mirror: Optional[Mapping[str, bytes]]        # out/override/npcadd_*.utc bytes by lowercase resref; None if absent


@dataclass
class BaselineInputs:
    sha: str
    git: Mapping[str, Any]
    resources: Mapping[Key, bytes]


@dataclass
class Context:
    built: Optional[BuiltInputs]
    live_sha: Optional[str]                      # None when <game>/Modules/204tel.mod is absent
    baseline: Optional[BaselineInputs]
    sources: Optional[Sources]                   # patch / live / chitin name sets for check 2 and 4
    appearance: Optional[Mapping[int, Mapping[str, str]]]
    heads: Optional[Mapping[int, Mapping[str, str]]]
    soundset_rows: Optional[int]
    tables: Optional[Mapping[str, List[int]]]
    app_consts: Optional[Mapping[str, int]]


# ---------------------------------------------------------------- pure helpers

def _int(value: Any) -> Optional[int]:
    try:
        return int(value)
    except (TypeError, ValueError):
        return None


def waypoint_points(waypoints: Sequence[Mapping[str, Any]]) -> Dict[str, List[Tuple[int, float, float]]]:
    """Group WP_npcadd_<slug>_NN waypoints by route key (npcadd_<slug>) as (index, x, y)."""
    routes: Dict[str, List[Tuple[int, float, float]]] = {}
    for w in waypoints:
        m = WP_RE.match(str(w.get("Tag", "")))
        if not m:
            continue
        routes.setdefault(m.group(1).lower(), []).append(
            (int(m.group(2)), float(w.get("XPosition", 0.0)), float(w.get("YPosition", 0.0))))
    return {k: sorted(v) for k, v in routes.items()}


def check_git(built: Mapping[str, Any], baseline: Optional[Mapping[str, Any]], new_utc_count: int,
              utc_resrefs: AbstractSet[str], built_res: Mapping[Key, bytes],
              base_res: Optional[Mapping[Key, bytes]]) -> Result:
    name = "1 GIT re-read"
    creatures, waypoints = built.get("Creature List"), built.get("WaypointList")
    if not isinstance(creatures, list) or not isinstance(waypoints, list):
        return Result(name, FAIL, "Creature List or WaypointList missing from the built GIT")
    routes = waypoint_points(waypoints)
    exp_c = BASE_CREATURES + new_utc_count
    exp_w = BASE_WAYPOINTS + WAYPOINTS_PER_ROUTE * len(routes)
    problems: List[str] = []
    if len(creatures) != exp_c:
        problems.append(f"Creature List has {len(creatures)} entries, expected 17+{new_utc_count}={exp_c}")
    if len(waypoints) != exp_w:
        problems.append(f"WaypointList has {len(waypoints)} entries, expected 48+3*{len(routes)}={exp_w}")
    for c in creatures[BASE_CREATURES:]:
        tpl = str(c.get("TemplateResRef", "")).lower()
        if tpl not in utc_resrefs:
            problems.append(f"new creature template {tpl!r} has no embedded UTC")
    info = [f"creatures {len(creatures)}=17+{new_utc_count}", f"waypoints {len(waypoints)}=48+3*{len(routes)}"]
    if baseline is None:
        info.append("baseline unavailable: list and resource comparison SKIPPED")
    else:
        base_c, base_w = baseline.get("Creature List", []), baseline.get("WaypointList", [])
        if len(base_c) != BASE_CREATURES or len(base_w) != BASE_WAYPOINTS:
            problems.append(f"baseline has {len(base_c)} creatures and {len(base_w)} waypoints, expected 17 and 48")
        if creatures[:len(base_c)] != base_c:
            problems.append("original Creature List entries changed")
        if waypoints[:len(base_w)] != base_w:
            problems.append("original WaypointList entries changed")
        for label in sorted((set(baseline) | set(built)) - set(GIT_LISTS)):
            if built.get(label) != baseline.get(label):
                problems.append(f"root field {label!r} differs from baseline")
        info.append("root lists bit-exact vs baseline")
    if base_res is not None:
        if len(base_res) != ORIGINAL_RESOURCES:
            problems.append(f"baseline has {len(base_res)} resources, expected {ORIGINAL_RESOURCES}")
        missing = [k for k in base_res if k not in built_res]
        changed = [k for k in base_res if k in built_res and k != GIT_KEY and built_res[k] != base_res[k]]
        problems += [f"original resource missing: {r}.{t}" for r, t in missing[:3]]
        problems += [f"original resource changed: {r}.{t}" for r, t in changed[:3]]
        info.append(f"{len(base_res) - len(missing)}/{len(base_res)} originals kept, {len(changed)} changed")
    return _finish(name, problems, info)


def check_mirror(embedded: Mapping[str, bytes], mirror: Optional[Mapping[str, bytes]]) -> List[str]:
    """out/override mirror must hold every embedded npcadd UTC byte for byte; extra mirror files (non-canary rows) are fine."""
    if mirror is None:
        return []
    problems = [f"mirror missing {r}.utc" for r in embedded if r not in mirror]
    problems += [f"mirror {r}.utc differs from embedded UTC" for r in embedded
                 if r in mirror and mirror[r] != embedded[r]]
    return problems


def check_unique(res_keys: Sequence[Key], utc_tags: Mapping[str, str], new_utcs: Sequence[str],
                 wp_tags: Sequence[str], creature_templates: Sequence[str]) -> Result:
    name = "3 unique tags/resrefs"
    problems: List[str] = []
    for key, n in Counter(res_keys).items():
        if n > 1:
            problems.append(f"duplicate resource {key[0]}.{key[1]} x{n}")
    tag_counts = Counter(t for t in utc_tags.values())
    for r in new_utcs:
        tag = utc_tags.get(r, "")
        if tag_counts[tag] > 1:
            problems.append(f"new UTC {r} tag {tag!r} shared by {tag_counts[tag]} UTCs")
    wp_counts = Counter(t.lower() for t in wp_tags if t.lower().startswith("wp_npcadd_"))
    problems += [f"waypoint tag {t!r} x{n}" for t, n in wp_counts.items() if n > 1]
    tpl_counts = Counter(t.lower() for t in creature_templates)
    problems += [f"creature template {t!r} used by {n} GIT entries" for t, n in tpl_counts.items() if n > 1]
    return _finish(name, problems, [f"{len(set(res_keys))} resource keys", f"{len(new_utcs)} new UTC tags"])


def check_vanilla_names(new_templates: Sequence[str], chitin: Optional[AbstractSet[str]]) -> Result:
    name = "4 no vanilla resref"
    if chitin is None:
        return Result(name, SKIP, "chitin unreadable; not checked")
    reused = sorted({t.lower() for t in new_templates if f"{t.lower()}.utc" in chitin})
    problems = [f"TemplateResRef {t} exists in chitin (vanilla reuse)" for t in reused]
    return _finish(name, problems, [f"{len(set(new_templates))} templates absent from chitin"])


def check_waypoints(routes: Mapping[str, Sequence[Tuple[int, float, float]]],
                    placeables: Sequence[Tuple[float, float]]) -> Result:
    name = "5 waypoints"
    problems: List[str] = []
    worst = math.inf
    for route, pts in sorted(routes.items()):
        if len(pts) < MIN_ROUTE_WAYPOINTS:
            problems.append(f"{route} has {len(pts)} waypoints, need >= {MIN_ROUTE_WAYPOINTS}")
        if [i for i, _, _ in pts] != list(range(1, len(pts) + 1)):
            problems.append(f"{route} waypoint indices not 1..{len(pts)}: {[i for i, _, _ in pts]}")
        for idx, x, y in pts:
            near = min((math.hypot(x - px, y - py) for px, py in placeables), default=math.inf)
            worst = min(worst, near)
            if near < PLACEABLE_KEEP_M:
                problems.append(f"{route} wp {idx:02d} is {near:.2f} m from a placeable (need >= {PLACEABLE_KEEP_M})")
    info = [f"{len(routes)} routes", f"{len(placeables)} placeables"]
    if worst is not math.inf:
        info.append(f"closest waypoint-placeable {worst:.2f} m")
    return _finish(name, problems, info)


def check_installed(built_sha: Optional[str], live_sha: Optional[str], baseline_sha: Optional[str]) -> Result:
    name = "6 installed == built"
    if built_sha is None:
        return Result(name, SKIP, "no built .mod yet")
    if live_sha is None:
        return Result(name, SKIP, "game Modules/204tel.mod absent")
    if live_sha == built_sha:
        return Result(name, PASS, f"installed: live sha256 {live_sha[:12]} == built")
    note = " (live is the unmodified baseline)" if baseline_sha is not None and live_sha == baseline_sha else ""
    return Result(name, SKIP, f"not installed: live {live_sha[:12]} != built {built_sha[:12]}{note}")


def check_forbidden(utcs: Mapping[str, Mapping[str, Any]]) -> Result:
    name = "7 forbidden UTC fields"
    problems: List[str] = []
    for r, u in sorted(utcs.items()):
        for f in ZERO_FIELDS:
            v = _int(u.get(f, 0))
            if v != 0:
                problems.append(f"{r}: {f}={u.get(f)!r}, must be 0")
        if _int(u.get("FactionID")) != FACTION_ID:
            problems.append(f"{r}: FactionID={u.get('FactionID')!r}, must be {FACTION_ID}")
        if _int(u.get("Interruptable")) != INTERRUPTABLE:
            problems.append(f"{r}: Interruptable={u.get('Interruptable')!r}, must be {INTERRUPTABLE}")
    return _finish(name, problems, [f"{len(utcs)} UTCs"])


def forbidden_rows(tables: Mapping[str, Sequence[int]], app_consts: Mapping[str, int]) -> Dict[int, str]:
    """Appearance rows a new UTC may not use, each with the reason it is forbidden."""
    out: Dict[int, str] = {}
    for fn in TABLE_NAMES:
        for n in tables.get(fn, []):
            out.setdefault(n, f"{fn} input list")
    for n in SNIPPET_B_ROWS:
        out.setdefault(n, "Snippet B collision list")
    for n in COLLISION_ROWS:
        out.setdefault(n, "diversity collision list")
    for name, n in app_consts.items():
        if n in REMAP_OUTPUTS:
            out.setdefault(n, f"{name} remap output")
    for n in REMAP_OUTPUTS:
        out.setdefault(n, "remap output 802-811")
    return out


def check_rows(utc_rows: Mapping[str, int], forbidden: Mapping[int, str]) -> Result:
    name = "8 collision rows"
    problems = [f"{r} uses appearance row {n}: {forbidden[n]}" for r, n in sorted(utc_rows.items()) if n in forbidden]
    return _finish(name, problems, [f"{len(utc_rows)} UTC rows checked against {len(forbidden)} forbidden rows"])


def utc_needs(utc: Mapping[str, Any], row: Mapping[str, str],
              heads: Mapping[int, Mapping[str, str]]) -> Tuple[List[Tuple[str, cda.Need]], List[str]]:
    """Every file one UTC needs, as (where, Need), plus structural problems. Model falls back to the race column when
    modela is blank (W1b rows 464, 72, 73, 63); texture base is texa, else racetex."""
    needs: List[Tuple[str, cda.Need]] = []
    problems: List[str] = []
    model = row.get("modela") or row.get("race", "")
    if model:
        needs.append(("model", cda.model_need(model)))
    else:
        problems.append("appearance row has neither modela nor race")
    base = row.get("texa") or row.get("racetex", "")
    if base:
        files = tuple(f"{base}{s}{ext}" for s in ("", *TEX_VARIANTS) for ext in (".tpc", ".tga"))
        needs.append(("texture", cda.Need(base, files, "any")))
    head_n = row.get("normalhead", "")
    if head_n:
        h = heads.get(int(head_n))
        if h is None:
            problems.append(f"heads row {head_n} missing from heads.2da")
        else:
            if h.get("head"):
                needs.append((f"head {head_n}", cda.model_need(h["head"])))
            if h.get("alttexture"):
                alt = h["alttexture"]
                needs.append((f"head {head_n} alttexture", cda.Need(alt, (f"{alt}.tpc", f"{alt}.tga"), "any")))
    for k, v in utc.items():
        if str(k).startswith("Script") and v:
            needs.append((k, cda.Need(str(v), (f"{v}.ncs",), "all")))
    conv = utc.get("Conversation")
    if conv:
        needs.append(("conversation", cda.Need(str(conv), (f"{conv}.dlg",), "all")))
    return needs, problems


def check_assets(utcs: Mapping[str, Mapping[str, Any]], appearance: Mapping[int, Mapping[str, str]],
                 heads: Mapping[int, Mapping[str, str]], soundset_rows: Optional[int], sources: Sources) -> Result:
    name = "2 assets"
    problems: List[str] = []
    unknown_lines: List[str] = []
    info: List[str] = []
    live_lines: List[str] = []
    n_ok = 0
    for r in sorted(utcs):
        u = utcs[r]
        row_n = _int(u.get("Appearance_Type"))
        row = appearance.get(row_n) if row_n is not None else None
        if row is None:
            problems.append(f"{r}: Appearance_Type {u.get('Appearance_Type')!r} not in appearance.2da")
            continue
        needs, struct_problems = utc_needs(u, row, heads)
        problems += [f"{r}: {p}" for p in struct_problems]
        ss = _int(u.get("SoundSetFile"))
        if soundset_rows is None:
            unknown_lines.append(f"{r}: soundset.2da unreadable; SoundSetFile {ss} unverified")
        elif ss is None or not 0 <= ss < soundset_rows:
            problems.append(f"{r}: SoundSetFile {u.get('SoundSetFile')!r} not a soundset.2da row")
        for where, need in needs:
            st = cda.need_status(need, sources)
            if st == cda.MISSING:
                problems.append(f"{r}: {where} {need.label} not found ({' | '.join(need.files[:3])})")
            elif st == cda.UNKNOWN:
                unknown_lines.append(f"{r}: {where} {need.label} unverified (source unavailable)")
            elif st == cda.LIVE:
                live_lines.append(f"{r}: {where} {need.label}: live Override only")
            else:
                n_ok += 1
    info.append(f"{len(utcs)} UTCs, {n_ok} refs OK, {len(live_lines)} live Override only")
    lines = [f"  {x}" for x in live_lines] + [f"  {x}" for x in unknown_lines]
    res = _finish(name, problems, info)
    res.lines = lines + res.lines
    if res.status == PASS and unknown_lines:
        res.status = SKIP
    return res


def _finish(name: str, problems: Sequence[str], info: Sequence[str]) -> Result:
    status = FAIL if problems else PASS
    lines = [f"  - {p}" for p in problems[:MAX_LINES]]
    if len(problems) > MAX_LINES:
        lines.append(f"  ... and {len(problems) - MAX_LINES} more")
    return Result(name, status, "; ".join(info), lines)


def run_all(ctx: Context) -> Tuple[int, List[str]]:
    results: List[Result] = []
    b = ctx.built
    if b is None:
        for name in ("1 GIT re-read", "2 assets", "3 unique tags/resrefs", "4 no vanilla resref", "5 waypoints",
                     "7 forbidden UTC fields", "8 collision rows"):
            results.append(Result(name, SKIP, "out/ not built yet: modules/204tel.mod missing"))
    else:
        embedded = {r: b.resources[(r, "UTC")] for r in b.utcs}
        base_res = ctx.baseline.resources if ctx.baseline else None
        base_git = ctx.baseline.git if ctx.baseline else None
        g1 = check_git(b.git, base_git, len(b.utcs), set(b.utcs), b.resources, base_res)
        mirror_problems = check_mirror(embedded, b.mirror)
        if b.mirror is None:
            g1.lines.append("  mirror out/override absent: byte comparison SKIPPED")
        elif mirror_problems:
            g1.status = FAIL
            g1.lines += [f"  - {p}" for p in mirror_problems[:MAX_LINES]]
        results.append(g1)
        if ctx.appearance is None or ctx.heads is None or ctx.sources is None:
            results.append(Result("2 assets", SKIP, "appearance, heads or name sources unreadable"))
        else:
            results.append(check_assets(b.utcs, ctx.appearance, ctx.heads, ctx.soundset_rows, ctx.sources))
        new_creatures = [str(c.get("TemplateResRef", "")) for c in b.git.get("Creature List", [])[BASE_CREATURES:]]
        wp_tags = [str(w.get("Tag", "")) for w in b.git.get("WaypointList", [])]
        results.append(check_unique(b.res_keys, b.utc_tags, sorted(b.utcs), wp_tags, new_creatures))
        chitin = ctx.sources.get("chitin") if ctx.sources else None
        results.append(check_vanilla_names(sorted(b.utcs), chitin))
        placeables = [(float(p.get("X", 0.0)), float(p.get("Y", 0.0))) for p in b.git.get("Placeable List", [])]
        results.append(check_waypoints(waypoint_points(b.git.get("WaypointList", [])), placeables))
        results.append(check_forbidden(b.utcs))
        if ctx.tables is None or ctx.app_consts is None:
            results.append(Result("8 collision rows", SKIP, "diversity sources unreadable"))
        else:
            utc_rows = {r: _int(u.get("Appearance_Type")) or -1 for r, u in b.utcs.items()}
            results.append(check_rows(utc_rows, forbidden_rows(ctx.tables, ctx.app_consts)))
    base_sha = ctx.baseline.sha if ctx.baseline else None
    results.append(check_installed(b.sha if b else None, ctx.live_sha, base_sha))
    results.sort(key=lambda r: r.name)

    out = []
    for r in results:
        head = f"{r.status:<5} {r.name}"
        if r.summary:
            head += f": {r.summary}"
        out.append(head)
        out += r.lines
    fails = sum(1 for r in results if r.status == FAIL)
    passes = sum(1 for r in results if r.status == PASS)
    skips = sum(1 for r in results if r.status == SKIP)
    out.append(f"RESULT: {'FAIL' if fails else 'PASS'} ({passes} pass, {fails} fail, {skips} skip)")
    return (1 if fails else 0), out


# ---------------------------------------------------------------- pykotor loaders (not used by tests)

def _pyk() -> Tuple[Any, Any, Any]:
    from pykotor.common.language import LocalizedString
    from pykotor.resource.formats.gff.gff_data import GFFList, GFFStruct
    return GFFStruct, GFFList, LocalizedString


def gff_plain(value: Any) -> Any:
    """Convert a pykotor GFF value to plain Python data. Structs keep their struct_id under '__struct_id'."""
    GFFStruct, GFFList, LocalizedString = _pyk()
    if isinstance(value, GFFStruct):
        out: Dict[str, Any] = {"__struct_id": int(value.struct_id)}
        for label, _ftype, field_value in value:
            out[str(label)] = gff_plain(field_value)
        return out
    if isinstance(value, GFFList):
        return [gff_plain(s) for s in value]
    if isinstance(value, LocalizedString):
        return {"stringref": int(getattr(value, "stringref", -1)),
                "text": sorted((int(lang), int(gen), str(t)) for lang, gen, t in value)}
    if value is None or isinstance(value, (bool, int, float, str, bytes)):
        return value
    if hasattr(value, "x") and hasattr(value, "y") and hasattr(value, "z"):
        return (value.x, value.y, value.z)
    return str(value)


def sha256_of(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read_module(path: Path) -> Tuple[List[Key], Dict[Key, bytes], Dict[str, Any], Dict[str, Dict[str, Any]], Dict[str, str]]:
    """Return (resource keys in file order, resources by key, plain GIT root, plain UTCs and UTC tags by resref).
    Needs pykotor."""
    from pykotor.resource.formats.erf import read_erf
    from pykotor.resource.formats.gff import read_gff

    logging.disable(logging.CRITICAL)
    erf = read_erf(str(path))
    keys: List[Key] = []
    resources: Dict[Key, bytes] = {}
    utcs: Dict[str, Dict[str, Any]] = {}
    tags: Dict[str, str] = {}
    git: Dict[str, Any] = {}
    for res in erf:
        key = (str(res.resref).lower(), str(res.restype))
        keys.append(key)
        resources[key] = bytes(res.data)
        if key[1] == "GIT":
            git = gff_plain(read_gff(bytes(res.data)).root)
        elif key[1] == "UTC":
            u = gff_plain(read_gff(bytes(res.data)).root)
            utcs[key[0]] = u
            tags[key[0]] = str(u.get("Tag", "")).lower()
    return keys, resources, git, utcs, tags


def load_built(mod: Path, mirror_dir: Path) -> BuiltInputs:
    keys, resources, git, utcs, tags = read_module(mod)
    embedded = {r: u for r, u in utcs.items() if r.startswith(NEW_PREFIX)}
    mirror: Optional[Dict[str, bytes]] = None
    if mirror_dir.is_dir():
        mirror = {p.stem.lower(): p.read_bytes() for p in mirror_dir.glob(f"{NEW_PREFIX}*.utc")}
    return BuiltInputs(sha256_of(mod), git, resources, keys, embedded, tags, mirror)


def load_baseline(path: Path) -> BaselineInputs:
    _keys, resources, git, _utcs, _tags = read_module(path)
    return BaselineInputs(sha256_of(path), git, resources)


def pick_baseline(explicit: Optional[Path], live: Path, built_sha: Optional[str], backups: Path) -> Optional[Path]:
    if explicit is not None:
        return explicit
    if live.is_file() and sha256_of(live) != built_sha:
        return live
    found = sorted(backups.glob("npc_add_*/204tel.mod")) if backups.is_dir() else []
    return found[-1] if found else None


def load_appearance_cols(path: Path) -> Dict[int, Dict[str, str]]:
    from pykotor.resource.formats.twoda import read_2da

    logging.disable(logging.CRITICAL)
    t = read_2da(str(path))
    cols = ("race", "racetex", "normalhead", "modela", "texa")
    out: Dict[int, Dict[str, str]] = {}
    for i in range(t.get_height()):
        row = t.get_row(i)
        out[i] = {c: cda._cell(row.get_string(c)) for c in cols}
    return out


def load_heads_all(path: Path) -> Dict[int, Dict[str, str]]:
    from pykotor.resource.formats.twoda import read_2da

    logging.disable(logging.CRITICAL)
    t = read_2da(str(path))
    out: Dict[int, Dict[str, str]] = {}
    for i in range(t.get_height()):
        row = t.get_row(i)
        out[i] = {"head": cda._cell(row.get_string("head")), "alttexture": cda._cell(row.get_string("alttexture"))}
    return out


def open_install(game_dir: Path) -> Any:
    if not (game_dir / "chitin.key").is_file():
        return None
    from pykotor.extract.installation import Installation

    logging.disable(logging.CRITICAL)
    try:
        return Installation(str(game_dir))
    except Exception as exc:  # pykotor raises several types on a damaged install
        print(f"WARN install unreadable under {game_dir}: {exc}", file=sys.stderr)
        return None


def install_names(inst: Any) -> Optional[FrozenSet[str]]:
    """Lowercase names of every chitin resource and every texture-pack resource (TPA/TPB/TPC/GUI)."""
    if inst is None:
        return None
    names = {r.filename().lower() for r in inst.chitin_resources()}
    for tp in inst.texturepacks_list():
        names |= {r.filename().lower() for r in inst.texturepack_resources(tp)}
    return frozenset(names)


def soundset_row_count(inst: Any) -> Optional[int]:
    if inst is None:
        return None
    from pykotor.extract.installation import SearchLocation
    from pykotor.resource.formats.twoda import read_2da
    from pykotor.resource.type import ResourceType

    res = inst.resource("soundset", ResourceType.TwoDA, [SearchLocation.CHITIN])
    return read_2da(res.data).get_height() if res is not None else None


def _ensure_pykotor() -> None:
    try:
        import pykotor  # noqa: F401
        return
    except ImportError:
        pass
    venv_py = REPO / ".venv" / "bin" / "python"
    if venv_py.exists() and not os.environ.get("_CNA_REEXEC"):
        os.environ["_CNA_REEXEC"] = "1"
        os.execv(str(venv_py), [str(venv_py), __file__] + sys.argv[1:])
    print("Error: pykotor must be importable; run with the repo venv: .venv/bin/python scripts/check_npc_add.py",
          file=sys.stderr)
    sys.exit(1)


def _rel(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO))
    except ValueError:
        return str(path)


def load_context(out: Path, game: Path, explicit_baseline: Optional[Path]) -> Context:
    mod = out / MOD_REL
    built_sha = sha256_of(mod) if mod.is_file() else None
    live = game / "Modules" / "204tel.mod"
    live_sha = sha256_of(live) if live.is_file() else None
    built = load_built(mod, out / "override") if built_sha else None
    base_path = pick_baseline(explicit_baseline, live, built_sha, game / "backups")
    baseline = load_baseline(base_path) if base_path else None

    inst = open_install(game)
    chitin = install_names(inst)
    live_names = cda.list_dir_names(game / "override")
    if live_names is None and (game / "chitin.key").is_file():
        live_names = frozenset()  # clean install: no loose Override files, so missing names must fail
    patch_names: Optional[FrozenSet[str]] = None
    dirs = [d for d in (cda.DEFAULT_OVERRIDE, out / "override") if d.is_dir()]
    if dirs:
        patch_names = frozenset().union(*[cda.list_dir_names(d) or frozenset() for d in dirs])
    sources: Optional[Sources] = {"patch": patch_names, "live": live_names, "chitin": chitin}
    if chitin is None and live_names is None:
        sources = None
    tables = cda.parse_tables(cda.DEFAULT_TABLES_HEADER.read_text(encoding="utf-8"))
    app_consts = cda.parse_app_constants(cda.DEFAULT_ROWS_HEADER.read_text(encoding="utf-8"))
    return Context(
        built=built,
        live_sha=live_sha,
        baseline=baseline,
        sources=sources,
        appearance=load_appearance_cols(cda.DEFAULT_APPEARANCE),
        heads=load_heads_all(cda.DEFAULT_HEADS),
        soundset_rows=soundset_row_count(inst),
        tables=tables,
        app_consts=app_consts,
    )


def main(argv: Optional[Sequence[str]] = None) -> int:
    ap = argparse.ArgumentParser(description="Offline checks for the NPC-add PoC build and its live install.")
    ap.add_argument("--out", type=Path, default=DEFAULT_OUT, help="build output dir (holds modules/204tel.mod)")
    ap.add_argument("--game", type=Path, default=DEFAULT_GAME_DIR, help="game dir (read only)")
    ap.add_argument("--baseline", type=Path, default=None,
                    help="unmodified 204tel.mod to compare against (default: live file or newest backup)")
    args = ap.parse_args(argv)
    _ensure_pykotor()
    ctx = load_context(args.out, args.game, args.baseline)
    code, lines = run_all(ctx)
    print(f"check_npc_add: out={_rel(args.out)} game={args.game}")
    print("\n".join(lines))
    return code


if __name__ == "__main__":
    sys.exit(main())
