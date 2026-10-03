#!/usr/bin/env python3
"""Static check that every WndProc subclasser in the mod sources is registered and restores itself (doc 20 T0 extra, Phase 3).

  python3 scripts/check_subclassers.py
  python3 scripts/check_subclassers.py --quiet
  python3 scripts/check_subclassers.py --root DIR [--registry subclassers.toml]

Scans the same sources as check_raw_addresses.py (patch_manager_mods/*_src/**/*.{cpp,h,inc}; testdata/, retired/,
*_test.cpp, GameAddr.h and generated .inc excluded) for
  SetWindowLong(Ptr)?[AW]?\\s*\\([^,]+,\\s*GWLP?_WNDPROC
Several DLLs chaining WndProcs over one game window must unhook in a safe order (restore only if still on top), so every
subclasser is listed in tests/static/subclassers.toml (schema in that file) with an install and a restore pattern.
  FAIL  a file with a match that is not registered ("new WndProc subclasser: ...")
  FAIL  a registered file whose install or restore pattern is missing from the file
  WARN  a registered file with no SetWindowLong(GWLP_WNDPROC) match at all (stale entry)
  WARN  a registered file that does not exist / is not scanned
  PASS  every subclasser registered with both patterns present
Line numbers in the registry are informational hints only (reported on drift as INFO, never checked).
Exit 1 on any FAIL. Read-only.
"""
import argparse
import re
import sys
import tomllib
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import _static_common as sc  # noqa: E402
import check_raw_addresses as cra  # noqa: E402  (scan_files: one source file set for both checkers)

CHECK = "subclasser"
SUBCLASS_RE = re.compile(r"SetWindowLong(?:Ptr)?[AW]?\s*\([^,]+,\s*GWLP?_WNDPROC")
REGISTRY_REL = "tests/static/subclassers.toml"
NEW_MSG = "new WndProc subclasser: use Common WindowHook.h when it lands; or register it in tests/static/subclassers.toml"


def load_registry(path):
    """{repo-relative file: {install, restore (regex strings), install_hint, restore_hint}}; a missing file is empty."""
    p = Path(path)
    if not p.is_file():
        return {}
    with open(p, "rb") as f:
        data = tomllib.load(f)
    return {e["file"]: e for e in data.get("subclasser", [])}


def line_of(text, pos):
    return text.count("\n", 0, pos) + 1


def run(root, registry=None, reporter=None):
    root = Path(root)
    rep = reporter or sc.Reporter()
    reg = load_registry(Path(registry) if registry else root / REGISTRY_REL)
    scanned = dict(cra.scan_files(root))
    seen_registered = 0
    for rel, p in scanned.items():
        text = p.read_text(errors="replace")
        matches = list(SUBCLASS_RE.finditer(text))
        entry = reg.get(rel)
        if matches and entry is None:
            for m in matches:
                rep.fail(CHECK, f"{rel}:{line_of(text, m.start())} {NEW_MSG}")
            continue
        if entry is None:
            continue
        if not matches:
            rep.warn(CHECK, f"stale registry entry {rel}: no SetWindowLong(GWLP_WNDPROC) call in the file")
            continue
        ok = True
        hints = {}
        for kind in ("install", "restore"):
            pat = entry.get(kind)
            m = re.search(pat, text, re.S) if pat else None
            if not m:
                ok = False
                rep.fail(CHECK, f"{rel}: {kind} pattern {'missing from the registry' if not pat else 'not found in the file'}"
                                f"{'' if kind == 'install' else ' (restore only if still ours: GetWindowLong == our proc, then SetWindowLong with the saved old proc)'}")
            else:
                hints[kind] = line_of(text, m.start())
        if ok:
            seen_registered += 1
            drift = [f"{k} line {hints[k]} (hint {entry.get(k + '_hint')})" for k in hints if entry.get(k + "_hint") != hints[k]]
            rep.info(CHECK, f"{rel}: install and restore present" + (f"; hints drifted: {', '.join(drift)}" if drift else ""))
    for rel in sorted(set(reg) - set(scanned)):
        rep.warn(CHECK, f"stale registry entry {rel}: file missing or not scanned")
    if not rep.of("FAIL"):
        rep.passed(CHECK, f"{seen_registered} subclasser file(s), all registered with install and restore present")
    return rep


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--root", default=str(sc.REPO), help="repo root holding patch_manager_mods/ (default: this repo)")
    ap.add_argument("--registry", help=f"subclassers.toml (default <root>/{REGISTRY_REL})")
    ap.add_argument("--quiet", action="store_true", help="hide PASS and INFO lines")
    a = ap.parse_args(argv)
    rep = sc.Reporter(quiet=a.quiet)
    run(a.root, a.registry, rep)
    return rep.finish("check_subclassers")


if __name__ == "__main__":
    sys.exit(main())
