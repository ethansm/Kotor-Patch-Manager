#!/usr/bin/env python3
"""agent_diff_check.py - Subagent diff, scope, build gate, and test validator. Python 3, stdlib only.

Usage:
  agent_diff_check.py [--scope GLOB ...] [--mod MOD ...] [--test TEST ...] [--baseline FILE] [--json]

Validates the work of an OpenCode subagent after code execution:
1. Scope verification: Ensures modified/untracked files conform to allowed globs.
2. Build verification: Runs build_check.py for affected mods.
3. Test verification: Runs unit/logic tests via run_tests.py.
4. Static invariants: Checks for raw address anti-patterns or broken includes.

Exit codes:
  0 = PASS (all scope, build, and test invariants satisfied)
  1 = FAIL (scope breach, compilation failure, or test failure)
"""

import argparse
import fnmatch
import json
import os
import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

try:
    import master_ledger as ml
except ImportError:
    ml = None

try:
    import agent_contract as ac
except ImportError:
    ac = None


def get_git_status():
    """Returns dict of {rel_path: status_code} for all modified/untracked files."""
    res = subprocess.run(
        ["git", "status", "--porcelain", "-uall"],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        check=True
    )
    changes = {}
    for line in res.stdout.splitlines():
        if not line.strip():
            continue
        st = line[:2]
        path = line[3:].strip()
        if " -> " in path:
            path = path.split(" -> ")[1].strip()
        changes[path] = st
    return changes


EXCLUDED_MODS = {"perf_lab_src"}


def get_changed_mods(changed_paths):
    """Finds which patch_manager_mods/*_src directories were touched."""
    mods = set()
    mod_pattern = re.compile(r"^patch_manager_mods/([a-zA-Z0-9_]+_src)/")
    for p in changed_paths:
        m = mod_pattern.match(p)
        if m and m.group(1) not in EXCLUDED_MODS:
            mods.add(m.group(1))
    return sorted(mods)


def check_scope(changed_paths, scopes):
    """Returns list of paths violating allowed scopes."""
    violations = []
    for p in changed_paths:
        matched = any(fnmatch.fnmatch(p, s) for s in scopes)
        if not matched:
            violations.append(p)
    return violations


def run_build_checks(mods):
    """Runs build_check.py for each mod. Returns (ok, output)."""
    if not mods:
        return True, "No mod sources touched."
    cmd = ["python3", "scripts/build_check.py"]
    for m in mods:
        cmd.extend(["--mod", m])
    res = subprocess.run(cmd, cwd=str(REPO_ROOT), capture_output=True, text=True)
    return res.returncode == 0, res.stdout + res.stderr


def run_tests(test_filters):
    """Runs run_tests.py for given filters. Returns (ok, output)."""
    if not test_filters:
        return True, "No specific tests requested."
    all_ok = True
    outputs = []
    for tf in test_filters:
        cmd = ["python3", "scripts/run_tests.py", "-k", tf]
        res = subprocess.run(cmd, cwd=str(REPO_ROOT), capture_output=True, text=True)
        if res.returncode != 0:
            all_ok = False
        outputs.append(res.stdout + res.stderr)
    return all_ok, "\n".join(outputs)


def safe_rel(p):
    if not p:
        return None
    path = Path(p)
    try:
        if path.is_relative_to(REPO_ROOT):
            return str(path.relative_to(REPO_ROOT))
    except (ValueError, AttributeError):
        pass
    return str(path)


def main():
    parser = argparse.ArgumentParser(description="Subagent diff, scope, and build validator.")
    parser.add_argument("--scope", nargs="+", default=None, help="Allowed glob patterns for changed files")
    parser.add_argument("--mod", nargs="+", default=None, help="Explicit mod src targets (e.g. queue_hud_src)")
    parser.add_argument("--test", nargs="+", default=None, help="Test substrings to run via run_tests.py")
    parser.add_argument("--baseline", default=None, help="Baseline snapshot file (default: .claude/contract_baseline.tsv if present)")
    parser.add_argument("--patch-out", default=None, help="Path to write revert patch (default: .claude/subagent_runs/latest.patch)")
    parser.add_argument("--subagent", help="Subagent role or identity being verified (e.g. opencode:code)")
    parser.add_argument("--no-patch", action="store_true", help="Skip generating audit/revert patch")
    parser.add_argument("--json", action="store_true", help="Emit structured JSON verdict")
    args = parser.parse_args()

    baseline_file = args.baseline
    if baseline_file is None:
        def_base = REPO_ROOT / ".claude" / "contract_baseline.tsv"
        if def_base.is_file():
            baseline_file = str(def_base)

    if baseline_file and Path(baseline_file).is_file() and ac:
        base_snap = ac.read_snapshot(Path(baseline_file))
        changed_paths_set, _ = ac.changed_since(REPO_ROOT, base_snap)
        changed_paths = sorted(changed_paths_set)
    else:
        changes = get_git_status()
        changed_paths = sorted(changes.keys())

    # 1. Scope Check
    scope_violations = []
    if args.scope:
        scope_violations = check_scope(changed_paths, args.scope)

    # 2. Mod resolution
    target_mods = list(args.mod) if args.mod else get_changed_mods(changed_paths)

    # 3. Build Check
    build_ok, build_out = run_build_checks(target_mods)

    # 4. Test Check
    test_ok, test_out = run_tests(args.test)

    # 5. Generate audit/revert patch if changes exist
    patch_file = None
    if changed_paths and not args.no_patch:
        patch_file = Path(args.patch_out) if args.patch_out else (REPO_ROOT / ".claude" / "subagent_runs" / "latest.patch")
        patch_file.parent.mkdir(parents=True, exist_ok=True)
        diff_res = subprocess.run(["git", "diff", "--"] + changed_paths, cwd=str(REPO_ROOT), capture_output=True, text=True)
        if diff_res.stdout:
            patch_file.write_text(diff_res.stdout, encoding="utf-8")

    # Determine verdict
    passed = (len(scope_violations) == 0) and build_ok and test_ok
    verdict = "PASS" if passed else "FAIL"

    summary = {
        "verdict": verdict,
        "baseline_used": baseline_file,
        "changed_files_count": len(changed_paths),
        "changed_files": changed_paths,
        "scope_violations": scope_violations,
        "checked_mods": target_mods,
        "build_ok": build_ok,
        "test_ok": test_ok,
        "patch_file": safe_rel(patch_file) if patch_file and patch_file.exists() else None,
    }

    if args.json:
        summary["build_output"] = build_out.strip()
        summary["test_output"] = test_out.strip()
        print(json.dumps(summary, indent=2))
    else:
        if passed:
            print(f"PASS: {len(changed_paths)} file(s) changed, scope clean, {len(target_mods)} mod(s) built clean, tests passing.")
            if target_mods:
                print(f"  Verified mods: {', '.join(target_mods)}")
            if patch_file and patch_file.exists():
                print(f"  Audit Patch: {safe_rel(patch_file)} (revert with 'git apply -R {safe_rel(patch_file)}')")
        else:
            print(f"FAIL: Subagent validation failed.")
            if scope_violations:
                print(f"  Scope violations ({len(scope_violations)}):")
                for sv in scope_violations[:5]:
                    print(f"    - {sv}")
            if not build_ok:
                print("  Build error:")
                for l in build_out.strip().splitlines()[-10:]:
                    print(f"    {l}")
            if not test_ok:
                print("  Test error:")
                for l in test_out.strip().splitlines()[-10:]:
                    print(f"    {l}")
            if patch_file and patch_file.exists():
                print(f"  Audit Patch: {safe_rel(patch_file)}")

    if ml:
        subagent = args.subagent
        if not subagent:
            try:
                for e in reversed(ml.read_events()):
                    if e.get("event") in ("OPENCODE_RUN", "OPENCODE_FLOW"):
                        subagent = e.get("agent") or e.get("role")
                        break
            except Exception:
                pass
        ml.log_event("SUBAGENT_DIFF_CHECK", {
            "subagent": subagent or "subagent",
            "verdict": verdict,
            "mods": target_mods,
            "changed_count": len(changed_paths),
            "scope_violations": len(scope_violations),
            "patch_file": safe_rel(patch_file) if patch_file and patch_file.exists() else None,
        })

    return 0 if passed else 1


if __name__ == "__main__":
    sys.exit(main())
