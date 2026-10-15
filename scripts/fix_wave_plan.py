#!/usr/bin/env python3
"""fix_wave_plan.py - `plan` step of the variant fix wave: pack fix-unit rows into batches. Python 3, stdlib only.

Usage:
  fix_wave_plan.py [plan] <items.tsv> --wave W [--findings F --cols a,b,c]
                   [--max-units N] [--max-agents N] [--max-cost X] [--limits P]
                   [--waves-root D] [--repo-root D]

Input: items TSV, one row per fix unit (columns in COLUMNS). Rows sharing a non-free `group` form one block and are
never split. Free rows (group `-` or empty) are a block of one. Blocks and the rows inside them are ordered so every
`after` id sits in the same or an earlier batch and earlier in order.

Variants: bug S = [A]; bug M/L = [A,B,C]; gap S/M = [T2]; gap L = [T1,T2].
Caps: limits.toml at <repo>/.claude/skills/variant-fix-wave/limits.toml (or --limits P); built-in defaults when absent.
--max-* flags override the file. Per unit: agents = len(variants) + 2;
usd = (len(variants) * haiku + consolidator + reviewer) * revision_factor.

Writes <waves-root>/<W>/ items.tsv (copy of the input), plan.json and batches.tsv (wave_id = <W>-<letter>), and prints
a table on stdout. A block over a cap, an `after` cycle or an unknown `after` id exits 2 and writes nothing.
A leading `plan` token is accepted so `fix_wave.py plan ...` can forward argv unchanged.
Exit: 0 ok; 2 bad input or over-cap block.
"""
import argparse
import copy
import csv
import heapq
import json
import re
import shutil
import sys
import tomllib
from pathlib import Path

SCRIPTS_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPTS_DIR.parent
WAVES_REL = Path(".claude") / "waves"
LIMITS_REL = Path(".claude") / "skills" / "variant-fix-wave" / "limits.toml"
WAVE_ID_RE = r"[A-Za-z0-9][A-Za-z0-9._-]*"
COLUMNS = ("id", "kind", "size", "group", "after", "fids", "write", "tests", "mods",
           "entries", "anchors", "constraint", "context", "doc")
KINDS = ("bug", "gap")
SIZES = ("S", "M", "L")
VARIANT_TABLE = {
    ("bug", "S"): ("A",),
    ("bug", "M"): ("A", "B", "C"),
    ("bug", "L"): ("A", "B", "C"),
    ("gap", "S"): ("T2",),
    ("gap", "M"): ("T2",),
    ("gap", "L"): ("T1", "T2"),
}
OVERHEAD_AGENTS = 2  # consolidator + reviewer
LIMIT_KEYS = ("max_units", "max_agents", "max_cost_usd", "max_inflight")
DEFAULT_LIMITS = {
    "max_units": 12,
    "max_agents": 60,
    "max_cost_usd": 15.0,
    "max_inflight": 20,
    "rates": {"haiku": 0.045, "consolidator": 0.62, "reviewer": 0.34, "revision_factor": 1.3},
}
EPS = 1e-9


class PlanError(Exception):
    """Bad input, cycle, unknown `after` id or over-cap block. main() prints the message and exits 2."""


def lst(s):
    """Split a `;` list cell. Empty cells and the `-` placeholder give []."""
    if not s:
        return []
    return [x.strip() for x in s.split(";") if x.strip() not in ("", "-")]


def ent(p):
    """Repo-relative entry path. Bare paths get the patch_manager_mods/ prefix; scripts/ and patch_manager_mods/ paths stay."""
    p = p.strip()
    if p.startswith(("scripts/", "patch_manager_mods/")):
        return p
    return "patch_manager_mods/" + p


def variants(kind, size):
    """Variant ids for a unit. Empty size counts as M."""
    key = (kind, size or "M")
    if key not in VARIANT_TABLE:
        raise PlanError(f"no variants for kind={kind!r} size={size!r}")
    return list(VARIANT_TABLE[key])


def read_items(path):
    """Parse the items TSV into row dicts (stripped string values; empty size becomes M)."""
    path = Path(path)
    with path.open(newline="", encoding="utf-8") as fh:
        reader = csv.DictReader(fh, delimiter="\t", quoting=csv.QUOTE_NONE)
        header = reader.fieldnames or []
        for need in ("id", "kind"):
            if need not in header:
                raise PlanError(f"{path}: header lacks the {need!r} column")
        rows, seen = [], set()
        for raw in reader:
            if None in raw:
                raise PlanError(f"{path}: row {raw.get('id') or '?'} has more fields than the header")
            row = {k: (v or "").strip() for k, v in raw.items()}
            for col in COLUMNS:
                row.setdefault(col, "")
            if not any(row.values()):
                continue
            rid = row["id"]
            if not rid:
                raise PlanError(f"{path}: a row has an empty id")
            if rid in seen:
                raise PlanError(f"{path}: duplicate id {rid}")
            seen.add(rid)
            if row["kind"] not in KINDS:
                raise PlanError(f"row {rid}: kind {row['kind']!r} is not one of {'|'.join(KINDS)}")
            row["size"] = row["size"] or "M"
            if row["size"] not in SIZES:
                raise PlanError(f"row {rid}: size {row['size']!r} is not one of {'|'.join(SIZES)}")
            rows.append(row)
    if not rows:
        raise PlanError(f"{path}: no rows")
    return rows


def group_of(row):
    """Block key: ('group', letter) for grouped rows, ('free', id) for free rows."""
    g = row.get("group", "").strip()
    return ("free", row["id"]) if g in ("", "-") else ("group", g)


def _kahn(nodes, deps, rank):
    """Topological order; deps[n] must come before n, ties broken by rank. Returns (order, unordered nodes)."""
    indeg = {n: len(deps[n]) for n in nodes}
    unlocks = {n: [] for n in nodes}
    for n in nodes:
        for d in deps[n]:
            unlocks[d].append(n)
    heap = [(rank[n], n) for n in nodes if indeg[n] == 0]
    heapq.heapify(heap)
    order = []
    while heap:
        _, n = heapq.heappop(heap)
        order.append(n)
        for m in unlocks[n]:
            indeg[m] -= 1
            if indeg[m] == 0:
                heapq.heappush(heap, (rank[m], m))
    done = set(order)
    return order, [n for n in nodes if n not in done]


def _order_rows(block, pos):
    """Order the rows of one block so every `after` id inside the block comes first."""
    ids = [r["id"] for r in block]
    deps = {r["id"]: {a for a in lst(r.get("after", "")) if a in ids} for r in block}
    rank = {r["id"]: pos[r["id"]] for r in block}
    order, stuck = _kahn(ids, deps, rank)
    if stuck:
        raise PlanError("cycle in `after` among: " + ", ".join(sorted(stuck)))
    by_id = {r["id"]: r for r in block}
    return [by_id[i] for i in order]


def blocks(rows):
    """Group rows into blocks, topologically ordered by `after`. Raises PlanError on unknown ids or cycles."""
    by_id = {r["id"]: r for r in rows}
    pos = {r["id"]: i for i, r in enumerate(rows)}
    unknown = sorted({a for r in rows for a in lst(r.get("after", "")) if a not in by_id})
    if unknown:
        raise PlanError("unknown `after` id(s): " + ", ".join(unknown))
    key = {r["id"]: group_of(r) for r in rows}
    members = {}
    for r in rows:
        members.setdefault(key[r["id"]], []).append(r)
    first = {k: min(pos[r["id"]] for r in m) for k, m in members.items()}
    deps = {k: set() for k in members}
    for r in rows:
        for a in lst(r.get("after", "")):
            if key[a] != key[r["id"]]:
                deps[key[r["id"]]].add(key[a])
    order, stuck = _kahn(list(members), deps, first)
    if stuck:
        raise PlanError("cycle in `after` among: " + ", ".join(sorted(r["id"] for k in stuck for r in members[k])))
    return [_order_rows(members[k], pos) for k in order]


def unit_cost(row, limits):
    """(agents, usd) for one unit row."""
    vs = variants(row["kind"], row.get("size", ""))
    rates = limits["rates"]
    agents = len(vs) + OVERHEAD_AGENTS
    usd = (len(vs) * rates["haiku"] + rates["consolidator"] + rates["reviewer"]) * rates["revision_factor"]
    return agents, usd


def batch_totals(batch, limits):
    """(units, agents, usd) for a list of rows."""
    costs = [unit_cost(r, limits) for r in batch]
    return len(batch), sum(a for a, _ in costs), sum(u for _, u in costs)


def _is_int(v):
    return isinstance(v, int) and not isinstance(v, bool)


def _is_num(v):
    return isinstance(v, (int, float)) and not isinstance(v, bool)


def _check_limits(limits):
    for key in ("max_units", "max_agents", "max_inflight"):
        v = limits[key]
        if not _is_int(v) or v < 1:
            raise PlanError(f"{key} must be an integer >= 1, got {v!r}")
    v = limits["max_cost_usd"]
    if not _is_num(v) or v <= 0:
        raise PlanError(f"max_cost_usd must be a number > 0, got {v!r}")
    for key, v in limits["rates"].items():
        if not _is_num(v) or v < 0:
            raise PlanError(f"rates.{key} must be a number >= 0, got {v!r}")


def load_limits(path, overrides=None):
    """Built-in defaults, then the TOML file at path if it exists, then non-None overrides (max_units, ...)."""
    limits = copy.deepcopy(DEFAULT_LIMITS)
    if path is not None and Path(path).is_file():
        try:
            data = tomllib.loads(Path(path).read_text(encoding="utf-8"))
        except tomllib.TOMLDecodeError as exc:
            raise PlanError(f"{path}: {exc}") from exc
        for key, val in data.items():
            if key == "rates":
                if not isinstance(val, dict):
                    raise PlanError(f"{path}: [rates] must be a table")
                for rk, rv in val.items():
                    if rk not in limits["rates"]:
                        raise PlanError(f"{path}: unknown [rates] key {rk!r}")
                    limits["rates"][rk] = rv
            elif key in LIMIT_KEYS:
                limits[key] = val
            else:
                raise PlanError(f"{path}: unknown key {key!r}")
    for key, val in (overrides or {}).items():
        if val is None:
            continue
        if key not in LIMIT_KEYS:
            raise PlanError(f"unknown limit override {key!r}")
        limits[key] = val
    _check_limits(limits)
    return limits


def _label(block):
    head = block[0]
    return head["group"].strip() if group_of(head)[0] == "group" else head["id"]


def _breaches(units, agents, usd, limits):
    """Names of the caps exceeded, joined with '+'; empty string when all caps hold."""
    hit = []
    if units > limits["max_units"]:
        hit.append("units")
    if agents > limits["max_agents"]:
        hit.append("agents")
    if usd > limits["max_cost_usd"] + EPS:
        hit.append("cost")
    return "+".join(hit)


def pack_batches(blocks, limits):
    """Greedy in-order packing of blocks into batches; a block never splits. Returns a list of row lists."""
    batches, cur = [], []
    cur_units, cur_agents, cur_usd = 0, 0, 0.0
    for block in blocks:
        units, agents, usd = batch_totals(block, limits)
        why = _breaches(units, agents, usd, limits)
        if why:
            raise PlanError(f"BLOCK {_label(block)} over cap ({why}): split the group or raise the limit with --max-*")
        if cur and _breaches(cur_units + units, cur_agents + agents, cur_usd + usd, limits):
            batches.append(cur)
            cur, cur_units, cur_agents, cur_usd = [], 0, 0, 0.0
        cur.extend(block)
        cur_units += units
        cur_agents += agents
        cur_usd += usd
    if cur:
        batches.append(cur)
    return batches


def batch_letter(i):
    """0 -> A ... 25 -> Z, 26 -> AA."""
    s, n = "", i + 1
    while n:
        n, r = divmod(n - 1, 26)
        s = chr(65 + r) + s
    return s


def write_plan(waves_root, wave, items_path, rows, batches, limits, findings, cols):
    """Write <waves_root>/<wave>/{items.tsv, plan.json, batches.tsv}. Returns the plan dir."""
    if sorted(r["id"] for b in batches for r in b) != sorted(r["id"] for r in rows):
        raise PlanError("internal: batches do not cover every row exactly once")
    plan_dir = Path(waves_root) / wave
    plan_dir.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(items_path, plan_dir / "items.tsv")
    plan = {
        "items": str(Path(items_path).resolve()),
        "findings": str(Path(findings).resolve()) if findings else None,
        "cols": list(cols) if cols else None,
        "limits": limits,
    }
    (plan_dir / "plan.json").write_text(json.dumps(plan, indent=2) + "\n", encoding="utf-8")
    lines = ["batch\twave_id\tunits\tagents\ttest_usd\tids"]
    for i, b in enumerate(batches):
        letter = batch_letter(i)
        units, agents, usd = batch_totals(b, limits)
        ids = ";".join(r["id"] for r in b)
        lines.append(f"{letter}\t{wave}-{letter}\t{units}\t{agents}\t{usd:.2f}\t{ids}")
    (plan_dir / "batches.tsv").write_text("\n".join(lines) + "\n", encoding="utf-8")
    return plan_dir


def format_table(batches, limits):
    out = [f"{'batch':<6}{'units':>6}{'agents':>7}{'est_usd':>9}  ids"]
    tot_units = tot_agents = 0
    tot_usd = 0.0
    for i, b in enumerate(batches):
        units, agents, usd = batch_totals(b, limits)
        tot_units, tot_agents, tot_usd = tot_units + units, tot_agents + agents, tot_usd + usd
        out.append(f"{batch_letter(i):<6}{units:>6}{agents:>7}{usd:>9.2f}  {';'.join(r['id'] for r in b)}")
    out.append(f"total: {tot_units} units, {tot_agents} agents, ${tot_usd:.2f}, {len(batches)} batch(es)")
    return "\n".join(out)


def build_parser():
    p = argparse.ArgumentParser(prog="fix_wave_plan.py", description="Pack fix-unit rows into batches (plan step).")
    p.add_argument("items", help="items TSV, one row per fix unit")
    p.add_argument("--wave", required=True, help="wave id W; writes <waves-root>/W/")
    p.add_argument("--findings", help="findings TSV keyed by the first --cols column")
    p.add_argument("--cols", help="comma-separated findings columns, key column first")
    p.add_argument("--max-units", type=int, help="override max_units")
    p.add_argument("--max-agents", type=int, help="override max_agents")
    p.add_argument("--max-cost", type=float, help="override max_cost_usd")
    p.add_argument("--limits", help="limits.toml path (default: repo .claude/skills/variant-fix-wave/limits.toml)")
    p.add_argument("--waves-root", help="waves root (default: repo .claude/waves)")
    p.add_argument("--repo-root", help="repo root (default: parent of scripts/)")
    return p


def run_plan(args):
    if not re.fullmatch(WAVE_ID_RE, args.wave):
        raise PlanError(f"invalid wave id {args.wave!r}: use [A-Za-z0-9._-], starting with a letter or digit")
    repo_root = Path(args.repo_root).resolve() if args.repo_root else REPO_ROOT
    items = Path(args.items)
    if not items.is_file():
        raise PlanError(f"items file not found: {args.items}")
    if bool(args.findings) != bool(args.cols):
        raise PlanError("--findings and --cols must be given together")
    findings, cols = None, None
    if args.findings:
        findings = Path(args.findings)
        if not findings.is_file():
            raise PlanError(f"findings file not found: {args.findings}")
        cols = [c.strip() for c in args.cols.split(",") if c.strip()]
        if not cols:
            raise PlanError("--cols lists no column")
    limits_path = Path(args.limits) if args.limits else repo_root / LIMITS_REL
    if args.limits and not limits_path.is_file():
        raise PlanError(f"limits file not found: {args.limits}")
    limits = load_limits(limits_path, {"max_units": args.max_units, "max_agents": args.max_agents,
                                       "max_cost_usd": args.max_cost})
    rows = read_items(items)
    batches = pack_batches(blocks(rows), limits)
    waves_root = Path(args.waves_root).resolve() if args.waves_root else repo_root / WAVES_REL
    write_plan(waves_root, args.wave, items, rows, batches, limits, findings, cols)
    print(format_table(batches, limits))
    return 0


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    if argv[:1] == ["plan"]:
        argv = argv[1:]
    args = build_parser().parse_args(argv)
    try:
        return run_plan(args)
    except PlanError as exc:
        print(exc, file=sys.stderr)
        return 2
    except OSError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
