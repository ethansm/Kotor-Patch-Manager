#!/usr/bin/env python3
"""generate_repo_map.py - Deterministic structural repository skeleton generator

Parses C++, Python, and Rust source files to extract classes, structs, functions,
and signatures, emitting a compact structural map (~500-1000 tokens) for agent context.
Zero external dependencies (uses standard library).

Usage:
  python3 scripts/generate_repo_map.py [directory] [--max-lines 100]
"""

import argparse
import os
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

CPP_CLASS_RE = re.compile(r'^\s*(?:class|struct)\s+([A-Za-z0-9_]+)(?:\s*:\s*(?:public|protected|private)\s+([A-Za-z0-9_]+))?\s*\{?', re.MULTILINE)
CPP_FN_RE = re.compile(r'^\s*(?:(?:static|virtual|inline|explicit)\s+)*(?:[A-Za-z0-9_:<>&*]+\s+)+([A-Za-z0-9_~]+)\s*\(([^)]*)\)\s*(?:const)?\s*(?:override|noexcept)?\s*[{;]', re.MULTILINE)
PY_DEF_RE = re.compile(r'^\s*(?:async\s+)?def\s+([A-Za-z0-9_]+)\s*\(([^)]*)\)', re.MULTILINE)
PY_CLASS_RE = re.compile(r'^\s*class\s+([A-Za-z0-9_]+)(?:\(([^)]*)\))?:', re.MULTILINE)

SKIP_DIRS = {".venv", "__pycache__", ".git", "node_modules", "retired", "testdata", "fixtures", "logs", "bin"}

def parse_file(path: Path) -> dict:
    try:
        content = path.read_text(encoding="utf-8", errors="ignore")
    except Exception:
        return {}

    ext = path.suffix.lower()
    classes = []
    functions = []

    if ext in (".h", ".hpp", ".cpp"):
        for m in CPP_CLASS_RE.finditer(content):
            cls_name = m.group(1)
            base = m.group(2)
            classes.append(f"{cls_name}" + (f" : {base}" if base else ""))
        for m in CPP_FN_RE.finditer(content):
            fn_name = m.group(1)
            args = m.group(2).strip()
            # filter common control statements that look like functions
            if fn_name not in ("if", "for", "while", "switch", "catch"):
                # simplify args (strip types or keep short)
                short_args = ", ".join(a.strip().split()[-1] for a in args.split(",") if a.strip()) if args else ""
                functions.append(f"{fn_name}({short_args})")

    elif ext == ".py":
        for m in PY_CLASS_RE.finditer(content):
            cls_name = m.group(1)
            base = m.group(2)
            classes.append(f"class {cls_name}" + (f"({base})" if base else ""))
        for m in PY_DEF_RE.finditer(content):
            fn_name = m.group(1)
            args = m.group(2).strip()
            functions.append(f"def {fn_name}({args})")

    return {
        "classes": list(dict.fromkeys(classes))[:10],
        "functions": list(dict.fromkeys(functions))[:15],
    }

def generate_map(target_dir: Path, max_lines: int = 150) -> str:
    lines = []
    try:
        header_path = target_dir.relative_to(REPO_ROOT) if target_dir != REPO_ROOT else '.'
    except ValueError:
        header_path = str(target_dir)
    lines.append(f"# Structural Repo Map: {header_path}")
    
    file_entries = []
    for root, dirs, files in os.walk(target_dir):
        dirs[:] = [d for d in dirs if d not in SKIP_DIRS and not d.startswith(".")]
        for f in sorted(files):
            p = Path(root) / f
            if p.suffix.lower() in (".h", ".hpp", ".cpp", ".py"):
                info = parse_file(p)
                if info.get("classes") or info.get("functions"):
                    try:
                        rel_p = p.relative_to(REPO_ROOT)
                    except ValueError:
                        rel_p = p
                    file_entries.append((rel_p, info))

    for rel_path, info in file_entries:
        if len(lines) >= max_lines:
            lines.append("... [remaining files truncated to preserve context budget]")
            break
        lines.append("")
        lines.append(f"`{rel_path}`:")
        for c in info["classes"]:
            lines.append(f"  - {c}")
        if info["functions"]:
            summary_fns = ", ".join(f.split("(")[0] for f in info["functions"][:8])
            lines.append(f"  - fns: {summary_fns}")

    return "\n".join(lines)

def main():
    parser = argparse.ArgumentParser(description="Generate structural repository map")
    parser.add_argument("dir", nargs="?", default=".", help="Target directory (relative to repo root)")
    parser.add_argument("--max-lines", type=int, default=120, help="Maximum lines in output")
    args = parser.parse_args()

    target = (REPO_ROOT / args.dir).resolve()
    if not target.exists():
        sys.stderr.write(f"Error: Directory not found: {target}\n")
        sys.exit(2)

    repo_map = generate_map(target, max_lines=args.max_lines)
    print(repo_map)

if __name__ == "__main__":
    main()
