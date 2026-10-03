"""cmdtable.main(): end-to-end run on a mini world (GOG / Steam / Mac stores, GOG function DB, nwscript.nss).

Everything is redirected into a private temp dir through BD_ROOT / BD_SERVER / BD_DUMP and the BD_*_PROGRAM names, so
main() writes only into <tmp>/root/B99 (never the real B15 and never a database). Contract: DESIGN section 4.8.

World (slot -> handlers; GOG 0x005xxxxx, Steam 0x006xxxxx, Mac 0x007xxxxx):
  0      G100 / S100 / M100            ExecuteCommandAlpha        exact, name yes, mac yes
  1,2    G200 / S200 / M200,M210       ExecuteCommandBeta_2       exact, name yes (_N suffix), mac yes
  3,4    G300 / S300+S310 / M300+M310  ExecuteCommandDelta        no_equal_set (partial extras), name yes, mac yes
  5      G400 / S400 / M400            ExecuteCommandTheta (slot 7)  exact, name NO (name_steam = S600), mac NO
  6      G500 / S500 / M500            ExecuteCommandEta          exact, name yes
  7      G600 / S600 / -               (no ExecuteCommand* row: reported as an unnamed table handler)
  8      G700 / S700 / -               ExecuteCommandNotInNss     exact, name n/a, mac n/a
  156    G1000 then G1100 / S1000 then S1100 / M1100   dead pair G1000 -> S1000 (overwritten_slot_156), live exact
  (none) G900 is in no table      ExecuteCommandNeverStored   match none
"""
import contextlib
import io
import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import cmdtable  # noqa: E402

GOG_FNS, STEAM_FNS, MAC_FNS = "0x1000,0x2000", "0x3000,0x4000", "0x5000,0x6000"
GOG_PROG, STEAM_PROG, MAC_PROG = "gogprog", "steamprog", "macprog"
CLS = "CSWVirtualMachineCommands"
DEAD_SLOT = 156
NAMED = ["Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta"]


def nss_text(n_funcs=cmdtable.SLOTS):
    names = NAMED + ["Fn%d" % i for i in range(len(NAMED), n_funcs)]
    lines = ["// int Commented(int a);", "int    NUM = 3;", "/* void Gone();", "   int AlsoGone(); */"]
    types = ["int", "float", "void", "object", "string", "effect"]
    for i, n in enumerate(names):
        lines.append("%s %s(int nParam%d);   // trailing comment" % (types[i % len(types)], n, i))
    return "\n".join(lines) + "\n"


class _Stores:
    """Builds a cmdstores.tsv; instruction addresses grow so store order = call order."""
    def __init__(self, stride):
        self.stride, self.rows, self.insn = stride, [], 0x10

    def add(self, fn, slot, target, disp=None):
        self.insn += 4
        d = slot * self.stride if disp is None else disp
        self.rows.append("0x%08x\t0x%08x\tECX\t0x%x\t0x%08x" % (fn, fn + self.insn, d, target))

    def text(self):
        return "fn\tinsn\tbase\tdisp\ttarget\n" + "\n".join(self.rows) + "\n"


def gog_stores():
    s = _Stores(4)
    for slot, h in [(0, 0x500100), (1, 0x500200), (2, 0x500200), (3, 0x500300), (4, 0x500300), (5, 0x500400),
                    (6, 0x500500), (7, 0x500600), (DEAD_SLOT, 0x501000)]:
        s.add(0x1000, slot, h)
    s.add(0x1000, 0, 0x5FFFFF, disp=0x6)            # unaligned: ignored
    s.add(0x1000, 0, 0x5FFFFE, disp=0x4000)         # beyond the table: ignored
    s.add(0x9000, 0, 0x5FFFFD)                      # unlisted init function: ignored
    s.add(0x2000, 8, 0x500700)
    s.add(0x2000, DEAD_SLOT, 0x501100)              # second init function overwrites the dead handler
    return s


def steam_stores():
    s = _Stores(4)
    for slot, h in [(0, 0x600100), (1, 0x600200), (2, 0x600200), (3, 0x600300), (4, 0x600310), (5, 0x600400),
                    (6, 0x600500), (7, 0x600600), (DEAD_SLOT, 0x601000), (DEAD_SLOT, 0x601100)]:
        s.add(0x3000, slot, h)
    s.add(0x4000, 8, 0x600700)
    return s


def mac_stores():
    s = _Stores(8)
    for slot, h in [(0, 0x700100), (1, 0x700200), (2, 0x700210), (3, 0x700300), (4, 0x700310), (6, 0x700500),
                    (DEAD_SLOT, 0x701100)]:
        s.add(0x5000, slot, h)
    s.add(0x5000, 0, 0x7FFFFF, disp=0x4)            # stride 8: not a slot start, ignored
    s.add(0x6000, 5, 0x700400)
    return s


GOG_FUNCS = [  # (addr, class, function)
    (0x500100, CLS, "ExecuteCommandAlpha"),
    (0x500200, CLS, "ExecuteCommandBeta_2"),
    (0x500300, CLS, "ExecuteCommandDelta"),
    (0x500400, CLS, "ExecuteCommandTheta"),
    (0x500500, CLS, "ExecuteCommandEta"),
    (0x500700, CLS, "ExecuteCommandNotInNss"),
    (0x500900, CLS, "ExecuteCommandNeverStored"),
    (0x501000, CLS, "ExecuteCommandDead"),
    (0x501100, CLS, "ExecuteCommandFn156"),
    (0x502000, CLS, "ExecuteCommand"),              # bare prefix: skipped
    (0x502100, CLS, "SomethingElse"),               # not an ExecuteCommand*: skipped
]
MAC_FP = [(0x700100, "ExecuteCommandAlpha"), (0x700200, "ExecuteCommandBeta"), (0x700210, "ExecuteCommandGamma"),
          (0x700300, "ExecuteCommandDelta"), (0x700310, ""), (0x700400, "ExecuteCommandZeta"),
          (0x700500, "ExecuteCommandEta"), (0x701100, "ExecuteCommandFn156")]


def tsv(path, header, rows):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\t".join(header) + "\n" + "".join("\t".join(r) + "\n" for r in rows), encoding="utf-8")


def read_tsv(path):
    lines = path.read_text(encoding="utf-8").splitlines()
    head = lines[0].split("\t")
    return head, [dict(zip(head, ln.split("\t"))) for ln in lines[1:]]


class MainTestBase(unittest.TestCase):
    batch = "B99"

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = Path(self._tmp.name)
        self.root, self.server, self.dump = self.tmp / "root", self.tmp / "server", self.tmp / "dump"
        self.nss = self.tmp / "nwscript.nss"
        self.nss.write_text(nss_text())
        (self.server / GOG_PROG).mkdir(parents=True)
        (self.server / GOG_PROG / "cmdstores.tsv").write_text(gog_stores().text())
        (self.server / STEAM_PROG).mkdir(parents=True)
        (self.server / STEAM_PROG / "cmdstores.tsv").write_text(steam_stores().text())
        (self.server / MAC_PROG).mkdir(parents=True)
        (self.server / MAC_PROG / "cmdstores.tsv").write_text(mac_stores().text())
        tsv(self.server / MAC_PROG / "fp.tsv", ["addr", "name", "namespace"],
            [["0x%08x" % a, n, "CSWVirtualMachineCommands"] for a, n in MAC_FP])
        tsv(self.dump / "inputs" / "gog_functions.tsv",
            ["gog_addr", "class", "function", "callconv", "param_size_bytes", "notes"],
            [["0x%08x" % a, c, f, "__thiscall", "4", ""] for a, c, f in GOG_FUNCS])
        env = {"BD_ROOT": str(self.root), "BD_SERVER": str(self.server), "BD_DUMP": str(self.dump),
               "BD_GOG_PROGRAM": GOG_PROG, "BD_STEAM_PROGRAM": STEAM_PROG, "BD_MAC_PROGRAM": MAC_PROG}
        p = mock.patch.dict(os.environ, env)
        p.start()
        self.addCleanup(p.stop)
        self.addCleanup(self._tmp.cleanup)

    def run_main(self, *extra, nss=None):
        argv = [self.batch, "--gog-fns", GOG_FNS, "--steam-fns", STEAM_FNS, "--mac-fns", MAC_FNS,
                "--nss", str(nss or self.nss)] + list(extra)
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            rc = cmdtable.main(argv)
        self.out = out.getvalue()
        self.bdir = self.root / self.batch
        return rc

    def rows(self):
        return {r["name"].split("::")[-1]: r for r in read_tsv(self.bdir / "cmdtable_rows.tsv")[1]}


class MainEndToEndTests(MainTestBase):
    def setUp(self):
        super().setUp()
        self.assertEqual(self.run_main(), 0)
        self.r = self.rows()

    def test_writes_only_into_the_temp_batch_dir(self):
        self.assertEqual(sorted(p.name for p in self.root.iterdir()), ["B99"])
        self.assertEqual(sorted(p.name for p in self.bdir.iterdir()),
                         ["card_facts.tsv", "cmdtable.tsv", "cmdtable_report.md", "cmdtable_rows.tsv",
                          "extra_candidates.tsv"])

    def test_stdout_is_first_report_summary_lines(self):
        lines = self.out.strip().splitlines()
        self.assertEqual(len(lines), 3)
        self.assertTrue(lines[0].startswith("- slots 877; filled GOG 10 / Steam 10"))

    def test_headers(self):
        self.assertEqual(read_tsv(self.bdir / "cmdtable.tsv")[0], ["index", "nwscript", "gog_handler", "steam_handler"])
        self.assertEqual(read_tsv(self.bdir / "cmdtable_rows.tsv")[0],
                         ["gog", "name", "gog_slots", "steam_counterpart", "match", "name_slot", "name_steam",
                          "mac_names", "name_mac"])
        self.assertEqual(read_tsv(self.bdir / "extra_candidates.tsv")[0], ["gog", "steam", "source"])
        self.assertEqual(read_tsv(self.bdir / "card_facts.tsv")[0], ["gog", "fact"])

    def test_cmdtable_tsv_has_one_row_per_slot(self):
        _, rows = read_tsv(self.bdir / "cmdtable.tsv")
        self.assertEqual(len(rows), cmdtable.SLOTS)
        self.assertEqual([int(r["index"]) for r in rows], list(range(cmdtable.SLOTS)))
        self.assertEqual(rows[0], {"index": "0", "nwscript": "Alpha", "gog_handler": "0x00500100",
                                   "steam_handler": "0x00600100"})
        self.assertEqual(rows[4]["steam_handler"], "0x00600310")
        self.assertEqual(rows[9]["gog_handler"], "")           # unfilled slot
        self.assertEqual(rows[9]["steam_handler"], "")
        # last store wins: slot 156 holds the live handler on both sides
        self.assertEqual(rows[DEAD_SLOT]["gog_handler"], "0x00501100")
        self.assertEqual(rows[DEAD_SLOT]["steam_handler"], "0x00601100")
        self.assertEqual(rows[DEAD_SLOT]["nwscript"], "Fn156")

    def test_only_execute_command_prefixed_rows_are_reported(self):
        self.assertEqual(set(self.r), {"ExecuteCommandAlpha", "ExecuteCommandBeta_2", "ExecuteCommandDelta",
                                       "ExecuteCommandTheta", "ExecuteCommandEta", "ExecuteCommandNotInNss",
                                       "ExecuteCommandNeverStored", "ExecuteCommandDead", "ExecuteCommandFn156"})

    def test_match_exact(self):
        a = self.r["ExecuteCommandAlpha"]
        self.assertEqual((a["gog"], a["gog_slots"], a["steam_counterpart"], a["match"]),
                         ("0x00500100", "0", "0x00600100", "exact"))
        b = self.r["ExecuteCommandBeta_2"]
        self.assertEqual((b["gog_slots"], b["steam_counterpart"], b["match"]), ("1,2", "0x00600200", "exact"))
        self.assertEqual(self.r["ExecuteCommandFn156"]["match"], "exact")
        self.assertEqual(self.r["ExecuteCommandFn156"]["steam_counterpart"], "0x00601100")

    def test_match_no_equal_set(self):
        d = self.r["ExecuteCommandDelta"]
        self.assertEqual((d["gog_slots"], d["match"], d["steam_counterpart"]), ("3,4", "no_equal_set", ""))

    def test_match_none_when_handler_is_in_no_table(self):
        n = self.r["ExecuteCommandNeverStored"]
        self.assertEqual((n["gog_slots"], n["match"], n["steam_counterpart"], n["name_slot"], n["name_mac"],
                          n["mac_names"]), ("", "none", "", "n/a", "n/a", ""))

    def test_ambiguous_label_is_unreachable_by_construction(self):
        # Two Steam handlers cannot share a slot set (a slot has one handler), so no row is ever "ambiguous".
        self.assertNotIn("ambiguous", {r["match"] for r in self.r.values()})

    def test_overwritten_slot_pair(self):
        dead = self.r["ExecuteCommandDead"]
        self.assertEqual((dead["gog"], dead["gog_slots"], dead["match"], dead["steam_counterpart"]),
                         ("0x00501000", "", "overwritten_slot_156", "0x00601000"))
        _, extra = read_tsv(self.bdir / "extra_candidates.tsv")
        self.assertIn({"gog": "0x00501000", "steam": "0x00601000", "source": "cmdtable_overwritten"}, extra)

    def test_name_slot_yes_no_na(self):
        self.assertEqual(self.r["ExecuteCommandAlpha"]["name_slot"], "yes")
        self.assertEqual(self.r["ExecuteCommandBeta_2"]["name_slot"], "yes")     # slot 1 in {1,2}; _N stripped
        self.assertEqual(self.r["ExecuteCommandDelta"]["name_slot"], "yes")
        self.assertEqual(self.r["ExecuteCommandFn156"]["name_slot"], "yes")
        t = self.r["ExecuteCommandTheta"]                                         # Theta = slot 7, row owns slot 5
        self.assertEqual((t["name_slot"], t["gog_slots"], t["name_steam"]), ("no", "5", "0x00600600"))
        self.assertEqual(self.r["ExecuteCommandNotInNss"]["name_slot"], "n/a")    # name not in nwscript.nss
        self.assertEqual(self.r["ExecuteCommandNeverStored"]["name_slot"], "n/a")  # no slots at all
        self.assertEqual(self.r["ExecuteCommandDead"]["name_slot"], "n/a")

    def test_name_steam_is_the_steam_handler_of_the_named_slot(self):
        self.assertEqual(self.r["ExecuteCommandAlpha"]["name_steam"], "0x00600100")
        self.assertEqual(self.r["ExecuteCommandBeta_2"]["name_steam"], "0x00600200")
        self.assertEqual(self.r["ExecuteCommandNotInNss"]["name_steam"], "")

    def test_name_mac_yes_no_na(self):
        self.assertEqual(self.r["ExecuteCommandAlpha"]["name_mac"], "yes")
        b = self.r["ExecuteCommandBeta_2"]                                        # `_2` stripped before comparing
        self.assertEqual((b["mac_names"], b["name_mac"]), ("ExecuteCommandBeta,ExecuteCommandGamma", "yes"))
        t = self.r["ExecuteCommandTheta"]
        self.assertEqual((t["mac_names"], t["name_mac"]), ("ExecuteCommandZeta", "no"))
        self.assertEqual(self.r["ExecuteCommandEta"]["name_mac"], "yes")
        n = self.r["ExecuteCommandNotInNss"]                                      # slot 8 absent from the Mac table
        self.assertEqual((n["mac_names"], n["name_mac"]), ("", "n/a"))

    def test_mac_stride_8_decoding(self):
        # slot i lives at disp 8*i; the disp-0x4 junk store (target 0x007fffff) must not show up as a name or slot
        self.assertNotIn("7fffff", "".join(r["mac_names"] for r in self.r.values()))
        self.assertEqual(self.r["ExecuteCommandFn156"]["mac_names"], "ExecuteCommandFn156")   # disp 0x4e0
        # Mac second init function (0x6000) is decoded too: slot 5 -> Zeta
        self.assertEqual(self.r["ExecuteCommandTheta"]["mac_names"], "ExecuteCommandZeta")

    def test_mac_handler_without_fp_name_falls_back_to_address(self):
        d = self.r["ExecuteCommandDelta"]
        self.assertEqual(d["mac_names"], "0x00700310,ExecuteCommandDelta")
        self.assertEqual(d["name_mac"], "yes")

    def test_extra_candidates(self):
        _, extra = read_tsv(self.bdir / "extra_candidates.tsv")
        got = {(r["gog"], r["steam"], r["source"]) for r in extra}
        self.assertEqual(len(extra), len(got))
        self.assertEqual(got, {
            ("0x00500100", "0x00600100", "cmdtable"),
            ("0x00500200", "0x00600200", "cmdtable"),
            ("0x00500300", "0x00600300", "cmdtable_partial"),      # no_equal_set: every Steam handler of its slots
            ("0x00500300", "0x00600310", "cmdtable_partial"),
            ("0x00500400", "0x00600400", "cmdtable"),
            ("0x00500500", "0x00600500", "cmdtable"),
            ("0x00500700", "0x00600700", "cmdtable"),
            ("0x00501100", "0x00601100", "cmdtable"),
            ("0x00501000", "0x00601000", "cmdtable_overwritten"),
        })
        self.assertNotIn("0x00500900", {g for g, _s, _o in got})   # match none: no candidate

    def test_card_facts_one_per_gog_handler_and_no_steam_address(self):
        _, facts = read_tsv(self.bdir / "card_facts.tsv")
        by_gog = {f["gog"]: f["fact"] for f in facts}
        handlers = {"0x00500100", "0x00500200", "0x00500300", "0x00500400", "0x00500500", "0x00500600",
                    "0x00500700", "0x00501100"}
        self.assertEqual(set(by_gog), handlers | {"0x00501000"})      # + the dead handler
        self.assertEqual(len(facts), len(by_gog))
        self.assertIn("1 Beta, 2 Gamma", by_gog["0x00500200"])
        self.assertIn("156 Fn156", by_gog["0x00501100"])
        self.assertIn("slot 156 (Fn156)", by_gog["0x00501000"])
        self.assertIn("dead code", by_gog["0x00501000"])
        text = (self.bdir / "card_facts.tsv").read_text()
        self.assertNotIn("0x006", text)
        for steam in ("600100", "600200", "600300", "600310", "600400", "600500", "600600", "600700", "601000",
                      "601100"):
            self.assertNotIn(steam, text)

    def test_report_sections_and_counts(self):
        rep = (self.bdir / "cmdtable_report.md").read_text()
        self.assertTrue(rep.startswith("# B99 command tables (GOG %s, Steam %s)" % (GOG_FNS, STEAM_FNS)))
        for sec in ("## Slots stored twice (the later store is the runtime handler)",
                    "## GOG label contradicts the dispatch table (name_slot = no)",
                    "## GOG name is not a Mac symbol of its slots (name_mac = no)",
                    "## GOG rows without an exact counterpart",
                    "## GOG table handlers with no `ExecuteCommand*` name in the GOG DB (1)"):
            self.assertIn(sec, rep)
        self.assertIn("- slots 877; filled GOG 10 / Steam 10; distinct handlers GOG 8 / Steam 9", rep)
        self.assertIn("- slot sets equal on both sides: 7 of 8 GOG handlers", rep)
        self.assertIn("GOG `ExecuteCommand*` rows 9: mac:n/a 3, mac:no 1, mac:yes 5, match:exact 6, match:no_equal_set 1, "
                      "match:none 1, match:overwritten_slot_156 1, name:n/a 3, name:no 1, name:yes 5", rep)
        self.assertIn("- GOG slot 156 Fn156: 0x00501000 overwritten by 0x00501100 at 0x00", rep)
        self.assertIn("- Steam slot 156 Fn156: 0x00601000 overwritten by 0x00601100 at 0x00", rep)
        self.assertIn("- 0x00500400 %s::ExecuteCommandTheta slots {5}; slot of the name = 7 -> name-correct Steam "
                      "0x00600600" % CLS, rep)
        self.assertIn("- 0x00500400 %s::ExecuteCommandTheta slots {5}; Mac: ExecuteCommandZeta; nwscript slot check: no"
                      % CLS, rep)
        self.assertIn("- 0x00500300 %s::ExecuteCommandDelta slots {3,4} -> - (no_equal_set)" % CLS, rep)
        self.assertIn("-> 0x00601000 (overwritten_slot_156)", rep)
        self.assertNotIn("(exact)", rep)
        self.assertIn("- 0x00500600 (no DB row) slots {7}", rep)


class MainVariantTests(MainTestBase):
    def test_skip_mac_gives_na_everywhere(self):
        (self.server / MAC_PROG / "cmdstores.tsv").unlink()      # nothing Mac needed when --mac-fns ''
        argv = [self.batch, "--gog-fns", GOG_FNS, "--steam-fns", STEAM_FNS, "--mac-fns", "", "--nss", str(self.nss)]
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(cmdtable.main(argv), 0)
        _, rows = read_tsv(self.root / self.batch / "cmdtable_rows.tsv")
        self.assertEqual(len(rows), 9)
        self.assertEqual({r["name_mac"] for r in rows}, {"n/a"})
        self.assertEqual({r["mac_names"] for r in rows}, {""})

    def test_prefix_option_selects_other_rows(self):
        self.assertEqual(self.run_main("--prefix", "Something"), 0)
        _, rows = read_tsv(self.bdir / "cmdtable_rows.tsv")
        self.assertEqual([r["name"] for r in rows], ["%s::SomethingElse" % CLS])
        self.assertEqual(rows[0]["match"], "none")

    def test_rerun_overwrites_outputs(self):
        self.run_main()
        first = (self.bdir / "cmdtable_rows.tsv").read_text()
        self.run_main()
        self.assertEqual((self.bdir / "cmdtable_rows.tsv").read_text(), first)

    def test_wrong_nss_function_count_is_an_error(self):
        short = self.tmp / "short.nss"
        short.write_text(nss_text(cmdtable.SLOTS - 1))
        with self.assertRaises(SystemExit) as cm:
            self.run_main(nss=short)
        self.assertIn("expected 877", str(cm.exception))

    def test_duplicate_nss_function_is_an_error(self):
        dup = self.tmp / "dup.nss"
        dup.write_text(nss_text() + "int Alpha(int x);\n")      # 878 functions
        with self.assertRaises(SystemExit):
            self.run_main(nss=dup)

    def test_missing_init_function_stores_is_an_error(self):
        with self.assertRaises(SystemExit) as cm:
            cmdtable.main([self.batch, "--gog-fns", "0x1000,0x7777", "--steam-fns", STEAM_FNS,
                           "--mac-fns", MAC_FNS, "--nss", str(self.nss)])
        self.assertIn("0x7777", str(cm.exception))

    def test_equal_dead_pairs_need_both_sides(self):
        # Steam without the slot-156 double store: the GOG dead handler has no pair -> match none, no candidate
        s = _Stores(4)
        for slot, h in [(0, 0x600100), (1, 0x600200), (2, 0x600200), (3, 0x600300), (4, 0x600310), (5, 0x600400),
                        (6, 0x600500), (7, 0x600600), (DEAD_SLOT, 0x601100)]:
            s.add(0x3000, slot, h)
        s.add(0x4000, 8, 0x600700)
        (self.server / STEAM_PROG / "cmdstores.tsv").write_text(s.text())
        self.assertEqual(self.run_main(), 0)
        dead = self.rows()["ExecuteCommandDead"]
        self.assertEqual((dead["match"], dead["steam_counterpart"]), ("none", ""))
        _, extra = read_tsv(self.bdir / "extra_candidates.tsv")
        self.assertNotIn("cmdtable_overwritten", {r["source"] for r in extra})
        _, facts = read_tsv(self.bdir / "card_facts.tsv")
        self.assertNotIn("0x00501000", {f["gog"] for f in facts})


if __name__ == "__main__":
    unittest.main()
