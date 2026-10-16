#!/usr/bin/env python3
"""fix_wave_select.py - `select` and `draft-collect` steps of the variant fix wave. Python 3, stdlib only.

Usage:
  fix_wave_select.py select --wave W [--top N] [--dry-run] [--waves-root D] [--repo-root D] [--claims-dir D]
  fix_wave_select.py draft-collect --wave W [--waves-root D] [--repo-root D] [--claims-dir D]

select picks up to N (default 10, max 12) work items that are eligible, `ready` and have in-repo write files, in rank
order. Each kept item is claimed (wave_prefix VW-W). Items whose locksets overlap share a conflict group g1, g2, ...
(`-` for none). The items row is pre-filled from the WI file and its gate. candidates.tsv is written under
<waves-root>/W/ and the recon wave W-draft is created with one drafter unit per candidate (u01, u02, ...); the task text
comes from .claude/skills/variant-fix-wave/draft.md. --dry-run prints the candidates and nothing else.

draft-collect merges each drafter row <waves-root>/W-draft/<WI>.row.tsv into its candidate and lints it. Fit rows go to
<waves-root>/W/items.input.tsv (COLUMNS only); the rest go to <waves-root>/W/unfit.tsv as `id<TAB>reason`. Unfit,
lint-failed and row-less candidates release their claims. A fit row's optional `playtest` cell (live-game steps and
pass signal, `-` for none) goes to <waves-root>/W/playtests.tsv; the close step copies applied items' rows into
patch_manager_mods/PLAY_TEST_QUEUE.md. Next step: fix_wave.py plan <items.input.tsv> --wave W.

The wave_tools runner, the git dirty list and git HEAD are injectable, so tests need no git and no real wave_tools run.
Exit: 0 ok; 2 bad args, missing input, --top over the cap, or a wave_tools failure.
"""
import argparse
import csv
import os
import re
import shlex
import subprocess
import sys
from pathlib import Path

SCRIPTS_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPTS_DIR))

import agent_contract as ac  # noqa: E402
import fix_wave_plan as fwp  # noqa: E402
import wi_model as wm  # noqa: E402
import wi_state as ws  # noqa: E402

WAVE_TOOLS = SCRIPTS_DIR / "wave_tools.py"
DRAFT_TEMPLATE = Path(".claude") / "skills" / "variant-fix-wave" / "draft.md"
DEFAULT_TOP = 10
MAX_TOP = 12
KINDS = ("bug", "gap")
ROW_COLUMNS = ("id", "kind", "entries", "anchors", "tests", "constraint")
CANDIDATE_COLUMNS = fwp.COLUMNS + ("why",)
UNFIT_COLUMNS = ("id", "reason")
PLAYTEST_COLUMNS = ("id", "playtest")
SIZING_REL = Path(".claude") / "work_items" / "sizing.tsv"
SIZES = ("S", "M", "L", "XL")  # XL is never selected: split it first (SIZING_RUBRIC.md)
UNFIT_PREFIX = "UNFIT:"
ENTRY_RE = re.compile(r"([^:\s]+)(?::(\d+)-(\d+))?")


class SelectError(Exception):
    """Bad input or a failed wave_tools call. main() prints it and exits 2."""


def check_wave_id(wave):
    if not re.fullmatch(fwp.WAVE_ID_RE, wave or ""):
        raise SelectError(f"invalid wave id {wave!r}: use [A-Za-z0-9._-], starting with a letter or digit")


def session_id():
    return os.environ.get("WI_SESSION") or os.environ.get("CLAUDE_CODE_SESSION_ID") or f"ppid-{os.getppid()}"


def default_head(root):
    out = subprocess.run(["git", "rev-parse", "--short", "HEAD"], cwd=str(root), capture_output=True, text=True,
                         check=True)
    return out.stdout.strip()


def default_dirty(root):
    return ac.git_dirty(root)


def default_wave_runner(argv):
    """Run wave_tools.py with argv; returns (returncode, stdout, stderr)."""
    done = subprocess.run([sys.executable, str(WAVE_TOOLS), *argv], capture_output=True, text=True)
    return done.returncode, done.stdout, done.stderr


def load_template(root):
    path = Path(root) / DRAFT_TEMPLATE
    if not path.is_file():
        raise SelectError(f"drafter template missing: {path}")
    return path.read_text(encoding="utf-8")


def _clean(value):
    return re.sub(r"[\t\r\n]+", " ", str(value or "")).strip()


def write_tsv(path, columns, rows):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    lines = ["\t".join(columns)]
    lines += ["\t".join(_clean(row.get(c, "")) for c in columns) for row in rows]
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def read_tsv(path):
    with Path(path).open(newline="", encoding="utf-8") as fh:
        return [{k: (v or "") for k, v in row.items() if k is not None}
                for row in csv.DictReader(fh, delimiter="\t", quoting=csv.QUOTE_NONE)]


def rel(root, path):
    try:
        return Path(path).relative_to(root).as_posix()
    except ValueError:
        return str(path)


def _last(text):
    lines = [ln for ln in (text or "").strip().splitlines() if ln.strip()]
    return lines[-1] if lines else "no output"


def conflict_groups(cands, locks):
    """{wi_id: 'gN' or '-'}. Overlapping locksets join one group; groups are numbered in rank order of their first member."""
    parent = {w.id: w.id for w in cands}

    def find(x):
        while parent[x] != x:
            parent[x] = parent[parent[x]]
            x = parent[x]
        return x

    for i, a in enumerate(cands):
        for b in cands[i + 1:]:
            if wm.locksets_conflict(locks[a.id], locks[b.id]):
                parent[find(b.id)] = find(a.id)
    members = {}
    for w in cands:
        members[find(w.id)] = members.get(find(w.id), 0) + 1
    names, out = {}, {}
    for w in cands:
        root = find(w.id)
        if members[root] < 2:
            out[w.id] = "-"
            continue
        if root not in names:
            names[root] = f"g{len(names) + 1}"
        out[w.id] = names[root]
    return out


def gate_cells(wi, root):
    """(tests, mods) cells from the WI gate: run_tests.py stems and build_check.py --mod dirs."""
    tests, mods = [], []
    cmds, _why = wm.derive_gate(wi, root)
    for cmd in cmds:
        try:
            argv = shlex.split(cmd)
        except ValueError:
            continue
        for i, tok in enumerate(argv):
            if tok.endswith("run_tests.py"):
                tests += [a for a in argv[i + 1:] if not a.startswith("-")]
                break
        if "--mod" in argv and argv.index("--mod") + 1 < len(argv):
            mods.append(argv[argv.index("--mod") + 1])
    return ";".join(dict.fromkeys(tests)), ";".join(dict.fromkeys(mods))


def load_sizes(root):
    """{wi_id: S|M|L|XL} from .claude/work_items/sizing.tsv (effort rubric scores); {} when the file is absent."""
    path = Path(root) / SIZING_REL
    if not path.is_file():
        return {}
    return {r["id"]: r["size"] for r in read_tsv(path) if r.get("size") in SIZES}


def prefill_row(wi, group, after, root, size=None):
    tests, mods = gate_cells(wi, root)
    return {
        "id": wi.id, "kind": "", "size": size or wi.size or "M", "group": group, "after": after, "fids": "",
        "write": ";".join(wi.files), "tests": tests, "mods": mods, "entries": "", "anchors": "",
        "constraint": "", "context": f"{wm.WORK_DIR.as_posix()}/{wi.id}.md#Goal", "doc": "",
    }


def render_task(template, wi_id, row_path, prefill):
    """Fill {WI}, {ROW}, {PREFILL}. PREFILL goes in last so its text is never re-substituted."""
    cells = "\n".join(f"{c}={prefill.get(c, '')}" for c in fwp.COLUMNS)
    return template.replace("{WI}", wi_id).replace("{ROW}", row_path).replace("{PREFILL}", cells)


def create_draft(root, wr, draft, rows, template, runner):
    """Create the recon wave `draft` with one unit per candidate row (u01, u02, ...). Returns [(uid, wi_id)]."""
    units = [(f"u{i:02d}", row["id"]) for i, row in enumerate(rows, start=1)]
    write_tsv(Path(wr) / draft / "units.map.tsv", ("unit", "wi"), [{"unit": u, "wi": w} for u, w in units])
    rc, out, err = runner(["new", draft, "--kind", "recon", "--repo-root", str(root)])
    if rc != 0:
        raise SelectError(f"wave_tools new {draft} failed (exit {rc}): {_last(err or out)}")
    for (uid, wid), row in zip(units, rows):
        wi_file = f"{wm.WORK_DIR.as_posix()}/{wid}.md"
        row_path = rel(root, Path(wr) / draft / f"{wid}.row.tsv")
        task = render_task(template, wid, row_path, row)
        files = [f for f in row["write"].split(";") if f][:2]
        argv = ["add", draft, "--id", uid, f"--task={task}", "--entries", wi_file, *files,
                "--repo-root", str(root)]
        rc, out, err = runner(argv)
        if rc == 3:  # over the token budget: retry with the WI file alone
            argv = ["add", draft, "--id", uid, f"--task={task}", "--entries", wi_file, "--repo-root", str(root)]
            rc, out, err = runner(argv)
        if rc != 0:
            raise SelectError(f"wave_tools add {draft} {uid} ({wid}) failed (exit {rc}): {_last(err or out)}")
    return units


def select(wave, *, root, waves_root, top=DEFAULT_TOP, dry_run=False, claims_dir=None, session=None,
           wave_runner=None, dirty_fn=None, head_fn=None, template=None, ids=None):
    """Pick, claim and pre-fill the candidates, then create the W-draft recon wave. Returns the kept WI ids.
    `ids` names the candidates (in that order) instead of taking the top ready ones; XL (sizing.tsv) is skipped."""
    check_wave_id(wave)
    if not 1 <= top <= MAX_TOP:
        raise SelectError(f"--top must be between 1 and {MAX_TOP}, got {top}")
    root = Path(root).resolve()
    wr = Path(waves_root).resolve()
    draft = f"{wave}-draft"
    if not dry_run:
        if (wr / wave).exists():
            raise SelectError(f"{wr / wave} already exists: wave {wave} was selected before")
        if (root / fwp.WAVES_REL / draft).exists():
            raise SelectError(f"draft wave already exists: {root / fwp.WAVES_REL / draft}")
        template = load_template(root) if template is None else template
    runner = wave_runner or default_wave_runner
    dirty_fn = dirty_fn or default_dirty
    session = session or session_id()
    st = ws.Store(root, claims_dir=claims_dir)
    claimed = []
    with st.flock(".select.lock"):
        items = wm.load_all(root)
        done, parked = st.state_maps()
        live = {w: c.get("locks", []) for w, c in st.all_claims().items()}
        dirty = set(dirty_fn(root))
        sizes = load_sizes(root)
        unknown = [w for w in (ids or []) if w not in items]
        if unknown:
            raise SelectError(f"unknown WI id(s): {', '.join(unknown)}")
        pool = [items[w] for w in ids] if ids else list(items.values())
        eligible = []
        for wi in pool:
            if len(eligible) >= top:
                break
            if wi.state != "ready" or not wi.files or wi.out_of_repo or sizes.get(wi.id) == "XL":
                if ids:
                    print(f"skipped {wi.id}: state {wi.state}, size {sizes.get(wi.id, wi.size)}, "
                          f"out-of-repo {bool(wi.out_of_repo)}")
                continue
            verdict = wm.eligibility(wi, items, done, live, dirty, parked)
            if verdict.ok:
                eligible.append(wi)
            elif ids:
                print(f"skipped {wi.id}: {verdict.reason[:100]}")
        if not eligible:
            print("no eligible candidates")
            return []
        head = None if dry_run else (head_fn or default_head)(root)
        kept = []
        try:
            for wi in eligible:
                if not dry_run:
                    claim = st.create_claim(wi.id, session, wm.lockset(wi), head, wave_prefix=f"VW-{wave}")
                    if claim is None:
                        print(f"dropped {wi.id}: already claimed")
                        continue
                    claimed.append(wi.id)
                kept.append(wi)
            if not kept:
                print("no candidates left after claims")
                return []
            ids = {w.id for w in kept}
            locks = {w.id: wm.lockset(w) for w in kept}
            groups = conflict_groups(kept, locks)
            rows = []
            for wi in kept:
                after = ";".join(a for a in wi.after if a in ids)
                rows.append(prefill_row(wi, groups[wi.id], after, root, sizes.get(wi.id)))
            for row in rows:
                print(f"{row['id']} {items[row['id']].prio} {row['size']} {row['group']} {row['write']}")
            if dry_run:
                print(f"dry-run: {len(rows)} candidate(s); nothing claimed or written")
                return [w.id for w in kept]
            write_tsv(wr / wave / "candidates.tsv", CANDIDATE_COLUMNS, [dict(r, why="ready") for r in rows])
            create_draft(root, wr, draft, rows, template, runner)
            print(f"claimed {len(rows)}; candidates: {rel(root, wr / wave / 'candidates.tsv')}; "
                  f"draft wave: {draft} ({len(rows)} unit(s))")
            return [r["id"] for r in rows]
        except BaseException:
            for wid in claimed:
                st.release(wid)
            raise


def read_drafter_row(path):
    """(cells, reason). cells is the header->value dict of the drafter's one data row, or None with the reason text.
    An `UNFIT: <why>` file gives the reason; an empty file means no row; a malformed file is a lint failure."""
    lines = [ln for ln in Path(path).read_text(encoding="utf-8").splitlines() if ln.strip()]
    if not lines:
        return None, "no row"
    first = lines[0].strip()
    if first.startswith(UNFIT_PREFIX):
        return None, first[len(UNFIT_PREFIX):].strip() or "UNFIT (no reason given)"
    if len(lines) != 2:
        return None, f"lint: row file needs a header and one data row, has {len(lines)} line(s)"
    header = [h.strip() for h in first.split("\t")]
    cells = lines[1].split("\t")
    if len(cells) != len(header):
        return None, f"lint: {len(cells)} cells for {len(header)} header columns"
    return {h: c.strip() for h, c in zip(header, cells)}, None


def merge_row(cand, cells):
    """Candidate COLUMNS with the drafter's non-empty cells on top. The id always stays the candidate's."""
    merged = {c: cand.get(c, "") for c in fwp.COLUMNS}
    for col in ROW_COLUMNS:
        if col != "id" and cells.get(col, "").strip():
            merged[col] = cells[col].strip()
    return merged


def lint_row(merged, drafter_id):
    problems = []
    if drafter_id and drafter_id != merged["id"]:
        problems.append(f"row id {drafter_id} is not {merged['id']}")
    if merged["kind"] not in KINDS:
        problems.append(f"kind {merged['kind']!r} is not bug|gap")
    if not merged["constraint"].strip():
        problems.append("constraint empty")
    entries = [e.strip() for e in merged["entries"].split(";") if e.strip()]
    if not entries:
        problems.append("entries empty")
    for entry in entries:
        m = ENTRY_RE.fullmatch(entry)
        if not m:
            problems.append(f"entry {entry!r} is not path or path:lo-hi")
        elif m.group(2) and int(m.group(2)) > int(m.group(3)):
            problems.append(f"entry {entry!r} has lo > hi")
    return problems


def release_own(st, wi_id, wave):
    """Release the claim only when this wave made it (wave_prefix VW-W). Returns True when released."""
    claim = st.read_claim(wi_id)
    if claim and claim.get("wave_prefix") == f"VW-{wave}":
        st.release(wi_id)
        return True
    return False


def draft_collect(wave, *, root, waves_root, claims_dir=None):
    """Merge drafter rows into items.input.tsv (fit) and unfit.tsv. Returns the fit WI ids."""
    check_wave_id(wave)
    root = Path(root).resolve()
    wr = Path(waves_root).resolve()
    wave_dir = wr / wave
    cand_path = wave_dir / "candidates.tsv"
    if not cand_path.is_file():
        raise SelectError(f"candidates not found: {cand_path} (run select first)")
    cands = read_tsv(cand_path)
    if not cands:
        raise SelectError(f"{cand_path} has no candidate rows")
    draft_dir = wr / f"{wave}-draft"
    st = ws.Store(root, claims_dir=claims_dir)
    fit, unfit, released, playtests = [], [], [], []
    with st.flock(".select.lock"):
        for cand in cands:
            wid = cand["id"]
            row_file = draft_dir / f"{wid}.row.tsv"
            cells, reason = read_drafter_row(row_file) if row_file.is_file() else (None, "no row")
            if reason is None:
                merged = merge_row(cand, cells)
                problems = lint_row(merged, cells.get("id", ""))
                if not problems:
                    fit.append(merged)
                    live = cells.get("playtest", "").strip()
                    if live and live != "-":
                        playtests.append({"id": wid, "playtest": live})
                    continue
                reason = "lint: " + "; ".join(problems)
            unfit.append((wid, reason))
            if release_own(st, wid, wave):
                released.append(wid)
    items_path = wave_dir / "items.input.tsv"
    write_tsv(items_path, fwp.COLUMNS, fit)
    write_tsv(wave_dir / "unfit.tsv", UNFIT_COLUMNS, [{"id": w, "reason": r} for w, r in unfit])
    write_tsv(wave_dir / "playtests.tsv", PLAYTEST_COLUMNS, playtests)
    print(f"fit={len(fit)} unfit={len(unfit)} playtests={len(playtests)}")
    if released:
        print(f"released: {';'.join(released)}")
    print(f"plan input: {items_path}")
    print(f"next: python3 scripts/fix_wave.py plan {items_path} --wave {wave}")
    return [r["id"] for r in fit]


def _common(p):
    p.add_argument("--waves-root", help="default: <repo_root>/.claude/waves")
    p.add_argument("--repo-root", help="default: parent of scripts/")
    p.add_argument("--claims-dir", help="claim store (tests); default: git common dir")


def build_parser():
    ap = argparse.ArgumentParser(prog="fix_wave_select.py",
                                 description="Select fix-wave candidates and collect the drafter rows.")
    sub = ap.add_subparsers(dest="cmd", required=True)
    sel = sub.add_parser("select", help="claim up to --top ready WIs and create the W-draft recon wave")
    sel.add_argument("--wave", required=True, metavar="W")
    sel.add_argument("--top", type=int, default=DEFAULT_TOP, help=f"candidates to take, 1..{MAX_TOP} (default {DEFAULT_TOP})")
    sel.add_argument("--dry-run", action="store_true", help="print the candidates; claim and write nothing")
    sel.add_argument("--ids", help="comma-separated WI ids to take instead of the top ready ones")
    _common(sel)
    dc = sub.add_parser("draft-collect", help="merge drafter rows into items.input.tsv and unfit.tsv")
    dc.add_argument("--wave", required=True, metavar="W")
    _common(dc)
    return ap


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    args = build_parser().parse_args(argv)
    repo_root = Path(args.repo_root).resolve() if args.repo_root else SCRIPTS_DIR.parent
    waves_root = Path(args.waves_root).resolve() if args.waves_root else repo_root / fwp.WAVES_REL
    try:
        if args.cmd == "select":
            ids = [w.strip() for w in args.ids.split(",") if w.strip()] if args.ids else None
            select(args.wave, root=repo_root, waves_root=waves_root, top=args.top, dry_run=args.dry_run,
                   claims_dir=args.claims_dir, ids=ids)
        else:
            draft_collect(args.wave, root=repo_root, waves_root=waves_root, claims_dir=args.claims_dir)
    except (SelectError, OSError, ValueError, subprocess.SubprocessError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
