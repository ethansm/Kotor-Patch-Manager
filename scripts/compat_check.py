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
        if "provider consumer:" in part:
            return f"({part})"
        if "covers:" in part:
            return f"({part})"

    return f"(includes {first_name})"


HUNK_HEADER_RE = re.compile(r"^@@\s+-(\d+)(?:,(\d+))?\s+\+(\d+)(?:,(\d+))?\s+@@")


def find_k2addr_gog_range(content: str) -> tuple[int, int] | None:
    """Find 1-based (start_line, end_line) of the makeGog/kGog block in K2Addr.h."""
    lines = content.splitlines()
    start_line = None
    end_line = None
    for idx, line in enumerate(lines, 1):
        if start_line is None and ("makeGog()" in line and "constexpr" in line and "kGog" not in line):
            start_line = idx
        if start_line is not None and ("kGog =" in line or "kGog=" in line):
            end_line = idx
            break
    if start_line is not None and end_line is not None:
        return start_line, end_line
    return None


def is_k2addr_gog_only_diff(
    diff_text: str,
    base_content: str,
    new_content: str,
) -> bool:
    """Check if all hunks in a unified diff for K2Addr.h touch only the kGog initializer (Doc 22 U11)."""
    if not diff_text.strip():
        return False

    base_range = find_k2addr_gog_range(base_content)
    new_range = find_k2addr_gog_range(new_content)
    if not base_range or not new_range:
        return False

    base_start, base_end = base_range
    new_start, new_end = new_range

    hunk_count = 0
    for line in diff_text.splitlines():
        m = HUNK_HEADER_RE.match(line)
        if not m:
            continue
        hunk_count += 1
        old_s = int(m.group(1))
        old_c = int(m.group(2)) if m.group(2) is not None else 1
        new_s = int(m.group(3))
        new_c = int(m.group(4)) if m.group(4) is not None else 1

        old_range = (old_s, old_s + old_c - 1) if old_c > 0 else (old_s, old_s)
        new_range_hunk = (new_s, new_s + new_c - 1) if new_c > 0 else (new_s, new_s)

        if not (base_start <= old_range[0] and old_range[1] <= base_end):
            return False
        if not (new_start <= new_range_hunk[0] and new_range_hunk[1] <= new_end):
            return False

    return hunk_count > 0


def get_k2addr_git_state(
    repo_root: Path,
    ref: str | None = None,
    staged: bool = False,
) -> tuple[str, str, str]:
    """Extract (diff_text, base_content, new_content) for K2Addr.h from git."""
    k2addr_rel = "patch_manager_mods/_shared_src/K2Addr.h"
    k2addr_path = repo_root / k2addr_rel

    new_content = ""
    if staged:
        try:
            sproc = subprocess.run(
                ["git", "show", f":{k2addr_rel}"],
                cwd=repo_root,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=False,
            )
            if sproc.returncode == 0:
                new_content = sproc.stdout
        except Exception:
            pass
    if not new_content and k2addr_path.is_file():
        try:
            new_content = k2addr_path.read_text(encoding="utf-8", errors="ignore")
        except Exception:
            pass

    base_ref = ref if ref else "HEAD"
    base_content = ""
    try:
        sproc = subprocess.run(
            ["git", "show", f"{base_ref}:{k2addr_rel}"],
            cwd=repo_root,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        if sproc.returncode == 0:
            base_content = sproc.stdout
    except Exception:
        pass

    diff_cmd = ["git", "diff", "-U0"]
    if staged:
        diff_cmd.append("--cached")
    elif ref:
        diff_cmd.append(ref)
    else:
        diff_cmd.append("HEAD")
    diff_cmd.extend(["--", k2addr_rel])

    diff_text = ""
    try:
        dproc = subprocess.run(
            diff_cmd,
            cwd=repo_root,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        if dproc.returncode == 0:
            diff_text = dproc.stdout
    except Exception:
        pass

    return diff_text, base_content, new_content


def narrow_plan_to_gog_scoping(plan: Plan) -> None:
    """Narrow plan for GOG-only K2Addr.h changes: cpp:k2addr_test + compile-only builds."""
    narrowed_items = []
    has_k2addr_test = False
    for it in plan.items:
        if it.kind == "build":
            narrowed_items.append(it)
        elif it.kind == "test" and it.id == "cpp:k2addr_test":
            narrowed_items.append(it)
            has_k2addr_test = True

    if not has_k2addr_test:
        narrowed_items.append(
            PlanItem(
                kind="test",
                id="cpp:k2addr_test",
                why=("GOG-only: k2addr_test",),
            )
        )

    plan.items = narrowed_items
    plan.gaps.clear()
    plan.info_notes.append("GOG-only: Steam runtime unchanged")


def check_stale_wine_smokes(plan: Plan, repo_root: Path, kpm_dir: Path) -> tuple[dict[str, str], list[str]]:
    """Check wine smokes in plan. If dll_from != 'build' and kpatch is older than newest affected source,
    return (stale_messages_by_item_id, list_of_stale_item_ids).
    """
    smokes_toml = repo_root / "patch_manager_mods/perf_lab_src/harness/smokes.toml"
    if not smokes_toml.is_file():
        return {}, []

    try:
        cfg = tomllib.loads(smokes_toml.read_text(encoding="utf-8"))
    except Exception:
        return {}, []

    smokes_by_name = {s.get("name"): s for s in cfg.get("smoke", []) if s.get("name")}

    mods_by_stem = {}
    try:
        import _static_common as sc
        mods = sc.load_mods(repo_root, kpm_dir, variants=True)
        for m in mods:
            p_name = (m.kpm_dir.name if m.kpm_dir else m.dir.name).lower()
            mods_by_stem[p_name] = m
            mods_by_stem[m.dir.name.lower()] = m
            mods_by_stem[m.id.replace("-", "").lower()] = m
    except Exception:
        pass

    stale_map: dict[str, str] = {}
    stale_ids: list[str] = []

    for it in plan.items:
        if it.kind != "wine":
            continue
        sm_name = it.id.split(":", 1)[1].split("[")[0]
        s = smokes_by_name.get(sm_name)
        if not s:
            continue
        dll_from = s.get("dll_from", "kpatch")
        if dll_from == "build":
            continue

        kpatch_rel = s.get("kpatch", "")
        if not kpatch_rel:
            continue
        kpatch_path = kpm_dir / "Patches" / kpatch_rel

        # Resolve mod directory
        mod_dir = None
        if s.get("mod"):
            p = Path(s["mod"])
            mod_dir = p if p.is_absolute() else repo_root / "patch_manager_mods" / p
        else:
            stem = Path(kpatch_rel).stem.lower()
            parent = Path(kpatch_rel).parent.name.lower()
            m = mods_by_stem.get(stem) or mods_by_stem.get(parent)
            if m:
                mod_dir = m.dir
            else:
                for d in (repo_root / "patch_manager_mods").iterdir():
                    if d.is_dir():
                        clean = d.name.replace("_src", "").replace("-", "").lower()
                        if clean and (clean in stem or stem in clean):
                            mod_dir = d
                            break

        # Collect candidate source files affecting this smoke/mod
        candidate_sources: list[Path] = []
        for w in it.why:
            wp = repo_root / w
            if wp.is_file():
                candidate_sources.append(wp)
            elif (repo_root / "patch_manager_mods" / w).is_file():
                candidate_sources.append(repo_root / "patch_manager_mods" / w)

        if mod_dir and mod_dir.is_dir():
            for p in mod_dir.rglob("*"):
                if p.is_file() and p.suffix.lower() in (".cpp", ".h", ".hpp", ".inc", ".def"):
                    candidate_sources.append(p)

        if not candidate_sources:
            continue

        newest_source = max(candidate_sources, key=lambda p: p.stat().st_mtime)
        newest_mtime = newest_source.stat().st_mtime

        kpatch_mtime = kpatch_path.stat().st_mtime if kpatch_path.is_file() else -1.0
        if kpatch_mtime < newest_mtime:
            try:
                src_rel = _norm_posix(newest_source.relative_to(repo_root))
            except ValueError:
                src_rel = _norm_posix(newest_source)
            msg = f"STALE: {kpatch_rel} older than {src_rel}"
            stale_map[it.id] = msg
            stale_ids.append(it.id)

    return stale_map, stale_ids


def known_run_test_ids(repo_root: Path) -> set[str]:
    """Item ids `run_tests.py --list` prints (first token per line); empty set if the listing fails."""
    proc = subprocess.run([sys.executable, str(repo_root / "scripts" / "run_tests.py"), "--list"], cwd=repo_root,
                          capture_output=True, text=True, check=False)
    if proc.returncode != 0:
        return set()
    return {ln.split()[0] for ln in proc.stdout.splitlines() if ln.strip()}


def print_plan(plan: Plan, why_mode: bool = False, repo_root: Path | None = None, kpm_dir: Path | None = None) -> None:
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
        if repo_root and kpm_dir:
            stale_map, _ = check_stale_wine_smokes(plan, repo_root, kpm_dir)
            seen_msgs = set()
            for it in wine_items:
                if it.id in stale_map and stale_map[it.id] not in seen_msgs:
                    print(stale_map[it.id])
                    seen_msgs.add(stale_map[it.id])

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


def run_lint(
    repo_root: Path,
    graph: CompatGraph,
    touched_files: list[str] | None = None,
) -> tuple[int, list[str]]:
    """Run lint checks (doc 22 section U7 / --lint). Returns (exit_code, warnings).

    Checks:
    1. Stale covers patterns in tests/compat.toml and test.toml files.
    2. Stale interface strings in covers (interfaces not provided by any mod or header).
    3. Contract-test gaps: mods including Common headers without a *_contract_test.cpp.
    4. Interface provides vs header string: manifest provides == <PREFIX>_IFACE@<PREFIX>_VERSION in header.
    5. Manifests with requires_interfaces but no covering test (test with covers naming the interface).
    6. Unreached mods: mods with no reaching test and no smoke (GAP), and mods with only wine smokes (WARN).
    7. UNCLASSIFIED file kinds under patch_manager_mods/ or passed files.
    """
    warnings: list[str] = []

    # 1. Stale covers check in tests/compat.toml
    compat_toml_path = repo_root / "tests/compat.toml"
    if compat_toml_path.is_file():
        try:
            data = tomllib.loads(compat_toml_path.read_text(encoding="utf-8"))
            for cov in data.get("covers", []):
                for p in cov.get("paths", []):
                    matches = list(repo_root.glob(p))
                    if not matches:
                        warnings.append(f"WARN stale covers pattern in tests/compat.toml: {p}")
        except Exception as exc:
            warnings.append(f"WARN unreadable tests/compat.toml: {exc}")

    # 2. Header interfaces (<PREFIX>_IFACE and <PREFIX>_VERSION)
    candidate_headers = list((repo_root / "patch_manager_mods/_shared_src").glob("*.h"))
    for mod_dir in graph.mod_dirs:
        candidate_headers.extend((repo_root / f"patch_manager_mods/{mod_dir}").glob("*.h"))

    iface_pattern = re.compile(r'#define\s+([A-Z0-9_]+)_IFACE\s+"([^"]+)"')
    ver_pattern = re.compile(r'#define\s+([A-Z0-9_]+)_VERSION\s+(\d+)')

    header_interfaces: dict[str, list[Path]] = {}
    header_definitions: list[tuple[str, str, Path]] = []

    for hdr in candidate_headers:
        try:
            content = hdr.read_text(encoding="utf-8", errors="ignore")
        except Exception:
            continue
        ifaces = dict(iface_pattern.findall(content))
        versions = dict(ver_pattern.findall(content))
        for pfx, iname in ifaces.items():
            ver = versions.get(pfx, "1")
            full_iface = f"{iname}@{ver}"
            header_interfaces.setdefault(full_iface, []).append(hdr)
            header_definitions.append((full_iface, pfx, hdr))

    all_provided_interfaces = set(header_interfaces.keys())
    for mod, minfo in graph.manifests.items():
        for iface in minfo.get("provides", []):
            all_provided_interfaces.add(iface)

    # 3. Check test.toml covers (paths and interface strings)
    covered_interfaces_by_mod: dict[str, set[str]] = {}
    all_covered_interfaces: set[str] = set()

    pm_root = repo_root / "patch_manager_mods"
    if pm_root.is_dir():
        for root, dirs, files in os.walk(pm_root):
            if "test.toml" in files:
                tpath = Path(root) / "test.toml"
                rel_tpath = _norm_posix(tpath.relative_to(repo_root))
                mod_dir = graph._find_mod_dir(tpath)
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
                                if mod_dir:
                                    covered_interfaces_by_mod.setdefault(mod_dir, set()).add(pat)
                                all_covered_interfaces.add(pat)
                                if pat not in all_provided_interfaces:
                                    warnings.append(f"WARN stale covers interface in {rel_tpath}: {pat}")
                            else:
                                pat_path = repo_root / pat if not (Path(root) / pat).exists() else Path(root) / pat
                                matches = list(repo_root.glob(pat)) or list(Path(root).glob(pat))
                                if not matches and not pat_path.exists():
                                    warnings.append(f"WARN stale covers in {rel_tpath}: {pat}")
                except Exception as exc:
                    warnings.append(f"WARN unreadable {rel_tpath}: {exc}")

    # 4. Interface provides vs header check
    for mod, minfo in sorted(graph.manifests.items()):
        provides = minfo.get("provides", [])
        for iface in provides:
            if iface not in header_interfaces:
                warnings.append(f"WARN interface {iface} provided by {mod} not found in header")

    all_manifest_provides = set()
    for mod, minfo in graph.manifests.items():
        all_manifest_provides.update(minfo.get("provides", []))
    for full_iface, pfx, hdr in header_definitions:
        if full_iface not in all_manifest_provides:
            rel_hdr = _norm_posix(hdr.relative_to(repo_root))
            warnings.append(f"WARN interface {full_iface} defined in {rel_hdr} is not provided by any manifest")

    # 5. Contract test gaps (mods including Common headers without a contract test)
    common_targets = set()
    for target in graph.reverse_includes:
        if target.startswith("patch_manager_mods/_shared_src/") or target.startswith("kpm:Patches/Common/"):
            common_targets.add(target)

    visited = set(common_targets)
    queue = list(common_targets)
    while queue:
        curr = queue.pop(0)
        for inc, _ in graph.reverse_includes.get(curr, []):
            if inc not in visited:
                visited.add(inc)
                queue.append(inc)

    mods_including_common = set()
    for f in visited:
        if f.endswith("_test.cpp"):
            continue
        if f.startswith("patch_manager_mods/"):
            parts = f.split("/")
            if len(parts) >= 2:
                candidate_mod = parts[1]
                if candidate_mod in graph.mod_dirs:
                    mods_including_common.add(candidate_mod)

    for mod in sorted(graph.mod_dirs):
        if mod in mods_including_common and not graph.mod_has_contract_test.get(mod, False):
            warnings.append(f"GAP {mod} includes Common headers but owns no *_contract_test.cpp (doc 20 4.1)")

    # 6. Manifests with requires_interfaces but no covering test
    for mod, minfo in sorted(graph.manifests.items()):
        requires = minfo.get("requires_interfaces", [])
        mod_covered = covered_interfaces_by_mod.get(mod, set())
        for iface in requires:
            if iface not in mod_covered and iface not in all_covered_interfaces:
                warnings.append(f"GAP {mod} requires interface {iface} but has no test covering it")
            elif iface not in mod_covered:
                warnings.append(f"GAP {mod} requires interface {iface} but owns no test covering it")

    # 7. Mods with no reaching test and no smoke
    for mod in sorted(graph.mod_dirs):
        mod_sources = set()
        for cpp in graph.all_cpp_files:
            if cpp.startswith(f"patch_manager_mods/{mod}/") and not cpp.endswith("_test.cpp"):
                mod_sources.add(cpp)
        for h in (repo_root / f"patch_manager_mods/{mod}").glob("*.h"):
            mod_sources.add(_norm_posix(h.relative_to(repo_root)))

        reachers = set()
        for src in mod_sources:
            for inc_file, _ in graph.reverse_includes.get(src, []):
                if inc_file.endswith("_test.cpp") and not inc_file.startswith(f"patch_manager_mods/{mod}/common_contract_test"):
                    reachers.add(inc_file)

        covers_tests = set()
        for src in mod_sources:
            for pat, item_id in graph.covers_map:
                if fnmatch.fnmatch(src, pat) or src == pat:
                    covers_tests.add(item_id)

        has_reaching_test = bool(reachers or covers_tests)
        smokes = graph.mod_to_smokes.get(mod, [])

        if not has_reaching_test and not smokes:
            warnings.append(f"GAP {mod} has no reaching test and no smoke")
        elif not has_reaching_test:
            warnings.append(f"WARN {mod} has no reaching host test (smoke only: {', '.join(sorted(smokes))})")

    # 8. Check unclassified touched files if provided
    if touched_files:
        for t in touched_files:
            if not graph._is_classified(t, Plan()):
                warnings.append(f"WARN UNCLASSIFIED file: {t}")

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
        touched: list[str] = []
        if args.files:
            for f in args.files:
                resolved, _ = resolve_file_arg(repo_root, f, graph.all_cpp_files)
                touched.append(resolved)
        code, warnings = run_lint(repo_root, graph, touched_files=touched if touched else None)
        if args.json:
            payload = {
                "lint": True,
                "warnings": [w for w in warnings if w.startswith("WARN")],
                "gaps": [w for w in warnings if w.startswith("GAP")],
                "all": warnings,
            }
            if args.json == "-":
                print(json.dumps(payload, indent=2))
            else:
                Path(args.json).write_text(json.dumps(payload, indent=2), encoding="utf-8")
        else:
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

    # Version scoping (Doc 22 U11): GOG-only K2Addr.h narrowing
    is_k2addr_touched = any(p.endswith("K2Addr.h") for p in touched_paths)
    other_touched = [p for p in touched_paths if not p.endswith("K2Addr.h") and not p.endswith("k2addr_test.cpp")]
    if is_k2addr_touched and not other_touched:
        diff_text, base_content, new_content = get_k2addr_git_state(
            repo_root, ref=args.since, staged=args.staged
        )
        if is_k2addr_gog_only_diff(diff_text, base_content, new_content):
            narrow_plan_to_gog_scoping(plan)

    # If --why or --list, print plan
    if args.list or args.why:
        print_plan(plan, why_mode=args.why, repo_root=repo_root, kpm_dir=graph.kpm_dir)
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
                "info_notes": plan.info_notes,
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
    print_plan(plan, why_mode=False, repo_root=repo_root, kpm_dir=graph.kpm_dir)

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
        # A cpp test the graph found but run_tests does not register (e.g. research/**/parity/*_test.cpp, run by the
        # suite:levelup-* suites) would make run_tests exit "no test item matched" with no JSON: drop it, say so.
        known = known_run_test_ids(repo_root)
        unknown = {it.id for it in plan.items if it.kind == "test"} - known if known else set()
        for u in sorted(unknown):
            print(f"NOTE    {u} is not a run_tests item (covered by its suite, if any); not run")
        test_suite_ids = [i for i in test_suite_ids if i not in unknown]
        build_mods = plan.builds()

        # Check stale wine smokes if wine is enabled
        stale_map, stale_ids = check_stale_wine_smokes(plan, repo_root, graph.kpm_dir) if args.wine else ({}, [])
        for s_id in stale_ids:
            s_msg = stale_map[s_id]
            all_results.append({
                "id": s_id,
                "kind": "wine",
                "status": "SKIP",
                "seconds": 0.0,
                "reason": "unverified (stale kpatch)",
                "tail": s_msg,
            })
            counts["SKIP"] = counts.get("SKIP", 0) + 1

        test_suite_ids = [tid for tid in test_suite_ids if tid not in stale_ids]

        def run_build_mods(build_mods_list: list[str]) -> None:
            if not build_mods_list:
                return
            build_check_script = repo_root / "scripts/build_check.py"
            b_cmd = [sys.executable, str(build_check_script)]
            for m in build_mods_list:
                b_cmd += ["--mod", m]
            if args.jobs:
                b_cmd += ["--jobs", str(args.jobs)]
            if args.kpm_dir:
                b_cmd += ["--kpm-dir", str(args.kpm_dir)]
            if args.wine:
                b_cmd.append("--keep")

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

            if b_proc.returncode != 0 and not any(
                r["kind"] == "build" and r["status"] == "FAIL" and r["id"].startswith("build_check:")
                for r in all_results
            ):
                all_results.append(
                    {
                        "id": "build_check",
                        "kind": "build",
                        "status": "ERROR",
                        "seconds": 0.0,
                        "reason": f"build_check.py exited {b_proc.returncode}",
                        "tail": "\n".join(out_lines[-20:]),
                    }
                )
                counts["ERROR"] = counts.get("ERROR", 0) + 1

        # If running wine smokes, run builds first with --keep so built DLLs are available for smokes
        if args.wine and build_mods:
            run_build_mods(build_mods)

        # 1. Run tests and suites via run_tests.py
        if test_suite_ids:
            run_tests_script = repo_root / "scripts/run_tests.py"
            test_cmd = [sys.executable, str(run_tests_script), "--build"]
            if args.jobs:
                test_cmd += ["--jobs", str(args.jobs)]
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
                # The temp file is pre-created by NamedTemporaryFile, so an empty file means the runner wrote no results
                if Path(json_tmp_path).is_file() and Path(json_tmp_path).stat().st_size > 0:
                    try:
                        runner_json = json.loads(Path(json_tmp_path).read_text(encoding="utf-8"))
                        for r in runner_json.get("results", []):
                            all_results.append(r)
                            st = r.get("status", "ERROR")
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
                        counts["ERROR"] = counts.get("ERROR", 0) + len(test_suite_ids)
                else:
                    # No usable runner output (crash, early exit, empty file): fail closed
                    counts["ERROR"] = counts.get("ERROR", 0) + len(test_suite_ids)
            finally:
                if os.path.exists(json_tmp_path):
                    os.unlink(json_tmp_path)
                if os.path.exists(ids_file_path):
                    os.unlink(ids_file_path)

        # 2. Run builds if not already run
        if not args.wine and build_mods:
            run_build_mods(build_mods)

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
            "info_notes": plan.info_notes,
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
