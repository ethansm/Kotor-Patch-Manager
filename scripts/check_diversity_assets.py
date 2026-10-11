#!/usr/bin/env python3
"""Verify the NPC diversity generic-row tables, APP_* rows, and the resources those rows reference.

Reads the four IsGeneric* case lists straight from DiversityTables.h (no second copy of the rows) and the APP_*
constants from DiversityRows.h, then checks them against the appearance.2da that the generator writes.

  tables     every `case N:` in IsGenericMaleCommoner / IsGenericFemaleCommoner / IsGenericTwilekMale / IsGenericTwilekFemale
             (a) row N exists in appearance.2da                                          -> FAIL
             (b) label starts with Commoner_Mal_ / Commoner_Fem_ (commoner tables) or
                 Twilek_Commoner_M_ / Twilek_Commoner_F_ (Twi'lek tables)                 -> WARN (listed with actual labels)
             (c) modela starts with N_CommM (male tables) or N_CommF (female tables)     -> FAIL
                 appearance.2da has no gender column; modela is the model-family gender marker (190 = N_CommM, 185 = N_CommF).
  rows       every APP_* constant in DiversityRows.h: row exists and modela is non-empty; rows 802-811 also need texa.
             Rows 812-815 are KNOWN-BROKEN (to be dropped by the generator): reported, and they fail the run unless
             --allow-known-broken is passed.
  resources  (default on, --no-resources skips) every model and texture named by appearance rows 802-811 (modela, modelb,
             texa, texb) and heads rows 192-196, 198, 200, 204 (head model, alttexture) must resolve in patch override/,
             the live Override (game dir /override) or chitin.
             - a model needs both .mdl and .mdx;
             - a texture needs .tpc or .tga; an N_CommM / N_CommF base needs every slot of DiversityLogic.h
               kCommonerSlots[] (N_CommMNN), any other appearance base needs variant 01 (engine fallback, D1 Method);
             - heads alttexture names are looked up exactly (no NN suffix).
             Found only in the live Override -> LIVE-ONLY (prerequisite): reported, does not fail.
             Found nowhere -> FAIL. When the live Override or chitin is unavailable, an unresolved name is a WARN.

Exit 0 when nothing fails, 1 otherwise. Warnings and LIVE-ONLY lines never fail the run.

  .venv/bin/python scripts/check_diversity_assets.py
  .venv/bin/python scripts/check_diversity_assets.py --allow-known-broken
  .venv/bin/python scripts/check_diversity_assets.py --no-resources
  .venv/bin/python scripts/check_diversity_assets.py --appearance /path/to/appearance.2da

Pure parsing and resolving (parse_tables, parse_app_constants, parse_commoner_slots, build_needs, check_resources,
run_checks) needs no pykotor; only load_appearance, load_heads and chitin_names do, and main() re-execs itself with the
repo .venv when pykotor is not importable.
"""

from __future__ import annotations

import argparse
import os
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import AbstractSet, Dict, FrozenSet, List, Mapping, Optional, Sequence, Tuple

REPO = Path(__file__).resolve().parent.parent
SRC = REPO / "patch_manager_mods" / "npc_diversity_src"
DEFAULT_APPEARANCE = SRC / "override" / "appearance.2da"
DEFAULT_TABLES_HEADER = SRC / "DiversityTables.h"
DEFAULT_ROWS_HEADER = SRC / "DiversityRows.h"
DEFAULT_LOGIC_HEADER = SRC / "DiversityLogic.h"
DEFAULT_HEADS = SRC / "override" / "heads.2da"
DEFAULT_OVERRIDE = SRC / "override"
DEFAULT_GAME_DIR = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
DEFAULT_LIVE_OVERRIDE = DEFAULT_GAME_DIR / "override"

COLUMNS = ("label", "modela", "modelb", "texa", "texb")
NULL_CELL = "****"

# IsGeneric* function -> (gender, kind)
TABLES: Dict[str, Tuple[str, str]] = {
    "IsGenericMaleCommoner": ("male", "commoner"),
    "IsGenericFemaleCommoner": ("female", "commoner"),
    "IsGenericTwilekMale": ("male", "twilek"),
    "IsGenericTwilekFemale": ("female", "twilek"),
}
LABEL_PREFIX: Dict[Tuple[str, str], str] = {
    ("male", "commoner"): "Commoner_Mal_",
    ("female", "commoner"): "Commoner_Fem_",
    ("male", "twilek"): "Twilek_Commoner_M_",
    ("female", "twilek"): "Twilek_Commoner_F_",
}
MODEL_PREFIX: Dict[str, str] = {"male": "N_CommM", "female": "N_CommF"}

APP_BOUND_CONSTS = frozenset({"APP_FIRST_APPENDED", "APP_COUNT"})
KNOWN_BROKEN_APP_ROWS = frozenset({812, 813, 814, 815})
TEXA_REQUIRED_APP_ROWS = frozenset(range(802, 812))
APP_RESOURCE_ROWS = frozenset(range(802, 812))
HEAD_ROWS = (192, 193, 194, 195, 196, 198, 200, 204)
COMMONER_BASES = frozenset({"n_commm", "n_commf"})
MAX_FAIL_LINES = 8

_FUNC_RE = re.compile(r"inline\s+bool\s+(\w+)\s*\(\s*uint16_t\s+app\s*\)\s*\{")
_CASE_RE = re.compile(r"\bcase\s+(\d+)\s*:")
_APP_CONST_RE = re.compile(r"constexpr\s+uint(?:8|16|32)_t\s+(APP_\w+)\s*=\s*(\d+)\s*;")
_SLOTS_RE = re.compile(r"kCommonerSlots\[\]\s*=\s*\{([^}]*)\}")

Rows = Dict[int, Dict[str, str]]
Sources = Mapping[str, Optional[AbstractSet[str]]]  # "patch", "live", "chitin"; None = source unavailable

OK, LIVE, UNKNOWN, MISSING = "OK", "LIVE", "UNKNOWN", "MISSING"
_RANK = {OK: 0, LIVE: 1, UNKNOWN: 2, MISSING: 3}


def _strip_line_comments(text: str) -> str:
    return "\n".join(line.split("//", 1)[0] for line in text.splitlines())


def parse_tables(header_text: str) -> Dict[str, List[int]]:
    """Return {function name: [case rows in source order]} for each `inline bool X(uint16_t app)` in the header.

    A function body runs from its signature to the next signature (or end of text). Comments are stripped first, so a
    `case N:` inside a comment is ignored.
    """
    text = _strip_line_comments(header_text)
    starts = list(_FUNC_RE.finditer(text))
    tables: Dict[str, List[int]] = {}
    for i, m in enumerate(starts):
        end = starts[i + 1].start() if i + 1 < len(starts) else len(text)
        tables[m.group(1)] = [int(n) for n in _CASE_RE.findall(text[m.end():end])]
    return tables


def parse_app_constants(rows_text: str) -> Dict[str, int]:
    """Return {APP_* name: row index} for each `constexpr uintN_t APP_X = N;` in DiversityRows.h.

    Table-bound constants (APP_FIRST_APPENDED, APP_COUNT) are edges of the table, not appended rows, so they are skipped.
    """
    return {m.group(1): int(m.group(2)) for m in _APP_CONST_RE.finditer(_strip_line_comments(rows_text))
            if m.group(1) not in APP_BOUND_CONSTS}


def parse_commoner_slots(logic_text: str) -> List[int]:
    """Return the slot numbers of `constexpr uint8_t kCommonerSlots[] = {...};` in DiversityLogic.h, in source order."""
    m = _SLOTS_RE.search(_strip_line_comments(logic_text))
    return [int(x) for x in re.findall(r"\d+", m.group(1))] if m else []


def _cell(value: Optional[str]) -> str:
    v = (value or "").strip()
    return "" if v == NULL_CELL else v


def load_appearance(path: Path) -> Rows:
    """Return {row index: {label, modela, modelb, texa, texb}} from a 2DA file. Needs pykotor."""
    import logging

    from pykotor.resource.formats.twoda import read_2da

    logging.disable(logging.CRITICAL)
    twoda = read_2da(str(path))
    rows: Rows = {}
    for i in range(twoda.get_height()):
        row = twoda.get_row(i)
        rows[i] = {col: _cell(row.get_string(col)) for col in COLUMNS}
    return rows


def load_heads(path: Path) -> Rows:
    """Return {heads row: {head, alttexture}} for HEAD_ROWS from heads.2da. Needs pykotor."""
    import logging

    from pykotor.resource.formats.twoda import read_2da

    logging.disable(logging.CRITICAL)
    twoda = read_2da(str(path))
    heads: Rows = {}
    for n in HEAD_ROWS:
        if n < twoda.get_height():
            row = twoda.get_row(n)
            heads[n] = {"head": _cell(row.get_string("head")), "alttexture": _cell(row.get_string("alttexture"))}
    return heads


def list_dir_names(path: Path) -> Optional[FrozenSet[str]]:
    """Lowercase file names directly inside `path`, or None when the directory is absent."""
    if not path.is_dir():
        return None
    return frozenset(p.name.lower() for p in path.iterdir() if p.is_file())


def chitin_names(game_dir: Path) -> Optional[FrozenSet[str]]:
    """Lowercase resource file names in chitin.key + BIFs (pykotor). None when the game dir cannot be read."""
    if not (game_dir / "chitin.key").is_file():
        return None
    import logging

    from pykotor.extract.installation import Installation

    logging.disable(logging.CRITICAL)
    try:
        inst = Installation(str(game_dir))
        return frozenset(r.filename().lower() for r in inst.chitin_resources())
    except Exception as exc:  # pykotor raises several exception types on a damaged install
        print(f"WARN chitin unreadable under {game_dir}: {exc}", file=sys.stderr)
        return None


@dataclass(frozen=True)
class Need:
    """One resource to resolve. mode "all": every file in `files` is required (model .mdl + .mdx).
    mode "any": one of `files` is enough (texture .tpc or .tga)."""

    label: str
    files: Tuple[str, ...]
    mode: str


def model_need(name: str) -> Need:
    return Need(name, (f"{name}.mdl", f"{name}.mdx"), "all")


def texture_need(name: str) -> Need:
    return Need(name, (f"{name}.tpc", f"{name}.tga"), "any")


def file_status(fname: str, sources: Sources) -> str:
    """Resolve one file name against patch, chitin, then live. Source sets hold lowercase names."""
    key = fname.lower()
    for src in ("patch", "chitin"):
        names = sources.get(src)
        if names is not None and key in names:
            return OK
    live = sources.get("live")
    if live is not None and key in live:
        return LIVE
    if any(v is None for v in sources.values()):
        return UNKNOWN
    return MISSING


def need_status(need: Need, sources: Sources) -> str:
    statuses = [file_status(f, sources) for f in need.files]
    pick = max if need.mode == "all" else min
    return pick(statuses, key=_RANK.__getitem__)


def build_needs(rows: Rows, heads: Rows, slots: Sequence[int]) -> List[Tuple[Need, List[str]]]:
    """Return [(Need, [where it is referenced])] for appearance rows 802-811 and the heads rows in HEAD_ROWS.

    Texture bases N_CommM / N_CommF expand to every slot; other appearance texture bases use variant 01. Duplicate
    file sets collapse into one Need with all of their references.
    """
    index: Dict[Tuple[str, Tuple[str, ...]], Tuple[Need, List[str]]] = {}

    def add(need: Need, where: str) -> None:
        key = (need.mode, tuple(f.lower() for f in need.files))
        refs = index.setdefault(key, (need, []))[1]
        if where not in refs:
            refs.append(where)

    for n in sorted(APP_RESOURCE_ROWS):
        row = rows.get(n)
        if row is None:
            continue
        for col in ("modela", "modelb"):
            if row.get(col):
                add(model_need(row[col]), f"app{n}.{col}")
        for col in ("texa", "texb"):
            base = row.get(col, "")
            if not base:
                continue
            where = f"app{n}.{col}"
            if base.lower() in COMMONER_BASES:
                for s in slots:
                    add(texture_need(f"{base}{s:02d}"), where)
            else:
                add(texture_need(f"{base}01"), where)

    for n in HEAD_ROWS:
        h = heads.get(n)
        if h is None:
            continue
        if h.get("head"):
            add(model_need(h["head"]), f"head{n}.head")
        if h.get("alttexture"):
            add(texture_need(h["alttexture"]), f"head{n}.alttexture")

    return list(index.values())


@dataclass
class ResourceInputs:
    rows: Rows
    heads: Rows
    slots: Sequence[int]
    sources: Mapping[str, Optional[AbstractSet[str]]]


@dataclass
class ResourceReport:
    checked: int = 0
    ok: int = 0
    live: List[str] = field(default_factory=list)
    unknown: List[str] = field(default_factory=list)
    fails: List[str] = field(default_factory=list)


def _refs(refs: List[str]) -> str:
    extra = len(refs) - 3
    return ", ".join(refs[:3]) + (f" +{extra} more" if extra > 0 else "")


def check_resources(rows: Rows, heads: Rows, slots: Sequence[int], sources: Sources) -> ResourceReport:
    rep = ResourceReport()
    for n in sorted(APP_RESOURCE_ROWS):
        if n not in rows:
            rep.fails.append(f"appearance row {n} missing from appearance.2da")
    for n in HEAD_ROWS:
        if n not in heads:
            rep.fails.append(f"heads row {n} missing from heads.2da")
    if not slots:
        rep.fails.append("no kCommonerSlots[] parsed from DiversityLogic.h")
    unavailable = ", ".join(sorted(k for k, v in sources.items() if v is None))
    for need, refs in build_needs(rows, heads, slots):
        rep.checked += 1
        st = need_status(need, sources)
        if st == OK:
            rep.ok += 1
        elif st == LIVE:
            rep.live.append(f"LIVE-ONLY (prerequisite) {need.label}: needed by {_refs(refs)}")
        elif st == UNKNOWN:
            rep.unknown.append(f"WARN {need.label} unresolved, source unavailable ({unavailable}): {_refs(refs)}")
        else:
            rep.fails.append(f"{need.label} not found in patch, live or chitin ({' | '.join(need.files)}): {_refs(refs)}")
    return rep


def run_checks(tables_text: str, rows_text: str, rows: Rows, allow_known_broken: bool = False,
               resources: Optional[ResourceInputs] = None) -> Tuple[int, List[str]]:
    """Run every check on in-memory inputs. Returns (exit code, output lines)."""
    fails: List[str] = []
    warns: List[str] = []
    table_lines = [f"{'TABLE':<26}{'ROWS':>5}{'LABEL-WARN':>12}{'FAIL':>6}"]
    parsed = parse_tables(tables_text)

    for name, (gender, kind) in TABLES.items():
        ids = parsed.get(name, [])
        if not ids:
            fails.append(f"{name}: no `case N:` rows parsed from DiversityTables.h")
            table_lines.append(f"{name:<26}{0:>5}{'-':>12}{1:>6}")
            continue
        label_prefix = LABEL_PREFIX[(gender, kind)]
        model_prefix = MODEL_PREFIX[gender]
        label_mismatch: List[str] = []
        n_fail = 0
        for n in ids:
            row = rows.get(n)
            if row is None:
                fails.append(f"{name}: row {n} missing from appearance.2da")
                n_fail += 1
                continue
            if not row["label"].startswith(label_prefix):
                label_mismatch.append(f"{n}={row['label']}")
            if not row["modela"].startswith(model_prefix):
                fails.append(f"{name}: row {n} ({row['label']}) modela={row['modela']!r}, expected {model_prefix}*")
                n_fail += 1
        table_lines.append(f"{name:<26}{len(ids):>5}{len(label_mismatch):>12}{n_fail:>6}")
        if label_mismatch:
            warns.append(f"WARN {name}: label not {label_prefix}*: " + ", ".join(label_mismatch))

    consts = parse_app_constants(rows_text)
    known: List[str] = []
    app_ok = 0
    app_fail = 0
    if not consts:
        fails.append("no APP_* constants parsed from DiversityRows.h")
    for name, n in sorted(consts.items(), key=lambda kv: (kv[1], kv[0])):
        if n in KNOWN_BROKEN_APP_ROWS:
            known.append(f"{name}={n}")
            continue
        row = rows.get(n)
        if row is None:
            fails.append(f"{name}: row {n} missing from appearance.2da")
            app_fail += 1
            continue
        problems = []
        if not row["modela"]:
            problems.append("empty modela")
        if n in TEXA_REQUIRED_APP_ROWS and not row["texa"]:
            problems.append("empty texa")
        if problems:
            fails.append(f"{name}: row {n} has {' and '.join(problems)}")
            app_fail += 1
        else:
            app_ok += 1

    out = list(table_lines)
    out.append(
        f"APP_* rows: {len(consts)} parsed, {app_ok} OK, {app_fail} FAIL, {len(known)} KNOWN-BROKEN"
    )
    prereq: List[str] = []
    if resources is not None:
        rep = check_resources(resources.rows, resources.heads, resources.slots, resources.sources)
        out.append(
            f"RESOURCES: {rep.checked} refs, {rep.ok} OK, {len(rep.live)} LIVE-ONLY (prerequisite), "
            f"{len(rep.unknown)} UNVERIFIED, {len(rep.fails)} FAIL"
        )
        fails.extend(rep.fails)
        warns.extend(rep.unknown[:MAX_FAIL_LINES])
        if len(rep.unknown) > MAX_FAIL_LINES:
            warns.append(f"... and {len(rep.unknown) - MAX_FAIL_LINES} more UNVERIFIED lines")
        prereq = rep.live[:MAX_FAIL_LINES]
        if len(rep.live) > MAX_FAIL_LINES:
            prereq.append(f"... and {len(rep.live) - MAX_FAIL_LINES} more LIVE-ONLY lines")
    if known:
        suffix = "  [accepted: --allow-known-broken]" if allow_known_broken else ""
        out.append("KNOWN-BROKEN (to be dropped by generator): " + ", ".join(known) + suffix)
    out.extend(warns)
    out.extend(prereq)
    for line in fails[:MAX_FAIL_LINES]:
        out.append("FAIL " + line)
    if len(fails) > MAX_FAIL_LINES:
        out.append(f"... and {len(fails) - MAX_FAIL_LINES} more FAIL lines")

    problems = len(fails) + (0 if allow_known_broken else len(known))
    out.append("RESULT: PASS" if problems == 0 else f"RESULT: FAIL ({problems} problem(s))")
    return (0 if problems == 0 else 1), out


def _ensure_pykotor() -> None:
    try:
        import pykotor  # noqa: F401
        return
    except ImportError:
        pass
    venv_py = REPO / ".venv" / "bin" / "python"
    if venv_py.exists() and not os.environ.get("_CDA_REEXEC"):
        os.environ["_CDA_REEXEC"] = "1"
        os.execv(str(venv_py), [str(venv_py), __file__] + sys.argv[1:])
    print("Error: pykotor must be importable; run with the repo venv: .venv/bin/python scripts/check_diversity_assets.py",
          file=sys.stderr)
    sys.exit(1)


def _rel(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO))
    except ValueError:
        return str(path)


def main(argv: Optional[Sequence[str]] = None) -> int:
    ap = argparse.ArgumentParser(description="Check NPC diversity tables, APP_* rows and the resources they reference.")
    ap.add_argument("--appearance", type=Path, default=DEFAULT_APPEARANCE, help="appearance.2da to check against")
    ap.add_argument("--tables-header", type=Path, default=DEFAULT_TABLES_HEADER, help="DiversityTables.h")
    ap.add_argument("--rows-header", type=Path, default=DEFAULT_ROWS_HEADER, help="DiversityRows.h")
    ap.add_argument("--allow-known-broken", action="store_true",
                    help="KNOWN-BROKEN APP rows (812-815) are reported but do not fail the run")
    ap.add_argument("--resources", action=argparse.BooleanOptionalAction, default=True,
                    help="check that every model/texture named by rows 802-811 and heads 192-196,198,200,204 "
                         "resolves (default on; --no-resources skips)")
    ap.add_argument("--logic-header", type=Path, default=DEFAULT_LOGIC_HEADER, help="DiversityLogic.h (kCommonerSlots)")
    ap.add_argument("--heads", type=Path, default=DEFAULT_HEADS, help="heads.2da to check against")
    ap.add_argument("--override", type=Path, default=DEFAULT_OVERRIDE, help="patch override/ directory")
    ap.add_argument("--live-override", type=Path, default=DEFAULT_LIVE_OVERRIDE,
                    help="live game Override directory (may be absent: unresolved names then only warn)")
    ap.add_argument("--game-dir", type=Path, default=DEFAULT_GAME_DIR, help="game directory with chitin.key")
    args = ap.parse_args(argv)

    required = [args.appearance, args.tables_header, args.rows_header]
    if args.resources:
        required += [args.heads, args.logic_header]
    for p in required:
        if not p.is_file():
            print(f"Error: not found: {p}", file=sys.stderr)
            return 1

    _ensure_pykotor()
    rows = load_appearance(args.appearance)
    resources: Optional[ResourceInputs] = None
    if args.resources:
        resources = ResourceInputs(
            rows=rows,
            heads=load_heads(args.heads),
            slots=parse_commoner_slots(args.logic_header.read_text(encoding="utf-8")),
            sources={
                "patch": list_dir_names(args.override),
                "live": list_dir_names(args.live_override),
                "chitin": chitin_names(args.game_dir),
            },
        )
    code, lines = run_checks(args.tables_header.read_text(encoding="utf-8"),
                             args.rows_header.read_text(encoding="utf-8"),
                             rows, args.allow_known_broken, resources)
    print(f"check_diversity_assets: {_rel(args.appearance)} ({len(rows)} rows)")
    print("\n".join(lines))
    return code


if __name__ == "__main__":
    sys.exit(main())
