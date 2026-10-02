import os
import unittest

import helpers as h
import atlas_common as ac
import field_census as fc


def census(*argv):
    rc, out, err = h.run_main(fc, list(argv) + ["--root", h.FIXROOT, "--no-names", "--tsv"])
    rows = [l.split("\t") for l in out.strip().split("\n")[1:] if l]
    return [dict(zip(["fn", "fn_name", "addr", "bytes", "insn", "rw", "base", "prov"], r)) for r in rows]


class ClassifyTests(unittest.TestCase):
    def cls(self, insn, off=0x524):
        return fc.classify(insn, off)

    def test_write_ops(self):
        for insn in ("MOV dword ptr [EAX + 0x524],0x1f40", "FSTP dword ptr [EAX + 0x524]",
                     "FISTP dword ptr [EAX + 0x524]", "SETZ byte ptr [ESI + 0x524]", "MOVSS dword ptr [ECX + 0x524],XMM0"):
            self.assertEqual(self.cls(insn)[0], "W", insn)

    def test_rmw_ops(self):
        for insn in ("ADD dword ptr [EDX + 0x524],ECX", "INC dword ptr [ESI + 0x524]", "AND dword ptr [ESI + 0x524],0xfe",
                     "SHL dword ptr [ESI + 0x524],0x1", "XCHG dword ptr [ESI + 0x524],EAX", "XCHG EAX,dword ptr [ESI + 0x524]"):
            self.assertEqual(self.cls(insn)[0], "RW", insn)

    def test_read_ops(self):
        for insn in ("MOV EAX,dword ptr [EDX + 0x524]", "CMP dword ptr [ESI + 0x524],0x0", "TEST dword ptr [ESI + 0x524],0x1",
                     "PUSH dword ptr [EDX + 0x524]", "FLD dword ptr [EAX + 0x524]", "MOVZX EAX,byte ptr [ESI + 0x524]",
                     "ADD EAX,dword ptr [ESI + 0x524]", "FCOMP dword ptr [EAX + 0x524]", "FADD dword ptr [EAX + 0x524]",
                     "IMUL EAX,dword ptr [ESI + 0x524],0x5", "CALL dword ptr [ESI + 0x524]"):
            self.assertEqual(self.cls(insn)[0], "R", insn)

    def test_lea_is_addr(self):
        self.assertEqual(self.cls("LEA EAX,[ECX + 0x524]")[0], "ADDR")

    def test_indexed_and_base(self):
        rw, base, idx = self.cls("MOV dword ptr [EAX + EDX*0x4 + 0x524],0x0")
        self.assertEqual((rw, base, idx), ("W", "EAX", "EDX"))

    def test_exact_displacement_only(self):
        self.assertIsNone(self.cls("MOV ECX,dword ptr [ESI + 0x1524]"))
        self.assertIsNone(self.cls("MOV ECX,dword ptr [ESI + 0x52]"))
        self.assertIsNone(self.cls("MOV ECX,dword ptr [0x00000524]"))   # absolute, not a field
        self.assertIsNone(self.cls("MOV EAX,0x524"))

    def test_negative_offsets_match_both_spellings(self):
        off = 0xFFFFFFF8
        self.assertEqual(fc.classify("FSTP float ptr [EBP + -0x8]", off)[0], "W")
        self.assertEqual(fc.classify("MOV EAX,dword ptr [EBP + 0xfffffff8]", off)[0], "R")
        self.assertEqual(set(fc.prefilter_needles(off)), {"0xfffffff8]", "-0x8]"})


class FixtureCensusTests(unittest.TestCase):
    def test_fns_scan_and_rw_columns(self):
        hits = census("--offset", "0x524", "--fns", "0x402000")
        by = {h_["addr"]: h_ for h_ in hits}
        self.assertEqual(sorted(by), ["00402015", "00402025", "0040202b", "0040203b", "00402045", "00402052",
                                      "00402059", "00402066", "0040206c"])
        self.assertEqual(by["00402015"]["rw"], "W")
        self.assertEqual(by["00402025"]["rw"], "R")
        self.assertEqual(by["0040202b"]["rw"], "RW")
        self.assertEqual(by["00402045"]["rw"], "ADDR")
        self.assertEqual(by["0040206c"]["rw"], "W")
        self.assertEqual(by["00402015"]["bytes"], "c78024050000401f0000")

    def test_provenance_this_via_ebp_slot(self):
        by = {x["addr"]: x for x in census("--offset", "0x524", "--fns", "0x402000")}
        self.assertEqual(by["00402015"]["prov"], "this (via [ebp-0x194])")
        self.assertEqual(by["00402025"]["prov"], "this (via [ebp-0x194])")

    def test_provenance_field_of_and_call_result(self):
        by = {x["addr"]: x for x in census("--offset", "0x524", "--fns", "0x402000")}
        # [EAX + EDX*4 + 0x524], EAX = [ECX + 0xc], ECX = [EDX + 0x524] load result (unknown chain)
        self.assertTrue(by["0040203b"]["prov"].startswith("field 0xc of "), by["0040203b"]["prov"])
        self.assertEqual(by["00402052"]["prov"], "ret of 00403000")

    def test_provenance_entry_ecx_is_this(self):
        hits = census("--offset", "0x524", "--fns", "0x402100")
        self.assertEqual([x["rw"] for x in hits], ["W", "W", "R"])
        self.assertEqual(hits[0]["prov"], "this")           # MOV [ECX+0x524]: ECX untouched since entry
        self.assertTrue(hits[1]["prov"].startswith("unknown"))  # EAX at entry

    def test_run_together_bytes_not_a_hit_and_other_offsets(self):
        hits = census("--offset", "0x10bc", "--fns", "0x402000")
        self.assertEqual(len(hits), 1)
        self.assertEqual(hits[0]["bytes"], "c78490bc10000000000000")
        self.assertEqual(hits[0]["rw"], "W")
        self.assertEqual(hits[0]["base"], "eax")

    def test_negative_offset_cli(self):
        hits = census("--offset=-0x8", "--fns", "0x402000")
        self.assertEqual([x["rw"] for x in hits], ["W", "R"])

    def test_writers_flag(self):
        hits = census("--offset", "0x524", "--fns", "0x402000,0x402100", "--writers")
        self.assertTrue(hits and all(x["rw"] in ("W", "RW") for x in hits))

    def test_all_mode_over_fixture_root(self):
        hits = census("--offset", "0x524", "--all")
        self.assertEqual({x["fn"] for x in hits}, {"00402000", "00402100"})

    def test_subtree_mode(self):
        # 00401000 -> 00401100 (no asm: warning only) -> ... does not reach 402000
        hits = census("--offset", "0x524", "--subtree", "0x401000", "--depth", "2")
        self.assertEqual(hits, [])

    def test_text_output_has_summary(self):
        rc, out, err = h.run_main(fc, ["--offset", "0x524", "--fns", "0x402100", "--root", h.FIXROOT, "--no-names"])
        self.assertEqual(rc, 0)
        self.assertIn("3 hits in 1 functions", out)


REAL = os.path.isdir(os.path.join(h.REAL_ROOT, "decomp"))


@unittest.skipUnless(REAL, "real ROOT not present")
class FieldCensusCalibration(unittest.TestCase):
    """Calibration against the real dump (lessons 276 and 226)."""

    def real(self, *argv):
        rc, out, err = h.run_main(fc, list(argv) + ["--root", h.REAL_ROOT, "--no-names", "--tsv"])
        return [l.split("\t") for l in out.strip().split("\n")[1:] if l]

    def test_offset_0x524_writers(self):
        rows = self.real("--offset", "0x524", "--all")
        writers = {(r[0], r[2]): r[5] for r in rows if r[5] in ("W", "RW")}
        self.assertEqual(writers[("0057a560", "0057a64f")], "W")   # =8000 (0x1f40)
        self.assertEqual(writers[("0057a560", "0057a67c")], "W")   # =0
        self.assertEqual(writers[("00569320", "00569957")], "W")   # countdown store in AIUpdate
        insn = {r[2]: r[4] for r in rows}
        self.assertEqual(insn["0057a64f"], "MOV dword ptr [EAX + 0x524],0x1f40")
        self.assertEqual(insn["0057a67c"], "MOV dword ptr [EDX + 0x524],0x0")

    def test_offset_0x570_writers(self):
        rows = self.real("--offset", "0x570", "--all")
        writers = {(r[0], r[2]) for r in rows if r[5] in ("W", "RW")}
        self.assertIn(("006d7390", "006d740f"), writers)   # AIActionPhysicalAttacks prologue writes 3000
        self.assertIn(("00569320", "00569a72"), writers)   # AIUpdate sets 3000
        insn = {r[2]: r[4] for r in rows}
        self.assertEqual(insn["00569a72"], "MOV dword ptr [ECX + 0x570],0xbb8")


if __name__ == "__main__":
    unittest.main()
