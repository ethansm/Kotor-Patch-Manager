#!/usr/bin/env python3
"""agent_contract.py - subagent contract templates and validator. Python 3, stdlib only.

Usage:
  agent_contract.py print {worker|recon}
  agent_contract.py snapshot [--out FILE] [--repo-root DIR]
  agent_contract.py check [--type auto|worker|recon] [--scope GLOB ...] [--baseline FILE] [--repo-root DIR] [--json] {FILE|-}

print     emit the contract block to append to a subagent brief (worker = edits code/docs/tests;
          recon = read-only research/audit/log triage, may write only its OUTPUT file).
snapshot  record `git status` + content hashes before launch (default .claude/contract_baseline.tsv).
check     validate a 5-line reply: shape, key order, values, claimed paths vs git, write scope.
          Uses --baseline, else the default snapshot file if present. Scope globs are fnmatch
          (`*` also matches `/`, so `dir/*` covers the subtree).

Output: `FAIL|WARN <CODE> <detail>` lines, then `VERDICT: PASS|FAIL type=<t> fails=F warns=W`. Exit 1 on FAIL.
Read-only except `snapshot`. Never runs the TESTS command. Never reads *credential* / *secret* files.
"""
import argparse
import fnmatch
import hashlib
import json
import os
import re
import subprocess
import sys
from pathlib import Path

DEFAULT_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_BASELINE = Path(".claude") / "contract_baseline.tsv"

SPECS = {
    "worker": {
        "header": "## Output Schema: WORKER. Reply with exactly these 5 lines in this order (evidence first, "
                  "verdict last). NO prose, NO diffs, NO compiler output.",
        "fields": [
            ("CREATED", "<comma-separated repo-relative paths, or None>"),
            ("MODIFIED", "<comma-separated repo-relative paths, or None>"),
            ("TESTS", "<exact command> -> <result, e.g. 52/52 PASS>, or None"),
            ("NOTES", "<one sentence on blockers/traps, or None>"),
            ("STATUS", "<PASS | FAIL>"),
        ],
        "verdict_key": "STATUS",
        "verdicts": ("PASS", "FAIL"),
    },
    "recon": {
        "header": "## Output Schema: RECON (read-only; write only your OUTPUT file). Reply with exactly these 5 lines "
                  "in this order (evidence first, verdict last). NO prose, NO pasted code.",
        "fields": [
            ("OUTPUT", "<repo-relative path of your findings file, or None>"),
            ("EVIDENCE", "<1-3 anchors: path:line | 0xADDR | log marker; ';'-separated>"),
            ("FINDING", "<most important result, <= 25 words>"),
            ("OPEN", "<one unresolved question, or None>"),
            ("VERDICT", "<CONFIRMED | REFUTED | PARTIAL | UNSURE>"),
        ],
        "verdict_key": "VERDICT",
        "verdicts": ("CONFIRMED", "REFUTED", "PARTIAL", "UNSURE"),
    },
}
RE_FIELD = re.compile(r"^([A-Z]+):\s*(.*)$")


def keys(kind):
    return [k for k, _ in SPECS[kind]["fields"]]


def template(kind):
    s = SPECS[kind]
    return "\n".join([s["header"]] + ["%s: %s" % kv for kv in s["fields"]])


def unsafe_name(path):
    n = os.path.basename(str(path)).lower()
    return "credential" in n or "secret" in n


# ---------------------------------------------------------------- parsing
def parse(text, kind="auto"):
    """Return (kind, fields{key: (value, lineno)}, problems[(level, code, detail)])."""
    probs = []
    fields = {}
    order = []
    all_keys = {k: t for t in SPECS for k in keys(t)}
    for ln, raw in enumerate(text.splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith("```") or line.startswith("~~~"):
            continue
        m = RE_FIELD.match(line)
        if not m or m.group(1) not in all_keys:
            probs.append(("FAIL", "PROSE", "line %d: %s" % (ln, line[:80])))
            continue
        k, v = m.group(1), m.group(2).strip()
        if k in fields:
            probs.append(("FAIL", "DUP_KEY", "%s (lines %d and %d)" % (k, fields[k][1], ln)))
            continue
        fields[k] = (v, ln)
        order.append(k)
    if kind == "auto":
        votes = {t: sum(1 for k in order if all_keys[k] == t) for t in SPECS}
        kind = max(votes, key=lambda t: (votes[t], t == "worker"))
    spec = SPECS[kind]
    for k in order:
        if all_keys[k] != kind:
            probs.append(("FAIL", "WRONG_KEY", "%s is not a %s key" % (k, kind)))
    missing = [k for k in keys(kind) if k not in fields]
    for k in missing:
        probs.append(("FAIL", "MISSING_KEY", k))
    present = [k for k in order if all_keys[k] == kind]
    if not missing and present != keys(kind):
        probs.append(("WARN", "ORDER", "got %s, want %s" % (",".join(present), ",".join(keys(kind)))))
    vk = spec["verdict_key"]
    if vk in fields and fields[vk][0].upper() not in spec["verdicts"]:
        probs.append(("FAIL", "BAD_VALUE", "%s=%r (want %s)" % (vk, fields[vk][0], "|".join(spec["verdicts"]))))
    if kind == "worker" and "TESTS" in fields:
        t = fields["TESTS"][0]
        if not is_none(t) and "->" not in t:
            probs.append(("WARN", "TESTS_FORM", "TESTS should be '<command> -> <result>': %r" % t[:80]))
    if kind == "recon" and "EVIDENCE" in fields and is_none(fields["EVIDENCE"][0]):
        probs.append(("FAIL", "EVIDENCE_EMPTY", "recon reply cites no anchor"))
    return kind, fields, probs


def is_none(v):
    return v.strip().strip("`").lower() in ("", "none", "n/a", "-")


def split_paths(v):
    if is_none(v):
        return []
    out = []
    for p in v.split(","):
        p = p.strip().strip("`'\"").strip()
        if p.startswith("./"):
            p = p[2:]
        if p:
            out.append(p)
    return out


# ---------------------------------------------------------------- git state
def file_hash(root, rel):
    full = Path(root) / rel
    if unsafe_name(rel):
        return "SKIP"
    if not full.exists():
        return "DEL"
    if full.is_dir():
        return "DIR"
    try:
        return hashlib.sha1(full.read_bytes()).hexdigest()
    except OSError:
        return "ERR"


def git_dirty(root):
    """{path: status} from `git status --porcelain -z -uall`."""
    out = subprocess.run(["git", "status", "--porcelain", "-z", "-uall"], cwd=str(root),
                         capture_output=True, text=True, check=True).stdout
    parts = out.split("\0")
    res = {}
    i = 0
    while i < len(parts):
        e = parts[i]
        i += 1
        if len(e) < 4:
            continue
        st, path = e[:2], e[3:]
        if st[0] in "RC":  # rename/copy: next field is the source path
            i += 1
        res[path] = st
    return res


def snapshot(root):
    return {p: (st, file_hash(root, p)) for p, st in git_dirty(root).items()
            if not os.path.basename(p).startswith("contract_baseline")}


def write_snapshot(root, out):
    snap = snapshot(root)
    out.parent.mkdir(parents=True, exist_ok=True)
    with open(out, "w", encoding="utf-8") as f:
        for p in sorted(snap):
            f.write("%s\t%s\t%s\n" % (snap[p][0], snap[p][1], p))
    return len(snap)


def read_snapshot(path):
    snap = {}
    with open(path, encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\n")
            if not line:
                continue
            st, h, p = line.split("\t", 2)
            snap[p] = (st, h)
    return snap


def changed_since(root, base):
    now = snapshot(root)
    changed = {p for p, v in now.items() if p not in base or base[p][1] != v[1]}
    changed |= {p for p in base if p not in now}  # reverted to clean
    return changed, now


# ---------------------------------------------------------------- check
def check(text, root, kind="auto", scopes=None, baseline=None):
    kind, fields, probs = parse(text, kind)
    root = Path(root)
    base = read_snapshot(baseline) if baseline else None
    if base is not None:
        changed, now = changed_since(root, base)
    else:
        now = snapshot(root)
        changed = None
    dirty = set(now)
    touched = changed if changed is not None else dirty

    if kind == "worker":
        created = split_paths(fields.get("CREATED", ("",))[0])
        modified = split_paths(fields.get("MODIFIED", ("",))[0])
        for p in created:
            if not (root / p).exists():
                probs.append(("FAIL", "NO_PATH", "CREATED %s does not exist" % p))
            elif p not in touched:
                probs.append(("FAIL", "NOT_DIRTY", "CREATED %s shows no change" % p))
        for p in modified:
            if p not in touched:
                probs.append(("FAIL", "NOT_DIRTY", "MODIFIED %s shows no change" % p))
        claimed = set(created) | set(modified)
    else:
        out = split_paths(fields.get("OUTPUT", ("",))[0])
        for p in out:
            if not (root / p).exists():
                probs.append(("FAIL", "NO_PATH", "OUTPUT %s does not exist" % p))
        claimed = set(out)

    if changed is not None:
        for p in sorted(changed - claimed):
            probs.append(("WARN", "UNCLAIMED", "%s changed since baseline but is not listed" % p))
    if scopes:
        candidates = claimed | (changed if changed is not None else set())
        for p in sorted(candidates):
            if not any(fnmatch.fnmatch(p, g) for g in scopes):
                probs.append(("FAIL", "OUT_OF_SCOPE", "%s outside %s" % (p, " ".join(scopes))))
    return kind, probs


def main(argv=None, out=sys.stdout, stdin=sys.stdin):
    ap = argparse.ArgumentParser(prog="agent_contract.py", description="Subagent contract templates and validator.")
    sub = ap.add_subparsers(dest="cmd", required=True)
    p_print = sub.add_parser("print", help="emit a contract block")
    p_print.add_argument("kind", choices=sorted(SPECS))
    p_snap = sub.add_parser("snapshot", help="record git state before launching a subagent")
    p_snap.add_argument("--out", default=None, help="baseline file (default .claude/contract_baseline.tsv)")
    p_snap.add_argument("--repo-root", default=None)
    p_chk = sub.add_parser("check", help="validate a subagent reply")
    p_chk.add_argument("reply", help="file with the reply, or - for stdin")
    p_chk.add_argument("--type", default="auto", choices=["auto"] + sorted(SPECS))
    p_chk.add_argument("--scope", nargs="+", default=None, metavar="GLOB", help="allowed write globs")
    p_chk.add_argument("--baseline", default=None, help="snapshot file (default: .claude/contract_baseline.tsv if present)")
    p_chk.add_argument("--repo-root", default=None)
    p_chk.add_argument("--json", action="store_true")
    a = ap.parse_args(argv)

    if a.cmd == "print":
        out.write(template(a.kind) + "\n")
        return 0
    root = Path(a.repo_root).expanduser().resolve() if a.repo_root else DEFAULT_ROOT
    if a.cmd == "snapshot":
        dest = Path(a.out) if a.out else root / DEFAULT_BASELINE
        n = write_snapshot(root, dest)
        out.write("baseline %s (%d dirty paths)\n" % (dest, n))
        return 0

    text = stdin.read() if a.reply == "-" else Path(a.reply).read_text(encoding="utf-8", errors="replace")
    baseline = a.baseline
    if baseline is None and (root / DEFAULT_BASELINE).is_file():
        baseline = str(root / DEFAULT_BASELINE)
    kind, probs = check(text, root, a.type, a.scope, baseline)
    fails = sum(1 for p in probs if p[0] == "FAIL")
    warns = len(probs) - fails
    verdict = "FAIL" if fails else "PASS"
    if a.json:
        out.write(json.dumps({"type": kind, "verdict": verdict, "fails": fails, "warns": warns,
                              "baseline": baseline,
                              "problems": [{"level": l, "code": c, "detail": d} for l, c, d in probs]},
                             sort_keys=True) + "\n")
    else:
        for l, c, d in probs:
            out.write("%s %s %s\n" % (l, c, d))
        out.write("VERDICT: %s type=%s fails=%d warns=%d\n" % (verdict, kind, fails, warns))
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
