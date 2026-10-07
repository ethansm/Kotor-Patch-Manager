#!/usr/bin/env python3
"""
scripts/compat_graph.py - Dependency and reachability graph for KOTOR-II patch manager mods.

Constructs:
  - E1: C++ include graph (#include "x" and <x> with stub / shared / KPM resolution)
  - E2: Python import graph (ast parse over scripts, tests, mods, research)
  - E3: Manifest library edges (manifest.toml: requires, requires_interfaces, provides, kind=library)
  - E4: Non-include test coverage:
        (a) Colocation: tests in a mod dir cover that mod's sources
        (b) Suite scripts: .cpp literals in suite runner scripts (parity.py, etc.)
        (c) Wine smokes: smokes.toml kpatch -> manifest id -> mod dir
        (d) Covers: test.toml and tests/compat.toml [[covers]] annotations
  - E5: Generator tables: tests/compat.toml [[generator]] or scripts/gen_*.py docstrings
  - E6: File class -> static checkers table

Answers affected(paths) -> Plan deterministically with shortest edge-chain reasons.
Doc 22 (update compatibility check).
"""

from __future__ import annotations

import ast
import fnmatch
import os
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

if sys.version_info >= (3, 11):
    import tomllib
else:
    import tomli as tomllib  # type: ignore

PRUNE_DIRS = {
    ".git",
    ".venv",
    "__pycache__",
    "node_modules",
    "retired",
    "testdata",
    "fixtures",
}

CPP_EXTS = {".cpp", ".h", ".hpp", ".inc"}
INCLUDE_RE = re.compile(r'^\s*#\s*include\s*([<"])([^>"]+)[>"]', re.MULTILINE)
CPP_LITERAL_RE = re.compile(r'[\'"]([^\'"\s]+\.cpp)[\'"]')
GEN_INC_RE = re.compile(r'(\b\w+\.inc\b)')
PY_IMPORT_RE = re.compile(r'^\s*(?:from\s+([a-zA-Z0-9_]+)|import\s+([a-zA-Z0-9_]+))', re.MULTILINE)


def _norm_posix(path: Path | str) -> str:
    return str(path).replace("\\", "/")


def _is_pruned(p: Path) -> bool:
    return any(part in PRUNE_DIRS for part in p.parts)


@dataclass(frozen=True)
class PlanItem:
    kind: str  # "build", "test", "suite", "wine", "static", "gap", "info"
    id: str  # e.g. "cpp:gui2d_test", "build_check:buff_duration_hud_src", "suite:guikit"
    why: tuple[str, ...]  # chain of nodes/reasons leading to this item
    cmd: tuple[str, ...] = ()
    details: dict[str, Any] = field(default_factory=dict)


@dataclass
class Plan:
    touched: list[str]
    items: list[PlanItem] = field(default_factory=list)
    gaps: list[str] = field(default_factory=list)
    unclassified: list[str] = field(default_factory=list)
    kpm_consumers: list[str] = field(default_factory=list)
    info_notes: list[str] = field(default_factory=list)

    def tests(self) -> list[str]:
        return [it.id for it in self.items if it.kind == "test"]

    def builds(self) -> list[str]:
        res: list[str] = []
        for it in self.items:
            if it.kind == "build":
                mod_val = it.details.get("mod", it.id.replace("build_check:", ""))
                if mod_val not in res:
                    res.append(mod_val)
        return res

    def suites(self) -> list[str]:
        return [it.id for it in self.items if it.kind == "suite"]

    def smokes(self) -> list[str]:
        return [it.id for it in self.items if it.kind == "wine"]

    def static(self) -> list[str]:
        return [it.id for it in self.items if it.kind == "static"]


def find_repo_root(start: Path | None = None) -> Path:
    curr = (start or Path.cwd()).resolve()
    while True:
        if (curr / "patch_manager_mods").is_dir() and (curr / "scripts").is_dir():
            return curr
        if curr.parent == curr:
            break
        curr = curr.parent
    return (start or Path.cwd()).resolve()


class CompatGraph:
    def __init__(self, repo_root: Path | None = None, kpm_dir: Path | None = None):
        self.repo_root = repo_root.resolve() if repo_root else find_repo_root()
        kpm_env = os.environ.get("KPM_DIR")
        if kpm_dir is not None:
            self.kpm_dir = kpm_dir.resolve()
        elif kpm_env:
            self.kpm_dir = Path(kpm_env).resolve()
        else:
            self.kpm_dir = (Path.home() / "Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager").resolve()

        # Nodes: repo-relative posix path
        self.all_cpp_files: set[str] = set()
        self.all_py_files: set[str] = set()
        self.py_stem_map: dict[str, str] = {}  # module stem -> relpath

        # E1: includes
        # target -> list of (includer, is_stub_angle)
        self.reverse_includes: dict[str, list[tuple[str, bool]]] = {}
        # includer -> list of raw target names that could not be resolved
        self.unresolved_includes: dict[str, list[str]] = {}

        # E2: python imports
        # imported_file -> list of importer_files
        self.reverse_imports: dict[str, list[str]] = {}

        # E3: manifests & libraries
        # mod_dir_rel -> manifest dict
        self.manifests: dict[str, dict[str, Any]] = {}
        # library_mod_dir -> list of consumer_mod_dirs
        self.library_consumers: dict[str, list[str]] = {}
        # interface name@major -> list of consumer_mod_dirs
        self.interface_consumers: dict[str, list[str]] = {}

        # Mod mapping
        # file_rel -> nearest mod dir rel
        self.mod_dirs: set[str] = set()
        # mod_dir -> list of colocated test files
        self.mod_colocated_tests: dict[str, list[str]] = {}
        # mod_dir -> bool (whether mod owns common_contract_test.cpp)
        self.mod_has_contract_test: dict[str, bool] = {}

        # E4b: suite scripts -> .cpp references
        # cpp_rel -> list of suite_names
        self.cpp_to_suites: dict[str, list[str]] = {}

        # E4c: wine smokes
        # mod_dir -> list of smoke_names
        self.mod_to_smokes: dict[str, list[str]] = {}

        # E4d: covers annotations
        # pattern_or_iface -> list of item_ids (e.g. "suite:levelup-parity", "cpp:hotbar_bugs_test")
        self.covers_map: list[tuple[str, str]] = []  # (glob/pattern, item_id)

        # E5: generators
        # script_rel -> list of output .inc rel paths
        self.generators: dict[str, list[str]] = {}
        # inc_rel -> script_rel
        self.inc_to_generator: dict[str, str] = {}

        # Test item IDs cache
        self.test_ids: dict[str, str] = {}

        self.build_graph()

    def _rel(self, p: Path) -> str:
        try:
            return _norm_posix(p.resolve().relative_to(self.repo_root))
        except ValueError:
            return _norm_posix(p)

    def _safe_realpath(self, p: Path) -> Path:
        try:
            return p.resolve()
        except Exception:
            return p

    def _find_mod_dir(self, file_path: Path) -> str | None:
        """Find the nearest ancestor directory under patch_manager_mods that contains manifest.toml."""
        curr = file_path if file_path.is_dir() else file_path.parent
        pm_root = self.repo_root / "patch_manager_mods"
        while True:
            if (curr / "manifest.toml").is_file():
                try:
                    return _norm_posix(curr.resolve().relative_to(pm_root))
                except ValueError:
                    return _norm_posix(curr.relative_to(self.repo_root))
            if curr == self.repo_root or curr == pm_root or curr.parent == curr:
                break
            curr = curr.parent
        return None

    def build_graph(self) -> None:
        """Scan and build the complete dependency graph."""
        self._scan_manifests()
        self._scan_cpp_files()
        self._scan_py_files()
        self._scan_suites()
        self._scan_smokes()
        self._scan_covers_and_generators()

    def _scan_manifests(self) -> None:
        pm_root = self.repo_root / "patch_manager_mods"
        if not pm_root.is_dir():
            return

        for root, dirs, files in os.walk(pm_root):
            rpath = Path(root)
            if _is_pruned(rpath):
                continue
            if "manifest.toml" in files:
                mfile = rpath / "manifest.toml"
                try:
                    data = tomllib.loads(mfile.read_text(encoding="utf-8"))
                    patch = data.get("patch", {})
                    rel_mod = _norm_posix(rpath.resolve().relative_to(pm_root))
                    self.mod_dirs.add(rel_mod)
                    self.manifests[rel_mod] = {
                        "id": patch.get("id", rel_mod),
                        "name": patch.get("name", ""),
                        "kind": patch.get("kind", "mod"),
                        "provides": patch.get("provides", []),
                        "requires": patch.get("requires", []),
                        "requires_interfaces": patch.get("requires_interfaces", []),
                        "dir": rpath,
                        "rel": rel_mod,
                    }
                except Exception:
                    continue

        # Build library consumer relationships (E3)
        for lib_mod, lib_info in self.manifests.items():
            if lib_info["kind"] == "library":
                lib_id = lib_info["id"]
                provides = set(lib_info["provides"])
                consumers: list[str] = []
                for c_mod, c_info in self.manifests.items():
                    if c_mod == lib_mod:
                        continue
                    reqs = set(c_info.get("requires", []))
                    req_ifaces = set(c_info.get("requires_interfaces", []))
                    if lib_id in reqs or (provides and req_ifaces.intersection(provides)):
                        consumers.append(c_mod)
                self.library_consumers[lib_mod] = consumers

            for iface in lib_info.get("provides", []):
                self.interface_consumers.setdefault(iface, [])

        for c_mod, c_info in self.manifests.items():
            for iface in c_info.get("requires_interfaces", []):
                self.interface_consumers.setdefault(iface, [])
                if c_mod not in self.interface_consumers[iface]:
                    self.interface_consumers[iface].append(c_mod)

    def _scan_cpp_files(self) -> None:
        search_roots = [
            self.repo_root / "patch_manager_mods",
            self.repo_root / "research/companion_levelup",
            self.repo_root / "tests",
        ]

        seen_stems: dict[str, str] = {}
        candidate_files: list[Path] = []

        for root in search_roots:
            if not root.is_dir():
                continue
            for dp, dirs, fns in os.walk(root):
                dpath = Path(dp)
                if _is_pruned(dpath):
                    continue
                for fn in fns:
                    ext = os.path.splitext(fn)[1].lower()
                    if ext in CPP_EXTS:
                        candidate_files.append(dpath / fn)

        for src in candidate_files:
            rel_src = self._rel(src)
            self.all_cpp_files.add(rel_src)

            # Assign test item ID if it's a test file
            if src.name.endswith("_test.cpp") and "selftest" not in src.parts:
                stem = src.stem
                if stem in seen_stems:
                    item_id = f"cpp:{rel_src}"
                else:
                    item_id = f"cpp:{stem}"
                    seen_stems[stem] = rel_src
                self.test_ids[rel_src] = item_id

            # Colocation tracking (E4a)
            mod_dir = self._find_mod_dir(src)
            if mod_dir:
                if src.name.endswith("_test.cpp"):
                    self.mod_colocated_tests.setdefault(mod_dir, []).append(rel_src)
                if src.name == "common_contract_test.cpp":
                    self.mod_has_contract_test[mod_dir] = True

        # Parse includes (E1)
        for src in candidate_files:
            rel_src = self._rel(src)
            try:
                content = src.read_text(encoding="utf-8", errors="ignore")
            except Exception:
                continue

            for match in INCLUDE_RE.finditer(content):
                quote, inc_target = match.group(1), match.group(2)
                resolved, is_stub_angle = self._resolve_include(src, quote, inc_target)
                if resolved is not None:
                    target_key = self._rel(resolved) if isinstance(resolved, Path) else resolved
                    self.reverse_includes.setdefault(target_key, []).append((rel_src, is_stub_angle))
                else:
                    if quote == '"':
                        self.unresolved_includes.setdefault(rel_src, []).append(inc_target)

    def _resolve_include(self, source_file: Path, quote: str, inc_target: str) -> tuple[Path | str | None, bool]:
        """Resolve include target according to doc 22 resolution order."""
        shared_root = self.repo_root / "patch_manager_mods/_shared_src"

        # Quoted include: "x"
        if quote == '"':
            # 1. Includer's directory
            p = source_file.parent / inc_target
            if p.is_file():
                return self._safe_realpath(p), False

            # 2. _shared_src (with ../_shared/ or ../_shared_src/ stripped)
            stripped = re.sub(r"^(?:\.\./)?(?:_shared|_shared_src|Common)/", "", inc_target)
            p_shared = shared_root / stripped
            if p_shared.is_file():
                return self._safe_realpath(p_shared), False
            p_shared_direct = shared_root / inc_target
            if p_shared_direct.is_file():
                return self._safe_realpath(p_shared_direct), False

            # 3. KPM Patches/Common / GameAPI (external read-only)
            if self.kpm_dir and self.kpm_dir.is_dir():
                kpm_common = self.kpm_dir / "Patches/Common" / stripped
                if kpm_common.is_file():
                    return f"kpm:Patches/Common/{stripped}", False
                kpm_gameapi = self.kpm_dir / "Patches/Common/GameAPI" / stripped
                if kpm_gameapi.is_file():
                    return f"kpm:Patches/Common/GameAPI/{stripped}", False

            # 4. Roots under research/ or tests/ -> stub search path
            if any(part in source_file.parts for part in ("research", "tests")):
                for stub_dir in (
                    self.repo_root / "research/companion_levelup/stub",
                    self.repo_root / "research/companion_levelup/stub/inc",
                    self.repo_root / "tests/include",
                    self.repo_root / "stub",
                    self.repo_root / "stub/inc",
                ):
                    p_stub = stub_dir / inc_target
                    if p_stub.is_file():
                        return self._safe_realpath(p_stub), False

            return None, False

        # Angle include: <x>
        if quote == "<":
            # Angle includes resolve only through the stub rule for host roots
            for stub_dir in (
                self.repo_root / "research/companion_levelup/stub",
                self.repo_root / "research/companion_levelup/stub/inc",
                self.repo_root / "tests/include",
                self.repo_root / "stub",
                self.repo_root / "stub/inc",
            ):
                p_stub = stub_dir / inc_target
                if p_stub.is_file():
                    return self._safe_realpath(p_stub), True

            return None, False

        return None, False

    def _scan_py_files(self) -> None:
        """Scan Python files and build import graph (E2)."""
        search_roots = [
            self.repo_root / "scripts",
            self.repo_root / "tests",
            self.repo_root / "patch_manager_mods",
            self.repo_root / "research",
        ]

        candidate_files: list[Path] = []
        for root in search_roots:
            if not root.is_dir():
                continue
            for dp, dirs, fns in os.walk(root):
                dpath = Path(dp)
                if _is_pruned(dpath):
                    continue
                for fn in fns:
                    if fn.endswith(".py"):
                        candidate_files.append(dpath / fn)

        for p in candidate_files:
            rel_p = self._rel(p)
            self.all_py_files.add(rel_p)
            stem = p.stem
            # Map bare module name -> unique file with that stem
            if stem not in self.py_stem_map:
                self.py_stem_map[stem] = rel_p

            if p.name.startswith("test_"):
                self.test_ids[rel_p] = f"py:{rel_p}"

        # Parse imports
        for p in candidate_files:
            rel_importer = self._rel(p)
            try:
                content = p.read_text(encoding="utf-8", errors="ignore")
            except Exception:
                continue

            for m in PY_IMPORT_RE.finditer(content):
                mod_name = m.group(1) or m.group(2)
                if mod_name and mod_name in self.py_stem_map:
                    target_file = self.py_stem_map[mod_name]
                    if target_file != rel_importer:
                        self.reverse_imports.setdefault(target_file, []).append(rel_importer)

    def _scan_suites(self) -> None:
        """Parse tests/suites.toml and detect .cpp literals (E4b)."""
        suites_toml = self.repo_root / "tests/suites.toml"
        if not suites_toml.is_file():
            return

        try:
            cfg = tomllib.loads(suites_toml.read_text(encoding="utf-8"))
        except Exception:
            return

        for suite in cfg.get("suite", []):
            name = suite.get("name")
            cmd = suite.get("cmd", [])
            if not name or not cmd:
                continue

            # Look for python script in cmd
            for arg in cmd:
                if arg.endswith(".py"):
                    script_path = (self.repo_root / arg).resolve()
                    if script_path.is_file():
                        # Read script text for .cpp literals
                        try:
                            stext = script_path.read_text(encoding="utf-8", errors="ignore")
                            for cpp_match in CPP_LITERAL_RE.findall(stext):
                                # Resolve .cpp match
                                cpp_candidates = [
                                    script_path.parent / cpp_match,
                                    self.repo_root / "research/companion_levelup/tools/parity" / cpp_match,
                                    self.repo_root / "patch_manager_mods/level_up_override_src" / cpp_match,
                                    self.repo_root / cpp_match,
                                ]
                                for cand in cpp_candidates:
                                    if cand.is_file():
                                        rel_c = self._rel(cand)
                                        self.cpp_to_suites.setdefault(rel_c, []).append(name)
                                        break
                        except Exception:
                            pass

    def _scan_smokes(self) -> None:
        """Parse smokes.toml and map to mod dirs (E4c)."""
        smokes_toml = self.repo_root / "patch_manager_mods/perf_lab_src/harness/smokes.toml"
        if not smokes_toml.is_file():
            return

        try:
            cfg = tomllib.loads(smokes_toml.read_text(encoding="utf-8"))
        except Exception:
            return

        for s in cfg.get("smoke", []):
            name = s.get("name")
            kpatch = s.get("kpatch", "")
            if not name or not kpatch:
                continue

            # kpatch is e.g. "LoadHangProbe/LoadHangProbe.kpatch" or "../split_build/PortraitSlots.kpatch"
            patch_token = Path(kpatch).stem
            # Match against known manifests
            matched_mod = None
            for mod_dir, m_info in self.manifests.items():
                m_name = m_info.get("name", "").replace(" ", "").lower()
                m_id = m_info.get("id", "").replace("-", "").lower()
                token_clean = patch_token.replace("-", "").replace("_", "").lower()

                if token_clean in m_id or token_clean in m_name or m_id in token_clean:
                    matched_mod = mod_dir
                    break

            if matched_mod:
                self.mod_to_smokes.setdefault(matched_mod, []).append(name)

    def _scan_covers_and_generators(self) -> None:
        """Scan test.toml covers and tests/compat.toml (E4d, E5)."""
        # test.toml files
        for root, dirs, files in os.walk(self.repo_root / "patch_manager_mods"):
            if "test.toml" in files:
                tpath = Path(root) / "test.toml"
                try:
                    data = tomllib.loads(tpath.read_text(encoding="utf-8"))
                    # Top-level or per-test tables
                    for key, val in data.items():
                        if isinstance(val, dict) and "covers" in val:
                            cov = val["covers"]
                            if isinstance(cov, str):
                                cov = [cov]
                            test_id = f"cpp:{key}"
                            for c in cov:
                                self.covers_map.append((c, test_id))
                        elif key == "covers" and isinstance(val, list):
                            # Directory default covers
                            for c in val:
                                for test_file in self.mod_colocated_tests.get(self._rel(Path(root)), []):
                                    self.covers_map.append((c, self.test_ids.get(test_file, f"cpp:{Path(test_file).stem}")))
                except Exception:
                    continue

        # tests/compat.toml
        compat_toml = self.repo_root / "tests/compat.toml"
        if compat_toml.is_file():
            try:
                data = tomllib.loads(compat_toml.read_text(encoding="utf-8"))
                for row in data.get("covers", []):
                    item_id = row.get("id")
                    for p in row.get("paths", []):
                        if item_id:
                            self.covers_map.append((p, item_id))
                for gen in data.get("generator", []):
                    script = gen.get("script")
                    writes = gen.get("writes", [])
                    if script:
                        self.generators[script] = writes
                        for w in writes:
                            self.inc_to_generator[w] = script
            except Exception:
                pass

        # Also inspect scripts/gen_*.py docstrings for generator edges
        scripts_dir = self.repo_root / "scripts"
        if scripts_dir.is_dir():
            for sp in scripts_dir.glob("gen_*.py"):
                try:
                    doc = sp.read_text(encoding="utf-8", errors="ignore")[:1000]
                    for match in GEN_INC_RE.findall(doc):
                        rel_script = self._rel(sp)
                        inc_target = f"patch_manager_mods/_shared_src/{match}"
                        self.generators.setdefault(rel_script, []).append(inc_target)
                        self.inc_to_generator[inc_target] = rel_script
                except Exception:
                    continue

    def affected(
        self,
        paths: list[str | Path],
        include_wine: bool = False,
        include_static: bool = True,
    ) -> Plan:
        """Derive all affected builds, tests, suites, smokes and checks for given touched paths."""
        norm_touched: list[str] = []
        deleted_basenames: set[str] = set()

        for p in paths:
            p_obj = p if isinstance(p, Path) else Path(p)
            # Check existence before resolve
            full_path = p_obj if p_obj.is_absolute() else self.repo_root / p_obj
            exists = full_path.exists()
            rel_str = self._rel(self._safe_realpath(full_path))
            norm_touched.append(rel_str)
            if not exists:
                deleted_basenames.add(p_obj.name)

        plan = Plan(touched=norm_touched)

        # BFS queue: (current_node, chain, host_only, from_shared)
        queue: list[tuple[str, list[str], bool, bool]] = []
        visited: set[tuple[str, bool, bool]] = set()

        for t in norm_touched:
            is_sh = t.startswith("patch_manager_mods/_shared_src")
            queue.append((t, [t], False, is_sh))

        # Check deleted headers against unresolved includes
        if deleted_basenames:
            for includer, targets in self.unresolved_includes.items():
                for target in targets:
                    t_name = Path(target).name
                    if t_name in deleted_basenames:
                        queue.append((includer, [f"deleted {t_name}", includer], False, False))

        # Process generator notifications (E5)
        for t in norm_touched:
            if t in self.inc_to_generator:
                gen_script = self.inc_to_generator[t]
                plan.info_notes.append(f"INFO: {t} is generated by {gen_script}; regenerate, do not hand-edit.")

        # Static checkers and file class table (E6)
        if include_static:
            self._apply_static_checkers(norm_touched, plan)

        # BFS Traversal
        while queue:
            node, chain, host_only, from_shared = queue.pop(0)
            state = (node, host_only, from_shared)
            if state in visited:
                continue
            visited.add(state)

            # 1. C++ Host Test Root
            if node.endswith("_test.cpp") and "selftest" not in node:
                test_id = self.test_ids.get(node, f"cpp:{Path(node).stem}")
                plan.items.append(
                    PlanItem(
                        kind="test",
                        id=test_id,
                        why=tuple(chain),
                        details={"src": node},
                    )
                )

            # 2. Python Test File
            if node.endswith(".py") and Path(node).name.startswith("test_"):
                test_id = self.test_ids.get(node, f"py:{node}")
                plan.items.append(
                    PlanItem(
                        kind="test",
                        id=test_id,
                        why=tuple(chain),
                        details={"src": node},
                    )
                )

            # 3. Suite .cpp match (E4b)
            if node in self.cpp_to_suites:
                for sname in self.cpp_to_suites[node]:
                    plan.items.append(
                        PlanItem(
                            kind="suite",
                            id=f"suite:{sname}",
                            why=tuple(chain),
                        )
                    )

            # 4. Mod Directory Code (build_check + colocation + library fan-out)
            mod_dir = self._find_mod_dir(self.repo_root / node)
            if mod_dir and not node.startswith("patch_manager_mods/_shared_src"):
                is_test = node.endswith("_test.cpp")

                # If non-test source and not host-only, schedule DLL build
                if not host_only and not is_test:
                    plan.items.append(
                        PlanItem(
                            kind="build",
                            id=f"build_check:{mod_dir}",
                            why=tuple(chain),
                            details={"mod": mod_dir},
                        )
                    )

                # Colocation (E4a): a test in a mod dir covers that dir's non-test sources.
                # (a) does not apply inside _shared_src (E1 is precise there).
                if not is_test and not from_shared:
                    for col_test in self.mod_colocated_tests.get(mod_dir, []):
                        if col_test not in chain:
                            queue.append((col_test, chain + [f"colocated in {mod_dir}", col_test], host_only, from_shared))

                # Manifest library fan-out (E3): library touched -> consumers
                # Does not fan out for test files or when reached from _shared_src includes
                if not is_test and not from_shared and mod_dir in self.library_consumers:
                    for consumer_mod in self.library_consumers[mod_dir]:
                        c_manifest = self.manifests[consumer_mod]
                        # Add consumer builds & colocated tests
                        if not host_only:
                            plan.items.append(
                                PlanItem(
                                    kind="build",
                                    id=f"build_check:{consumer_mod}",
                                    why=tuple(chain + [f"library consumer: {consumer_mod}"]),
                                    details={"mod": consumer_mod},
                                )
                            )
                        for col_test in self.mod_colocated_tests.get(consumer_mod, []):
                            if col_test not in chain:
                                queue.append(
                                    (
                                        col_test,
                                        chain + [f"library consumer: {consumer_mod}", col_test],
                                        host_only,
                                        from_shared,
                                    )
                                )

                # Wine smokes (E4c)
                if include_wine and mod_dir in self.mod_to_smokes:
                    for sm_name in self.mod_to_smokes[mod_dir]:
                        plan.items.append(
                            PlanItem(
                                kind="wine",
                                id=f"wine:{sm_name}",
                                why=tuple(chain),
                            )
                        )

            # 5. Covers annotations (E4d)
            for pattern, item_id in self.covers_map:
                if fnmatch.fnmatch(node, pattern) or node == pattern:
                    k = "suite" if item_id.startswith("suite:") else "test"
                    plan.items.append(PlanItem(kind=k, id=item_id, why=tuple(chain + [f"covers:{pattern}"])))

            # Follow reverse includes (E1)
            for includer, is_stub_angle in self.reverse_includes.get(node, []):
                next_host_only = host_only or is_stub_angle
                queue.append((includer, chain + [includer], next_host_only, from_shared))

            # Follow reverse python imports (E2)
            for importer in self.reverse_imports.get(node, []):
                queue.append((importer, chain + [importer], True, from_shared))

        # Check for unclassified touched files
        for t in norm_touched:
            if not self._is_classified(t, plan):
                plan.unclassified.append(t)

        # Detect Contract-Test GAPs
        self._detect_contract_gaps(plan)

        # Compute KPM consumers if _shared_src was touched
        if any(t.startswith("patch_manager_mods/_shared_src") for t in norm_touched):
            plan.kpm_consumers = self._compute_kpm_consumers(plan)

        # Deterministic deduplication & sort
        self._dedup_and_sort_plan(plan)

        return plan

    def _is_classified(self, path: str, plan: Plan) -> bool:
        if path.startswith("patch_manager_mods/_shared_src/"):
            return True
        if any(path.startswith(f"patch_manager_mods/{m}") for m in self.mod_dirs):
            return True
        if path.startswith("scripts/") or path.startswith("tests/") or path.startswith("research/"):
            return True
        return False

    def _apply_static_checkers(self, touched: list[str], plan: Plan) -> None:
        """Apply E6 file-class table."""
        has_shared = False
        has_mod_source = False
        has_hooks = False
        has_k2addr = False

        for t in touched:
            p = Path(t)
            if t.startswith("patch_manager_mods/_shared_src/"):
                has_shared = True
            if p.name in ("GameAddr.h", "K2Addr.h"):
                has_k2addr = True
            if any(t.startswith(f"patch_manager_mods/{m}/") for m in self.mod_dirs):
                if p.suffix in CPP_EXTS:
                    has_mod_source = True
            if "hooks.toml" in p.name or p.name == "code_patches.toml":
                has_hooks = True

        if has_shared:
            plan.items.append(
                PlanItem(
                    kind="static",
                    id="suite:check-shared-headers",
                    why=("touched _shared_src",),
                )
            )
            plan.items.append(
                PlanItem(
                    kind="static",
                    id="suite:check-raw-addresses",
                    why=("touched _shared_src",),
                )
            )

        if has_mod_source and not has_shared:
            plan.items.append(
                PlanItem(
                    kind="static",
                    id="suite:check-raw-addresses",
                    why=("touched mod source",),
                )
            )
            plan.items.append(
                PlanItem(
                    kind="static",
                    id="suite:check-subclassers",
                    why=("touched mod source",),
                )
            )

        if has_hooks:
            plan.items.append(
                PlanItem(
                    kind="static",
                    id="suite:kpm-validate-patches",
                    why=("touched hooks/code_patches",),
                )
            )

        if has_k2addr:
            plan.items.append(
                PlanItem(
                    kind="static",
                    id="suite:check-hook-bytes",
                    why=("touched GameAddr/K2Addr",),
                )
            )

    def _detect_contract_gaps(self, plan: Plan) -> None:
        """Find mods that include a Common header but lack common_contract_test.cpp."""
        # Check all mods selected for build
        for mod in plan.builds():
            if not self.mod_has_contract_test.get(mod, False):
                plan.gaps.append(f"{mod} includes Common headers but owns no *_contract_test.cpp (doc 20 4.1)")

    def _compute_kpm_consumers(self, plan: Plan) -> list[str]:
        """Compute the list of KPM patch consumers corresponding to affected mods."""
        kpm_names: list[str] = []
        for mod in plan.builds():
            m_info = self.manifests.get(mod)
            if m_info and m_info.get("name"):
                name = m_info["name"].split(" (")[0].replace(" ", "")
                kpm_names.append(name)
            else:
                kpm_names.append(mod)
        return sorted(set(kpm_names))

    def _dedup_and_sort_plan(self, plan: Plan) -> None:
        """Deduplicate items by ID keeping shortest reason chain, then sort deterministically."""
        seen: dict[tuple[str, str], PlanItem] = {}
        for it in plan.items:
            key = (it.kind, it.id)
            if key not in seen:
                seen[key] = it
            else:
                # Keep shortest chain
                if len(it.why) < len(seen[key].why):
                    seen[key] = it

        kind_order = {"build": 0, "test": 1, "suite": 2, "wine": 3, "static": 4}
        plan.items = sorted(
            seen.values(),
            key=lambda x: (kind_order.get(x.kind, 9), x.id),
        )
