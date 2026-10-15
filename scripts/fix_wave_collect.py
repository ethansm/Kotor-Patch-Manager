#!/usr/bin/env python3
"""fix_wave_collect.py - status and cost for one variant-fix batch. Python 3, stdlib only (tomllib for limits).

Usage:
  fix_wave_collect.py status <W-A> [--mark <id> cons|rev] [--revise <id>] [--waves-root DIR] [--repo-root DIR] [--limits PATH]
  fix_wave_collect.py cost <W-A> [--session S ...] [--waves-root DIR] [--repo-root DIR]

Batch dir: <waves_root>/<W-A>/ holds items.tsv (falls back to units.tsv labels when missing), units.tsv,
uNN.reply.txt, state.tsv (id, stage, commit), manifest.json (created), and per item <id>/ with consolidate.md,
review.brief.md, final.diff, verdict.md, review.md, rev.reply.txt, cons.launched, rev.launched, review.rN.md.
waves_root defaults to <repo_root>/.claude/waves; repo_root defaults to the parent of scripts/.

status prints `inflight=<n> free=<max_inflight-n>` then one line per item: `<id> <stage> <next action>`.
  inflight = missing Haiku replies (0-byte counts as missing) + items with cons.launched and no final.diff
  + items with rev.launched and no review.md. max_inflight comes from limits.toml (default 20).
  Copy-out first: a missing <id>/review.md is copied from the newest <repo_root>/.claude/worktrees/agent-*/
  .claude/waves/<W-A>/<id>/review.md, else from the OUTPUT path in <id>/rev.reply.txt. A candidate whose bytes equal
  an archived review.rN.md is skipped (stale copy), and when rev.launched exists only candidates written at or
  after it count.
  --revise <id>: archive review.md -> review.rN.md, final.diff -> final.rN.diff, verdict.md -> verdict.rN.md and
  delete cons.launched / rev.launched (N = 1 + count of review.r*.md). Runs before --mark.
  --mark <id> cons|rev: touch <id>/cons.launched or <id>/rev.launched.
  Stages (first match): applied (state.tsv commit set), haiku-pending, cons-ready, rev-ready, revise (DECISION
  REQUEST_CHANGES or BLOCKING > 0 with fewer than 2 rounds), park (same with 2 rounds), apply-ready (APPROVE or
  APPROVE_MINOR, BLOCKING 0).
  review.md: DECISION and BLOCKING are read from the first 10 lines; a missing BLOCKING counts as 0; a BLOCKING
  value with no digits counts as 1 (fail safe).

cost sums the agent_usage.py --json cost_usd by role (description prefix `VW haiku`, `VW cons`, `VW rev`, else
`other`) and by model. A null cost counts as 0 and is flagged. agent_usage.py takes one --session, so it runs once
per --session given, or once for the current session when none is given. Covers the current session only unless
--session is repeated. --since is the batch manifest.json `created` time.

Exit: 0 ok; 2 bad args, missing batch, unknown item, bad limits file, or unreadable agent_usage output.
Writes only under the batch dir (copy-out, markers, revise archives).
"""
import argparse
import csv
import json
import os
import re
import shutil
import subprocess
import sys
import tomllib
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parent
DEFAULT_REPO = SCRIPTS.parent
DEFAULT_MAX_INFLIGHT = 20
DECISIONS = ("APPROVE", "APPROVE_MINOR", "REQUEST_CHANGES")
MARKS = ("cons", "rev")
ROLE_NAMES = {"haiku": "VW haiku", "cons": "VW cons", "rev": "VW rev"}
ROLE_RE = re.compile(r"^VW (haiku|cons|rev)\b")
LABEL_SUFFIX_RE = re.compile(r"-(A|B|C|T1|T2)$")
ROUND_RE = re.compile(r"^review\.r(\d+)\.md$")
HEAD_LINES = 10

STAGE_APPLIED = "applied"
STAGE_HAIKU = "haiku-pending"
STAGE_CONS = "cons-ready"
STAGE_REV = "rev-ready"
STAGE_REVISE = "revise"
STAGE_PARK = "park"
STAGE_APPLY = "apply-ready"


class FixWaveError(Exception):
    """Bad input: missing batch, unknown item, unreadable file. main() prints it and exits 2."""


def _read_tsv(path):
    if not path.is_file():
        return []
    with path.open(newline="", encoding="utf-8") as fh:
        return list(csv.DictReader(fh, delimiter="\t", quoting=csv.QUOTE_NONE))


def load_units(batch_dir):
    """[(uid, label)] from units.tsv, in file order."""
    return [((r.get("uid") or "").strip(), (r.get("label") or "").strip())
            for r in _read_tsv(batch_dir / "units.tsv") if (r.get("uid") or "").strip()]


def label_item(label):
    """WI-73-A -> WI-73, FU-holster-T2 -> FU-holster."""
    return LABEL_SUFFIX_RE.sub("", label)


def load_items(batch_dir):
    """Item ids in batch items.tsv order; without items.tsv, first-seen order of units.tsv labels."""
    rows = _read_tsv(batch_dir / "items.tsv")
    ids = [(r.get("id") or "").strip() for r in rows] if rows else [label_item(lbl) for _uid, lbl in load_units(batch_dir)]
    out = []
    for item in ids:
        if item and item not in out:
            out.append(item)
    return out


def units_for(batch_dir, item_id):
    return [(uid, lbl) for uid, lbl in load_units(batch_dir) if label_item(lbl) == item_id]


def reply_missing(batch_dir, uid):
    reply = batch_dir / f"{uid}.reply.txt"
    return not reply.is_file() or reply.stat().st_size == 0


def state_row(batch_dir, item_id):
    """(commit, stage) of the last state.tsv row for item_id, or ('', '')."""
    found = ("", "")
    for row in _read_tsv(batch_dir / "state.tsv"):
        if (row.get("id") or "").strip() == item_id:
            found = ((row.get("commit") or "").strip(), (row.get("stage") or "").strip())
    return found


def read_review(path):
    """(decision|None, blocking:int) from review.md. Missing file -> (None, 0).

    DECISION must be APPROVE, APPROVE_MINOR or REQUEST_CHANGES, else None. Missing BLOCKING -> 0.
    BLOCKING with no digits -> 1 (fail safe).
    """
    path = Path(path)
    if not path.is_file():
        return None, 0
    head = path.read_text(encoding="utf-8", errors="replace").splitlines()[:HEAD_LINES]
    decision, blocking = None, 0
    have_decision = have_blocking = False
    for raw in head:
        key, sep, value = raw.strip().strip("*`").strip().partition(":")
        if not sep:
            continue
        key = key.strip().strip("*").upper()
        value = value.strip().strip("*`").strip()
        if key == "DECISION" and not have_decision:
            have_decision = True
            token = value.rstrip(".").upper()
            decision = token if token in DECISIONS else None
        elif key == "BLOCKING" and not have_blocking:
            have_blocking = True
            digits = re.search(r"\d+", value)
            blocking = int(digits.group()) if digits else 1
    return decision, blocking


def round_files(item_dir):
    """review.rN.md files of one item, sorted by N."""
    if not item_dir.is_dir():
        return []
    found = []
    for p in item_dir.iterdir():
        m = ROUND_RE.match(p.name)
        if m and p.is_file():
            found.append((int(m.group(1)), p))
    return [p for _n, p in sorted(found)]


def _reply_output(reply):
    if not reply.is_file():
        return None
    for line in reply.read_text(encoding="utf-8", errors="replace").splitlines():
        if line.startswith("OUTPUT:"):
            value = line.split(":", 1)[1].strip()
            return Path(value) if value else None
    return None


def copy_out(batch_dir, item_id, repo_root):
    """Copy the reviewer's review.md into the batch dir when missing. Returns the source path or None."""
    batch_dir = Path(batch_dir)
    item_dir = batch_dir / item_id
    target = item_dir / "review.md"
    if target.is_file():
        return None
    archived = {p.read_bytes() for p in round_files(item_dir)}
    launched = item_dir / "rev.launched"
    floor = launched.stat().st_mtime if launched.is_file() else None

    def usable(path):
        if not path.is_file() or path.read_bytes() in archived:
            return False
        return floor is None or path.stat().st_mtime >= floor

    best = None
    wt_root = Path(repo_root) / ".claude" / "worktrees"
    rel = Path(".claude", "waves", batch_dir.name, item_id, "review.md")
    if wt_root.is_dir():
        for wt in sorted(wt_root.glob("agent-*")):
            cand = wt / rel
            if usable(cand) and (best is None or cand.stat().st_mtime > best.stat().st_mtime):
                best = cand
    if best is None:
        out = _reply_output(item_dir / "rev.reply.txt")
        if out is not None:
            cand = out if out.is_absolute() else Path(repo_root) / out
            if usable(cand):
                best = cand
    if best is None:
        return None
    shutil.copyfile(best, target)
    return best


def revise_item(batch_dir, item_id):
    """Archive review/final/verdict to round N and clear the launch markers. Returns N."""
    item_dir = Path(batch_dir) / item_id
    if not (item_dir / "review.md").is_file():
        raise FixWaveError(f"revise {item_id}: no review.md to archive in {batch_dir.name}")
    commit, _stage = state_row(batch_dir, item_id)
    if commit:
        raise FixWaveError(f"revise {item_id}: already applied at {commit}")
    n = len(round_files(item_dir)) + 1
    for src, dst in (("review.md", f"review.r{n}.md"), ("final.diff", f"final.r{n}.diff"),
                     ("verdict.md", f"verdict.r{n}.md")):
        if (item_dir / src).is_file():
            os.replace(item_dir / src, item_dir / dst)
    for marker in ("cons.launched", "rev.launched"):
        (item_dir / marker).unlink(missing_ok=True)
    return n


def stage_of(batch_dir, item_id, repo_root):
    """(stage, next_action) for one item. Runs copy-out first, reads files otherwise."""
    batch_dir = Path(batch_dir)
    item_dir = batch_dir / item_id
    copy_out(batch_dir, item_id, repo_root)
    commit, state_stage = state_row(batch_dir, item_id)
    if commit:
        return STAGE_APPLIED, f"committed {commit}"
    missing = [(uid, lbl) for uid, lbl in units_for(batch_dir, item_id) if reply_missing(batch_dir, uid)]
    if missing:
        names = ", ".join(f"{uid} ({lbl})" for uid, lbl in missing)
        return STAGE_HAIKU, f"await Haiku reply: {names}"
    rounds = round_files(item_dir)
    if not (item_dir / "final.diff").is_file():
        verb = "await consolidator" if (item_dir / "cons.launched").is_file() else "launch consolidator"
        nxt = f"{verb} {item_id}/consolidate.md"
        if rounds:
            nxt += f"; revision round {len(rounds)}, relay issues from {item_id}/{rounds[-1].name}"
        return STAGE_CONS, nxt
    review = item_dir / "review.md"
    if not review.is_file():
        verb = "await reviewer" if (item_dir / "rev.launched").is_file() else "launch reviewer"
        return STAGE_REV, f"{verb} {item_id}/review.brief.md"
    decision, blocking = read_review(review)
    if blocking > 0 or decision == "REQUEST_CHANGES":
        if 1 + len(rounds) < 2:
            return STAGE_REVISE, f"python3 scripts/fix_wave.py status {batch_dir.name} --revise {item_id}"
        return STAGE_PARK, f"park for Opus: {item_id}/review.md still blocks after {len(rounds)} revision round(s)"
    if decision is None:
        return STAGE_REV, f"review.md has no valid DECISION line: re-run reviewer {item_id}/review.brief.md"
    if state_stage == "gated":
        return STAGE_APPLY, "gates passed: run the close command printed by apply"
    return STAGE_APPLY, f"python3 scripts/fix_wave.py apply {batch_dir.name} {item_id}"


def _as_pair(mark):
    """None | (id, kind) | [(id, kind), ...] | [[id, kind], ...] -> list of (id, kind)."""
    if not mark:
        return []
    if isinstance(mark, (tuple, list)) and len(mark) == 2 and all(isinstance(x, str) for x in mark):
        return [tuple(mark)]
    return [tuple(m) for m in mark]


def _as_ids(revise):
    if not revise:
        return []
    return [revise] if isinstance(revise, str) else list(revise)


def inflight_count(batch_dir, items):
    n = sum(1 for uid, _lbl in load_units(batch_dir) if reply_missing(batch_dir, uid))
    for item in items:
        item_dir = batch_dir / item
        if (item_dir / "cons.launched").is_file() and not (item_dir / "final.diff").is_file():
            n += 1
        if (item_dir / "rev.launched").is_file() and not (item_dir / "review.md").is_file():
            n += 1
    return n


def status(waves_root, repo_root, wave_batch, max_inflight, mark=None, revise=None):
    """Status lines for one batch: inflight line, then one `<id> <stage> <next action>` line per item."""
    batch_dir = Path(waves_root) / wave_batch
    if not batch_dir.is_dir():
        raise FixWaveError(f"no batch dir: {batch_dir}")
    items = load_items(batch_dir)
    for item in items:
        copy_out(batch_dir, item, repo_root)
    for item in _as_ids(revise):
        if item not in items:
            raise FixWaveError(f"unknown item {item} in {wave_batch}")
        revise_item(batch_dir, item)
    for item, kind in _as_pair(mark):
        if item not in items:
            raise FixWaveError(f"unknown item {item} in {wave_batch}")
        if kind not in MARKS:
            raise FixWaveError(f"--mark kind must be cons or rev, got {kind!r}")
        (batch_dir / item).mkdir(exist_ok=True)
        (batch_dir / item / f"{kind}.launched").touch()
    inflight = inflight_count(batch_dir, items)
    lines = [f"inflight={inflight} free={max_inflight - inflight}"]
    for item in items:
        stage, nxt = stage_of(batch_dir, item, repo_root)
        lines.append(f"{item} {stage} {nxt}")
    return lines


def role_of(description):
    if not isinstance(description, str):
        return "other"
    m = ROLE_RE.match(description)
    return ROLE_NAMES[m.group(1)] if m else "other"


def run_agent_usage(argv):
    """Default runner: agent_usage.py --json with argv, parsed to a dict."""
    proc = subprocess.run([sys.executable, str(SCRIPTS / "agent_usage.py"), *argv],
                          capture_output=True, text=True, check=False)
    try:
        data = json.loads(proc.stdout)
    except json.JSONDecodeError:
        raise FixWaveError(f"agent_usage.py rc={proc.returncode} printed no JSON: {proc.stderr.strip()[-300:]}")
    if not isinstance(data, dict):
        raise FixWaveError("agent_usage.py JSON is not an object")
    return data


def _manifest_created(batch_dir):
    try:
        data = json.loads((batch_dir / "manifest.json").read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise FixWaveError(f"no readable manifest.json in {batch_dir}: {exc}")
    created = data.get("created") if isinstance(data, dict) else None
    if not isinstance(created, str) or not created:
        raise FixWaveError(f"manifest.json in {batch_dir} has no created time")
    return created


def cost(waves_root, wave_batch, sessions, run_usage=None):
    """Cost lines for one batch: per role, per model, total. One agent_usage call per session (or one, current session)."""
    run = run_usage or run_agent_usage
    batch_dir = Path(waves_root) / wave_batch
    created = _manifest_created(batch_dir)
    sessions = list(sessions or [])
    agents = []
    for sess in (sessions or [None]):
        argv = ["--since", created, "--json"]
        if sess:
            argv += ["--session", sess]
        agents.extend(run(argv).get("agents") or [])
    roles = {name: [0, 0.0] for name in (*ROLE_NAMES.values(), "other")}
    models = {}
    nulls = 0
    for agent in agents:
        usd = agent.get("cost_usd")
        if usd is None:
            nulls += 1
            usd = 0.0
        role = roles[role_of(agent.get("description"))]
        role[0] += 1
        role[1] += float(usd)
        model = models.setdefault(str(agent.get("model") or "unknown"), [0, 0.0])
        model[0] += 1
        model[1] += float(usd)
    total_usd = sum(v[1] for v in roles.values())
    lines = [f"{'role':<10} {'agents':>6} {'cost_usd':>10}"]
    lines += [f"{name:<10} {n:>6} {usd:>10.4f}" for name, (n, usd) in roles.items()]
    lines.append(f"{'total':<10} {len(agents):>6} {total_usd:>10.4f}")
    lines.append("by model:")
    lines += [f"  {name:<28} {n:>6} {usd:>10.4f}"
              for name, (n, usd) in sorted(models.items(), key=lambda kv: -kv[1][1])]
    if nulls:
        lines.append(f"FLAG: {nulls} agent(s) with null cost_usd counted as 0")
    lines.append(f"scope: since {created}; sessions: {', '.join(sessions) if sessions else 'current'}")
    return lines


def load_max_inflight(path):
    """max_inflight from limits.toml; default 20 when the file is absent or has no key."""
    path = Path(path)
    if not path.is_file():
        return DEFAULT_MAX_INFLIGHT
    try:
        data = tomllib.loads(path.read_text(encoding="utf-8"))
    except (OSError, tomllib.TOMLDecodeError) as exc:
        raise FixWaveError(f"bad limits file {path}: {exc}")
    value = data.get("max_inflight", DEFAULT_MAX_INFLIGHT)
    if not isinstance(value, int) or isinstance(value, bool) or value < 1:
        raise FixWaveError(f"max_inflight in {path} must be a positive integer, got {value!r}")
    return value


def build_parser():
    ap = argparse.ArgumentParser(description="Status and cost for one variant-fix batch (W-A).")
    sub = ap.add_subparsers(dest="cmd", required=True)
    for name in ("status", "cost"):
        p = sub.add_parser(name)
        p.add_argument("wave_batch", metavar="W-A")
        p.add_argument("--waves-root", help="default: <repo_root>/.claude/waves")
        p.add_argument("--repo-root", help="default: parent of scripts/")
    st = sub.choices["status"]
    st.add_argument("--mark", nargs=2, action="append", metavar=("ID", "KIND"), help="touch <id>/<cons|rev>.launched")
    st.add_argument("--revise", action="append", metavar="ID", help="archive review/final/verdict to round N")
    st.add_argument("--limits", help="default: <repo_root>/.claude/skills/variant-fix-wave/limits.toml")
    co = sub.choices["cost"]
    co.add_argument("--session", action="append", default=[], help="repeatable; default current session")
    return ap


def main(argv=None):
    args = build_parser().parse_args(argv)
    repo_root = Path(args.repo_root) if args.repo_root else DEFAULT_REPO
    waves_root = Path(args.waves_root) if args.waves_root else repo_root / ".claude" / "waves"
    try:
        if args.cmd == "status":
            limits = Path(args.limits) if args.limits else repo_root / ".claude" / "skills" / "variant-fix-wave" / "limits.toml"
            lines = status(waves_root, repo_root, args.wave_batch, load_max_inflight(limits),
                           mark=args.mark, revise=args.revise)
        else:
            lines = cost(waves_root, args.wave_batch, args.session)
    except FixWaveError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    print("\n".join(lines))
    return 0


if __name__ == "__main__":
    sys.exit(main())
