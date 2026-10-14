#!/usr/bin/env python3
"""Host-only tests for k1_hd_overlay.py: synthetic fake K1 install and fake CR extract under tmp dirs, no real game."""
import hashlib
import io
import json
import os
import shutil
import sys
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import k1_hd_overlay as g  # noqa: E402

TGA_NTB = b"ntb-tga-bytes" * 64
TXI_NTB = b"ntb-txi-bytes\n"
TGA_NOTXI = b"notxi-tga-bytes" * 32
TGA_NESTED = b"nested-tga-bytes" * 16
LINKED = ("chitin.key", "dialog.tlk", "data", "modules", "rims", "TexturePacks", "swkotor.exe", "swkotor.ini")
STEMS = ["missing_one", "nested_tex", "notxi_tex", "ntb_tex"]


def sha(data):
    return hashlib.sha256(data).hexdigest()


class OverlayTests(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="k1hd_")).resolve()
        self.addCleanup(shutil.rmtree, self.tmp, ignore_errors=True)
        self.k1 = self.tmp / "k1"
        self.cr = self.tmp / "cr"
        self.make_k1()
        self.make_cr()
        self.sel1 = self.tmp / "sel1.tsv"
        self.sel2 = self.tmp / "sel2.tsv"
        self.sel1.write_text("stem\tnotes\nntb_tex\tx\nnested_tex\ty\nnotxi_tex\t\nmissing_one\tz\n",
                             encoding="utf-8")
        self.sel2.write_text("stem\nntb_tex\n", encoding="utf-8")

    def make_k1(self):
        k1 = self.k1
        (k1 / "data").mkdir(parents=True)
        (k1 / "data" / "fake.bif").write_bytes(b"bif")
        (k1 / "modules").mkdir()
        (k1 / "modules" / "m.mod").write_bytes(b"mod")
        (k1 / "rims").mkdir()
        (k1 / "TexturePacks").mkdir()
        (k1 / "Override").mkdir()
        (k1 / "Override" / "keep.tga").write_bytes(b"k1 override")
        for name in ("chitin.key", "dialog.tlk", "swkotor.exe", "swkotor.ini"):
            (k1 / name).write_bytes(name.encode())
        # lips and patch.erf are deliberately absent: they must be skipped, not linked

    def make_cr(self):
        files = {
            "shard_a/ntb_tex.tga": TGA_NTB,
            "shard_a/ntb_tex.txi": TXI_NTB,
            "shard_a/notxi_tex.tga": TGA_NOTXI,
            "shard_b/sub/NESTED_TEX.TGA": TGA_NESTED,  # found only by the case-insensitive recursive search
        }
        rows = [
            ("ntb_tex", "tga", TGA_NTB, "shard_a"),
            ("ntb_tex", "txi", TXI_NTB, "shard_a"),
            ("notxi_tex", "tga", TGA_NOTXI, "shard_a"),  # no txi row: txi is optional
            ("nested_tex", "tga", TGA_NESTED, "shard_b"),
        ]
        for rel, data in files.items():
            p = self.cr / rel
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_bytes(data)
        lines = ["stem\text\tsize\tsha256\tshard"]
        for stem, ext, data, shard in rows:
            lines.append(f"{stem}\t{ext}\t{len(data)}\t{sha(data)}\t{shard}")
        (self.cr / "manifest.tsv").write_text("\n".join(lines) + "\n", encoding="utf-8")

    def run_cli(self, argv):
        out, err = io.StringIO(), io.StringIO()
        with redirect_stdout(out), redirect_stderr(err):
            rc = g.main(argv)
        return rc, out.getvalue(), err.getvalue()

    def build_argv(self, out, *extra):
        return ["build", "--k1", str(self.k1), "--cr", str(self.cr),
                "--select", str(self.sel1), "--select", str(self.sel2),
                "--out", str(out), *extra]

    def build_ok(self, out):
        rc, _o, err = self.run_cli(self.build_argv(out))
        self.assertEqual(rc, 0, err)
        return out

    def read_marker(self, out):
        return json.loads((out / "K1_HD_OVERLAY.json").read_text(encoding="utf-8"))

    def assert_refused(self, out, argv=None):
        rc, _o, err = self.run_cli(argv if argv is not None else self.build_argv(out))
        self.assertEqual(rc, 2, err)
        self.assertIn("refused", err)

    def test_build_happy_path(self):
        out = self.build_ok(self.tmp / "ov")
        marker = self.read_marker(out)
        self.assertEqual(marker["source_id"], "characters_revisited_main")
        self.assertIs(marker["flagged"], True)
        self.assertEqual(marker["license_class"], "unknown")
        self.assertEqual(marker["select_sha256"], sha("\n".join(STEMS).encode("utf-8")))
        self.assertEqual(marker["missing"], ["missing_one"])
        self.assertEqual(sorted(marker["files"]), [
            "override/nested_tex.tga", "override/notxi_tex.tga",
            "override/ntb_tex.tga", "override/ntb_tex.txi"])
        self.assertEqual(marker["files"]["override/ntb_tex.tga"], sha(TGA_NTB))
        self.assertEqual((out / "override" / "ntb_tex.tga").read_bytes(), TGA_NTB)
        self.assertEqual((out / "override" / "nested_tex.tga").read_bytes(), TGA_NESTED)
        rows = (out / "overlay_manifest.tsv").read_text(encoding="utf-8").splitlines()
        self.assertEqual(rows[0], "stem\text\tsha256\tshard")
        self.assertEqual(len(rows), 5)  # header + 4 copies

    def test_symlink_list(self):
        out = self.build_ok(self.tmp / "ov")
        marker = self.read_marker(out)
        self.assertEqual(marker["symlinks"], {n: str(self.k1 / n) for n in LINKED})
        self.assertEqual(marker["skipped"], ["lips", "patch.erf"])
        for n in LINKED:
            self.assertTrue((out / n).is_symlink(), n)
            self.assertEqual(os.readlink(out / n), str(self.k1 / n))
        self.assertNotIn("override", marker["symlinks"])
        self.assertNotIn("Override", marker["symlinks"])
        self.assertFalse((out / "override").is_symlink())
        self.assertTrue((out / "override").is_dir())

    def test_copies_are_real_files(self):
        out = self.build_ok(self.tmp / "ov")
        for name in ("ntb_tex.tga", "ntb_tex.txi", "nested_tex.tga", "notxi_tex.tga"):
            p = out / "override" / name
            self.assertTrue(p.is_file(), name)
            self.assertFalse(p.is_symlink(), name)

    def test_txi_optional(self):
        out = self.build_ok(self.tmp / "ov")
        self.assertFalse((out / "override" / "notxi_tex.txi").exists())
        marker = self.read_marker(out)
        self.assertNotIn("override/notxi_tex.txi", marker["files"])
        self.assertIn("override/notxi_tex.tga", marker["files"])

    def test_stem_in_two_shards_takes_updates(self):
        old, new = b"old-dup-tga" * 8, b"new-dup-tga" * 8
        for shard, data in (("shard_a", old), ("updates", new)):
            p = self.cr / shard / "dup_tex.tga"
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_bytes(data)
        with open(self.cr / "manifest.tsv", "a", encoding="utf-8") as f:  # shard_a is listed first
            f.write(f"dup_tex\ttga\t{len(old)}\t{sha(old)}\tshard_a\n")
            f.write(f"dup_tex\ttga\t{len(new)}\t{sha(new)}\tupdates\n")
        sel = self.tmp / "sel_dup.tsv"
        sel.write_text("stem\ndup_tex\n", encoding="utf-8")
        out = self.tmp / "ov"
        rc, _o, err = self.run_cli(["build", "--k1", str(self.k1), "--cr", str(self.cr), "--select", str(sel),
                                    "--out", str(out)])
        self.assertEqual(rc, 0, err)
        self.assertEqual((out / "override" / "dup_tex.tga").read_bytes(), new)

    def test_missing_stem_reported_not_fatal(self):
        out = self.build_ok(self.tmp / "ov")
        self.assertEqual(self.read_marker(out)["missing"], ["missing_one"])
        stems = [r.split("\t")[0] for r in (out / "overlay_manifest.tsv").read_text(encoding="utf-8").splitlines()[1:]]
        self.assertNotIn("missing_one", stems)

    def test_refuse_inside_repo(self):
        out = g.REPO / "k1_hd_overlay_probe_should_not_exist"
        self.assert_refused(out)
        self.assertFalse(out.exists())

    def test_refuse_steamapps_common(self):
        out = self.tmp / "steamapps" / "common" / "ov"
        self.assert_refused(out)
        self.assertFalse((self.tmp / "steamapps").exists())

    def test_refuse_steam_kotor2_install(self):
        out = g.STEAM_K1 / "ov"
        self.assert_refused(out)
        self.assertFalse(out.exists())

    def test_refuse_equal_to_k1(self):
        before = sorted(os.listdir(self.k1))
        self.assert_refused(self.k1)
        self.assertEqual(sorted(os.listdir(self.k1)), before)

    def test_refuse_inside_k1(self):
        out = self.k1 / "hd"
        self.assert_refused(out)
        self.assertFalse(out.exists())

    def test_refuse_k1_inside_out_on_rebuild(self):
        (self.tmp / "K1_HD_OVERLAY.json").write_text("{}", encoding="utf-8")
        self.assert_refused(self.tmp, self.build_argv(self.tmp, "--rebuild"))
        self.assertTrue((self.k1 / "chitin.key").is_file())

    def test_nonempty_unmarked_out_refused(self):
        out = self.tmp / "ov"
        out.mkdir()
        (out / "keep.txt").write_text("x", encoding="utf-8")
        self.assert_refused(out)
        self.assertTrue((out / "keep.txt").exists())

    def test_rebuild_refuses_unmarked_dir(self):
        out = self.tmp / "ov"
        out.mkdir()
        (out / "keep.txt").write_text("x", encoding="utf-8")
        self.assert_refused(out, self.build_argv(out, "--rebuild"))
        self.assertTrue((out / "keep.txt").exists())

    def test_rebuild_replaces_marked_overlay(self):
        out = self.build_ok(self.tmp / "ov")
        (out / "override" / "stale.tga").write_bytes(b"stale")
        rc, _o, err = self.run_cli(self.build_argv(out, "--rebuild"))
        self.assertEqual(rc, 0, err)
        self.assertFalse((out / "override" / "stale.tga").exists())
        # rmtree must not follow the symlinks back into the K1 install
        self.assertTrue((self.k1 / "modules" / "m.mod").is_file())
        self.assertTrue((self.k1 / "chitin.key").is_file())
        rc, o, _e = self.run_cli(["check", "--out", str(out)])
        self.assertEqual(rc, 0, o)

    def test_check_ok(self):
        out = self.build_ok(self.tmp / "ov")
        rc, o, _e = self.run_cli(["check", "--out", str(out)])
        self.assertEqual(rc, 0, o)
        self.assertEqual(o.strip(), "OK n_files=4 n_links=8")

    def test_check_detects_modified_copy(self):
        out = self.build_ok(self.tmp / "ov")
        (out / "override" / "ntb_tex.tga").write_bytes(b"tampered")
        rc, o, _e = self.run_cli(["check", "--out", str(out)])
        self.assertEqual(rc, 1)
        self.assertIn("modified copy: override/ntb_tex.tga", o)

    def test_check_detects_swapped_symlink(self):
        out = self.build_ok(self.tmp / "ov")
        other = self.tmp / "other"
        other.mkdir()
        os.remove(out / "modules")
        os.symlink(str(other), out / "modules")
        rc, o, _e = self.run_cli(["check", "--out", str(out)])
        self.assertEqual(rc, 1)
        self.assertIn("swapped symlink: modules", o)

    def test_check_detects_extra_file_and_symlink_in_override(self):
        out = self.build_ok(self.tmp / "ov")
        (out / "override" / "extra.tga").write_bytes(b"x")
        os.symlink(str(self.k1 / "chitin.key"), out / "override" / "evil.tga")
        rc, o, _e = self.run_cli(["check", "--out", str(out)])
        self.assertEqual(rc, 1)
        self.assertIn("file not in marker: override/extra.tga", o)
        self.assertIn("symlink in override/: override/evil.tga", o)

    def test_check_without_marker_fails(self):
        out = self.tmp / "empty"
        out.mkdir()
        rc, o, _e = self.run_cli(["check", "--out", str(out)])
        self.assertEqual(rc, 1)
        self.assertIn("no valid K1_HD_OVERLAY.json", o)

    def test_sha_mismatch_fails_build(self):
        manifest = self.cr / "manifest.tsv"
        manifest.write_text(manifest.read_text(encoding="utf-8").replace(sha(TGA_NTB), "0" * 64),
                            encoding="utf-8")
        rc, _o, err = self.run_cli(self.build_argv(self.tmp / "ov"))
        self.assertEqual(rc, 1)
        self.assertIn("sha256 mismatch for ntb_tex.tga", err)


if __name__ == "__main__":
    unittest.main()
