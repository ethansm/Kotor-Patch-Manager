#!/usr/bin/env python3
"""Management CLI for the K1 -> K2 asset removal ledger (k1_port_sources.Ledger).

Assets from sources that need the original author's approval are flagged in the ledger
(removal_group pending_author_approval). This tool lists them, deletes the ones never approved,
records approvals, gates copies out of the repo or a release, registers existing files and lists credits.

Stdlib only. Runs under system python3 and under python -I (this directory is put on sys.path below).

Subcommands (all accept --ledger PATH; default env K1PORT_LEDGER, else the Ledger default):
  list     [--flagged-only] [--json]
  prune    [--removal-group pending_author_approval] [--source-id ID] [--apply]   dry-run unless --apply
  approve  --source-id ID --note TEXT
  gate     [--strict] PATH [PATH ...]
  adopt    --file PATH --kind texture|model|other --source-id ID
  credits  [--json]

Stage root = the ledger's own directory. Relative ledger entries are resolved against it.
Exit: 0 ok, 1 refusal / gate block / registry or ledger error, 2 bad arguments.
"""
import argparse
import hashlib
import json
import os
import stat
import sys
from pathlib import Path

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import k1_port_sources as ks  # noqa: E402
from k1_port_sources import KINDS, REMOVAL_PENDING, Ledger, SourceError  # noqa: E402

ASSET_SUFFIXES = (".tpc", ".tga", ".mdl", ".mdx")
FORBIDDEN_PARTS = ("steamapps/common", "compatdata")


def is_flagged(e: dict) -> bool:
    return bool(e.get("removal_group")) and not e.get("author_approved")


def entry_abs(raw, stage: Path) -> Path:
    p = Path(str(raw)).expanduser()
    if not p.is_absolute():
        p = stage / p
    return Path(os.path.abspath(p))


def is_under(child: Path, parent: Path) -> bool:
    try:
        child.relative_to(parent)
    except ValueError:
        return False
    return True


def repo_root_of(path: Path):
    """Nearest ancestor of path (inclusive) that holds a .git entry (directory or worktree file), else None."""
    for d in (path, *path.parents):
        if (d / ".git").exists():
            return d
    return None


REPO_ROOT = repo_root_of(HERE)  # git toplevel of the repo this script lives in (None outside a repo)


def sha256_of(path: Path) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def classify_entry(raw, stage_root) -> dict:
    """Decide what prune does with one ledger entry. Changes nothing on disk.

    Returns {"action": "delete" | "missing" | "refuse", "reason", "target", "link"}.
    target is the resolved regular file to delete; link is the entry path as written (absolute).
    """
    stage = Path(stage_root).resolve()
    link = entry_abs(raw, stage)
    out = {"action": "refuse", "reason": "", "target": None, "link": link}
    if not os.path.lexists(link):
        out.update(action="missing", reason="file not found")
        return out
    try:
        target = link.resolve()
    except (OSError, RuntimeError) as e:
        out["reason"] = f"cannot resolve path: {e}"
        return out
    for text in (link.as_posix(), target.as_posix()):
        if any(part in text for part in FORBIDDEN_PARTS):
            out["reason"] = "path contains steamapps/common or compatdata"
            return out
    if not is_under(target, stage):
        out["reason"] = f"resolves outside stage root {stage}"
        return out
    if not is_under(link.parent.resolve(), stage):
        out["reason"] = f"symlink lives outside stage root {stage}"
        return out
    repo = repo_root_of(target)
    if repo is not None or (REPO_ROOT is not None and is_under(target, REPO_ROOT)):
        out["reason"] = f"inside git repo {repo or REPO_ROOT}"
        return out
    if not os.path.exists(target):
        out["reason"] = "dangling symlink (target missing)"
        return out
    if not stat.S_ISREG(os.lstat(target).st_mode):
        out["reason"] = "not a regular file"
        return out
    out.update(action="delete", reason="", target=target)
    return out


def print_table(headers, rows) -> None:
    widths = [max(len(str(x)) for x in col) for col in zip(headers, *rows)]
    fmt = "  ".join("{:<%d}" % w for w in widths)
    print(fmt.format(*headers).rstrip())
    for r in rows:
        print(fmt.format(*[str(x) for x in r]).rstrip())


def cmd_list(led: Ledger, stage: Path, args) -> int:
    records = []
    for e in led.entries():
        flagged = is_flagged(e)
        if args.flagged_only and not flagged:
            continue
        sha = e.get("sha256") or ""
        records.append({
            "file": e.get("file"),
            "kind": e.get("kind"),
            "source_id": e.get("source_id"),
            "license_class": e.get("license_class"),
            "removal_group": e.get("removal_group"),
            "flagged": flagged,
            "approved": bool(e.get("author_approved")),
            "sha256": sha or None,
            "exists": entry_abs(e.get("file", ""), stage).is_file(),
        })
    if args.json:
        print(json.dumps({"ledger": str(led.path), "stage_root": str(stage), "entries": records}, indent=2))
        return 0
    print(f"ledger: {led.path}")
    if not records:
        print("no entries")
        return 0
    rows = [[r["file"], r["kind"], r["source_id"], r["license_class"], "yes" if r["flagged"] else "no",
             "yes" if r["approved"] else "no", (r["sha256"] or "-")[:12], "yes" if r["exists"] else "no"]
            for r in records]
    print_table(["file", "kind", "source_id", "class", "flagged", "approved", "sha256", "exists"], rows)
    print(f"entries: {len(records)}  flagged: {sum(r['flagged'] for r in records)}")
    return 0


def cmd_prune(led: Ledger, stage: Path, args) -> int:
    if args.source_id is not None:
        ks.get_source(args.source_id)
    selected = [e for e in led.flagged()
                if e.get("removal_group") == args.removal_group
                and (args.source_id is None or e.get("source_id") == args.source_id)]
    tally = {"delete": 0, "drop": 0, "refuse": 0}
    dirty = False
    for e in selected:
        raw = e["file"]
        c = classify_entry(raw, stage)
        if c["action"] == "refuse":
            tally["refuse"] += 1
            print(f"REFUSED  {raw}: {c['reason']}")
            continue
        if c["action"] == "missing":
            tally["drop"] += 1
            print(f"{'dropped (missing)' if args.apply else 'would drop (missing)'}  {raw}")
            if args.apply:
                led.remove(raw)
                dirty = True
            continue
        if not args.apply:
            tally["delete"] += 1
            print(f"would delete  {c['target']}  [{e.get('source_id')}]")
            continue
        try:
            os.unlink(c["target"])
            if c["link"] != c["target"] and os.path.islink(c["link"]):
                os.unlink(c["link"])
            tally["delete"] += 1
            print(f"deleted  {c['target']}  [{e.get('source_id')}]")
        except FileNotFoundError:
            tally["drop"] += 1
            print(f"dropped (already gone)  {raw}")
        except OSError as ex:
            tally["refuse"] += 1
            print(f"REFUSED  {raw}: delete failed: {ex}")
            continue
        led.remove(raw)
        dirty = True
    if not selected:
        print("nothing to prune: no flagged entries match")
    if args.apply:
        saved = f"ledger saved: {led.save()}" if dirty else "ledger unchanged"
        print(f"applied: deleted {tally['delete']}, dropped missing {tally['drop']}, "
              f"refused {tally['refuse']}. {saved}")
    else:
        print(f"dry-run: would delete {tally['delete']}, would drop missing {tally['drop']}, "
              f"refused {tally['refuse']}. Nothing changed; re-run with --apply to delete.")
    return 1 if tally["refuse"] else 0


def cmd_approve(led: Ledger, stage: Path, args) -> int:
    src = ks.get_source(args.source_id)
    if not src["requires_author_approval"]:
        raise SourceError(f"source {args.source_id!r} does not require author approval "
                          f"(class {src['license_class']}); nothing to approve")
    note = (args.note or "").strip()
    if not note:
        raise SourceError("--note must not be empty")
    n = led.mark_approved(args.source_id, note)
    led.save()
    print(f"approved {args.source_id}: {n} ledger entries unflagged; approval recorded "
          f"(future adds from this source inherit it)")
    return 0


def iter_files(paths):
    files, missing = [], []
    for raw in paths:
        p = Path(raw).expanduser()
        if p.is_dir():
            for root, dirs, names in os.walk(p):
                dirs.sort()
                files.extend(Path(root) / n for n in sorted(names))
        elif p.is_file():
            files.append(p)
        else:
            missing.append(raw)
    return files, missing


def cmd_gate(led: Ledger, stage: Path, args) -> int:
    """Content decides. Same basenames are allowed by design (an HD copy may sit next to, or over,
    the approval-free copy), so the basename is only used against flagged entries with no sha256."""
    files, missing = iter_files(args.paths)
    if missing:
        for raw in missing:
            print(f"error: no such file or directory: {raw}", file=sys.stderr)
        return 1
    flagged = [e for e in led.entries() if is_flagged(e)]
    flagged_nosha_by_name = {}  # basename-only matching: flagged entries with no sha256 recorded
    flagged_sha_by_name = {}    # same basename, but content is judged by sha256 only
    for e in flagged:
        name = Path(str(e["file"])).name.lower()
        bucket = flagged_sha_by_name if e.get("sha256") else flagged_nosha_by_name
        bucket.setdefault(name, []).append(e)
    registered = {entry_abs(e["file"], stage).resolve() for e in led.entries() if e.get("file")}
    blocked = warned = 0
    for f in files:
        sha = sha256_of(f)
        known = led.find_by_sha(sha)  # every ledger entry with this content, flagged or not
        reasons = {}
        for e in known:
            if is_flagged(e):
                reasons.setdefault(e["file"], f"sha256 match -> {e['file']} [{e.get('source_id')}]")
        for e in flagged_nosha_by_name.get(f.name.lower(), []):
            reasons.setdefault(e["file"], f"name match (no sha256 in ledger) -> {e['file']} [{e.get('source_id')}]")
        file_warned = False
        if reasons:
            blocked += 1
            print(f"BLOCKED  {f}  {next(iter(reasons.values()))}")
        elif not known and flagged_sha_by_name.get(f.name.lower()):
            # same name as a flagged asset, but its content is neither the flagged content nor any
            # known approval-free content (a known unflagged sha passes silently)
            e0 = flagged_sha_by_name[f.name.lower()][0]
            print(f"WARNING: same name as flagged asset, content unknown: {f} "
                  f"(flagged: {e0['file']} [{e0.get('source_id')}])", file=sys.stderr)
            file_warned = True
        if f.suffix.lower() in ASSET_SUFFIXES and not known and f.resolve() not in registered:
            print(f"WARNING: unregistered asset: {f} (not in ledger)", file=sys.stderr)
            file_warned = True
        warned += file_warned
    strict_note = " (--strict: warnings fail)" if args.strict else ""
    print(f"gate: scanned {len(files)} file(s), blocked {blocked}, warned {warned}{strict_note}")
    return 1 if blocked or (args.strict and warned) else 0


def cmd_adopt(led: Ledger, stage: Path, args) -> int:
    ks.get_source(args.source_id)
    f = Path(args.file).expanduser()
    if not f.is_file():
        raise SourceError(f"not a regular file: {f}")
    absf = Path(os.path.abspath(f))
    real = absf.resolve()
    if not is_under(real, stage):
        raise SourceError(f"file is outside stage root {stage}: {real}")
    sha = sha256_of(real)
    entry = led.add(str(absf), args.kind, args.source_id, sha256=sha)
    led.save()
    print(f"adopted {absf}  kind={args.kind} source={args.source_id} "
          f"flagged={is_flagged(entry)} sha256={sha[:12]}")
    return 0


def cmd_credits(led: Ledger, stage: Path, args) -> int:
    by_credit = {}
    with_credits, seen = set(), set()
    for e in led.entries():
        sid = e.get("source_id")
        seen.add(sid)
        creds = e.get("credits") or []
        if creds:
            with_credits.add(sid)
        for c in creds:
            rec = by_credit.setdefault(c, {"sources": set(), "entries": 0})
            rec["sources"].add(sid)
            rec["entries"] += 1
    rows = [{"credit": c, "sources": sorted(v["sources"]), "entries": v["entries"]}
            for c, v in sorted(by_credit.items(), key=lambda kv: kv[0].lower())]
    uncredited = sorted(seen - with_credits)
    if args.json:
        print(json.dumps({"credits": rows, "uncredited_sources": uncredited}, indent=2))
        return 0
    if rows:
        print_table(["credit", "sources", "entries"],
                    [[r["credit"], ", ".join(r["sources"]), r["entries"]] for r in rows])
    else:
        print("no credits recorded")
    if uncredited:
        print(f"no credit recorded for: {', '.join(uncredited)}")
    return 0


HANDLERS = {"list": cmd_list, "prune": cmd_prune, "approve": cmd_approve,
            "gate": cmd_gate, "adopt": cmd_adopt, "credits": cmd_credits}


def build_parser() -> argparse.ArgumentParser:
    common = argparse.ArgumentParser(add_help=False)
    common.add_argument("--ledger", metavar="PATH", default=None,
                        help="ledger json (default: env K1PORT_LEDGER, else the Ledger default)")
    parser = argparse.ArgumentParser(description="K1 port asset removal ledger: list, prune, approve, gate, adopt, credits.")
    sub = parser.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("list", parents=[common], help="table of ledger entries")
    p.add_argument("--flagged-only", action="store_true")
    p.add_argument("--json", action="store_true")

    p = sub.add_parser("prune", parents=[common], help="delete flagged files (dry-run unless --apply)")
    p.add_argument("--removal-group", default=REMOVAL_PENDING)
    p.add_argument("--source-id", default=None)
    p.add_argument("--apply", action="store_true", help="actually delete files and drop their entries")

    p = sub.add_parser("approve", parents=[common], help="record the author's approval for a source")
    p.add_argument("--source-id", required=True)
    p.add_argument("--note", required=True, help="who approved, where, when")

    p = sub.add_parser("gate", parents=[common], help="fail if files match flagged ledger entries")
    p.add_argument("paths", nargs="+", metavar="PATH", help="files or directories (walked recursively)")
    p.add_argument("--strict", action="store_true",
                   help="also fail on same-name files with unknown content and on unregistered assets")

    p = sub.add_parser("adopt", parents=[common], help="register an existing file under a registered source")
    p.add_argument("--file", required=True)
    p.add_argument("--kind", required=True, choices=KINDS)
    p.add_argument("--source-id", required=True)

    p = sub.add_parser("credits", parents=[common], help="unique credits across the ledger")
    p.add_argument("--json", action="store_true")
    return parser


def main(argv=None) -> int:
    args = build_parser().parse_args(argv)
    try:
        led = Ledger(args.ledger)
        stage = led.path.parent.resolve()
        return HANDLERS[args.cmd](led, stage, args)
    except (SourceError, OSError) as e:
        print(f"error: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
