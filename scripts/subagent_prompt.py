#!/usr/bin/env python3
"""subagent_prompt.py - Deterministic zero-read prompt generator for OpenCode subagents.

Usage:
  subagent_prompt.py <phase_or_task> [--role ROLE] [--plan PATH] [--run auto|start] [--json]

Extracts phase context directly from the active master plan, inlines necessary
file targets, verification commands, and agent contracts to produce a 100%
self-contained prompt ready for OpenCode (oc-flow auto / oc-flow start / oc-subagent).

Roles:
  code (default), scout, verify, ghidra, gate, test, review, deep, doc

Examples:
  python3 scripts/subagent_prompt.py QH3
  python3 scripts/subagent_prompt.py QH3 --role scout
  python3 scripts/subagent_prompt.py QH3 --role verify
  python3 scripts/subagent_prompt.py PCM1 --run auto
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
except ImportError as exc:
    ac = None
    print(f"warning: agent_contract unavailable, output schema omitted: {exc}", file=sys.stderr)

try:
    import plan_manager as pm
except ImportError as exc:
    pm = None
    print(f"warning: plan_manager unavailable, plan shorthand resolution degraded: {exc}", file=sys.stderr)

RECON_ROLES = {"scout", "verify", "review", "deep", "ghidra"}
WORKER_ROLES = {"code", "gate", "test", "doc"}


def extract_ghidra_symbols(text: str) -> list[str]:
    """Extracts reverse engineering symbol candidates (K2Addr, GameAddr, addresses, symbols)."""
    symbols = []
    # K2Addr::Symbol
    for m in re.finditer(r"K2Addr::([A-Za-z0-9_]+)", text):
        sym = m.group(1)
        if sym not in symbols:
            symbols.append(sym)
    # Hex addresses 0x00xxxxxx
    for m in re.finditer(r"\b(0x00[0-9a-fA-F]{6})\b", text):
        addr = m.group(1)
        if addr not in symbols:
            symbols.append(addr)
    # Function names SceneCamVt_*, etc.
    for m in re.finditer(r"\b(SceneCamVt_[A-Za-z0-9_]+|GetPartyMemberByIndex)\b", text):
        fn = m.group(1)
        if fn not in symbols:
            symbols.append(fn)
    return symbols


def query_ghidra_symbol_info(symbol: str) -> str | None:
    """Attempts to query query_ghidra.py for symbol info; returns None on failure."""
    q_script = REPO_ROOT / "scripts" / "query_ghidra.py"
    if not q_script.exists():
        return None
    try:
        cmd = [sys.executable, str(q_script)]
        if symbol.startswith("0x"):
            cmd.extend(["address", symbol])
        else:
            cmd.extend(["symbol", symbol, "--limit", "2"])
        res = subprocess.run(cmd, cwd=str(REPO_ROOT), capture_output=True, text=True, timeout=5)
        if res.returncode == 0 and res.stdout.strip():
            return res.stdout.strip()
    except Exception:
        pass
    return None


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

    # No fallback to .claude/active_plan.txt: every session rewrites it, so an unresolved phase
    # would silently pull another session's plan into the brief.
    return plan_path


def extract_phase_info(plan_path: Path, phase_key: str):
    if not plan_path or not plan_path.exists():
        return phase_key, "", [], [], None

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
    words = re.findall(r"[\w\./-]+\.(?:cpp|hpp|h|c|py|ini|md|sh)", combined_text)
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
                    try:
                        rel = str(cand.relative_to(REPO_ROOT))
                    except ValueError:
                        continue
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
    all_context = f"{combined_text}\n{' '.join(verif_cmds)}"

    # 5a. Check verif_cmds for --mod <name>_src
    m_cmd = re.search(r"--mod\s+([a-zA-Z0-9_]+_src)", " ".join(verif_cmds))
    if m_cmd:
        mod_scope = f"patch_manager_mods/{m_cmd.group(1)}/"

    # 5b. Plan-path owner (strongest signal; beats text heuristics)
    if not mod_scope:
        plan_str = str(plan_path) if plan_path else ""
        if "companion_levelup" in plan_str or "level_up" in plan_str:
            mod_scope = "patch_manager_mods/level_up_override_src/ and research/companion_levelup/"

    # 5c. Check combined context for explicit mod directory
    if not mod_scope:
        m_ctx = re.search(r"(?:patch_manager_mods/)?([a-zA-Z0-9_]+_src)", all_context)
        if m_ctx:
            mod_scope = f"patch_manager_mods/{m_ctx.group(1)}/"

    # 5d. Check mod aliases / keywords in combined context
    if not mod_scope:
        ctx_lower = all_context.lower()
        if "queue hud" in ctx_lower or "queue_hud" in ctx_lower or "queue" in ctx_lower:
            mod_scope = "patch_manager_mods/queue_hud_src/"
        elif "action hotbar" in ctx_lower or "action_hotbar" in ctx_lower:
            mod_scope = "patch_manager_mods/action_hotbar_src/"
        elif "combat hotbar" in ctx_lower or "combat_hotbar" in ctx_lower or "hotbar" in ctx_lower:
            mod_scope = "patch_manager_mods/combat_hotbar_src/"
        elif "diversity" in ctx_lower or "npc" in ctx_lower:
            mod_scope = "patch_manager_mods/npc_diversity_src/"
        elif "letterbox" in ctx_lower:
            mod_scope = "patch_manager_mods/dialogue_letterbox_fix_src/"
        elif "companion" in ctx_lower and "heal" in ctx_lower:
            mod_scope = "patch_manager_mods/companion_selfheal_src/"
        elif "companion" in ctx_lower:
            mod_scope = "patch_manager_mods/companion_ai_src/"

    return title, description, valid_paths, verif_cmds, mod_scope


def build_self_contained_prompt(
    phase_key: str,
    title: str,
    description: str,
    files: list,
    verif_cmds: list,
    mod_scope: str = None,
    role: str = "code",
    ghidra_symbols: list = None
):
    role = (role or "code").lower()
    is_recon = role in RECON_ROLES

    # Header
    if role == "scout":
        header_action = f"discover and scout {title}"
    elif role == "verify":
        header_action = f"verify {title}"
    elif role == "ghidra":
        header_action = f"audit Ghidra reversing data and symbols for {title}"
    elif role == "review":
        header_action = f"review code changes and invariants for {title}"
    elif role == "deep":
        header_action = f"conduct deep technical investigation for {title}"
    elif role == "test":
        header_action = f"author and verify test suites for {title}"
    elif role == "doc":
        header_action = f"update documentation and master plans for {title}"
    elif role == "gate":
        header_action = f"verify build gates and hook bytes for {title}"
    else:
        header_action = f"implement {title}"

    prompt_lines = [
        f"You are working on the KOTOR II mod repository to {header_action}.",
        "",
        "### Objective & Requirements:",
        description if description else f"Process {phase_key} as specified in the master plan.",
        "",
    ]

    # Mod Scope Boundary
    if mod_scope:
        if is_recon:
            prompt_lines.extend([
                "### Target Scope Boundary:",
                f"- Owning Mod Scope: `{mod_scope}`",
                "- Read-Only Role: Do NOT create or modify source code files under patch_manager_mods/.",
                "",
            ])
        else:
            prompt_lines.extend([
                "### Owning Mod & Scope Boundary (CRITICAL):",
                f"- Owning Mod Scope: `{mod_scope}`",
                "- Confine ALL research, planning, and code changes strictly to this mod.",
                "- NEVER propose or make changes to other mods under patch_manager_mods/.",
                "",
            ])

    # Target files
    if files:
        target_heading = "### Relevant Files to Inspect:" if is_recon else "### Target Files:"
        prompt_lines.append(target_heading)
        for f in files:
            prompt_lines.append(f"- `{f}`")
        prompt_lines.append("")

    # Ghidra specific symbols
    if role == "ghidra":
        symbols = ghidra_symbols or []
        if not symbols and description:
            symbols = extract_ghidra_symbols(f"{title}\n{description}")
        if symbols:
            prompt_lines.append("### Target Symbols / Anchors to Resolve:")
            for s in symbols:
                res = query_ghidra_symbol_info(s)
                if res:
                    prompt_lines.append(f"- `{s}`: {res.splitlines()[0]}")
                else:
                    prompt_lines.append(f"- `{s}`")
            prompt_lines.append("")

        prompt_lines.extend([
            "### Ghidra Analysis & Reversing Focus:",
            "- Query addresses/symbols using: `python3 scripts/query_ghidra.py symbol <name> | address <hex>`.",
            "- Audit calling conventions (__thiscall, __cdecl, __stdcall), register arguments, and stack cleanup.",
            "- Check struct offsets and virtual method table layout against K2Addr / GameAddr.",
            "- Read-only reversing: Do NOT edit C++ files; output resolved addresses and signatures in your report.",
            "",
        ])

    elif role == "scout":
        prompt_lines.extend([
            "### Discovery & Recon Instructions:",
            "- Search code patterns, symbols, and offline logs (`python3 scripts/kotor2_logs.py check <log>`).",
            "- Identify relevant file anchors, struct layouts, and integration points.",
            "- Report findings concisely. Do NOT edit source files.",
            "",
        ])

    elif role == "verify":
        prompt_lines.append("### Required Verification Commands:")
        if verif_cmds:
            for c in verif_cmds:
                prompt_lines.append(f"- `{c}`")
        prompt_lines.append("- `python3 scripts/agent_diff_check.py`")
        prompt_lines.append("")
        prompt_lines.extend([
            "### Verification Instructions:",
            "- Execute the verification commands above.",
            "- Check compiler errors, test assertions, and changed diff boundaries.",
            "- Report PASS/FAIL status with concrete error lines or evidence.",
            "",
        ])

    elif role == "review":
        prompt_lines.extend([
            "### Code Review Checklist:",
            "1. Safety: Null pointer checks, bounds checking, buffer overflow hazards.",
            "2. Mod Isolation: Verify no out-of-scope files or headers were modified.",
            "3. Calling Conventions: Verify hook trampolines and GameAddr function calls preserve registers.",
            "4. Memory Addresses: Verify zero hardcoded magic addresses (must use K2Addr/constants).",
            "",
        ])

    else:
        # Worker roles (code, gate, test, doc)
        prompt_lines.append("### Required Verification Commands:")
        if verif_cmds:
            for c in verif_cmds:
                prompt_lines.append(f"- `{c}`")
        prompt_lines.append("- `python3 scripts/agent_diff_check.py`")
        prompt_lines.append("")

        if role == "code":
            prompt_lines.extend([
                "### Plan Specification Requirements (When formulating plan):",
                "Your plan must explicitly detail:",
                "1. EXACT CODE CHANGES: Target file paths, function anchors, line numbers, exact textures/colors/constants, and struct offsets.",
                "2. EDGE CASES & TEST MATRIX: State transitions (pause/unpause, save/load, area transition), boundary conditions, null safety, and test assertions in *_test.cpp.",
                "3. TOOLING USAGE: Exact commands for scripts/build_check.py, scripts/run_tests.py, and scripts/agent_diff_check.py.",
                "4. DOCUMENTATION & SCHEMA: Master plan status updates and compliance with the 5-line WORKER schema.",
                "",
            ])
        elif role == "test":
            prompt_lines.extend([
                "### Test Implementation Requirements:",
                "1. Write host-based unit tests (*_test.cpp) or common contract tests.",
                "2. Test boundary conditions, null pointer handling, and state transitions.",
                "3. Verify all tests pass with python3 scripts/run_tests.py.",
                "",
            ])
        elif role == "doc":
            prompt_lines.extend([
                "### Documentation Requirements:",
                "1. Update master plan status tables and handoffs.",
                "2. Ensure README/CHANGELOG and architecture notes are aligned.",
                "",
            ])

        prompt_lines.extend([
            "### Invariants & Non-Negotiables:",
            "- Edit ONLY targeted files. Do NOT touch unrelated mod directories or headers.",
            "- Do NOT write hardcoded memory addresses; use K2Addr or existing constants.",
            "- Run verification commands to ensure the build compiles and tests pass before concluding.",
            "",
        ])

    # Contract schema template
    if ac:
        contract_type = "recon" if is_recon else "worker"
        prompt_lines.append(ac.template(contract_type))

    return "\n".join(prompt_lines)


def main():
    parser = argparse.ArgumentParser(description="Deterministic subagent prompt generator.")
    parser.add_argument("phase", help="Phase identifier (e.g. QH3, PCM1, AH1)")
    parser.add_argument("--role", default="code",
                        choices=["code", "scout", "verify", "ghidra", "gate", "test", "review", "deep", "doc"],
                        help="Subagent role (determines schema and prompt structure)")
    parser.add_argument("--plan", default=None, help="Explicit plan file path")
    parser.add_argument("--run", choices=["auto", "start"], default=None, help="Launch directly with oc-flow auto or start")
    parser.add_argument("--json", action="store_true", help="Emit JSON object")
    args = parser.parse_args()

    plan_path = resolve_phase_and_plan(args.phase, args.plan)
    if args.plan and (not plan_path or not plan_path.exists()):
        print(f"error: --plan file not found: {args.plan}", file=sys.stderr)
        return 2
    if not plan_path or not plan_path.exists():
        print(f"error: phase {args.phase!r} did not resolve to a plan; pass --plan <path>", file=sys.stderr)
        return 2
    title, desc, files, verif_cmds, mod_scope = extract_phase_info(plan_path, args.phase)

    symbols = extract_ghidra_symbols(f"{title}\n{desc}")
    prompt = build_self_contained_prompt(
        args.phase, title, desc, files, verif_cmds,
        mod_scope=mod_scope, role=args.role, ghidra_symbols=symbols
    )

    if args.json:
        print(json.dumps({
            "phase": args.phase,
            "role": args.role,
            "title": title,
            "plan_path": str(plan_path) if plan_path else None,
            "mod_scope": mod_scope,
            "files": files,
            "verification_commands": verif_cmds,
            "ghidra_symbols": symbols,
            "prompt": prompt
        }, indent=2))
        return 0

    if args.run:
        flow_cmd = ["oc-flow", args.run, prompt]
        print(f"Launching: oc-flow {args.run} for phase {args.phase} (role={args.role})...")
        res = subprocess.run(flow_cmd, cwd=str(REPO_ROOT))
        return res.returncode

    print(prompt)
    return 0


if __name__ == "__main__":
    sys.exit(main())
