#!/usr/bin/env python3
"""field_census.py - list every read/write of a struct field offset in the .asm dump.

  field_census.py --offset 0x524 (--fns A,B | --subtree ADDR [--depth N] | --all) [--root R] [--tsv] [--writers]

A hit is any asm line with a memory operand [REG + 0x524] or [REG + IDX*S + 0x524] whose displacement equals the
offset exactly (compared mod 2**32, so '--offset -0x8' also matches '[EBP + 0xfffffff8]' and '[EBP + -0x8]').
Absolute operands ([0x00a7e098]) are not field accesses and are ignored.

Access kind (column RW):
  W    destination is the memory operand and the op overwrites it: MOV/MOVSS/MOVSD/MOVQ/MOVAPS/MOVUPS/MOVD, POP,
       FST/FSTP/FIST/FISTP/FISTTP/FNSTCW/FNSTSW/FSTCW/FSTSW/FBSTP, SETcc
  RW   read-modify-write: ADD SUB ADC SBB INC DEC AND OR XOR NOT NEG SHL SHR SAR SAL ROL ROR BTS BTR BTC XCHG XADD CMPXCHG
  R    memory is a source, or CMP/TEST/PUSH/FLD/FILD/FCOM*/FADD.../MUL/DIV/CALL/JMP/BT on a memory destination
  ADDR LEA (address taken, not an access; the pointer may be used to read/write elsewhere)
  ?    memory destination with a mnemonic this tool does not know (shown so you can look)

Provenance (column PROV, best effort, linear scan of the function ignoring control flow - treat as a hint):
  walks back from the access to the last write of the base register and follows it:
    MOV REG,ECX with ECX untouched since entry              -> "this"
    MOV REG,dword ptr [EBP + X] and the slot was written    -> follows the source (e.g. "this (via [ebp-0x194])")
      from a register (typically MOV [EBP+X],ECX at entry)
    MOV REG,dword ptr [REG2 + 0xNN]                         -> "field 0xNN of <prov of REG2>"
    MOV REG,dword ptr [0x00xxxxxx]                          -> "global 0xXXXXXXXX"
    CALL then EAX                                           -> "ret of <callee>"
    LEA REG,[REG2 + 0xNN]                                   -> "&field 0xNN of <prov of REG2>"
    positive [EBP + 8/0xc/...] never written                -> "arg1/arg2/..."
  anything else -> "unknown (<why>)".  ECX at function entry is "this" (thiscall); EDX/EAX/callee-saved at entry are unknown.
"""
import argparse
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import atlas_common as ac  # noqa: E402

W_OPS = {"MOV", "MOVSS", "MOVSD", "MOVQ", "MOVAPS", "MOVUPS", "MOVD", "MOVDQA", "MOVDQU", "POP", "FST", "FSTP", "FIST",
         "FISTP", "FISTTP", "FNSTCW", "FNSTSW", "FSTCW", "FSTSW", "FBSTP", "FNSTENV", "FSTENV", "MOVNTI", "MOVLPS",
         "MOVHPS", "MOVLPD", "MOVHPD"}
RW_OPS = {"ADD", "SUB", "ADC", "SBB", "INC", "DEC", "AND", "OR", "XOR", "NOT", "NEG", "SHL", "SHR", "SAR", "SAL",
          "ROL", "ROR", "RCL", "RCR", "BTS", "BTR", "BTC", "XCHG", "XADD", "CMPXCHG"}
R_OPS = {"CMP", "TEST", "PUSH", "BT", "MUL", "IMUL", "DIV", "IDIV", "CALL", "JMP", "FLDCW", "FLDENV", "FLD", "FILD",
         "FBLD", "MOVZX", "MOVSX", "MOVSXD", "CMPXCHG8B"}
_SETCC = re.compile(r"^SET[A-Z]+$")
_MEM = re.compile(r"\[([^\]]*)\]")


def classify(insn, offset):
    """-> (rw, base, index) for a matching memory operand, or None when the line does not touch [reg+offset]."""
    mnem, ops = ac.split_insn(insn)
    base = index = None
    which = None
    for i, o in enumerate(ops):
        m = _MEM.search(o)
        if not m:
            continue
        b, ix, _s, disp = ac.parse_mem(m.group(1))
        if (b or ix) and disp == offset:
            base, index, which = b, ix, i
            break
    if which is None:
        return None
    if mnem == "LEA":
        return "ADDR", base, index
    if mnem.endswith(".REP") or mnem.startswith("REP"):
        return "?", base, index
    if which == 0:
        if mnem in RW_OPS:
            rw = "RW"
        elif mnem in W_OPS or _SETCC.match(mnem):
            rw = "W"
        elif mnem in R_OPS or mnem.startswith("F"):
            rw = "R"
        else:
            rw = "?"
    else:
        rw = "RW" if mnem in ("XCHG", "XADD", "CMPXCHG") else "R"
    return rw, base, index


# ---------------------------------------------------------------- provenance
_IMM = re.compile(r"^-?(0x[0-9a-f]+|\d+)$", re.I)


def _slot_name(disp):
    s = ac.signed32(disp)
    return "[ebp-0x%x]" % -s if s < 0 else "[ebp+0x%x]" % s


def _dest_reg(mnem, ops):
    if mnem in ("CMP", "TEST", "PUSH", "CALL", "JMP") or not ops:
        return None
    if mnem in ("MOV", "LEA", "XOR", "ADD", "SUB", "POP", "MOVZX", "MOVSX", "IMUL", "AND", "OR", "SHL", "SHR", "SAR",
                "INC", "DEC", "NEG", "NOT", "ADC", "SBB", "XCHG", "CWDE", "SHLD", "SHRD", "ROL", "ROR", "MOVSXD",
                "BSF", "BSR", "SETZ", "SETNZ") or mnem.startswith("CMOV") or mnem.startswith("SET"):
        return ac.reg32(ops[0])
    if mnem in ("CDQ",):
        return "EDX"
    if mnem in ("MUL", "DIV", "IDIV"):
        return "EAX"
    return None


def provenance(lines, idx, reg, depth=0, seen=None):
    """Best-effort description of where register `reg` came from just before lines[idx]."""
    if reg is None:
        return "unknown (no base)"
    if depth > 5:
        return "unknown (depth)"
    if reg in ("ESP",):
        return "stack"
    if reg == "EBP":
        return "frame"
    j = idx - 1
    while j >= 0:
        addr, _b, t = lines[j]
        mnem, ops = ac.split_insn(t)
        if mnem == "CALL":
            if reg == "EAX":
                m = re.match(r"CALL 0x([0-9a-f]+)$", t.strip(), re.I)
                if m:
                    return "ret of %08x" % int(m.group(1), 16)
                return "ret of indirect call @%08x" % addr
            if reg in ("ECX", "EDX"):
                return "unknown (clobbered by call @%08x)" % addr
        if mnem in ("CDQ", "MUL", "DIV", "IDIV", "CWDE") and reg in ("EAX", "EDX"):
            return "unknown (%s @%08x)" % (mnem.lower(), addr)
        if _dest_reg(mnem, ops) == reg:
            return _follow(lines, j, reg, mnem, ops, depth)
        j -= 1
    # reached function entry
    if reg == "ECX":
        return "this"
    return "unknown (%s at entry)" % reg.lower()


def _follow(lines, j, reg, mnem, ops, depth):
    addr = lines[j][0]
    if mnem == "XOR" and len(ops) == 2 and ac.reg32(ops[1]) == reg:
        return "0"
    if mnem == "POP":
        return "unknown (pop @%08x)" % addr
    if mnem == "MOV" and len(ops) == 2:
        src = ops[1]
        r2 = ac.reg32(src)
        if r2:
            return provenance(lines, j, r2, depth + 1)
        if _IMM.match(src):
            return "imm %s" % src
        m = _MEM.search(src)
        if m and src.lower().startswith("dword ptr"):
            base, ix, _s, disp = ac.parse_mem(m.group(1))
            if ix:
                return "unknown (indexed load @%08x)" % addr
            if base is None:
                return "global 0x%08x" % disp
            if base == "EBP":
                return _local_slot(lines, j, disp, depth)
            if base == "ESP":
                return "unknown (stack load @%08x)" % addr
            return "field 0x%x of %s" % (disp, provenance(lines, j, base, depth + 1))
        return "unknown (@%08x %s)" % (addr, lines[j][2])
    if mnem == "LEA" and len(ops) == 2:
        m = _MEM.search(ops[1])
        if m:
            base, ix, _s, disp = ac.parse_mem(m.group(1))
            if base == "EBP" and not ix:
                return "&local %s" % _slot_name(disp)
            if base and not ix:
                return "&field 0x%x of %s" % (disp, provenance(lines, j, base, depth + 1))
    return "unknown (@%08x %s)" % (addr, lines[j][2])


def _local_slot(lines, j, disp, depth):
    """MOV reg,[EBP+disp] at lines[j]: find the last write to that slot before j."""
    s = ac.signed32(disp)
    k = j - 1
    while k >= 0:
        _a, _b, t = lines[k]
        mnem, ops = ac.split_insn(t)
        if mnem in ("MOV", "ADD", "SUB", "INC", "DEC", "XOR", "AND", "OR", "POP", "LEA") and ops:
            m = _MEM.search(ops[0])
            if m and ops[0].lower().startswith(("dword ptr", "float ptr")):
                base, ix, _s, d2 = ac.parse_mem(m.group(1))
                if base == "EBP" and not ix and d2 == (disp & 0xFFFFFFFF):
                    if mnem == "MOV" and len(ops) == 2:
                        r2 = ac.reg32(ops[1])
                        if r2:
                            p = provenance(lines, k, r2, depth + 1)
                            return "%s (via %s)" % (p, _slot_name(disp))
                        if _IMM.match(ops[1]):
                            return "imm %s (via %s)" % (ops[1], _slot_name(disp))
                    return "unknown (slot %s modified by %s @%08x)" % (_slot_name(disp), mnem.lower(), lines[k][0])
        k -= 1
    if s > 0:
        return "arg%d (via %s)" % ((s - 8) // 4 + 1, _slot_name(disp))
    return "unknown (local %s never written)" % _slot_name(disp)


# ---------------------------------------------------------------- scanning
def prefilter_needles(offset):
    forms = {"0x%x]" % offset}
    if offset & 0x80000000:
        forms.add("-0x%x]" % (0x100000000 - offset))
    return tuple(forms)


def scan_text(fn, text, offset, needles):
    """-> list of hit dicts for one function's asm text (parsed lazily)."""
    if not any(n in text for n in needles):
        return []
    lines = ac.parse_asm_text(text)
    hits = []
    for i, (addr, byt, insn) in enumerate(lines):
        if not any(n in insn for n in needles):
            continue
        c = classify(insn, offset)
        if c is None:
            continue
        rw, base, index = c
        hits.append({"fn": fn, "addr": addr, "bytes": byt, "insn": insn, "rw": rw, "base": base or index,
                     "prov": provenance(lines, i, base or index)})
    return hits


def fn_files_all(root):
    d = os.path.join(root, "decomp")
    for bb in sorted(os.listdir(d)):
        p = os.path.join(d, bb)
        if not os.path.isdir(p):
            continue
        for name in sorted(os.listdir(p)):
            if name.endswith(".asm") and len(name) == 12:
                yield int(name[:8], 16), os.path.join(p, name)


def subtree_fns(g, start, depth):
    seen = {start}
    frontier = [start]
    for _ in range(depth):
        nxt = []
        for fn in frontier:
            for site, callee, typ, insn in g.by_caller.get(fn, []):
                if callee is not None and typ in ac.Graph.EDGE_TYPES and callee not in seen:
                    seen.add(callee)
                    nxt.append(callee)
        frontier = nxt
    return sorted(seen)


def run(args):
    root = ac.expand_root(args.root)
    offset = ac.parse_addr(args.offset) if not args.offset.strip().startswith("-") else (
        -ac.parse_addr(args.offset.strip()[1:])) & 0xFFFFFFFF
    needles = prefilter_needles(offset)
    names = ac.Names(root, ac.open_db_ro(args.db) if args.names else None, enabled=args.names)
    hits = []
    missing = []
    if args.all:
        for fn, path in fn_files_all(root):
            with open(path, encoding="utf-8", errors="replace") as f:
                hits.extend(scan_text(fn, f.read(), offset, needles))
        scope = "all functions"
    else:
        if args.fns:
            fns = [ac.parse_addr(x) for x in args.fns.split(",") if x.strip()]
        else:
            g = ac.Graph(root)
            fns = subtree_fns(g, ac.parse_addr(args.subtree), args.depth)
        for fn in fns:
            try:
                with open(ac.asm_path(root, fn), encoding="utf-8", errors="replace") as f:
                    hits.extend(scan_text(fn, f.read(), offset, needles))
            except OSError:
                missing.append(fn)
        scope = "%d functions" % len(fns)
    if args.writers:
        hits = [h for h in hits if h["rw"] in ("W", "RW")]
    hits.sort(key=lambda h: h["addr"])
    return hits, scope, missing, names, offset


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ac.add_common_args(ap)
    ap.add_argument("--offset", required=True, help="field offset, e.g. 0x524 (negative: --offset=-0x8)")
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--fns", help="comma-separated function addresses")
    g.add_argument("--subtree", help="start function; scan it and its direct callees to --depth")
    g.add_argument("--all", action="store_true", help="scan every .asm under ROOT/decomp")
    ap.add_argument("--depth", type=int, default=2)
    ap.add_argument("--writers", action="store_true", help="only W / RW hits")
    ap.add_argument("--names", action=argparse.BooleanOptionalAction, default=True)
    ap.add_argument("--tsv", action="store_true")
    args = ap.parse_args(argv)
    hits, scope, missing, names, offset = run(args)
    for fn in missing:
        print("warning: no .asm for %08x" % fn, file=sys.stderr)
    if args.tsv:
        print("\t".join(["fn", "fn_name", "addr", "bytes", "insn", "rw", "base", "prov"]))
        for h in hits:
            print("\t".join([ac.fmt8(h["fn"]), names.name(h["fn"]), ac.fmt8(h["addr"]), h["bytes"], h["insn"], h["rw"],
                             (h["base"] or "").lower(), h["prov"]]))
        return 0
    cnt = {}
    for h in hits:
        cnt[h["rw"]] = cnt.get(h["rw"], 0) + 1
    print("offset 0x%x over %s: %d hits in %d functions  %s" % (
        offset, scope, len(hits), len({h["fn"] for h in hits}), dict(sorted(cnt.items()))))
    last = None
    for h in hits:
        if h["fn"] != last:
            print("%s" % names.label(h["fn"]))
            last = h["fn"]
        print("  %s  %-4s %-4s %-44s base=%-3s prov: %s" % (
            ac.fmt8(h["addr"]), h["rw"], "", h["insn"], (h["base"] or "-").lower(), h["prov"]))
    return 0


if __name__ == "__main__":
    import signal
    signal.signal(signal.SIGPIPE, signal.SIG_DFL)  # quiet when piped into head
    sys.exit(main())
