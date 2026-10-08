#!/usr/bin/env python3
"""subagent_prompt.py - Deterministic zero-read prompt generator for OpenCode subagents.

Usage:
  subagent_prompt.py <phase_or_task> [--plan PATH] [--run auto|start] [--json]

Extracts phase context directly from the active master plan, inlines necessary
file targets, verification commands, and agent contracts to produce a 100%
self-contained prompt ready for OpenCode (oc-flow auto / oc-flow start).

Examples:
  python3 scripts/subagent_prompt.py QH3
  python3 scripts/subagent_prompt.py PCM1
  python3 scripts/subagent_prompt.py QH3 --run auto
"""

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

try:
    import agent_contract as ac
except ImportError:
    ac = None

try:
    import plan_manager as pm
except ImportError:
    pm = None


def resolve_phase_and_plan(phase_key: str, explicit_plan: str = None):
    plan_path = None
    if explicit_plan:
        plan_path = Path(explicit_plan)
        if not plan_path.is_absolute():
            plan_path = REPO_ROOT / plan_path
    elif pm:
        resolved, _ = pm.resolve_plan_shorthand(phase_key)
        if resolved:
            plan_path = resolved

    if not plan_path or not plan_path.exists():
        active_txt = REPO_ROOT / ".claude" / "active_plan.txt"
        if active_txt.exists():
            content = active_txt.read_text().strip()
            if content:
                p = Path(content)
                plan_path = p if p.is_absolute() else REPO_ROOT / p

    return plan_path


def extract_phase_info(plan_path: Path, phase_key: str):
    if not plan_path or not plan_path.exists():
        return phase_key, "", [], []

    text = plan_path.read_text(encoding="utf-8", errors="replace")

    title = f"Phase {phase_key}"
    description = ""
    verif_cmds = []

    # 1. Check table rows: | ID | desc | ... |
    for line in text.splitlines():
        if not line.strip().startswith("|"):
            continue
        cells = [c.strip() for c in line.strip().split("|")[1:-1]]
        if len(cells) >= 2 and cells[0].strip("*") == phase_key:
            desc_cell = cells[1]
            status_cell = cells[2] if len(cells) > 2 else ""
            verif_cell = cells[3] if len(cells) > 3 else ""
            title = f"{phase_key}: {desc_cell}"
            description = f"- Goal: {desc_cell}\n- Status: {status_cell}"
            if verif_cell and verif_cell != "-":
                cmds = re.findall(r"`([^`]+)`", verif_cell)
                if cmds:
                    verif_cmds.extend(cmds)
                else:
                    verif_cmds.append(verif_cell)
            break

    # 2. Check full section headers
    sec_pattern = re.compile(rf"(^#+\s+(?:(?:Phase|Batch|Chunk|Task|Step)\s+)?{re.escape(phase_key)}[:\s].*?)(?=^#+\s|\Z)", re.MULTILINE | re.DOTALL | re.IGNORECASE)
    sec_match = sec_pattern.search(text)
    if sec_match:
        section_text = sec_match.group(1).strip()
        if not description:
            description = section_text
        else:
            description += f"\n\n### Plan Section Details:\n{section_text}"
        header_title = section_text.splitlines()[0].strip("# ").strip()
        if header_title:
            title = header_title

    # 3. Find files mentioned in section
    combined_text = f"{title}\n{description}"
    words = re.findall(r"[\w\./-]+\.(?:cpp|h|hpp|c|py|ini|md|sh)", combined_text)
    valid_paths = []
    for w in words:
        clean = w.strip("`'\"(),:;[]")
        p = REPO_ROOT / clean
        if p.exists() and p.is_file() and clean not in valid_paths:
            valid_paths.append(clean)
        elif plan_path:
            # Check relative to plan directory and subdirectories
            for cand in [plan_path.parent / clean, plan_path.parent / "tools" / clean, plan_path.parent / "tools" / "parity" / clean]:
                if cand.exists() and cand.is_file():
                    rel = str(cand.relative_to(REPO_ROOT))
                    if rel not in valid_paths:
                        valid_paths.append(rel)
                    break

    # 4. Extract verification commands from text
    for line in combined_text.splitlines():
        if "build_check.py" in line or "run_tests.py" in line or "kpm_stage_check.py" in line:
            m = re.search(r"(python3\s+scripts/[\w\._-]+\s+[^`\n]+)", line)
            if m:
                verif_cmds.append(m.group(1).strip().strip("`\"'"))

    verif_cmds = list(dict.fromkeys(verif_cmds))

    # 5. Infer owning mod scope
    mod_scope = None
    plan_str = str(plan_path) if plan_path else ""
    if "companion_levelup" in plan_str or "level_up" in plan_str:
        mod_scope = "patch_manager_mods/level_up_override_src/ and research/companion_levelup/"
    else:
        m = re.search(r"(?:patch_manager_mods/)?([a-zA-Z0-9_]+_src)", combined_text)
        if not m:
            m = re.search(r"(?:patch_manager_mods/)?([a-zA-Z0-9_]+_src)", text)
        if m:
            mod_scope = f"patch_manager_mods/{m.group(1)}/"

    if mod_scope:
        description = f"- Owning Mod Scope: `{mod_scope}` (STRICT: Confine all edits strictly to this mod; NEVER edit other mods in patch_manager_mods/)\n" + description

    return title, description, valid_paths, verif_cmds


def build_self_contained_prompt(phase_key: str, title: str, description: str, files: list, verif_cmds: list, mod_scope: str = None):
    prompt_lines = [
        f"You are working on the KOTOR II mod repository to implement {title}.",
        "",
        "### Objective & Requirements:",
        description if description else f"Implement {phase_key} as specified in the master plan.",
        "",
    ]

    if mod_scope:
        prompt_lines.extend([
            "### Owning Mod & Scope Boundary (CRITICAL):",
            f"- Owning Mod Scope: `{mod_scope}`",
            "- Confine ALL research, planning, and code changes strictly to this mod.",
            "- NEVER propose or make changes to other mods under patch_manager_mods/.",
            "",
        ])

    if files:
        prompt_lines.append("### Target Files:")
        for f in files:
            prompt_lines.append(f"- `{f}`")
        prompt_lines.append("")

    prompt_lines.append("### Required Verification Commands:")
    if verif_cmds:
        for c in verif_cmds:
            prompt_lines.append(f"- `{c}`")
    prompt_lines.append("- `python3 scripts/agent_diff_check.py`")
    prompt_lines.append("")

    prompt_lines.extend([
        "### Plan Specification Requirements (When formulating plan):",
        "Your plan must explicitly detail:",
        "1. EXACT CODE CHANGES: Target file paths, function anchors, line numbers, exact textures/colors/constants, and struct offsets.",
        "2. EDGE CASES & TEST MATRIX: State transitions (pause/unpause, save/load, area transition), boundary conditions, null safety, and test assertions in *_test.cpp.",
        "3. TOOLING USAGE: Exact commands for scripts/build_check.py, scripts/run_tests.py, and scripts/agent_diff_check.py.",
        "4. DOCUMENTATION & SCHEMA: Master plan status updates and compliance with the 5-line WORKER schema.",
        "",
        "### Invariants & Non-Negotiables:",
        "- Edit ONLY targeted files. Do NOT touch unrelated mod directories or headers.",
        "- Do NOT write hardcoded memory addresses; use K2Addr or existing constants.",
        "- Run verification commands to ensure the build compiles and tests pass before concluding.",
        "",
    ])

    if ac:
        prompt_lines.append(ac.template("worker"))

    return "\n".join(prompt_lines)


def main():
    parser = argparse.ArgumentParser(description="Deterministic subagent prompt generator.")
    parser.add_argument("phase", help="Phase identifier (e.g. QH3, PCM1, AH1)")
    parser.add_argument("--plan", default=None, help="Explicit plan file path")
    parser.add_argument("--run", choices=["auto", "start"], default=None, help="Launch directly with oc-flow auto or start")
    parser.add_argument("--json", action="store_true", help="Emit JSON object")
    args = parser.parse_args()

    plan_path = resolve_phase_and_plan(args.phase, args.plan)
    title, desc, files, verif_cmds = extract_phase_info(plan_path, args.phase)

    prompt = build_self_contained_prompt(args.phase, title, desc, files, verif_cmds)

    if args.json:
        print(json.dumps({
            "phase": args.phase,
            "title": title,
            "plan_path": str(plan_path) if plan_path else None,
            "files": files,
            "verification_commands": verif_cmds,
            "prompt": prompt
        }, indent=2))
        return 0

    if args.run:
        flow_cmd = ["oc-flow", args.run, prompt]
        print(f"Launching: oc-flow {args.run} for phase {args.phase}...")
        res = subprocess.run(flow_cmd, cwd=str(REPO_ROOT))
        return res.returncode

    print(prompt)
    return 0


if __name__ == "__main__":
    sys.exit(main())
