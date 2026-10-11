#!/usr/bin/env python3
"""wave_tools.py - per-wave manifest, file briefs and token budget for subagent waves. Python 3, stdlib only.

Usage:
  wave_tools.py new <wave_id> --kind recon|worker [--repo-root DIR]
  wave_tools.py add <wave_id> --id uNN --task "<text>" --entries <entry>... [--scope GLOB...] [--repo-root DIR]
  wave_tools.py list <wave_id> [--repo-root DIR]
  wave_tools.py check <wave_id> [--json] [--repo-root DIR]
  wave_tools.py report <wave_id> [--usage-json FILE] [--repo-root DIR]

Layout, all under <root>/.claude/waves/<wave_id>/ (paths inside manifest.json are root-relative):
  baseline.tsv    agent_contract.py snapshot taken by `new`
  manifest.json   {wave, kind, created, baseline, units: [{id, description, entries, scope, out, reply, brief, est_tokens}]}
  uNN.brief.md    brief handed to the subagent
  uNN.md          recon output file (written by the subagent)
  uNN.reply.txt   5-line reply file (written by the subagent)

Repo root: --repo-root, else $WAVE_REPO_ROOT, else the parent of scripts/.
Entries are repo-relative `path` or `path:lo-hi`. Budgets come from wave_budget.py (WAVE_OVERHEAD_TOKENS honoured).
check: runs agent_contract.py check on each unit's reply. The worktree is shared, so a changed path outside every
  scope is WARN, not FAIL, unless the reply claims it or it is one of the unit's own entry files and no
  unit owns it. A missing or empty reply is MISSING. Exit 1 if any unit is FAIL or MISSING.
report: per unit, est_tokens vs the peak context of the matching agent_usage row (matched by description; a row
  with the 40-char '...' cut still matches). Peak > 60000 is FAIL (exit 1) and prints a suggested overhead.

Exit: 0 ok; 1 usage, state, or check/report failure; 2 entry file missing; 3 over token budget (nothing written).
Writes only under <root>/.claude/waves/. check and report write nothing there. Never modifies agent_contract.py,
but check runs it, and that appends a CONTRACT_CHECK event to master_ledger.jsonl.
"""
import argparse
import datetime as dt
import fnmatch
import json
import os
import posixpath
import re
import shutil
import subprocess
import sys
from pathlib import Path

SCRIPTS_DIR = Path(__file__).resolve().parent
if str(SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_DIR))
import wave_budget  # noqa: E402
import agent_contract as ac  # noqa: E402

AGENT_CONTRACT = SCRIPTS_DIR / "agent_contract.py"
AGENT_USAGE = SCRIPTS_DIR / "agent_usage.py"
WAVES_DIR = Path(".claude") / "waves"
KINDS = ("recon", "worker")
WAVE_ID_RE = r"[A-Za-z0-9][A-Za-z0-9._-]*"
UNIT_ID_RE = r"u[0-9]+"
RANGE_RE = r"(.*):(\d+)-(\d+)"
STOP_LINE = "If you have read more than the listed ranges or are unsure, stop and put it in OPEN."
REPORT_CAP_LINE = "Report cap: facts/out file <= 60 lines."
DESC_WORDS = 5


class WaveError(Exception):
    """Usage or state error. Message goes to stderr; `code` is the exit status."""

    def __init__(self, msg, code=1):
        super().__init__(msg)
        self.code = code


def resolve_root(arg):
    raw = arg or os.environ.get("WAVE_REPO_ROOT") or ""
    root = Path(raw).resolve() if raw else SCRIPTS_DIR.parent
    if not root.is_dir():
        raise WaveError(f"repo root is not a directory: {root}")
    return root


def check_wave_id(wave_id):
    if not re.fullmatch(WAVE_ID_RE, wave_id):
        raise WaveError(f"invalid wave id {wave_id!r}: use [A-Za-z0-9._-], starting with a letter or digit")


def wave_dir(root, wave_id):
    return root / WAVES_DIR / wave_id


def rel(root, path):
    return Path(path).relative_to(root).as_posix()


def write_json(path, obj):
    path.write_text(json.dumps(obj, indent=2) + "\n", encoding="utf-8")


def load_manifest(root, wave_id):
    d = wave_dir(root, wave_id)
    mpath = d / "manifest.json"
    if not mpath.is_file():
        raise WaveError(f"wave {wave_id} not found ({rel(root, d)}/manifest.json missing); create it with: new {wave_id} --kind recon|worker")
    try:
        manifest = json.loads(mpath.read_text(encoding="utf-8"))
    except (OSError, ValueError) as e:
        raise WaveError(f"cannot read {rel(root, mpath)}: {e}")
    return mpath, manifest


def last_line(text):
    lines = text.strip().splitlines()
    return lines[-1] if lines else "no output"


def contract_text(kind):
    r = subprocess.run([sys.executable, str(AGENT_CONTRACT), "print", kind], capture_output=True, text=True)
    if r.returncode != 0:
        raise WaveError(f"agent_contract.py print {kind} failed: {last_line(r.stderr or r.stdout)}")
    return r.stdout


def clean_glob(glob):
    g = glob.strip()
    if g.startswith("./"):
        g = g[2:]
    if not g or g.startswith("/") or ".." in g.split("/"):
        raise WaveError(f"scope glob must be repo-relative with no '..': {glob!r}")
    return g


def globs_overlap(a, b):
    """True when two scope globs can match the same path: either pattern matches the other, or one is a directory prefix of the other."""
    if fnmatch.fnmatch(a, b) or fnmatch.fnmatch(b, a):
        return True
    a, b = a.rstrip("/"), b.rstrip("/")
    return a == b or a.startswith(b + "/") or b.startswith(a + "/")


def resolve_entry(root, entry):
    """Repo-relative `path` or `path:lo-hi` -> absolute form for wave_budget. Missing file -> exit 2."""
    m = re.fullmatch(RANGE_RE, entry, re.DOTALL)
    path, suffix = (m.group(1), entry[len(m.group(1)):]) if m else (entry, "")
    p = Path(path)
    if not p.is_absolute():
        p = root / p
    if not p.is_file():
        raise WaveError(f"entry file not found: {entry}", code=2)
    return f"{p}{suffix}"


def entry_windows(root, resolved):
    """Split-suggestions for one over-cap entry: the file's suggest_windows clipped to the entry's line range."""
    m = re.fullmatch(RANGE_RE, resolved, re.DOTALL)
    path, lo, hi = (m.group(1), int(m.group(2)), int(m.group(3))) if m else (resolved, 1, sys.maxsize)
    shown = os.path.relpath(path, root)
    out = []
    for w in wave_budget.suggest_windows(path):
        wlo, whi = (int(x) for x in w.rsplit(":", 1)[1].split("-"))
        a, b = max(wlo, lo), min(whi, hi)
        if a <= b:
            out.append(f"{shown}:{a}-{b}")
    return out


def build_brief(kind, scope, entries, task, reply, contract):
    lines = [
        f"Contract: {kind}",
        f"Write Scope: {', '.join(scope)}",
        "Files (read only these ranges):",
    ]
    lines += [f"- {e}" for e in entries]
    lines += [
        "",
        f"Task: {task}",
        STOP_LINE,
        REPORT_CAP_LINE,
        f"Also write your 5-line reply to {reply}",
        "",
    ]
    return "\n".join(lines) + "\n" + contract


def report_over_budget(root, entries, resolved, res):
    print(f"OVER BUDGET total={res['total']} (cap {wave_budget.TOTAL_CAP}) entries={len(entries)} (max {wave_budget.MAX_ENTRIES})")
    if res["total"] > wave_budget.TOTAL_CAP:
        print("brief plus entries exceed the total cap; use fewer or smaller entries")
    if res["too_many"]:
        print(f"too many entries; split the unit so each has at most {wave_budget.MAX_ENTRIES}")
    for orig, r, n in zip(entries, resolved, res["per_entry"]):
        if r in res["over_cap"]:
            print(f"entry {orig} est={n} tokens > per-entry cap {wave_budget.PER_ENTRY_CAP}; split into:")
            for w in entry_windows(root, r):
                print(f"  {w}")
    print("nothing written")


def cmd_new(a, root):
    check_wave_id(a.wave_id)
    d = wave_dir(root, a.wave_id)
    try:
        d.mkdir(parents=True)
    except FileExistsError:
        raise WaveError(f"wave already exists: {rel(root, d)}")
    try:
        baseline = d / "baseline.tsv"
        r = subprocess.run(
            [sys.executable, str(AGENT_CONTRACT), "snapshot", "--out", str(baseline), "--repo-root", str(root)],
            capture_output=True, text=True, cwd=str(root))
        if r.returncode != 0:
            raise WaveError(f"agent_contract.py snapshot failed: {last_line(r.stderr or r.stdout)}")
        manifest = {
            "wave": a.wave_id,
            "kind": a.kind,
            "created": dt.datetime.now(dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
            "baseline": rel(root, baseline),
            "units": [],
        }
        write_json(d / "manifest.json", manifest)
    except Exception:
        shutil.rmtree(d, ignore_errors=True)
        raise
    print(f"created {rel(root, d)}/ kind={a.kind}")
    print(r.stdout.strip())
    return 0


def cmd_add(a, root):
    check_wave_id(a.wave_id)
    if not re.fullmatch(UNIT_ID_RE, a.id):
        raise WaveError(f"invalid unit id {a.id!r}: expected uNN, e.g. u01")
    task = a.task.strip()
    if not task:
        raise WaveError("--task must not be empty")
    mpath, manifest = load_manifest(root, a.wave_id)
    kind = manifest["kind"]
    units = manifest.setdefault("units", [])
    if any(u["id"] == a.id for u in units):
        raise WaveError(f"duplicate unit id {a.id} in wave {a.wave_id}")
    if kind == "worker" and not a.scope:
        raise WaveError("worker units need --scope GLOB [GLOB ...]")
    d = wave_dir(root, a.wave_id)
    out = rel(root, d / f"{a.id}.md")
    reply = rel(root, d / f"{a.id}.reply.txt")
    brief_path = d / f"{a.id}.brief.md"
    scope = [clean_glob(g) for g in a.scope] if a.scope else [out]
    if kind == "worker":
        for u in units:
            for other in u.get("scope", []):
                for mine in scope:
                    if globs_overlap(mine, other):
                        raise WaveError(f"scope {mine} overlaps unit {u['id']} scope {other}")
    resolved = [resolve_entry(root, e) for e in a.entries]
    text = build_brief(kind, scope, a.entries, task, reply, contract_text(kind))
    res = wave_budget.check(resolved, brief_bytes=len(text.encode("utf-8")))
    if not res["ok"]:
        report_over_budget(root, a.entries, resolved, res)
        return 3
    unit = {
        "id": a.id,
        "description": f"{a.wave_id}-{a.id} {' '.join(task.split()[:DESC_WORDS])}",
        "entries": list(a.entries),
        "scope": scope,
        "out": out,
        "reply": reply,
        "brief": rel(root, brief_path),
        "est_tokens": res["total"],
    }
    brief_path.write_text(text, encoding="utf-8")
    units.append(unit)
    write_json(mpath, manifest)
    print(f"added {a.id} est_tokens={unit['est_tokens']} brief={unit['brief']}")
    print(f"description: {unit['description']}")
    print(f"Follow the brief at {brief_path} exactly. Final message: the 5-line schema only.")
    print("model: haiku, subagent_type: general-purpose")
    return 0


def cmd_list(a, root):
    check_wave_id(a.wave_id)
    _, manifest = load_manifest(root, a.wave_id)
    units = manifest.get("units", [])
    if not units:
        print("(no units)")
    for u in units:
        print(f"{u['id']}\t{u['est_tokens']}\t{u['description']}")
    return 0


# ---------------------------------------------------------------- check / report
CONTRACT_PROBLEM_RE = re.compile(r"^(FAIL|WARN) (\S+) ?(.*)$")
CONTRACT_VERDICT_RE = re.compile(r"^VERDICT: (PASS|FAIL) ")
OUT_OF_SCOPE = "OUT_OF_SCOPE"
PEAK_FAIL_TOKENS = 60000
FINDING_WORDS = 25


def contract_check(root, kind, scope, baseline, reply):
    """Run agent_contract.py check on one reply. Returns (returncode, stdout, stderr)."""
    cmd = [sys.executable, str(AGENT_CONTRACT), "check", "--type", kind, "--scope", *scope,
           "--baseline", str(baseline), "--repo-root", str(root), str(reply)]
    r = subprocess.run(cmd, capture_output=True, text=True)
    return r.returncode, r.stdout, r.stderr


def parse_contract(stdout):
    """Problems [(level, code, detail)] from contract check text, and whether its VERDICT line was printed."""
    probs, saw_verdict = [], False
    for line in stdout.splitlines():
        m = CONTRACT_PROBLEM_RE.match(line)
        if m:
            probs.append((m.group(1), m.group(2), m.group(3)))
        elif CONTRACT_VERDICT_RE.match(line):
            saw_verdict = True
    return probs, saw_verdict


def entry_dir(entry):
    """Directory of a repo-relative `path` or `path:lo-hi` entry ('' for a root-level file)."""
    m = re.fullmatch(RANGE_RE, entry, re.DOTALL)
    return posixpath.dirname(posixpath.normpath(m.group(1) if m else entry))


def entry_path(entry):
    """File path of a repo-relative `path` or `path:lo-hi` entry."""
    m = re.fullmatch(RANGE_RE, entry, re.DOTALL)
    return posixpath.normpath(m.group(1) if m else entry)


def under_dir(path, directory):
    return directory == "" or path.startswith(directory + "/")


def out_of_scope_level(path, unit, claimed, all_scopes):
    """Level of a changed path outside the unit's scope.

    The worktree is shared with other sessions and sibling units, so an unclaimed path is FAIL only when it is one of
    this unit's own entry files (an agent that edits what it was reading) and no unit in the wave owns it. A path the reply claims is always FAIL.
    """
    if path in claimed:
        return "FAIL"
    owned = any(fnmatch.fnmatch(path, g) for g in all_scopes)
    if not owned and any(entry_path(e) == path for e in unit["entries"]):
        return "FAIL"
    return "WARN"


def finding_text(fails, warns):
    if fails:
        text = "; ".join(f"{code} {detail}" for _, code, detail in fails)
    elif warns:
        text = f"no FAIL; {len(warns)} WARN, first: {warns[0][1]} {warns[0][2]}"
    else:
        text = "clean"
    return " ".join(text.split()[:FINDING_WORDS])


def nonempty_file(path):
    return path.is_file() and path.stat().st_size > 0


def check_unit(root, kind, baseline, unit, all_scopes):
    reply = root / unit["reply"]
    text = reply.read_text(encoding="utf-8", errors="replace") if reply.is_file() else ""
    if not text.strip():
        return {"id": unit["id"], "status": "MISSING", "verdict": "-", "problems": [],
                "finding": f"reply missing or empty: {unit['reply']}"}
    _, out, err = contract_check(root, kind, unit["scope"], baseline, reply)
    probs, saw_verdict = parse_contract(out)
    _, fields, _ = ac.parse(text, kind)
    if not saw_verdict:
        probs = [("FAIL", "CONTRACT_ERROR", last_line(err or out))]
    else:
        claimed = set()
        for k in ("CREATED", "MODIFIED", "OUTPUT"):
            if k in fields:
                claimed.update(ac.split_paths(fields[k][0]))
        # .claude/waves/ is gitignored, so git cannot see the unit's own files: drop the git-based findings for paths
        # inside the wave dir and verify claimed ones by existence instead
        wave_dir = posixpath.dirname(unit["reply"]) + "/"
        probs = [p for p in probs if not (p[1] in ("NOT_DIRTY", OUT_OF_SCOPE) and wave_dir in p[2])]
        probs += [("FAIL", "WAVE_FILE_MISSING", f"{c} claimed but missing or empty")
                  for c in sorted(claimed) if c.startswith(wave_dir) and not nonempty_file(root / c)]
        recomputed = []
        for level, code, detail in probs:
            if code == OUT_OF_SCOPE:
                level = out_of_scope_level(detail.split(" outside ", 1)[0], unit, claimed, all_scopes)
            recomputed.append((level, code, detail))
        probs = recomputed
    fails = [p for p in probs if p[0] == "FAIL"]
    warns = [p for p in probs if p[0] == "WARN"]
    key = ac.SPECS[kind]["verdict_key"]
    words = fields[key][0].split() if key in fields else []
    return {"id": unit["id"], "status": "FAIL" if fails else "PASS",
            "verdict": (words[0].strip("`") if words else "") or "-",
            "finding": finding_text(fails, warns),
            "problems": [{"level": lvl, "code": code, "detail": detail} for lvl, code, detail in probs]}


def cmd_check(a, root):
    check_wave_id(a.wave_id)
    _, manifest = load_manifest(root, a.wave_id)
    units = manifest.get("units", [])
    baseline = root / manifest.get("baseline", "")
    if units and not baseline.is_file():
        raise WaveError(f"baseline missing for wave {a.wave_id} ({manifest.get('baseline')}); cannot classify changed paths")
    all_scopes = [g for u in units for g in u.get("scope", [])]
    results = [check_unit(root, manifest["kind"], baseline, u, all_scopes) for u in units]
    rerun = [r["id"] for r in results if r["status"] != "PASS"]
    if a.json:
        print(json.dumps({"wave": a.wave_id, "kind": manifest["kind"], "units": results, "rerun": rerun}, indent=2))
    else:
        if not units:
            print("(no units)")
        for r in results:
            print(f"{r['id']} {r['status']} {r['verdict']} {r['finding']}")
        print("RERUN: " + (" ".join(rerun) if rerun else "none"))
    return 1 if rerun else 0


def current_overhead():
    raw = os.environ.get("WAVE_OVERHEAD_TOKENS") or ""
    if not raw:
        return wave_budget.DEFAULT_OVERHEAD
    try:
        return int(raw)
    except ValueError:
        raise WaveError(f"WAVE_OVERHEAD_TOKENS must be an integer, got {raw!r}")


def load_usage_rows(usage_json):
    if usage_json:
        try:
            text = Path(usage_json).read_text(encoding="utf-8")
        except OSError as e:
            raise WaveError(f"cannot read --usage-json {usage_json}: {e}")
    else:
        r = subprocess.run([sys.executable, str(AGENT_USAGE), "--all", "--json"], capture_output=True, text=True)
        if r.returncode != 0:
            raise WaveError(f"agent_usage.py failed: {last_line(r.stderr or r.stdout)}")
        text = r.stdout
    try:
        data = json.loads(text)
    except ValueError as e:
        raise WaveError(f"usage JSON is not valid: {e}")
    rows = data.get("agents") if isinstance(data, dict) else None
    if not isinstance(rows, list):
        raise WaveError("usage JSON has no 'agents' list (expected agent_usage.py --json output)")
    return rows


def desc_matches(row_desc, unit_desc):
    """True when an agent_usage description belongs to this unit. agent_usage cuts long descriptions to 40 chars plus '...'."""
    row, want = " ".join(row_desc.split()), " ".join(unit_desc.split())
    if row == want:
        return True
    return len(row) > 3 and row.endswith("...") and want.startswith(row[:-3])


def cmd_report(a, root):
    check_wave_id(a.wave_id)
    _, manifest = load_manifest(root, a.wave_id)
    units = manifest.get("units", [])
    rows = load_usage_rows(a.usage_json)
    overhead = current_overhead()
    if not units:
        print("(no units)")
    flagged = no_data = 0
    for u in units:
        est = u["est_tokens"]
        peaks = [r["peak_ctx"] for r in rows
                 if isinstance(r.get("peak_ctx"), int) and isinstance(r.get("description"), str)
                 and desc_matches(r["description"], u["description"])]
        if not peaks:
            no_data += 1
            print(f"{u['id']} est={est} NO_DATA")
            continue
        peak = max(peaks)
        if peak > PEAK_FAIL_TOKENS:
            flagged += 1
            print(f"{u['id']} est={est} peak={peak} FAIL (> {PEAK_FAIL_TOKENS})")
            print(f"  suggest WAVE_OVERHEAD_TOKENS={max(0, peak - (est - overhead))}")
        else:
            print(f"{u['id']} est={est} peak={peak} OK")
    print(f"VERDICT: {'FAIL' if flagged else 'PASS'} units={len(units)} flagged={flagged} no_data={no_data}")
    return 1 if flagged else 0


def _common(p):
    p.add_argument("--repo-root", default=None, metavar="DIR",
                   help="repo root (default: $WAVE_REPO_ROOT, else the parent of scripts/)")


def build_parser():
    ap = argparse.ArgumentParser(prog="wave_tools.py", description="Subagent wave manifests, briefs and token budget.")
    sub = ap.add_subparsers(dest="cmd", required=True)

    p_new = sub.add_parser("new", help="create a wave: baseline snapshot and manifest")
    p_new.add_argument("wave_id")
    p_new.add_argument("--kind", required=True, choices=KINDS)
    _common(p_new)
    p_new.set_defaults(func=cmd_new)

    p_add = sub.add_parser("add", help="add a unit, check its token budget, write its brief")
    p_add.add_argument("wave_id")
    p_add.add_argument("--id", required=True, metavar="uNN")
    p_add.add_argument("--task", required=True, help="task text placed in the brief")
    p_add.add_argument("--entries", nargs="+", required=True, metavar="ENTRY", help="repo-relative path or path:lo-hi")
    p_add.add_argument("--scope", nargs="+", default=None, metavar="GLOB",
                       help="write globs (required for worker; recon defaults to its out path)")
    _common(p_add)
    p_add.set_defaults(func=cmd_add)

    p_list = sub.add_parser("list", help="list the units of a wave")
    p_list.add_argument("wave_id")
    _common(p_list)
    p_list.set_defaults(func=cmd_list)

    p_check = sub.add_parser("check", help="validate each unit's reply; list FAIL/MISSING units for rerun")
    p_check.add_argument("wave_id")
    p_check.add_argument("--json", action="store_true", help="machine-readable output")
    _common(p_check)
    p_check.set_defaults(func=cmd_check)

    p_report = sub.add_parser("report", help="estimated vs actual peak context per unit")
    p_report.add_argument("wave_id")
    p_report.add_argument("--usage-json", default=None, metavar="FILE",
                          help="agent_usage.py --all --json output to read instead of running it")
    _common(p_report)
    p_report.set_defaults(func=cmd_report)
    return ap


def main(argv=None):
    args = build_parser().parse_args(argv)
    try:
        root = resolve_root(args.repo_root)
        return args.func(args, root)
    except WaveError as e:
        print(f"error: {e}", file=sys.stderr)
        return e.code


if __name__ == "__main__":
    sys.exit(main())
