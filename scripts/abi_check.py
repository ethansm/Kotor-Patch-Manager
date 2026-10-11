#!/usr/bin/env python3
"""
scripts/abi_check.py - ABI golden validation and append-only check for KOTOR-II interface contracts.

Validates that provider struct layouts, field offsets, and constants at -m32 adhere to
the append-only prefix rule within a major version (Doc 22 U6).

Rules:
  - Within a major version, the golden layout must be an append-only prefix of the current layout.
  - Existing fields cannot move, change size, or reorder.
  - New fields must be appended at the end (offset >= old sizeof), and struct_size must grow.
  - Constants must remain identical.
  - A breaking ABI change prints which field moved and which consumer mods read it (from requires_interfaces).

Usage:
  python3 scripts/abi_check.py
  python3 scripts/abi_check.py --header patch_manager_mods/_shared_src/PortraitSlots.h
  python3 scripts/abi_check.py --update
  python3 scripts/abi_check.py --dump-only
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any

if sys.version_info >= (3, 11):
    import tomllib
else:
    import tomli as tomllib  # type: ignore


SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent


def find_kpm_dir(repo_root: Path, explicit_kpm: Path | None = None) -> Path:
    if explicit_kpm:
        return explicit_kpm.resolve()
    env_kpm = os.environ.get("KPM_DIR")
    if env_kpm:
        return Path(env_kpm).resolve()
    default_path = Path.home() / "Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager"
    return default_path.resolve()


def find_interface_consumers(repo_root: Path, iface_tag: str) -> list[str]:
    """Find all mods in patch_manager_mods whose manifest requires the interface."""
    consumers: list[str] = []
    pm_root = repo_root / "patch_manager_mods"
    if not pm_root.is_dir():
        return consumers

    for mfile in pm_root.glob("*/manifest.toml"):
        try:
            data = tomllib.loads(mfile.read_text(encoding="utf-8"))
            patch = data.get("patch", {})
            req_ifaces = patch.get("requires_interfaces", [])
            if iface_tag in req_ifaces:
                consumers.append(mfile.parent.name)
        except Exception as exc:
            sys.stderr.write(f"Skipping unreadable manifest {mfile}: {exc}\n")
            continue
    return sorted(consumers)


def compile_and_dump_abi(
    repo_root: Path,
    kpm_dir: Path,
    dumper_src: Path,
    header_name: str = "PortraitSlots.h",
    override_header: Path | None = None,
) -> tuple[dict[str, Any], str | None]:
    """Compile abi_dump_<stem>.cpp at -m32, run it, and parse the output TOML."""
    if not dumper_src.is_file():
        return {}, f"ABI dump source file not found: {dumper_src}"

    tmp_dir = tempfile.mkdtemp(prefix="abi_check_")
    bin_path = Path(tmp_dir) / "abi_dump_bin"

    try:
        inc_dirs = [
            "-I", str(repo_root / "patch_manager_mods/_shared_src"),
            "-I", str(kpm_dir / "Patches/Common"),
            "-I", str(repo_root / "tests/include"),
        ]

        # If override_header is specified, create an include redirection dir
        if override_header:
            hdr_dir = Path(tmp_dir) / "inc_override"
            hdr_dir.mkdir(parents=True)
            target_name = header_name or "PortraitSlots.h"
            shutil.copy2(override_header, hdr_dir / target_name)
            inc_dirs = ["-I", str(hdr_dir)] + inc_dirs

        compile_cmd = [
            "g++", "-m32", "-O0",
            str(dumper_src),
            "-o", str(bin_path),
        ] + inc_dirs

        c_proc = subprocess.run(
            compile_cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        if c_proc.returncode != 0:
            return {}, f"Compilation failed (g++ -m32):\n{c_proc.stderr}"

        # Run compiled dump binary
        r_proc = subprocess.run(
            [str(bin_path)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        if r_proc.returncode != 0:
            return {}, f"ABI dumper binary failed:\n{r_proc.stderr}"

        toml_text = r_proc.stdout
        try:
            dumped_data = tomllib.loads(toml_text)
            return dumped_data, None
        except Exception as exc:
            return {}, f"Failed to parse dumped TOML: {exc}\nRaw output:\n{toml_text}"

    finally:
        shutil.rmtree(tmp_dir, ignore_errors=True)


def interface_identity_mismatch(golden_data: dict[str, Any], dumped_data: dict[str, Any]) -> str | None:
    """Return an error line when the dumped interface name/major differs from the golden, else None."""
    iface_golden = golden_data.get("interface", {})
    iface_dumped = dumped_data.get("interface", {})

    name = iface_golden.get("name", "")
    major = iface_golden.get("major", 1)
    tag = f"{name}@{major}"

    if iface_dumped.get("name") != name or iface_dumped.get("major") != major:
        return (
            f"Interface mismatch: golden has {tag}, dumped has {iface_dumped.get('name')}@{iface_dumped.get('major')}"
        )
    return None


def diff_abi_golden(
    golden_data: dict[str, Any],
    dumped_data: dict[str, Any],
    consumers: list[str],
) -> tuple[bool, list[str]]:
    """Compare dumped ABI against golden using append-only prefix rules."""
    errors: list[str] = []

    iface_golden = golden_data.get("interface", {})

    name = iface_golden.get("name", "")
    major = iface_golden.get("major", 1)
    tag = f"{name}@{major}"

    # 1. Interface name and major
    mismatch = interface_identity_mismatch(golden_data, dumped_data)
    if mismatch:
        errors.append(mismatch)

    # 2. Constants validation
    consts_golden = golden_data.get("constants", {})
    consts_dumped = dumped_data.get("constants", {})
    for c_key, c_val in consts_golden.items():
        if c_key not in consts_dumped:
            errors.append(f"Constant '{c_key}' missing in current interface (golden value: {c_val})")
        elif consts_dumped[c_key] != c_val:
            errors.append(
                f"Constant '{c_key}' changed from {c_val} to {consts_dumped[c_key]}"
            )

    # 3. Struct layouts validation (append-only prefix)
    structs_golden = golden_data.get("struct", {})
    structs_dumped = dumped_data.get("struct", {})

    for s_name, s_golden in structs_golden.items():
        if s_name not in structs_dumped:
            errors.append(f"Struct '{s_name}' missing in current interface")
            continue

        s_dumped = structs_dumped[s_name]
        golden_size = s_golden.get("sizeof", 0)
        dumped_size = s_dumped.get("sizeof", 0)

        if dumped_size < golden_size:
            errors.append(
                f"Struct '{s_name}' shrunk from {golden_size} to {dumped_size} bytes"
            )

        golden_fields = s_golden.get("fields", [])
        dumped_fields = s_dumped.get("fields", [])

        if len(dumped_fields) < len(golden_fields):
            errors.append(
                f"Struct '{s_name}' has fewer fields ({len(dumped_fields)}) than golden ({len(golden_fields)})"
            )

        # Check every golden field has matching offset and size at the exact same prefix index
        for idx, g_field in enumerate(golden_fields):
            fname = g_field.get("name")
            goffset = g_field.get("offset")
            gsize = g_field.get("size")

            if idx >= len(dumped_fields):
                errors.append(f"Struct '{s_name}' field '{fname}' was deleted/missing")
                break

            d_field = dumped_fields[idx]
            dname = d_field.get("name")
            doffset = d_field.get("offset")
            dsize = d_field.get("size")

            if dname != fname:
                errors.append(
                    f"Struct '{s_name}' field reordered: expected field '{fname}' at position {idx}, found '{dname}'"
                )
            elif doffset != goffset:
                errors.append(
                    f"Struct '{s_name}' field '{fname}' offset changed from {goffset} to {doffset}"
                )
            elif dsize != gsize:
                errors.append(
                    f"Struct '{s_name}' field '{fname}' size changed from {gsize} to {dsize}"
                )

        # Check any appended fields come after golden sizeof
        if len(dumped_fields) > len(golden_fields):
            for extra_field in dumped_fields[len(golden_fields):]:
                ex_name = extra_field.get("name")
                ex_offset = extra_field.get("offset", 0)
                if ex_offset < golden_size:
                    errors.append(
                        f"Struct '{s_name}' appended field '{ex_name}' overlaps golden boundary (offset {ex_offset} < {golden_size})"
                    )

    if errors:
        consumer_str = ", ".join(consumers) if consumers else "none"
        header_err = f"BREAKING ABI for {tag}:"
        detailed_errors = [header_err]
        for e in errors:
            detailed_errors.append(f"  - {e}")
        detailed_errors.append(f"Consumers reading {tag}: {consumer_str}")
        return False, detailed_errors

    return True, []


def dump_to_toml_str(data: dict[str, Any]) -> str:
    """Format ABI dump dictionary into clean TOML string."""
    lines: list[str] = []

    if "interface" in data:
        lines.append("[interface]")
        for k, v in data["interface"].items():
            if isinstance(v, str):
                lines.append(f'{k} = "{v}"')
            else:
                lines.append(f"{k} = {v}")
        lines.append("")

    if "constants" in data:
        lines.append("[constants]")
        for k, v in data["constants"].items():
            if isinstance(v, str):
                lines.append(f'{k} = "{v}"')
            else:
                lines.append(f"{k} = {v}")
        lines.append("")

    for s_name, s_info in data.get("struct", {}).items():
        lines.append(f"[struct.{s_name}]")
        lines.append(f"sizeof = {s_info.get('sizeof', 0)}")
        lines.append("fields = [")
        for f in s_info.get("fields", []):
            lines.append(
                f'    {{ name = "{f.get("name")}", offset = {f.get("offset")}, size = {f.get("size")} }},'
            )
        lines.append("]")
        lines.append("")

    return "\n".join(lines).strip() + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Verify ABI golden layouts and append-only rules for KOTOR-II interfaces (Doc 22 U6).",
    )
    parser.add_argument(
        "--golden",
        type=Path,
        default=None,
        help="Path to golden TOML file (default: tests/abi/*.toml).",
    )
    parser.add_argument(
        "--header",
        type=Path,
        default=None,
        help="Path to override interface header file to test against golden.",
    )
    parser.add_argument(
        "--update",
        action="store_true",
        help="Update the golden TOML file with the current dumped ABI.",
    )
    parser.add_argument(
        "--dump-only",
        action="store_true",
        help="Dump current ABI to stdout and exit.",
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

    args = parser.parse_args()
    if args.update and args.header:
        sys.stderr.write("--update cannot be combined with --header (would rebaseline the golden from a scratch header).\n")
        return 2
    repo_root = (args.repo or REPO_ROOT).resolve()
    kpm_dir = find_kpm_dir(repo_root, args.kpm_dir)

    # Locate golden files
    golden_files: list[Path] = []
    if args.golden:
        golden_files.append(args.golden.resolve())
    else:
        abi_dir = repo_root / "tests/abi"
        if abi_dir.is_dir():
            golden_files = sorted(abi_dir.glob("*.toml"))

    if not golden_files:
        sys.stderr.write("No ABI golden files found under tests/abi/.\n")
        return 2

    overall_exit = 0

    for g_path in golden_files:
        try:
            golden_data = tomllib.loads(g_path.read_text(encoding="utf-8"))
        except Exception as exc:
            sys.stderr.write(f"Error reading golden {g_path}: {exc}\n")
            overall_exit = 2
            continue

        iface_info = golden_data.get("interface", {})
        iface_name = iface_info.get("name", "")
        iface_major = iface_info.get("major", 1)
        iface_tag = f"{iface_name}@{iface_major}"

        # Match corresponding abi_dump_<stem>.cpp
        stem_simple = iface_name.replace("k2.", "").replace(".", "_")
        dumper_src = repo_root / f"tests/abi/abi_dump_{stem_simple}.cpp"
        if not dumper_src.is_file():
            # Try direct match
            candidates = sorted((repo_root / "tests/abi").glob("abi_dump_*.cpp"))
            if candidates:
                dumper_src = candidates[0]
                sys.stderr.write(f"No dumper for {iface_tag}; falling back to {dumper_src.name}\n")
            else:
                sys.stderr.write(f"No dumper found for {iface_tag} (expected {dumper_src})\n")
                overall_exit = 2
                continue

        hdr_target = Path(iface_info.get("header", "PortraitSlots.h")).name
        dumped_data, err = compile_and_dump_abi(repo_root, kpm_dir, dumper_src, hdr_target, args.header)
        if err:
            sys.stderr.write(f"Error checking {iface_tag}: {err}\n")
            overall_exit = 2
            continue

        if args.dump_only:
            print(dump_to_toml_str(dumped_data))
            continue

        if args.update:
            # Refuse to rebaseline a golden from another interface's dumper (fallback to candidates[0]).
            mismatch = interface_identity_mismatch(golden_data, dumped_data)
            if mismatch:
                sys.stderr.write(
                    f"Refusing to update golden {g_path}: {mismatch} (dumper {dumper_src.name}); golden not written.\n"
                )
                overall_exit = 2
                continue
            toml_content = dump_to_toml_str(dumped_data)
            g_path.write_text(toml_content, encoding="utf-8")
            print(f"Updated golden {g_path}")
            continue

        # Look up consumers
        consumers = find_interface_consumers(repo_root, iface_tag)

        # Validate against golden
        ok, diff_errors = diff_abi_golden(golden_data, dumped_data, consumers)
        if ok:
            print(f"PASS abi_check: {iface_tag} (matches golden {g_path.relative_to(repo_root)}, append-only OK)")
        else:
            for line in diff_errors:
                print(line)
            overall_exit = 1

    return overall_exit


if __name__ == "__main__":
    sys.exit(main())
