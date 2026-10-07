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

sys.path.insert(0, str(Path(__file__).resolve().parent))
try:
    import master_ledger as ml
except ImportError:
    ml = None

REPO_ROOT = Path(__file__).resolve().parent.parent
ACTIVE_PLAN_FILE = REPO_ROOT / ".claude" / "active_plan.txt"
HANDOFF_FILE = REPO_ROOT / ".claude" / "session_handoff.md"
PREV_HANDOFF_FILE = REPO_ROOT / ".claude" / "session_handoff.prev.md"
HANDOFFS_DIR = REPO_ROOT / ".claude" / "handoffs"

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

def resolve_plan_shorthand(token: str) -> tuple[Path | None, str]:
    token = token.strip()
    if not token or "\n" in token:
        return None, token
    
    # 1. Exact path match
    if len(token) <= 255:
        try:
            p = Path(token)
            if p.exists() and p.is_file():
                return p.resolve(), ""
            if (REPO_ROOT / p).exists() and (REPO_ROOT / p).is_file():
                return (REPO_ROOT / p).resolve(), ""
        except (OSError, ValueError):
            pass

    # 2. Numbered doc shorthand: '25 §12', 'doc 25 §12', 'doc25:12', '19e', '24:T6', '25'
    m = re.match(r'^(?:doc\s*)?([0-9]{1,2}[a-z]?)(?:[:\s§—-]+(.*))?$', token, re.IGNORECASE)
    if m:
        prefix = m.group(1).lower()
        rest = (m.group(2) or "").strip()
        pattern = f"{int(prefix[:2]):02d}" if prefix[:2].isdigit() and len(prefix) <= 2 else prefix
        candidates = list(REPO_ROOT.glob(f"patch_manager_mods/{pattern}_*.md")) + list(REPO_ROOT.glob(f"patch_manager_mods/{prefix}_*.md")) + list(REPO_ROOT.glob(f"*{pattern}*.md"))
        if candidates:
            if len(candidates) > 1 and rest:
                rest_lower = rest.lower()
                for cand in candidates:
                    if cand.name.lower() in rest_lower or cand.stem.lower() in rest_lower:
                        return cand.resolve(), rest
                best_cand = None
                max_score = 0
                for cand in candidates:
                    cand_words = set(cand.stem.lower().split('_'))
                    rest_words = set(re.findall(r'[a-z0-9_]+', rest_lower))
                    overlap = len(cand_words & rest_words)
                    if overlap > max_score:
                        max_score = overlap
                        best_cand = cand
                if best_cand and max_score > 0:
                    return best_cand.resolve(), rest
            return candidates[0].resolve(), rest

    # 3. Phase ID shorthand: 'T4', 'T5d', 'M0', 'R10', 'HL4', 'CH1c', 'U3', 'QH1'
    m_phase = re.match(r'^([A-Za-z]{1,3}[0-9]{1,2}[a-z]?)(?:[:\s§—-]+(.*))?$', token)
    if m_phase:
        phase = m_phase.group(1)
        rest = (m_phase.group(2) or "").strip()
        pattern = re.compile(rf'(?:\|\s*\*{{0,2}}{re.escape(phase)}\*{{0,2}}\s*\||#+\s*(?:Phase\s*)?{re.escape(phase)}\b)', re.IGNORECASE)
        cands = [REPO_ROOT / "patch_manager_mods" / "24_master_plan_status.md"] + sorted(REPO_ROOT.glob("patch_manager_mods/*plan*.md")) + sorted(REPO_ROOT.glob("patch_manager_mods/*.md"))
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
        p, _ = resolve_plan_shorthand(explicit_path)
        if p:
            return p
        sys.stderr.write(f"Error: Specified plan file or shorthand not found: {explicit_path}\n")
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
            status_col_idx = None
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

    plan_stem = plan["path"].stem
    scoped_handoff = HANDOFFS_DIR / f"{plan_stem}.handoff.md"
    handoff_direction = ""
    if scoped_handoff.is_file():
        try:
            h_txt = scoped_handoff.read_text(encoding="utf-8")
            m_sec0 = re.search(r'## 0\.\s*Next-Phase Direction\s*\n+([^\n#]+)', h_txt)
            if m_sec0:
                handoff_direction = m_sec0.group(1).strip()
        except Exception:
            pass

    if json_mode:
        import json
        upcoming = remaining[1:4]
        res_data = {
            "plan_path": str(plan["path"].relative_to(REPO_ROOT)),
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
            res_data["scoped_handoff"] = str(scoped_handoff.relative_to(REPO_ROOT))
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

        print(f"\nNext Steps:")
        print(f"  - Resume: python3 scripts/plan_manager.py resume {current['ident']}")
        print(f"  - Handoff / Plan Mode: /plan-handoff {current['ident']} (or 'python3 scripts/plan_manager.py handoff {current['ident']}')")
        print(f"  - Mark Complete: python3 scripts/plan_manager.py mark-done {current['ident']} [--commit <sha>]")
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
    print(f"\nNext Handoff: /plan-handoff {cur['ident']} (or 'python3 scripts/plan_manager.py handoff {cur['ident']}')")
    if contract:
        sys.path.insert(0, str(REPO_ROOT / "scripts"))
        from agent_contract import template
        print("\n" + template(contract))

def cmd_mark_done(plan: dict, target_str: str, commit_sha: str = "", notes: str = "", state: str = "DONE"):
    plan_path = plan["path"]
    text = plan_path.read_text(encoding="utf-8")
    state_str = state.strip().upper() if state else "DONE"
    
    # 1. Try Markdown table update
    lines = text.split("\n")
    table_updated = False
    status_col_idx = None
    id_col_idx = 0
    commit_col_idx = None
    notes_col_idx = None

    for idx, line in enumerate(lines):
        if not line.strip().startswith("|"):
            status_col_idx = None
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
            clean_ident = ident.strip("*_ ")
            clean_target = target_str.strip("*_ ")
            if clean_ident.lower() == clean_target.lower() or clean_target.lower() in clean_ident.lower():
                raw_cells = line.split("|")
                raw_cells[status_col_idx + 1] = f" {state_str} "
                if commit_sha and commit_col_idx is not None and len(raw_cells) > commit_col_idx + 1:
                    raw_cells[commit_col_idx + 1] = f" {commit_sha.strip()} "
                if notes and notes_col_idx is not None and len(raw_cells) > notes_col_idx + 1:
                    raw_cells[notes_col_idx + 1] = f" {notes.strip()} "
                lines[idx] = "|".join(raw_cells)
                table_updated = True
                break

    def _log_done():
        if ml:
            ml.log_event("PHASE_DONE", {
                "plan": str(plan_path),
                "phase": target_str,
                "state": state_str,
                "commit": commit_sha,
                "notes": notes,
            })

    if table_updated:
        plan_path.write_text("\n".join(lines), encoding="utf-8")
        print(f"Marked table row '{target_str}' {state_str} in {plan_path.name}")
        _log_done()
        return

    # 2. Try checkbox update
    pattern = re.compile(rf'(-\s*\[)[ /](\]\s*.*{re.escape(target_str)}.*)', re.IGNORECASE)
    if pattern.search(text):
        new_text = pattern.sub(r'\1x\2', text, count=1)
        plan_path.write_text(new_text, encoding="utf-8")
        print(f"Marked checkbox done in {plan_path.name}: {target_str}")
        _log_done()
        return

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

def cmd_handoff(plan: dict, git_info: dict, user_direction: str = "", handoff_dir: Path | str | None = None):
    target_dir = Path(handoff_dir).resolve() if handoff_dir else HANDOFF_FILE.parent
    target_dir.mkdir(parents=True, exist_ok=True)
    handoff_file = target_dir / "session_handoff.md"
    prev_handoff_file = target_dir / "session_handoff.prev.md"
    handoffs_dir = target_dir / "handoffs"

    if handoff_file.exists():
        import shutil
        shutil.copy2(handoff_file, prev_handoff_file)

    remaining = [p for p in plan["phases"] if not p["done"]]
    current = remaining[0] if remaining else None
    if user_direction:
        m_target = re.search(r'\b([A-Za-z]{1,3}[0-9]{1,2}[a-z]?)\b', user_direction)
        if m_target:
            target_id = m_target.group(1).upper()
            for p in plan["phases"]:
                if p.get("ident", "").upper() == target_id:
                    current = p
                    remaining = [p] + [x for x in remaining if x != p]
                    break
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

        if len(remaining) > 1:
            content.append("## 2b. Upcoming Batches in Pipeline")
            for p in remaining[1:4]:
                p_prefix = p['ident'][0].upper() if p['ident'] else ""
                p_cat = PREFIX_LEGEND.get(p_prefix, "")
                p_badge = f"[{p_cat}] " if p_cat else ""
                content.append(f"- **{p['ident']}** {p_badge}: {p['name']}")
            content.append("")

    content.append("## 3. Strict Context Budget & Read Policy")
    content.append("- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.")
    content.append("- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.")
    content.append("- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.")
    content.append("- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.\n")

    handoff_file.write_text("\n".join(content), encoding="utf-8")
    rel_h = handoff_file.relative_to(REPO_ROOT) if handoff_file.is_relative_to(REPO_ROOT) else handoff_file
    rel_prev = prev_handoff_file.relative_to(REPO_ROOT) if prev_handoff_file.is_relative_to(REPO_ROOT) else prev_handoff_file
    print(f"Handoff written to {rel_h}")
    print(f"Rotated previous handoff to {rel_prev}")
    if ml:
        ml.log_event("SESSION_HANDOFF", {
            "plan": str(plan["path"]) if plan and "path" in plan else "",
            "notes": user_direction,
        })

    # Plan-scoped handoff file (prevents multi-session clobbering in same worktree)
    handoffs_dir.mkdir(parents=True, exist_ok=True)
    plan_stem = plan["path"].stem
    plan_handoff_file = handoffs_dir / f"{plan_stem}.handoff.md"
    plan_handoff_file.write_text("\n".join(content), encoding="utf-8")
    rel_plan_h = plan_handoff_file.relative_to(REPO_ROOT) if plan_handoff_file.is_relative_to(REPO_ROOT) else plan_handoff_file
    print(f"Plan-scoped handoff written to {rel_plan_h}")
    print("Safe to run /clear or exit session.")

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
    
    p_set = subparsers.add_parser("set", parents=[parent_parser], help="Set active plan")
    p_set.add_argument("plan_path", help="Path to master plan markdown file")

    p_mark = subparsers.add_parser("mark-done", parents=[parent_parser], help="Mark phase complete or update state")
    p_mark.add_argument("phase", help="Phase or chunk name/substring to mark done")
    p_mark.add_argument("--state", default="DONE", help="Target status to record in table (e.g. DONE, PARTIAL, BUILT, STAGED)")
    p_mark.add_argument("--commit", default="", help="Optional commit SHA to record in table")
    p_mark.add_argument("--notes", default="", help="Optional notes/verification result")
    p_mark.add_argument("--handoff-dir", default="", help="Optional directory to write handoff files (default .claude)")
    p_mark.add_argument("--no-handoff", action="store_true", help="Skip automatic handoff generation")

    subparsers.add_parser("legend", help="Display the master key and phase prefix legend")
    subparsers.add_parser("key", help="Display the master key and phase prefix legend")
    subparsers.add_parser("list", parents=[parent_parser], help="List all known master and subdomain plans")
    subparsers.add_parser("plans", parents=[parent_parser], help="List all known master and subdomain plans")

    p_hand = subparsers.add_parser("handoff", parents=[parent_parser], help="Prepare session handoff file")
    p_hand.add_argument("direction", nargs="?", default="", help="Optional user instructions for next phase")
    p_hand.add_argument("--handoff-dir", default="", help="Optional directory to write handoff files (default .claude)")

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
        ACTIVE_PLAN_FILE.parent.mkdir(parents=True, exist_ok=True)
        ACTIVE_PLAN_FILE.write_text(str(p), encoding="utf-8")
        print(f"Active plan set to: {p.relative_to(REPO_ROOT)}")
        sys.exit(0)

    explicit_plan = getattr(args, "plan", "") or getattr(args, "plan_path", "")

    # Auto-resolve plan from phase when mark-done is called without explicit plan
    if args.cmd == "mark-done" and not explicit_plan and getattr(args, "phase", ""):
        resolved_p, _ = resolve_plan_shorthand(args.phase)
        if resolved_p:
            explicit_plan = str(resolved_p)

    # Auto-resolve plan when next is called with a phase or plan shorthand
    if args.cmd == "next" and not explicit_plan and getattr(args, "plan_path", ""):
        resolved_p, _ = resolve_plan_shorthand(args.plan_path)
        if resolved_p:
            explicit_plan = str(resolved_p)

    plan_path = discover_active_plan(explicit_path=explicit_plan)
    if not plan_path:
        sys.stderr.write("No active master plan found. Set one with 'python3 scripts/plan_manager.py set <path>'.\n")
        sys.exit(1)

    # When explicit plan/phase was provided, anchor active_plan.txt for this session
    if explicit_plan and not getattr(args, "handoff_dir", None):
        ACTIVE_PLAN_FILE.parent.mkdir(parents=True, exist_ok=True)
        ACTIVE_PLAN_FILE.write_text(str(plan_path), encoding="utf-8")

    plan = parse_plan(plan_path)
    git_info = get_git_info()

    if args.cmd in ("resume", "status"):
        cmd_resume(plan, git_info, json_mode=getattr(args, "json", False))
    elif args.cmd == "next":
        cmd_next(plan, contract=getattr(args, "contract", ""))
    elif args.cmd == "mark-done":
        cmd_mark_done(plan, args.phase, commit_sha=args.commit, notes=args.notes, state=args.state)
        # Ensure active plan reflects the plan we just marked done
        ACTIVE_PLAN_FILE.parent.mkdir(parents=True, exist_ok=True)
        ACTIVE_PLAN_FILE.write_text(str(plan["path"]), encoding="utf-8")

        # Automatically update handoff and output Saved State block
        if not getattr(args, "no_handoff", False):
            updated_plan = parse_plan(plan["path"])
            remaining = [p for p in updated_plan["phases"] if not p["done"]]
            next_phase = remaining[0] if remaining else None
            next_ident = next_phase["ident"] if next_phase else ""
            user_dir = f"Execute {next_phase['ident']}: {next_phase['name']}" if next_phase else "All phases complete"
            h_dir = getattr(args, "handoff_dir", "") or None
            cmd_handoff(updated_plan, git_info, user_direction=user_dir, handoff_dir=h_dir)

            plan_stem = updated_plan["path"].stem
            target_h_dir = Path(h_dir).resolve() if h_dir else HANDOFFS_DIR
            plan_handoff_file = target_h_dir / f"{plan_stem}.handoff.md"
            rel_plan_h = plan_handoff_file.relative_to(REPO_ROOT) if plan_handoff_file.is_relative_to(REPO_ROOT) else plan_handoff_file

            print("\n=== SAVED STATE (Safe to /clear) ===")
            print(f"  - Plan: {updated_plan['path'].name}")
            commit_str = f" @ {args.commit}" if args.commit else ""
            print(f"  - Completed: {args.phase} ({args.state}){commit_str}")
            print(f"  - Handoff: {rel_plan_h}")
            if next_phase:
                print(f"  - Next Phase: {next_phase['ident']} — {next_phase['name']}")
                print(f"  - Resume: python3 scripts/plan_manager.py resume {next_ident} (or 'resume {next_ident}')")
            else:
                print("  - Next Phase: None (all phases complete)")
    elif args.cmd == "handoff":
        user_dir = args.direction
        plan_from_dir, rest = resolve_plan_shorthand(user_dir)
        if plan_from_dir and not explicit_plan:
            plan = parse_plan(plan_from_dir)
            if rest:
                cleaned_rest = rest.lstrip("— -").strip()
                if cleaned_rest.isdigit() or cleaned_rest.startswith("§"):
                    user_dir = f"Execute {cleaned_rest} in {plan_from_dir.name}"
                elif cleaned_rest:
                    user_dir = cleaned_rest
                else:
                    user_dir = f"Continue {plan_from_dir.name}"
            else:
                user_dir = f"Continue {plan_from_dir.name}"
        active_plan_target = (Path(args.handoff_dir) / "active_plan.txt") if getattr(args, "handoff_dir", None) else ACTIVE_PLAN_FILE
        active_plan_target.parent.mkdir(parents=True, exist_ok=True)
        active_plan_target.write_text(str(plan["path"]), encoding="utf-8")
        cmd_handoff(plan, git_info, user_direction=user_dir, handoff_dir=getattr(args, "handoff_dir", "") or None)

if __name__ == "__main__":
    main()
