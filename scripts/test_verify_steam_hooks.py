#!/usr/bin/env python3
"""Tests for verify_steam_hooks.py. Fixtures (a tiny PE32 + fake KPM worktrees) are built in a tempdir;
nothing outside it is touched. Boundary tests are skipped when objdump is missing.

    python3 -m unittest scripts.test_verify_steam_hooks -v
"""
import contextlib
import hashlib
import io
import os
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import verify_steam_hooks as vsh  # noqa: E402

HAVE_OBJDUMP = shutil.which("objdump") is not None
BASE, TEXT_RVA, TEXT_RAW, TEXT_SIZE = 0x400000, 0x1000, 0x200, 0x200
TEXT_VA = BASE + TEXT_RVA

BLOCK1 = bytes.fromhex(
    "55 8B EC 83 EC 10 8B 45 08"      # push ebp; mov ebp,esp; sub esp,10; mov eax,[ebp+8]
    " 89 85 BC FE FF FF"               # +9  mov [ebp-0x144],eax
    " C7 45 FC 01 00 00 00"            # +15 mov dword [ebp-4],1
    " 8B 4D FC 03 C1 8B E5 5D C3")     # mov ecx,[ebp-4]; add eax,ecx; mov esp,ebp; pop ebp; ret
BLOCK2 = bytes.fromhex(
    "B8 78 56 34 12"                   # +0x40 mov eax,0x12345678
    " 8B 55 08 03 C2"                  # +0x45 mov edx,[ebp+8]; add eax,edx
    " 89 45 F8 83 C0 05 50 90 C3")     # mov [ebp-8],eax; add eax,5; push eax; nop; ret


def build_text():
    t = bytearray(b"\xCC" * TEXT_SIZE)
    t[0:len(BLOCK1)] = BLOCK1
    t[0x40:0x40 + len(BLOCK2)] = BLOCK2
    t[0x80:0x80 + len(BLOCK1)] = BLOCK1  # duplicate of block 1 -> non-unique patterns there
    return bytes(t)


def build_pe():
    """Minimal PE32: DOS hdr, PE sig, COFF, 224-byte optional header, one .text section."""
    hdr = bytearray(0x200)
    hdr[0:2] = b"MZ"
    struct.pack_into("<I", hdr, 0x3C, 0x40)
    hdr[0x40:0x44] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", hdr, 0x44, 0x14C, 1, 0, 0, 0, 224, 0x102)
    opt = 0x40 + 24
    struct.pack_into("<H", hdr, opt, 0x10B)
    struct.pack_into("<I", hdr, opt + 28, BASE)
    struct.pack_into("<I", hdr, opt + 60, 0x200)  # SizeOfHeaders
    sh = opt + 224
    hdr[sh:sh + 8] = b".text\0\0\0"
    struct.pack_into("<IIII", hdr, sh + 8, TEXT_SIZE, TEXT_RVA, TEXT_SIZE, TEXT_RAW)
    return bytes(hdr) + build_text()


def hexlist(b):
    return "[" + ", ".join(f"0x{x:02X}" for x in b) + "]"


def hook_toml(address, original, htype="detour", function="Fn", extra="", replacement=None, params=True):
    s = f"[[hooks]]\naddress = 0x{address:08X}\ntype = \"{htype}\"\n"
    if function:
        s += f"function = \"{function}\"\n"
    if original is not None:
        s += f"original_bytes = {hexlist(original)}\n"
    if replacement is not None:
        s += f"replacement_bytes = {hexlist(replacement)}\n"
    s += extra
    if htype == "detour" and params:
        s += "\n[[hooks.parameters]]\nsource = \"eax\"\ntype = \"pointer\"\n"
    return s + "\n"


def run_cli(args):
    out, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        rc = vsh.main(args)
    return rc, out.getvalue(), err.getvalue()


def parse_tsv(text):
    lines = text.strip("\n").split("\n")
    assert lines[0].split("\t") == vsh.COLUMNS, lines[0]
    return [dict(zip(vsh.COLUMNS, ln.split("\t"))) for ln in lines[1:]]


class Fixture(unittest.TestCase):
    def setUp(self):
        self.td = tempfile.TemporaryDirectory()
        self.root = Path(self.td.name)
        self.exe = self.root / "swkotor2.exe"
        self.exe.write_bytes(build_pe())
        self.sha = hashlib.sha256(self.exe.read_bytes()).hexdigest().upper()
        self.wt = self.root / "kpm"
        (self.wt / "Patches").mkdir(parents=True)

    def tearDown(self):
        self.td.cleanup()

    def make_patch(self, name, steam_hooks, gog_hooks="same", sha=None):
        d = self.wt / "Patches" / name
        d.mkdir(parents=True, exist_ok=True)
        meta = f'[metadata]\ntarget_versions = ["{sha or self.sha}"]\n\n'
        (d / vsh.STEAM_TOML).write_text(meta + steam_hooks)
        if gog_hooks == "same":
            gog_hooks = steam_hooks
        if gog_hooks is not None:
            (d / vsh.GOG_TOML).write_text('[metadata]\ntarget_versions = ["DEADBEEF"]\n\n' + gog_hooks)

    def verify(self, *extra):
        rc, out, err = run_cli([str(self.wt), "--exe", str(self.exe), *extra])
        return rc, parse_tsv(out), err

    @staticmethod
    def pick(rows, check, idx=None):
        return [r for r in rows if r["check"] == check and (idx is None or r["hook_index"] == str(idx))]


class PeTests(Fixture):
    def test_section_mapping_and_read(self):
        pe = vsh.PE(self.exe.read_bytes())
        self.assertEqual(pe.base, BASE)
        self.assertEqual(pe.read(TEXT_VA, 3), bytes.fromhex("55 8B EC"))
        self.assertEqual(pe.read(TEXT_VA + 0x40, 5), BLOCK2[:5])
        self.assertIsNone(pe.read(TEXT_VA + TEXT_SIZE - 2, 8))  # runs past raw data
        self.assertIsNone(pe.read(0x900000, 1))
        self.assertEqual(pe.text()["name"], ".text")

    def test_header_region_is_mapped(self):
        pe = vsh.PE(self.exe.read_bytes())
        self.assertEqual(pe.read(BASE, 2), b"MZ")

    def test_not_a_pe(self):
        with self.assertRaises(ValueError):
            vsh.PE(b"\x00" * 100)


class HelperTests(unittest.TestCase):
    def test_operand_like(self):
        a = [0x90] * 20
        b = list(a)
        b[5] = 0x91
        b[6] = 0x92
        self.assertTrue(vsh.operand_like(a, b))
        self.assertFalse(vsh.operand_like(a, a[:-1]))  # length differs
        c = [0x91 if i % 5 == 0 else 0x90 for i in range(20)]  # 4 separate windows spread out
        self.assertTrue(vsh.operand_like(a, c))
        d = [0x91] * 20
        # one contiguous run of 20 differing bytes = 5 windows > 4
        self.assertFalse(vsh.operand_like(a, d))

    def test_params_stack_only(self):
        s = [{"source": "ebp-0x13C", "type": "pointer"}, {"source": "const:32", "type": "uint"}]
        g = [{"source": "ebp-0xBC", "type": "pointer"}, {"source": "const:32", "type": "uint"}]
        self.assertTrue(vsh.params_stack_only(s, g))
        g2 = [{"source": "ebp-0xBC", "type": "pointer"}, {"source": "const:16", "type": "uint"}]
        self.assertFalse(vsh.params_stack_only(s, g2))
        g3 = [{"source": "ebp-0xBC", "type": "int"}, {"source": "const:32", "type": "uint"}]
        self.assertFalse(vsh.params_stack_only(s, g3))
        self.assertFalse(vsh.params_stack_only(s, g[:1]))

    def test_drift_fields_names_field_and_exempts_address(self):
        s = {"address": 1, "type": "detour", "function": "A", "original_bytes": [1], "consumed_exit_address": 5}
        g = {"address": 2, "type": "detour", "function": "B", "original_bytes": [2], "consumed_exit_address": 9}
        d = vsh.drift_fields(s, g)
        self.assertEqual([r for r, _ in d], ["FAIL"])
        self.assertTrue(d[0][1].startswith("function:"))
        del g["consumed_exit_address"]
        self.assertTrue(any("consumed_exit_address" in t for _, t in vsh.drift_fields(s, g)))

    def test_allowlist_demotes_fail(self):
        tmp = tempfile.TemporaryDirectory()
        f = Path(tmp.name) / "a.tsv"
        f.write_text("# c\nP\t1\tUNIQUE\tbecause\nQ\t*\tDRIFT\tany\n")
        rows = [("P", 1, "0x1", "-", "UNIQUE", "FAIL", "x"), ("P", 2, "0x1", "-", "UNIQUE", "FAIL", "x"),
                ("Q", 9, "0x1", "-", "DRIFT", "FAIL", "y")]
        out = vsh.apply_allowlist(rows, vsh.load_allowlist([f]))
        self.assertEqual([r[5] for r in out], ["WARN", "FAIL", "WARN"])
        self.assertIn("allowlisted: because", out[0][6])
        tmp.cleanup()


class VerifyTests(Fixture):
    def good_hook(self, extra_gog=None):
        # whole-instruction, unique, on a boundary: mov edx,[ebp+8]; add eax,edx
        return hook_toml(TEXT_VA + 0x45, BLOCK2[5:10])

    @unittest.skipUnless(HAVE_OBJDUMP, "objdump not available")
    def test_fully_matching_hook_all_pass(self):
        self.make_patch("Good", self.good_hook())
        rc, rows, _ = self.verify()
        self.assertEqual(rc, 0)
        res = {r["check"]: r["result"] for r in rows if r["hook_index"] == "0"}
        self.assertEqual(res, {"BYTES": "PASS", "BOUNDARY": "PASS", "UNIQUE": "PASS", "DRIFT": "PASS"})
        self.assertEqual(self.pick(rows, "SHA")[0]["result"], "PASS")
        self.assertEqual(rows[1]["address"], f"0x{TEXT_VA + 0x45:08X}")
        self.assertEqual(rows[1]["patch"], "Good")

    def test_byte_mismatch_fails(self):
        bad = bytearray(BLOCK2[5:10])
        bad[0] ^= 0xFF
        self.make_patch("Bad", hook_toml(TEXT_VA + 0x45, bytes(bad)))
        rc, rows, err = self.verify()
        self.assertEqual(rc, 1)
        self.assertEqual(self.pick(rows, "BYTES")[0]["result"], "FAIL")
        self.assertIn("FAIL=", err)

    def test_missing_original_bytes_is_warn_not_fail(self):
        self.make_patch("NoOrig", hook_toml(TEXT_VA + 0x45, None, htype="simple", function=None, replacement=[0x90]))
        rc, rows, _ = self.verify()
        self.assertEqual(self.pick(rows, "BYTES")[0]["result"], "WARN")
        self.assertEqual(self.pick(rows, "UNIQUE")[0]["result"], "SKIP")
        self.assertEqual(rc, 0)

    def test_non_unique_pattern_fails(self):
        # block 1 is duplicated at +0x80, so this pattern (plus 8 following bytes) occurs twice
        self.make_patch("Dup", hook_toml(TEXT_VA + 9, BLOCK1[9:15]))
        rc, rows, _ = self.verify()
        u = self.pick(rows, "UNIQUE")[0]
        self.assertEqual(u["result"], "FAIL")
        self.assertIn("2x", u["detail"])
        self.assertIn(f"0x{TEXT_VA + 0x80 + 9:08X}", u["detail"])
        self.assertEqual(rc, 1)

    def test_non_unique_allowlisted_is_warn(self):
        self.make_patch("Dup", hook_toml(TEXT_VA + 9, BLOCK1[9:15]))
        al = self.root / "allow.tsv"
        al.write_text("Dup\t0\tUNIQUE\tintentional copy\n")
        rc, rows, _ = self.verify("--allow", str(al))
        self.assertEqual(self.pick(rows, "UNIQUE")[0]["result"], "WARN")
        self.assertIn("allowlisted", self.pick(rows, "UNIQUE")[0]["detail"])
        self.assertEqual(rc, 0)

    def test_gog_field_drift_fails_with_field_named(self):
        steam = hook_toml(TEXT_VA + 0x45, BLOCK2[5:10], function="SteamFn")
        gog = hook_toml(TEXT_VA + 0x45, BLOCK2[5:10], function="OtherFn")
        self.make_patch("Drift", steam, gog)
        rc, rows, _ = self.verify()
        d = self.pick(rows, "DRIFT", 0)[0]
        self.assertEqual(d["result"], "FAIL")
        self.assertIn("function", d["detail"])
        self.assertIn("SteamFn", d["detail"])
        self.assertEqual(rc, 1)

    def test_gog_address_and_original_bytes_may_differ(self):
        steam = hook_toml(TEXT_VA + 0x45, BLOCK2[5:10])
        gog = hook_toml(0x00613863, [1, 2, 3, 4, 5])
        self.make_patch("Same", steam, gog)
        _, rows, _ = self.verify()
        self.assertEqual(self.pick(rows, "DRIFT", 0)[0]["result"], "PASS")

    def test_replacement_operand_diff_is_warn_but_structural_diff_fails(self):
        base = [0xE8, 0x10, 0x20, 0x30, 0x40, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90]
        other = list(base)
        other[1], other[2] = 0x11, 0x21
        steam = hook_toml(TEXT_VA + 0x45, BLOCK2[5:10], htype="simple", function=None, replacement=base)
        self.make_patch("Operand", steam, hook_toml(TEXT_VA + 0x45, BLOCK2[5:10], htype="simple", function=None, replacement=other))
        self.make_patch("Length", steam, hook_toml(TEXT_VA + 0x45, BLOCK2[5:10], htype="simple", function=None, replacement=base[:-1]))
        rc, rows, _ = self.verify()
        res = {r["patch"]: r["result"] for r in rows if r["check"] == "DRIFT"}
        self.assertEqual(res, {"Operand": "WARN", "Length": "FAIL"})
        self.assertEqual(rc, 1)

    def test_hook_count_mismatch_fails(self):
        h = hook_toml(TEXT_VA + 0x45, BLOCK2[5:10])
        self.make_patch("Count", h + h, h)
        rc, rows, _ = self.verify()
        self.assertTrue(any(r["check"] == "DRIFT" and r["hook_index"] == "-" and r["result"] == "FAIL"
                            and "count" in r["detail"] for r in rows))
        self.assertEqual(rc, 1)

    def test_missing_gog_file_skips_drift(self):
        self.make_patch("NoGog", self.good_hook(), gog_hooks=None)
        rc, rows, _ = self.verify()
        self.assertEqual(self.pick(rows, "DRIFT")[0]["result"], "SKIP")
        self.assertEqual(rc, 0)

    def test_legacy_named_gog_file_found_by_target_sha(self):
        # Older patches name the GOG file hooks.toml / kotor2_hooks.toml; KPM matches by target sha.
        self.make_patch("Legacy", hook_toml(TEXT_VA + 0x45, BLOCK2[5:10], function="SteamFn"), gog_hooks=None)
        d = self.wt / "Patches" / "Legacy"
        (d / "hooks.toml").write_text('[metadata]\ntarget_versions = ["9C10E045"]\n\n'
                                      + hook_toml(0x401000, [1], function="K1Fn"))
        (d / "kotor2_hooks.toml").write_text(f'[metadata]\ntarget_versions = ["{vsh.GOG_SHA.lower()}"]\n\n'
                                             + hook_toml(0x613863, [1, 2], function="GogFn"))
        rc, rows, _ = self.verify()
        drift = self.pick(rows, "DRIFT", 0)[0]
        self.assertEqual(drift["result"], "FAIL")
        self.assertIn("GogFn", drift["detail"])
        self.assertEqual(rc, 1)

    def test_sha_mismatch_fails(self):
        self.make_patch("Sha", self.good_hook(), sha="AB" * 32)
        rc, rows, _ = self.verify()
        self.assertEqual(self.pick(rows, "SHA")[0]["result"], "FAIL")
        self.assertEqual(rc, 1)

    def test_non_text_hook_skips_boundary_and_unique(self):
        self.make_patch("Hdr", hook_toml(BASE, b"MZ", htype="static", function=None, replacement=[0x4D, 0x5A]))
        rc, rows, _ = self.verify()
        self.assertEqual(self.pick(rows, "BYTES")[0]["result"], "PASS")
        self.assertEqual(self.pick(rows, "BOUNDARY")[0]["result"], "SKIP")
        self.assertEqual(self.pick(rows, "UNIQUE")[0]["result"], "SKIP")
        self.assertEqual(rc, 0)

    @unittest.skipUnless(HAVE_OBJDUMP, "objdump not available")
    def test_mid_instruction_detour_fails_boundary(self):
        va = TEXT_VA + 0x0B  # inside `mov [ebp-0x144],eax` (starts at +9)
        self.make_patch("Mid", hook_toml(va, bytes(build_text()[0x0B:0x10])))
        rc, rows, _ = self.verify()
        self.assertEqual(self.pick(rows, "BYTES")[0]["result"], "PASS")
        self.assertEqual(self.pick(rows, "BOUNDARY")[0]["result"], "FAIL")
        self.assertIn("mid-instruction", self.pick(rows, "BOUNDARY")[0]["detail"])
        self.assertEqual(rc, 1)

    @unittest.skipUnless(HAVE_OBJDUMP, "objdump not available")
    def test_mid_instruction_simple_patch_is_warn(self):
        va = TEXT_VA + 0x42  # inside `mov eax,0x12345678` (starts at +0x40); unique 2-byte original
        self.make_patch("MidSimple", hook_toml(va, bytes(build_text()[0x42:0x44]), htype="simple",
                                               function=None, replacement=[0x90, 0x90]))
        rc, rows, _ = self.verify()
        self.assertEqual(self.pick(rows, "BOUNDARY")[0]["result"], "WARN")
        self.assertEqual(rc, 0)

    @unittest.skipUnless(HAVE_OBJDUMP, "objdump not available")
    def test_detour_original_bytes_must_end_on_boundary(self):
        # starts on the boundary at +9 but covers only 5 of the instruction's 6 bytes
        self.make_patch("ShortOrig", hook_toml(TEXT_VA + 9, BLOCK1[9:14]))
        rc, rows, _ = self.verify()
        b = self.pick(rows, "BOUNDARY")[0]
        self.assertEqual(b["result"], "FAIL")
        self.assertIn("do not end", b["detail"])
        self.assertEqual(rc, 1)

    def test_patch_filter_and_missing_exe_exit_code(self):
        self.make_patch("A", self.good_hook())
        self.make_patch("B", self.good_hook())
        _, rows, _ = self.verify("--patch", "B")
        self.assertEqual({r["patch"] for r in rows}, {"B"})
        rc, _, err = run_cli([str(self.wt), "--exe", str(self.root / "nope.exe")])
        self.assertEqual(rc, 2)
        self.assertIn("error", err)

    def test_tsv_option_and_cli_subprocess_exit_codes(self):
        self.make_patch("Good", self.good_hook())
        tsv = self.root / "out.tsv"
        script = str(Path(vsh.__file__))
        (self.root / "empty.tsv").write_text("")
        r = subprocess.run([sys.executable, script, str(self.wt), "--exe", str(self.exe), "--tsv", str(tsv),
                            "--allow", str(self.root / "empty.tsv")], capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertEqual(r.stdout, "")
        self.assertIn("verify_steam_hooks:", r.stderr)
        self.assertEqual(parse_tsv(tsv.read_text())[0]["patch"], "Good")
        self.make_patch("Bad", hook_toml(TEXT_VA + 0x45, b"\x00" * 5))
        r = subprocess.run([sys.executable, script, str(self.wt), "--exe", str(self.exe)],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 1)
        self.assertTrue(r.stdout.startswith("patch\thook_index"))


if __name__ == "__main__":
    unittest.main()
