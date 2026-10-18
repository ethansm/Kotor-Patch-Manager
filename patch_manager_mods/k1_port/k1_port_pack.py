#!/usr/bin/env python3
"""`.k2pack` writer and reader (format version 1), stdlib only.

Spec (authoritative): K2PACK_FORMAT.md next to this file. One pack = one imported character:
a 32-byte header, a 208-byte character block, a 32-byte-per-entry asset table, a 120-byte-per-entry
2DA cell table, then 8-byte-aligned asset blobs. Little-endian, CRC-32 over [32, total_size).
The C++ loader (character_packs_src/PackFormat.h) parses the same bytes.

API
  assets_from_stage(paths, stream_files=None) -> list[Asset]   (validation by k1_port_vfs_pack.collect_assets)
  cells_from_vfs_pack(cells) -> list[Cell]                     (adapter from k1_port_vfs_pack.TwoDaCell)
  write_pack(path, character, assets, cells) -> dict           (summary: size, crc32, counts, sha256)
  read_pack(data) -> (CharacterInfo, [Asset], [Cell])          (raises PackFormatError on the first failed rule)

CLI
  k1_port_pack.py inspect FILE [--json]   print a pack; exit 2 on a bad or missing pack
  k1_port_pack.py fixture OUT             write the deterministic synthetic fixture pack (no game data)

Fixture (used by the C++ round-trip test; sha256 pinned in test_k1_port_pack.py)
  character: id `fixture`, display_name `Fixture`, default_tag `fixture_tag`, default_utc `fixture_utc`,
             default_dlg `` (empty), appearance_id 900, head_id 300, soundset_id 11
  assets, in file order (flags, res_type, resref -> bytes):
    0  RESMAN  2002 mdl  `fixture_mdl`       24 bytes  b"K2PKTEST" * 3
    1  RESMAN  2022 txi  `fixture_sixteen_`   8 bytes  b"K2PKTXI\\n"          (16-char resref, no NUL)
    2  RESMAN  2027 utc  `fixture_utc`       21 bytes  b"K2PKUTC:fake-utc-blob"
    3  STREAM     4 wav  `fixture_vo`          7 bytes  b"K2PKWAV"
  cells, in file order (table, row, template_row, column = value):
    0  appearance  900  template 17  label = `Fixture`
    1  heads       300  template -1  heads = `fixture_hd`
  Layout: blob start 240 + 32*4 + 120*2 = 608; blobs at 608, 632, 640, 664 (each padded to 8 with
  zeros), total_size 671.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import struct
import sys
import zlib
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any, Iterable

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent / "kvoice"))

import k1_port_vfs_pack as _vp  # noqa: E402
from k1_port_vfs_pack import PackError  # noqa: E402,F401  (re-exported)

MAGIC = b"K2PK"
VERSION = 1
HEADER_SIZE = 32
CHARACTER_SIZE = 208
ASSET_ENTRY_SIZE = 32
CELL_ENTRY_SIZE = 120
FIXED_SIZE = HEADER_SIZE + CHARACTER_SIZE  # 240
MAX_ASSETS = 192
MAX_CELLS = 256
MAX_FILE_BYTES = 256 * 1024 * 1024
U32_MAX = 0xFFFFFFFF
I32_MAX = 0x7FFFFFFF

FLAG_RESMAN = 0
FLAG_STREAM = 1
RES_TYPE_WAV = 4
# res_type -> extension name. 3 (tga) and 2017 (2da) are deliberately absent (refused).
RES_TYPES = {
    2002: "mdl", 3008: "mdx", 3007: "tpc", 2022: "txi", 2027: "utc",
    2029: "dlg", 2010: "ncs", 2060: "ssf", 3004: "lip", 4: "wav",
}
REFUSED_RES_TYPES = {3: "tga", 2017: "2da"}

_HEADER = struct.Struct("<4sIIIIIII")
_CHARACTER = struct.Struct("<32s64s32s32s32sIIII")
_ASSET = struct.Struct("<16sHHIII")
_CELL = struct.Struct("<16sIi32s64s")
assert _HEADER.size == HEADER_SIZE and _CHARACTER.size == CHARACTER_SIZE
assert _ASSET.size == ASSET_ENTRY_SIZE and _CELL.size == CELL_ENTRY_SIZE

_ID_RE = re.compile(r"[a-z0-9_]{1,31}")
_RESREF_RE = re.compile(r"[a-z0-9_]{1,16}")
_PRINTABLE_RE = re.compile(r"[\x20-\x7e]*")
_COLUMN_RE = re.compile(r"[\x21-\x7e]{1,31}")


class PackFormatError(ValueError):
    """Raised by read_pack with the first failed reader rule."""


@dataclass
class CharacterInfo:
    character_id: str
    display_name: str
    default_tag: str
    default_utc: str
    default_dlg: str
    appearance_id: int
    head_id: int
    soundset_id: int


@dataclass
class Cell:
    table: str
    row: int
    template_row: int
    column: str
    value: str


@dataclass
class Asset:
    resref: str
    res_type: int
    flags: int
    data: bytes


# --------------------------------------------------------------------------- rule checks
# Each returns an error string or None; the writer raises PackError, the reader PackFormatError.

def _check_text(label: str, s: Any, pattern: re.Pattern[str], maxlen: int, minlen: int = 0) -> str | None:
    if not isinstance(s, str):
        return f"{label}: not a string"
    if len(s) > maxlen:
        return f"{label}: {len(s)} chars exceeds the {maxlen}-char limit"
    if len(s) < minlen:
        return f"{label}: shorter than {minlen} char(s)"
    if not s.isascii() or not pattern.fullmatch(s):
        return f"{label}: {s!r} violates the field charset"
    return None


def _check_u32(label: str, v: Any) -> str | None:
    if not isinstance(v, int) or isinstance(v, bool) or not 0 <= v <= U32_MAX:
        return f"{label}: {v!r} is not a u32"
    return None


def _check_character(c: CharacterInfo) -> str | None:
    checks = (
        _check_text("character_id", c.character_id, _ID_RE, 31, 1),
        _check_text("display_name", c.display_name, _PRINTABLE_RE, 63, 1),
        _check_text("default_tag", c.default_tag, _PRINTABLE_RE, 31),
        _check_text("default_utc", c.default_utc, re.compile(r"[a-z0-9_]{0,16}"), 16),
        _check_text("default_dlg", c.default_dlg, re.compile(r"[a-z0-9_]{0,16}"), 16),
        _check_u32("appearance_id", c.appearance_id),
        _check_u32("head_id", c.head_id),
        _check_u32("soundset_id", c.soundset_id),
    )
    return next((e for e in checks if e), None)


def _check_asset_meta(a: Asset) -> str | None:
    err = _check_text("resref", a.resref, _RESREF_RE, 16, 1)
    if err:
        return err
    if a.res_type in REFUSED_RES_TYPES:
        return f"asset {a.resref}: res_type {a.res_type} ({REFUSED_RES_TYPES[a.res_type]}) is refused"
    if a.res_type not in RES_TYPES:
        return f"asset {a.resref}: unknown res_type {a.res_type!r}"
    if a.flags not in (FLAG_RESMAN, FLAG_STREAM):
        return f"asset {a.resref}: flags {a.flags!r} must be 0 (RESMAN) or 1 (STREAM)"
    if a.flags == FLAG_STREAM and a.res_type != RES_TYPE_WAV:
        return f"asset {a.resref}: STREAM flag is only valid for res_type 4 (wav), got {a.res_type}"
    return None


def _check_cell(c: Cell) -> str | None:
    err = _check_text("cell table", c.table, _RESREF_RE, 16, 1)
    if err:
        return err
    where = f"cell {c.table}[{c.row}].{c.column}"
    err = _check_u32(f"{where} row", c.row)
    if err:
        return err
    if not isinstance(c.template_row, int) or isinstance(c.template_row, bool) \
            or not -1 <= c.template_row <= I32_MAX:
        return f"{where}: template_row {c.template_row!r} must be -1 or a non-negative i32"
    err = _check_text(f"{where} column", c.column, _COLUMN_RE, 31, 1)
    if err:
        return err
    return _check_text(f"{where} value", c.value, _PRINTABLE_RE, 63)


def _align8(n: int) -> int:
    return (n + 7) & ~7


def _blob_start(asset_count: int, cell_count: int) -> int:
    return _align8(FIXED_SIZE + ASSET_ENTRY_SIZE * asset_count + CELL_ENTRY_SIZE * cell_count)


# --------------------------------------------------------------------------- writer

def assets_from_stage(paths: Iterable[str | Path], stream_files: Iterable[str | Path] | None = None) -> list[Asset]:
    """Collect staged files through k1_port_vfs_pack.collect_assets (all its refusals apply)."""
    rows = _vp.collect_assets(list(paths), stream_files, allow_tga=False)
    return [Asset(r["resref"], r["res_type"], r["flags"], bytes(r["data"])) for r in rows]


def cells_from_vfs_pack(cells: Iterable[Any]) -> list[Cell]:
    """Adapt k1_port_vfs_pack.TwoDaCell objects (or anything parse_2da_cells accepts) to Cell."""
    return [Cell(c.table, c.row, c.template_row, c.column, c.value) for c in _vp.parse_2da_cells(list(cells))]


def _field(s: str, n: int) -> bytes:
    return s.encode("ascii").ljust(n, b"\0")


def build_pack_bytes(character: CharacterInfo, assets: Iterable[Asset], cells: Iterable[Cell]) -> bytes:
    """Validate every spec rule and return the deterministic pack bytes."""
    err = _check_character(character)
    if err:
        raise PackError(err)
    asset_list = sorted(assets, key=lambda a: (a.flags, a.res_type, a.resref))
    cell_list = sorted(cells, key=lambda c: (c.table, c.row, c.column))
    if not 1 <= len(asset_list) <= MAX_ASSETS:
        raise PackError(f"asset count {len(asset_list)} outside 1..{MAX_ASSETS}")
    if len(cell_list) > MAX_CELLS:
        raise PackError(f"cell count {len(cell_list)} outside 0..{MAX_CELLS}")

    seen_assets: set[tuple[int, int, str]] = set()
    for a in asset_list:
        err = _check_asset_meta(a)
        if err:
            raise PackError(err)
        if not isinstance(a.data, (bytes, bytearray)) or len(a.data) == 0:
            raise PackError(f"asset {a.resref}: data must be non-empty bytes")
        key = (a.flags, a.res_type, a.resref)
        if key in seen_assets:
            raise PackError(f"duplicate asset (flags {a.flags}, res_type {a.res_type}, resref {a.resref!r})")
        seen_assets.add(key)
    seen_cells: set[tuple[str, int, str]] = set()
    for c in cell_list:
        err = _check_cell(c)
        if err:
            raise PackError(err)
        key3 = (c.table, c.row, c.column)
        if key3 in seen_cells:
            raise PackError(f"duplicate cell {c.table}[{c.row}].{c.column}")
        seen_cells.add(key3)

    offset = _blob_start(len(asset_list), len(cell_list))
    entries = bytearray()
    blobs = bytearray()
    for a in asset_list:
        pad = _align8(offset) - offset
        blobs += b"\0" * pad
        offset += pad
        entries += _ASSET.pack(_field(a.resref, 16), a.res_type, a.flags, offset, len(a.data), 0)
        blobs += a.data
        offset += len(a.data)
    total = offset
    if total > U32_MAX or total > MAX_FILE_BYTES:
        raise PackError(f"pack would be {total} bytes, over the {MAX_FILE_BYTES}-byte cap")

    cell_bytes = b"".join(
        _CELL.pack(_field(c.table, 16), c.row, c.template_row, _field(c.column, 32), _field(c.value, 64))
        for c in cell_list
    )
    char_bytes = _CHARACTER.pack(
        _field(character.character_id, 32), _field(character.display_name, 64),
        _field(character.default_tag, 32), _field(character.default_utc, 32),
        _field(character.default_dlg, 32),
        character.appearance_id, character.head_id, character.soundset_id, 0,
    )
    gap = _blob_start(len(asset_list), len(cell_list)) - (FIXED_SIZE + len(entries) + len(cell_bytes))
    body = char_bytes + bytes(entries) + cell_bytes + b"\0" * gap + bytes(blobs)
    crc = zlib.crc32(body) & U32_MAX
    header = _HEADER.pack(MAGIC, VERSION, HEADER_SIZE, len(asset_list), len(cell_list), total, crc, 0)
    out = header + body
    assert len(out) == total
    return out


def _refuse_symlinked_path(path: Path) -> None:
    p = Path(os.path.abspath(path))
    if p.is_symlink():
        raise PackError(f"refusing to write through a symlink: {p}")
    for parent in p.parents:
        if parent.is_symlink():
            raise PackError(f"refusing to write inside a symlinked directory: {parent}")


def write_pack(path: str | Path, character: CharacterInfo, assets: Iterable[Asset],
               cells: Iterable[Cell]) -> dict[str, Any]:
    """Write a `.k2pack` atomically and return {path, size, crc32, sha256, asset_count, cell_count}."""
    data = build_pack_bytes(character, list(assets), list(cells))
    dest = Path(os.path.abspath(path))
    _refuse_symlinked_path(dest)
    dest.parent.mkdir(parents=True, exist_ok=True)
    _refuse_symlinked_path(dest)
    tmp = dest.with_name(dest.name + f".tmp{os.getpid()}")
    try:
        with open(tmp, "wb") as fh:
            fh.write(data)
            fh.flush()
            os.fsync(fh.fileno())
        os.replace(tmp, dest)
    finally:
        if tmp.exists():
            tmp.unlink()
    hdr = _HEADER.unpack_from(data, 0)
    return {
        "path": str(dest),
        "size": len(data),
        "crc32": hdr[6],
        "sha256": hashlib.sha256(data).hexdigest(),
        "asset_count": hdr[3],
        "cell_count": hdr[4],
    }


# --------------------------------------------------------------------------- reader

def _fail(msg: str) -> "PackFormatError":
    return PackFormatError(msg)


def _cstr(raw: bytes, label: str, *, needs_nul: bool) -> str:
    """Decode a NUL-padded ASCII field; reject a missing NUL (when required) and non-ASCII bytes."""
    i = raw.find(b"\0")
    if i < 0:
        if needs_nul:
            raise _fail(f"{label}: no NUL terminator in char[{len(raw)}] field")
        i = len(raw)
    body = raw[:i]
    if any(raw[i:]):
        raise _fail(f"{label}: non-NUL bytes after the first NUL")
    if any(b > 0x7F for b in body):
        raise _fail(f"{label}: non-ASCII byte")
    return body.decode("ascii")


def _reader_text(label: str, raw: bytes, pattern: re.Pattern[str], minlen: int, *, needs_nul: bool = True) -> str:
    s = _cstr(raw, label, needs_nul=needs_nul)
    if len(s) < minlen or not pattern.fullmatch(s):
        raise _fail(f"{label}: {s!r} violates the field charset/length")
    return s


def read_pack(data: bytes) -> tuple[CharacterInfo, list[Asset], list[Cell]]:
    """Parse and fully validate a pack. Raises PackFormatError naming the first failed rule."""
    data = bytes(data)
    n = len(data)
    if n > MAX_FILE_BYTES:
        raise _fail(f"file is {n} bytes, over the {MAX_FILE_BYTES}-byte cap")
    if n < HEADER_SIZE:
        raise _fail(f"file is {n} bytes, shorter than the 32-byte header")
    magic, version, header_size, acount, ccount, total, crc, reserved = _HEADER.unpack_from(data, 0)
    if magic != MAGIC:
        raise _fail(f"bad magic {magic!r}")
    if version != VERSION:
        raise _fail(f"unsupported version {version}")
    if header_size != HEADER_SIZE:
        raise _fail(f"header_size {header_size} != 32")
    if not 1 <= acount <= MAX_ASSETS:
        raise _fail(f"asset_count {acount} outside 1..{MAX_ASSETS}")
    if ccount > MAX_CELLS:
        raise _fail(f"cell_count {ccount} outside 0..{MAX_CELLS}")
    if total != n:
        raise _fail(f"total_size {total} != file length {n}")
    if reserved != 0:
        raise _fail("header reserved field is not 0")
    if zlib.crc32(data[HEADER_SIZE:total]) & U32_MAX != crc:
        raise _fail("crc32 mismatch")
    cell_table_end = FIXED_SIZE + ASSET_ENTRY_SIZE * acount + CELL_ENTRY_SIZE * ccount
    if cell_table_end > total:
        raise _fail("asset/cell tables extend past total_size")
    blob_start = _align8(cell_table_end)
    if blob_start > total:
        raise _fail("blob area starts past total_size")

    (cid, dname, dtag, dutc, ddlg, appearance, head, soundset, creserved) = _CHARACTER.unpack_from(data, HEADER_SIZE)
    if creserved != 0:
        raise _fail("character reserved field is not 0")
    resref_or_empty = re.compile(r"[a-z0-9_]{0,16}")
    character = CharacterInfo(
        character_id=_reader_text("character_id", cid, _ID_RE, 1),
        display_name=_reader_text("display_name", dname, _PRINTABLE_RE, 1),
        default_tag=_reader_text("default_tag", dtag, _PRINTABLE_RE, 0),
        default_utc=_reader_text("default_utc", dutc, resref_or_empty, 0),
        default_dlg=_reader_text("default_dlg", ddlg, resref_or_empty, 0),
        appearance_id=appearance, head_id=head, soundset_id=soundset,
    )

    assets: list[Asset] = []
    spans: list[tuple[int, int, str]] = []
    seen_assets: set[tuple[int, int, str]] = set()
    for i in range(acount):
        raw_ref, rtype, flags, off, size, res = _ASSET.unpack_from(data, FIXED_SIZE + ASSET_ENTRY_SIZE * i)
        label = f"asset[{i}]"
        resref = _reader_text(f"{label}.resref", raw_ref, _RESREF_RE, 1, needs_nul=False)
        if rtype in REFUSED_RES_TYPES:
            raise _fail(f"{label}: res_type {rtype} ({REFUSED_RES_TYPES[rtype]}) is refused")
        if rtype not in RES_TYPES:
            raise _fail(f"{label}: unknown res_type {rtype}")
        if flags not in (FLAG_RESMAN, FLAG_STREAM):
            raise _fail(f"{label}: flags {flags} not 0 or 1")
        if flags == FLAG_STREAM and rtype != RES_TYPE_WAV:
            raise _fail(f"{label}: STREAM flag on res_type {rtype}")
        if res != 0:
            raise _fail(f"{label}: reserved field is not 0")
        if size == 0:
            raise _fail(f"{label}: size is 0")
        if off % 8 != 0:
            raise _fail(f"{label}: data_offset {off} is not a multiple of 8")
        if off < blob_start:
            raise _fail(f"{label}: data_offset {off} is before the blob area ({blob_start})")
        if off + size > total:
            raise _fail(f"{label}: blob [{off}, {off + size}) extends past total_size {total}")
        key = (flags, rtype, resref)
        if key in seen_assets:
            raise _fail(f"{label}: duplicate (flags, res_type, resref) {key}")
        seen_assets.add(key)
        spans.append((off, off + size, label))
        assets.append(Asset(resref, rtype, flags, data[off:off + size]))
    spans.sort()
    for (_, end, la), (start, _, lb) in zip(spans, spans[1:]):
        if end > start:
            raise _fail(f"{la} and {lb} blobs overlap")

    cells: list[Cell] = []
    seen_cells: set[tuple[str, int, str]] = set()
    cell_base = FIXED_SIZE + ASSET_ENTRY_SIZE * acount
    for i in range(ccount):
        raw_t, row, trow, raw_c, raw_v = _CELL.unpack_from(data, cell_base + CELL_ENTRY_SIZE * i)
        label = f"cell[{i}]"
        table = _reader_text(f"{label}.table", raw_t, _RESREF_RE, 1, needs_nul=False)
        if trow < -1:
            raise _fail(f"{label}: template_row {trow} < -1")
        column = _reader_text(f"{label}.column", raw_c, _COLUMN_RE, 1)
        value = _reader_text(f"{label}.value", raw_v, _PRINTABLE_RE, 0)
        key3 = (table, row, column)
        if key3 in seen_cells:
            raise _fail(f"{label}: duplicate (table, row, column) {key3}")
        seen_cells.add(key3)
        cells.append(Cell(table, row, trow, column, value))
    return character, assets, cells


# --------------------------------------------------------------------------- fixture

def fixture_parts() -> tuple[CharacterInfo, list[Asset], list[Cell]]:
    character = CharacterInfo("fixture", "Fixture", "fixture_tag", "fixture_utc", "", 900, 300, 11)
    assets = [
        Asset("fixture_vo", RES_TYPE_WAV, FLAG_STREAM, b"K2PKWAV"),
        Asset("fixture_utc", 2027, FLAG_RESMAN, b"K2PKUTC:fake-utc-blob"),
        Asset("fixture_sixteen_", 2022, FLAG_RESMAN, b"K2PKTXI\n"),
        Asset("fixture_mdl", 2002, FLAG_RESMAN, b"K2PKTEST" * 3),
    ]
    cells = [
        Cell("heads", 300, -1, "heads", "fixture_hd"),
        Cell("appearance", 900, 17, "label", "Fixture"),
    ]
    return character, assets, cells


# --------------------------------------------------------------------------- CLI

def _describe(data: bytes) -> dict[str, Any]:
    character, assets, cells = read_pack(data)
    magic, version, hsize, acount, ccount, total, crc, _ = _HEADER.unpack_from(data, 0)
    offsets = [_ASSET.unpack_from(data, FIXED_SIZE + ASSET_ENTRY_SIZE * i)[3] for i in range(acount)]
    return {
        "header": {"magic": magic.decode("ascii"), "version": version, "header_size": hsize,
                   "asset_count": acount, "cell_count": ccount, "total_size": total, "crc32": f"{crc:08x}"},
        "character": asdict(character),
        "assets": [
            {"resref": a.resref, "type": RES_TYPES[a.res_type], "res_type": a.res_type,
             "flags": a.flags, "offset": off, "size": len(a.data),
             "sha256": hashlib.sha256(a.data).hexdigest()}
            for a, off in zip(assets, offsets)
        ],
        "cells": [asdict(c) for c in cells],
    }


def _cmd_inspect(args: argparse.Namespace) -> int:
    try:
        data = Path(args.file).read_bytes()
        info = _describe(data)
    except OSError as exc:
        print(f"error: cannot read {args.file}: {exc}", file=sys.stderr)
        return 2
    except PackFormatError as exc:
        print(f"error: bad pack {args.file}: {exc}", file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps(info, indent=2, sort_keys=True))
        return 0
    h = info["header"]
    print(f"{args.file}: K2PK v{h['version']}  {h['total_size']} bytes  crc32 {h['crc32']}  "
          f"{h['asset_count']} assets  {h['cell_count']} cells")
    print("character:")
    for k, v in info["character"].items():
        print(f"  {k:14} {v!r}")
    print("assets:")
    print(f"  {'#':>3} {'resref':16} {'type':5} {'flags':6} {'offset':>8} {'size':>8}  sha256")
    for i, a in enumerate(info["assets"]):
        print(f"  {i:>3} {a['resref']:16} {a['type']:5} {'STREAM' if a['flags'] else 'RESMAN':6} "
              f"{a['offset']:>8} {a['size']:>8}  {a['sha256'][:16]}")
    print("cells:")
    for c in info["cells"]:
        print(f"  {c['table']}[{c['row']}] template={c['template_row']} {c['column']} = {c['value']!r}")
    return 0


def _cmd_fixture(args: argparse.Namespace) -> int:
    try:
        summary = write_pack(args.out, *fixture_parts())
    except PackError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    print(json.dumps(summary, sort_keys=True))
    return 0


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    pi = sub.add_parser("inspect", help="print a .k2pack; exit 2 when it is bad")
    pi.add_argument("file")
    pi.add_argument("--json", action="store_true")
    pi.set_defaults(func=_cmd_inspect)
    pf = sub.add_parser("fixture", help="write the deterministic synthetic fixture pack")
    pf.add_argument("out")
    pf.set_defaults(func=_cmd_fixture)
    args = ap.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
