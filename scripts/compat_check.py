#!/usr/bin/env python3
"""
scripts/compat_check.py - Update compatibility check for KOTOR-II patch manager mods.

Answers: If a contributor changes something in Common or a mod, what does it break?
Builds dependency relationships at command time via include graph, manifest graph,
and git diff intake.

Usage:
  python3 scripts/compat_check.py patch_manager_mods/_shared_src/Gui2D.h
  python3 scripts/compat_check.py Gui2D.h --list
  python3 scripts/compat_check.py Gui2D.h --why
  python3 scripts/compat_check.py Gui2D.h --json out.json
  python3 scripts/compat_check.py --since main
  python3 scripts/compat_check.py --since HEAD -- patch_manager_mods/_shared_src
  python3 scripts/compat_check.py --staged
  python3 scripts/compat_check.py --lint
  python3 scripts/compat_check.py --all

Doc 22 (update compatibility check).
"""

from __future__ import annotations

import argparse
import fnmatch
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
from dataclasses import asdict
from pathlib import Path
from typing import Any

# Ensure scripts dir is on sys.path
SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from compat_graph import CompatGraph, Plan, PlanItem, find_repo_root

if sys.version_info >= (3, 11):
    import tomllib
else:
    import tomli as tomllib  # type: ignore


def _norm_posix(p: Path | str) -> str:
    return str(p).replace("\\", "/")


def resolve_file_arg(repo_root: Path, raw_arg: str, known_files: set[str]) -> tuple[str, bool]:
    """Resolve a user-provided file argument.
    Returns (repo_relative_posix_path, exists_on_disk).
    """
    raw_path = Path(raw_arg)

    # 1. Direct path relative to cwd or repo_root
    for candidate in (Path.cwd() / raw_path, repo_root / raw_path):
        if candidate.is_file():
            try:
                return _norm_posix(candidate.resolve().relative_to(repo_root.resolve())), True
            except ValueError:
                return _norm_posix(candidate), True

    # 2. Check if raw_arg exactly matches a known file path
    norm_arg = _norm_posix(raw_arg)
    if norm_arg in known_files:
        return norm_arg, (repo_root / norm_arg).is_file()

    # 3. Match basename among known files
    matches = [f for f in known_files if Path(f).name == raw_path.name]
    if len(matches) == 1:
        return matches[0], (repo_root / matches[0]).is_file()
    elif len(matches) > 1:
        # Prefer patch_manager_mods
        pm_matches = [f for f in matches if f.startswith("patch_manager_mods/")]
        if len(pm_matches) == 1:
            return pm_matches[0], (repo_root / pm_matches[0]).is_file()
        return matches[0], (repo_root / matches[0]).is_file()

    # 4. Search under repo_root if not found in known_files
    hits = list(repo_root.glob(f"**/{raw_path.name}"))
    hits = [h for h in hits if not any(part in (".git", ".venv", "node_modules", "retired") for part in h.parts)]
    if len(hits) == 1:
        return _norm_posix(hits[0].relative_to(repo_root)), True

    # If file was deleted or unresolvable
    return norm_arg, False


def get_git_diff_records(
    repo_root: Path,
    ref: str | None,
    staged: bool,
    pathspecs: list[str],
) -> tuple[list[tuple[str, str]], str | None]:
    """Extract git diff files with statuses. Returns ([(status, path), ...], error_msg)."""
    records: list[tuple[str, str]] = []

    # 1. Run git diff
    diff_cmd = ["git", "diff", "--name-status", "-M", "-z"]
    if staged:
        diff_cmd.append("--cached")
    elif ref:
        diff_cmd.append(ref)
    else:
        diff_cmd.append("HEAD")

    if pathspecs:
        diff_cmd.append("--")
        diff_cmd.extend(pathspecs)

    try:
        proc = subprocess.run(
            diff_cmd,
            cwd=repo_root,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
        if proc.returncode != 0:
            err = proc.stderr.decode("utf-8", errors="ignore").strip()
            return [], f"git diff failed (code {proc.returncode}): {err}"
    except Exception as exc:
        return [], f"Failed to execute git: {exc}"

    tokens = proc.stdout.split(b"\x00")
    idx = 0
    while idx < len(tokens):
        tok = tokens[idx].decode("utf-8", errors="ignore").strip()
        if not tok:
            idx += 1
            continue
        status_char = tok[0]
        if status_char in ("R", "C") and idx + 2 < len(tokens):
            old_p = tokens[idx + 1].decode("utf-8", errors="ignore")
            new_p = tokens[idx + 2].decode("utf-8", errors="ignore")
            records.append(("D", old_p))
            records.append(("A", new_p))
            idx += 3
        elif idx + 1 < len(tokens):
            path_val = tokens[idx + 1].decode("utf-8", errors="ignore")
            records.append((status_char, path_val))
            idx += 2
        else:
            idx += 1

    # 2. If not staged, also include untracked files
    if not staged:
        ls_cmd = ["git", "ls-files", "-o", "--exclude-standard", "-z"]
        if pathspecs:
            ls_cmd.append("--")
            ls_cmd.extend(pathspecs)
        try:
            ls_proc = subprocess.run(
                ls_cmd,
                cwd=repo_root,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            if ls_proc.returncode == 0:
                for ut in ls_proc.stdout.split(b"\x00"):
                    ut_str = ut.decode("utf-8", errors="ignore").strip()
                    if ut_str:
                        records.append(("?", ut_str))
        except Exception:
            pass

    return records, None


def summarize_reason(why: tuple[str, ...]) -> str:
    """Summarize shortest reason chain for concise one-line output."""
    if not why:
        return ""
    if len(why) == 1:
        return f"({why[0]})"

    # Check for direct include
    first, last = why[0], why[-1]
    first_name = Path(first).name
    if len(why) == 2:
        if Path(first).stem == Path(last).stem or first in last:
            return f"(self: includes {first_name})"
        return f"(includes {first_name})"

    # Check for colocation
    for part in why:
        if "colocated in" in part:
            return f"({part})"
        if "library consumer:" in part:
            return f"({part})"
        if "covers:" in part:
            return f"({part})"

    return f"(includes {first_name})"


def print_plan(plan: Plan, why_mode: bool = False) -> None:
    """Print the plan actions and reasons matching doc 22 layout."""
    for t in plan.touched:
        print(f"touched: {t}")

    # 1. BUILD items
    build_mods = plan.builds()
    if build_mods:
        mod_args = " ".join(f"--mod {m}" for m in build_mods)
        # Summarize reason from first build item
        build_items = [it for it in plan.items if it.kind == "build"]
        build_reason = summarize_reason(build_items[0].why) if build_items else ""
        print(f"BUILD   build_check {mod_args}   {build_reason}")

    # 2. TEST items
    test_items = [it for it in plan.items if it.kind == "test"]
    for it in test_items:
        reason_str = summarize_reason(it.why)
        print(f"TEST    {it.id:<22} {reason_str}")

    # 3. SUITE items
    suite_items = [it for it in plan.items if it.kind == "suite"]
    if suite_items:
        s_list = ", ".join(it.id for it in suite_items)
        print(f"SUITE   {s_list}")

    # 4. STATIC items
    static_items = [it for it in plan.items if it.kind == "static"]
    if static_items:
        st_list = ", ".join(it.id for it in static_items)
        print(f"STATIC  {st_list}")

    # 5. WINE items
    wine_items = [it for it in plan.items if it.kind == "wine"]
    if wine_items:
        w_list = ", ".join(it.id for it in wine_items)
        print(f"WINE    {w_list}")

    # 6. KPM consumers
    if plan.kpm_consumers:
        kpm_str = " ".join(plan.kpm_consumers)
        print(f"KPM     consumers after sync: {kpm_str}")

    # 7. GAPs
    for g in plan.gaps:
        print(f"GAP     {g}")

    # 8. UNCLASSIFIED
    for u in plan.unclassified:
        print(f"UNCLASSIFIED {u}")

    # 9. INFO notes
    for inf in plan.info_notes:
        print(inf)

    # Detailed --why output
    if why_mode:
        print("\n--- Edge Chains (--why) ---")
        for it in plan.items:
            chain_str = " -> ".join(it.why)
            print(f"  {it.kind.upper():<6} {it.id}: {chain_str}")


def run_lint(repo_root: Path, graph: CompatGraph) -> tuple[int, list[str]]:
    """Run lint checks (doc 22 section U7 / --lint). Returns (exit_code, warnings)."""
    warnings: list[str] = []

    # 1. Stale covers check
    compat_toml_path = repo_root / "tests/compat.toml"
    if compat_toml_path.is_file():
        try:
            data = tomllib.loads(compat_toml_path.read_text(encoding="utf-8"))
            for cov in data.get("covers", []):
                for p in cov.get("paths", []):
                    # Check if pattern matches at least one file
                    matches = list(repo_root.glob(p))
                    if not matches:
                        warnings.append(f"WARN stale covers pattern in tests/compat.toml: {p}")
        except Exception as exc:
            warnings.append(f"WARN unreadable tests/compat.toml: {exc}")

    # 2. Check test.toml covers
    pm_root = repo_root / "patch_manager_mods"
    if pm_root.is_dir():
        for root, dirs, files in os.walk(pm_root):
            if "test.toml" in files:
                tpath = Path(root) / "test.toml"
                try:
                    data = tomllib.loads(tpath.read_text(encoding="utf-8"))
                    for key, val in data.items():
                        covers_list: list[str] = []
                        if isinstance(val, dict) and "covers" in val:
                            c = val["covers"]
                            covers_list = [c] if isinstance(c, str) else list(c)
                        elif key == "covers" and isinstance(val, list):
                            covers_list = val
                        for pat in covers_list:
                            if "@" in pat:
                                # Interface string name@major
                                continue
                            pat_path = repo_root / pat if not (Path(root) / pat).exists() else Path(root) / pat
                            matches = list(repo_root.glob(pat)) or list(Path(root).glob(pat))
                            if not matches and not pat_path.exists():
                                warnings.append(f"WARN stale covers in {tpath}: {pat}")
                except Exception:
                    pass

    # 3. Contract test gaps
    for mod in sorted(graph.mod_dirs):
        if not graph.mod_has_contract_test.get(mod, False):
            # Check if mod includes any common headers
            includes_common = False
            for cpp_f in graph.all_cpp_files:
                if cpp_f.startswith(f"patch_manager_mods/{mod}/") and not cpp_f.endswith("_test.cpp"):
                    # Check reverse includes of common headers
                    for sh in (
                        "patch_manager_mods/_shared_src/SafeRead.h",
                        "patch_manager_mods/_shared_src/K2Addr.h",
                        "patch_manager_mods/_shared_src/GameAddr.h",
                        "patch_manager_mods/_shared_src/Gui2D.h",
                        "patch_manager_mods/_shared_src/PortraitSlots.h",
                    ):
                        for inc_file, _ in graph.reverse_includes.get(sh, []):
                            if inc_file == cpp_f:
                                includes_common = True
                                break
                    if includes_common:
                        break
            if includes_common:
                warnings.append(f"GAP {mod} includes Common headers but owns no common_contract_test.cpp (doc 20 4.1)")

    # 4. Interface provides vs header check
    for mod, minfo in graph.manifests.items():
        provides = minfo.get("provides", [])
        for iface in provides:
            # Match name@major
            if "@" in iface:
                iname, imajor = iface.split("@", 1)
                # Check headers in _shared_src or mod
                hdr_path = repo_root / "patch_manager_mods/_shared_src/PortraitSlots.h"
                if hdr_path.is_file():
                    content = hdr_path.read_text(encoding="utf-8", errors="ignore")
                    if iname not in content:
                        warnings.append(f"WARN interface {iface} provided by {mod} not found in header")

    return 0, warnings


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Update compatibility check for KOTOR-II patch manager mods (Doc 22).",
    )
    parser.add_argument(
        "files",
        nargs="*",
        help="Touched files to check (e.g. patch_manager_mods/_shared_src/Gui2D.h or Gui2D.h).",
    )
    parser.add_argument(
        "--since",
        nargs="?",
        const="HEAD",
        default=None,
        help="Git diff against REF (default HEAD) plus untracked files.",
    )
    parser.add_argument(
        "--staged",
        action="store_true",
        help="Check staged git changes only.",
    )
    parser.add_argument(
        "--list",
        action="store_true",
        help="Print the execution plan, run nothing.",
    )
    parser.add_argument(
        "--why",
        action="store_true",
        help="Print edge chain reasons per selected item (implies plan print).",
    )
    parser.add_argument(
        "--json",
        nargs="?",
        const="-",
        default=None,
        metavar="PATH",
        help="Export machine-readable plan and results JSON (to PATH or stdout).",
    )
    parser.add_argument(
        "--wine",
        action="store_true",
        help="Include affected wine smoke tests.",
    )
    parser.add_argument(
        "--lint",
        action="store_true",
        help="Run lint mode for covers staleness, contract gaps, and interface definitions.",
    )
    parser.add_argument(
        "--all",
        action="store_true",
        help="Escape hatch: run all tests and builds via run_tests.py --build.",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Fail (exit 3) if UNCLASSIFIED files, GAPs or unverified items are present.",
    )
    parser.add_argument(
        "--jobs",
        "-j",
        type=int,
        default=None,
        help="Number of parallel jobs for builds and tests.",
    )
    parser.add_argument(
        "--no-sanitize",
        action="store_true",
        help="Disable sanitizers in test runner.",
    )
    parser.add_argument(
        "--shared-tmp",
        action="store_true",
        help="Use shared system TMPDIR instead of private TMPDIR=<tmp>/compat-<pid>.",
    )
    parser.add_argument(
        "--yes",
        "-y",
        action="store_true",
        help="Bypass safety guardrail when >25 files are touched under --since.",
    )
    parser.add_argument(
        "--kpm-dir",
        type=Path,
        default=None,
        help="Path to KPM repository.",
    )
    parser.add_argument(
        "--repo",
        type=Path,
        default=None,
        help="Path to KOTOR-II repository.",
    )

    # Separate pathspecs after -- if provided
    raw_argv = sys.argv[1:]
    pathspecs: list[str] = []
    if "--" in raw_argv:
        dash_idx = raw_argv.index("--")
        pathspecs = raw_argv[dash_idx + 1 :]
        raw_argv = raw_argv[:dash_idx]

    args = parser.parse_args(raw_argv)

    repo_root = (args.repo or find_repo_root()).resolve()

    # Build dependency graph
    graph = CompatGraph(repo_root=repo_root, kpm_dir=args.kpm_dir)

    # Lint mode
    if args.lint:
        code, warnings = run_lint(repo_root, graph)
        for w in warnings:
            print(w)
        if args.strict and warnings:
            return 3
        return 0

    # Escape hatch: --all
    if args.all:
        cmd = [sys.executable, str(repo_root / "scripts/run_tests.py"), "--build"]
        if args.jobs:
            cmd += ["-j", str(args.jobs)]
        if args.no_sanitize:
            cmd.append("--no-sanitize")
        if args.wine:
            cmd.append("--wine")
        res = subprocess.run(cmd, cwd=repo_root)
        return res.returncode

    # Collect touched files
    touched_records: list[dict[str, str]] = []  # [{"path": ..., "status": ...}]
    unresolved_positionals: list[str] = []

    # 1. From git diff (--since or --staged)
    if args.since is not None or args.staged:
        records, err = get_git_diff_records(repo_root, args.since, args.staged, pathspecs)
        if err:
            sys.stderr.write(f"Error: {err}\n")
            return 2
        for status, p in records:
            touched_records.append({"path": _norm_posix(p), "status": status})

        # Guardrail: dirty worktree safety check
        if len(touched_records) > 25 and not args.yes:
            sys.stderr.write(
                f"Error: {len(touched_records)} files touched since {args.since or 'HEAD'}.\n"
                "The shared worktree has uncommitted files from other sessions.\n"
                "Refuse to run without --yes, or restrict pathspec with '-- <pathspec>'.\n"
            )
            return 2

    # 2. From positional file arguments
    for raw_f in args.files:
        norm_p, exists = resolve_file_arg(repo_root, raw_f, graph.all_cpp_files | graph.all_py_files)
        status = "M" if exists else "D"
        touched_records.append({"path": norm_p, "status": status})

    if not touched_records:
        if args.since is None and not args.staged and not args.files:
            sys.stderr.write("No touched files specified. Provide file paths or use --since / --staged.\n")
            return 2
        print("No touched files found.")
        return 0

    # Deduplicate touched paths preserving order
    seen_paths: set[str] = set()
    unique_touched_records: list[dict[str, str]] = []
    touched_paths: list[str] = []
    for rec in touched_records:
        p = rec["path"]
        if p not in seen_paths:
            seen_paths.add(p)
            unique_touched_records.append(rec)
            touched_paths.append(p)

    # Derive affected plan
    plan = graph.affected(touched_paths, include_wine=args.wine)

    # If --why or --list, print plan
    if args.list or args.why:
        print_plan(plan, why_mode=args.why)
        if args.json:
            json_data = {
                "touched": unique_touched_records,
                "plan": [
                    {
                        "kind": it.kind,
                        "id": it.id,
                        "why": list(it.why),
                        "cmd": list(it.cmd),
                    }
                    for it in plan.items
                ],
                "gaps": plan.gaps,
                "unclassified": plan.unclassified,
                "results": [],
                "counts": {"PASS": 0, "FAIL": 0, "SKIP": 0, "ERROR": 0},
                "wall_seconds": 0.0,
            }
            json_str = json.dumps(json_data, indent=2)
            if args.json == "-":
                print(json_str)
            else:
                Path(args.json).write_text(json_str, encoding="utf-8")

        if args.strict and (plan.unclassified or plan.gaps):
            return 3
        return 0

    # Print plan header before execution
    print_plan(plan, why_mode=False)

    # Set up private TMPDIR unless --shared-tmp
    private_tmpdir: str | None = None
    exec_env = os.environ.copy()
    if not args.shared_tmp:
        private_tmpdir = tempfile.mkdtemp(prefix=f"compat-{os.getpid()}-", dir=tempfile.gettempdir())
        exec_env["TMPDIR"] = private_tmpdir

    start_wall = time.time()
    all_results: list[dict[str, Any]] = []
    counts = {"PASS": 0, "FAIL": 0, "SKIP": 0, "ERROR": 0}
    breaking_errors: list[str] = []

    try:
        # Separate test and build items
        test_suite_ids = [it.id for it in plan.items if it.kind in ("test", "suite", "wine", "static")]
        build_mods = plan.builds()

        # 1. Run tests and suites via run_tests.py
        if test_suite_ids:
            run_tests_script = repo_root / "scripts/run_tests.py"
            test_cmd = [sys.executable, str(run_tests_script), "--build"]
            if args.jobs:
                test_cmd += ["-j", str(args.jobs)]
            if args.no_sanitize:
                test_cmd.append("--no-sanitize")
            if args.wine:
                test_cmd.append("--wine")

            with tempfile.NamedTemporaryFile("w+", suffix=".json", delete=False) as tf:
                json_tmp_path = tf.name

            # Pass IDs via --ids-file
            with tempfile.NamedTemporaryFile("w+", suffix=".txt", delete=False) as id_tf:
                id_tf.write("\n".join(test_suite_ids) + "\n")
                ids_file_path = id_tf.name

            test_cmd += ["--ids-file", ids_file_path, "--json", json_tmp_path]

            try:
                proc = subprocess.run(
                    test_cmd,
                    cwd=repo_root,
                    env=exec_env,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    text=True,
                    check=False,
                )
                # Parse runner output JSON
                if Path(json_tmp_path).is_file():
                    try:
                        runner_json = json.loads(Path(json_tmp_path).read_text(encoding="utf-8"))
                        for r in runner_json.get("results", []):
                            all_results.append(r)
                            st = r.get("status", "PASS")
                            counts[st] = counts.get(st, 0) + 1

                            # Check for contract test breakage
                            if st in ("FAIL", "ERROR") and ("contract_test" in r.get("id", "") or "contract_test" in r.get("src", "")):
                                tail_txt = r.get("tail", "")
                                relies_match = re.search(r"(([a-zA-Z0-9_\-]+)\s+relies on:[^\r\n]*)", tail_txt)
                                if relies_match:
                                    relies_line = relies_match.group(1).strip()
                                    mod_name = relies_match.group(2).strip()
                                    breaking_errors.append(f"BREAKING for {mod_name}: {relies_line}")
                                else:
                                    relies_generic = re.search(r"([^\r\n]*relies on:[^\r\n]*)", tail_txt)
                                    if relies_generic:
                                        relies_line = relies_generic.group(1).strip()
                                        mod_match = re.search(r"patch_manager_mods/([^/]+)/", r.get("src", "") or r.get("id", ""))
                                        mod_name = mod_match.group(1).replace("_src", "") if mod_match else "unknown"
                                        breaking_errors.append(f"BREAKING for {mod_name}: {relies_line}")
                    except Exception:
                        pass
                else:
                    if proc.returncode != 0:
                        counts["ERROR"] = counts.get("ERROR", 0) + len(test_suite_ids)
            finally:
                if os.path.exists(json_tmp_path):
                    os.unlink(json_tmp_path)
                if os.path.exists(ids_file_path):
                    os.unlink(ids_file_path)

        # 2. Run builds via build_check.py
        if build_mods:
            build_check_script = repo_root / "scripts/build_check.py"
            b_cmd = [sys.executable, str(build_check_script)]
            for m in build_mods:
                b_cmd += ["--mod", m]
            if args.jobs:
                b_cmd += ["--jobs", str(args.jobs)]
            if args.kpm_dir:
                b_cmd += ["--kpm-dir", str(args.kpm_dir)]

            b_proc = subprocess.run(
                b_cmd,
                cwd=repo_root,
                env=exec_env,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                check=False,
            )

            # Parse build_check output lines
            out_lines = b_proc.stdout.splitlines()
            for line in out_lines:
                m_pass = re.search(
                    r"PASS\s+build:\s+([^\s:]+)(?:\s+\([^)]+\))?:\s*(.*?)(?:,\s*([0-9.]+)s)?$",
                    line,
                )
                m_fail = re.search(r"FAIL\s+build:\s+([^\s:]+):\s*(.*)$", line)
                if m_pass:
                    mod_id = m_pass.group(1)
                    sec = float(m_pass.group(3)) if m_pass.group(3) else 0.0
                    all_results.append(
                        {
                            "id": f"build_check:{mod_id}",
                            "kind": "build",
                            "status": "PASS",
                            "seconds": sec,
                            "reason": "",
                            "tail": m_pass.group(2),
                        }
                    )
                    counts["PASS"] = counts.get("PASS", 0) + 1
                elif m_fail:
                    mod_id = m_fail.group(1)
                    all_results.append(
                        {
                            "id": f"build_check:{mod_id}",
                            "kind": "build",
                            "status": "FAIL",
                            "seconds": 0.0,
                            "reason": "",
                            "tail": m_fail.group(2),
                        }
                    )
                    counts["FAIL"] = counts.get("FAIL", 0) + 1

    finally:
        if private_tmpdir and os.path.isdir(private_tmpdir):
            shutil.rmtree(private_tmpdir, ignore_errors=True)

    wall_seconds = round(time.time() - start_wall, 2)

    # Print breaking messages if any
    for b_err in breaking_errors:
        print(b_err)

    # Print RESULT line
    pass_cnt = counts.get("PASS", 0)
    fail_cnt = counts.get("FAIL", 0)
    skip_cnt = counts.get("SKIP", 0)
    err_cnt = counts.get("ERROR", 0)

    print(
        f"RESULT  PASS {pass_cnt}  FAIL {fail_cnt + err_cnt}  SKIP {skip_cnt}   "
        f"(unverified: {skip_cnt})   wall {wall_seconds:.1f} s"
    )

    # Handle --json output
    if args.json:
        json_data = {
            "touched": unique_touched_records,
            "plan": [
                {
                    "kind": it.kind,
                    "id": it.id,
                    "why": list(it.why),
                    "cmd": list(it.cmd),
                }
                for it in plan.items
            ],
            "gaps": plan.gaps,
            "unclassified": plan.unclassified,
            "results": all_results,
            "counts": counts,
            "wall_seconds": wall_seconds,
        }
        json_str = json.dumps(json_data, indent=2)
        if args.json == "-":
            print(json_str)
        else:
            Path(args.json).write_text(json_str, encoding="utf-8")

    # Exit codes:
    # 1: Any FAIL or ERROR
    if fail_cnt > 0 or err_cnt > 0:
        return 1

    # 3: --strict only: UNCLASSIFIED files, GAPs or unverified items present
    if args.strict and (plan.unclassified or plan.gaps or skip_cnt > 0):
        return 3

    return 0


if __name__ == "__main__":
    sys.exit(main())
