#!/usr/bin/env python3
"""plan_manager.py - Deterministic Master Plan & Context Handoff Engine

Maintains master plan progress across context clears (/clear).
Provides token-efficient bootstrap for fresh Claude/Opus sessions.

Usage:
  python3 scripts/plan_manager.py resume             # Instant token-efficient bootstrap (<250 tokens)
  python3 scripts/plan_manager.py next               # Next phase prompt for subagents
  python3 scripts/plan_manager.py status             # Detailed phase checklist
  python3 scripts/plan_manager.py phase-text <plan|shorthand> <ID> [--max-tokens N]  # One phase body, token-capped (read-only)
  python3 scripts/plan_manager.py set <plan_path>    # Set active master plan
  python3 scripts/plan_manager.py mark-done <phase>  # Mark phase complete in plan
  python3 scripts/plan_manager.py handoff <doc|phase|--plan P> [notes]  # Write .claude/handoffs/<plan_stem>.handoff.md before /clear (refuses to guess the plan)
"""

import argparse
import os
import re
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
try:
    import master_ledger as ml
except ImportError:
    ml = None

REPO_ROOT = Path(__file__).resolve().parent.parent
ACTIVE_PLAN_FILE = Path(os.environ.get("PLAN_MANAGER_ACTIVE_FILE") or (REPO_ROOT / ".claude" / "active_plan.txt"))
HANDOFFS_DIR = Path(os.environ.get("PLAN_MANAGER_HANDOFFS_DIR") or (REPO_ROOT / ".claude" / "handoffs"))
WAVES_DIR = Path(os.environ.get("PLAN_MANAGER_WAVES_DIR") or (REPO_ROOT / ".claude" / "waves"))

PREFIX_LEGEND = {
    "T": "Testing & Contracts",
    "S": "Sync / Header Mirroring",
    "L": "Library DLLs & Packaging",
    "C": "Common Extraction",
    "M": "Meta / Planning Setup",
    "B": "Steam Address DB Batches",
    "R": "Research & Probes",
    "P": "Parity Verification",
    "U": "Unit / Compatibility Tooling",
}

PHASE_HEADER_RE = re.compile(r'^#{2,3}[ \t]+(Phase[ \t]+\d+|Chunk[ \t]+[A-Za-z0-9]+|Step[ \t]+\d+)[ \t:\u2014\u2013-]*(.*)$', re.MULTILINE)
CHECKBOX_RE = re.compile(r'^\s*-\s*\[([ xX/])\]\s*(.*)$', re.MULTILINE)
EXEC_LOG_RE = re.compile(r'^\s*-\s*(Phase\s+\d+|Chunk\s+[A-Za-z0-9]+|Step\s+\d+)[^:\n]*:\s*(.*)$', re.MULTILINE)

# Status cells are upper-cased before matching. INSTALLED counts as done: the remaining play test is a human task, not an agent one.
DONE_STATUS_RE = re.compile(r'(?<!NOT )\b(DONE|COMPLETED?|MERGED|PASS(?:ED)?|INSTALLED)\b')
NOT_DONE_LEAD_RE = re.compile(r'^[\W_]*(PARTIAL|PARTLY|PENDING|TODO|NOT)\b')
EXEC_DONE_TEXT_RE = re.compile(r'(?<!not )\b(done|complete[d]?|merged|passed?|landed|verified|installed|committed|shipped|finished)\b', re.IGNORECASE)
SEPARATOR_CELL_RE = re.compile(r':?-+:?')
# A table is a phase table only with a status/state column plus one of these identifier-like columns
# ('| Gate | State |' or '| Layer | Contents | State |' are not phases)
TABLE_ID_HEADERS = ("id", "#", "batch", "phase", "task", "name", "step", "chunk", "half", "item", "rule id", "wave")
PHASE_TOKEN_RE = re.compile(r'\b([A-Za-z]{1,3}[0-9]{1,3}[A-Za-z]?)\b')
# Phase labels as headers print them ('Phase 12', 'Step 3', 'Chunk A', '§12'): the bare key is what remains
PHASE_WORD_RE = re.compile(r'^(?:(?:PHASE|STEP|CHUNK)(?:\s+|(?=\d))|§\s*)')
# Leading doc number or identifier of a plan stem: '24_master' -> '24', '19e_x' -> '19e', 'PHASE7_COMMON' -> 'PHASE7'
WAVE_PREFIX_RE = re.compile(r'^([A-Za-z]{0,6}[0-9]+[A-Za-z]?)(?![A-Za-z0-9])')
MAX_WAVE_LINES = 5
HANDOFF_TITLE_RE = re.compile(r'^# Session Handoff & Plan State: (.+)$', re.MULTILINE)
DEAD_ENDS_HEADING = "## 5. Dead Ends & Verification (manual)"
DEAD_ENDS_STUB = f"{DEAD_ENDS_HEADING}\n_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_\n"

# Directories that hold plan docs; stem and doc-number lookups never walk outside them.
PLAN_SEARCH_GLOBS = ("patch_manager_mods/**/*.md", "research/**/*.md", ".claude/plans/*.md")


class AmbiguousPlan(Exception):
    """A shorthand matched several plan files and cannot be resolved safely."""


def atomic_write_text(path: Path, text: str) -> None:
    """Write via a temp file + os.replace so concurrent readers never see a truncated file."""
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_name(f"{path.name}.tmp.{os.getpid()}")
    tmp.write_text(text, encoding="utf-8")
    os.replace(tmp, path)


def read_pointer(strict: bool = False) -> Path | None:
    """Plan named by the shared active-plan pointer. None when the pointer is absent or empty.

    A pointer naming a missing file is an error when strict (exit 2) and ignored otherwise,
    so a renamed plan is never replaced by a silent mtime guess."""
    try:
        raw = ACTIVE_PLAN_FILE.read_text(encoding="utf-8").strip()
    except OSError:
        return None
    if not raw:
        return None
    p = Path(raw)
    if not p.is_absolute():
        p = REPO_ROOT / p
    if p.is_file():
        return p
    if strict:
        sys.stderr.write(f"Error: active plan pointer {ACTIVE_PLAN_FILE} names a missing file: {raw}\n"
                         f"Fix: python3 scripts/plan_manager.py set <plan path>, or pass a doc/phase/--plan.\n")
        sys.exit(2)
    return None


def is_done_status(status: str) -> bool:
    status = status.upper()
    if NOT_DONE_LEAD_RE.match(status):
        return False
    return bool(DONE_STATUS_RE.search(status))


def parse_table_header(cells: list[str]) -> dict | None:
    """Column map for a phase table (needs a status/state column plus an id-like column), else None."""
    lower = [c.lower() for c in cells]
    status_key = "status" if "status" in lower else ("state" if "state" in lower else None)
    if status_key is None or not any(k in lower for k in TABLE_ID_HEADERS):
        return None
    name_key = next((k for k in ("batch", "task", "name") if k in lower), None)
    return {
        "status": lower.index(status_key),
        "id": lower.index("id") if "id" in lower else 0,
        "name": lower.index(name_key) if name_key else 1,
        "commit": lower.index("commit") if "commit" in lower else None,
        "notes": lower.index("next" if "next" in lower else "notes") if ("next" in lower or "notes" in lower) else None,
    }


def is_separator_row(cells: list[str]) -> bool:
    return not cells or all(SEPARATOR_CELL_RE.fullmatch(c) or not c for c in cells)


def norm_ident(ident: str) -> str:
    return ident.strip("*_` ").upper()


def pick_target_phase(direction: str, phases: list[dict]) -> dict | None:
    """First not-done phase whose ident appears as a token in the direction text."""
    open_by_id: dict[str, dict] = {}
    for p in phases:
        if not p["done"]:
            open_by_id.setdefault(norm_ident(p.get("ident", "")), p)
    for m in PHASE_TOKEN_RE.finditer(direction):
        hit = open_by_id.get(m.group(1).upper())
        if hit:
            return hit
    return None

def get_git_info() -> dict:
    try:
        branch = subprocess.check_output(["git", "rev-parse", "--abbrev-ref", "HEAD"], cwd=REPO_ROOT, text=True).strip()
        # rstrip only: .strip() would eat the leading space of the first " M path" line
        status_out = subprocess.check_output(["git", "status", "--short"], cwd=REPO_ROOT, text=True).rstrip("\n")
        dirty_lines = [l for l in status_out.split("\n") if l.strip()] if status_out else []
        head_commit = subprocess.check_output(["git", "log", "-1", "--oneline"], cwd=REPO_ROOT, text=True).strip()
        return {
            "branch": branch,
            "dirty_count": len(dirty_lines),
            "dirty_preview": dirty_lines[:10],
            "head": head_commit
        }
    except Exception:
        return {"branch": "unknown", "dirty_count": 0, "dirty_preview": [], "head": "unknown"}

def _plan_files_with_stem(stem: str) -> list[Path]:
    """Plan docs named <stem>.md under the known plan directories (never handoff files or dot-dirs like .pytest_cache)."""
    found: dict[Path, None] = {}
    for pat in PLAN_SEARCH_GLOBS:
        for cand in sorted(REPO_ROOT.glob(pat)):
            if cand.stem == stem and cand.is_file() and not cand.name.endswith(".handoff.md"):
                found.setdefault(cand.resolve(), None)
    # Repo-root docs count only when plan-like, so 'README' or 'CLAUDE' never resolve to a plan
    for cand in sorted(REPO_ROOT.glob("*.md")):
        if cand.stem == stem and re.search(r'plan|master', cand.stem, re.IGNORECASE):
            found.setdefault(cand.resolve(), None)
    return list(found)


def _ambiguity(token: str, cands: list[Path], strict: bool) -> Path:
    """First candidate (deterministic, sorted) for lenient callers; AmbiguousPlan for CLI callers."""
    if len(cands) > 1 and strict:
        listing = "\n".join(f"  - {c.relative_to(REPO_ROOT) if c.is_relative_to(REPO_ROOT) else c}" for c in cands[:8])
        raise AmbiguousPlan(f"{token!r} matches {len(cands)} plan files; pass the repo-relative path instead:\n{listing}")
    return cands[0]


def resolve_plan_shorthand(token: str, strict: bool = False) -> tuple[Path | None, str]:
    """Map a path, stem, doc number ('25 §12', '24:T6', '19e') or phase ID ('T5d') to (plan path, leftover text).

    Unresolvable text returns (None, token). With strict, an ambiguous match raises AmbiguousPlan
    instead of silently picking the first file."""
    token = token.strip()
    if not token or "\n" in token:
        return None, token

    # 1. Exact path match (handoff files are never plans)
    if len(token) <= 255:
        try:
            p = Path(token)
            for cand in (p, REPO_ROOT / p):
                if cand.is_file() and not cand.name.endswith(".handoff.md"):
                    return cand.resolve(), ""
        except (OSError, ValueError):
            pass

    # 1b. Exact stem match inside the plan directories (e.g. 'PHASE7_COMMON_MASTER', 'swarm_library_architecture')
    stem_token = token[:-3] if token.endswith(".md") else token
    stem_cands = _plan_files_with_stem(stem_token)
    if stem_cands:
        return _ambiguity(token, stem_cands, strict).resolve(), ""
    # 'swarm_library_architecture M3b next': stem first, notes after
    first, _, notes = token.partition(" ")
    if notes.strip():
        first_cands = _plan_files_with_stem(first[:-3] if first.endswith(".md") else first)
        if first_cands:
            return _ambiguity(first, first_cands, strict).resolve(), notes.strip()

    # 2. Numbered doc shorthand: '25 §12', 'doc 25 §12', 'doc25:12', '19e', '24:T6', '25'
    m = re.match(r'^(?:doc\s*)?([0-9]{1,2}[a-z]?)(?:([:\s§—-]+)(.*))?$', token, re.IGNORECASE)
    if m:
        prefix = m.group(1).lower()
        sep = m.group(2) or ""
        rest = (m.group(3) or "").strip()
        # '2 hotbar fixes left' / '25 minutes of testing' is prose, not shorthand: a bare-space
        # separator is only shorthand when followed by a phase token or a number.
        prose = bool(rest) and not re.search(r'[:§—-]', sep) and not re.match(r'(?:[A-Za-z]{1,3}[0-9]|[0-9])', rest)
        if not prose:
            pattern = f"{int(prefix[:2]):02d}" if prefix[:2].isdigit() and len(prefix) <= 2 else prefix
            # Numbered docs first; root-level '*NN*.md' only as a last resort (it matches any digits in a name)
            groups = [
                sorted(REPO_ROOT.glob(f"patch_manager_mods/{pattern}_*.md")) + sorted(REPO_ROOT.glob(f"patch_manager_mods/{prefix}_*.md")),
                sorted(REPO_ROOT.glob(f"*{pattern}*.md")),
            ]
            for grp in groups:
                candidates = list(dict.fromkeys(c.resolve() for c in grp if not c.name.endswith(".handoff.md")))
                if not candidates:
                    continue
                if len(candidates) > 1 and rest:
                    rest_lower = rest.lower()
                    for cand in candidates:
                        if cand.name.lower() in rest_lower or cand.stem.lower() in rest_lower:
                            return cand, rest
                    rest_words = set(re.findall(r'[a-z0-9_]+', rest_lower))
                    scored = sorted(((len(set(c.stem.lower().split('_')) & rest_words), c) for c in candidates), key=lambda t: -t[0])
                    if scored[0][0] > 0 and (len(scored) == 1 or scored[0][0] > scored[1][0]):
                        return scored[0][1], rest
                return _ambiguity(token, candidates, strict), rest

    # 3. Phase ID shorthand (case-sensitive, so 'v2' or 'b1' in prose is not a phase): 'T4', 'T5d', 'M0', 'R10', 'HL4', 'CH1c', 'U3', 'QH1'
    m_phase = re.match(r'^([A-Za-z]{1,3}[0-9]{1,2}[a-z]?)(?:[:\s§—-]+(.*))?$', token)
    if m_phase:
        phase = m_phase.group(1)
        rest = (m_phase.group(2) or "").strip()
        pattern = re.compile(rf'(?:\|\s*\*{{0,2}}{re.escape(phase)}\*{{0,2}}\s*\||#+\s*(?:Phase\s*)?{re.escape(phase)}\b)')
        active = read_pointer()
        cands = ([active] if active else []) + sorted(REPO_ROOT.glob("research/**/*MASTER*.md")) + sorted(REPO_ROOT.glob("research/**/*plan*.md")) + [REPO_ROOT / "patch_manager_mods" / "24_master_plan_status.md"] + sorted(REPO_ROOT.glob("patch_manager_mods/*plan*.md")) + sorted(REPO_ROOT.glob("patch_manager_mods/*.md"))
        seen = set()
        for p_cand in cands:
            if p_cand in seen or not p_cand.is_file():
                continue
            seen.add(p_cand)
            txt = p_cand.read_text(encoding="utf-8", errors="ignore")
            if pattern.search(txt):
                detail = f"Phase {phase}" + (f" - {rest}" if rest else "")
                return p_cand.resolve(), detail

    return None, token

def discover_active_plan(explicit_path: str = "") -> Path | None:
    if explicit_path:
        try:
            p, _ = resolve_plan_shorthand(explicit_path, strict=True)
        except AmbiguousPlan as exc:
            sys.stderr.write(f"Error: {exc}\n")
            sys.exit(2)
        if p:
            return p
        sys.stderr.write(f"Error: Specified plan file or shorthand not found: {explicit_path}\n")
        sys.exit(2)

    pointed = read_pointer(strict=True)
    if pointed:
        return pointed

    candidates = []
    # Search root and patch_manager_mods
    for pattern in ["*plan*.md", "patch_manager_mods/*plan*.md"]:
        for f in REPO_ROOT.glob(pattern):
            if f.name not in ("session_handoff.md", "session_handoff.prev.md") and not f.name.startswith("."):
                candidates.append((f.stat().st_mtime, f))

    if not candidates:
        return None
    candidates.sort(key=lambda x: x[0], reverse=True)
    return candidates[0][1]

def parse_plan(plan_path: Path) -> dict:
    text = plan_path.read_text(encoding="utf-8", errors="ignore")
    title = plan_path.name
    for line in text.split("\n")[:10]:
        if line.startswith("# "):
            title = line[2:].strip()
            break

    phases = []

    # 1. Flexible Markdown Table Parser
    lines = text.split("\n")
    hdr = None  # column map of the phase table we are inside, else None

    for line in lines:
        if not line.strip().startswith("|"):
            hdr = None
            continue
        cells = [c.strip() for c in line.strip().split("|")[1:-1]]
        if is_separator_row(cells):
            continue

        # Header detection: only tables with a status column AND an id-like column are phase tables,
        # so a preface table like '| Gate | State |' cannot hide the real Phase headers below it.
        header = parse_table_header(cells)
        if header:
            hdr = header
            continue

        # Data row
        if hdr is not None and len(cells) > max(hdr["status"], hdr["id"], hdr["name"]):
            ident = cells[hdr["id"]]
            name = cells[hdr["name"]]
            status = cells[hdr["status"]].upper()
            if ident.lower() in ("id", "---", "--", "-") or not ident:
                continue
            is_done = is_done_status(status)
            phases.append({
                "ident": ident,
                "name": name,
                "done": is_done,
                "body": f"Status: {status} | Batch: {name}",
                "tasks": [f"Status: {status}", f"Task: {name}"]
            })

    if phases:
        return {
            "path": plan_path,
            "title": title,
            "phases": phases
        }

    # 2. Check for Header-based plan (## Chunk X / ## Phase X / ## Step X)
    matches = list(PHASE_HEADER_RE.finditer(text))
    exec_done = set()
    for m in EXEC_LOG_RE.finditer(text):
        # '- Phase 3: started, blocked' is a progress note, not a completion: require a done word
        if EXEC_DONE_TEXT_RE.search(m.group(2)):
            exec_done.add(m.group(1).strip().lower())

    for i, m in enumerate(matches):
        ident = m.group(1).strip()
        name = m.group(2).strip()
        start = m.end()
        end = matches[i+1].start() if i+1 < len(matches) else len(text)
        body = text[start:end].strip()

        is_done = False
        if ident.lower() in exec_done:
            is_done = True
        elif f"- [x] {name}" in body or f"- [X] {name}" in body:
            is_done = True

        tasks = []
        for line in body.split("\n"):
            line_s = line.strip()
            if line_s.startswith("- ") or line_s.startswith("1.") or line_s.startswith("2."):
                tasks.append(line_s)

        phases.append({
            "ident": ident,
            "name": name,
            "done": is_done,
            "body": body,
            "tasks": tasks[:8]
        })

    # 3. Fallback to Checkbox parsing (- [ ] / - [x])
    if not phases:
        for m in CHECKBOX_RE.finditer(text):
            is_done = m.group(1).lower() == "x"
            name = m.group(2).strip()
            phases.append({
                "ident": f"Task {len(phases)+1}",
                "name": name,
                "done": is_done,
                "body": name,
                "tasks": []
            })

    return {
        "path": plan_path,
        "title": title,
        "phases": phases
    }

def cmd_resume(plan: dict, git_info: dict, json_mode: bool = False):
    total = len(plan["phases"])
    completed = [p for p in plan["phases"] if p["done"]]
    remaining = [p for p in plan["phases"] if not p["done"]]
    current = remaining[0] if remaining else None

    plan_stem = plan["path"].stem
    scoped_handoff = handoff_file_for(plan["path"], HANDOFFS_DIR)
    handoff_direction = ""
    if scoped_handoff.is_file():
        try:
            h_txt = scoped_handoff.read_text(encoding="utf-8")
            m_sec0 = re.search(r'## 0\.\s*Next-Phase Direction\s*\n+(?!#)([^\n]+)', h_txt)
            if m_sec0:
                handoff_direction = m_sec0.group(1).strip()
        except Exception:
            pass

    if json_mode:
        import json
        upcoming = remaining[1:4]
        res_data = {
            "plan_path": str(plan["path"].relative_to(REPO_ROOT) if plan["path"].is_relative_to(REPO_ROOT) else plan["path"]),
            "title": plan["title"],
            "branch": git_info["branch"],
            "head": git_info["head"],
            "progress": f"{len(completed)}/{total}",
            "current_phase": current["ident"] if current else "ALL_DONE",
            "current_title": current["name"] if current else "Complete",
            "upcoming_phases": [{"ident": p["ident"], "name": p["name"]} for p in upcoming],
            "next_handoff": f"/plan-handoff {current['ident']}" if current else "",
            "dirty_files": git_info["dirty_count"]
        }
        if handoff_direction:
            res_data["handoff_direction"] = handoff_direction
            res_data["scoped_handoff"] = str(scoped_handoff.relative_to(REPO_ROOT) if scoped_handoff.is_relative_to(REPO_ROOT) else scoped_handoff)
        print(json.dumps(res_data, indent=2))
        return

    rel_path = plan["path"].relative_to(REPO_ROOT) if plan["path"].is_relative_to(REPO_ROOT) else plan["path"].name
    print(f"=== ACTIVE PLAN: {rel_path} ===")
    print(f"Goal: {plan['title']}")
    if handoff_direction:
        print(f"Handoff Direction: {handoff_direction}")
    print(f"Git: {git_info['branch']} ({git_info['head']}) | {git_info['dirty_count']} uncommitted file(s)")
    print(f"Progress: {len(completed)}/{total} phases complete")

    if completed:
        done_str = ", ".join(p['ident'] for p in completed)
        print(f"Done: {done_str}")

    if current:
        upcoming = remaining[1:4]
        ident_prefix = current['ident'][0].upper() if current['ident'] else ""
        category = PREFIX_LEGEND.get(ident_prefix, "")
        badge = f"[{category}] " if category else ""
        print(f"\n>>> NEXT OBJECTIVE: {badge}{current['ident']} — {current['name']} <<<")
        for t in current['tasks'][:5]:
            print(f"  {t}")

        if upcoming:
            print("\nUpcoming Batches:")
            for p in upcoming:
                p_prefix = p['ident'][0].upper() if p['ident'] else ""
                p_cat = PREFIX_LEGEND.get(p_prefix, "")
                p_badge = f"[{p_cat}] " if p_cat else ""
                print(f"  - {p_badge}{p['ident']} — {p['name']}")
        plan_stem = plan["path"].stem
        doc_prefix = plan_stem[:2] if (len(plan_stem) >= 2 and plan_stem[:2].isdigit()) else plan_stem
        clean_ident = current['ident'].strip('*_ ')
        scoped_ident = f"{doc_prefix}:{clean_ident}" if not clean_ident.lower().startswith(doc_prefix.lower()) else clean_ident

        print(f"\nNext Steps:")
        print(f"  - Delegate to Subagent: python3 scripts/plan_manager.py delegate \"{scoped_ident}\" (or 'python3 scripts/subagent_prompt.py \"{scoped_ident}\" --run auto')")
        print(f"  - Resume: python3 scripts/plan_manager.py resume \"{scoped_ident}\" (or 'resume \"{scoped_ident}\"')")
        print(f"  - Handoff / Plan Mode: /plan-handoff \"{scoped_ident}\" (or 'python3 scripts/plan_manager.py handoff \"{scoped_ident}\"')")
        print(f"  - Mark Complete: python3 scripts/plan_manager.py mark-done \"{scoped_ident}\" [--commit <sha>]")
    else:
        print("\n[ALL PHASES COMPLETE] Ready for final verification & PR submission.")

def cmd_legend():
    print("=== KOTOR II MASTER KEY & PLANNING LEGEND ===\n")
    print("Phase Prefixes:")
    for k, v in PREFIX_LEGEND.items():
        print(f"  {k:<3} : {v}")
    print("\nCommon Planning Documents:")
    print("  Doc 24 : Master Plan Status (Patches/Common, Test Coverage, Shared DLLs)")
    print("  Doc 25 : PC Level-Up Filter & Companion Level-Up")
    print("  Doc 20 : Testing Plan & Harness Architecture")
    print("  Doc 19 : KpLog Shared Logging Design & Adoption")
    print("  Doc 21 : Portrait Slots & Library DLL Split")
    print("  Doc 07 : Action Hotbar (formerly Force Power Hotbar)")
    print("  Doc 08 : Level Up Override & Shield Bar")
    print("  Doc 17 : Dialogue Letterbox & Word Wrap Fixes")
    print("  Doc 10 : Engine Research & Perf-Lab Optimization")
    print("\nFull Legend Guide: patch_manager_mods/MASTER_KEY.md\n")

def cmd_list_plans(json_mode: bool = False):
    cands = sorted(REPO_ROOT.glob("patch_manager_mods/*plan*.md")) + sorted(REPO_ROOT.glob("patch_manager_mods/*.md"))
    seen = set()
    plans = []
    for p in cands:
        if p in seen or not p.is_file() or p.name.startswith(".") or "feedback" in p.name:
            continue
        seen.add(p)
        try:
            pl = parse_plan(p)
            if pl["phases"]:
                done_cnt = sum(1 for ph in pl["phases"] if ph["done"])
                tot_cnt = len(pl["phases"])
                curr = next((ph for ph in pl["phases"] if not ph["done"]), None)
                plans.append({
                    "path": str(p.relative_to(REPO_ROOT)),
                    "stem": p.stem,
                    "title": pl["title"],
                    "done": done_cnt,
                    "total": tot_cnt,
                    "next": curr["ident"] if curr else "DONE",
                })
        except Exception:
            pass
    if json_mode:
        import json
        print(json.dumps(plans, indent=2))
        return
    print("=== KNOWN MASTER & SUBDOMAIN PLANS ===\n")
    for pl in plans:
        prog = f"[{pl['done']}/{pl['total']}]"
        print(f"  {pl['path']:<45} {prog:<8} next: {pl['next']:<8} ({pl['title']})")

def cmd_next(plan: dict, contract: str = ""):
    remaining = [p for p in plan["phases"] if not p["done"]]
    if not remaining:
        print("All phases in active plan are marked complete.")
        return
    cur = remaining[0]
    upcoming = remaining[1:4]
    print(f"## {cur['ident']}: {cur['name']}\n")
    print(cur['body'])
    if upcoming:
        print("\n### Next Batches in Pipeline:")
        for p in upcoming:
            p_prefix = p['ident'][0].upper() if p['ident'] else ""
            p_cat = PREFIX_LEGEND.get(p_prefix, "")
            p_badge = f"[{p_cat}] " if p_cat else ""
            print(f"- {p_badge}{p['ident']}: {p['name']}")
    print(f"\nSubagent Delegation (Autonomous):")
    print(f"  python3 scripts/plan_manager.py delegate {cur['ident']}")
    print(f"\nNext Handoff: /plan-handoff {cur['ident']} (or 'python3 scripts/plan_manager.py handoff {cur['ident']}')")
    if contract:
        sys.path.insert(0, str(REPO_ROOT / "scripts"))
        from agent_contract import template
        print("\n" + template(contract))

def phase_key(label: str) -> str:
    """Comparable phase label: 'Phase 12', 'step 12', '§12' and '12' all map to '12'; 'h1' and 'H1' both map to 'H1'."""
    return PHASE_WORD_RE.sub("", norm_ident(label))


def _leftover_phase_id(rest: str) -> str:
    """Phase label carried by a shorthand leftover: 'M3b next' -> 'M3b', 'Phase T5e - x' -> 'T5e'."""
    m = re.match(r"^(?:Phase\s+)?(\S+)", rest.strip(), re.IGNORECASE)
    return m.group(1) if m else ""


def cmd_phase_text(plan: dict, phase_id: str, max_tokens: int = 12000) -> int:
    """Print one phase's body, cut at a whole line once the running ceil(bytes/4) total would pass max_tokens. Returns exit code."""
    key = phase_key(phase_id)
    phase = next((p for p in plan["phases"] if key and phase_key(p.get("ident", "")) == key), None)
    if phase is None:
        known = ", ".join(p["ident"] for p in plan["phases"][:8]) or "none parsed"
        sys.stderr.write(f"Error: phase {phase_id!r} not found in {plan_rel(plan['path'])}. Known: {known}\n")
        return 1
    lines = phase["body"].split("\n")
    out: list[str] = []
    used = 0
    for i, line in enumerate(lines):
        cost = -(-len(line.encode("utf-8")) // 4)  # ceil(bytes / 4)
        if used + cost > max_tokens:
            out.append(f"[truncated: {len(lines) - i} more lines]")
            break
        out.append(line)
        used += cost
    print("\n".join(out))
    return 0

def cmd_delegate(plan: dict, target_phase: str = "", mode: str = "auto", dry_run: bool = False):
    remaining = [p for p in plan["phases"] if not p["done"]]
    if not target_phase:
        if not remaining:
            print("All phases in active plan are complete. Nothing to delegate.")
            return 0
        target_phase = remaining[0]["ident"]

    sys.path.insert(0, str(REPO_ROOT / "scripts"))
    import subagent_prompt as sap
    import agent_contract as ac

    info = sap.extract_phase_info(plan["path"], target_phase)
    if len(info) >= 5:
        title, desc, files, verif_cmds, mod_scope = info[:5]
    else:
        title, desc, files, verif_cmds = info[:4]
        mod_scope = None
    prompt = sap.build_self_contained_prompt(target_phase, title, desc, files, verif_cmds, mod_scope=mod_scope, role="code")

    if dry_run:
        print(f"=== DRY-RUN DELEGATION PROMPT FOR PHASE {target_phase} ===")
        print(prompt)
        return 0

    print(f"=== DELEGATING PHASE {target_phase} VIA oc-flow ({mode}) ===")
    snap_file = REPO_ROOT / ".claude" / "contract_baseline.tsv"
    snap_count = ac.write_snapshot(REPO_ROOT, snap_file)
    print(f"Recorded pre-delegation git baseline ({snap_count} dirty paths).")

    flow_cmd = ["oc-flow", mode, prompt]
    res = subprocess.run(flow_cmd, cwd=str(REPO_ROOT))
    if res.returncode != 0:
        # Check if plan artifact was still generated despite non-zero exit (e.g. post-generation logging race)
        subagent_runs = REPO_ROOT / ".claude" / "subagent_runs"
        recent_plans = sorted(subagent_runs.glob("*_plan.md"), key=lambda p: p.stat().st_mtime, reverse=True) if subagent_runs.exists() else []
        if mode == "start" and recent_plans and (time.time() - recent_plans[0].stat().st_mtime < 120):
            print(f"\n[WARN] oc-flow exited with code {res.returncode}, but recovered generated plan: {recent_plans[0].name}")
        else:
            print(f"Error: oc-flow exited with code {res.returncode}")
            return res.returncode

    if mode == "start":
        print(f"\n[PLAN GENERATED] Plan mode complete for phase {target_phase}.")
        print("Orchestrator Actions:")
        print("  1. Review plan values (offsets, textures, types) against query_ghidra.py.")
        print("  2. If critique needed: oc-flow revise <SESSION_ID> \"<feedback>\"")
        print("  3. If sound: oc-flow fork-build <SESSION_ID>")
        return 0

    print("\n=== RUNNING POST-EXECUTION DIFF & BUILD GATE ===")
    diff_chk = subprocess.run(["python3", "scripts/agent_diff_check.py"], cwd=str(REPO_ROOT))
    if diff_chk.returncode == 0:
        print(f"\n[PASS] Subagent successfully completed phase {target_phase}.")
        cmd_mark_done(plan, target_phase, notes="Completed autonomously via oc-flow auto")
        return 0
    else:
        print(f"\n[FAIL] Post-execution verification failed for phase {target_phase}. Orchestrator intervention required.")
        return 1

def split_phase_arg(arg: str) -> str:
    """'24:T6' / 'doc24 T6' -> 'T6'. Plain phase IDs and free text come back unchanged."""
    m = re.match(r'^(?:doc\s*)?[0-9]{1,2}[a-z]?\s*[:\u00a7]\s*(\S.*)$', arg.strip(), re.IGNORECASE)
    return m.group(1).strip() if m else arg.strip()


def cmd_mark_done(plan: dict, target_str: str, commit_sha: str = "", notes: str = "", state: str = "DONE") -> bool:
    """Record a phase state in the plan. Returns False (plan untouched) when nothing matched."""
    plan_path = plan["path"]
    text = plan_path.read_text(encoding="utf-8")
    state_str = state.strip().upper() if state else "DONE"
    target_str = split_phase_arg(target_str)
    if not target_str.strip("*_ "):
        print(f"Error: phase argument {target_str!r} has no usable identifier; plan not modified.")
        return False

    # 1. Try Markdown table update: exact ID match first, else a unique substring match ('T6' never hits both T6 and T6a)
    lines = text.split("\n")
    clean_target = target_str.strip("*_ ").lower()
    rows = []  # (line index, ident, column map)
    hdr = None
    for idx, line in enumerate(lines):
        if not line.strip().startswith("|"):
            hdr = None
            continue
        cells = [c.strip() for c in line.strip().split("|")[1:-1]]
        if is_separator_row(cells):
            continue
        header = parse_table_header(cells)
        if header:
            hdr = header
            continue
        if hdr is not None and len(cells) > max(hdr["status"], hdr["id"]):
            rows.append((idx, cells[hdr["id"]].strip("*_ "), hdr))

    exact = [r for r in rows if r[1].lower() == clean_target]
    fuzzy = [r for r in rows if clean_target in r[1].lower()]
    hits = exact[:1] or (fuzzy if len(fuzzy) == 1 else [])
    if not hits and len(fuzzy) > 1:
        print(f"Error: {target_str!r} matches {len(fuzzy)} rows ({', '.join(r[1] for r in fuzzy[:6])}); pass the exact ID. Plan not modified.")
        return False

    def _log_done():
        if ml:
            ml.log_event("PHASE_DONE", {
                "plan": str(plan_path),
                "phase": target_str,
                "state": state_str,
                "commit": commit_sha,
                "notes": notes,
            })

    if hits:
        idx, _ident, h = hits[0]
        raw_cells = lines[idx].split("|")
        raw_cells[h["status"] + 1] = f" {state_str} "
        if commit_sha and h["commit"] is not None and len(raw_cells) > h["commit"] + 1:
            raw_cells[h["commit"] + 1] = f" {commit_sha.strip()} "
        if notes and h["notes"] is not None and len(raw_cells) > h["notes"] + 1:
            raw_cells[h["notes"] + 1] = f" {notes.strip()} "
        lines[idx] = "|".join(raw_cells)
        plan_path.write_text("\n".join(lines), encoding="utf-8")
        print(f"Marked table row '{target_str}' {state_str} in {plan_path.name}")
        _log_done()
        return True

    if rows:
        # The plan has a phase table but no row matches: appending a stray checkbox would corrupt it
        print(f"Error: no phase row matches {target_str!r} in {plan_path.name} (rows: {', '.join(r[1] for r in rows[:8])}). Plan not modified.")
        return False

    # 2. Try checkbox update
    pattern = re.compile(rf'(-\s*\[)[ /](\]\s*.*{re.escape(target_str)}.*)', re.IGNORECASE)
    if pattern.search(text):
        new_text = pattern.sub(r'\1x\2', text, count=1)
        plan_path.write_text(new_text, encoding="utf-8")
        print(f"Marked checkbox done in {plan_path.name}: {target_str}")
        _log_done()
        return True

    # 3. Append to Execution log if present
    if "## Execution log" in text:
        entry = f"- {target_str} (done): verified\n"
        new_text = text.replace("## Execution log\n", f"## Execution log\n\n{entry}")
        plan_path.write_text(new_text, encoding="utf-8")
        print(f"Appended to Execution log in {plan_path.name}: {target_str}")
        _log_done()
    else:
        # Append checklist update at bottom
        with open(plan_path, "a", encoding="utf-8") as fp:
            fp.write(f"\n- [x] {target_str} (completed)\n")
        print(f"Appended completed item to {plan_path.name}: {target_str}")
        _log_done()
    return True

def plan_rel(path: Path) -> Path:
    """Repo-relative plan path (the identity written on line 1 of its handoff); the full path for plans outside the repo."""
    return path.relative_to(REPO_ROOT) if path.is_relative_to(REPO_ROOT) else path


def handoff_root(handoff_dir: Path | str | None) -> Path:
    """Directory holding handoffs/ and active_plan.txt for one run. A relative --handoff-dir resolves against the repo, not the cwd."""
    if not handoff_dir:
        return REPO_ROOT / ".claude"
    p = Path(handoff_dir)
    return (p if p.is_absolute() else REPO_ROOT / p).resolve()


def handoffs_dir_for(handoff_dir: Path | str | None) -> Path:
    """Writer and reader share this: --handoff-dir X means X/handoffs, otherwise HANDOFFS_DIR (honors PLAN_MANAGER_HANDOFFS_DIR)."""
    return handoff_root(handoff_dir) / "handoffs" if handoff_dir else HANDOFFS_DIR


def handoff_file_for(plan_path: Path, handoffs_dir: Path) -> Path:
    """<stem>.handoff.md, unless that file already belongs to a different plan with the same stem
    (two PHASE4_PLAN.md files); then <parent-dir>__<stem>.handoff.md."""
    base = handoffs_dir / f"{plan_path.stem}.handoff.md"
    if base.is_file():
        try:
            m = HANDOFF_TITLE_RE.search(base.read_text(encoding="utf-8")[:500])
        except OSError:
            m = None
        if m and m.group(1).strip() != str(plan_rel(plan_path)):
            return handoffs_dir / f"{plan_path.parent.name}__{plan_path.stem}.handoff.md"
    return base


def wave_dirs_for(plan_path: Path) -> list[Path]:
    """WAVES_DIR/*-<doc prefix>-* dirs for this plan (sorted, at most MAX_WAVE_LINES); none when the stem has no leading number."""
    m = WAVE_PREFIX_RE.match(plan_path.stem)
    if not m or not WAVES_DIR.is_dir():
        return []
    return sorted(d for d in WAVES_DIR.glob(f"*-{m.group(1)}-*") if d.is_dir())[:MAX_WAVE_LINES]


def cmd_handoff(plan: dict, git_info: dict, user_direction: str = "", handoff_dir: Path | str | None = None, dry_run: bool = False) -> Path:
    """Write (or with dry_run, print) the plan-scoped handoff; returns its path."""
    handoffs_dir = handoffs_dir_for(handoff_dir)

    remaining = [p for p in plan["phases"] if not p["done"]]
    current = remaining[0] if remaining else None
    if user_direction:
        target = pick_target_phase(user_direction, plan["phases"])
        if target:
            current = target
            remaining = [target] + [x for x in remaining if x is not target]
    rel_path = plan_rel(plan["path"])

    # Pull last commit summary
    try:
        last_commit_stat = subprocess.check_output(
            ["git", "log", "-1", "--stat", "--oneline"], cwd=REPO_ROOT, text=True
        ).strip()
    except Exception:
        last_commit_stat = git_info["head"]

    content = []
    content.append(f"# Session Handoff & Plan State: {rel_path}\n")
    content.append("## 0. Next-Phase Direction")
    if user_direction:
        content.append(f"{user_direction}\n")
    elif current:
        content.append(f"Execute {current['ident']}: {current['name']}\n")
    elif not plan["phases"]:
        content.append(f"No phases parsed from {rel_path}; check its header levels (## Phase N / table with Id + Status columns).\n")
    else:
        content.append("All phases complete. Run master tests and prepare PR.\n")

    content.append("## 1. Active State & Provenance")
    content.append(f"- **Master Plan:** `{rel_path}`")
    for wd in wave_dirs_for(plan["path"]):
        wrel = wd.relative_to(REPO_ROOT) if wd.is_relative_to(REPO_ROOT) else wd
        content.append(f"- **Waves:** {wrel}/")
    content.append(f"- **Branch:** `{git_info['branch']}` ({git_info['head']})")
    content.append(f"- **Generated:** {time.strftime('%Y-%m-%d %H:%M:%S')} in `{REPO_ROOT}`")
    content.append(f"- **Last Landed Commit:**\n```\n{last_commit_stat}\n```")
    content.append(f"- **Dirty Files ({git_info['dirty_count']}):**")
    for f in git_info['dirty_preview']:
        content.append(f"  - `{f}`")
    if git_info['dirty_count'] > len(git_info['dirty_preview']):
        content.append(f"  - ... +{git_info['dirty_count'] - len(git_info['dirty_preview'])} more (run `git status --short`)")
    content.append("")

    if current:
        content.append(f"## 2. Immediate Objective ({current['ident']})")
        content.append(f"**{current['name']}**\n")
        for t in current['tasks']:
            content.append(f"{t}")
        content.append("")

        if len(remaining) > 1:
            content.append("## 2b. Upcoming Batches in Pipeline")
            for p in remaining[1:4]:
                p_prefix = p['ident'][0].upper() if p['ident'] else ""
                p_cat = PREFIX_LEGEND.get(p_prefix, "")
                p_badge = f"[{p_cat}] " if p_cat else ""
                content.append(f"- **{p['ident']}** {p_badge}: {p['name']}")
            content.append("")

    content.append("## 3. Strict Context Budget & Read Policy")
    # The repo-relative path is unambiguous even when two plans share a stem
    content.append(f"- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume {rel_path}`.\n")

    if current:
        content.append("## 4. Subagent Delegation Direction")
        content.append("- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.")
        content.append(f"- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate {current['ident']}` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.\n")

    # Plan-scoped handoff file (prevents multi-session clobbering in same worktree)
    plan_handoff_file = handoff_file_for(plan["path"], handoffs_dir)
    rel_plan_h = plan_handoff_file.relative_to(REPO_ROOT) if plan_handoff_file.is_relative_to(REPO_ROOT) else plan_handoff_file

    # Dead Ends / Verification Commands are appended by hand after generation: carry them across rewrites
    carried = DEAD_ENDS_STUB
    if plan_handoff_file.is_file():
        try:
            old = plan_handoff_file.read_text(encoding="utf-8")
            if DEAD_ENDS_HEADING in old:
                carried = old[old.index(DEAD_ENDS_HEADING):].rstrip("\n") + "\n"
        except OSError:
            pass
    text = "\n".join(content) + "\n" + carried

    if dry_run:
        print(f"DRY RUN: would write {rel_plan_h} for plan {rel_path}\n")
        print(text)
        return plan_handoff_file

    if ml:
        ml.log_event("SESSION_HANDOFF", {
            "plan": str(plan["path"]) if plan and "path" in plan else "",
            "notes": user_direction,
        })

    atomic_write_text(plan_handoff_file, text)
    # Naming the plan here lets the caller spot a wrong-plan write straight away
    print(f"Plan-scoped handoff written to {rel_plan_h} for plan {rel_path}")
    print("Safe to run /clear or exit session.")
    return plan_handoff_file

def main():
    parent_parser = argparse.ArgumentParser(add_help=False)
    parent_parser.add_argument("--json", action="store_true", default=argparse.SUPPRESS, help="Output in JSON format")
    parent_parser.add_argument("--plan", default=argparse.SUPPRESS, help="Explicit master plan path (bypasses global active_plan.txt)")

    parser = argparse.ArgumentParser(description="Deterministic Master Plan & Context Handoff Engine", parents=[parent_parser])
    subparsers = parser.add_subparsers(dest="cmd")

    p_res = subparsers.add_parser("resume", parents=[parent_parser], help="Bootstrap from active plan")
    p_res.add_argument("plan_path", nargs="?", default="", help="Optional explicit master plan path")

    p_stat = subparsers.add_parser("status", parents=[parent_parser], help="Show full plan status")
    p_stat.add_argument("plan_path", nargs="?", default="", help="Optional explicit master plan path")

    p_nxt = subparsers.add_parser("next", parents=[parent_parser], help="Output next phase details")
    p_nxt.add_argument("plan_path", nargs="?", default="", help="Optional explicit master plan path")
    p_nxt.add_argument("--contract", choices=["worker", "recon"], default="", help="Append subagent contract block (scripts/agent_contract.py)")
    
    p_ptxt = subparsers.add_parser("phase-text", parents=[parent_parser], help="Print one phase's body, capped to a token budget (read-only)")
    p_ptxt.add_argument("plan_path", nargs="?", default="", help="Master plan path or shorthand (e.g. 24:H1, 25 §12)")
    p_ptxt.add_argument("phase_id", nargs="?", default="", help="Phase ID (e.g. H1, T5e, 'Phase 3'); optional when the shorthand carries it")
    p_ptxt.add_argument("--max-tokens", type=int, default=12000, help="Token budget: ceil(bytes/4) per line (default 12000)")

    p_set = subparsers.add_parser("set", parents=[parent_parser], help="Set active plan")
    p_set.add_argument("plan_path", help="Path to master plan markdown file")

    p_mark = subparsers.add_parser("mark-done", parents=[parent_parser], help="Mark phase complete or update state")
    p_mark.add_argument("phase", help="Phase or chunk name/substring to mark done")
    p_mark.add_argument("--state", default="DONE", help="Target status to record in table (e.g. DONE, PARTIAL, BUILT, STAGED)")
    p_mark.add_argument("--commit", default="", help="Optional commit SHA to record in table")
    p_mark.add_argument("--notes", default="", help="Optional notes/verification result")
    p_mark.add_argument("--handoff-dir", default="", help="Optional directory to write handoff files (default .claude)")
    p_mark.add_argument("--no-handoff", action="store_true", help="Skip automatic handoff generation")

    p_del = subparsers.add_parser("delegate", parents=[parent_parser], help="Autonomously delegate phase to subagent via oc-flow")
    p_del.add_argument("phase", nargs="?", default="", help="Target phase ID (defaults to current active phase)")
    p_del.add_argument("--mode", choices=["start", "auto"], default="start", help="Delegation mode: start (plan-mode for orchestrator review, default) or auto (unsupervised end-to-end)")
    p_del.add_argument("--dry-run", action="store_true", help="Print synthesized prompt without launching oc-flow")

    subparsers.add_parser("legend", help="Display the master key and phase prefix legend")
    subparsers.add_parser("key", help="Display the master key and phase prefix legend")
    subparsers.add_parser("list", parents=[parent_parser], help="List all known master and subdomain plans")
    subparsers.add_parser("plans", parents=[parent_parser], help="List all known master and subdomain plans")

    p_hand = subparsers.add_parser("handoff", parents=[parent_parser], help="Prepare session handoff file")
    p_hand.add_argument("direction", nargs="?", default="", help="Optional user instructions for next phase")
    p_hand.add_argument("--handoff-dir", default="", help="Optional directory to write handoff files (default .claude)")
    p_hand.add_argument("--dry-run", action="store_true", help="Print the target path and handoff text without writing anything (pointer included)")

    if len(sys.argv) == 1:
        sys.argv.append("resume")

    args = parser.parse_args()

    if args.cmd in ("legend", "key"):
        cmd_legend()
        sys.exit(0)

    if args.cmd in ("list", "plans"):
        cmd_list_plans(json_mode=getattr(args, "json", False))
        sys.exit(0)

    if args.cmd == "set":
        p = Path(args.plan_path).resolve()
        if not p.exists():
            sys.stderr.write(f"Error: Plan file not found: {p}\n")
            sys.exit(2)
        atomic_write_text(ACTIVE_PLAN_FILE, str(p))
        print(f"Active plan set to: {p.relative_to(REPO_ROOT) if p.is_relative_to(REPO_ROOT) else p}")
        sys.exit(0)

    explicit_plan = getattr(args, "plan", "") or getattr(args, "plan_path", "")

    def resolve_or_die(token: str) -> tuple[Path | None, str]:
        try:
            return resolve_plan_shorthand(token, strict=True)
        except AmbiguousPlan as exc:
            sys.stderr.write(f"Error: {exc}\n")
            sys.exit(2)

    # Auto-resolve plan from a phase/shorthand argument when no explicit plan was given
    for cmd_name, attr in (("mark-done", "phase"), ("next", "plan_path"), ("delegate", "phase")):
        if args.cmd == cmd_name and not explicit_plan and getattr(args, attr, ""):
            resolved_p, _ = resolve_or_die(getattr(args, attr))
            if resolved_p:
                explicit_plan = str(resolved_p)

    # handoff never guesses: the plan comes from --plan or from the direction itself. The shared
    # active_plan.txt is rewritten by every session, so falling back to it wrote handoffs to the wrong plan.
    handoff_rest = None
    if args.cmd == "handoff" and not explicit_plan:
        direction = args.direction.strip()
        resolved_p, handoff_rest = resolve_or_die(direction) if direction else (None, "")
        if not resolved_p:
            sys.stderr.write(
                "Error: handoff needs a plan. Pass a doc/phase shorthand first ('24:T6 <notes>', '25 §12', 'T5e'), "
                "a plan path, or --plan <path>.\n"
                "Free text alone is refused: .claude/active_plan.txt is shared between sessions, so guessing from it targets another session's plan.\n")
            sys.exit(2)
        explicit_plan = str(resolved_p)

    # phase-text resolves its plan like next but never falls back to the shared pointer. The phase ID is the
    # second positional, or the shorthand leftover when only one positional is given ('24:H1').
    phase_text_id = ""
    if args.cmd == "phase-text":
        if args.max_tokens < 1:
            sys.stderr.write("Error: --max-tokens must be 1 or more.\n")
            sys.exit(2)
        phase_text_id = args.phase_id
        if getattr(args, "plan", ""):
            # --plan given: the lone positional is the phase ID
            phase_text_id = phase_text_id or args.plan_path
        else:
            if not args.plan_path:
                sys.stderr.write("Error: phase-text needs a plan path or shorthand, e.g. 'phase-text 24:H1'.\n")
                sys.exit(2)
            resolved_p, phase_rest = resolve_or_die(args.plan_path)
            if not resolved_p:
                sys.stderr.write(f"Error: plan or shorthand not found: {args.plan_path}\n")
                sys.exit(2)
            explicit_plan = str(resolved_p)
            phase_text_id = phase_text_id or _leftover_phase_id(phase_rest)
        if not phase_text_id:
            sys.stderr.write("Error: phase-text needs a phase ID, e.g. 'phase-text 24 H1' or 'phase-text 24:H1'.\n")
            sys.exit(2)

    plan_path = discover_active_plan(explicit_path=explicit_plan)
    if not plan_path:
        sys.stderr.write("No active master plan found. Set one with 'python3 scripts/plan_manager.py set <path>'.\n")
        sys.exit(1)

    def pointer_target(h_dir) -> Path:
        return (handoff_root(h_dir) / "active_plan.txt") if h_dir else ACTIVE_PLAN_FILE

    plan = parse_plan(plan_path)
    git_info = get_git_info()

    if args.cmd in ("resume", "status"):
        cmd_resume(plan, git_info, json_mode=getattr(args, "json", False))
    elif args.cmd == "next":
        cmd_next(plan, contract=getattr(args, "contract", ""))
    elif args.cmd == "phase-text":
        sys.exit(cmd_phase_text(plan, phase_text_id, max_tokens=args.max_tokens))
    elif args.cmd == "delegate":
        sys.exit(cmd_delegate(plan, target_phase=getattr(args, "phase", ""), mode=args.mode, dry_run=args.dry_run))
    elif args.cmd == "mark-done":
        if not cmd_mark_done(plan, args.phase, commit_sha=args.commit, notes=args.notes, state=args.state):
            sys.exit(1)  # nothing matched: no handoff, no Saved State banner for work that was not recorded

        # Automatically update handoff and output Saved State block
        if not getattr(args, "no_handoff", False):
            updated_plan = parse_plan(plan["path"])
            remaining = [p for p in updated_plan["phases"] if not p["done"]]
            next_phase = remaining[0] if remaining else None
            next_ident = next_phase["ident"] if next_phase else ""
            user_dir = f"Execute {next_phase['ident']}: {next_phase['name']}" if next_phase else "All phases complete"
            h_dir = getattr(args, "handoff_dir", "") or None
            plan_handoff_file = cmd_handoff(updated_plan, git_info, user_direction=user_dir, handoff_dir=h_dir)
            # The pointer follows the plan whose handoff was just written (--no-handoff leaves it alone)
            atomic_write_text(pointer_target(h_dir), str(updated_plan["path"]))

            plan_stem = updated_plan["path"].stem
            rel_plan_h = plan_handoff_file.relative_to(REPO_ROOT) if plan_handoff_file.is_relative_to(REPO_ROOT) else plan_handoff_file

            print("\n=== SAVED STATE (Safe to /clear) ===")
            print(f"  - Plan: {updated_plan['path'].name}")
            commit_str = f" @ {args.commit}" if args.commit else ""
            print(f"  - Completed: {split_phase_arg(args.phase)} ({args.state}){commit_str}")
            print(f"  - Handoff: {rel_plan_h}")
            if next_phase:
                doc_prefix = plan_stem[:2] if (len(plan_stem) >= 2 and plan_stem[:2].isdigit()) else plan_stem
                clean_next = next_ident.strip('*_ ')
                scoped_next = f"{doc_prefix}:{clean_next}" if not clean_next.lower().startswith(doc_prefix.lower()) else clean_next
                print(f"  - Next Phase: {next_phase['ident']} — {next_phase['name']}")
                print(f"  - Resume: python3 scripts/plan_manager.py resume \"{scoped_next}\" (or 'resume \"{scoped_next}\"')")
            else:
                print("  - Next Phase: None (all phases complete)")
    elif args.cmd == "handoff":
        user_dir = args.direction.strip()
        if handoff_rest is not None:
            # The plan was resolved from the direction itself; keep only the leftover text as the direction
            rest = handoff_rest.lstrip("— -").strip()
            if rest.isdigit() or rest.startswith("§"):
                user_dir = f"Execute §{rest.lstrip('§')} in {plan_path.name}"
            elif rest.startswith("Phase "):
                user_dir = f"Execute {rest[len('Phase '):]} ({plan_path.name})"
            elif rest:
                user_dir = rest
            else:
                user_dir = f"Continue {plan_path.name}"
        h_dir = getattr(args, "handoff_dir", "") or None
        cmd_handoff(plan, git_info, user_direction=user_dir, handoff_dir=h_dir, dry_run=args.dry_run)
        if not args.dry_run:
            atomic_write_text(pointer_target(h_dir), str(plan["path"]))

if __name__ == "__main__":
    main()
