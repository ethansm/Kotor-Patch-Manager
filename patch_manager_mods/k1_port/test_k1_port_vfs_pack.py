#!/usr/bin/env python3
"""Unit tests for k1_port_vfs_pack.py.

Verifies:
1. Packing a complete synthetic Juhani bundle: per-asset .bin blobs, header tables, kCharacterDef, registerAll.
2. The generated header compiles and links with g++ -std=c++17 -Wall -Wextra -Werror against the REAL
   _shared_src/CharacterProvider.h. A driver with fake API tables checks registration counts, cell fields,
   the character def, and that blob bytes equal the input files.
3. registerAll returns false when RegisterAssets accepts fewer entries than kAssetCount (driver scenario).
4. Refusing loose .tga textures (--allow-tga ships them as restype 3), corrupt .lip files, physical .2da files,
   invalid >16-char resrefs, zero-byte assets, and symlinks, all with PackError.
5. Deterministic sorting by (flags, res_type, resref); resref case normalization; optional license gate.
6. Layer 1 audio container rules (bare RIFF WAVE PCM only).
7. --asset-list parsing, --2da-rows (pykotor mocked; missing pykotor is a clear error), 64 MB blob warning.
8. i686-w64-mingw32 blob symbols carry the leading underscore (skipped when the toolchain is absent).

All files live in tempfile directories; nothing is written into the repo.
"""
from __future__ import annotations

import contextlib
import io
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent / "kvoice"))

import k1_port_vfs_pack as vp  # noqa: E402
from k1_port_vfs_pack import (  # noqa: E402
    K2VFS_FLAG_RESMAN,
    K2VFS_FLAG_STREAM,
    PackError,
    TwoDaCell,
    pack_character_bundle,
)

SHARED_SRC = HERE.parent / "_shared_src"
KPM_COMMON = Path.home() / "Games" / "KOTOR2_Mod_Workspace" / "tools" / "kotor-patch-manager" / "Patches" / "Common"

# Driver for the Juhani fixture. kAssets order is (flags, res_type, resref):
# 0 k1p_juhaniba.mdl, 1 k1p_juhani.utc, 2 kvjdlg01.dlg, 3 k1p_juhani.ssf, 4 kvjv0001.lip,
# 5 k1p_juhanih.tpc, 6 k1p_juhaniba.mdx, 7 kvjv0001.wav (stream). argv[1..8] are the source files in that order.
DRIVER_CPP = r'''
#include "CharacterProvider.h"
#include "juhani_vfs_bundle.h"

#include <cstdio>
#include <cstring>
#include <vector>

using namespace k2vfs_juhani;

namespace {

int g_fail = 0;
#define EXPECT(cond) do { if (!(cond)) { std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #cond); ++g_fail; } } while (0)

bool g_short_assets = false;
const K2VfsAssetEntry* g_asset_seen = nullptr;
uint32_t g_asset_seen_count = 0;
const char* g_asset_owner = nullptr;
const K2Vfs2DaCell* g_cells_seen = nullptr;
uint32_t g_cell_seen_count = 0;
uint32_t g_char_calls = 0;
K2CharacterDefinition g_char_seen = {};

int32_t KPATCH_CALL fakeRegisterAssets(const K2VfsAssetEntry* entries, uint32_t count, const char* owner) {
    g_asset_seen = entries;
    g_asset_seen_count = count;
    g_asset_owner = owner;
    return static_cast<int32_t>(g_short_assets ? count - 1 : count);
}

int32_t KPATCH_CALL fakeRegister2DaCells(const K2Vfs2DaCell* cells, uint32_t count, const char* owner) {
    (void)owner;
    g_cells_seen = cells;
    g_cell_seen_count = count;
    return static_cast<int32_t>(count);
}

int32_t KPATCH_CALL fakeRegisterCharacter(const K2CharacterDefinition* def, const char* owner) {
    (void)owner;
    ++g_char_calls;
    g_char_seen = *def;
    return 1;
}

bool hasCell(const char* table, uint32_t row, int32_t tmpl, const char* column, const char* value) {
    for (uint32_t i = 0; i < g_cell_seen_count; ++i) {
        const K2Vfs2DaCell& c = g_cells_seen[i];
        if (std::strcmp(c.table_name, table) == 0 && c.row_index == row && c.template_row == tmpl
            && std::strcmp(c.column_name, column) == 0 && std::strcmp(c.value, value) == 0) {
            return true;
        }
    }
    return false;
}

bool readFile(const char* path, std::vector<unsigned char>& out) {
    std::FILE* f = std::fopen(path, "rb");
    if (!f) return false;
    out.clear();
    unsigned char buf[4096];
    size_t n = 0;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) out.insert(out.end(), buf, buf + n);
    std::fclose(f);
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 1 + static_cast<int>(kAssetCount)) {
        std::fprintf(stderr, "driver: want %u source paths, got %d\n", kAssetCount, argc - 1);
        return 2;
    }
    EXPECT(kAssetCount == 8);
    EXPECT(k2DaCellCount == 5);

    K2AssetVfsApi vfs = {};
    vfs.struct_size = sizeof(K2AssetVfsApi);
    vfs.RegisterAssets = &fakeRegisterAssets;
    vfs.Register2DaCells = &fakeRegister2DaCells;
    K2CharacterLibApi lib = {};
    lib.struct_size = sizeof(K2CharacterLibApi);
    lib.RegisterCharacter = &fakeRegisterCharacter;

    // Scenario 1: every registration accepted.
    RegisterCounts counts = {-1, -1, -1};
    EXPECT(registerAll(&vfs, &lib, "juhani", &counts));
    EXPECT(counts.assets == 8);
    EXPECT(counts.cells == 5);
    EXPECT(counts.character == 1);
    EXPECT(g_asset_seen == kAssets && g_asset_seen_count == 8);
    EXPECT(g_asset_owner != nullptr && std::strcmp(g_asset_owner, "juhani") == 0);
    EXPECT(g_cells_seen == k2DaCells && g_cell_seen_count == 5);
    EXPECT(hasCell("appearance", 818, 17, "label", "K1P_Juhani"));
    EXPECT(hasCell("appearance", 818, 17, "race", "5"));
    EXPECT(hasCell("appearance", 818, 17, "modela", "k1p_juhaniba"));
    EXPECT(hasCell("heads", 250, -1, "head", "k1p_juhanih"));
    EXPECT(hasCell("soundset", 70, -1, "resref", "ss_juhani"));
    EXPECT(g_char_calls == 1);
    EXPECT(g_char_seen.struct_size == sizeof(K2CharacterDefinition));
    EXPECT(std::strcmp(g_char_seen.character_id, "juhani") == 0);
    EXPECT(std::strcmp(g_char_seen.display_name, "Juhani") == 0);
    EXPECT(std::strcmp(g_char_seen.default_tag, "k1p_juhani") == 0);
    EXPECT(std::strcmp(g_char_seen.default_utc, "k1p_juhani") == 0);
    EXPECT(std::strcmp(g_char_seen.default_dlg, "kvjdlg01") == 0);
    EXPECT(g_char_seen.appearance_id == 818 && g_char_seen.head_id == 250 && g_char_seen.soundset_id == 70);
    EXPECT(std::strcmp(kAssets[0].resref, "k1p_juhaniba") == 0 && kAssets[0].res_type == 2002);
    EXPECT(std::strcmp(kAssets[7].resref, "kvjv0001") == 0 && kAssets[7].res_type == 4 && kAssets[7].flags == 1);
    // The asm end label sits exactly size bytes after the start label.
    EXPECT(static_cast<uint32_t>(kvfs_0_end - kvfs_0) == kAssets[0].size);

    for (uint32_t i = 0; i < kAssetCount; ++i) {
        std::vector<unsigned char> src;
        if (!readFile(argv[1 + i], src)) {
            std::fprintf(stderr, "driver: cannot read %s\n", argv[1 + i]);
            return 3;
        }
        const K2VfsAssetEntry& e = g_asset_seen[i];
        EXPECT(e.size == src.size());
        EXPECT(e.size == 0 || std::memcmp(e.data, src.data(), e.size) == 0);
    }

    // Scenario 2: RegisterAssets accepts one entry fewer than kAssetCount -> false.
    g_short_assets = true;
    RegisterCounts short_counts = {-1, -1, -1};
    EXPECT(!registerAll(&vfs, &lib, "juhani", &short_counts));
    EXPECT(short_counts.assets == 7);
    g_short_assets = false;

    // Scenario 3: no asset VFS -> false.
    EXPECT(!registerAll(nullptr, &lib, "juhani", nullptr));

    if (g_fail) {
        std::fprintf(stderr, "%d check(s) failed\n", g_fail);
        return 1;
    }
    std::printf("OK 8 assets, 5 cells, character registered\n");
    return 0;
}
'''


def make_valid_lip_bytes(duration: float = 1.0) -> bytes:
    """Create valid binary LIP bytes (16-byte header + 2 keyframes)."""
    # 8-byte magic + float32 length + uint32 count + 2 x (float32 time, uint8 shape)
    return struct.pack("<8sfIfBfB", b"LIP V1.0", float(duration), 2, 0.0, 0, float(duration), 0)


class TestK1PortVfsPack(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory(prefix="k1p_vfs_pack_")
        self.test_dir = Path(self._tmp.name)

    def tearDown(self):
        self._tmp.cleanup()

    def _compile_and_run_driver(self, bundle_dir: Path, sources: list[str]) -> subprocess.CompletedProcess:
        """Compile the driver against the real CharacterProvider.h, link it, and run it on the sources."""
        driver_cpp = self.test_dir / "driver.cpp"
        driver_cpp.write_text(DRIVER_CPP, encoding="utf-8")
        exe = self.test_dir / "driver"
        cmd = [
            "g++",
            "-std=c++17",
            "-Wall",
            "-Wextra",
            "-Werror",
            f"-I{bundle_dir}",
            f"-I{SHARED_SRC}",
            f"-I{KPM_COMMON}",
            str(driver_cpp),
            "-o",
            str(exe),
        ]
        res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        self.assertEqual(res.returncode, 0, f"g++ compilation/link failed:\n{res.stderr}")
        run = subprocess.run([str(exe), *sources], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        self.assertEqual(run.returncode, 0, f"driver failed:\n{run.stdout}\n{run.stderr}")
        return run

    def test_pack_juhani_complete_bundle(self):
        """Pack a complete synthetic Juhani bundle, verify manifest, blobs, header, and compile/link/run."""
        stage = self.test_dir / "stage"
        stage.mkdir()
        stream_dir = stage / "StreamVoice"
        stream_dir.mkdir()

        # Synthetic assets
        mdl_bytes = b"JUHANI_MDL_GEOMETRY_1234"
        mdx_bytes = b"JUHANI_MDX_ANIMATION_5678"
        tpc_bytes = b"JUHANI_TPC_TEXTURE_DATA"
        utc_bytes = b"JUHANI_UTC_CREATURE_DATA"
        dlg_bytes = b"JUHANI_DLG_CONVERSATION"
        ssf_bytes = b"JUHANI_SSF_SOUNDSET_MAP"
        lip_bytes = make_valid_lip_bytes(1.5)
        wav_bytes = b"RIFF\x24\x00\x00\x00WAVEfmt \x10\x00\x00\x00\x01\x00\x01\x00"

        (stage / "k1p_juhaniba.mdl").write_bytes(mdl_bytes)
        (stage / "k1p_juhaniba.mdx").write_bytes(mdx_bytes)
        (stage / "k1p_juhanih.tpc").write_bytes(tpc_bytes)
        (stage / "k1p_juhani.utc").write_bytes(utc_bytes)
        (stage / "kvjdlg01.dlg").write_bytes(dlg_bytes)
        (stage / "k1p_juhani.ssf").write_bytes(ssf_bytes)
        (stage / "kvjv0001.lip").write_bytes(lip_bytes)
        (stream_dir / "kvjv0001.wav").write_bytes(wav_bytes)

        # Virtual 2DA cells
        cells = [
            TwoDaCell(table="appearance", row=818, column="label", value="K1P_Juhani", template_row=17),
            TwoDaCell(table="appearance", row=818, column="race", value="5", template_row=17),
            TwoDaCell(table="appearance", row=818, column="modela", value="k1p_juhaniba", template_row=17),
            TwoDaCell(table="heads", row=250, column="head", value="k1p_juhanih"),
            TwoDaCell(table="soundset", row=70, column="resref", value="ss_juhani"),
        ]

        bundle = self.test_dir / "bundle"
        manifest = pack_character_bundle(
            character_id="juhani",
            asset_paths=[stage],
            output_dir=bundle,
            display_name="Juhani",
            default_tag="k1p_juhani",
            default_utc="k1p_juhani",
            default_dlg="kvjdlg01",
            appearance_id=818,
            head_id=250,
            soundset_id=70,
            two_da_cells=cells,
        )

        # 1. Verify Manifest
        out_header = bundle / "juhani_vfs_bundle.h"
        self.assertTrue(out_header.is_file())
        self.assertEqual(manifest["header_path"], str(out_header))
        self.assertEqual(manifest["character_id"], "juhani")
        self.assertEqual(manifest["asset_count"], 8)
        self.assertEqual(manifest["cell_count"], 5)
        expected_total_bytes = (
            len(mdl_bytes)
            + len(mdx_bytes)
            + len(tpc_bytes)
            + len(utc_bytes)
            + len(dlg_bytes)
            + len(ssf_bytes)
            + len(lip_bytes)
            + len(wav_bytes)
        )
        self.assertEqual(manifest["total_bytes"], expected_total_bytes)
        self.assertEqual(len(manifest["header_sha256"]), 64)

        # Verify character_def in manifest
        cdef = manifest["character_def"]
        self.assertEqual(cdef["character_id"], "juhani")
        self.assertEqual(cdef["display_name"], "Juhani")
        self.assertEqual(cdef["default_tag"], "k1p_juhani")
        self.assertEqual(cdef["default_utc"], "k1p_juhani")
        self.assertEqual(cdef["default_dlg"], "kvjdlg01")
        self.assertEqual(cdef["appearance_id"], 818)
        self.assertEqual(cdef["head_id"], 250)
        self.assertEqual(cdef["soundset_id"], 70)

        # 2. One raw .bin per asset in the blob dir, bytes identical to the input file
        blob_dir = Path(manifest["blob_dir"])
        self.assertEqual(blob_dir, bundle)
        for a in manifest["assets"]:
            blob = Path(a["blob_path"])
            self.assertEqual(blob.parent, blob_dir)
            self.assertEqual(blob.read_bytes(), Path(a["source_path"]).read_bytes())
        self.assertEqual((blob_dir / "kvfs_0_k1p_juhaniba.bin").read_bytes(), mdl_bytes)
        self.assertEqual((blob_dir / "kvfs_7_kvjv0001.bin").read_bytes(), wav_bytes)

        # 3. Verify generated C++ header contents
        header_text = out_header.read_text(encoding="utf-8")

        self.assertIn("#pragma once", header_text)
        self.assertIn('#include "CharacterProvider.h"', header_text)
        self.assertIn("#include <stdint.h>", header_text)
        self.assertIn("namespace k2vfs_juhani {", header_text)

        # Blob bytes are linked with .incbin, never emitted as C byte arrays
        self.assertIn("__asm__(", header_text)
        self.assertNotIn("alignas(8)", header_text)
        self.assertNotIn("0x4a, 0x55", header_text)  # 'JU' would start the MDL byte array
        incbin = (blob_dir / "kvfs_0_k1p_juhaniba.bin").as_posix()
        self.assertIn(f'".incbin \\"{incbin}\\"\\n"', header_text)
        for n in range(8):
            self.assertIn(f"extern const uint8_t kvfs_{n}[];", header_text)
            self.assertIn(f"extern const uint8_t kvfs_{n}_end[];", header_text)

        # kAssets table: data points at the blob symbol, size is a constant
        self.assertIn("static const K2VfsAssetEntry kAssets[] = {", header_text)
        self.assertIn(f'{{ "k1p_juhaniba", 2002, 0, kvfs_0, {len(mdl_bytes)}u }}', header_text)
        self.assertIn(f'{{ "k1p_juhani", 2027, 0, kvfs_1, {len(utc_bytes)}u }}', header_text)
        self.assertIn(f'{{ "kvjdlg01", 2029, 0, kvfs_2, {len(dlg_bytes)}u }}', header_text)
        self.assertIn(f'{{ "k1p_juhani", 2060, 0, kvfs_3, {len(ssf_bytes)}u }}', header_text)
        self.assertIn(f'{{ "kvjv0001", 3004, 0, kvfs_4, {len(lip_bytes)}u }}', header_text)
        self.assertIn(f'{{ "k1p_juhanih", 3007, 0, kvfs_5, {len(tpc_bytes)}u }}', header_text)
        self.assertIn(f'{{ "k1p_juhaniba", 3008, 0, kvfs_6, {len(mdx_bytes)}u }}', header_text)
        # StreamVoice audio must have flags = 1
        self.assertIn(f'{{ "kvjv0001", 4, 1, kvfs_7, {len(wav_bytes)}u }}', header_text)

        # Verify k2DaCells table (5-field K2Vfs2DaCell with template_row)
        self.assertIn("static const K2Vfs2DaCell k2DaCells[] = {", header_text)
        self.assertIn('{ "appearance", 818, 17, "label", "K1P_Juhani" }', header_text)
        self.assertIn('{ "appearance", 818, 17, "race", "5" }', header_text)
        self.assertIn('{ "appearance", 818, 17, "modela", "k1p_juhaniba" }', header_text)
        self.assertIn('{ "heads", 250, -1, "head", "k1p_juhanih" }', header_text)
        self.assertIn('{ "soundset", 70, -1, "resref", "ss_juhani" }', header_text)

        # Verify kCharacterDef
        self.assertIn("static const K2CharacterDefinition kCharacterDef = {", header_text)
        self.assertIn("sizeof(K2CharacterDefinition),", header_text)
        self.assertIn('"juhani",', header_text)
        self.assertIn('"Juhani",', header_text)
        self.assertIn('"k1p_juhani",', header_text)
        self.assertIn('"kvjdlg01",', header_text)
        self.assertIn("818,", header_text)
        self.assertIn("250,", header_text)
        self.assertIn("70", header_text)

        # Verify registerAll helper: counts out-param, Register2DaCells call, RegisterAssets threshold
        self.assertIn("struct RegisterCounts {", header_text)
        self.assertIn(
            "inline bool registerAll(const K2AssetVfsApi* vfs, const K2CharacterLibApi* charlib, "
            "const char* owner, RegisterCounts* counts)",
            header_text,
        )
        self.assertIn("vfs->Register2DaCells(k2DaCells, k2DaCellCount, pkgOwner)", header_text)
        self.assertIn("charlib->RegisterCharacter(&kCharacterDef, pkgOwner)", header_text)
        self.assertIn("static_cast<uint32_t>(out->assets) < kAssetCount", header_text)

        # 4. Compile and link against the REAL _shared_src/CharacterProvider.h, run the driver
        self._compile_and_run_driver(bundle, [a["source_path"] for a in manifest["assets"]])

    def test_mingw_i686_blob_symbols_get_underscore_prefix(self):
        """On i686-w64-mingw32 the asm labels carry the leading underscore C symbols need."""
        cxx = shutil.which("i686-w64-mingw32-g++")
        nm = shutil.which("i686-w64-mingw32-nm")
        if not cxx or not nm:
            self.skipTest("i686-w64-mingw32 toolchain not installed")
        stage = self.test_dir / "stage"
        stage.mkdir()
        (stage / "k1p_juhani.utc").write_bytes(b"UTC_MINGW_BLOB")
        bundle = self.test_dir / "bundle"
        pack_character_bundle("juhani", [stage], output_dir=bundle)

        tu = self.test_dir / "mingw_tu.cpp"
        tu.write_text(
            '#include "juhani_vfs_bundle.h"\n'
            'int probe() { return k2vfs_juhani::registerAll(nullptr, nullptr, "x", nullptr) ? 1 : 0; }\n',
            encoding="utf-8",
        )
        obj = self.test_dir / "mingw_tu.o"
        res = subprocess.run(
            [
                cxx, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                f"-I{bundle}", f"-I{SHARED_SRC}", f"-I{KPM_COMMON}",
                "-c", str(tu), "-o", str(obj),
            ],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
        )
        self.assertEqual(res.returncode, 0, f"mingw compile failed:\n{res.stderr}")
        listing = subprocess.run([nm, str(obj)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        names = {line.split()[-1] for line in listing.stdout.splitlines() if line.strip()}
        self.assertIn("_kvfs_0", names)
        self.assertIn("_kvfs_0_end", names)
        self.assertNotIn("kvfs_0", names)

    def test_reject_loose_tga(self):
        """Reject loose .tga textures with exact PackError message."""
        tga_file = self.test_dir / "juhani_tex.tga"
        tga_file.write_bytes(b"TGA_RAW_TEXTURE_BYTES")
        out_header = self.test_dir / "bundle.h"

        with self.assertRaises(PackError) as ctx:
            pack_character_bundle(
                character_id="juhani",
                asset_paths=[tga_file],
                output_header=out_header,
            )
        self.assertEqual(
            str(ctx.exception),
            "loose .tga textures bypass CExoResMan::Demand via ReadRaw; convert to .tpc first",
        )

    def test_tga_refused_by_default_and_restype_3_when_allowed(self):
        """--allow-tga ships .tga as restype 3 on Layer 1; without it the CLI refuses."""
        tga = self.test_dir / "k1p_juhanit.tga"
        tga.write_bytes(b"TGA_BYTES")
        out_dir = self.test_dir / "tga_out"

        err = io.StringIO()
        with contextlib.redirect_stderr(err), contextlib.redirect_stdout(io.StringIO()):
            rc = vp.main(["--character-id", "juhani", "--assets", str(tga), "--out-dir", str(out_dir)])
        self.assertEqual(rc, 1)
        self.assertIn("loose .tga textures", err.getvalue())

        with contextlib.redirect_stdout(io.StringIO()):
            rc = vp.main(["--character-id", "juhani", "--assets", str(tga), "--out-dir", str(out_dir), "--allow-tga"])
        self.assertEqual(rc, 0)
        manifest = pack_character_bundle("juhani", [tga], output_dir=out_dir, allow_tga=True)
        self.assertEqual(manifest["assets"][0]["res_type"], 3)
        self.assertEqual(manifest["assets"][0]["flags"], K2VFS_FLAG_RESMAN)

    def test_reject_corrupt_lip(self):
        """Reject corrupt .lip files (bad magic or missing t=0 keyframe)."""
        out_header = self.test_dir / "bundle.h"

        # 1. Bad magic signature
        bad_magic_file = self.test_dir / "bad_magic.lip"
        bad_magic_file.write_bytes(b"NOT_LIP!123456789012345678")
        with self.assertRaises(PackError) as ctx:
            pack_character_bundle(
                character_id="juhani",
                asset_paths=[bad_magic_file],
                output_header=out_header,
            )
        self.assertIn("corrupt .lip file", str(ctx.exception))
        self.assertIn("bad signature", str(ctx.exception))

        # 2. Missing t=0 keyframe (first keyframe at t=0.5)
        bad_keys_file = self.test_dir / "bad_keys.lip"
        bad_keys_bytes = struct.pack("<8sfIfBfB", b"LIP V1.0", 1.0, 2, 0.5, 0, 1.0, 0)
        bad_keys_file.write_bytes(bad_keys_bytes)
        with self.assertRaises(PackError) as ctx:
            pack_character_bundle(
                character_id="juhani",
                asset_paths=[bad_keys_file],
                output_header=out_header,
            )
        self.assertIn("corrupt .lip file", str(ctx.exception))
        self.assertIn("first key time", str(ctx.exception))

        # 3. Short file (< 16 bytes)
        short_file = self.test_dir / "short.lip"
        short_file.write_bytes(b"LIP V1.0\x00")
        with self.assertRaises(PackError) as ctx:
            pack_character_bundle(
                character_id="juhani",
                asset_paths=[short_file],
                output_header=out_header,
            )
        self.assertIn("corrupt .lip file", str(ctx.exception))
        self.assertIn("short file", str(ctx.exception))

    def test_reject_physical_2da(self):
        """Reject physical .2da files in the asset list."""
        twoda_file = self.test_dir / "appearance.2da"
        twoda_file.write_bytes(b"2DA V2.0\n\n  label\n0 test\n")
        out_header = self.test_dir / "bundle.h"

        with self.assertRaises(PackError) as ctx:
            pack_character_bundle(
                character_id="juhani",
                asset_paths=[twoda_file],
                output_header=out_header,
            )
        self.assertIn("physical .2da files cannot be packed as raw assets", str(ctx.exception))
        self.assertIn("K2Vfs2DaCell", str(ctx.exception))

    def test_reject_invalid_resref(self):
        """Reject resrefs longer than 16 characters or with invalid characters."""
        out_header = self.test_dir / "bundle.h"

        # 17-character stem
        long_stem_file = self.test_dir / "this_is_too_long_17c.utc"
        long_stem_file.write_bytes(b"UTC")
        with self.assertRaises(PackError) as ctx:
            pack_character_bundle(
                character_id="juhani",
                asset_paths=[long_stem_file],
                output_header=out_header,
            )
        self.assertIn("invalid resref stem", str(ctx.exception))

        # Invalid characters (hyphen)
        hyphen_file = self.test_dir / "bad-stem.utc"
        hyphen_file.write_bytes(b"UTC")
        with self.assertRaises(PackError) as ctx:
            pack_character_bundle(
                character_id="juhani",
                asset_paths=[hyphen_file],
                output_header=out_header,
            )
        self.assertIn("invalid resref stem", str(ctx.exception))

    def test_resref_case_normalization(self):
        """Uppercase stems and extensions must be normalized to lowercase."""
        upper_file = self.test_dir / "K1P_JUHANI.MDL"
        upper_file.write_bytes(b"MODEL_DATA")
        out_header = self.test_dir / "bundle.h"

        manifest = pack_character_bundle(
            character_id="juhani",
            asset_paths=[upper_file],
            output_header=out_header,
        )
        self.assertEqual(manifest["assets"][0]["resref"], "k1p_juhani")
        self.assertEqual(manifest["assets"][0]["extension"], ".mdl")
        self.assertEqual(manifest["assets"][0]["res_type"], 2002)

    def test_deterministic_sorting(self):
        """kAssets must be sorted by (flags, res_type, resref)."""
        stage = self.test_dir / "sort_stage"
        stage.mkdir()
        stream_dir = stage / "streamvoice"
        stream_dir.mkdir()

        (stage / "b_model.mdl").write_bytes(b"MDL_B")  # flags=0, type=2002, resref=b_model
        (stage / "a_model.mdl").write_bytes(b"MDL_A")  # flags=0, type=2002, resref=a_model
        (stage / "z_creature.utc").write_bytes(b"UTC")  # flags=0, type=2027, resref=z_creature
        (stream_dir / "a_voice.wav").write_bytes(b"WAV")  # flags=1, type=4, resref=a_voice
        (stage / "a_tex.tpc").write_bytes(b"TPC")  # flags=0, type=3007, resref=a_tex

        out_header = self.test_dir / "bundle.h"
        manifest = pack_character_bundle(
            character_id="juhani",
            asset_paths=[stage],
            output_header=out_header,
        )

        expected_order = [
            ("a_model", 2002, K2VFS_FLAG_RESMAN),
            ("b_model", 2002, K2VFS_FLAG_RESMAN),
            ("z_creature", 2027, K2VFS_FLAG_RESMAN),
            ("a_tex", 3007, K2VFS_FLAG_RESMAN),
            ("a_voice", 4, K2VFS_FLAG_STREAM),
        ]
        actual_order = [(a["resref"], a["res_type"], a["flags"]) for a in manifest["assets"]]
        self.assertEqual(actual_order, expected_order)
        self.assertEqual([a["symbol"] for a in manifest["assets"]], [f"kvfs_{i}" for i in range(5)])

    def test_license_gate(self):
        """License gate triggers when ledger is provided."""
        import hashlib
        from k1_port_sources import Ledger

        ledger_path = self.test_dir / "test_ledger.json"
        stage = self.test_dir / "stage"
        stage.mkdir()

        asset_file = stage / "k1p_flagged.mdl"
        content = b"FLAGGED_ASSET_BYTES"
        asset_file.write_bytes(content)
        sha = hashlib.sha256(content).hexdigest()

        # Create ledger with flagged asset (upscale_plus_v0.52 requires author approval)
        led = Ledger(ledger_path)
        led.add(str(asset_file), kind="model", source_id="upscale_plus_v0.52", sha256=sha)
        led.save()

        out_header = self.test_dir / "bundle.h"

        with self.assertRaises(PackError) as ctx:
            pack_character_bundle(
                character_id="juhani",
                asset_paths=[asset_file],
                output_header=out_header,
                ledger_path=ledger_path,
                stage_root=stage,
            )
        self.assertIn("License gate check failed", str(ctx.exception))

    def test_reject_layer1_audio_containers_and_mp3(self):
        """Enforce VFS_COLLISION_FEEDBACK.md B1-B3 & C2-C3: Layer 1 requires bare RIFF WAVE PCM."""
        out_header = self.test_dir / "bundle.h"

        # 1. Loose .mp3 on Layer 1 (K2VFS_FLAG_RESMAN) must be rejected
        mp3_file = self.test_dir / "vo_line.mp3"
        mp3_file.write_bytes(b"\xff\xfb\x90\x00MP3_PAYLOAD")
        with self.assertRaises(PackError) as ctx:
            pack_character_bundle(
                character_id="juhani",
                asset_paths=[mp3_file],
                output_header=out_header,
            )
        self.assertIn("never queried by Layer 1 PlaySound", str(ctx.exception))

        # 2. 58-byte StreamVoice MP3-in-WAV container on Layer 1 must be rejected
        vo_wav = self.test_dir / "kvjv0001.wav"
        vo_buf = bytearray(68)
        vo_buf[0:4] = b"RIFF"
        struct.pack_into("<I", vo_buf, 4, 50)
        vo_buf[8:12] = b"WAVE"
        vo_buf[58:60] = b"\xff\xfb"
        vo_wav.write_bytes(bytes(vo_buf))
        with self.assertRaises(PackError) as ctx:
            pack_character_bundle(
                character_id="juhani",
                asset_paths=[vo_wav],
                output_header=out_header,
            )
        self.assertIn("58-byte StreamVoice MP3 container", str(ctx.exception))

        # 3. 470-byte (0x1D6) StreamSounds SFX container on Layer 1 must be rejected
        sfx_wav = self.test_dir / "p_juhani_atk1.wav"
        sfx_buf = bytearray(0x1D6 + 16)
        sfx_buf[0x1D6:0x1DA] = b"RIFF"
        sfx_wav.write_bytes(bytes(sfx_buf))
        with self.assertRaises(PackError) as ctx:
            pack_character_bundle(
                character_id="juhani",
                asset_paths=[sfx_wav],
                output_header=out_header,
            )
        self.assertIn("470-byte (0x1D6) StreamSounds", str(ctx.exception))

        # 4. Bare RIFF WAVE PCM on Layer 1 (for NWScript PlaySound) is accepted!
        pcm_wav = self.test_dir / "pcm_sfx.wav"
        pcm_wav.write_bytes(b"RIFF\x24\x00\x00\x00WAVEfmt \x10\x00\x00\x00\x01\x00\x01\x00")
        manifest = pack_character_bundle(
            character_id="juhani",
            asset_paths=[pcm_wav],
            output_header=out_header,
        )
        self.assertEqual(manifest["assets"][0]["flags"], K2VFS_FLAG_RESMAN)
        self.assertEqual(manifest["assets"][0]["res_type"], 4)

    def test_reject_symlinks(self):
        """Enforce VFS_COLLISION_FEEDBACK.md C5: refuse symlinks during staging scan."""
        out_header = self.test_dir / "bundle.h"
        real_file = self.test_dir / "real_model.mdl"
        real_file.write_bytes(b"MDL_DATA")
        link_file = self.test_dir / "link_model.mdl"
        link_file.symlink_to(real_file)

        with self.assertRaises(PackError) as ctx:
            pack_character_bundle(
                character_id="juhani",
                asset_paths=[link_file],
                output_header=out_header,
            )
        self.assertIn("symlinks are not allowed", str(ctx.exception))

    def test_reject_zero_byte_asset(self):
        """A zero-byte blob cannot be registered (K2VfsAssetEntry.size must be > 0)."""
        empty = self.test_dir / "k1p_empty.utc"
        empty.write_bytes(b"")
        with self.assertRaises(PackError) as ctx:
            pack_character_bundle("juhani", [empty], output_dir=self.test_dir / "empty_out")
        self.assertIn("zero-byte asset refused", str(ctx.exception))

    def test_legacy_out_header_puts_blobs_beside_header(self):
        """--out-header keeps working: the header is the given path and the .bin blobs sit next to it."""
        stage = self.test_dir / "stage"
        stage.mkdir()
        (stage / "k1p_juhani.utc").write_bytes(b"UTC_LEGACY")
        legacy_dir = self.test_dir / "legacy"
        manifest = pack_character_bundle("juhani", [stage], legacy_dir / "custom_bundle.h")
        self.assertEqual(Path(manifest["header_path"]), legacy_dir.resolve() / "custom_bundle.h")
        self.assertTrue((legacy_dir / "custom_bundle.h").is_file())
        self.assertEqual((legacy_dir / "kvfs_0_k1p_juhani.bin").read_bytes(), b"UTC_LEGACY")

    def test_symbol_prefix_keeps_two_bundles_apart(self):
        """--symbol-prefix renames the asm labels and .bin names so two bundles can share one DLL."""
        stage = self.test_dir / "stage"
        stage.mkdir()
        (stage / "k1p_carth.utc").write_bytes(b"UTC_CARTH")
        out_dir = self.test_dir / "carth_out"
        manifest = pack_character_bundle("carth", [stage], output_dir=out_dir, symbol_prefix="kvfs_carth")
        self.assertEqual([a["symbol"] for a in manifest["assets"]], ["kvfs_carth_0"])
        self.assertEqual((out_dir / "kvfs_carth_0_k1p_carth.bin").read_bytes(), b"UTC_CARTH")
        header = (out_dir / "juhani_vfs_bundle.h").read_text(encoding="utf-8")
        self.assertIn("namespace k2vfs_carth {", header)
        self.assertIn("extern const uint8_t kvfs_carth_0[];", header)
        self.assertNotIn("kvfs_0", header)
        with self.assertRaises(PackError):
            pack_character_bundle("carth", [stage], output_dir=out_dir, symbol_prefix="9bad prefix")

    def test_cli_asset_list_2da_rows_and_stage_dir(self):
        """--asset-list (relative paths, comments, blanks) plus --2da-rows (pykotor mocked) write the stage dir."""
        stage = self.test_dir / "stage"
        stage.mkdir()
        (stage / "k1p_juhani.utc").write_bytes(b"UTC_CLI")
        (stage / "k1p_juhaniba.mdl").write_bytes(b"MDL_CLI")
        twoda = stage / "appearance.2da"
        twoda.write_bytes(b"2DA V2.0\n")  # never parsed: read_twoda_rows is mocked
        asset_list = self.test_dir / "assets.txt"
        asset_list.write_text(
            "# Juhani bundle list\n\nstage/k1p_juhani.utc\n  stage/k1p_juhaniba.mdl  \n",
            encoding="utf-8",
        )
        out_dir = self.test_dir / "stage_out"

        def fake_rows(path, rows):
            self.assertEqual(Path(path).name, "appearance.2da")
            self.assertEqual(rows, [818])
            return {818: {"label": "K1P_Juhani", "race": "5", "modela": "k1p_juhaniba"}}

        out, err = io.StringIO(), io.StringIO()
        with mock.patch.object(vp, "read_twoda_rows", side_effect=fake_rows), \
                contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            rc = vp.main([
                "--character-id", "juhani",
                "--asset-list", str(asset_list),
                "--out-dir", str(out_dir),
                "--2da-rows", f"appearance={twoda}:818",
                "--appearance-id", "818",
            ])
        self.assertEqual(rc, 0, err.getvalue())
        header = (out_dir / "juhani_vfs_bundle.h").read_text(encoding="utf-8")
        self.assertIn('{ "appearance", 818, 17, "label", "K1P_Juhani" }', header)
        self.assertIn('{ "appearance", 818, 17, "modela", "k1p_juhaniba" }', header)
        self.assertEqual((out_dir / "kvfs_0_k1p_juhaniba.bin").read_bytes(), b"MDL_CLI")
        self.assertEqual((out_dir / "kvfs_1_k1p_juhani.bin").read_bytes(), b"UTC_CLI")
        self.assertIn("total blob bytes 14", out.getvalue())
        self.assertEqual(err.getvalue(), "")

    def test_2da_rows_without_pykotor_is_a_clear_error(self):
        """Without pykotor the 2DA row reader fails with a PackError that names the k1port venv."""
        twoda = self.test_dir / "heads.2da"
        twoda.write_bytes(b"2DA V2.0\n")
        blocked = {m: None for m in (
            "pykotor", "pykotor.resource", "pykotor.resource.formats", "pykotor.resource.formats.twoda",
        )}
        with mock.patch.dict(sys.modules, blocked):
            with self.assertRaises(PackError) as ctx:
                vp.parse_2da_row_spec(f"heads={twoda}:249")
        self.assertIn("needs pykotor", str(ctx.exception))
        self.assertIn("k1port", str(ctx.exception))

    def test_2da_rows_spec_is_validated(self):
        """A --2da-rows spec without TABLE=, FILE:, or row numbers is refused before any file is read."""
        for bad in ("appearance", "appearance=", "appearance=x.2da", "appearance=x.2da:", "appearance=x.2da:abc"):
            with self.subTest(spec=bad), self.assertRaises(PackError):
                vp.parse_2da_row_spec(bad)

    def test_warns_above_blob_limit(self):
        """Total blob bytes are printed, and a warning goes to stderr above the 64 MB threshold."""
        stage = self.test_dir / "stage"
        stage.mkdir()
        (stage / "k1p_juhani.utc").write_bytes(b"0123456789ABCDEF")  # 16 bytes
        out, err = io.StringIO(), io.StringIO()
        with mock.patch.object(vp, "BLOB_WARN_BYTES", 8), \
                contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            rc = vp.main([
                "--character-id", "juhani",
                "--assets", str(stage),
                "--out-dir", str(self.test_dir / "warn_out"),
            ])
        self.assertEqual(rc, 0)
        self.assertIn("total blob bytes 16", out.getvalue())
        self.assertIn("warning: total blob bytes 16 exceed", err.getvalue())

    def test_repack_is_deterministic_and_writes_only_out_dir(self):
        """Repacking the same inputs gives the same header up to the out-dir path; each out dir holds only its files."""
        stage = self.test_dir / "stage"
        stage.mkdir()
        (stage / "k1p_juhani.utc").write_bytes(b"UTC_DET")
        (stage / "k1p_juhaniba.mdl").write_bytes(b"MDL_DET")
        out_a = self.test_dir / "out_a"
        out_b = self.test_dir / "out_b"
        pack_character_bundle("juhani", [stage], output_dir=out_a)
        pack_character_bundle("juhani", [stage], output_dir=out_b)
        header_a = (out_a / "juhani_vfs_bundle.h").read_text(encoding="utf-8")
        header_b = (out_b / "juhani_vfs_bundle.h").read_text(encoding="utf-8")
        self.assertEqual(header_a.replace(str(out_a), "<OUT>"), header_b.replace(str(out_b), "<OUT>"))
        self.assertEqual(
            sorted(p.name for p in out_a.iterdir()),
            ["juhani_vfs_bundle.h", "kvfs_0_k1p_juhaniba.bin", "kvfs_1_k1p_juhani.bin"],
        )


if __name__ == "__main__":
    unittest.main()
