#!/usr/bin/env python3
"""Generate testdata/keybif_fixture/ for keybif_test.cpp. Deterministic; the output is committed.

Run:  python3 patch_manager_mods/action_hotbar_src/testdata/make_keybif_fixture.py

Good set: chitin.key + data/2da.bif holding `spells` (2017) = spells_fixture.2da bytes, a decoy `spells` of another type
(2002), and a decoy `decoy` (2017). The BIF var table is deliberately out of order (IDs 2,0,1). case.key names the BIF
"Data\\2DA.BIF" to exercise the lowercase fallback. Broken set (bad/): bad_magic.key, trunc.key, bad_bif_index.key,
bad_res.key (+ data/bad.bif, data/badmagic.bif) for oversize / out-of-range / missing / bad-magic BIF cases.
zero_name.key, big_name.key, name_eof.key, short_key.key (byte patches of one good key) and bif_bad.key (+ data/short.bif,
data/noid.bif) cover the name-size, name-offset, key-length, short-BIF and missing-resource-id guards.
Layout: see KeyBif.h. Reads everything back and verifies; exit 1 on mismatch.
"""
import struct
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
OUT = HERE / "keybif_fixture"
SPELLS = (HERE / "spells_fixture.2da").read_bytes()


def make_bif(entries, magic=b"BIFFV1  "):
    """entries: [(id, restype, data_bytes)] in table order -> BIF bytes. data follows the var table in list order."""
    off_vars = 20
    data_start = off_vars + 16 * len(entries)
    table, blob = b"", b""
    for rid, rtype, data in entries:
        table += struct.pack("<IIII", rid, data_start + len(blob), len(data), rtype)
        blob += data
    return magic + struct.pack("<III", len(entries), 0, off_vars) + table + blob


def make_key(bif_names, keys, magic=b"KEY V1  "):
    """bif_names: [str]; keys: [(resref, restype, resid)] -> KEY bytes (file table, then names, then key table)."""
    off_files = 24
    off_names = off_files + 12 * len(bif_names)
    names = [n.encode() + b"\0" for n in bif_names]
    off_keys = off_names + sum(len(n) for n in names)
    ft, noff = b"", off_names
    for n in names:
        ft += struct.pack("<IIHH", 0, noff, len(n), 1)
        noff += len(n)
    kt = b"".join(struct.pack("<16sHI", r.encode(), t, i) for r, t, i in keys)
    return magic + struct.pack("<IIII", len(bif_names), len(keys), off_files, off_keys) + ft + b"".join(names) + kt


def build():
    files = {}
    good_bif = make_bif([(2, 2017, b"decoy-2da-bytes"), (0, 2017, SPELLS), (1, 2002, b"wrong-type-decoy")])
    files["data/2da.bif"] = good_bif
    good_keys = [("decoy", 2017, 2), ("spells", 2002, 1), ("spells", 2017, 0)]
    files["chitin.key"] = make_key(["data\\2da.bif"], good_keys)
    files["case.key"] = make_key(["Data\\2DA.BIF"], good_keys)

    bad = "bad/"
    files[bad + "bad_magic.key"] = make_key(["data\\2da.bif"], good_keys, magic=b"KEY V9  ")
    full = make_key(["data\\2da.bif"], good_keys)
    files[bad + "trunc.key"] = full[:24 + 12 + 5]   # header + file table, name/key table cut off
    files[bad + "bad_bif_index.key"] = make_key(["data\\2da.bif"], [("spells", 2017, (3 << 20) | 0)])
    # bad.bif: res 0 declares 9 MiB, res 1 points past EOF. badmagic.bif: wrong magic. bif 3 does not exist on disk.
    bb = bytearray(make_bif([(0, 2017, b"x"), (1, 2017, b"y")]))
    struct.pack_into("<I", bb, 20 + 8, 9 * 1024 * 1024)      # entry 0 FileSize
    struct.pack_into("<I", bb, 20 + 16 + 4, 0x7000)          # entry 1 Offset beyond EOF
    files[bad + "data/bad.bif"] = bytes(bb)
    files[bad + "data/badmagic.bif"] = make_bif([(0, 2017, b"z")], magic=b"BIFFV9  ")
    files[bad + "bad_res.key"] = make_key(
        ["data\\bad.bif", "data\\badmagic.bif", "data\\nope.bif"],
        [("big", 2017, 0), ("past", 2017, 1), ("badmagic", 2017, (1 << 20) | 0), ("ghost", 2017, (2 << 20) | 0)])
    # Byte patches of the good single-BIF key `full`: file table entry 0 starts at 24 (NameOffset +4, NameSize +8).
    zero_name = bytearray(full)
    struct.pack_into("<H", zero_name, 24 + 8, 0)           # NameSize 0 -> "bad bif name size"
    files[bad + "zero_name.key"] = bytes(zero_name)
    big_name = bytearray(full)
    struct.pack_into("<H", big_name, 24 + 8, 261)          # NameSize 261 (> 260 cap) -> "bad bif name size"
    files[bad + "big_name.key"] = bytes(big_name)
    name_eof = bytearray(full)
    struct.pack_into("<I", name_eof, 24 + 4, len(full))    # NameOffset == EOF, NameSize > 0 -> "bif name out of range"
    files[bad + "name_eof.key"] = bytes(name_eof)
    files[bad + "short_key.key"] = full[:23]               # 23 of the 24 header bytes -> "key too short"
    # bif 0 is 19 of its 20 header bytes; bif 1 holds only id 5, so resource index 0 is absent -> "resource id not in bif".
    files[bad + "bif_bad.key"] = make_key(["data\\short.bif", "data\\noid.bif"],
                                          [("short", 2017, 0), ("absent", 2017, (1 << 20) | 0)])
    files[bad + "data/short.bif"] = make_bif([(0, 2017, b"x")])[:19]
    files[bad + "data/noid.bif"] = make_bif([(5, 2017, b"y")])
    return files


def verify(files):
    k = files["chitin.key"]
    assert k[:8] == b"KEY V1  " and struct.unpack_from("<II", k, 8) == (1, 3)
    b = files["data/2da.bif"]
    assert b[:8] == b"BIFFV1  " and SPELLS in b


def main():
    files = build()
    verify(files)
    for rel, data in files.items():
        p = OUT / rel
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_bytes(data)
    print(f"wrote {len(files)} files, {sum(len(d) for d in files.values())} bytes under {OUT}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
