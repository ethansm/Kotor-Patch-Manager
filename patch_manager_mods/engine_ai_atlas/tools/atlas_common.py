"""Shared helpers for the engine-AI-atlas text tools (stdlib only, read-only).

Everything here works on the text dump under ROOT and on the knowledge DB
(opened read-only).  Addresses are normalised to ints internally and printed
as 8 lowercase hex digits.
"""
import os
import re
import sqlite3

DEFAULT_ROOT = "~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/"
_HERE = os.path.dirname(os.path.abspath(__file__))
# tools/ -> engine_ai_atlas/ -> patch_manager_mods/
DEFAULT_DB = os.path.join(_HERE, "..", "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db")
STEAM_HASH_PREFIX = "6a522e71"  # kotor2_steam_aspyr (binaries.hash)

REGS32 = ("EAX", "ECX", "EDX", "EBX", "ESI", "EDI", "EBP", "ESP")
_SUBREG = {
    "AL": "EAX", "AH": "EAX", "AX": "EAX", "CL": "ECX", "CH": "ECX", "CX": "ECX",
    "DL": "EDX", "DH": "EDX", "DX": "EDX", "BL": "EBX", "BH": "EBX", "BX": "EBX",
    "SI": "ESI", "DI": "EDI", "BP": "EBP", "SP": "ESP",
}


def parse_addr(s):
    """'0x5BFC00' / '5bfc00' / '005bfc00' -> int."""
    s = str(s).strip().lower()
    if s.startswith("0x"):
        s = s[2:]
    return int(s, 16)


def fmt8(a):
    return "%08x" % a


def reg32(name):
    """Map AL/AX/EAX... to the 32-bit register name, or None."""
    n = name.strip().upper()
    if n in REGS32:
        return n
    return _SUBREG.get(n)


def expand_root(root):
    return os.path.abspath(os.path.expanduser(root or DEFAULT_ROOT))


def open_db_ro(path):
    """Open the knowledge DB read-only; returns None when it is missing."""
    path = os.path.abspath(os.path.expanduser(path or DEFAULT_DB))
    if not os.path.exists(path):
        return None
    return sqlite3.connect("file:%s?mode=ro" % path, uri=True)


def add_common_args(ap):
    ap.add_argument("--root", default=None, help="dump root (default %s)" % DEFAULT_ROOT)
    ap.add_argument("--db", default=None, help="knowledge DB (opened read-only)")


# --------------------------------------------------------------------------
# names
# --------------------------------------------------------------------------
class Names:
    """addr(int) -> (name, confidence).  name_map first, then callers_count, then FUN_xxxxxxxx."""

    def __init__(self, root, db, enabled=True):
        self.enabled = enabled
        self.nm = {}
        self.cc = {}
        if not enabled:
            return
        if db is not None:
            try:
                for a, n, conf in db.execute("select steam_addr, name, confidence from name_map"):
                    try:
                        self.nm[parse_addr(a)] = (n, conf or "")
                    except (ValueError, TypeError):
                        pass
            except sqlite3.Error:
                pass
        p = os.path.join(root, "callers_count.tsv")
        if os.path.exists(p):
            with open(p, encoding="utf-8", errors="replace") as f:
                next(f, None)
                for line in f:
                    parts = line.rstrip("\n").split("\t")
                    if len(parts) >= 2:
                        try:
                            self.cc[int(parts[0], 16)] = parts[1]
                        except ValueError:
                            pass

    def raw(self, a):
        """(name, conf) without decoration."""
        if not self.enabled:
            return ("FUN_%08x" % a, "")
        if a in self.nm:
            return self.nm[a]
        if a in self.cc:
            return (self.cc[a], "")
        return ("FUN_%08x" % a, "")

    def label(self, a, maxlen=90):
        """'00569320 CSWSCreature::AIUpdate [medium]' (name part truncated)."""
        if not self.enabled:
            return fmt8(a)
        n, conf = self.raw(a)
        if len(n) > maxlen:
            n = n[: maxlen - 3] + "..."
        return "%s %s%s" % (fmt8(a), n, (" [%s]" % conf) if conf else "")

    def name(self, a):
        return self.raw(a)[0]


# --------------------------------------------------------------------------
# callgraph
# --------------------------------------------------------------------------
class Graph:
    EDGE_TYPES = ("CALL", "TAILJMP")

    def __init__(self, root):
        self.root = root
        self.by_caller = {}   # fn -> [(site, callee|None, type, insn)] sorted by site
        self.by_callee = {}   # fn -> [(site, caller, type)]
        p = os.path.join(root, "callgraph.tsv")
        if not os.path.exists(p):
            raise FileNotFoundError(p)
        with open(p, encoding="utf-8", errors="replace") as f:
            next(f, None)
            for line in f:
                parts = line.rstrip("\n").split("\t")
                if len(parts) < 5:
                    continue
                try:
                    caller = int(parts[0], 16)
                    site = int(parts[1], 16)
                except ValueError:
                    continue
                typ = parts[3]
                insn = parts[4]
                try:
                    callee = int(parts[2], 16) if len(parts[2]) == 8 else None
                except ValueError:
                    callee = None  # '-' (indirect) or an import name
                if callee is None and parts[2] != "-":
                    insn = "%s (%s)" % (insn, parts[2])
                self.by_caller.setdefault(caller, []).append((site, callee, typ, insn))
                if callee is not None:
                    self.by_callee.setdefault(callee, []).append((site, caller, typ))
        for v in self.by_caller.values():
            v.sort(key=lambda r: r[0])
        for v in self.by_callee.values():
            v.sort()


# --------------------------------------------------------------------------
# asm
# --------------------------------------------------------------------------
_ASM_RE = re.compile(r"^([0-9a-f]{8})  ([0-9a-f]+) *([A-Z].*?)\s*$")


def asm_path(root, addr):
    h = fmt8(addr)
    return os.path.join(root, "decomp", h[2:4], h + ".asm")


def parse_asm_text(text):
    """-> list of (addr:int, bytes:str, insn:str); header/comment lines skipped.

    The bytes column is padded to 22 chars with no separator, so an 11-byte
    instruction runs straight into the mnemonic; bytes are lowercase hex and the
    mnemonic is upper case, which is how the two are split.
    """
    out = []
    for line in text.split("\n"):
        m = _ASM_RE.match(line)
        if m:
            out.append((int(m.group(1), 16), m.group(2), m.group(3)))
    return out


def read_asm(root, addr):
    p = asm_path(root, addr)
    try:
        with open(p, encoding="utf-8", errors="replace") as f:
            return parse_asm_text(f.read())
    except OSError:
        return None


_MEM_RE = re.compile(r"\[([^\]]*)\]")


def parse_mem(inner):
    """Parse the inside of [...] -> (base, index, scale, disp) ; disp is an int mod 2**32.

    base/index are 32-bit register names or None; absolute '[0x00a7e098]' gives
    base None, index None.
    """
    base = index = None
    scale = 1
    disp = 0
    for term in re.split(r"\s*\+\s*", inner.strip()):
        if not term:
            continue
        if "*" in term:
            r, s = term.split("*", 1)
            index = reg32(r)
            try:
                scale = int(s, 0)
            except ValueError:
                scale = 1
            continue
        r = reg32(term)
        if r:
            if base is None:
                base = r
            else:
                index = r
            continue
        try:
            disp = (disp + int(term, 0)) & 0xFFFFFFFF
        except ValueError:
            pass
    return base, index, scale, disp


def signed32(v):
    v &= 0xFFFFFFFF
    return v - 0x100000000 if v & 0x80000000 else v


def split_insn(insn):
    """'MOV dword ptr [EAX + 0x524],0x1f40' -> ('MOV', ['dword ptr [EAX + 0x524]', '0x1f40'])."""
    s = insn.strip()
    if s.startswith("LOCK "):
        s = s[5:]
    parts = s.split(None, 1)
    mnem = parts[0].upper()
    ops = []
    if len(parts) > 1:
        ops = [o.strip() for o in parts[1].split(",")]
    return mnem, ops


# --------------------------------------------------------------------------
# vtable-slot resolution for INDIRECT call sites
# --------------------------------------------------------------------------
_WRITE_DST = {"MOV", "LEA", "XOR", "ADD", "SUB", "POP", "MOVZX", "MOVSX", "IMUL", "AND", "OR",
              "SHL", "SHR", "SAR", "INC", "DEC", "NEG", "NOT", "ADC", "SBB", "XCHG", "CMOVZ",
              "CMOVNZ", "CWDE", "CDQ", "MOVSXD", "SHLD", "SHRD", "ROL", "ROR", "BSF", "BSR"}


def resolve_indirect(lines, site, insn, index=None):
    """Work out the vtable slot an INDIRECT call site uses.

    lines: parsed asm of the caller (parse_asm_text); site: call-site address.
    Returns (slot:int|None, how:str).  Cases:
      * 'CALL dword ptr [REG + 0xNN]' (REG not EBP) -> slot NN directly
      * 'CALL dword ptr [EBP + ...]' / '[0x...]' -> not a vtable slot (local/global fn pointer)
      * 'CALL REG' -> scan back <= 30 instructions for the last write of REG; if it is
        'MOV REG,dword ptr [R2 + 0xNN]' (or [R2]) the slot is NN (0).  A CALL in between
        clobbers EAX/ECX/EDX so the scan gives up there.  Linear scan, ignores branches.
    """
    insn = insn.strip()
    m = re.match(r"CALL dword ptr \[([^\]]*)\]$", insn)
    if m:
        base, idx, _s, disp = parse_mem(m.group(1))
        if base and base not in ("EBP", "ESP") and idx is None:
            return disp, "direct [%s + 0x%x]" % (base.lower(), disp)
        if base in ("EBP", "ESP"):
            return None, "local function pointer"
        return None, "global/indexed function pointer"
    m = re.match(r"CALL ([A-Z]{2,3})$", insn)
    if not m:
        return None, "unrecognised"
    reg = reg32(m.group(1))
    if reg is None:
        return None, "unrecognised"
    if index is None:
        index = {a: i for i, (a, _b, _t) in enumerate(lines)}
    i = index.get(site)
    if i is None:
        return None, "site not in asm"
    steps = 0
    j = i - 1
    while j >= 0 and steps < 30:
        a, _b, t = lines[j]
        mnem, ops = split_insn(t)
        if mnem == "CALL":
            if reg in ("EAX", "ECX", "EDX"):
                return None, "clobbered by CALL at %08x" % a
        elif mnem in _WRITE_DST and ops and reg32(ops[0]) == reg:
            if mnem == "MOV" and len(ops) == 2:
                mm = _MEM_RE.search(ops[1])
                if mm and ops[1].lower().startswith("dword ptr"):
                    base, idx, _s, disp = parse_mem(mm.group(1))
                    if base and idx is None and base != "ESP" and not (base == "EBP"):
                        return disp, "%08x %s" % (a, t)
                    return None, "last write %08x %s" % (a, t)
            return None, "last write %08x %s" % (a, t)
        j -= 1
        steps += 1
    return None, "no write of %s within 30 insns" % reg.lower()


# --------------------------------------------------------------------------
# class_methods slot candidates
# --------------------------------------------------------------------------
def slot_candidates(db, slot, hash_prefix=STEAM_HASH_PREFIX):
    """class_methods rows with vtable_index*4 == slot -> [(class, index, addr:int, symbol)]."""
    if db is None or slot is None or slot % 4:
        return []
    try:
        rows = db.execute(
            "select class_name, vtable_index, address, symbol from class_methods "
            "where vtable_index=? and binary_hash like ? order by class_name",
            (slot // 4, hash_prefix + "%"),
        ).fetchall()
    except sqlite3.Error:
        return []
    out = []
    for c, vi, a, s in rows:
        try:
            out.append((c, vi, parse_addr(a), s))
        except (ValueError, TypeError):
            pass
    return out
