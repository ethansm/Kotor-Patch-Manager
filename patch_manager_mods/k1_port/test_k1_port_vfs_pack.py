#!/usr/bin/env python3
"""Unit tests for k1_port_vfs_pack.py.

Verifies:
1. Packing complete synthetic Juhani bundle into juhani_vfs_bundle.h.
2. Exact byte arrays, K2VfsAssetEntry table, K2Vfs2DaCell table, K2CharacterDefinition, and registerAll helper.
3. C++ compilation with g++ -std=c++17 -Wall -Wextra -Werror against CharacterProvider.h.
4. Refusing loose .tga textures with PackError.
5. Refusing corrupt .lip files (bad magic or missing t=0 keyframe) with PackError.
6. Refusing physical .2da files and invalid >16-char resrefs with PackError.
7. Deterministic sorting by (flags, res_type, resref).
8. ResRef stem case normalization to lowercase.
9. Optional license gate verification.
"""
from __future__ import annotations

import os
import shutil
import struct
import subprocess
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO_ROOT = HERE.parent.parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent / "kvoice"))

from k1_port_vfs_pack import (  # noqa: E402
    K2VFS_FLAG_RESMAN,
    K2VFS_FLAG_STREAM,
    PackError,
    TwoDaCell,
    pack_character_bundle,
)

SCRATCH_DIR = REPO_ROOT / ".claude" / "scratch" / "vfs_p1"


def make_valid_lip_bytes(duration: float = 1.0) -> bytes:
    """Create valid binary LIP bytes (16-byte header + 2 keyframes)."""
    # 8-byte magic + float32 length + uint32 count + 2 x (float32 time, uint8 shape)
    return struct.pack("<8sfIfBfB", b"LIP V1.0", float(duration), 2, 0.0, 0, float(duration), 0)


class TestK1PortVfsPack(unittest.TestCase):
    def setUp(self):
        self.test_dir = SCRATCH_DIR / "test_run"
        if self.test_dir.exists():
            shutil.rmtree(self.test_dir)
        self.test_dir.mkdir(parents=True, exist_ok=True)

    def tearDown(self):
        if self.test_dir.exists():
            shutil.rmtree(self.test_dir)

    def test_pack_juhani_complete_bundle(self):
        """Test packing a complete synthetic Juhani bundle and verify header contents."""
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

        out_header = self.test_dir / "juhani_vfs_bundle.h"

        manifest = pack_character_bundle(
            character_id="juhani",
            asset_paths=[stage],
            output_header=out_header,
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
        self.assertTrue(out_header.is_file())
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

        # 2. Verify Generated C++ Header Contents
        header_text = out_header.read_text(encoding="utf-8")

        self.assertIn("#pragma once", header_text)
        self.assertIn('#include "CharacterProvider.h"', header_text)
        self.assertIn("#include <stdint.h>", header_text)
        self.assertIn("namespace k2vfs_juhani {", header_text)

        # Verify byte arrays and alignas(8)
        self.assertIn("alignas(8) static const uint8_t kAsset_0_k1p_juhaniba[] = {", header_text)
        # Check that actual byte values appear in header
        for b in mdl_bytes:
            self.assertIn(f"0x{b:02x}", header_text)

        # Verify kAssets table
        self.assertIn("static const K2VfsAssetEntry kAssets[] = {", header_text)
        self.assertIn('{ "k1p_juhaniba", 2002, 0, kAsset_0_k1p_juhaniba, sizeof(kAsset_0_k1p_juhaniba) }', header_text)
        self.assertIn('{ "k1p_juhani", 2027, 0, kAsset_1_k1p_juhani, sizeof(kAsset_1_k1p_juhani) }', header_text)
        self.assertIn('{ "kvjdlg01", 2029, 0, kAsset_2_kvjdlg01, sizeof(kAsset_2_kvjdlg01) }', header_text)
        self.assertIn('{ "k1p_juhani", 2060, 0, kAsset_3_k1p_juhani, sizeof(kAsset_3_k1p_juhani) }', header_text)
        self.assertIn('{ "kvjv0001", 3004, 0, kAsset_4_kvjv0001, sizeof(kAsset_4_kvjv0001) }', header_text)
        self.assertIn('{ "k1p_juhanih", 3007, 0, kAsset_5_k1p_juhanih, sizeof(kAsset_5_k1p_juhanih) }', header_text)
        self.assertIn('{ "k1p_juhaniba", 3008, 0, kAsset_6_k1p_juhaniba, sizeof(kAsset_6_k1p_juhaniba) }', header_text)
        # StreamVoice audio must have flags = 1
        self.assertIn('{ "kvjv0001", 4, 1, kAsset_7_kvjv0001, sizeof(kAsset_7_kvjv0001) }', header_text)

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

        # Verify registerAll helper
        self.assertIn(
            "inline bool registerAll(const K2AssetVfsApi* vfs, const K2CharacterLibApi* charlib, const char* owner)",
            header_text,
        )

        # 3. Verify Compilation with g++ against the REAL _shared_src/CharacterProvider.h
        shared_src = HERE.parent / "_shared_src"
        kpm_common = Path.home() / "Games" / "KOTOR2_Mod_Workspace" / "tools" / "kotor-patch-manager" / "Patches" / "Common"

        test_cpp = self.test_dir / "test_compile.cpp"
        test_cpp.write_text(
            """#include "CharacterProvider.h"
#include "juhani_vfs_bundle.h"

int main() {
    using namespace k2vfs_juhani;
    if (kAssetCount != 8) return 1;
    if (k2DaCellCount != 5) return 2;
    if (kCharacterDef.appearance_id != 818) return 3;
    if (kCharacterDef.head_id != 250) return 4;
    if (kCharacterDef.soundset_id != 70) return 5;
    return registerAll(nullptr, nullptr, "juhani") ? 4 : 0;
}
""",
            encoding="utf-8",
        )

        cmd = [
            "g++",
            "-std=c++17",
            "-Wall",
            "-Wextra",
            "-Werror",
            f"-I{self.test_dir}",
            f"-I{shared_src}",
            f"-I{kpm_common}",
            "-c",
            str(test_cpp),
            "-o",
            str(self.test_dir / "test_compile.o"),
        ]
        res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        self.assertEqual(res.returncode, 0, f"g++ compilation failed:\n{res.stderr}")

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


if __name__ == "__main__":
    unittest.main()
