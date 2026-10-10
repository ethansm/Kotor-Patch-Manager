#!/usr/bin/env python3
"""shadow_replay.py - 3-Way Decision Router Benchmark.

Extracts historical routing prompts and clef-flash baseline outputs from
.claude/master_ledger.jsonl, replays them concurrently across Kev-4B (port 8100)
and Kev-0.8B (port 8101), and produces an agreement and latency report.

Usage:
  python3 scripts/shadow_replay.py [--ledger <path>] [--limit <n>]
"""

import argparse
import json
import os
import sys
import time
import urllib.request
import urllib.error
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_LEDGER = REPO_ROOT / ".claude" / "master_ledger.jsonl"

KEV_4B_URL = os.environ.get("KEV_4B_URL", "http://127.0.0.1:8100/v1/systemone")
KEV_08B_URL = os.environ.get("KEV_08B_URL", "http://127.0.0.1:8101/v1/systemone")

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

BACKEND_CRITERIA = {
    "local": "Strictly offline local execution for scout, verify, or gate inspection.",
    "cloud": "Cloud-first OpenRouter free tier execution for implementation, test authoring, or deep reasoning.",
    "custom": "Specialized custom model override requested by user or workflow."
}

MODEL_CRITERIA = {
    "poolside/laguna-s-2.1:free": "Targeted single-file bug fixing or iterative test failure debugging.",
    "thinkingmachines/inkling:free": "Complex multi-file implementation, deep algorithmic refactoring, or cross-module feature generation.",
    "cohere/north-mini-code:free": "Fast boilerplate generation, unit test scaffolding, or lint/formatting fixes.",
    "local:qwen3.5-9b": "Local offline execution for verification, logs, or fast exploration."
}


def query_systemone(url: str, payload: dict, timeout_sec: float = 3.0) -> tuple[dict | None, float]:
    t0 = time.perf_counter()
    try:
        data = json.dumps(payload).encode("utf-8")
        req = urllib.request.Request(
            url,
            data=data,
            headers={"Content-Type": "application/json", "User-Agent": "shadow-replay/1.0"}
        )
        with urllib.request.urlopen(req, timeout=timeout_sec) as resp:
            raw = resp.read().decode("utf-8")
            elapsed_ms = (time.perf_counter() - t0) * 1000.0
            return json.loads(raw), elapsed_ms
    except Exception as e:
        elapsed_ms = (time.perf_counter() - t0) * 1000.0
        return None, elapsed_ms


def extract_evaluation_cases(ledger_path: Path, limit: int = 20) -> list[dict]:
    cases = []
    seen_prompts = set()

    if ledger_path.is_file():
        with open(ledger_path, "r", encoding="utf-8", errors="replace") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                try:
                    rec = json.loads(line)
                except Exception:
                    continue

                if rec.get("event") == "CLEF_ROUTE":
                    prompt = rec.get("prompt")
                    clef_out = rec.get("clef_output")
                    if not isinstance(clef_out, dict):
                        clef_out = {}
                    if prompt and prompt not in seen_prompts:
                        seen_prompts.add(prompt)
                        cases.append({
                            "prompt": prompt,
                            "active_mod": rec.get("active_mod"),
                            "baseline_engine": rec.get("engine", "clef-flash"),
                            "baseline_role": rec.get("decision") or clef_out.get("role"),
                            "baseline_backend": rec.get("backend") or clef_out.get("backend"),
                            "baseline_model": rec.get("routed_model") or clef_out.get("model"),
                            "baseline_latency_ms": rec.get("latency_ms"),
                            "baseline_confidence": rec.get("confidence", 0.75),
                        })

    # Supplementary realistic workspace prompt cases for high-coverage benchmarking
    test_benchmarks = [
        ("Fix pointer math in QueueHud hook detour", "queue_hud_src", "code"),
        ("Run build gate across all 28 workspace mods", None, "gate"),
        ("Check combat hotbar logic test failure in combat_logic_test.cpp", "combat_hotbar_src", "test"),
        ("Review memory safety of GuiInput detour wrapper", "gui_input_src", "review"),
        ("Inspect Ghidra GameAddr for CharacterStats at 0x005bda40", None, "ghidra"),
        ("Find all log files generated in latest play session", None, "scout"),
        ("Prepare master plan handoff documentation for npc diversity dispatcher", "npc_diversity_src", "doc"),
        ("Architect multi-file dialogue letterbox refactor", "dialogue_letterbox_fix_src", "deep"),
        ("Verify output assertion of active effects panel", "active_effects_panel_src", "verify"),
        ("Add null pointer check to CompanionAI execute detour", "companion_ai_src", "code")
    ]

    for p, mod, expected_role in test_benchmarks:
        if p not in seen_prompts:
            seen_prompts.add(p)
            cases.append({
                "prompt": p,
                "active_mod": mod,
                "baseline_engine": "clef-flash (synthetic)",
                "baseline_role": expected_role,
                "baseline_backend": "cloud" if expected_role not in ("gate", "scout", "verify") else "local",
                "baseline_model": "openrouter/poolside/laguna-s-2.1:free",
                "baseline_latency_ms": None,
                "baseline_confidence": 0.85,
            })

    return cases[:limit]


def extract_transcript_cases(limit: int = 15) -> list[dict]:
    """Extracts and sanitizes real user modding prompts from Antigravity session transcripts."""
    import glob, re
    brain_transcripts = glob.glob(os.path.expanduser("~/.gemini/antigravity-cli/brain/*/.system_generated/logs/transcript.jsonl"))
    tech_keywords = [
        "hook", "detour", "crash", "compile", "build_check", "ghidra", "kpm",
        "hotbar", "queue_hud", "letterbox", "diversity", "c++", "patch", "manifest",
        "gameaddr", "pointer", "vram", "test", "ledger", "viewport"
    ]

    cases = []
    seen = set()

    for t in brain_transcripts:
        try:
            with open(t, "r", encoding="utf-8") as f:
                for line in f:
                    rec = json.loads(line)
                    if rec.get("type") == "USER_INPUT":
                        content = rec.get("content", "").strip()
                        if "<SYSTEM_MESSAGE>" in content:
                            continue
                        m = re.search(r"<USER_REQUEST>(.*?)</USER_REQUEST>", content, re.DOTALL)
                        p = m.group(1).strip() if m else content
                        p = re.sub(r"<ADDITIONAL_METADATA>.*", "", p, flags=re.DOTALL).strip()
                        # Strict PII redaction
                        p = re.sub(r"[\w\.-]+@[\w\.-]+\.\w+", "[EMAIL]", p)
                        p = re.sub(r"password[:\s=]+[^\s]+", "[REDACTED]", p, flags=re.I)
                        if p in seen or len(p) < 25 or len(p) > 350:
                            continue
                        p_lower = p.lower()
                        if any(k in p_lower for k in tech_keywords):
                            seen.add(p)
                            expected_role = "code" if any(w in p_lower for w in ["fix", "implement", "tweak", "replace", "add"]) else "review"
                            cases.append({
                                "prompt": p,
                                "active_mod": None,
                                "baseline_engine": "transcript (real)",
                                "baseline_role": expected_role,
                                "baseline_backend": "cloud",
                                "baseline_model": "openrouter/poolside/laguna-s-2.1:free",
                                "baseline_latency_ms": None,
                                "baseline_confidence": 0.8,
                            })
                            if len(cases) >= limit:
                                return cases
        except Exception:
            pass
    return cases


def parse_answer(ans_dict: dict) -> tuple[str, float, float]:
    """Returns (choice, confidence, margin)."""
    if not ans_dict:
        return ("unknown", 0.0, 1.0)
    choice = ans_dict.get("choice", "unknown")
    conf = float(ans_dict.get("confidence", 0.0))
    probs = sorted(ans_dict.get("probabilities", {}).values(), reverse=True)
    margin = round(probs[0] / probs[1], 2) if len(probs) > 1 and probs[1] > 0 else 99.0
    return (choice, conf, margin)


def main():
    parser = argparse.ArgumentParser(description="3-Way Shadow Replay Evaluation")
    parser.add_argument("--ledger", default=str(DEFAULT_LEDGER), help="Path to master ledger")
    parser.add_argument("--source", choices=["ledger", "transcripts", "both"], default="transcripts", help="Prompt dataset source")
    parser.add_argument("--limit", type=int, default=12, help="Number of test cases to evaluate")
    parser.add_argument("--export", type=str, default="", help="Export extracted test cases to JSON file")
    parser.add_argument("--input", type=str, default="", help="Load test cases from exported JSON file")
    args = parser.parse_args()

    if args.input:
        with open(args.input, "r", encoding="utf-8") as f:
            cases = json.load(f)[:args.limit]
    elif args.source == "transcripts":
        cases = extract_transcript_cases(limit=args.limit)
    elif args.source == "ledger":
        cases = extract_evaluation_cases(Path(args.ledger), limit=args.limit)
    else:
        cases = extract_evaluation_cases(Path(args.ledger), limit=args.limit // 2) + extract_transcript_cases(limit=args.limit // 2)

    if args.export:
        export_p = Path(args.export)
        export_p.parent.mkdir(parents=True, exist_ok=True)
        with open(export_p, "w", encoding="utf-8") as f:
            json.dump(cases, f, indent=2)
        print(f"Exported {len(cases)} cases to {export_p}")

    if not cases:
        print("No evaluation cases found.", file=sys.stderr)
        sys.exit(1)

    # Synthetic/transcript baselines are heuristic labels with an assumed latency, not clef-flash output.
    real_baseline = all(not str(c.get("baseline_engine", "")).endswith((" (synthetic)", "(real)")) for c in cases)
    base_name = "clef-flash" if real_baseline else "baseline"
    lat_measured = all(c.get("baseline_latency_ms") is not None for c in cases)

    print(f"\n{'='*80}")
    print(f"3-WAY SHADOW BENCHMARK: {base_name} vs Kev-4B (:8100) vs Kev-0.8B (:8101)")
    if not real_baseline:
        print("NOTE: some baseline roles are heuristic/hand-written labels and baseline latency is not measured, not clef-flash output.")
    print(f"Evaluating {len(cases)} prompt cases extracted from master ledger & test matrix...")
    print(f"{'='*80}\n")

    kev4b_roles, kev08b_roles, baseline_roles = [], [], []
    kev4b_lats, kev08b_lats, baseline_lats = [], [], []
    n_cached_4b = 0

    print(f"{'Prompt Case':<42} | {base_name:<10} | {'Kev-4B':<10} | {'Kev-0.8B':<10} | {'4B ms':<6} | {'0.8B ms':<7}")
    print(f"{'-'*42}-+-{'-'*10}-+-{'-'*10}-+-{'-'*10}-+-{'-'*6}-+-{'-'*7}")

    for idx, c in enumerate(cases):
        prompt = c["prompt"]
        active_mod = c["active_mod"]
        base_role = c["baseline_role"] or "code"
        base_lat = c["baseline_latency_ms"]

        payload = {
            "model": "kev-latest",
            "state": {
                "prompt": prompt[:500],
                "active_mod": active_mod or "none"
            },
            "questions": {
                "role": {
                    "type": "choice",
                    "instructions": "Which subagent role is best suited for this task?",
                    "criteria": ROLE_CRITERIA
                }
            }
        }

        # Query Kev-4B (:8100, fallback to GPU benchmark cache)
        res_4b, lat_4b = query_systemone(KEV_4B_URL, payload)
        cached_4b = False
        if not res_4b:
            cached_4b_file = REPO_ROOT / ".claude" / "kev_4b_benchmark.json"
            if cached_4b_file.is_file():
                try:
                    with open(cached_4b_file, "r", encoding="utf-8") as f:
                        cached_4b_data = json.load(f)
                    if prompt in cached_4b_data:
                        c_entry = cached_4b_data[prompt]
                        lat_4b = c_entry.get("latency_ms", 185.0)
                        res_4b = {"answers": {"role": c_entry}}
                        cached_4b = True
                except Exception:
                    pass

        # Query Kev-0.8B (:8101 live)
        res_08b, lat_08b = query_systemone(KEV_08B_URL, payload)

        r_4b, conf_4b, margin_4b = parse_answer(res_4b.get("answers", {}).get("role", {}) if res_4b else {})
        r_08b, conf_08b, margin_08b = parse_answer(res_08b.get("answers", {}).get("role", {}) if res_08b else {})

        # Unavailable endpoints and stale cached Kev-4B answers are recorded as None so they are excluded from agreement
        kev4b_roles.append(r_4b if res_4b and not cached_4b else None)
        kev08b_roles.append(r_08b if res_08b else None)
        baseline_roles.append(base_role)

        if cached_4b:
            n_cached_4b += 1
        if res_4b and not cached_4b:
            kev4b_lats.append(lat_4b)
        if res_08b:
            kev08b_lats.append(lat_08b)
        if base_lat is not None:
            baseline_lats.append(base_lat)

        short_prompt = prompt[:40] + ".." if len(prompt) > 42 else prompt
        lat_4b_str = ("cache" if cached_4b else f"{lat_4b:.1f}") if res_4b else "OFF"
        lat_08b_str = f"{lat_08b:.1f}" if res_08b else "OFF"
        # Mark cached Kev-4B roles so the table does not show them as live answers
        r_4b_str = f"{r_4b}*" if cached_4b else f"{r_4b}"

        print(f"{short_prompt:<42} | {base_role:<10} | {r_4b_str:<10} | {r_08b:<10} | {lat_4b_str:<6} | {lat_08b_str:<7}")

    print(f"{'-'*42}-+-{'-'*10}-+-{'-'*10}-+-{'-'*10}-+-{'-'*6}-+-{'-'*7}\n")

    # Metrics computation
    def agreement(a_roles, b_roles):
        pairs = [(a, b) for a, b in zip(a_roles, b_roles) if a is not None and b is not None]
        return sum(1 for a, b in pairs if a == b) / len(pairs) * 100.0 if pairs else 0.0

    agree_4b_base = agreement(kev4b_roles, baseline_roles)
    agree_08b_base = agreement(kev08b_roles, baseline_roles)
    agree_08b_4b = agreement(kev08b_roles, kev4b_roles)
    total = len(cases)
    missing_4b = sum(1 for r in kev4b_roles if r is None) - n_cached_4b
    missing_08b = sum(1 for r in kev08b_roles if r is None)

    avg_base_lat = sum(baseline_lats) / len(baseline_lats) if baseline_lats else 0.0
    avg_base_str = f"{avg_base_lat:.1f}ms" if lat_measured and baseline_lats else "n/a"
    avg_4b_lat = sum(kev4b_lats) / len(kev4b_lats) if kev4b_lats else None
    avg_08b_lat = sum(kev08b_lats) / len(kev08b_lats) if kev08b_lats else 0.0
    avg_4b_str = f"{avg_4b_lat:.1f}ms" if avg_4b_lat is not None else "n/a"

    print(f"=== SUMMARY METRICS ===")
    print(f"  • Top-1 Agreement (Kev-4B vs {base_name}):  {agree_4b_base:.1f}%")
    print(f"  • Top-1 Agreement (Kev-0.8B vs {base_name}):{agree_08b_base:.1f}%")
    print(f"  • Top-1 Agreement (Kev-0.8B vs Kev-4B):    {agree_08b_4b:.1f}%")
    print(f"  • Cases without endpoint response (excluded): Kev-4B {missing_4b} | Kev-0.8B {missing_08b}")
    print(f"  • Mean Latency: {base_name}: {avg_base_str} | Kev-4B: {avg_4b_str} | Kev-0.8B: {avg_08b_lat:.1f}ms")
    if n_cached_4b:
        print(f"  • Kev-4B cache fallback used for {n_cached_4b}/{total} cases (excluded from Kev-4B agreement and latency)")
    if avg_08b_lat > 0 and real_baseline and lat_measured and avg_base_lat > 0:
        speedup = avg_base_lat / avg_08b_lat
        print(f"  • Kev-0.8B Speedup over {base_name}:       {speedup:.1f}x (baseline latency / 0.8B latency)")
    elif avg_08b_lat > 0:
        print("  • Kev-0.8B Speedup: n/a (baseline latency is not measured)")
    print()


if __name__ == "__main__":
    main()
