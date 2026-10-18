#!/usr/bin/env python3
"""Unit tests for k1_port_pack.py (.k2pack writer and reader, format version 1).

Verifies:
1. Round trip write -> read, deterministic bytes, atomic write, symlink refusal.
2. Byte-level layout against the K2PACK_FORMAT.md tables (header, character block, asset entry, cell entry, crc).
3. Every writer refusal (field lengths with NUL room, charsets, caps, duplicates, stream flag, refused types).
4. Reader fuzz: truncation at many lengths, crc, magic, version, offsets, overlap, counts, missing NUL.
5. Fixture command output is stable (sha256 pinned) and the inspect CLI.

All files live in tempfile directories; nothing is written into the repo.
"""
from __future__ import annotations

import contextlib
import hashlib
import io
import json
import os
import struct
import sys
import tempfile
import unittest
import zlib
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent / "kvoice"))

import k1_port_pack as kp  # noqa: E402
from k1_port_pack import Asset, Cell, CharacterInfo, PackError, PackFormatError  # noqa: E402
from k1_port_vfs_pack import TwoDaCell  # noqa: E402

FIXTURE_SHA256 = "e8bec0671f3303d45970d89ca3266e9b70d3591ff1010aaae71ca36932fa1e7e"


def make_char(**kw):
    base = dict(character_id="juhani", display_name="Juhani", default_tag="tag", default_utc="juhani_utc",
                default_dlg="juhani_dlg", appearance_id=901, head_id=302, soundset_id=12)
    base.update(kw)
    return CharacterInfo(**base)


def make_assets():
    return [
        Asset("b_mdl", 2002, 0, b"MDLBYTES!"),
        Asset("a_mdx", 3008, 0, b"X" * 16),
        Asset("a_vo", 4, 1, b"WAV"),
    ]


def make_cells():
    return [Cell("appearance", 901, 17, "label", "Juhani"), Cell("heads", 302, -1, "head", "x")]


def build(character=None, assets=None, cells=None) -> bytes:
    return kp.build_pack_bytes(character or make_char(),
                               make_assets() if assets is None else assets,
                               make_cells() if cells is None else cells)


def refix(data: bytearray) -> bytes:
    """Recompute total_size-preserving crc after a body mutation."""
    struct.pack_into("<I", data, 24, zlib.crc32(bytes(data[32:])) & 0xFFFFFFFF)
    return bytes(data)


class RoundTripTests(unittest.TestCase):
    def test_round_trip_and_sorting(self):
        raw = build()
        ch, assets, cells = kp.read_pack(raw)
        self.assertEqual(ch, make_char())
        # sorted by (flags, res_type, resref)
        self.assertEqual([(a.flags, a.res_type, a.resref) for a in assets],
                         [(0, 2002, "b_mdl"), (0, 3008, "a_mdx"), (1, 4, "a_vo")])
        self.assertEqual([a.data for a in assets], [b"MDLBYTES!", b"X" * 16, b"WAV"])
        self.assertEqual([(c.table, c.row, c.template_row, c.column, c.value) for c in cells],
                         [("appearance", 901, 17, "label", "Juhani"), ("heads", 302, -1, "head", "x")])

    def test_deterministic_regardless_of_input_order(self):
        a = build()
        b = build(assets=list(reversed(make_assets())), cells=list(reversed(make_cells())))
        self.assertEqual(a, b)

    def test_write_pack_summary_and_atomic(self):
        with tempfile.TemporaryDirectory() as td:
            out = Path(td) / "sub" / "x.k2pack"
            s = kp.write_pack(out, make_char(), make_assets(), make_cells())
            data = out.read_bytes()
            self.assertEqual(s["size"], len(data))
            self.assertEqual(s["sha256"], hashlib.sha256(data).hexdigest())
            self.assertEqual(s["crc32"], zlib.crc32(data[32:]) & 0xFFFFFFFF)
            self.assertEqual((s["asset_count"], s["cell_count"]), (3, 2))
            self.assertEqual(data, build())
            self.assertEqual([p.name for p in out.parent.iterdir()], ["x.k2pack"])  # no tmp left
            # overwrite in place
            kp.write_pack(out, make_char(), make_assets(), [])
            self.assertEqual(kp.read_pack(out.read_bytes())[2], [])

    def test_no_cells_ok(self):
        ch, assets, cells = kp.read_pack(build(cells=[]))
        self.assertEqual(cells, [])
        self.assertEqual(len(assets), 3)

    def test_symlinked_parent_refused(self):
        with tempfile.TemporaryDirectory() as td:
            real = Path(td) / "real"
            real.mkdir()
            link = Path(td) / "link"
            link.symlink_to(real, target_is_directory=True)
            with self.assertRaises(PackError):
                kp.write_pack(link / "x.k2pack", make_char(), make_assets(), [])
            with self.assertRaises(PackError):
                kp.write_pack(link / "deeper" / "x.k2pack", make_char(), make_assets(), [])
            self.assertEqual(list(real.iterdir()), [])

    def test_symlinked_target_refused(self):
        with tempfile.TemporaryDirectory() as td:
            victim = Path(td) / "victim.bin"
            victim.write_bytes(b"keep")
            link = Path(td) / "x.k2pack"
            link.symlink_to(victim)
            with self.assertRaises(PackError):
                kp.write_pack(link, make_char(), make_assets(), [])
            self.assertEqual(victim.read_bytes(), b"keep")

    def test_16_char_resref_accepted(self):
        ref = "a" * 16
        raw = build(assets=[Asset(ref, 2002, 0, b"abc")], cells=[Cell("b" * 16, 0, -1, "c", "")])
        _, assets, cells = kp.read_pack(raw)
        self.assertEqual((assets[0].resref, cells[0].table), (ref, "b" * 16))
        # no NUL in the 16 resref bytes
        self.assertEqual(raw[240:256], ref.encode())


class LayoutTests(unittest.TestCase):
    def test_offsets_match_spec(self):
        raw = build()
        # header
        magic, ver, hsize, na, nc, total, crc, res = struct.unpack_from("<4sIIIIIII", raw, 0)
        self.assertEqual((magic, ver, hsize, na, nc, total, res), (b"K2PK", 1, 32, 3, 2, len(raw), 0))
        self.assertEqual(crc, zlib.crc32(raw[32:total]) & 0xFFFFFFFF)
        # character block at 32
        self.assertEqual(raw[32:64].rstrip(b"\0"), b"juhani")
        self.assertEqual(raw[64:128].rstrip(b"\0"), b"Juhani")
        self.assertEqual(raw[128:160].rstrip(b"\0"), b"tag")
        self.assertEqual(raw[160:192].rstrip(b"\0"), b"juhani_utc")
        self.assertEqual(raw[192:224].rstrip(b"\0"), b"juhani_dlg")
        self.assertEqual(struct.unpack_from("<IIII", raw, 32 + 192), (901, 302, 12, 0))
        # asset table at 240, first entry = (0, 2002, b_mdl)
        resref, rtype, flags, off, size, res = struct.unpack_from("<16sHHIII", raw, 240)
        self.assertEqual((resref.rstrip(b"\0"), rtype, flags, size, res), (b"b_mdl", 2002, 0, 9, 0))
        blob_start = 240 + 32 * 3 + 120 * 2
        self.assertEqual(blob_start % 8, 0)
        self.assertEqual(off, blob_start)
        self.assertEqual(raw[off:off + size], b"MDLBYTES!")
        # second blob aligned, zero padding between
        off2 = struct.unpack_from("<I", raw, 240 + 32 + 20)[0]
        self.assertEqual(off2, blob_start + 16)
        self.assertEqual(raw[off + size:off2], b"\0" * (off2 - off - size))
        # cell table at 240 + 32*3
        cbase = 240 + 32 * 3
        table, row, trow, col, val = struct.unpack_from("<16sIi32s64s", raw, cbase)
        self.assertEqual((table.rstrip(b"\0"), row, trow, col.rstrip(b"\0"), val.rstrip(b"\0")),
                         (b"appearance", 901, 17, b"label", b"Juhani"))
        self.assertEqual(struct.calcsize("<16sIi32s64s"), 120)
        # last blob ends the file (no trailing padding)
        loff, lsize = struct.unpack_from("<II", raw, 240 + 64 + 20)
        self.assertEqual(loff + lsize, len(raw))

    def test_cell_table_always_aligned_blob_start(self):
        for na in (1, 2, 5):
            assets = [Asset(f"r{i}", 2002, 0, b"z") for i in range(na)]
            raw = build(assets=assets, cells=[])
            off0 = struct.unpack_from("<I", raw, 240 + 20)[0]
            self.assertEqual(off0, 240 + 32 * na)
            self.assertEqual(off0 % 8, 0)


class WriterRefusalTests(unittest.TestCase):
    def refused(self, **kw):
        with self.assertRaises(PackError):
            build(**kw)

    def test_character_fields(self):
        self.refused(character=make_char(character_id=""))
        self.refused(character=make_char(character_id="a" * 32))  # no NUL room
        self.refused(character=make_char(character_id="Upper"))
        self.refused(character=make_char(character_id="has space"))
        self.refused(character=make_char(display_name=""))
        self.refused(character=make_char(display_name="d" * 64))
        self.refused(character=make_char(display_name="tab\there"))
        self.refused(character=make_char(display_name="café"))
        self.refused(character=make_char(default_tag="t" * 32))
        self.refused(character=make_char(default_utc="u" * 17))
        self.refused(character=make_char(default_utc="Bad"))
        self.refused(character=make_char(default_dlg="d" * 17))
        self.refused(character=make_char(appearance_id=-1))
        self.refused(character=make_char(head_id=2**32))
        self.refused(character=make_char(soundset_id="3"))
        # boundaries accepted
        build(character=make_char(character_id="a" * 31, display_name="d" * 63, default_tag="t" * 31,
                                  default_utc="u" * 16, default_dlg="", appearance_id=2**32 - 1))

    def test_asset_counts(self):
        self.refused(assets=[])
        self.refused(assets=[Asset(f"r{i}", 2002, 0, b"z") for i in range(193)])
        build(assets=[Asset(f"r{i}", 2002, 0, b"z") for i in range(192)], cells=[])

    def test_cell_counts(self):
        cells = [Cell("t", i, -1, "c", "") for i in range(257)]
        self.refused(cells=cells)
        build(cells=cells[:256])

    def test_asset_rules(self):
        self.refused(assets=[Asset("a" * 17, 2002, 0, b"z")])
        self.refused(assets=[Asset("", 2002, 0, b"z")])
        self.refused(assets=[Asset("UP", 2002, 0, b"z")])
        self.refused(assets=[Asset("a.b", 2002, 0, b"z")])
        self.refused(assets=[Asset("a", 2002, 0, b"")])
        self.refused(assets=[Asset("a", 3, 0, b"z")])        # tga
        self.refused(assets=[Asset("a", 2017, 0, b"z")])     # 2da
        self.refused(assets=[Asset("a", 9999, 0, b"z")])
        self.refused(assets=[Asset("a", 2002, 1, b"z")])     # stream on non-wav
        self.refused(assets=[Asset("a", 4, 2, b"z")])
        self.refused(assets=[Asset("a", 2002, 0, b"z"), Asset("a", 2002, 0, b"y")])
        build(assets=[Asset("a", 4, 0, b"z"), Asset("a", 4, 1, b"y"), Asset("a", 2002, 0, b"y")])

    def test_cell_rules(self):
        self.refused(cells=[Cell("", 0, -1, "c", "")])
        self.refused(cells=[Cell("t" * 17, 0, -1, "c", "")])
        self.refused(cells=[Cell("T", 0, -1, "c", "")])
        self.refused(cells=[Cell("t", -1, -1, "c", "")])
        self.refused(cells=[Cell("t", 0, -2, "c", "")])
        self.refused(cells=[Cell("t", 0, -1, "", "")])
        self.refused(cells=[Cell("t", 0, -1, "c" * 32, "")])
        self.refused(cells=[Cell("t", 0, -1, "has space", "")])
        self.refused(cells=[Cell("t", 0, -1, "c", "v" * 64)])
        self.refused(cells=[Cell("t", 0, -1, "c", "new\nline")])
        self.refused(cells=[Cell("t", 0, -1, "c", "a"), Cell("t", 0, 5, "c", "b")])
        build(cells=[Cell("t", 0, -1, "c" * 31, "v" * 63)])

    def test_oversize_total_refused(self):
        class Big(bytes):
            def __len__(self):
                return 300 * 1024 * 1024
        with self.assertRaises(PackError):
            build(assets=[Asset("a", 2002, 0, Big(b"z"))])


class AdapterTests(unittest.TestCase):
    def test_assets_from_stage_reuses_vfs_pack_refusals(self):
        with tempfile.TemporaryDirectory() as td:
            stage = Path(td) / "stage"
            stage.mkdir()
            (stage / "K1P_Test.MDL").write_bytes(b"mdl")
            (stage / "k1p_test.txi").write_bytes(b"txi")
            assets = kp.assets_from_stage([stage])
            self.assertEqual([(a.resref, a.res_type, a.flags, a.data) for a in assets],
                             [("k1p_test", 2002, 0, b"mdl"), ("k1p_test", 2022, 0, b"txi")])
            (stage / "bad.tga").write_bytes(b"tga")
            with self.assertRaises(PackError):
                kp.assets_from_stage([stage])
            (stage / "bad.tga").unlink()
            (stage / "t.2da").write_bytes(b"2da")
            with self.assertRaises(PackError):
                kp.assets_from_stage([stage])

    def test_stream_files(self):
        with tempfile.TemporaryDirectory() as td:
            sd = Path(td) / "streamvoice"
            sd.mkdir()
            (sd / "vo1.mp3").write_bytes(b"id3")
            assets = kp.assets_from_stage([sd])
            self.assertEqual((assets[0].res_type, assets[0].flags), (4, 1))

    def test_cells_from_vfs_pack(self):
        cells = kp.cells_from_vfs_pack([TwoDaCell("heads.2da", 3, "head", "v", 2), TwoDaCell("a", 1, "c", "d")])
        self.assertEqual(cells, [Cell("a", 1, -1, "c", "d"), Cell("heads", 3, 2, "head", "v")])


class ReaderFuzzTests(unittest.TestCase):
    def setUp(self):
        self.raw = build()

    def fails(self, data, needle=None):
        with self.assertRaises(PackFormatError) as cm:
            kp.read_pack(data)
        if needle:
            self.assertIn(needle, str(cm.exception))

    def test_every_truncation_rejected(self):
        for n in sorted(set(list(range(0, 300)) + list(range(300, len(self.raw), 7)) + [len(self.raw) - 1])):
            with self.assertRaises(PackFormatError, msg=f"len {n}"):
                kp.read_pack(self.raw[:n])

    def test_trailing_bytes_rejected(self):
        self.fails(self.raw + b"\0", "total_size")

    def test_bad_magic_version_header_size(self):
        d = bytearray(self.raw)
        d[0:4] = b"XXXX"
        self.fails(bytes(d), "magic")
        for off, val, needle in ((4, 2, "version"), (4, 0, "version"), (8, 36, "header_size")):
            d = bytearray(self.raw)
            struct.pack_into("<I", d, off, val)
            self.fails(bytes(d), needle)

    def test_crc_flip(self):
        d = bytearray(self.raw)
        d[24] ^= 1
        self.fails(bytes(d), "crc")
        d = bytearray(self.raw)
        d[-1] ^= 0xFF  # body flip with stale crc
        self.fails(bytes(d), "crc")

    def test_reserved_nonzero(self):
        d = bytearray(self.raw)
        struct.pack_into("<I", d, 28, 1)
        self.fails(bytes(d), "reserved")
        d = bytearray(self.raw)
        struct.pack_into("<I", d, 32 + 204, 1)
        self.fails(refix(d), "reserved")
        d = bytearray(self.raw)
        struct.pack_into("<I", d, 240 + 28, 1)
        self.fails(refix(d), "reserved")

    def test_counts(self):
        for off, val in ((12, 0), (12, 193), (12, 0xFFFFFFFF), (16, 257), (16, 0xFFFFFFFF)):
            d = bytearray(self.raw)
            struct.pack_into("<I", d, off, val)
            self.fails(bytes(d))
        # in-range count that makes the tables run past the end
        d = bytearray(self.raw[:300])
        struct.pack_into("<II", d, 12, 192, 256)
        struct.pack_into("<I", d, 20, len(d))
        self.fails(refix(d), "tables")

    def test_total_size_mismatch(self):
        d = bytearray(self.raw)
        struct.pack_into("<I", d, 20, len(d) + 8)
        self.fails(bytes(d), "total_size")

    def asset_field(self, idx, off, fmt, val):
        d = bytearray(self.raw)
        struct.pack_into(fmt, d, 240 + 32 * idx + off, val)
        return refix(d)

    def test_offset_past_end(self):
        self.fails(self.asset_field(0, 20, "<I", (len(self.raw) + 15) // 8 * 8), "extends past")
        self.fails(self.asset_field(0, 24, "<I", 0xFFFFFFF0), "extends past")

    def test_unaligned_offset(self):
        off = struct.unpack_from("<I", self.raw, 260)[0]
        self.fails(self.asset_field(0, 20, "<I", off + 1), "multiple of 8")

    def test_offset_before_blob_area(self):
        self.fails(self.asset_field(0, 20, "<I", 8), "before the blob area")

    def test_zero_size(self):
        self.fails(self.asset_field(0, 24, "<I", 0), "size is 0")

    def test_overlap(self):
        off0 = struct.unpack_from("<I", self.raw, 240 + 20)[0]
        self.fails(self.asset_field(1, 20, "<I", off0), "overlap")
        # partial overlap, still aligned
        self.fails(self.asset_field(0, 24, "<I", 24), "overlap")

    def test_flags_and_types(self):
        self.fails(self.asset_field(0, 18, "<H", 2), "flags")
        self.fails(self.asset_field(0, 18, "<H", 1), "STREAM")  # mdl with stream flag
        self.fails(self.asset_field(0, 16, "<H", 3), "refused")
        self.fails(self.asset_field(0, 16, "<H", 2017), "refused")
        self.fails(self.asset_field(0, 16, "<H", 1234), "unknown res_type")

    def test_duplicate_asset(self):
        d = bytearray(self.raw)
        d[240 + 32:240 + 32 + 16] = d[240:256]
        struct.pack_into("<H", d, 240 + 32 + 16, 2002)
        self.fails(refix(d), "duplicate")

    def test_bad_resref_chars(self):
        d = bytearray(self.raw)
        d[240] = ord("A")
        self.fails(refix(d), "resref")
        d = bytearray(self.raw)
        d[240:256] = b"\0" * 16
        self.fails(refix(d), "resref")

    def test_missing_nul_in_char32(self):
        d = bytearray(self.raw)
        d[32:64] = b"a" * 32
        self.fails(refix(d), "NUL")
        d = bytearray(self.raw)
        d[64:128] = b"d" * 64
        self.fails(refix(d), "NUL")
        d = bytearray(self.raw)
        d[160:192] = b"u" * 32
        self.fails(refix(d), "NUL")
        # cell column / value fields
        cbase = 240 + 32 * 3
        d = bytearray(self.raw)
        d[cbase + 24:cbase + 56] = b"c" * 32
        self.fails(refix(d), "NUL")
        d = bytearray(self.raw)
        d[cbase + 56:cbase + 120] = b"v" * 64
        self.fails(refix(d), "NUL")

    def test_31_char_field_accepted(self):
        raw = build(character=make_char(character_id="a" * 31))
        self.assertEqual(kp.read_pack(raw)[0].character_id, "a" * 31)

    def test_charset_violations(self):
        d = bytearray(self.raw)
        d[32] = ord("-")
        self.fails(refix(d), "character_id")
        d = bytearray(self.raw)
        d[64] = 0x07
        self.fails(refix(d), "display_name")
        d = bytearray(self.raw)
        d[64] = 0xC3
        self.fails(refix(d), "non-ASCII")
        d = bytearray(self.raw)
        d[64] = 0  # empty display name
        self.fails(refix(d), "display_name")
        cbase = 240 + 32 * 3
        d = bytearray(self.raw)
        d[cbase + 24] = ord(" ")
        self.fails(refix(d), "column")

    def test_cell_rules(self):
        cbase = 240 + 32 * 3
        d = bytearray(self.raw)
        struct.pack_into("<i", d, cbase + 20, -2)
        self.fails(refix(d), "template_row")
        d = bytearray(self.raw)
        d[cbase + 120:cbase + 240] = d[cbase:cbase + 120]
        self.fails(refix(d), "duplicate")

    def test_oversize_file_refused_before_parse(self):
        class Huge(bytes):
            def __len__(self):
                return 257 * 1024 * 1024
        with self.assertRaises(PackFormatError):
            kp.read_pack(Huge(b"K2PK"))


class FixtureAndCliTests(unittest.TestCase):
    def run_cli(self, *argv):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            rc = kp.main(list(argv))
        return rc, out.getvalue(), err.getvalue()

    def test_fixture_contents_and_stable_hash(self):
        with tempfile.TemporaryDirectory() as td:
            p1, p2 = Path(td) / "a.k2pack", Path(td) / "b.k2pack"
            rc, out, _ = self.run_cli("fixture", str(p1))
            self.assertEqual(rc, 0)
            self.run_cli("fixture", str(p2))
            data = p1.read_bytes()
            self.assertEqual(data, p2.read_bytes())
            self.assertEqual(json.loads(out)["sha256"], hashlib.sha256(data).hexdigest())
            self.assertEqual(len(data), 671)
            self.assertEqual(hashlib.sha256(data).hexdigest(), FIXTURE_SHA256)
            ch, assets, cells = kp.read_pack(data)
            self.assertEqual(ch, CharacterInfo("fixture", "Fixture", "fixture_tag", "fixture_utc", "", 900, 300, 11))
            self.assertEqual([(a.flags, a.res_type, a.resref, a.data) for a in assets], [
                (0, 2002, "fixture_mdl", b"K2PKTEST" * 3),
                (0, 2022, "fixture_sixteen_", b"K2PKTXI\n"),
                (0, 2027, "fixture_utc", b"K2PKUTC:fake-utc-blob"),
                (1, 4, "fixture_vo", b"K2PKWAV"),
            ])
            self.assertEqual(cells, [Cell("appearance", 900, 17, "label", "Fixture"),
                                     Cell("heads", 300, -1, "heads", "fixture_hd")])
            offs = [struct.unpack_from("<I", data, 240 + 32 * i + 20)[0] for i in range(4)]
            self.assertEqual(offs, [608, 632, 640, 664])

    def test_inspect_text_and_json(self):
        with tempfile.TemporaryDirectory() as td:
            p = Path(td) / "f.k2pack"
            self.run_cli("fixture", str(p))
            rc, out, _ = self.run_cli("inspect", str(p))
            self.assertEqual(rc, 0)
            self.assertIn("fixture_sixteen_", out)
            self.assertIn("STREAM", out)
            rc, out, _ = self.run_cli("inspect", str(p), "--json")
            self.assertEqual(rc, 0)
            info = json.loads(out)
            self.assertEqual(info["header"]["asset_count"], 4)
            self.assertEqual(info["character"]["appearance_id"], 900)
            self.assertEqual(len(info["cells"]), 2)

    def test_inspect_bad_pack_exit_2(self):
        with tempfile.TemporaryDirectory() as td:
            p = Path(td) / "bad.k2pack"
            p.write_bytes(b"K2PK" + b"\0" * 40)
            rc, _, err = self.run_cli("inspect", str(p))
            self.assertEqual(rc, 2)
            self.assertIn("bad pack", err)
            rc, _, _ = self.run_cli("inspect", str(Path(td) / "missing"))
            self.assertEqual(rc, 2)


if __name__ == "__main__":
    unittest.main()
