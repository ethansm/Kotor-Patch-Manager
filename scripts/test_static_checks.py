#!/usr/bin/env python3
"""Tests for scripts/_static_common.py and scripts/check_hook_bytes.py (doc 20 Phase 1).

Needs neither the real exe nor the KPM tree: PE images are built in memory, loaders read the tiny fixture tree in
scripts/testdata/static/. Disassembly cases need i686-w64-mingw32-objdump and skip with a message without it.
"""
import io
import shutil
import struct
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import _static_common as sc  # noqa: E402
import check_hook_bytes as chb  # noqa: E402

FIX = HERE / "testdata" / "static"
HAVE_OBJDUMP = shutil.which(sc.OBJDUMP) is not None
needs_objdump = unittest.skipUnless(HAVE_OBJDUMP, f"{sc.OBJDUMP} not installed")

BASE = 0x400000
TEXT_VA, RDATA_VA = 0x401000, 0x402000


def build_pe(sections, exports=None, base=BASE):
    """A minimal PE32 image. sections: [(name, rva, bytes, flags)]; exports: [name] -> an .edata section is added."""
    sections = list(sections)
    if exports is not None:
        rva = 0x3000
        n = len(exports)
        funcs_off, names_off, ords_off = 40, 40 + 4 * n, 40 + 8 * n
        strs_off = ords_off + 2 * n
        blob = bytearray(strs_off)
        offs, cur = [], strs_off
        for e in exports:
            offs.append(cur)
            cur += len(e) + 1
        struct.pack_into("<IIHHIIIIIII", blob, 0, 0, 0, 0, 0, 0, 1, n, n, rva + funcs_off, rva + names_off, rva + ords_off)
        for i, o in enumerate(offs):
            struct.pack_into("<I", blob, funcs_off + 4 * i, 0x1000)
            struct.pack_into("<I", blob, names_off + 4 * i, rva + o)
            struct.pack_into("<H", blob, ords_off + 2 * i, i)
        for e in exports:
            blob += e.encode() + b"\0"
        sections.append((".edata", rva, bytes(blob), 0x40000040))
    nsec = len(sections)
    hdr_size = 0x40 + 4 + 20 + 224 + 40 * nsec
    raw = (hdr_size + 0x1FF) & ~0x1FF
    img = bytearray(raw)
    img[0:2] = b"MZ"
    struct.pack_into("<I", img, 0x3C, 0x40)
    img[0x40:0x44] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", img, 0x44, 0x14C, nsec, 0, 0, 0, 224, 0x2102)
    opt = 0x40 + 24
    struct.pack_into("<H", img, opt, 0x10B)
    struct.pack_into("<I", img, opt + 28, base)
    struct.pack_into("<I", img, opt + 92, 16)
    if exports is not None:
        struct.pack_into("<II", img, opt + 96, 0x3000, 40)
    sh = opt + 224
    for i, (name, rva, data, flags) in enumerate(sections):
        padded = bytes(data) + b"\0" * (-len(data) % 0x200)
        o = sh + 40 * i
        img[o:o + 8] = name.encode().ljust(8, b"\0")
        struct.pack_into("<IIII", img, o + 8, len(data), rva, len(padded), len(img))
        struct.pack_into("<I", img, o + 36, flags)
        img += padded
    return bytes(img)


CODE_FLAGS, DATA_FLAGS = 0x60000020, 0x40000040
# .text: +0x00 call rel32, +0x10 call eax + nops, +0x20 mov eax,imm32 + nops, +0x30 jne +5 + nops, +0x40 nops
TEXT = bytearray(b"\x90" * 0x80)
TEXT[0x00:0x05] = bytes.fromhex("E800000000")
TEXT[0x10:0x15] = bytes.fromhex("FFD0909090")
TEXT[0x20:0x25] = bytes.fromhex("B811223344")
TEXT[0x30:0x32] = bytes.fromhex("7505")
RDATA = bytes.fromhex("000000000000F03F") + b"\0" * 0x38
EXE = build_pe([(".text", 0x1000, bytes(TEXT), CODE_FLAGS), (".rdata", 0x2000, RDATA, DATA_FLAGS)])


def pe():
    return sc.PeImage(data=EXE)


def site(address, orig, kind="code", type_="code_patch", skip=False, repl=None):
    rep = sc.Reporter(out=None)
    ok = chb.check_site(rep, pe(), "t", address, bytes.fromhex(orig.replace(" ", "")), kind, type_, skip,
                        bytes.fromhex(repl.replace(" ", "")) if repl else None)
    return rep, ok


def sevs(rep, check=None):
    return [f.sev for f in rep.findings if check is None or f.check == check]


class PeImageTests(unittest.TestCase):
    def test_sections_and_reads(self):
        p = pe()
        self.assertEqual(p.image_base, BASE)
        self.assertEqual([s.name for s in p.sections], [".text", ".rdata"])
        self.assertEqual(p.section_of(TEXT_VA + 0x10).name, ".text")
        self.assertEqual(p.section_of(RDATA_VA).name, ".rdata")
        self.assertIsNone(p.section_of(0x500000))
        self.assertTrue(p.section_of(TEXT_VA).is_code)
        self.assertFalse(p.section_of(RDATA_VA).is_code)
        self.assertEqual(p.read_va(TEXT_VA + 0x10, 2), b"\xff\xd0")
        with self.assertRaises(ValueError):
            p.read_va(0x500000, 4)

    def test_exports_in_memory(self):
        dll = build_pe([(".text", 0x1000, b"\xc3" * 16, CODE_FLAGS)], exports=["AlphaHook", "BetaHook", "TestSeam"])
        self.assertEqual(sc.PeImage(data=dll).exports(), ["AlphaHook", "BetaHook", "TestSeam"])
        self.assertEqual(pe().exports(), [])

    def test_exports_from_kpatch_zip(self):
        dll = build_pe([(".text", 0x1000, b"\xc3" * 16, CODE_FLAGS)], exports=["AlphaHook"])
        with tempfile.TemporaryDirectory() as d:
            kp = Path(d) / "x.kpatch"
            with zipfile.ZipFile(kp, "w") as z:
                z.writestr("manifest.toml", '[patch]\nid = "alpha"\nversion = "1.0.0"\n')
                z.writestr("kotor2-steam-aspyr.hooks.toml", "[metadata]\n")
                z.writestr("binaries/windows_x86.dll", dll)
            km = sc.kpatch_members(kp)
            self.assertEqual(km.manifest_doc["id"], "alpha")
            self.assertEqual(sc.PeImage(data=km.dll).exports(), ["AlphaHook"])

    def test_not_a_pe(self):
        with self.assertRaises(ValueError):
            sc.PeImage(data=b"not a pe file at all" * 8)


class LoaderTests(unittest.TestCase):
    def test_load_mods_skips_and_variants(self):
        mods = sc.load_mods(FIX / "repo", kpm_dir=None)
        self.assertEqual([m.id for m in mods], ["alpha"])          # retired/ and testdata/ dirs skipped
        self.assertEqual(mods[0].version, "1.0.0")                   # primary = the plain version, not 1.0.1-probe
        both = sc.load_mods(FIX / "repo", kpm_dir=None, variants=True)
        self.assertEqual(sorted(m.version for m in both), ["1.0.0", "1.0.1-probe"])

    def test_mod_contents(self):
        m = sc.load_mods(FIX / "repo", kpm_dir=None)[0]
        self.assertEqual(m.target_versions, ["AAAA", "BBBB"])
        self.assertEqual(m.exports, ["AlphaHook", "AlphaTestSeam"])
        self.assertIsNone(m.kpm_dir)
        d, s = m.hooks
        self.assertEqual((d.address, d.type, d.function, d.skip_original, d.orig), (0x401000, "detour", "AlphaHook", True, b"\xe8\0\0\0\0"))
        self.assertEqual((s.address, s.type, s.orig, s.replacement), (0x401020, "simple", b"\x90\x90", b"\xeb\x00"))
        site_, const = m.code_patches
        self.assertEqual((site_.name, site_.address, site_.orig, site_.kind, site_.verifies_original, site_.mod),
                         ("site", 0x401040, b"\x74\x06", "code", True, "alpha"))
        self.assertEqual((const.kind, const.verifies_original, len(const.orig)), ("data", False, 8))

    def test_kpm_dir_matches_by_id_and_version(self):
        with tempfile.TemporaryDirectory() as d:
            for name, ver in (("AlphaProbe", "1.0.1-probe"), ("Alpha", "1.0.0")):
                p = Path(d) / "Patches" / name
                p.mkdir(parents=True)
                (p / "manifest.toml").write_text(f'[patch]\nid = "alpha"\nversion = "{ver}"\n')
            mods = sc.load_mods(FIX / "repo", kpm_dir=d, variants=True)
            self.assertEqual({m.version: m.kpm_dir.name for m in mods}, {"1.0.0": "Alpha", "1.0.1-probe": "AlphaProbe"})

    def test_live_config(self):
        live = sc.load_live_config(FIX / "live_config.toml")
        self.assertEqual(list(live), ["alpha", "dll-only"])
        self.assertEqual(live["dll-only"], [])
        h = live["alpha"][0]
        self.assertEqual((h.address, h.orig, h.function, h.mod), (0x401000, b"\xe8\0\0\0\0", "AlphaHook", "alpha"))
        self.assertEqual(sc.live_target_sha(FIX / "live_config.toml"), "6A52")
        self.assertIsNone(sc.live_target_sha(FIX / "nope.toml"))

    def test_code_patches_errors_and_absence(self):
        self.assertEqual(sc.load_code_patches(FIX / "repo"), [])
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "code_patches.toml"
            p.write_text('[[code_patches]]\nname = "x"\naddress = 1\n')
            with self.assertRaises(ValueError):
                sc.load_code_patches(d)
            p.write_text('[[code_patches]]\nname = "x"\naddress = 1\noriginal_bytes = [1]\nkind = "weird"\n')
            with self.assertRaises(ValueError):
                sc.load_code_patches(p)

    def test_parse_def(self):
        self.assertEqual(sc.parse_def("LIBRARY X\nEXPORTS\n  A\n  B @3 ; c\n  C=D\n"), ["A", "B", "C"])

    def test_reporter(self):
        out = io.StringIO()
        r = sc.Reporter(quiet=True, out=out)
        r.passed("a", "x"); r.info("b", "y"); r.warn("c", "z")
        self.assertEqual(out.getvalue(), "WARN c: z\n")
        self.assertEqual(r.exit_code(), 0)
        r.fail("d", "w")
        self.assertEqual(r.exit_code(), 1)
        self.assertEqual(r.summary(), "FAIL=1 WARN=1 INFO=1 PASS=1 SKIP=0")

    def test_require_version(self):
        with tempfile.TemporaryDirectory() as d:
            exe = Path(d) / "x.exe"
            r = sc.Reporter(out=None)
            self.assertFalse(sc.require_version(exe, "AA", r))
            self.assertEqual(sevs(r), ["SKIP"])
            exe.write_bytes(b"abc")
            sha = sc.sha256_file(exe)
            r = sc.Reporter(out=None)
            self.assertTrue(sc.require_version(exe, sha.lower(), r))
            self.assertFalse(sc.require_version(exe, "00" * 32, r))
            self.assertEqual(sevs(r), ["FAIL"])


@needs_objdump
class DisasmTests(unittest.TestCase):
    def test_disasm(self):
        ins = sc.disasm(bytes.fromhex("E800000000FFD07505C7820401000000000000F3A490"), 0x8A7E40)
        self.assertEqual([(i.va, i.length, i.mnemonic) for i in ins],
                         [(0x8A7E40, 5, "call"), (0x8A7E45, 2, "call"), (0x8A7E47, 2, "jne"),
                          (0x8A7E49, 10, "movl"), (0x8A7E53, 2, "rep"), (0x8A7E55, 1, "nop")])
        self.assertEqual(sc.branch_target(ins[0]), 0x8A7E45)
        self.assertIsNone(sc.branch_target(ins[1]))                  # call *%eax
        self.assertEqual(sc.branch_target(ins[2]), 0x8A7E4E)
        self.assertIsNone(sc.branch_target(ins[3]))

    def test_indirect_memory_forms_are_not_branches(self):
        ins = sc.disasm(bytes.fromhex("FF1500104000FF2485001040009090"), 0x401000)   # call [abs], jmp [abs+eax*4]
        self.assertEqual([sc.branch_target(i) for i in ins[:2]], [None, None])


@needs_objdump
class HookBytesTests(unittest.TestCase):
    def test_clean_site_passes(self):
        rep, ok = site(TEXT_VA + 0x10, "FF D0 90 90 90", type_="detour")
        self.assertTrue(ok)
        self.assertEqual(sevs(rep), ["PASS"])

    def test_rel_call_in_stolen_bytes_skip_false_fails(self):
        rep, ok = site(TEXT_VA, "E8 00 00 00 00", type_="detour", skip=False)
        self.assertFalse(ok)
        self.assertEqual(sevs(rep, "rel-branch"), ["FAIL"])

    def test_rel_call_in_stolen_bytes_skip_true_is_info(self):
        rep, ok = site(TEXT_VA, "E8 00 00 00 00", type_="detour", skip=True)
        self.assertTrue(ok)
        self.assertEqual(sevs(rep, "rel-branch"), ["INFO"])
        self.assertNotIn("FAIL", sevs(rep))

    def test_jcc_in_stolen_bytes(self):
        rep, ok = site(TEXT_VA + 0x30, "75 05 90 90 90", type_="detour", skip=False)
        self.assertFalse(ok)
        self.assertEqual(sevs(rep, "rel-branch"), ["FAIL"])

    def test_call_eax_is_not_flagged(self):
        rep, _ = site(TEXT_VA + 0x10, "FF D0 90 90 90", type_="detour", skip=False)
        self.assertEqual(sevs(rep, "rel-branch"), [])

    def test_code_patch_and_simple_hook_do_not_judge_branches(self):
        for t in ("code_patch", "simple"):
            rep, ok = site(TEXT_VA, "E8 00 00 00 00", type_=t, repl="E8 00 00 00 00")
            self.assertTrue(ok, t)
            self.assertEqual(sevs(rep, "rel-branch"), [])

    def test_window_ending_mid_instruction_fails(self):
        rep, ok = site(TEXT_VA + 0x20, "B8 11 22 33", type_="detour")      # mov eax,imm32 cut after 4 bytes
        self.assertFalse(ok)
        self.assertEqual(sevs(rep, "boundary"), ["FAIL"])
        rep, ok = site(TEXT_VA + 0x20, "B8 11 22 33 44", type_="detour")
        self.assertTrue(ok)

    def test_byte_mismatch_fails(self):
        rep, ok = site(TEXT_VA + 0x10, "FF D1 90 90 90", type_="detour")
        self.assertFalse(ok)
        self.assertEqual(sevs(rep, "bytes"), ["FAIL"])

    def test_unmapped_address_fails(self):
        rep, ok = site(0x500000, "90")
        self.assertFalse(ok)
        self.assertEqual(sevs(rep, "bytes"), ["FAIL"])

    def test_simple_hook_replacement_length(self):
        rep, ok = site(TEXT_VA + 0x10, "FF D0 90 90 90", type_="simple", repl="E9 00 00 00")
        self.assertFalse(ok)
        self.assertEqual(sevs(rep, "simple-len"), ["FAIL"])
        rep, ok = site(TEXT_VA + 0x10, "FF D0 90 90 90", type_="simple", repl="E9 00 00 00 00")
        self.assertTrue(ok)

    def test_sections(self):
        data = "00 00 00 00 00 00 F0 3F"
        rep, ok = site(RDATA_VA, data, kind="data")                       # data patch: no boundary/branch checks
        self.assertTrue(ok)
        rep, ok = site(RDATA_VA, data, kind="code")                       # code patch in .rdata
        self.assertFalse(ok)
        self.assertEqual(sevs(rep, "section"), ["FAIL"])
        rep, ok = site(TEXT_VA + 0x10, "FF D0", kind="data")              # data patch in .text
        self.assertFalse(ok)
        self.assertEqual(sevs(rep, "section"), ["FAIL"])


@needs_objdump
class RunTests(unittest.TestCase):
    """run() end to end on the synthetic exe and hand-built Mods."""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.exe = Path(self.tmp.name) / "swkotor2.exe"
        self.exe.write_bytes(EXE)
        self.sha = sc.sha256_file(self.exe)

    def tearDown(self):
        self.tmp.cleanup()

    def mod(self, hooks=(), cps=(), mid="m", version="1.0.0", **kw):
        return sc.Mod(id=mid, version=version, dir=Path(self.tmp.name), kpm_dir=None, hooks=list(hooks), target_versions=[],
                      exports=[], code_patches=list(cps), **kw)

    def run_(self, mods, **kw):
        return chb.run(exe=self.exe, mods=mods, live_config=None, expected_sha=self.sha, **kw)

    def hook(self, off, orig, type_="detour", skip=False, repl=None, mod="m"):
        return sc.Hook(TEXT_VA + off, type_, bytes.fromhex(orig.replace(" ", "")), "F" if type_ == "detour" else None, skip,
                       bytes.fromhex(repl.replace(" ", "")) if repl else None, mod)

    def test_counts_and_fail(self):
        f = self.run_([self.mod([self.hook(0x10, "FF D0 90 90 90"), self.hook(0x00, "E8 00 00 00 00", skip=True)],
                                [sc.CodePatch("c", RDATA_VA, bytes.fromhex("000000000000F03F"), "data", True, mod="m")])])
        self.assertEqual([x.sev for x in f].count("FAIL"), 0)
        self.assertEqual([x.sev for x in f].count("INFO"), 2)    # scope line + the skip=true rel call
        self.assertEqual([x.sev for x in f].count("PASS"), 3)
        f = self.run_([self.mod([self.hook(0x00, "E8 00 00 00 00", skip=False)])])
        self.assertEqual([x.check for x in f if x.sev == "FAIL"], ["rel-branch"])

    def test_wrong_exe_version_fails_and_missing_exe_skips(self):
        f = chb.run(exe=self.exe, mods=[self.mod()], live_config=None, expected_sha="00" * 32)
        self.assertEqual([x.sev for x in f], ["FAIL"])
        f = chb.run(exe=Path(self.tmp.name) / "none.exe", mods=[self.mod()], live_config=None)
        self.assertEqual([x.sev for x in f], ["SKIP"])

    def test_mod_filter(self):
        bad = self.mod([self.hook(0x10, "FF D1 90 90 90", mod="bad")], mid="bad")
        good = self.mod([self.hook(0x10, "FF D0 90 90 90")], mid="good")
        self.assertFalse([x for x in self.run_([bad, good], mod_ids={"good"}) if x.sev == "FAIL"])
        self.assertTrue([x for x in self.run_([bad, good], mod_ids={"bad"}) if x.sev == "FAIL"])

    def kpatch(self, members, name="s.kpatch"):
        p = Path(self.tmp.name) / name
        with zipfile.ZipFile(p, "w") as z:
            for k, v in members.items():
                z.writestr(k, v)
        return p

    def test_kpatch_packaging(self):
        src = self.mod([self.hook(0x10, "FF D0 90 90 90")], mid="alpha", has_def=True)
        manifest = '[patch]\nid = "alpha"\nversion = "1.0.0"\n'
        hooks_ok = ('[metadata]\ntarget_versions = ["%s", "EXTRA"]\n[[hooks]]\naddress = %d\ntype = "detour"\nfunction = "F"\n'
                    'original_bytes = [255, 208, 144, 144, 144]\nskip_original_bytes = false\n' % (self.sha, TEXT_VA + 0x10))
        dll = build_pe([(".text", 0x1000, b"\xc3" * 16, CODE_FLAGS)], exports=["F"])
        full = {"manifest.toml": manifest, "kotor2-steam-aspyr.hooks.toml": hooks_ok, "binaries/windows_x86.dll": dll}
        # complete and equal (extra target_versions entry and comments are fine)
        f = self.run_([src], kpatches=[self.kpatch(full)])
        self.assertFalse([x for x in f if x.sev in ("FAIL", "WARN")], f)
        # missing DLL
        no_dll = {k: v for k, v in full.items() if k != "binaries/windows_x86.dll"}
        f = self.run_([src], kpatches=[self.kpatch(no_dll)])
        self.assertEqual([x.check for x in f if x.sev == "FAIL"], ["packaging"])
        # stale hooks toml: different hook set -> WARN
        stale = dict(full, **{"kotor2-steam-aspyr.hooks.toml": "[metadata]\n"})
        f = self.run_([src], kpatches=[self.kpatch(stale)])
        self.assertEqual([x.check for x in f if x.sev == "WARN"], ["stale-kpatch"])
        self.assertFalse([x for x in f if x.sev == "FAIL"])
        # staged hooks are byte-checked: a wrong original_bytes inside the .kpatch is a FAIL
        bad = dict(full, **{"kotor2-steam-aspyr.hooks.toml": hooks_ok.replace("255, 208", "255, 209")})
        f = self.run_([src], kpatches=[self.kpatch(bad)])
        self.assertIn("bytes", [x.check for x in f if x.sev == "FAIL"])
        # DLL-less mod (no exports.def, only a simple hook) needs no DLL
        simple = self.mod([self.hook(0x10, "FF D0 90 90 90", "simple", repl="FF D0 90 90 91")], mid="alpha")
        hooks_s = hooks_ok.replace('"detour"\nfunction = "F"', '"simple"').replace("skip_original_bytes = false",
                                                                                 "replacement_bytes = [255, 208, 144, 144, 145]")
        f = self.run_([simple], kpatches=[self.kpatch({"manifest.toml": manifest, "kotor2-steam-aspyr.hooks.toml": hooks_s})])
        self.assertFalse([x for x in f if x.sev in ("FAIL", "WARN")], f)


if __name__ == "__main__":
    unittest.main(verbosity=1)
