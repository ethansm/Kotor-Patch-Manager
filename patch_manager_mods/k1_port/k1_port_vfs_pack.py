#!/usr/bin/env python3
"""Deterministic C/C++ header pack generator for modular in-memory character VFS bundles.

Packs staged assets (.mdl, .mdx, .tpc, .txi, .utc, .dlg, .ssf, .lip, stream .wav/.mp3)
and virtual 2DA cell definitions (appearance, heads, soundset, portraits) into a
deterministic C/C++ header (e.g. juhani_vfs_bundle.h) for compiling directly into a
single DLL / kpatch without loose file footprints.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import re
import struct
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent / "kvoice"))

# Flag definitions
K2VFS_FLAG_RESMAN = 0
K2VFS_FLAG_STREAM = 1

# Resource type mapping
EXTENSION_TO_RESTYPE = {
    ".mdl": 2002,
    ".mdx": 3008,
    ".tpc": 3007,
    ".txi": 2022,
    ".utc": 2027,
    ".dlg": 2029,
    ".ncs": 2010,
    ".ssf": 2060,
    ".lip": 3004,
    ".wav": 4,
    ".mp3": 4,
}

RESREF_RE = re.compile(r"^[a-z0-9_]{1,16}$")
STREAM_DIRS = ("streamvoice", "streamsounds")


class PackError(Exception):
    """Raised on invalid asset, corrupt file, or refusal by pack gates."""
    pass


@dataclass(frozen=True)
class TwoDaCell:
    table: str
    row: int
    column: str
    value: str
    template_row: int = -1

    def __post_init__(self):
        norm_table = self.table.lower()
        if norm_table.endswith(".2da"):
            norm_table = norm_table[:-4]
        object.__setattr__(self, "table", norm_table)
        if self.template_row == -1 and norm_table == "appearance":
            object.__setattr__(self, "template_row", 17)


def c_escape_string(s: str) -> str:
    """Escape a string for C/C++ string literals."""
    res = []
    for ch in s:
        if ch == "\\":
            res.append("\\\\")
        elif ch == '"':
            res.append('\\"')
        elif ch == "\n":
            res.append("\\n")
        elif ch == "\r":
            res.append("\\r")
        elif ch == "\t":
            res.append("\\t")
        elif 32 <= ord(ch) <= 126:
            res.append(ch)
        else:
            res.append(f"\\x{ord(ch):02x}")
    return "".join(res)


def format_c_bytes(data: bytes, indent: str = "    ", per_line: int = 12) -> str:
    """Format raw bytes as C hex array lines."""
    if not data:
        return indent + "0x00\n"
    lines = []
    for i in range(0, len(data), per_line):
        chunk = data[i : i + per_line]
        hex_bytes = ", ".join(f"0x{b:02x}" for b in chunk)
        if i + per_line < len(data):
            hex_bytes += ","
        lines.append(indent + hex_bytes)
    return "\n".join(lines)


def validate_lip_data(data: bytes, path_display: str = "") -> None:
    """Validate LIP file structure so corrupt LIPs that crash 0x0091CF5B are refused."""
    prefix = f"corrupt .lip file ({path_display})" if path_display else "corrupt .lip file"
    if len(data) < 16:
        raise PackError(f"{prefix}: short file ({len(data)} bytes, minimum 16)")
    if not data.startswith(b"LIP V1.0"):
        raise PackError(f"{prefix}: bad signature {data[:8]!r}, expected b'LIP V1.0'")

    # Try kvoice_lip.check_lip if available
    try:
        import kvoice_lip
        problems = kvoice_lip.check_lip(data)
        if problems:
            raise PackError(f"{prefix}: {'; '.join(problems)}")
        return
    except ImportError:
        pass

    # Fallback validation matching kvoice_lip rules
    try:
        length, count = struct.unpack_from("<fI", data, 8)
    except struct.error as exc:
        raise PackError(f"{prefix}: unpack error: {exc}")

    problems = []
    if not math.isfinite(length) or length <= 0:
        problems.append(f"length {length!r} is not a positive finite number")
    if len(data) != 16 + 5 * count:
        problems.append(f"size {len(data)} != 16 + 5*{count}")
    if count == 0:
        problems.append("no keys")
    else:
        times = []
        for i in range(count):
            t, s = struct.unpack_from("<fB", data, 16 + 5 * i)
            if not math.isfinite(t):
                problems.append(f"non-finite key time at {i}")
            if s > 15:
                problems.append(f"shape {s} at key {i} outside 0-15")
            times.append(t)
        if times and times[0] != 0.0:
            problems.append(f"first key time {times[0]:.6f} != 0")
        if times and abs(times[-1] - length) > length * 1e-5 + 1e-9:
            problems.append(f"last key {times[-1]:.6f} != length {length:.6f}")
        if any(b < a for a, b in zip(times, times[1:])):
            problems.append("times not monotonic")

    if problems:
        raise PackError(f"{prefix}: {'; '.join(problems)}")


def validate_resman_wav_data(data: bytes, path_display: str = "") -> None:
    """Validate that a Layer 1 (K2VFS_FLAG_RESMAN / PlaySound) WAV is bare RIFF WAVE PCM.

    Live tests E2b/E2c showed that Layer 1 PlaySound only plays genuine RIFF WAVE PCM:
    58-byte StreamVoice MP3 containers (RIFF size == 50) play silently on Layer 1, and
    470-byte (0x1D6) StreamSounds prefixes or raw MP3 belong on Layer 3 (K2VFS_FLAG_STREAM).
    """
    prefix = f"invalid Layer 1 ResMan WAV ({path_display})" if path_display else "invalid Layer 1 ResMan WAV"
    if len(data) > 0x1D6 and data[0x1D6 : 0x1DA] == b"RIFF":
        raise PackError(
            f"{prefix}: 470-byte (0x1D6) StreamSounds container cannot be served on Layer 1 ResMan; "
            "place in streamsounds/ or pass via --stream-files (K2VFS_FLAG_STREAM)"
        )
    if len(data) < 12 or data[:4] != b"RIFF" or data[8:12] != b"WAVE":
        raise PackError(
            f"{prefix}: Layer 1 (PlaySound) requires bare RIFF WAVE PCM (.wav); "
            "raw MP3 or non-RIFF audio must be transcoded to RIFF PCM or placed in streamvoice/ (K2VFS_FLAG_STREAM)"
        )
    riff_sz = struct.unpack_from("<I", data, 4)[0]
    if riff_sz == 50 and len(data) > 58:
        raise PackError(
            f"{prefix}: 58-byte StreamVoice MP3 container (RIFF size 50) is silent on Layer 1 PlaySound; "
            "place in streamvoice/ or pass via --stream-files (K2VFS_FLAG_STREAM)"
        )


def check_license_gate(paths: Iterable[Path], ledger_path: str | Path, stage_root: str | Path | None = None) -> None:
    """Run k1_port_prune license gate check on input asset paths."""
    from types import SimpleNamespace
    try:
        from k1_port_sources import Ledger
        import k1_port_prune as kp
    except ImportError as e:
        raise PackError(f"cannot import k1_port_prune for license gate: {e}")

    led_p = Path(ledger_path).resolve()
    if not led_p.is_file():
        raise PackError(f"ledger file not found: {led_p}")
    led = Ledger(led_p)
    stage = Path(stage_root).resolve() if stage_root else led_p.parent
    args = SimpleNamespace(paths=[str(p) for p in paths], strict=True)
    ret = kp.cmd_gate(led, stage, args)
    if ret != 0:
        raise PackError(f"License gate check failed for input assets against ledger {led_p}")


def collect_assets(
    asset_paths: Iterable[str | Path],
    stream_files: Iterable[str | Path] | None = None,
) -> list[dict[str, Any]]:
    """Gather, validate, and categorize all asset files (refusing symlinks per kvoice R9)."""
    stream_set: set[Path] = set()
    for sp in (stream_files or []):
        raw_sp = Path(sp)
        if raw_sp.is_symlink():
            raise PackError(f"symlinks are not allowed in stream_files: {raw_sp}")
        stream_set.add(raw_sp.resolve())

    raw_files: list[Path] = []
    for raw in asset_paths:
        raw_p = Path(raw)
        if raw_p.is_symlink():
            raise PackError(f"symlinks are not allowed in asset_paths: {raw_p}")
        p = raw_p.resolve()
        if not p.exists():
            raise PackError(f"asset path does not exist: {raw}")
        if p.is_dir():
            for root, dirs, files in os.walk(p, followlinks=False):
                dirs.sort()
                root_p = Path(root)
                for d in dirs:
                    if (root_p / d).is_symlink():
                        raise PackError(f"symlinked directory is not allowed in stage: {root_p / d}")
                for fn in sorted(files):
                    fp = root_p / fn
                    if fp.is_symlink():
                        raise PackError(f"symlinked file is not allowed in stage: {fp}")
                    raw_files.append(fp.resolve())
        elif p.is_file():
            raw_files.append(p)
        else:
            raise PackError(f"not a regular file or directory: {raw}")

    # Deduplicate preserving order
    unique_files: list[Path] = []
    seen_files = set()
    for f in raw_files:
        if f not in seen_files:
            seen_files.add(f)
            unique_files.append(f)

    assets: list[dict[str, Any]] = []
    seen_keys = set()

    for f in unique_files:
        ext = f.suffix.lower()
        if ext == ".tga":
            raise PackError("loose .tga textures bypass CExoResMan::Demand via ReadRaw; convert to .tpc first")
        if ext == ".2da":
            raise PackError(
                f"physical .2da files cannot be packed as raw assets ({f}); "
                "2DA rows must be emitted as K2Vfs2DaCell entries, never whole-file overrides"
            )
        if ext not in EXTENSION_TO_RESTYPE:
            raise PackError(f"unsupported file extension {ext!r} for asset {f}")

        stem = f.stem.lower()
        if not RESREF_RE.fullmatch(stem):
            raise PackError(f"invalid resref stem {stem!r} for {f}: must match ^[a-z0-9_]{{1,16}}$")

        res_type = EXTENSION_TO_RESTYPE[ext]

        # Flags calculation:
        # - Layer 3 (K2VFS_FLAG_STREAM = 1): Dialog VO (streamvoice/) and TLK/SSF barks (streamsounds/ or --stream-files)
        # - Layer 1 (K2VFS_FLAG_RESMAN = 0): Script PlaySound SFX (must be bare RIFF WAVE PCM .wav)
        if ext in (".wav", ".mp3"):
            is_stream = f in stream_set or any(part.lower() in STREAM_DIRS for part in f.parts)
            flags = K2VFS_FLAG_STREAM if is_stream else K2VFS_FLAG_RESMAN
            if flags == K2VFS_FLAG_RESMAN and ext == ".mp3":
                raise PackError(
                    f"bare .mp3 ({f}) is never queried by Layer 1 PlaySound (live test E2c); "
                    "transcode to RIFF PCM .wav for Layer 1 or place in streamvoice/streamsounds for Layer 3"
                )
        else:
            flags = K2VFS_FLAG_RESMAN

        key = (flags, res_type, stem)
        if key in seen_keys:
            raise PackError(
                f"duplicate asset entry for resref {stem!r} (type {res_type}, flags {flags}); "
                "note that .wav and .mp3 share res_type=4 and collide if given the same stem in the same layer"
            )
        seen_keys.add(key)

        data = f.read_bytes()
        if ext == ".lip":
            validate_lip_data(data, str(f))
        elif ext == ".wav" and flags == K2VFS_FLAG_RESMAN:
            validate_resman_wav_data(data, str(f))

        sha = hashlib.sha256(data).hexdigest()
        assets.append({
            "source_path": f,
            "stem": stem,
            "resref": stem,
            "extension": ext,
            "res_type": res_type,
            "flags": flags,
            "data": data,
            "size": len(data),
            "sha256": sha,
        })

    # Sort deterministically by (flags, res_type, resref)
    assets.sort(key=lambda a: (a["flags"], a["res_type"], a["resref"]))
    for idx, a in enumerate(assets):
        a["idx"] = idx
        a["symbol"] = f"kAsset_{idx}_{a['stem']}"

    return assets


def parse_2da_cells(
    cells: Iterable[Any] | None = None,
    rows: dict[str, dict[int, dict[str, Any]]] | None = None,
) -> list[TwoDaCell]:
    """Parse virtual 2DA cells from list of cell dicts/tuples or nested row dicts."""
    parsed: list[TwoDaCell] = []
    if cells:
        for c in cells:
            if isinstance(c, TwoDaCell):
                parsed.append(c)
            elif isinstance(c, dict):
                t_row = int(c.get("template_row", -1))
                if "cells" in c:
                    table = str(c["table"])
                    row = int(c["row"])
                    for col, val in c["cells"].items():
                        parsed.append(TwoDaCell(table=table, row=row, column=str(col), value=str(val), template_row=t_row))
                else:
                    parsed.append(TwoDaCell(
                        table=str(c["table"]),
                        row=int(c["row"]),
                        column=str(c["column"]),
                        value=str(c.get("value", "")),
                        template_row=t_row,
                    ))
            elif isinstance(c, (list, tuple)) and len(c) in (4, 5):
                t_row = int(c[4]) if len(c) == 5 else -1
                parsed.append(TwoDaCell(
                    table=str(c[0]),
                    row=int(c[1]),
                    column=str(c[2]),
                    value=str(c[3]),
                    template_row=t_row,
                ))
            else:
                raise PackError(f"unrecognized 2DA cell format: {c!r}")

    if rows:
        for table, row_map in rows.items():
            for row, col_map in row_map.items():
                for col, val in col_map.items():
                    parsed.append(TwoDaCell(
                        table=str(table),
                        row=int(row),
                        column=str(col),
                        value=str(val),
                    ))

    # Sort deterministically by (table, row, column)
    parsed.sort(key=lambda c: (c.table, c.row, c.column))
    return parsed


def generate_c_header(
    character_id: str,
    assets: list[dict[str, Any]],
    cells: list[TwoDaCell],
    *,
    display_name: str | None = None,
    default_tag: str | None = None,
    default_utc: str | None = None,
    default_dlg: str | None = None,
    appearance_id: int | None = None,
    head_id: int | None = None,
    soundset_id: int | None = None,
) -> str:
    """Emit the complete C/C++ header for the character VFS bundle."""
    clean_char_id = re.sub(r"[^a-zA-Z0-9_]", "_", character_id.lower())
    disp_name = display_name or character_id.capitalize()
    tag = default_tag or f"k1p_{clean_char_id}"
    utc = default_utc or tag

    if default_dlg is None:
        dlg_assets = [a for a in assets if a["extension"] == ".dlg"]
        dlg = dlg_assets[0]["resref"] if dlg_assets else ""
    else:
        dlg = default_dlg

    if appearance_id is None:
        app_rows = [c.row for c in cells if c.table == "appearance"]
        app_id = min(app_rows) if app_rows else 0
    else:
        app_id = appearance_id

    if head_id is None:
        head_rows = [c.row for c in cells if c.table == "heads"]
        h_id = min(head_rows) if head_rows else 0
    else:
        h_id = head_id

    if soundset_id is None:
        snd_rows = [c.row for c in cells if c.table == "soundset"]
        s_id = min(snd_rows) if snd_rows else 0
    else:
        s_id = soundset_id

    lines = [
        "// Generated by k1_port_vfs_pack.py. DO NOT EDIT.",
        "#pragma once",
        "",
        '#include "CharacterProvider.h"',
        "#include <stdint.h>",
        "#include <stddef.h>",
        "",
        f"namespace k2vfs_{clean_char_id} {{",
        "",
    ]

    # Emit binary asset byte arrays
    for a in assets:
        lines.append(f"// {a['resref']}{a['extension']} ({a['size']} bytes, sha256={a['sha256'][:16]}...)")
        lines.append(f"alignas(8) static const uint8_t {a['symbol']}[] = {{")
        lines.append(format_c_bytes(a["data"], indent="    ", per_line=12))
        lines.append("};")
        lines.append("")

    # Emit kAssets table
    if assets:
        lines.append("static const K2VfsAssetEntry kAssets[] = {")
        for a in assets:
            lines.append(
                f'    {{ "{a["resref"]}", {a["res_type"]}, {a["flags"]}, {a["symbol"]}, sizeof({a["symbol"]}) }},'
            )
        lines.append("};")
        lines.append("static const uint32_t kAssetCount = static_cast<uint32_t>(sizeof(kAssets) / sizeof(kAssets[0]));")
    else:
        lines.append("static const K2VfsAssetEntry kAssets[1] = {")
        lines.append('    { "", 0, 0, nullptr, 0 }')
        lines.append("};")
        lines.append("static const uint32_t kAssetCount = 0;")
    lines.append("")

    # Emit k2DaCells table (matches K2Vfs2DaCell: { table_name, row_index, template_row, column_name, value })
    if cells:
        lines.append("static const K2Vfs2DaCell k2DaCells[] = {")
        for c in cells:
            lines.append(
                f'    {{ "{c_escape_string(c.table)}", {c.row}, {c.template_row}, "{c_escape_string(c.column)}", "{c_escape_string(c.value)}" }},'
            )
        lines.append("};")
        lines.append("static const uint32_t k2DaCellCount = static_cast<uint32_t>(sizeof(k2DaCells) / sizeof(k2DaCells[0]));")
    else:
        lines.append("static const K2Vfs2DaCell k2DaCells[1] = {")
        lines.append('    { "", 0, -1, "", "" }')
        lines.append("};")
        lines.append("static const uint32_t k2DaCellCount = 0;")
    lines.append("")

    # Emit kCharacterDef
    lines.extend([
        "static const K2CharacterDefinition kCharacterDef = {",
        "    sizeof(K2CharacterDefinition),",
        f'    "{c_escape_string(clean_char_id)}",',
        f'    "{c_escape_string(disp_name)}",',
        f'    "{c_escape_string(tag)}",',
        f'    "{c_escape_string(utc)}",',
        f'    "{c_escape_string(dlg)}",',
        f"    {app_id},",
        f"    {h_id},",
        f"    {s_id}",
        "};",
        "",
        "inline bool registerAll(const K2AssetVfsApi* vfs, const K2CharacterLibApi* charlib, const char* owner) {",
        "    if (!vfs) {",
        "        return false;",
        "    }",
        f'    const char* pkgOwner = (owner && *owner) ? owner : "{c_escape_string(clean_char_id)}";',
        "    if (kAssetCount > 0) {",
        "        if (vfs->RegisterAssets) {",
        "            if (vfs->RegisterAssets(kAssets, kAssetCount, pkgOwner) < 0) {",
        "                return false;",
        "            }",
        "        } else if (vfs->RegisterAsset) {",
        "            for (uint32_t i = 0; i < kAssetCount; ++i) {",
        "                if (vfs->RegisterAsset(&kAssets[i], pkgOwner) < 0) {",
        "                    return false;",
        "                }",
        "            }",
        "        } else {",
        "            return false;",
        "        }",
        "    }",
        "    if (k2DaCellCount > 0) {",
        "        if (!vfs->Register2DaCells || vfs->Register2DaCells(k2DaCells, k2DaCellCount, pkgOwner) < 0) {",
        "            return false;",
        "        }",
        "    }",
        "    if (charlib && charlib->RegisterCharacter) {",
        "        if (charlib->RegisterCharacter(&kCharacterDef, pkgOwner) <= 0) {",
        "            return false;",
        "        }",
        "    }",
        "    return true;",
        "}",
        "",
        f"}} // namespace k2vfs_{clean_char_id}",
        "",
    ])

    return "\n".join(lines)


def pack_character_bundle(
    character_id: str,
    asset_paths: Iterable[str | Path],
    output_header: str | Path,
    *,
    display_name: str | None = None,
    default_tag: str | None = None,
    default_utc: str | None = None,
    default_dlg: str | None = None,
    appearance_id: int | None = None,
    head_id: int | None = None,
    soundset_id: int | None = None,
    two_da_cells: Iterable[Any] | None = None,
    two_da_rows: dict[str, dict[int, dict[str, Any]]] | None = None,
    stream_files: Iterable[str | Path] | None = None,
    ledger_path: str | Path | None = None,
    stage_root: str | Path | None = None,
) -> dict[str, Any]:
    """Pack assets and virtual 2DA cells into a deterministic C/C++ header bundle."""
    if not character_id or not re.fullmatch(r"^[a-zA-Z0-9_]{1,32}$", character_id):
        raise PackError(f"invalid character_id {character_id!r}: must match ^[a-zA-Z0-9_]{{1,32}}$")

    # Collect and validate assets
    assets = collect_assets(asset_paths, stream_files=stream_files)

    # Optional license gate
    if ledger_path is not None:
        check_license_gate([a["source_path"] for a in assets], ledger_path, stage_root=stage_root)

    # Parse 2DA cells
    cells = parse_2da_cells(cells=two_da_cells, rows=two_da_rows)

    header_content = generate_c_header(
        character_id=character_id,
        assets=assets,
        cells=cells,
        display_name=display_name,
        default_tag=default_tag,
        default_utc=default_utc,
        default_dlg=default_dlg,
        appearance_id=appearance_id,
        head_id=head_id,
        soundset_id=soundset_id,
    )

    out_p = Path(output_header).resolve()
    out_p.parent.mkdir(parents=True, exist_ok=True)
    out_p.write_text(header_content, encoding="utf-8")

    header_sha = hashlib.sha256(header_content.encode("utf-8")).hexdigest()

    app_id = (
        appearance_id
        if appearance_id is not None
        else (min([c.row for c in cells if c.table == "appearance"]) if [c for c in cells if c.table == "appearance"] else 0)
    )
    h_id = (
        head_id
        if head_id is not None
        else (min([c.row for c in cells if c.table == "heads"]) if [c for c in cells if c.table == "heads"] else 0)
    )
    s_id = (
        soundset_id
        if soundset_id is not None
        else (min([c.row for c in cells if c.table == "soundset"]) if [c for c in cells if c.table == "soundset"] else 0)
    )

    clean_char_id = re.sub(r"[^a-zA-Z0-9_]", "_", character_id.lower())
    disp_name = display_name or character_id.capitalize()
    tag = default_tag or f"k1p_{clean_char_id}"
    utc = default_utc or tag
    if default_dlg is None:
        dlg_assets = [a for a in assets if a["extension"] == ".dlg"]
        dlg = dlg_assets[0]["resref"] if dlg_assets else ""
    else:
        dlg = default_dlg

    manifest = {
        "character_id": character_id,
        "header_path": str(out_p),
        "header_sha256": header_sha,
        "asset_count": len(assets),
        "cell_count": len(cells),
        "total_bytes": sum(a["size"] for a in assets),
        "assets": [
            {
                "stem": a["stem"],
                "resref": a["resref"],
                "extension": a["extension"],
                "res_type": a["res_type"],
                "flags": a["flags"],
                "size": a["size"],
                "sha256": a["sha256"],
                "symbol": a["symbol"],
                "source_path": str(a["source_path"]),
            }
            for a in assets
        ],
        "cells": [
            {
                "table": c.table,
                "row": c.row,
                "column": c.column,
                "value": c.value,
            }
            for c in cells
        ],
        "character_def": {
            "character_id": clean_char_id,
            "display_name": disp_name,
            "default_tag": tag,
            "default_utc": utc,
            "default_dlg": dlg,
            "appearance_id": app_id,
            "head_id": h_id,
            "soundset_id": s_id,
        },
    }

    return manifest


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Pack character assets and virtual 2DA cells into a C header bundle.")
    parser.add_argument("--character-id", required=True, help="Character identifier (e.g. juhani)")
    parser.add_argument("--assets", nargs="+", required=True, help="Asset files or directories to include")
    parser.add_argument("--out-header", required=True, help="Output C/C++ header path")
    parser.add_argument("--display-name", help="Display name (default: capitalized character-id)")
    parser.add_argument("--tag", help="Default creature tag (default: k1p_<character-id>)")
    parser.add_argument("--utc", help="Default creature template resref (default: tag)")
    parser.add_argument("--dlg", help="Default conversation resref (default: first packed .dlg)")
    parser.add_argument("--appearance-id", type=int, help="Virtual appearance.2da row ID")
    parser.add_argument("--head-id", type=int, help="Virtual heads.2da row ID")
    parser.add_argument("--soundset-id", type=int, help="Virtual soundset.2da row ID")
    parser.add_argument("--stream-files", nargs="*", default=[], help="Audio files to mark as streaming audio")
    parser.add_argument("--ledger", help="Optional path to k1_port_prune ledger to gate assets")
    parser.add_argument("--2da-json", help="Optional path to JSON file with virtual 2DA cells")
    parser.add_argument("--json", action="store_true", help="Print manifest JSON to stdout")

    args = parser.parse_args(argv)

    cells = []
    if args.__dict__.get("2da_json"):
        p = Path(args.__dict__["2da_json"]).resolve()
        data = json.loads(p.read_text(encoding="utf-8"))
        if isinstance(data, list):
            cells = parse_2da_cells(cells=data)
        elif isinstance(data, dict):
            cells = parse_2da_cells(rows=data)

    try:
        manifest = pack_character_bundle(
            character_id=args.character_id,
            asset_paths=args.assets,
            output_header=args.out_header,
            display_name=args.display_name,
            default_tag=args.tag,
            default_utc=args.utc,
            default_dlg=args.dlg,
            appearance_id=args.appearance_id,
            head_id=args.head_id,
            soundset_id=args.soundset_id,
            two_da_cells=cells,
            stream_files=args.stream_files,
            ledger_path=args.ledger,
        )
    except PackError as e:
        print(f"error: {e}", file=sys.stderr)
        return 1

    if args.json:
        print(json.dumps(manifest, indent=2))
    else:
        print(
            f"packed {manifest['asset_count']} asset(s) and {manifest['cell_count']} 2DA cell(s) "
            f"({manifest['total_bytes']} bytes) -> {manifest['header_path']} "
            f"[sha256={manifest['header_sha256'][:16]}...]"
        )
    return 0


if __name__ == "__main__":
    sys.exit(main())
