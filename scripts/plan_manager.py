#!/usr/bin/env python3
"""plan_manager.py - Deterministic Master Plan & Context Handoff Engine

Maintains master plan progress across context clears (/clear).
Provides token-efficient bootstrap for fresh Claude/Opus sessions.

Usage:
  python3 scripts/plan_manager.py resume             # Instant token-efficient bootstrap (<250 tokens)
  python3 scripts/plan_manager.py next               # Next phase prompt for subagents
  python3 scripts/plan_manager.py status             # Detailed phase checklist
  python3 scripts/plan_manager.py set <plan_path>    # Set active master plan
  python3 scripts/plan_manager.py mark-done <phase>  # Mark phase complete in plan
  python3 scripts/plan_manager.py handoff [prompt]   # Prepare .claude/session_handoff.md before /clear
"""

import argparse
import glob
import os
import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
ACTIVE_PLAN_FILE = REPO_ROOT / ".claude" / "active_plan.txt"
HANDOFF_FILE = REPO_ROOT / ".claude" / "session_handoff.md"
PREV_HANDOFF_FILE = REPO_ROOT / ".claude" / "session_handoff.prev.md"
HANDOFFS_DIR = REPO_ROOT / ".claude" / "handoffs"

PHASE_HEADER_RE = re.compile(r'^(?:#{2,3})\s+(?:(Phase\s+\d+|Chunk\s+[A-Za-z0-9]+|Step\s+\d+)[^:\n]*:?\s*(.*))$', re.MULTILINE)
CHECKBOX_RE = re.compile(r'^\s*-\s*\[([ xX/])\]\s*(.*)$', re.MULTILINE)
EXEC_LOG_RE = re.compile(r'^\s*-\s*(Phase\s+\d+|Chunk\s+[A-Za-z0-9]+|Step\s+\d+)[^:\n]*:\s*(.*)$', re.MULTILINE)

def get_git_info() -> dict:
    try:
        branch = subprocess.check_output(["git", "rev-parse", "--abbrev-ref", "HEAD"], cwd=REPO_ROOT, text=True).strip()
        status_out = subprocess.check_output(["git", "status", "--short"], cwd=REPO_ROOT, text=True).strip()
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

def discover_active_plan(explicit_path: str = "") -> Path | None:
    if explicit_path:
        p = Path(explicit_path)
        if p.exists():
            return p.resolve()
        if (REPO_ROOT / p).exists():
            return (REPO_ROOT / p).resolve()
        sys.stderr.write(f"Error: Specified plan file not found: {explicit_path}\n")
        sys.exit(2)

    if ACTIVE_PLAN_FILE.exists():
        p = Path(ACTIVE_PLAN_FILE.read_text().strip())
        if p.exists():
            return p
        if (REPO_ROOT / p).exists():
            return (REPO_ROOT / p).resolve()

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
    status_col_idx = None
    id_col_idx = 0
    name_col_idx = 1

    for line in lines:
        if not line.strip().startswith("|"):
            continue
        cells = [c.strip() for c in line.strip().split("|")[1:-1]]
        if not cells or all(c.startswith("-") for c in cells):
            continue

        # Header detection
        lower_cells = [c.lower() for c in cells]
        if "status" in lower_cells or "state" in lower_cells:
            status_col_idx = lower_cells.index("status" if "status" in lower_cells else "state")
            if "id" in lower_cells:
                id_col_idx = lower_cells.index("id")
            if "batch" in lower_cells or "task" in lower_cells or "name" in lower_cells:
                name_col_idx = lower_cells.index("batch" if "batch" in lower_cells else ("task" if "task" in lower_cells else "name"))
            continue

        # Data row
        if status_col_idx is not None and len(cells) > max(status_col_idx, id_col_idx, name_col_idx):
            ident = cells[id_col_idx]
            name = cells[name_col_idx]
            status = cells[status_col_idx].upper()
            if ident.lower() in ("id", "---", "--", "-") or not ident:
                continue
            is_done = any(k in status for k in ("DONE", "COMPLETED", "MERGED", "PASS"))
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
        ident = m.group(1).strip()
        exec_done.add(ident.lower())

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

    if json_mode:
        import json
        print(json.dumps({
            "plan_path": str(plan["path"].relative_to(REPO_ROOT)),
            "title": plan["title"],
            "branch": git_info["branch"],
            "head": git_info["head"],
            "progress": f"{len(completed)}/{total}",
            "current_phase": current["ident"] if current else "ALL_DONE",
            "current_title": current["name"] if current else "Complete",
            "dirty_files": git_info["dirty_count"]
        }, indent=2))
        return

    rel_path = plan["path"].relative_to(REPO_ROOT) if plan["path"].is_relative_to(REPO_ROOT) else plan["path"].name
    print(f"=== ACTIVE PLAN: {rel_path} ===")
    print(f"Goal: {plan['title']}")
    print(f"Git: {git_info['branch']} ({git_info['head']}) | {git_info['dirty_count']} uncommitted file(s)")
    print(f"Progress: {len(completed)}/{total} phases complete")

    if completed:
        done_str = ", ".join(p['ident'] for p in completed)
        print(f"Done: {done_str}")

    if current:
        print(f"\n>>> NEXT OBJECTIVE: {current['ident']} - {current['name']} <<<")
        for t in current['tasks'][:5]:
            print(f"  {t}")
        print("\nAction: Implement this objective, then run 'python3 scripts/plan_manager.py mark-done <phase>'.")
    else:
        print("\n[ALL PHASES COMPLETE] Ready for final verification & PR submission.")

def cmd_next(plan: dict):
    remaining = [p for p in plan["phases"] if not p["done"]]
    if not remaining:
        print("All phases in active plan are marked complete.")
        return
    cur = remaining[0]
    print(f"## {cur['ident']}: {cur['name']}\n")
    print(cur['body'])

def cmd_mark_done(plan: dict, target_str: str, commit_sha: str = "", notes: str = ""):
    plan_path = plan["path"]
    text = plan_path.read_text(encoding="utf-8")
    
    # 1. Try Markdown table update
    lines = text.split("\n")
    table_updated = False
    status_col_idx = None
    id_col_idx = 0
    commit_col_idx = None
    notes_col_idx = None

    for idx, line in enumerate(lines):
        if not line.strip().startswith("|"):
            continue
        cells = [c.strip() for c in line.strip().split("|")[1:-1]]
        if not cells or all(c.startswith("-") for c in cells):
            continue

        lower_cells = [c.lower() for c in cells]
        if "status" in lower_cells or "state" in lower_cells:
            status_col_idx = lower_cells.index("status" if "status" in lower_cells else "state")
            if "id" in lower_cells:
                id_col_idx = lower_cells.index("id")
            if "commit" in lower_cells:
                commit_col_idx = lower_cells.index("commit")
            if "next" in lower_cells or "notes" in lower_cells:
                notes_col_idx = lower_cells.index("next" if "next" in lower_cells else "notes")
            continue

        if status_col_idx is not None and len(cells) > max(status_col_idx, id_col_idx):
            ident = cells[id_col_idx].strip()
            if ident.lower() == target_str.strip().lower() or target_str.strip().lower() in ident.lower():
                raw_cells = line.split("|")
                raw_cells[status_col_idx + 1] = " DONE "
                if commit_sha and commit_col_idx is not None and len(raw_cells) > commit_col_idx + 1:
                    raw_cells[commit_col_idx + 1] = f" {commit_sha} "
                if notes and notes_col_idx is not None and len(raw_cells) > notes_col_idx + 1:
                    raw_cells[notes_col_idx + 1] = f" {notes} "
                lines[idx] = "|".join(raw_cells)
                table_updated = True
                break

    if table_updated:
        plan_path.write_text("\n".join(lines), encoding="utf-8")
        print(f"Marked table row '{target_str}' DONE in {plan_path.name}")
        return

    # 2. Try checkbox update
    pattern = re.compile(rf'(-\s*\[)[ /](\]\s*.*{re.escape(target_str)}.*)', re.IGNORECASE)
    if pattern.search(text):
        new_text = pattern.sub(r'\1x\2', text, count=1)
        plan_path.write_text(new_text, encoding="utf-8")
        print(f"Marked checkbox done in {plan_path.name}: {target_str}")
        return

    # 3. Append to Execution log if present
    if "## Execution log" in text:
        entry = f"- {target_str} (done): verified\n"
        new_text = text.replace("## Execution log\n", f"## Execution log\n\n{entry}")
        plan_path.write_text(new_text, encoding="utf-8")
        print(f"Appended to Execution log in {plan_path.name}: {target_str}")
    else:
        # Append checklist update at bottom
        with open(plan_path, "a", encoding="utf-8") as fp:
            fp.write(f"\n- [x] {target_str} (completed)\n")
        print(f"Appended completed item to {plan_path.name}: {target_str}")

def cmd_handoff(plan: dict, git_info: dict, user_direction: str = ""):
    HANDOFF_FILE.parent.mkdir(parents=True, exist_ok=True)
    if HANDOFF_FILE.exists():
        import shutil
        shutil.copy2(HANDOFF_FILE, PREV_HANDOFF_FILE)

    remaining = [p for p in plan["phases"] if not p["done"]]
    current = remaining[0] if remaining else None
    rel_path = plan["path"].relative_to(REPO_ROOT) if plan["path"].is_relative_to(REPO_ROOT) else plan["path"].name

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
    else:
        content.append("All phases complete. Run master tests and prepare PR.\n")

    content.append("## 1. Active State & Provenance")
    content.append(f"- **Master Plan:** `{rel_path}`")
    content.append(f"- **Branch:** `{git_info['branch']}` ({git_info['head']})")
    content.append(f"- **Last Landed Commit:**\n```\n{last_commit_stat}\n```")
    content.append(f"- **Dirty Files ({git_info['dirty_count']}):**")
    for f in git_info['dirty_preview']:
        content.append(f"  - `{f}`")
    content.append("")

    if current:
        content.append(f"## 2. Immediate Objective ({current['ident']})")
        content.append(f"**{current['name']}**\n")
        for t in current['tasks']:
            content.append(f"{t}")
        content.append("")

    content.append("## 3. Strict Context Budget & Read Policy")
    content.append("- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.")
    content.append("- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.")
    content.append("- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.")
    content.append("- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.\n")

    HANDOFF_FILE.write_text("\n".join(content), encoding="utf-8")
    print(f"Handoff written to {HANDOFF_FILE.relative_to(REPO_ROOT)}")
    print(f"Rotated previous handoff to {PREV_HANDOFF_FILE.relative_to(REPO_ROOT)}")

    # Plan-scoped handoff file (prevents multi-session clobbering in same worktree)
    HANDOFFS_DIR.mkdir(parents=True, exist_ok=True)
    plan_stem = plan["path"].stem
    plan_handoff_file = HANDOFFS_DIR / f"{plan_stem}.handoff.md"
    plan_handoff_file.write_text("\n".join(content), encoding="utf-8")
    print(f"Plan-scoped handoff written to {plan_handoff_file.relative_to(REPO_ROOT)}")
    print("Safe to run /clear or exit session.")

def main():
    parent_parser = argparse.ArgumentParser(add_help=False)
    parent_parser.add_argument("--json", action="store_true", help="Output in JSON format")
    parent_parser.add_argument("--plan", default="", help="Explicit master plan path (bypasses global active_plan.txt)")

    parser = argparse.ArgumentParser(description="Deterministic Master Plan & Context Handoff Engine", parents=[parent_parser])
    subparsers = parser.add_subparsers(dest="cmd")

    p_res = subparsers.add_parser("resume", parents=[parent_parser], help="Bootstrap from active plan")
    p_res.add_argument("plan_path", nargs="?", default="", help="Optional explicit master plan path")

    p_stat = subparsers.add_parser("status", parents=[parent_parser], help="Show full plan status")
    p_stat.add_argument("plan_path", nargs="?", default="", help="Optional explicit master plan path")

    p_nxt = subparsers.add_parser("next", parents=[parent_parser], help="Output next phase details")
    p_nxt.add_argument("plan_path", nargs="?", default="", help="Optional explicit master plan path")
    
    p_set = subparsers.add_parser("set", parents=[parent_parser], help="Set active plan")
    p_set.add_argument("plan_path", help="Path to master plan markdown file")

    p_mark = subparsers.add_parser("mark-done", parents=[parent_parser], help="Mark phase complete")
    p_mark.add_argument("phase", help="Phase or chunk name/substring to mark done")
    p_mark.add_argument("--commit", default="", help="Optional commit SHA to record in table")
    p_mark.add_argument("--notes", default="", help="Optional notes/verification result")

    p_hand = subparsers.add_parser("handoff", parents=[parent_parser], help="Prepare session handoff file")
    p_hand.add_argument("direction", nargs="?", default="", help="Optional user instructions for next phase")

    if len(sys.argv) == 1:
        sys.argv.append("resume")

    args = parser.parse_args()

    if args.cmd == "set":
        p = Path(args.plan_path).resolve()
        if not p.exists():
            sys.stderr.write(f"Error: Plan file not found: {p}\n")
            sys.exit(2)
        ACTIVE_PLAN_FILE.parent.mkdir(parents=True, exist_ok=True)
        ACTIVE_PLAN_FILE.write_text(str(p), encoding="utf-8")
        print(f"Active plan set to: {p.relative_to(REPO_ROOT)}")
        sys.exit(0)

    explicit_plan = getattr(args, "plan", "") or getattr(args, "plan_path", "")
    plan_path = discover_active_plan(explicit_path=explicit_plan)
    if not plan_path:
        sys.stderr.write("No active master plan found. Set one with 'python3 scripts/plan_manager.py set <path>'.\n")
        sys.exit(1)

    plan = parse_plan(plan_path)
    git_info = get_git_info()

    if args.cmd in ("resume", "status"):
        cmd_resume(plan, git_info, json_mode=args.json)
    elif args.cmd == "next":
        cmd_next(plan)
    elif args.cmd == "mark-done":
        cmd_mark_done(plan, args.phase, commit_sha=args.commit, notes=args.notes)
    elif args.cmd == "handoff":
        cmd_handoff(plan, git_info, user_direction=args.direction)

if __name__ == "__main__":
    main()
