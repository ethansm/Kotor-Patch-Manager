#!/usr/bin/env python3
"""Deterministic packet builder for the post-quota find-only audit (no LLM calls).

Subcommands (run from anywhere; paths resolve from this file):
  build                 scope -> sanitized worker tree -> hunks -> tools -> packets, coverage plan,
                        snapshot, deny settings, human-only list   (the default)
  verify-snapshot       exit 0 iff the worker tree still matches snapshot.json
  render <packet_id>    print the user prompt for one packet (content + candidates + facts)
  pending               print packet ids without a COMPLETE run, in launch order
  is-complete <id>      exit 0 iff runs/<id>.json is complete
  resize --from runs/<first>.json [--canary canary.json]
                        derive bytes-per-token and per-call overhead from measured usage, write
                        params.json, then re-pack packets that have not completed (no model call)
  build-tree <dest>     build a sanitized, history-free copy at <dest> (fact runner scratch copy)

Writes only under .claude/review/ and $KOTOR2_REVIEW_HOME (default ~/.cache/kotor2_review/).
Security: excluded paths (see EXCLUDE_*) are never read, diffed or copied; only their names are
listed (human-only list). Every git diff carries ':(exclude)' pathspecs as a second guard.
"""
from __future__ import annotations

import argparse
import fnmatch
import hashlib
import json
import os
import re
import shutil
import stat
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path, PurePosixPath

# ---------------------------------------------------------------- parameters
BOUNDARY_UTC = "2026-10-07T01:05:42Z"       # last Claude Code assistant turn; one parameter
BASE = "11e1284"
PINNED_HEAD = "22b56b427125e12bb7b2914d96b820973d06c445"
EXPECTED_COMMITS = 61
PACKET_TOKENS = 25_000                       # target content tokens per packet (initial guess)
BYTES_PER_TOKEN = 3.0                        # initial guess; resize replaces it from measured usage
MAX_FILES_PER_PACKET = 3
WP4_COMMITS = ["c5c67f8", "15c9bfd", "7daaec6", "f19df9a", "ec95c98", "776de4c", "7aeb296"]
NPC_COMMITS = ["57837e0", "83354c0"]

REVIEW = Path(__file__).resolve().parent
REPO = REVIEW.parent.parent
RHOME = Path(os.environ.get("KOTOR2_REVIEW_HOME", str(Path.home() / ".cache/kotor2_review")))
TREE = RHOME / "worktree"
WORK = REVIEW / "work"

# Security exclusions: case-insensitive; *_NAME match any path component, *_PATH match path suffix.
EXCLUDE_NAME_GLOBS = [".env", "*.pem", ".credentials.json", "endpoint.env", "*credential*", "*secret*"]
EXCLUDE_PATH_SUFFIX = ["tests/static/raw_addresses.toml"]
EXCLUDE_PATH_PARTS = ["sunshine/credentials/"]
GIT_EXCLUDES = [f":(exclude,glob,icase){p}" for p in (
    "**/.env", "**/*.pem", "**/.credentials.json", "**/endpoint.env", "**/sunshine/credentials/**",
    "tests/static/raw_addresses.toml", "**/*credential*", "**/*credential*/**",
    "**/*secret*", "**/*secret*/**")]
# This audit's own artifacts: out of scope and absent from the worker tree.
AUDIT_ARTIFACTS = [".opencode/plans/pending-agent-review.md", ".claude/review/", ".claude/reviews/",
                   ".claude/plans/master_review_audit_plan.md"]
# Never copied to the worker tree (agent config, history, caches, vendored, logs).
TREE_STRIP = [".git", ".claude/review", ".claude/reviews", ".claude/skills", ".claude/hooks",
              ".claude/agents", ".mcp.json", ".agents", ".opencode", "tests/.cache", ".venv",
              "logs", "node_modules", "__pycache__"]
TREE_STRIP_ANY_DEPTH = {".git", "__pycache__", "node_modules", ".venv"}
TREE_STRIP_GLOBS = [".claude/settings*.json", ".claude/plans/master_review_audit_plan.md"]
OUT_OF_SCOPE_PARTS = ["tests/.cache/", "/dump_excerpts/"]
BINARY_EXT = {".tpc", ".mdx", ".mdl", ".tga", ".dds", ".png", ".jpg", ".jpeg", ".bmp", ".gif", ".wav",
              ".mp3", ".ogg", ".db", ".sqlite", ".exe", ".dll", ".kpatch", ".zip", ".7z", ".gz", ".xz",
              ".bin", ".pdb", ".lib", ".a", ".o", ".obj", ".pyc", ".bak", ".ico", ".ttf", ".erf", ".rim",
              ".mod", ".bik", ".lip", ".ncs"}
CODE_EXT = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".inc", ".py", ".sh", ".bash", ".toml",
            ".def", ".ps1", ".bat", ".cmd", ".mk", ".cmake", ".ini", ".cfg", ".yml", ".yaml", ".nss"}
TREE_MAX_FILE = 2_000_000

DOMAINS = {
    "wp1": ("Gate integrity",
            "These files implement or configure the build/test gates, their selection logic, "
            "allowlists and the policy text agents follow. Gate output elsewhere is trusted only as far "
            "as these files are sound. Look for checks that were weakened, skipped, made unconditional "
            "or unfalsifiable; ignored exit codes; selection logic that silently drops tests; broad "
            "exception handling; tooling that can reach the live game directory or install paths; allowlist or "
            "config edits that silence a check instead of fixing its cause; policy text that loosens "
            "the gate rules."),
    "wp2": ("Uncommitted hooks and shared headers",
            "Uncommitted (working tree) changes to hook manifests, exports and shared headers. Check "
            "every new or changed detour byte by byte (stolen bytes, exits, registers restored or "
            "excluded, calling convention, stack frame), that every exported or hooked handler has a "
            "definition, Steam/GOG parity, new engine addresses (source of the address, consistency "
            "with other files), and shared-header changes that alter behaviour for other mods. Specific: the new "
            "PerformCastSpell detour at 0x00775703 (6 stolen bytes, consumed exit 0x0077574C, eax excluded "
            "from restore, params ecx/ebp): check the stolen window, the EBP-frame and RET-cleanup "
            "assumptions, and that the handler named in the hooks file and exports.def is defined."),
    "wp3": ("Uncommitted mod behaviour",
            "Uncommitted behaviour changes that agents recorded as done without committing. Check each "
            "change end to end inside the file: new UI paths and their state, predicates and key "
            "functions, behaviour on character change / screen close, logging ports (every old call "
            "site converted, fallbacks), version strings across files, build-flag portability (include paths in "
            "manifest.toml and test.toml). Specific: a leader (party member 0) heal button was added to the "
            "self-heal UI; LevelUpOverride.cpp was ported to the shared KpLog logger with a version bump."),
    "wp4": ("Provider batch c5c67f8..7aeb296",
            "Seven commits made within five minutes: new input/HUD/save-event providers, their hook "
            "files, consumer migrations and perf-lab changes. Check provider/consumer API contracts, "
            "migrations that removed code still needed, new detours, registration/unregistration and "
            "lifetime of callbacks."),
    "wp5": ("NPC diversity",
            "New NPC diversity dispatcher (commit 57837e0) and follow-up fix (83354c0): dispatcher "
            "code, hooks, generator scripts for 2DA and texture assets, manifest. Check dispatcher "
            "logic and hooks, generator determinism and output paths, manifest fields against how the "
            "build/gate tools read them. Binary assets are summarised under Known facts (size, blob, "
            "where referenced): check that each is referenced and plausible; never read binaries. No recorded "
            "gate run ever names this mod."),
    "wp6": ("Test changes",
            "Test files and test harness changes. For every removed or replaced assertion (see the "
            "assertion delta under Known facts) find its replacement, in this file or in a test file "
            "the code now delegates to, or flag it as dropped. Check that harness changes keep "
            "failures counted and reflected in the exit code."),
    "wp7": ("Small adoption batch, mouse probe, remaining tooling",
            "Smaller committed changes (shared-library adoption, mouse probe, equipment 3D fix, agent "
            "tooling scripts). Apply the generic checks."),
}
DOMAIN_ORDER = ["wp1", "wp2", "wp3", "wp4", "wp5", "wp6", "wp7"]
WP1_SCRIPTS = {"run_tests", "pre_install_gate", "plan_manager", "compat_check", "compat_graph",
               "abi_check", "master_ledger", "agent_diff_check", "agent_contract", "compat_breakage",
               "run_tests_select"}
WP3_DIRS = ["action_hotbar_src/", "companion_selfheal_src/", "level_up_override_src/",
            "inventory_3d_viewport_src/"]


# ---------------------------------------------------------------- helpers
def die(msg: str) -> None:
    print(f"prep.py: ERROR: {msg}", file=sys.stderr)
    sys.exit(2)


def git(*args: str, check: bool = True) -> str:
    r = subprocess.run(["git", "-C", str(REPO), *args], capture_output=True)
    if check and r.returncode not in (0, 1):
        die(f"git {' '.join(args[:4])} failed: {r.stderr.decode(errors='replace')[:300]}")
    return r.stdout.decode("utf-8", errors="replace")


def dump(path: Path, obj) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(obj, indent=1, sort_keys=True, ensure_ascii=False) + "\n")


def sha256_bytes(b: bytes) -> str:
    return hashlib.sha256(b).hexdigest()


def is_excluded(path: str) -> bool:
    p = path.replace("\\", "/").lower().lstrip("./")
    parts = p.split("/")
    if any(fnmatch.fnmatch(part, g) for part in parts for g in EXCLUDE_NAME_GLOBS):
        return True
    if any(p == s or p.endswith("/" + s) for s in EXCLUDE_PATH_SUFFIX):
        return True
    return any(x in "/" + p for x in ("/" + q for q in EXCLUDE_PATH_PARTS))


def is_audit_artifact(path: str) -> bool:
    return any(path == a or (a.endswith("/") and path.startswith(a)) for a in AUDIT_ARTIFACTS)


def out_of_scope_part(path: str) -> str | None:
    for part in OUT_OF_SCOPE_PARTS:
        if ("/" + path).find(part if part.startswith("/") else "/" + part) >= 0:
            return part.strip("/")
    return None


def tree_stripped(path: str) -> bool:
    for s in TREE_STRIP:
        if path == s or path.startswith(s + "/"):
            return True
        if s in TREE_STRIP_ANY_DEPTH and s in path.split("/"):   # .git, __pycache__ ... at any depth
            return True
    return any(fnmatch.fnmatch(path, g) for g in TREE_STRIP_GLOBS) or is_audit_artifact(path)


def boundary_epoch() -> float:
    return datetime.strptime(BOUNDARY_UTC, "%Y-%m-%dT%H:%M:%SZ").replace(tzinfo=timezone.utc).timestamp()


def looks_binary(p: Path) -> bool:
    try:
        with open(p, "rb") as f:
            return b"\0" in f.read(8192)
    except OSError:
        return False


# ---------------------------------------------------------------- worker tree
def build_tree(dest: Path) -> dict:
    """Sanitized, history-free copy of the working tree (tracked + untracked)."""
    if dest.resolve() == REPO or REPO in dest.resolve().parents:
        die("worker tree must live outside the repo")
    files, links, dropped = [], [], {"excluded": 0, "binary": 0, "stripped": 0, "unsafe_link": 0}
    for root, dirs, names in os.walk(REPO):
        rel_root = os.path.relpath(root, REPO)
        rel_root = "" if rel_root == "." else rel_root
        keep = []
        for d in sorted(dirs):
            rp = f"{rel_root}/{d}" if rel_root else d
            if tree_stripped(rp) or is_excluded(rp) or os.path.isdir(os.path.join(root, d, ".git")):
                dropped["stripped" if not is_excluded(rp) else "excluded"] += 1
                if os.path.islink(os.path.join(root, d)):
                    continue
                continue
            keep.append(d)
        dirs[:] = keep
        for n in sorted(names):
            rp = f"{rel_root}/{n}" if rel_root else n
            full = Path(root) / n
            if is_excluded(rp):
                dropped["excluded"] += 1
                continue
            if tree_stripped(rp):
                dropped["stripped"] += 1
                continue
            if full.is_symlink():
                tgt = (full.parent / os.readlink(full)).resolve()
                if REPO not in tgt.parents or is_excluded(str(tgt.relative_to(REPO))) or not tgt.is_file() \
                        or tree_stripped(str(tgt.relative_to(REPO))):
                    dropped["unsafe_link"] += 1
                    continue
                links.append(rp)
                continue
            if not full.is_file():
                continue
            if full.suffix.lower() in BINARY_EXT or full.stat().st_size > TREE_MAX_FILE or looks_binary(full):
                dropped["binary"] += 1
                continue
            files.append(rp)
    if dest.exists():
        shutil.rmtree(dest)
    dest.mkdir(parents=True)
    lst = RHOME / f"{dest.name}.files.txt"
    lst.write_text("\n".join(files + links) + "\n")
    r = subprocess.run(["rsync", "-a", "--files-from", str(lst), f"{REPO}/", f"{dest}/"], capture_output=True)
    if r.returncode:
        die(f"rsync failed: {r.stderr.decode()[:300]}")
    for d in dest.rglob("*"):                      # read-only snapshot
        if d.is_file() and not d.is_symlink():
            d.chmod(stat.S_IRUSR | stat.S_IRGRP)
    assert_tree_clean(dest)
    return {"files": len(files), "links": len(links), "dropped": dropped}


def assert_tree_clean(dest: Path) -> None:
    """find-based assertion: no .git, no review dir, no agent config, no excluded path, no escape."""
    expr = ["(", "-name", ".git", "-o", "-path", "*/.claude/review", "-o", "-path", "*/.claude/review/*",
            "-o", "-path", "*/.claude/reviews*", "-o", "-path", "*/.claude/skills*", "-o", "-path",
            "*/.claude/hooks*", "-o", "-path", "*/.claude/agents*", "-o", "-path", "*/.claude/settings*.json",
            "-o", "-name", ".mcp.json", "-o", "-name", ".agents", "-o", "-name", ".opencode",
            "-o", "-iname", ".env", "-o", "-iname", "*.pem", "-o", "-iname", ".credentials.json",
            "-o", "-iname", "endpoint.env", "-o", "-iname", "*credential*", "-o", "-iname", "*secret*",
            "-o", "-ipath", "*/sunshine/credentials*", "-o", "-ipath", "*/tests/static/raw_addresses.toml",
            "-o", "-name", "master_review_audit_plan.md", "-o", "-name", "pending-agent-review.md", ")", "-print"]
    hits = subprocess.run(["find", str(dest), *expr], capture_output=True, text=True).stdout.split()
    escapes = []
    for l in subprocess.run(["find", str(dest), "-type", "l"], capture_output=True, text=True).stdout.split():
        real = Path(os.path.realpath(l))
        if dest.resolve() not in real.parents:
            escapes.append(l)
    if hits or escapes:
        die(f"sanitized tree check FAILED: forbidden={hits[:10]} escaping_links={escapes[:10]}")


def tree_manifest(dest: Path) -> dict:
    out = {}
    for p in sorted(dest.rglob("*")):
        rp = str(p.relative_to(dest))
        if p.is_symlink():
            out[rp] = "link:" + os.readlink(p)
        elif p.is_file():
            out[rp] = sha256_bytes(p.read_bytes())
    return out


# ---------------------------------------------------------------- scope
def name_status(a: str, b: str | None) -> dict:
    args = ["diff", "--no-color", "--name-status", "-z", "-M", a] + ([b] if b else []) + ["--", "."]
    toks = git(*args).split("\0")
    out, i = {}, 0
    while i < len(toks) and toks[i]:
        st = toks[i]
        if st[0] in "RC":
            out[toks[i + 2]] = (st[0], toks[i + 1], int(st[1:] or 0))
            i += 3
        else:
            out[toks[i + 1]] = (st[0], toks[i + 1], 0)
            i += 2
    return out


def commits_by_file() -> tuple[list[dict], dict]:
    log = git("log", "--format=@@%h", "--name-only", "--no-renames", f"{BASE}..{PINNED_HEAD}")
    commits, by_file, cur = [], {}, None
    for line in log.splitlines():
        if line.startswith("@@"):
            cur = line[2:]
            commits.append(cur)
        elif line.strip():
            by_file.setdefault(line.strip(), []).append(cur)
    return commits, by_file


def dirty_entries() -> list[tuple[str, str]]:
    toks = git("status", "--porcelain=v1", "-z", "-uall").split("\0")
    out, i = [], 0
    while i < len(toks) and toks[i]:
        st, path = toks[i][:2], toks[i][3:]
        out.append((st, path))
        i += 2 if st[0] in "RC" else 1
    return out


def classify(path: str, binary: bool) -> tuple[str, str]:
    if is_excluded(path):
        return "excluded", "security-exclusion"
    p = PurePosixPath(path)
    ext = p.suffix.lower()
    if path.startswith(".claude/subagent_runs/"):
        return "c", "stale agent artifact (latest.patch is not a scope source)"
    if path in ("AGENTS.md", "CLAUDE.md"):
        return "a", "policy file (gate-rule text)"
    if binary or ext in BINARY_EXT:
        return "c", "binary"
    if ext in (".md", ".markdown", ".rst") or path.startswith(".claude/handoffs/") or path == ".claude/active_plan.txt":
        return "b", "docs-out-of-scope"
    if ext == ".2da":
        return "c", "generated game data table"
    if ext in (".jsonl", ".tsv", ".csv", ".log") or (path.startswith(".claude/") and ext == ".json") \
            or path.startswith("tests/benchmarks/"):
        return "c", "generated data / agent state"
    if ext in CODE_EXT or ext == ".json" or p.name in ("Makefile", "CMakeLists.txt"):
        return "a", "code"
    return "c", f"unclassified type '{ext or p.name}' (not code)"


def domain_for(path: str, source: str, commits: list[str]) -> str:
    p = PurePosixPath(path)
    stem = p.stem
    core = stem[5:] if stem.startswith("test_") else stem[:-5] if stem.endswith("_test") else stem
    if path in ("AGENTS.md", "CLAUDE.md") or path.startswith(("tests/static/", "tests/abi/")) \
            or path in ("tests/compat.toml", "tests/suites.toml") \
            or (path.startswith("scripts/") and core in WP1_SCRIPTS):
        return "wp1"
    is_test = stem.endswith("_test") or stem.startswith("test_") or "/tools/parity/" in path
    if is_test:
        return "wp6"
    dirty = source in ("dirty", "both", "untracked")
    if dirty and ("/_shared_src/" in path or "/queue_hud_src/" in path or path.endswith(".hooks.toml")
                  or (p.name == "exports.def" and not any(d in path for d in WP3_DIRS))):
        return "wp2"
    if dirty and any(d in path for d in WP3_DIRS):
        return "wp3"
    if "/npc_diversity_src/" in path or "diversity" in p.name:
        return "wp5"
    if any(c in WP4_COMMITS for c in commits) or any(d in path for d in (
            "/gui_input_src/", "/hud_host_src/", "/save_events_src/", "/perf_lab_src/")):
        return "wp4"
    return "wp7"


def module_key(path: str) -> str:
    p = PurePosixPath(path)
    if path.startswith("scripts/"):
        s = p.stem
        return "scripts/" + (s[5:] if s.startswith("test_") else s)
    return str(p.parent)


# ---------------------------------------------------------------- diffs
HUNK_RE = re.compile(r"^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@(.*)$")


def file_diff(rec: dict, ctx: str) -> str:
    args = ["diff", "--no-color", "--no-ext-diff", "-M", ctx, rec["diff_base"]]
    if rec["diff_target"] == "HEAD":
        args.append(PINNED_HEAD)
    paths = sorted({rec["file"], rec["old_path"] or rec["file"]})
    return git(*args, "--", *paths, *GIT_EXCLUDES)


def parse_hunks(text: str) -> tuple[list[dict], dict]:
    meta = {"binary": False, "symlink": False, "similarity": None, "mode_change": False}
    hunks, cur = [], None
    for line in text.split("\n"):
        m = HUNK_RE.match(line)
        if m:
            cur = {"os": int(m[1]), "ol": int(m[2]) if m[2] is not None else 1, "ns": int(m[3]),
                   "nl": int(m[4]) if m[4] is not None else 1, "func": m[5].strip(), "lines": []}
            hunks.append(cur)
            continue
        if cur is None:
            if line.startswith("Binary files") or line.startswith("GIT binary patch"):
                meta["binary"] = True
            if "120000" in line and ("mode" in line):
                meta["symlink"] = True
            if line.startswith("similarity index"):
                meta["similarity"] = int(line.split()[-1].rstrip("%"))
            if line.startswith(("old mode", "new mode")):
                meta["mode_change"] = True
            continue
        if line.startswith(("+", "-", " ")):
            cur["lines"].append(line)
        elif line.startswith("\\"):
            continue
    return hunks, meta


def hunk_id(file: str, source: str, os_: int, ol: int, ns: int, nl: int, kind: str) -> str:
    return "h-" + sha256_bytes(f"{file}|{source}|{kind}|{os_},{ol}|{ns},{nl}".encode())[:12]


# ---------------------------------------------------------------- build
def scope() -> tuple[list[dict], dict]:
    head = git("rev-parse", "HEAD").strip()
    if head != PINNED_HEAD:
        die(f"HEAD moved: {head} != pinned {PINNED_HEAD}. Re-plan or check out the pinned HEAD.")
    if subprocess.run(["git", "-C", str(REPO), "merge-base", "--is-ancestor", BASE, PINNED_HEAD]).returncode:
        die(f"{BASE} is not an ancestor of {PINNED_HEAD}")
    commits, by_file = commits_by_file()
    if len(commits) != EXPECTED_COMMITS:
        print(f"prep.py: WARNING: {len(commits)} commits in range, expected {EXPECTED_COMMITS}", file=sys.stderr)
    committed = name_status(BASE, PINNED_HEAD)
    worktree_ns = name_status(BASE, None)
    numstat_bin = set()
    for line in git("diff", "--numstat", "-z", BASE, PINNED_HEAD, "--", ".", *GIT_EXCLUDES).split("\0"):
        if line.startswith("-\t-\t"):
            numstat_bin.add(line.split("\t", 2)[2])
    bnd = boundary_epoch()
    stats = {"commits": len(commits), "committed_files": len(committed), "dirty_total": 0,
             "dirty_post": 0, "dirty_pre": 0, "dirty_out_of_scope": {}, "audit_artifacts": [],
             "dirty_mtime_unknown": []}
    dirty_post, untracked = {}, set()
    for st, path in dirty_entries():
        stats["dirty_total"] += 1
        if is_audit_artifact(path):
            stats["audit_artifacts"].append(path)
            continue
        oos = out_of_scope_part(path)
        if oos:
            stats["dirty_out_of_scope"][oos] = stats["dirty_out_of_scope"].get(oos, 0) + 1
            continue
        try:
            mt = os.lstat(REPO / path).st_mtime
        except FileNotFoundError:
            stats["dirty_mtime_unknown"].append(path)    # deleted in worktree: include, flagged
            mt = bnd + 1
        if mt <= bnd:
            stats["dirty_pre"] += 1
            continue
        stats["dirty_post"] += 1
        dirty_post[path] = st
        if st == "??":
            untracked.add(path)
    records, human_only = [], []
    files = sorted(set(committed) | set(dirty_post))
    for f in files:
        if is_audit_artifact(f) or out_of_scope_part(f):
            continue
        in_c, in_d = f in committed, f in dirty_post
        source = "untracked" if f in untracked else "both" if in_c and in_d else "dirty" if in_d else "committed"
        cms = sorted(set(by_file.get(f, [])), key=commits.index) if by_file.get(f) else []
        if is_excluded(f):
            for c in cms or ["WORKTREE"]:
                human_only.append({"commit": c, "path": f})
            records.append(base_rec(f, None, source, None, None, "binary", "?", "excluded",
                                    "security-exclusion", cms))
            continue
        if source == "untracked":
            full = REPO / f
            binary = full.suffix.lower() in BINARY_EXT or looks_binary(full)
            cls, why = classify(f, binary)
            records.append(base_rec(f, None, source, None, None, "whole-file", "A", cls, why, cms))
            continue
        st, old, sim = (worktree_ns if in_d else committed).get(f, committed.get(f, ("M", f, 0)))
        target = "WORKTREE" if in_d else "HEAD"
        binary = f in numstat_bin
        cls, why = classify(f, binary)
        records.append(base_rec(f, old if old != f else None, source, BASE, target, "diff", st, cls, why, cms,
                                similarity=sim))
    return records, {"stats": stats, "human_only": human_only, "commits": commits}


def base_rec(f, old, source, dbase, dtarget, kind, status, cls, why, commits, similarity=0) -> dict:
    return {"file": f, "old_path": old, "source": source, "diff_base": dbase, "diff_target": dtarget,
            "kind": kind, "status": status, "class": cls, "class_reason": why, "commits": commits,
            "similarity": similarity}


def explode_hunks(records: list[dict]) -> list[dict]:
    """Turn file records into coverage units (one per -U0 change block or one file-level unit)."""
    units = []
    for r in records:
        dom = domain_for(r["file"], r["source"], r["commits"]) if r["class"] == "a" else None
        base = {k: r[k] for k in ("file", "old_path", "source", "diff_base", "diff_target", "status",
                                  "class", "class_reason", "commits")}
        base.update(domain=dom, symlink_target=None)
        if r["class"] in ("excluded", "b", "c") and r["kind"] != "whole-file" or r["class"] == "excluded":
            units.append({**base, "kind": "binary" if r["class"] in ("excluded", "c") else "diff",
                          "old_start": 0, "old_len": 0, "new_start": 0, "new_len": 0, "minus": [], "plus": [],
                          "hunk_id": hunk_id(r["file"], r["source"], 0, 0, 0, 0, "file")})
            continue
        if r["kind"] == "whole-file":
            full = REPO / r["file"]
            if full.is_symlink():
                tgt = os.readlink(full)
                units.append({**base, "kind": "symlink", "symlink_target": tgt, "old_start": 0, "old_len": 0,
                              "new_start": 1, "new_len": 1, "minus": [], "plus": [tgt],
                              "hunk_id": hunk_id(r["file"], r["source"], 0, 0, 1, 1, "symlink")})
                continue
            lines = [] if r["class"] != "a" else full.read_text(errors="replace").split("\n")
            if lines and lines[-1] == "":
                lines.pop()
            units.append({**base, "kind": "whole-file", "old_start": 0, "old_len": 0, "new_start": 1,
                          "new_len": len(lines), "minus": [], "plus": lines,
                          "hunk_id": hunk_id(r["file"], r["source"], 0, 0, 1, len(lines), "whole")})
            continue
        hunks, meta = parse_hunks(file_diff(r, "-U0"))
        if meta["binary"]:
            units.append({**base, "class": "c", "class_reason": "binary", "kind": "binary", "old_start": 0,
                          "old_len": 0, "new_start": 0, "new_len": 0, "minus": [], "plus": [],
                          "hunk_id": hunk_id(r["file"], r["source"], 0, 0, 0, 0, "file")})
            continue
        if not hunks:
            kind = "rename" if r["status"] == "R" else "mode"
            units.append({**base, "kind": kind, "old_start": 0, "old_len": 0, "new_start": 0, "new_len": 0,
                          "minus": [], "plus": [], "similarity": r["similarity"],
                          "hunk_id": hunk_id(r["file"], r["source"], 0, 0, 0, 0, kind)})
            continue
        for h in hunks:
            kind = "symlink" if meta["symlink"] else "diff"
            plus = [l[1:] for l in h["lines"] if l.startswith("+")]
            units.append({**base, "kind": kind, "old_start": h["os"], "old_len": h["ol"], "new_start": h["ns"],
                          "new_len": h["nl"], "minus": [l[1:] for l in h["lines"] if l.startswith("-")],
                          "plus": plus, "symlink_target": plus[0] if kind == "symlink" and plus else None,
                          "hunk_id": hunk_id(r["file"], r["source"], h["os"], h["ol"], h["ns"], h["nl"], kind)})
    ids = [u["hunk_id"] for u in units]
    if len(ids) != len(set(ids)):
        die("duplicate hunk ids")
    return units


def mirror_dupes(records: list[dict]) -> None:
    """Hash-compare class-a post-images; keep the first of identical files, mark others class c."""
    seen = {}
    for r in records:
        if r["class"] != "a" or r["status"] == "D":
            continue
        p = TREE / r["file"]
        if not p.is_file() or p.is_symlink():
            continue
        h = sha256_bytes(p.read_bytes())
        if p.stat().st_size > 0 and h in seen:
            r["class"], r["class_reason"] = "c", f"mirror-duplicate of {seen[h]}"
        else:
            seen.setdefault(h, r["file"])


def run_tools() -> None:
    for t in ("classify_hunks", "assertion_delta", "scan_patterns"):
        tool = REVIEW / "tools" / f"{t}.py"
        if not tool.exists():
            die(f"missing helper {tool}")
        r = subprocess.run([sys.executable, "-I", str(tool), "--review-dir", str(REVIEW), "--repo", str(REPO),
                            "--tree", str(TREE), "--base", BASE], capture_output=True, text=True)
        print(f"  {t}: {r.stdout.strip()[:200]}")
        if r.returncode:
            die(f"{t} failed: {r.stderr[-500:]}")


# ---------------------------------------------------------------- packets
def params() -> dict:
    p = {"packet_tokens": PACKET_TOKENS, "bytes_per_token": BYTES_PER_TOKEN, "overhead_tokens": None}
    f = REVIEW / "params.json"
    if f.exists():
        p.update(json.loads(f.read_text()))
    return p


def render_lines(hunk_lines: list[str], os_: int, ns: int) -> list[tuple[str, int | None, int | None]]:
    """-> [(rendered, old_no, new_no)] with post-image numbers on context/added lines."""
    out, o, n = [], os_, ns
    for l in hunk_lines:
        tag, text = l[:1], l[1:]
        if tag == " ":
            out.append((f"{n:>6}   {text}", o, n)); o += 1; n += 1
        elif tag == "+":
            out.append((f"{n:>6} + {text}", None, n)); n += 1
        else:
            out.append((f"{'o' + str(o):>6} - {text}", o, None)); o += 1
    return out


def units_in(rendered, units: list[dict]) -> list[str]:
    olds = {o for _, o, n in rendered if n is None and o is not None}
    news = {n for _, o, n in rendered if o is None and n is not None}
    got = []
    for u in units:
        uo = set(range(u["old_start"], u["old_start"] + u["old_len"]))
        un = set(range(u["new_start"], u["new_start"] + u["new_len"]))
        if uo & olds or un & news:
            got.append(u["hunk_id"])
    return got


def chunk(rendered, budget: int):
    cur, size = [], 0
    for item in rendered:
        b = len(item[0].encode()) + 1
        if cur and size + b > budget:
            yield cur
            cur, size = [], 0
        cur.append(item); size += b
    if cur:
        yield cur


def file_parts(rec_units: list[dict], review_ids: set, budget: int) -> list[dict]:
    u0 = rec_units[0]
    f = u0["file"]
    if u0["kind"] == "whole-file":
        lines = [(f"{i + 1:>6}   {t}", None, i + 1) for i, t in enumerate(u0["plus"])]
        return [{"file": f, "mode": "whole", "text": "\n".join(x[0] for x in c),
                 "new_range": f"{c[0][2]}-{c[-1][2]}", "hunk_ids": [u0["hunk_id"]]} for c in chunk(lines, budget)]
    if u0["kind"] == "symlink":
        return [{"file": f, "mode": "whole", "text": f"(symlink) -> {u0['symlink_target']}", "new_range": "1-1",
                 "hunk_ids": [u0["hunk_id"]]}]
    parts = []
    rec = {"file": f, "old_path": u0["old_path"], "diff_base": u0["diff_base"], "diff_target": u0["diff_target"]}
    wh, _ = parse_hunks(file_diff(rec, "-W"))
    u12 = None
    for h in wh:
        rendered = render_lines(h["lines"], h["os"], h["ns"])
        ids = units_in(rendered, rec_units)
        if not set(ids) & review_ids:
            continue
        pieces = [(rendered, h["func"], "W")]
        if sum(len(x[0]) + 1 for x in rendered) > budget:     # fall back to -U12 inside this region
            if u12 is None:
                u12, _ = parse_hunks(file_diff(rec, "-U12"))
            lo, hi = h["ns"], h["ns"] + h["nl"]
            pieces = [(render_lines(x["lines"], x["os"], x["ns"]), x["func"], "U12") for x in u12
                      if lo <= x["ns"] <= hi and set(units_in(render_lines(x["lines"], x["os"], x["ns"]), rec_units)) & review_ids]
        for r_lines, func, mode in pieces:
            for c in chunk(r_lines, budget):
                cids = units_in(c, rec_units)
                if not set(cids) & review_ids:
                    continue
                nn = [n for _, _, n in c if n is not None] or [0]
                head = f"@@ {func}" if func else "@@"
                parts.append({"file": f, "mode": mode if len(c) == len(r_lines) else "range",
                              "text": head + "\n" + "\n".join(x[0] for x in c),
                              "new_range": f"{min(nn)}-{max(nn)}", "hunk_ids": cids})
    return parts


def build_packets(units: list[dict], frozen: dict) -> tuple[list[dict], dict]:
    prm = params()
    budget = int(prm["packet_tokens"] * prm["bytes_per_token"])
    verdicts = json.loads((WORK / "classifier_verdicts.json").read_text())["verdicts"]
    mech = {v["hunk_id"] for v in verdicts if v["pass"]}
    comment_only = {v["hunk_id"] for v in verdicts if v["pass"] and v["rule"] == "comment-only"}
    by_file = {}
    for u in units:
        by_file.setdefault(u["file"], []).append(u)
    frozen_files = {f for p in frozen.values() for f in p["files"]}
    work = []
    for f, us in sorted(by_file.items()):
        if us[0]["class"] != "a" or f in frozen_files:
            continue
        review = {u["hunk_id"] for u in us if u["hunk_id"] not in mech}
        if not review:
            continue
        # comment-only hunks keep their mechanical verdict but ride along as context (comment/code drift)
        shown = review | {u["hunk_id"] for u in us if u["hunk_id"] in comment_only}
        for part in file_parts(us, shown, budget):
            work.append((DOMAIN_ORDER.index(us[0]["domain"]), us[0]["domain"], module_key(f), f, part))
    work.sort(key=lambda x: (x[0], x[2], x[3]))
    packets, counters = dict(frozen), {}
    for p in frozen.values():
        d, n = p["packet_id"].rsplit("-", 1)
        counters[d] = max(counters.get(d, 0), int(n))
    cur = None
    for _, dom, mod, f, part in work:
        size = len(part["text"].encode())
        if cur is None or cur["domain"] != dom or cur["_mod"] != mod or cur["est_bytes"] + size > budget \
                or (f not in cur["files"] and len(cur["files"]) >= MAX_FILES_PER_PACKET):
            counters[dom] = counters.get(dom, 0) + 1
            cur = {"packet_id": f"{dom}-{counters[dom]:03d}", "domain": dom, "_mod": mod, "files": [],
                   "parts": [], "est_bytes": 0}
            packets[cur["packet_id"]] = cur
        if f not in cur["files"]:
            cur["files"].append(f)
        cur["parts"].append(part)
        cur["est_bytes"] += size
    return packets, {"mechanical": mech, "budget_bytes": budget}


def finalize_packets(packets: dict, units: list[dict], frozen: dict) -> None:
    prm = params()
    cands = json.loads((WORK / "scanner_candidates.json").read_text())["candidates"]
    for pid, p in packets.items():
        if pid in frozen:
            continue
        p.pop("_mod", None)
        p["hunks"] = sorted({h for part in p["parts"] for h in part["hunk_ids"]})
        p["mods"] = sorted({f.split("/")[1] for f in p["files"] if f.startswith("patch_manager_mods/")})
        ranges = {}
        for part in p["parts"]:
            a, b = (int(x) for x in part["new_range"].split("-"))
            ranges.setdefault(part["file"], []).append((a - 3, b + 3))
        p["candidates"] = sorted(c["candidate_id"] for c in cands if c["file"] in ranges and any(
            lo <= int(c["lines"].split("-")[0]) <= hi for lo, hi in ranges[c["file"]]))
        p["est_tokens"] = int(p["est_bytes"] / prm["bytes_per_token"])
        text_path = REVIEW / "packets" / f"{pid}.txt"
        text_path.write_text("\n\n".join(f"### {part['file']}  [{part['mode']}, post-image lines "
                                         f"{part['new_range']}]\n{part['text']}" for part in p["parts"]) + "\n")
        p["content"] = f"packets/{pid}.txt"
        p["parts"] = [{k: v for k, v in part.items() if k != "text"} for part in p["parts"]]
        dump(REVIEW / "packets" / f"{pid}.json", p)


def npc_asset_facts(records: list[dict]) -> None:
    gens = [TREE / "scripts/generate_diversity_2da.py", TREE / "scripts/normalize_diversity_textures.py"]
    blob_text = "".join(g.read_text(errors="replace") for g in gens if g.exists())
    for m in (TREE / "patch_manager_mods/npc_diversity_src").glob("*") if (TREE / "patch_manager_mods/npc_diversity_src").exists() else []:
        if m.is_file():
            blob_text += m.read_text(errors="replace")
    assets = []
    for r in records:
        if r["class"] == "c" and r["class_reason"] in ("binary", "generated game data table") \
                and any(c in NPC_COMMITS for c in r["commits"]):
            spec = f"{PINNED_HEAD}:{r['file']}"
            size = git("cat-file", "-s", spec, check=False).strip()
            blob = git("rev-parse", spec, check=False).strip()
            stem = PurePosixPath(r["file"]).stem
            assets.append({"path": r["file"], "size": int(size) if size.isdigit() else None, "blob": blob[:12],
                           "commits": r["commits"], "name_referenced_in_code_or_generators": stem in blob_text})
    dump(REVIEW / "facts" / "npc_assets.json", {"note": "deterministic; sizes from git at the pinned HEAD",
                                                 "assets": assets})


def write_settings() -> None:
    rel = [".env", "*.pem", ".credentials.json", "endpoint.env", "*credential*", "*secret*",
           "*credential*/**", "*secret*/**", "sunshine/credentials/**", "tests/static/raw_addresses.toml"]
    deny = []
    for p in rel:
        for form in (f"Read({p})", f"Read(**/{p})", f"Read(//**/{p})"):
            if form not in deny:
                deny.append(form)
    deny += [f"Read(/{REPO}/**)", f"Read(/{REPO.parent}/**)", "Read(~/.claude/**)", "Read(~/.config/**)",
             "Read(~/.ssh/**)", "Read(//**/.git/**)", "Edit(**)", "Edit(//**)",
             "Bash", "Edit", "Write", "NotebookEdit", "WebFetch", "WebSearch", "Agent", "Task", "Skill"]
    dump(REVIEW / "worker_settings.json", {"permissions": {"deny": deny, "defaultMode": "dontAsk"}})


def frozen_packets() -> dict:
    out = {}
    for pj in sorted((REVIEW / "packets").glob("*.json")):
        p = json.loads(pj.read_text())
        if run_complete(p["packet_id"]):
            out[p["packet_id"]] = p
    files = {f for p in out.values() for f in p["files"]}       # freeze every packet of a touched file
    for pj in sorted((REVIEW / "packets").glob("*.json")):
        p = json.loads(pj.read_text())
        if p["packet_id"] not in out and set(p["files"]) & files:
            out[p["packet_id"]] = p
    return out


def run_complete(pid: str) -> bool:
    f = REVIEW / "runs" / f"{pid}.json"
    if not f.exists():
        return False
    r = json.loads(f.read_text())
    return r.get("status") == "ok" and r.get("schema_valid") and not r.get("truncated")


def cmd_build(repack_only: bool = False) -> None:
    RHOME.mkdir(parents=True, exist_ok=True)
    print(f"prep.py: boundary {BOUNDARY_UTC}, base {BASE}, pinned HEAD {PINNED_HEAD[:7]}")
    records, info = scope()
    if not repack_only:
        tinfo = build_tree(TREE)
        print(f"  worker tree: {tinfo}")
        manifest = tree_manifest(TREE)
        snap = {"head": PINNED_HEAD, "head_tree": git("rev-parse", f"{PINNED_HEAD}^{{tree}}").strip(),
                "boundary_utc": BOUNDARY_UTC, "base": BASE, "tree_path": str(TREE), "file_count": len(manifest),
                "manifest_sha256": sha256_bytes(json.dumps(manifest, sort_keys=True).encode()),
                "manifest": manifest}
        dump(REVIEW / "snapshot.json", snap)
    else:
        verify_snapshot(strict=True)
    mirror_dupes(records)
    units = explode_hunks(records)
    WORK.mkdir(parents=True, exist_ok=True)
    with open(WORK / "hunks.jsonl", "w") as fh:
        for u in units:
            fh.write(json.dumps(u, sort_keys=True, ensure_ascii=False) + "\n")
    run_tools()
    npc_asset_facts(records)
    frozen = frozen_packets() if repack_only else {}
    if not repack_only:
        for d in ("packets",):
            shutil.rmtree(REVIEW / d, ignore_errors=True)
    (REVIEW / "packets").mkdir(exist_ok=True)
    for pid in list((REVIEW / "packets").glob("*")):
        if pid.stem not in frozen:
            pid.unlink()
    packets, meta = build_packets(units, frozen)
    finalize_packets(packets, units, frozen)
    membership = {}
    for p in packets.values():
        for h in p["hunks"]:
            membership.setdefault(h, []).append(p["packet_id"])
    plan_units = []
    for u in units:
        rng = f"-{u['old_start']},{u['old_len']} +{u['new_start']},{u['new_len']}"
        if u["class"] == "excluded":
            pl, why = "excluded", "security-exclusion (human-only)"
        elif u["class"] in ("b", "c"):
            pl, why = "skipped", u["class_reason"]
        elif u["hunk_id"] in meta["mechanical"]:
            pl, why = "mechanical", "classifier verdict"
        else:
            pl, why = "review", u["domain"]
            if u["hunk_id"] not in membership:
                die(f"review unit {u['hunk_id']} ({u['file']} {rng}) landed in no packet")
        plan_units.append({"hunk_id": u["hunk_id"], "file": u["file"], "range": rng, "planned": pl,
                           "packets": sorted(membership.get(u["hunk_id"], [])), "reason": why,
                           "domain": u["domain"], "source": u["source"], "kind": u["kind"]})
    snap_sha = sha256_bytes((REVIEW / "snapshot.json").read_bytes())
    dump(REVIEW / "coverage_plan.json", {"snapshot": snap_sha, "units": plan_units})
    dump(REVIEW / "human_only.json", {"note": "names only; content never read",
                                      "items": info["human_only"]})
    write_settings()
    summary = summarize(records, units, plan_units, packets, info, meta)
    dump(REVIEW / "prep_summary.json", summary)
    print(json.dumps({k: v for k, v in summary.items() if k not in ("files_by_class", "skipped_docs",
                                                                     "skipped_other")}, indent=1))


def summarize(records, units, plan_units, packets, info, meta) -> dict:
    by_cls, by_src, docs, other = {}, {}, [], []
    for r in records:
        by_cls[r["class"]] = by_cls.get(r["class"], 0) + 1
        by_src[r["source"]] = by_src.get(r["source"], 0) + 1
        if r["class"] == "b":
            docs.append(r["file"])
        elif r["class"] == "c":
            other.append(f"{r['file']}  ({r['class_reason']})")
    plan = {}
    for u in plan_units:
        plan[u["planned"]] = plan.get(u["planned"], 0) + 1
    dom = {}
    for p in packets.values():
        d = dom.setdefault(p["domain"], {"packets": 0, "est_tokens": 0, "max_tokens": 0})
        d["packets"] += 1; d["est_tokens"] += p["est_tokens"]; d["max_tokens"] = max(d["max_tokens"], p["est_tokens"])
    st = info["stats"]
    return {"boundary_utc": BOUNDARY_UTC, "base": BASE, "head": PINNED_HEAD, "commits": st["commits"],
            "committed_files_after_security_exclusion": sum(1 for r in records if r["source"] in ("committed", "both")
                                                            and r["class"] != "excluded"),
            "committed_files_total": st["committed_files"],
            "dirty_post_boundary_in_scope": sum(1 for r in records if r["source"] in ("dirty", "both", "untracked")),
            "dirty_entries_total": st["dirty_total"], "dirty_pre_boundary": st["dirty_pre"],
            "dirty_out_of_scope": st["dirty_out_of_scope"], "audit_artifacts_excluded": st["audit_artifacts"],
            "dirty_mtime_unknown": st["dirty_mtime_unknown"], "files_in_scope": len(records),
            "files_by_class": by_cls, "files_by_source": by_src, "coverage_units": len(units),
            "units_by_plan": plan, "packets": len(packets), "packets_by_domain": dom,
            "budget_bytes": meta["budget_bytes"], "human_only": len(info["human_only"]),
            "skipped_docs": sorted(docs), "skipped_other": sorted(other)}


# ---------------------------------------------------------------- other subcommands
def verify_snapshot(strict: bool = False) -> bool:
    snap = json.loads((REVIEW / "snapshot.json").read_text())
    ok = tree_manifest(Path(snap["tree_path"])) == snap["manifest"]
    if strict and not ok:
        die("worker tree no longer matches snapshot.json; rebuild with `prep.py build`")
    return ok


def cmd_render(pid: str) -> None:
    p = json.loads((REVIEW / "packets" / f"{pid}.json").read_text())
    title, notes = DOMAINS[p["domain"]]
    snap = json.loads((REVIEW / "snapshot.json").read_text())
    units = {}
    with open(WORK / "hunks.jsonl") as fh:
        for line in fh:
            u = json.loads(line)
            units.setdefault(u["file"], u)
    out = [f"# Packet {pid} (domain {p['domain']}: {title})",
           f"Snapshot: HEAD {PINNED_HEAD[:7]}, worker tree manifest {snap['manifest_sha256'][:12]}. "
           f"Diff base {BASE}. Lines marked `+` are added, `-` removed (shown with their old line number "
           f"`oN`), others context; numbers are post-image line numbers in the snapshot.",
           "", "## Domain notes", notes, "", "## Files"]
    for f in p["files"]:
        u = units[f]
        cm = ", ".join(u["commits"]) or "none"
        src = {"committed": "committed", "both": "committed + uncommitted edits", "dirty": "uncommitted (working tree)",
               "untracked": "new untracked file (whole file shown)"}[u["source"]]
        dl = " DELETED in post-image (cite old line numbers)" if u["status"] == "D" else ""
        out.append(f"- {f}: {src}; commits: {cm}{dl}")
    cands = {c["candidate_id"]: c for c in json.loads((WORK / "scanner_candidates.json").read_text())["candidates"]}
    if p["candidates"]:
        out += ["", "## Scanner candidates (mechanical pattern hits, unverified: confirm with source=both and "
                    "candidate_id, or ignore)"]
        out += [f"- {cid} [{cands[cid]['scanner']}] {cands[cid]['file']}:{cands[cid]['lines']}: "
                f"{cands[cid]['detail'][:300]}" for cid in p["candidates"] if cid in cands]
    facts = []
    if p["domain"] == "wp6" or any("test" in PurePosixPath(f).stem for f in p["files"]):
        ad = json.loads((WORK / "assertion_delta.json").read_text())
        for e in ad["files"]:
            if e["file"] in p["files"]:
                facts.append(json.dumps({"assertion_delta": e}, ensure_ascii=False)[:6000])
    if p["domain"] == "wp5":
        facts.append(json.dumps(json.loads((REVIEW / "facts" / "npc_assets.json").read_text()), sort_keys=True)[:6000])
    for mod in p.get("mods", []):
        fx = REVIEW / "facts" / f"{mod}.json"
        if fx.exists():
            facts.append("checker facts (provisional, scratch copy): " + fx.read_text()[:4000])
    if facts:
        out += ["", "## Known facts (deterministic, from scripts)"] + facts
    out += ["", "## Content", (REVIEW / p["content"]).read_text(), "",
            f"Return the JSON object for packet_id \"{pid}\"."]
    sys.stdout.write("\n".join(out) + "\n")


def cmd_resize(run_file: str, canary_file: str | None) -> None:
    """Two-point fit: first-turn prompt tokens = overhead + bytes / bytes_per_token."""
    pts = []
    rj = json.loads(Path(run_file).read_text())
    if rj.get("first_turn_prompt_tokens") and rj.get("prompt_bytes"):
        pts.append((rj["prompt_bytes"], rj["first_turn_prompt_tokens"]))
    if canary_file and Path(canary_file).exists():
        cj = json.loads(Path(canary_file).read_text())
        if cj.get("first_turn_prompt_tokens") and cj.get("prompt_bytes"):
            pts.append((cj["prompt_bytes"], cj["first_turn_prompt_tokens"]))
    if len(pts) < 2:
        die("resize needs first_turn_prompt_tokens + prompt_bytes in both the first run and canary.json")
    (b1, t1), (b2, t2) = pts[:2]
    if b1 == b2 or t1 == t2:
        die("degenerate points")
    tpb = (t1 - t2) / (b1 - b2)
    overhead = t1 - tpb * b1
    prm = params()
    prm.update(bytes_per_token=round(1 / tpb, 4), overhead_tokens=int(overhead))
    dump(REVIEW / "params.json", prm)
    print(f"resize: bytes/token={prm['bytes_per_token']} overhead={prm['overhead_tokens']} tokens")
    cmd_build(repack_only=True)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("cmd", nargs="?", default="build",
                    choices=["build", "verify-snapshot", "render", "pending", "is-complete", "resize", "build-tree"])
    ap.add_argument("arg", nargs="?")
    ap.add_argument("--from", dest="from_", help="resize: runs/<first>.json")
    ap.add_argument("--canary", default=str(REVIEW / "canary.json"))
    a = ap.parse_args()
    if a.cmd == "build":
        cmd_build()
    elif a.cmd == "verify-snapshot":
        ok = verify_snapshot()
        print("snapshot OK" if ok else "snapshot MISMATCH")
        sys.exit(0 if ok else 1)
    elif a.cmd == "render":
        cmd_render(a.arg)
    elif a.cmd == "pending":
        for pj in sorted((REVIEW / "packets").glob("*.json"),
                         key=lambda x: (DOMAIN_ORDER.index(x.stem.split("-")[0]), x.stem)):
            if not run_complete(pj.stem):
                print(pj.stem)
    elif a.cmd == "is-complete":
        sys.exit(0 if run_complete(a.arg) else 1)
    elif a.cmd == "resize":
        cmd_resize(a.from_, a.canary)
    elif a.cmd == "build-tree":
        if not a.arg:
            die("build-tree needs <dest>")
        print(json.dumps(build_tree(Path(a.arg).expanduser())))


if __name__ == "__main__":
    main()
