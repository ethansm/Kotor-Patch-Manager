#!/usr/bin/env python3
"""Lint the `source` of every [[hooks.parameters]] entry in hooks.toml files (review F-2, npc_diversity).

  python3 scripts/check_hook_sources.py                                  # every hooks.toml under patch_manager_mods/
  python3 scripts/check_hook_sources.py path/a.hooks.toml [path/b.toml]  # explicit files

KPM reads a parameter `source` as a VALUE only when it is bracketed: `[esp+4]` pushes the value at [esp+4]. An unbracketed
stack-relative source such as `esp+4` pushes the ADDRESS esp+4 instead, and no gate caught it before (F-2).
  FAIL  source matches ^(esp|ebp)\\s*[+-]\\s*(0x[0-9a-fA-F]+|\\d+)$ (case-insensitive; surrounding spaces ignored)
  FAIL  a file cannot be read or parsed (it would otherwise pass unchecked)
Allowed: plain registers (eax ecx edx ebx esi edi ebp esp), bracketed forms ([esp+4], [ebp+0x08], [ebp]), anything else.
Exit 1 on any FAIL, 0 otherwise. Read-only.
"""
import argparse
import os
import re
import sys
import tomllib
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import _static_common as sc  # noqa: E402

BAD_SOURCE = re.compile(r"^(esp|ebp)\s*[+-]\s*(0x[0-9a-fA-F]+|\d+)$", re.IGNORECASE)
CHECK = "source"
# Deliberate address-passing sources (hook comment says the detour receives a POINTER to the pushed args). Keyed by
# (hooks file path suffix, hook address, 1-based parameter index) -> the exact source string allowed there.
ALLOWED_ADDRESS_SOURCES = {
    ("activation_fix_src/kotor2-steam-aspyr.hooks.toml", 0x0040D69E, 1): "esp+0",   # dropSyntheticDeactivate(ptr to hwnd,msg,wParam,lParam)
}


def bad_source(source) -> bool:
    """True for an unbracketed stack-relative source (esp+4, ebp - 8, esp-0x10)."""
    return isinstance(source, str) and bool(BAD_SOURCE.match(source.strip()))


def fix_for(source) -> str:
    """The bracketed form that reads the value: 'ebp + 8' -> '[ebp+8]'."""
    return "[" + re.sub(r"\s+", "", source.strip()) + "]"


def find_hooks_files(repo=sc.REPO) -> list:
    """Every *hooks.toml under <repo>/patch_manager_mods, skipping sc.SKIP_DIRS (testdata, retired, ...)."""
    root = Path(repo) / "patch_manager_mods"
    out = []
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = sorted(d for d in dirnames if d not in sc.SKIP_DIRS)
        out += [Path(dirpath) / f for f in sorted(filenames) if f.endswith("hooks.toml")]
    return out


def _rel(path) -> str:
    try:
        return str(Path(path).resolve().relative_to(sc.REPO))
    except ValueError:
        return str(path)


def _addr(hook) -> str:
    try:
        return f"0x{sc._to_int(hook['address']):08X}"
    except (KeyError, ValueError, TypeError):
        return "?"


def _allowed(path, hook, idx, src) -> bool:
    """True when this exact (file, address, parameter, source) is on the deliberate-address allowlist."""
    try:
        addr = sc._to_int(hook["address"])
    except (KeyError, ValueError, TypeError):
        return False
    norm = str(path).replace("\\", "/")
    return any(norm.endswith(f) and addr == a and idx == n and src.strip() == want
               for (f, a, n), want in ALLOWED_ADDRESS_SOURCES.items())


def check_file(path, rep: sc.Reporter) -> None:
    """Add one FAIL per unbracketed stack-relative parameter source in one hooks file to rep."""
    try:
        doc = tomllib.loads(Path(path).read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, tomllib.TOMLDecodeError) as e:
        rep.fail("parse", f"{_rel(path)}: {e}")
        return
    for hook in doc.get("hooks", []):
        if not isinstance(hook, dict):
            continue
        params = hook.get("parameters", [])
        for i, p in enumerate(params if isinstance(params, list) else [], 1):
            src = p.get("source") if isinstance(p, dict) else None
            if bad_source(src) and not _allowed(path, hook, i, src):
                rep.fail(CHECK, f'{_rel(path)} hook {_addr(hook)} parameter #{i}: source "{src.strip()}" passes the '
                                f'ADDRESS, not the value; fix: source = "{fix_for(src)}"')


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("paths", nargs="*", help="hooks.toml files (default: every hooks.toml under patch_manager_mods/)")
    a = ap.parse_args(argv)
    files = [Path(p) for p in a.paths] or find_hooks_files()
    rep = sc.Reporter(out=sys.stdout)
    for f in files:
        check_file(f, rep)
    return rep.finish(f"check_hook_sources: {len(files)} file(s)")


if __name__ == "__main__":
    sys.exit(main())
