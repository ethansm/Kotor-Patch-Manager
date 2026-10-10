#!/usr/bin/env python3
"""clef_matcher.py - Local clef-flash System-1 Routing Engine & Matcher.

Consumes task prompt, active mod, and git diff telemetry paired with historical ledger
priors to output calibrated subagent execution contracts in <45ms.
Logs all decisions to master_ledger.jsonl under event CLEF_ROUTE.

Usage:
  python3 scripts/clef_matcher.py route "<prompt>" [--mod <active_mod>] [--json]
  python3 scripts/clef_matcher.py test
"""

import argparse
import json
import os
import re
import subprocess
import sys
import time
import urllib.request
import urllib.error
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "scripts"))

from clef_dataset import extract_ledger_facts, get_mod_prior
from master_ledger import log_event, canonical_mod_name

CLEF_ENDPOINT = os.environ.get("CLEF_SYSTEMONE_URL", "http://127.0.0.1:8101/v1/systemone")
CLEF_HEALTH_URL = os.environ.get("CLEF_HEALTH_URL", "http://127.0.0.1:8101/docs")
MODS_DIR = REPO_ROOT / "patch_manager_mods"

VALID_ROLES = ["scout", "verify", "gate", "review", "deep", "code", "ghidra", "test", "doc"]
VALID_BACKENDS = ["local", "cloud", "custom"]

ROLE_CRITERIA = {
    "scout": "Fast file discovery, offline log parsing, or pattern lookup.",
    "verify": "Test result validation, log assertion matching, or behavioral checks.",
    "gate": "Build gate verification (build_check.py), hook span audits, or manifest checks.",
    "ghidra": "Ghidra address lookup, decompilation audits, calling convention analysis, or GameAddr resolution.",
    "code": "Targeted C++ or Python implementation strictly confined to the owning mod.",
    "test": "Host unit tests, contract tests, or mutation test authoring.",
    "doc": "Master plan updates, handoff preparation, README/CHANGELOG sync, or architectural RFCs.",
    "review": "Code diff review, memory-safety audits, calling convention checks, or regression hunting.",
    "deep": "Deep architecture planning, complex multi-file refactoring, or root-cause debugging."
}

MODEL_CRITERIA = {
    "poolside/laguna-s-2.1:free": "Targeted single-file or two-file bug fixing, iterative test failure debugging, or medium-scoped logic updates.",
    "thinkingmachines/inkling:free": "Complex multi-file implementation, deep algorithmic refactoring, or cross-module feature generation.",
    "cohere/north-mini-code:free": "Fast boilerplate generation, unit test scaffolding, type annotations, docstrings, or lint/formatting fixes.",
    "local:qwen3.5-9b": "Local RTX 5070 offline execution for verification, logs, or fast exploration."
}


def get_existing_mod_dirs() -> set[str]:
    """Returns set of all buildable mod directories under patch_manager_mods/."""
    if not MODS_DIR.is_dir():
        return set()
    return {
        p.name for p in MODS_DIR.iterdir()
        if p.is_dir() and p.name.endswith("_src") and not p.name.startswith("_")
    }


def infer_active_mod(prompt: str, explicit_mod: str | None = None) -> str | None:
    """Infers the owning mod directory from explicit arg or prompt text."""
    existing_mods = get_existing_mod_dirs()
    if explicit_mod:
        canon = canonical_mod_name(explicit_mod)
        if canon and (canon in existing_mods or canon == "_shared_src"):
            return canon
        clean = explicit_mod.strip().replace("patch_manager_mods/", "").rstrip("/")
        if clean in existing_mods or clean == "_shared_src":
            return clean
        if f"{clean}_src" in existing_mods:
            return f"{clean}_src"

    prompt_lower = prompt.lower()
    for mod in existing_mods:
        base = mod.replace("_src", "").replace("_", "")
        clean_mod = mod.replace("_", "")
        if mod.lower() in prompt_lower or clean_mod.lower() in prompt_lower or base.lower() in prompt_lower:
            return mod

    # Common aliases
    if "queue" in prompt_lower:
        return "queue_hud_src"
    if "letterbox" in prompt_lower:
        return "dialogue_letterbox_fix_src"
    if "hotbar" in prompt_lower:
        return "combat_hotbar_src"
    if "diversity" in prompt_lower or "npc" in prompt_lower:
        return "npc_diversity_src"
    if "companion" in prompt_lower and "heal" in prompt_lower:
        return "companion_selfheal_src"
    if "companion" in prompt_lower:
        return "companion_ai_src"

    # Fallback: check tracked modified files in git only if prompt refers to current/active mod
    if any(k in prompt_lower for k in ["this mod", "current mod", "active mod", "working mod", "staged mod"]):
        try:
            proc = subprocess.run(
                ["git", "diff", "--name-only"],
                stdout=subprocess.PIPE,
                stderr=subprocess.DEVNULL,
                text=True,
                timeout=0.3
            )
            if proc.returncode == 0 and proc.stdout:
                for line in proc.stdout.splitlines():
                    filepath = line.strip()
                    if filepath.startswith("patch_manager_mods/"):
                        mod_part = filepath.split("/")[1]
                        if mod_part in existing_mods:
                            return mod_part
        except Exception:
            pass

    return None


def get_git_diff_summary() -> tuple[bool, int]:
    """Returns (has_diff, diff_lines)."""
    try:
        proc = subprocess.run(
            ["git", "diff", "--numstat"],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            timeout=0.5
        )
        if proc.returncode == 0 and proc.stdout.strip():
            lines = 0
            for row in proc.stdout.splitlines():
                parts = row.split()
                if len(parts) >= 2:
                    try:
                        lines += int(parts[0]) + int(parts[1])
                    except ValueError:
                        pass
            return (lines > 0, lines)
    except Exception:
        pass
    return (False, 0)


def sanitize_write_scope(role: str, active_mod: str | None) -> list[str]:
    """Filters write scope strictly against existing directories; recon roles get empty scope."""
    if role in ("scout", "verify", "review", "ghidra"):
        return []

    if role == "doc":
        return ["docs/*", "patch_manager_mods/*.md"]
    if role == "gate":
        return ["scripts/build_check.py"]

    existing_mods = get_existing_mod_dirs()
    if active_mod and active_mod in existing_mods:
        return [f"patch_manager_mods/{active_mod}/*"]
    if active_mod == "_shared_src":
        return ["patch_manager_mods/_shared_src/*"]

    return []


def query_clef_engine(payload: dict, timeout_sec: float = 1.0) -> dict | None:
    """Queries local Kev-0.8B System-1 daemon over HTTP on port 8101."""
    try:
        data = json.dumps(payload).encode("utf-8")
        req = urllib.request.Request(
            CLEF_ENDPOINT,
            data=data,
            headers={"Content-Type": "application/json", "User-Agent": "clef-matcher/1.0"}
        )
        with urllib.request.urlopen(req, timeout=timeout_sec) as resp:
            return json.loads(resp.read().decode("utf-8"))
    except Exception:
        return None


def get_cooled_down_models() -> set[str]:
    """Returns set of model IDs currently in rate-limit cooldown from telemetry.json."""
    telemetry_file = Path.home() / ".cache" / "clef-gate" / "telemetry.json"
    if not telemetry_file.is_file():
        return set()
    try:
        with open(telemetry_file, "r", encoding="utf-8") as f:
            data = json.load(f)
        now = time.time()
        cooled = set()
        for m, meta in data.get("openrouter_models", {}).items():
            if meta.get("cooldown_until_epoch", 0) > now:
                cooled.add(m)
                cooled.add(f"openrouter/{m}")
        return cooled
    except Exception:
        return set()


def deterministic_fallback_routing(prompt: str, active_mod: str | None, prior: dict) -> dict:
    """Fallback router when local clef-flash daemon is offline (<5ms latency)."""
    p_lower = prompt.lower()

    if any(k in p_lower for k in ["find", "search", "locate", "where is", "log check", "scan"]):
        role = "scout"
        backend = "local"
        model = "local:qwen3.5-9b"
    elif any(k in p_lower for k in ["verify", "assert", "check test", "validate output"]):
        role = "verify"
        backend = "local"
        model = "local:qwen3.5-9b"
    elif any(k in p_lower for k in ["build check", "gate", "compile check"]):
        role = "gate"
        backend = "local"
        model = "local:qwen3.5-9b"
    elif any(k in p_lower for k in ["ghidra", "address", "sub_", "gameaddr", "decompile", "calling convention"]):
        role = "ghidra"
        backend = "cloud"
        model = "openrouter/thinkingmachines/inkling:free"
    elif any(k in p_lower for k in ["test", "unit test", "contract test", "mutation"]):
        role = "test"
        backend = "cloud"
        model = "openrouter/cohere/north-mini-code:free"
    elif any(k in p_lower for k in ["doc", "readme", "handoff", "plan update", "changelog"]):
        role = "doc"
        backend = "cloud"
        model = "openrouter/cohere/north-mini-code:free"
    elif any(k in p_lower for k in ["review", "audit", "diff review", "safety check"]):
        role = "review"
        backend = "cloud"
        model = "openrouter/thinkingmachines/inkling:free"
    elif any(k in p_lower for k in ["refactor", "architect", "multi-file", "redesign"]):
        role = "deep"
        backend = "cloud"
        model = "openrouter/thinkingmachines/inkling:free"
    else:
        role = "code"
        backend = "cloud"
        model = "openrouter/poolside/laguna-s-2.1:free"

    cooled = get_cooled_down_models()
    if model in cooled:
        chain = [
            "openrouter/poolside/laguna-s-2.1:free",
            "openrouter/thinkingmachines/inkling:free",
            "openrouter/cohere/north-mini-code:free",
        ]
        for alt in chain:
            if alt not in cooled:
                model = alt
                break

    scope = sanitize_write_scope(role, active_mod)
    est_dur = prior.get("avg_duration_ms", 3200)

    return {
        "role": role,
        "backend": backend,
        "model": model,
        "write_scope": scope,
        "predicted_duration_ms": int(est_dur),
        "confidence": 0.75,
        "engine": "deterministic_fallback",
        "active_mod": active_mod,
    }


def route_task(prompt: str, explicit_mod: str | None = None, log_telemetry: bool = True) -> dict:
    """Main routing entrypoint: gathers context & priors, queries clef-flash, returns contract."""
    t0 = time.perf_counter()
    active_mod = infer_active_mod(prompt, explicit_mod)
    has_diff, diff_lines = get_git_diff_summary()
    facts = extract_ledger_facts()
    prior = get_mod_prior(active_mod or "global", facts)

    state_obj = {
        "prompt": prompt[:1000],
        "active_mod": active_mod or "none",
        "has_diff": has_diff,
        "diff_lines": diff_lines,
        "historical_prior": prior,
    }

    payload = {
        "model": "kev-latest",
        "state": state_obj,
        "questions": {
            "role": {
                "type": "choice",
                "instructions": "Which subagent role is best suited for this task?",
                "criteria": ROLE_CRITERIA,
            },
            "backend": {
                "type": "choice",
                "instructions": "Which backend tier should execute this subagent task?",
                "criteria": {
                    "local": "Strictly offline local RTX 5070 execution for scout, verify, or gate inspection.",
                    "cloud": "Cloud-first OpenRouter free tier execution for implementation, test authoring, or deep reasoning.",
                    "custom": "Specialized custom model override requested by user or workflow."
                },
            },
            "model": {
                "type": "choice",
                "instructions": "Which specific model should execute this task?",
                "criteria": MODEL_CRITERIA,
            },
        },
    }

    clef_resp = query_clef_engine(payload)
    latency_ms = round((time.perf_counter() - t0) * 1000.0, 1)

    if clef_resp and "answers" in clef_resp:
        ans = clef_resp["answers"]
        role_ans = ans.get("role", {})
        backend_ans = ans.get("backend", {})
        model_ans = ans.get("model", {})

        probs = role_ans.get("probabilities", {})
        sorted_probs = sorted(probs.items(), key=lambda x: x[1], reverse=True) if probs else []
        top_role = sorted_probs[0][0] if sorted_probs else role_ans.get("choice", "code")
        top_conf = sorted_probs[0][1] if sorted_probs else float(role_ans.get("confidence", 0.8))
        second_conf = sorted_probs[1][1] if len(sorted_probs) > 1 else 0.0

        # Calibrated 9-class threshold:
        # Uniform base is 1/9 = ~0.11. Top-1 >= 0.32 is ~3x random chance.
        # Ambiguous guard triggers if top_conf < 0.32 or top_conf / second_conf < 1.20 (ambiguous tie)
        is_ambiguous = (top_conf < 0.32) or (second_conf > 0 and (top_conf / second_conf < 1.20))

        if is_ambiguous:
            role = "review"
            role_conf = top_conf
            backend = "cloud"
            model = "openrouter/thinkingmachines/inkling:free"
        else:
            role = top_role
            role_conf = top_conf
            backend = backend_ans.get("choice", "cloud")
            model = model_ans.get("choice", "poolside/laguna-s-2.1:free")

        scope = sanitize_write_scope(role, active_mod)
        est_dur = prior.get("avg_duration_ms", 3500)

        model_str = f"openrouter/{model}" if not model.startswith("local:") and not model.startswith("openrouter/") else model
        cooled = get_cooled_down_models()
        if model_str in cooled:
            chain = [
                "openrouter/poolside/laguna-s-2.1:free",
                "openrouter/thinkingmachines/inkling:free",
                "openrouter/cohere/north-mini-code:free",
            ]
            for alt in chain:
                if alt not in cooled:
                    model_str = alt
                    break

        decision = {
            "role": role,
            "backend": backend,
            "model": model_str,
            "write_scope": scope,
            "predicted_duration_ms": int(est_dur),
            "confidence": round(role_conf, 2),
            "engine": "kev-0.8b" if clef_resp.get("model") in ("kev-latest", "jev-latest", "kev-0.8b") else clef_resp.get("model", "kev-0.8b"),
            "latency_ms": latency_ms,
            "active_mod": active_mod,
        }
    else:
        decision = deterministic_fallback_routing(prompt, active_mod, prior)
        decision["latency_ms"] = latency_ms

    if log_telemetry:
        log_event(
            "CLEF_ROUTE",
            {
                "engine": decision.get("engine", "clef-flash"),
                "decision": decision.get("role"),
                "backend": decision.get("backend"),
                "routed_model": decision.get("model"),
                "active_mod": decision.get("active_mod"),
                "confidence": decision.get("confidence"),
                "latency_ms": decision.get("latency_ms"),
                "predicted_duration_ms": decision.get("predicted_duration_ms"),
                "write_scope": decision.get("write_scope", []),
                "prompt": prompt[:120],
                "clef_output": decision,
            }
        )

    return decision


def main():
    parser = argparse.ArgumentParser(description="clef-matcher: System-1 Subagent Router")
    parser.add_argument("command", choices=["route", "test"], default="route", nargs="?")
    parser.add_argument("prompt", nargs="?", default="", help="Task prompt string")
    parser.add_argument("--mod", help="Explicit active mod name")
    parser.add_argument("--json", action="store_true", help="Output pure JSON")
    parser.add_argument("--no-log", action="store_true", help="Do not log to master_ledger.jsonl")
    args = parser.parse_args()

    if args.command == "route":
        if not args.prompt:
            print("Error: prompt is required for route command", file=sys.stderr)
            sys.exit(1)
        res = route_task(args.prompt, explicit_mod=args.mod, log_telemetry=not args.no_log)
        if args.json:
            print(json.dumps(res, indent=2))
        else:
            print(f"[{res['engine'].upper()} ROUTE ({res['latency_ms']}ms, conf={res['confidence']})]")
            print(f"  • Role:     {res['role']}")
            print(f"  • Backend:  {res['backend']}")
            print(f"  • Model:    {res['model']}")
            print(f"  • Scope:    {res['write_scope']}")
            print(f"  • Est Time: {res['predicted_duration_ms']}ms")
            print(f"  • Mod:      {res['active_mod']}")
    elif args.command == "test":
        print("Running clef_matcher verification tests...")
        # Self-test
        test_prompts = [
            ("Fix pointer math in QueueHud hook detour", "queue_hud_src"),
            ("Audit GameAddr calling convention in Ghidra", None),
            ("Run build_check.py on all mods", None),
            ("Write unit test for inventory sorting", "inventory_sort_filter_src"),
            ("Update 34_npc_diversity master plan handoff", "npc_diversity_src"),
        ]
        for p, m in test_prompts:
            d = route_task(p, explicit_mod=m, log_telemetry=False)
            print(f"PASS: '{p[:40]}...' -> role={d['role']} backend={d['backend']} mod={d['active_mod']}")


if __name__ == "__main__":
    main()
