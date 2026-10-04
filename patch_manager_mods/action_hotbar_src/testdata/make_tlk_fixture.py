#!/usr/bin/env python3
"""Generate testdata/fixture.tlk (TLK V3.0) for tlk_test.cpp. Deterministic; the output is committed.

Run:  python3 patch_manager_mods/action_hotbar_src/testdata/make_tlk_fixture.py

Layout (same as Tlk.h): "TLK V3.0", u32 language, u32 count, u32 stringsOffset, then count * 40-byte entries
(u32 flags, char[16] sound, u32 volVar, u32 pitchVar, u32 offset, u32 size, f32 soundLen), then the string blob.
No pykotor needed. Reads the file back and verifies every entry; exit 1 on mismatch.
"""
import struct
import sys
from pathlib import Path

# (raw bytes as stored, text Tlk::Get must return). Keep in sync with tlk_test.cpp.
ENTRIES = [
    (b"Hello, world.", "Hello, world."),
    (b"\x91quoted\x92 \x93double\x94 a\x96b\x97c", "'quoted' \"double\" a-b-c"),   # CP1252 quotes/dashes
    (b"line1\r\nline2\tcol\n\nend", "line1 line2 col end"),                          # CR/LF/tab collapse
    (b"", ""),                                                                      # empty entry
    (b"  padded text   ", "padded text"),                                           # leading/trailing spaces
    (b"Force Speed\0garbage after NUL", "Force Speed"),                             # embedded NUL
]


def build():
    count = len(ENTRIES)
    str_off = 20 + count * 40
    head = b"TLK V3.0" + struct.pack("<III", 0, count, str_off)
    table, blob = b"", b""
    for raw, _ in ENTRIES:
        flags = 0x0001 if raw else 0
        table += struct.pack("<I16sIIIIf", flags, b"", 0, 0, len(blob), len(raw), 0.0)
        blob += raw
    return head + table + blob


def main():
    out = Path(__file__).resolve().parent / "fixture.tlk"
    data = build()
    out.write_bytes(data)
    d = out.read_bytes()
    magic, = struct.unpack_from("<8s", d, 0)
    _, count, str_off = struct.unpack_from("<III", d, 8)
    bad = 0
    if magic != b"TLK V3.0" or count != len(ENTRIES) or str_off != 20 + count * 40:
        print("error: bad header", file=sys.stderr)
        bad += 1
    for i, (raw, _) in enumerate(ENTRIES):
        off, size = struct.unpack_from("<II", d, 20 + i * 40 + 28)
        if d[str_off + off:str_off + off + size] != raw:
            print(f"error: entry {i} mismatch", file=sys.stderr)
            bad += 1
    if bad:
        return 1
    print(f"wrote {out} ({len(d)} bytes, {count} entries, verified)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
