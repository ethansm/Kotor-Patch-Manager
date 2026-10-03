"""Unit tests for re_card_prep.py - synthetic dump fixture built in a temp dir (no real dump needed)."""
import contextlib
import io
import os
import shutil
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import re_card_prep as rp  # noqa: E402

C_MAIN = """// FN 005f2fe0 FUN_005f2fe0 size=64 ret=8 callers= callees=00522af0,00000024,00919723

/* Library Function - Single Match */
void __thiscall
FUN_005f2fe0(int *param_1,int param_2,undefined2 *param_3)

{
  int local_1c [8];
  undefined4 *puVar1;
  char *pcVar2;
  int iVar3;

  *(short *)(param_1 + 100) = 1;
  iVar3 = *(int *)((int)param_1 + 100);
  *(int *)(param_1 + 0x1c) = param_2 + 3;
  puVar1[100] = local_1c[0x19];
  pcVar2[100] = 'a';
  param_3[100] = 0;
  fVar = 1.5 + 100.0;
  fVar = 2e+10 - 5;
  FUN_00522af0("x + 100", param_1 + 12); /* a + 100 */
  // b + 100
  _atexit(FUN_00522af0);
  FID_conflict:_free(param_1);
  iVar3 = iVar3 - 25;
  iVar3 = iVar3 - 7;
  return;
}
"""

ASM_MAIN = """; FN 005f2fe0 FUN_005f2fe0
005f2fe0  55                    PUSH EBP
005f2fe1  8bec                  MOV EBP,ESP
005f2fe3  6aff                  PUSH -0x1
005f2fe5  688b459500            PUSH 0x95458b
005f2fea  8b4130                MOV EAX,dword ptr [ECX + 0x30]
005f2fed  8b5130                MOV EDX,dword ptr [ECX + 0x30]
005f2ff0  8b4104                MOV EAX,dword ptr [ECX + 0x4]
005f2ff3  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
005f2ff6  68d4689800            PUSH 0x009868d4
005f2ffb  e8f0fffeff            CALL 0x00522af0
005f3000  e8f0fffeff            CALL 0x00919723
005f3005  c20800                RET 0x8
"""

C_THUNK = """// FN 009196fd FUN_009196fd size=11 ret=-1 callers=1 callees=0091c6b5

void FUN_009196fd(void *param_1)

{
  _free(param_1);
  return;
}
"""
ASM_THUNK = """; FN 009196fd FUN_009196fd
009196fd  8bff                  MOV EDI,EDI
009196ff  55                    PUSH EBP
00919700  8bec                  MOV EBP,ESP
00919702  5d                    POP EBP
00919703  e9ad2f0000            JMP 0x0091c6b5
"""

C_RET_FWD = """// FN 00401020 FUN_00401020 size=10 ret=4 callers= callees=00401500

undefined4 __thiscall FUN_00401020(void *this,int param_1)

{
  undefined4 uVar1;

  uVar1 = FUN_00401500(param_1);
  return uVar1;
}
"""
C_RET_FWD2 = C_RET_FWD.replace("  uVar1 = FUN_00401500(param_1);\n  return uVar1;", "  return FUN_00401500(param_1);")
C_NOT_THUNK = """// FN 00401040 FUN_00401040 size=10 ret=0 callers= callees=00401500,00401600

void FUN_00401040(void)

{
  FUN_00401500();
  FUN_00401600();
  return;
}
"""
ASM_BOOL = """; FN 00401060 FUN_00401060
00401060  56                    PUSH ESI
00401061  8bf1                  MOV ESI,ECX
00401063  8a4604                MOV AL,byte ptr [ESI + 0x4]
00401066  c20400                RET 0x4
"""
C_BOOL = "// FN 00401060 FUN_00401060 size=7 ret=4 callers= callees=\n\nvoid __thiscall FUN_00401060(void *this,bool param_1)\n\n{\n  return;\n}\n"

GOG_TSV = (
    "gog_addr\tclass\tfunction\tcallconv\tparam_size_bytes\tnotes\n"
    "005a58a0\tCFoo\tSetFlag\t\t1\tundefined1 param_1\n"
    "005a58b0\tCFoo\tSetCdecl\t__cdecl\t8\ta,b\n"
    "005a58c0\tCDup\tGo\t__thiscall\t4\tx\n"
    "005a58d0\tCDup\tGo\t__thiscall\t8\ty\n"
)
NAME_TSV = (
    "steam_addr\tname\tsource\tconfidence\tevidence\n"
    "0x00522af0\tCSWSArea::GetFirst\tx\thigh\te\n"
    "0x00919723\toperator new\tx\thigh\te\n"
)
STR_TSV = "addr\tstring\tref_count\tfunctions\n009868d4\tmalmov12\t1\t005f2fe0:FUN_005f2fe0\n"


def write(root, rel, text):
    p = os.path.join(root, rel)
    os.makedirs(os.path.dirname(p), exist_ok=True)
    with open(p, "w") as f:
        f.write(text)


def run(*argv):
    buf, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(buf), contextlib.redirect_stderr(err):
        try:
            rc = rp.main(list(argv))
        except SystemExit as e:
            rc = e.code
    return rc, buf.getvalue(), err.getvalue()


class Fixture(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.root = tempfile.mkdtemp(prefix="re_card_prep_")
        for addr, c, a in (("005f2fe0", C_MAIN, ASM_MAIN), ("009196fd", C_THUNK, ASM_THUNK),
                           ("00401060", C_BOOL, ASM_BOOL)):
            write(cls.root, "decomp/%s/%s.c" % (addr[2:4], addr), c)
            write(cls.root, "decomp/%s/%s.asm" % (addr[2:4], addr), a)
        write(cls.root, "decomp/40/00401020.c", C_RET_FWD)
        write(cls.root, "decomp/40/00401040.c", C_NOT_THUNK)
        write(cls.root, "inputs/gog_functions.tsv", GOG_TSV)
        write(cls.root, "inputs/name_map.tsv", NAME_TSV)
        write(cls.root, "strings.tsv", STR_TSV)

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.root, ignore_errors=True)


class AnnotateTests(Fixture):
    def setUp(self):
        rc, self.out, _ = run("annotate", "0x5F2FE0", "--root", self.root)
        self.assertEqual(rc, 0)
        self.lines = self.out.split("\n")

    def line_with(self, needle):
        hits = [l for l in self.lines if needle in l]
        self.assertTrue(hits, "no line with %r in:\n%s" % (needle, self.out))
        return hits[0]

    def test_header_counts(self):
        self.assertEqual(self.lines[0], "// FN 005f2fe0 FUN_005f2fe0 size=64 ret=8 callers=0 callees=3")

    def test_int_ptr_offset_hex_and_byte_note(self):
        l = self.line_with("*(short *)")
        self.assertIn("(param_1 + 0x64 [dec 100] {int*: byte +0x190})", l)

    def test_cast_to_int_is_byte_arithmetic_no_scaling(self):
        l = self.line_with("(int)param_1")
        self.assertIn("(int)param_1 + 0x64 [dec 100]", l)
        self.assertNotIn("byte", l)

    def test_existing_hex_untouched(self):
        l = self.line_with("0x1c")
        self.assertIn("(param_1 + 0x1c)", l)
        self.assertIn("param_2 + 3;", l)  # int param_2, small number: untouched

    def test_index_on_4byte_pointer_and_array(self):
        l = self.line_with("puVar1[")
        self.assertIn("puVar1[100 = byte +0x190]", l)
        self.assertIn("local_1c[0x19 = byte +0x64]", l)

    def test_non4byte_index_left_alone(self):
        self.assertIn("pcVar2[100] = 'a';", self.out)
        self.assertIn("param_3[100] = 0;", self.out)

    def test_float_literals_untouched(self):
        self.assertIn("1.5 + 100.0;", self.out)
        self.assertIn("2e+10 - 5;", self.out)

    def test_strings_and_comments_untouched(self):
        self.assertIn('"x + 100"', self.out)
        self.assertIn("/* a + 100 */", self.out)
        self.assertIn("// b + 100", self.out)
        l = self.line_with('"x + 100"')
        self.assertIn("param_1 + 0xc [dec 12] {int*: byte +0x30}", l)

    def test_plain_int_subtraction_hex_dec(self):
        self.assertIn("iVar3 - 0x19 [dec 25];", self.out)
        self.assertIn("iVar3 - 7;", self.out)

    def test_auto_labels_flagged(self):
        self.assertIn("_atexit" + rp.LABEL_TAG, self.out)
        self.assertIn("FID_conflict:_free" + rp.LABEL_TAG, self.out)
        self.assertIn("Library Function - Single Match */" + rp.LABEL_TAG, self.out)
        self.assertNotIn("__thiscall" + rp.LABEL_TAG, self.out)
        self.assertNotIn("local_1c" + rp.LABEL_TAG, self.out)

    def test_not_a_thunk(self):
        self.assertNotIn("THUNK", self.out)

    def test_max_lines(self):
        _, out, _ = run("annotate", "5f2fe0", "--max-lines", "5", "--root", self.root)
        self.assertIn("more lines truncated", out)
        self.assertEqual(len(out.strip().split("\n")), 1 + 5 + 1)

    def test_missing_function(self):
        rc, _, err = run("annotate", "0x123456", "--root", self.root)
        self.assertEqual(rc, 2)
        self.assertIn("no decompile", err)


class ThunkTests(Fixture):
    def test_asm_jmp_thunk(self):
        _, out, _ = run("annotate", "0x9196FD", "--root", self.root)
        self.assertIn("THUNK -> 0x0091C6B5 (Ghidra may omit __thiscall on thunks; ECX is passed through)", out)
        self.assertIn("_free" + rp.LABEL_TAG, out)

    def test_c_thunk_forms(self):
        self.assertEqual(rp.c_thunk_callee(C_RET_FWD2), "FUN_00401500")
        self.assertEqual(rp.c_thunk_callee(C_THUNK), "_free")
        self.assertEqual(rp.c_thunk_callee(C_NOT_THUNK), None)
        self.assertEqual(rp.c_thunk_callee(C_MAIN), None)

    def test_c_only_thunk_target_from_name(self):
        _, out, _ = run("annotate", "0x401020", "--root", self.root)
        self.assertIn("THUNK -> 0x00401500", out)  # assigned-then-returned variant is still one call + return
        _, out2, _ = run("annotate", "0x401040", "--root", self.root)
        self.assertNotIn("THUNK", out2)

    def test_asm_thunk_target_mov_ecx_jmp(self):
        asm = rp.parse_asm_text("00401000  8b0d00104000  MOV ECX,dword ptr [0x00401000]\n"
                                "00401006  e9f5000000  JMP 0x00401100\n")
        self.assertEqual(rp.asm_thunk_target(asm), 0x401100)
        asm2 = rp.parse_asm_text("00401000  50  PUSH EAX\n00401001  e9f5000000  JMP 0x00401100\n")
        self.assertIsNone(rp.asm_thunk_target(asm2))


class AbiTests(Fixture):
    def test_expected_ret_math(self):
        self.assertEqual(rp.expected_ret(1, "thiscall"), 4)
        self.assertEqual(rp.expected_ret(4, "stdcall"), 4)
        self.assertEqual(rp.expected_ret(5, "thiscall"), 8)
        self.assertEqual(rp.expected_ret(0, "thiscall"), 0)
        self.assertEqual(rp.expected_ret(12, "cdecl"), 0)
        self.assertEqual(rp.expected_ret(8, "fastcall"), 0)
        self.assertEqual(rp.expected_ret(16, "fastcall"), 8)
        self.assertEqual(rp.expected_ret(1, "fastcall"), 0)
        self.assertEqual(rp.expected_ret(3, "thiscall", [1, 1, 1]), 12)  # per-arg slot promotion
        self.assertEqual(rp.slots_bytes("1,4,1"), (6, [1, 4, 1]))

    def test_bool_arg_true(self):
        rc, out, _ = run("abi", "0x401060", "--gog-bytes", "1", "--root", self.root)
        self.assertEqual(rc, 0)
        self.assertIn("Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)", out)

    def test_false_when_mismatch(self):
        _, out, _ = run("abi", "0x401060", "--gog-bytes", "8", "--root", self.root)
        self.assertIn("Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 4)", out)

    def test_cdecl_expects_zero(self):
        _, out, _ = run("abi", "0x401060", "--gog-bytes", "8", "--callconv", "cdecl", "--root", self.root)
        self.assertIn("expected_ret = 0", out)
        self.assertIn("Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 0; ASM RET 4)", out)

    def test_fastcall_documented(self):
        _, out, _ = run("abi", "0x5f2fe0", "--gog-bytes", "16", "--callconv", "fastcall", "--root", self.root)
        self.assertIn("ASSUMPTION", out)
        self.assertIn("Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 8; ASM RET 8)", out)

    def test_gog_name_lookup(self):
        rc, out, _ = run("abi", "0x401060", "--gog-name", "CFoo::SetFlag", "--root", self.root)
        self.assertEqual(rc, 0)
        self.assertIn("assumed thiscall", out)
        self.assertIn("Stack Slot Match: TRUE", out)
        _, out, _ = run("abi", "0x401060", "--gog-name", "CFoo::SetCdecl", "--root", self.root)
        self.assertIn("callconv: cdecl", out)
        self.assertIn("Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 0; ASM RET 4)", out)

    def test_gog_name_errors(self):
        rc, _, err = run("abi", "0x401060", "--gog-name", "CNope::X", "--root", self.root)
        self.assertEqual(rc, 2)
        rc, _, err = run("abi", "0x401060", "--gog-name", "CDup::Go", "--root", self.root)
        self.assertEqual(rc, 2)
        self.assertIn("ambiguous", err)
        rc, out, _ = run("abi", "0x401060", "--gog-name", "CDup::Go", "--gog-addr", "5a58c0", "--root", self.root)
        self.assertEqual(rc, 0)
        self.assertIn("slots 4", out)

    def test_asm_ret_listing(self):
        asm = rp.parse_asm_text("00401000  c3  RET\n00401001  c20800  RET 0x8\n")
        self.assertEqual(rp.asm_rets(asm), [0, 8])


class AnchorsTests(Fixture):
    def setUp(self):
        rc, self.out, _ = run("anchors", "0x5F2FE0", "--root", self.root)
        self.assertEqual(rc, 0)

    def test_string(self):
        self.assertIn("0x009868D4  at 005f2ff6  'malmov12'", self.out)

    def test_callees_named_and_filtered(self):
        self.assertIn("0x00522AF0  CSWSArea::GetFirst", self.out)
        self.assertIn("0x00919723  operator new", self.out)
        self.assertNotIn("0x00000024", self.out)  # header junk below code base is dropped

    def test_struct_offsets(self):
        self.assertIn("+0x30 [dec 48]  x2  via ECX", self.out)
        self.assertIn("+0x4 [dec 4]  x1  via ECX", self.out)
        self.assertNotIn("+0xC ", self.out)

    def test_top_20_cap(self):
        lines = ["00401000  8b4100  MOV EAX,dword ptr [ECX + 0x%x]" % (0x10 + i) for i in range(30)]
        asm = rp.parse_asm_text("\n".join(lines))
        self.assertEqual(len(rp.struct_offsets(asm)), 20)


class PrologueTests(Fixture):
    def test_aligned_seh(self):
        _, out, _ = run("prologue", "0x5F2FE0", "--root", self.root)
        self.assertIn("16-byte aligned: YES", out)
        self.assertIn("standard prologue: YES", out)
        self.assertIn("SEH", out)
        self.assertIn("PUSH EBP", out)

    def test_unaligned_flagged(self):
        _, out, _ = run("prologue", "0x9196FD", "--root", self.root)
        self.assertIn("UNALIGNED", out)
        self.assertIn("1785/1796", out)

    def test_classify(self):
        self.assertTrue(rp.classify_prologue("568bf1"))
        self.assertTrue(rp.classify_prologue("e9ad2f0000"))
        self.assertTrue(rp.classify_prologue("83ec10"))
        self.assertTrue(rp.classify_prologue("81eca0020000"))
        self.assertTrue(rp.classify_prologue("53"))
        self.assertFalse(rp.classify_prologue("33c0"))


class ReadOnlyTests(Fixture):
    def test_root_not_modified(self):
        def snap():
            return sorted((d, f, os.path.getmtime(os.path.join(d, f))) for d, _, fs in os.walk(self.root) for f in fs)
        before = snap()
        for a in (["annotate", "5f2fe0"], ["abi", "401060", "--gog-bytes", "1"], ["anchors", "5f2fe0"],
                  ["prologue", "5f2fe0"]):
            run(*(a + ["--root", self.root]))
        self.assertEqual(before, snap())


if __name__ == "__main__":
    unittest.main()
