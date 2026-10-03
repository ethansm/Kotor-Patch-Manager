"""Unit tests for steam_db_batches/tools (bdlib.py + batch.py) on a synthetic mini dump built in a temp dir.

World layout (class CFoo): GOG address = 0x00410000 + off, Steam address = 0x00510000 + off (delta +0x100000).
Trusted helper functions (CHelper::H<n>) are B1-verified and give the pairs their A-CALLEE anchors.
Run:  python3 -m unittest steam_db_batches/tools/tests/test_batch.py   (or discover from the tools directory)
"""
import contextlib
import csv
import hashlib
import io
import json
import os
import sqlite3
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))
import batch  # noqa: E402
import bdlib as bd  # noqa: E402

GOG_BASE, STEAM_BASE = 0x00410000, 0x00510000
GOG_PROG, STEAM_PROG = bd.GOG_PROGRAM_DEFAULT, bd.STEAM_PROGRAM_DEFAULT


def h(a):
    return "0x%08x" % a


def hexlist(vals):
    return ",".join(h(v) for v in sorted(vals))


# ------------------------------------------------------------------------------------------------ fixture builder
class World:
    """Collects a mini Ghidra dump + server exports + DBs, then writes them under a root directory."""

    def __init__(self, root: Path, with_gog_fp: bool = True):
        self.root = Path(root)
        self.with_gog_fp = with_gog_fp
        self.steam = {}         # addr -> dict(name,size,retn,source,thunk_to)
        self.gog = []           # dicts
        self.gog_fp, self.steam_fp = {}, {}
        self.calls = []         # (caller, callee) Steam call edges
        self.sstr, self.gstr = {}, {}   # string addr -> (text, [referring function addrs])
        self.name_map, self.branch, self.b1 = [], [], []
        self.b1_rejected = []   # (class, name, cand_orig address)
        self.mac = []           # (namespace, name, disp list, imm list) rows of the Mac x86 fingerprint export
        self.mapper = []
        self.n_helper = 0
        self.n_pair = 0
        self.exe_bytes = {}     # VA -> bytes placed into the fake exe's .text section

    # -- primitives
    def steam_fn(self, addr, name, retn=0, size=0x30, source="DEFAULT", thunk_to=None, asm=True):
        self.steam[addr] = dict(name=name, retn=retn, size=size, source=source, thunk_to=thunk_to, asm=asm)

    def gog_fn(self, addr, cls, fn, cc="__thiscall", param=0):
        self.gog.append(dict(addr=addr, cls=cls, fn=fn, cc=cc, param=param))

    def fp(self, side, addr, retn, callees=(), strs=(), disp=(), imm=(), size=48):
        d = self.gog_fp if side == "gog" else self.steam_fp
        d[addr] = dict(retn=retn, callees=set(callees), strs=set(strs), disp=list(disp), imm=list(imm), size=size)

    def string(self, side, addr, text, refs=()):
        (self.gstr if side == "gog" else self.sstr)[addr] = (text, list(refs))

    def helper(self):
        """A B1-verified helper function pair; -> (gog_addr, steam_addr)."""
        n = self.n_helper
        self.n_helper += 1
        g, s = 0x00400000 + 0x40 * n, 0x00500000 + 0x40 * n
        self.gog_fn(g, "CHelper", "H%d" % n, param=0)
        self.steam_fn(s, "FUN_%08x" % s, retn=0)
        self.b1.append(("CHelper", "H%d" % n, h(s)))
        return g, s

    def pair(self, cls, fn, g, s, retn, param, anchors=("STR", "CALLEE", "SIG"), cc="__thiscall", size=0x30):
        """A GOG function and its true Steam function with distinct fingerprints for the chosen anchor families."""
        i = self.n_pair
        self.n_pair += 1
        self.gog_fn(g, cls, fn, cc=cc, param=param)
        self.steam_fn(s, "FUN_%08x" % s, retn=retn, size=size)
        gcal, scal, gstr, sstr, disp, imm = set(), set(), set(), set(), [], []
        if "STR" in anchors:
            ga, sa = 0x00900000 + 0x20 * i, 0x00A00000 + 0x20 * i
            text = "unique anchor string number %d for %s" % (i, fn)
            self.string("gog", ga, text, [g])
            self.string("steam", sa, text, [s])
            gstr.add(ga)
            sstr.add(sa)
        if "CALLEE" in anchors:
            for _ in range(2):
                hg, hs = self.helper()
                gcal.add(hg)
                scal.add(hs)
                self.calls.append((s, hs))
        if "SIG" in anchors:
            disp = [0x1000 + 0x10 * i + 4 * j for j in range(4)]
            imm = [0x5000 + 0x10 * i + j for j in range(4)]
        self.fp("gog", g, retn, gcal, gstr, disp, imm)
        self.fp("steam", s, retn, scal, sstr, disp, imm)

    def filler(self, cls, fn, g, s, retn=8, param=8):
        self.gog_fn(g, cls, fn, param=param)
        self.steam_fn(s, "FUN_%08x" % s, retn=retn)

    # -- writers
    def build(self):
        self.dump = self.root / "dump"
        self.server = self.root / "server"
        (self.dump / "inputs").mkdir(parents=True, exist_ok=True)
        self._write_dump()
        self._write_exe()
        self.server.mkdir(parents=True, exist_ok=True)
        self._write_mac()
        self._write_server()
        self._write_dbs()
        self._write_mapper()
        return self

    def _write_dump(self):
        rows = ["addr\tsize\tretn\tname\tnamespace\tsource\tthunk"]
        for a in sorted(self.steam):
            f = self.steam[a]
            rows.append("%s\t%d\t%d\t%s\t\t%s\t%s" % (h(a), f["size"], f["retn"], f["name"], f["source"],
                                                       "true" if f["thunk_to"] else "false"))
            if f.get("asm", True):
                self._write_asm(a, f)
        (self.dump / "inputs" / "steam_functions.tsv").write_text("\n".join(rows) + "\n")
        rows = ["gog_addr\tclass\tfunction\tcallconv\tparam_size_bytes\tnotes"]
        for g in sorted(self.gog, key=lambda x: x["addr"]):
            rows.append("%08x\t%s\t%s\t%s\t%d\tvoid" % (g["addr"], g["cls"], g["fn"], g["cc"], g["param"]))
        (self.dump / "inputs" / "gog_functions.tsv").write_text("\n".join(rows) + "\n")
        (self.dump / "inputs" / "name_map.tsv").write_text("steam_addr\tname\tsource\tconfidence\tevidence\n")
        rows = ["caller_fn\tcall_site\tcallee_fn\ttype\tinsn"]
        for a, b in self.calls:
            rows.append("%08x\t%08x\t%08x\tCALL\tCALL 0x%08x" % (a, a + 8, b, b))
        (self.dump / "callgraph.tsv").write_text("\n".join(rows) + "\n")
        rows = ["addr\tstring\tref_count\tfunctions"]
        for a, (text, refs) in sorted(self.sstr.items()):
            rows.append("%08x\t%s\t%d\t%s" % (a, text, len(refs), ";".join("%08x:FUN_%08x" % (r, r) for r in refs)))
        (self.dump / "strings.tsv").write_text("\n".join(rows) + "\n")

    def _write_asm(self, a, f):
        d = self.dump / "decomp" / ("%08x" % a)[2:4]
        d.mkdir(parents=True, exist_ok=True)
        if f["thunk_to"]:
            body = ["%08x  e9%s  JMP 0x%08x" % (a, "00000000", f["thunk_to"])]
        else:
            ret = "c3  RET" if f["retn"] == 0 else "c2%02x00  RET 0x%x" % (f["retn"], f["retn"])
            rb, rt = ret.split("  ")
            body = ["%08x  55        PUSH EBP" % a, "%08x  8bec      MOV EBP,ESP" % (a + 1),
                    "%08x  5d        POP EBP" % (a + 3), "%08x  %s  %s" % (a + 4, rb, rt)]
        (d / ("%08x.asm" % a)).write_text("; FN %08x %s\n" % (a, f["name"]) + "\n".join(body) + "\n")
        (d / ("%08x.c" % a)).write_text("// FN %08x %s size=%d ret=%d callers= callees=\n\nvoid %s(void)\n\n{\n  return;\n}\n"
                                        % (a, f["name"], f["size"], f["retn"], f["name"]))

    def _fp_rows(self, fps, thunks=None):
        rows = ["addr\tsize\tretn\tn_insn\tcallconv\tthunk_target\tcallees\tstr_refs\tdisp\timm\tname\tnamespace\tsource"]
        for a in sorted(fps):
            f = fps[a]
            rows.append("\t".join([h(a), str(f.get("size", 48)), str(f["retn"]), "4", "unknown", "", hexlist(f["callees"]),
                                   hexlist(f["strs"]), hexlist(f["disp"]), hexlist(f["imm"]), "", "", "ANALYSIS"]))
        return "\n".join(rows) + "\n"

    def _write_server(self):
        if self.with_gog_fp:
            gdir = self.server / GOG_PROG
            gdir.mkdir(parents=True, exist_ok=True)
            (gdir / "fp.tsv").write_text(self._fp_rows(self.gog_fp))
            (gdir / "strings.tsv").write_text("addr\ttext\n" + "".join(
                "%s\t%s\n" % (h(a), t) for a, (t, _r) in sorted(self.gstr.items())))
            sdir = self.server / STEAM_PROG
            sdir.mkdir(parents=True, exist_ok=True)
            (sdir / "fp.tsv").write_text(self._fp_rows(self.steam_fp))
            (sdir / "strings.tsv").write_text("addr\ttext\n" + "".join(
                "%s\t%s\n" % (h(a), t) for a, (t, _r) in sorted(self.sstr.items())))
        else:
            self.server.mkdir(parents=True, exist_ok=True)

    def _write_exe(self):
        """A tiny PE32 whose only section is .text at RVA 0x100000 (VA 0x500000), raw data at file offset 0x400."""
        if not self.exe_bytes:
            return
        import struct
        text = bytearray(0x20000)
        for va, data in self.exe_bytes.items():
            off = va - 0x500000
            text[off:off + len(data)] = data
        hdr = bytearray(0x400)
        pe = 0x80
        hdr[0:2] = b"MZ"
        struct.pack_into("<I", hdr, 0x3C, pe)
        hdr[pe:pe + 4] = b"PE\0\0"
        struct.pack_into("<HHIIIHH", hdr, pe + 4, 0x14C, 1, 0, 0, 0, 0xE0, 0x102)
        struct.pack_into("<H", hdr, pe + 24, 0x10B)
        struct.pack_into("<I", hdr, pe + 24 + 28, 0x400000)
        struct.pack_into("<8sIIIIIIHHI", hdr, pe + 24 + 0xE0, b".text", len(text), 0x100000, len(text), 0x400,
                         0, 0, 0, 0, 0x60000020)
        (self.dump / "swkotor2_vanilla_6a522e71.exe").write_bytes(bytes(hdr) + bytes(text))

    def _write_mac(self):
        if not self.mac:
            return
        mdir = self.server / bd.MAC_PROGRAM_DEFAULT
        mdir.mkdir(parents=True, exist_ok=True)
        rows = ["addr\tsize\tretn\tn_insn\tcallconv\tthunk_target\tcallees\tstr_refs\tdisp\timm\tname\tnamespace\tsource"]
        for i, (ns, name, disp, imm) in enumerate(self.mac):
            rows.append("\t".join([h(0x1000 + 0x40 * i), "40", "0", "5", "__thiscall", "", "", "", hexlist(disp), hexlist(imm),
                                   name, ns, "ANALYSIS"]))
        (mdir / "fp.tsv").write_text("\n".join(rows) + "\n")

    def _write_dbs(self):
        self.kdb = self.root / "kdb.db"
        self.branchdb = self.root / "branch.db"
        for p in (self.kdb, self.branchdb):
            p.unlink(missing_ok=True)
        con = sqlite3.connect(self.kdb)
        con.executescript("""
            CREATE TABLE binaries(hash TEXT PRIMARY KEY,label TEXT NOT NULL,ghidra_project TEXT,ghidra_program_name TEXT,notes TEXT);
            CREATE TABLE class_methods(binary_hash TEXT NOT NULL,class_name TEXT NOT NULL,vtable_index INTEGER,address TEXT NOT NULL,
                size INTEGER,symbol TEXT,purpose TEXT,notes TEXT,PRIMARY KEY(binary_hash,address));
            CREATE TABLE lessons(id INTEGER PRIMARY KEY AUTOINCREMENT,scope TEXT,topic TEXT,note TEXT,date TEXT);
            CREATE TABLE name_map(steam_addr text primary key,name text,source text,confidence text,evidence text,date text);
        """)
        con.execute("insert into binaries(hash,label) values('6a522e71aaaa','kotor2_steam_aspyr')")
        con.execute("insert into binaries(hash,label) values('777bee23bbbb','kotor2_gog_aspyr')")
        con.execute("insert into lessons(scope,topic,note,date) values('method','existing lesson','n','2026-10-01')")
        for row in self.name_map:
            con.execute("insert into name_map values(?,?,?,?,?,?)", row + ("2026-10-01",))
        con.commit()
        con.close()
        self.branchdb = self.root / "branch.db"
        con = sqlite3.connect(self.branchdb)
        con.execute("CREATE TABLE functions(id INTEGER PRIMARY KEY AUTOINCREMENT,class_name TEXT NOT NULL,function_name TEXT NOT NULL,"
                    "address INTEGER NOT NULL,notes TEXT,calling_convention TEXT,param_size_bytes INTEGER,UNIQUE(class_name,function_name))")
        for cls, fn, addr in self.branch:
            con.execute("insert into functions(class_name,function_name,address) values(?,?,?)", (cls, fn, addr))
        con.commit()
        con.close()
        self.b1_path = self.root / "b1_verified.tsv"
        cols = "kind\tclass\tname\tgog\tsteam_addr\tverdict\tcalling_convention\tparam_size_bytes\tevidence\tcand_orig"
        lines = [cols] + ["F\t%s\t%s\t0x0\t%s\tverified\tthiscall\t0\tfixture\t" % r for r in self.b1]
        lines.append("F\tCRej\tBad\t0x0\t0x00600000\trejected\tthiscall\t0\tnot trusted\t")
        lines += ["F\t%s\t%s\t0x0\t\trejected\tthiscall\t0\twrong candidate\t%s" % (c, n, h(a))
                  for c, n, a in self.b1_rejected]
        self.b1_path.write_text("\n".join(lines) + "\n")

    def _write_mapper(self):
        self.mapper_rows = self.root / "mapper_rows.tsv"
        self.mapper_rows.write_text("".join(
            "0x%010x\t0x%x\t%s\t%s\tretn=%d\n" % (s, g, n, t, r) for s, g, n, t, r in self.mapper))
        self.mapper_stub = self.root / "mapper_stub.py"
        self.mapper_stub.write_text(
            "import os, sys\nout = sys.argv[sys.argv.index('--out') + 1]\n"
            "open(out, 'w').write(open(os.environ['BD_STUB_ROWS']).read())\n")

    def env(self):
        return {"BD_DUMP": str(self.dump), "BD_SERVER": str(self.server), "BD_KDB": str(self.kdb),
                "BD_BRANCHDB": str(self.branchdb), "BD_GOGDB": str(self.root / "none.db"),
                "BD_ROOT": str(self.root / "batches"), "BD_B1_VERIFIED": str(self.b1_path),
                "BD_EXE": str(self.dump / "swkotor2_vanilla_6a522e71.exe"),
                "BD_MAPPER": str(self.mapper_stub), "BD_STUB_ROWS": str(self.mapper_rows)}


def full_world(root, with_gog_fp=True) -> World:
    """The shared scenario used by most tests (see module docstring)."""
    w = World(root, with_gog_fp)
    g = lambda off: GOG_BASE + off      # noqa: E731
    s = lambda off: STEAM_BASE + off    # noqa: E731
    w.pair("CFoo", "Alpha", g(0), s(0), 8, 8)
    w.filler("CFoo", "FillerA", g(0x40), s(0x40))
    w.pair("CFoo", "Beta", g(0x80), s(0x80), 8, 8)
    w.filler("CFoo", "FillerB", g(0xC0), s(0xC0))
    w.pair("CFoo", "Gamma", g(0x100), s(0x100), 4, 1)                  # bool param: GOG 1 byte, ret 4
    # Delta: Steam entry is a thunk to the real function far away
    w.pair("CFoo", "Delta", g(0x140), 0x00520000, 4, 4)
    w.steam_fn(s(0x140), "FUN_%08x" % s(0x140), retn=0, size=5, thunk_to=0x00520000)
    w.steam_fn(0x00520040, "FUN_00520040", retn=4)
    w.pair("CFoo", "Eps", g(0x180), s(0x180), 8, 8)
    w.steam_fn(s(0x1C0), "EpsDecoy", retn=8)                           # same-ret neighbour without content
    w.filler("CFoo", "EpsDecoyG", g(0x1C0), s(0x1C0) + 0x4000, retn=8)  # GOG side filler (its Steam twin is elsewhere)
    w.pair("CFoo", "Zeta", g(0x200), s(0x200), 8, 8)                   # address held by a trusted other name
    w.name_map.append((h(s(0x200)), "COther::Thing", "engine-ai-atlas wave2", "high", "audited"))
    w.pair("CFoo", "Eta", g(0x240), s(0x240), 8, 8)                    # held by an untrusted name
    w.name_map.append((h(s(0x240)), "COther::Stuff", "batch5_reverse_engineering", "high", "projection only"))
    w.gog_fn(g(0x280), "CFoo", "Mu", param=8)                          # GOG retn 8 but the candidate returns 4
    w.fp("gog", g(0x280), 8)
    w.steam_fn(s(0x280), "FUN_%08x" % s(0x280), retn=4)
    w.name_map.append((h(s(0x280)), "CFoo::Mu", "gog_cluster_delta_map.py", "medium", "mapper"))
    w.pair("CFoo", "Theta", g(0x2C0), s(0x2C0), 8, 8, anchors=("STR",))            # C1
    w.pair("CFoo", "Iota", g(0x300), s(0x300), 8, 8, anchors=())                   # C0
    w.pair("CFoo", "Kappa", g(0x340), s(0x340), 8, 8, anchors=("STR",))            # C1
    w.gog_fn(g(0x380), "CFoo", "Lambda", param=0)                                   # no candidate
    w.pair("CFoo", "Nu", g(0x3C0), s(0x3C0), 32, 12)                   # GOG DB param 12 but real ret 32
    # permuted pair: GOG X1, X2 adjacent; Steam P1 and P2 two functions apart
    hA1, hA2, hC1, hC2 = (w.helper() for _ in range(4))
    ga1, gb1 = g(0x500), g(0x580)
    p1, mid, p2, nxt = s(0x500), s(0x540), s(0x580), s(0x5C0)
    w.gog_fn(ga1, "CFoo", "X1", param=8)
    w.gog_fn(gb1, "CFoo", "X2", param=8)
    for a, nm in ((p1, "P1"), (mid, "PMid"), (p2, "P2"), (nxt, "PNext")):
        w.steam_fn(a, nm, retn=8, size=0x30)
    texts = {k: "permutation test text %s unique" % k for k in "ABCD"}
    for i, k in enumerate("ABCD"):
        w.string("gog", 0x00910000 + 0x10 * i, texts[k])
    w.string("steam", 0x00A10000, texts["A"], [p1, p2])
    w.string("steam", 0x00A10010, texts["B"], [p1])
    w.string("steam", 0x00A10020, texts["C"], [p2])
    w.string("steam", 0x00A10030, texts["D"], [p2])
    dispA, immA = [0x2000, 0x2004, 0x2008], [0x7001, 0x7002, 0x7003]
    dispC, immC = dispA + [0x200C], immA + [0x7004]
    w.fp("gog", ga1, 8, {hA1[0], hA2[0]}, {0x00910000, 0x00910010}, dispA, immA)
    w.fp("gog", gb1, 8, {hC1[0], hC2[0]}, {0x00910020, 0x00910030}, dispC, immC)
    w.fp("steam", p1, 8, {hA1[1], hA2[1]}, {0x00A10000, 0x00A10010}, dispA, immA)
    w.fp("steam", p2, 8, {hA1[1], hC1[1], hC2[1]}, {0x00A10000, 0x00A10020, 0x00A10030}, dispC, immC)
    for a, b in ((p1, hA1[1]), (p1, hA2[1]), (p2, hA1[1]), (p2, hC1[1]), (p2, hC2[1])):
        w.calls.append((a, b))
    # mapper rows (steam, gog, name, tag, retn)
    w.mapper = [(s(0), g(0), "CFoo::Alpha", "vote", 8), (s(0x200), g(0x200), "CFoo::Zeta", "vote", 8),
                (s(0x240), g(0x240), "CFoo::Eta", "vote", 8), (s(0x280), g(0x280), "CFoo::Mu", "vote", 8),
                (p2, ga1, "CFoo::X1", "vote", 8), (p1, gb1, "CFoo::X2", "vote", 8)]
    return w


CAND_COLS = batch.CANDIDATE_COLS


def write_candidates(root, name, spec):
    """spec: {fn: candidate cell}; cells are written verbatim; GOG data is looked up from the dump."""
    gog = {r["function"]: r for r in bd.iter_tsv(Path(root) / "dump" / "inputs" / "gog_functions.tsv")}
    rows = []
    for fn, cell in spec.items():
        r = gog[fn]
        rows.append({"class": r["class"], "name": fn, "gog": h(int(r["gog_addr"], 16)), "gog_callconv": r["callconv"],
                     "gog_param": r["param_size_bytes"], "gog_size": "", "gog_retn": "", "candidates": cell})
    batch.write_tsv(Path(root) / "batches" / name / "candidates.tsv", CAND_COLS, rows)


def run(*argv):
    """Run batch.main in-process; -> (exit code, stdout)."""
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        rc = batch.main(list(argv))
    return rc, buf.getvalue()


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


B1_SPEC = {
    "Alpha": "0x00510000|mapper:vote",
    "Beta": "0x00510080|near",
    "Gamma": "0x00510100|name_map:medium:x",
    "Delta": "0x00510140|mapper:drift+0x0",
    "Eps": "0x005101c0|mapper:drift+0x40;0x00510180|near",
    "Zeta": "0x00510200|mapper:vote",
    "Eta": "0x00510240|mapper:vote",
    "Mu": "0x00510280|mapper:vote+name_map:medium:gog_cluster_delta_map.py",
    "Theta": "0x005102c0|near",
    "Iota": "0x00510300|near",
    "Kappa": "0x00510340|near",
    "Lambda": "",
    "Nu": "0x005103c0|near",
    "X1": "0x00510580|mapper:vote",
    "X2": "0x00510500|mapper:vote",
}


class Base(unittest.TestCase):
    """Creates a temp root and patches the BD_* environment for the test."""

    with_gog_fp = True

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.world = full_world(self.root, self.with_gog_fp).build()
        patcher = mock.patch.dict(os.environ, self.world.env())
        patcher.start()
        self.addCleanup(patcher.stop)

    def evidence(self, name_batch="B1"):
        rows = list(bd.iter_tsv(self.root / "batches" / name_batch / "evidence.tsv"))
        return {r["name"]: r for r in rows}


# ------------------------------------------------------------------------------------------------ trust tiers
class TrustTests(unittest.TestCase):
    def test_unaudited_sources_are_not_trusted(self):
        for src in ("batch5_reverse_engineering", "batch8_reverse_engineering", "batch2_item_partytable",
                    "gog_creaturestats_cluster", "gog_partytable_block", "gog_cluster_delta_map.py"):
            self.assertFalse(bd.is_trusted(src, "high"), src)
        self.assertTrue(bd.is_unaudited_source("batch5_reverse_engineering"))

    def test_audited_sources_and_routes(self):
        self.assertTrue(bd.is_trusted("engine-ai-atlas wave2", "high"))
        self.assertFalse(bd.is_trusted("engine-ai-atlas wave2", "medium"))
        self.assertFalse(bd.is_trusted("engine-ai-atlas wave2", "wrong"))
        self.assertTrue(bd.is_trusted("level-up-P1-decompile-confirmed", "high"))
        # an evidence text in the confidence column (old column swap) counts as medium
        self.assertEqual(bd.norm_conf("phase3_evidence/G1_dec_create_server_401bc0.txt"), "medium")
        self.assertFalse(bd.is_trusted("level-up-P5-static-confirmed", "some evidence text"))
        # explicit routes win over the source test
        self.assertTrue(bd.is_trusted("batch5_reverse_engineering", "high", b1_verified=True))
        self.assertTrue(bd.is_trusted("batch5_reverse_engineering", "high", later_batch_verified=True))

    def test_b1_only_source_is_trusted_only_through_the_b1_route(self):
        self.assertIn("steam-db-pr3-phase2", bd.B1_ONLY_SOURCES)
        self.assertFalse(bd.is_trusted("steam-db-pr3-phase2", "high"))
        self.assertTrue(bd.is_trusted("steam-db-pr3-phase2", "high", b1_verified=True))

    def test_crt_addresses(self):
        for a in (0x9196FD, 0x91D288, 0x9196EE, 0x9215E6, 0x919723, 0x91C6AA):
            self.assertIn(a, bd.CRT_ADDRS)

    def test_b1_default_is_the_committed_head_file(self):
        with mock.patch.dict(os.environ, {}, clear=False):
            os.environ.pop("BD_B1_VERIFIED", None)
            self.assertEqual(bd.get_paths().b1_verified.name, "b1_verified_head.tsv")
            self.assertEqual(bd.get_paths().b1_verified.parent.name, "gold")


# ------------------------------------------------------------------------------------------------ gates
class GateTests(Base):
    @classmethod
    def setUpClass(cls):
        cls._tmp = tempfile.TemporaryDirectory()
        cls.root = Path(cls._tmp.name)
        cls.world = full_world(cls.root).build()
        cls.patch = mock.patch.dict(os.environ, cls.world.env())
        cls.patch.start()
        write_candidates(cls.root, "B1", B1_SPEC)
        rc, cls.out = run("gates", "B1")
        assert rc == 0
        cls.ev = {r["name"]: r for r in bd.iter_tsv(cls.root / "batches" / "B1" / "evidence.tsv")}

    @classmethod
    def tearDownClass(cls):
        cls.patch.stop()
        cls._tmp.cleanup()

    def setUp(self):   # the shared world is built once; skip Base.setUp
        pass

    def test_good_row_is_auto(self):
        e = self.ev["Alpha"]
        self.assertEqual(e["gate_class"], "AUTO", e)
        self.assertEqual((e["g1"], e["g2"], e["g3"], e["g4"], e["g6"]), ("pass",) * 5)
        self.assertEqual(e["families"], "STR,CALLEE,SIG")
        self.assertEqual(e["content_level"], "C2")
        self.assertEqual(e["steam_addr"], "0x00510000")
        self.assertIn("STR 'unique anchor string", e["evidence"])
        self.assertIn("CALLEE 2", e["evidence"])

    def test_run_consistency_chains_through_auto_neighbours(self):
        self.assertEqual(self.ev["Beta"]["gate_class"], "AUTO")
        self.assertEqual(self.ev["Beta"]["run_ok"], "yes")
        self.assertEqual(self.ev["Beta"]["mapper_tag"], "")

    def test_same_ret_neighbour_decoy_is_not_picked_and_not_auto(self):
        e = self.ev["Eps"]
        self.assertEqual(e["steam_addr"], "0x00510180", "real function must win over the same-ret neighbour")
        self.assertEqual(e["gate_class"], "AUTO")
        self.assertIn("0x005101c0|mapper:drift+0x40", e["cand_sources"])
        self.assertEqual(e["cand_status"], "changed")           # the original (mapper) candidate was the decoy
        # decoy as the only candidate: no content anchor, never AUTO
        write_candidates(self.root, "B4", {"Eps": "0x005101c0|mapper:vote"})
        run("gates", "B4")
        d = self.evidence("B4")["Eps"]
        self.assertEqual(d["gate_class"], "REVIEW")
        self.assertEqual(d["content_level"], "C0")
        self.assertEqual(d["families"], "")
        self.assertIn("c0", d["flags"])
        # the decoy score of the neighbour is recorded for the card
        self.assertTrue(d["decoy_best"].startswith("0x00510180:"), d["decoy_best"])

    def test_bool_param_passes_g4_metadata(self):
        e = self.ev["Gamma"]
        self.assertEqual(e["gog_param_size"], "1")
        self.assertEqual((e["gog_retn"], e["steam_retn"]), ("4", "4"))
        self.assertEqual(e["g4"], "pass")
        self.assertNotIn("gog_db_param_bad", e["flags"])
        self.assertEqual(e["gate_class"], "AUTO")

    def test_bad_gog_db_param_is_flagged_not_failed(self):
        e = self.ev["Nu"]
        self.assertEqual(e["g4"], "pass")
        self.assertIn("gog_db_param_bad", e["flags"])
        self.assertEqual(e["param_size_bytes"], "32")           # the Steam retn, not the GOG metadata
        report = (self.root / "batches" / "B1" / "gate_report.md").read_text()
        self.assertIn("gog_db_param_bad", report)

    def test_thunk_resolved_to_target(self):
        e = self.ev["Delta"]
        self.assertEqual(e["steam_addr"], "0x00510140")
        self.assertEqual(e["g5_target"], "0x00520000")
        self.assertEqual(e["g4"], "pass")                       # the thunk itself returns 0, the target 4
        self.assertEqual(e["steam_retn"], "4")
        self.assertEqual(e["families"], "STR,CALLEE,SIG")       # anchors come from the target
        self.assertEqual(e["gate_class"], "AUTO")

    def test_duplicate_held_by_trusted_name_is_rejected(self):
        e = self.ev["Zeta"]
        self.assertEqual(e["gate_class"], "REJECT")
        self.assertTrue(e["g6"].startswith("fail:trusted_holder=COther::Thing"), e["g6"])

    def test_duplicate_held_by_untrusted_name_goes_to_review(self):
        e = self.ev["Eta"]
        self.assertEqual(e["gate_class"], "REVIEW")
        self.assertTrue(e["g6"].startswith("review:held_by=COther::Stuff"), e["g6"])
        self.assertIn("g6", e["flags"])
        report = (self.root / "batches" / "B1" / "gate_report.md").read_text()
        self.assertIn("COther::Stuff", report)

    def test_g4_mismatch_is_rejected(self):
        e = self.ev["Mu"]
        self.assertEqual(e["gate_class"], "REJECT")
        self.assertTrue(e["g4"].startswith("fail:retn"), e["g4"])

    def test_permuted_same_size_pair_is_review_not_auto(self):
        e = self.ev["X1"]
        self.assertEqual(e["mapper_tag"], "vote")
        self.assertEqual(e["content_level"], "C2", "the swapped candidate itself looks convincing")
        self.assertEqual(e["gate_class"], "REVIEW")
        self.assertEqual(e["run_ok"], "permuted")
        self.assertIn("permuted", e["flags"])

    def test_unpermuted_control_is_auto(self):
        write_candidates(self.root, "B2", {"X1": "0x00510500|mapper:vote", "X2": "0x00510580|mapper:vote"})
        run("gates", "B2")
        ev = self.evidence("B2")
        self.assertEqual(ev["X1"]["gate_class"], "AUTO", ev["X1"])
        self.assertEqual(ev["X2"]["gate_class"], "AUTO", ev["X2"])

    def test_content_levels_and_nocand(self):
        self.assertEqual(self.ev["Theta"]["content_level"], "C1")
        self.assertEqual(self.ev["Theta"]["gate_class"], "REVIEW")
        self.assertEqual(self.ev["Iota"]["content_level"], "C0")
        self.assertEqual(self.ev["Lambda"]["gate_class"], "NOCAND")
        self.assertEqual(self.ev["Lambda"]["steam_addr"], "")

    def test_evidence_columns_and_report(self):
        path = self.root / "batches" / "B1" / "evidence.tsv"
        self.assertEqual(path.read_text().split("\n")[0].split("\t"), batch.EVIDENCE_COLS)
        self.assertEqual(batch.EVIDENCE_COLS[:15], [
            "kind", "class", "name", "gog", "steam_addr", "verdict", "calling_convention", "param_size_bytes", "evidence",
            "cand_orig", "cand_status", "content_level", "wrapper_issue", "steam_size", "gog_param_size"])
        report = (self.root / "batches" / "B1" / "gate_report.md").read_text()
        for section in ("## Counts per class and gate class", "## REJECT rows", "## Duplicates and conflicts (G6)",
                        "## G4 metadata flags", "## Lessons"):
            self.assertIn(section, report)
        self.assertIn("COther::Thing", report)


class B1OnlyTrustTests(Base):
    """name_map rows from steam-db-pr3-phase2 count only when the B1 gold file verifies them."""

    def test_row_not_in_b1_is_untrusted_and_row_in_b1_is_trusted(self):
        w = self.world
        w.name_map.append((h(0x00510080), "CFoo::Beta", "steam-db-pr3-phase2", "high", "Option A rewrite"))   # not in B1
        w.name_map.append((h(0x00510000), "CFoo::Alpha", "steam-db-pr3-phase2", "high", "audited"))
        w.b1.append(("CFoo", "Alpha", h(0x00510000)))                                                       # in B1
        w._write_dbs()
        corpus = bd.Corpus(bd.get_paths())
        beta, alpha = corpus.gog_by_name["CFoo::Beta"], corpus.gog_by_name["CFoo::Alpha"]
        self.assertFalse(corpus.trust.is_trusted(0x00510080))
        self.assertNotIn(beta.addr, corpus.pairs)
        self.assertTrue(corpus.trust.is_trusted(0x00510000, "CFoo::Alpha"))
        self.assertEqual(corpus.pairs[alpha.addr], 0x00510000)
        # an untrusted holder only sends a conflicting candidate to REVIEW, never REJECT
        write_candidates(self.root, "B1", {"Zeta": "0x00510080|mapper:vote"})
        run("gates", "B1")
        self.assertEqual(self.evidence()["Zeta"]["gate_class"], "REVIEW")
        self.assertTrue(self.evidence()["Zeta"]["g6"].startswith("review:held_by=CFoo::Beta"))


class ViewTests(Base):
    """VT is informational: it never counts toward C1/C2."""

    def _ce(self, counts):
        ce = batch.CandEval(0x00510000, [])
        ce.counts = dict(counts)
        return ce

    def test_vt_alone_is_c0_and_vt_plus_one_family_is_c1(self):
        self.assertEqual(self._ce({"VT": True}).n_fam, 0)
        self.assertEqual(self._ce({"VT": True, "SIG": True}).n_fam, 1)
        self.assertEqual(self._ce({"VT": True, "SIG": True, "CALLEE": True}).n_fam, 2)
        self.assertEqual(self._ce({"VT": True, "SIG": True}).families, ["SIG", "VT"])    # still reported

    def test_vt_row_in_gates_is_not_c1_and_never_auto(self):
        con = sqlite3.connect(self.world.kdb)
        con.execute("insert into class_methods(binary_hash,class_name,vtable_index,address,symbol) "
                    "values('6a522e71aaaa','CFoo',3,'00510100','Gamma')")
        con.commit()
        con.close()
        write_candidates(self.root, "B1", {"Gamma": "0x00510100|mapper:vote"})
        run("gates", "B1")
        e = self.evidence()["Gamma"]
        self.assertIn("VT", e["families"])            # reported, but Gamma's C2 comes from STR+CALLEE+SIG
        self.assertEqual(e["content_level"], "C2")
        # a VT-only function:
        self.world.pair("CFoo", "Vt1", GOG_BASE + 0x700, STEAM_BASE + 0x700, 8, 8, anchors=())
        self.world.build()
        con = sqlite3.connect(self.world.kdb)
        con.execute("insert into class_methods(binary_hash,class_name,vtable_index,address,symbol) "
                    "values('6a522e71aaaa','CFoo',4,'00510700','Vt1')")
        con.commit()
        con.close()
        write_candidates(self.root, "B1", {"Vt1": "0x00510700|mapper:vote"})
        run("gates", "B1")
        e = self.evidence()["Vt1"]
        self.assertEqual(e["families"], "VT")
        self.assertEqual(e["content_level"], "C0")
        self.assertEqual(e["gate_class"], "REVIEW")
        self.assertIn("c0", e["flags"])


class UntrustedPairTests(Base):
    def test_projection_only_rows_are_not_trusted_pairs(self):
        """A name_map row from batch5_* (confidence high) must not become a trusted GOG->Steam pair."""
        self.world.name_map.append((h(0x00510080), "CFoo::Beta", "batch5_reverse_engineering", "high", "projection"))
        self.world.name_map.append((h(0x00510000), "CFoo::Alpha", "engine-ai-atlas wave2", "high", "audited"))
        self.world._write_dbs()
        corpus = bd.Corpus(bd.get_paths(), exclude_batch="B1")
        alpha, beta = corpus.gog_by_name["CFoo::Alpha"], corpus.gog_by_name["CFoo::Beta"]
        self.assertIn(alpha.addr, corpus.pairs)
        self.assertNotIn(beta.addr, corpus.pairs)
        self.assertFalse(corpus.trust.is_trusted(0x00510080))
        self.assertIn(h(0x00510080), [h(r.addr) for r in corpus.names_by_base["CFoo::Beta"]])   # still a candidate
        h0 = corpus.gog_by_name["CHelper::H0"]
        self.assertIn(h0.addr, corpus.pairs)                       # B1 `verified` rows are trusted
        self.assertFalse(corpus.trust.is_trusted(0x00600000))      # B1 `rejected` rows are not


class MissingFingerprintTests(Base):
    with_gog_fp = False

    def test_missing_gog_fingerprint_degrades_to_review(self):
        write_candidates(self.root, "B1", {"Alpha": "0x00510000|mapper:vote", "Beta": "0x00510080|near"})
        rc, out = run("gates", "B1")
        self.assertEqual(rc, 0)
        ev = self.evidence()
        for name in ("Alpha", "Beta"):
            e = ev[name]
            self.assertEqual(e["gate_class"], "REVIEW", e)
            self.assertEqual((e["a_str"], e["a_callee"], e["a_sig"]), ("n/a", "n/a", "n/a"))
            self.assertEqual(e["g4"], "n/a")
        self.assertIn("NOTE: no GOG fingerprints", (self.root / "batches" / "B1" / "gate_report.md").read_text())
        rc, out = run("cards", "B1")
        self.assertEqual(rc, 0)


# ------------------------------------------------------------------------------------------------ candidates / cards
class CandidatesTests(Base):
    def test_candidates_collects_all_sources(self):
        self.world.branch.append(("CFoo", "Beta", 0x00510080))
        self.world._write_dbs()
        # trusted CFoo row so run_delta comes from it, plus the mapper stub for Alpha only
        self.world.name_map.append((h(0x00510000), "CFoo::Alpha", "engine-ai-atlas wave2", "high", "audited"))
        self.world._write_dbs()
        rc, out = run("candidates", "B1", "--classes", "CFoo", "--no-reexport",
                      "--findings-dir", str(self.root / "findings"))
        self.assertEqual(rc, 0, out)
        rows = {r["name"]: r for r in bd.iter_tsv(self.root / "batches" / "B1" / "candidates.tsv")}
        self.assertEqual(list(next(iter(rows.values())).keys()), CAND_COLS)
        alpha = batch.parse_candidates(rows["Alpha"]["candidates"])
        srcs = dict(alpha)
        self.assertIn("mapper:vote", srcs[0x00510000])
        self.assertTrue(any(s.startswith("name_map:high:engine-ai-atlas_wave2") for s in srcs[0x00510000]))
        beta = dict(batch.parse_candidates(rows["Beta"]["candidates"]))
        self.assertIn("branch", beta[0x00510080])
        self.assertIn("near", beta[0x00510080])                 # ranked first by fingerprint score
        self.assertLessEqual(sum(1 for s in beta.values() if "near" in s), 3)
        self.assertEqual(rows["Gamma"]["gog_param"], "1")
        self.assertEqual(rows["Gamma"]["gog_retn"], "4")       # from the GOG fingerprint
        self.assertTrue((self.root / "findings" / "B1_map_CFoo.tsv").exists())

    def test_from_branch_lists_branch_address_first(self):
        self.world.branch.append(("CFoo", "Beta", 0x00510040))   # wrong branch row (the neighbour)
        self.world._write_dbs()
        rc, out = run("candidates", "B0", "--from-branch", "--classes", "CFoo", "--no-reexport",
                      "--findings-dir", str(self.root / "findings"))
        self.assertEqual(rc, 0, out)
        rows = list(bd.iter_tsv(self.root / "batches" / "B0" / "candidates.tsv"))
        self.assertEqual([r["name"] for r in rows], ["Beta"])
        first = batch.parse_candidates(rows[0]["candidates"])[0]
        self.assertEqual(first[0], 0x00510040)
        self.assertEqual(first[1][0], "branch")

    @unittest.skipUnless((bd.get_paths().gogdb).exists() and
                         bd.tool_path("BD_MAPPER_REAL", "ghidra_knowledge/gog_cluster_delta_map.py").exists(),
                         "real GOG DB not present")
    def test_real_mapper_selftest_still_passes(self):
        import subprocess
        mapper = bd.tool_path("BD_MAPPER_REAL", "ghidra_knowledge/gog_cluster_delta_map.py")
        res = subprocess.run([sys.executable, str(mapper), "--selftest"], capture_output=True, text=True)
        self.assertEqual(res.returncode, 0, res.stdout[-600:])


class LibraryNameTests(Base):
    def test_library_looking_names(self):
        w = self.world
        for off, name, src in ((0x700, "Statistics", "ANALYSIS"), (0x740, "__tsopen_nolock", "ANALYSIS"),
                               (0x780, "Catch_All@00510780", "ANALYSIS")):
            w.pair("CFoo", "Lib%x" % off, GOG_BASE + off, STEAM_BASE + off, 8, 8)
            w.steam[STEAM_BASE + off]["name"], w.steam[STEAM_BASE + off]["source"] = name, src
        w.build()
        write_candidates(self.root, "B1", {"Lib700": "0x00510700|mapper:vote", "Lib740": "0x00510740|mapper:vote",
                                           "Lib780": "0x00510780|mapper:vote"})
        run("gates", "B1")
        ev = self.evidence()
        self.assertEqual(ev["Lib700"]["g1"], "warn:autolabel")          # FID-style label on a game function
        self.assertEqual(ev["Lib700"]["gate_class"], "REVIEW")
        self.assertTrue(ev["Lib740"]["g1"].startswith("fail:library"))   # CRT-style name
        self.assertEqual(ev["Lib740"]["gate_class"], "REJECT")
        self.assertEqual(ev["Lib780"]["gate_class"], "REJECT")


class ExeFallbackTests(Base):
    """G3 reads the first bytes from the exe copy when a function has no .asm (section table mapping)."""

    def test_g3_from_exe(self):
        w = self.world
        for off, name, code in ((0x600, "Exe1", b"\x55\x8b\xec" + b"\x90" * 13),      # push ebp; mov ebp,esp
                                (0x640, "Exe2", b"\x90\x90\x90\x90" + b"\xcc" * 12),   # no standard prologue
                                (0x680, "Exe3", None)):                                  # not mapped by any section
            va = STEAM_BASE + off if code is not None else 0x00300000 + off
            w.pair("CFoo", name, GOG_BASE + off, va, 8, 8)
            w.steam[va]["asm"] = False
            if code is not None:
                w.exe_bytes[va] = code
        w.build()
        os.environ.update(w.env())
        write_candidates(self.root, "B1", {"Exe1": "0x00510600|mapper:vote", "Exe2": "0x00510640|mapper:vote",
                                           "Exe3": "0x00300680|mapper:vote"})
        run("gates", "B1")
        ev = self.evidence()
        self.assertEqual(ev["Exe1"]["g3"], "pass(exe)")
        self.assertEqual(ev["Exe1"]["gate_class"], "AUTO", ev["Exe1"])      # pass(exe) counts as a pass
        self.assertEqual(ev["Exe2"]["g3"], "fail:no_standard_prologue(exe)")
        self.assertEqual(ev["Exe2"]["gate_class"], "REVIEW")
        self.assertEqual(ev["Exe3"]["g3"], "n/a")
        self.assertIn("g3", ev["Exe3"]["flags"])

    def test_pe_image_maps_through_the_section_table(self):
        w = self.world
        w.exe_bytes[0x00510010] = b"\xde\xad\xbe\xef"
        w.build()
        pe = bd.PeImage(w.dump / "swkotor2_vanilla_6a522e71.exe")
        self.assertEqual(pe.read(0x00510010, 4), b"\xde\xad\xbe\xef")
        self.assertIsNone(pe.read(0x00401000, 4))


class UntrustTests(Base):
    def test_untrust_file_removes_trust_for_the_run(self):
        write_candidates(self.root, "B1", {"Alpha": "0x00510000|near"})
        run("gates", "B1")
        self.assertEqual(self.evidence()["Alpha"]["a_callee"], "2")         # helpers H0 and H1 are trusted callees
        unt = self.root / "untrust.txt"
        unt.write_text("# comment\n0x00500000\n\n")                       # Steam address of helper H0
        rc, out = run("gates", "B1", "--untrust", str(unt))
        self.assertIn("untrusted 1 addresses", out)
        self.assertEqual(self.evidence()["Alpha"]["a_callee"], "1")
        # library-level view: removed from the trust index and from pairs; other rows stay trusted
        corpus = bd.Corpus(bd.get_paths(), untrust={0x00500000})
        h0 = corpus.gog_by_name["CHelper::H0"]
        self.assertNotIn(h0.addr, corpus.pairs)
        self.assertFalse(corpus.trust.is_trusted(0x00500000))
        self.assertTrue(corpus.trust.is_trusted(0x00500040))
        corpus.apply_untrust(set())
        self.assertIn(h0.addr, corpus.pairs)


class ReexportTests(Base):
    def test_reexport_runs_only_when_name_map_is_stale(self):
        marker = self.root / "exporter_ran"
        stub = self.root / "exporter_stub.py"
        stub.write_text("import sys\nopen(%r, 'w').write(' '.join(sys.argv[1:]))\n" % str(marker))
        nm = self.world.dump / "inputs" / "name_map.tsv"
        args = ("candidates", "B1", "--classes", "CFoo", "--findings-dir", str(self.root / "findings"))
        with mock.patch.dict(os.environ, {"BD_EXPORTER": str(stub)}):
            os.utime(nm, (1, 1))                                  # older than the KDB
            run(*args, "--no-reexport")
            self.assertFalse(marker.exists(), "--no-reexport must never run the exporter")
            run(*args)
            self.assertTrue(marker.exists())
            self.assertIn("--root %s" % self.world.dump, marker.read_text())
            marker.unlink()
            os.utime(nm, None)                                    # fresh: no re-export
            run(*args)
            self.assertFalse(marker.exists())


class FamilyTests(Base):
    def test_server_name_and_class_methods_families(self):
        sf = self.world.root / "server" / bd.STEAM_PROGRAM_DEFAULT
        sf.mkdir(parents=True, exist_ok=True)
        (sf / "swkotor2.exe_functions.tsv").write_text(
            "addr\tsize\tretn\tname\tnamespace\tcallconv\tprototype\tsource\tthunk\n"
            "0x00510080\t48\t8\tBeta\tCFoo\t__thiscall\tvoid Beta()\tUSER_DEFINED\tfalse\n"
            "0x00510000\t48\t8\tAlpha\tCFoo\t__thiscall\tvoid Alpha()\tANALYSIS\tfalse\n")
        con = sqlite3.connect(self.world.kdb)
        con.execute("insert into class_methods(binary_hash,class_name,vtable_index,address,symbol) "
                    "values('6a522e71aaaa','CFoo',3,'00510100','Gamma')")
        con.commit()
        con.close()
        corpus = bd.Corpus(bd.get_paths())
        scorer = bd.Scorer(corpus)
        beta, alpha, gamma = (corpus.gog_by_name["CFoo::" + n] for n in ("Beta", "Alpha", "Gamma"))
        self.assertTrue(scorer.a_srv(beta, 0x00510080).ok)
        self.assertFalse(scorer.a_srv(alpha, 0x00510000).ok, "ANALYSIS-sourced server names are not trusted")
        self.assertFalse(scorer.a_srv(beta, 0x00510000).ok)
        self.assertTrue(scorer.a_vt(gamma, 0x00510100).ok)
        self.assertFalse(scorer.a_vt(gamma, 0x00510080).ok)
        write_candidates(self.root, "B1", {"Beta": "0x00510080|near"})
        run("gates", "B1")
        self.assertIn("SRV", self.evidence()["Beta"]["families"])


class SigRouteTests(Base):
    """`auto_route=sig`: SIG-only rows reach AUTO under strict conditions and never seed other rows' run_ok."""

    def build_rows(self):
        w = self.world
        g = lambda off: GOG_BASE + off      # noqa: E731
        s = lambda off: STEAM_BASE + off    # noqa: E731
        w.pair("CFoo", "SgSeed", g(0x1000), s(0x1000), 8, 8)                              # c2 route (with mapper:vote)
        w.pair("CFoo", "Sg1", g(0x1040), s(0x1040), 8, 8, anchors=("SIG",))               # sig route, seeded by SgSeed
        w.pair("CFoo", "Sg3", g(0x1080), s(0x1080), 8, 8, anchors=("SIG",))               # decoy neighbour too similar
        w.pair("CFoo", "Sg4", g(0x10C0), s(0x10C0), 8, 8, anchors=("SIG",), size=0x38)    # steam size != gog size
        for k in range(9):                                                                  # 9 fillers: Sg2 is 12 GOG
            w.filler("CFoo", "Fill%d" % k, g(0x1100 + 0x40 * k), s(0x1100 + 0x40 * k))      # functions after Sg1
        w.pair("CFoo", "Sg2", g(0x1340), s(0x1340), 8, 8, anchors=("SIG",))               # only Sg1 could seed it
        # Dec3: steam-only next neighbour of Sg3 whose operand sets nearly equal Sg3's (SIG 0.9 against our 1.0)
        sg3 = w.steam_fp[s(0x1080)]
        w.steam_fn(s(0x10B0), "FUN_%08x" % s(0x10B0), retn=8)
        w.fp("steam", s(0x10B0), 8, (), (), sg3["disp"] + [0x9999], sg3["imm"])
        w.build()
        write_candidates(self.root, "B1", {
            "SgSeed": "0x00511000|mapper:vote", "Sg1": "0x00511040|near", "Sg3": "0x00511080|near",
            "Sg4": "0x005110c0|near", "Sg2": "0x00511340|near"})
        run("gates", "B1")
        return self.evidence()

    def test_sig_route_reaches_auto_and_is_tagged(self):
        ev = self.build_rows()
        self.assertEqual(ev["SgSeed"]["gate_class"], "AUTO")
        self.assertIn("auto_route=c2", ev["SgSeed"]["flags"])
        e = ev["Sg1"]
        self.assertEqual((e["families"], e["run_ok"], e["gate_class"]), ("SIG", "yes", "AUTO"), e)
        self.assertIn("auto_route=sig", e["flags"])
        self.assertEqual(e["a_sig"], "1.00")
        report = (self.root / "batches" / "B1" / "gate_report.md").read_text()
        self.assertIn("AUTO by route: c2=1 sig=1", report)

    def test_sig_route_does_not_seed_its_neighbour(self):
        ev = self.build_rows()
        e = ev["Sg2"]
        self.assertEqual(e["families"], "SIG")
        self.assertEqual(e["a_sig"], "1.00")
        self.assertEqual(e["gate_class"], "REVIEW", "Sg2 would only be run_ok through the sig-route row Sg1")
        self.assertNotEqual(e["run_ok"], "yes")

    def test_margin_and_size_rules_are_enforced(self):
        ev = self.build_rows()
        self.assertEqual(ev["Sg3"]["families"], "SIG")           # still counts: 1.0 > 0.9 ...
        self.assertEqual(ev["Sg3"]["gate_class"], "REVIEW")      # ... but the best decoy is within 0.25
        self.assertIn("0x005110b0:", ev["Sg3"]["decoy_best"])
        self.assertEqual(ev["Sg4"]["families"], "SIG")
        self.assertEqual((ev["Sg4"]["steam_size"], ev["Sg4"]["gog_size"]), ("56", "48"))
        self.assertEqual(ev["Sg4"]["gate_class"], "REVIEW")      # sizes differ

    def test_vote_does_not_help_the_sig_route(self):
        ev = self.build_rows()
        write_candidates(self.root, "B2", {"Sg2": "0x00511340|mapper:vote"})
        run("gates", "B2")
        self.assertEqual(self.evidence("B2")["Sg2"]["gate_class"], "REVIEW")


class DecoyMixTests(Base):
    """Real batches always mix >= 10 decoy cards per chunk; the decoy labels live only in key.tsv."""

    def test_default_chunk_carries_at_least_ten_decoys_of_review_rows(self):
        write_candidates(self.root, "B1", B1_SPEC)
        run("gates", "B1")
        rc, out = run("cards", "B1")
        self.assertEqual(rc, 0, out)
        cards = self.root / "batches" / "B1" / "cards"
        key = list(bd.iter_tsv(cards / "key.tsv"))
        decoys = [k for k in key if k["kind"].startswith("decoy_")]
        self.assertGreaterEqual(len(decoys), batch.MIN_DECOYS)
        self.assertTrue(all(k["expected"] == "mismatch" for k in decoys))
        review = {e["gog"]: e for e in bd.iter_tsv(self.root / "batches" / "B1" / "evidence.tsv")
                  if e["gate_class"] == "REVIEW"}
        for k in decoys:                                         # every decoy hangs off a REVIEW row ...
            host = review[k["gog"]]
            self.assertNotEqual(k["steam_addr"], host["steam_addr"])
            f = bd.load_steam_functions(bd.get_paths().dump)
            self.assertEqual(f[int(k["steam_addr"], 16)].retn, f[int(host["steam_addr"], 16)].retn)   # ... same retn
        chunk = (cards / "chunk_1.md").read_text()
        self.assertEqual(chunk.count("## Card "), len(key))
        for label in ("decoy_prev", "decoy_next", "mismatch\t"):
            self.assertNotIn(label, chunk)                       # labels only in the key
        self.assertNotIn("expected", (cards / "chunk_1.md").read_text().split("## Card ")[1].split("###")[0])


class CardTests(Base):
    def test_cards_and_key(self):
        write_candidates(self.root, "B1", B1_SPEC)
        run("gates", "B1")
        rc, out = run("cards", "B1", "--chunk", "10", "--decoys", "2")
        self.assertEqual(rc, 0, out)
        chunk = (self.root / "batches" / "B1" / "cards" / "chunk_1.md").read_text()
        self.assertIn("DECOY NOTE", chunk)
        self.assertIn("Stack Slot Match", chunk)
        self.assertIn("### annotate", chunk)
        key = list(bd.iter_tsv(self.root / "batches" / "B1" / "cards" / "key.tsv"))
        self.assertTrue(any(k["kind"] in ("decoy_prev", "decoy_next") and k["expected"] == "mismatch" for k in key))
        self.assertNotIn("\tmismatch", chunk)                    # the answer key is never inside the cards
        # GOG-side section sits before ### abi, stays compact and shows callee mapping + operand intersection
        sections = chunk.split("### GOG side (from the server GOG fingerprint)")[1:]
        self.assertEqual(len(sections), chunk.count("## Card "))
        eta = next(c for c in chunk.split("## Card ")[1:] if "Candidate Steam address: 0x00510240" in c)
        self.assertLess(eta.index("### GOG side"), eta.index("### abi"))
        side = eta.split("### GOG side (from the server GOG fingerprint)")[1].split("### abi")[0].strip("\n").split("\n")
        self.assertLessEqual(len(side) + 1, 25)
        text = "\n".join(side)
        self.assertRegex(text, r"0x[0-9a-f]{8} CHelper::H\d+ -> Steam 0x0050[0-9a-f]{4}  in candidate callees: yes")
        self.assertIn("GOG strings (1): 'unique anchor string", text)
        self.assertRegex(text, r"disp sets \(\* = in both, 4 common\): GOG \{0x[0-9a-f]+\*")
        n_cards = chunk.count("## Card ")
        self.assertEqual(n_cards, len([k for k in key if k["chunk"] == "1"]))


# ------------------------------------------------------------------------------------------------ finalize / persist
def write_reviews_and_overrides(root):
    bdir = Path(root) / "batches" / "B1"
    (bdir / "review").mkdir(parents=True, exist_ok=True)
    cols = "gog_addr\tsteam_addr\tverdict\tcorrected_addr\tcontent_anchor\tconfidence\treason\n"
    (bdir / "review" / "haiku.tsv").write_text(cols + "".join(
        "\t".join(r) + "\n" for r in [
            (h(GOG_BASE + 0x2C0), "0x005102c0", "match", "", "string 'unique anchor' at 00a00040", "4", "string xref"),
            (h(GOG_BASE + 0x300), "0x00510300", "mismatch", "", "", "2", "different callees"),
            (h(GOG_BASE + 0x340), "0x00510340", "match", "", "", "3", "looks right"),      # no anchor
            (h(GOG_BASE + 0x240), "0x00510240", "match", "", "string at 00a00000", "4", "ok"),
            (h(GOG_BASE + 0x80), "0x00510080", "label_suspect", "", "00510084 CALL 0x00500040", "4",
             "body dismisses popups, fits RemoveGUIPopUps"),                                  # AUTO row, flagged by review
        ]))
    (bdir / "overrides.tsv").write_text(
        "gog_addr\tsteam_addr\tfinal_verdict\tnote\n"
        "%s\t0x00510340\trejected\tOpus: wrong function\n"
        "%s\t0x00510200\tverified\tOpus: gate was wrong, name_map holder is a mapper row\n"
        "%s\t0x005103c0\tlabel_suspect\tOpus: body is a setter, name says getter\n" % (
            h(GOG_BASE + 0x340), h(GOG_BASE + 0x200), h(GOG_BASE + 0x3C0)))


class FinalizePersistTests(Base):
    def setUp(self):
        super().setUp()
        write_candidates(self.root, "B1", B1_SPEC)
        run("gates", "B1")
        write_reviews_and_overrides(self.root)
        # a lesson for persist
        rep = self.root / "batches" / "B1" / "gate_report.md"
        text = rep.read_text().replace(batch.LESSONS_PLACEHOLDER, "- method | fixture lesson | note one\n"
                                       "- method | existing lesson | already present")
        rep.write_text(text)
        rc, self.fin_out = run("finalize", "B1", "--date", "2026-10-03")
        self.assertEqual(rc, 0, self.fin_out)
        self.final = {r["name"]: r for r in bd.iter_tsv(self.root / "batches" / "B1" / "final_rows.tsv")}

    def test_finalize_merges_reviews_and_overrides(self):
        f = self.final
        self.assertEqual(f["Alpha"]["final_verdict"], "verified")              # AUTO
        self.assertEqual(f["Theta"]["final_verdict"], "verified")              # REVIEW + anchor + C1
        self.assertEqual(f["Iota"]["final_verdict"], "unresolved")             # mismatch, no correction
        self.assertEqual(f["Kappa"]["final_verdict"], "rejected")              # override beats reviewer match
        self.assertIn("Opus: wrong function", f["Kappa"]["final_note"])
        self.assertEqual(f["Eta"]["final_verdict"], "verified")                # review match + anchor, C2 despite G6 review
        self.assertEqual(f["Zeta"]["final_verdict"], "verified")               # override of a REJECT
        self.assertEqual(f["Mu"]["final_verdict"], "rejected")                 # gate REJECT
        self.assertEqual(f["X1"]["final_verdict"], "unresolved")               # REVIEW without a reviewer
        self.assertEqual(f["Lambda"]["final_verdict"], "unresolved")           # NOCAND
        self.assertEqual(list(next(iter(f.values())).keys()), batch.EVIDENCE_COLS + ["final_verdict", "final_note"])

    def test_label_suspect_is_recorded_and_never_shipped(self):
        f = self.final
        self.assertEqual(f["Beta"]["final_verdict"], "label_suspect")          # AUTO row + reviewer label_suspect
        self.assertEqual(f["Nu"]["final_verdict"], "label_suspect")            # Opus override
        self.assertEqual(f["Alpha"]["final_verdict"], "verified")
        rows = {r["gog_name"]: r for r in bd.iter_tsv(self.root / "batches" / "B1" / "gog_label_suspects.tsv")}
        self.assertEqual(set(rows), {"CFoo::Beta", "CFoo::Nu"})
        beta, nu = rows["CFoo::Beta"], rows["CFoo::Nu"]
        self.assertEqual((beta["gog_addr"], beta["steam_counterpart"]), (h(GOG_BASE + 0x80), "0x00510080"))
        self.assertIn("fits RemoveGUIPopUps", beta["body_fits_name"])
        self.assertEqual(beta["source"], "review:haiku.tsv")
        self.assertEqual(nu["source"], "override")
        self.assertIn("setter", nu["body_fits_name"])
        self.assertEqual(list(next(iter(bd.iter_tsv(self.root / "batches" / "B1" / "gog_label_suspects.tsv"))).keys()),
                         batch.LABEL_SUSPECT_COLS)
        csv_names = {r[1] for r in csv.reader((self.root / "batches" / "B1" / "functions.csv").read_text().split("\n")[1:-1])}
        self.assertFalse({"Beta", "Nu"} & csv_names)                           # not in functions.csv
        upserts = {r["name"] for r in json.loads((self.root / "batches" / "B1" / "kdb_upsert.json").read_text())}
        self.assertFalse({"CFoo::Beta", "CFoo::Nu"} & upserts)                 # not persisted as high/medium
        # mismatch alongside label_suspect is a disagreement, not a label bug
        ev = self.evidence()["Theta"]
        v, _a, _n = batch.decide_review(ev, [{"steam_addr": ev["steam_addr"], "verdict": "label_suspect",
                                              "content_anchor": "x", "reason": "r"},
                                             {"steam_addr": ev["steam_addr"], "verdict": "mismatch"}])
        self.assertEqual(v, "unresolved")
        v, _a, _n = batch.decide_review(ev, [{"steam_addr": ev["steam_addr"], "verdict": "label_suspect",
                                              "content_anchor": "", "reason": "r"}])
        self.assertEqual(v, "unresolved")                                       # no anchor: counterpart not shown

    def test_match_without_anchor_never_verifies(self):
        # Kappa's reviewer `match` had no anchor; without the override it must stay below `verified`
        ev = self.evidence()["Kappa"]
        verdict, _addr, _note = batch.decide_review(ev, [{"steam_addr": ev["steam_addr"], "verdict": "match",
                                                          "content_anchor": ""}])
        self.assertEqual(verdict, "likely")
        verdict, _a, _n = batch.decide_review(dict(ev, content_level="C0"), [
            {"steam_addr": ev["steam_addr"], "verdict": "match", "content_anchor": ""}])
        self.assertEqual(verdict, "unresolved")

    def test_functions_csv_format(self):
        lines = (self.root / "batches" / "B1" / "functions.csv").read_text().split("\n")
        self.assertEqual(lines[0], "class_name,function_name,address,calling_convention,param_size_bytes,notes")
        rows = list(csv.reader(lines[1:-1]))
        self.assertTrue(rows)
        by = {r[1]: r for r in rows}
        self.assertEqual(by["Alpha"][:5], ["CFoo", "Alpha", "0x00510000", "__thiscall", "8"])
        self.assertTrue(by["Alpha"][5].startswith("Steam batch B1: STR '"), by["Alpha"][5])
        self.assertTrue(by["Alpha"][5].endswith("(evidence steam_db_batches/B1/evidence.tsv)"))
        self.assertEqual(by["Gamma"][4], "4")                    # Steam retn (GOG recorded 1 byte)
        self.assertIn("; GOG param_size_bytes 1", by["Gamma"][5])
        self.assertNotIn("Kappa", by)                            # rejected rows are not exported
        self.assertNotIn("Mu", by)
        self.assertTrue(all(len(r) == 6 for r in rows))
        # likely rows only with --with-likely
        self.assertNotIn("likely", {self.final[n]["final_verdict"] for n in by})

    def test_kdb_upsert_rows(self):
        rows = json.loads((self.root / "batches" / "B1" / "kdb_upsert.json").read_text())
        by = {r["name"]: r for r in rows}
        self.assertEqual(by["CFoo::Alpha"]["source"], "steam-db-B1")
        self.assertEqual(by["CFoo::Alpha"]["confidence"], "high")
        self.assertEqual(by["CFoo::Alpha"]["date"], "2026-10-03")
        self.assertNotIn("CFoo::Mu", by)

    def kdb(self):
        con = sqlite3.connect(self.world.kdb)
        con.row_factory = sqlite3.Row
        return con

    def test_persist_dry_run_writes_nothing(self):
        before = sha(self.world.kdb)
        rc, out = run("persist", "B1")
        self.assertEqual(rc, 0)
        self.assertIn("DRY-RUN", out)
        self.assertEqual(before, sha(self.world.kdb))

    def test_persist_apply_is_idempotent_and_correct(self):
        rc, out = run("persist", "B1", "--apply")
        self.assertEqual(rc, 0, out)
        self.assertIn("integrity_check: ok", out)
        con = self.kdb()
        row = con.execute("select * from name_map where steam_addr='0x00510000'").fetchone()
        self.assertEqual((row["name"], row["source"], row["confidence"]), ("CFoo::Alpha", "steam-db-B1", "high"))
        # untrusted holder replaced, old name recorded
        eta = con.execute("select * from name_map where steam_addr='0x00510240'").fetchone()
        self.assertEqual(eta["name"], "CFoo::Eta")
        self.assertIn("replaces COther::Stuff (batch5_reverse_engineering/high) 2026-10-03", eta["evidence"])
        # trusted holder (Zeta override): never silently replaced
        zeta = con.execute("select * from name_map where steam_addr='0x00510200'").fetchone()
        self.assertEqual(zeta["name"], "COther::Thing")
        self.assertIn("CONFLICT: trusted holder", out)
        # rejected address: the mapper row with the same name is demoted
        mu = con.execute("select * from name_map where steam_addr='0x00510280'").fetchone()
        self.assertEqual(mu["confidence"], "wrong")
        self.assertIn("demoted by steam-db-B1", mu["evidence"])
        # lessons deduped on scope+topic
        topics = [r["topic"] for r in con.execute("select topic from lessons")]
        self.assertEqual(topics.count("fixture lesson"), 1)
        self.assertEqual(topics.count("existing lesson"), 1)
        snapshot = con.execute("select * from name_map order by steam_addr").fetchall()
        lessons_n = con.execute("select count(*) from lessons").fetchone()[0]
        con.close()
        # second apply: no change at all
        rc, out2 = run("persist", "B1", "--apply")
        self.assertEqual(rc, 0)
        self.assertIn("applied: 0 rows changed", out2)
        con = self.kdb()
        self.assertEqual([tuple(r) for r in snapshot],
                         [tuple(r) for r in con.execute("select * from name_map order by steam_addr")])
        self.assertEqual(lessons_n, con.execute("select count(*) from lessons").fetchone()[0])
        self.assertEqual(con.execute("pragma integrity_check").fetchone()[0], "ok")
        con.close()


# ------------------------------------------------------------------------------------------------ Opus gate R1-R5, G7
WAVE2 = "engine-ai-atlas wave2"


class SeedHygieneTests(Base):
    """R1: a trusted row seeds pairs only with an exact name and an agreeing body; conflicts are listed."""

    def test_pairs_need_exact_name_and_body_agreement(self):
        w = self.world
        w.name_map.append((h(0x00510000), "CFoo::Alpha", WAVE2, "high", "ok"))                  # agrees: pair
        w.name_map.append((h(0x00510080), "CFoo::Gamma", WAVE2, "high", "wrong body"))          # retn 4 vs 8: conflict
        w.name_map.append((h(0x005102c0), "CFoo::Theta (char*)", WAVE2, "high", "suffix"))      # not exact: conflict
        w.name_map.append((h(0x005103c0), "CFoo::Nu", WAVE2, "high", "size differs, SIG equal"))
        w.steam_fp[0x005103C0]["size"] = 0x38                    # still SIG-identical to the GOG fingerprint
        w.build()
        corpus = bd.Corpus(bd.get_paths())
        g = lambda n: corpus.gog_by_name["CFoo::" + n].addr      # noqa: E731
        self.assertEqual(corpus.pairs[g("Alpha")], 0x00510000)
        self.assertNotIn(g("Gamma"), corpus.pairs)
        self.assertNotIn(g("Theta"), corpus.pairs)
        self.assertIn(g("Nu"), corpus.pairs)                       # A-SIG >= 0.5 with intersection >= 3 is enough
        reasons = {name: why for _g, _s, name, why in corpus.pair_conflicts}
        self.assertIn("retn GOG 4 vs Steam 8", reasons["CFoo::Gamma"])
        self.assertIn("not exact", reasons["CFoo::Theta"])
        self.assertNotIn("CFoo::Nu", reasons)
        # a conflicting row is still trusted as a G6 holder
        self.assertTrue(corpus.trust.is_trusted(0x00510080, "CFoo::Gamma"))
        write_candidates(self.root, "B1", {"Beta": "0x00510080|mapper:vote"})
        run("gates", "B1")
        self.assertEqual(self.evidence()["Beta"]["gate_class"], "REJECT")
        report = (self.root / "batches" / "B1" / "gate_report.md").read_text()
        self.assertIn("## Trusted pair body conflicts", report)
        self.assertRegex(report, r"CFoo::Gamma -> Steam 0x00510080: retn GOG 4 vs Steam 8")

    def test_retn_minus_one_is_unknown_not_a_conflict(self):
        w = self.world
        w.name_map.append((h(0x00510000), "CFoo::Alpha", WAVE2, "high", "ok"))
        w.gog_fp[GOG_BASE]["retn"] = -1                                    # GOG body without a RET, Steam returns 8
        w.build()
        corpus = bd.Corpus(bd.get_paths())
        self.assertIn(corpus.gog_by_name["CFoo::Alpha"].addr, corpus.pairs)

    def test_suffix_row_is_no_seed(self):
        self.world.name_map.append((h(0x00510000), "CFoo::Alpha", WAVE2, "high", "ok"))
        self.world.build()
        self.assertIn(self.world.gog[0]["addr"], bd.Corpus(bd.get_paths()).pairs)
        con = sqlite3.connect(self.world.kdb)
        con.execute("update name_map set name='CFoo::Alpha (overload)' where steam_addr=?", (h(0x00510000),))
        con.commit()
        con.close()
        corpus = bd.Corpus(bd.get_paths())
        self.assertNotIn(self.world.gog[0]["addr"], corpus.pairs)
        self.assertTrue(corpus.trust.is_trusted(0x00510000, "CFoo::Alpha"))        # still a holder


class SigTwinTests(SigRouteTests):
    """R2: same-size/same-retn starts within +-8 starts count as decoys for the sig margin."""

    def test_twin_window_blocks_the_sig_route(self):
        w = self.world
        w.pair("CFoo", "SgSeed", GOG_BASE + 0x1000, STEAM_BASE + 0x1000, 8, 8)
        w.pair("CFoo", "Sg1", GOG_BASE + 0x1040, STEAM_BASE + 0x1040, 8, 8, anchors=("SIG",))
        for k in range(4):
            w.filler("CFoo", "Fill%d" % k, GOG_BASE + 0x1080 + 0x40 * k, STEAM_BASE + 0x1080 + 0x40 * k)
        sg1 = w.steam_fp[STEAM_BASE + 0x1040]
        # a twin three starts after Sg1 (not previous / next): same size 0x30, retn 8, identical operand sets
        w.fp("steam", STEAM_BASE + 0x1100, 8, (), (), sg1["disp"], sg1["imm"])
        w.steam_fn(STEAM_BASE + 0x1100, "FUN_00511100", retn=8)
        w.build()
        write_candidates(self.root, "B1", {"SgSeed": "0x00511000|mapper:vote", "Sg1": "0x00511040|near"})
        run("gates", "B1")
        e = self.evidence()["Sg1"]
        self.assertEqual((e["families"], e["run_ok"]), ("SIG", "yes"))
        self.assertEqual(e["gate_class"], "REVIEW")
        self.assertIn("sig_twin", e["flags"])
        # without the twin the very same row is AUTO (sig route)
        w.fp("steam", STEAM_BASE + 0x1100, 8, (), (), [], [])
        w.build()
        run("gates", "B1")
        self.assertEqual(self.evidence()["Sg1"]["gate_class"], "AUTO")
        self.assertIn("auto_route=sig", self.evidence()["Sg1"]["flags"])


class SigMacTests(SigRouteTests):
    def test_mac_contra_blocks_the_sig_route(self):
        self.world.mac = [("CFoo", "Sg1", [0x20000, 0x20010], [0x30000, 0x30004])]
        e = self.build_rows()["Sg1"]
        self.assertEqual((e["families"], e["run_ok"]), ("SIG", "yes"))
        self.assertEqual(e["g7"], "mac_contra(0.00)")
        self.assertEqual(e["gate_class"], "REVIEW")


class MacContraTests(Base):
    """R3 / G7: values within 4 of a Mac function of the same name; zero overlap everywhere -> mac_contra (REVIEW)."""

    def setup_mac(self, mac_rows):
        self.world.mac = mac_rows
        self.world.build()
        write_candidates(self.root, "B1", {"Alpha": "0x00510000|mapper:vote"})
        run("gates", "B1")
        return self.evidence()["Alpha"]

    def test_contradiction_review_never_reject(self):
        e = self.setup_mac([("CFoo", "Alpha", [0x20000, 0x20010], [0x30000, 0x30004])])        # disjoint values
        self.assertEqual(e["g7"], "mac_contra(0.00)")
        self.assertEqual(e["gate_class"], "REVIEW")                # even with C2 and mapper:vote
        self.assertIn("mac_contra", e["flags"])

    def test_overlap_within_four_passes_and_does_not_block(self):
        e = self.setup_mac([("CFoo", "Alpha", [0x1002, 0x1006 + 8], [0x5001, 0x5003])])        # Alpha disp 0x1000.. imm 0x5000..
        self.assertTrue(e["g7"].startswith("pass("), e["g7"])
        self.assertEqual(e["gate_class"], "AUTO")

    def test_one_agreeing_mac_function_is_enough_and_missing_data_is_na(self):
        e = self.setup_mac([("CFoo", "Alpha", [0x20000, 0x20010], [0x30000, 0x30004]),
                            ("CFoo", "Alpha", [0x1000, 0x1004], [])])                            # an overload that agrees
        self.assertTrue(e["g7"].startswith("pass("))
        e = self.setup_mac([("CFoo", "Alpha", [0x20000], [])])                                   # < 2 values
        self.assertEqual(e["g7"], "n/a")
        e = self.setup_mac([("COther", "Alpha", [0x20000, 0x20010], [0x30000])])                  # other class
        self.assertEqual(e["g7"], "n/a")
        self.assertEqual(e["gate_class"], "AUTO")

    def test_small_values_are_ignored(self):
        e = self.setup_mac([("CFoo", "Alpha", [0x4, 0x8, 0xC], [0x4])])                          # all below 0x10
        self.assertEqual(e["g7"], "n/a")


class NameHeldElsewhereTests(Base):
    def test_trusted_name_at_another_address_blocks_auto(self):
        self.world.name_map.append((h(0x00510040), "CFoo::Alpha", WAVE2, "high", "trusted elsewhere"))
        self.world._write_dbs()
        write_candidates(self.root, "B1", {"Alpha": "0x00510000|mapper:vote"})
        run("gates", "B1")
        e = self.evidence()["Alpha"]
        self.assertTrue(e["g6"].startswith("review:name_held_elsewhere=0x00510040"), e["g6"])
        self.assertEqual(e["gate_class"], "REVIEW")                # families would give C2 + vote -> AUTO otherwise
        self.assertIn("g6", e["flags"])


class ReviewerCorrectionTests(Base):
    def test_corrected_addr_is_unresolved_rechecked_and_regated(self):
        write_candidates(self.root, "B1", B1_SPEC)
        run("gates", "B1")
        bdir = self.root / "batches" / "B1"
        (bdir / "review").mkdir(exist_ok=True)
        (bdir / "review" / "r.tsv").write_text(
            "gog_addr\tsteam_addr\tverdict\tcorrected_addr\tcontent_anchor\tconfidence\treason\n"
            "%s\t0x005102c0\tmismatch\t0x00510340\tx\t4\tthe body sits at Kappa's address\n" % h(GOG_BASE + 0x2C0))
        rc, out = run("finalize", "B1", "--date", "2026-10-03")
        f = {r["name"]: r for r in bd.iter_tsv(bdir / "final_rows.tsv")}
        self.assertEqual(f["Theta"]["final_verdict"], "unresolved")          # never `likely` by itself
        self.assertIn("recheck", f["Theta"]["final_note"])
        rows = list(bd.iter_tsv(bdir / "recheck.tsv"))
        self.assertEqual([(r["gog_addr"], r["corrected_addr"]) for r in rows], [(h(GOG_BASE + 0x2C0), "0x00510340")])
        export = {r[1] for r in csv.reader((bdir / "functions.csv").read_text().split("\n")[1:-1])}
        self.assertNotIn("Theta", export)
        run("gates", "B1")                                                    # the next gates run re-gates it
        ev = self.evidence()["Theta"]
        self.assertIn("0x00510340|recheck", ev["cand_sources"])
        self.assertIn("0x005102c0", ev["cand_sources"])


class PersistR4Tests(Base):
    def make_con(self):
        return sqlite3.connect(self.world.kdb)

    def test_no_second_address_for_a_trusted_name_and_no_unforced_demotion(self):
        con = self.make_con()
        con.execute("insert into name_map values(?,?,?,?,?,?)",
                    ("0x00510040", "CFoo::Alpha", WAVE2, "high", "trusted", "2026-10-01"))
        con.execute("insert into name_map values(?,?,?,?,?,?)",
                    ("0x00510080", "CFoo::Beta", WAVE2, "high", "trusted", "2026-10-01"))
        con.commit()
        up = [{"steam_addr": "0x00510000", "name": "CFoo::Alpha", "source": "steam-db-B1", "confidence": "high",
               "evidence": "e", "date": "2026-10-03"}]
        rej = [{"steam_addr": "0x00510080", "class": "CFoo", "name": "Beta", "final_note": "gate"}]
        ops = batch.plan_persist(con, up, rej, [], "B1", "2026-10-03", set())
        kinds = [o[0] for o in ops]
        self.assertEqual(kinds, ["skip", "skip"])
        self.assertIn("already held by a trusted row at 0x00510040", [o for o in ops if o[1] == "0x00510000"][0][2])
        self.assertIn("demoted only by an Opus overrides.tsv row", [o for o in ops if o[1] == "0x00510080"][0][2])
        # an Opus override naming the address allows the demotion
        ops = batch.plan_persist(con, [], rej, [], "B1", "2026-10-03", {"0x00510080"})
        self.assertEqual([o[0] for o in ops], ["demote"])
        con.close()


class OpusGateTests(Base):
    def setup_bdir(self, n_decoys, accepted, chunks=1):
        bdir = self.root / "batches" / "B7"
        (bdir / "cards").mkdir(parents=True, exist_ok=True)
        key, reviews, evs = [], {}, []
        for c in range(1, chunks + 1):
            for i in range(n_decoys):
                ga = 0x00400000 + 0x100 * (c * 100 + i)
                key.append({"card_id": "B7-%d-%02d" % (c, i), "chunk": c, "gog": h(ga), "steam_addr": h(ga + 0x100000),
                            "kind": "decoy_next", "expected": "mismatch"})
                reviews[ga] = [{"steam_addr": h(ga + 0x100000), "verdict": "match" if (c, i) in accepted else "mismatch"}]
        key.append({"card_id": "B7-1-99", "chunk": 1, "gog": h(0x00600000), "steam_addr": h(0x00700000),
                    "kind": "real", "expected": ""})
        batch.write_tsv(bdir / "cards" / "key.tsv", ["card_id", "chunk", "gog", "steam_addr", "kind", "expected"], key)
        return bdir, reviews

    def test_decisions(self):
        bdir, reviews = self.setup_bdir(10, set())
        res = batch.opus_gate(bdir, "B7", [], [], reviews, 0)
        self.assertTrue(res["decision"].startswith("OK"), res)
        self.assertTrue(res["clean"])
        self.assertIn("Decision: **OK", (bdir / "opus_gate.md").read_text())
        bdir, reviews = self.setup_bdir(10, {(1, 3)})
        res = batch.opus_gate(bdir, "B7", [], [], reviews, 0)
        self.assertTrue(res["decision"].startswith("ESCALATE"), res)
        self.assertEqual(res["decoys_accepted"], 1)
        self.assertFalse(res["clean"])
        bdir, reviews = self.setup_bdir(9, set())
        res = batch.opus_gate(bdir, "B7", [], [], reviews, 0)
        self.assertTrue(res["decision"].startswith("INSUFFICIENT DECOYS"), res)
        bdir, reviews = self.setup_bdir(10, set())
        res = batch.opus_gate(bdir, "B7", [], [], reviews, 2)           # Opus found errors in its sample
        self.assertTrue(res["decision"].startswith("ESCALATE"), res)

    def test_sample_rate_drops_after_three_clean_batches(self):
        for n in (4, 5, 6):
            d = self.root / "batches" / ("B%d" % n)
            d.mkdir(parents=True)
            (d / "opus_gate.json").write_text(json.dumps({"clean": True}))
        bdir, reviews = self.setup_bdir(10, set())
        self.assertEqual(batch.opus_gate(bdir, "B7", [], [], reviews, 0)["sample_rate"], 0.10)
        (self.root / "batches" / "B5" / "opus_gate.json").write_text(json.dumps({"clean": False}))
        self.assertEqual(batch.opus_gate(bdir, "B7", [], [], reviews, 0)["sample_rate"], 0.20)

    def test_full_read_classes_and_sample_minimum(self):
        bdir, reviews = self.setup_bdir(10, set())
        evs, finals = [], []
        for i in range(12):                                            # 12 plain reviewer matches, some C0
            ga = 0x00800000 + 0x100 * i
            evs.append({"gog": h(ga), "gate_class": "REVIEW", "content_level": "C0" if i < 6 else "C1", "flags": "",
                        "g6": "pass", "class": "C", "name": "F%d" % i, "steam_addr": h(ga + 0x100000)})
            finals.append({"gog": h(ga), "class": "C", "name": "F%d" % i, "steam_addr": h(ga + 0x100000),
                           "final_verdict": "verified"})
            reviews[ga] = [{"steam_addr": h(ga + 0x100000), "verdict": "match", "reason": "ok", "content_anchor": "a"}]
        ga = 0x00900000                                                 # layout citation + mac_contra -> 100%
        evs.append({"gog": h(ga), "gate_class": "REVIEW", "content_level": "C1", "flags": "why:mac_contra", "g6": "pass",
                    "class": "C", "name": "Lay", "steam_addr": h(ga + 0x100000)})
        finals.append({"gog": h(ga), "class": "C", "name": "Lay", "steam_addr": h(ga + 0x100000), "final_verdict": "verified"})
        reviews[ga] = [{"steam_addr": h(ga + 0x100000), "verdict": "match", "reason": "tail field moved: layout grew",
                        "content_anchor": "a"}]
        res = batch.opus_gate(bdir, "B7", evs, finals, reviews, 0)
        self.assertEqual(res["full_read"], 1)
        self.assertEqual((res["sample_pool"], res["sample_size"]), (12, 5))      # 20% of 12 = 3 -> minimum 5
        text = (bdir / "opus_gate.md").read_text()
        self.assertIn("layout/offset difference", text)
        self.assertIn("mac_contra row", text)
        sample_lines = text.split("## Sample")[1].split("##")[0]
        self.assertEqual(sample_lines.count("(C0)"), 5)                         # C0 rows first


if __name__ == "__main__":
    unittest.main()
