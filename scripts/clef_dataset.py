#!/usr/bin/env python3
"""clef_dataset.py - Fact building & feature extraction flow from master_ledger.jsonl.

Parses the worktree audit ledger to extract historical priors (average durations,
best backends, common roles, pass rates) per mod and per workflow for the
clef-flash System-1 decision router.

Usage:
  python3 scripts/clef_dataset.py summary
  python3 scripts/clef_dataset.py json [--mod <mod>]
  python3 scripts/clef_dataset.py prior <mod>
"""

import argparse
import json
import os
import sys
from collections import defaultdict
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "scripts"))

from master_ledger import canonical_mod_name

DEFAULT_LEDGER = REPO_ROOT / ".claude" / "master_ledger.jsonl"
DEFAULT_CACHE = REPO_ROOT / ".claude" / "clef_priors.json"

DEFAULT_ROLE_DURATIONS_MS = {
    "scout": 2500,
    "verify": 3500,
    "gate": 4000,
    "ghidra": 6000,
    "code": 45000,
    "test": 30000,
    "doc": 15000,
    "review": 25000,
    "deep": 120000,
}


def parse_duration_ms(val) -> float | None:
    if val is None:
        return None
    try:
        s = str(val).strip()
        if s.endswith("ms"):
            return float(s[:-2])
        if s.endswith("s"):
            return float(s[:-1]) * 1000.0
        # raw number; if small (< 100), likely seconds; if large (> 100), likely ms
        f = float(s)
        if f < 100.0:
            return f * 1000.0
        return f
    except (ValueError, TypeError):
        return None


def extract_ledger_facts(ledger_path: Path | str | None = None, force_refresh: bool = False) -> dict:
    """Extracts structured priors and event statistics from master_ledger.jsonl with mtime caching."""
    path = Path(ledger_path) if ledger_path else DEFAULT_LEDGER
    if not path.is_file():
        return {
            "total_events": 0,
            "global_prior": {
                "avg_duration_ms": 3200,
                "best_backend": "local",
                "common_role": "code",
                "pass_rate": 0.95,
            },
            "mods": {},
            "roles": {},
        }

    cache_path = path.parent / "clef_priors.json"
    if not force_refresh and cache_path.is_file():
        try:
            if cache_path.stat().st_mtime >= path.stat().st_mtime:
                with open(cache_path, "r", encoding="utf-8") as f:
                    return json.load(f)
        except Exception:
            pass

    mod_stats = defaultdict(lambda: {
        "runs": 0,
        "passes": 0,
        "fails": 0,
        "durations_ms": [],
        "roles": defaultdict(int),
        "backends": defaultdict(int),
        "models": defaultdict(int),
        "tokens_in": [],
        "tokens_out": [],
    })

    role_stats = defaultdict(lambda: {
        "runs": 0,
        "durations_ms": [],
        "backends": defaultdict(int),
        "models": defaultdict(int),
        "tokens_in": [],
        "tokens_out": [],
    })

    total_events = 0
    all_durations = []
    global_backends = defaultdict(int)
    global_roles = defaultdict(int)

    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                rec = json.loads(line)
            except Exception:
                continue

            total_events += 1
            event = rec.get("event", "").upper()

            # Ignore test pollution
            if rec.get("mod") in ("demo", "demo_src") or "demo_swap_" in str(rec.get("backup", "")):
                continue

            # Extract associated mod with canonicalization
            raw_mod = rec.get("mod") or rec.get("mods")
            mod_list = []
            if isinstance(raw_mod, list):
                mod_list = [canonical_mod_name(m) for m in raw_mod if m]
            elif isinstance(raw_mod, str) and raw_mod.strip():
                mod_list = [canonical_mod_name(m) for m in raw_mod.split(",") if m.strip()]
            elif "plan" in rec:
                plan = str(rec["plan"])
                if "diversity" in plan:
                    mod_list = ["npc_diversity_src"]
                elif "queue" in plan:
                    mod_list = ["queue_hud_src"]
                elif "hotbar" in plan:
                    mod_list = ["combat_hotbar_src"]

            mod_list = [m for m in mod_list if m and m != "demo_src"]

            role = rec.get("role")
            backend = rec.get("backend")
            model = rec.get("actual_model") or rec.get("model")
            dur = parse_duration_ms(rec.get("duration_ms") or rec.get("duration"))
            status = rec.get("status") or rec.get("verdict")

            tin = None
            tout = None
            try:
                if "tokens_in" in rec:
                    tin = int(rec["tokens_in"])
                if "tokens_out" in rec:
                    tout = int(rec["tokens_out"])
            except (ValueError, TypeError):
                pass

            if event in ("OPENCODE_RUN", "SUBAGENT_DIFF_CHECK", "CLEF_ROUTE", "BUILD_GATE"):
                if dur is not None and dur > 0:
                    all_durations.append(dur)
                if backend:
                    global_backends[backend] += 1
                if role:
                    global_roles[role] += 1

                for mod in mod_list:
                    m = mod_stats[mod]
                    m["runs"] += 1
                    if status in ("PASS", "OK"):
                        m["passes"] += 1
                    elif status in ("FAIL", "ERROR"):
                        m["fails"] += 1
                    if dur is not None and dur > 0:
                        m["durations_ms"].append(dur)
                    if role:
                        m["roles"][role] += 1
                    if backend:
                        m["backends"][backend] += 1
                    if model:
                        m["models"][model] += 1
                    if tin is not None:
                        m["tokens_in"].append(tin)
                    if tout is not None:
                        m["tokens_out"].append(tout)

                if role:
                    r = role_stats[role]
                    r["runs"] += 1
                    if dur is not None and dur > 0:
                        r["durations_ms"].append(dur)
                    if backend:
                        r["backends"][backend] += 1
                    if model:
                        r["models"][model] += 1
                    if tin is not None:
                        r["tokens_in"].append(tin)
                    if tout is not None:
                        r["tokens_out"].append(tout)

    # Aggregate summaries
    global_avg_dur = round(sum(all_durations) / len(all_durations), 1) if all_durations else 45000.0
    best_global_backend = max(global_backends.keys(), key=lambda k: global_backends[k]) if global_backends else "local"
    common_global_role = max(global_roles.keys(), key=lambda k: global_roles[k]) if global_roles else "code"

    mods_summary = {}
    for mod, data in mod_stats.items():
        comm_r = max(data["roles"].keys(), key=lambda k: data["roles"][k]) if data["roles"] else common_global_role
        dur_list = data["durations_ms"]
        if dur_list:
            avg_d = round(sum(dur_list) / len(dur_list), 1)
        else:
            avg_d = float(DEFAULT_ROLE_DURATIONS_MS.get(comm_r, 45000))

        best_b = max(data["backends"].keys(), key=lambda k: data["backends"][k]) if data["backends"] else best_global_backend
        # Laplace smoothing against global prior (p0 = 0.95, weight = 2)
        pass_rate = round((data["passes"] + 1.9) / (data["runs"] + 2.0), 2)

        avg_tin = round(sum(data["tokens_in"]) / len(data["tokens_in"])) if data["tokens_in"] else 0
        avg_tout = round(sum(data["tokens_out"]) / len(data["tokens_out"])) if data["tokens_out"] else 0

        mods_summary[mod] = {
            "runs": data["runs"],
            "avg_duration_ms": avg_d,
            "best_backend": best_b,
            "common_role": comm_r,
            "pass_rate": pass_rate,
            "avg_tokens_in": avg_tin,
            "avg_tokens_out": avg_tout,
            "top_models": sorted(data["models"].keys(), key=lambda k: data["models"][k], reverse=True)[:3],
        }

    roles_summary = {}
    for r, data in role_stats.items():
        d_list = data["durations_ms"]
        fb = DEFAULT_ROLE_DURATIONS_MS.get(r, 30000)
        if len(d_list) >= 3:
            avg_role_dur = round(sum(d_list) / len(d_list), 1)
        else:
            avg_role_dur = round(0.7 * fb + 0.3 * (sum(d_list) / len(d_list) if d_list else fb), 1)

        roles_summary[r] = {
            "runs": data["runs"],
            "avg_duration_ms": avg_role_dur,
            "preferred_backend": max(data["backends"].keys(), key=lambda k: data["backends"][k]) if data["backends"] else best_global_backend,
            "avg_tokens_in": round(sum(data["tokens_in"]) / len(data["tokens_in"])) if data["tokens_in"] else 0,
            "avg_tokens_out": round(sum(data["tokens_out"]) / len(data["tokens_out"])) if data["tokens_out"] else 0,
        }

    res = {
        "total_events": total_events,
        "global_prior": {
            "avg_duration_ms": 3500.0,
            "best_backend": best_global_backend,
            "common_role": common_global_role,
            "pass_rate": 0.95,
        },
        "mods": mods_summary,
        "roles": roles_summary,
    }

    try:
        tmp_cache = cache_path.with_suffix(".tmp")
        with open(tmp_cache, "w", encoding="utf-8") as f:
            json.dump(res, f, indent=2)
        tmp_cache.replace(cache_path)
    except Exception:
        pass

    return res


def get_ledger_facts(force_refresh: bool = False) -> dict:
    """Convenience alias for extract_ledger_facts with mtime caching."""
    return extract_ledger_facts(force_refresh=force_refresh)


def get_mod_prior(mod_name: str, facts: dict | None = None) -> dict:
    """Returns the prior dict for a specific mod name, falling back to global prior."""
    if facts is None:
        facts = extract_ledger_facts()
    mod_info = facts.get("mods", {}).get(mod_name)
    if mod_info:
        return {
            "avg_duration_ms": mod_info["avg_duration_ms"],
            "best_backend": mod_info["best_backend"],
            "common_role": mod_info["common_role"],
            "pass_rate": mod_info["pass_rate"],
        }
    return dict(facts["global_prior"])


def main():
    parser = argparse.ArgumentParser(description="Extract facts and priors from master_ledger.jsonl")
    parser.add_argument("command", choices=["summary", "json", "prior"], default="summary", nargs="?")
    parser.add_argument("--mod", help="Specific mod to query")
    parser.add_argument("target_mod", nargs="?", help="Mod name for 'prior' command")
    parser.add_argument("--ledger", help="Path to master_ledger.jsonl override")
    args = parser.parse_args()

    facts = extract_ledger_facts(args.ledger)

    if args.command == "summary":
        print(f"Master Ledger Fact Base: {facts['total_events']} events processed.")
        print(f"Global Prior: avg_duration={facts['global_prior']['avg_duration_ms']}ms, "
              f"backend={facts['global_prior']['best_backend']}, "
              f"role={facts['global_prior']['common_role']}")
        print(f"\nTracked Mods ({len(facts['mods'])}):")
        for mod, info in sorted(facts["mods"].items()):
            print(f"  • {mod:<26} runs={info['runs']:<3} avg_dur={info['avg_duration_ms']:<7}ms "
                  f"backend={info['best_backend']:<6} role={info['common_role']:<6} pass={info['pass_rate']*100:.0f}%")
        print(f"\nTracked Roles ({len(facts['roles'])}):")
        for role, rinfo in sorted(facts["roles"].items()):
            print(f"  • {role:<10} runs={rinfo['runs']:<3} avg_dur={rinfo['avg_duration_ms']:<7}ms backend={rinfo['preferred_backend']}")

    elif args.command == "json":
        if args.mod:
            out = facts["mods"].get(args.mod, get_mod_prior(args.mod, facts))
        else:
            out = facts
        print(json.dumps(out, indent=2))

    elif args.command == "prior":
        mod = args.target_mod or args.mod or "global"
        prior = get_mod_prior(mod, facts)
        print(json.dumps(prior, indent=2))


if __name__ == "__main__":
    main()
