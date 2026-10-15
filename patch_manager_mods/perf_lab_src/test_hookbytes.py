#!/usr/bin/env python3
"""Unit tests for hookbytes.py (run from this directory:  python3 -m unittest   or   python3 test_hookbytes.py).

hookbytes.py reads ~/Games/... at import time, so every test runs it as a subprocess with HOME pointed at a temp tree
that holds inputs/steam_functions.tsv and decomp/40/00401000.asm (one function at 0x00401000).
Covers: REL-BRANCH flags (Jcc and direct CALL/JMP yes, JMP [mem] no) and the function-end bound on the byte loop.
"""
import os
import shutil
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
HOOK = os.path.join(HERE, "hookbytes.py")
DUMP = os.path.join("Games", "KOTOR2_Mod_Workspace", "ghidra_dumps", "steam_6a522e71")
FN = 0x00401000


class HookBytes(unittest.TestCase):
    def setUp(self):
        self.home = tempfile.mkdtemp(prefix="hookbytes_home_")
        self.addCleanup(shutil.rmtree, self.home, True)
        root = os.path.join(self.home, DUMP)
        os.makedirs(os.path.join(root, "inputs"))
        os.makedirs(os.path.join(root, "decomp", "40"))
        with open(os.path.join(root, "inputs", "steam_functions.tsv"), "w") as f:
            f.write("addr\tname\n%08x\tfn_test\n" % FN)
        self.asm = os.path.join(root, "decomp", "40", "%08x.asm" % FN)

    def hook(self, asm_rows, *args):
        # asm_rows are "ADDR BYTES TEXT" lines of the test function; the dump's first line is a header.
        with open(self.asm, "w") as f:
            f.write("addr bytes text\n" + "".join(row + "\n" for row in asm_rows))
        env = dict(os.environ, HOME=self.home)
        return subprocess.run([sys.executable, HOOK, *args], env=env, capture_output=True, text=True)

    def test_jmp_mem_is_not_rel_branch(self):
        # JMP [mem] is an indirect jump, not a relative branch; the old bare startswith('J') clause flagged it.
        r = self.hook(["00401000 55 PUSH EBP", "00401001 8bec MOV EBP,ESP",
                       "00401003 ff2585101000 JMP dword ptr [0x00101085]"], "%08x" % FN)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertIn("558becff2585101000", r.stdout)
        self.assertNotIn("REL-BRANCH-IN-STOLEN", r.stdout)
        self.assertNotIn("FN-END-SHORT", r.stdout)

    def test_jcc_is_rel_branch(self):
        r = self.hook(["00401000 55 PUSH EBP", "00401001 8bec MOV EBP,ESP",
                       "00401003 7508 JNZ 0x00401007"], "%08x" % FN)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertIn("REL-BRANCH-IN-STOLEN", r.stdout)

    def test_direct_call_is_rel_branch(self):
        r = self.hook(["00401000 e8fb000000 CALL 0x00401100"], "%08x" % FN)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertIn("REL-BRANCH-IN-STOLEN", r.stdout)

    def test_direct_jmp_is_rel_branch(self):
        r = self.hook(["00401000 e9fb000000 JMP 0x00401100"], "%08x" % FN)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertIn("REL-BRANCH-IN-STOLEN", r.stdout)

    def test_plain_instructions_are_not_flagged(self):
        r = self.hook(["00401000 55 PUSH EBP", "00401001 8bec MOV EBP,ESP", "00401003 83ec10 SUB ESP,0x10"],
                      "%08x" % FN)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertNotIn("REL-BRANCH-IN-STOLEN", r.stdout)
        self.assertNotIn("FN-END-SHORT", r.stdout)

    def test_function_end_before_enough_bytes_is_reported(self):
        # Only 3 bytes (6 hex chars) before the function ends; the default need is 5 bytes. The old loop ran past ins[].
        r = self.hook(["00401000 55 PUSH EBP", "00401001 8bec MOV EBP,ESP"], "%08x" % FN)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertNotIn("Traceback", r.stderr)
        self.assertIn("558bec", r.stdout)
        self.assertIn("FN-END-SHORT", r.stdout)


if __name__ == "__main__":
    unittest.main()
