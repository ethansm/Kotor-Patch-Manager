import os
import tempfile
import unittest

import helpers as h
import atlas_common as ac
import cg


class CgFixtureTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.db = os.path.join(self.tmp.name, "k.db")
        h.build_db(self.db)
        self.base = ["--root", h.FIXROOT, "--db", self.db]

    def tearDown(self):
        self.tmp.cleanup()

    def run_cg(self, *args, pre=()):
        return h.run_main(cg, list(self.base) + list(pre) + list(args))

    def test_address_forms(self):
        for s in ("0x401000", "401000", "00401000", "0x00401000", "0X401000"):
            self.assertEqual(ac.parse_addr(s), 0x401000)

    def test_callers_with_names(self):
        rc, out, _ = self.run_cg("callers", "0x401300")
        self.assertEqual(rc, 0)
        self.assertIn("2 call sites from 2 functions", out)
        self.assertIn("00401100 Foo::Beta [medium]", out)       # DB name + confidence
        self.assertIn("00401200 Foo::Gamma2 [high]", out)
        self.assertIn("TAILJMP", out)

    def test_name_fallback_order(self):
        names = ac.Names(h.FIXROOT, ac.open_db_ro(self.db))
        self.assertEqual(names.raw(0x401000), ("Foo::Alpha", "high"))   # name_map
        self.assertEqual(names.raw(0x401300), ("FUN_00401300", ""))     # callers_count (FUN_ there)
        self.assertEqual(names.raw(0x999999), ("FUN_00999999", ""))     # synthetic
        rc, out, _ = self.run_cg("callers", "0x401300", pre=["--no-names"])
        self.assertNotIn("Foo::", out)

    def test_callers_tsv(self):
        _, out, _ = self.run_cg("callers", "401300", pre=["--tsv"])
        lines = out.strip().split("\n")
        self.assertEqual(lines[0].split("\t"), ["callee", "site", "caller", "caller_name", "type"])
        self.assertEqual(len(lines), 3)  # header + the two call sites
        self.assertTrue(all(len(l.split("\t")) == 5 for l in lines))

    def test_callees_order_and_indirect_slot(self):
        _, out, _ = self.run_cg("callees", "0x401000")
        sites = [l.split()[0] for l in out.split("\n") if l.startswith("  00")]
        self.assertEqual(sites, sorted(sites))
        self.assertIn("slot 0x70", out)
        self.assertIn("slot match, class unverified: Foo idx 28 -> 00401400", out)
        self.assertIn("IMPORT", out)
        self.assertIn("LoadIconA", out)  # import name kept in the insn text

    def test_callees_direct_memory_slot(self):
        _, out, _ = self.run_cg("callees", "0x401200")
        self.assertIn("slot 0x74", out)
        self.assertIn("Baz idx 29", out)

    def test_path_direct_two_paths(self):
        rc, out, _ = self.run_cg("path", "0x401000", "0x401300", "--max", "5")
        self.assertEqual(rc, 0)
        self.assertIn("2 shortest path(s) of 2 hops", out)
        self.assertIn("TAILJMP", out)
        _, out1, _ = self.run_cg("path", "0x401000", "0x401300", "--max", "1")
        self.assertIn("1 shortest path(s)", out1)

    def test_path_none_without_vtable_then_found_with_flag(self):
        rc, out, _ = self.run_cg("path", "0x401000", "0x401400")
        self.assertEqual(rc, 1)
        self.assertIn("no path", out)
        rc, out, _ = self.run_cg("path", "0x401000", "0x401400", "--via-vtable")
        self.assertEqual(rc, 0)
        self.assertIn("VTABLE", out)
        self.assertIn("class unverified", out)

    def test_path_manual_bridge(self):
        rc, out, _ = self.run_cg("path", "0x401000", "0x401500", "--bridge", "0x401000:0x401500")
        self.assertEqual(rc, 0)
        self.assertIn("BRIDGE", out)
        rc, out, _ = self.run_cg("path", "0x401000", "0x401500")
        self.assertEqual(rc, 1)

    def test_path_depth_limit(self):
        rc, _out, _ = self.run_cg("path", "0x401000", "0x401300", "--depth", "1")
        self.assertEqual(rc, 1)

    def test_path_with_cycle_terminates(self):
        # 401300 -> 401000 -> 401100 -> 401300 is a cycle
        rc, out, _ = self.run_cg("path", "0x401300", "0x401200")
        self.assertEqual(rc, 0)
        self.assertIn("2 hops", out)

    def test_subtree_marks_repeats(self):
        _, out, _ = self.run_cg("subtree", "0x401000", "--depth", "4")
        self.assertIn("(seen)", out)
        self.assertEqual(out.count("00401300 FUN_00401300") , 2)  # once expanded, once "(seen)"
        self.assertIn("distinct functions", out)

    def test_subtree_depth_one(self):
        _, out, _ = self.run_cg("subtree", "0x401000", "--depth", "1")
        self.assertIn("00401100", out)
        self.assertNotIn("00401300", out)

    def test_indirect_slot_scan(self):
        _, out, _ = self.run_cg("indirect", "--slot", "0x70")
        self.assertIn("with vtable slot 0x70: 2", out)  # 401000@401020 and 401300@401320
        self.assertIn("0040101", out)
        _, out, _ = self.run_cg("indirect", "--slot", "0x70", "--in", "0x401300")
        self.assertIn(": 1", out)
        self.assertIn("00401320", out)
        _, out, _ = self.run_cg("indirect", "--slot", "0x74")
        self.assertIn(": 1", out.split("\n")[0])

    def test_resolve_indirect_clobber_by_call(self):
        lines = ac.parse_asm_text(
            "00000010  8b4270                MOV EAX,dword ptr [EDX + 0x70]\n"
            "00000013  e800000000            CALL 0x00000100\n"
            "00000018  ffd0                  CALL EAX\n")
        slot, how = ac.resolve_indirect(lines, 0x18, "CALL EAX")
        self.assertIsNone(slot)
        self.assertIn("clobbered", how)

    def test_resolve_indirect_slot_zero_and_ebp(self):
        lines = ac.parse_asm_text("00000010  8b02                  MOV EAX,dword ptr [EDX]\n00000012  ffd0                  CALL EAX\n")
        self.assertEqual(ac.resolve_indirect(lines, 0x12, "CALL EAX")[0], 0)
        self.assertIsNone(ac.resolve_indirect(lines, 0x12, "CALL dword ptr [EBP + -0x8]")[0])

    def test_asm_parser_run_together_bytes(self):
        lines = ac.parse_asm_text("0040203a  c78490bc10000000000000MOV dword ptr [EAX + EDX*0x4 + 0x10bc],0x0\n"
                                  "0040203b  90                    NOP\n; comment\n")
        self.assertEqual(lines[0], (0x40203a, "c78490bc10000000000000", "MOV dword ptr [EAX + EDX*0x4 + 0x10bc],0x0"))
        self.assertEqual(len(lines), 2)

    def test_parse_mem(self):
        self.assertEqual(ac.parse_mem("EAX + EDX*0x4 + 0x10bc"), ("EAX", "EDX", 4, 0x10bc))
        self.assertEqual(ac.parse_mem("EBP + -0x8"), ("EBP", None, 1, 0xFFFFFFF8))
        self.assertEqual(ac.parse_mem("0x00a7e098"), (None, None, 1, 0xA7E098))


@unittest.skipUnless(os.path.isdir(os.path.join(h.REAL_ROOT, "decomp")) and os.path.exists(h.REAL_DB),
                     "real ROOT/DB not present")
class CgRealDataTests(unittest.TestCase):
    base = ["--root", h.REAL_ROOT, "--db", h.REAL_DB]

    def test_callers_attack_handler(self):
        _, out, _ = h.run_main(cg, self.base + ["callers", "0x6D7390"])
        self.assertIn("005d635e", out)
        self.assertIn("RunActions", out)

    def test_scheduler_to_attack_handler_needs_bridge(self):
        rc, _, _ = h.run_main(cg, self.base + ["path", "0x51D790", "0x6D7390"])
        self.assertEqual(rc, 1)  # honest: no pure direct chain (the hop is vtable +0x70)
        rc, out, _ = h.run_main(cg, self.base + ["path", "0x51D790", "0x6D7390", "--bridge", "0x51D790:0x569320"])
        self.assertEqual(rc, 0)
        self.assertIn("005d56b0", out)
        self.assertIn("006d7390", out)

    def test_scheduler_slot_70_site(self):
        _, out, _ = h.run_main(cg, self.base + ["indirect", "--slot", "0x70", "--in", "0x51D790"])
        self.assertIn("0051dc7b", out)


if __name__ == "__main__":
    unittest.main()
