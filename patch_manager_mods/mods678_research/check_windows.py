#!/usr/bin/env python3
"""Validate splice windows (Mods 6-8 + companion-ai) against the full-program dump (lesson 166) and the live hooks.

For each window (name, start, length, function entry, skip, consumed exit) it checks, from the dump's <fn>.asm:
  * the window starts and ends on whole instruction boundaries (length is the sum of whole instructions)
  * length >= 5
  * no relative branch/call (E8/E9/EB/7x/0F 8x) INSIDE the window (a re-executed copy would land wrong),
    unless the window is a skip_original_bytes=true window (bytes never re-run) -> reported as 'skip-only'
  * no branch from OUTSIDE the window targets an address strictly inside it (start is fine, it lands on our jump)
  * a RET may only be the LAST stolen instruction (the wrapper restores ESP exactly, so a trailing RET returns normally)
  * no overlap with any live hook window: the snapshot live_hooks_2026-09-30g.tsv AND (read-only, when present) the
    current live patch_config.toml in the game folder; a window's own installed patch id is skipped
  * no overlap with any OTHER window in the table (ranged-ai vs companion-ai vs the inventory windows, trace vs fixes)
  * consumed-exit hooks (framework fact, KotorPatcher wrapper_x86.cpp): EAX must be excluded from the restore so the
    handler's result reaches the TEST, so EAX = the handler result on BOTH exits: the stolen bytes (which run before
    the test) must not touch EAX, and EAX must be dead at the resume address and at the consumed exit (liveness walk
    over fall-through + Jcc/JMP targets inside the function; review SF2 2026-10-01: explicit AND implicit EAX readers
    (CDQ/CWD/CBW/CWDE, one-operand MUL/IMUL/DIV/IDIV, STOS/LODS/SCAS/CMPXCHG, XCHG/IN/OUT, SAHF, PUSHAD, CMOVcc EAX),
    RET (EAX = return value) and a direct CALL to a callee that reads EAX before writing it all count as reads; XOR/SUB
    EAX,EAX are writes; an indirect JMP (switch table) or an unmodelled mnemonic is "unknown" and fails the check)
  * consumed exits and skip-only resumes keep the stack: the ESP depth (forward walk from the function entry; direct
    CALL pops = the callee's RET n, an indirect CALL pops its pending arguments unless an ADD ESP follows) at the
    consumed exit equals the depth at which the wrapper jumps there (window start for skip-only, else window end),
    and for a skip-only window the depth at its end equals the depth at its start
  * TOML cross-check: every hook in the owning patch's source hooks.toml (ranged_ai_src, companion_ai_src,
    inventory_sort_filter_src) has a window here with the same address, length, bytes, skip flag and consumed exit;
    consumed hooks list "eax" in exclude_from_restore and non-consumed hooks do not; windows marked not shipped must
    NOT be in the toml
Prints the original bytes as a TOML-ready list.

Usage: check_windows.py            (runs the built-in table of windows)
"""
import re, sys, os, tomllib

DUMP = os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp")
HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.dirname(HERE)   # patch_manager_mods
LIVE = os.path.join(HERE, "live_hooks_2026-09-30g.tsv")   # regenerated 2026-09-30 23:58 from the live patch_config.toml (120 hooks: sibling added level-up-override, perf-lab 0.3.0, load-hang-probe 1.4.0)
LIVE_CONFIG = "/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/patch_config.toml"  # read only

# window name prefix -> the patch id that owns it (its own installed hooks are not an overlap) and its source toml
OWNERS = {"M6": ("inventory-sort-filter", "inventory_sort_filter_src"), "M8": ("ranged-ai", "ranged_ai_src"),
          "CA": ("companion-ai", "companion_ai_src"), "M7": (None, None)}


def W(name, start, length, entry, skip=False, cexit=None, eax_ret_ok=False, shipped=True):
    """eax_ret_ok: the consumed exit is the function epilogue and the handler's result IS the return value (by design).
    shipped=False: validated for later use, but deliberately absent from the owner's hooks.toml."""
    return (name, start, length, entry, skip, cexit, eax_ret_ok, shipped)


WINDOWS = [
    # Mod 6 (inventory-sort-filter 0.2.0, live)
    W("M6 tab click entry",         0x8A8CF0, 6,  0x8A8CF0, cexit=0x8A8DE2, eax_ret_ok=True),  # handler result = the click handler's return value
    W("M6 backpack row buffer",     0x8A7E22, 79, 0x8A7BA0, skip=True),
    W("M6 flush before finalise",   0x8A7E76, 22, 0x8A7BA0),
    W("M6 row-select epilogue (re-tint)", 0x8A8478, 6, 0x8A83D0),  # H6d 0.2.0: MOV ESP,EBP; POP EBP; RET 4 (RET is the last stolen insn)
    # Mod 7 (designed, not built)
    W("M7 SetMouseCursor entry",    0x7B3870, 7,  0x7B3870),
    W("M7 hilite placeable colour", 0x7AA747, 10, 0x7AA460),
    W("M7 client update tick",      0x7AC470, 5,  0x7AC470),
    # Mod 8 engine (ranged-ai 0.2.0 live; 0.3.0 changes only the E-H3 handler, same windows)
    W("M8 hold-on-blocked-LOS",     0x6D83EA, 12, 0x6D7390, cexit=0x6D83FF),
    W("M8 retarget gate (IsPC)",    0x6D7C79, 6,  0x6D7390, cexit=0x6D7C8C),   # moved from 0x6D7C73 (stolen MOV ECX,[EAX+..] ran with EAX=handler result)
    W("M8 ranged-stance rules local", 0x6D850F, 6, 0x6D7390, skip=True),     # E-H3: MOV [EBP-0x174],EAX (skip-only; 0.3.0 kite_approach)
    W("M8 stance clamp",            0x77AA5E, 7,  0x77A9E0, cexit=0x77AAB4),
    W("M8 stance row fill",         0x77ACBC, 9,  0x77A9E0, cexit=0x77AE77),   # re-sited 2026-09-30: was 15 bytes ending in MOV EAX,[EBP-0xF8] (EAX rule)
    W("M8 stance click",            0x77D8D4, 6,  0x77D890, cexit=0x77D9CA),   # re-sited 2026-09-30: was 14 bytes incl. MOV EAX,..; SUB EAX,0x410 (EAX rule)
    W("M8 HUD stance default branch", 0x74DBDD, 7, 0x74DA40, skip=True),     # S-H4: MOV [ebp-0x1c],-1 (skip-only; handler writes 0x415 or -1)
    # companion-ai 0.1.0: AI trace (design_ai_trace.md; E4 dropped, the FM1 handler logs instead)
    W("CA T1 ShipBuild value",      0x692C50, 10, 0x692C40),                   # MOV [0xA7E114],1; handler overwrites [ebp-4] (pushed at 0x692C5A)
    W("CA T2 PrintString capture",  0x68F301, 7,  0x68F260, cexit=0x68F39F),   # MOV [EBP-0x10],0 (argc<2 path); exit = success tail
    W("CA T3 AurPostString capture", 0x6F62F2, 7, 0x6F6200),                   # MOV [EBP-0x34],0 after the 4 pops succeeded
    W("CA E1 attack entry (+0x570 reset)", 0x6D740F, 10, 0x6D7390),            # MOV [EAX+0x570],0xBB8, EAX = attacker (EAX restored)
    W("CA E1b approach-branch exit", 0x6D8BE2, 5, 0x6D7390),                   # MOV EAX,[0x9944B8] (return 2)
    W("CA E2 slot-6 fallback fires", 0x569A08, 6, 0x569320),                   # MOV ECX,[EBP-0x194]
    W("CA E3 P1 block-2 UpdatePerception(1)", 0x56BDBD, 8, 0x56B8A0),          # PUSH 1; MOV ECX,[EBP-0xD4]
    # companion-ai 0.1.0: engine fixes (design_ai_engine_fixes.md)
    W("CA A1 approach gate end/retarget", 0x6D853E, 10, 0x6D7390, cexit=0x6D7D2F),  # MOV [EBP-0x16C],0x7F000000
    W("CA P1 throttle interval",    0x56BDB4, 7,  0x56B8A0),                   # CMP [EBP-0x28],0xFA0 (flags read by the JC after it)
    W("CA P1 throttle stamp store", 0x56BDE3, 6,  0x56B8A0),                   # MOV [EAX+0x39C],EDX
    W("CA FM1 bump clear replace",  0x586F43, 10, 0x586B70, skip=True),        # PUSH 1; MOV ECX,[EBP+8]; CALL ClearAllActions (handler calls it)
    W("CA FM4 snapshot FollowInfo", 0x57451F, 6,  0x574500),                   # MOV ECX,[EAX+0x11AC]
    W("CA FM4 restore FollowInfo",  0x575183, 7,  0x574500),                   # MOV EAX,[EBP+0xC]; PUSH EAX; MOV ECX,[EBP+8]
    W("CA A4 NULL spell row",       0x6D9E2A, 15, 0x6D9500, skip=True, cexit=0x6D9EA4),  # MOV EDX,[EBP-0x68]; MOV EAX,[EDX+0x154]; MOV [EBP-0x15C],EAX
    W("CA C1 restart idx (0xC)",    0x5868B9, 7,  0x5867E0, skip=True),        # MOV [EBP-0x10],0
    W("CA C1 restart idx (0xF)",    0x5868F1, 7,  0x5867E0, skip=True),
    W("CA C1 restart idx (0x2E)",   0x586929, 7,  0x5867E0, skip=True),
    W("CA A3 dead-target lock clear", 0x6D78B7, 7, 0x6D7390),                  # MOV [EBP-0x3C],0 (lock-clear variant, no consumed exit)
    W("CA FM3 sentinel probe (NOT SHIPPED)", 0x51DBE7, 6, 0x51D790, shipped=False),  # MOV EDX,[0xA1B488]; log-only probe, dropped
                                                                               # from 0.1.0 (wrapper cost in the scheduler's hottest loop, review N2)
    W("CA P FM2C partial-vs-fail",      0x55B69A, 6,  0x55AC80),   # MOV ECX,[0x99B5A0]; [EBP-0x18] res 2/3
    W("CA P FM2F fail leg (log-only)",  0x5C2438, 9,  0x5BFC00),   # plain this round (no consumed exit)
    W("CA P FM2R 0x11 re-approach",     0x5BC5F4, 9,  0x5BC1D0),   # start = JNZ 0x5BC5E2 / JZ 0x5BC5E8 target
    W("CA P FM3S sentinel set",         0x51DBE1, 6,  0x51D790),   # ends at 0x51DBE7 (the unshipped hot probe)
    W("CA P FM3E level end",            0x51DD33, 6,  0x51D790),   # stolen SUB feeds the SBB (stolen bytes last)
    W("CA P FM3R RemoveObject",         0x51E0C9, 6,  0x51E0C0),
    W("CA P RS RunScript entry",        0x6FD8D0, 6,  0x6FD8D0),   # PUSH EBP; MOV EBP,ESP; SUB ESP,8
    W("CA P P2 party cadence",          0x56BE79, 6,  0x56B8A0),   # HOT; 144 B from CA P1 stamp 0x56BDE3
    W("CA P PF party flag write",       0x58A35E, 6,  0x58A330),
    W("CA P RAR RunActions result",     0x5D6A60, 9,  0x5D56B0),   # HOT; stolen CMP feeds JNZ
    W("CA P SHX shout loop exit",       0x5475E8, 10, 0x547100),   # SEH unlink, absolute FS store
    W("CA P SS1 SetState commit",       0x59D966, 6,  0x59CBA0),   # stolen reads AL
    W("CA P GCA GetCurrentAction",      0x67B204, 10, 0x67B100),   # stolen PUSH ECX
]


def load_fn(entry):
    bb = "%02x" % ((entry >> 16) & 0xff)
    path = os.path.join(DUMP, bb, "%08x.asm" % entry)
    ins = []
    for l in open(path).read().splitlines():
        m = re.match(r"([0-9a-f]{8})\s+([0-9a-f]+)\s+(.*)", l)
        if m:
            ins.append((int(m.group(1), 16), bytes.fromhex(m.group(2)), m.group(3)))
    return ins


def live_windows():
    w = []
    for l in open(LIVE):
        a, n, ty, f, p = l.rstrip("\n").split("\t")
        w.append((int(a, 16), int(n), p, "TSV " + p + ":" + f))
    return w


def live_config_windows():
    """Current live hooks (read-only). Returns (list, note)."""
    if not os.path.exists(LIVE_CONFIG):
        return [], "live patch_config.toml not found (skipped)"
    d = tomllib.load(open(LIVE_CONFIG, "rb"))
    w = []
    for p in d.get("patches", []):
        for h in p.get("hooks", []):
            w.append((int(h["address"]), len(h["original_bytes"]), p["id"], "LIVE " + p["id"] + ":" + h.get("function", h.get("type", "?"))))
    return w, f"live patch_config.toml: {len(d.get('patches', []))} ids, {len(w)} hooks"


# ---- EAX liveness (review SF2) ------------------------------------------------------------------------------------
EAXRE = r"\b(EAX|AX|AL|AH)\b"
FULL_WRITE = ("MOV", "MOVZX", "MOVSX", "LEA", "POP")
# mnemonics whose ONLY EAX contact is through explicit operands (anything else is modelled below or is "unknown")
EXPLICIT_ONLY = {"MOV", "MOVZX", "MOVSX", "LEA", "PUSH", "POP", "ADD", "ADC", "SUB", "SBB", "AND", "OR", "XOR", "CMP",
                 "TEST", "INC", "DEC", "NEG", "NOT", "SHL", "SHR", "SAR", "SAL", "ROL", "ROR", "RCL", "RCR", "SHLD", "SHRD",
                 "BT", "BTS", "BTR", "BTC", "BSF", "BSR", "NOP", "LEAVE", "CLD", "STD", "CLC", "STC", "CMC", "INT3",
                 "MOVSB", "MOVSW", "MOVSD", "CMPSB", "CMPSW", "CMPSD", "PUSHFD", "POPFD", "PUSHF", "POPF", "WAIT", "FWAIT"}
IMPLICIT_READ = {"CDQ", "CWD", "CBW", "CWDE", "STOSB", "STOSW", "STOSD", "LODSB", "LODSW", "LODSD", "SCASB", "SCASW",
                 "SCASD", "CMPXCHG", "XCHG", "IN", "OUT", "SAHF", "PUSHAD", "PUSHA", "XLAT", "XLATB", "AAA", "AAS", "DAA",
                 "DAS", "AAM", "AAD"}
PARTIAL_WRITE = {"LAHF"}   # writes AH only: not a kill, not a read
_fn_cache, _callee_cache = {}, {}


def mnemonic(t):
    m = re.match(r"(\S+)\s*(.*)", t)
    op = m.group(1).upper().split(".")[0]   # Ghidra writes string-op prefixes as suffixes: MOVSD.REP, STOSD.REPNE
    return op, m.group(2)


def classify(op, args):
    """-> 'read' | 'write' | 'none' | 'unknown' for EAX, ignoring control flow (handled by the walkers)."""
    dst, _, src = args.partition(",")
    dst, src = dst.strip(), src.strip()
    if op in IMPLICIT_READ:
        return "read"
    if op in PARTIAL_WRITE:
        return "none"
    if op in ("MUL", "DIV", "IDIV") or (op == "IMUL" and "," not in args):
        return "read"   # one-operand forms read EDX:EAX / EAX
    if op == "IMUL":
        op = "ADD"      # two/three-operand IMUL behaves like an ALU op on its explicit operands
    if op.startswith("CMOV"):
        return "read" if re.search(EAXRE, args) else "none"   # conditional: dest kept (read) when not taken
    if op.startswith("SET"):
        return "none" if not re.search(EAXRE, dst) or "[" not in dst else "read"
    if op in ("FNSTSW", "FSTSW"):
        return "none" if dst in ("AX",) else ("read" if re.search(EAXRE, dst) else "none")   # writes AX only
    if op.startswith("F"):
        return "read" if re.search(EAXRE, args) else "none"   # x87: EAX only as an address register
    if op in ("XOR", "SUB", "SBB") and dst == "EAX" and src == "EAX":
        return "write" if op != "SBB" else "read"   # XOR/SUB EAX,EAX zero it; SBB EAX,EAX reads CF only, but keep safe
    if op not in EXPLICIT_ONLY:
        return "unknown"
    if re.search(EAXRE, src) or (re.search(EAXRE, dst) and "[" in dst):
        return "read"
    if dst == "EAX" and op in FULL_WRITE:
        return "write"
    if re.search(EAXRE, dst) and op not in FULL_WRITE and op != "PUSH" or op == "PUSH" and re.search(EAXRE, dst):
        return "read"   # ALU on EAX/AX/AL reads it; PUSH EAX reads it
    return "none"


def load_fn_cached(entry):
    if entry not in _fn_cache:
        try:
            _fn_cache[entry] = load_fn(entry)
        except OSError:
            _fn_cache[entry] = None
    return _fn_cache[entry]


def callee_reads_eax(target):
    """True when the callee (from the dump) reads EAX before writing it on some path (e.g. __chkstk, _alloca_probe).
    Its own CALLs count as clobbers and its RETs as 'no read' (a void callee does not consume EAX)."""
    if target in _callee_cache:
        return _callee_cache[target]
    _callee_cache[target] = False
    ins = load_fn_cached(target)
    if not ins:
        return False
    v, _ = walk_eax(ins, target, callee=True)
    _callee_cache[target] = v.startswith("LIVE")
    return _callee_cache[target]


def walk_eax(ins, at, callee=False):
    pos = {a: i for i, (a, b, t) in enumerate(ins)}
    seen, verdicts = set(), []
    def walk(addr, depth):
        while True:
            if addr not in pos: verdicts.append(f"unknown ({addr:#x} outside fn)"); return
            if addr in seen: return
            seen.add(addr)
            i = pos[addr]; a, b, t = ins[i]
            op, args = mnemonic(t)
            if op == "RET":
                if not callee: verdicts.append(f"LIVE (returned by {a:#x} {t})")
                return
            if op == "CALL":
                if re.search(EAXRE, args): verdicts.append(f"LIVE (read by {a:#x} {t})"); return
                mc = re.match(r"0x([0-9a-f]+)$", args.strip())
                if mc and not callee and callee_reads_eax(int(mc.group(1), 16)):
                    verdicts.append(f"LIVE (callee of {a:#x} {t} reads EAX)")
                return   # otherwise a CALL clobbers EAX
            if op == "JMP" and not re.match(r"0x[0-9a-f]+$", args.strip()):
                if re.search(EAXRE, args): verdicts.append(f"LIVE (read by {a:#x} {t})")
                else: verdicts.append(f"unknown (indirect jump {a:#x} {t})")
                return
            mj = re.match(r"(J\w+|LOOP\w*)\s+0x([0-9a-f]+)$", t)
            if not mj:
                c = classify(op, args)
                if c == "read": verdicts.append(f"LIVE (read by {a:#x} {t})"); return
                if c == "write": return
                if c == "unknown": verdicts.append(f"unknown (unmodelled {a:#x} {t})"); return
            else:
                if op.startswith("LOOP") or op == "JECXZ": pass
                if depth > 40: verdicts.append("unknown (depth)"); return
                walk(int(mj.group(2), 16), depth + 1)
                if mj.group(1) == "JMP": return
            if i + 1 >= len(ins): verdicts.append("unknown (end)"); return
            addr = ins[i + 1][0]
    walk(at, 0)
    if not verdicts: return "dead on all paths", False
    only_ret = all(v.startswith("LIVE (returned") for v in verdicts)
    return "; ".join(verdicts), only_ret


def eax_state(ins, at):
    """'dead on all paths' only if on EVERY path EAX is fully written before any (explicit or implicit) read.
    Returns (verdict, only_ret) where only_ret is True when every live verdict is a RET."""
    return walk_eax(ins, at)


# ---- ESP depth (review SF2: the consumed exit must be reached at the same stack depth) --------------------------------
# A full-function depth walk cannot attribute arguments around indirect (vtable) calls nested inside an argument push
# sequence, so the check is local: pending(X) = bytes already pushed at X that a later CALL will consume, found by
# walking forward from X to the next CALL whose argument size is known (direct: the callee's RET n, or the ADD ESP,n
# that follows a cdecl call; indirect: only the ADD ESP,n form). Reaching the epilogue (MOV ESP,EBP / LEAVE) first
# means ESP is re-derived from EBP, so any depth is safe there ("reset"). Every explored path must agree.
def callee_pops(target):
    ins = load_fn_cached(target)
    if not ins:
        return None
    pops = set()
    for a, b, t in ins:
        op, args = mnemonic(t)
        if op == "RET":
            m = re.match(r"0x([0-9a-f]+)", args.strip())
            pops.add(int(m.group(1), 16) if m else 0)
    return pops.pop() if len(pops) == 1 else None


def call_args(ins, i):
    """Bytes of arguments the CALL at index i consumes (callee pops or caller's ADD ESP), or None if not determinable."""
    a, b, t = ins[i]
    op, args = mnemonic(t)
    nop, nargs = mnemonic(ins[i + 1][2]) if i + 1 < len(ins) else ("", "")
    cdecl = None
    m = re.match(r"ESP,(0x[0-9a-f]+|\d+)$", nargs.replace(" ", "")) if nop == "ADD" else None
    if m:
        cdecl = int(m.group(1), 0)
    mc = re.match(r"0x([0-9a-f]+)$", args.strip())
    if mc:
        pops = callee_pops(int(mc.group(1), 16))
        if pops is not None:
            return pops + (cdecl or 0)
    return cdecl


def pending_at(ins, x, limit=80):
    """-> (set of results, notes). Results: int pending bytes, 'reset' (epilogue first), 'ret'."""
    pos = {a: i for i, (a, b, t) in enumerate(ins)}
    results, notes, seen = set(), [], set()
    work = [(x, 0, 0)]
    while work:
        addr, pushed, steps = work.pop()
        while True:
            if addr not in pos or steps > limit:
                notes.append(f"path from {x:#x} not resolved at {addr:#x}"); break
            if (addr, pushed) in seen: break
            seen.add((addr, pushed))
            i = pos[addr]; a, b, t = ins[i]
            op, args = mnemonic(t)
            dst = args.partition(",")[0].strip()
            steps += 1
            if op == "PUSH": pushed += 4
            elif op == "POP": pushed -= 4
            elif op in ("PUSHFD", "PUSHF"): pushed += 4
            elif op in ("POPFD", "POPF"): pushed -= 4
            elif op in ("SUB", "ADD") and dst == "ESP" and re.match(r"ESP,(0x[0-9a-f]+|\d+)$", args.replace(" ", "")):
                k = int(args.replace(" ", "").split(",")[1], 0)
                pushed += k if op == "SUB" else -k   # SUB ESP,n = an argument passed by value (e.g. a Vector)
            elif (op == "MOV" and dst == "ESP" and args.replace(" ", "") == "ESP,EBP") or op == "LEAVE":
                results.add("reset"); break
            elif op == "RET":
                results.add("ret" if pushed == 0 else f"ret with {pushed} pushed"); break
            elif re.search(r"\bESP\b", dst) and "[" not in dst:
                notes.append(f"ESP written at {a:#x} {t}"); break
            elif op == "CALL":
                n = call_args(ins, i)
                if n is None:
                    notes.append(f"argument size of {a:#x} {t} unknown"); break
                if pushed <= n:
                    results.add(n - pushed); break
                pushed -= n   # extra pushes belong to a later call (nested call inside an argument sequence)
                if i + 1 < len(ins) and mnemonic(ins[i + 1][2])[0] == "ADD" and ins[i + 1][2].replace(" ", "").startswith("ADDESP,"):
                    i += 1   # the cdecl cleanup was already counted in n
            mj = re.match(r"(J\w+|LOOP\w*)\s+0x([0-9a-f]+)$", t)
            if mj:
                work.append((int(mj.group(2), 16), pushed, steps))
                if mj.group(1) == "JMP": break
            elif op == "JMP":
                notes.append(f"indirect jump {a:#x}"); break
            if i + 1 >= len(ins): break
            addr = ins[i + 1][0]
    return results, notes


def depth_verdict(ins, x):
    """A single pending value (int) or 'reset'; None when unresolved/inconsistent. Returns (value, text)."""
    res, notes = pending_at(ins, x)
    nums = {r for r in res if isinstance(r, int)}
    txt = f"{sorted(map(str, res))}" + (f" notes={notes[:2]}" if notes else "")
    if len(nums) > 1 or any(isinstance(r, str) and r.startswith("ret with") for r in res):
        return None, txt
    if nums:
        return nums.pop(), txt
    if "reset" in res or "ret" in res:
        return "reset", txt
    return None, txt


def is_rel(b, txt):
    return bool(re.match(r"(J\w+|CALL|LOOP\w*)\s+0x", txt))


def owner_of(name):
    return OWNERS.get(name.split(" ", 1)[0], (None, None))


def toml_crosscheck(checked):
    """checked: {addr: (len, bytes, skip, cexit, name, shipped)} from the asm walk. Returns list of problem strings."""
    problems, lines = [], []
    for prefix, (pid, srcdir) in OWNERS.items():
        if not srcdir:
            continue
        path = os.path.join(SRC, srcdir, "kotor2-steam-aspyr.hooks.toml")
        if not os.path.exists(path):
            problems.append(f"{pid}: {path} missing"); continue
        hooks = tomllib.load(open(path, "rb")).get("hooks", [])
        mine = {a: v for a, v in checked.items() if v[4].startswith(prefix + " ") and v[5]}
        unshipped = {a: v for a, v in checked.items() if v[4].startswith(prefix + " ") and not v[5]}
        seen = set()
        for h in hooks:
            a = int(h["address"]); ob = bytes(h["original_bytes"])
            skip = bool(h.get("skip_original_bytes", False)); cexit = h.get("consumed_exit_address")
            excl = [x.lower() for x in h.get("exclude_from_restore", [])]
            if a in unshipped:
                problems.append(f"{pid}: toml hook {a:#x} ({h.get('function')}) is marked NOT shipped in WINDOWS"); continue
            if a not in mine:
                problems.append(f"{pid}: toml hook {a:#x} ({h.get('function')}) has no window in WINDOWS"); continue
            seen.add(a)
            ln, blob, wskip, wexit, wname, _ = mine[a]
            bad = []
            if ob != blob: bad.append("original_bytes differ from the asm")
            if len(ob) != ln: bad.append(f"length {len(ob)} != window {ln}")
            if skip != wskip: bad.append(f"skip_original_bytes {skip} != window {wskip}")
            if (cexit or None) != wexit: bad.append(f"consumed_exit {cexit} != window {wexit}")
            if cexit and "eax" not in excl: bad.append("consumed hook does not exclude eax from the restore")
            if not cexit and "eax" in excl: bad.append("non-consumed hook excludes eax (the handler result would leak into the game's EAX)")
            if bad: problems.append(f"{pid}: {a:#x} {h.get('function')}: " + "; ".join(bad))
            else: lines.append(f"  toml {pid} {a:#010x} {h.get('function')}: matches window '{wname}'")
        for a, v in mine.items():
            if a not in seen:
                problems.append(f"{pid}: window '{v[4]}' {a:#x} is not in {srcdir}/kotor2-steam-aspyr.hooks.toml")
    return problems, lines


def main():
    live = live_windows()
    cfg, cfg_note = live_config_windows()
    ok = True
    spans = []      # (start, end, name) of every window that resolved
    checked = {}
    print(f"live snapshot: {os.path.basename(LIVE)} ({len(live)} hooks); {cfg_note}")
    if not cfg:
        print("NOTE: only the 09-30g TSV is checked; it predates ranged-ai, character-effects-panel and inventory-sort-filter "
              "0x8A8478 (review N9)")
    for name, start, length, entry, skip, cexit, eax_ret_ok, shipped in WINDOWS:
        ins = load_fn(entry)
        addrs = [a for a, _, _ in ins]
        if start not in addrs:
            print(f"[FAIL] {name}: {start:#x} is not an instruction start in fn {entry:#x}")
            ok = False
            continue
        # walk whole instructions
        total, inside = 0, []
        for a, b, t in ins:
            if a >= start and total < length:
                inside.append((a, b, t))
                total += len(b)
        end = start + total
        problems = []
        if total != length:
            problems.append(f"length {length} is not on an instruction boundary (whole insns = {total})")
        if length < 5:
            problems.append("window < 5 bytes")
        rets = [i for i, (a, b, t) in enumerate(inside) if t.startswith("RET")]
        if rets and rets != [len(inside) - 1]:
            problems.append("RET inside the window is not the last stolen instruction")
        rel_in = [(a, t) for a, b, t in inside if is_rel(b, t)]
        if rel_in and not skip:
            problems.append("relative branch inside window: " + ", ".join(f"{a:#x} {t}" for a, t in rel_in))
        # external targets into the window interior
        for a, b, t in ins:
            m = re.match(r"(J\w+|LOOP\w*)\s+0x([0-9a-f]+)", t)
            if m:
                tg = int(m.group(2), 16)
                if start < tg < end and not (start <= a < end):
                    problems.append(f"external branch {a:#x} -> {tg:#x} lands inside window")
        own_id = owner_of(name)[0]
        for la, ln, pid, lname in live + cfg:
            if own_id and pid == own_id:
                continue  # the window's own installed hooks (window == hook)
            if la < end and start < la + ln:
                problems.append(f"overlaps live hook {la:#x}+{ln} ({lname})")
        if cexit is not None:
            if not skip:
                eaxuse = [f"{a:#x} {t}" for a, b, t in inside if re.search(r"\b(EAX|AX|AL|AH)\b", t)]
                if eaxuse:
                    problems.append("consumed hook whose stolen bytes touch EAX: " + "; ".join(eaxuse))
            if cexit not in addrs:
                problems.append(f"consumed exit {cexit:#x} is not an instruction start in fn {entry:#x}")
            for label, at in (("resume", end), ("consumed exit", cexit)):
                v, only_ret = eax_state(ins, at)
                accepted = v.startswith("dead") or (eax_ret_ok and label == "consumed exit" and only_ret)
                note = "  (accepted: handler result is the return value by design)" if accepted and not v.startswith("dead") else ""
                print(f"        EAX at {label} {at:#x}: {v}{note}")
                if not accepted:
                    problems.append(f"EAX may be live at {label} {at:#x} (EAX = handler result there)")
        if cexit is not None or skip:
            v_start, t_start = depth_verdict(ins, start)
            v_end, t_end = depth_verdict(ins, end)
            # the wrapper jumps to the consumed exit after the stolen bytes ran (stack = vanilla at the window end),
            # or, for a skip-only window, with the stack as at the window start
            v_jump = v_start if skip else v_end
            def compatible(u, v):
                return u is not None and v is not None and (u == v or "reset" in (u, v))
            if cexit is not None:
                v_exit, t_exit = depth_verdict(ins, cexit)
                print(f"        stack: pending at the jump ({'start' if skip else 'end'}) {v_jump}, at consumed exit {cexit:#x} {v_exit} {t_exit}")
                if not compatible(v_jump, v_exit):
                    problems.append(f"stack depth at the consumed exit {cexit:#x} ({v_exit}) != at the jump ({v_jump})")
            if skip:
                print(f"        stack: skip-only pending at start {v_start}, at resume {end:#x} {v_end} {t_end}")
                if not compatible(v_start, v_end):
                    problems.append(f"skip-only window: pending at the resume {end:#x} ({v_end}) != at the start ({v_start})")
        blob = b"".join(b for _, b, _ in inside)
        checked[start] = (total, blob, skip, cexit, name, shipped)
        spans.append((start, end, name))
        status = "OK  " if not problems else "FAIL"
        if problems:
            ok = False
        print(f"[{status}] {name}: {start:#010x} len {total}{' (skip-only)' if skip else ''}"
              f"{f' consumed->{cexit:#010x}' if cexit else ''}  bytes = [{', '.join('0x%02X' % x for x in blob)}]")
        for p in problems:
            print("        ! " + p)
        for a, b, t in inside:
            print(f"        {a:08x}  {b.hex():22} {t}")
    # windows against each other (ranged-ai vs companion-ai vs inventory, trace vs fixes)
    pair_problems = []
    for i in range(len(spans)):
        for j in range(i + 1, len(spans)):
            s1, e1, n1 = spans[i]; s2, e2, n2 = spans[j]
            if s1 < e2 and s2 < e1:
                pair_problems.append(f"window '{n1}' {s1:#x}..{e1:#x} overlaps window '{n2}' {s2:#x}..{e2:#x}")
    print(f"\nPairwise overlap among {len(spans)} windows: " + ("none" if not pair_problems else f"{len(pair_problems)} PROBLEM(S)"))
    for p in pair_problems:
        print("        ! " + p)
    ok &= not pair_problems
    tprob, tlines = toml_crosscheck(checked)
    print("\nTOML cross-check (source hooks.toml vs these windows): " + ("all hooks match" if not tprob else f"{len(tprob)} PROBLEM(S)"))
    for l in tlines:
        print(l)
    for p in tprob:
        print("        ! " + p)
    ok &= not tprob
    print("\nALL OK" if ok else "\nPROBLEMS FOUND")


if __name__ == "__main__":
    main()
